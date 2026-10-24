#ifndef __AZ_CONTEXT_H__
#define __AZ_CONTEXT_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2026
*/

#include <stdint.h>

#include <az/az.h>

/**
 * @brief The subsystem level that emitted the exception
 * 
 * Normally the highest-level subsystem should create the (extended) context. Whenever there is
 * an exception, each subsystem level knows whether it was emitted by itself or higher level (and
 * thus has subsystem-specific data set in context) or level below (and thus does not).
 */
#define AZ_EXCEPTION_LEVEL 0

/* Exception categories (each exception has associated category mask) */
/* System exception (out of memory memory etc.) */
#define AZ_EXCEPTION_CATEGORY_SYSTEM  0b0001
/* Runtime exceptions (invalid type, out of bound etc.) */
#define AZ_EXCEPTION_CATEGORY_RUNTIME 0b0010
/* IO exceptions (read/write errors, file not found etc.) */
#define AZ_EXCEPTION_CATEGORY_IO      0b0100

enum AZExceptionType {
    AZ_EXCEPTION_NONE,
    /**
     * @brief System exception
     * 
     * Some invariant in az library internals did not hold. Unrecoverable error.
     * 
     * Category: SYSTEM
     */
    AZ_EXCEPTION_SYSTEM,
    /**
     * @brief Memory allocation failed
     * 
     * Category: SYSTEM
     */
    AZ_EXCEPTION_OUT_OF_MEMORY,
    /**
     * @brief Null specified as a non-nullable instance
     * 
     * Category: RUNTIME
     */
    AZ_EXCEPTION_NULL_DEREFERENCE,
    /**
     * @brief The supplied instance type is not valid for given operation
     * 
     * Category: RUNTIME
     */
    AZ_EXCEPTION_INVALID_TYPE,
    /**
     * @brief The supplied instance value is invalid for the given operation
     * 
     * Category: RUNTIME
     */
    AZ_EXCEPTION_INVALID_VALUE,
    /**
     * @brief Index or offset is out of bounds
     * 
     * Category: RUNTIME
     */
    AZ_EXCEPTION_OUT_OF_BOUNDS,
    AZ_NUM_EXCEPTIONS
};

#ifdef __cplusplus
extern "C" {
#endif

/** @struct AZException
 *
 * Both AZException and AZContext have special handling rules.
 * They behave as final types for function signatures (e.g. no implementation specified), yet they
 * can be extended in dependent libraries. As this would break destructors they are left out of AZ type system atm.
 * Level indicates a level an which the exception is emitted. AZ_EXCEPTION_LEVEL indicates for possible users, that
 * the exception was emitted by az and thus does not contain any higher-level data, even if the context is actually higher
 * level.
 */
struct _AZException {
    /* The exception level */
    unsigned int level;
    /* The exception type */
    unsigned int type;
    /* The category mask */
    uint64_t mask;
    /* Message */
    AZString *msg;
};

void az_exception_set(AZException *exc, unsigned int type, uint64_t category, const uint8_t *msg);
void az_exception_clear(AZException *exc);

uint64_t az_get_exception_category(unsigned int type);

struct _AZContext {
    AZException exc;
};

#ifdef __cplusplus
};
#endif

#endif
