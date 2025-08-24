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

//-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------//

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CSOCKET_H
#include "CSocket.h"
#define GUCEF_COMCORE_CSOCKET_H
#endif /* GUCEF_COMCORE_CSOCKET_H ? */

#ifndef GUCEF_COMCORE_DVSOCKET_H
#include "dvwinsock.h"
#define GUCEF_COMCORE_DVSOCKET_H
#endif /* GUCEF_COMCORE_DVSOCKET_H ? */

#include "CPing.h"           /* declaration of the class */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  /* #include <Icmpapi.h> -> this is the header for the functions that we dynamicly link */

  #ifndef GUCEF_COMCORE_ICMPAPI_H
  #include "gucefCOMCORE_icmpApi.h"
  #define GUCEF_COMCORE_ICMPAPI_H
  #endif /* GUCEF_COMCORE_ICMPAPI_H ? */

  #ifndef GUCEF_COMCORE_CPINGTASKCONSUMER_H
  #include "gucefCOMCORE_CPingTaskConsumer.h"
  #define GUCEF_COMCORE_CPINGTASKCONSUMER_H
  #endif /* GUCEF_COMCORE_CPINGTASKCONSUMER_H ? */

#endif /* GUCEF_PLATFORM_MSWIN ? */

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

const CORE::CEvent CPing::PingStartedEvent = "GUCEF::COMCORE::CPing::PingStartedEvent";
const CORE::CEvent CPing::PingReponseEvent = "GUCEF::COMCORE::CPing::PingReponseEvent";
const CORE::CEvent CPing::PingTimeoutEvent = "GUCEF::COMCORE::CPing::PingTimeoutEvent";
const CORE::CEvent CPing::PingFailedEvent = "GUCEF::COMCORE::CPing::PingFailedEvent";
const CORE::CEvent CPing::PingStoppedEvent = "GUCEF::COMCORE::CPing::PingStoppedEvent";

bool icmpLinked = false;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CPing::RegisterEvents( void )
{GUCEF_TRACE;

    PingStartedEvent.Initialize();
    PingReponseEvent.Initialize();
    PingTimeoutEvent.Initialize();
    PingFailedEvent.Initialize();
    PingStoppedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CPing::CPingEventData::CPingEventData( const CHostAddress& host   ,
                                       const UInt32 echoSize      ,
                                       const UInt32 roundTripTime )
    : CORE::CICloneable()              ,
      m_hostAddress( host )            ,
      m_echoSize( echoSize )           ,
      m_roundTripTime( roundTripTime )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPing::CPingEventData::CPingEventData( const CPingEventData& src )
    : CORE::CICloneable( src)                ,
      m_hostAddress( src.m_hostAddress )     ,
      m_echoSize( src.m_echoSize )           ,
      m_roundTripTime( src.m_roundTripTime )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPing::CPingEventData::~CPingEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPing::CPingEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CPingEventData( *this );
}

/*-------------------------------------------------------------------------*/

const CHostAddress&
CPing::CPingEventData::GetHostAddress( void ) const
{GUCEF_TRACE;

    return m_hostAddress;
}

/*-------------------------------------------------------------------------*/

UInt32
CPing::CPingEventData::GetEchoSize( void ) const
{GUCEF_TRACE;

    return m_echoSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CPing::CPingEventData::GetRoundTripTime( void ) const
{GUCEF_TRACE;

    return m_roundTripTime;
}

/*-------------------------------------------------------------------------*/

CPing::CPing( CORE::PulseGeneratorPtr pulsGenerator )
    : CTSGNotifier( pulsGenerator ) ,
      m_isActive( false )           ,
      m_remoteHosts()               ,
      m_maxPings( 0 )               ,
      m_bytesToSend( 32 )           ,
      m_timeout( 1000 )             ,
      m_minimalPingDelta( 500 )     ,
      m_taskId( 0 )                 ,
      m_pingTaskConsumer()
{GUCEF_TRACE;

    RegisterEvents();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    icmpLinked = LinkICMP() > 0;
    #endif /* GUCEF_PLATFORM_MSWIN ? */
}

/*-------------------------------------------------------------------------*/

CPing::CPing( void )
    : CTSGNotifier()            ,
      m_isActive( false )       ,
      m_remoteHosts()           ,
      m_maxPings( 0 )           ,
      m_bytesToSend( 32 )       ,
      m_timeout( 1000 )         ,
      m_minimalPingDelta( 500 ) ,
      m_taskId( 0 )
{GUCEF_TRACE;

    RegisterEvents();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    icmpLinked = LinkICMP() > 0;
    #endif /* GUCEF_PLATFORM_MSWIN ? */
}

/*-------------------------------------------------------------------------*/

CPing::~CPing()
{GUCEF_TRACE;

    Stop();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    UnlinkICMP();
    #endif /* GUCEF_PLATFORM_MSWIN ? */
}

/*-------------------------------------------------------------------------*/

bool
CPing::Start( const CORE::CString& remoteHost           ,
              const Int32 maxPings /* = 0 */            ,
              const UInt32 bytesToSend /* = 32 */       ,
              const UInt32 timeout /* = 1000 */         ,
              const UInt32 minimalPingDelta /* = 500 */ )
{GUCEF_TRACE;

    TStringVector hostList;
    hostList.push_back( remoteHost );
    return Start( hostList         ,
                  maxPings         ,
                  bytesToSend      ,
                  timeout          ,
                  minimalPingDelta );
}

/*-------------------------------------------------------------------------*/

bool
CPing::Start( const TStringVector& remoteHosts          ,
              const Int32 maxPings /* = 0 */            ,
              const UInt32 bytesToSend /* = 32 */       ,
              const UInt32 timeout /* = 1000 */         ,
              const UInt32 minimalPingDelta /* = 500 */ )
{GUCEF_TRACE;

    if ( !m_isActive && ( remoteHosts.size() > 0 ) )
    {
        m_remoteHosts = remoteHosts;
        m_maxPings = maxPings;
        m_timeout = timeout;
        m_bytesToSend = bytesToSend;
        m_minimalPingDelta = minimalPingDelta;

        CPingTaskConsumer::CPingTaskData taskData( remoteHosts, maxPings, bytesToSend, timeout, minimalPingDelta );
        CORE::CFutureResult result = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool()->StartTaskWithConsumer( m_pingTaskConsumer ,
                                                                                                                             &taskData          );
        return result.HasAFuture();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPing::Stop( void )
{GUCEF_TRACE;

    if ( m_isActive )
    {
       CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool()->RequestTaskToStop( m_taskId, false );
    }
}

/*-------------------------------------------------------------------------*/

void
CPing::OnPumpedNotify( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventid  ,
                       CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( CPingTaskConsumer::PingStartedEvent == eventid )
    {
        CPingTaskConsumer::CEchoReceivedEventData* taskData = static_cast< CPingTaskConsumer::CEchoReceivedEventData* >( eventdata );
        CPingEventData eventData( taskData->GetHostAddress(), taskData->GetEchoSize(), taskData->GetRoundTripTime() );
        NotifyObservers( PingStartedEvent, &eventData );
    }
    else
    if ( CPingTaskConsumer::PingReponseEvent == eventid )
    {
        CPingTaskConsumer::CEchoReceivedEventData* taskData = static_cast< CPingTaskConsumer::CEchoReceivedEventData* >( eventdata );
        CPingEventData eventData( taskData->GetHostAddress(), taskData->GetEchoSize(), taskData->GetRoundTripTime() );
        NotifyObservers( PingReponseEvent, &eventData );
    }
    else
    if ( CPingTaskConsumer::PingTimeoutEvent == eventid )
    {
        CPingTaskConsumer::CEchoReceivedEventData* taskData = static_cast< CPingTaskConsumer::CEchoReceivedEventData* >( eventdata );
        CPingEventData eventData( taskData->GetHostAddress(), taskData->GetEchoSize(), taskData->GetRoundTripTime() );
        NotifyObservers( PingTimeoutEvent, &eventData );
    }
    else
    if ( CPingTaskConsumer::PingFailedEvent == eventid )
    {
        CPingTaskConsumer::CEchoReceivedEventData* taskData = static_cast< CPingTaskConsumer::CEchoReceivedEventData* >( eventdata );
        CPingEventData eventData( taskData->GetHostAddress(), taskData->GetEchoSize(), taskData->GetRoundTripTime() );
        NotifyObservers( PingFailedEvent, &eventData );
    }
    else
    if ( CPingTaskConsumer::PingStoppedEvent == eventid )
    {
        CPingTaskConsumer::CEchoReceivedEventData* taskData = static_cast< CPingTaskConsumer::CEchoReceivedEventData* >( eventdata );
        CPingEventData eventData( taskData->GetHostAddress(), taskData->GetEchoSize(), taskData->GetRoundTripTime() );
        NotifyObservers( PingStoppedEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

bool
CPing::IsActive( void ) const
{GUCEF_TRACE;

    return m_isActive;
}

/*-------------------------------------------------------------------------*/

const CPing::TStringVector&
CPing::GetRemoteHosts( void ) const
{GUCEF_TRACE;

    return m_remoteHosts;
}

/*-------------------------------------------------------------------------*/

Int32
CPing::GetMaxPings( void ) const
{GUCEF_TRACE;

    return m_maxPings;
}

/*-------------------------------------------------------------------------*/

void
CPing::SetUserData( void* userData )
{
    m_userData = userData;
}

/*-------------------------------------------------------------------------*/

void*
CPing::GetUserData( void ) const
{
    return m_userData;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
