/*
 *  AssetWorkbenchLib: the general settings page of the workbench
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

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#include "AssetWorkbenchLib_CWorkbenchGeneralSettingsForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CWorkbenchGeneralSettingsForm::ClearAutomaticThumbnailsRequestedEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchGeneralSettingsForm::ClearAutomaticThumbnailsRequestedEvent";

/*-------------------------------------------------------------------------*/

void
CWorkbenchGeneralSettingsForm::RegisterEvents( void )
{GUCEF_TRACE;

    ClearAutomaticThumbnailsRequestedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWorkbenchGeneralSettingsForm::CWorkbenchGeneralSettingsForm( void )
    : GUI::CGeneralSettingsForm()
    , m_preferences( GUCEF_NULL )
    , m_presentationChanged( false )
    , m_resourceViewStartChanged( false )
    , m_thumbnailsChanged( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWorkbenchGeneralSettingsForm::~CWorkbenchGeneralSettingsForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchGeneralSettingsForm::SetPreferences( CWorkbenchPreferences* preferences )
{GUCEF_TRACE;

    m_preferences = preferences;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchGeneralSettingsForm::TakeResourceViewStartChanged( void )
{GUCEF_TRACE;

    const bool changed = m_resourceViewStartChanged;
    m_resourceViewStartChanged = false;
    return changed;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchGeneralSettingsForm::TakePresentationChanged( void )
{GUCEF_TRACE;

    const bool changed = m_presentationChanged;
    m_presentationChanged = false;
    return changed;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchGeneralSettingsForm::TakeThumbnailsChanged( void )
{GUCEF_TRACE;

    const bool changed = m_thumbnailsChanged;
    m_thumbnailsChanged = false;
    return changed;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchGeneralSettingsForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* page = GUI::CFormLayout::AddWidgetNode( layout, "Panel", "GeneralSettings", "0,0 0,0", "1,0 1,0" );
    CORE::CDataNode* groups = GUI::CFormLayout::AddWidgetNode( *page, "VBox", "Groups", "0,24 0,24", "1,-48 1,-48" );
    GUI::CFormLayout::SetWidgetNodeProperty( *groups, GUI::EWidgetProperty_Spacing, "16" );

    CORE::CDataNode* logGroup = GUI::CFormLayout::AddWidgetNode( *groups, "GroupBox", "LogGroup", "0,0 0,0", "1,0 0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *logGroup, GUI::EWidgetProperty_Caption, "Log" );
    CORE::CDataNode* logWindow = GUI::CFormLayout::AddWidgetNode( *logGroup, "Checkbox", "ConsoleLogWindow", "0,0 0,0", "1,0 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *logWindow, GUI::EWidgetProperty_Text, "Show the log in a console" );
    CORE::CDataNode* ownWindow = GUI::CFormLayout::AddWidgetNode( *logGroup, "Checkbox", "ConsoleLogOwnWindow", "0,0 0,0", "1,-28 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *ownWindow, GUI::EWidgetProperty_HAlign, "Right" );
    GUI::CFormLayout::SetWidgetNodeProperty( *ownWindow, GUI::EWidgetProperty_EnabledBy, "ConsoleLogWindow" );
    GUI::CFormLayout::SetWidgetNodeProperty( *ownWindow, GUI::EWidgetProperty_Text, "In a window of its own, not in the console the program was started from" );
    CORE::CDataNode* userActionLog = GUI::CFormLayout::AddWidgetNode( *logGroup, "Checkbox", "UserActionLog", "0,0 0,0", "1,0 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *userActionLog, GUI::EWidgetProperty_Text, "Keep a log of what I do, for finding problems" );

    CORE::CDataNode* developerGroup = GUI::CFormLayout::AddWidgetNode( *groups, "GroupBox", "DeveloperGroup", "0,0 0,0", "1,0 0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *developerGroup, GUI::EWidgetProperty_Caption, "Developer" );
    CORE::CDataNode* console = GUI::CFormLayout::AddWidgetNode( *developerGroup, "Checkbox", "DeveloperConsole", "0,0 0,0", "1,0 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *console, GUI::EWidgetProperty_Text, "Developer console, drops down with the " + GetKeyDisplayName( GetGeneralSettings().GetDeveloperConsoleKey() ) + " key" );

    // what the workbench adds: the presentation of packs and the automatic thumbnails
    CORE::CDataNode* packsGroup = GUI::CFormLayout::AddWidgetNode( *groups, "GroupBox", "PacksGroup", "0,0 0,0", "1,0 0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *packsGroup, GUI::EWidgetProperty_Caption, "Packs and thumbnails" );
    CORE::CDataNode* presentation = GUI::CFormLayout::AddWidgetNode( *packsGroup, "Checkbox", "OpenPackPresentation", "0,0 0,0", "1,0 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *presentation, GUI::EWidgetProperty_Text, "Show a presentation when a pack is opened" );
    CORE::CDataNode* resourceView = GUI::CFormLayout::AddWidgetNode( *packsGroup, "Checkbox", "OpenPacksInResourceView", "0,0 0,0", "1,0 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *resourceView, GUI::EWidgetProperty_Text, "Open packs in the resource view" );
    CORE::CDataNode* cooked = GUI::CFormLayout::AddWidgetNode( *packsGroup, "Checkbox", "PreferCookedOutput", "0,0 0,0", "1,0 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *cooked, GUI::EWidgetProperty_Text, "Prefer cooked output" );
    CORE::CDataNode* thumbnails = GUI::CFormLayout::AddWidgetNode( *packsGroup, "Checkbox", "AutomaticThumbnails", "0,0 0,0", "1,0 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *thumbnails, GUI::EWidgetProperty_Text, "Create automatic thumbnails" );
    CORE::CDataNode* clear = GUI::CFormLayout::AddWidgetNode( *packsGroup, "Button", "ClearAutomaticThumbnails", "0,0 0,0", "0,280 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *clear, GUI::EWidgetProperty_Text, "Clear automatic thumbnails" );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchGeneralSettingsForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CGeneralSettingsForm::OnPostLayoutLoad();
    GUI::CWidget* clear = GetWidget( "ClearAutomaticThumbnails" );
    if ( GUCEF_NULL != clear )
    {
        TEventCallback callback( this, &CWorkbenchGeneralSettingsForm::OnClearActivated );
        SubscribeTo( clear, GUI::CWidget::ActivatedEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchGeneralSettingsForm::OnClearActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Settings: Clear automatic thumbnails" );
    NotifyObservers( ClearAutomaticThumbnailsRequestedEvent );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchGeneralSettingsForm::ReadSettings( void )
{GUCEF_TRACE;

    GUI::CGeneralSettingsForm::ReadSettings();
    if ( GUCEF_NULL == GetRootWidget() || GUCEF_NULL == m_preferences )
        return;
    SetReadingSettings( true );
    SetWidgetProperty( "OpenPackPresentation", GUI::EWidgetProperty_Checked, CORE::BoolToString( m_preferences->openPackPresentation ) );
    SetWidgetProperty( "OpenPacksInResourceView", GUI::EWidgetProperty_Checked, CORE::BoolToString( m_preferences->openPacksInResourceView ) );
    SetWidgetProperty( "PreferCookedOutput", GUI::EWidgetProperty_Checked, CORE::BoolToString( m_preferences->preferCookedOutput ) );
    SetWidgetProperty( "AutomaticThumbnails", GUI::EWidgetProperty_Checked, CORE::BoolToString( m_preferences->automaticThumbnails ) );
    SetReadingSettings( false );
    SetHasChanges( false );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchGeneralSettingsForm::ApplySettings( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != GetRootWidget() && GUCEF_NULL != m_preferences )
    {
        const bool presentation = CORE::StringToBool( GetWidgetProperty( "OpenPackPresentation", GUI::EWidgetProperty_Checked ), true );
        const bool resourceViewStart = CORE::StringToBool( GetWidgetProperty( "OpenPacksInResourceView", GUI::EWidgetProperty_Checked ), true );
        const bool thumbnails = CORE::StringToBool( GetWidgetProperty( "AutomaticThumbnails", GUI::EWidgetProperty_Checked ), true );
        m_preferences->preferCookedOutput = CORE::StringToBool( GetWidgetProperty( "PreferCookedOutput", GUI::EWidgetProperty_Checked ), true );
        m_presentationChanged = presentation != m_preferences->openPackPresentation;
        m_thumbnailsChanged = thumbnails != m_preferences->automaticThumbnails;
        m_preferences->openPackPresentation = presentation;
        m_resourceViewStartChanged = resourceViewStart != m_preferences->openPacksInResourceView;
        m_preferences->openPacksInResourceView = resourceViewStart;
        m_preferences->automaticThumbnails = thumbnails;
    }
    return GUI::CGeneralSettingsForm::ApplySettings();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchGeneralSettingsForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CWorkbenchGeneralSettingsForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
