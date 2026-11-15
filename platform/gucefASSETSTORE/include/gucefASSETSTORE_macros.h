/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_MACROS_H
#define GUCEF_ASSETSTORE_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */
#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */
#ifndef GUCEF_ASSETSTORE_CONFIG_H
#include "gucefASSETSTORE_config.h"
#define GUCEF_ASSETSTORE_CONFIG_H
#endif /* GUCEF_ASSETSTORE_CONFIG_H ? */
/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  When compiling the gucefASSETSTORE library GUCEF_ASSETSTORE_BUILD_MODULE is defined
 */
#undef GUCEF_ASSETSTORE_EXPORTSPEC
#ifdef GUCEF_ASSETSTORE_BUILD_MODULE
  #define GUCEF_ASSETSTORE_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_ASSETSTORE_EXPORTSPEC GUCEF_IMPORT
#endif /* GUCEF_ASSETSTORE_BUILD_MODULE ? */

/*-------------------------------------------------------------------------*/

#undef GUCEF_ASSETSTORE_PUBLIC_CPP
#ifdef GUCEF_ASSETSTORE_PUBLIC_CPP_CODE
  #define GUCEF_ASSETSTORE_PUBLIC_CPP GUCEF_ASSETSTORE_EXPORTSPEC
#else
  #define GUCEF_ASSETSTORE_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_ASSETSTORE_PUBLIC_CPP_CODE */

#undef GUCEF_ASSETSTORE_PUBLIC_C
#ifdef GUCEF_ASSETSTORE_PUBLIC_C_CODE
  #define GUCEF_ASSETSTORE_PUBLIC_C GUCEF_ASSETSTORE_EXPORTSPEC
#else
  #define GUCEF_ASSETSTORE_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_ASSETSTORE_PUBLIC_C_CODE */

#undef GUCEF_ASSETSTORE_PRIVATE_CPP
#define GUCEF_ASSETSTORE_PRIVATE_CPP GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_MACROS_H ? */
