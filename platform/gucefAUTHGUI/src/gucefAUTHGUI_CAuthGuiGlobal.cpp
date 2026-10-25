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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CSETTINGSFORM_H
#include "gucefGUI_CSettingsForm.h"
#define GUCEF_GUI_CSETTINGSFORM_H
#endif /* GUCEF_GUI_CSETTINGSFORM_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTHGUI_CACCOUNTSPANEL_H
#include "gucefAUTHGUI_CAccountsPanel.h"
#define GUCEF_AUTHGUI_CACCOUNTSPANEL_H
#endif /* GUCEF_AUTHGUI_CACCOUNTSPANEL_H ? */

#ifndef GUCEF_AUTHGUI_CLOGINFORM_H
#include "gucefAUTHGUI_CLoginForm.h"
#define GUCEF_AUTHGUI_CLOGINFORM_H
#endif /* GUCEF_AUTHGUI_CLOGINFORM_H ? */

#ifndef GUCEF_AUTHGUI_CACCOUNTSETTINGSFORM_H
#include "gucefAUTHGUI_CAccountSettingsForm.h"
#define GUCEF_AUTHGUI_CACCOUNTSETTINGSFORM_H
#endif /* GUCEF_AUTHGUI_CACCOUNTSETTINGSFORM_H ? */

#include "gucefAUTHGUI_CAuthGuiGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CAuthGuiGlobal::g_dataLock;
CAuthGuiGlobal* CAuthGuiGlobal::g_instance = GUCEF_NULL;

const char* const CAuthGuiGlobal::AccountPageName = "Account";

namespace {

CORE::CTFactory< GUI::CSettingsPageForm, CAccountSettingsForm, MT::CMutex > g_accountSettingsPageFactory;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAuthGuiGlobal::CAuthGuiGlobal( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAuthGuiGlobal::Initialize( void )
{GUCEF_TRACE;

    // The modules we depend on
    CORE::CCoreGlobal::Instance();
    GUI::CGuiGlobal::Instance();
    AUTH::CAuthGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefAUTHGUI Global systems initializing" );

    CAccountsPanel::RegisterEvents();
    CLoginForm::RegisterEvents();

    // a tool that has this module has the page
    GUI::CSettingsForm::GetPageFactory().RegisterConcreteFactory( AccountPageName, &g_accountSettingsPageFactory );

    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefAUTHGUI" );
}

/*-------------------------------------------------------------------------*/

CAuthGuiGlobal::~CAuthGuiGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefAUTHGUI Global systems shutting down" );
    GUI::CSettingsForm::GetPageFactory().UnregisterConcreteFactory( AccountPageName );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefAUTHGUI" );
}

/*-------------------------------------------------------------------------*/

CAuthGuiGlobal*
CAuthGuiGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CAuthGuiGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CAuthGuiGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CAuthGuiGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTHGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
