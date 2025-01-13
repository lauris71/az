#ifndef __AZ_BASE_H__
#define __AZ_BASE_H__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2016
 */

#include <arikkei/arikkei-utils.h>

#ifdef __cplusplus
extern "C" {
#endif
	
/*
 * Turn on runtime safety checks in library
 * Well-behaving implementation may disable these for speed
 */

#ifndef AZ_NO_SAFETY_CHECKS
#define AZ_SAFETY_CHECKS 1
#endif

/*
 * Disable specific parts of library
 */

#define _AZ_NO_STRING
#define _AZ_NO_BOXED_INTERFACE
#define _AZ_NO_VALUE
#define _AZ_NO_PROPERTIES

#ifndef AZ_NO_PACKED_VALUE
#define AZ_HAS_PACKED_VALUE
#endif

#ifndef AZ_NO_PROPERTIES
#define AZ_HAS_PROPERTIES
#endif

#ifdef __cplusplus
};
#endif

#endif
