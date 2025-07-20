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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CTASKDELEGATOR_H
#include "gucefCORE_CTaskDelegator.h"
#define GUCEF_CORE_CTASKDELEGATOR_H
#endif /* GUCEF_CORE_CTASKDELEGATOR_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef GUCEF_COMCORE_CPING_H
#include "CPing.h"
#define GUCEF_COMCORE_CPING_H
#endif /* GUCEF_COMCORE_CPING_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  #ifndef GUCEF_COMCORE_ICMPAPI_H
  #include "gucefCOMCORE_icmpApi.h"
  #define GUCEF_COMCORE_ICMPAPI_H
  #endif /* GUCEF_COMCORE_ICMPAPI_H ? */

#endif

#include "gucefCOMCORE_CPingTaskConsumer.h"

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

const CORE::CString CPingTaskConsumer::TaskType = "PingTask";
const CORE::CString CPingTaskConsumer::ClassTypeName = "GUCEF::COMCORE::CPingTaskConsumer";

const CORE::CEvent CPingTaskConsumer::PingStartedEvent = "GUCEF::COMCORE::CPingTaskConsumer::PingStartedEvent";
const CORE::CEvent CPingTaskConsumer::PingReponseEvent = "GUCEF::COMCORE::CPingTaskConsumer::PingReponseEvent";
const CORE::CEvent CPingTaskConsumer::PingTimeoutEvent = "GUCEF::COMCORE::CPingTaskConsumer::PingTimeoutEvent";
const CORE::CEvent CPingTaskConsumer::PingFailedEvent = "GUCEF::COMCORE::CPingTaskConsumer::PingFailedEvent";
const CORE::CEvent CPingTaskConsumer::PingStoppedEvent = "GUCEF::COMCORE::CPingTaskConsumer::PingStoppedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

struct SMsWinPingData
{
    HANDLE pingEvent;
    HANDLE icmpHandle;
};
typedef struct SMsWinPingData TMsWinPingData;

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CPingTaskConsumer::RegisterEvents( void )
{GUCEF_TRACE;

    PingStartedEvent.Initialize();
    PingReponseEvent.Initialize();
    PingTimeoutEvent.Initialize();
    PingFailedEvent.Initialize();
    PingStoppedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CPingTaskConsumer::CPingTaskData::CPingTaskData( const TStringVector& remoteHosts ,
                                                 const Int32 maxPings             ,
                                                 const UInt32 bytesToSend         ,
                                                 const UInt32 timeout             ,
                                                 const Int32 minimalPingDelta     )
    : CICloneable()                          ,
      m_hostList( remoteHosts )              ,
      m_maxPings( maxPings )                 ,
      m_bytesToSend( bytesToSend )           ,
      m_timeout( timeout )                   ,
      m_minimalPingDelta( minimalPingDelta )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
        
CPingTaskConsumer::CPingTaskData::CPingTaskData( const CPingTaskData& src )
    : CICloneable( src )                           ,
      m_hostList( src.m_hostList )                 ,
      m_maxPings( src.m_maxPings )                 ,
      m_bytesToSend( src.m_bytesToSend )           ,
      m_timeout( src.m_timeout )                   ,
      m_minimalPingDelta( src.m_minimalPingDelta )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
        
CORE::CICloneable*
CPingTaskConsumer::CPingTaskData::Clone( void ) const
{GUCEF_TRACE;
    
    return GUCEF_NEW CPingTaskData( *this );
}

/*-------------------------------------------------------------------------*/
        
void
CPingTaskConsumer::CPingTaskData::SetRemoteHosts( const TStringVector& hostList )
{GUCEF_TRACE;

    m_hostList = hostList;
}

/*-------------------------------------------------------------------------*/
        
const CPingTaskConsumer::TStringVector&
CPingTaskConsumer::CPingTaskData::GetRemoteHosts( void ) const
{GUCEF_TRACE;

    return m_hostList;
}

/*-------------------------------------------------------------------------*/
        
void
CPingTaskConsumer::CPingTaskData::SetMaxPings( const Int32 maxPings )
{GUCEF_TRACE;

    m_maxPings = maxPings;
}

/*-------------------------------------------------------------------------*/
        
Int32
CPingTaskConsumer::CPingTaskData::GetMaxPings( void ) const
{GUCEF_TRACE;

    return m_maxPings;
}

/*-------------------------------------------------------------------------*/
        
void
CPingTaskConsumer::CPingTaskData::SetBytesToSend( const UInt32 bytesToSend )
{GUCEF_TRACE;

    m_bytesToSend = bytesToSend;
}
        
/*-------------------------------------------------------------------------*/

UInt32
CPingTaskConsumer::CPingTaskData::GetBytesToSend( void ) const
{GUCEF_TRACE;

    return m_bytesToSend;
}

/*-------------------------------------------------------------------------*/
        
void
CPingTaskConsumer::CPingTaskData::SetTimeout( const UInt32 timeout )
{GUCEF_TRACE;

    m_timeout = timeout;
}
        
/*-------------------------------------------------------------------------*/

UInt32
CPingTaskConsumer::CPingTaskData::GetTimeout( void ) const
{GUCEF_TRACE;

    return m_timeout;
}

/*-------------------------------------------------------------------------*/
        
void
CPingTaskConsumer::CPingTaskData::SetMinimalPingDelta( const UInt32 minimalPingDelta )
{GUCEF_TRACE;

    m_minimalPingDelta = minimalPingDelta;
}
        
/*-------------------------------------------------------------------------*/        

UInt32
CPingTaskConsumer::CPingTaskData::GetMinimalPingDelta( void ) const
{GUCEF_TRACE;

    return m_minimalPingDelta;
}

/*-------------------------------------------------------------------------*/
    
CPingTaskConsumer::CEchoReceivedEventData::CEchoReceivedEventData( const CHostAddress& host   ,
                                                                   const UInt32 echoSize      ,
                                                                   const UInt32 roundTripTime )
    : CORE::CICloneable()              ,
      m_hostAddress( host )            ,
      m_echoSize( echoSize )           ,
      m_roundTripTime( roundTripTime )
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/

CPingTaskConsumer::CEchoReceivedEventData::CEchoReceivedEventData( const CEchoReceivedEventData& src )
    : CORE::CICloneable( src)                ,
      m_hostAddress( src.m_hostAddress )     ,
      m_echoSize( src.m_echoSize )           ,
      m_roundTripTime( src.m_roundTripTime )
{GUCEF_TRACE;    

}

/*-------------------------------------------------------------------------*/
        
CPingTaskConsumer::CEchoReceivedEventData::~CEchoReceivedEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
                
CORE::CICloneable*
CPingTaskConsumer::CEchoReceivedEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEchoReceivedEventData( *this );
}

/*-------------------------------------------------------------------------*/
        
const CHostAddress&
CPingTaskConsumer::CEchoReceivedEventData::GetHostAddress( void ) const
{GUCEF_TRACE;

    return m_hostAddress;
}

/*-------------------------------------------------------------------------*/
        
UInt32
CPingTaskConsumer::CEchoReceivedEventData::GetEchoSize( void ) const
{GUCEF_TRACE;

    return m_echoSize;
}

/*-------------------------------------------------------------------------*/
        
UInt32
CPingTaskConsumer::CEchoReceivedEventData::GetRoundTripTime( void ) const
{GUCEF_TRACE;

    return m_roundTripTime;
}

/*-------------------------------------------------------------------------*/

CPingTaskConsumer::CPingTaskConsumer( void )
    : CTaskConsumer()        ,
      m_platformData( NULL ) ,
      m_notDone( true )      ,
      m_taskData( NULL )     ,
      m_pingCounters()
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    TMsWinPingData* platformData = GUCEF_NEW TMsWinPingData;
    platformData->icmpHandle = IcmpCreateFile();
    platformData->pingEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_platformData = platformData;
    
    #endif
    
    RegisterEvents();
}

/*-------------------------------------------------------------------------*/
    
CPingTaskConsumer::~CPingTaskConsumer()
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    CloseHandle( ((TMsWinPingData*)m_platformData)->pingEvent );
    IcmpCloseHandle( ((TMsWinPingData*)m_platformData)->icmpHandle );
    
    #endif
}
    
/*-------------------------------------------------------------------------*/

CString
CPingTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/
    
const CString&
CPingTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

void
CPingTaskConsumer::IcmpCallback( void* vdata )
{GUCEF_TRACE;

    TPingEntry* entry = static_cast< TPingEntry* >( vdata );
    
    //UInt32 replyCount = IcmpParseReplies( entry->replyBuffer, sizeof(ICMP_ECHO_REPLY) + entry->echoSize ); 
    PICMP_ECHO_REPLY echoReply = (PICMP_ECHO_REPLY) entry->replyBuffer;
    
    if ( echoReply->Status == IP_SUCCESS )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPingTaskConsumer: ICMP callback: ping response recieved from " + entry->host->GetHostname() + " with RTT " + CORE::UInt32ToString( echoReply->RoundTripTime ) );
        
        CEchoReceivedEventData eventData( *entry->host, entry->echoSize, echoReply->RoundTripTime );
        entry->taskConsumer->NotifyObservers( PingReponseEvent, &eventData );
    }
    else
    if ( ( echoReply->Status == IP_REQ_TIMED_OUT )       ||
         ( echoReply->Status == IP_TTL_EXPIRED_TRANSIT ) ||
         ( echoReply->Status == IP_TTL_EXPIRED_REASSEM )  )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPingTaskConsumer: ICMP callback: ping timed out to " + entry->host->GetHostname() );
        
        CEchoReceivedEventData eventData( *entry->host, entry->echoSize, echoReply->RoundTripTime );
        entry->taskConsumer->NotifyObservers( PingTimeoutEvent, &eventData );        
    }
    else
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPingTaskConsumer: ICMP callback: ping failed to " + entry->host->GetHostname() );
        
        CEchoReceivedEventData eventData( *entry->host, entry->echoSize, echoReply->RoundTripTime );
        entry->taskConsumer->NotifyObservers( PingFailedEvent, &eventData );
    }

    // Make sure we set the wait flag to false to signal that we finished this ping attempt for
    // the given ping entry
    TPingCounters::iterator n = entry->taskConsumer->m_pingCounters.find( *entry->host );
    if ( n != entry->taskConsumer->m_pingCounters.end() )
    {
        (*n).second.areWeWaitingForPingResult = false;
    }
        
    if ( entry->pingCount == (UInt32)entry->taskConsumer->m_taskData->GetMaxPings() )
    {
        // We reached the max number of pings for this host,.. send a ping stopped event
        CEchoReceivedEventData eventData( *entry->host, entry->echoSize, 0 );
        entry->taskConsumer->NotifyObservers( PingStoppedEvent, &eventData );                            
    }
}

#endif

/*-------------------------------------------------------------------------*/

bool
CPingTaskConsumer::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CPingTaskConsumer::OnTaskEnding( CORE::CTaskPtr task ,
                                 bool willBeForced   )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPingTaskConsumer::OnTaskEnded( CORE::CTaskPtr task ,
                                bool wasForced      )
{GUCEF_TRACE;

    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
bool
CPingTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true; // no task to process, nothing to do

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPingTaskConsumer: Starting to process the task" );
    
    CORE::CICloneable* opaqueTaskData = task->GetTaskData();
    m_taskData = static_cast< CPingTaskData* >( opaqueTaskData );
    m_notDone = true;
    
    // Prepare our ping packet
    UInt8* pingData = GUCEF_NEW UInt8[ m_taskData->GetBytesToSend() ];
    memset( pingData, 'x', m_taskData->GetBytesToSend() );
        
    // Setup all ping counters
    const TStringVector& hosts = m_taskData->GetRemoteHosts();
    for ( UInt32 i=0; i<hosts.size(); ++i )
    {
        CHostAddress host;
        host.SetHostname( hosts[ i ] );
        
        TPingEntry pingEntry;
        pingEntry.pingCount = 0;
        pingEntry.ticksAtLastPing = MT::PrecisionTickCount();
        pingEntry.areWeWaitingForPingResult = false;
        pingEntry.echoSize = m_taskData->GetBytesToSend();
        pingEntry.replyBuffer = GUCEF_NEW Int8[ sizeof(ICMP_ECHO_REPLY) + pingEntry.echoSize ];
        pingEntry.taskConsumer = this;
        
        m_pingCounters.insert( std::pair< CHostAddress, TPingEntry >( host, pingEntry ) );
        TPingCounters::iterator n = m_pingCounters.find( host );
        (*n).second.host = &(*n).first;
        
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPingTaskConsumer: Initialized ping counter for " + (*n).second.host->GetHostname() );
    }
    
    // Now we have to stick around untill all pings are done or this task is cancelled
    TTaskDelegatorBasicPtr delegator = GetTaskDelegator();
    while ( m_notDone && delegator->IsActive() )
    {
        m_notDone = false;
        TPingCounters::iterator i = m_pingCounters.begin();
        while ( i != m_pingCounters.end() && delegator->IsActive() )
        {
            TPingEntry& pingEntry = (*i).second;
            
            // Make sure we give a ping time to complete
            if ( pingEntry.areWeWaitingForPingResult )
            {
                m_notDone = true;
                ++i;
                continue;
            }
            
            // Make sure we don't ping a host to often
            if ( m_taskData->GetMaxPings() < 1 || pingEntry.pingCount < (UInt32)m_taskData->GetMaxPings() )
            {
                m_notDone = true;
                
                // Make sure we wait the desired time before sending out another ping
                if ( (UInt32)( MT::PrecisionTickCount() - pingEntry.ticksAtLastPing ) > m_taskData->GetMinimalPingDelta() )
                {
                    if ( pingEntry.pingCount == 0 )
                    {
                        // This is the first time we are pinging this host, send out a ping start event
                        CEchoReceivedEventData eventData( *pingEntry.host, pingEntry.echoSize, 0 );
                        NotifyObservers( PingStartedEvent, &eventData );
                    }
                                            
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPingTaskConsumer: Sending ping to " + pingEntry.host->GetHostname() );
                    
                    IPAddr netIp;
                    netIp.S_un.S_addr = pingEntry.host->GetRandomIPv4Address().GetAddress();
                    pingEntry.areWeWaitingForPingResult = true;
                    
                    DWORD result = IcmpSendEcho2( ((TMsWinPingData*)m_platformData)->icmpHandle          ,
                                                  NULL                                                   ,
                                                  (PIO_APC_ROUTINE_OLD)&IcmpCallback                     ,
                                                  &pingEntry                                             ,
                                                  netIp                                                  ,
                                                  pingData                                               ,
                                                  (WORD) m_taskData->GetBytesToSend()                    ,
                                                  NULL                                                   ,
                                                  pingEntry.replyBuffer                                  ,
                                                  sizeof(ICMP_ECHO_REPLY) + m_taskData->GetBytesToSend() ,
                                                  m_taskData->GetTimeout()                               );
                                                  
                    if ( 0 == result )
                    {
                        DWORD errorNr = GetLastError();
                        bool a = errorNr == ERROR_INVALID_PARAMETER;
                        bool b = errorNr == ERROR_IO_PENDING;
                        bool c = errorNr == ERROR_NOT_ENOUGH_MEMORY;
                        bool d = errorNr == ERROR_NOT_SUPPORTED;
                        bool e = errorNr == IP_BUF_TOO_SMALL;
                        
                    }
                    else
                    if ( ERROR_IO_PENDING == result )
                    {                                   
                        // This means the async call was successfull but we have to wait for the result, which is what this code is designed to do                        
                        ++pingEntry.pingCount;
                        pingEntry.ticksAtLastPing = MT::PrecisionTickCount();
                        
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPingTaskConsumer: Successfully performed an async ping dispatch to " + pingEntry.host->GetHostname() );
                    }
                    else
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPingTaskConsumer: There was a problem sending a ping to " + pingEntry.host->GetHostname() );
                        
                        ++pingEntry.pingCount;
                        pingEntry.ticksAtLastPing = MT::PrecisionTickCount();

                        // We did not get the expected return value,.. something went wrong
                        CEchoReceivedEventData eventData( *pingEntry.host, pingEntry.echoSize, 0 );
                        NotifyObservers( PingFailedEvent, &eventData );
                    }
                }
            }            
            ++i;
        }
        
        // Idle a bit before we try another round of pings (yield the thread)
        if ( delegator->IsActive() )
        {
            MT::PrecisionDelay( 25 );
        }
    }
    
    // Cleanup all ping counters
    TPingCounters::iterator i = m_pingCounters.begin();
    while ( i != m_pingCounters.end() )
    {
        TPingEntry& pingEntry = (*i).second;
        delete[] pingEntry.replyBuffer;
        ++i;        
    }
    m_pingCounters.clear();
    
    return true;
}

#else

bool
CPingTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    return true;
}

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
