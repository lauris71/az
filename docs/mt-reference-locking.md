# Plan: streamlining reference locking

Status: plan (2026-11-15). Replaces the single global refcount mutex with
lock-free strong refs + a lifecycle R/W lock for weak-ref promotion and
final destruction.

## 1. Goals and non-goals

Goals:
- Ordinary `ref`/`unref` lock-free in MT builds (inline atomics; today's MT
  path is an out-of-line call + global mutex — the worst of all worlds).
- Weak→strong promotion that is *safe* in MT (today it is racy: read weak
  pointer, then incref — the object can be disposed/freed in between).
- Final destruction excludes in-flight promotions from freed memory.
- **Dispose (signals, resource release, cascading unrefs) stays out of any
  lock.** Only the memory release is lock-protected.
- Work both for azo (VM) and for az as a plain C library — no safepoints,
  no deferred reclamation requirement.

Non-goals:
- Changing ST (`!AZ_MT_REFERENCES`) behavior: plain increments, no locks,
  weak acquire = read + incref. All new machinery is MT-only.
- Deferred reclamation / epochs (fine for azo internally, wrong as a
  library-level requirement).
- Per-instance mutexes (memory blowup on small references) and per-class
  refcount locks (relocates the hotspot to the string lock; pointless once
  strong refs are atomic).

## 2. Current state (baseline)

- `AZReference` = `{ uint32_t refcount; }`. `refcount == 0` = immortal/static
  (ref skips incrementing; never disposed).
- MT: one file-static `mtx_t` guards all refcount ops of all types.
- Final unref (reference.c): if count > 1 → decrement, done. If count == 1 →
  call `drop` virtual **unlocked** (a resource cache may claim ownership by
  acquiring a new reference — *revival is a supported pattern*); re-check
  count; then `dispose` (signals — this is where weak pointers get cleared)
  and `az_instance_delete` (finalize + free) as separate calls.
- `AZWeakReference` = bare pointer, registered on the target's dispose
  signal at set time (`az/weak-reference.h`, targets `AZActiveObject`).
  Extraction = read pointer, then ref — **racy in MT** (the read→ref window
  vs a concurrent final drop).

## 3. Refcount word encoding

```
0          = invalid (never occurs: instances are born at 1)
1..0x7fff..= strong count
0xffffffff = DEAD (disposal claimed; promotion and revival impossible)
```

The legacy `refcount == 0` = immortal/permanent behavior is an artefact and
is **removed** — every reference instance is born at 1 (instance.c:118 sets
it inline in `az_instance_init`), nothing in the tree constructs or relies
on 0-permanent objects (verified by audit), and the defensive
`if (refcount)` skip in `az_reference_ref` goes away. Reading 0 is a bug:
assert in `AZ_SAFETY_CHECKS` builds.

The DEAD sentinel is claimed with a single atomic `CAS(1 → DEAD)` — exactly
one thread can win it, and it makes the whole destruction tail
revival-proof (today revival is possible during `drop`; after DEAD it is
impossible — an intentional semantic tightening, see §8).

## 4. Algorithms

Memory orders: `ref` relaxed; `unref`/CAS-decrement `acq_rel`; DEAD-claim
`acq_rel`; `try_ref` success `acquire`. (The thread observing the count
reach DEAD must see all prior writes to the object before disposing it.)

### ref

```c
void az_reference_ref (AZReference *ref)      /* MT, inline */
{
    uint32_t n = atomic_load_explicit (&ref->refcount, relaxed);
    for (;;) {
        /* 0 = invalid, DEAD = dead: assert in safety builds, no-op in release */
        if (n == 0 || n == DEAD) return;
        if (atomic_compare_exchange_weak (&ref->refcount, &n, n + 1)) return;
    }
}
```

### unref

```c
void az_reference_unref (AZReferenceClass *klass, AZReference *ref)
{
    uint32_t n = atomic_load_explicit (&ref->refcount, relaxed);
    for (;;) {
        if (n == 0) return;                            /* invalid (assert) */
        if (n == 1) goto slow;                         /* potential final ref */
        if (atomic_compare_exchange_weak (&ref->refcount, &n, n - 1)) return;
    }
slow:
    /* drop virtual gets its claim chance exactly as today (called unlocked) */
    if (klass->drop) klass->drop (klass, ref);   /* may revive (acquire refs) */
    /* Claim disposal rights: only one thread can turn 1 into DEAD. */
    uint32_t expect = 1;
    if (!atomic_compare_exchange_strong (&ref->refcount, &expect, DEAD)) {
        /* Someone revived (cache claim or promotion): release our ref.
         * If the reviver's ref was itself released first, ours was the last
         * one after all - we are the disposer (count 0 is promotion-proof
         * like DEAD), so the tail must run here. */
        if (atomic_fetch_sub_explicit (&ref->refcount, 1, acq_rel) == 1) {
            az_reference_destroy_tail (klass, ref, disposed);
        }
        return;
    }
    az_reference_destroy_tail (klass, ref, disposed);   /* §5 */
}
```

`az_reference_dispose` (public explicit dispose) keeps its contract (caller
holds a strong ref): run `dispose`, then release the reference through the
same slow path.

## 5. Destruction tail — dispose unlocked, only free under the write lock

```c
static void
az_reference_destroy_tail (AZReferenceClass *klass, AZReference *ref)
{
    /* refcount == DEAD: unreachable for strong refs and promotions */
    if (klass->dispose) klass->dispose (klass, ref);   /* UNLOCKED: signals
         fire, weak pointers cleared, resources released, cascading unrefs */
    az_instance_finalize (...);                          /* UNLOCKED */
    az_lc_write_lock ();                                 /* drain promoters */
    az_instance_free (...);                              /* memory release */
    az_lc_write_unlock ();
}
```

Why only the free is locked:

- A promoter inside its read-lock window touches the object only via its
  refcount word (`load` + CAS). The header stays valid through dispose and
  finalize; the word reads DEAD, so its CAS fails harmlessly. Only `free`
  (returning memory to the allocator/OS) makes the word invalid — so only
  `free` must exclude promoters.
- Finalize unrefs member references, which can recursively hit other
  objects' final drops → those take the write lock for their own frees.
  Holding our write lock across finalize would self-deadlock; locking only
  the free cannot deadlock (promoters never unref or block inside their
  window, and we hold no lock while disposing/finalizing).
- Signal handlers may unref/promote other objects — safe, no lock is held
  during dispose. (Rule, to be documented: handlers must not re-register
  weak references on the dying object. They can't strong-ref it — DEAD —
  so no new weak refs can be created post-claim.)

Requires splitting `az_instance_delete` into finalize + free entry points
(the two already exist conceptually in the drop path; expose
`az_instance_free` or an internal `delete_no_finalize`).

## 6. Lifecycle R/W lock

Purpose: mutual exclusion between (many, tiny) promotion windows and
(rare) memory release. C11-portable (`mtx_t` + `cnd_t`; no rwlock in C11 /
MSVC).

```c
static _Atomic uint32_t lc_readers;   /* active promoters */
static _Atomic int lc_writer;
static mtx_t lc_mtx;
static cnd_t lc_cnd;

/* Flag/count accesses are seq_cst (Dekker pattern): reader and writer each
 * publish their own action before reading the other side's, so one of them
 * must observe the other. acquire/release is NOT sufficient here. */
void az_lc_read_lock (void)           /* promotion window entry */
{
    for (;;) {
        atomic_fetch_add (&lc_readers, 1, seq_cst);
        if (!atomic_load (&lc_writer, seq_cst)) return;   /* fast path */
        az_lc_read_unlock ();   /* backoff IS a reader exit: must signal the drain */
        mtx_lock (&lc_mtx);
        while (atomic_load (&lc_writer, seq_cst)) cnd_wait (&lc_cnd, &lc_mtx);
        mtx_unlock (&lc_mtx);
    }
}
void az_lc_read_unlock (void)
{
    if (atomic_fetch_sub (&lc_readers, 1, seq_cst) == 1) {
        if (atomic_load (&lc_writer, seq_cst)) {   /* only signal when a writer is active */
            mtx_lock (&lc_mtx);
            cnd_broadcast (&lc_cnd);   /* two waiter classes -> broadcast, not signal */
            mtx_unlock (&lc_mtx);
        }
    }
}
void az_lc_write_lock (void)          /* only around free */
{
    mtx_lock (&lc_mtx);
    atomic_store (&lc_writer, 1, seq_cst);
    while (atomic_load (&lc_readers, seq_cst) != 0) cnd_wait (&lc_cnd, &lc_mtx);
}
void az_lc_write_unlock (void)
{
    atomic_store (&lc_writer, 0, seq_cst);
    cnd_broadcast (&lc_cnd);
    mtx_unlock (&lc_mtx);
}
```

Two signaling failure modes to keep out (found in review):
- *Missed drain wakeup*: any decrement of `lc_readers` to 0 outside
  `az_lc_read_unlock` (e.g. an open-coded backoff) leaves the writer asleep
  on `readers == 0` forever — all reader exits go through read_unlock.
- *Stolen wakeup*: the cnd has two waiter classes (writer on `readers == 0`,
  backoff readers on `lc_writer == 0`); a single `cnd_signal` may wake the
  wrong class — always `cnd_broadcast`.

Cost profile: promotion ≈ 2 atomic RMWs + CAS (+ an uncontended mutex on
the last-reader transition) — tens of cycles, no syscall. Free pays a
drain wait only while a promotion window is actually open (~ns).

Optional refinement (documented, not v1): per-class lifecycle locks in
`AZReferenceClass`, so an object's death only drains promoters of that
class. Requires the promotion API to know the expected class up front
(the weak ref's static type), so it changes the API shape — defer until
profiling says the global drain domain matters.

## 7. Promotion API

```c
unsigned int az_reference_try_ref (AZReference *ref);   /* MT */
/* CAS loop vs DEAD; fails on DEAD; 0 is invalid (assert in safety builds).
   Success acquires a strong reference (acquire ordering). */

AZActiveObject *az_weak_reference_acquire (AZWeakReference *weak);
/* MT: read_lock; p = weak->object; (p && try_ref(p)) ? p : NULL; read_unlock.
   ST: read + plain ref.
   Returns an owned strong reference or NULL (object died). */
```

All weak-ref extraction sites (`weak-object-list` element access,
active-object attribute lookups, future containers) must go through this
primitive — never open-coded read + ref. That is the whole point: one
audited place.

## 8. Semantic changes (intended, small)

- **No revival after disposal is claimed.** Today a `drop` virtual can
  resurrect; afterwards the count hits 0 and the object is gone. With DEAD,
  revival is possible *only* during the `drop` virtual (as designed for
  resource caches); the subsequent `CAS(1→DEAD)` re-validates, and if
  revival happened the claim fails and the object lives on. Public
  `az_reference_dispose` on a live object is unchanged.
- **The `refcount == 0` = permanent artefact is removed** (see §3):
  instances are always born at 1, so 0 is invalid and asserts in safety
  builds. Nothing in the tree relied on it.
- **Reference hooks** (`AZ_REFERENCE_HOOKS`) preserved, called from the
  same operations — but now *outside* any lock.

## 9. Phasing

**R1 — atomic strong refs. DONE (2026-11-15).** `_Atomic uint32_t refcount`
(MT only); algorithms of §4; DEAD sentinel; delete-split (§5) with the
lifecycle lock around free only; global refcount mutex deleted.
Implementation notes: `az_reference_drop` folded into
`az_reference_release` (private.h decl removed); `az_instance_delete` split
into `az_instance_finalize` + new public `az_instance_free`; public
`az_reference_dispose` passes a `disposed` flag so the tail does not
double-dispose; lifecycle lock ops are seq_cst (Dekker pattern) and
declared in private.h for R2.
*Exit: full suite passes (29/29, incl. `strings-mt`, `types-mt`,
5 repeated runs); MT ref/unref lock-free; ST variant compiles.*

**R2 — promotion. DONE (2026-11-15).** `az_reference_try_ref` (reference.h:
CAS loop vs DEAD with acquire ordering on success; plain check+increment in
ST), `az_weak_reference_acquire` (weak-reference.c: lc read lock + read ptr
+ try_ref in MT; plain read + ref in ST). Audit: `weak_object_list_get_element`
promoted via the same primitives in MT (was open-coded plain ref);
active-object attributes hold no weak refs (no change); `contains()` is
address-compare only (no change). `AZWeakReference.object` renamed to `ptr`
so remaining direct accesses fail at compile time in external code.
Functional test added (acquire promotes on live object; NULL after shutdown).
*Exit: no open-coded weak extraction left in the tree; 29/29 tests pass;
ST variant compiles.*

**R3 — hardening tests. DONE (2026-11-15).** New `refcount-mt` suite
(tests/test.c): ref/unref churn on shared objects/strings (atomicity);
deterministic promotion races via freeze points in the drop virtual and a
dispose listener (promote-before-claim wins with the revival path releasing
the unref's reference; promote-after-claim fails); drop-claim revival;
revival-leak storm asserting the fetch_sub==1 regression invariant (every
object ends alive or disposed exactly once); promotion storm (pre-death
acquires succeed, post-death fail, disposed exactly once).
*TSan caught one real race the design review missed*: dispose clears the
weak pointer unlocked, so the pointer *word* itself raced with
`az_weak_reference_acquire`'s read — fixed by making `AZWeakReference.ptr`
`_Atomic` in MT builds (the lifecycle lock protects the object memory, the
atomic word protects the pointer).
*Exit: 30/30 suites green in the normal build; 30/30 green under
`-fsanitize=thread` (build-tsan); ST variant compiles.*
Follow-up fix (same day): **the weak accessors were also racy.**
`az_weak_reference_clear` checked `ref->ptr` and then called
`az_active_object_remove_listener_by_data` — if the object died in between,
the remove touched the (possibly freed) callback block (UAF / heap
corruption; the dispose emission iterates and frees the callbacks array).
Fixed by promoting through `az_weak_reference_acquire` before touching the
object: promotion failure means the object is dead and its dispose has
already fired our listener, so there is nothing to remove.
`az_weak_reference_set` now just calls the fixed clear; the contract that
the caller holds a strong reference to the *new* object is documented
(it cannot die between the pointer store and the listener registration).
Deterministic test added (clear racing mid-dispose).

Known remaining pre-existing issue (out of scope): `AZWeakObjectList`'s
dispose handler mutates the entries array (`remove_object_internal`,
memmove/free) without list-level synchronization — concurrent
`get_element` is only protected for the object memory, not the array
mutation. Needs a per-list lock design if the list is ever used cross-thread.
The active-object *listener list* itself (callbacks array add/remove/emit)
likewise has no internal synchronization for concurrent mutation of a live
object's listeners — same per-object lock design question.

## 10. Risks and open questions

- **Allocator-internal locking is unchanged**: the lifecycle write lock
  protects *the object's header from promoters*, not the pool/freelist
  (the extend/pool allocator keeps its own synchronization, or TLS pools).
  Don't conflate the two.
- **cnd/mtx availability**: C11 `<threads.h>` — MSVC has it since VS2022
  17.8 (already our baseline for other features).
- **Read-side never blocks in practice**: windows are ~ns; writer
  starvation is a non-issue. If a pathological promotion storm appears,
  per-class locks (§6) are the answer, not a fairer lock.
- **ABA on freed-then-reused memory**: closed by construction — free
  happens only after all read windows drain, and weak pointers were
  cleared during dispose (before free), so no promoter can carry a
  pre-free pointer past the write lock.
