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

#ifndef GUCEF_CORE_CINOTIFIER_H
#include "gucefCORE_CINotifier.h"
#define GUCEF_CORE_CINOTIFIER_H
#endif /* GUCEF_CORE_CINOTIFIER_H ? */

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

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#include "gucefGAMEGUI_CPlayerProfileForm.h"

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

const CORE::CEvent CPlayerProfileForm::ProfileChangedEvent = "GUCEF::GAMEGUI::CPlayerProfileForm::ProfileChangedEvent";
const CORE::CEvent CPlayerProfileForm::CancelledEvent = "GUCEF::GAMEGUI::CPlayerProfileForm::CancelledEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CPlayerProfileForm::RegisterEvents( void )
{GUCEF_TRACE;

    ProfileChangedEvent.Initialize();
    CancelledEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CPlayerProfileForm::CPlayerProfileForm( void )
    : GUI::CFormEx()
    , m_identity()
    , m_provider( GUCEF_NULL )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CPlayerProfileForm::CPlayerProfileForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_identity()
    , m_provider( GUCEF_NULL )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CPlayerProfileForm::~CPlayerProfileForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CPlayerProfileForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "PlayerProfileForm", "0.5,-240 0.5,-150", "0,480 0,300" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, "Title", "Player profile" );

    CORE::CDataNode* nameLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "NameLabel", "0,20 0,56", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *nameLabel, "Text", "The name other players see" );
    CORE::CDataNode* name = GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "DisplayName", "0,20 0,84", "1,-40 0,36" );
    GUI::CFormLayout::SetWidgetNodeProperty( *name, "MaxLength", CORE::UInt32ToString( MaxDisplayNameLength ) );

    CORE::CDataNode* idLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "UserIdLabel", "0,20 0,132", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *idLabel, "Text", "The id of this profile" );
    CORE::CDataNode* id = GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "UserId", "0,20 0,160", "1,-40 0,36" );
    GUI::CFormLayout::SetWidgetNodeProperty( *id, "ReadOnly", "true" );

    CORE::CDataNode* error = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Error", "0,20 0,204", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *error, "TextColour", "255,110,110,255" );

    CORE::CDataNode* save = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Save", "1,-252 1,-56", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *save, "Text", "Save" );
    GUI::CFormLayout::SetWidgetNodeProperty( *save, "StyleClass", "Primary" );
    CORE::CDataNode* cancel = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Cancel", "1,-128 1,-56", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *cancel, "Text", "Cancel" );
}

/*-------------------------------------------------------------------------*/

bool
CPlayerProfileForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    ShowIdentity();
    Hide();
    m_open = false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPlayerProfileForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    const char* buttons[] = { "Save", "Cancel" };
    for ( UInt32 i=0; i<2; ++i )
    {
        GUI::CWidget* button = GetWidget( buttons[ i ] );
        if ( GUCEF_NULL != button )
            SubscribeTo( button, GUI::CWidget::ActivatedEvent );
    }
    GUI::CWidget* name = GetWidget( "DisplayName" );
    if ( GUCEF_NULL != name )
        SubscribeTo( name, GUI::CEditbox::TextEnteredEvent );
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
        SubscribeTo( root, GUI::CWidget::NavigationEvent );
}

/*-------------------------------------------------------------------------*/

void
CPlayerProfileForm::ShowIdentity( void )
{GUCEF_TRACE;

    GUI::CWidget* name = GetWidget( "DisplayName" );
    if ( GUCEF_NULL != name )
        name->SetProperty( "Text", m_identity.displayName );
    GUI::CWidget* id = GetWidget( "UserId" );
    if ( GUCEF_NULL != id )
        id->SetProperty( "Text", m_identity.userId );
    GUI::CWidget* error = GetWidget( "Error" );
    if ( GUCEF_NULL != error )
        error->SetProperty( "Text", CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CPlayerProfileForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;

    if ( GUCEF_NULL != m_provider )
        m_provider->GetIdentity( m_identity );
    ShowIdentity();

    m_open = true;
    bool shown = ShowModal();
    GUI::CGUIContext* context = GetGUIContext();
    GUI::CWidget* name = GetWidget( "DisplayName" );
    if ( shown && GUCEF_NULL != context && GUCEF_NULL != name )
        context->SetFocus( name );
    return shown;
}

/*-------------------------------------------------------------------------*/

void
CPlayerProfileForm::Close( void )
{GUCEF_TRACE;

    m_open = false;
    Hide();
}

/*-------------------------------------------------------------------------*/

bool
CPlayerProfileForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CPlayerProfileForm::SetIdentity( const MULTIPLAY::CPlayerIdentity& identity )
{GUCEF_TRACE;

    m_identity = identity;
    ShowIdentity();
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CPlayerIdentity&
CPlayerProfileForm::GetIdentity( void ) const
{GUCEF_TRACE;

    return m_identity;
}

/*-------------------------------------------------------------------------*/

void
CPlayerProfileForm::SetIdentityProvider( MULTIPLAY::CIIdentityProvider* provider )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_provider )
        UnsubscribeFrom( m_provider );
    m_provider = provider;
    if ( GUCEF_NULL != m_provider )
    {
        SubscribeTo( m_provider, CORE::CINotifier::DestructionEvent );
        if ( !m_provider->GetIdentity( m_identity ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PlayerProfileForm: The identity provider " << m_provider->GetProviderName() << " gave no identity" );
        ShowIdentity();
    }
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CIIdentityProvider*
CPlayerProfileForm::GetIdentityProvider( void ) const
{GUCEF_TRACE;

    return m_provider;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPlayerProfileForm::GetDisplayNameText( void )
{GUCEF_TRACE;

    GUI::CWidget* name = GetWidget( "DisplayName" );
    return GUCEF_NULL != name ? name->GetProperty( "Text" ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CPlayerProfileForm::SetDisplayNameText( const CORE::CString& name )
{GUCEF_TRACE;

    GUI::CWidget* field = GetWidget( "DisplayName" );
    if ( GUCEF_NULL != field )
        field->SetProperty( "Text", name );
}

/*-------------------------------------------------------------------------*/

bool
CPlayerProfileForm::Save( void )
{GUCEF_TRACE;

    GUI::CWidget* error = GetWidget( "Error" );
    CORE::CString name = TrimText( GetDisplayNameText() );
    if ( name.IsNULLOrEmpty() )
    {
        if ( GUCEF_NULL != error )
            error->SetProperty( "Text", "Enter a name" );
        return false;
    }
    if ( GUCEF_NULL != error )
        error->SetProperty( "Text", CORE::CString::Empty );

    if ( name == m_identity.displayName )
    {
        // nothing to save
        Close();
        return true;
    }

    if ( GUCEF_NULL != m_provider && !m_provider->SetDisplayName( name ) )
    {
        if ( GUCEF_NULL != error )
            error->SetProperty( "Text", "The name could not be saved" );
        return false;
    }

    m_identity.displayName = name;
    if ( GUCEF_NULL != m_provider )
        m_provider->GetIdentity( m_identity );
    Close();
    NotifyObservers( ProfileChangedEvent, &m_identity );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPlayerProfileForm::Cancel( void )
{GUCEF_TRACE;

    ShowIdentity();
    Close();
    NotifyObservers( CancelledEvent );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPlayerProfileForm::GetErrorText( void )
{GUCEF_TRACE;

    GUI::CWidget* error = GetWidget( "Error" );
    return GUCEF_NULL != error ? error->GetProperty( "Text" ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CPlayerProfileForm::OnNotify( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CINotifier::DestructionEvent == eventId && GUCEF_NULL != m_provider && notifier == m_provider )
    {
        m_provider = GUCEF_NULL;
        return;
    }
    if ( GUI::CWidget::ActivatedEvent == eventId )
    {
        if ( notifier == GetWidget( "Save" ) )
            Save();
        else
        if ( notifier == GetWidget( "Cancel" ) )
            Cancel();
        return;
    }
    if ( GUI::CEditbox::TextEnteredEvent == eventId )
    {
        if ( m_open )
            Save();
        return;
    }
    if ( GUI::CWidget::NavigationEvent == eventId && GUCEF_NULL != eventData )
    {
        GUI::CNavigationEventData* navigation = static_cast< GUI::CNavigationEventData* >( eventData );
        if ( GUI::NAVIGATION_CANCEL == navigation->direction && GUI::EVENTPHASE_TUNNEL != navigation->phase && m_open )
        {
            navigation->handled = true;
            Cancel();
        }
        return;
    }
    GUI::CFormEx::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPlayerProfileForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CPlayerProfileForm";
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
