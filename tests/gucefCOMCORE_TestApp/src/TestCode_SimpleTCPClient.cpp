/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCode_Pump.h"

#include "TestCode_SimpleTCPClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::COMCORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A minimal client: connects, sends a request and collects whatever comes back
 */
class CSimpleTCPClient : public CORE::CObserver
{
    public:

    CSimpleTCPClient( const CORE::CString& remoteaddr  ,
                      CORE::UInt16 port                ,
                      const CORE::CString& information )
        : CORE::CObserver()
        , m_clientsock( false )
        , m_remoteAddr( remoteaddr )
        , m_port( port )
        , m_information( information )
        , m_connected( false )
        , m_errorCode( 0 )
        , m_received()
    {GUCEF_TRACE;

        SubscribeTo( &m_clientsock );
    }

    virtual ~CSimpleTCPClient()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CSimpleTCPClient";
        return classTypeName;
    }

    bool Start( void )
    {GUCEF_TRACE;

        return m_clientsock.ConnectTo( m_remoteAddr, m_port );
    }

    bool m_connected;
    CORE::Int32 m_errorCode;
    CORE::CString m_received;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( eventid == CTCPClientSocket::ConnectedEvent )
        {
            m_connected = true;
            m_clientsock.SendString( m_information );
        }
        else
        if ( eventid == CTCPClientSocket::DataRecievedEvent )
        {
            CTCPClientSocket::TDataRecievedEventData* data = static_cast< CTCPClientSocket::TDataRecievedEventData* >( eventdata );
            m_received.Append( data->GetData().AsConstTypePtr< char >(), data->GetData().GetDataSize() );
        }
        else
        if ( eventid == CTCPClientSocket::SocketErrorEvent )
        {
            if ( GUCEF_NULL != eventdata )
                m_errorCode = static_cast< CTCPClientSocket::TSocketErrorEventData* >( eventdata )->GetData();
        }
    }

    private:

    CTCPClientSocket m_clientsock;
    CORE::CString m_remoteAddr;
    CORE::UInt16 m_port;
    CORE::CString m_information;
};

/*-------------------------------------------------------------------------*/

/** a server which answers a request with a fixed response and closes the connection, like a basic HTTP/1.0 server */
class CAnswerOnceServer : public CORE::CObserver
{
    public:

    CAnswerOnceServer( const CORE::CString& response )
        : CORE::CObserver(), m_socket( false ), m_response( response ), m_request()
    {GUCEF_TRACE;

        SubscribeTo( &m_socket );
    }

    virtual ~CAnswerOnceServer() { UnsubscribeAllFromObserver(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CAnswerOnceServer";
        return classTypeName;
    }

    bool Start( UInt16 port ) { return m_socket.ListenOnPort( port ); }

    /** the port the server listens on, also when it was told to listen on port 0 */
    UInt16 GetPort( void ) const { return m_socket.GetBoundPort(); }

    CORE::CString m_request;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( CTCPServerSocket::ClientConnectedEvent == eventid )
        {
            CTCPServerSocket::TClientConnectedEventData* data = static_cast< CTCPServerSocket::TClientConnectedEventData* >( eventdata );
            SubscribeTo( data->GetData().connection );
        }
        else
        if ( CTCPConnection::DataRecievedEvent == eventid && notifier != &m_socket )
        {
            CTCPConnection::TDataRecievedEventData* data = static_cast< CTCPConnection::TDataRecievedEventData* >( eventdata );
            m_request.Append( data->GetData().AsConstTypePtr< char >(), data->GetData().GetDataSize() );
            CTCPServerConnection* connection = static_cast< CTCPServerConnection* >( notifier );
            connection->Send( m_response.C_String(), m_response.ByteSize() - 1 );    // ByteSize includes the null terminator
        }
    }

    private:

    CTCPServerSocket m_socket;
    CORE::CString m_response;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSimpleTCPClientTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "SimpleTCPClient" );

    GUCEF_TESTFW_TESTCASE( "Test: A simple client sends a request and receives the response of a server" )
        try
        {
            CAnswerOnceServer server( "HTTP/1.0 200 OK\r\nContent-Length: 2\r\n\r\nok" );
            ASSERT_TRUE( server.Start( 0 ) );

            CSimpleTCPClient client( "127.0.0.1", server.GetPort(), "GET /\r\n" );
            ASSERT_TRUE( client.Start() );

            ASSERT_TRUE( PumpUntil( [&client]() { return client.m_received.Length() >= 40; }, 5000 ) );
            ASSERT_TRUE( client.m_connected );
            ASSERT_TRUE( client.m_received == "HTTP/1.0 200 OK\r\nContent-Length: 2\r\n\r\nok" );
            ASSERT_TRUE( server.m_request == "GET /\r\n" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
