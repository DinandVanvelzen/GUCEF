/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#include "gucefGUI_CGuiGlobal.h"

#include "gucefGUI_CWidgetTypeRegistry.h"
#include "gucefGUI_CFontRasterizerRegistry.h"
#include "gucefGUI_CKeyboardLayoutRegistry.h"
#include "gucefGUI_StandardWidgetTypes.h"
#include "gucefGUI_CSettingsForm.h"
#include "gucefGUI_CVideoSettingsForm.h"
#include "gucefGUI_CDeveloperConsoleForm.h"
#include "gucefGUI_CWebPageView.h"
#include "gucefGUI_CWebViewImageBinding.h"
#include "gucefGUI_CWebViewForm.h"
#include "gucefWEB_CWebViewGlobal.h"
#include "gucefGUI_CFileSystemDialogForm.h"
#include "gucefGUI_CGridColumnChooserForm.h"
#include "gucefGUI_CKeyboardShortcutMap.h"
#include "gucefGUI_CTabAreaNavigator.h"
#include "gucefGUI_CNavigationHistory.h"
#include "gucefGUI_CBreadcrumbBar.h"
#include "gucefGUI_CAsyncGridCsvExport.h"
#include "gucefCORE_CTaskManager.h"

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

MT::CMutex CGuiGlobal::g_datalock;
CGuiGlobal* CGuiGlobal::g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGuiGlobal*
CGuiGlobal::Instance()
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_datalock );
        
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = new CGuiGlobal();
            g_instance->Initialize();
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CGuiGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_datalock );

    delete g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CGuiGlobal::Initialize( void )
{GUCEF_TRACE;

    // Ensure the Core systems are initialzed first since this module depends on Core
    CORE::CCoreGlobal::Instance();

    // Ensure the Vfs systems are initialzed first since this module depends on Vfs
    VFS::CVfsGlobal::Instance();

    // the web view page widget and form build on the web views of gucefWEB
    WEB::CWebViewGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Initializing gucefGUI Global systems" );

    /*
     *  Register events
     */
    CWidget::RegisterEvents();
    CForm::RegisterEvents();
    CButton::RegisterEvents();
    CCheckbox::RegisterEvents();
    CCombobox::RegisterEvents();
    CEditbox::RegisterEvents();
    CSecretInput::RegisterEvents();
    CMessageBoxForm::RegisterEvents();
    CActivityService::RegisterEvents();
    CPathFieldForm::RegisterEvents();
    CFileSystemDialogForm::RegisterEvents();
    CGridView::RegisterEvents();
    CGridColumnChooserForm::RegisterEvents();
    CKeyboardShortcutMap::RegisterEvents();
    CTabAreaNavigator::RegisterEvents();
    CNavigationHistory::RegisterEvents();
    CBreadcrumbBar::RegisterEvents();
    CLabel::RegisterEvents();
    CListbox::RegisterEvents();
    CPushButton::RegisterEvents();
    CSpinner::RegisterEvents();
    CTabControl::RegisterEvents();
    CTextbox::RegisterEvents();
    CSettingsPageForm::RegisterEvents();
    CSettingsForm::RegisterEvents();
    CVideoSettingsForm::RegisterEvents();
    CDeveloperConsoleForm::RegisterEvents();
    CWebPageView::RegisterEvents();
    CWebViewImageBinding::RegisterEvents();
    CWebViewForm::RegisterEvents();
    CGUIManager::RegisterEvents();

    /*
     *      Instantiate all singletons
     */
    m_widgetTypeRegistry = new CWidgetTypeRegistry();
    RegisterStandardWidgetTypes( *m_widgetTypeRegistry );
    m_fontRasterizerRegistry = new CFontRasterizerRegistry();
    m_keyboardLayoutRegistry = new CKeyboardLayoutRegistry();
    CSettingsForm::RegisterStandardPages();
    m_gridCsvExportTaskFactory = GUCEF_NEW TAsyncGridCsvExportTaskFactory();
    CORE::CCoreGlobal::Instance()->GetTaskManager().RegisterTaskConsumerFactory( CAsyncGridCsvExport::TaskType, m_gridCsvExportTaskFactory );

    m_windowManager = new CWindowManager();
    m_guiManager = new CGUIManager();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefGUI Global systems initialized" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefGUI" );
}

/*-------------------------------------------------------------------------*/

const CString CGuiGlobal::DataMountPoint = "gucefGUI";
const CString CGuiGlobal::DefaultUiFontPath = "gucefGUI/fonts/NotoSans-Regular.ttf";

/*-------------------------------------------------------------------------*/

CGuiGlobal::CGuiGlobal( void )
    : m_windowManager( GUCEF_NULL ) ,
      m_guiManager( GUCEF_NULL )    ,
      m_widgetTypeRegistry( GUCEF_NULL ) ,
      m_fontRasterizerRegistry( GUCEF_NULL ) ,
      m_keyboardLayoutRegistry( GUCEF_NULL ) ,
      m_gridCsvExportTaskFactory( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGuiGlobal::~CGuiGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Shutting down gucefGUI global systems" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefGUI" );

    /*
     *      cleanup all singletons
     *      Take care to deinstance them in the correct order !!!
     */
    delete m_guiManager;
    m_guiManager = GUCEF_NULL;
    delete m_windowManager;
    m_windowManager = GUCEF_NULL;
    CSettingsForm::UnregisterStandardPages();
    CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterTaskConsumerFactory( CAsyncGridCsvExport::TaskType );
    GUCEF_DELETE m_gridCsvExportTaskFactory;
    m_gridCsvExportTaskFactory = GUCEF_NULL;
    delete m_keyboardLayoutRegistry;
    m_keyboardLayoutRegistry = GUCEF_NULL;
    delete m_fontRasterizerRegistry;
    m_fontRasterizerRegistry = GUCEF_NULL;
    delete m_widgetTypeRegistry;
    m_widgetTypeRegistry = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWindowManager&
CGuiGlobal::GetWindowManager( void )
{GUCEF_TRACE;

    return *m_windowManager;
}

/*-------------------------------------------------------------------------*/

CGUIManager&
CGuiGlobal::GetGuiManager( void )
{GUCEF_TRACE;

    return *m_guiManager;
}

/*-------------------------------------------------------------------------*/

CWidgetTypeRegistry&
CGuiGlobal::GetWidgetTypeRegistry( void )
{GUCEF_TRACE;

    return *m_widgetTypeRegistry;
}

/*-------------------------------------------------------------------------*/

CFontRasterizerRegistry&
CGuiGlobal::GetFontRasterizerRegistry( void )
{GUCEF_TRACE;

    return *m_fontRasterizerRegistry;
}

/*-------------------------------------------------------------------------*/

CKeyboardLayoutRegistry&
CGuiGlobal::GetKeyboardLayoutRegistry( void )
{GUCEF_TRACE;

    return *m_keyboardLayoutRegistry;
}

/*-------------------------------------------------------------------------*/

bool
CGuiGlobal::MountData( void )
{GUCEF_TRACE;

    return MountModuleData( DataMountPoint );
}

/*-------------------------------------------------------------------------*/

bool
CGuiGlobal::MountModuleData( const CString& moduleName )
{GUCEF_TRACE;

    return VFS::CVfsGlobal::Instance()->GetVfs().MountModuleData( moduleName );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
