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
 * AZ_GLOBALS_STATIC - fixed-size static array (AZ_MAX_TYPES); thread-safe
 *     registration with a lock-free AZ_CLASS_FROM_TYPE fast path
 * AZ_GLOBALS_SINGLE_THREAD - completely ignore concurrency 
 * AZ_GLOBALS_MULTI_THREAD - mutex-protected dynamically grown array, all
 *     class access goes through the locked az_type_get_class
 *
 * In all cases az_init() has to be called before spawning threads.
 * 
 * The following macros are redefined depending on globals handling:
 * - AZ_CLASS_FROM_TYPE
 * - AZ_IMPL_FROM_TYPE
 */

#if !defined(AZ_GLOBALS_STATIC) && !defined(AZ_GLOBALS_SINGLE_THREAD) && !defined(AZ_GLOBALS_MULTI_THREAD)
    //#define AZ_GLOBALS_STATIC
    //#define AZ_GLOBALS_SINGLE_THREAD
    #define AZ_GLOBALS_MULTI_THREAD
#endif

/*
 * Use fixed-size static array
 */
#ifdef AZ_GLOBALS_STATIC
    #ifndef AZ_MAX_TYPES
        #define AZ_MAX_TYPES 256
    #endif
#endif

#if !defined(AZ_GLOBALS_SINGLE_THREAD)
    #define AZ_MT_REFERENCES
#endif

#ifdef __cplusplus
};
#endif

#endif
