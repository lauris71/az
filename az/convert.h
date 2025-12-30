#ifndef __AZ_CONVERT_H__
#define __AZ_CONVERT_H__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

#include <az/az.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Checks whether a value of certain type can automatically be converted
 * 
 * True if:
 *   type is test
 *   both type and test are arithmetic and have automatic conversion rule
 * 
 * @param type the query
 * @param test the type to be tested against
 * @return 1 if type can be converted
 */
unsigned int az_type_is_convertible_to (unsigned int type, unsigned int test);

#ifdef __cplusplus
}
#endif

#endif
