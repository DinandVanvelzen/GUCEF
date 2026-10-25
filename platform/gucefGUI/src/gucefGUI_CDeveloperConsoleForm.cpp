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

#ifndef GUCEF_INPUT_CINPUTGLOBAL_H
#include "gucefINPUT_CInputGlobal.h"
#define GUCEF_INPUT_CINPUTGLOBAL_H
#endif /* GUCEF_INPUT_CINPUTGLOBAL_H ? */

#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
#include "CInputController.h"
#define GUCEF_INPUT_CINPUTCONTROLLER_H
#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */

#ifndef GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H
#include "gucefINPUT_CKeyStateChangedEventData.h"
#define GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H
#endif /* GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#ifndef GUCEF_GUI_CTEXTBOX_H
#include "gucefGUI_CTextbox.h"
#define GUCEF_GUI_CTEXTBOX_H
#endif /* GUCEF_GUI_CTEXTBOX_H ? */

#include "gucefGUI_CDeveloperConsoleForm.h"

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

const CORE::CEvent CDeveloperConsoleForm::CommandEnteredEvent = "GUCEF::GUI::CDeveloperConsoleForm::CommandEnteredEvent";
const CORE::CEvent CDeveloperConsoleForm::OpenedEvent = "GUCEF::GUI::CDeveloperConsoleForm::OpenedEvent";
const CORE::CEvent CDeveloperConsoleForm::ClosedEvent = "GUCEF::GUI::CDeveloperConsoleForm::ClosedEvent";

const Int32 CDeveloperConsoleForm::AnyInputContext = -1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::RegisterEvents( void )
{GUCEF_TRACE;

    CommandEnteredEvent.Initialize();
    OpenedEvent.Initialize();
    ClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDeveloperConsoleForm::CDeveloperConsoleForm( void )
    : CFormEx()
    , m_enabled( false )
    , m_open( false )
    , m_swallowText( false )
    , m_keyboardsAttached( false )
    , m_consoleKey( INPUT::KEYCODE_GRAVE )
    , m_inputContextId( AnyInputContext )
    , m_slideTimeInMs( 150 )
    , m_height( 0.45f )
    , m_slide( 0.0f )
    , m_maxLines( 500 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CDeveloperConsoleForm::CDeveloperConsoleForm( CIGUIContext* context )
    : CFormEx( context )
    , m_enabled( false )
    , m_open( false )
    , m_swallowText( false )
    , m_keyboardsAttached( false )
    , m_consoleKey( INPUT::KEYCODE_GRAVE )
    , m_inputContextId( AnyInputContext )
    , m_slideTimeInMs( 150 )
    , m_height( 0.45f )
    , m_slide( 0.0f )
    , m_maxLines( 500 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CDeveloperConsoleForm::~CDeveloperConsoleForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CDeveloperConsoleForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    CORE::CDataNode* panel = CFormLayout::AddWidgetNode( layout, "Panel", "DeveloperConsole", "0,0 0,0", "1,0 " + CORE::FloatToString( m_height ) + ",0" );
    CFormLayout::SetWidgetNodeProperty( *panel, EWidgetProperty_BackgroundColour, "14,16,20,236" );
    CFormLayout::SetWidgetNodeProperty( *panel, EWidgetProperty_BorderColour, "74,136,232,255" );
    CFormLayout::SetWidgetNodeProperty( *panel, EWidgetProperty_BorderWidth, "1" );
    CORE::CDataNode* output = CFormLayout::AddWidgetNode( *panel, "Textbox", "Output", "0,8 0,8", "1,-16 1,-60" );
    CFormLayout::SetWidgetNodeProperty( *output, EWidgetProperty_BackgroundColour, "0,0,0,0" );
    CFormLayout::SetWidgetNodeProperty( *output, EWidgetProperty_BorderColour, "0,0,0,0" );
    CFormLayout::SetWidgetNodeProperty( *output, EWidgetProperty_WordWrap, "true" );
    CFormLayout::AddWidgetNode( *panel, "Editbox", "CommandLine", "0,8 1,-44", "1,-16 0,36" );

    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    m_open = false;
    m_slide = 0.0f;
    ApplySlidePosition();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback textInputCallback( this, &CDeveloperConsoleForm::OnTextInput );
        SubscribeTo( root, CWidget::TextInputEvent, textInputCallback );
    }
    CWidget* commandLine = GetWidget( "CommandLine" );
    if ( GUCEF_NULL != commandLine )
    {
        TEventCallback textEnteredCallback( this, &CDeveloperConsoleForm::OnCommandLineTextEntered );
        SubscribeTo( commandLine, CEditbox::TextEnteredEvent, textEnteredCallback );
    }
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context )
    {
        TEventCallback frameStartedCallback( this, &CDeveloperConsoleForm::OnFrameStarted );
        SubscribeTo( context, CGUIContext::FrameStartedEvent, frameStartedCallback );
    }
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::AttachToKeyboards( void )
{GUCEF_TRACE;

    if ( m_keyboardsAttached )
        return;
    m_keyboardsAttached = true;

    INPUT::CInputController& controller = INPUT::CInputGlobal::Instance()->GetInputController();
    TEventCallback attachedCallback( this, &CDeveloperConsoleForm::OnKeyboardAttached );
    SubscribeTo( &controller, INPUT::CInputController::KeyboardAttachedEvent, attachedCallback );

    // only the keys matter: the other events of the keyboards are not subscribed to
    TEventCallback keyStateCallback( this, &CDeveloperConsoleForm::OnKeyStateChanged );
    const INPUT::CInputController::TKeyboardMap& keyboards = controller.GetKeyboardMap();
    for ( INPUT::CInputController::TKeyboardMap::const_iterator i = keyboards.begin(); i != keyboards.end(); ++i )
        SubscribeTo( (*i).second, INPUT::CKeyboard::KeyStateChangedEvent, keyStateCallback );
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::SetEnabled( bool enabled )
{GUCEF_TRACE;

    m_enabled = enabled;
    if ( enabled )
        AttachToKeyboards();
    else
    if ( m_open )
        Close();
}

/*-------------------------------------------------------------------------*/

bool
CDeveloperConsoleForm::IsEnabled( void ) const
{GUCEF_TRACE;

    return m_enabled;
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::SetConsoleKey( INPUT::KeyCode key )
{GUCEF_TRACE;

    m_consoleKey = key;
}

/*-------------------------------------------------------------------------*/

INPUT::KeyCode
CDeveloperConsoleForm::GetConsoleKey( void ) const
{GUCEF_TRACE;

    return m_consoleKey;
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::SetInputContextId( Int32 inputContextId )
{GUCEF_TRACE;

    m_inputContextId = inputContextId;
}

/*-------------------------------------------------------------------------*/

Int32
CDeveloperConsoleForm::GetInputContextId( void ) const
{GUCEF_TRACE;

    return m_inputContextId;
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::SetSlideTimeInMs( UInt32 slideTimeInMs )
{GUCEF_TRACE;

    m_slideTimeInMs = slideTimeInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CDeveloperConsoleForm::GetSlideTimeInMs( void ) const
{GUCEF_TRACE;

    return m_slideTimeInMs;
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::SetHeight( Float32 fractionOfContext )
{GUCEF_TRACE;

    if ( fractionOfContext < 0.05f )
        fractionOfContext = 0.05f;
    if ( fractionOfContext > 1.0f )
        fractionOfContext = 1.0f;
    m_height = fractionOfContext;
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
        root->SetProperty( EWidgetProperty_Size, "1,0 " + CORE::FloatToString( m_height ) + ",0" );
    ApplySlidePosition();
}

/*-------------------------------------------------------------------------*/

Float32
CDeveloperConsoleForm::GetHeight( void ) const
{GUCEF_TRACE;

    return m_height;
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::SetMaxLines( UInt32 maxLines )
{GUCEF_TRACE;

    m_maxLines = maxLines > 0 ? maxLines : 1;
}

/*-------------------------------------------------------------------------*/

bool
CDeveloperConsoleForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return false;
    if ( m_open )
        return true;

    m_open = true;
    Show();
    CWidget* commandLine = GetWidget( "CommandLine" );
    if ( GUCEF_NULL != commandLine )
        commandLine->SetFocus();
    if ( 0 == m_slideTimeInMs )
        m_slide = 1.0f;
    ApplySlidePosition();
    NotifyObservers( OpenedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDeveloperConsoleForm::Close( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return false;
    if ( !m_open )
        return true;

    m_open = false;
    if ( 0 == m_slideTimeInMs )
        Update( 0 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDeveloperConsoleForm::Toggle( void )
{GUCEF_TRACE;

    return m_open ? Close() : Open();
}

/*-------------------------------------------------------------------------*/

bool
CDeveloperConsoleForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

Float32
CDeveloperConsoleForm::GetSlidePosition( void ) const
{GUCEF_TRACE;

    return m_slide;
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    Float32 step = 0 == m_slideTimeInMs ? 1.0f : (Float32) deltaInMs / (Float32) m_slideTimeInMs;
    if ( m_open )
    {
        if ( m_slide >= 1.0f )
            return;
        m_slide += step;
        if ( m_slide > 1.0f )
            m_slide = 1.0f;
        ApplySlidePosition();
    }
    else
    {
        if ( m_slide <= 0.0f && !IsVisible() )
            return;
        m_slide -= step;
        if ( m_slide <= 0.0f )
        {
            m_slide = 0.0f;
            ApplySlidePosition();
            Hide();
            NotifyObservers( ClosedEvent );
            return;
        }
        ApplySlidePosition();
    }
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::ApplySlidePosition( void )
{GUCEF_TRACE;

    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL == root )
        return;
    // above the top of the context when gone, at the top when all the way down
    Float32 top = -m_height * ( 1.0f - m_slide );
    root->SetProperty( EWidgetProperty_Position, "0,0 " + CORE::FloatToString( top ) + ",0" );
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::AppendLine( const CString& line )
{GUCEF_TRACE;

    CTextbox* output = GetWidgetAs< CTextbox >( "Output" );
    if ( GUCEF_NULL == output )
        return;

    output->AppendLine( line );
    if ( output->GetLineCount() > m_maxLines )
    {
        CTextbox::TStringVector lines = output->GetText();
        UInt32 drop = (UInt32) lines.size() - m_maxLines;
        CString text;
        for ( UInt32 i=drop; i<(UInt32)lines.size(); ++i )
        {
            if ( i > drop )
                text += '\n';
            text += lines[ i ];
        }
        output->SetProperty( EWidgetProperty_Text, text );
    }

    // the newest line in view
    Int32 hidden = (Int32) output->GetDisplayLines().size() - output->GetVisibleLineCount();
    output->SetIntProperty( EWidgetProperty_ScrollPosition, hidden > 0 ? hidden : 0 );
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::ClearOutput( void )
{GUCEF_TRACE;

    CTextbox* output = GetWidgetAs< CTextbox >( "Output" );
    if ( GUCEF_NULL != output )
        output->Clear();
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::OnCommandEntered( void )
{GUCEF_TRACE;

    CEditbox* commandLine = GetWidgetAs< CEditbox >( "CommandLine" );
    if ( GUCEF_NULL == commandLine )
        return;
    CString command = commandLine->GetText();
    if ( command.IsNULLOrEmpty() )
        return;
    commandLine->SetText( CString::Empty );
    AppendLine( "> " + command );
    TCommandEnteredEventData eventData( command );
    NotifyObservers( CommandEnteredEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::OnKeyStateChanged( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const INPUT::CKeyStateChangedEventData* key = static_cast< INPUT::CKeyStateChangedEventData* >( eventData );
    if ( !m_enabled || !key->GetKeyPressedState() )
        return;
    if ( AnyInputContext != m_inputContextId && key->GetContextId() != m_inputContextId )
        return;
    if ( key->GetKeyCode() == m_consoleKey )
    {
        // the character the key types next is not for the command line
        m_swallowText = true;
        Toggle();
    }
    else
    {
        m_swallowText = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::OnTextInput( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    CTextEventData* text = static_cast< CTextEventData* >( eventData );
    if ( m_swallowText && EVENTPHASE_BUBBLE != text->phase )
    {
        m_swallowText = false;
        text->handled = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::OnCommandLineTextEntered( CORE::CNotifier* notifier    ,
                                                 const CORE::CEvent& eventId  ,
                                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    OnCommandEntered();
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    Update( static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CDeveloperConsoleForm::OnKeyboardAttached( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    Int32 deviceId = static_cast< INPUT::CInputController::TKeyboardAttachedEventData* >( eventData )->GetData();
    TEventCallback keyStateCallback( this, &CDeveloperConsoleForm::OnKeyStateChanged );
    SubscribeTo( &INPUT::CInputGlobal::Instance()->GetInputController().GetKeyboard( deviceId ), INPUT::CKeyboard::KeyStateChangedEvent, keyStateCallback );
}

/*-------------------------------------------------------------------------*/

const CString&
CDeveloperConsoleForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CDeveloperConsoleForm";
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
