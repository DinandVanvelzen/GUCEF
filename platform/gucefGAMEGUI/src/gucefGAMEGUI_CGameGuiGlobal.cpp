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

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#ifndef GUCEF_GAMEGUI_CSERVERBROWSERFORM_H
#include "gucefGAMEGUI_CServerBrowserForm.h"
#define GUCEF_GAMEGUI_CSERVERBROWSERFORM_H
#endif /* GUCEF_GAMEGUI_CSERVERBROWSERFORM_H ? */

#ifndef GUCEF_GAMEGUI_CCREATELOBBYFORM_H
#include "gucefGAMEGUI_CCreateLobbyForm.h"
#define GUCEF_GAMEGUI_CCREATELOBBYFORM_H
#endif /* GUCEF_GAMEGUI_CCREATELOBBYFORM_H ? */

#ifndef GUCEF_GAMEGUI_CLOBBYFORM_H
#include "gucefGAMEGUI_CLobbyForm.h"
#define GUCEF_GAMEGUI_CLOBBYFORM_H
#endif /* GUCEF_GAMEGUI_CLOBBYFORM_H ? */

#ifndef GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H
#include "gucefGAMEGUI_CLobbySettingsPanelForm.h"
#define GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H
#endif /* GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H ? */

#ifndef GUCEF_GAMEGUI_CCHATFORM_H
#include "gucefGAMEGUI_CChatForm.h"
#define GUCEF_GAMEGUI_CCHATFORM_H
#endif /* GUCEF_GAMEGUI_CCHATFORM_H ? */

#ifndef GUCEF_GAMEGUI_CVOTEPROMPTFORM_H
#include "gucefGAMEGUI_CVotePromptForm.h"
#define GUCEF_GAMEGUI_CVOTEPROMPTFORM_H
#endif /* GUCEF_GAMEGUI_CVOTEPROMPTFORM_H ? */

#ifndef GUCEF_GAMEGUI_CCONNECTINGFORM_H
#include "gucefGAMEGUI_CConnectingForm.h"
#define GUCEF_GAMEGUI_CCONNECTINGFORM_H
#endif /* GUCEF_GAMEGUI_CCONNECTINGFORM_H ? */

#ifndef GUCEF_GAMEGUI_CDIRECTCONNECTFORM_H
#include "gucefGAMEGUI_CDirectConnectForm.h"
#define GUCEF_GAMEGUI_CDIRECTCONNECTFORM_H
#endif /* GUCEF_GAMEGUI_CDIRECTCONNECTFORM_H ? */

#ifndef GUCEF_GAMEGUI_CPLAYERPROFILEFORM_H
#include "gucefGAMEGUI_CPlayerProfileForm.h"
#define GUCEF_GAMEGUI_CPLAYERPROFILEFORM_H
#endif /* GUCEF_GAMEGUI_CPLAYERPROFILEFORM_H ? */

#include "gucefGAMEGUI_CGameGuiGlobal.h"

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

MT::CMutex CGameGuiGlobal::g_dataLock;
CGameGuiGlobal* CGameGuiGlobal::g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGameGuiGlobal::CGameGuiGlobal( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGameGuiGlobal::Initialize( void )
{GUCEF_TRACE;

    // The modules we depend on
    CORE::CCoreGlobal::Instance();
    GUI::CGuiGlobal::Instance();
    MULTIPLAY::CMultiplayGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefGAMEGUI Global systems initializing" );

    CServerBrowserForm::RegisterEvents();
    CCreateLobbyForm::RegisterEvents();
    CLobbyForm::RegisterEvents();
    CLobbySettingsPanelForm::RegisterEvents();
    CChatForm::RegisterEvents();
    CVotePromptForm::RegisterEvents();
    CConnectingForm::RegisterEvents();
    CDirectConnectForm::RegisterEvents();
    CPlayerProfileForm::RegisterEvents();

    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefGAMEGUI" );
}

/*-------------------------------------------------------------------------*/

CGameGuiGlobal::~CGameGuiGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefGAMEGUI Global systems shutting down" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefGAMEGUI" );
}

/*-------------------------------------------------------------------------*/

CGameGuiGlobal*
CGameGuiGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CGameGuiGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CGameGuiGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CGameGuiGlobal::Deinstance( void )
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

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
