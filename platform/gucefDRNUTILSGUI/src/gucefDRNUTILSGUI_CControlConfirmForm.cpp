/*
 *  gucefDRNUTILSGUI: GUCEF module providing the forms that view and control job schedulers and the task manager
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

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#include "gucefDRNUTILSGUI_CControlConfirmForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILSGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CControlConfirmForm::ClassTypeName = "GUCEF::DRNUTILSGUI::CControlConfirmForm";
const CORE::CEvent CControlConfirmForm::ConfirmedEvent = "GUCEF::DRNUTILSGUI::CControlConfirmForm::ConfirmedEvent";
const CORE::CEvent CControlConfirmForm::CancelledEvent = "GUCEF::DRNUTILSGUI::CControlConfirmForm::CancelledEvent";

/*-------------------------------------------------------------------------*/

void
CControlConfirmForm::RegisterEvents( void )
{GUCEF_TRACE;

    ConfirmedEvent.Initialize();
    CancelledEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CControlConfirmForm::CControlConfirmForm( void )
    : GUI::CFormEx()
    , m_reason()
    , m_actionText()
    , m_targetText()
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CControlConfirmForm::CControlConfirmForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_reason()
    , m_actionText()
    , m_targetText()
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CControlConfirmForm::~CControlConfirmForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CControlConfirmForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "ControlConfirmForm", "0.5,0 0.5,0", "0.9,0 0.9,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Confirm" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_VAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0,520 0,300" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    CORE::CDataNode* action = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Action", "0,20 0,56", "1,-40 0,28" );
    GUI::CFormLayout::SetWidgetNodeProperty( *action, GUI::EWidgetProperty_WordWrap, "true" );
    CORE::CDataNode* target = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Target", "0,20 0,88", "1,-40 0,28" );
    GUI::CFormLayout::SetWidgetNodeProperty( *target, GUI::EWidgetProperty_WordWrap, "true" );

    CORE::CDataNode* reasonLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "ReasonLabel", "0,20 0,128", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *reasonLabel, GUI::EWidgetProperty_Text, "Reason (goes into the audit trail of the node)" );
    GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "Reason", "0,20 0,156", "1,-40 0,36" );

    CORE::CDataNode* confirm = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Confirm", "1,-252 1,-56", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *confirm, GUI::EWidgetProperty_Text, "Confirm" );
    GUI::CFormLayout::SetWidgetNodeProperty( *confirm, GUI::EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* cancel = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Cancel", "1,-128 1,-56", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *cancel, GUI::EWidgetProperty_Text, "Cancel" );
}

/*-------------------------------------------------------------------------*/

bool
CControlConfirmForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    m_open = false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CControlConfirmForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    GUI::CWidget* confirm = GetWidget( "Confirm" );
    if ( GUCEF_NULL != confirm )
    {
        TEventCallback callback( this, &CControlConfirmForm::OnConfirmActivated );
        SubscribeTo( confirm, GUI::CWidget::ActivatedEvent, callback );
    }
    GUI::CWidget* cancel = GetWidget( "Cancel" );
    if ( GUCEF_NULL != cancel )
    {
        TEventCallback callback( this, &CControlConfirmForm::OnCancelActivated );
        SubscribeTo( cancel, GUI::CWidget::ActivatedEvent, callback );
    }
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CControlConfirmForm::OnNavigation );
        SubscribeTo( root, GUI::CWidget::NavigationEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

void
CControlConfirmForm::SetLabelText( const char* widgetName, const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* label = GetWidget( widgetName );
    if ( GUCEF_NULL != label )
        label->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

bool
CControlConfirmForm::Ask( const CORE::CString& title, const CORE::CString& actionText, const CORE::CString& targetText )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;

    m_actionText = actionText;
    m_targetText = targetText;
    SetLabelText( "Action", actionText );
    SetLabelText( "Target", targetText );
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
        root->SetProperty( GUI::EWidgetProperty_Title, title );
    GUI::CWidget* reason = GetWidget( "Reason" );
    if ( GUCEF_NULL != reason )
        reason->SetProperty( GUI::EWidgetProperty_Text, CORE::CString::Empty );

    m_open = true;
    return ShowModal();
}

/*-------------------------------------------------------------------------*/

bool
CControlConfirmForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CControlConfirmForm::Confirm( const CORE::CString& reason )
{GUCEF_TRACE;

    if ( !m_open )
        return;
    m_reason = reason;
    m_open = false;
    Hide();
    TConfirmedEventData eventData( reason );
    NotifyObservers( ConfirmedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CControlConfirmForm::Cancel( void )
{GUCEF_TRACE;

    if ( !m_open )
        return;
    m_open = false;
    Hide();
    NotifyObservers( CancelledEvent );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CControlConfirmForm::GetReason( void ) const
{GUCEF_TRACE;

    return m_reason;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CControlConfirmForm::GetActionText( void ) const
{GUCEF_TRACE;

    return m_actionText;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CControlConfirmForm::GetTargetText( void ) const
{GUCEF_TRACE;

    return m_targetText;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CControlConfirmForm::GetTypedReason( void ) const
{GUCEF_TRACE;

    const GUI::CWidget* reason = const_cast< CControlConfirmForm* >( this )->GetWidget( "Reason" );
    return GUCEF_NULL != reason ? reason->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CControlConfirmForm::OnConfirmActivated( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Confirm( GetTypedReason() );
}

/*-------------------------------------------------------------------------*/

void
CControlConfirmForm::OnCancelActivated( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Cancel();
}

/*-------------------------------------------------------------------------*/

void
CControlConfirmForm::OnNavigation( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CNavigationEventData* navigation = static_cast< GUI::CNavigationEventData* >( eventData );
    if ( GUI::NAVIGATION_CANCEL == navigation->direction && GUI::EVENTPHASE_TUNNEL != navigation->phase && m_open )
    {
        navigation->handled = true;
        Cancel();
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CControlConfirmForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILSGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
