/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CRpcTransportProfile.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcTransportProfile::CRpcTransportProfile( void )
    : name( "tcp" )
    , maxFrameSize( 16384 )
    , ackDelayMs( 50 )
    , ackEveryFrames( 32 )
    , initialRtoMs( 1000 )
    , minRtoMs( 200 )
    , maxRtoMs( 60000 )
    , dupThreshold( 3 )
    , maxBurstFrames( 16 )
    , keepaliveIntervalMs( 15000 )
    , keepaliveTimeoutMs( 45000 )
    , resumeGraceMs( 600000 )
    , helloRetryMs( 1000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcTransportProfile
CRpcTransportProfile::GetProfile( const CORE::CString& profileName )
{GUCEF_TRACE;

    CRpcTransportProfile p;
    p.name = profileName;
    if ( "loopback" == profileName )
    {
        p.maxFrameSize = 1024 * 1024;
        p.ackDelayMs = 0;
        p.keepaliveIntervalMs = 0;
        p.keepaliveTimeoutMs = 0;
        p.resumeGraceMs = 0;
    }
    else if ( "drn" == profileName )
    {
        p.ackDelayMs = 100;
        p.keepaliveIntervalMs = 30000;
        p.keepaliveTimeoutMs = 90000;
    }
    else if ( "websocket" == profileName )
    {
        p.ackDelayMs = 50;
    }
    else if ( "http" == profileName )
    {
        p.maxFrameSize = 64 * 1024;
        p.keepaliveIntervalMs = 25000;
        p.keepaliveTimeoutMs = 75000;
    }
    else if ( "udp" == profileName )
    {
        p.maxFrameSize = 1200;
        p.ackDelayMs = 10;
        p.initialRtoMs = 200;
        p.minRtoMs = 200;
        p.maxRtoMs = 10000;
        p.keepaliveIntervalMs = 5000;
        p.keepaliveTimeoutMs = 15000;
        p.resumeGraceMs = 120000;
        p.helloRetryMs = 500;
    }
    else if ( "mail" == profileName )
    {
        p.maxFrameSize = 1024 * 1024;
        p.ackDelayMs = 5 * 60 * 1000;
        p.initialRtoMs = 30 * 60 * 1000;
        p.minRtoMs = 30 * 60 * 1000;
        p.maxRtoMs = 24 * 60 * 60 * 1000;
        p.keepaliveIntervalMs = 0;
        p.keepaliveTimeoutMs = 0;
        p.resumeGraceMs = 7 * 24 * 60 * 60 * 1000;
        p.helloRetryMs = 30 * 60 * 1000;
    }
    else if ( "quic" == profileName )
    {
        p.ackDelayMs = 25;
    }
    else
    {
        p.name = "tcp" == profileName ? profileName : CORE::CString( "tcp" );
    }
    return p;
}

/*-------------------------------------------------------------------------*/

static UInt32
ScaleTime( UInt32 value, UInt32 divisor )
{
    if ( 0 == value || divisor <= 1 )
        return value;
    UInt32 scaled = value / divisor;
    return scaled > 0 ? scaled : 1;
}

/*-------------------------------------------------------------------------*/

CRpcTransportProfile
CRpcTransportProfile::Scaled( UInt32 divisor ) const
{GUCEF_TRACE;

    CRpcTransportProfile p( *this );
    p.ackDelayMs = ScaleTime( ackDelayMs, divisor );
    p.initialRtoMs = ScaleTime( initialRtoMs, divisor );
    p.minRtoMs = ScaleTime( minRtoMs, divisor );
    p.maxRtoMs = ScaleTime( maxRtoMs, divisor );
    p.keepaliveIntervalMs = ScaleTime( keepaliveIntervalMs, divisor );
    p.keepaliveTimeoutMs = ScaleTime( keepaliveTimeoutMs, divisor );
    p.resumeGraceMs = ScaleTime( resumeGraceMs, divisor );
    p.helloRetryMs = ScaleTime( helloRetryMs, divisor );
    return p;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
