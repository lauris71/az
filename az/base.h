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

extern AZClass AZAnyClass;
/* Primitives */
extern AZClass AZBooleanClass;
extern AZClass AZInt8Class;
extern AZClass AZUint8Class;
extern AZClass AZInt16Class;
extern AZClass AZUint16Class;
extern AZClass AZInt32Class;
extern AZClass AZUint32Class;
extern AZClass AZInt64Class;
extern AZClass AZUint64Class;
extern AZClass AZFloatClass;
extern AZClass AZDoubleClass;
extern AZClass AZComplexFloatClass;
extern AZClass AZComplexDoubleClass;
extern AZClass AZPointerClass;
/* Fundamental */
extern AZClass AZStructClass;
extern AZClass AZBlockClass;
/* Base */
extern AZClass AZImplClass;
extern AZClass AZClassClass;

#ifdef __cplusplus
};
#endif

#endif
