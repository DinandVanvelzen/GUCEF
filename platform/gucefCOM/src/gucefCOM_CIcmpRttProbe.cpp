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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include "gucefCOM_CIcmpRttProbe.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
#endif

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

const CORE::CString CIcmpRttProbe::MethodName = "icmp";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIcmpRttProbe::CIcmpRttProbe( void )
    : COMCORE::CIRttProbe()
    , m_ping( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_target()
    , m_sequence( 0 )
{GUCEF_TRACE;

    SubscribeTo( &m_ping );
}

/*-------------------------------------------------------------------------*/

CIcmpRttProbe::CIcmpRttProbe( const CORE::PulseGeneratorPtr& pulseGenerator )
    : COMCORE::CIRttProbe()
    , m_ping( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator )
    , m_target()
    , m_sequence( 0 )
{GUCEF_TRACE;

    SubscribeTo( &m_ping );
}

/*-------------------------------------------------------------------------*/

CIcmpRttProbe::~CIcmpRttProbe()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_ping.Stop();
}

/*-------------------------------------------------------------------------*/

bool
CIcmpRttProbe::IsIcmpAvailable( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // IcmpSendEcho needs no privileges
    return true;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

    // The same sockets CPing uses: an unprivileged ICMP socket, or a raw one with CAP_NET_RAW
    int sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_ICMP );
    if ( sock < 0 )
        sock = socket( AF_INET, SOCK_RAW, IPPROTO_ICMP );
    if ( sock < 0 )
        return false;
    close( sock );
    return true;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CIcmpRttProbe::IsAvailable( void ) const
{GUCEF_TRACE;

    return IsIcmpAvailable();
}

/*-------------------------------------------------------------------------*/

bool
CIcmpRttProbe::Start( const COMCORE::CHostAddress& target ,
                      UInt32 count                        ,
                      UInt32 intervalMs                   ,
                      UInt32 timeoutMs                    )
{GUCEF_TRACE;

    if ( m_ping.IsActive() || !IsIcmpAvailable() )
        return false;

    m_target = target;
    m_sequence = 0;

    CORE::CString host = target.GetIPv4Addresses().empty() ? target.GetHostname() : target.GetFirstAddressAsString();
    return m_ping.Start( host, (Int32) count, 32, timeoutMs, intervalMs );
}

/*-------------------------------------------------------------------------*/

void
CIcmpRttProbe::Stop( void )
{GUCEF_TRACE;

    m_ping.Stop();
}

/*-------------------------------------------------------------------------*/

bool
CIcmpRttProbe::IsActive( void ) const
{GUCEF_TRACE;

    return m_ping.IsActive();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIcmpRttProbe::GetMethodName( void ) const
{GUCEF_TRACE;

    return MethodName;
}

/*-------------------------------------------------------------------------*/

void
CIcmpRttProbe::OnNotify( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier != &m_ping )
        return;

    if ( COMCORE::CPing::PingReponseEvent == eventId )
    {
        const COMCORE::CPing::CPingEventData* pingData = static_cast< const COMCORE::CPing::CPingEventData* >( eventData );
        UInt64 rttInMicroSecs = GUCEF_NULL != pingData ? pingData->GetRoundTripTime() * (UInt64) 1000 : 0;
        COMCORE::CRttProbeEventData eData( m_target, rttInMicroSecs, m_sequence++ );
        NotifyObservers( ProbeResultEvent, &eData );
    }
    else
    if ( COMCORE::CPing::PingTimeoutEvent == eventId )
    {
        COMCORE::CRttProbeEventData eData( m_target, 0, m_sequence++ );
        NotifyObservers( ProbeTimeoutEvent, &eData );
    }
    else
    if ( COMCORE::CPing::PingFailedEvent == eventId )
    {
        COMCORE::CRttProbeEventData eData( m_target, 0, m_sequence++ );
        NotifyObservers( ProbeFailedEvent, &eData );
    }
    else
    if ( COMCORE::CPing::PingStoppedEvent == eventId )
    {
        COMCORE::CRttProbeEventData eData( m_target, 0, m_sequence > 0 ? m_sequence-1 : 0 );
        NotifyObservers( ProbeFinishedEvent, &eData );
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIcmpRttProbe::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CIcmpRttProbe";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
