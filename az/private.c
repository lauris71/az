#define __AZ_PRIVATE_C__

#include <string.h>

#include "private.h"

static unsigned int classes_size = 0;

void
az_globals_init (void)
{
	if (az_num_types) return;
#if defined AZ_GLOBALS_FIXED_SIZE
#else
	classes_size = AZ_NUM_BASE_TYPES + 32;
	az_classes = (AZClass **) malloc (classes_size * sizeof (AZClass *));
	az_types = (AZTypeInfo *) malloc (classes_size * sizeof (AZTypeInfo));
	az_classes[AZ_TYPE_INDEX(AZ_TYPE_NONE)] = NULL;
	az_num_classes = 1;
	memset (&az_types[AZ_TYPE_INDEX(AZ_TYPE_NONE)], 0, sizeof(AZTypeInfo));
#endif
}

unsigned int
az_reserve_type()
{
#if defined AZ_GLOBALS_FIXED_SIZE
#else
	if (az_num_classes >= classes_size) {
		classes_size += 32;
		az_classes = (AZClass **) realloc (az_classes, classes_size * sizeof (AZClass *));
		az_types = (AZTypeInfo *) realloc (az_types, classes_size * sizeof(AZTypeInfo));
	}
#endif
	return az_num_types++;
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

