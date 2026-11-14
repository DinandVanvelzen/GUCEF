/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 *      This is the main include header for the gucefCOMPUTE library.
 *      It is part of a framework called GUCEF.
 */

#ifndef GUCEF_COMPUTE_H
#define GUCEF_COMPUTE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library build configuration
 */
#ifndef GUCEF_COMPUTE_CONFIG_H
#include "gucefCOMPUTE_config.h"
#define GUCEF_COMPUTE_CONFIG_H
#endif /* GUCEF_COMPUTE_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_COMPUTE_MACROS_H
#include "gucefCOMPUTE_macros.h"
#define GUCEF_COMPUTE_MACROS_H
#endif /* GUCEF_COMPUTE_MACROS_H ? */

/*
 *      Simple types
 */
#ifndef GUCEF_COMPUTE_ETYPES_H
#include "gucefCOMPUTE_ETypes.h"
#define GUCEF_COMPUTE_ETYPES_H
#endif /* GUCEF_COMPUTE_ETYPES_H ? */

#ifdef __cplusplus

#ifndef GUCEF_COMPUTE_CMODULE_H
#include "gucefCOMPUTE_CModule.h"
#define GUCEF_COMPUTE_CMODULE_H
#endif /* GUCEF_COMPUTE_CMODULE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#include "gucefCOMPUTE_CComputeGlobal.h"
#define GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEGLOBAL_H ? */

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_H ? */
