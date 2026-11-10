/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_VIDEO_CVIDEOCODECREGISTRY_H
#include "gucefVIDEO_CVideoCodecRegistry.h"
#define GUCEF_VIDEO_CVIDEOCODECREGISTRY_H
#endif /* GUCEF_VIDEO_CVIDEOCODECREGISTRY_H ? */

#ifndef GUCEF_VIDEO_CVIDEOCODECPLUGINMANAGER_H
#include "gucefVIDEO_CVideoCodecPluginManager.h"
#define GUCEF_VIDEO_CVIDEOCODECPLUGINMANAGER_H
#endif /* GUCEF_VIDEO_CVIDEOCODECPLUGINMANAGER_H ? */

#ifndef GUCEF_VIDEO_CVIDEOPLAYER_H
#include "gucefVIDEO_CVideoPlayer.h"
#define GUCEF_VIDEO_CVIDEOPLAYER_H
#endif /* GUCEF_VIDEO_CVIDEOPLAYER_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#include "gucefVIDEO_CAsyncVideoTranscode.h"
#include "gucefVIDEO_CVideoGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

CVideoGlobal* CVideoGlobal::g_instance = GUCEF_NULL;

/** Makes the tasks of CVideoTranscoder::TranscodeAsync() */
static TAsyncVideoTranscodeTaskFactory g_asyncVideoTranscodeTaskFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVideoGlobal*
CVideoGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        g_instance = GUCEF_NEW CVideoGlobal();
        g_instance->Initialize();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CVideoGlobal::Deinstance( void )
{GUCEF_TRACE;

    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CVideoGlobal::CVideoGlobal( void )
    : m_videoCodecRegistry( GUCEF_NULL )
    , m_videoCodecPluginManager( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVideoGlobal::~CVideoGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Shutting down gucefVIDEO global systems" );
    CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterTaskConsumerFactory( CAsyncVideoTranscode::TaskType );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefVIDEO" );

    // Codec plugins first: their codecs are in the registry and their videos may still be open
    if ( GUCEF_NULL != m_videoCodecPluginManager )
        m_videoCodecPluginManager->UnlinkAll();
    GUCEF_DELETE m_videoCodecPluginManager;
    m_videoCodecPluginManager = GUCEF_NULL;

    GUCEF_DELETE m_videoCodecRegistry;
    m_videoCodecRegistry = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CVideoCodecRegistry&
CVideoGlobal::GetVideoCodecRegistry( void )
{GUCEF_TRACE;

    return *m_videoCodecRegistry;
}

/*-------------------------------------------------------------------------*/

CVideoCodecPluginManager&
CVideoGlobal::GetVideoCodecPluginManager( void )
{GUCEF_TRACE;

    return *m_videoCodecPluginManager;
}

/*-------------------------------------------------------------------------*/

void
CVideoGlobal::Initialize( void )
{GUCEF_TRACE;

    // Ensure the systems this module builds on are initialized first
    CORE::CCoreGlobal::Instance();
    AUDIO::CAudioGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Initializing gucefVIDEO Global systems" );

    CVideoPlayer::RegisterEvents();

    m_videoCodecRegistry = GUCEF_NEW CVideoCodecRegistry();
    m_videoCodecPluginManager = GUCEF_NEW CVideoCodecPluginManager();
    CORE::CCoreGlobal::Instance()->GetTaskManager().RegisterTaskConsumerFactory( CAsyncVideoTranscode::TaskType, &g_asyncVideoTranscodeTaskFactory );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefVIDEO Global systems initialized" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefVIDEO" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
