/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#include "gucefGAMEGUI_CTurnWaitOverlayForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CTurnWaitOverlayForm::TimeExpiredEvent = "GUCEF::GAMEGUI::CTurnWaitOverlayForm::TimeExpiredEvent";

const UInt32 CTurnWaitOverlayForm::DefaultWarningThresholdInMs = 5000;
const UInt32 CTurnWaitOverlayForm::FlashHalfPeriodInMs = 250;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const BoxColour = "28,31,38,240";
const char* const BoxBorderColour = "74,136,232,255";
const char* const ClockColour = "226,229,236,255";
const char* const RedBoxColour = "120,16,16,240";
const char* const RedBoxBorderColour = "255,64,64,255";
const char* const RedClockColour = "255,80,80,255";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CTurnWaitOverlayForm::RegisterEvents( void )
{GUCEF_TRACE;

    TimeExpiredEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CTurnWaitOverlayForm::CTurnWaitOverlayForm( void )
    : GUI::CFormEx()
    , m_message()
    , m_remainingInMs( 0 )
    , m_warningThresholdInMs( DefaultWarningThresholdInMs )
    , m_shownSeconds( -1 )
    , m_showClock( false )
    , m_open( false )
    , m_expired( false )
    , m_red( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTurnWaitOverlayForm::CTurnWaitOverlayForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_message()
    , m_remainingInMs( 0 )
    , m_warningThresholdInMs( DefaultWarningThresholdInMs )
    , m_shownSeconds( -1 )
    , m_showClock( false )
    , m_open( false )
    , m_expired( false )
    , m_red( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTurnWaitOverlayForm::~CTurnWaitOverlayForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CTurnWaitOverlayForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    // a notification, not a window of the player: no title bar, so it cannot be moved or folded, and in the middle of the screen
    CORE::CDataNode* box = GUI::CFormLayout::AddWidgetNode( layout, "Window", "TurnWaitOverlay", "0.5,0 0.5,0", "0.9,0 0,140" );
    GUI::CFormLayout::SetWidgetNodeProperty( *box, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *box, GUI::EWidgetProperty_VAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *box, GUI::EWidgetProperty_MaxSize, "0,480 0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *box, GUI::EWidgetProperty_TitleVisible, "false" );
    GUI::CFormLayout::SetWidgetNodeProperty( *box, GUI::EWidgetProperty_Movable, "false" );
    GUI::CFormLayout::SetWidgetNodeProperty( *box, GUI::EWidgetProperty_Collapsible, "false" );
    GUI::CFormLayout::SetWidgetNodeProperty( *box, GUI::EWidgetProperty_BackgroundColour, BoxColour );
    GUI::CFormLayout::SetWidgetNodeProperty( *box, GUI::EWidgetProperty_BorderColour, BoxBorderColour );
    GUI::CFormLayout::SetWidgetNodeProperty( *box, GUI::EWidgetProperty_BorderWidth, "2" );

    CORE::CDataNode* message = GUI::CFormLayout::AddWidgetNode( *box, "Label", "Message", "0,20 0,16", "1,-40 0,64" );
    GUI::CFormLayout::SetWidgetNodeProperty( *message, GUI::EWidgetProperty_WordWrap, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *message, GUI::EWidgetProperty_HTextAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *message, GUI::EWidgetProperty_VTextAlign, "Center" );

    CORE::CDataNode* clock = GUI::CFormLayout::AddWidgetNode( *box, "Label", "Clock", "0,20 0,88", "1,-40 0,36" );
    GUI::CFormLayout::SetWidgetNodeProperty( *clock, GUI::EWidgetProperty_HTextAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *clock, GUI::EWidgetProperty_VTextAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *clock, GUI::EWidgetProperty_TextColour, ClockColour );
}

/*-------------------------------------------------------------------------*/

bool
CTurnWaitOverlayForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    m_open = false;
    m_red = false;
    m_shownSeconds = -1;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTurnWaitOverlayForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context )
    {
        TEventCallback callback( this, &CTurnWaitOverlayForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

void
CTurnWaitOverlayForm::SetLabelText( const char* widgetName, const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* label = GetWidget( widgetName );
    if ( GUCEF_NULL != label )
        label->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

void
CTurnWaitOverlayForm::ShowTime( void )
{GUCEF_TRACE;

    // the text changes once a second, not on every frame
    Int64 seconds = ( m_remainingInMs + 999 ) / 1000;
    if ( seconds != m_shownSeconds )
    {
        m_shownSeconds = seconds;
        SetLabelText( "Clock", FormatCountdown( m_remainingInMs ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CTurnWaitOverlayForm::IsWarning( void ) const
{GUCEF_TRACE;

    return m_open && m_showClock && m_warningThresholdInMs > 0 && m_remainingInMs <= (Int64) m_warningThresholdInMs;
}

/*-------------------------------------------------------------------------*/

bool
CTurnWaitOverlayForm::IsRed( void ) const
{GUCEF_TRACE;

    return m_red;
}

/*-------------------------------------------------------------------------*/

void
CTurnWaitOverlayForm::ShowPhase( void )
{GUCEF_TRACE;

    // the phase is a function of the time left: red from the threshold for a half period, normal for the next, and so on
    bool red = false;
    if ( IsWarning() )
    {
        Int64 sinceThreshold = (Int64) m_warningThresholdInMs - m_remainingInMs;
        red = 0 == ( ( sinceThreshold / (Int64) FlashHalfPeriodInMs ) % 2 );
    }
    if ( red == m_red )
        return;
    m_red = red;

    GUI::CWidget* box = GetRootWidget();
    if ( GUCEF_NULL != box )
    {
        box->SetProperty( GUI::EWidgetProperty_BackgroundColour, red ? RedBoxColour : BoxColour );
        box->SetProperty( GUI::EWidgetProperty_BorderColour, red ? RedBoxBorderColour : BoxBorderColour );
    }
    GUI::CWidget* clock = GetWidget( "Clock" );
    if ( GUCEF_NULL != clock )
        clock->SetProperty( GUI::EWidgetProperty_TextColour, red ? RedClockColour : ClockColour );
}

/*-------------------------------------------------------------------------*/

bool
CTurnWaitOverlayForm::Open( const CORE::CString& message, Int64 remainingInMs, bool showClock )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;

    m_message = message;
    m_showClock = showClock;
    m_remainingInMs = remainingInMs < 0 ? 0 : remainingInMs;
    m_expired = false;
    m_open = true;
    m_shownSeconds = -1;

    SetLabelText( "Message", m_message );
    GUI::CWidget* clock = GetWidget( "Clock" );
    if ( GUCEF_NULL != clock )
        clock->SetVisibility( m_showClock );
    if ( m_showClock )
        ShowTime();
    ShowPhase();
    return Show();
}

/*-------------------------------------------------------------------------*/

void
CTurnWaitOverlayForm::SetRemainingMs( Int64 remainingInMs )
{GUCEF_TRACE;

    m_remainingInMs = remainingInMs < 0 ? 0 : remainingInMs;
    if ( m_remainingInMs > 0 )
        m_expired = false;
    if ( m_open && m_showClock )
    {
        ShowTime();
        ShowPhase();
    }
}

/*-------------------------------------------------------------------------*/

Int64
CTurnWaitOverlayForm::GetRemainingMs( void ) const
{GUCEF_TRACE;

    return m_remainingInMs;
}

/*-------------------------------------------------------------------------*/

void
CTurnWaitOverlayForm::SetMessage( const CORE::CString& message )
{GUCEF_TRACE;

    m_message = message;
    if ( m_open )
        SetLabelText( "Message", m_message );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTurnWaitOverlayForm::GetMessage( void ) const
{GUCEF_TRACE;

    return m_message;
}

/*-------------------------------------------------------------------------*/

void
CTurnWaitOverlayForm::Close( void )
{GUCEF_TRACE;

    m_open = false;
    ShowPhase();            // back to the normal colours for the next time
    Hide();
}

/*-------------------------------------------------------------------------*/

bool
CTurnWaitOverlayForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

bool
CTurnWaitOverlayForm::IsClockShown( void ) const
{GUCEF_TRACE;

    return m_open && m_showClock;
}

/*-------------------------------------------------------------------------*/

void
CTurnWaitOverlayForm::SetWarningThresholdMs( UInt32 thresholdInMs )
{GUCEF_TRACE;

    m_warningThresholdInMs = thresholdInMs;
    if ( m_open && m_showClock )
        ShowPhase();
}

/*-------------------------------------------------------------------------*/

UInt32
CTurnWaitOverlayForm::GetWarningThresholdMs( void ) const
{GUCEF_TRACE;

    return m_warningThresholdInMs;
}

/*-------------------------------------------------------------------------*/

void
CTurnWaitOverlayForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( !m_open || !m_showClock )
        return;

    m_remainingInMs = deltaInMs >= m_remainingInMs ? 0 : m_remainingInMs - deltaInMs;
    ShowTime();
    ShowPhase();

    if ( 0 == m_remainingInMs && !m_expired )
    {
        m_expired = true;
        NotifyObservers( TimeExpiredEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CTurnWaitOverlayForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
        Update( static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTurnWaitOverlayForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CTurnWaitOverlayForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
