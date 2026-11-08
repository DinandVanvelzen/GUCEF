/*
 *  audiodriverALSA: gucefAUDIO driver plugin for ALSA on Linux
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

#ifndef GUCEF_AUDIODRIVER_ALSA_H
#define GUCEF_AUDIODRIVER_ALSA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_AUDIODRIVER_ALSA_MACROS_H
#include "audiodriverALSA_macros.h"
#define GUCEF_AUDIODRIVER_ALSA_MACROS_H
#endif /* GUCEF_AUDIODRIVER_ALSA_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIODRIVER {
namespace ALSA {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*/

/** Registers the "alsa" driver with gucefAUDIO. Fails when libasound is not on the system */
GUCEF_AUDIODRIVER_ALSA_EXPORT_C CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUDIODRIVER_ALSA_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUDIODRIVER_ALSA_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUDIODRIVER_ALSA_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUDIODRIVER_ALSA_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_AUDIODRIVER_ALSA_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

/*
 *  Access for tests that run the driver on a fake libasound. The plugin is loaded by path and is not linked, so
 *  the tests reach these through the symbol table. The table type is CAlsaApi of audiodriverALSA_CAlsaApi.h.
 */

/** Makes an API table that is not loaded: every function is null, the test fills in the ones it fakes. Release with GUCEFPlugin_ALSA_DeleteTestApi() after the driver and its devices are gone */
GUCEF_AUDIODRIVER_ALSA_EXPORT_C void* GUCEFPlugin_ALSA_NewTestApi( void );

GUCEF_AUDIODRIVER_ALSA_EXPORT_C void GUCEFPlugin_ALSA_DeleteTestApi( void* api );

/** Makes a CAudioDriver (not registered) on a table of GUCEFPlugin_ALSA_NewTestApi(). The caller owns it: hold it in a TAudioDriverPtr */
GUCEF_AUDIODRIVER_ALSA_EXPORT_C void* GUCEFPlugin_ALSA_NewTestDriver( void* api );

/** CAlsaAudioDevice::GetCurrentDelayInFrames() of a CAudioDevice that was opened by such a driver */
GUCEF_AUDIODRIVER_ALSA_EXPORT_C CORE::Int32 GUCEFPlugin_ALSA_GetCurrentDelay( void* device, CORE::Int32* delayInFrames );

/** CAlsaAudioDriver::ComputeSoftwareParams() */
GUCEF_AUDIODRIVER_ALSA_EXPORT_C void GUCEFPlugin_ALSA_ComputeSoftwareParams( CORE::Int32 isCapture, CORE::UInt32 periodSizeInFrames, CORE::UInt32 bufferSizeInFrames, CORE::UInt32* availMin, CORE::UInt32* startThreshold );

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ALSA */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIODRIVER_ALSA_H ? */
