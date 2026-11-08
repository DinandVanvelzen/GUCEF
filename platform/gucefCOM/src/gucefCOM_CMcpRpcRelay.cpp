/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#include "gucefCOM_CMcpRpcRelay.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpRpcRelay::CMcpRpcRelay( void )
    : m_endpoint()
    , m_ref()
    , m_stream()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpRpcRelay::~CMcpRpcRelay()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

bool
CMcpRpcRelay::Open( const COMCORE::CRpcEndpointPtr& endpoint, COMCORE::CRpcStatus& status )
{GUCEF_TRACE;

    if ( endpoint.IsNULL() )
    {
        status.Set( COMCORE::RPC_STATUS_INVALID_ARGUMENT, "no endpoint" );
        return false;
    }
    m_endpoint = endpoint;
    if ( !m_endpoint->ResolveMethod( "gucef.mcp.Session/Exchange", m_ref, status ) )
        return false;
    m_stream = m_endpoint->OpenStream( m_ref, GUCEF_NULL );
    if ( m_stream.IsNULL() )
    {
        status.Set( COMCORE::RPC_STATUS_UNAVAILABLE, "the Exchange stream did not open" );
        return false;
    }
    status = COMCORE::CRpcStatus::Ok;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRpcRelay::IsOpen( void ) const
{GUCEF_TRACE;

    return !m_stream.IsNULL() && !m_stream->IsDone();
}

/*-------------------------------------------------------------------------*/

bool
CMcpRpcRelay::SendMessage( const CORE::CString& json )
{GUCEF_TRACE;

    if ( !IsOpen() )
        return false;
    COMCORE::CRpcMessage message = m_endpoint->NewRequest( m_ref );
    message.SetString( 1, json );
    return m_stream->Write( message );
}

/*-------------------------------------------------------------------------*/

bool
CMcpRpcRelay::ReadMessage( CORE::CString& json, COMCORE::CRpcStatus& status, UInt32 timeoutMs )
{GUCEF_TRACE;

    if ( m_stream.IsNULL() )
    {
        status.Set( COMCORE::RPC_STATUS_UNAVAILABLE, "the relay is not open" );
        return false;
    }
    COMCORE::CRpcMessage message;
    if ( !m_stream->ReadNext( message, status, timeoutMs ) )
        return false;
    json = message.GetString( 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpRpcRelay::RelayInput( const char* data, UInt32 size )
{GUCEF_TRACE;

    UInt32 consumed = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '\n' != data[ i ] )
            continue;
        UInt32 end = i;
        if ( end > consumed && '\r' == data[ end-1 ] )
            --end;
        if ( end > consumed )
            SendMessage( CORE::CString( data + consumed, end - consumed ) );
        consumed = i + 1;
    }
    return consumed;
}

/*-------------------------------------------------------------------------*/

void
CMcpRpcRelay::Close( void )
{GUCEF_TRACE;

    if ( !m_stream.IsNULL() && !m_stream->IsDone() )
        m_stream->WritesDone();
    m_stream.Unlink();
    m_endpoint.Unlink();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
