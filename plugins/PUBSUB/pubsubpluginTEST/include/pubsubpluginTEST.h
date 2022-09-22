/*
 *  pubsubpluginTEST: Generic GUCEF PUBSUB plugin for adding integration/system tests
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

#ifndef PUBSUBPLUGIN_TEST_H
#define PUBSUBPLUGIN_TEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef PUBSUBPLUGIN_TEST_MACROS_H
#include "pubsubpluginTEST_macros.h"
#define PUBSUBPLUGIN_TEST_MACROS_H
#endif /* PUBSUBPLUGIN_TEST_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      DEFINES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef PUBSUBPLUGIN_TEST_EMBED_CODE
  #define pubsubpluginTEST_GUCEFPlugin_Load              GUCEFPlugin_Load      
  #define pubsubpluginTEST_GUCEFPlugin_Unload            GUCEFPlugin_Unload
  #define pubsubpluginTEST_GUCEFPlugin_GetVersion        GUCEFPlugin_GetVersion
  #define pubsubpluginTEST_GUCEFPlugin_GetCopyright      GUCEFPlugin_GetCopyright
  #define pubsubpluginTEST_GUCEFPlugin_GetDescription    GUCEFPlugin_GetDescription
  #define PUBSUB_TEST_PLUGIN_CALLSPEC_PREFIX             GUCEF_PLUGIN_CALLSPEC_PREFIX
  #define PUBSUB_TEST_PLUGIN_CALLSPEC_SUFFIX             GUCEF_PLUGIN_CALLSPEC_SUFFIX
#else
  #define PUBSUB_TEST_PLUGIN_CALLSPEC_PREFIX
  #define PUBSUB_TEST_PLUGIN_CALLSPEC_SUFFIX
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
using namespace ::GUCEF;
using namespace ::GUCEF::CORE;

extern "C" {
#endif

/*---------------------------------------------------------------------------*/

PUBSUBPLUGIN_TEST_PLUGIN_PUBLIC_C CORE::Int32 PUBSUB_TEST_PLUGIN_CALLSPEC_PREFIX
pubsubpluginTEST_GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) PUBSUB_TEST_PLUGIN_CALLSPEC_SUFFIX;

/*--------------------------------------------------------------------------*/

PUBSUBPLUGIN_TEST_PLUGIN_PUBLIC_C void PUBSUB_TEST_PLUGIN_CALLSPEC_PREFIX
pubsubpluginTEST_GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*--------------------------------------------------------------------------*/

PUBSUBPLUGIN_TEST_PLUGIN_PUBLIC_C void PUBSUB_TEST_PLUGIN_CALLSPEC_PREFIX
pubsubpluginTEST_GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) PUBSUB_TEST_PLUGIN_CALLSPEC_SUFFIX;

/*--------------------------------------------------------------------------*/

PUBSUBPLUGIN_TEST_PLUGIN_PUBLIC_C const char* PUBSUB_TEST_PLUGIN_CALLSPEC_PREFIX
pubsubpluginTEST_GUCEFPlugin_GetCopyright( void ) PUBSUB_TEST_PLUGIN_CALLSPEC_SUFFIX;

/*--------------------------------------------------------------------------*/

PUBSUBPLUGIN_TEST_PLUGIN_PUBLIC_C const char* PUBSUB_TEST_PLUGIN_CALLSPEC_PREFIX
pubsubpluginTEST_GUCEFPlugin_GetDescription( void ) PUBSUB_TEST_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/                 

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*---------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_TEST_H ? */
