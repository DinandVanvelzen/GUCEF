/*
 *  comcorepluginUPNP: Generic GUCEF COMCORE plugin
 *  providing port mapping with UPnP IGD through miniupnpc.
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#include <string.h>
#include <stdio.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "miniupnpc.h"
#include "upnpcommands.h"
#include "upnperrors.h"

#include "comcorepluginUPNP_CUpnpPortMapper.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCOREPLUGIN {
namespace UPNP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CUpnpPortMapper::MapperName = "upnp";
const CORE::CString CUpnpPortMapperTask::TaskType = "UpnpPortMapper";

static const UInt32 TimerIntervalInMs = 50;
static const UInt32 MaxExternalPortAttempts = 8;
static const int UpnpErrorConflictInMappingEntry = 718;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What is known of the gateway: miniupnpc's description of it, found once and shared by the tasks.
 *  The tasks take turns, miniupnpc is not made for use from several threads at once.
 */
class CUpnpGateway
{
    public:

    CUpnpGateway( void )
        : m_lock()
        , m_discovered( false )
        , m_discoveryFailed( false )
    {GUCEF_TRACE;

        memset( &m_urls, 0, sizeof( m_urls ) );
        memset( &m_data, 0, sizeof( m_data ) );
        memset( m_lanAddress, 0, sizeof( m_lanAddress ) );
    }

    ~CUpnpGateway()
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        if ( m_discovered )
            FreeUPNPUrls( &m_urls );
    }

    bool HasDiscoveryFailed( void ) const
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        return m_discoveryFailed;
    }

    bool AddPortMapping( UInt32 discoveryTimeoutInMs           ,
                         const CORE::CString& multicastIf       ,
                         const char* protocol                   ,
                         UInt16 internalPort                    ,
                         UInt16 wantedExternalPort              ,
                         const CORE::CString& description       ,
                         UInt16& externalPort                   ,
                         CORE::CString& error                   )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        if ( !EnsureDiscovered( discoveryTimeoutInMs, multicastIf, error ) )
            return false;

        // UPnP IGD v1 has no 'any port': start at the wanted or internal port and move up on conflicts
        UInt16 candidate = 0 != wantedExternalPort ? wantedExternalPort : internalPort;
        UInt32 attempts = 0 != wantedExternalPort ? 1 : MaxExternalPortAttempts;
        char internalPortStr[ 16 ];
        sprintf( internalPortStr, "%u", (unsigned int) internalPort );
        for ( UInt32 i=0; i<attempts; ++i )
        {
            char externalPortStr[ 16 ];
            sprintf( externalPortStr, "%u", (unsigned int) candidate );
            int result = UPNP_AddPortMapping( m_urls.controlURL, m_data.servicetype, externalPortStr, internalPortStr,
                                              m_lanAddress, description.C_String(), protocol, GUCEF_NULL );
            if ( 0 == result )
            {
                externalPort = candidate;
                return true;
            }
            if ( UpnpErrorConflictInMappingEntry != result )
            {
                error = "The gateway refused the mapping: " + CORE::Int32ToString( result ) + " " + strupnperror( result );
                return false;
            }
            ++candidate;
        }
        error = "Every external port that was tried is in use";
        return false;
    }

    bool DeletePortMapping( UInt32 discoveryTimeoutInMs     ,
                            const CORE::CString& multicastIf ,
                            const char* protocol             ,
                            UInt16 externalPort              ,
                            CORE::CString& error             )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        if ( !EnsureDiscovered( discoveryTimeoutInMs, multicastIf, error ) )
            return false;

        char externalPortStr[ 16 ];
        sprintf( externalPortStr, "%u", (unsigned int) externalPort );
        int result = UPNP_DeletePortMapping( m_urls.controlURL, m_data.servicetype, externalPortStr, protocol, GUCEF_NULL );
        if ( 0 != result )
        {
            error = "The gateway did not remove the mapping: " + CORE::Int32ToString( result ) + " " + strupnperror( result );
            return false;
        }
        return true;
    }

    bool GetExternalAddress( UInt32 discoveryTimeoutInMs     ,
                             const CORE::CString& multicastIf ,
                             COMCORE::CIPv4Address& address   ,
                             CORE::CString& error             )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        if ( !EnsureDiscovered( discoveryTimeoutInMs, multicastIf, error ) )
            return false;

        char externalAddress[ 64 ];
        memset( externalAddress, 0, sizeof( externalAddress ) );
        int result = UPNP_GetExternalIPAddress( m_urls.controlURL, m_data.servicetype, externalAddress );
        if ( 0 != result || '\0' == externalAddress[ 0 ] || !address.SetAddress( CORE::CString( externalAddress ) ) )
        {
            error = "The gateway did not tell its external address: " + CORE::Int32ToString( result );
            return false;
        }
        return true;
    }

    private:

    bool EnsureDiscovered( UInt32 discoveryTimeoutInMs, const CORE::CString& multicastIf, CORE::CString& error )
    {GUCEF_TRACE;

        if ( m_discovered )
            return true;

        struct UPNPDev* devices = upnpDiscover( (int) discoveryTimeoutInMs, multicastIf.IsNULLOrEmpty() ? GUCEF_NULL : multicastIf.C_String(), GUCEF_NULL, 0 );
        if ( GUCEF_NULL == devices )
        {
            m_discoveryFailed = true;
            error = "No UPnP device answered";
            return false;
        }

        int igd = UPNP_GetValidIGD( devices, &m_urls, &m_data, m_lanAddress, (int) sizeof( m_lanAddress ) );
        freeUPNPDevlist( devices );
        if ( 1 != igd && 2 != igd )
        {
            if ( 0 != igd )
                FreeUPNPUrls( &m_urls );
            memset( &m_urls, 0, sizeof( m_urls ) );
            m_discoveryFailed = true;
            error = "None of the UPnP devices that answered is an internet gateway";
            return false;
        }

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "UpnpPortMapper: Found the internet gateway at " << m_urls.controlURL << ", our address is " << m_lanAddress );
        m_discovered = true;
        m_discoveryFailed = false;
        return true;
    }

    private:

    mutable MT::CMutex m_lock;
    bool m_discovered;
    bool m_discoveryFailed;
    struct UPNPUrls m_urls;
    struct IGDdatas m_data;
    char m_lanAddress[ 64 ];
};

/*-------------------------------------------------------------------------*/

/**
 *  One request for the thread pool: what to do and the future to complete when done
 */
class CUpnpPortMapperTaskData : public CORE::CICloneable
{
    public:

    int kind;
    COMCORE::CPortMappingResult::TProtocol protocol;
    UInt16 internalPort;
    UInt16 externalPort;
    UInt32 discoveryTimeoutInMs;
    CORE::CString multicastInterface;
    CORE::CString description;
    CUpnpGatewayPtr gateway;
    CORE::CFutureResultSource source;

    CUpnpPortMapperTaskData( void )
        : CORE::CICloneable()
        , kind( 0 )
        , protocol( COMCORE::CPortMappingResult::PROTOCOL_UDP )
        , internalPort( 0 )
        , externalPort( 0 )
        , discoveryTimeoutInMs( 2000 )
        , multicastInterface()
        , description()
        , gateway()
        , source()
    {GUCEF_TRACE;

    }

    CUpnpPortMapperTaskData( const CUpnpPortMapperTaskData& src )
        : CORE::CICloneable( src )
        , kind( src.kind )
        , protocol( src.protocol )
        , internalPort( src.internalPort )
        , externalPort( src.externalPort )
        , discoveryTimeoutInMs( src.discoveryTimeoutInMs )
        , multicastInterface( src.multicastInterface )
        , description( src.description )
        , gateway( src.gateway )
        , source( src.source )
    {GUCEF_TRACE;

    }

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW CUpnpPortMapperTaskData( *this );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUpnpPortMapperTask::CUpnpPortMapperTask( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUpnpPortMapperTask::~CUpnpPortMapperTask()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CUpnpPortMapperTask::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CUpnpPortMapperTask::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CUpnpPortMapperTaskData* data = static_cast< CUpnpPortMapperTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == data || data->gateway.IsNULL() )
        return true;

    const char* protocol = COMCORE::CPortMappingResult::ProtocolToString( data->protocol );
    COMCORE::CPortMappingResult result;
    result.protocol = data->protocol;
    result.internalPort = data->internalPort;
    result.mapperName = CUpnpPortMapper::MapperName;
    CORE::CString error;

    bool success = false;
    switch ( data->kind )
    {
        case 0:     // map
        {
            UInt16 externalPort = 0;
            success = data->gateway->AddPortMapping( data->discoveryTimeoutInMs, data->multicastInterface, protocol, data->internalPort,
                                                     data->externalPort, data->description, externalPort, error );
            if ( success )
            {
                result.externalPort = externalPort;
                COMCORE::CIPv4Address address;
                CORE::CString addressError;
                if ( data->gateway->GetExternalAddress( data->discoveryTimeoutInMs, data->multicastInterface, address, addressError ) )
                {
                    address.SetPortInHostByteOrder( externalPort );
                    result.externalAddress = address;
                }
            }
            break;
        }
        case 1:     // unmap
        {
            result.externalPort = data->externalPort;
            success = data->gateway->DeletePortMapping( data->discoveryTimeoutInMs, data->multicastInterface, protocol, data->externalPort, error );
            break;
        }
        default:    // external address
        {
            COMCORE::CIPv4Address address;
            success = data->gateway->GetExternalAddress( data->discoveryTimeoutInMs, data->multicastInterface, address, error );
            if ( success )
                result.externalAddress = address;
            break;
        }
    }

    if ( success )
        data->source.Complete( result.Clone(), true );
    else
        data->source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, error );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUpnpPortMapperTask::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    // A task that did not get to run its cycle still owes its future an answer
    if ( task.IsNULL() )
        return;
    CUpnpPortMapperTaskData* data = static_cast< CUpnpPortMapperTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL != data && !data->source.IsCompleted() )
        data->source.Cancel();
}

/*-------------------------------------------------------------------------*/

CUpnpPortMapper::SPendingRequest::SPendingRequest( void )
    : kind( REQUESTKIND_MAP )
    , protocol( COMCORE::CPortMappingResult::PROTOCOL_UDP )
    , internalPort( 0 )
    , future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUpnpPortMapper::CUpnpPortMapper( void )
    : COMCORE::CIPortMapper()
    , m_gateway( GUCEF_NEW CUpnpGateway() )
    , m_timer( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), TimerIntervalInMs )
    , m_pending()
    , m_mappings()
    , m_discoveryTimeoutInMs( 2000 )
    , m_multicastInterface()
    , m_description( "GUCEF" )
    , m_threadPoolName()
{GUCEF_TRACE;

    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CUpnpPortMapper::~CUpnpPortMapper()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();

    // The mappings we made go with us. The tasks that are still running share the gateway and finish on their own.
    for ( size_t i=0; i<m_mappings.size(); ++i )
    {
        CORE::CString error;
        if ( !m_gateway->DeletePortMapping( m_discoveryTimeoutInMs, m_multicastInterface,
                                            COMCORE::CPortMappingResult::ProtocolToString( m_mappings[ i ].protocol ),
                                            m_mappings[ i ].externalPort, error ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "UpnpPortMapper: " << error );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CUpnpPortMapper::IsAvailable( void ) const
{GUCEF_TRACE;

    return !m_gateway->HasDiscoveryFailed();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUpnpPortMapper::GetMapperName( void ) const
{GUCEF_TRACE;

    return MapperName;
}

/*-------------------------------------------------------------------------*/

bool
CUpnpPortMapper::QueueRequest( ERequestKind kind, TProtocol protocol, UInt16 internalPort, UInt16 externalPort, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CUpnpPortMapperTaskData taskData;
    taskData.kind = (int) kind;
    taskData.protocol = protocol;
    taskData.internalPort = internalPort;
    taskData.externalPort = externalPort;
    taskData.discoveryTimeoutInMs = m_discoveryTimeoutInMs;
    taskData.multicastInterface = m_multicastInterface;
    taskData.description = m_description;
    taskData.gateway = m_gateway;

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    CORE::ThreadPoolPtr threadPool = taskManager.GetOrCreateThreadPool( m_threadPoolName.IsNULLOrEmpty() ? CORE::CTaskManager::DefaultThreadPoolName : m_threadPoolName );
    if ( threadPool.IsNULL() )
        return false;

    CORE::CFutureResult taskFuture = threadPool->QueueTask( CUpnpPortMapperTask::TaskType, &taskData );
    if ( taskFuture.HasNoFuture() )
    {
        TPortMapperErrorEventData eData( "Unable to queue the UPnP request in the thread pool" );
        NotifyObservers( PortMapperErrorEvent, &eData );
        return false;
    }

    TPendingRequest pending;
    pending.kind = kind;
    pending.protocol = protocol;
    pending.internalPort = internalPort;
    pending.future = taskData.source.GetFuture();
    m_pending.push_back( pending );
    m_timer.SetEnabled( true );

    future = pending.future;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUpnpPortMapper::RequestMapping( TProtocol protocol            ,
                                 UInt16 internalPort           ,
                                 UInt16 externalPort           ,
                                 UInt32 leaseInSecs            ,
                                 CORE::CFutureResult& future   )
{GUCEF_TRACE;

    if ( 0 == internalPort )
        return false;
    return QueueRequest( REQUESTKIND_MAP, protocol, internalPort, externalPort, future );
}

/*-------------------------------------------------------------------------*/

bool
CUpnpPortMapper::RemoveMapping( TProtocol protocol            ,
                                UInt16 internalPort           ,
                                CORE::CFutureResult& future   )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_mappings.size(); ++i )
    {
        if ( m_mappings[ i ].protocol == protocol && m_mappings[ i ].internalPort == internalPort )
        {
            UInt16 externalPort = m_mappings[ i ].externalPort;
            m_mappings.erase( m_mappings.begin() + i );
            return QueueRequest( REQUESTKIND_UNMAP, protocol, internalPort, externalPort, future );
        }
    }

    // Not one of ours: the external port is assumed to be the internal one, the way most mappings are made
    return QueueRequest( REQUESTKIND_UNMAP, protocol, internalPort, internalPort, future );
}

/*-------------------------------------------------------------------------*/

bool
CUpnpPortMapper::GetExternalAddress( CORE::CFutureResult& future )
{GUCEF_TRACE;

    return QueueRequest( REQUESTKIND_EXTERNAL_ADDRESS, COMCORE::CPortMappingResult::PROTOCOL_UDP, 0, 0, future );
}

/*-------------------------------------------------------------------------*/

void
CUpnpPortMapper::CheckPendingRequests( void )
{GUCEF_TRACE;

    size_t i = 0;
    while ( i < m_pending.size() )
    {
        if ( !m_pending[ i ].future.IsReady() )
        {
            ++i;
            continue;
        }

        TPendingRequest done = m_pending[ i ];
        m_pending.erase( m_pending.begin() + i );

        COMCORE::CPortMappingResult result;
        if ( COMCORE::CPortMappingResult::TryGetFromFuture( done.future, result ) )
        {
            if ( REQUESTKIND_MAP == done.kind )
            {
                TMapping mapping;
                mapping.protocol = result.protocol;
                mapping.internalPort = result.internalPort;
                mapping.externalPort = result.externalPort;
                m_mappings.push_back( mapping );
                if ( !NotifyObservers( MappingCreatedEvent, &result ) ) return;
            }
        }
        else
        {
            CORE::CTaskPtr task = done.future.GetResult();
            CORE::CString reason = task.IsNULL() ? CORE::CString( "The UPnP request failed" ) : task->GetTaskStatusExtraInfo();
            TPortMapperErrorEventData eData( reason );
            if ( !NotifyObservers( PortMapperErrorEvent, &eData ) ) return;
        }
    }
    if ( m_pending.empty() )
        m_timer.SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

void
CUpnpPortMapper::OnNotify( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier == &m_timer && CORE::CTimer::TimerUpdateEvent == eventId )
        CheckPendingRequests();
}

/*-------------------------------------------------------------------------*/

bool
CUpnpPortMapper::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_discoveryTimeoutInMs = config.GetAttributeValueOrChildValueByName( "discoveryTimeoutInMs" ).AsUInt32( m_discoveryTimeoutInMs, true );
    m_multicastInterface = config.GetAttributeValueOrChildValueByName( "multicastInterface" ).AsString( m_multicastInterface, true );
    m_description = config.GetAttributeValueOrChildValueByName( "description" ).AsString( m_description, true );
    m_threadPoolName = config.GetAttributeValueOrChildValueByName( "threadPool" ).AsString( m_threadPoolName, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUpnpPortMapper::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "discoveryTimeoutInMs", m_discoveryTimeoutInMs );
    config.SetAttribute( "multicastInterface", m_multicastInterface );
    config.SetAttribute( "description", m_description );
    config.SetAttribute( "threadPool", m_threadPoolName );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUpnpPortMapper::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCOREPLUGIN::UPNP::CUpnpPortMapper";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UPNP */
}; /* namespace COMCOREPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
