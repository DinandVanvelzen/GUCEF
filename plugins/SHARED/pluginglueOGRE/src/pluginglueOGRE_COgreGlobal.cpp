/*
 *  pluginglueOGRE: Support library of the Ogre 1.x plugin set: Ogre Root setup, log, plugin loading
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGRELOGADAPTER_H
#include "pluginglueOGRE_COgreLogAdapter.h"
#define GUCEF_PLUGINGLUE_OGRE_COGRELOGADAPTER_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGRELOGADAPTER_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#include "pluginglueOGRE_COgreRootSetup.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREPLUGINMANAGER_H
#include "pluginglueOGRE_COgrePluginManager.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREPLUGINMANAGER_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREPLUGINMANAGER_H ? */

#include "pluginglueOGRE_COgreGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreGlobal* COgreGlobal::g_instance = GUCEF_NULL;
MT::CMutex COgreGlobal::g_dataLock;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreGlobal::COgreGlobal( void )
    : m_logAdapter( GUCEF_NULL )
    , m_rootSetup( GUCEF_NULL )
    , m_pluginManager( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreGlobal::~COgreGlobal()
{GUCEF_TRACE;

    // opposite of the order of making: the Ogre plugins go before the Root they are installed in,
    // and the Root before the log it writes to
    delete m_pluginManager;
    m_pluginManager = GUCEF_NULL;
    delete m_rootSetup;
    m_rootSetup = GUCEF_NULL;
    delete m_logAdapter;
    m_logAdapter = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
COgreGlobal::Initialize( void )
{GUCEF_TRACE;

    m_logAdapter = GUCEF_NEW COgreLogAdapter();
    m_rootSetup = GUCEF_NEW COgreRootSetup();
    m_rootSetup->CreateRoot();
    m_pluginManager = GUCEF_NEW COgrePluginManager();
}

/*-------------------------------------------------------------------------*/

COgreGlobal*
COgreGlobal::Instance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    if ( GUCEF_NULL == g_instance )
    {
        g_instance = GUCEF_NEW COgreGlobal();
        g_instance->Initialize();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
COgreGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

COgreLogAdapter&
COgreGlobal::GetLogAdapter( void )
{GUCEF_TRACE;

    return *m_logAdapter;
}

/*-------------------------------------------------------------------------*/

COgreRootSetup&
COgreGlobal::GetRootSetup( void )
{GUCEF_TRACE;

    return *m_rootSetup;
}

/*-------------------------------------------------------------------------*/

COgrePluginManager&
COgreGlobal::GetPluginManager( void )
{GUCEF_TRACE;

    return *m_pluginManager;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
