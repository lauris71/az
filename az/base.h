#ifndef __AZ_BASE_H__
#define __AZ_BASE_H__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2016-2025
 */

 #include <az/class.h>

#ifdef __cplusplus
extern "C" {
#endif

extern AZClass AZAnyKlass;
/* Primitives */
extern AZClass AZBooleanKlass;
extern AZClass AZInt8Klass;
extern AZClass AZUint8Klass;
extern AZClass AZInt16Klass;
extern AZClass AZUint16Klass;
extern AZClass AZInt32Klass;
extern AZClass AZUint32Klass;
extern AZClass AZInt64Klass;
extern AZClass AZUint64Klass;
extern AZClass AZFloatKlass;
extern AZClass AZDoubleKlass;
extern AZClass AZComplexFloatKlass;
extern AZClass AZComplexDoubleKlass;
extern AZClass AZPointerKlass;
/* Fundamental */
extern AZClass AZStructKlass;
extern AZClass AZBlockKlass;
/* Base */
extern AZClass AZImplKlass;
extern AZClass AZClassKlass;

/**
 * @brief Fallback to_string method
 * 
 * Prints "Any" for pure Any, or "Instance of <class name> (<instance pointer>)" for subclasses
 * 
 * @param impl an implementation
 * @param inst an instance
 * @param d the destination buffer
 * @param d_len the destination buffer length
 * @return the number of bytes needed/written (including the terminating '\0')
 */
unsigned int az_any_to_string (const AZImplementation* impl, void *inst, unsigned char *d, unsigned int d_len);

#ifdef __cplusplus
};
#endif

#endif
