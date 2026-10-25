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

#ifndef GUCEF_GUI_CGENERALSETTINGS_H
#define GUCEF_GUI_CGENERALSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  General settings of a game or tool: the console log window (the log in a terminal or console window
 *  of the platform), the developer console (a console that drops down over the application on a key) and
 *  the user action log (what the user did, in a file of its own, for finding problems).
 */
class GUCEF_GUI_PUBLIC_CPP CGeneralSettings : public CORE::CIConfigurable
{
    public:

    /** The name of the config section: SaveConfig() writes it in the node it gets, LoadConfig() reads it from there */
    static const CString ConfigSectionName;

    /** The window system the application opens its window on, when the platform has more than one (Linux: X11, also through XWayland, and Wayland) */
    enum EWindowSystem
    {
        WINDOWSYSTEM_X11 = 0 ,      /**< X11, on Wayland sessions through XWayland */
        WINDOWSYSTEM_WAYLAND ,      /**< Wayland natively */
        WINDOWSYSTEM_AUTO           /**< Wayland where the session offers it and the driver can use it, else X11 */
    };

    /** "auto", "wayland" or "x11" (also "xwayland"), in any case; false for anything else */
    static bool ParseWindowSystem( const CString& text, EWindowSystem& windowSystem );

    static const char* WindowSystemToString( EWindowSystem windowSystem );

    /** What window system an application opens its window on: auto is Wayland when it is available, else X11 */
    static EWindowSystem ResolveWindowSystem( EWindowSystem requested, bool waylandAvailable );

    CGeneralSettings( void );

    CGeneralSettings( const CGeneralSettings& src );

    virtual ~CGeneralSettings();

    CGeneralSettings& operator=( const CGeneralSettings& src );

    void SetConsoleLogWindow( bool enabled );

    bool GetConsoleLogWindow( void ) const;

    /** The console log in a window of its own, also when the application was started from a console (true by default) */
    void SetConsoleLogOwnWindow( bool ownWindow );

    bool GetConsoleLogOwnWindow( void ) const;

    void SetDeveloperConsole( bool enabled );

    bool GetDeveloperConsole( void ) const;

    /** The key that opens and closes the developer console, the key left of 1 (` and ~) by default */
    void SetDeveloperConsoleKey( INPUT::KeyCode key );

    INPUT::KeyCode GetDeveloperConsoleKey( void ) const;

    /** A log of what the user does, see CLogManager::SetUserActionLogEnabled(); off by default */
    void SetUserActionLog( bool enabled );

    bool GetUserActionLog( void ) const;

    /** The file of the user action log, a path that may have variables like $CACHEDIR$; an application sets its own */
    void SetUserActionLogFile( const CString& filePath );

    const CString& GetUserActionLogFile( void ) const;

    /** The window system for the window of the application, X11 by default; ignored where the platform has one window system */
    void SetWindowSystem( EWindowSystem windowSystem );

    EWindowSystem GetWindowSystem( void ) const;

    /** Writes the settings as the section ConfigSectionName in 'tree' */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the section ConfigSectionName of 'treeroot', or 'treeroot' itself when it is that section; what it lacks stays as it is */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool m_consoleLogWindow;
    bool m_consoleLogOwnWindow;
    bool m_developerConsole;
    INPUT::KeyCode m_developerConsoleKey;
    bool m_userActionLog;
    CString m_userActionLogFile;
    EWindowSystem m_windowSystem;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGENERALSETTINGS_H ? */
