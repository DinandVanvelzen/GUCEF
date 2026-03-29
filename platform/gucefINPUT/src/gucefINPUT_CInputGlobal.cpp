/*
 *  gucefINPUT: GUCEF module providing input device interaction
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_INPUT_CINPUTACTIONMAP_H
#include "CInputActionMap.h"
#define GUCEF_INPUT_CINPUTACTIONMAP_H
#endif /* GUCEF_INPUT_CINPUTACTIONMAP_H ? */

#ifndef GUCEF_INPUT_CKEYBOARD_H
#include "gucefINPUT_CKeyboard.h"
#define GUCEF_INPUT_CKEYBOARD_H
#endif /* GUCEF_INPUT_CKEYBOARD_H ? */

#ifndef GUCEF_INPUT_CMOUSE_H
#include "gucefINPUT_CMouse.h"
#define GUCEF_INPUT_CMOUSE_H
#endif /* GUCEF_INPUT_CMOUSE_H ? */

#ifndef GUCEF_INPUT_CINPUTDEVICE_H
#include "gucefINPUT_CInputDevice.h"
#define GUCEF_INPUT_CINPUTDEVICE_H
#endif /* GUCEF_INPUT_CINPUTDEVICE_H ? */

#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
#include "CInputController.h"
#define GUCEF_INPUT_CINPUTCONTROLLER_H
#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */

#ifndef GUCEF_INPUT_CINPUTDRIVERPLUGINMANAGER_H
#include "gucefINPUT_CInputDriverPluginManager.h"
#define GUCEF_INPUT_CINPUTDRIVERPLUGINMANAGER_H
#endif /* GUCEF_INPUT_CINPUTDRIVERPLUGINMANAGER_H ? */

#include "gucefINPUT_CInputGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

CInputGlobal* CInputGlobal::g_instance = NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CInputGlobal*
CInputGlobal::Instance()
{GUCEF_TRACE;

    if ( NULL == g_instance )
    {
        g_instance = new CInputGlobal();
        g_instance->Initialize();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CInputGlobal::Deinstance( void )
{GUCEF_TRACE;

    delete g_instance;
    g_instance = NULL;
}

/*-------------------------------------------------------------------------*/

void
CInputGlobal::Initialize( void )
{GUCEF_TRACE;

    // Ensure the Core systems are initialzed first since this module depends on Core
    CORE::CCoreGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Initializing gucefINPUT Global systems" );

    /*
     *      register all events
     */
    CKeyboard::RegisterEvents();
    CMouse::RegisterEvents();
    //CInputDevice::RegisterEvents();
    CInputActionMap::RegisterEvents();
    CInputController::RegisterEvents();

    /*
     *      Instantiate all singletons
     */
    m_inputController = new CInputController();
    m_inputDriverPluginManager = new CInputDriverPluginManager();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefINPUT Global systems initialized" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefINPUT" );
}

/*-------------------------------------------------------------------------*/

CInputGlobal::CInputGlobal( void )
    : m_inputController( NULL )          ,
      m_inputDriverPluginManager( NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CInputGlobal::~CInputGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Shutting down gucefINPUT global systems" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefINPUT" );

    /*
     *      cleanup all singletons
     *      Take care to deinstance them in the correct order !!!
     */

    delete m_inputDriverPluginManager;
    m_inputDriverPluginManager = NULL;
    delete m_inputController;
    m_inputController = NULL;
}

/*-------------------------------------------------------------------------*/

CInputController&
CInputGlobal::GetInputController( void )
{GUCEF_TRACE;

    return *m_inputController;
}

/*-------------------------------------------------------------------------*/

CInputDriverPluginManager&
CInputGlobal::GetInputDriverPluginManager( void )
{GUCEF_TRACE;

    return *m_inputDriverPluginManager;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
