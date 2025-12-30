#define __AZ_BASE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <arikkei/arikkei-strlib.h>

#include <az/base.h>
#include <az/class.h>
#include <az/private.h>

static unsigned int
impl_to_string (const AZImplementation* impl, void *inst, unsigned char *buf, unsigned int len)
{
	AZImplementation *inst_impl = (AZImplementation *) inst;
	AZClass *inst_class = AZ_CLASS_FROM_IMPL(inst_impl);
	unsigned int pos = arikkei_memcpy_str (buf, len, inst_class->name);
	return pos + arikkei_strncpy (buf + pos, (len > pos) ? len - pos : 0, (const unsigned char *) " implementation");
}

static unsigned int
class_to_string (const AZImplementation* impl, void *inst, unsigned char *buf, unsigned int len)
{
	AZClass *inst_class = (AZClass *) inst;
	unsigned int pos = arikkei_memcpy_str (buf, len, inst_class->name);
	return pos + arikkei_strncpy (buf + pos, (len > pos) ? len - pos : 0, (const unsigned char *) " class");
}

AZClass AZStructKlass = {
	{AZ_FLAG_ABSTRACT | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_STRUCT},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "struct",
	3, sizeof(AZClass), 0,
	NULL,
	NULL, NULL,
	NULL, NULL, az_any_to_string,
	NULL, NULL
};

AZClass AZBlockKlass = {
	{AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_BLOCK},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "block",
	7, sizeof(AZClass), 0,
	NULL,
	NULL, NULL,
	NULL, NULL, az_any_to_string,
	NULL, NULL
};

AZClass AZImplKlass = {
	{AZ_FLAG_BLOCK | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_IMPLEMENTATION},
	&AZBlockKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "implementation",
	7, sizeof(AZClass), 0,
	NULL,
	NULL, NULL,
	NULL, NULL, impl_to_string,
	NULL, NULL
};

AZClass AZClassKlass = {
	{AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_CLASS},
	&AZImplKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "class",
	7, sizeof(AZClass), 0,
	NULL,
	NULL, NULL,
	NULL, NULL, class_to_string,
	NULL, NULL
};

void
az_init_base_classes (void)
{
	az_class_new_with_value(&AZStructKlass);
	az_class_new_with_value(&AZBlockKlass);
	az_class_new_with_value(&AZImplKlass);
	az_class_new_with_value(&AZClassKlass);
}

void
az_post_init_base_classes (void)
{
    az_class_post_init(&AZStructKlass);
    az_class_post_init(&AZBlockKlass);
    az_impl_class_post_init();
    az_class_post_init(&AZImplKlass);
    az_class_post_init(&AZClassKlass);
}
