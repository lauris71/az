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
 * @brief Conversion types
 * 
 * A possible conversion from one type to another falls into one of the following categories:
 * 
 */
typedef enum AZConversionType {
    /**
     * @brief Value-preserving conversion is possible regardless of the actual value
     * 
     * Everything can be converted to itself, it's supertype and to an interface implemented by it.
     * Integers of smaller bit width can be automatically converted to larger ones. The larger
     * one has to fit the full value range of the smaller one.
     * E.g. int8 -> int16, uint8 -> int16, but NOT int8 -> uint16
     * Float can be converted to double, complex float or complex double
     * Double can be converted to complex double
     * Complex float can be converted to complex double
     * NONE can be converted to typed null, i.e. block type with null block pointer
     * (fixme: temporarily AUTO, should be EXPLICIT - has to be fixed in dependent projects first)
     * Result is always EXACT
     */
    AZ_CONVERT_AUTO,
    /** 
     * @brief Value-preserving conversion may be possible depending on actual value
     * 
     * All numeric types can be converted to each other given that the actual value is exactly representable by the target type.
     */
    AZ_CONVERT_CONDITIONAL,
    /**
     * @brief Conversion is possible but the value or type semantics may change
     * 
     * All numeric types can be converted to each other, result is either EXACT, ROUNDED or CLAMPED.
     * Uint64 can be converted to pointer and vice-versa.
     * Block types can be converted to pointers.
     */
    AZ_CONVERT_EXPLICIT,
    /**
     * @brief No conversion is possible
     * 
     * No conversion is possible between these types.
     */
    AZ_CANNOT_CONVERT
} AZConversionType;

typedef enum AZConversionResult {
    /**
     * @brief The conversion preserved the exact value
     *
     */
    AZ_CONVERSION_EXACT,
    /**
     * @brief The conversion resulted in rounding of value
     *
     */
    AZ_CONVERSION_ROUNDED,
    /**
     * @brief The conversion resulted in clamping of value
     *
     */
    AZ_CONVERSION_CLAMPED,
    /**
     * @brief The conversion was not possible
     *
     */
    AZ_CONVERSION_FAILED
} AZConversionResult;


/**
 * @brief Gets the conversion category between two types
 *
 * The caller checks the result against the required conversion level, e.g.
 * az_type_get_conversion_to (type, to_type) <= AZ_CONVERT_CONDITIONAL
 *
 * @param type the query
 * @param to_type the target type
 * @return the conversion category (AZ_CONVERT_AUTO ... AZ_CANNOT_CONVERT)
 */
AZConversionType az_type_get_conversion_to (unsigned int type, unsigned int to_type);

#ifdef __cplusplus
}
#endif

#endif
