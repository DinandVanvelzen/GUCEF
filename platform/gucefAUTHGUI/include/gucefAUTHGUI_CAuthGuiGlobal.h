/*
 *  gucefAUTHGUI: GUCEF module providing the user interface for logging in with external identities and the account page of settings
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

#ifndef GUCEF_AUTHGUI_CAUTHGUIGLOBAL_H
#define GUCEF_AUTHGUI_CAUTHGUIGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_AUTHGUI_MACROS_H
#include "gucefAUTHGUI_macros.h"
#define GUCEF_AUTHGUI_MACROS_H
#endif /* GUCEF_AUTHGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The global systems of gucefAUTHGUI, set up when the module loads: the systems it builds on (gucefCORE, gucefGUI,
 *  gucefAUTH), the events of the forms and the settings page "Account" in the page factory of GUI::CSettingsForm, which
 *  is taken out again when the module goes. A tool that links the module has the page with
 *  GUI::CSettingsForm::AddPage( "Account" ).
 */
class GUCEF_AUTHGUI_EXPORT_CPP CAuthGuiGlobal
{
    public:

    /** The name of the page in the page factory of the settings form; also its title */
    static const char* const AccountPageName;

    static CAuthGuiGlobal* Instance( void );

    private:

    friend class CModule;

    static void Deinstance( void );

    void Initialize( void );

    CAuthGuiGlobal( void );

    ~CAuthGuiGlobal();

    CAuthGuiGlobal( const CAuthGuiGlobal& src );            /**< not implemented */
    CAuthGuiGlobal& operator=( const CAuthGuiGlobal& src ); /**< not implemented */

    private:

    static MT::CMutex g_dataLock;
    static CAuthGuiGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTHGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_CAUTHGUIGLOBAL_H ? */
