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

#ifndef GUCEF_GUI_CGENERALSETTINGSFORM_H
#define GUCEF_GUI_CGENERALSETTINGSFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CGENERALSETTINGS_H
#include "gucefGUI_CGeneralSettings.h"
#define GUCEF_GUI_CGENERALSETTINGS_H
#endif /* GUCEF_GUI_CGENERALSETTINGS_H ? */

#ifndef GUCEF_GUI_CSETTINGSPAGEFORM_H
#include "gucefGUI_CSettingsPageForm.h"
#define GUCEF_GUI_CSETTINGSPAGEFORM_H
#endif /* GUCEF_GUI_CSETTINGSPAGEFORM_H ? */

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

class CDeveloperConsoleForm;

/**
 *  The general settings: a prebaked page that shows a CGeneralSettings. Widgets (by name):
 *
 *      ConsoleLogWindow    the log in a console window of the platform
 *      DeveloperConsole    the developer console that drops down on its key
 *
 *  Apply takes the values into the settings and applies them with ApplyGeneralSettings().
 */
class GUCEF_GUI_PUBLIC_CPP CGeneralSettingsForm : public CSettingsPageForm
{
    public:

    CGeneralSettingsForm( void );

    virtual ~CGeneralSettingsForm();

    /** The settings in use; ReadSettings() shows them */
    void SetGeneralSettings( const CGeneralSettings& settings );

    const CGeneralSettings& GetGeneralSettings( void ) const;

    /** The developer console the settings switch on and off, NULL for none */
    void SetDeveloperConsole( CDeveloperConsoleForm* console );

    CDeveloperConsoleForm* GetDeveloperConsole( void ) const;

    /**
     *  Applies general settings: the console log window of the log manager and, when there is one, the
     *  developer console with its key. Applications do this at start with the settings from their config.
     */
    static void ApplyGeneralSettings( const CGeneralSettings& settings, CDeveloperConsoleForm* console );

    virtual CString GetTitle( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void ReadSettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ApplySettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** How the console key is called on the page: "~" for the key left of 1, else its key code name */
    static CString GetKeyDisplayName( INPUT::KeyCode key );

    protected:

    virtual void BuildLayout( CORE::CDataNode& layout ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CGeneralSettingsForm( const CGeneralSettingsForm& src );
    CGeneralSettingsForm& operator=( const CGeneralSettingsForm& src );

    private:

    CGeneralSettings m_settings;
    CDeveloperConsoleForm* m_console;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGENERALSETTINGSFORM_H ? */
