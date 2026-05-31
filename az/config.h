#ifndef __AZ_CONFIG_H__
#define __AZ_CONFIG_H__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2016
 */

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
 * Three variants of handling global type arrays:
 * AZ_GLOBALS_STATIC - use compile-time fixed size arrays (AZ_MAX_TYPES)
 * AZ_GLOBALS_SINGLE_THREAD - completely ignore concurrency 
 * AZ_GLOBALS_MULTI_THREAD - use mutex
 * 
 * The following macros are redefined depending on globals handling:
 * - AZ_CLASS_FROM_TYPE
 * - AZ_IMPL_FROM_TYPE
 */

#define AZ_GLOBALS_STATIC

/*
 * Use fixed-size static array
 */
#ifdef AZ_GLOBALS_STATIC
#ifndef AZ_MAX_TYPES
#define AZ_MAX_TYPES 256
#endif
#endif

#ifdef __cplusplus
};
#endif

#endif
