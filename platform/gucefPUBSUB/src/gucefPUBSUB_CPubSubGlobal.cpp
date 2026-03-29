/*
 *  gucefPUBSUB: GUCEF module providing pub-sub communication facilities
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_PUBSUB_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_PUBSUB_CCOMCOREGLOBAL_H
#endif /* GUCEF_PUBSUB_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#include "gucefPUBSUB_CPubSubClientFactory.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#include "gucefPUBSUB_CPubSubClientTopic.h"    
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTC_H
#include "gucefPUBSUB_CPubSubClient.h"    
#define GUCEF_PUBSUB_CPUBSUBCLIENT_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENT_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H
#include "gucefPUBSUB_CPubSubClientSide.h"    
#define GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H ? */

#ifndef GUCEF_PUBSUB_CVFSPUBSUBBOOKMARKPERSISTENCE_H
#include "gucefPUBSUB_CVfsPubSubBookmarkPersistence.h"
#define GUCEF_PUBSUB_CVFSPUBSUBBOOKMARKPERSISTENCE_H
#endif /* GUCEF_PUBSUB_CVFSPUBSUBBOOKMARKPERSISTENCE_H ? */

#ifndef GUCEF_PUBSUB_CBINARYPUBSUBJOURNAL_H
#include "gucefPUBSUB_CBinaryPubSubJournal.h"    
#define GUCEF_PUBSUB_CBINARYPUBSUBJOURNAL_H
#endif /* GUCEF_PUBSUB_CBINARYPUBSUBJOURNAL_H ? */

#include "gucefPUBSUB_CPubSubGlobal.h"  /* definition of the class implemented here */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTFactoryWithParam< CIPubSubBookmarkPersistence, CVfsPubSubBookmarkPersistence, CPubSubBookmarkPersistenceConfig, MT::CMutex >    TVfsPubSubBookmarkPersistenceFactory;
typedef CORE::CTFactoryWithParam< CIPubSubJournal, CBinaryPubSubJournal, CPubSubJournalConfig, MT::CMutex >                                     TBinaryPubSubJournalFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CPubSubGlobal::g_dataLock;
CPubSubGlobal* CPubSubGlobal::g_instance = GUCEF_NULL;
TVfsPubSubBookmarkPersistenceFactory g_vfsPubSubBookmarkPersistenceFactory;
TBinaryPubSubJournalFactory g_binaryPubSubJournalFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPubSubGlobal::CPubSubGlobal( void )
    : m_pubsubClientFactory( GUCEF_NULL )
    , m_pubsubBookmarkPersistenceFactory( GUCEF_NULL )
    , m_pubsubJournalFactory( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPubSubGlobal::Initialize( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();
    COM::CComGlobal::Instance();
    VFS::CVfsGlobal::Instance();
    WEB::CWebGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefPUBSUB Global systems initializing" );

    CPubSubClientTopic::RegisterEvents();
    CPubSubClient::RegisterEvents();
    CPubSubClientSide::RegisterEvents();

    m_pubsubClientFactory = GUCEF_NEW CPubSubClientFactory();
    m_pubsubBookmarkPersistenceFactory = GUCEF_NEW CPubSubBookmarkPersistenceFactory();
    m_pubsubJournalFactory = GUCEF_NEW CPubSubJournalFactory();

    m_pubsubBookmarkPersistenceFactory->RegisterConcreteFactory( CVfsPubSubBookmarkPersistence::BookmarkPersistenceType, &g_vfsPubSubBookmarkPersistenceFactory );
    m_pubsubJournalFactory->RegisterConcreteFactory( CBinaryPubSubJournal::JournalType, &g_binaryPubSubJournalFactory );
    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefPUBSUB" );
}

/*-------------------------------------------------------------------------*/

CPubSubGlobal::~CPubSubGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefPUBSUB Global systems shutting down" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefPUBSUB" );

    m_pubsubBookmarkPersistenceFactory->UnregisterConcreteFactory( CVfsPubSubBookmarkPersistence::BookmarkPersistenceType );
    m_pubsubJournalFactory->UnregisterConcreteFactory( CBinaryPubSubJournal::JournalType );
    
    GUCEF_DELETE m_pubsubBookmarkPersistenceFactory;
    m_pubsubBookmarkPersistenceFactory = GUCEF_NULL;

    GUCEF_DELETE m_pubsubClientFactory;
    m_pubsubClientFactory = GUCEF_NULL;

    GUCEF_DELETE m_pubsubJournalFactory;
    m_pubsubJournalFactory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CPubSubGlobal*
CPubSubGlobal::Instance()
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CPubSubGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CPubSubGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CPubSubGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = NULL;
}

/*-------------------------------------------------------------------------*/

CPubSubClientFactory& 
CPubSubGlobal::GetPubSubClientFactory( void )
{GUCEF_TRACE;

    return *m_pubsubClientFactory;
}

/*-------------------------------------------------------------------------*/

CPubSubBookmarkPersistenceFactory& 
CPubSubGlobal::GetPubSubBookmarkPersistenceFactory( void )
{GUCEF_TRACE;

    return *m_pubsubBookmarkPersistenceFactory;
}

/*-------------------------------------------------------------------------*/

CPubSubJournalFactory&
CPubSubGlobal::GetPubSubJournalFactory( void )
{GUCEF_TRACE;

    return *m_pubsubJournalFactory;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
