/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CCOM_H
#include "CCom.h"      /* header for the main communication manager */
#define GUCEF_COMCORE_CCOM_H
#endif /* GUCEF_COMCORE_CCOM_H ? */

#ifndef CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define CTCPCLIENTSOCKET_H
#endif /* CTCPCLIENTSOCKET_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CPING_H
#include "CPing.h"
#define GUCEF_COMCORE_CPING_H
#endif /* GUCEF_COMCORE_CPING_H ? */

#ifndef GUCEF_COMCORE_CUDPMASTERSOCKET_H
#include "CUDPMasterSocket.h"
#define GUCEF_COMCORE_CUDPMASTERSOCKET_H
#endif /* GUCEF_COMCORE_CUDPMASTERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CUDPCHANNEL_H
#include "CUDPChannel.h"
#define GUCEF_COMCORE_CUDPCHANNEL_H
#endif /* GUCEF_COMCORE_CUDPCHANNEL_H ? */

#ifndef GUCEF_COMCORE_DVSOCKET_H
#include "dvwinsock.h"
#define GUCEF_COMCORE_DVSOCKET_H
#endif /* GUCEF_COMCORE_DVSOCKET_H */

#ifndef GUCEF_COMCORE_CPINGTASKCONSUMER_H
#include "gucefCOMCORE_CPingTaskConsumer.h"
#define GUCEF_COMCORE_CPINGTASKCONSUMER_H
#endif /* GUCEF_COMCORE_CPINGTASKCONSUMER_H ? */

#ifndef GUCEF_COMCORE_CDNSCACHEREFRESHTASKCONSUMER_H
#include "gucefCOMCORE_CDnsCacheRefreshTaskConsumer.h"
#define GUCEF_COMCORE_CDNSCACHEREFRESHTASKCONSUMER_H
#endif /* GUCEF_COMCORE_CDNSCACHEREFRESHTASKCONSUMER_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYMANAGER_H
#include "gucefCOMCORE_CDiscoveryManager.h"
#define GUCEF_COMCORE_CDISCOVERYMANAGER_H
#endif /* GUCEF_COMCORE_CDISCOVERYMANAGER_H ? */

#ifndef GUCEF_COMCORE_CGEOLOCATIONLOOKUPSERVICE_H
#include "gucefCOMCORE_CGeoLocationLookupService.h"
#define GUCEF_COMCORE_CGEOLOCATIONLOOKUPSERVICE_H
#endif /* GUCEF_COMCORE_CGEOLOCATIONLOOKUPSERVICE_H ? */

#include "gucefCOMCORE_CComCoreGlobal.h"  /* definition of the class implemented here */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    #ifndef GUCEF_COMCORE_IPHLPAPI_H
    #include "gucefCOMCORE_IphlpapiApi.h"
    #define GUCEF_COMCORE_IPHLPAPI_H
    #endif /* GUCEF_COMCORE_IPHLPAPI_H ? */

    #ifndef GUCEF_COMCORE_ICMPAPI_H
    #include "gucefCOMCORE_icmpApi.h"
    #define GUCEF_COMCORE_ICMPAPI_H
    #endif /* GUCEF_COMCORE_ICMPAPI_H ? */

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CComCoreGlobal::g_dataLock;
CComCoreGlobal* CComCoreGlobal::g_instance = GUCEF_NULL;
TPingTaskConsumerFactory g_pingTaskConsumerFactory;
TDnsCacheRefreshTaskConsumerFactory g_dnsCacheRefreshTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CComCoreGlobal::CComCoreGlobal( void )
    : m_com( GUCEF_NULL )
    , m_dnsCache()
    , m_discoveryManager( GUCEF_NULL )
    , m_geoLocationLookupService( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CComCoreGlobal::Initialize( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefCOMCORE Global systems initializing" );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    InitWinsock( 1 );
    LinkICMP();
    LinkLPHLPAPI();
    #endif

    CTCPConnection::RegisterEvents();
    CTCPServerSocket::RegisterEvents();
    CTCPClientSocket::RegisterEvents();
    CUDPSocket::RegisterEvents();
    CPing::RegisterEvents();
    CPingTaskConsumer::RegisterEvents();
    CUDPMasterSocket::RegisterEvents();
    CUDPChannel::RegisterEvents();
    CDnsCache::RegisterEvents();
    CDnsCacheEntry::RegisterEvents();
    
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();

    // Make the task manager capable of handling ping tasks
    taskManager.RegisterTaskConsumerFactory( CPingTaskConsumer::TaskType, &g_pingTaskConsumerFactory  );

    // Make the task manager capable of handling dns cache refresh tasks
    taskManager.RegisterTaskConsumerFactory( CDnsCacheRefreshTaskConsumer::TaskType, &g_dnsCacheRefreshTaskConsumerFactory  );

    m_com = GUCEF_NEW CCom();
    m_dnsCache = ( GUCEF_NEW CGlobalDnsCache() )->CreateSharedPtr();
    m_discoveryManager = GUCEF_NEW CDiscoveryManager();
    m_geoLocationLookupService = GUCEF_NEW CGeoLocationLookupService();
    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefCOMCORE" );
}

/*-------------------------------------------------------------------------*/

CComCoreGlobal::~CComCoreGlobal()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CComCoreGlobal::Shutdown( void )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefCOMCORE Global systems shutting down" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefCOMCORE" );

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    taskManager.UnregisterTaskConsumerFactory( CPingTaskConsumer::TaskType );
    taskManager.UnregisterTaskConsumerFactory( CDnsCacheRefreshTaskConsumer::TaskType );
    
    GUCEF_DELETE m_geoLocationLookupService;
    m_geoLocationLookupService = GUCEF_NULL;
    GUCEF_DELETE m_discoveryManager;
    m_discoveryManager = GUCEF_NULL;
    m_dnsCache.Unlink();
    GUCEF_DELETE m_com;
    m_com = GUCEF_NULL;


    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    UnlinkLPHLPAPI();
    UnlinkICMP();    
    ShutdownWinsock();
    #endif
}

/*-------------------------------------------------------------------------*/

CComCoreGlobal*
CComCoreGlobal::Instance()
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CComCoreGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CComCoreGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CComCoreGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    if ( GUCEF_NULL != g_instance )
    {
        g_instance->Shutdown();
        GUCEF_DELETE g_instance;
        g_instance = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CCom&
CComCoreGlobal::GetCom( void )
{GUCEF_TRACE;

    return *m_com;
}

/*-------------------------------------------------------------------------*/

CDnsCache&
CComCoreGlobal::GetDnsCache( void )
{GUCEF_TRACE;

    return *m_dnsCache;
}

/*-------------------------------------------------------------------------*/

CDiscoveryManager&
CComCoreGlobal::GetDiscoveryManager( void )
{GUCEF_TRACE;

    return *m_discoveryManager;
}

/*-------------------------------------------------------------------------*/

CGeoLocationLookupService& 
CComCoreGlobal::GetGeoLocationLookupService( void )
{GUCEF_TRACE;

    return *m_geoLocationLookupService;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
