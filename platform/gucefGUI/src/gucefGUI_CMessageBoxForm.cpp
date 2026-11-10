/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CMessageBoxForm.h"

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

const CORE::CEvent CMessageBoxForm::ClosedEvent = "GUCEF::GUI::CMessageBoxForm::ClosedEvent";

namespace {

const char* const ButtonNames[] = { "Button1", "Button2", "Button3" };

const char*
ButtonText( CMessageBoxForm::EMessageBoxResult result )
{
    switch ( result )
    {
        case CMessageBoxForm::MESSAGEBOXRESULT_OK:      return "OK";
        case CMessageBoxForm::MESSAGEBOXRESULT_CANCEL:  return "Cancel";
        case CMessageBoxForm::MESSAGEBOXRESULT_YES:     return "Yes";
        case CMessageBoxForm::MESSAGEBOXRESULT_NO:      return "No";
        default:                                        return "";
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMessageBoxForm::RegisterEvents( void )
{GUCEF_TRACE;

    ClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CMessageBoxForm::CMessageBoxForm( void )
    : CFormEx()
    , m_result( MESSAGEBOXRESULT_NONE )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
    for ( UInt32 i=0; i<MaxButtons; ++i )
        m_buttonResults[ i ] = MESSAGEBOXRESULT_NONE;
}

/*-------------------------------------------------------------------------*/

CMessageBoxForm::CMessageBoxForm( CIGUIContext* context )
    : CFormEx( context )
    , m_result( MESSAGEBOXRESULT_NONE )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
    for ( UInt32 i=0; i<MaxButtons; ++i )
        m_buttonResults[ i ] = MESSAGEBOXRESULT_NONE;
}

/*-------------------------------------------------------------------------*/

CMessageBoxForm::~CMessageBoxForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CMessageBoxForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "MessageBoxForm", "0.5,-280 0.5,-130", "0,560 0,260" );
    CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, "Message" );
    CORE::CDataNode* text = CFormLayout::AddWidgetNode( *window, "Label", "Text", "0,20 0,52", "1,-40 1,-180" );
    CFormLayout::SetWidgetNodeProperty( *text, EWidgetProperty_WordWrap, "true" );
    CFormLayout::AddWidgetNode( *window, "Editbox", "Input", "0,20 1,-120", "1,-40 0,36" );
    for ( UInt32 i=0; i<MaxButtons; ++i )
    {
        CORE::CDataNode* button = CFormLayout::AddWidgetNode( *window, "Button", ButtonNames[ i ],
                                                              "1," + CORE::Int32ToString( -128 - 124 * (Int32) ( MaxButtons - 1 - i ) ) + " 1,-60",
                                                              "0,112 0,38" );
        CFormLayout::SetWidgetNodeProperty( *button, EWidgetProperty_Text, "OK" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMessageBoxForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMessageBoxForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    CWidget* button = GetWidget( ButtonNames[ 0 ] );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CMessageBoxForm::OnButton1Activated );
        SubscribeTo( button, CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( ButtonNames[ 1 ] );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CMessageBoxForm::OnButton2Activated );
        SubscribeTo( button, CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( ButtonNames[ 2 ] );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CMessageBoxForm::OnButton3Activated );
        SubscribeTo( button, CWidget::ActivatedEvent, callback );
    }
    CWidget* input = GetWidget( "Input" );
    if ( GUCEF_NULL != input )
    {
        TEventCallback callback( this, &CMessageBoxForm::OnInputTextEntered );
        SubscribeTo( input, CEditbox::TextEnteredEvent, callback );
    }
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CMessageBoxForm::OnNavigation );
        SubscribeTo( root, CWidget::NavigationEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMessageBoxForm::Show( const CString& title, const CString& text, const CString* initialInput, EMessageBoxButtons buttons )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;

    // the buttons fill the slots from the right: the last one is the rightmost
    EMessageBoxResult results[ MaxButtons ] = { MESSAGEBOXRESULT_NONE, MESSAGEBOXRESULT_NONE, MESSAGEBOXRESULT_NONE };
    switch ( buttons )
    {
        case MESSAGEBOXBUTTONS_OK_CANCEL:       results[ 1 ] = MESSAGEBOXRESULT_OK; results[ 2 ] = MESSAGEBOXRESULT_CANCEL; break;
        case MESSAGEBOXBUTTONS_YES_NO:          results[ 1 ] = MESSAGEBOXRESULT_YES; results[ 2 ] = MESSAGEBOXRESULT_NO; break;
        case MESSAGEBOXBUTTONS_YES_NO_CANCEL:   results[ 0 ] = MESSAGEBOXRESULT_YES; results[ 1 ] = MESSAGEBOXRESULT_NO; results[ 2 ] = MESSAGEBOXRESULT_CANCEL; break;
        case MESSAGEBOXBUTTONS_OK:
        default:                                results[ 2 ] = MESSAGEBOXRESULT_OK; break;
    }
    for ( UInt32 i=0; i<MaxButtons; ++i )
    {
        m_buttonResults[ i ] = results[ i ];
        CWidget* button = GetWidget( ButtonNames[ i ] );
        if ( GUCEF_NULL != button )
        {
            button->SetProperty( EWidgetProperty_Text, ButtonText( results[ i ] ) );
            button->SetVisibility( MESSAGEBOXRESULT_NONE != results[ i ] );
            button->SetProperty( EWidgetProperty_StyleClass, MESSAGEBOXRESULT_OK == results[ i ] || MESSAGEBOXRESULT_YES == results[ i ] ? "Primary" : "" );
        }
    }

    CWidget* root = GetRootWidget();
    root->SetProperty( EWidgetProperty_Title, title );
    CWidget* label = GetWidget( "Text" );
    if ( GUCEF_NULL != label )
        label->SetProperty( EWidgetProperty_Text, text );
    CWidget* input = GetWidget( "Input" );
    if ( GUCEF_NULL != input )
    {
        input->SetVisibility( GUCEF_NULL != initialInput );
        input->SetProperty( EWidgetProperty_Text, GUCEF_NULL != initialInput ? *initialInput : CString::Empty );
    }

    m_result = MESSAGEBOXRESULT_NONE;
    m_open = true;
    bool shown = ShowModal();
    CGUIContext* context = GetGUIContext();
    if ( shown && GUCEF_NULL != context && GUCEF_NULL != initialInput && GUCEF_NULL != input )
        context->SetFocus( input );
    return shown;
}

/*-------------------------------------------------------------------------*/

bool
CMessageBoxForm::Open( const CString& title, const CString& text, EMessageBoxButtons buttons )
{GUCEF_TRACE;

    return Show( title, text, GUCEF_NULL, buttons );
}

/*-------------------------------------------------------------------------*/

bool
CMessageBoxForm::OpenInput( const CString& title, const CString& text, const CString& initialInput, EMessageBoxButtons buttons )
{GUCEF_TRACE;

    return Show( title, text, &initialInput, buttons );
}

/*-------------------------------------------------------------------------*/

void
CMessageBoxForm::Choose( EMessageBoxResult result )
{GUCEF_TRACE;

    if ( !m_open )
        return;
    m_open = false;
    m_result = result;
    Hide();
    CORE::TCloneableInt32 eventData( (Int32) result );
    NotifyObservers( ClosedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

CMessageBoxForm::EMessageBoxResult
CMessageBoxForm::GetResult( void ) const
{GUCEF_TRACE;

    return m_result;
}

/*-------------------------------------------------------------------------*/

CString
CMessageBoxForm::GetInputText( void )
{GUCEF_TRACE;

    CWidget* input = GetWidget( "Input" );
    return GUCEF_NULL != input ? input->GetProperty( EWidgetProperty_Text ) : CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CMessageBoxForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CMessageBoxForm::ChooseSlot( UInt32 slot )
{GUCEF_TRACE;

    if ( MESSAGEBOXRESULT_NONE != m_buttonResults[ slot ] )
        Choose( m_buttonResults[ slot ] );
}

/*-------------------------------------------------------------------------*/

void
CMessageBoxForm::OnButton1Activated( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ChooseSlot( 0 );
}

/*-------------------------------------------------------------------------*/

void
CMessageBoxForm::OnButton2Activated( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ChooseSlot( 1 );
}

/*-------------------------------------------------------------------------*/

void
CMessageBoxForm::OnButton3Activated( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ChooseSlot( 2 );
}

/*-------------------------------------------------------------------------*/

void
CMessageBoxForm::OnInputTextEntered( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // Enter chooses the first button that is there
    for ( UInt32 i=0; i<MaxButtons; ++i )
    {
        if ( MESSAGEBOXRESULT_NONE != m_buttonResults[ i ] )
        {
            Choose( m_buttonResults[ i ] );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMessageBoxForm::OnNavigation( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    CNavigationEventData* navigation = static_cast< CNavigationEventData* >( eventData );
    if ( NAVIGATION_CANCEL == navigation->direction && EVENTPHASE_TUNNEL != navigation->phase && m_open )
    {
        // Escape chooses the last button: Cancel, No or OK
        navigation->handled = true;
        Choose( m_buttonResults[ MaxButtons - 1 ] );
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CMessageBoxForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CMessageBoxForm";
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
