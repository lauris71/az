#define __AZ_PRIVATE_C__

#include <string.h>

#include "private.h"

#if defined(AZ_GLOBALS_MULTI_THREAD)
	#include <arikkei/arikkei-threads.h>
#endif

#if defined(AZ_GLOBALS_STATIC)
	AZClass *az_types[AZ_MAX_TYPES];
	unsigned int az_num_types = 0;
	#define ensure_type()
#elif defined(AZ_GLOBALS_SINGLE_THREAD)
	AZClass **az_types = NULL;
	unsigned int az_num_types = 0;
	static unsigned int az_num_types_allocated = 0;
	static inline void ensure_type() {
		if (az_num_types >= az_num_types_allocated) {
			az_num_types_allocated += 32;
			az_types = (AZClass **) realloc (az_types, az_num_types_allocated * sizeof(AZClass *));
		}
	}
#elif defined(AZ_GLOBALS_MULTI_THREAD)
	static mtx_t mutex;
	static AZClass **az_types = NULL;
	static unsigned int az_num_types = 0;
	static unsigned int az_num_types_allocated = 0;
	static inline void ensure_type() {
		if (az_num_types >= az_num_types_allocated) {
			az_num_types_allocated += 32;
			az_types = (AZClass **) realloc (az_types, az_num_types_allocated * sizeof(AZClass *));
		}
	}
#endif

void
az_globals_init (void)
{
	if (az_num_types) return;

#if defined(AZ_GLOBALS_STATIC)
	// No action needed
#elif defined(AZ_GLOBALS_SINGLE_THREAD)
	az_num_types_allocated = AZ_NUM_BASE_TYPES + 32;
	az_types = (AZClass **) malloc (az_num_types_allocated * sizeof(AZClass *));
#elif defined(AZ_GLOBALS_MULTI_THREAD)
	mtx_init(&mutex, mtx_plain | mtx_recursive);
	az_num_types_allocated = AZ_NUM_BASE_TYPES + 32;
	az_types = (AZClass **) malloc (az_num_types_allocated * sizeof(AZClass *));
#endif
	memset(az_types, 0, AZ_NUM_BASE_TYPES * sizeof(AZClass *));

	az_num_types = AZ_NUM_BASE_TYPES;
}

void
az_register_class(AZClass *klass)
{
#if defined(AZ_GLOBALS_MULTI_THREAD)
	mtx_lock(&mutex);
#endif
	if (!klass->impl.type) {
		ensure_type();
		klass->impl.type = az_num_types++ | (klass->impl.flags & ~AZ_TYPE_MASK);
	}
	az_types[AZ_TYPE_INDEX(klass->impl.type)] = klass;
#if defined(AZ_GLOBALS_MULTI_THREAD)
	mtx_unlock(&mutex);
#endif
}

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
		unsigned int valid = (AZ_TYPE_INDEX(type) != 0) && (AZ_TYPE_INDEX(type) < az_num_types)  && (az_types[AZ_TYPE_INDEX(type)]->impl.type == type);
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
