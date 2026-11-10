/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the COMCORE module
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

#include <stdio.h>
#include <time.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#include "TestCode_Pump.h"
#include "TestCode_TlsClient.h"

#include "TestCode_TlsOpenSsl.h"

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

static const char* HttpRequest = "GET / HTTP/1.0\r\n\r\n";

/** the provider of a test with a registry of its own, so that what it caches belongs to the test */
struct SProviderSetup
{
    CTlsProviderRegistry registry;
    COpenSslTlsProvider* provider;

    SProviderSetup( void ) : registry(), provider( new COpenSslTlsProvider() )
    {
        registry.RegisterProvider( TITlsProviderPtr( provider ), CTlsProviderRegistry::SystemLibraryTlsProviderPriority );
    }
};

/** settings that trust the test authority and only that */
static CTlsSettings
TrustTestCa( const TESTFW::CTlsTestCerts& certs, const char* ca = "ca" )
{
    CTlsSettings settings;
    settings.SetCaBundlePath( certs.GetPath( ( CORE::CString( ca ) + ".crt" ).C_String() ) );
    settings.SetUseSystemTrust( false );
    return settings;
}

/** connects, sends a request when connected and waits until it is over, one way or the other */
static void
RunClient( SProviderSetup& setup, CTlsTestClient& client, const CTlsSettings& settings, const CORE::CString& host, UInt16 port, const char* request = HttpRequest )
{
    client.Connection()->SetProviderRegistry( &setup.registry );
    client.m_sendOnConnected = request;
    if ( !client.Connection()->ConnectTo( host, port, &settings ) )
        return;
    PumpUntilTrue( client.m_disconnected, 10000 );
}

/** the class of the failure, TLSERR_NONE when there was none */
static ETlsErrorClass
FailureOf( const CTlsTestClient& client )
{
    return client.m_gotTlsError ? client.m_tlsErrorClass : TLSERR_NONE;
}

/*-------------------------------------------------------------------------*/

/** the bytes of the stream as the OS gives them are cut up as the test wants: this drives a session directly, over a plain socket */
class CRawSessionClient : public CORE::CObserver
{
    public:

    CRawSessionClient( ITlsProvider& provider, const CTlsSettings& settings, const CORE::CString& serverName, UInt32 bytesPerPush, UInt32 maxRead )
        : CORE::CObserver(), m_provider( provider ), m_settings( settings ), m_serverName( serverName ), m_bytesPerPush( bytesPerPush )
        , m_socket( false ), m_session(), m_error(), m_requestSent( false ), m_established( false ), m_failed( false ), m_closed( false )
        , m_plain(), m_plainBytes( 0 ), m_cipherBytes( 0 ), m_out(), m_plainIn(), m_request( "GET /big.bin HTTP/1.0\r\n\r\n" )
    {
        SubscribeTo( &m_socket );
        m_socket.SetMaxRead( maxRead );
    }

    virtual ~CRawSessionClient() { UnsubscribeAllFromObserver(); m_socket.Close(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CRawSessionClient";
        return classTypeName;
    }

    bool Start( const CORE::CString& host, UInt16 port ) { return m_socket.ConnectTo( host, port ); }

    void SetRequest( const CORE::CString& request ) { m_request = request; }

    CORE::CString m_plain;      /**< the first part of what was received, for checks */
    UInt64 m_plainBytes;
    UInt64 m_cipherBytes;
    bool m_established;
    bool m_failed;
    bool m_closed;
    CTlsError m_error;
    TTlsSessionPtr m_session;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( notifier != &m_socket )
            return;

        if ( CTCPConnection::ConnectedEvent == eventId )
        {
            m_session = m_provider.CreateClientSession( m_settings, m_serverName, m_error );
            if ( m_session.IsNULL() || !m_session->Start() )
            {
                m_failed = true;
                return;
            }
            Flush();
        }
        else
        if ( CTCPConnection::DataRecievedEvent == eventId && !m_session.IsNULL() && !m_failed )
        {
            CTCPConnection::TDataRecievedEventData* data = static_cast< CTCPConnection::TDataRecievedEventData* >( eventData );
            const char* bytes = data->GetData().AsConstTypePtr< char >();
            UInt32 size = data->GetData().GetDataSize();
            m_cipherBytes += size;

            UInt32 step = 0 == m_bytesPerPush ? size : m_bytesPerPush;
            for ( UInt32 offset=0; offset<size && !m_failed; offset += step )
            {
                UInt32 take = size - offset < step ? size - offset : step;
                m_session->PushCiphertext( bytes + offset, take );
                Pull();
            }
        }
        else
        if ( CTCPConnection::DisconnectedEvent == eventId )
            m_closed = true;
    }

    private:

    void Flush( void )
    {
        m_out.SetDataSize( 0 );
        if ( m_session->PullCiphertext( m_out ) )
            m_socket.Send( m_out.GetConstBufferPtr(), m_out.GetDataSize() );
    }

    void Pull( void )
    {
        Flush();
        if ( TLSSTATE_FAILED == m_session->GetState() )
        {
            m_failed = true;
            m_error = m_session->GetError();
            m_socket.Close();
            return;
        }
        if ( TLSSTATE_ESTABLISHED == m_session->GetState() )
        {
            m_established = true;
            if ( !m_requestSent )
            {
                m_requestSent = true;
                m_session->PushPlaintext( m_request.C_String(), m_request.Length() );
                Flush();
            }
        }
        m_plainIn.SetDataSize( 0 );
        if ( m_session->PullPlaintext( m_plainIn ) )
        {
            m_plainBytes += m_plainIn.GetDataSize();
            if ( m_plain.Length() < 4096 )
                m_plain.Append( m_plainIn.AsConstTypePtr< char >(), m_plainIn.GetDataSize() > 4096 ? 4096 : m_plainIn.GetDataSize() );
        }
        if ( TLSSTATE_CLOSED == m_session->GetState() )
        {
            // the peer is done: we answer in kind and close
            m_session->InitiateClose();
            Flush();
            m_socket.Close();
        }
    }

    private:

    ITlsProvider& m_provider;
    CTlsSettings m_settings;
    CORE::CString m_serverName;
    UInt32 m_bytesPerPush;
    CTCPClientSocket m_socket;
    bool m_requestSent;
    CORE::CDynamicBuffer m_out;
    CORE::CDynamicBuffer m_plainIn;
    CORE::CString m_request;
};

/*-------------------------------------------------------------------------*/

/** a plain server on a port: answers the first bytes it gets with a fixed reply, or closes without a word */
class CPlainTcpPeer : public CORE::CObserver
{
    public:

    CPlainTcpPeer( const CORE::CString& reply, bool closeOnData )
        : CORE::CObserver(), m_socket( false ), m_reply( reply ), m_closeOnData( closeOnData ), m_dataEvents( 0 )
    {
        SubscribeTo( &m_socket );
    }

    virtual ~CPlainTcpPeer() { UnsubscribeAllFromObserver(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CPlainTcpPeer";
        return classTypeName;
    }

    bool Start( void ) { return m_socket.ListenOnPort( 0 ); }

    UInt16 GetPort( void ) const { return m_socket.GetBoundPort(); }

    UInt32 m_dataEvents;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CTCPServerSocket::ClientConnectedEvent == eventId )
        {
            CTCPServerSocket::TClientConnectedEventData* data = static_cast< CTCPServerSocket::TClientConnectedEventData* >( eventData );
            SubscribeTo( data->GetData().connection );
        }
        else
        if ( CTCPConnection::DataRecievedEvent == eventId && notifier != &m_socket )
        {
            ++m_dataEvents;
            CTCPServerConnection* connection = static_cast< CTCPServerConnection* >( notifier );
            if ( m_closeOnData )
                connection->Close();
            else
            if ( 1 == m_dataEvents )
                connection->Send( m_reply.C_String(), m_reply.Length() );
        }
    }

    private:

    CTCPServerSocket m_socket;
    CORE::CString m_reply;
    bool m_closeOnData;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE LOADER                                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestLoader( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: libraries that are absent make it unavailable with the reason of the loader, and nothing else breaks" )
    try
    {
        COpenSslTlsProvider::TLibraryCandidateVector candidates;
        COpenSslTlsProvider::SLibraryCandidate missing;
        missing.sslName = "libssl.so.99";
        missing.cryptoName = "libcrypto.so.99";
        candidates.push_back( missing );
        COpenSslTlsProvider provider( candidates );

        CORE::CString whyNot;
        ASSERT_FALSE( provider.IsAvailable( whyNot ) );
        ASSERT_TRUE( whyNot.HasSubstr( "libcrypto.so.99" ) >= 0 );
        ASSERT_TRUE( whyNot.HasSubstr( "cannot open shared object file" ) >= 0 );
        ASSERT_TRUE( provider.GetVersionInfo().IsNULLOrEmpty() );

        // asked again it says the same, without trying again
        CORE::CString again;
        ASSERT_FALSE( provider.IsAvailable( again ) );
        ASSERT_TRUE( again == whyNot );

        CTlsError error;
        CTlsSettings settings;
        ASSERT_TRUE( provider.CreateClientSession( settings, "example.com", error ).IsNULL() );
        ASSERT_TRUE( error.errorClass == TLSERR_NO_PROVIDER );
        ASSERT_TRUE( error.message.HasSubstr( "libcrypto.so.99" ) >= 0 );

        // a connection that is told to use it fails with that, and the registry says why
        CTlsProviderRegistry registry;
        registry.RegisterProvider( TITlsProviderPtr( new COpenSslTlsProvider( candidates ) ), CTlsProviderRegistry::SystemLibraryTlsProviderPriority );
        CTlsClientConnection connection( false );
        connection.SetProviderRegistry( &registry );
        ASSERT_FALSE( connection.ConnectTo( "localhost", 1, &settings ) );
        ASSERT_TRUE( connection.GetLastError().errorClass == TLSERR_NO_PROVIDER );
        ASSERT_TRUE( connection.GetLastError().message.HasSubstr( "openssl-dynamic: libcrypto.so.99" ) >= 0 );
        ASSERT_TRUE( connection.GetLastError().GetDescription( "example.com" ).HasSubstr( "libcrypto.so.99" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: a library that is there but is not OpenSSL says which entry points it lacks, the next candidate is tried" )
    try
    {
        COpenSslTlsProvider::TLibraryCandidateVector candidates;
        COpenSslTlsProvider::SLibraryCandidate notOpenSsl;
        notOpenSsl.sslName = "libm.so.6";
        notOpenSsl.cryptoName = "libm.so.6";
        candidates.push_back( notOpenSsl );
        COpenSslTlsProvider provider( candidates );

        CORE::CString whyNot;
        ASSERT_FALSE( provider.IsAvailable( whyNot ) );
        ASSERT_TRUE( whyNot.HasSubstr( "libm.so.6: the library lacks" ) >= 0 );
        ASSERT_TRUE( whyNot.HasSubstr( "OpenSSL_version_num" ) >= 0 );

        // the default candidates after a bad one: the one that works is used when there is one
        COpenSslTlsProvider::TLibraryCandidateVector withFallback;
        withFallback.push_back( notOpenSsl );
        COpenSslTlsProvider::TLibraryCandidateVector defaults;
        COpenSslTlsProvider::GetDefaultLibraryCandidates( defaults );
        ASSERT_TRUE( 3 == defaults.size() );
        ASSERT_TRUE( defaults[ 0 ].sslName == "libssl.so.3" && defaults[ 0 ].cryptoName == "libcrypto.so.3" );
        ASSERT_TRUE( defaults[ 1 ].sslName == "libssl.so.1.1" && defaults[ 1 ].cryptoName == "libcrypto.so.1.1" );
        ASSERT_TRUE( defaults[ 2 ].sslName == "libssl.so" && defaults[ 2 ].cryptoName == "libcrypto.so" );
        withFallback.insert( withFallback.end(), defaults.begin(), defaults.end() );
        COpenSslTlsProvider second( withFallback );
        COpenSslTlsProvider reference;
        CORE::CString referenceWhyNot;
        ASSERT_TRUE( second.IsAvailable( whyNot ) == reference.IsAvailable( referenceWhyNot ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: a table of entry points is only used for OpenSSL 1.1.1 and newer" )
    try
    {
        struct SFake
        {
            static unsigned long V110( void ) { return 0x1010000fUL; }
            static unsigned long V111( void ) { return 0x1010100fUL; }
            static unsigned long V3( void ) { return 0x30500050UL; }
            static const char* Text( int ) { return "OpenSSL fake"; }
        };

        SOpenSslApi api;
        CORE::CString whyNot;
        ASSERT_FALSE( COpenSslTlsProvider::CheckApi( api, whyNot ) );           // nothing there at all
        ASSERT_FALSE( whyNot.IsNULLOrEmpty() );

        api.OpenSSL_version = &SFake::Text;
        api.OpenSSL_version_num = &SFake::V110;
        ASSERT_FALSE( COpenSslTlsProvider::CheckApi( api, whyNot ) );
        ASSERT_TRUE( whyNot.HasSubstr( "too old" ) >= 0 );
        ASSERT_TRUE( whyNot.HasSubstr( "OpenSSL fake" ) >= 0 );

        api.OpenSSL_version_num = &SFake::V111;
        ASSERT_TRUE( COpenSslTlsProvider::CheckApi( api, whyNot ) );
        api.OpenSSL_version_num = &SFake::V3;
        ASSERT_TRUE( COpenSslTlsProvider::CheckApi( api, whyNot ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      AGAINST s_server                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
Prepare( SProviderSetup& setup, const TESTFW::CTlsTestCerts& certs )
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

static CORE::CProcess::TStringVector
Args( const char* a1 = GUCEF_NULL, const char* a2 = GUCEF_NULL, const char* a3 = GUCEF_NULL, const char* a4 = GUCEF_NULL )
{
    CORE::CProcess::TStringVector args;
    if ( GUCEF_NULL != a1 ) args.push_back( a1 );
    if ( GUCEF_NULL != a2 ) args.push_back( a2 );
    if ( GUCEF_NULL != a3 ) args.push_back( a3 );
    if ( GUCEF_NULL != a4 ) args.push_back( a4 );
    return args;
}

static void
TestTrustAndNames( const TESTFW::CTlsTestCerts& certs )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: a server whose certificate is signed by the trusted authority is reached, negotiated parameters are reported" )
    {
        SProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            TESTFW::CTlsTestServer server;
            ASSERT_TRUE( server.Start( certs, "good", TESTFW::CTlsTestServer::SERVE_STATUS_PAGE, Args( "-alpn", "http/1.1" ) ) );

            CTlsSettings settings = TrustTestCa( certs );
            settings.AddAlpnProtocol( "http/1.1" );
            CTlsTestClient client;
            RunClient( setup, client, settings, "localhost", server.GetPort() );

            ASSERT_TRUE( TLSERR_NONE == FailureOf( client ) );
            ASSERT_TRUE( client.m_events.StartsWith( "Connecting,HandshakeCompleted,Connected," ) );
            ASSERT_TRUE( client.m_plain.StartsWith( "HTTP/1.0 200 ok" ) );
            const CTlsSessionInfo& info = client.Connection()->GetSessionInfo();
            ASSERT_TRUE( info.protocolVersion == "TLSv1.3" );
            ASSERT_FALSE( info.cipherName.IsNULLOrEmpty() );
            ASSERT_TRUE( info.alpnProtocol == "http/1.1" );
            ASSERT_TRUE( info.peerSubject.HasSubstr( "CN=localhost" ) >= 0 );
            ASSERT_TRUE( info.providerName == "openssl-dynamic" );
            ASSERT_TRUE( info.providerVersion.StartsWith( "OpenSSL " ) );
            ASSERT_FALSE( info.resumed );
            ASSERT_TRUE( info.peerClosedCleanly );             // the server said goodbye properly
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    static const struct
    {
        const char* what;
        const char* cert;
        const char* ca;
        bool systemTrust;
        const char* host;
        ETlsErrorClass expected;
        Int32 verifyCode;
    } cases[] =
    {
        { "an authority nobody trusts, only the system store is used", "good", GUCEF_NULL, true, "localhost", TLSERR_CERT_UNTRUSTED, 20 },
        { "another authority is trusted than the one that signed it", "good", "other_ca", false, "localhost", TLSERR_CERT_UNTRUSTED, 20 },
        { "a certificate that signed itself", "selfsigned", "ca", false, "localhost", TLSERR_CERT_UNTRUSTED, 18 },
        { "a certificate signed by an authority that is not the trusted one", "otherca_leaf", "ca", false, "localhost", TLSERR_CERT_UNTRUSTED, 20 },
        { "a certificate for another name", "wrongname", "ca", false, "localhost", TLSERR_CERT_HOSTNAME_MISMATCH, 62 },
        { "a certificate that is expired", "expired", "ca", false, "localhost", TLSERR_CERT_EXPIRED, 10 },
        { "a certificate that is not valid yet", "future", "ca", false, "localhost", TLSERR_CERT_NOT_YET_VALID, 9 },
        { "a certificate with a common name but no subject alternative names", "cnonly", "ca", false, "localhost", TLSERR_CERT_NO_SAN, 62 }
    };

    for ( size_t i=0; i<sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
    {
        GUCEF_TESTFW_TESTCASE( ( CORE::CString( "OpenSslProvider: refused: " ) + cases[ i ].what ).C_String() )
        {
            SProviderSetup setup;
            Prepare( setup, certs );
            try
            {
                TESTFW::CTlsTestServer server;
                ASSERT_TRUE( server.Start( certs, cases[ i ].cert, TESTFW::CTlsTestServer::SERVE_STATUS_PAGE ) );

                CTlsSettings settings;
                if ( GUCEF_NULL != cases[ i ].ca )
                    settings = TrustTestCa( certs, cases[ i ].ca );
                settings.SetUseSystemTrust( cases[ i ].systemTrust );
                CTlsTestClient client;
                RunClient( setup, client, settings, cases[ i ].host, server.GetPort() );

                ASSERT_TRUE( cases[ i ].expected == FailureOf( client ) );
                ASSERT_TRUE( cases[ i ].verifyCode == client.Connection()->GetLastError().verifyCode );
                ASSERT_FALSE( client.m_connected );
                ASSERT_TRUE( client.m_plain.IsNULLOrEmpty() );          // not one byte of a server that did not pass
                ASSERT_FALSE( client.Connection()->GetLastError().message.IsNULLOrEmpty() );
                ASSERT_TRUE( client.Connection()->GetLastError().message.HasSubstr( "localhost" ) < 0 );    // no host in the error: the caller adds what it wants
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: an address is verified against the addresses of the certificate" )
    {
        SProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            TESTFW::CTlsTestServer server;
            ASSERT_TRUE( server.Start( certs, "good", TESTFW::CTlsTestServer::SERVE_STATUS_PAGE ) );

            // the certificate has 127.0.0.1
            CTlsTestClient accepted;
            RunClient( setup, accepted, TrustTestCa( certs ), "127.0.0.1", server.GetPort() );
            ASSERT_TRUE( TLSERR_NONE == FailureOf( accepted ) );
            ASSERT_TRUE( accepted.m_plain.StartsWith( "HTTP/1.0 200 ok" ) );

            // but not 127.0.0.2
            CTlsSettings other = TrustTestCa( certs );
            other.SetServerName( "127.0.0.2" );
            CTlsTestClient refused;
            RunClient( setup, refused, other, "127.0.0.1", server.GetPort() );
            ASSERT_TRUE( TLSERR_CERT_HOSTNAME_MISMATCH == FailureOf( refused ) );
            ASSERT_TRUE( 64 == refused.Connection()->GetLastError().verifyCode );

            // a server name that is not the host connected to
            CTlsSettings named = TrustTestCa( certs );
            named.SetServerName( "localhost" );
            CTlsTestClient byName;
            RunClient( setup, byName, named, "127.0.0.1", server.GetPort() );
            ASSERT_TRUE( TLSERR_NONE == FailureOf( byName ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: without verification a server nobody trusts is reached, with a warning per connection" )
    {
        SProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            TESTFW::CTlsTestServer server;
            ASSERT_TRUE( server.Start( certs, "expired", TESTFW::CTlsTestServer::SERVE_STATUS_PAGE ) );

            TESTFW::CLogTextCapture capture;
            CTlsSettings settings;
            settings.DisableVerificationInsecurely();
            CTlsTestClient client;
            RunClient( setup, client, settings, "localhost", server.GetPort() );
            ASSERT_TRUE( TLSERR_NONE == FailureOf( client ) );
            ASSERT_TRUE( client.m_plain.StartsWith( "HTTP/1.0 200 ok" ) );
            ASSERT_TRUE( capture.Contains( "TLS verification is DISABLED for localhost:" + CORE::ToString( server.GetPort() ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestTrustSources( const TESTFW::CTlsTestCerts& certs )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: a CA file that cannot be read fails the connection, verification is never skipped because of it" )
    {
        SProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            TESTFW::CTlsTestServer server;
            ASSERT_TRUE( server.Start( certs, "good", TESTFW::CTlsTestServer::SERVE_STATUS_PAGE ) );

            CTlsSettings settings;
            settings.SetCaBundlePath( certs.GetPath( "no_such_file.crt" ) );
            CTlsTestClient client;
            RunClient( setup, client, settings, "localhost", server.GetPort() );
            ASSERT_TRUE( TLSERR_SETTINGS == FailureOf( client ) );
            ASSERT_TRUE( client.Connection()->GetLastError().message.HasSubstr( "no_such_file.crt" ) >= 0 );
            ASSERT_FALSE( client.m_connected );
            ASSERT_TRUE( client.m_plain.IsNULLOrEmpty() );

            // a file that is there but holds no certificate
            FILE* empty = fopen( certs.GetPath( "empty.crt" ).C_String(), "wb" );
            ASSERT_TRUE( GUCEF_NULL != empty );
            fputs( "not a certificate\n", empty );
            fclose( empty );
            CTlsSettings noCerts;
            noCerts.SetCaBundlePath( certs.GetPath( "empty.crt" ) );
            CTlsTestClient second;
            RunClient( setup, second, noCerts, "localhost", server.GetPort() );
            ASSERT_TRUE( TLSERR_SETTINGS == FailureOf( second ) );

            // a directory that is not there, and nothing to trust at all
            CTlsSettings noDir;
            noDir.SetCaDir( certs.GetPath( "no_such_dir" ) );
            CTlsTestClient third;
            RunClient( setup, third, noDir, "localhost", server.GetPort() );
            ASSERT_TRUE( TLSERR_SETTINGS == FailureOf( third ) );
            CTlsSettings nothing;
            nothing.SetUseSystemTrust( false );
            CTlsTestClient fourth;
            RunClient( setup, fourth, nothing, "localhost", server.GetPort() );
            ASSERT_TRUE( TLSERR_SETTINGS == FailureOf( fourth ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: the CA can be given as data, and the trust of the system is extended by the CA of the settings" )
    {
        SProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            TESTFW::CTlsTestServer server;
            ASSERT_TRUE( server.Start( certs, "good", TESTFW::CTlsTestServer::SERVE_STATUS_PAGE ) );

            CORE::CDynamicBuffer pem;
            ASSERT_TRUE( pem.LoadContentFromFile( certs.GetPath( "ca.crt" ) ) );
            CTlsSettings fromData;
            fromData.SetCaBundleData( CORE::CString( pem.AsConstTypePtr< char >(), pem.GetDataSize() ) );
            fromData.SetUseSystemTrust( false );
            CTlsTestClient viaData;
            RunClient( setup, viaData, fromData, "localhost", server.GetPort() );
            ASSERT_TRUE( TLSERR_NONE == FailureOf( viaData ) );

            CTlsSettings garbage;
            garbage.SetCaBundleData( "-----BEGIN CERTIFICATE-----\nnope\n-----END CERTIFICATE-----\n" );
            garbage.SetUseSystemTrust( false );
            CTlsTestClient viaGarbage;
            RunClient( setup, viaGarbage, garbage, "localhost", server.GetPort() );
            ASSERT_TRUE( TLSERR_SETTINGS == FailureOf( viaGarbage ) );

            // the system store and the CA of the settings together
            CTlsSettings both;
            both.SetCaBundlePath( certs.GetPath( "ca.crt" ) );
            CTlsTestClient viaBoth;
            RunClient( setup, viaBoth, both, "localhost", server.GetPort() );
            ASSERT_TRUE( TLSERR_NONE == FailureOf( viaBoth ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: SSL_CERT_FILE is honoured as the trust of the system" )
    {
        SProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            TESTFW::CTlsTestServer server;
            ASSERT_TRUE( server.Start( certs, "good", TESTFW::CTlsTestServer::SERVE_STATUS_PAGE ) );

            // without it the authority of the test is not known
            CTlsTestClient before;
            RunClient( setup, before, CTlsSettings(), "localhost", server.GetPort() );
            ASSERT_TRUE( TLSERR_CERT_UNTRUSTED == FailureOf( before ) );

            // a provider of its own: the context with the system trust is made again, and it asks for the variable then
            CORE::CString previous = CORE::GetEnv( "SSL_CERT_FILE" );
            CORE::SetEnv( "SSL_CERT_FILE", certs.GetPath( "ca.crt" ) );
            SProviderSetup withVariable;
            CTlsTestClient after;
            RunClient( withVariable, after, CTlsSettings(), "localhost", server.GetPort() );
            ASSERT_TRUE( TLSERR_NONE == FailureOf( after ) );
            ASSERT_TRUE( after.m_plain.StartsWith( "HTTP/1.0 200 ok" ) );

            // The variable is part of what makes the context: with another file the same provider has another idea of what is trusted, and the
            // session that was kept for the host is not used in it (that would be a connection that nobody verified)
            ASSERT_TRUE( 1 == withVariable.provider->GetResumableSessionCount() );
            CORE::SetEnv( "SSL_CERT_FILE", certs.GetPath( "other_ca.crt" ) );
            CTlsTestClient other;
            RunClient( withVariable, other, CTlsSettings(), "localhost", server.GetPort() );
            CORE::SetEnv( "SSL_CERT_FILE", previous );
            ASSERT_TRUE( TLSERR_CERT_UNTRUSTED == FailureOf( other ) );
            ASSERT_FALSE( other.m_connected );
            ASSERT_TRUE( 2 == withVariable.provider->GetContextCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: the time the certificates are checked at can be set by tests, outside of the settings of an application" )
    {
        SProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            TESTFW::CTlsTestServer server;
            ASSERT_TRUE( server.Start( certs, "good", TESTFW::CTlsTestServer::SERVE_STATUS_PAGE ) );

            setup.provider->SetVerificationTimeForTesting( 4102444800LL );          // 2100: long after it ended
            CTlsTestClient late;
            RunClient( setup, late, TrustTestCa( certs ), "localhost", server.GetPort() );
            ASSERT_TRUE( TLSERR_CERT_EXPIRED == FailureOf( late ) );
            ASSERT_TRUE( late.Connection()->GetLastError().GetDescription( "localhost" ).HasSubstr( "clock" ) >= 0 );

            setup.provider->SetVerificationTimeForTesting( 1577836800LL );          // 2020: before it was made
            CTlsTestClient early;
            RunClient( setup, early, TrustTestCa( certs ), "localhost", server.GetPort() );
            ASSERT_TRUE( TLSERR_CERT_NOT_YET_VALID == FailureOf( early ) );

            setup.provider->SetVerificationTimeForTesting( 0 );
            CTlsTestClient now;
            RunClient( setup, now, TrustTestCa( certs ), "localhost", server.GetPort() );
            ASSERT_TRUE( TLSERR_NONE == FailureOf( now ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestVersions( const TESTFW::CTlsTestCerts& certs )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: servers that speak only TLS 1.2 or only TLS 1.3 are reached, and the minimum of the settings is kept" )
    {
        SProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            {
                TESTFW::CTlsTestServer server;
                ASSERT_TRUE( server.Start( certs, "good", TESTFW::CTlsTestServer::SERVE_STATUS_PAGE, Args( "-tls1_2" ) ) );
                CTlsTestClient client;
                RunClient( setup, client, TrustTestCa( certs ), "localhost", server.GetPort() );
                ASSERT_TRUE( TLSERR_NONE == FailureOf( client ) );
                ASSERT_TRUE( client.Connection()->GetSessionInfo().protocolVersion == "TLSv1.2" );

                // a client that wants 1.3 at least does not get a 1.2 server
                CTlsSettings strict = TrustTestCa( certs );
                strict.SetMinVersion( TLSVERSION_1_3 );
                CTlsTestClient refused;
                RunClient( setup, refused, strict, "localhost", server.GetPort() );
                ASSERT_TRUE( TLSERR_PROTOCOL_VERSION == FailureOf( refused ) );
            }
            {
                TESTFW::CTlsTestServer server;
                ASSERT_TRUE( server.Start( certs, "good", TESTFW::CTlsTestServer::SERVE_STATUS_PAGE, Args( "-tls1_3" ) ) );
                CTlsTestClient client;
                RunClient( setup, client, TrustTestCa( certs ), "localhost", server.GetPort() );
                ASSERT_TRUE( TLSERR_NONE == FailureOf( client ) );
                ASSERT_TRUE( client.Connection()->GetSessionInfo().protocolVersion == "TLSv1.3" );

                // a client that wants 1.2 at most does not get a 1.3 server
                CTlsSettings capped = TrustTestCa( certs );
                capped.SetMaxVersion( TLSVERSION_1_2 );
                CTlsTestClient refused;
                RunClient( setup, refused, capped, "localhost", server.GetPort() );
                ASSERT_TRUE( TLSERR_NONE != FailureOf( refused ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    static const char* oldVersions[] = { "-tls1_1", "-tls1" };
    static const char* oldLabels[] = { "TLS 1.1", "TLS 1.0" };
    for ( size_t i=0; i<2; ++i )
    {
        GUCEF_TESTFW_TESTCASE( ( CORE::CString( "OpenSslProvider: a server that only speaks " ) + oldLabels[ i ] + " is refused, there is no setting that allows it" ).C_String() )
        {
            SProviderSetup setup;
            Prepare( setup, certs );
            TESTFW::CTlsTestServer server;
            if ( !server.Start( certs, "good", TESTFW::CTlsTestServer::SERVE_STATUS_PAGE, Args( oldVersions[ i ], "-cipher", "ALL:@SECLEVEL=0" ) ) )
            {
                GUCEF_TESTFW_SKIP( "this openssl tool does not serve the old protocol version" );
            }
            try
            {
                CTlsTestClient client;
                RunClient( setup, client, TrustTestCa( certs ), "localhost", server.GetPort() );
                ASSERT_TRUE( TLSERR_PROTOCOL_VERSION == FailureOf( client ) );
                ASSERT_FALSE( client.m_connected );

                // asking for a lower minimum changes nothing: the floor is not a setting
                CTlsSettings lower = TrustTestCa( certs );
                lower.SetMinVersion( (ETlsVersion) 0x0301 );
                CTlsTestClient again;
                RunClient( setup, again, lower, "localhost", server.GetPort() );
                ASSERT_TRUE( TLSERR_PROTOCOL_VERSION == FailureOf( again ) );
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

static void
TestRobustness( const TESTFW::CTlsTestCerts& certs )
{GUCEF_TRACE;

    static const char* versions[] = { "-tls1_2", "-tls1_3" };
    for ( size_t i=0; i<2; ++i )
    {
        GUCEF_TESTFW_TESTCASE( ( CORE::CString( "OpenSslProvider: ciphertext delivered one byte at a time, " ) + ( 0 == i ? "TLS 1.2" : "TLS 1.3" ) + ", completes the handshake and the response" ).C_String() )
        {
            SProviderSetup setup;
            Prepare( setup, certs );
            try
            {
                TESTFW::CTlsTestServer server;
                ASSERT_TRUE( server.Start( certs, "good", TESTFW::CTlsTestServer::SERVE_FILES, Args( versions[ i ] ) ) );

                CRawSessionClient client( *setup.provider, TrustTestCa( certs ), "localhost", 1, 0 );
                client.SetRequest( "GET /small.txt HTTP/1.0\r\n\r\n" );
                ASSERT_TRUE( client.Start( "localhost", server.GetPort() ) );
                ASSERT_TRUE( PumpUntilTrue( client.m_closed, 10000 ) );
                ASSERT_FALSE( client.m_failed );
                ASSERT_TRUE( client.m_established );
                ASSERT_TRUE( client.m_plain.HasSubstr( "small file over tls" ) >= 0 );
                ASSERT_TRUE( client.m_session->GetInfo().protocolVersion == ( 0 == i ? "TLSv1.2" : "TLSv1.3" ) );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: a plain HTTP server that answers the ClientHello is a handshake failure and none of its text becomes data" )
    {
        SProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            CPlainTcpPeer peer( "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nplain", false );
            ASSERT_TRUE( peer.Start() );
            CTlsTestClient client;
            RunClient( setup, client, TrustTestCa( certs ), "localhost", peer.GetPort() );
            ASSERT_TRUE( TLSERR_HANDSHAKE_FAILURE == FailureOf( client ) );
            ASSERT_TRUE( client.m_plain.IsNULLOrEmpty() );
            ASSERT_FALSE( client.m_connected );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: a server that closes during the handshake is a failure, not an empty success" )
    {
        SProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            CPlainTcpPeer peer( "", true );
            ASSERT_TRUE( peer.Start() );
            CTlsTestClient client;
            RunClient( setup, client, TrustTestCa( certs ), "localhost", peer.GetPort() );
            ASSERT_TRUE( TLSERR_PEER_CLOSED == FailureOf( client ) );
            ASSERT_FALSE( client.m_connected );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: a server that does not answer the handshake is a timeout" )
    {
        SProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            // accepts and reads, never answers
            CPlainTcpPeer peer( "", false );
            ASSERT_TRUE( peer.Start() );
            CTlsSettings settings = TrustTestCa( certs );
            settings.SetHandshakeTimeoutInMs( 300 );
            CTlsTestClient client;
            RunClient( setup, client, settings, "localhost", peer.GetPort() );
            // the peer answers with a reply that is not TLS only when it has one: here it says nothing
            ASSERT_TRUE( TLSERR_HANDSHAKE_TIMEOUT == FailureOf( client ) || TLSERR_HANDSHAKE_FAILURE == FailureOf( client ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestResumption( const TESTFW::CTlsTestCerts& certs )
{GUCEF_TRACE;

    static const char* versions[] = { "-tls1_3", "-tls1_2" };
    for ( size_t i=0; i<2; ++i )
    {
        GUCEF_TESTFW_TESTCASE( ( CORE::CString( "OpenSslProvider: a second connection to the same host resumes the session, " ) + ( 0 == i ? "TLS 1.3" : "TLS 1.2" ) ).C_String() )
        {
            SProviderSetup setup;
            Prepare( setup, certs );
            try
            {
                TESTFW::CTlsTestServer server;
                ASSERT_TRUE( server.Start( certs, "good", TESTFW::CTlsTestServer::SERVE_STATUS_PAGE, Args( versions[ i ] ) ) );

                CTlsSettings settings = TrustTestCa( certs );
                CTlsTestClient first;
                RunClient( setup, first, settings, "localhost", server.GetPort() );
                ASSERT_TRUE( TLSERR_NONE == FailureOf( first ) );
                ASSERT_FALSE( first.Connection()->GetSessionInfo().resumed );
                ASSERT_TRUE( 1 == setup.provider->GetContextCount() );
                // closing is what makes the session resumable: it is kept when the connection is closed
                ASSERT_TRUE( 1 == setup.provider->GetResumableSessionCount() );

                CTlsTestClient second;
                RunClient( setup, second, settings, "localhost", server.GetPort() );
                ASSERT_TRUE( TLSERR_NONE == FailureOf( second ) );
                ASSERT_TRUE( second.Connection()->GetSessionInfo().resumed );
                ASSERT_TRUE( second.m_plain.StartsWith( "HTTP/1.0 200 ok" ) );
                ASSERT_TRUE( 1 == setup.provider->GetContextCount() );          // the same settings share a context

                // a session is for the name it was made for
                CTlsSettings other = TrustTestCa( certs );
                other.SetServerName( "127.0.0.1" );
                CTlsTestClient third;
                RunClient( setup, third, other, "localhost", server.GetPort() );
                ASSERT_TRUE( TLSERR_NONE == FailureOf( third ) );
                ASSERT_FALSE( third.Connection()->GetSessionInfo().resumed );

                // resumption off: never resumed, nothing kept
                CTlsSettings off = TrustTestCa( certs );
                off.SetAllowResumption( false );
                CTlsTestClient fourth, fifth;
                RunClient( setup, fourth, off, "localhost", server.GetPort() );
                RunClient( setup, fifth, off, "localhost", server.GetPort() );
                ASSERT_FALSE( fifth.Connection()->GetSessionInfo().resumed );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        }
        GUCEF_TESTFW_TESTCASE_END
    }

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: the number of sessions kept is limited by the settings" )
    {
        SProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            TESTFW::CTlsTestServer server;
            ASSERT_TRUE( server.Start( certs, "good", TESTFW::CTlsTestServer::SERVE_STATUS_PAGE ) );

            CTlsSettings settings = TrustTestCa( certs );
            settings.SetSessionCacheSize( 1 );
            const char* names[] = { "localhost", "127.0.0.1" };
            for ( size_t i=0; i<2; ++i )
            {
                CTlsSettings named = settings;
                named.SetServerName( names[ i ] );
                CTlsTestClient client;
                RunClient( setup, client, named, "127.0.0.1", server.GetPort() );
                ASSERT_TRUE( TLSERR_NONE == FailureOf( client ) );
            }
            ASSERT_TRUE( 1 == setup.provider->GetResumableSessionCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestThroughput( const TESTFW::CTlsTestCerts& certs )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "OpenSslProvider: a large download arrives complete, for the read block sizes of the socket" )
    {
        SProviderSetup setup;
        Prepare( setup, certs );
        try
        {
            const UInt32 size = 16 * 1024 * 1024;
            {
                FILE* file = fopen( certs.GetPath( "big.bin" ).C_String(), "wb" );
                ASSERT_TRUE( GUCEF_NULL != file );
                char block[ 4096 ];
                for ( UInt32 i=0; i<sizeof( block ); ++i )
                    block[ i ] = (char) ( i * 31 + 7 );
                for ( UInt32 i=0; i<size / sizeof( block ); ++i )
                    fwrite( block, 1, sizeof( block ), file );
                fclose( file );
            }

            TESTFW::CTlsTestServer server;
            ASSERT_TRUE( server.Start( certs, "good", TESTFW::CTlsTestServer::SERVE_FILES ) );

            // the socket reads 1 KiB when it is not told otherwise; a TLS record is up to 16 KiB
            // every size a few times: the wall time depends on the pulses and on the server that shares the computer, the CPU time of this process on the number of calls
            static const UInt32 blockSizes[] = { 0, 4096, 16384, 65536 };
            static const int repetitions = 5;
            for ( size_t i=0; i<sizeof( blockSizes ) / sizeof( blockSizes[ 0 ] ); ++i )
            {
                double wall[ repetitions ];
                double cpu[ repetitions ];
                for ( int r=0; r<repetitions; ++r )
                {
                    CRawSessionClient client( *setup.provider, TrustTestCa( certs ), "localhost", 0, blockSizes[ i ] );
                    clock_t cpuStart = clock();
                    UInt64 start = MT::PrecisionTickCount();
                    ASSERT_TRUE( client.Start( "localhost", server.GetPort() ) );
                    ASSERT_TRUE( PumpUntilTrue( client.m_closed, 60000 ) );
                    wall[ r ] = (double) ( MT::PrecisionTickCount() - start ) / (double) MT::PrecisionTimerResolution();
                    cpu[ r ] = (double) ( clock() - cpuStart ) / (double) CLOCKS_PER_SEC;
                    ASSERT_FALSE( client.m_failed );
                    ASSERT_TRUE( client.m_plainBytes > size );          // the body and the headers in front of it
                    ASSERT_TRUE( client.m_plainBytes < size + 1024 );
                }
                for ( int a=0; a<repetitions; ++a )
                    for ( int b=a+1; b<repetitions; ++b )
                    {
                        if ( wall[ b ] < wall[ a ] ) { double t = wall[ a ]; wall[ a ] = wall[ b ]; wall[ b ] = t; }
                        if ( cpu[ b ] < cpu[ a ] ) { double t = cpu[ a ]; cpu[ a ] = cpu[ b ]; cpu[ b ] = t; }
                    }
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "OpenSslProvider throughput: socket read block " << ( 0 == blockSizes[ i ] ? 1024 : blockSizes[ i ] ) << " bytes, median of " << repetitions << ": "
                    << ( (UInt32) ( size / ( 1024.0 * 1024.0 ) / wall[ repetitions / 2 ] ) ) << " MiB/s in " << ( (UInt32) ( wall[ repetitions / 2 ] * 1000 ) ) << " ms, "
                    << ( (UInt32) ( cpu[ repetitions / 2 ] * 1000 ) ) << " ms of CPU of this process" );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformTlsOpenSslTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "TlsOpenSsl" );

    TestLoader();

    TESTFW::CTlsTestCerts certs;
    if ( TESTFW::CTlsTestCerts::IsToolAvailable() && certs.IsReady() )
    {
        // a file the servers can serve
        FILE* small = fopen( certs.GetPath( "small.txt" ).C_String(), "wb" );
        if ( GUCEF_NULL != small )
        {
            fputs( "small file over tls\n", small );
            fclose( small );
        }
    }

    TestTrustAndNames( certs );
    TestTrustSources( certs );
    TestVersions( certs );
    TestRobustness( certs );
    TestResumption( certs );
    TestThroughput( certs );
}

/*-------------------------------------------------------------------------*/
