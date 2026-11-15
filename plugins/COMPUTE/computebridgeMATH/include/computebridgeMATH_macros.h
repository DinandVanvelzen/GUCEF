/*
 *  computebridgeMATH: GUCEF compute bridge plugin of gucefMATH: multi-core and accelerator providers for the batch kernels
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

#ifndef GUCEF_COMPUTEBRIDGEMATH_MACROS_H
#define GUCEF_COMPUTEBRIDGEMATH_MACROS_H

#ifndef GUCEF_COMPUTE_MACROS_H
#include "gucefCOMPUTE_macros.h"
#define GUCEF_COMPUTE_MACROS_H
#endif /* GUCEF_COMPUTE_MACROS_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_CONFIG_H
#include "computebridgeMATH_config.h"
#define GUCEF_COMPUTEBRIDGEMATH_CONFIG_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_CONFIG_H ? */

#undef COMPUTEBRIDGEMATH_EXPORTSPEC
#ifdef COMPUTEBRIDGEMATH_BUILD_MODULE
  #define COMPUTEBRIDGEMATH_EXPORTSPEC GUCEF_EXPORT
#else
  #define COMPUTEBRIDGEMATH_EXPORTSPEC GUCEF_IMPORT
#endif /* COMPUTEBRIDGEMATH_BUILD_MODULE ? */

#undef COMPUTEBRIDGEMATH_PUBLIC_CPP
#ifdef COMPUTEBRIDGEMATH_PUBLIC_CPP_CODE
  #define COMPUTEBRIDGEMATH_PUBLIC_CPP COMPUTEBRIDGEMATH_EXPORTSPEC
#else
  #define COMPUTEBRIDGEMATH_PUBLIC_CPP GUCEF_HIDDEN
#endif /* COMPUTEBRIDGEMATH_PUBLIC_CPP_CODE ? */

#undef COMPUTEBRIDGEMATH_PUBLIC_C
#ifdef COMPUTEBRIDGEMATH_PUBLIC_C_CODE
  #define COMPUTEBRIDGEMATH_PUBLIC_C COMPUTEBRIDGEMATH_EXPORTSPEC
#else
  #define COMPUTEBRIDGEMATH_PUBLIC_C GUCEF_HIDDEN
#endif /* COMPUTEBRIDGEMATH_PUBLIC_C_CODE ? */

#endif /* GUCEF_COMPUTEBRIDGEMATH_MACROS_H ? */
