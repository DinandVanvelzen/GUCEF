/*
 *  pubsubpluginFIX: Generic GUCEF COMCORE plugin for providing pubsub via FIX protocol
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef PUBSUBPLUGIN_FIX_MACROS_H
#define PUBSUBPLUGIN_FIX_MACROS_H

/*
 *      Build configuration specific macros.
 *      Also includes the generic macros which are build config independant.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef PUBSUBPLUGIN_FIX_CONFIG_H
#include "pubsubpluginFIX_config.h"
#define PUBSUBPLUGIN_FIX_CONFIG_H
#endif /* PUBSUBPLUGIN_FIX_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      When compiling the library you should define PUBSUBPLUGIN_FIX_BUILD_PLUGIN_DLL
 *      before including this macro file.
 */
#undef PUBSUBPLUGIN_FIX_PLUGIN_EXPORTSPEC
#ifdef PUBSUBPLUGIN_FIX_BUILD_PLUGIN_DLL
  #define PUBSUBPLUGIN_FIX_PLUGIN_EXPORTSPEC GUCEF_EXPORT
#else
  #define PUBSUBPLUGIN_FIX_PLUGIN_EXPORTSPEC GUCEF_IMPORT
#endif

/*-------------------------------------------------------------------------*/

#undef PUBSUBPLUGIN_FIX_PUBLIC_CPP
#ifdef PUBSUBPLUGIN_FIX_PUBLIC_CPP_CODE
  #define PUBSUBPLUGIN_FIX_PUBLIC_CPP PUBSUBPLUGIN_FIX_EXPORTSPEC
#else
  #define PUBSUBPLUGIN_FIX_PUBLIC_CPP GUCEF_HIDDEN
#endif /* PUBSUBPLUGIN_FIX_PUBLIC_CPP_CODE */

#undef PUBSUBPLUGIN_FIX_PUBLIC_C
#ifdef PUBSUBPLUGIN_FIX_PUBLIC_C_CODE
  #define PUBSUBPLUGIN_FIX_PUBLIC_C PUBSUBPLUGIN_FIX_EXPORTSPEC
#else
  #define PUBSUBPLUGIN_FIX_PUBLIC_C GUCEF_HIDDEN
#endif /* PUBSUBPLUGIN_FIX_PUBLIC_C_CODE */

#define PUBSUBPLUGIN_FIX_PRIVATE_CPP GUCEF_HIDDEN
#define PUBSUBPLUGIN_FIX_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#undef PUBSUBPLUGIN_FIX_PLUGIN_PUBLIC_CPP
#ifdef PUBSUBPLUGIN_FIX_PLUGIN_PUBLIC_CPP_CODE
  #define PUBSUBPLUGIN_FIX_PLUGIN_PUBLIC_CPP PUBSUBPLUGIN_FIX_PLUGIN_EXPORTSPEC
#else
  #define PUBSUBPLUGIN_FIX_PLUGIN_PUBLIC_CPP GUCEF_HIDDEN
#endif /* PUBSUBPLUGIN_FIX_PLUGIN_PUBLIC_CPP_CODE */

#undef PUBSUBPLUGIN_FIX_PLUGIN_PUBLIC_C
#ifdef PUBSUBPLUGIN_FIX_PLUGIN_PUBLIC_C_CODE
  #define PUBSUBPLUGIN_FIX_PLUGIN_PUBLIC_C PUBSUBPLUGIN_FIX_PLUGIN_EXPORTSPEC
#else
  #define PUBSUBPLUGIN_FIX_PLUGIN_PUBLIC_C GUCEF_HIDDEN
#endif /* PUBSUBPLUGIN_FIX_PLUGIN_PUBLIC_C_CODE */

#define PUBSUBPLUGIN_FIX_PLUGIN_PRIVATE_CPP GUCEF_HIDDEN
#define PUBSUBPLUGIN_FIX_PLUGIN_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_FIX_MACROS_H ? */
