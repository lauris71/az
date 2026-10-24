#define __AZ_CONTEXT_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2026
*/

#include <stdlib.h>
#include <assert.h>

#include <az/context.h>
#include <az/string.h>

#ifdef __cplusplus
extern "C" {
#endif

void
az_exception_set(AZException *exc, unsigned int type, uint64_t category, const uint8_t *msg)
{
    exc->level = AZ_EXCEPTION_LEVEL;
    exc->type = type;
    exc->mask = category;
    if (exc->msg) az_string_unref(exc->msg);
    exc->msg = az_string_new(msg);
}

void
az_exception_clear(AZException *exc)
{
    if (exc->msg) {
        az_string_unref(exc->msg);
        exc->msg = NULL;
    }
    exc->level = 0;
    exc->type = 0;
    exc->mask = 0;
}

static uint64_t exc_categories[] = {
    0,
    AZ_EXCEPTION_CATEGORY_SYSTEM,
    AZ_EXCEPTION_CATEGORY_SYSTEM,
    AZ_EXCEPTION_CATEGORY_RUNTIME,
    AZ_EXCEPTION_CATEGORY_RUNTIME,
    AZ_EXCEPTION_CATEGORY_RUNTIME,
    AZ_EXCEPTION_CATEGORY_RUNTIME
};

static_assert(sizeof(exc_categories) / sizeof(exc_categories[0]) == AZ_NUM_EXCEPTIONS, "Exception category names out of sync with categories");

uint64_t
az_get_exception_category(unsigned int type)
{
    if (type < sizeof(exc_categories)/sizeof(exc_categories[0])) {
        return exc_categories[type];
    }
    return 0;
}

#ifdef __cplusplus
};
#endif

