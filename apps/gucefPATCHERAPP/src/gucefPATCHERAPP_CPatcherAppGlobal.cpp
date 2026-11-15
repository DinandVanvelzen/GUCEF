/*
 *  gucefPATCHERAPP: Application module for the patcher application
 *  Copyright (C) 2002 - 2013.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#ifndef GUCEF_INPUT_CINPUTGLOBAL_H
#include "gucefINPUT_CInputGlobal.h"
#define GUCEF_INPUT_CINPUTGLOBAL_H
#endif /* GUCEF_INPUT_CINPUTGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PATCHERAPP_CPATCHERAPPMAINCONFIG_H
#include "gucefPATCHERAPP_CPatcherAppConfig.h"
#define GUCEF_PATCHERAPP_CPATCHERAPPMAINCONFIG_H
#endif /* GUCEF_PATCHERAPP_CPATCHERAPPMAINCONFIG_H ? */

#ifndef GUCEF_PATCHERAPP_CMAINPATCHERAPPLOGIC_H
#include "gucefPATCHERAPP_CMainPatcherAppLogic.h"
#define GUCEF_PATCHERAPP_CMAINPATCHERAPPLOGIC_H
#endif /* GUCEF_PATCHERAPP_CMAINPATCHERAPPLOGIC_H ? */

#include "gucefPATCHERAPP_CPatcherAppGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PATCHERAPP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

CPatcherAppGlobal* CPatcherAppGlobal::g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPatcherAppGlobal::CPatcherAppGlobal( void )
    : m_config( GUCEF_NULL )    ,
      m_mainLogic( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPatcherAppGlobal::~CPatcherAppGlobal()
{GUCEF_TRACE;

    delete m_mainLogic;
    m_mainLogic = GUCEF_NULL;

    delete m_config;
    m_config = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CPatcherAppGlobal*
CPatcherAppGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        g_instance = new CPatcherAppGlobal();         
        g_instance->Init();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CPatcherAppGlobal::Deinstance( void )
{GUCEF_TRACE;

    delete g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CPatcherAppGlobal::Init( void )
{GUCEF_TRACE;

    // Explictly initialize systems we depend on
    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();
    COM::CComGlobal::Instance();
    GUI::CGuiGlobal::Instance();
    INPUT::CInputGlobal::Instance();
    IMAGE::CImageGlobal::Instance();

    m_config = new CPatcherAppConfig();
    m_mainLogic = new CMainPatcherAppLogic(); 
}

/*-------------------------------------------------------------------------*/

CMainPatcherAppLogic&
CPatcherAppGlobal::GetMainLogic( void )
{GUCEF_TRACE;

    return *m_mainLogic;
}

/*-------------------------------------------------------------------------*/

CPatcherAppConfig&
CPatcherAppGlobal::GetConfig( void )
{GUCEF_TRACE;

    return *m_config;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PATCHERAPP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
