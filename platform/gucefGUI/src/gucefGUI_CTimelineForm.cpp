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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CLISTBOX_H
#include "gucefGUI_CListBox.h"
#define GUCEF_GUI_CLISTBOX_H
#endif /* GUCEF_GUI_CLISTBOX_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CCHECKBOX_H
#include "gucefGUI_CCheckbox.h"
#define GUCEF_GUI_CCHECKBOX_H
#endif /* GUCEF_GUI_CCHECKBOX_H ? */

#ifndef GUCEF_GUI_CSLIDER_H
#include "gucefGUI_CSlider.h"
#define GUCEF_GUI_CSLIDER_H
#endif /* GUCEF_GUI_CSLIDER_H ? */

#include "gucefGUI_CTimelineForm.h"

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

const CORE::CEvent CTimelineForm::ClipSelectedEvent = "GUCEF::GUI::CTimelineForm::ClipSelectedEvent";
const CORE::CEvent CTimelineForm::TimeChangedEvent = "GUCEF::GUI::CTimelineForm::TimeChangedEvent";
const CORE::CEvent CTimelineForm::PlayStateChangedEvent = "GUCEF::GUI::CTimelineForm::PlayStateChangedEvent";

namespace {

/** The entries of the speed choice */
const char* const SpeedTexts[] = { "0.25x", "0.5x", "1x", "2x", "4x" };
const Float32 SpeedValues[] = { 0.25f, 0.5f, 1.0f, 2.0f, 4.0f };
const Int32 DefaultSpeedChoice = 2;

/** The height of the title bar of the window form, and the room of its rows */
const char* const TitleHeight = "28";

CString
TwoDigits( UInt32 value )
{
    return value < 10 ? "0" + CORE::UInt32ToString( value ) : CORE::UInt32ToString( value );
}

CString
ThreeDigits( UInt32 value )
{
    return value < 10 ? "00" + CORE::UInt32ToString( value ) : ( value < 100 ? "0" + CORE::UInt32ToString( value ) : CORE::UInt32ToString( value ) );
}

CString
FormatOne( UInt32 timeInMs )
{
    const UInt32 seconds = timeInMs / 1000;
    return CORE::UInt32ToString( seconds / 60 ) + ":" + TwoDigits( seconds % 60 ) + "." + ThreeDigits( timeInMs % 1000 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTimelineForm::SClip::SClip( void )
    : name()
    , lengthInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTimelineForm::SClip::SClip( const CString& clipName, UInt32 clipLengthInMs )
    : name( clipName )
    , lengthInMs( clipLengthInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::RegisterEvents( void )
{GUCEF_TRACE;

    ClipSelectedEvent.Initialize();
    TimeChangedEvent.Initialize();
    PlayStateChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CTimelineForm::CTimelineForm( void )
    : CFormEx()
    , m_clips()
    , m_selectedClip( -1 )
    , m_time( 0.0 )
    , m_playing( false )
    , m_speed( 1.0f )
    , m_loop( false )
    , m_stepInMs( 33 )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTimelineForm::CTimelineForm( CIGUIContext* context )
    : CFormEx( context )
    , m_clips()
    , m_selectedClip( -1 )
    , m_time( 0.0 )
    , m_playing( false )
    , m_speed( 1.0f )
    , m_loop( false )
    , m_stepInMs( 33 )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTimelineForm::~CTimelineForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title )
{GUCEF_TRACE;

    CORE::CDataNode* content = GUCEF_NULL;
    if ( asWindow )
    {
        CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "TimelineForm", "0.1,0 0.1,0", "0.8,0 0.8,0" );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, title );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_TitleHeight, TitleHeight );
        content = CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0,28", "1,0 1,-28" );
    }
    else
        content = CFormLayout::AddWidgetNode( layout, "Panel", "TimelineForm", "0,0 0,0", "1,0 1,0" );

    // the clips fill the form above the transport rows
    CFormLayout::AddWidgetNode( *content, "Listbox", "Clips", "0,0 0,0", "1,0 1,-84" );

    struct SButtonSpec { const char* name; const char* text; const char* tooltip; const char* position; const char* size; };
    static const SButtonSpec buttons[] = { { "StepBack", "Back", "Step back", "0,6 1,-78", "0,60 0,30" },
                                           { "Play", "Play", "Play", "0,72 1,-78", "0,60 0,30" },
                                           { "Pause", "Pause", "Pause", "0,138 1,-78", "0,70 0,30" },
                                           { "StepForward", "Forward", "Step forward", "0,214 1,-78", "0,76 0,30" } };
    for ( UInt32 i=0; i<sizeof( buttons ) / sizeof( buttons[ 0 ] ); ++i )
    {
        CORE::CDataNode* button = CFormLayout::AddWidgetNode( *content, "Button", buttons[ i ].name, buttons[ i ].position, buttons[ i ].size );
        CFormLayout::SetWidgetNodeProperty( *button, EWidgetProperty_Text, buttons[ i ].text );
        CFormLayout::SetWidgetNodeProperty( *button, EWidgetProperty_Tooltip, buttons[ i ].tooltip );
    }

    CORE::CDataNode* speed = CFormLayout::AddWidgetNode( *content, "Combobox", "Speed", "0,298 1,-78", "0,90 0,30" );
    CString items;
    for ( UInt32 i=0; i<(UInt32) SpeedCount; ++i )
        items += ( i > 0 ? "\n" : "" ) + CString( SpeedTexts[ i ] );
    CFormLayout::SetWidgetNodeProperty( *speed, EWidgetProperty_Items, items );
    CFormLayout::SetWidgetNodeProperty( *speed, EWidgetProperty_SelectedIndex, CORE::Int32ToString( DefaultSpeedChoice ) );
    CFormLayout::SetWidgetNodeProperty( *speed, EWidgetProperty_Tooltip, "Speed" );

    CORE::CDataNode* loop = CFormLayout::AddWidgetNode( *content, "Checkbox", "Loop", "0,394 1,-78", "0,80 0,30" );
    CFormLayout::SetWidgetNodeProperty( *loop, EWidgetProperty_Text, "Loop" );

    CFormLayout::AddWidgetNode( *content, "Label", "Readout", "0,482 1,-72", "1,-488 0,24" );

    CORE::CDataNode* time = CFormLayout::AddWidgetNode( *content, "Slider", "Time", "0,6 1,-42", "1,-12 0,30" );
    CFormLayout::SetWidgetNodeProperty( *time, EWidgetProperty_Min, "0" );
    CFormLayout::SetWidgetNodeProperty( *time, EWidgetProperty_Max, "0" );
    CFormLayout::SetWidgetNodeProperty( *time, EWidgetProperty_Step, "1" );
    CFormLayout::SetWidgetNodeProperty( *time, EWidgetProperty_Enabled, "false" );
}

/*-------------------------------------------------------------------------*/

bool
CTimelineForm::Build( const CString& title )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetContext() )
        return false;
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, true, title );
    if ( !LoadLayout( layout ) )
        return false;
    return Show();
}

/*-------------------------------------------------------------------------*/

bool
CTimelineForm::Build( CWidget* placeholder )
{GUCEF_TRACE;

    if ( GUCEF_NULL == placeholder )
        return false;
    SetContext( placeholder->GetContext() );
    SetParent( placeholder );
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, false, CString::Empty );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    CWidget* widget = GetWidget( "Play" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CTimelineForm::OnPlayActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Pause" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CTimelineForm::OnPauseActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "StepBack" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CTimelineForm::OnStepBackActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "StepForward" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CTimelineForm::OnStepForwardActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Clips" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CTimelineForm::OnClipsSelectionChanged );
        SubscribeTo( widget, CListbox::SelectionChangedEvent, callback );
    }
    widget = GetWidget( "Time" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CTimelineForm::OnTimeValueChanged );
        SubscribeTo( widget, CWidget::ValueChangedEvent, callback );
    }
    widget = GetWidget( "Speed" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CTimelineForm::OnSpeedValueChanged );
        SubscribeTo( widget, CWidget::ValueChangedEvent, callback );
    }
    widget = GetWidget( "Loop" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback checkedCallback( this, &CTimelineForm::OnLoopChecked );
        SubscribeTo( widget, CCheckbox::BoxCheckedEvent, checkedCallback );
        TEventCallback uncheckedCallback( this, &CTimelineForm::OnLoopUnchecked );
        SubscribeTo( widget, CCheckbox::BoxUncheckedEvent, uncheckedCallback );
    }
    widget = GetRootWidget();
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CTimelineForm::OnKeyDown );
        SubscribeTo( widget, CWidget::KeyDownEvent, callback );
    }
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        m_frameSubscribed = true;
        TEventCallback callback( this, &CTimelineForm::OnFrameStarted );
        SubscribeTo( context, CGUIContext::FrameStartedEvent, callback );
    }

    ShowClips();
    UpdateWidgets();
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::ShowClips( void )
{GUCEF_TRACE;

    CListbox* list = GetWidgetAs< CListbox >( "Clips" );
    if ( GUCEF_NULL == list )
        return;
    CListbox::TStringVector items;
    for ( UInt32 i=0; i<(UInt32)m_clips.size(); ++i )
        items.push_back( m_clips[ i ].name + "  (" + FormatOne( m_clips[ i ].lengthInMs ) + ")" );
    m_updatingWidgets = true;
    list->SetItems( items );
    list->SetSelectedIndex( m_selectedClip );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::UpdateWidgets( void )
{GUCEF_TRACE;

    const UInt32 length = GetClipLength();
    const bool haveClip = m_selectedClip >= 0 && length > 0;
    m_updatingWidgets = true;

    CWidget* time = GetWidget( "Time" );
    if ( GUCEF_NULL != time )
    {
        time->SetFloatProperty( EWidgetProperty_Max, (Float32) length );
        time->SetFloatProperty( EWidgetProperty_Value, (Float32) GetTime() );
        time->SetBoolProperty( EWidgetProperty_Enabled, haveClip );
    }
    CWidget* readout = GetWidget( "Readout" );
    if ( GUCEF_NULL != readout )
        readout->SetProperty( EWidgetProperty_Text, FormatTime( GetTime(), length ) );
    CWidget* play = GetWidget( "Play" );
    if ( GUCEF_NULL != play )
        play->SetBoolProperty( EWidgetProperty_Enabled, haveClip && !m_playing );
    CWidget* pause = GetWidget( "Pause" );
    if ( GUCEF_NULL != pause )
        pause->SetBoolProperty( EWidgetProperty_Enabled, m_playing );
    CWidget* back = GetWidget( "StepBack" );
    if ( GUCEF_NULL != back )
        back->SetBoolProperty( EWidgetProperty_Enabled, haveClip );
    CWidget* forward = GetWidget( "StepForward" );
    if ( GUCEF_NULL != forward )
        forward->SetBoolProperty( EWidgetProperty_Enabled, haveClip );
    CWidget* loop = GetWidget( "Loop" );
    if ( GUCEF_NULL != loop )
        loop->SetBoolProperty( EWidgetProperty_Checked, m_loop );
    CCombobox* speed = GetWidgetAs< CCombobox >( "Speed" );
    if ( GUCEF_NULL != speed )
    {
        Int32 choice = DefaultSpeedChoice;
        for ( UInt32 i=0; i<(UInt32) SpeedCount; ++i )
            if ( SpeedValues[ i ] == m_speed )
                choice = (Int32) i;
        if ( speed->GetSelectedIndex() != choice )
            speed->SetSelectedIndex( choice );
    }
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::SetClips( const TClipVector& clips )
{GUCEF_TRACE;

    const bool wasPlaying = m_playing;
    const Int32 previous = m_selectedClip;
    m_clips = clips;
    m_selectedClip = -1;
    m_time = 0.0;
    m_playing = false;
    ShowClips();
    UpdateWidgets();
    if ( wasPlaying )
        ChangePlayState( false );
    if ( previous >= 0 )
    {
        CORE::TCloneableInt32 eventData( -1 );
        NotifyObservers( ClipSelectedEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::AddClip( const CString& name, UInt32 lengthInMs )
{GUCEF_TRACE;

    m_clips.push_back( TClip( name, lengthInMs ) );
    ShowClips();
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::ClearClips( void )
{GUCEF_TRACE;

    SetClips( TClipVector() );
}

/*-------------------------------------------------------------------------*/

const CTimelineForm::TClipVector&
CTimelineForm::GetClips( void ) const
{GUCEF_TRACE;

    return m_clips;
}

/*-------------------------------------------------------------------------*/

bool
CTimelineForm::SelectClip( Int32 index )
{GUCEF_TRACE;

    if ( index < -1 || index >= (Int32) m_clips.size() )
        return false;
    if ( index == m_selectedClip )
        return true;
    m_selectedClip = index;
    const bool timeMoved = m_time != 0.0;
    m_time = 0.0;
    if ( index < 0 || 0 == m_clips[ index ].lengthInMs )
    {
        const bool wasPlaying = m_playing;
        m_playing = false;
        if ( wasPlaying )
            ChangePlayState( false );
    }
    ShowClips();
    UpdateWidgets();
    CORE::TCloneableInt32 eventData( index );
    NotifyObservers( ClipSelectedEvent, &eventData );
    if ( timeMoved )
        ChangeTime( 0.0, false );
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CTimelineForm::GetSelectedClip( void ) const
{GUCEF_TRACE;

    return m_selectedClip;
}

/*-------------------------------------------------------------------------*/

UInt32
CTimelineForm::GetClipLength( void ) const
{GUCEF_TRACE;

    return m_selectedClip >= 0 && m_selectedClip < (Int32) m_clips.size() ? m_clips[ m_selectedClip ].lengthInMs : 0;
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::ChangeTime( Float64 timeInMs, bool scrubbed )
{GUCEF_TRACE;

    m_time = timeInMs;
    UpdateWidgets();
    TTimeChange change;
    change.timeInMs = GetTime();
    change.scrubbed = scrubbed;
    TTimeChangedEventData eventData( change );
    NotifyObservers( TimeChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CTimelineForm::SetTime( UInt32 timeInMs )
{GUCEF_TRACE;

    if ( m_selectedClip < 0 )
        return false;
    const UInt32 length = GetClipLength();
    if ( timeInMs > length )
        timeInMs = length;
    if ( (Float64) timeInMs == m_time )
        return true;
    ChangeTime( (Float64) timeInMs, false );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CTimelineForm::GetTime( void ) const
{GUCEF_TRACE;

    return (UInt32) m_time;
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::ChangePlayState( bool playing )
{GUCEF_TRACE;

    m_playing = playing;
    UpdateWidgets();
    CORE::TCloneableBool eventData( playing );
    NotifyObservers( PlayStateChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CTimelineForm::Play( void )
{GUCEF_TRACE;

    const UInt32 length = GetClipLength();
    if ( m_selectedClip < 0 || 0 == length )
        return false;
    if ( m_playing )
        return true;
    if ( m_time >= (Float64) length )
        ChangeTime( 0.0, false );
    ChangePlayState( true );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::Pause( void )
{GUCEF_TRACE;

    if ( m_playing )
        ChangePlayState( false );
}

/*-------------------------------------------------------------------------*/

bool
CTimelineForm::IsPlaying( void ) const
{GUCEF_TRACE;

    return m_playing;
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::SetStepInMs( UInt32 stepInMs )
{GUCEF_TRACE;

    m_stepInMs = stepInMs > 0 ? stepInMs : 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CTimelineForm::GetStepInMs( void ) const
{GUCEF_TRACE;

    return m_stepInMs;
}

/*-------------------------------------------------------------------------*/

bool
CTimelineForm::StepForward( void )
{GUCEF_TRACE;

    if ( m_selectedClip < 0 )
        return false;
    return SetTime( GetTime() + m_stepInMs );
}

/*-------------------------------------------------------------------------*/

bool
CTimelineForm::StepBack( void )
{GUCEF_TRACE;

    if ( m_selectedClip < 0 )
        return false;
    return SetTime( GetTime() > m_stepInMs ? GetTime() - m_stepInMs : 0 );
}

/*-------------------------------------------------------------------------*/

bool
CTimelineForm::SetSpeed( Float32 speed )
{GUCEF_TRACE;

    if ( !( speed > 0.0f ) )
        return false;
    m_speed = speed;
    UpdateWidgets();
    return true;
}

/*-------------------------------------------------------------------------*/

Float32
CTimelineForm::GetSpeed( void ) const
{GUCEF_TRACE;

    return m_speed;
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::SetLooping( bool loop )
{GUCEF_TRACE;

    m_loop = loop;
    UpdateWidgets();
}

/*-------------------------------------------------------------------------*/

bool
CTimelineForm::IsLooping( void ) const
{GUCEF_TRACE;

    return m_loop;
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( !m_playing || 0 == deltaInMs )
        return;
    const Float64 length = (Float64) GetClipLength();
    Float64 next = m_time + (Float64) deltaInMs * (Float64) m_speed;
    bool ended = false;
    if ( next >= length )
    {
        if ( m_loop && length > 0.0 )
        {
            while ( next >= length )
                next -= length;
        }
        else
        {
            next = length;
            ended = true;
        }
    }
    if ( (UInt32) next != (UInt32) m_time || ended )
        ChangeTime( next, false );
    else
        m_time = next;
    if ( ended )
        ChangePlayState( false );
}

/*-------------------------------------------------------------------------*/

CString
CTimelineForm::FormatTime( UInt32 timeInMs, UInt32 lengthInMs )
{GUCEF_TRACE;

    return FormatOne( timeInMs ) + " / " + FormatOne( lengthInMs );
}

/*-------------------------------------------------------------------------*/

Float32
CTimelineForm::GetSpeedOfChoice( UInt32 index )
{GUCEF_TRACE;

    return index < (UInt32) SpeedCount ? SpeedValues[ index ] : 1.0f;
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::OnPlayActivated( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Play();
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::OnPauseActivated( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Pause();
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::OnStepBackActivated( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    StepBack();
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::OnStepForwardActivated( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    StepForward();
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::OnClipsSelectionChanged( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CListbox* list = GetWidgetAs< CListbox >( "Clips" );
    if ( GUCEF_NULL != list )
        SelectClip( list->GetSelectedIndex() );
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::OnTimeValueChanged( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets || m_selectedClip < 0 )
        return;
    CSlider* slider = GetWidgetAs< CSlider >( "Time" );
    if ( GUCEF_NULL == slider )
        return;
    Float32 value = slider->GetValue();
    if ( value < 0.0f )
        value = 0.0f;
    Float64 time = (Float64) value;
    if ( time > (Float64) GetClipLength() )
        time = (Float64) GetClipLength();
    if ( (UInt32) time != GetTime() )
        ChangeTime( time, true );
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::OnSpeedValueChanged( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CCombobox* speed = GetWidgetAs< CCombobox >( "Speed" );
    if ( GUCEF_NULL != speed && speed->GetSelectedIndex() >= 0 )
        SetSpeed( GetSpeedOfChoice( (UInt32) speed->GetSelectedIndex() ) );
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::OnLoopChecked( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        m_loop = true;
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::OnLoopUnchecked( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_updatingWidgets )
        m_loop = false;
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::OnKeyDown( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CKeyEventData* key = static_cast< CKeyEventData* >( eventData );
    if ( GUCEF_NULL == key || key->handled || INPUT::KEYCODE_SPACE != key->keyCode || EVENTPHASE_BUBBLE != key->phase )
        return;

    // the list, the slider and the form itself: a button or check box keeps the key for its own use
    if ( GUCEF_NULL != key->source && key->source != GetWidget( "Clips" ) && key->source != GetWidget( "Time" ) && key->source != GetRootWidget() )
        return;
    key->handled = true;
    if ( m_playing )
        Pause();
    else
        Play();
}

/*-------------------------------------------------------------------------*/

void
CTimelineForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
        Update( static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

const CString&
CTimelineForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CTimelineForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
