/*
 *  comcorepluginGEOOSM: Generic GUCEF COMCORE plugin 
 *  providing Geo Data services based on OpenStreetMap data.
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

#ifndef COMCOREPLUGIN_GEOOSM_H
#define COMCOREPLUGIN_GEOOSM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef COMCOREPLUGIN_GEOOSM_MACROS_H
#include "comcorepluginGEOOSM_macros.h"
#define COMCOREPLUGIN_GEOOSM_MACROS_H
#endif /* COMCOREPLUGIN_GEOOSM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace COMCOREPLUGIN {
namespace GEOOSM {
#endif /* __cplusplus */

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

COMCOREPLUGIN_GEOOSM_PLUGIN_PUBLIC_C Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( UInt32 argc, const char** argv );

/*--------------------------------------------------------------------------*/

COMCOREPLUGIN_GEOOSM_PLUGIN_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*--------------------------------------------------------------------------*/

COMCOREPLUGIN_GEOOSM_PLUGIN_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*--------------------------------------------------------------------------*/

COMCOREPLUGIN_GEOOSM_PLUGIN_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*--------------------------------------------------------------------------*/

COMCOREPLUGIN_GEOOSM_PLUGIN_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*--------------------------------------------------------------------------*/

COMCOREPLUGIN_GEOOSM_PLUGIN_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace GEOOSM */
}; /* namespace COMCOREPLUGIN */
}; /* namespace GUCEF */
#endif /* __cplusplus */

/*--------------------------------------------------------------------------*/

#endif /* COMCOREPLUGIN_GEOOSM_H ? */
