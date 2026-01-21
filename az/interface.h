#ifndef __AZ_INTERFACE_H__
#define __AZ_INTERFACE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

typedef struct _AZInterfaceValue AZInterfaceValue;
typedef struct _AZInterfaceClass AZInterfaceClass;

#include <az/class.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZInterfaceValue {
	AZImplementation* impl;
	void* inst;
};

/**
 * @brief An abstract base class of all interface types
 * 
 * All interface types (i.e. the ones whose Class and Implementation are separate) have to
 * derive from this class.
 * 
 */
struct _AZInterfaceClass {
	AZClass klass;
	/**
	 * @brief The size of the implementation
	 * 
	 */
	unsigned int implementation_size;
	/**
	 * @brief The implementation constructor
	 * 
	 */
	void (*implementation_init) (AZImplementation *impl);
};

extern AZInterfaceClass AZInterfaceKlass;

/**
 * @brief Get new typecode, allocate and initialize a class structure
 * 
 * Gets a new typecode, initializes its class to the minimal functional state and then calls class_init,
 * where a more specialized setup (e.g. registering interfaces and fields) should be done. The final setup
 * is performed only after class_init returns (and thus the sub-interfaces and fields are known).
 * 
 * @param type pointer where the typecode will be written
 * @param name the name of the new type
 * @param parent_type the parent typecode
 * @param class_size the size of the class
 * @param implementation_size the size of the implementation
 * @param instance_size the size of the instance
 * @param flags type flags
 * @param class_init the class constructor
 * @param implementation_init the implementation constructor
 * @param instance_init the instance constructor
 * @param instance_finalize the instance destructor
 * @return allocated and initialized class
 */
AZInterfaceClass *az_register_interface_type (unsigned int *type, const unsigned char *name, unsigned int parent_type,
	unsigned int class_size, unsigned int implementation_size, unsigned int instance_size, unsigned int flags,
	void (*class_init) (AZClass *),
	void (*implementation_init) (AZImplementation *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *));

/* Initialize a new implementation for interface */
void az_implementation_init_by_type (AZImplementation *impl, unsigned int type);

#ifdef __cplusplus
};
#endif

#endif
