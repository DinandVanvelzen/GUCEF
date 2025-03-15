/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

/* 
 *  This header provides a basic C interface to some basic CORE functionality
 *  The purpose of these functions is to provide a way to link against the
 *  C++ module and start a sequence of events (trigger use-cases) without being
 *  tied to C++ linking. Instead you can do dynamic linking against these C
 *  functions to provide a more resiliant bridge to C++ modules, isolating you
 *  from changes within the C++ code which would normally require re-linking.
 */
 
#ifndef GUCEF_CORE_C_API_H
#define GUCEF_CORE_C_API_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_C_LOGTYPES_H
#include "gucefCORE_c_logtypes.h"
#define GUCEF_CORE_C_LOGTYPES_H
#endif /* GUCEF_CORE_C_LOGTYPES_H ? */

#ifndef GUCEF_CORE_C_LOGLEVELS_H
#include "gucefCORE_c_loglevels.h"
#define GUCEF_CORE_C_LOGLEVELS_H
#endif /* GUCEF_CORE_C_LOGLEVELS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGucefCoreCApi_Log ) ( int logType, int logLevel, const char* msg ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGucefCoreCApi_SetDefaultPluginLoaderLogicType )( const char* defaultLoaderLogicType ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGucefCoreCApi_AddPluginDir ) ( const char* pluginDir ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef int ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGucefCoreCApi_LoadPlugin ) ( const char* pluginPath, const char* pluginType, const char* groupName, int argc, char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef int ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGucefCoreCApi_LoadConfig ) ( const char* configPath, const char* dataCodec ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

/**
 *  Structure that hold all the function pointers to the C API functions
 *  Should be called by plugins as needed.
 */
struct SGucefCoreCApi
{
    TGucefCoreCApi_Log Log;
    TGucefCoreCApi_SetDefaultPluginLoaderLogicType SetDefaultPluginLoaderLogicType;
    TGucefCoreCApi_AddPluginDir AddPluginDir;
    TGucefCoreCApi_LoadPlugin LoadPlugin;
    TGucefCoreCApi_LoadConfig LoadConfig;
};
typedef struct SGucefCoreCApi TGucefCoreCApi;

/*-------------------------------------------------------------------------*/                 

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_C_API_H ? */
