/*
 *  computebridgeMATH: GUCEF compute bridge plugin of gucefMATH: multi-core and accelerator providers for the batch kernels
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
  #ifndef _GNU_SOURCE
    #define _GNU_SOURCE
  #endif
  #include <dlfcn.h>
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_CMATHBRIDGEGLOBAL_H
#include "computebridgeMATH_CMathBridgeGlobal.h"
#define GUCEF_COMPUTEBRIDGEMATH_CMATHBRIDGEGLOBAL_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_CMATHBRIDGEGLOBAL_H ? */

#include "computebridgeMATH.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTEBRIDGEMATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/**
 *  Keeps the image of the plugin mapped for the rest of the process. Needed only when CPU kernel objects made by the
 *  plugin outlive it (a device or an application holds one): their code is the plugin's. Batch calls need no such help:
 *  CMathBatchDispatcher::UnregisterProviders() returns after the last call that could reach a provider has left it.
 */
void
PinModule( void )
{GUCEF_TRACE;

    static bool pinned = false;
    if ( pinned )
        return;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
    Dl_info info;
    if ( 0 != dladdr( reinterpret_cast< void* >( &PinModule ), &info ) && GUCEF_NULL != info.dli_fname )
        pinned = GUCEF_NULL != dlopen( info.dli_fname, RTLD_LAZY | RTLD_NODELETE );
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    HMODULE module = GUCEF_NULL;
    pinned = 0 != GetModuleHandleExA( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, reinterpret_cast< LPCSTR >( &PinModule ), &module );
    #else
    pinned = true;      // statically linked (the web): the code is never unmapped
    #endif

    if ( !pinned )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: unable to pin the plugin image, unloading it while other threads run batch calls is not safe" );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
COMPUTEBRIDGEMATH_ENTRY( GUCEFPlugin_Load )( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return CMathBridgeGlobal::Instance()->Load() ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
COMPUTEBRIDGEMATH_ENTRY( GUCEFPlugin_Unload )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    // when CPU kernels of the plugin are still held, the image stays: pinned, and the global with it
    if ( !CMathBridgeGlobal::Deinstance() )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: CPU kernels of the plugin are still in use, its code stays loaded" );
        PinModule();
    }
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
COMPUTEBRIDGEMATH_ENTRY( GUCEFPlugin_GetVersion )( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    versionInfo->major = 0;
    versionInfo->minor = 1;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
COMPUTEBRIDGEMATH_ENTRY( GUCEFPlugin_GetCopyright )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
COMPUTEBRIDGEMATH_ENTRY( GUCEFPlugin_GetDescription )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefCOMPUTE bridge plugin that adds multi-core and accelerator providers to the gucefMATH batch kernels";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
COMPUTEBRIDGEMATH_ENTRY( GUCEFPlugin_GetLinkBackModuleDependencies )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefMT,gucefCORE,gucefMATH,gucefCOMPUTE";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
