/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
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

#include <string.h>

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <ifaddrs.h>
  #include <net/if.h>
  #define GUCEF_WEB_TEST_POSIX_SOCKETS
#endif

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestHttpServerBind.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

static const UInt16 BIND_TEST_BASE_PORT = 47660;

/*-------------------------------------------------------------------------*/

/** An IPv4 address of this machine that is not on the loopback interface, as dotted text. False when there is none */
static bool
FindNonLoopbackIPv4Address( CORE::CString& address )
{GUCEF_TRACE;

    #ifdef GUCEF_WEB_TEST_POSIX_SOCKETS

    struct ifaddrs* interfaces = GUCEF_NULL;
    if ( 0 != getifaddrs( &interfaces ) )
        return false;

    bool found = false;
    for ( struct ifaddrs* entry = interfaces; GUCEF_NULL != entry && !found; entry = entry->ifa_next )
    {
        if ( GUCEF_NULL == entry->ifa_addr || AF_INET != entry->ifa_addr->sa_family || 0 == ( entry->ifa_flags & IFF_UP ) )
            continue;
        const struct sockaddr_in* ipv4 = reinterpret_cast< const struct sockaddr_in* >( entry->ifa_addr );
        if ( 127 == ( ntohl( ipv4->sin_addr.s_addr ) >> 24 ) || 0 == ipv4->sin_addr.s_addr )
            continue;
        char text[ INET_ADDRSTRLEN ];
        if ( GUCEF_NULL != inet_ntop( AF_INET, &ipv4->sin_addr, text, sizeof( text ) ) )
        {
            address = text;
            found = true;
        }
    }
    freeifaddrs( interfaces );
    return found;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

/** Sends a request to an address and port with a plain TCP client and collects what comes back */
class CHttpProbe : public CORE::CObserver
{
    public:

    CHttpProbe( const CORE::CString& address, UInt16 port )
        : CORE::CObserver()
        , m_connected( false )
        , m_failed( false )
        , m_response()
        , m_client( false )
    {GUCEF_TRACE;

        SubscribeTo( &m_client );
        m_client.ConnectTo( address, port );
    }

    virtual ~CHttpProbe()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CHttpProbe";
        return classTypeName;
    }

    /** Pulses until a response started to come in or the connection failed */
    void Wait( UInt32 timeoutInMs )
    {GUCEF_TRACE;

        for ( UInt32 waited=0; waited<timeoutInMs && !m_failed && m_response.IsNULLOrEmpty(); ++waited )
        {
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            MT::ThreadDelay( 1 );
        }

        // some more time for a refused connection to show up as an answer
        for ( UInt32 extra=0; extra<50; ++extra )
        {
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            MT::ThreadDelay( 1 );
        }
    }

    bool GotHttpResponse( void ) const { return m_response.HasSubstr( "HTTP/1.", 0, true ) == 0; }

    void Close( void ) { m_client.Close(); }

    bool m_connected;
    bool m_failed;
    CORE::CString m_response;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( COMCORE::CTCPClientSocket::ConnectedEvent == eventid )
        {
            m_connected = true;
            const char* request = "GET /bind-test HTTP/1.1\r\nHost: test\r\nConnection: close\r\n\r\n";
            m_client.Send( request, (UInt32) strlen( request ) );
        }
        else
        if ( COMCORE::CTCPClientSocket::DataRecievedEvent == eventid )
        {
            COMCORE::CTCPClientSocket::TDataRecievedEventData* data = static_cast< COMCORE::CTCPClientSocket::TDataRecievedEventData* >( eventdata );
            m_response.Append( data->GetData().AsConstTypePtr< char >(), data->GetData().GetDataSize() );
        }
        else
        if ( COMCORE::CTCPClientSocket::SocketErrorEvent == eventid || COMCORE::CTCPClientSocket::DisconnectedEvent == eventid )
        {
            m_failed = true;
        }
    }

    private:

    COMCORE::CTCPClientSocket m_client;
};

/*-------------------------------------------------------------------------*/

/** Whether the HTTP server answers a request sent to the address and port */
static bool
IsServed( const CORE::CString& address, UInt16 port )
{GUCEF_TRACE;

    CHttpProbe probe( address, port );
    probe.Wait( 5000 );
    bool served = probe.m_connected && probe.GotHttpResponse();
    probe.Close();
    return served;
}

/*-------------------------------------------------------------------------*/

/** Whether a connection to the address and port is refused: no connection and no answer */
static bool
IsRefused( const CORE::CString& address, UInt16 port )
{GUCEF_TRACE;

    CHttpProbe probe( address, port );
    probe.Wait( 5000 );
    bool refused = !probe.m_connected && probe.m_failed && probe.m_response.IsNULLOrEmpty();
    probe.Close();
    return refused;
}

/*-------------------------------------------------------------------------*/

void
PerformHttpServerBindTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING HTTP server bind address TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "HttpServerBind" );

    GUCEF_TESTFW_TESTCASE( "Test: The bind address of the HTTP server settings defaults to empty, round trips through the config and copies" )
        try
        {
            WEB::CHttpServerSettings defaults;
            ASSERT_TRUE( defaults.bindAddress.IsNULLOrEmpty() );

            // the other defaults are what they were
            ASSERT_TRUE( 100 == defaults.maxClientConnections );
            ASSERT_TRUE( defaults.keepAliveConnections );
            ASSERT_FALSE( defaults.processRequestsAsync );
            ASSERT_FALSE( defaults.allowWebSocketUpgrades );

            // saved defaults do not mention the address, so a config of before and after are the same
            CORE::CDataNode defaultsNode( "HttpServer" );
            ASSERT_TRUE( defaults.SaveConfig( defaultsNode ) );
            ASSERT_TRUE( GUCEF_NULL == defaultsNode.GetAttribute( "bindAddress" ) );

            WEB::CHttpServerSettings settings;
            settings.bindAddress = "127.0.0.1";
            settings.maxClientConnections = 7;
            settings.allowWebSocketUpgrades = true;

            CORE::CDataNode node( "HttpServer" );
            ASSERT_TRUE( settings.SaveConfig( node ) );
            ASSERT_TRUE( GUCEF_NULL != node.GetAttribute( "bindAddress" ) );
            ASSERT_TRUE( "127.0.0.1" == node.GetAttributeValue( "bindAddress" ).AsString() );

            WEB::CHttpServerSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( node ) );
            ASSERT_TRUE( "127.0.0.1" == loaded.bindAddress );
            ASSERT_TRUE( 7 == loaded.maxClientConnections && loaded.allowWebSocketUpgrades );

            // a config without the key leaves the value alone
            WEB::CHttpServerSettings keeps;
            keeps.bindAddress = "0.0.0.0";
            ASSERT_TRUE( keeps.LoadConfig( defaultsNode ) );
            ASSERT_TRUE( "0.0.0.0" == keeps.bindAddress );

            // through a text format too
            CORE::CDataNode root( "Root" );
            CORE::CDataNode* child = root.AddChild( "HttpServer" );
            child->SetAttribute( "bindAddress", "127.0.0.1" );
            WEB::CHttpServerSettings fromChild;
            ASSERT_TRUE( fromChild.LoadConfig( *child ) );
            ASSERT_TRUE( "127.0.0.1" == fromChild.bindAddress );

            // copies
            WEB::CHttpServerSettings copy( settings );
            ASSERT_TRUE( "127.0.0.1" == copy.bindAddress );
            WEB::CHttpServerSettings assigned;
            assigned = settings;
            ASSERT_TRUE( "127.0.0.1" == assigned.bindAddress );
            assigned = defaults;
            ASSERT_TRUE( assigned.bindAddress.IsNULLOrEmpty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An HTTP server bound to 127.0.0.1 serves loopback clients and refuses another address of the machine, the default and 0.0.0.0 serve both" )
        try
        {
            CORE::CString otherAddress;
            bool haveOtherAddress = FindNonLoopbackIPv4Address( otherAddress );
            if ( !haveOtherAddress )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "HttpServerBind TESTS: This machine has no address besides the loopback one, the refusal on another address is not tested" );
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "HttpServerBind TESTS: Using the address " << otherAddress << " as the address besides the loopback one" );
            }

            const UInt16 port = BIND_TEST_BASE_PORT;

            // default settings: unchanged, every NIC
            {
                WEB::CHttpServerSettings settings;
                WEB::CHTTPServer server( GUCEF_NULL, &settings );
                ASSERT_TRUE( server.ListenOnPort( port ) );
                ASSERT_TRUE( IsServed( "127.0.0.1", port ) );
                if ( haveOtherAddress )
                {
                    ASSERT_TRUE( IsServed( otherAddress, port ) );
                }
                server.Close();
            }

            // no settings at all: the same
            {
                WEB::CHTTPServer server;
                ASSERT_TRUE( server.ListenOnPort( port ) );
                ASSERT_TRUE( IsServed( "127.0.0.1", port ) );
                if ( haveOtherAddress )
                {
                    ASSERT_TRUE( IsServed( otherAddress, port ) );
                }
                server.Close();
            }

            // loopback only, given at construction
            {
                WEB::CHttpServerSettings settings;
                settings.bindAddress = "127.0.0.1";
                WEB::CHTTPServer server( GUCEF_NULL, &settings );
                ASSERT_TRUE( server.ListenOnPort( port ) );
                ASSERT_TRUE( IsServed( "127.0.0.1", port ) );
                if ( haveOtherAddress )
                {
                    ASSERT_TRUE( IsRefused( otherAddress, port ) );
                }
                server.Close();
                ASSERT_TRUE( server.Listen() );
                ASSERT_TRUE( IsServed( "127.0.0.1", port ) );
                if ( haveOtherAddress )
                {
                    ASSERT_TRUE( IsRefused( otherAddress, port ) );
                }
                server.Close();
            }

            // loopback only, given with SetSettings() before listening
            {
                WEB::CHTTPServer server;
                WEB::CHttpServerSettings settings;
                server.GetSettings( settings );
                ASSERT_TRUE( settings.bindAddress.IsNULLOrEmpty() );
                settings.bindAddress = "127.0.0.1";
                server.SetSettings( settings );
                ASSERT_TRUE( server.ListenOnPort( port ) );
                ASSERT_TRUE( IsServed( "127.0.0.1", port ) );
                if ( haveOtherAddress )
                {
                    ASSERT_TRUE( IsRefused( otherAddress, port ) );
                }

                // while listening: switching to every NIC and back to loopback takes effect
                settings.bindAddress = "0.0.0.0";
                server.SetSettings( settings );
                ASSERT_TRUE( server.IsActive() );
                ASSERT_TRUE( IsServed( "127.0.0.1", port ) );
                if ( haveOtherAddress )
                {
                    ASSERT_TRUE( IsServed( otherAddress, port ) );
                }
                settings.bindAddress = "127.0.0.1";
                server.SetSettings( settings );
                ASSERT_TRUE( server.IsActive() );
                ASSERT_TRUE( IsServed( "127.0.0.1", port ) );
                if ( haveOtherAddress )
                {
                    ASSERT_TRUE( IsRefused( otherAddress, port ) );
                }
                server.Close();
            }

            // 0.0.0.0
            {
                WEB::CHttpServerSettings settings;
                settings.bindAddress = "0.0.0.0";
                WEB::CHTTPServer server( GUCEF_NULL, &settings );
                ASSERT_TRUE( server.ListenOnPort( port ) );
                ASSERT_TRUE( IsServed( "127.0.0.1", port ) );
                if ( haveOtherAddress )
                {
                    ASSERT_TRUE( IsServed( otherAddress, port ) );
                }
                server.Close();
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An HTTP server told to listen on port 0 on the loopback address reports the free port it got and serves there" )
        try
        {
            WEB::CHttpServerSettings settings;
            settings.bindAddress = "127.0.0.1";
            WEB::CHTTPServer server( GUCEF_NULL, &settings );
            ASSERT_TRUE( 0 == server.GetBoundPort() );
            ASSERT_TRUE( server.ListenOnPort( 0 ) );
            UInt16 boundPort = server.GetBoundPort();
            ASSERT_TRUE( 0 != boundPort );
            if ( 0 != boundPort )
            {
                ASSERT_TRUE( IsServed( "127.0.0.1", boundPort ) );
            }
            server.Close();
            ASSERT_TRUE( 0 == server.GetBoundPort() );

            const UInt16 port = BIND_TEST_BASE_PORT + 2;
            ASSERT_TRUE( server.ListenOnPort( port ) );
            ASSERT_TRUE( port == server.GetBoundPort() );
            server.Close();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An HTTP server with a bind address that is not a dotted IPv4 address does not listen at all" )
        try
        {
            const UInt16 port = BIND_TEST_BASE_PORT + 1;
            const char* invalid[] = { "localhost", "300.0.0.1", "1.2.3", "::1", "garbage", "127.0.0.1 " };
            for ( UInt32 i=0; i<sizeof( invalid ) / sizeof( invalid[ 0 ] ); ++i )
            {
                WEB::CHttpServerSettings settings;
                settings.bindAddress = invalid[ i ];
                WEB::CHTTPServer server( GUCEF_NULL, &settings );
                ASSERT_FALSE( server.ListenOnPort( port ) );
                ASSERT_FALSE( server.IsActive() );
                ASSERT_FALSE( server.Listen() );
                ASSERT_TRUE( IsRefused( "127.0.0.1", port ) );

                // and listens once the address is right
                settings.bindAddress = "127.0.0.1";
                server.SetSettings( settings );
                ASSERT_TRUE( server.ListenOnPort( port ) );
                ASSERT_TRUE( IsServed( "127.0.0.1", port ) );
                server.Close();
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL HTTP server bind address TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
