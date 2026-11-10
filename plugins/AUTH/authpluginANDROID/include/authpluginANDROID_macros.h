/*
 *  authpluginANDROID: gucefAUTH plugin for logging in on Android devices
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

#ifndef GUCEF_AUTHPLUGIN_ANDROID_MACROS_H
#define GUCEF_AUTHPLUGIN_ANDROID_MACROS_H

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

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CONFIG_H
#include "authpluginANDROID_config.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CONFIG_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      When compiling the plugin GUCEF_AUTHPLUGIN_ANDROID_BUILD_MODULE is defined.
 *      The provider class is exported as well so that the tests can link it.
 */
#undef GUCEF_AUTHPLUGIN_ANDROID_EXPORT
#ifdef GUCEF_AUTHPLUGIN_ANDROID_BUILD_MODULE
  #define GUCEF_AUTHPLUGIN_ANDROID_EXPORT GUCEF_EXPORT
#else
  #define GUCEF_AUTHPLUGIN_ANDROID_EXPORT GUCEF_IMPORT
#endif /* GUCEF_AUTHPLUGIN_ANDROID_BUILD_MODULE ? */

#undef GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP
#ifdef GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP_CODE
  #define GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP GUCEF_AUTHPLUGIN_ANDROID_EXPORT
#else
  #define GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP
#endif /* GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP_CODE */

#undef GUCEF_AUTHPLUGIN_ANDROID_EXPORT_C
#ifdef GUCEF_AUTHPLUGIN_ANDROID_EXPORT_C_CODE
  #define GUCEF_AUTHPLUGIN_ANDROID_EXPORT_C GUCEF_AUTHPLUGIN_ANDROID_EXPORT
#else
  #define GUCEF_AUTHPLUGIN_ANDROID_EXPORT_C
#endif /* GUCEF_AUTHPLUGIN_ANDROID_EXPORT_C_CODE */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_ANDROID_MACROS_H ? */
