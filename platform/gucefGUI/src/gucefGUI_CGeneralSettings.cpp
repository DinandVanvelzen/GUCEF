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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefGUI_CGeneralSettings.h"

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

const CString CGeneralSettings::ConfigSectionName = "GeneralSettings";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CGeneralSettings::ParseWindowSystem( const CString& text, EWindowSystem& windowSystem )
{GUCEF_TRACE;

    const CString lower = text.Lowercase().Trim( true ).Trim( false );
    if ( "auto" == lower )
        windowSystem = WINDOWSYSTEM_AUTO;
    else
    if ( "wayland" == lower )
        windowSystem = WINDOWSYSTEM_WAYLAND;
    else
    if ( "x11" == lower || "xwayland" == lower )
        windowSystem = WINDOWSYSTEM_X11;
    else
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

const char*
CGeneralSettings::WindowSystemToString( EWindowSystem windowSystem )
{GUCEF_TRACE;

    switch ( windowSystem )
    {
        case WINDOWSYSTEM_AUTO :    return "auto";
        case WINDOWSYSTEM_WAYLAND : return "wayland";
        default :                   return "x11";
    }
}

/*-------------------------------------------------------------------------*/

CGeneralSettings::EWindowSystem
CGeneralSettings::ResolveWindowSystem( EWindowSystem requested, bool waylandAvailable )
{GUCEF_TRACE;

    if ( WINDOWSYSTEM_AUTO == requested )
        return waylandAvailable ? WINDOWSYSTEM_WAYLAND : WINDOWSYSTEM_X11;
    return requested;
}

/*-------------------------------------------------------------------------*/

CGeneralSettings::CGeneralSettings( void )
    : CORE::CIConfigurable()
    , m_consoleLogWindow( false )
    , m_consoleLogOwnWindow( true )
    , m_developerConsole( false )
    , m_developerConsoleKey( INPUT::KEYCODE_GRAVE )
    , m_userActionLog( false )
    , m_userActionLogFile( "$CACHEDIR$/GUCEF/UserActions.log" )
    , m_windowSystem( WINDOWSYSTEM_X11 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGeneralSettings::CGeneralSettings( const CGeneralSettings& src )
    : CORE::CIConfigurable()
    , m_consoleLogWindow( src.m_consoleLogWindow )
    , m_consoleLogOwnWindow( src.m_consoleLogOwnWindow )
    , m_developerConsole( src.m_developerConsole )
    , m_developerConsoleKey( src.m_developerConsoleKey )
    , m_userActionLog( src.m_userActionLog )
    , m_userActionLogFile( src.m_userActionLogFile )
    , m_windowSystem( src.m_windowSystem )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGeneralSettings::~CGeneralSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGeneralSettings&
CGeneralSettings::operator=( const CGeneralSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_consoleLogWindow = src.m_consoleLogWindow;
        m_consoleLogOwnWindow = src.m_consoleLogOwnWindow;
        m_developerConsole = src.m_developerConsole;
        m_developerConsoleKey = src.m_developerConsoleKey;
        m_userActionLog = src.m_userActionLog;
        m_userActionLogFile = src.m_userActionLogFile;
        m_windowSystem = src.m_windowSystem;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CGeneralSettings::SetConsoleLogWindow( bool enabled )
{GUCEF_TRACE;

    m_consoleLogWindow = enabled;
}

/*-------------------------------------------------------------------------*/

bool
CGeneralSettings::GetConsoleLogWindow( void ) const
{GUCEF_TRACE;

    return m_consoleLogWindow;
}

/*-------------------------------------------------------------------------*/

void
CGeneralSettings::SetConsoleLogOwnWindow( bool ownWindow )
{GUCEF_TRACE;

    m_consoleLogOwnWindow = ownWindow;
}

/*-------------------------------------------------------------------------*/

bool
CGeneralSettings::GetConsoleLogOwnWindow( void ) const
{GUCEF_TRACE;

    return m_consoleLogOwnWindow;
}

/*-------------------------------------------------------------------------*/

void
CGeneralSettings::SetDeveloperConsole( bool enabled )
{GUCEF_TRACE;

    m_developerConsole = enabled;
}

/*-------------------------------------------------------------------------*/

bool
CGeneralSettings::GetDeveloperConsole( void ) const
{GUCEF_TRACE;

    return m_developerConsole;
}

/*-------------------------------------------------------------------------*/

void
CGeneralSettings::SetDeveloperConsoleKey( INPUT::KeyCode key )
{GUCEF_TRACE;

    m_developerConsoleKey = key;
}

/*-------------------------------------------------------------------------*/

INPUT::KeyCode
CGeneralSettings::GetDeveloperConsoleKey( void ) const
{GUCEF_TRACE;

    return m_developerConsoleKey;
}

/*-------------------------------------------------------------------------*/

void
CGeneralSettings::SetUserActionLog( bool enabled )
{GUCEF_TRACE;

    m_userActionLog = enabled;
}

/*-------------------------------------------------------------------------*/

bool
CGeneralSettings::GetUserActionLog( void ) const
{GUCEF_TRACE;

    return m_userActionLog;
}

/*-------------------------------------------------------------------------*/

void
CGeneralSettings::SetUserActionLogFile( const CString& filePath )
{GUCEF_TRACE;

    m_userActionLogFile = filePath;
}

/*-------------------------------------------------------------------------*/

const CString&
CGeneralSettings::GetUserActionLogFile( void ) const
{GUCEF_TRACE;

    return m_userActionLogFile;
}

/*-------------------------------------------------------------------------*/

void
CGeneralSettings::SetWindowSystem( EWindowSystem windowSystem )
{GUCEF_TRACE;

    m_windowSystem = windowSystem;
}

/*-------------------------------------------------------------------------*/

CGeneralSettings::EWindowSystem
CGeneralSettings::GetWindowSystem( void ) const
{GUCEF_TRACE;

    return m_windowSystem;
}

/*-------------------------------------------------------------------------*/

bool
CGeneralSettings::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;
    section->SetAttribute( "consoleLogWindow", m_consoleLogWindow );
    section->SetAttribute( "consoleLogOwnWindow", m_consoleLogOwnWindow );
    section->SetAttribute( "developerConsole", m_developerConsole );
    section->SetAttribute( "developerConsoleKey", INPUT::KeyCodeToString( m_developerConsoleKey ) );
    section->SetAttribute( "userActionLog", m_userActionLog );
    section->SetAttribute( "userActionLogFile", m_userActionLogFile );
    section->SetAttribute( "windowSystem", CString( WindowSystemToString( m_windowSystem ) ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGeneralSettings::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    m_consoleLogWindow = section->GetAttributeValueOrChildValueByName( "consoleLogWindow" ).AsBool( m_consoleLogWindow, true );
    m_consoleLogOwnWindow = section->GetAttributeValueOrChildValueByName( "consoleLogOwnWindow" ).AsBool( m_consoleLogOwnWindow, true );
    m_developerConsole = section->GetAttributeValueOrChildValueByName( "developerConsole" ).AsBool( m_developerConsole, true );
    m_userActionLog = section->GetAttributeValueOrChildValueByName( "userActionLog" ).AsBool( m_userActionLog, true );
    // the path as it is written, so its variables ($CACHEDIR$, ...) stay when it is saved again
    m_userActionLogFile = section->GetAttributeValueOrChildValueByName( "userActionLogFile" ).AsString( m_userActionLogFile, false );
    const CString windowSystem = section->GetAttributeValueOrChildValueByName( "windowSystem" ).AsString( CString::Empty, true );
    if ( !windowSystem.IsNULLOrEmpty() && !ParseWindowSystem( windowSystem, m_windowSystem ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CGeneralSettings: The window system \"" << windowSystem << "\" of the settings is not auto, wayland or x11: it stays " << WindowSystemToString( m_windowSystem ) );
    CString key = section->GetAttributeValueOrChildValueByName( "developerConsoleKey" ).AsString( CString::Empty, true );
    if ( !key.IsNULLOrEmpty() )
    {
        INPUT::KeyCode keyCode = INPUT::StringToKeyCode( key );
        if ( INPUT::KeyCodeToString( keyCode ) == key )
            m_developerConsoleKey = keyCode;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGeneralSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUI::CGeneralSettings";
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
