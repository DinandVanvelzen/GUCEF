/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCODECREGISTRY_H
#include "gucefAUDIO_CAudioCodecRegistry.h"
#define GUCEF_AUDIO_CAUDIOCODECREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOCODECREGISTRY_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "gucefGUI_CAudioPlayer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CAudioPlayer::AudioEndedEvent = "GUCEF::GUI::CAudioPlayer::AudioEndedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAudioPlayer::RegisterEvents( void )
{GUCEF_TRACE;

    AudioEndedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAudioPlayer::CAudioPlayer( void )
    : CWidget( "AudioPlayer" )
    , m_engine( GUCEF_NULL )
    , m_bus( GUCEF_NULL )
    , m_voice()
    , m_file()
    , m_paused( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CAudioPlayer::~CAudioPlayer()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

CWidget*
CAudioPlayer::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CAudioPlayer();
}

/*-------------------------------------------------------------------------*/

void
CAudioPlayer::SetAudioOutput( AUDIO::CAudioEngine* engine, AUDIO::CAudioBus* bus )
{GUCEF_TRACE;

    m_engine = engine;
    m_bus = bus;
}

/*-------------------------------------------------------------------------*/

bool
CAudioPlayer::Play( void )
{GUCEF_TRACE;

    Stop();
    CString source = GetProperty( EWidgetProperty_Source );
    if ( GUCEF_NULL == m_engine || source.IsNULLOrEmpty() )
        return false;

    AUDIO::TAudioCodecPtr codec;
    if ( !AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodecForFilename( source, codec ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AudioPlayer \"" << GetNamePath() << "\": no audio codec for " << source );
        return false;
    }
    m_file = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( source );
    if ( m_file.IsNULL() || m_file->GetAccess().IsNULL() )
        return false;
    AUDIO::TAudioSourcePtr audioSource;
    if ( !codec->OpenSource( m_file->GetAccess(), audioSource ) )
        return false;
    VFS::TBasicVfsResourcePtr file = m_file;
    if ( !Play( audioSource ) )
        return false;
    m_file = file;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioPlayer::Play( const AUDIO::TAudioSourcePtr& audioSource )
{GUCEF_TRACE;

    if ( !m_voice.IsNULL() )
    {
        if ( GUCEF_NULL != m_engine )
            m_engine->Stop( m_voice );
        m_voice.Unlink();
    }
    if ( GUCEF_NULL == m_engine || audioSource.IsNULL() )
        return false;
    m_voice = m_engine->CreateVoice( audioSource, m_bus );
    if ( m_voice.IsNULL() )
        return false;
    m_voice->SetVolume( GetFloatProperty( EWidgetProperty_Volume ) );
    m_voice->SetLoopCount( GetBoolProperty( EWidgetProperty_Loop ) ? 0 : 1 );
    CORE::CFutureResult started = m_engine->Start( m_voice );
    if ( started.HasNoFuture() )
    {
        m_voice.Unlink();
        return false;
    }
    m_paused = false;
    SetBoolProperty( EWidgetProperty_Playing, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioPlayer::Pause( void )
{GUCEF_TRACE;

    if ( m_voice.IsNULL() || GUCEF_NULL == m_engine || m_paused )
        return false;
    m_engine->Pause( m_voice );
    m_paused = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioPlayer::Resume( void )
{GUCEF_TRACE;

    if ( m_voice.IsNULL() || GUCEF_NULL == m_engine || !m_paused )
        return false;
    m_engine->Resume( m_voice );
    m_paused = false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioPlayer::Stop( void )
{GUCEF_TRACE;

    if ( !m_voice.IsNULL() && GUCEF_NULL != m_engine )
        m_engine->Stop( m_voice );
    m_voice.Unlink();
    m_file.Unlink();
    m_paused = false;
    SetBoolProperty( EWidgetProperty_Playing, false );
}

/*-------------------------------------------------------------------------*/

bool
CAudioPlayer::IsPlaying( void ) const
{GUCEF_TRACE;

    return GetBoolProperty( EWidgetProperty_Playing );
}

/*-------------------------------------------------------------------------*/

bool
CAudioPlayer::IsPaused( void ) const
{GUCEF_TRACE;

    return m_paused;
}

/*-------------------------------------------------------------------------*/

bool
CAudioPlayer::IsActivatable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioPlayer::NeedsUpdate( void ) const
{GUCEF_TRACE;

    return !m_voice.IsNULL();
}

/*-------------------------------------------------------------------------*/

void
CAudioPlayer::OnUpdate( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( m_voice.IsNULL() )
        return;
    UInt32 rate = m_voice->GetSource().IsNULL() ? 0 : m_voice->GetSource()->GetFormat().GetSampleRate();
    if ( rate > 0 )
        SetIntProperty( EWidgetProperty_PositionInMs, (Int32) ( m_voice->GetPositionInFrames() * 1000 / rate ) );
    if ( AUDIO::AUDIOVOICESTATE_ENDED == m_voice->GetState() )
    {
        m_voice.Unlink();
        m_file.Unlink();
        SetBoolProperty( EWidgetProperty_Playing, false );
        NotifyObservers( AudioEndedEvent );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAudioPlayer::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( ActivatedEvent != eventId )
        return false;
    if ( !IsPlaying() )
        return Play();
    return m_paused ? Resume() : Pause();
}

/*-------------------------------------------------------------------------*/

void
CAudioPlayer::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    if ( property == EWidgetProperty_Volume && !m_voice.IsNULL() )
        m_voice->SetVolume( GetFloatProperty( EWidgetProperty_Volume ) );
    else
    if ( property == EWidgetProperty_Source && GetBoolProperty( EWidgetProperty_AutoPlay ) && !newValue.IsNULLOrEmpty() )
        Play();
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioPlayer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CAudioPlayer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
