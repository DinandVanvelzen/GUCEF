/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#include "gucefWEB_CHttpServerExchange.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHttpServerExchange::CHttpServerExchange( CHTTPServer* server                         ,
                                          UInt32 connectionIndex                      ,
                                          UInt64 exchangeId                           ,
                                          const COMCORE::CHostAddress& remoteAddress  )
    : CORE::CTSharedObjCreator< CHttpServerExchange, MT::CMutex >( this )
    , m_server( server )
    , m_connectionIndex( connectionIndex )
    , m_exchangeId( exchangeId )
    , m_remoteAddress( remoteAddress )
    , m_state( EXCHANGE_RECEIVING )
    , m_aborted( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpServerExchange::~CHttpServerExchange()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpServerExchangePtr
CHttpServerExchange::GetSharedPtr( void ) const
{GUCEF_TRACE;

    return CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

UInt64
CHttpServerExchange::GetExchangeId( void ) const
{GUCEF_TRACE;

    return m_exchangeId;
}

/*-------------------------------------------------------------------------*/

const COMCORE::CHostAddress&
CHttpServerExchange::GetRemoteAddress( void ) const
{GUCEF_TRACE;

    return m_remoteAddress;
}

/*-------------------------------------------------------------------------*/

CHttpServerExchange::EState
CHttpServerExchange::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

bool
CHttpServerExchange::IsOpen( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_server && !m_aborted && ( EXCHANGE_OPEN == m_state || EXCHANGE_STREAMING == m_state );
}

/*-------------------------------------------------------------------------*/

bool
CHttpServerExchange::IsStreaming( void ) const
{GUCEF_TRACE;

    return IsOpen() && EXCHANGE_STREAMING == m_state;
}

/*-------------------------------------------------------------------------*/

bool
CHttpServerExchange::SendResponse( const CHttpResponseData& response )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_server || m_aborted || EXCHANGE_OPEN != m_state )
        return false;
    return m_server->OnExchangeSendResponse( *this, response );
}

/*-------------------------------------------------------------------------*/

bool
CHttpServerExchange::BeginStream( const CHttpResponseData& head )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_server || m_aborted || EXCHANGE_OPEN != m_state )
        return false;
    return m_server->OnExchangeBeginStream( *this, head );
}

/*-------------------------------------------------------------------------*/

bool
CHttpServerExchange::SendStreamData( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_server || m_aborted || EXCHANGE_STREAMING != m_state )
        return false;
    if ( 0 == size )
        return true;
    if ( GUCEF_NULL == data )
        return false;
    return m_server->OnExchangeStreamData( *this, data, size );
}

/*-------------------------------------------------------------------------*/

bool
CHttpServerExchange::SendStreamData( const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    if ( 0 == data.GetDataSize() )
        return SendStreamData( GUCEF_NULL, 0 );
    return SendStreamData( data.GetConstBufferPtr(), data.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CHttpServerExchange::EndStream( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_server || m_aborted || EXCHANGE_STREAMING != m_state )
        return false;
    return m_server->OnExchangeEndStream( *this );
}

/*-------------------------------------------------------------------------*/

UInt32
CHttpServerExchange::GetStreamBacklogBytes( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_server )
        return 0;
    return m_server->GetExchangeBacklogBytes( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
