/*
 *  audiodriverWASAPI: gucefAUDIO driver plugin for WASAPI on Windows
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H
#include "gucefAUDIO_CAudioDriverRegistry.h"
#define GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H ? */

#ifndef GUCEF_AUDIODRIVER_WASAPI_CWASAPIAUDIODRIVER_H
#include "audiodriverWASAPI_CWasapiAudioDriver.h"
#define GUCEF_AUDIODRIVER_WASAPI_CWASAPIAUDIODRIVER_H
#endif /* GUCEF_AUDIODRIVER_WASAPI_CWASAPIAUDIODRIVER_H ? */

#include "audiodriverWASAPI.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIODRIVER {
namespace WASAPI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Load called on audio driver plugin WASAPI" );

    CWasapiAudioDriver* wasapi = GUCEF_NEW CWasapiAudioDriver();
    if ( !wasapi->IsAvailable() )
    {
        GUCEF_DELETE wasapi;
        return 0;
    }

    AUDIO::TAudioDriverPtr driver( wasapi );
    if ( !AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry().TryRegister( "wasapi", driver ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Audio driver plugin WASAPI: a driver named \"wasapi\" is registered already" );
        return 0;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unload called on audio driver plugin WASAPI" );

    // the registry can be emptied before plugins are unloaded at shutdown, Unregister() would throw then.
    // Devices opened through the driver hold COM objects of this module, the application closes them before unloading
    AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry().TryUnregister( "wasapi" );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    versionInfo->major = 0;
    versionInfo->minor = 1;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefAUDIO driver plugin for WASAPI on Windows";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefMT,gucefCORE,gucefAUDIO";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WASAPI */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
