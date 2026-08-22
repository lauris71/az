#define __AZ_PRIVATE_C__

#include <string.h>

#include "private.h"

#if defined(AZ_GLOBALS_STATIC) || defined(AZ_GLOBALS_MULTI_THREAD)
	#include <arikkei/arikkei-threads.h>
#endif

#if defined(AZ_GLOBALS_STATIC)
	static mtx_t mutex;
	_Atomic(AZClass *) az_types[AZ_MAX_TYPES];
	_Atomic unsigned int az_num_types = 0;
	#define ensure_type()
#elif defined(AZ_GLOBALS_SINGLE_THREAD)
	AZClass **az_types = NULL;
	unsigned int az_num_types = 0;
	static unsigned int az_num_types_allocated = 0;
	static inline void ensure_type() {
		if (az_num_types >= az_num_types_allocated) {
			unsigned int old_allocated = az_num_types_allocated;
			az_num_types_allocated += 32;
			az_types = (AZClass **) realloc (az_types, az_num_types_allocated * sizeof(AZClass *));
			/* New slots have to be NULL: NULL denotes "reserved but not yet published" */
			memset (az_types + old_allocated, 0, 32 * sizeof (AZClass *));
		}
	}
#elif defined(AZ_GLOBALS_MULTI_THREAD)
	static mtx_t mutex;
	static AZClass **az_types = NULL;
	static unsigned int az_num_types = 0;
	static unsigned int az_num_types_allocated = 0;
	static inline void ensure_type() {
		if (az_num_types >= az_num_types_allocated) {
			unsigned int old_allocated = az_num_types_allocated;
			az_num_types_allocated += 32;
			az_types = (AZClass **) realloc (az_types, az_num_types_allocated * sizeof(AZClass *));
			/* New slots have to be NULL: NULL denotes "reserved but not yet published" */
			memset (az_types + old_allocated, 0, 32 * sizeof (AZClass *));
		}
	}
#endif

void
az_globals_init (void)
{
	if (az_num_types) return;

#if defined(AZ_GLOBALS_STATIC)
	/* Fixed-length array is zero-initialized statically */
	mtx_init(&mutex, mtx_plain | mtx_recursive);
#elif defined(AZ_GLOBALS_SINGLE_THREAD)
	az_num_types_allocated = AZ_NUM_BASE_TYPES + 32;
	az_types = (AZClass **) malloc (az_num_types_allocated * sizeof(AZClass *));
	/* All slots have to be NULL: NULL denotes "reserved but not yet published" */
	memset(az_types, 0, az_num_types_allocated * sizeof(AZClass *));
#elif defined(AZ_GLOBALS_MULTI_THREAD)
	mtx_init(&mutex, mtx_plain | mtx_recursive);
	az_num_types_allocated = AZ_NUM_BASE_TYPES + 32;
	az_types = (AZClass **) malloc (az_num_types_allocated * sizeof(AZClass *));
	/* All slots have to be NULL: NULL denotes "reserved but not yet published" */
	memset(az_types, 0, az_num_types_allocated * sizeof(AZClass *));
#endif

	az_num_types = AZ_NUM_BASE_TYPES;
}

void
az_register_class(AZClass *klass)
{
#if defined(AZ_GLOBALS_STATIC) || defined(AZ_GLOBALS_MULTI_THREAD)
	mtx_lock(&mutex);
#endif
	if (!klass->impl.type) {
		ensure_type();
		klass->impl.type = az_num_types++ | (klass->impl.flags & ~AZ_TYPE_MASK);
	}
	/* The slot is written only by az_class_publish once construction has finished */
#if defined(AZ_GLOBALS_STATIC) || defined(AZ_GLOBALS_MULTI_THREAD)
	mtx_unlock(&mutex);
#endif
}

void
az_class_publish (AZClass *klass)
{
#if defined(AZ_GLOBALS_STATIC)
#ifdef AZ_SAFETY_CHECKS
	/* A slot may only be empty or already contain this class (idempotent re-publish) */
	AZClass *old = atomic_load_explicit (&az_types[AZ_TYPE_INDEX(klass->impl.type)], memory_order_acquire);
	arikkei_return_if_fail (!old || old == klass);
#endif
	/* Release store pairs with the acquire load in the AZ_CLASS_FROM_TYPE fast path */
	atomic_store_explicit (&az_types[AZ_TYPE_INDEX(klass->impl.type)], klass, memory_order_release);
#elif defined(AZ_GLOBALS_SINGLE_THREAD)
#ifdef AZ_SAFETY_CHECKS
	/* A slot may only be empty or already contain this class (idempotent re-publish) */
	AZClass *old = az_types[AZ_TYPE_INDEX(klass->impl.type)];
	arikkei_return_if_fail (!old || old == klass);
#endif
	az_types[AZ_TYPE_INDEX(klass->impl.type)] = klass;
#elif defined(AZ_GLOBALS_MULTI_THREAD)
	mtx_lock(&mutex);
	AZClass **slot = &az_types[AZ_TYPE_INDEX(klass->impl.type)];
	/* A slot may only be empty or already contain this class (idempotent re-publish) */
	unsigned int ok = !*slot || (*slot == klass);
	if (ok) *slot = klass;
	mtx_unlock(&mutex);
	arikkei_return_if_fail (ok);
#endif
}

#if defined(AZ_GLOBALS_STATIC)
	AZClass *
	az_type_get_class (unsigned int type)
	{
		AZClass *ret = NULL;
		mtx_lock(&mutex);
		if (AZ_TYPE_INDEX(type) < az_num_types) ret = atomic_load_explicit (&az_types[AZ_TYPE_INDEX(type)], memory_order_acquire);
		mtx_unlock(&mutex);
#ifdef AZ_SAFETY_CHECKS
		arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) < az_num_types, NULL);
#endif
		return ret;
	}

	void
	az_types_lock()
	{
		mtx_lock(&mutex);
	}

	void
	az_types_unlock()
	{
		mtx_unlock(&mutex);
	}
#endif

#if defined(AZ_GLOBALS_MULTI_THREAD)
	AZClass *
	az_type_get_class (unsigned int type)
	{
		AZClass *ret = NULL;
		mtx_lock(&mutex);
		if (AZ_TYPE_INDEX(type) < az_num_types) ret = az_types[AZ_TYPE_INDEX(type)];
		mtx_unlock(&mutex);
#ifdef AZ_SAFETY_CHECKS
		arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) < az_num_types, NULL);
#endif
		return ret;
	}

	unsigned int
	az_type_is_valid(uint32_t type)
	{
		mtx_lock(&mutex);
		unsigned int valid = (AZ_TYPE_INDEX(type) != 0) && (AZ_TYPE_INDEX(type) < az_num_types)
			&& (!AZ_IMPL_FROM_TYPE(type) || (AZ_IMPL_FROM_TYPE(type)->type == type));
		mtx_unlock(&mutex);
		return valid;
	}

	void
	az_types_lock()
	{
		mtx_lock(&mutex);
	}

	void
	az_types_unlock()
	{
		mtx_unlock(&mutex);
	}
#endif
