/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#include "gucefGUI_CGuiGlobal.h"

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
    CFileSystemDialog::RegisterEvents();
    CGridView::RegisterEvents();
    CLabel::RegisterEvents();
    CListbox::RegisterEvents();
    CPushButton::RegisterEvents();
    CSpinner::RegisterEvents();
    CTabControl::RegisterEvents();
    CTextbox::RegisterEvents();
    CGUIManager::RegisterEvents();

    /*
     *      Instantiate all singletons
     */
    m_windowManager = new CWindowManager();
    m_guiManager = new CGUIManager();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefGUI Global systems initialized" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefGUI" );
}

/*-------------------------------------------------------------------------*/

CGuiGlobal::CGuiGlobal( void )
    : m_windowManager( NULL ) ,
      m_guiManager( NULL )
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
