/*
 *  ProjectGen: GUCEF module providing the business logic to generate module/project files
 *
 *  Copyright (C) 1998 - 2025.  Dinand Vanvelzen
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

#ifndef GUCEF_PROJECTGEN_MACROS_H
#define GUCEF_PROJECTGEN_MACROS_H

/*
 *      Build configuration specific macros.
 *      Also includes the generic macros which are build config independant.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"         /* generic often used macros */
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef GUCEF_PROJECTGEN_ETYPES_H
#include "gucefProjectGen_ETypes.h"      /* simple types */
#define GUCEF_PROJECTGEN_ETYPES_H
#endif /* GUCEF_PROJECTGEN_ETYPES_H ? */

#ifndef GUCEF_PROJECTGEN_CONFIG_H
#include "gucefProjectGen_config.h"      /* Module build configuration */
#define GUCEF_PROJECTGEN_CONFIG_H
#endif /* GUCEF_PROJECTGEN_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Auto detection of debug mode
 */
#ifndef GUCEF_PROJECTGEN_DEBUG_MODE
  #if defined( NDEBUG )
    #undef GUCEF_PROJECTGEN_DEBUG_MODE
  #else
    #if defined( _DEBUG ) || defined( __DEBUG ) || defined( __DEBUG__ )
      #define GUCEF_PROJECTGEN_DEBUG_MODE
    #else
      #undef GUCEF_PROJECTGEN_DEBUG_MODE
    #endif /* compiler DEBUG switches */
  #endif /* NDEBUG ? */
#endif /* GUCEF_INPUT_DEBUG_MODE ? */

/*-------------------------------------------------------------------------*/

/*
 *      When compiling the gucefVFS library you should define GUCEF_PROJECTGEN_BUILD_MODULE
 *      before including this macro file.
 */
#undef GUCEF_PROJECTGEN_EXPORTSPEC
#ifdef GUCEF_PROJECTGEN_BUILD_MODULE
  #define GUCEF_PROJECTGEN_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_PROJECTGEN_EXPORTSPEC GUCEF_IMPORT
#endif

/*-------------------------------------------------------------------------*/

#undef GUCEF_PROJECTGEN_PLUGIN_EXPORTSPEC
#ifdef GUCEF_PROJECTGEN_BUILD_PLUGIN_DLL
  #define GUCEF_PROJECTGEN_PLUGIN_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_PROJECTGEN_PLUGIN_EXPORTSPEC GUCEF_IMPORT
#endif

/*-------------------------------------------------------------------------*/

#undef GUCEF_PROJECTGEN_PUBLIC_CPP
#ifdef GUCEF_PROJECTGEN_PUBLIC_CPP_CODE
  #define GUCEF_PROJECTGEN_PUBLIC_CPP GUCEF_PROJECTGEN_EXPORTSPEC
#else
  #define GUCEF_PROJECTGEN_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_PROJECTGEN_PUBLIC_CPP_CODE */

#undef GUCEF_PROJECTGEN_PUBLIC_C
#ifdef GUCEF_PROJECTGEN_PUBLIC_C_CODE
  #define GUCEF_PROJECTGEN_PUBLIC_C GUCEF_PROJECTGEN_EXPORTSPEC
#else
  #define GUCEF_PROJECTGEN__PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_PROJECTGEN_PUBLIC_C_CODE */

#define GUCEF_PROJECTGEN_PRIVATE_CPP GUCEF_HIDDEN
#define GUCEF_PROJECTGEN_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#undef GUCEF_PROJECTGEN_PLUGIN_PUBLIC_CPP
#ifdef GUCEF_PROJECTGEN_PLUGIN_PUBLIC_CPP_CODE
  #define GUCEF_PROJECTGEN_PLUGIN_PUBLIC_CPP GUCEF_PROJECTGEN_PLUGIN_EXPORTSPEC
#else
  #define GUCEF_PROJECTGEN_PLUGIN_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_PROJECTGEN_PLUGIN_PUBLIC_CPP_CODE */

#undef GUCEF_PROJECTGEN_PLUGIN_PUBLIC_C
#ifdef GUCEF_PROJECTGEN_PLUGIN_PUBLIC_C_CODE
  #define GUCEF_PROJECTGEN_PLUGIN_PUBLIC_C GUCEF_PROJECTGEN_PLUGIN_EXPORTSPEC
#else
  #define GUCEF_PROJECTGEN_PLUGIN_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_PROJECTGEN_PLUGIN_PUBLIC_C_CODE */

#define GUCEF_PROJECTGEN_PLUGIN_PRIVATE_CPP GUCEF_HIDDEN
#define GUCEF_PROJECTGEN_PLUGIN_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 26-03-2005 :
       - Initial version of this file.

-----------------------------------------------------------------------------*/
