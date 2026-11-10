/*
 *  httpclientpluginCURL: GUCEF plugin providing a HTTP client backend using libcurl
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

#ifndef HTTPCLIENTPLUGIN_CURL_MACROS_H
#define HTTPCLIENTPLUGIN_CURL_MACROS_H

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

#ifndef HTTPCLIENTPLUGIN_CURL_CONFIG_H
#include "httpclientpluginCURL_config.h"
#define HTTPCLIENTPLUGIN_CURL_CONFIG_H
#endif /* HTTPCLIENTPLUGIN_CURL_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      When compiling the library you should define HTTPCLIENTPLUGIN_CURL_BUILD_PLUGIN_DLL
 *      before including this macro file.
 */
#undef HTTPCLIENTPLUGIN_CURL_PLUGIN_EXPORTSPEC
#ifdef HTTPCLIENTPLUGIN_CURL_BUILD_PLUGIN_DLL
  #define HTTPCLIENTPLUGIN_CURL_PLUGIN_EXPORTSPEC GUCEF_EXPORT
#else
  #define HTTPCLIENTPLUGIN_CURL_PLUGIN_EXPORTSPEC GUCEF_IMPORT
#endif

/*-------------------------------------------------------------------------*/

#undef HTTPCLIENTPLUGIN_CURL_PUBLIC_CPP
#ifdef HTTPCLIENTPLUGIN_CURL_PUBLIC_CPP_CODE
  #define HTTPCLIENTPLUGIN_CURL_PUBLIC_CPP HTTPCLIENTPLUGIN_CURL_EXPORTSPEC
#else
  #define HTTPCLIENTPLUGIN_CURL_PUBLIC_CPP GUCEF_HIDDEN
#endif /* HTTPCLIENTPLUGIN_CURL_PUBLIC_CPP_CODE */

#undef HTTPCLIENTPLUGIN_CURL_PUBLIC_C
#ifdef HTTPCLIENTPLUGIN_CURL_PUBLIC_C_CODE
  #define HTTPCLIENTPLUGIN_CURL_PUBLIC_C HTTPCLIENTPLUGIN_CURL_EXPORTSPEC
#else
  #define HTTPCLIENTPLUGIN_CURL_PUBLIC_C GUCEF_HIDDEN
#endif /* HTTPCLIENTPLUGIN_CURL_PUBLIC_C_CODE */

#define HTTPCLIENTPLUGIN_CURL_PRIVATE_CPP GUCEF_HIDDEN
#define HTTPCLIENTPLUGIN_CURL_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#undef HTTPCLIENTPLUGIN_CURL_PLUGIN_PUBLIC_CPP
#ifdef HTTPCLIENTPLUGIN_CURL_PLUGIN_PUBLIC_CPP_CODE
  #define HTTPCLIENTPLUGIN_CURL_PLUGIN_PUBLIC_CPP HTTPCLIENTPLUGIN_CURL_PLUGIN_EXPORTSPEC
#else
  #define HTTPCLIENTPLUGIN_CURL_PLUGIN_PUBLIC_CPP GUCEF_HIDDEN
#endif /* HTTPCLIENTPLUGIN_CURL_PLUGIN_PUBLIC_CPP_CODE */

#undef HTTPCLIENTPLUGIN_CURL_PLUGIN_PUBLIC_C
#ifdef HTTPCLIENTPLUGIN_CURL_PLUGIN_PUBLIC_C_CODE
  #define HTTPCLIENTPLUGIN_CURL_PLUGIN_PUBLIC_C HTTPCLIENTPLUGIN_CURL_PLUGIN_EXPORTSPEC
#else
  #define HTTPCLIENTPLUGIN_CURL_PLUGIN_PUBLIC_C GUCEF_HIDDEN
#endif /* HTTPCLIENTPLUGIN_CURL_PLUGIN_PUBLIC_C_CODE */

#define HTTPCLIENTPLUGIN_CURL_PLUGIN_PRIVATE_CPP GUCEF_HIDDEN
#define HTTPCLIENTPLUGIN_CURL_PLUGIN_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* HTTPCLIENTPLUGIN_CURL_MACROS_H ? */

