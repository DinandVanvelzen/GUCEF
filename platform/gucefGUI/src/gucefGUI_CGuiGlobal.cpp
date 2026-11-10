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
#include "gucefGUI_StandardWidgetTypes.h"
#include "gucefGUI_CSettingsForm.h"
#include "gucefGUI_CVideoSettingsForm.h"
#include "gucefGUI_CDeveloperConsoleForm.h"
#include "gucefGUI_CFileSystemDialogForm.h"

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
    g_instance = NULL;
}

/*-------------------------------------------------------------------------*/

void
CGuiGlobal::Initialize( void )
{GUCEF_TRACE;

    // Ensure the Core systems are initialzed first since this module depends on Core
    CORE::CCoreGlobal::Instance();

    // Ensure the Vfs systems are initialzed first since this module depends on Vfs
    VFS::CVfsGlobal::Instance();

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
    CPathFieldForm::RegisterEvents();
    CFileSystemDialogForm::RegisterEvents();
    CGridView::RegisterEvents();
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
    CGUIManager::RegisterEvents();

    /*
     *      Instantiate all singletons
     */
    m_widgetTypeRegistry = new CWidgetTypeRegistry();
    RegisterStandardWidgetTypes( *m_widgetTypeRegistry );
    m_fontRasterizerRegistry = new CFontRasterizerRegistry();
    CSettingsForm::RegisterStandardPages();

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
    : m_windowManager( NULL ) ,
      m_guiManager( NULL )    ,
      m_widgetTypeRegistry( NULL ) ,
      m_fontRasterizerRegistry( NULL )
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
    m_guiManager = NULL;
    delete m_windowManager;
    m_windowManager = NULL;
    CSettingsForm::UnregisterStandardPages();
    delete m_fontRasterizerRegistry;
    m_fontRasterizerRegistry = NULL;
    delete m_widgetTypeRegistry;
    m_widgetTypeRegistry = NULL;
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
