# Boxed values and boxed interfaces — unified API rules

AZBoxedValue and AZBoxedInterface are recent additions and are not yet recognized
uniformly across the API. This document fixes the semantics and names the places
that need rework.

## The two boxes

| | AZBoxedValue | AZBoxedInterface |
|---|---|---|
| Purpose | **Size adapter**: wraps a value-type instance bigger than AZValue into a reference block | **Lifecycle anchor**: owns the container so an interface view stays valid |
| Content | the value instance (POD, freely copyable) | the *presented view* `(boxed->impl, boxed->inst)` |
| Extra state | `klass` | `val_impl`/`val` — the packed container value, "owns the lifecycle" (boxed-interface.h) |
| When it exists | only for value classes with `instance_size > AZ_VALUE_MAX_SIZE` (enforced in `az_boxed_value_new`) | whenever an interface handle must outlive the caller's container guarantee |

Both are ordinary **final reference types** (`AZ_FLAG_REFERENCE | AZ_FLAG_BOXED |
AZ_FLAG_VARIABLE_SIZE`), parent Reference.

**Minimum size.** The enforced minimum content size of a boxed value is the
AZValue size (16 bytes): `az_boxed_value_new*` rejects anything with
`instance_size <= AZ_VALUE_MAX_SIZE`, and a big value is boxed only when it does
not fit the destination slot. In particular **primitives are never boxed** —
including complex double, which sits at exactly 16 bytes and always fits an
AZValue.

## The four laws

**Law A — Invisibility.** Boxing is a storage detail. Every content-facing
operation on a boxed handle must yield the same result as the same operation on
the unboxed content. A boxed value *is* its content; a boxed interface *is* its
presented view. (This is already the documented conversion semantics: "a boxed
interface is a storage detail: its type is the contained interface type",
az/value.c; "Nothing is converted to AZ_TYPE_BOXED_VALUE", never a target.)

**Law B — Lifecycle.** The box is the lifecycle anchor. Anything that may
outlive the call keeps the box reference, never a naked `(impl, inst)` view of
it. Naked views are borrowed: valid while the caller's box lives.

Precisely: **a box always owns its container's lifetime** — a value container is
*copied* into the box, a reference container is *reffed* by it. Ownership is
only definable for the lifecycles az manages, so AZBoxedInterface is restricted
to value and reference containers. A boxed interface of a plain block would
promise safety it cannot deliver (it would hold a borrowed pointer), so plain
blocks follow the naked-interface rule instead: the caller manages the
lifetime. Equivalently: **boxed ⟺ az-managed lifetime; naked ⟺ caller-managed
lifetime** (plain blocks, static/immortal pairings, naked interfaces).

**Law C — Size law.** AZBoxedValue exists only because content did not fit a
slot. Where the storage knows it can hold any size (ValueStore / ValueArray data
pools), boxing must never persist: always unbox on input. Where the slot size is
fixed (AZValue 16B, AZPackedValue, caller buffers), keep or create boxes as
needed (the autobox policy).

Consequently every API output slot is an **`AZValue *dst, unsigned int dst_size`
pair**: the caller provides as big a buffer as it wants and larger results are
boxed automatically. The convenience containers AZValue64 and AZPackedValue64
remain as *structs only* (stack-allocating a 64-byte destination, passed as
`&x.value, sizeof (AZValue64)`); they must never appear as a parameter type.

**Law D — Shuffling opacity.** Value *movement* (init, clear, copy, transfer,
set into storage) is completely opaque to boxed interfaces: they move exactly
like ordinary reference types — the box pointer is copied/transfered and
ref/unref'd, never torn open. (Boxed values during shuffling follow Law C
instead: unbox if the target fits, keep the box otherwise. The asymmetry is
deliberate — value content is POD and safe to copy out, while an interface view
must stay anchored to its box.)

## API rules

### R0. Type level — opaque (no change)

Type codes see boxes as plain final reference types. `az_type_is_a`,
`az_type_is_assignable_to`, signature matching never see content — content is
per-instance. `AZ_TYPE_IS_BOXED` exists only for storage policies. Delegates
correctly return NULL for instance-less (type-level) queries.

### R1. impl/value API — `f(const AZImplementation *impl, const AZValue *val)`

Accepts both boxes; forwards to content (Law A):

- boxed value → `(boxed->klass->impl, &boxed->val)`
- boxed interface → the presented view `(boxed->impl, boxed->inst)`, borrowed
  for the duration of the call (Law B: the caller's value holds the box)

This applies to content-facing operations: to_string, equals, convert,
is/implements questions, function and property resolution, invoke-by-value,
`az_value_get_inst` family. Shuffling operations in this family obey Law D.

### R2. impl/inst API — `f(const AZImplementation *impl, void *inst)`

- **Boxed values are not part of the contract.** The caller unboxes first
  (`az_value_get_inst`); the content of a box is an ordinary value instance.
  Existing leniency (is_a/to_string/serialize forwarding for boxed values) may
  remain but is not required.
- **Boxed interfaces MUST be accepted** as `(AZ_BOXED_INTERFACE_IMPL, box)` and
  forwarded to the presented view. The box is the only lifecycle-safe handle to
  an interface instance (Law B) — callers must never have to degrade it to a
  naked view just to call something. If the callee stores a handle, it stores
  the box.

- **Lifecycle/identity operations never forward**: init, finalize, ref/unref,
  dispose, allocation always act on the box itself.

### R3. Naming — default is boxing-transparent

The default (unsuffixed) name obeys Law A: unbox at input, box at output when
needed. A representation-exact variant exists **only** where the caller knows
the content/size rules (Law C storage internals), suffixed `_raw`. The current
`_autobox` suffix conflates input transparency and output boxing and becomes the
default everywhere **except interface extraction** (R6), which keeps `_autobox`
with its distinct, documented meaning.

Pair inventory to flip (current raw → `_raw`, current `_autobox` → plain):
`az_value_init(_by_type)`, `az_value_transfer`, `az_value_copy`,
`az_value_set_from_inst(_by_type)`, `az_value_get_inst`, `az_value_equals`,
`az_value_equals_instance`, `az_value_convert(_in_place)`,
`az_value_to_string(_new)`, and the `az_packed_value_*`/`az_packed_value_64_*`
setters (`init`, `set_from_val`, `set_from_type`, `get_inst`, `set`).

### R4. ValueStore and ValueArray — always unbox incoming values

Both store arbitrary-length values in their data pools (Law C), so a mix of
boxed and plain representations of the same content has no reason to exist:

- write paths unwrap incoming AZBoxedValue to its content (store the true
  `klass->impl` and the content bytes in the pool);
- boxed **interfaces** are stored as-is — the box *is* the value form of
  interface-typed data (Law B/D); unwrapping would strip the lifecycle anchor;
- read paths copy into the caller's slot with the box-if-needed policy
  (`az_value_copy_autobox`-equivalent) — already correct.

azo already follows this policy (azo/stack.c: "plain value and stored as such.
Boxed interfaces are lifecycle guards and kept") and serves as the reference
implementation.

### R5. Property/interface/function lookups — delegation (verified, no change)

The delegate slots (`get_interface`, `lookup_property`, `lookup_function`,
`foreach_property`, `foreach_interface`) are implemented on both box classes and
forward to content, so all key/id lookups and enumerations are already agnostic.

Interface resolution is always well-defined: **an interface never has a
sub-interface of its own type**. Querying a view for the type it presents
returns self (the same view); any other hit is a genuine nested view. This is
what makes the "same box vs. sub-interface box" decision in R6 total.

### R6. Interface extraction — `_autobox` = box the output, if possible

`az_instance_get_interface[_autobox]` is the one family where `_autobox` has a
different meaning from everywhere else: it does not describe input handling, it
means **box the output, if possible** — the result is returned as a
lifecycle-safe AZBoxedInterface handle when a container is available.

The decision is one uniform question — **can the box own the container's
lifetime?** (Law B) — not an input-shape special case:

| Input handle | Output | Why |
|---|---|---|
| naked interface | naked (sub)interface | container unknown — cannot own |
| boxed interface | same box, or boxed sub-interface | ownership already held |
| boxed value | boxed (over a copy of the content) | value is copied — owned |
| value container | boxed (over a copy) | value is copied — owned |
| reference container | boxed (holds a reference) | reference is reffed — owned |
| plain block | naked (sub)interface | nothing to own — caller-managed |

- If the input handle is a **naked interface, it cannot be boxed**: a naked view
  carries no container, so there is nothing to anchor the lifecycle to. The
  result is the naked (sub)interface.
- A **plain block container also stays naked**: the box would hold a borrowed
  pointer while looking like a safe handle (see Law B). This is the same rule as
  for naked interfaces — both are caller-managed.
- The plain `az_instance_get_interface` remains the borrowed-view worker
  (result valid while the caller's `inst` lives) — that is the "we know the
  lifecycle" case (R3). Its `void **if_inst` result is always a naked pointer;
  the `_autobox` frontend instead fills a provided **`AZValue *dst`** — the
  handle may hold a reference (the box), so the output type itself tells the
  API user the result has to be cleared. This is the R3 output-slot convention
  applied to interface handles (`az_value_set_interface_autobox` is folded into
  this signature — identical semantics). The `_autobox` suffix is retained only
  in this family with the meaning defined here.

### R7. Parallel frontends — `az_instance_*` and `az_value_*` side by side

To make both levels equally usable, every content-facing operation should exist
both as an `az_instance_*` (impl/inst) and an `az_value_*` (impl/value) frontend
whenever it makes sense. Lifecycle (new/init/finalize/delete) and shuffling
(copy/transfer/set/clear) operations are family-specific by nature and do not
need parallels.

### R8. Interface virtual receivers are always naked

The `this` part of interface virtuals — e.g. `az_collection_get_size(impl,
inst)`, `az_list_get_element(...)`, the stream methods — is always a **naked**
view. Frontends resolve/unbox boxed handles (R2) before dispatch, so an
implementation never sees a boxed receiver.

Consequence: **no implementation may store the `(impl, inst)` receiver.** It is
borrowed for the duration of the call (Law B). Anything that needs to retain the
handle must re-anchor it: keep or obtain the boxed form, or own the container
itself.

### Content semantics table (derived from Laws A and D)

| Operation | Boxed value | Boxed interface |
|---|---|---|
| lookups, foreach, get_interface, is_a/implements | content | presented view |
| equals | content comparison | presented view comparison `(impl, inst)` |
| to_string | content string | presented view string |
| serialize | content | presented view |
| convert (autobox) | unbox at input, never a target | "its type is the contained interface type" |
| get_inst | content instance (borrowed) | presented view (borrowed) |
| shuffling (copy/transfer/set/clear) | unbox if fits, else box (Law C) | plain reference movement, opaque (Law D) |
| store into unbounded pool (R4) | always unbox | keep box |
| init/finalize/ref/unref | the box | the box |

## Audit: what already conforms

- Type-level opacity; `AZ_TYPE_IS_BOXED` for policy only (az/types.h:116)
- Minimum size enforced: `az_boxed_value_new*` asserts
  `instance_size > AZ_VALUE_MAX_SIZE` (az/boxed-value.c) — primitives never boxed
- Both delegate tables forward lookups/foreach; instance-less queries return
  NULL (az/boxed-value.c, az/boxed-interface.c) — R5
- `az_instance_is_a`/`implements` special-case both boxes ("the underlying type
  is checked") (az/instance.c:272-296) — R2
- `az_value_convert_autobox`: boxed values transparent; boxed interfaces keep
  the presented view, re-box on sub-interface resolution (az/value.c:183-253),
  pinned by `boxed-value` and `boxed-interface-conversion` tests
- Shuffling opacity for boxed interfaces: `az_value_copy`/`transfer`/`clear`
  treat them as plain references (block branch, no inspection) — Law D
- `az_value_transfer/copy/set_from_inst/init_autobox`: size-law box/unbox for
  boxed values (az/value.c) — R3 behavior (needs only the naming flip)
- `serialize_boxed_value`/`serialize_boxed_interface` forward to content
  (az/boxed-value.c:11, az/boxed-interface.c:26) — Law A
- Interface extraction semantics already match R6 (az/boxed-interface.c
  `az_instance_get_interface_autobox`: "Naked interface, no boxing possible",
  same-box ref, sub-interface re-box, boxed-value unbox-then-box)
- output-stream write/print paths unbox boxed stream interfaces
  (az/io/output-stream.c:47,57,70,94) — R2
- collection iterator helpers unbox boxed interfaces (az/collections/collection.c:56,72)
- value-array `transfer_element` and `set_element_from_val` unwrap incoming
  boxed values (az/classes/value-array.c:297,290) — R4
- `az_value_store_instance_at_autobox` unboxes defensively on reads
  (az/value-store.c:21) — R4 read side
- azo stack/datablock follow the policy (azo/stack.c:86-91,247-250)

## Rework list

1. **`boxed_interface_to_string` breaks Law A** — done: it now forwards to
   `az_instance_to_string(boxed->impl, boxed->inst)` (the presented view), like
   the boxed value does; the `"Boxed <iface> in <container>"` label is gone.
2. **`az_value_equals_autobox` / `az_value_equals_instance_autobox` break Law A
   for interfaces** (az/value.c): they unbox values but compare boxed interfaces
   by box identity, so two boxes presenting the same view compare unequal.
   Fix: also unbox/forward boxed interfaces to `(boxed->impl, boxed->inst)` and
   compare views (and mixed box-vs-naked comparisons).
3. **`az_value_get_inst_autobox` does not forward boxed interfaces**
   (az/value.c): returns the box (block passthrough). Fix: return the presented
   view, borrowed while the caller's value holds the box (document the
   lifetime). Callers that need the lifecycle anchor pass the box itself (R2).
   (This is content extraction, not shuffling — Law D does not apply here.)
4. **ValueStore write paths do not unwrap boxed values** — `az_value_store_set`
   and `az_value_store_transfer_in` (az/value-store.c:35,57) store the box if
   given one; same for `az_value_array_set_element` (az/classes/value-array.c:265).
   Fix: unwrap incoming AZBoxedValue to content at these entries (the
   transfer/from_val paths already do) so mixed representations cannot occur.
   Boxed interfaces pass through untouched (Law D).
5. **`az_packed_value_64_set_from_val_autobox` never unboxes** (packed-value.h:138):
   routes through plain `get_inst` + `set_from_inst_autobox`, so a boxed value
   whose content fits the 64-byte slot stays boxed. Fix: route through
   `az_value_copy_autobox(..., 64)` (unbox-if-fits). The 16-byte variant keeping
   the box is correct (content is > 16 bytes by construction).
6. **Naming flip (R3)**: default = boxing-transparent, `_raw` =
   representation-exact, across the `az_value_*`/`az_packed_value_*` pairs and
   all in-repo callers (az, azo, tests). The interface extraction family keeps
   `_autobox` (R6, output-boxing meaning).
7. **Missing parallel frontends (R7)**:
   - `az_value_serialize` (pair of `az_instance_serialize`) and
     `az_instance_deserialize` (pair of `az_value_deserialize`);
   - `az_value_get_interface[_autobox]` (pair of the instance extraction family,
     R6 semantics: box the output if possible, naked input cannot be boxed);
   - `az_value_is_a` / `az_value_implements` (pair of the instance predicates);
   - value-level property/function/foreach frontends
     (`az_value_get/set_property_by_key`, `az_value_get_function_by_key`,
     `az_value_foreach_property/_interface`) as thin wrappers over `get_inst`;
   - `az_instance_equals` to complete the equals family
     (`az_value_equals` = val/val, `az_value_equals_instance` = mixed).
8. **Restrict AZBoxedInterface to value/reference containers (Law B).** — done.
   The invariant "a box always owns its container" is enforced in the
   constructors (private.h), extraction obeys the R6 table, and the convert
   path only boxes value/reference sources (naked interfaces and plain blocks
   convert to borrowed views). Two existing muddles were fixed along the way:
   `az_boxed_interface_new` now accepts value containers (relocating the view
   into the box's copy) instead of rejecting them while accepting plain blocks,
   and `az_boxed_interface_new_from_impl_value` no longer mis-asserts on the
   source box's container kind. Public API is now the extraction frontends
   (`az_instance_get_interface[_autobox]`, `az_value_get_interface[_autobox]`,
   `az_value_set_interface_autobox`) plus handle management
   (`az_boxed_interface_ref/unref/unbox`); the constructors live in az/private.h.
9. **Ad-hoc unboxing copies**: output-stream has the same 3-line
   `az_boxed_interface_unbox` special case in 4+ places (az/io/output-stream.c),
   and instance.c/collection.c have their own. Once R2 is uniform, these become
   one shared helper (or disappear into the forwarding entry points).
10. **Minor**: `az_value_deserialize` should reject boxed impls explicitly
    (`AZ_TYPE_IS_BOXED(impl)` → `AZ_NOT_IMPLEMENTED`) — a box is storage, not a
    serializable type; and `function_build_arguments` (az/function.c) should stay
    as-is: va arguments arrive in their natural C form, boxing is a value-layer
    concern.

## Notes

- R2's asymmetry is deliberate: unboxing a boxed **value** is always safe (POD
  content, copy semantics), while unboxing a boxed **interface** yields a
  borrowed view whose container the caller no longer anchors — exactly the
  use-after-free AZBoxedInterface was introduced to prevent. Law D states the
  same asymmetry for shuffling.
- Law A ("boxing is invisible") is the master rule behind the to_string and
  equals fixes: if flipping a value between boxed and plain form changes any
  observable content result, the API is wrong. Shuffling is exempt only in the
  sense that it never looks inside a boxed interface at all (Law D) — the box is
  the datum.
- Why naked interfaces cannot be boxed at extraction time (R6): boxing requires
  the container (`az_boxed_interface_new` takes the container impl+inst), and a
  naked view does not know its container. Deriving it would require interface
  instances to live at a fixed offset of the container — true only "normally"
  (AGENTS.md), not guaranteed.
- **Safe naked interfaces exist** when both sides are immortal: a static
  implementation plus a permanent instance never needs a box. The canonical
  example is a class's `properties` list (az/base.c:243, `prop_list_impl` with
  the `AZClass` itself as the list instance): `prop_list_get_element` simply
  casts `list_inst` back to `AZClass *`, and the view is valid forever. It is
  deliberately not registered as a list type — a class *is not* a list; it only
  presents the list interface for that one property. The same pattern applies
  to any static/immortal pairing (e.g. handles to static interfaces, per the
  AZValue comment in az/value.h).
