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

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#include "gucefDRN_CDRNNamespaceRequest.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNNamespaceRequest::CDRNNamespaceRequest( UInt32 peerId, UInt32 requestId, UInt32 kind )
    : m_lock()
    , m_signal( 0 )
    , m_peerId( peerId )
    , m_requestId( requestId )
    , m_kind( kind )
    , m_done( false )
    , m_status( DRN_RESSTATUS_TIMEOUT )
    , m_message()
    , m_body()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceRequest::GetPeerId( void ) const
{GUCEF_TRACE;

    return m_peerId;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceRequest::GetRequestId( void ) const
{GUCEF_TRACE;

    return m_requestId;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceRequest::GetKind( void ) const
{GUCEF_TRACE;

    return m_kind;
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceRequest::Complete( TDRNResourceStatus status, const CORE::CString& message, const void* body, UInt32 bodySize )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_done )
            return;
        m_status = status;
        m_message = message;
        m_body.SetDataSize( 0 );
        if ( bodySize > 0 && m_body.SetBufferSize( bodySize, false, true ) )
        {
            m_body.SetDataSize( bodySize );
            memcpy( m_body.GetBufferPtr(), body, bodySize );
        }
        m_done = true;
    }
    m_signal.Post();
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceRequest::Fail( TDRNResourceStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    Complete( status, message, GUCEF_NULL, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceRequest::IsDone( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_done;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceRequest::Wait( UInt32 timeoutInMs )
{GUCEF_TRACE;

    UInt64 deadline = CDRNClock::NowInMilliseconds() + timeoutInMs;
    while ( !IsDone() )
    {
        UInt64 now = CDRNClock::NowInMilliseconds();
        if ( now >= deadline )
            return false;
        m_signal.WaitOrTimeout( (UInt32) ( deadline - now ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNNamespaceRequest::GetStatus( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_status;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNNamespaceRequest::GetMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_message;
}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer&
CDRNNamespaceRequest::GetBody( void ) const
{GUCEF_TRACE;

    return m_body;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
