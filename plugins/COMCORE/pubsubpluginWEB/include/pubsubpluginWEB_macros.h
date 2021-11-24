/*
 *  pubsubpluginWEB: Generic GUCEF COMCORE plugin for providing pubsub approximation via the WEB
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef PUBSUBPLUGIN_WEB_MACROS_H
#define PUBSUBPLUGIN_WEB_MACROS_H

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

#ifndef PUBSUBPLUGIN_WEB_CONFIG_H
#include "pubsubpluginWEB_config.h"
#define PUBSUBPLUGIN_WEB_CONFIG_H
#endif /* PUBSUBPLUGIN_WEB_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      When compiling the library you should define PUBSUBPLUGIN_WEB_BUILD_PLUGIN_DLL
 *      before including this macro file.
 */
#undef PUBSUBPLUGIN_WEB_PLUGIN_EXPORTSPEC
#ifdef PUBSUBPLUGIN_WEB_BUILD_PLUGIN_DLL
  #define PUBSUBPLUGIN_WEB_PLUGIN_EXPORTSPEC GUCEF_EXPORT
#else
  #define PUBSUBPLUGIN_WEB_PLUGIN_EXPORTSPEC GUCEF_IMPORT
#endif

/*-------------------------------------------------------------------------*/

#undef PUBSUBPLUGIN_WEB_PUBLIC_CPP
#ifdef PUBSUBPLUGIN_WEB_PUBLIC_CPP_CODE
  #define PUBSUBPLUGIN_WEB_PUBLIC_CPP PUBSUBPLUGIN_WEB_EXPORTSPEC
#else
  #define PUBSUBPLUGIN_WEB_PUBLIC_CPP GUCEF_HIDDEN
#endif /* PUBSUBPLUGIN_WEB_PUBLIC_CPP_CODE */

#undef PUBSUBPLUGIN_WEB_PUBLIC_C
#ifdef PUBSUBPLUGIN_WEB_PUBLIC_C_CODE
  #define PUBSUBPLUGIN_WEB_PUBLIC_C PUBSUBPLUGIN_WEB_EXPORTSPEC
#else
  #define PUBSUBPLUGIN_WEB_PUBLIC_C GUCEF_HIDDEN
#endif /* PUBSUBPLUGIN_WEB_PUBLIC_C_CODE */

#define PUBSUBPLUGIN_WEB_PRIVATE_CPP GUCEF_HIDDEN
#define PUBSUBPLUGIN_WEB_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#undef PUBSUBPLUGIN_WEB_PLUGIN_PUBLIC_CPP
#ifdef PUBSUBPLUGIN_WEB_PLUGIN_PUBLIC_CPP_CODE
  #define PUBSUBPLUGIN_WEB_PLUGIN_PUBLIC_CPP PUBSUBPLUGIN_WEB_PLUGIN_EXPORTSPEC
#else
  #define PUBSUBPLUGIN_WEB_PLUGIN_PUBLIC_CPP GUCEF_HIDDEN
#endif /* PUBSUBPLUGIN_WEB_PLUGIN_PUBLIC_CPP_CODE */

#undef PUBSUBPLUGIN_WEB_PLUGIN_PUBLIC_C
#ifdef PUBSUBPLUGIN_WEB_PLUGIN_PUBLIC_C_CODE
  #define PUBSUBPLUGIN_WEB_PLUGIN_PUBLIC_C PUBSUBPLUGIN_WEB_PLUGIN_EXPORTSPEC
#else
  #define PUBSUBPLUGIN_WEB_PLUGIN_PUBLIC_C GUCEF_HIDDEN
#endif /* PUBSUBPLUGIN_WEB_PLUGIN_PUBLIC_C_CODE */

#define PUBSUBPLUGIN_WEB_PLUGIN_PRIVATE_CPP GUCEF_HIDDEN
#define PUBSUBPLUGIN_WEB_PLUGIN_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_UDP_MACROS_H ? */

