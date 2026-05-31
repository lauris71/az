#define __AZ_PRIVATE_C__

#include <string.h>

#include "private.h"

#if defined AZ_GLOBALS_STATIC
AZTypeInfo az_types[AZ_MAX_TYPES];
unsigned int az_num_types = 0;
#elif defined AZ_GLOBALS_SINGLE_THREAD
AZTypeInfo *az_types = NULL;
unsigned int az_num_types = 0;
#elif defined AZ_GLOBALS_MULTI_THREAD
AZTypeInfo *az_types = NULL;
unsigned int az_num_types = 0;
#endif

static unsigned int classes_size = 0;

void
az_globals_init (void)
{
	if (az_num_types) return;
#if defined AZ_GLOBALS_STATIC
#else
	classes_size = AZ_NUM_BASE_TYPES + 32;
	az_classes = (AZClass **) malloc (classes_size * sizeof (AZClass *));
	az_types = (AZTypeInfo *) malloc (classes_size * sizeof (AZTypeInfo));
	az_classes[AZ_TYPE_INDEX(AZ_TYPE_NONE)] = NULL;
	az_num_classes = 1;
	memset (&az_types[AZ_TYPE_INDEX(AZ_TYPE_NONE)], 0, sizeof(AZTypeInfo));
#endif
}

void
az_register_class(AZClass *klass)
{
	if (!klass->impl.type) {
#if defined AZ_GLOBALS_STATIC
#else
		if (az_num_classes >= classes_size) {
			classes_size += 32;
			az_classes = (AZClass **) realloc (az_classes, classes_size * sizeof (AZClass *));
			az_types = (AZTypeInfo *) realloc (az_types, classes_size * sizeof(AZTypeInfo));
		}
#endif
		klass->impl.type = az_num_types++ | (klass->impl.flags & ~AZ_TYPE_MASK);
	}
	az_types[AZ_TYPE_INDEX(klass->impl.type)].klass = klass;
	/* We have to use class flags here because of parent chaining */
	az_types[AZ_TYPE_INDEX(klass->impl.type)].pidx = (klass->parent ? AZ_TYPE_INDEX(AZ_CLASS_TYPE(klass->parent)) : AZ_TYPE_NONE);
}

#ifdef ARIKKEI_MEMCHECK
void
arikkei_check_integrity (void)
{
#ifdef WIN32
    assert (_CrtCheckMemory());
#endif
}
#endif

