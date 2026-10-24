/*
 *  gucefCOM: GUCEF module providing communication implementations 
 *  for standardized protocols
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

#ifndef GUCEF_CORE_CMETRICSCLIENTMANAGER_H
#include "gucefCORE_CMetricsClientManager.h"
#define GUCEF_CORE_CMETRICSCLIENTMANAGER_H
#endif /* GUCEF_CORE_CMETRICSCLIENTMANAGER_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */
                     
#ifndef GUCEF_COM_CPHUDPSOCKET_H
#include "CPHUDPSocket.h"
#define GUCEF_COM_CPHUDPSOCKET_H
#endif /* GUCEF_COM_CPHUDPSOCKET_H ? */

#ifndef GUCEF_COM_CSTATSDCLIENT_H
#include "gucefCOM_CStatsDClient.h"
#define GUCEF_COM_CSTATSDCLIENT_H
#endif /* GUCEF_COM_CSTATSDCLIENT_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYMANAGER_H
#include "gucefCOMCORE_CDiscoveryManager.h"
#define GUCEF_COMCORE_CDISCOVERYMANAGER_H
#endif /* GUCEF_COMCORE_CDISCOVERYMANAGER_H ? */

#ifndef GUCEF_COM_CUDPBROADCASTDISCOVERYCLIENT_H
#include "gucefCOM_CUdpBroadcastDiscoveryClient.h"
#define GUCEF_COM_CUDPBROADCASTDISCOVERYCLIENT_H
#endif /* GUCEF_COM_CUDPBROADCASTDISCOVERYCLIENT_H ? */

#ifndef GUCEF_COM_CUDPMULTICASTDISCOVERYCLIENT_H
#include "gucefCOM_CUdpMulticastDiscoveryClient.h"
#define GUCEF_COM_CUDPMULTICASTDISCOVERYCLIENT_H
#endif /* GUCEF_COM_CUDPMULTICASTDISCOVERYCLIENT_H ? */

#ifndef GUCEF_COM_CMDNSDISCOVERYCLIENT_H
#include "gucefCOM_CMdnsDiscoveryClient.h"
#define GUCEF_COM_CMDNSDISCOVERYCLIENT_H
#endif /* GUCEF_COM_CMDNSDISCOVERYCLIENT_H ? */

#ifndef GUCEF_COM_CSSDPDISCOVERYCLIENT_H
#include "gucefCOM_CSsdpDiscoveryClient.h"
#define GUCEF_COM_CSSDPDISCOVERYCLIENT_H
#endif /* GUCEF_COM_CSSDPDISCOVERYCLIENT_H ? */

#ifndef GUCEF_COM_CICMPRTTPROBE_H
#include "gucefCOM_CIcmpRttProbe.h"
#define GUCEF_COM_CICMPRTTPROBE_H
#endif /* GUCEF_COM_CICMPRTTPROBE_H ? */

#ifndef GUCEF_COM_CTCPCONNECTRTTPROBE_H
#include "gucefCOM_CTcpConnectRttProbe.h"
#define GUCEF_COM_CTCPCONNECTRTTPROBE_H
#endif /* GUCEF_COM_CTCPCONNECTRTTPROBE_H ? */

#ifndef GUCEF_COM_CUDPECHORTTPROBE_H
#include "gucefCOM_CUdpEchoRttProbe.h"
#define GUCEF_COM_CUDPECHORTTPROBE_H
#endif /* GUCEF_COM_CUDPECHORTTPROBE_H ? */

#ifndef GUCEF_COM_CNATPMPPORTMAPPER_H
#include "gucefCOM_CNatPmpPortMapper.h"
#define GUCEF_COM_CNATPMPPORTMAPPER_H
#endif /* GUCEF_COM_CNATPMPPORTMAPPER_H ? */

#ifndef GUCEF_COM_CSTUNCLIENT_H
#include "gucefCOM_CStunClient.h"
#define GUCEF_COM_CSTUNCLIENT_H
#endif /* GUCEF_COM_CSTUNCLIENT_H ? */

#ifndef GUCEF_COM_CIRCTCPTRANSPORT_H
#include "gucefCOM_CIrcTcpTransport.h"
#define GUCEF_COM_CIRCTCPTRANSPORT_H
#endif /* GUCEF_COM_CIRCTCPTRANSPORT_H ? */

#ifndef GUCEF_COM_CIRCCLIENT_H
#include "gucefCOM_CIrcClient.h"
#define GUCEF_COM_CIRCCLIENT_H
#endif /* GUCEF_COM_CIRCCLIENT_H ? */

#include "gucefCOM_CComGlobal.h"  /* definition of the class implemented here */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CComGlobal::g_dataLock;
CComGlobal* CComGlobal::g_instance = GUCEF_NULL;

static TUdpBroadcastDiscoveryClientFactory g_udpBroadcastDiscoveryClientFactory;
static TUdpMulticastDiscoveryClientFactory g_udpMulticastDiscoveryClientFactory;
static TMdnsDiscoveryClientFactory g_mdnsDiscoveryClientFactory;
static TSsdpDiscoveryClientFactory g_ssdpDiscoveryClientFactory;
static TIcmpRttProbeFactory g_icmpRttProbeFactory;
static TTcpConnectRttProbeFactory g_tcpConnectRttProbeFactory;
static TUdpEchoRttProbeFactory g_udpEchoRttProbeFactory;
static TNatPmpPortMapperFactory g_natPmpPortMapperFactory;
static TIrcTcpTransportFactory g_ircTcpTransportFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CComGlobal::CComGlobal( void )
    : m_ircTransportFactory( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CComGlobal::Initialize( void )
{GUCEF_TRACE;

    // Initialize modules we depend on
    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefCOM Global systems initializing" );

    CPHUDPSocket::RegisterEvents();

    CIIrcTransport::RegisterEvents();
    CIrcClient::RegisterEvents();

    m_ircTransportFactory = GUCEF_NEW CIrcTransportFactory();
    m_ircTransportFactory->RegisterConcreteFactory( CIIrcTransport::TcpTransportName, &g_ircTcpTransportFactory );

    // The implementations of the COMCORE interfaces that need nothing but COMCORE
    COMCORE::CComCoreGlobal* comCore = COMCORE::CComCoreGlobal::Instance();
    COMCORE::CDiscoveryClientFactory& discoveryClientFactory = comCore->GetDiscoveryClientFactory();
    discoveryClientFactory.RegisterConcreteFactory( CUdpBroadcastDiscoveryClient::TypeName, &g_udpBroadcastDiscoveryClientFactory );
    discoveryClientFactory.RegisterConcreteFactory( CUdpMulticastDiscoveryClient::TypeName, &g_udpMulticastDiscoveryClientFactory );
    discoveryClientFactory.RegisterConcreteFactory( CMdnsDiscoveryClient::TypeName, &g_mdnsDiscoveryClientFactory );
    discoveryClientFactory.RegisterConcreteFactory( CSsdpDiscoveryClient::TypeName, &g_ssdpDiscoveryClientFactory );

    COMCORE::CRttProbeFactory& rttProbeFactory = comCore->GetRttProbeFactory();
    rttProbeFactory.RegisterConcreteFactory( CIcmpRttProbe::MethodName, &g_icmpRttProbeFactory );
    rttProbeFactory.RegisterConcreteFactory( CTcpConnectRttProbe::MethodName, &g_tcpConnectRttProbeFactory );
    rttProbeFactory.RegisterConcreteFactory( CUdpEchoRttProbe::MethodName, &g_udpEchoRttProbeFactory );

    comCore->GetPortMapperFactory().RegisterConcreteFactory( CNatPmpPortMapper::MapperName, &g_natPmpPortMapperFactory );

    CORE::CMetricsClientManager::CIMetricsSystemClientPtr statsDClient( GUCEF_NEW CStatsDClient() );
    CORE::CCoreGlobal::Instance()->GetMetricsClientManager().AddMetricsClient( CStatsDClient::Type, statsDClient );
    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefCOM" );
}

/*-------------------------------------------------------------------------*/

CComGlobal::~CComGlobal()
{GUCEF_TRACE;
  
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefCOM Global systems shutting down" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefCOM" );

    CORE::CCoreGlobal::Instance()->GetMetricsClientManager().RemoveMetricsClient( CStatsDClient::Type );

    // Our concrete factories go away with this module, so do the clients the discovery manager made with them
    COMCORE::CComCoreGlobal* comCore = COMCORE::CComCoreGlobal::Instance();
    COMCORE::CDiscoveryManager& discoveryManager = comCore->GetDiscoveryManager();
    discoveryManager.UnregisterDiscoveryClientsOfType( CUdpBroadcastDiscoveryClient::TypeName );
    discoveryManager.UnregisterDiscoveryClientsOfType( CUdpMulticastDiscoveryClient::TypeName );
    discoveryManager.UnregisterDiscoveryClientsOfType( CMdnsDiscoveryClient::TypeName );
    discoveryManager.UnregisterDiscoveryClientsOfType( CSsdpDiscoveryClient::TypeName );

    COMCORE::CDiscoveryClientFactory& discoveryClientFactory = comCore->GetDiscoveryClientFactory();
    discoveryClientFactory.UnregisterConcreteFactory( CUdpBroadcastDiscoveryClient::TypeName );
    discoveryClientFactory.UnregisterConcreteFactory( CUdpMulticastDiscoveryClient::TypeName );
    discoveryClientFactory.UnregisterConcreteFactory( CMdnsDiscoveryClient::TypeName );
    discoveryClientFactory.UnregisterConcreteFactory( CSsdpDiscoveryClient::TypeName );

    COMCORE::CRttProbeFactory& rttProbeFactory = comCore->GetRttProbeFactory();
    rttProbeFactory.UnregisterConcreteFactory( CIcmpRttProbe::MethodName );
    rttProbeFactory.UnregisterConcreteFactory( CTcpConnectRttProbe::MethodName );
    rttProbeFactory.UnregisterConcreteFactory( CUdpEchoRttProbe::MethodName );

    comCore->GetPortMapperFactory().UnregisterConcreteFactory( CNatPmpPortMapper::MapperName );

    m_ircTransportFactory->UnregisterAllConcreteFactories();
    GUCEF_DELETE m_ircTransportFactory;
    m_ircTransportFactory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CComGlobal*
CComGlobal::Instance()
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CComGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CComGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

CIrcTransportFactory&
CComGlobal::GetIrcTransportFactory( void )
{GUCEF_TRACE;

    return *m_ircTransportFactory;
}

/*-------------------------------------------------------------------------*/

void
CComGlobal::Deinstance( void )
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

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
