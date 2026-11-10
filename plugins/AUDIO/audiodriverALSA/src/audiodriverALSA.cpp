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

#ifndef GUCEF_AUDIODRIVER_ALSA_CALSAAUDIODRIVER_H
#include "audiodriverALSA_CAlsaAudioDriver.h"
#define GUCEF_AUDIODRIVER_ALSA_CALSAAUDIODRIVER_H
#endif /* GUCEF_AUDIODRIVER_ALSA_CALSAAUDIODRIVER_H ? */

#include "audiodriverALSA.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

// Made when the plugin is loaded and released when it is unloaded. Devices opened through the driver use the
// library, so the application has to close them before the plugin is unloaded
static CAlsaApi* g_alsaApi = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Load called on audio driver plugin ALSA" );

    g_alsaApi = GUCEF_NEW CAlsaApi();
    if ( !g_alsaApi->Load() )
    {
        GUCEF_DELETE g_alsaApi;
        g_alsaApi = GUCEF_NULL;
        return 0;
    }

    AUDIO::TAudioDriverPtr driver( GUCEF_NEW CAlsaAudioDriver( *g_alsaApi ) );
    if ( !AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry().TryRegister( "alsa", driver ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Audio driver plugin ALSA: a driver named \"alsa\" is registered already" );
        driver.Unlink();
        GUCEF_DELETE g_alsaApi;
        g_alsaApi = GUCEF_NULL;
        return 0;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unload called on audio driver plugin ALSA" );

    if ( GUCEF_NULL != g_alsaApi )
    {
        // the registry can be emptied before plugins are unloaded at shutdown, Unregister() would throw then
        AUDIO::CAudioGlobal::Instance()->GetAudioDriverRegistry().TryUnregister( "alsa" );
        GUCEF_DELETE g_alsaApi;
        g_alsaApi = GUCEF_NULL;
    }
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

    return "gucefAUDIO driver plugin for ALSA on Linux";
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

}; /* namespace ALSA */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
