# Migration plan: hybrid type/signature system

Status: analysis/plan (2026-11-10). Phase 0 is already implemented.

## 1. Goal

Let a 32-bit typecode denote either a plain type (as today) or a *signature* —
a parameterization of a head type by a list of component types:

```
list<String,int32>   =>   typecode [ IS_SIGNATURE | index ]  ->  signature node
```

Signature typecodes must flow through every place plain typecodes flow today
(function argument lists, field/property types, signature components, packed
value tags) without changing those structures, and nesting must be unbounded
(`myfunction(list<list<int32>>)`).

Non-goals for this migration:

- Changing the runtime value representation. Runtime values keep
  `impl = head class/implementation` with parameters reified per-instance
  (as `elem_impl` etc. today). Signature typecodes are *declared* types.
- Changing the native marshalling path (`az_function_call_native`,
  `AZFunctionSignature32`), which only needs the flat top-level projection.

## 2. Starting point (what exists)

- Signatures are interned, permanent, pool-allocated (`az/signature.c`):
  hash-consed via `ArikkeiDict`, `az_function_signature_delete` is a no-op
  macro, cap 255 args (`AZFunctionSignature255` template).
- Typecode-flag discipline: all flag *reads* go through
  `AZ_TYPE_IS_*`/`AZ_TYPE_HAS_CONSTRUCT` (typecodes) and
  `AZ_CLASS_IS_*`/`AZ_CLASS_HAS_CONSTRUCT` (classes); flag *writes* only in
  class definitions and `az_type_reserve`. Flag semantics can now be changed
  by touching only the macro layer and the setting sites.
- Typecode flag byte: used `BLOCK 0x01, INTERFACE 0x02, REFERENCE 0x04,
  OBJECT 0x10, FINAL 0x20, CONSTRUCT 0x80` (in `0xXX000000`);
  **free: `0x08000000` and `0x40000000`**.
- `AZFunctionSignature` is flat `{ret_type, n_args, arg_types[]}`; consumed by
  `az_class_define_method*`, function-valued property checks
  (`instance.c:468`), and native marshalling.
- Collections reify type parameters inconsistently:
  - in the *implementation* (class-level): `object-list`, `interface-list`,
    `hash-map`/`hash-set` (`key_impl`/`val_impl`), `array-list`;
  - in a **static class variable** overwritten per instance creation:
    `array-object` (`az_array_object_class->array_impl.elem_impl`) — broken
    for coexisting instances;
  - as **materialized classes** per parameter: `static-array-of`,
    `reference-of` (lazily registered, per-param subtype arrays).
- Data store migration (`datastore.h`/`value-store.h`) in progress:
  `AZValueArray` ported; fixes for `az_datastore_instance_at_autobox` and
  `az_value_array_transfer_element` already in.

## 3. Signature node format

Flat, interned, permanent, indexed in a global array:

```
struct AZSignature {
    uint32_t sig_type;       /* self typecode (IS_SIGNATURE | head flags | index);
                                alias nodes: the materialized class's plain typecode */
    uint32_t head_type;      /* plain typecode, e.g. AZ_TYPE_LIST */
    uint16_t sig_flags;      /* VARARG, PURE, ... */
    uint8_t  n_params;
    uint8_t  _reserved;
    uint32_t params[];       /* typecodes; may themselves be IS_SIGNATURE */
};
```

- **`sig_type` lives in the node, not the dict value** (mirrors
  `AZImplementation.type`). Memory: the interning dict entry stores key and
  value as one shared pointer (`key_offset == val_offset`), 16 bytes/entry
  (8 `next` + 8 ptr); making the value a distinct uint32 would round
  `entry_size = align16(...)` from 16 to 32 bytes — +16/signature vs +4/node
  for the u32 field. API: node → typecode is an O(1) field read (the reverse
  direction is the signature array's job); `intern()` returns `sig->sig_type`.
- **`sig_type` is not part of node identity.** Stack templates have
  `sig_type = 0`, interned nodes have it assigned; hash/compare must cover
  only `head/flags/n_params/params` (`sig_hash` must skip the field — it
  currently hashes the whole struct; `sig_equal` compares field-wise and must
  not grow the field). The stack-built `AZFunctionSignature32` in the native
  path leaves it unused.
- **Alias nodes (materialization hook)**: when the hook maps (head, params)
  to an existing class, still mint a dedup node, but with `sig_type` = the
  materialized class's *plain* typecode and **without** a signature-array
  slot. `AZ_TYPE_IS_SIGNATURE` is then false for aliases, so they flow
  through plain-type paths — correct, since a materialized class is a plain
  class.
- **Trees via indices**: components are typecodes, so a nested generic is just
  another interned node referenced by its typecode. Hash-consing is bottom-up
  (children interned first), so node hashing is `head + child pointers` — O(1).
- **Functions**: head = `AZ_TYPE_FUNCTION`, `params[0]` = return type,
  `params[1..]` = arguments. Variance is fixed by position for this head
  (return covariant, arguments contravariant). The existing flat
  `AZFunctionSignature {ret_type, n_args, arg_types[]}` layout is kept as the
  marshalling projection; the unified node is used at the *typing* layer.
  (Open decision: physically reuse `AZFunctionSignature` as the function-head
  node, or convert at the boundary. Leaning: keep both, one interning API.)
- **Varargs** (agreed model): `n_params` = fixed count; VARARG flag; one extra
  slot `params[n_params]` = tail element type; the actual count is a runtime
  argument of `invoke`/`convert_args_in_place`/`invoke_packed`. Tail args may
  individually be subtypes of the tail type (impls travel with values).
- **`n_params` is uint8_t (255)**: no realistic signature exceeds it (JVM and
  CPython cap at 255; current constructors already hard-fail above 64). The
  vararg *tail* is unbounded (runtime count).
- **Flags must join the interning key**: pure and impure variants of the same
  types are different signatures. Flag subtyping: a pure function satisfies a
  context expecting an impure one, not vice versa.

## 4. Typecode integration

- **`AZ_TYPE_IS_SIGNATURE = 0x40000000`**. Set ⇒ the low 24 bits index the
  global signature array, not the class array. `0x08000000` remains spare.
- **Signature indices are allocated top-down from `0xFFFFFF`**, classes grow
  up from 0. Benefits:
  - index-comparison macros (`AZ_TYPE_IS_BOXED`, `AZ_TYPE_IS_ARITHMETIC`,
    all `AZ_TYPE_IDX_*` equality/range tests) can never false-positive on a
    signature typecode, since builtin class indices are small;
  - a stray `AZ_CLASS_FROM_TYPE` on a signature typecode reads far out of
    bounds — a loud crash instead of a silently wrong class.
  - plus a debug assert (`!AZ_TYPE_IS_SIGNATURE(t)`) in the
    `AZ_CLASS_FROM_TYPE` fast path under `AZ_SAFETY_CHECKS`.
- **Copy the head's instance-nature flags** (BLOCK/REFERENCE/FINAL/CONSTRUCT/…)
  into the signature typecode at intern time. Then every flag-based dispatch
  (`AZ_TYPE_IS_BLOCK`, value-copy paths, the `AZ_TYPE_HAS_CONSTRUCT` gates in
  `instance.h`) keeps working unchanged — layout answers are per-head.
  Constraint: head must be registered before interning (natural order).
- **Accessors**:
  - `AZ_TYPE_IS_SIGNATURE(t)` (macro);
  - `az_signature_from_type(t)` → node (index into signature array);
  - `az_type_get_head_class(t)` → class of head, accepts plain and signature
    typecodes — for *layout* questions only.
  - `AZ_CLASS_FROM_TYPE` contract narrows to plain typecodes (with assert).
- **Interning API**: `az_signature_get_type(head, n_params, params,
  sig_flags) -> uint32_t typecode` (bottom-up interning, index assignment,
  MT-publish like `az_types`: release-store grown array, acquire-load readers,
  append-only indices).
- **Materialization hook** (decides the static-array-of duplication):
  generic heads may provide `materialize(head, n_params, params) -> typecode
  or 0`. Interning consults it first; if it yields an existing class (as
  `az_static_array_of_get_type` / `az_reference_of_get_type` would), the node
  becomes an alias node (§3): `sig_type` = that class's plain typecode, no
  signature-array slot. One identity per (head, params); new generics never
  materialize unless layout/behavior genuinely differs per instantiation.

## 5. The audit: `AZ_CLASS_FROM_TYPE` / `AZ_TYPE_INDEX` semantics

Library totals: **~50 `AZ_CLASS_FROM_TYPE` uses, ~20 `AZ_TYPE_INDEX` uses**
(excluding tests). Every use falls into exactly one of five categories:

| Category | Meaning w.r.t. signatures | Action |
|---|---|---|
| **A. Layout/dispatch** — instance size, alignment, value copy, init/finalize | Head class has the answer (per-head flags copied into typecode) | Route through macros / `az_type_get_head_class`; flag tests already safe |
| **B. Typing** — `az_type_is_a`, `implements`, conversions, `is_assignable_to`, property checks | Must be signature-aware (structural recursion) | Extend the typing engine (§6) |
| **C. Identity** — `AZ_TYPE_IS_BOXED`, `AZ_TYPE_IS_ARITHMETIC`, `AZ_TYPE_IDX_*` comparisons | Must *not* match signatures | Safe by construction (top-down indices) |
| **D. Naming/to_string** — `signature.c`, `class.c` dumps | Must recurse into components | Recursive printer (`list<int32>`) |
| **E. Registration/materialization** — `private.c`, `reference-of.c`, `static-array-of.c` | Plain typecodes only | Assert; untouched |

Per-file classification (library, current counts):

- `types.h` (6 CFT): macro layer itself — the assert lives here; `AZ_TYPE_IS_ABSTRACT` is category B.
- `class.c` (13): A (post_init layout, value-class checks), B (iface typing), D (name building).
- `function.c` (10): D (signature to_string), B (is_assignable_to, convert_args), A/E (invoke marshalling — stays plain).
- `instance.c` (7): A (init/finalize iface class lookup), B (`get_function_by_key`, property checks at :468).
- `reference-of.c` (7), `static-array-of.c` (1): E — materialization; later wired into the hook.
- `types.c` (3): B — `az_type_is_a` parent walk: must branch *before* any class lookup.
- `signature.c` (2): D.
- `private.c`, `interface.c`, `base.c`, `primitives.c`, `field.c`, `value.c`, `instance.h`, `private.h`, `config.h` (1–2 each): A/E.
- `AZ_TYPE_INDEX` uses are almost all C (safe) or E (subtype-array indexing in the two materializing generics — must learn to skip signature typecodes when a signature typecode can reach them; with the hook they get plain typecodes back).

**Audit rule**: after Phase 1, any *new* `AZ_CLASS_FROM_TYPE` call must be
justified against this table; the debug assert enforces it at runtime.

## 6. Typing semantics

`az_type_is_a(a, b)` — four cases:

| a \ b | plain b | signature b |
|---|---|---|
| **plain a** | as today | false (except: a is the materialization of b — handled by the hook returning identical typecodes) |
| **signature a** | `is_a(head_a, b)` — **raw-type rule**, keeps every existing unparameterized declaration accepting parameterized values | `is_a(head_a, head_b)` **and** per-component check under head_b's declared variance |

- **Variance is declaration-site**, a property of the generic class
  definition, not of each signature node. Default: invariant. For the
  function head: position 0 (return) covariant, positions 1..n contravariant.
  Placement: **not** a class-hierarchy change (Struct and Block both derive
  directly from Any; a common generic superclass would swallow everything
  except primitives — pointless). Instead, either a side table indexed by a
  `generic_idx` on `AZClass` (0 = not generic; mirrors the existing
  `allocator_idx`/`delegate_idx` pattern), or — see below — nothing at all,
  with variance expressed by *which stock assignability implementation* the
  class uses.
- **`az_type_is_assignable_to` is a class virtual, dispatched on the
  target's class** (the target defines what it accepts — Java's
  `Class.isAssignableFrom` precedent). Flag-gated with a structural default
  worker, consistent with the instance work-flag pattern:
  `AZ_CLASS_HAS_ASSIGNABLE` clear → default worker (nominal is_a + variance
  walk); set → the class's own rule. Discipline: dispatch once per (from,
  to) pair at the public entry; workers recurse into components *through the
  public entry* so nested classes with custom rules are consulted at every
  level. Consequences:
  - Function flag subtyping (pure ⇒ impure context OK, converse not) and
    argument contravariance become the Function class's virtual
    implementation, not special cases in a global checker.
  - Most generic classes need **no variance metadata**: default worker =
    invariant, one shared covariant implementation for read-only views.
    Variance becomes *which stock implementation a class uses*.
  - Vararg: strict initially (flags must match); the arity-flexible
    generalization `(int32, T...) → R` assignable to `(int32, T, T) → R`
    is the Function virtual's later extension.
- **Constraint types fall out as pseudo-heads** (future expansion, enabled
  by the virtual): `implements<List>` ("any object implementing List"),
  `super<Derived>` ("any supertype of Derived"), `extends<Node>`,
  `union<A,B>`, `nullable<T>` — each node's virtual is a one-line nominal
  query (`az_type_implements`, `az_type_is_a`). Guardrails: *declared
  positions only* (constraint types have no instances/layout/materialization;
  keep them out of the nominal `is_a` walk and `AZ_CLASS_FROM_TYPE` paths);
  *decidability preserved* (no F-bounded recursion à la Java's
  `T extends Comparable<T>` — every pseudo-head check reduces to one flat
  nominal query).
- **Conversions** (`az_type_get_conversion_to`/`convert.c`): signature
  typecodes participate by identity/subsumption only — the conversion table
  stays plain-only. No implicit conversions into/out of parameterizations
  (feature, not limitation).
- **Declared-vs-runtime boundary check**: assigning a runtime value to a
  signature-typed slot compares the *declared* component against the value's
  *runtime-reified* component (the instance's stored `elem_impl` etc.,
  surfaced via existing virtuals like `get_element_type`), variance-aware.
  This is the only place the two type universes meet; keep it in
  `az_type_is_assignable_to` + one instance-param query (§7).

## 7. Instance side: what can have a signature?

Requirement: *queriable like `az_function_get_signature` is today, but
generally.*

Option analysis:

- **Interfaces only** — add signature virtuals to the parameterized
  interfaces (Function already has one; Collection/List/Map get one).
  Cheap, but a standalone `list<int32>` value can't answer its signature
  except through an interface lens — and property/argument checking needs
  exactly the standalone answer. Rejected as the *only* mechanism (fine as
  an additional view).
- **All instances via AZClass** — no common non-primitive subclass exists,
  so support would have to live in `AZClass`/`AZImplementation` root.
  A per-instance or per-implementation field for everyone is the wrong cost
  model: the vast majority of classes are unparameterized.

**Recommended: one class virtual, class-specific provenance.**

The query must work from the bare `impl + inst` pair (that is all `invoke`
sees). The mechanism is a class-level virtual in the cold area of `AZClass`,
gated by `AZ_FLAG_HAS_SIGNATURE`:

```c
uint32_t (*signature) (const AZImplementation *impl, const void *inst);
```

The mechanism is uniform; the *provenance* of the answer is class-specific —
three provenance classes, both non-trivial ones already present in the
codebase:

- **Class-tied (materialized generics: `static-array-of`, `reference-of`).**
  The parameterization lives in the class; the class *is* the signature.
  Justified exactly where the parameter changes layout/behavior (element
  storage size, contained-type construction); anywhere the parameter is only
  a tag, materialization is waste (one implementation per variant, no
  behavioral gain). The virtual can answer with the class's own *plain*
  typecode — which is why the virtual returns a typecode, not a node.
- **Instance-tied (`FunctionValue` today; all collections after the §8
  cleanup).** One implementation serves all variants; the parameter lives in
  the instance. The signature typecode is interned **once at parameter-set
  time** and stored in the instance; the virtual returns the stored
  typecode — O(1), no dict lookup, no per-variant implementations.
  (This also fixes the `array-object` static-class-variable bug as a side
  effect of moving `elem_impl` into the instance.)
- **Unparameterized (default).** `HAS_SIGNATURE` clear, no virtual; generic
  code falls back to the trivial signature of the instance's own typecode
  (`impl->type`; for objects, the instance's class pointer).

Invoke-time verification:

```
declared = sig->params[i]                        (typecode, maybe IS_SIGNATURE)
actual   = HAS_SIGNATURE(class) ? class->signature(impl, inst)
                                : trivial_typecode_of(impl, inst)
check    = az_type_is_assignable_to(actual, declared)   /* fast path: equal typecodes */
```

The existing per-implementation `signature` virtual on
`AZFunctionImplementation` (function.h:67) becomes a *view* of this same
mechanism (the interface lens forwards to the underlying class's virtual) —
one mechanism, not two.

Cost: zero for unparameterized classes; no new common base class needed.

This preserves the core split: **signature typecodes are declared types;
instances reify parameters and *satisfy* declared signatures.** The query
direction (instance → signature) is available where it means something.

## 8. Collections cleanup (Phase 3; needed anyway)

1. Move `elem_impl`/`key_impl`/`val_impl` from *implementations* (class-level)
   to *instances*. Today two coexisting collections of the same class with
   different element types cannot work; `array-object` even mutates a static
   class variable at instance creation.
2. Finish the datastore/value-store backend migration (`AZValueArray` done;
   apply to the rest) so element storage is uniform.
3. Give each collection class a `signature` virtual (§7) and declare
   variance on the generic definitions (§6).
4. Rewire `static-array-of` / `reference-of` materialization through the
   interning hook (§4) instead of open-coded subtype arrays.

## 9. Phased plan

**Phase 0 — done.** Interned permanent signatures; flag discipline via
macros; `AZ_TYPE_HAS_CONSTRUCT`/`AZ_CLASS_HAS_CONSTRUCT`.

**Phase 1 — signature typecodes.**
`AZ_TYPE_IS_SIGNATURE`; global signature array with top-down indices;
node format (head/flags/n_params/params); interning API with bottom-up
hash-consing and flag copying; `AZ_TYPE_IS_SIGNATURE` macro;
`az_signature_from_type`, `az_type_get_head_class`; debug assert in
`AZ_CLASS_FROM_TYPE`; stats counter for table size.
*Exit: can mint and inspect `list<int32>`; nothing consumes it yet.*

**Phase 2 — typing engine.**
`az_type_is_a` four-case extension with raw-type rule; `az_type_is_assignable_to`
unification; recursive `to_string`; category-A audit fixes (the handful of
class lookups that can see signature typecodes).
*Exit: `is_a(list<int32>, collection<int32>)` true, `is_a(list<int32>,
list<float>)` false, `is_a(list<int32>, list)` true.*

**Phase 3 — collections + instance-side signatures.**
§8 cleanup; `AZ_FLAG_HAS_SIGNATURE` + class virtual; variance metadata on
generic definitions; boundary checks through `az_type_is_assignable_to`.
*Exit: a function declared `f(list<int32>)` rejects a runtime `list<float>`
value and accepts `list<int32>`; `az_instance_get_signature` answers
`list<int32>` for the instance.*

**Phase 4 — function types as signature nodes.**
Function head with params `[ret, args...]`; PURE/VARARG flags in the
interning key; `n_params` uint8 + vararg tail slot; `is_assignable_to`
generalized (return covariant, args contravariant, flag rules); property and
method checks (`instance.c:468`) routed through the engine. Marshalling
projection unchanged.
*Exit: function-typed properties can carry `pure`/`vararg` and generic
signatures.*

**Phase 5 — optional.**
Materialization hook for `static-array-of`/`reference-of`; read-only
interfaces with covariance (`list<subObj>` → `const list<Obj>`) as the
const-story alternative to C++-style const propagation.

Each phase is independently shippable; stopping after any phase leaves a
consistent system.

## 10. Risks and open questions

- **Silent `AZ_CLASS_FROM_TYPE` misuse** — the main hazard. Mitigations:
  top-down indices (loud crash + no index collisions), debug assert, the
  audit table (§5) as review checklist.
- **Serialization**: not a new problem class. Lazily assigned typecodes
  (`reference-of` subtype arrays, dynamically registered classes) are
  already process-local indices, so Aosora/Escher serialization already
  needs a stable encoding for them (by name or structure). Signature
  typecodes join the same solution: structural encoding (head + components)
  on write, re-interning on read. Plain stable typecodes are unaffected.
- **Boundary-check insertion points**: property setters, function argument
  conversion (`convert_args_in_place`/`invoke_packed`), field stores — must
  all route through `az_type_is_assignable_to`; audit in Phase 3.
- **Growth under code generation**: interning dedups, so growth is bounded
  by distinct shapes, not compilations; add the stats counter in Phase 1 and
  watch it (REPL/JIT loops are the only realistic growth source).
- **Flag budget**: IS_SIGNATURE consumes 0x40000000; 0x08000000 stays spare.
  If more are ever needed: nature-bit compaction (BLOCK/INTERFACE/REFERENCE/
  OBJECT → 3-bit field, frees 0x10000000) or index-space reclaim (24→22
  bits). Not needed now.
- **Open decisions**: (a) physically unify `AZFunctionSignature` with the
  function-head node or keep two formats with one interning API (leaning:
  keep both, unify at typing layer); (b) variance metadata: `generic_idx`
  side table vs nothing (leaning: stock assignability implementations make
  the table mostly unnecessary); (c) whether `AZ_TYPE_NONE`/`AZ_TYPE_ANY`
  are legal signature components (leaning: ANY yes, NONE only as function
  return).
