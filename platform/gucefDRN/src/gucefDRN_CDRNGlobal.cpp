/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNTCPCHANNEL_H
#include "gucefDRN_CDRNTcpChannel.h"
#define GUCEF_DRN_CDRNTCPCHANNEL_H
#endif /* GUCEF_DRN_CDRNTCPCHANNEL_H ? */

#ifndef GUCEF_DRN_CDRNTCPCHANNELLISTENER_H
#include "gucefDRN_CDRNTcpChannelListener.h"
#define GUCEF_DRN_CDRNTCPCHANNELLISTENER_H
#endif /* GUCEF_DRN_CDRNTCPCHANNELLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNUDPCHANNEL_H
#include "gucefDRN_CDRNUdpChannel.h"
#define GUCEF_DRN_CDRNUDPCHANNEL_H
#endif /* GUCEF_DRN_CDRNUDPCHANNEL_H ? */

#ifndef GUCEF_DRN_CDRNUDPCHANNELLISTENER_H
#include "gucefDRN_CDRNUdpChannelListener.h"
#define GUCEF_DRN_CDRNUDPCHANNELLISTENER_H
#endif /* GUCEF_DRN_CDRNUDPCHANNELLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNWEBSOCKETCHANNEL_H
#include "gucefDRN_CDRNWebSocketChannel.h"
#define GUCEF_DRN_CDRNWEBSOCKETCHANNEL_H
#endif /* GUCEF_DRN_CDRNWEBSOCKETCHANNEL_H ? */

#ifndef GUCEF_DRN_CDRNWEBSOCKETCHANNELLISTENER_H
#include "gucefDRN_CDRNWebSocketChannelListener.h"
#define GUCEF_DRN_CDRNWEBSOCKETCHANNELLISTENER_H
#endif /* GUCEF_DRN_CDRNWEBSOCKETCHANNELLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNANONYMOUSAUTHENTICATOR_H
#include "gucefDRN_CDRNAnonymousAuthenticator.h"
#define GUCEF_DRN_CDRNANONYMOUSAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNANONYMOUSAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNSHAREDSECRETAUTHENTICATOR_H
#include "gucefDRN_CDRNSharedSecretAuthenticator.h"
#define GUCEF_DRN_CDRNSHAREDSECRETAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNSHAREDSECRETAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#include "gucefDRN_CDRNTokenAuthenticator.h"
#define GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_DRN_CDRNCONNECTOR_H
#include "gucefDRN_CDRNConnector.h"
#define GUCEF_DRN_CDRNCONNECTOR_H
#endif /* GUCEF_DRN_CDRNCONNECTOR_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNREPLICATOR_H
#include "gucefDRN_CDRNReplicator.h"
#define GUCEF_DRN_CDRNREPLICATOR_H
#endif /* GUCEF_DRN_CDRNREPLICATOR_H ? */

#ifndef GUCEF_DRN_CDRNBULKTRANSFER_H
#include "gucefDRN_CDRNBulkTransfer.h"
#define GUCEF_DRN_CDRNBULKTRANSFER_H
#endif /* GUCEF_DRN_CDRNBULKTRANSFER_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#include "gucefDRN_CDRNGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CDRNGlobal::g_dataLock;
CDRNGlobal* CDRNGlobal::g_instance = GUCEF_NULL;

typedef CORE::CTFactory< CIDRNChannel, CDRNTcpChannel, MT::CMutex >                                 TTcpChannelFactory;
typedef CORE::CTFactory< CIDRNChannel, CDRNUdpChannel, MT::CMutex >                                 TUdpChannelFactory;
typedef CORE::CTFactory< CIDRNChannel, CDRNWebSocketChannel, MT::CMutex >                           TWebSocketChannelFactory;
typedef CORE::CTFactory< CIDRNChannelListener, CDRNTcpChannelListener, MT::CMutex >                 TTcpChannelListenerFactory;
typedef CORE::CTFactory< CIDRNChannelListener, CDRNUdpChannelListener, MT::CMutex >                 TUdpChannelListenerFactory;
typedef CORE::CTFactory< CIDRNChannelListener, CDRNWebSocketChannelListener, MT::CMutex >           TWebSocketChannelListenerFactory;
typedef CORE::CTFactory< CIDRNAuthenticator, CDRNAnonymousAuthenticator, MT::CMutex >               TAnonymousAuthenticatorFactory;
typedef CORE::CTFactory< CIDRNAuthenticator, CDRNSharedSecretAuthenticator, MT::CMutex >            TSharedSecretAuthenticatorFactory;
typedef CORE::CTFactory< CIDRNAuthenticator, CDRNTokenAuthenticator, MT::CMutex >                   TTokenAuthenticatorFactory;

static TTcpChannelFactory g_tcpChannelFactory;
static TUdpChannelFactory g_udpChannelFactory;
static TWebSocketChannelFactory g_webSocketChannelFactory;
static TTcpChannelListenerFactory g_tcpChannelListenerFactory;
static TUdpChannelListenerFactory g_udpChannelListenerFactory;
static TWebSocketChannelListenerFactory g_webSocketChannelListenerFactory;
static TAnonymousAuthenticatorFactory g_anonymousAuthenticatorFactory;
static TSharedSecretAuthenticatorFactory g_sharedSecretAuthenticatorFactory;
static TTokenAuthenticatorFactory g_tokenAuthenticatorFactory;
static CORE::CTFactory< CORE::CTaskConsumer, CDRNHashTaskConsumer, MT::CMutex > g_hashTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNGlobal::CDRNGlobal( void )
    : m_channelFactory( GUCEF_NULL )
    , m_channelListenerFactory( GUCEF_NULL )
    , m_authenticatorFactory( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNGlobal::Initialize( void )
{GUCEF_TRACE;

    // the modules we depend on come first
    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();
    COM::CComGlobal::Instance();
    WEB::CWebGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefDRN Global systems initializing" );

    CIDRNChannel::RegisterEvents();
    CIDRNChannelListener::RegisterEvents();
    CDRNPeerLink::RegisterEvents();
    CDRNConnector::RegisterEvents();
    CDRNNode::RegisterEvents();
    CDRNSession::RegisterEvents();
    CDRNDataItem::RegisterEvents();
    CDRNDataGrid::RegisterEvents();
    CDRNDataStream::RegisterEvents();
    CDRNBulkTransfer::RegisterEvents();
    CDRNBulkTransferManager::RegisterEvents();

    CORE::CCoreGlobal::Instance()->GetTaskManager().RegisterTaskConsumerFactory( CDRNHashTaskConsumer::TaskType, &g_hashTaskConsumerFactory );

    m_channelFactory = GUCEF_NEW CDRNChannelFactory();
    m_channelFactory->RegisterConcreteFactory( CDRNEndpoint::ChannelTypeTcp, &g_tcpChannelFactory );
    m_channelFactory->RegisterConcreteFactory( CDRNEndpoint::ChannelTypeUdp, &g_udpChannelFactory );
    m_channelFactory->RegisterConcreteFactory( CDRNEndpoint::ChannelTypeWebSocket, &g_webSocketChannelFactory );

    m_channelListenerFactory = GUCEF_NEW CDRNChannelListenerFactory();
    m_channelListenerFactory->RegisterConcreteFactory( CDRNEndpoint::ChannelTypeTcp, &g_tcpChannelListenerFactory );
    m_channelListenerFactory->RegisterConcreteFactory( CDRNEndpoint::ChannelTypeUdp, &g_udpChannelListenerFactory );
    m_channelListenerFactory->RegisterConcreteFactory( CDRNEndpoint::ChannelTypeWebSocket, &g_webSocketChannelListenerFactory );

    m_authenticatorFactory = GUCEF_NEW CDRNAuthenticatorFactory();
    m_authenticatorFactory->RegisterConcreteFactory( CDRNAnonymousAuthenticator::MethodName, &g_anonymousAuthenticatorFactory );
    m_authenticatorFactory->RegisterConcreteFactory( CDRNSharedSecretAuthenticator::MethodName, &g_sharedSecretAuthenticatorFactory );
    m_authenticatorFactory->RegisterConcreteFactory( CDRNTokenAuthenticator::MethodName, &g_tokenAuthenticatorFactory );

    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefDRN" );
}

/*-------------------------------------------------------------------------*/

CDRNGlobal::~CDRNGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefDRN Global systems shutting down" );

    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefDRN" );
    CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterTaskConsumerFactory( CDRNHashTaskConsumer::TaskType );

    m_channelFactory->UnregisterAllConcreteFactories();
    GUCEF_DELETE m_channelFactory;
    m_channelFactory = GUCEF_NULL;

    m_channelListenerFactory->UnregisterAllConcreteFactories();
    GUCEF_DELETE m_channelListenerFactory;
    m_channelListenerFactory = GUCEF_NULL;

    m_authenticatorFactory->UnregisterAllConcreteFactories();
    GUCEF_DELETE m_authenticatorFactory;
    m_authenticatorFactory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDRNGlobal*
CDRNGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex scopeLock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CDRNGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CDRNGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CDRNGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex scopeLock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDRNChannelFactory&
CDRNGlobal::GetChannelFactory( void )
{GUCEF_TRACE;

    return *m_channelFactory;
}

/*-------------------------------------------------------------------------*/

CDRNChannelListenerFactory&
CDRNGlobal::GetChannelListenerFactory( void )
{GUCEF_TRACE;

    return *m_channelListenerFactory;
}

/*-------------------------------------------------------------------------*/

CDRNAuthenticatorFactory&
CDRNGlobal::GetAuthenticatorFactory( void )
{GUCEF_TRACE;

    return *m_authenticatorFactory;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
