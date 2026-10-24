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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefDRN_CIDRNChannel.h"

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

const CORE::CEvent CIDRNChannel::ChannelConnectedEvent = "GUCEF::DRN::CIDRNChannel::ChannelConnectedEvent";
const CORE::CEvent CIDRNChannel::ChannelDisconnectedEvent = "GUCEF::DRN::CIDRNChannel::ChannelDisconnectedEvent";
const CORE::CEvent CIDRNChannel::PacketReceivedEvent = "GUCEF::DRN::CIDRNChannel::PacketReceivedEvent";
const CORE::CEvent CIDRNChannel::ChannelErrorEvent = "GUCEF::DRN::CIDRNChannel::ChannelErrorEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNChannelCapabilities::CDRNChannelCapabilities( void )
    : isReliable( false )
    , isDatagram( false )
    , maxPacketSize( 0 )
    , isBrowserCompatible( false )
    , canListen( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIDRNChannel::RegisterEvents( void )
{GUCEF_TRACE;

    ChannelConnectedEvent.Initialize();
    ChannelDisconnectedEvent.Initialize();
    PacketReceivedEvent.Initialize();
    ChannelErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIDRNChannel::CIDRNChannel( void )
    : CORE::CObservingNotifier()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIDRNChannel::~CIDRNChannel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIDRNLatencyMeter*
CIDRNChannel::GetLatencyMeter( void )
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CIDRNChannel::SetMaxReceivePacketSize( UInt32 /* maxPacketSize */ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIDRNChannel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CIDRNChannel";
    return classTypeName;
}

CDRNChannelReaper::CDRNChannelReaper( void )
    : CORE::CTSharedObjCreator< CDRNChannelReaper, MT::CMutex >( this )
    , m_lock()
    , m_ownerThreadId( 0 )
    , m_released()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNChannelReaper::~CDRNChannelReaper()
{GUCEF_TRACE;

    m_released.clear();
}

/*-------------------------------------------------------------------------*/

void
CDRNChannelReaper::SetOwnerThreadId( UInt32 threadId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_ownerThreadId = threadId;
}

/*-------------------------------------------------------------------------*/

void
CDRNChannelReaper::Release( CIDRNChannelPtr& channel )
{GUCEF_TRACE;

    if ( channel.IsNULL() )
        return;

    {
        MT::CScopeMutex lock( m_lock );
        if ( 0 != m_ownerThreadId && MT::GetCurrentTaskID() != m_ownerThreadId )
        {
            m_released.push_back( channel );
            channel.Unlink();
            return;
        }
    }
    channel.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CDRNChannelReaper::Reap( void )
{GUCEF_TRACE;

    TChannelVector released;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_released.empty() )
            return;
        released.swap( m_released );
    }
    released.clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
