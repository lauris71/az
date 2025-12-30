#ifndef __AZ_INSTANCE_H__
#define __AZ_INSTANCE_H__

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

void az_instance_init (const AZImplementation *impl, void *inst);
void az_instance_finalize (const AZImplementation *impl, void *inst);

#ifdef __cplusplus
}
#endif

#endif
