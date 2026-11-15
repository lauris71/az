#define __AZ_REFERENCE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>

#include <arikkei/arikkei-utils.h>

#include <az/base.h>
#include <az/class.h>
#include <az/instance.h>
#include <az/private.h>
#include <az/reference.h>

#ifdef AZ_MT_REFERENCES
#include <arikkei/arikkei-threads.h>
#endif

#ifdef AZ_MT_REFERENCES
/*
 * Lifecycle R/W lock (docs/mt-reference-locking.md §6)
 *
 * Mutual exclusion between (many, tiny) weak->strong promotion windows and
 * the (rare) object memory release. Readers are concurrent; only the free
 * path takes the write side and drains. Flag/count accesses are seq_cst:
 * reader and writer each publish their own action before reading the other
 * side's, so one of them must observe the other (Dekker pattern).
 */
static mtx_t lc_mtx;
static cnd_t lc_cnd;
static _Atomic uint32_t lc_readers;
static _Atomic int lc_writer;

void
az_lc_read_lock (void)
{
	for (;;) {
		atomic_fetch_add_explicit (&lc_readers, 1, memory_order_seq_cst);
		if (!atomic_load_explicit (&lc_writer, memory_order_seq_cst)) return;
		/* Backing out of the window is a reader exit: it may be the drain
		 * transition the writer waits for, so it must go through the
		 * signaling read_unlock */
		az_lc_read_unlock ();
		mtx_lock (&lc_mtx);
		while (atomic_load_explicit (&lc_writer, memory_order_seq_cst)) cnd_wait (&lc_cnd, &lc_mtx);
		mtx_unlock (&lc_mtx);
	}
}

void
az_lc_read_unlock (void)
{
	if (atomic_fetch_sub_explicit (&lc_readers, 1, memory_order_seq_cst) == 1) {
		/* Last reader out: wake a potentially draining writer. Skipped when no
		 * writer is active - if its lc_writer store comes after our decrement
		 * (sc order), its readers-load must observe 0, so it never waits */
		if (atomic_load_explicit (&lc_writer, memory_order_seq_cst)) {
			mtx_lock (&lc_mtx);
			/* Broadcast, not signal: the cnd has two waiter classes (the writer
			 * and backoff readers), so a single wakeup can be stolen */
			cnd_broadcast (&lc_cnd);
			mtx_unlock (&lc_mtx);
		}
	}
}

void
az_lc_write_lock (void)
{
	mtx_lock (&lc_mtx);
	atomic_store_explicit (&lc_writer, 1, memory_order_seq_cst);
	while (atomic_load_explicit (&lc_readers, memory_order_seq_cst) != 0) cnd_wait (&lc_cnd, &lc_mtx);
}

void
az_lc_write_unlock (void)
{
	atomic_store_explicit (&lc_writer, 0, memory_order_seq_cst);
	cnd_broadcast (&lc_cnd);
	mtx_unlock (&lc_mtx);
}

#ifdef AZ_REFERENCE_HOOKS
void (*az_reference_ref_hook) (AZReference *) = NULL;
void (*az_reference_unref_hook) (AZReferenceClass *, AZReference *) = NULL;
#endif
#endif

static void az_reference_destroy_tail (AZReferenceClass *klass, AZReference *ref, unsigned int disposed);

#ifdef AZ_MT_REFERENCES
static void
az_reference_release (AZReferenceClass *klass, AZReference *ref, unsigned int allow_claim, unsigned int disposed)
{
	uint32_t n = atomic_load_explicit (&ref->refcount, memory_order_relaxed);
	for (;;) {
#ifdef AZ_SAFETY_CHECKS
		arikkei_return_if_fail (n != 0 && n != AZ_REFERENCE_DEAD);
#else
		if (n == 0 || n == AZ_REFERENCE_DEAD) return;
#endif
		if (n == 1) break;
		if (atomic_compare_exchange_weak_explicit (&ref->refcount, &n, n - 1, memory_order_acq_rel, memory_order_relaxed)) return;
	}
	/* n == 1: potentially the last reference. The drop callback gets its claim
	 * chance unlocked, exactly as before; revival is detected by the claim CAS */
	if (allow_claim && klass->drop) klass->drop (klass, ref);
	uint32_t expect = 1;
	if (!atomic_compare_exchange_strong_explicit (&ref->refcount, &expect, AZ_REFERENCE_DEAD, memory_order_acq_rel, memory_order_relaxed)) {
		/* Revived during drop (cache claim or promotion): release our reference.
		 * If the reviver's reference was itself released first, ours was the last
		 * one after all - then we are the disposer (count 0 is as promotion-proof
		 * as DEAD), and the destruction tail must run here */
		if (atomic_fetch_sub_explicit (&ref->refcount, 1, memory_order_acq_rel) == 1) {
			az_reference_destroy_tail (klass, ref, disposed);
		}
		return;
	}
	az_reference_destroy_tail (klass, ref, disposed);
}
#else
static void
az_reference_release (AZReferenceClass *klass, AZReference *ref, unsigned int allow_claim, unsigned int disposed)
{
	unsigned int n = ref->refcount;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (n != 0 && n != AZ_REFERENCE_DEAD);
#else
	if (n == 0 || n == AZ_REFERENCE_DEAD) return;
#endif
	if (n > 1) {
		ref->refcount = n - 1;
		return;
	}
	if (allow_claim && klass->drop) klass->drop (klass, ref);
	if (ref->refcount != 1) {
		/* Revived during drop: release our reference */
		ref->refcount -= 1;
		return;
	}
	ref->refcount = AZ_REFERENCE_DEAD;
	az_reference_destroy_tail (klass, ref, disposed);
}
#endif

static void
az_reference_destroy_tail (AZReferenceClass *klass, AZReference *ref, unsigned int disposed)
{
	/* refcount == AZ_REFERENCE_DEAD: unreachable for strong refs and promotions.
	 * Dispose and finalize run unlocked (both may cascade unrefs of other objects);
	 * only the memory release takes the write side of the lifecycle lock, which
	 * waits for in-flight promotion windows to drain */
	if (!disposed && klass->dispose) klass->dispose (klass, ref);
	az_instance_finalize (&klass->klass.impl, ref);
	az_lc_write_lock ();
	/* Class-based free: no registry lookup (and no registry mutex) inside the lock */
	az_instance_free_with_class (&klass->klass, ref);
	az_lc_write_unlock ();
}

void
az_reference_unref (AZReferenceClass* klass, AZReference* ref)
{
#ifdef AZ_REFERENCE_HOOKS
	if (az_reference_unref_hook) az_reference_unref_hook(klass, ref);
#endif
	az_reference_release (klass, ref, 1, 0);
}

void
az_reference_dispose (AZReferenceClass *klass, AZReference *ref)
{
#ifdef AZ_MT_REFERENCES
	uint32_t n0 = atomic_load_explicit (&ref->refcount, memory_order_relaxed);
#else
	uint32_t n0 = ref->refcount;
#endif
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (n0 != 0 && n0 != AZ_REFERENCE_DEAD);
#else
	if (n0 == 0 || n0 == AZ_REFERENCE_DEAD) return;
#endif
	/* We hold a reference, so no other thread can auto-dispose by unref */
	if (klass->dispose) klass->dispose (klass, ref);
	az_reference_release (klass, ref, 0, 1);
}

AZ_CLASS_ALIGN AZReferenceClass AZReferenceKlass = {
	.klass = {
		.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_CONSTRUCT | AZ_FLAG_REFERENCE | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_REFERENCE },
		.parent = &AZBlockKlass,
		.name = (const uint8_t *) "reference",
		.alignment = 7,
		.class_size = sizeof(AZReferenceClass),
		.instance_size = sizeof(AZReference),
		/* No instance_init: az_instance_init sets the refcount directly (AZ_FLAG_REFERENCE) */
		.to_string = az_any_to_string
	},
	.drop = NULL,
	.dispose = NULL
};

void
az_init_reference_class (void)
{
	az_class_new_with_value(&AZReferenceKlass.klass);
#ifdef AZ_MT_REFERENCES
	mtx_init (&lc_mtx, mtx_plain);
	cnd_init (&lc_cnd);
#endif
}
