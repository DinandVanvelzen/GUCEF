/*
 *  audiovoicecodecOPUS: gucefAUDIO voice codec plugin for Opus
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

#ifndef GUCEF_AUDIO_CVOICECODECREGISTRY_H
#include "gucefAUDIO_CVoiceCodecRegistry.h"
#define GUCEF_AUDIO_CVOICECODECREGISTRY_H
#endif /* GUCEF_AUDIO_CVOICECODECREGISTRY_H ? */

#ifndef GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICECODECFACTORY_H
#include "audiovoicecodecOPUS_COpusVoiceCodecFactory.h"
#define GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICECODECFACTORY_H
#endif /* GUCEF_AUDIOVOICECODEC_OPUS_COPUSVOICECODECFACTORY_H ? */

#include "audiovoicecodecOPUS.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIOVOICECODEC {
namespace OPUS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Load called on voice codec plugin Opus" );

    AUDIO::TVoiceCodecFactoryPtr factory( GUCEF_NEW COpusVoiceCodecFactory() );
    if ( !AUDIO::CAudioGlobal::Instance()->GetVoiceCodecRegistry().RegisterFactory( factory ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Voice codec plugin Opus: a voice codec named \"opus\" is registered already" );
        return 0;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unload called on voice codec plugin Opus" );

    // Refused (and logged by the registry) while encoders or decoders made by the plugin live: their code is in this
    // module, the application has to let go of them before the plugin is unloaded
    AUDIO::CAudioGlobal::Instance()->GetVoiceCodecRegistry().TryUnregisterFactory( "opus" );
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

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0; libopus Copyright (C) Xiph.Org Foundation and contributors, BSD-3-Clause";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefAUDIO voice codec plugin for Opus";
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

}; /* namespace OPUS */
}; /* namespace AUDIOVOICECODEC */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
