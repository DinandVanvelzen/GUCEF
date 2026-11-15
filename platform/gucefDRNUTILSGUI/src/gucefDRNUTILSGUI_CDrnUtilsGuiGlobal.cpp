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

#ifndef GUCEF_DRNUTILS_CDRNUTILSGLOBAL_H
#include "gucefDRNUTILS_CDrnUtilsGlobal.h"
#define GUCEF_DRNUTILS_CDRNUTILSGLOBAL_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSGLOBAL_H ? */

#ifndef GUCEF_DRNUTILSGUI_CSCHEDULERFORM_H
#include "gucefDRNUTILSGUI_CSchedulerForm.h"
#define GUCEF_DRNUTILSGUI_CSCHEDULERFORM_H
#endif /* GUCEF_DRNUTILSGUI_CSCHEDULERFORM_H ? */

#ifndef GUCEF_DRNUTILSGUI_CJOBRUNHISTORYFORM_H
#include "gucefDRNUTILSGUI_CJobRunHistoryForm.h"
#define GUCEF_DRNUTILSGUI_CJOBRUNHISTORYFORM_H
#endif /* GUCEF_DRNUTILSGUI_CJOBRUNHISTORYFORM_H ? */

#ifndef GUCEF_DRNUTILSGUI_CTASKMANAGERFORM_H
#include "gucefDRNUTILSGUI_CTaskManagerForm.h"
#define GUCEF_DRNUTILSGUI_CTASKMANAGERFORM_H
#endif /* GUCEF_DRNUTILSGUI_CTASKMANAGERFORM_H ? */

#ifndef GUCEF_DRNUTILSGUI_CCONTROLCONFIRMFORM_H
#include "gucefDRNUTILSGUI_CControlConfirmForm.h"
#define GUCEF_DRNUTILSGUI_CCONTROLCONFIRMFORM_H
#endif /* GUCEF_DRNUTILSGUI_CCONTROLCONFIRMFORM_H ? */

#include "gucefDRNUTILSGUI_CDrnUtilsGuiGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILSGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CDrnUtilsGuiGlobal::g_dataLock;
CDrnUtilsGuiGlobal* CDrnUtilsGuiGlobal::g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDrnUtilsGuiGlobal::CDrnUtilsGuiGlobal( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsGuiGlobal::Initialize( void )
{GUCEF_TRACE;

    // The modules we depend on
    CORE::CCoreGlobal::Instance();
    GUI::CGuiGlobal::Instance();
    DRNUTILS::CDrnUtilsGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefDRNUTILSGUI Global systems initializing" );

    CSchedulerForm::RegisterEvents();
    CJobRunHistoryForm::RegisterEvents();
    CTaskManagerForm::RegisterEvents();
    CControlConfirmForm::RegisterEvents();

    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefDRNUTILSGUI" );
}

/*-------------------------------------------------------------------------*/

CDrnUtilsGuiGlobal::~CDrnUtilsGuiGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefDRNUTILSGUI Global systems shutting down" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefDRNUTILSGUI" );
}

/*-------------------------------------------------------------------------*/

CDrnUtilsGuiGlobal*
CDrnUtilsGuiGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CDrnUtilsGuiGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CDrnUtilsGuiGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsGuiGlobal::Deinstance( void )
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

}; /* namespace DRNUTILSGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
