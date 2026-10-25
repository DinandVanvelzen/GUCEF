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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_GUI_CDEVELOPERCONSOLEFORM_H
#include "gucefGUI_CDeveloperConsoleForm.h"
#define GUCEF_GUI_CDEVELOPERCONSOLEFORM_H
#endif /* GUCEF_GUI_CDEVELOPERCONSOLEFORM_H ? */

#include "gucefGUI_CGeneralSettingsForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGeneralSettingsForm::CGeneralSettingsForm( void )
    : CSettingsPageForm()
    , m_settings()
    , m_console( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGeneralSettingsForm::~CGeneralSettingsForm()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGeneralSettingsForm::SetGeneralSettings( const CGeneralSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

const CGeneralSettings&
CGeneralSettingsForm::GetGeneralSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CGeneralSettingsForm::SetDeveloperConsole( CDeveloperConsoleForm* console )
{GUCEF_TRACE;

    m_console = console;
}

/*-------------------------------------------------------------------------*/

CDeveloperConsoleForm*
CGeneralSettingsForm::GetDeveloperConsole( void ) const
{GUCEF_TRACE;

    return m_console;
}

/*-------------------------------------------------------------------------*/

void
CGeneralSettingsForm::ApplyGeneralSettings( const CGeneralSettings& settings, CDeveloperConsoleForm* console )
{GUCEF_TRACE;

    CORE::CLogManager& logManager = CORE::CCoreGlobal::Instance()->GetLogManager();
    logManager.SetNativeConsoleLoggerInOwnWindow( settings.GetConsoleLogOwnWindow() );
    logManager.SetNativeConsoleLoggerEnabled( settings.GetConsoleLogWindow() );
    logManager.SetUserActionLogEnabled( settings.GetUserActionLog(), settings.GetUserActionLogFile() );

    // what the player does is not even made into messages unless this is on, for all loggers (the console as well as the file)
    logManager.SetUserLogEnabled( settings.GetUserActionLog() );
    if ( GUCEF_NULL != console )
    {
        console->SetConsoleKey( settings.GetDeveloperConsoleKey() );
        console->SetEnabled( settings.GetDeveloperConsole() );
    }
}

/*-------------------------------------------------------------------------*/

CString
CGeneralSettingsForm::GetTitle( void ) const
{GUCEF_TRACE;

    return "General";
}

/*-------------------------------------------------------------------------*/

CString
CGeneralSettingsForm::GetKeyDisplayName( INPUT::KeyCode key )
{GUCEF_TRACE;

    if ( INPUT::KEYCODE_GRAVE == key )
        return "~";
    return INPUT::KeyCodeToString( key );
}

/*-------------------------------------------------------------------------*/

void
CGeneralSettingsForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* page = CFormLayout::AddWidgetNode( layout, "Panel", "GeneralSettings", "0,0 0,0", "1,0 1,0" );

    // the options in groups below each other; an option that only means something with another is indented below it
    CORE::CDataNode* groups = CFormLayout::AddWidgetNode( *page, "VBox", "Groups", "0,24 0,24", "1,-48 1,-48" );
    CFormLayout::SetWidgetNodeProperty( *groups, EWidgetProperty_Spacing, "16" );

    CORE::CDataNode* logGroup = CFormLayout::AddWidgetNode( *groups, "GroupBox", "LogGroup", "0,0 0,0", "1,0 0,0" );
    CFormLayout::SetWidgetNodeProperty( *logGroup, EWidgetProperty_Caption, "Log" );

    CORE::CDataNode* logWindow = CFormLayout::AddWidgetNode( *logGroup, "Checkbox", "ConsoleLogWindow", "0,0 0,0", "1,0 0,32" );
    CFormLayout::SetWidgetNodeProperty( *logWindow, EWidgetProperty_Text, "Show the log in a console" );

    CORE::CDataNode* ownWindow = CFormLayout::AddWidgetNode( *logGroup, "Checkbox", "ConsoleLogOwnWindow", "0,0 0,0", "1,-28 0,32" );
    CFormLayout::SetWidgetNodeProperty( *ownWindow, EWidgetProperty_HAlign, "Right" );
    CFormLayout::SetWidgetNodeProperty( *ownWindow, EWidgetProperty_EnabledBy, "ConsoleLogWindow" );
    CFormLayout::SetWidgetNodeProperty( *ownWindow, EWidgetProperty_Text, "In a window of its own, not in the console the program was started from" );

    CORE::CDataNode* userActionLog = CFormLayout::AddWidgetNode( *logGroup, "Checkbox", "UserActionLog", "0,0 0,0", "1,0 0,32" );
    CFormLayout::SetWidgetNodeProperty( *userActionLog, EWidgetProperty_Text, "Keep a log of what I do, for finding problems" );

    CORE::CDataNode* developerGroup = CFormLayout::AddWidgetNode( *groups, "GroupBox", "DeveloperGroup", "0,0 0,0", "1,0 0,0" );
    CFormLayout::SetWidgetNodeProperty( *developerGroup, EWidgetProperty_Caption, "Developer" );

    CORE::CDataNode* console = CFormLayout::AddWidgetNode( *developerGroup, "Checkbox", "DeveloperConsole", "0,0 0,0", "1,0 0,32" );
    CFormLayout::SetWidgetNodeProperty( *console, EWidgetProperty_Text, "Developer console, drops down with the " + GetKeyDisplayName( m_settings.GetDeveloperConsoleKey() ) + " key" );
}

/*-------------------------------------------------------------------------*/

void
CGeneralSettingsForm::ReadSettings( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return;

    SetReadingSettings( true );
    SetWidgetProperty( "ConsoleLogWindow", EWidgetProperty_Checked, CORE::BoolToString( m_settings.GetConsoleLogWindow() ) );
    SetWidgetProperty( "ConsoleLogOwnWindow", EWidgetProperty_Checked, CORE::BoolToString( m_settings.GetConsoleLogOwnWindow() ) );
    SetWidgetProperty( "DeveloperConsole", EWidgetProperty_Checked, CORE::BoolToString( m_settings.GetDeveloperConsole() ) );
    SetWidgetProperty( "UserActionLog", EWidgetProperty_Checked, CORE::BoolToString( m_settings.GetUserActionLog() ) );
    SetReadingSettings( false );
    SetHasChanges( false );
}

/*-------------------------------------------------------------------------*/

bool
CGeneralSettingsForm::ApplySettings( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != GetRootWidget() )
    {
        m_settings.SetConsoleLogWindow( CORE::StringToBool( GetWidgetProperty( "ConsoleLogWindow", EWidgetProperty_Checked ), false ) );
        m_settings.SetConsoleLogOwnWindow( CORE::StringToBool( GetWidgetProperty( "ConsoleLogOwnWindow", EWidgetProperty_Checked ), true ) );
        m_settings.SetDeveloperConsole( CORE::StringToBool( GetWidgetProperty( "DeveloperConsole", EWidgetProperty_Checked ), false ) );
        m_settings.SetUserActionLog( CORE::StringToBool( GetWidgetProperty( "UserActionLog", EWidgetProperty_Checked ), false ) );
    }
    ApplyGeneralSettings( m_settings, m_console );
    SetHasChanges( false );
    NotifyObservers( SettingsAppliedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CGeneralSettingsForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CGeneralSettingsForm";
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
