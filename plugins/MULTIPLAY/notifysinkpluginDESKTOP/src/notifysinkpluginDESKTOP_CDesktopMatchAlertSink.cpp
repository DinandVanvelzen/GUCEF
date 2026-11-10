/*
 *  notifysinkpluginDESKTOP: gucefMULTIPLAY match alert sink plugin for desktop alerts
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

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_CNOTIFYSENDLAUNCHER_H
#include "notifysinkpluginDESKTOP_CNotifySendLauncher.h"
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_CNOTIFYSENDLAUNCHER_H
#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_CNOTIFYSENDLAUNCHER_H ? */

#include "notifysinkpluginDESKTOP_CDesktopMatchAlertSink.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace DESKTOP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDesktopMatchAlertSink::SinkName = "desktop";

static const UInt32 MaxChildrenLimit = 64;
static const UInt32 MaxAppNameBytes = 64;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDesktopMatchAlertSink::CDesktopMatchAlertSink( void )
    : MULTIPLAY::CIMatchAlertSink()
    , m_launcher( GUCEF_NULL )
    , m_ownedLauncher( GUCEF_NULL )
    , m_appName( "Game" )
    , m_category()
    , m_expireTimeMs( 0 )
    , m_maxChildren( DefaultMaxChildren )
    , m_texts()
    , m_launchedCount( 0 )
    , m_droppedCount( 0 )
    , m_reportedUnavailable( false )
{GUCEF_TRACE;

    m_ownedLauncher = GUCEF_NEW CNotifySendLauncher();
    m_launcher = m_ownedLauncher;
}

/*-------------------------------------------------------------------------*/

CDesktopMatchAlertSink::CDesktopMatchAlertSink( INotifyLauncher* launcher )
    : MULTIPLAY::CIMatchAlertSink()
    , m_launcher( launcher )
    , m_ownedLauncher( GUCEF_NULL )
    , m_appName( "Game" )
    , m_category()
    , m_expireTimeMs( 0 )
    , m_maxChildren( DefaultMaxChildren )
    , m_texts()
    , m_launchedCount( 0 )
    , m_droppedCount( 0 )
    , m_reportedUnavailable( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDesktopMatchAlertSink::~CDesktopMatchAlertSink()
{GUCEF_TRACE;

    m_launcher = GUCEF_NULL;
    GUCEF_DELETE m_ownedLauncher;
    m_ownedLauncher = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CDesktopMatchAlertSink::IsInterrupting( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDesktopMatchAlertSink::GetSinkName( void ) const
{GUCEF_TRACE;

    return SinkName;
}

/*-------------------------------------------------------------------------*/

void
CDesktopMatchAlertSink::SetLauncher( INotifyLauncher* launcher )
{GUCEF_TRACE;

    m_launcher = launcher;
}

/*-------------------------------------------------------------------------*/

void
CDesktopMatchAlertSink::SetAppName( const CORE::CString& appName )
{GUCEF_TRACE;

    m_appName = CDesktopMatchAlertText::Sanitize( appName, MaxAppNameBytes, false );
    if ( m_appName.IsNULLOrEmpty() )
        m_appName = "Game";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDesktopMatchAlertSink::GetAppName( void ) const
{GUCEF_TRACE;

    return m_appName;
}

/*-------------------------------------------------------------------------*/

bool
CDesktopMatchAlertSink::SetCategory( const CORE::CString& category )
{GUCEF_TRACE;

    if ( !category.IsNULLOrEmpty() && !CDesktopMatchAlertText::IsValidCategory( category ) )
        return false;
    m_category = category;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDesktopMatchAlertSink::GetCategory( void ) const
{GUCEF_TRACE;

    return m_category;
}

/*-------------------------------------------------------------------------*/

void
CDesktopMatchAlertSink::SetExpireTimeMs( UInt32 expireTimeMs )
{GUCEF_TRACE;

    m_expireTimeMs = ( expireTimeMs > MaxExpireTimeMs ) ? MaxExpireTimeMs : expireTimeMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CDesktopMatchAlertSink::GetExpireTimeMs( void ) const
{GUCEF_TRACE;

    return m_expireTimeMs;
}

/*-------------------------------------------------------------------------*/

void
CDesktopMatchAlertSink::SetMaxChildren( UInt32 maxChildren )
{GUCEF_TRACE;

    if ( maxChildren < 1 )
        maxChildren = 1;
    else
    if ( maxChildren > MaxChildrenLimit )
        maxChildren = MaxChildrenLimit;
    m_maxChildren = maxChildren;
}

/*-------------------------------------------------------------------------*/

UInt32
CDesktopMatchAlertSink::GetMaxChildren( void ) const
{GUCEF_TRACE;

    return m_maxChildren;
}

/*-------------------------------------------------------------------------*/

void
CDesktopMatchAlertSink::SetTexts( const CDesktopMatchAlertTexts& texts )
{GUCEF_TRACE;

    m_texts = texts;
}

/*-------------------------------------------------------------------------*/

const CDesktopMatchAlertTexts&
CDesktopMatchAlertSink::GetTexts( void ) const
{GUCEF_TRACE;

    return m_texts;
}

/*-------------------------------------------------------------------------*/

UInt32
CDesktopMatchAlertSink::GetLaunchedCount( void ) const
{GUCEF_TRACE;

    return m_launchedCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CDesktopMatchAlertSink::GetDroppedCount( void ) const
{GUCEF_TRACE;

    return m_droppedCount;
}

/*-------------------------------------------------------------------------*/

bool
CDesktopMatchAlertSink::BuildArguments( const MULTIPLAY::CMatchAlert& alert, INotifyLauncher::TArgumentVector& argv ) const
{GUCEF_TRACE;

    // Only a turn and the end of a match are about one match: the title and the name are not used for the others
    CORE::CString title;
    CORE::CString mover;
    CORE::CString summary;
    CORE::CString body;
    const UInt32 count = alert.count;

    switch ( alert.kind )
    {
        case MULTIPLAY::MATCHALERTKIND_YOUR_TURN :
        {
            title = CDesktopMatchAlertText::Sanitize( alert.title, 512, false );
            mover = CDesktopMatchAlertText::Sanitize( alert.previousMover, 512, false );
            summary = title.IsNULLOrEmpty() ? CDesktopMatchAlertText::Expand( m_texts.yourTurnSummaryFallback, CORE::CString::Empty, CORE::CString::Empty, 1 ) : title;
            body = mover.IsNULLOrEmpty() ? CDesktopMatchAlertText::Expand( m_texts.yourTurnBody, title, CORE::CString::Empty, 1 )
                                         : CDesktopMatchAlertText::Expand( m_texts.yourTurnBodyAfterMover, title, mover, 1 );
            break;
        }
        case MULTIPLAY::MATCHALERTKIND_COALESCED :
        {
            summary = CDesktopMatchAlertText::Expand( 1 == count ? m_texts.coalescedSummaryOne : m_texts.coalescedSummary, CORE::CString::Empty, CORE::CString::Empty, count );
            break;
        }
        case MULTIPLAY::MATCHALERTKIND_DIGEST :
        {
            summary = CDesktopMatchAlertText::Expand( m_texts.digestSummary, CORE::CString::Empty, CORE::CString::Empty, count );
            body = CDesktopMatchAlertText::Expand( 1 == count ? m_texts.digestBodyOne : m_texts.digestBody, CORE::CString::Empty, CORE::CString::Empty, count );
            break;
        }
        case MULTIPLAY::MATCHALERTKIND_INVITE :
        {
            summary = CDesktopMatchAlertText::Expand( m_texts.inviteSummary, CORE::CString::Empty, CORE::CString::Empty, 1 );
            break;
        }
        case MULTIPLAY::MATCHALERTKIND_FINISHED :
        {
            title = CDesktopMatchAlertText::Sanitize( alert.title, 512, false );
            summary = CDesktopMatchAlertText::Expand( m_texts.finishedSummary, title, CORE::CString::Empty, 1 );
            body = CDesktopMatchAlertText::Expand( m_texts.finishedBody, title, CORE::CString::Empty, 1 );
            break;
        }
        default :
        {
            return false;
        }
    }

    summary = CDesktopMatchAlertText::SanitizeSummary( summary );
    body = CDesktopMatchAlertText::SanitizeBody( body );
    if ( summary.IsNULLOrEmpty() )
        return false;

    // The options come first and are ours alone. After the -- everything is text, so a title which starts with a dash is not an option.
    argv.clear();
    argv.push_back( CNotifySendLauncher::GetToolName() );
    argv.push_back( "--app-name=" + m_appName );
    argv.push_back( "--urgency=normal" );
    if ( !m_category.IsNULLOrEmpty() )
        argv.push_back( "--category=" + m_category );
    if ( m_expireTimeMs > 0 )
        argv.push_back( "--expire-time=" + CORE::UInt32ToString( m_expireTimeMs ) );
    argv.push_back( "--" );
    argv.push_back( summary );
    if ( !body.IsNULLOrEmpty() )
        argv.push_back( body );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDesktopMatchAlertSink::Deliver( const MULTIPLAY::CMatchAlert& alert )
{GUCEF_TRACE;

    // A notification that cannot be shown is never the reason for the application to fail
    try
    {
        if ( GUCEF_NULL == m_launcher )
            return;

        if ( !m_launcher->IsAvailable() )
        {
            if ( !m_reportedUnavailable )
            {
                m_reportedUnavailable = true;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DesktopMatchAlertSink: notify-send is not available, desktop notifications are not shown" );
            }
            return;
        }
        m_reportedUnavailable = false;

        if ( m_launcher->GetRunningCount() >= m_maxChildren )
        {
            ++m_droppedCount;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DesktopMatchAlertSink: A notification of kind " << MULTIPLAY::MatchAlertKindToString( alert.kind ) <<
                             " is dropped because " << m_maxChildren << " notify-send programs are running already" );
            return;
        }

        INotifyLauncher::TArgumentVector argv;
        if ( !BuildArguments( alert, argv ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DesktopMatchAlertSink: There is nothing to show for a notification of kind " << MULTIPLAY::MatchAlertKindToString( alert.kind ) );
            return;
        }

        if ( m_launcher->Launch( argv ) )
        {
            ++m_launchedCount;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DesktopMatchAlertSink: notify-send could not be started for a notification of kind " << MULTIPLAY::MatchAlertKindToString( alert.kind ) );
        }
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DesktopMatchAlertSink: Exception while showing a notification, it is not shown" );
    }
}

/*-------------------------------------------------------------------------*/

void
CDesktopMatchAlertSink::Clear( const CORE::CString& matchId )
{GUCEF_TRACE;

    // notify-send cannot take a notification back without reading what it prints, and nothing is read from the programs
    // that are started: the notification goes away when the player or the desktop closes it
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DESKTOP */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
