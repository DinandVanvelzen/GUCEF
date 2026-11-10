/*
 *  gucefAUTH_PluginsTestApp: Application for testing the gucefAUTH plugins
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "TestFakeHttp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

CFakeHttpServer::CThread::CThread( CFakeHttpServer& owner )
    : MT::CActiveObject()
    , m_owner( owner )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CFakeHttpServer::CThread::AsLockable( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CFakeHttpServer::CThread::OnThreadStart( void* taskdata )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeHttpServer::CThread::OnThreadCycle( void* taskdata )
{GUCEF_TRACE;

    m_owner.Pulse();
    return false;
}

/*-------------------------------------------------------------------------*/

CFakeHttpServer::CFakeHttpServer( void )
    : CORE::CObserver()
    , m_pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
    , m_socket( m_pulseGenerator, false )
    , m_thread( *this )
    , m_running( false )
    , m_lock()
    , m_connections()
    , m_status( 200 )
    , m_body()
    , m_contentType( "application/json" )
    , m_largeBodySize( 0 )
    , m_requestCount( 0 )
    , m_lastRequestLine()
    , m_lastRequestBody()
    , m_lastRequestText()
{GUCEF_TRACE;

    TEventCallback connectedCallback( this, &CFakeHttpServer::OnClientConnected );
    TEventCallback disconnectedCallback( this, &CFakeHttpServer::OnClientDisconnected );
    SubscribeTo( &m_socket, COMCORE::CTCPServerSocket::ClientConnectedEvent, connectedCallback );
    SubscribeTo( &m_socket, COMCORE::CTCPServerSocket::ClientDisconnectedEvent, disconnectedCallback );
}

/*-------------------------------------------------------------------------*/

CFakeHttpServer::~CFakeHttpServer()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CFakeHttpServer::Start( void )
{GUCEF_TRACE;

    if ( m_running )
        return true;
    if ( !m_socket.SetBindAddress( "127.0.0.1" ) || !m_socket.ListenOnPort( 0 ) )
        return false;
    m_running = true;
    m_thread.Activate( GUCEF_NULL, 1, 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFakeHttpServer::Stop( void )
{GUCEF_TRACE;

    if ( !m_running )
        return;
    m_running = false;
    m_thread.Deactivate( false, true );
    m_socket.Close();
}

/*-------------------------------------------------------------------------*/

UInt16
CFakeHttpServer::GetPort( void ) const
{GUCEF_TRACE;

    return m_socket.GetBoundPort();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeHttpServer::GetBaseUrl( void ) const
{GUCEF_TRACE;

    return "http://127.0.0.1:" + CORE::UInt32ToString( GetPort() );
}

/*-------------------------------------------------------------------------*/

void
CFakeHttpServer::SetResponse( int status, const CORE::CString& body, const CORE::CString& contentType )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_status = status;
    m_body = body;
    m_contentType = contentType;
    m_largeBodySize = 0;
}

/*-------------------------------------------------------------------------*/

void
CFakeHttpServer::SetLargeResponse( UInt32 bodySize )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_status = 200;
    m_largeBodySize = bodySize;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeHttpServer::GetRequestCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_requestCount;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeHttpServer::GetLastRequestLine( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastRequestLine;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeHttpServer::GetLastRequestBody( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastRequestBody;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeHttpServer::GetLastRequestText( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastRequestText;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFakeHttpServer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "CFakeHttpServer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CFakeHttpServer::Pulse( void )
{GUCEF_TRACE;

    m_pulseGenerator->ForceDirectPulse();
    MT::ThreadDelay( 1 );
}

/*-------------------------------------------------------------------------*/

void
CFakeHttpServer::OnClientConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    COMCORE::CTCPServerSocket::TClientConnectedEventData* data = static_cast< COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventData );
    COMCORE::CTCPServerConnection* connection = data->GetData().connection;
    TEventCallback dataCallback( this, &CFakeHttpServer::OnDataReceived );
    SubscribeTo( connection, COMCORE::CTCPConnection::DataRecievedEvent, dataCallback );
    SConnection& state = m_connections[ connection ];
    state = SConnection();
    state.connection = connection;
}

/*-------------------------------------------------------------------------*/

void
CFakeHttpServer::OnClientDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    COMCORE::CTCPServerSocket::TClientDisconnectedEventData* data = static_cast< COMCORE::CTCPServerSocket::TClientDisconnectedEventData* >( eventData );
    m_connections.erase( data->GetData().connectionInfo.connection );
}

/*-------------------------------------------------------------------------*/

void
CFakeHttpServer::OnDataReceived( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF::map< void*, SConnection >::iterator i = m_connections.find( notifier );
    if ( i == m_connections.end() )
        return;
    COMCORE::CTCPConnection::TDataRecievedEventData* data = static_cast< COMCORE::CTCPConnection::TDataRecievedEventData* >( eventData );
    (*i).second.received.Append( data->GetData().AsConstTypePtr< char >(), data->GetData().GetDataSize() );
    TryAnswer( (*i).second );
}

/*-------------------------------------------------------------------------*/

void
CFakeHttpServer::TryAnswer( SConnection& state )
{GUCEF_TRACE;

    if ( state.answered )
        return;

    Int32 headerEnd = state.received.HasSubstr( "\r\n\r\n", true );
    if ( headerEnd < 0 )
        return;

    CORE::CString head = state.received.SubstrFromRange( 0, (UInt32) headerEnd );
    UInt32 bodyStart = (UInt32) headerEnd + 4;

    // The body of a POST is as long as the head says
    UInt32 contentLength = 0;
    Int32 lengthAt = head.Lowercase().HasSubstr( "content-length:", true );
    if ( lengthAt >= 0 )
    {
        const CORE::CString lengthText = head.SubstrFromRange( (UInt32) lengthAt + 15, head.Length() );
        contentLength = (UInt32) strtoul( lengthText.C_String(), GUCEF_NULL, 10 );
    }
    if ( state.received.ByteSize() - 1 < bodyStart + contentLength )
        return;

    state.answered = true;
    CORE::CString body = state.received.SubstrFromRange( bodyStart, bodyStart + contentLength );
    Int32 lineEnd = head.HasSubstr( "\r\n", true );
    CORE::CString requestLine = lineEnd < 0 ? head : head.SubstrFromRange( 0, (UInt32) lineEnd );

    int status = 200;
    CORE::CString responseBody;
    CORE::CString contentType;
    {
        MT::CScopeMutex lock( m_lock );
        ++m_requestCount;
        m_lastRequestLine = requestLine;
        m_lastRequestBody = body;
        m_lastRequestText = state.received;
        status = m_status;
        responseBody = m_body;
        contentType = m_contentType;
        if ( m_largeBodySize > 0 )
        {
            // 32 bytes at a time, so at least the size that was asked for
            const CORE::CString chunk( "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" );
            responseBody.Clear();
            for ( UInt32 i=0; i<m_largeBodySize / 32 + 1; ++i )
                responseBody += chunk;
        }
    }

    CORE::CString response = "HTTP/1.1 " + CORE::Int32ToString( status ) + " X\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + CORE::UInt32ToString( responseBody.ByteSize() > 0 ? responseBody.ByteSize() - 1 : 0 ) + "\r\n";
    response += "Connection: close\r\n\r\n";
    response += responseBody;
    state.connection->Send( response.C_String(), response.ByteSize() - 1 );
    state.connection->Close();
}

/*-------------------------------------------------------------------------*/
