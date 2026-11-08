/*
 *  gucefCOMCORE_TestApp: Test application for the gucefCOMCORE module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#include "TestCode_Pump.h"
#include "TestCode_TlsClient.h"
#include "TestCode_TlsServer.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** the provider of a test with a registry of its own, so that what it caches belongs to the test */
struct SServerProviderSetup
{
    CTlsProviderRegistry registry;
    COpenSslTlsProvider* provider;

    SServerProviderSetup( void ) : registry(), provider( new COpenSslTlsProvider() )
    {
        registry.RegisterProvider( TITlsProviderPtr( provider ), CTlsProviderRegistry::SystemLibraryTlsProviderPriority );
    }
};

/*-------------------------------------------------------------------------*/

static bool
Prepare( SServerProviderSetup& setup, const TESTFW::CTlsTestCerts& certs )
{
    if ( !TESTFW::CTlsTestCerts::IsToolAvailable() )
    {
        GUCEF_TESTFW_SKIP( "the openssl tool is not installed" );
    }
    CORE::CString whyNot;
    if ( !setup.provider->IsAvailable( whyNot ) )
    {
        GUCEF_TESTFW_SKIP( "no OpenSSL library on this system" );
    }
    if ( !certs.IsReady() )
    {
        GUCEF_TESTFW_SKIP( "the test certificates could not be made" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static CTlsSettings
TrustTestCa( const TESTFW::CTlsTestCerts& certs )
{
    CTlsSettings settings;
    settings.SetCaBundlePath( certs.GetPath( "ca.crt" ) );
    settings.SetUseSystemTrust( false );
    return settings;
}

/*-------------------------------------------------------------------------*/

static CTlsServerSettings
ServeWith( const TESTFW::CTlsTestCerts& certs, const char* name )
{
    CTlsServerSettings settings;
    settings.SetCertificateChainPath( certs.GetPath( ( CORE::CString( name ) + ".crt" ).C_String() ) );
    settings.SetPrivateKeyPath( certs.GetPath( ( CORE::CString( name ) + ".key" ).C_String() ) );
    return settings;
}

/*-------------------------------------------------------------------------*/

/** moves the ciphertext between the two sessions until neither has anything more for the other */
static void
Exchange( ITlsSession& client, ITlsSession& server )
{
    CORE::CDynamicBuffer buffer;
    for ( int round=0; round<32; ++round )
    {
        bool moved = false;
        buffer.SetDataSize( 0 );
        if ( client.PullCiphertext( buffer ) )
        {
            server.PushCiphertext( buffer.GetConstBufferPtr(), buffer.GetDataSize() );
            moved = true;
        }
        buffer.SetDataSize( 0 );
        if ( server.PullCiphertext( buffer ) )
        {
            client.PushCiphertext( buffer.GetConstBufferPtr(), buffer.GetDataSize() );
            moved = true;
        }
        if ( !moved )
            return;
    }
}

/*-------------------------------------------------------------------------*/

static CORE::CString
TakePlaintext( ITlsSession& session )
{
    CORE::CDynamicBuffer buffer;
    if ( !session.PullPlaintext( buffer ) )
        return CORE::CString::Empty;
    return CORE::CString( buffer.AsConstTypePtr< char >(), buffer.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/** A server on a TCP server socket: answers each piece of data with "echo:" and the data, and greets when asked */
class CEchoServer : public CORE::CObserver
{
    public:

    CTCPServerSocket m_socket;
    CORE::CString m_received;
    CORE::CString m_greeting;
    UInt32 m_connects;
    UInt32 m_disconnects;
    CTCPServerConnection* m_lastConnection;
    CTlsSessionInfo m_lastInfo;

    CEchoServer( void )
        : CORE::CObserver()
        , m_socket( false )
        , m_received()
        , m_greeting()
        , m_connects( 0 )
        , m_disconnects( 0 )
        , m_lastConnection( GUCEF_NULL )
        , m_lastInfo()
    {
        SubscribeTo( &m_socket );
    }

    virtual ~CEchoServer()
    {
        UnsubscribeAllFromObserver();
        m_socket.Close();
    }

    bool Start( void )
    {
        m_socket.SetBindAddress( "127.0.0.1" );
        return m_socket.ListenOnPort( 0 );
    }

    UInt16 GetPort( void ) const { return m_socket.GetBoundPort(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CEchoServer";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CTCPServerSocket::ClientConnectedEvent == eventId )
        {
            const CTCPServerSocket::TConnectionInfo& info = static_cast< CTCPServerSocket::TClientConnectedEventData* >( eventData )->GetData();
            ++m_connects;
            m_lastConnection = info.connection;
            SubscribeTo( info.connection );
            if ( !m_greeting.IsNULLOrEmpty() )
                info.connection->Send( m_greeting.C_String(), m_greeting.Length() );
        }
        else
        if ( CTCPServerSocket::ClientDisconnectedEvent == eventId )
        {
            ++m_disconnects;
        }
        else
        if ( CTCPConnection::DataRecievedEvent == eventId )
        {
            CTCPServerConnection* connection = static_cast< CTCPServerConnection* >( notifier );
            const CORE::CDynamicBuffer& data = static_cast< CTCPConnection::TDataRecievedEventData* >( eventData )->GetData();
            CORE::CString text( data.AsConstTypePtr< char >(), data.GetDataSize() );
            m_received += text;
            m_lastInfo = connection->GetTlsSessionInfo();
            CORE::CString answer = "echo:" + text;
            connection->Send( answer.C_String(), answer.Length() );
        }
    }
};

/*-------------------------------------------------------------------------*/

/** condition: the client received at least this text */
struct PlainHas
{
    PlainHas( const CTlsTestClient& client, const char* text ) : m_client( client ), m_text( text ) {}
    bool operator()( void ) const { return m_client.m_plain.HasSubstr( m_text ) >= 0; }
    const CTlsTestClient& m_client;
    CORE::CString m_text;
};

/*-------------------------------------------------------------------------*/

/** pulses until the condition holds */
template < typename TCondition >
static bool
PumpUntilCondition( const TCondition& condition, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs && !condition(); ++waited )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return condition();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      SESSIONS                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestSessions( const TESTFW::CTlsTestCerts& certs )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "TlsServer: the OpenSSL provider has a server side, a client session and a server session complete the handshake and exchange data" )
    {
        SServerProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            ASSERT_TRUE( 0 != ( TLSCAP_SERVER & setup.provider->GetCapabilities() ) );

            CTlsServerSettings serverSettings = ServeWith( certs, "good" );
            serverSettings.AddAlpnProtocol( "http/1.1" );
            CTlsError error;
            TTlsSessionPtr server = setup.provider->CreateServerSession( serverSettings, error );
            ASSERT_FALSE( server.IsNULL() );
            ASSERT_TRUE( server->Start() );

            CTlsSettings clientSettings = TrustTestCa( certs );
            clientSettings.AddAlpnProtocol( "http/1.1" );
            TTlsSessionPtr client = setup.provider->CreateClientSession( clientSettings, "localhost", error );
            ASSERT_FALSE( client.IsNULL() );
            ASSERT_TRUE( client->Start() );

            Exchange( *client, *server );
            ASSERT_TRUE( TLSSTATE_ESTABLISHED == client->GetState() );
            ASSERT_TRUE( TLSSTATE_ESTABLISHED == server->GetState() );
            ASSERT_TRUE( client->GetInfo().protocolVersion == "TLSv1.3" );
            ASSERT_TRUE( client->GetInfo().alpnProtocol == "http/1.1" );
            ASSERT_TRUE( server->GetInfo().alpnProtocol == "http/1.1" );
            ASSERT_TRUE( client->GetInfo().peerSubject.HasSubstr( "CN=localhost" ) >= 0 );

            client->PushPlaintext( "ping", 4 );
            Exchange( *client, *server );
            ASSERT_TRUE( TakePlaintext( *server ) == "ping" );

            server->PushPlaintext( "pong", 4 );
            Exchange( *client, *server );
            ASSERT_TRUE( TakePlaintext( *client ) == "pong" );

            client->InitiateClose();
            Exchange( *client, *server );
            ASSERT_TRUE( TLSSTATE_CLOSED == server->GetState() );
            ASSERT_TRUE( server->GetInfo().peerClosedCleanly );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsServer: the maximum version of the server is kept, a client that needs a newer one is refused" )
    {
        SServerProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            CTlsServerSettings serverSettings = ServeWith( certs, "good" );
            serverSettings.SetMaxVersion( TLSVERSION_1_2 );
            CTlsError error;
            TTlsSessionPtr server = setup.provider->CreateServerSession( serverSettings, error );
            TTlsSessionPtr client = setup.provider->CreateClientSession( TrustTestCa( certs ), "localhost", error );
            ASSERT_FALSE( server.IsNULL() || client.IsNULL() );
            server->Start();
            client->Start();
            Exchange( *client, *server );
            ASSERT_TRUE( TLSSTATE_ESTABLISHED == client->GetState() );
            ASSERT_TRUE( client->GetInfo().protocolVersion == "TLSv1.2" );

            CTlsSettings newOnly = TrustTestCa( certs );
            newOnly.SetMinVersion( TLSVERSION_1_3 );
            TTlsSessionPtr server2 = setup.provider->CreateServerSession( serverSettings, error );
            TTlsSessionPtr client2 = setup.provider->CreateClientSession( newOnly, "localhost", error );
            ASSERT_FALSE( server2.IsNULL() || client2.IsNULL() );
            server2->Start();
            client2->Start();
            Exchange( *client2, *server2 );
            ASSERT_TRUE( TLSSTATE_FAILED == client2->GetState() );
            ASSERT_TRUE( TLSSTATE_FAILED == server2->GetState() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsServer: ALPN: a client that offers none is served, one that offers only protocols the server does not speak is refused" )
    {
        SServerProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            CTlsServerSettings serverSettings = ServeWith( certs, "good" );
            serverSettings.AddAlpnProtocol( "mcp-test" );
            serverSettings.AddAlpnProtocol( "http/1.1" );
            CTlsError error;

            TTlsSessionPtr server = setup.provider->CreateServerSession( serverSettings, error );
            TTlsSessionPtr client = setup.provider->CreateClientSession( TrustTestCa( certs ), "localhost", error );
            server->Start();
            client->Start();
            Exchange( *client, *server );
            ASSERT_TRUE( TLSSTATE_ESTABLISHED == server->GetState() );
            ASSERT_TRUE( server->GetInfo().alpnProtocol.IsNULLOrEmpty() );

            // the order of the server decides
            CTlsSettings both = TrustTestCa( certs );
            both.AddAlpnProtocol( "http/1.1" );
            both.AddAlpnProtocol( "mcp-test" );
            TTlsSessionPtr server2 = setup.provider->CreateServerSession( serverSettings, error );
            TTlsSessionPtr client2 = setup.provider->CreateClientSession( both, "localhost", error );
            server2->Start();
            client2->Start();
            Exchange( *client2, *server2 );
            ASSERT_TRUE( client2->GetInfo().alpnProtocol == "mcp-test" );

            CTlsSettings other = TrustTestCa( certs );
            other.AddAlpnProtocol( "h2" );
            TTlsSessionPtr server3 = setup.provider->CreateServerSession( serverSettings, error );
            TTlsSessionPtr client3 = setup.provider->CreateClientSession( other, "localhost", error );
            server3->Start();
            client3->Start();
            Exchange( *client3, *server3 );
            ASSERT_TRUE( TLSSTATE_FAILED == server3->GetState() );
            ASSERT_TRUE( TLSSTATE_FAILED == client3->GetState() );
            ASSERT_TRUE( TLSERR_PROTOCOL_ALERT == client3->GetError().errorClass );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsServer: a certificate or key that cannot be used is a settings error, no session is made" )
    {
        SServerProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            CTlsError error;
            CTlsServerSettings none;
            ASSERT_FALSE( none.IsEnabled() );
            ASSERT_TRUE( setup.provider->CreateServerSession( none, error ).IsNULL() );
            ASSERT_TRUE( TLSERR_SETTINGS == error.errorClass );

            CTlsServerSettings missing = ServeWith( certs, "good" );
            missing.SetCertificateChainPath( certs.GetPath( "nothere.crt" ) );
            ASSERT_TRUE( setup.provider->CreateServerSession( missing, error ).IsNULL() );
            ASSERT_TRUE( TLSERR_SETTINGS == error.errorClass );
            ASSERT_TRUE( error.message.HasSubstr( "nothere.crt" ) >= 0 );

            CTlsServerSettings mismatch = ServeWith( certs, "good" );
            mismatch.SetPrivateKeyPath( certs.GetPath( "wrongname.key" ) );
            ASSERT_TRUE( setup.provider->CreateServerSession( mismatch, error ).IsNULL() );
            ASSERT_TRUE( TLSERR_SETTINGS == error.errorClass );

            CTlsServerSettings notPem = ServeWith( certs, "good" );
            notPem.SetCertificateChainPath( certs.GetPath( "good.ext" ) );
            ASSERT_TRUE( setup.provider->CreateServerSession( notPem, error ).IsNULL() );
            ASSERT_TRUE( TLSERR_SETTINGS == error.errorClass );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsServer: the settings round trip through configuration, and no version below TLS 1.2 can be configured" )
    {
        try
        {
            CTlsServerSettings settings;
            settings.SetCertificateChainPath( "/etc/x/server.crt" );
            settings.SetPrivateKeyPath( "/etc/x/server.key" );
            settings.SetMinVersion( TLSVERSION_1_3 );
            settings.AddAlpnProtocol( "http/1.1" );
            settings.SetHandshakeTimeoutInMs( 1234 );

            CORE::CDataNode config;
            ASSERT_TRUE( settings.SaveConfig( config ) );
            CTlsServerSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( config ) );
            ASSERT_TRUE( loaded.IsEnabled() );
            ASSERT_TRUE( loaded.GetContextKey() == settings.GetContextKey() );
            ASSERT_TRUE( loaded.GetHandshakeTimeoutInMs() == 1234 );
            ASSERT_TRUE( TLSVERSION_1_3 == loaded.GetMinVersion() );

            CORE::CDataNode old;
            old.SetAttribute( "minVersion", "1.0" );
            CTlsServerSettings floor;
            floor.LoadConfig( old );
            ASSERT_TRUE( TLSVERSION_1_2 == floor.GetMinVersion() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TCP SERVER SOCKET                                                  //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestServerSocket( const TESTFW::CTlsTestCerts& certs )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "TlsServer: a TCP server socket with TLS settings serves a TLS client, its users see plaintext only" )
    {
        SServerProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            CEchoServer server;
            ASSERT_TRUE( server.m_socket.SetTlsSettings( ServeWith( certs, "good" ) ) );
            ASSERT_TRUE( server.m_socket.IsTlsEnabled() );
            ASSERT_TRUE( server.Start() );

            CTlsTestClient client;
            CTlsSettings settings = TrustTestCa( certs );
            client.m_sendOnConnected = "hello";
            ASSERT_TRUE( client.Connection()->ConnectTo( "127.0.0.1", server.GetPort(), &settings ) );
            ASSERT_TRUE( PumpUntilCondition( PlainHas( client, "echo:hello" ), 10000 ) );
            ASSERT_TRUE( server.m_received == "hello" );
            ASSERT_TRUE( client.Connection()->GetSessionInfo().peerSubject.HasSubstr( "CN=localhost" ) >= 0 );
            ASSERT_TRUE( server.m_lastInfo.protocolVersion == "TLSv1.3" );

            // a second piece of data on the same connection
            client.Connection()->Send( "again", 5 );
            ASSERT_TRUE( PumpUntilCondition( PlainHas( client, "echo:again" ), 10000 ) );

            // the server closes: the client hears a proper close
            server.m_lastConnection->Close();
            ASSERT_TRUE( PumpUntilTrue( client.m_disconnected, 10000 ) );
            ASSERT_TRUE( client.Connection()->GetSessionInfo().peerClosedCleanly );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsServer: what a server sends when the client connected arrives after the handshake" )
    {
        SServerProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            CEchoServer server;
            server.m_greeting = "220 ready\r\n";
            ASSERT_TRUE( server.m_socket.SetTlsSettings( ServeWith( certs, "good" ) ) );
            ASSERT_TRUE( server.Start() );

            CTlsTestClient client;
            CTlsSettings settings = TrustTestCa( certs );
            ASSERT_TRUE( client.Connection()->ConnectTo( "localhost", server.GetPort(), &settings ) );
            ASSERT_TRUE( PumpUntilCondition( PlainHas( client, "220 ready" ), 10000 ) );
            ASSERT_TRUE( client.m_plain.StartsWith( "220 ready\r\n" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsServer: a client that speaks plain text to a TLS port is disconnected and none of its text is delivered" )
    {
        SServerProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            CEchoServer server;
            ASSERT_TRUE( server.m_socket.SetTlsSettings( ServeWith( certs, "good" ) ) );
            ASSERT_TRUE( server.Start() );

            CTlsTestClient client;
            client.m_sendOnConnected = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
            ASSERT_TRUE( client.Connection()->ConnectTo( "127.0.0.1", server.GetPort() ) );
            ASSERT_TRUE( PumpUntilTrue( client.m_disconnected, 10000 ) );
            ASSERT_TRUE( server.m_received.IsNULLOrEmpty() );
            ASSERT_TRUE( client.m_plain.HasSubstr( "echo:" ) < 0 );
            ASSERT_TRUE( PumpUntilAtLeast( server.m_disconnects, 1, 5000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsServer: a client that does not complete the handshake in time is disconnected" )
    {
        SServerProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            CEchoServer server;
            CTlsServerSettings settings = ServeWith( certs, "good" );
            settings.SetHandshakeTimeoutInMs( 300 );
            ASSERT_TRUE( server.m_socket.SetTlsSettings( settings ) );
            ASSERT_TRUE( server.Start() );

            // connects and says nothing
            CTlsTestClient client;
            ASSERT_TRUE( client.Connection()->ConnectTo( "127.0.0.1", server.GetPort() ) );
            ASSERT_TRUE( PumpUntilTrue( client.m_connected, 5000 ) );
            ASSERT_TRUE( PumpUntilTrue( client.m_disconnected, 5000 ) );
            ASSERT_TRUE( server.m_received.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsServer: TLS that cannot be used is refused up front, the socket does not listen without it" )
    {
        SServerProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            CEchoServer server;
            CTlsServerSettings settings = ServeWith( certs, "good" );
            settings.SetPrivateKeyPath( certs.GetPath( "nothere.key" ) );
            ASSERT_FALSE( server.m_socket.SetTlsSettings( settings ) );
            ASSERT_FALSE( server.m_socket.IsTlsEnabled() );
            ASSERT_FALSE( server.Start() );
            ASSERT_FALSE( server.m_socket.IsActive() );

            // settings without a certificate turn TLS off again
            ASSERT_TRUE( server.m_socket.SetTlsSettings( CTlsServerSettings() ) );
            ASSERT_TRUE( server.Start() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsServer: a renewed certificate is used for new connections without a restart" )
    {
        SServerProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            CORE::CString certFile = certs.GetPath( "served.crt" );
            CORE::CString keyFile = certs.GetPath( "served.key" );
            ASSERT_TRUE( CORE::CopyFile( certs.GetPath( "good.crt" ), certFile, true ) );
            ASSERT_TRUE( CORE::CopyFile( certs.GetPath( "good.key" ), keyFile, true ) );

            CTlsServerSettings settings;
            settings.SetCertificateChainPath( certFile );
            settings.SetPrivateKeyPath( keyFile );

            CEchoServer server;
            ASSERT_TRUE( server.m_socket.SetTlsSettings( settings ) );
            ASSERT_TRUE( server.Start() );

            CTlsSettings trust = TrustTestCa( certs );
            {
                CTlsTestClient client;
                client.m_sendOnConnected = "one";
                ASSERT_TRUE( client.Connection()->ConnectTo( "localhost", server.GetPort(), &trust ) );
                ASSERT_TRUE( PumpUntilCondition( PlainHas( client, "echo:one" ), 10000 ) );
                ASSERT_TRUE( client.Connection()->GetSessionInfo().peerSubject.HasSubstr( "CN=localhost" ) >= 0 );
            }

            // the renewal: another certificate for the same names
            MT::ThreadDelay( 20 );
            ASSERT_TRUE( CORE::CopyFile( certs.GetPath( "named.crt" ), certFile, true ) );
            ASSERT_TRUE( CORE::CopyFile( certs.GetPath( "named.key" ), keyFile, true ) );

            CTlsTestClient client;
            client.m_sendOnConnected = "two";
            ASSERT_TRUE( client.Connection()->ConnectTo( "localhost", server.GetPort(), &trust ) );
            ASSERT_TRUE( PumpUntilCondition( PlainHas( client, "echo:two" ), 10000 ) );
            ASSERT_TRUE( client.Connection()->GetSessionInfo().peerSubject.HasSubstr( "CN=tls-target.test" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

}

/*-------------------------------------------------------------------------*/

void
PerformTlsServerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "TlsServer" );

    TESTFW::CTlsTestCerts certs;
    TestSessions( certs );
    TestServerSocket( certs );
}

/*-------------------------------------------------------------------------*/
