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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VIDEO_CVIDEOGLOBAL_H
#include "gucefVIDEO_CVideoGlobal.h"
#define GUCEF_VIDEO_CVIDEOGLOBAL_H
#endif /* GUCEF_VIDEO_CVIDEOGLOBAL_H ? */

#ifndef GUCEF_VIDEO_CVIDEOCODECREGISTRY_H
#include "gucefVIDEO_CVideoCodecRegistry.h"
#define GUCEF_VIDEO_CVIDEOCODECREGISTRY_H
#endif /* GUCEF_VIDEO_CVIDEOCODECREGISTRY_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CVideoView.h"

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

const CORE::CEvent CVideoView::VideoEndedEvent = "GUCEF::GUI::CVideoView::VideoEndedEvent";
const CORE::CEvent CVideoView::VideoSkippedEvent = "GUCEF::GUI::CVideoView::VideoSkippedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CVideoView::RegisterEvents( void )
{GUCEF_TRACE;

    VideoEndedEvent.Initialize();
    VideoSkippedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CVideoView::CVideoView( void )
    : CImageFrame( "VideoView" )
    , m_player()
    , m_file()
    , m_codecName()
    , m_engine( GUCEF_NULL )
    , m_bus( GUCEF_NULL )
    , m_pixels()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CVideoView::~CVideoView()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

CWidget*
CVideoView::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CVideoView();
}

/*-------------------------------------------------------------------------*/

void
CVideoView::SetAudioOutput( AUDIO::CAudioEngine* engine, AUDIO::CAudioBus* bus )
{GUCEF_TRACE;

    m_engine = engine;
    m_bus = bus;
}

/*-------------------------------------------------------------------------*/

bool
CVideoView::Play( void )
{GUCEF_TRACE;

    CString source = GetProperty( EWidgetProperty_Source );
    if ( source.IsNULLOrEmpty() )
        return false;
    VFS::TBasicVfsResourcePtr file = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( source );
    if ( file.IsNULL() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VideoView \"" << GetNamePath() << "\": cannot open " << source );
        return false;
    }
    return Play( file, source );
}

/*-------------------------------------------------------------------------*/

bool
CVideoView::Play( const VFS::TBasicVfsResourcePtr& file, const CString& nameForCodec )
{GUCEF_TRACE;

    Stop();
    m_file = file;
    if ( m_file.IsNULL() || m_file->GetAccess().IsNULL() )
    {
        m_file.Unlink();
        return false;
    }
    m_codecName = nameForCodec;
    return Start();
}

/*-------------------------------------------------------------------------*/

bool
CVideoView::Play( const VIDEO::TVideoSourcePtr& source )
{GUCEF_TRACE;

    Stop();
    if ( source.IsNULL() || !m_player.Open( source ) || !m_player.Start( m_engine, m_bus, 0 ) )
    {
        m_player.Close();
        return false;
    }
    SetBoolProperty( EWidgetProperty_Playing, true );
    ShowFrame();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVideoView::Start( void )
{GUCEF_TRACE;

    VIDEO::TVideoCodecPtr codec;
    if ( !VIDEO::CVideoGlobal::Instance()->GetVideoCodecRegistry().TryFindCodecForFilename( m_codecName, codec ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VideoView \"" << GetNamePath() << "\": no video codec for " << m_codecName );
        return false;
    }
    m_file->GetAccess()->Setpos( 0 );
    VIDEO::TVideoSourcePtr source;
    if ( !codec->OpenSource( m_file->GetAccess(), source ) || !m_player.Open( source ) || !m_player.Start( m_engine, m_bus, 0 ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VideoView \"" << GetNamePath() << "\": cannot play " << m_codecName );
        m_player.Close();
        return false;
    }
    SetBoolProperty( EWidgetProperty_Playing, true );
    ShowFrame();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVideoView::Stop( void )
{GUCEF_TRACE;

    if ( m_player.IsOpen() )
        m_player.Close();
    m_file.Unlink();
    SetBoolProperty( EWidgetProperty_Playing, false );
}

/*-------------------------------------------------------------------------*/

void
CVideoView::Skip( void )
{GUCEF_TRACE;

    if ( !IsPlaying() )
        return;
    Stop();
    NotifyObservers( VideoSkippedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CVideoView::IsPlaying( void ) const
{GUCEF_TRACE;

    return GetBoolProperty( EWidgetProperty_Playing );
}

/*-------------------------------------------------------------------------*/

const VIDEO::CVideoPlayer&
CVideoView::GetPlayer( void ) const
{GUCEF_TRACE;

    return m_player;
}

/*-------------------------------------------------------------------------*/

bool
CVideoView::NeedsUpdate( void ) const
{GUCEF_TRACE;

    return IsPlaying() || CImageFrame::NeedsUpdate();
}

/*-------------------------------------------------------------------------*/

void
CVideoView::ShowFrame( void )
{GUCEF_TRACE;

    const VIDEO::CVideoFrame& frame = m_player.GetCurrentFrame();
    if ( !m_player.HasFrame() || !frame.HasPicture() )
        return;
    if ( !frame.ConvertTo32Bit( IMAGE::PSF_RGBA, m_pixels ) )
        return;
    // the same pixel map stays registered; a new one (another size) is registered again
    if ( GetImageName() == GetOwnImageName() && GUCEF_NULL != GetContext() && GetContext()->HasImage( GetOwnImageName() ) )
    {
        IMAGE::TPixelMapPtr registered;
        UInt32 revision = 0;
        if ( GetContext()->ResolveImage( GetOwnImageName(), registered, revision ) && registered == m_pixels )
        {
            UpdateImage();
            return;
        }
    }
    SetImage( m_pixels );
}

/*-------------------------------------------------------------------------*/

void
CVideoView::OnUpdate( UInt32 deltaInMs )
{GUCEF_TRACE;

    CImageFrame::OnUpdate( deltaInMs );
    if ( !IsPlaying() )
        return;

    if ( m_player.Update() )
        ShowFrame();
    if ( m_player.IsEnded() )
    {
        if ( GetBoolProperty( EWidgetProperty_Loop ) && !m_file.IsNULL() )
        {
            m_player.Close();
            Start();
            return;
        }
        Stop();
        NotifyObservers( VideoEndedEvent );
    }
}

/*-------------------------------------------------------------------------*/

bool
CVideoView::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( ( MouseClickEvent == eventId || KeyDownEvent == eventId ) && IsPlaying() && GetBoolProperty( EWidgetProperty_SkipOnInput ) )
    {
        Skip();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CVideoView::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    CImageFrame::OnPropertyChanged( property, oldValue, newValue );
    if ( property == EWidgetProperty_Source && GetBoolProperty( EWidgetProperty_AutoPlay ) && !newValue.IsNULLOrEmpty() )
        Play();
}

/*-------------------------------------------------------------------------*/

const CString&
CVideoView::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CVideoView";
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
