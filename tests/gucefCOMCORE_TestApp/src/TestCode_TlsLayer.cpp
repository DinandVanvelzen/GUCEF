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

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#include "TestCode_TlsClient.h"
#include "TestCode_TlsLayer.h"

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
//      THE FAKE TLS                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/**
 *  A fake TLS that is a framed XOR filter: frames of 1 byte type, 4 bytes length and the payload. A client sends
 *  a hello frame, the server answers with a welcome frame or a failure, data frames are XORed with a constant
 *  and a close frame is the close notification. It lets the tests drive every path of the connection without a
 *  TLS library, on a real loopback TCP connection.
 */
static const char FrameHello = 'H';
static const char FrameWelcome = 'W';
static const char FrameFail = 'F';
static const char FrameData = 'D';
static const char FrameClose = 'C';
static const UInt8 DataMask = 0x5A;

/*-------------------------------------------------------------------------*/

class CFakeFrames
{
    public:

    CFakeFrames( void ) : m_in(), m_garbage( false ) {}

    static void Append( CORE::CDynamicBuffer& out, char type, const void* payload, UInt32 size )
    {
        UInt8 header[ 5 ];
        header[ 0 ] = (UInt8) type;
        header[ 1 ] = (UInt8) ( size >> 24 );
        header[ 2 ] = (UInt8) ( size >> 16 );
        header[ 3 ] = (UInt8) ( size >> 8 );
        header[ 4 ] = (UInt8) size;
        out.Append( header, 5 );
        if ( size > 0 )
            out.Append( payload, size );
    }

    void Push( const void* data, UInt32 size ) { m_in.Append( data, size ); }

    bool Next( char& type, CORE::CDynamicBuffer& payload )
    {
        if ( m_in.GetDataSize() < 5 )
            return false;

        const UInt8* p = static_cast< const UInt8* >( m_in.GetConstBufferPtr() );
        type = (char) p[ 0 ];
        if ( FrameHello != type && FrameWelcome != type && FrameFail != type && FrameData != type && FrameClose != type )
        {
            m_garbage = true;
            return false;
        }
        UInt32 length = ( (UInt32) p[ 1 ] << 24 ) | ( (UInt32) p[ 2 ] << 16 ) | ( (UInt32) p[ 3 ] << 8 ) | (UInt32) p[ 4 ];
        if ( length > 1024 * 1024 )
        {
            m_garbage = true;
            return false;
        }
        if ( m_in.GetDataSize() < 5 + length )
            return false;

        payload.SetDataSize( 0 );
        if ( length > 0 )
            payload.Append( p + 5, length );
        m_in.Downshift( 5 + length );
        return true;
    }

    bool IsGarbage( void ) const { return m_garbage; }

    private:

    CORE::CDynamicBuffer m_in;
    bool m_garbage;
};

/*-------------------------------------------------------------------------*/

struct SFakeConfig
{
    ETlsErrorClass clientFailsOnWelcome;    /**< the client session fails after the welcome with this class */
    ETlsErrorClass serverRepliesFail;       /**< the server session answers the hello with a failure of this class */

    SFakeConfig( void ) : clientFailsOnWelcome( TLSERR_NONE ), serverRepliesFail( TLSERR_NONE ) {}
};

/*-------------------------------------------------------------------------*/

class CFakeSession : public ITlsSession
{
    public:

    CFakeSession( bool isServer, const SFakeConfig& config, const CORE::CString& serverName )
        : ITlsSession(), m_isServer( isServer ), m_config( config ), m_state( TLSSTATE_IDLE ), m_serverName( serverName )
        , m_frames(), m_out(), m_plain(), m_payload(), m_error(), m_info(), m_helloName(), m_helloSeen( false )
    {
        m_info.providerName = "fake";
        m_info.protocolVersion = "FAKE1";
        m_info.cipherName = "XOR5A";
    }

    virtual bool Start( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( TLSSTATE_IDLE != m_state )
            return false;
        m_state = TLSSTATE_HANDSHAKING;
        if ( !m_isServer )
            CFakeFrames::Append( m_out, FrameHello, m_serverName.C_String(), m_serverName.Length() );
        return true;
    }

    virtual ETlsState PushCiphertext( const void* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( TLSSTATE_FAILED == m_state )
            return m_state;
        if ( TLSSTATE_IDLE == m_state )
            m_state = TLSSTATE_HANDSHAKING;
        m_frames.Push( data, size );

        char type = 0;
        while ( TLSSTATE_FAILED != m_state && m_frames.Next( type, m_payload ) )
            OnFrame( type );
        if ( TLSSTATE_FAILED != m_state && m_frames.IsGarbage() )
            Fail( TLSERR_HANDSHAKE_FAILURE, "the peer does not speak fake TLS" );
        return m_state;
    }

    virtual ETlsState PushPlaintext( const void* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( TLSSTATE_ESTABLISHED != m_state )
            return m_state;

        const UInt8* bytes = static_cast< const UInt8* >( data );
        const UInt32 recordSize = 4096;
        UInt8 record[ recordSize ];
        for ( UInt32 offset=0; offset<size; offset+=recordSize )
        {
            UInt32 take = size - offset < recordSize ? size - offset : recordSize;
            for ( UInt32 i=0; i<take; ++i )
                record[ i ] = (UInt8) ( bytes[ offset + i ] ^ DataMask );
            CFakeFrames::Append( m_out, FrameData, record, take );
        }
        return m_state;
    }

    virtual bool PullCiphertext( CORE::CDynamicBuffer& out ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 0 == m_out.GetDataSize() )
            return false;
        out.Append( m_out.GetConstBufferPtr(), m_out.GetDataSize() );
        m_out.SetDataSize( 0 );
        return true;
    }

    virtual bool PullPlaintext( CORE::CDynamicBuffer& out ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 0 == m_plain.GetDataSize() )
            return false;
        out.Append( m_plain.GetConstBufferPtr(), m_plain.GetDataSize() );
        m_plain.SetDataSize( 0 );
        return true;
    }

    virtual ETlsState GetState( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_state; }

    virtual void InitiateClose( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( TLSSTATE_ESTABLISHED == m_state || TLSSTATE_CLOSED == m_state )
        {
            CFakeFrames::Append( m_out, FrameClose, GUCEF_NULL, 0 );
            if ( TLSSTATE_ESTABLISHED == m_state )
                m_state = TLSSTATE_CLOSING;
        }
    }

    virtual const CTlsError& GetError( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_error; }

    virtual const CTlsSessionInfo& GetInfo( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_info; }

    const CORE::CString& GetHelloName( void ) const { return m_helloName; }
    bool HasSeenHello( void ) const { return m_helloSeen; }

    private:

    void Fail( ETlsErrorClass errorClass, const CORE::CString& message )
    {
        m_error = CTlsError( errorClass, message );
        m_state = TLSSTATE_FAILED;
    }

    void Establish( void )
    {
        m_state = TLSSTATE_ESTABLISHED;
    }

    void OnFrame( char type )
    {
        if ( TLSSTATE_HANDSHAKING == m_state )
        {
            if ( m_isServer && FrameHello == type )
            {
                m_helloSeen = true;
                m_helloName.Set( static_cast< const char* >( m_payload.GetConstBufferPtr() ), m_payload.GetDataSize() );
                if ( TLSERR_NONE != m_config.serverRepliesFail )
                {
                    UInt8 errorClass = (UInt8) m_config.serverRepliesFail;
                    CFakeFrames::Append( m_out, FrameFail, &errorClass, 1 );
                    m_state = TLSSTATE_CLOSED;
                    return;
                }
                CFakeFrames::Append( m_out, FrameWelcome, "ok", 2 );
                Establish();
                return;
            }
            if ( !m_isServer && FrameWelcome == type )
            {
                if ( TLSERR_NONE != m_config.clientFailsOnWelcome )
                {
                    Fail( m_config.clientFailsOnWelcome, "the fake provider was told to fail here" );
                    return;
                }
                Establish();
                return;
            }
            if ( !m_isServer && FrameFail == type && m_payload.GetDataSize() > 0 )
            {
                Fail( (ETlsErrorClass) *static_cast< const UInt8* >( m_payload.GetConstBufferPtr() ), "the fake peer refused the handshake" );
                return;
            }
            Fail( TLSERR_HANDSHAKE_FAILURE, "unexpected frame during the handshake" );
            return;
        }

        if ( TLSSTATE_ESTABLISHED == m_state || TLSSTATE_CLOSING == m_state )
        {
            if ( FrameData == type )
            {
                const UInt8* bytes = static_cast< const UInt8* >( m_payload.GetConstBufferPtr() );
                const UInt32 length = m_payload.GetDataSize();
                const UInt32 start = m_plain.GetDataSize();
                m_plain.SetBufferSize( start + length, false );
                UInt8* target = static_cast< UInt8* >( m_plain.GetBufferPtr( start ) );
                for ( UInt32 i=0; i<length; ++i )
                    target[ i ] = (UInt8) ( bytes[ i ] ^ DataMask );
                m_plain.SetDataSize( start + length );
                return;
            }
            if ( FrameClose == type )
            {
                m_info.peerClosedCleanly = true;
                m_state = TLSSTATE_CLOSED;
                return;
            }
            // the welcome that arrives after a handshake that was completed early is ignored
            if ( FrameWelcome == type )
                return;
        }
        Fail( TLSERR_HANDSHAKE_FAILURE, "unexpected frame" );
    }

    private:

    bool m_isServer;
    SFakeConfig m_config;
    ETlsState m_state;
    CORE::CString m_serverName;
    CFakeFrames m_frames;
    CORE::CDynamicBuffer m_out;
    CORE::CDynamicBuffer m_plain;
    CORE::CDynamicBuffer m_payload;
    CTlsError m_error;
    CTlsSessionInfo m_info;
    CORE::CString m_helloName;
    bool m_helloSeen;
};

/*-------------------------------------------------------------------------*/

class CFakeProvider : public ITlsProvider
{
    public:

    CFakeProvider( const CORE::CString& name = "fake" )
        : ITlsProvider(), m_name( name ), m_available( true ), m_whyNot(), m_capabilities( TLSCAP_ALPN ), m_config()
        , m_createError(), m_sessionsCreated( 0 ), m_lastServerName(), m_lastSettings()
    {}

    virtual CORE::CString GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_name; }

    virtual bool IsAvailable( CORE::CString& whyNot ) const GUCEF_VIRTUAL_OVERRIDE
    {
        whyNot = m_available ? CORE::CString::Empty : m_whyNot;
        return m_available;
    }

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_capabilities; }

    virtual CORE::CString GetVersionInfo( void ) const GUCEF_VIRTUAL_OVERRIDE { return "fake 1"; }

    virtual TTlsSessionPtr CreateClientSession( const CTlsSettings& settings, const CORE::CString& serverName, CTlsError& error ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++m_sessionsCreated;
        m_lastServerName = serverName;
        m_lastSettings = settings;
        if ( m_createError.IsError() )
        {
            error = m_createError;
            return TTlsSessionPtr();
        }
        return TTlsSessionPtr( new CFakeSession( false, m_config, serverName ) );
    }

    CORE::CString m_name;
    bool m_available;
    CORE::CString m_whyNot;
    UInt32 m_capabilities;
    SFakeConfig m_config;
    CTlsError m_createError;
    UInt32 m_sessionsCreated;
    CORE::CString m_lastServerName;
    CTlsSettings m_lastSettings;
};

/*-------------------------------------------------------------------------*/

/** the peer: a TCP server speaking the fake TLS in the way the test asks for */
class CFakePeer : public CORE::CObserver
{
    public:

    bool silent;                    /**< accepts and reads, never answers */
    bool closeOnConnect;            /**< closes the TCP connection as soon as it is accepted */
    bool garbageReply;              /**< answers the first bytes with an HTTP error, as a server that does not speak TLS */
    bool dribble;                   /**< sends what it has one byte per tick */
    bool echo;                      /**< sends the plaintext it receives back */
    bool closeAfterWelcome;         /**< closes the TCP connection after the welcome, without close notification */
    bool closeInsteadOfWelcome;     /**< closes the TCP connection when the hello arrives, without answering */
    bool dataWithWelcome;           /**< sends a first data frame together with the welcome */
    bool plainUpgrade;              /**< starts plain: a line "UPGRADE" is answered with "OK" and what follows is the fake TLS */
    bool welcomeWithUpgradeAnswer;  /**< with plainUpgrade: sends the welcome in the same write as the answer, so the client reads it as plain data */
    ETlsErrorClass failWith;

    CORE::CString m_received;       /**< plaintext received */
    CORE::CString m_plainLines;     /**< what arrived before an upgrade */
    CORE::CString m_events;         /**< "hello,data,close,disconnect" in order of happening */
    CORE::CString m_helloName;
    UInt32 m_connections;
    UInt32 m_disconnects;

    CFakePeer( void )
        : CORE::CObserver(), silent( false ), closeOnConnect( false ), garbageReply( false ), dribble( false ), echo( false )
        , closeAfterWelcome( false ), closeInsteadOfWelcome( false ), dataWithWelcome( false ), plainUpgrade( false ), welcomeWithUpgradeAnswer( false )
        , failWith( TLSERR_NONE ), m_received(), m_plainLines(), m_events(), m_helloName(), m_connections( 0 ), m_disconnects( 0 )
        , m_socket( false ), m_timer( 1 ), m_peers()
    {
        SubscribeTo( &m_socket );
        SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent );
    }

    virtual ~CFakePeer()
    {
        UnsubscribeAllFromObserver();
        for ( size_t i=0; i<m_peers.size(); ++i )
            delete m_peers[ i ];
        m_peers.clear();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CFakePeer";
        return classTypeName;
    }

    bool Start( void )
    {
        if ( !m_socket.ListenOnPort( 0 ) )
            return false;
        m_timer.SetEnabled( true );
        return true;
    }

    UInt16 GetPort( void ) const { return m_socket.GetBoundPort(); }

    /** sends bytes to the first connected client, plain or as they are */
    bool SendRaw( const CORE::CString& bytes )
    {
        if ( m_peers.empty() )
            return false;
        return m_peers[ 0 ]->connection->Send( bytes.C_String(), bytes.Length() );
    }

    /** ends the fake TLS of the first client the proper way: close notification, then the TCP connection */
    void CloseGracefully( void )
    {
        if ( m_peers.empty() )
            return;
        m_peers[ 0 ]->session->InitiateClose();
        SendOut( m_peers[ 0 ] );
        m_peers[ 0 ]->connection->Close();
    }

    /** closes the TCP connection of the first client at once */
    void DropClient( void )
    {
        if ( !m_peers.empty() )
            m_peers[ 0 ]->connection->Close();
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( notifier == &m_timer )
        {
            // one byte per tick of what is queued
            for ( size_t i=0; i<m_peers.size(); ++i )
            {
                SPeer* peer = m_peers[ i ];
                if ( peer->queue.GetDataSize() > 0 )
                {
                    peer->connection->Send( peer->queue.GetConstBufferPtr(), 1 );
                    peer->queue.Downshift( 1 );
                }
            }
            return;
        }

        if ( CTCPServerSocket::ClientConnectedEvent == eventId )
        {
            CTCPServerSocket::TClientConnectedEventData* data = static_cast< CTCPServerSocket::TClientConnectedEventData* >( eventData );
            ++m_connections;
            SPeer* peer = new SPeer;
            peer->connection = data->GetData().connection;
            peer->upgraded = !plainUpgrade;
            SFakeConfig config;
            config.serverRepliesFail = failWith;
            peer->session = new CFakeSession( true, config, "" );
            peer->sessionPtr = TTlsSessionPtr( peer->session );
            peer->session->Start();
            m_peers.push_back( peer );
            SubscribeTo( peer->connection );
            if ( closeOnConnect )
                peer->connection->Close();
            return;
        }

        if ( CTCPServerSocket::ClientDisconnectedEvent == eventId )
        {
            CTCPServerSocket::TClientDisconnectedEventData* data = static_cast< CTCPServerSocket::TClientDisconnectedEventData* >( eventData );
            ++m_disconnects;
            m_events += "disconnect,";
            for ( size_t i=0; i<m_peers.size(); ++i )
            {
                if ( m_peers[ i ]->connection == data->GetData().connectionInfo.connection )
                {
                    delete m_peers[ i ];
                    m_peers.erase( m_peers.begin() + i );
                    break;
                }
            }
            return;
        }

        if ( CTCPConnection::DataRecievedEvent == eventId && notifier != &m_socket )
        {
            SPeer* peer = FindPeer( notifier );
            if ( GUCEF_NULL == peer || GUCEF_NULL == eventData )
                return;
            CTCPConnection::TDataRecievedEventData* data = static_cast< CTCPConnection::TDataRecievedEventData* >( eventData );
            OnPeerData( peer, data->GetData() );
        }
    }

    private:

    struct SPeer
    {
        CTCPServerConnection* connection;
        CFakeSession* session;
        TTlsSessionPtr sessionPtr;
        bool upgraded;
        CORE::CDynamicBuffer out;
        CORE::CDynamicBuffer queue;
        CORE::CString plainLine;
        bool helloLogged;
        bool closeLogged;

        SPeer( void ) : connection( GUCEF_NULL ), session( GUCEF_NULL ), sessionPtr(), upgraded( true ), out(), queue(), plainLine(), helloLogged( false ), closeLogged( false ) {}
    };

    SPeer* FindPeer( CORE::CNotifier* connection )
    {
        for ( size_t i=0; i<m_peers.size(); ++i )
            if ( m_peers[ i ]->connection == connection )
                return m_peers[ i ];
        return GUCEF_NULL;
    }

    void SendOut( SPeer* peer )
    {
        peer->out.SetDataSize( 0 );
        if ( !peer->session->PullCiphertext( peer->out ) || 0 == peer->out.GetDataSize() )
            return;
        if ( dribble )
            peer->queue.Append( peer->out.GetConstBufferPtr(), peer->out.GetDataSize() );
        else
            peer->connection->Send( peer->out.GetConstBufferPtr(), peer->out.GetDataSize() );
    }

    void OnPeerData( SPeer* peer, const CORE::CDynamicBuffer& data )
    {
        if ( silent )
            return;

        if ( garbageReply )
        {
            static const char reply[] = "HTTP/1.0 400 Bad Request\r\n\r\nthis is not a TLS server\r\n";
            peer->connection->Send( reply, sizeof( reply ) - 1 );
            return;
        }

        const char* bytes = static_cast< const char* >( data.GetConstBufferPtr() );
        UInt32 size = data.GetDataSize();

        if ( !peer->upgraded )
        {
            // plain until the line that upgrades
            peer->plainLine.Append( bytes, size );
            m_plainLines.Append( bytes, size );
            Int32 lineEnd = peer->plainLine.HasSubstr( "UPGRADE\n" );
            if ( lineEnd < 0 )
                return;
            peer->upgraded = true;

            CORE::CDynamicBuffer answer;
            answer.Append( "OK\n", 3 );
            if ( welcomeWithUpgradeAnswer )
                CFakeFrames::Append( answer, FrameWelcome, "ok", 2 );
            peer->connection->Send( answer.GetConstBufferPtr(), answer.GetDataSize() );
            // what came behind the line is the start of the fake TLS: the hello
            UInt32 consumed = (UInt32) lineEnd + 8;
            if ( consumed < peer->plainLine.Length() )
                Process( peer, peer->plainLine.C_String() + consumed, peer->plainLine.Length() - consumed );
            return;
        }

        Process( peer, bytes, size );
    }

    void Process( SPeer* peer, const void* bytes, UInt32 size )
    {
        peer->session->PushCiphertext( bytes, size );

        if ( !peer->helloLogged && peer->session->HasSeenHello() )
        {
            peer->helloLogged = true;
            m_helloName = peer->session->GetHelloName();
            m_events += "hello,";
            if ( dataWithWelcome )
            {
                const char greeting[] = "greeting";
                peer->session->PushPlaintext( greeting, sizeof( greeting ) - 1 );
            }
        }

        CORE::CDynamicBuffer plain;
        if ( peer->session->PullPlaintext( plain ) )
        {
            m_events += "data,";
            m_received.Append( static_cast< const char* >( plain.GetConstBufferPtr() ), plain.GetDataSize() );
            if ( echo )
                peer->session->PushPlaintext( plain.GetConstBufferPtr(), plain.GetDataSize() );
        }

        if ( closeInsteadOfWelcome && peer->helloLogged )
        {
            peer->connection->Close();
            return;
        }

        SendOut( peer );

        if ( closeAfterWelcome && peer->helloLogged )
        {
            // gone without a close notification
            peer->connection->Close();
            return;
        }

        if ( ETlsState( peer->session->GetState() ) == TLSSTATE_CLOSED && peer->session->GetInfo().peerClosedCleanly && !peer->closeLogged )
        {
            peer->closeLogged = true;
            m_events += "close,";
            peer->session->InitiateClose();
            SendOut( peer );
            peer->connection->Close();
        }
    }

    private:

    CTCPServerSocket m_socket;
    CORE::CTimer m_timer;
    typedef GUCEF::vector< SPeer* > TPeerVector;
    TPeerVector m_peers;
};

/*-------------------------------------------------------------------------*/

/** condition: the peer saw this many disconnects */
struct PeerDisconnects
{
    PeerDisconnects( const CFakePeer& peer, UInt32 count ) : m_peer( peer ), m_count( count ) {}
    bool operator()( void ) const { return m_peer.m_disconnects >= m_count; }
    const CFakePeer& m_peer;
    UInt32 m_count;
};

/** condition: the peer received data this many times */
struct PeerData
{
    PeerData( const CFakePeer& peer, UInt32 length ) : m_peer( peer ), m_length( length ) {}
    bool operator()( void ) const { return m_peer.m_received.Length() >= m_length; }
    const CFakePeer& m_peer;
    UInt32 m_length;
};

/** a registry with a fake provider in it, the connection of a test uses it instead of the one of the process */
struct SFakeSetup
{
    CTlsProviderRegistry registry;
    CFakeProvider* provider;
    CFakePeer peer;

    SFakeSetup( void ) : registry(), provider( new CFakeProvider() ), peer()
    {
        registry.RegisterProvider( TITlsProviderPtr( provider ), CTlsProviderRegistry::SystemLibraryTlsProviderPriority );
    }
};

static bool
Connect( SFakeSetup& setup, CTlsTestClient& client, const CTlsSettings* settings = GUCEF_NULL, const char* host = "localhost" )
{
    client.Connection()->SetProviderRegistry( &setup.registry );
    CTlsSettings defaults;
    if ( GUCEF_NULL == settings )
        settings = &defaults;
    return client.Connection()->ConnectTo( host, setup.peer.GetPort(), settings );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestSettings( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "TlsSettings: the defaults are the safe ones and nothing sets the version below TLS 1.2" )
    try
    {
        CTlsSettings settings;
        ASSERT_TRUE( TLSVERSION_1_2 == settings.GetMinVersion() );
        ASSERT_TRUE( TLSVERSION_ANY == settings.GetMaxVersion() );
        ASSERT_TRUE( settings.IsVerificationEnabled() );
        ASSERT_TRUE( settings.GetUseSystemTrust() );
        ASSERT_TRUE( settings.GetAllowResumption() );

        // there is no value below 1.2, an unknown one is the floor
        settings.SetMinVersion( (ETlsVersion) 0x0301 );
        ASSERT_TRUE( TLSVERSION_1_2 == settings.GetMinVersion() );
        settings.SetMinVersion( (ETlsVersion) 0 );
        ASSERT_TRUE( TLSVERSION_1_2 == settings.GetMinVersion() );
        settings.SetMinVersion( TLSVERSION_1_3 );
        ASSERT_TRUE( TLSVERSION_1_3 == settings.GetMinVersion() );
        settings.SetMaxVersion( TLSVERSION_1_2 );
        ASSERT_TRUE( TLSVERSION_1_3 == settings.GetMaxVersion() );    // never below the minimum
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsSettings: verification only goes off by the explicit call, never from the configuration" )
    try
    {
        CTlsSettings settings;
        CORE::CDataNode config;
        config.SetAttribute( "verifyMode", CORE::CString( "none" ) );
        config.SetAttribute( "minVersion", CORE::CString( "1.0" ) );
        ASSERT_TRUE( settings.LoadConfig( config ) );
        ASSERT_TRUE( settings.IsVerificationEnabled() );
        ASSERT_TRUE( TLSVERSION_1_2 == settings.GetMinVersion() );

        settings.DisableVerificationInsecurely();
        ASSERT_FALSE( settings.IsVerificationEnabled() );

        // and it is not something a configuration written out and read again brings back
        CORE::CDataNode saved;
        ASSERT_TRUE( settings.SaveConfig( saved ) );
        CTlsSettings reloaded;
        ASSERT_TRUE( reloaded.LoadConfig( saved ) );
        ASSERT_TRUE( reloaded.IsVerificationEnabled() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsSettings: the configuration is read and written, the context key tells what a context depends on" )
    try
    {
        CORE::CDataNode config;
        config.SetAttribute( "minVersion", CORE::CString( "1.3" ) );
        config.SetAttribute( "caBundlePath", CORE::CString( "/etc/ca.pem" ) );
        config.SetAttribute( "caDir", CORE::CString( "/etc/cas" ) );
        config.SetAttribute( "useSystemTrust", false );
        config.SetAttribute( "alpn", CORE::CString( "http/1.1, h2" ) );
        config.SetAttribute( "handshakeTimeoutInMs", (UInt32) 1234 );
        config.SetAttribute( "allowResumption", false );
        config.SetAttribute( "sessionCacheSize", (UInt32) 7 );
        CTlsSettings settings;
        ASSERT_TRUE( settings.LoadConfig( config ) );
        ASSERT_TRUE( TLSVERSION_1_3 == settings.GetMinVersion() );
        ASSERT_TRUE( settings.GetCaBundlePath() == "/etc/ca.pem" );
        ASSERT_TRUE( settings.GetCaDir() == "/etc/cas" );
        ASSERT_FALSE( settings.GetUseSystemTrust() );
        ASSERT_TRUE( 2 == settings.GetAlpnProtocols().size() );
        ASSERT_TRUE( settings.GetAlpnProtocols()[ 1 ] == "h2" );
        ASSERT_TRUE( 1234 == settings.GetHandshakeTimeoutInMs() );
        ASSERT_FALSE( settings.GetAllowResumption() );
        ASSERT_TRUE( 7 == settings.GetSessionCacheSize() );

        CTlsSettings a, b;
        ASSERT_TRUE( a.GetContextKey() == b.GetContextKey() );
        b.SetServerName( "example.com" );
        ASSERT_TRUE( a.GetContextKey() == b.GetContextKey() );          // the name is per connection, not per context
        b.SetCaBundlePath( "/etc/ca.pem" );
        ASSERT_TRUE( a.GetContextKey() != b.GetContextKey() );
        CTlsSettings c;
        c.DisableVerificationInsecurely();
        ASSERT_TRUE( a.GetContextKey() != c.GetContextKey() );
        CTlsSettings d;
        d.AddAlpnProtocol( "http/1.1" );
        ASSERT_TRUE( a.GetContextKey() != d.GetContextKey() );
        CTlsSettings e;
        e.SetMinVersion( TLSVERSION_1_3 );
        ASSERT_TRUE( a.GetContextKey() != e.GetContextKey() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRegistry( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "TlsProviderRegistry: the provider with the highest priority that is available and has the capabilities is the best" )
    try
    {
        CTlsProviderRegistry registry;
        CFakeProvider* os = new CFakeProvider( "os" );
        CFakeProvider* system = new CFakeProvider( "system" );
        CFakeProvider* plugin = new CFakeProvider( "plugin" );
        plugin->m_capabilities = TLSCAP_ALPN | TLSCAP_SERVER;

        // registered in the wrong order on purpose
        ASSERT_TRUE( registry.RegisterProvider( TITlsProviderPtr( plugin ), CTlsProviderRegistry::PluginTlsProviderPriority ) );
        ASSERT_TRUE( registry.RegisterProvider( TITlsProviderPtr( os ), CTlsProviderRegistry::OsTlsProviderPriority ) );
        TITlsProviderPtr systemPtr( system );
        ASSERT_TRUE( registry.RegisterProvider( systemPtr, CTlsProviderRegistry::SystemLibraryTlsProviderPriority ) );
        ASSERT_FALSE( registry.RegisterProvider( systemPtr, 1 ) );          // the same object is registered once
        ASSERT_TRUE( 3 == registry.GetProviderCount() );

        CORE::CString whyNot;
        TITlsProviderPtr best = registry.GetBestAvailable( TLSCAP_NONE, whyNot );
        ASSERT_TRUE( !best.IsNULL() && best->GetProviderName() == "os" );

        os->m_available = false;
        os->m_whyNot = "SChannel: TLS 1.2 is not enabled";
        best = registry.GetBestAvailable( TLSCAP_NONE, whyNot );
        ASSERT_TRUE( !best.IsNULL() && best->GetProviderName() == "system" );

        // a capability only the lowest has
        best = registry.GetBestAvailable( TLSCAP_SERVER, whyNot );
        ASSERT_TRUE( !best.IsNULL() && best->GetProviderName() == "plugin" );

        CTlsProviderRegistry::TTlsProviderStatusVector status;
        registry.GetProviderStatus( status );
        ASSERT_TRUE( 3 == status.size() );
        ASSERT_TRUE( status[ 0 ].provider->GetProviderName() == "os" && !status[ 0 ].available && status[ 0 ].reason == "SChannel: TLS 1.2 is not enabled" );
        ASSERT_TRUE( status[ 1 ].provider->GetProviderName() == "system" && status[ 1 ].available && status[ 1 ].reason.IsNULLOrEmpty() );
        ASSERT_TRUE( status[ 2 ].priority == CTlsProviderRegistry::PluginTlsProviderPriority );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsProviderRegistry: nothing available says why for each provider, equal priorities keep their order, providers can be taken out" )
    try
    {
        CTlsProviderRegistry registry;
        CORE::CString whyNot;
        ASSERT_TRUE( registry.GetBestAvailable( TLSCAP_NONE, whyNot ).IsNULL() );
        ASSERT_FALSE( whyNot.IsNULLOrEmpty() );             // no provider at all is also an answer

        CFakeProvider* first = new CFakeProvider( "first" );
        CFakeProvider* second = new CFakeProvider( "second" );
        first->m_available = false;
        first->m_whyNot = "libssl.so.3: cannot open shared object file";
        second->m_available = false;
        second->m_whyNot = "too old";
        TITlsProviderPtr firstPtr( first ), secondPtr( second );
        registry.RegisterProvider( firstPtr, 50 );
        registry.RegisterProvider( secondPtr, 50 );

        ASSERT_TRUE( registry.GetBestAvailable( TLSCAP_NONE, whyNot ).IsNULL() );
        ASSERT_TRUE( whyNot.HasSubstr( "first: libssl.so.3: cannot open shared object file" ) >= 0 );
        ASSERT_TRUE( whyNot.HasSubstr( "second: too old" ) >= 0 );
        ASSERT_TRUE( whyNot.HasSubstr( "first" ) < whyNot.HasSubstr( "second" ) );
        ASSERT_FALSE( registry.IsAnyAvailable( whyNot ) );

        second->m_available = true;
        ASSERT_TRUE( registry.IsAnyAvailable( whyNot ) );
        ASSERT_TRUE( whyNot.IsNULLOrEmpty() );
        ASSERT_TRUE( registry.UnregisterProvider( secondPtr ) );
        ASSERT_FALSE( registry.UnregisterProvider( secondPtr ) );
        ASSERT_FALSE( registry.IsAnyAvailable( whyNot ) );
        registry.UnregisterAll();
        ASSERT_TRUE( 0 == registry.GetProviderCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsProviderRegistry: the registry of the process has the system provider, which probes only when asked" )
    try
    {
        CTlsProviderRegistry& registry = COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry();
        ASSERT_TRUE( registry.GetProviderCount() >= 1 );
        CTlsProviderRegistry::TTlsProviderStatusVector status;
        registry.GetProviderStatus( status );
        bool found = false;
        for ( size_t i=0; i<status.size(); ++i )
        {
            if ( status[ i ].provider->GetProviderName() == "openssl-dynamic" )
            {
                found = true;
                ASSERT_TRUE( status[ i ].priority == CTlsProviderRegistry::SystemLibraryTlsProviderPriority );
                // either it works or it says why not, never a silent false
                ASSERT_TRUE( status[ i ].available || !status[ i ].reason.IsNULLOrEmpty() );
            }
        }
        ASSERT_TRUE( found );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestErrorTypes( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "TlsError: every class has a name and a text for the user that names the host only when it is given one" )
    try
    {
        for ( int c = (int) TLSERR_NO_PROVIDER; c <= (int) TLSERR_INTERNAL; ++c )
        {
            CTlsError error( (ETlsErrorClass) c, "detail" );
            ASSERT_TRUE( error.IsError() );
            ASSERT_TRUE( 0 != strcmp( TlsErrorClassToString( (ETlsErrorClass) c ), "unknown" ) );
            ASSERT_FALSE( error.GetDescription( "example.com" ).IsNULLOrEmpty() );
        }
        CTlsError expired( TLSERR_CERT_EXPIRED, "certificate has expired" );
        CORE::CString text = expired.GetDescription( "example.com" );
        ASSERT_TRUE( text.HasSubstr( "example.com" ) >= 0 );
        ASSERT_TRUE( text.HasSubstr( "clock" ) >= 0 );          // an expired certificate is also what a wrong clock looks like
        ASSERT_TRUE( expired.GetDescription( CORE::CString::Empty ).HasSubstr( "example.com" ) < 0 );

        CTlsError none;
        ASSERT_FALSE( none.IsError() );
        ASSERT_TRUE( none.GetDescription( "x" ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestConnection( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: the handshake completes first, ConnectedEvent follows and plaintext flows both ways" )
    try
    {
        SFakeSetup setup;
        setup.peer.echo = true;
        ASSERT_TRUE( setup.peer.Start() );

        CTlsTestClient client;
        client.m_sendOnConnected = "hello over fake tls";
        ASSERT_TRUE( Connect( setup, client ) );
        ASSERT_TRUE( PumpUntilTrue( client.m_connected, 5000 ) );
        ASSERT_TRUE( PumpUntil( PlainReached( client, 19 ), 5000 ) );

        // the order: connecting, then the handshake, then connected, then the traffic
        ASSERT_TRUE( client.m_events.StartsWith( "Connecting,HandshakeCompleted,Connected," ) );
        ASSERT_TRUE( client.m_plain == "hello over fake tls" );
        ASSERT_TRUE( setup.peer.m_received == "hello over fake tls" );
        ASSERT_TRUE( setup.peer.m_helloName == "localhost" );
        ASSERT_TRUE( 1 == client.m_sentEvents );
        ASSERT_TRUE( 19 == client.m_sentBytes );                // the plaintext size, not the one of the records

        ASSERT_TRUE( client.Connection()->IsTlsRequested() );
        ASSERT_TRUE( client.Connection()->IsEstablished() );
        ASSERT_TRUE( client.Connection()->GetSessionInfo().protocolVersion == "FAKE1" );
        ASSERT_TRUE( client.Connection()->GetBytesTransmitted( false ) > 19 );   // the wire holds more than the plaintext
        ASSERT_FALSE( client.m_gotTlsError );
        ASSERT_TRUE( 1 == setup.provider->m_sessionsCreated );
        ASSERT_TRUE( setup.provider->m_lastServerName == "localhost" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestConnectionErrors( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: every class of failure of the handshake is reported as TlsError and as SocketError with the code of the class, and no plaintext follows" )
    try
    {
        static const ETlsErrorClass classes[] = { TLSERR_CERT_UNTRUSTED, TLSERR_CERT_EXPIRED, TLSERR_CERT_NOT_YET_VALID, TLSERR_CERT_HOSTNAME_MISMATCH,
                                                  TLSERR_CERT_NO_SAN, TLSERR_PROTOCOL_ALERT, TLSERR_HANDSHAKE_FAILURE, TLSERR_PROTOCOL_VERSION, TLSERR_CERT_REVOKED, TLSERR_PIN_MISMATCH };
        for ( size_t i=0; i<sizeof( classes ) / sizeof( classes[ 0 ] ); ++i )
        {
            SFakeSetup setup;
            setup.peer.failWith = classes[ i ];
            ASSERT_TRUE( setup.peer.Start() );

            CTlsTestClient client;
            client.m_sendOnConnected = "must never be sent";
            ASSERT_TRUE( Connect( setup, client ) );
            ASSERT_TRUE( PumpUntilTrue( client.m_disconnected, 5000 ) );

            ASSERT_TRUE( client.m_gotTlsError && client.m_tlsErrorClass == classes[ i ] );
            ASSERT_TRUE( client.m_gotSocketError && client.m_socketErrorCode == GUCEF_COMCORE_TLS_ERRORCODE_BASE + (Int32) classes[ i ] );
            ASSERT_TRUE( client.Connection()->GetLastError().errorClass == classes[ i ] );
            ASSERT_TRUE( client.m_events == "Connecting,TlsError,SocketError,Disconnected," );    // the error before the socket error, no Connected, no data
            ASSERT_TRUE( client.m_plain.IsNULLOrEmpty() );
            ASSERT_TRUE( setup.peer.m_received.IsNULLOrEmpty() );
            ASSERT_FALSE( client.Connection()->IsActive() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: a failure the provider finds itself after the welcome is reported the same way" )
    try
    {
        SFakeSetup setup;
        setup.provider->m_config.clientFailsOnWelcome = TLSERR_CERT_UNTRUSTED;
        ASSERT_TRUE( setup.peer.Start() );

        CTlsTestClient client;
        ASSERT_TRUE( Connect( setup, client ) );
        ASSERT_TRUE( PumpUntilTrue( client.m_disconnected, 5000 ) );
        ASSERT_TRUE( client.m_tlsErrorClass == TLSERR_CERT_UNTRUSTED );
        ASSERT_FALSE( client.HasEvent( "Connected" ) );
        ASSERT_FALSE( client.HasEvent( "HandshakeCompleted" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: without a provider the connect fails at once with the reason, without events and without a connection" )
    try
    {
        SFakeSetup setup;
        ASSERT_TRUE( setup.peer.Start() );
        CTlsProviderRegistry empty;

        CTlsTestClient client;
        client.Connection()->SetProviderRegistry( &empty );
        CTlsSettings settings;
        ASSERT_FALSE( client.Connection()->ConnectTo( "localhost", setup.peer.GetPort(), &settings ) );
        ASSERT_TRUE( client.Connection()->GetLastError().errorClass == TLSERR_NO_PROVIDER );
        ASSERT_FALSE( client.Connection()->GetLastError().message.IsNULLOrEmpty() );
        ASSERT_FALSE( client.Connection()->GetLastError().hint.IsNULLOrEmpty() );
        ASSERT_FALSE( client.Connection()->IsActive() );
        PumpFor( 50 );
        ASSERT_TRUE( client.m_events.IsNULLOrEmpty() );
        ASSERT_TRUE( 0 == setup.peer.m_connections );

        // a provider that says it cannot run: its reason is the one that is reported
        CFakeProvider* unavailable = new CFakeProvider( "gone" );
        unavailable->m_available = false;
        unavailable->m_whyNot = "libssl.so.3: cannot open shared object file";
        empty.RegisterProvider( TITlsProviderPtr( unavailable ), 50 );
        ASSERT_FALSE( client.Connection()->ConnectTo( "localhost", setup.peer.GetPort(), &settings ) );
        ASSERT_TRUE( client.Connection()->GetLastError().message.HasSubstr( "gone: libssl.so.3: cannot open shared object file" ) >= 0 );
        ASSERT_TRUE( client.Connection()->GetLastError().GetDescription( "example.com" ).HasSubstr( "libssl.so.3" ) >= 0 );

        // and a plain connection does not care
        ASSERT_TRUE( client.Connection()->ConnectTo( "localhost", setup.peer.GetPort() ) );
        ASSERT_TRUE( PumpUntilTrue( client.m_connected, 5000 ) );
        ASSERT_FALSE( client.Connection()->IsTlsRequested() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: settings the provider cannot use fail the connection with that error" )
    try
    {
        SFakeSetup setup;
        setup.provider->m_createError = CTlsError( TLSERR_SETTINGS, "cannot load the CA file" );
        ASSERT_TRUE( setup.peer.Start() );

        CTlsTestClient client;
        ASSERT_TRUE( Connect( setup, client ) );
        ASSERT_TRUE( PumpUntilTrue( client.m_disconnected, 5000 ) );
        ASSERT_TRUE( client.m_tlsErrorClass == TLSERR_SETTINGS );
        ASSERT_FALSE( client.m_connected );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: a refused TCP connection is the error of the socket, with its code, not a TLS error code" )
    try
    {
        SFakeSetup setup;
        CORE::CString::StringVector none;
        UInt16 closedPort = 0;
        {
            CTCPServerSocket probe( false );
            ASSERT_TRUE( probe.ListenOnPort( 0 ) );
            closedPort = probe.GetBoundPort();
        }
        CTlsTestClient client;
        client.Connection()->SetProviderRegistry( &setup.registry );
        CTlsSettings settings;
        ASSERT_TRUE( client.Connection()->ConnectTo( "127.0.0.1", closedPort, &settings ) );
        ASSERT_TRUE( PumpUntilTrue( client.m_disconnected, 5000 ) );
        ASSERT_TRUE( client.m_gotTlsError && client.m_tlsErrorClass == TLSERR_CONNECT );
        ASSERT_TRUE( client.m_gotSocketError );
        ASSERT_TRUE( client.m_socketErrorCode != 0 && client.m_socketErrorCode < GUCEF_COMCORE_TLS_ERRORCODE_BASE );
        ASSERT_TRUE( 0 == setup.provider->m_sessionsCreated );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: a handshake that does not finish in time fails with a timeout, also when connecting blocking" )
    try
    {
        SFakeSetup setup;
        setup.peer.silent = true;
        ASSERT_TRUE( setup.peer.Start() );

        CTlsSettings settings;
        settings.SetHandshakeTimeoutInMs( 200 );

        UInt64 start = MT::PrecisionTickCount();
        CTlsTestClient client;
        ASSERT_TRUE( Connect( setup, client, &settings ) );
        ASSERT_TRUE( PumpUntilTrue( client.m_disconnected, 5000 ) );
        UInt64 elapsedMs = ( MT::PrecisionTickCount() - start ) * 1000 / MT::PrecisionTimerResolution();
        ASSERT_TRUE( client.m_tlsErrorClass == TLSERR_HANDSHAKE_TIMEOUT );
        ASSERT_TRUE( elapsedMs >= 180 && elapsedMs < 3000 );
        ASSERT_FALSE( client.m_connected );

        // blocking: the call itself waits and fails, the timeout is measured from the end of the TCP connect
        CTlsTestClient blocking;
        blocking.Connection()->SetProviderRegistry( &setup.registry );
        start = MT::PrecisionTickCount();
        ASSERT_FALSE( blocking.Connection()->ConnectTo( "localhost", setup.peer.GetPort(), &settings, true ) );
        elapsedMs = ( MT::PrecisionTickCount() - start ) * 1000 / MT::PrecisionTimerResolution();
        ASSERT_TRUE( blocking.m_tlsErrorClass == TLSERR_HANDSHAKE_TIMEOUT );
        ASSERT_TRUE( elapsedMs >= 180 && elapsedMs < 3000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: a peer that closes during the handshake is a failure, and one that does not speak TLS is a handshake failure" )
    try
    {
        {
            SFakeSetup setup;
            setup.peer.closeInsteadOfWelcome = true;
            ASSERT_TRUE( setup.peer.Start() );
            CTlsTestClient client;
            ASSERT_TRUE( Connect( setup, client ) );
            ASSERT_TRUE( PumpUntilTrue( client.m_disconnected, 5000 ) );
            ASSERT_TRUE( client.m_tlsErrorClass == TLSERR_PEER_CLOSED );
            ASSERT_TRUE( client.m_socketErrorCode == GUCEF_COMCORE_TLS_ERRORCODE_BASE + (Int32) TLSERR_PEER_CLOSED );
            ASSERT_FALSE( client.m_connected );
        }
        {
            SFakeSetup setup;
            setup.peer.garbageReply = true;
            ASSERT_TRUE( setup.peer.Start() );
            CTlsTestClient client;
            ASSERT_TRUE( Connect( setup, client ) );
            ASSERT_TRUE( PumpUntilTrue( client.m_disconnected, 5000 ) );
            ASSERT_TRUE( client.m_tlsErrorClass == TLSERR_HANDSHAKE_FAILURE );
            ASSERT_TRUE( client.m_plain.IsNULLOrEmpty() );          // the text of the error page was never delivered as data
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestConnectionTraffic( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: ciphertext that arrives one byte at a time gives the same result" )
    try
    {
        SFakeSetup setup;
        setup.peer.echo = true;
        setup.peer.dribble = true;
        ASSERT_TRUE( setup.peer.Start() );

        CTlsTestClient client;
        client.m_sendOnConnected = "one byte at a time";
        ASSERT_TRUE( Connect( setup, client ) );
        ASSERT_TRUE( PumpUntil( PlainReached( client, 18 ), 10000 ) );
        ASSERT_TRUE( client.m_plain == "one byte at a time" );
        ASSERT_TRUE( client.m_events.StartsWith( "Connecting,HandshakeCompleted,Connected," ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: a large send that the socket cannot take at once arrives complete and in order" )
    try
    {
        SFakeSetup setup;
        setup.peer.echo = true;
        ASSERT_TRUE( setup.peer.Start() );

        CORE::CString payload;
        const UInt32 size = 3 * 1024 * 1024;
        char* raw = new char[ size ];
        for ( UInt32 i=0; i<size; ++i )
            raw[ i ] = (char) ( 'a' + ( i * 7 + ( i >> 8 ) ) % 26 );
        payload.Set( raw, size );

        CTlsTestClient client;
        client.m_sendOnConnected = payload;
        ASSERT_TRUE( Connect( setup, client ) );
        ASSERT_TRUE( PumpUntil( PlainReached( client, size ), 30000 ) );
        ASSERT_TRUE( size == client.m_plain.Length() );
        ASSERT_TRUE( 0 == memcmp( raw, client.m_plain.C_String(), size ) );
        ASSERT_TRUE( size == setup.peer.m_received.Length() );
        ASSERT_TRUE( 0 == memcmp( raw, setup.peer.m_received.C_String(), size ) );
        ASSERT_TRUE( 1 == client.m_sentEvents );
        ASSERT_TRUE( size == client.m_sentBytes );
        delete[] raw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: data that comes with the welcome is delivered after ConnectedEvent, and sending before it is refused" )
    try
    {
        SFakeSetup setup;
        setup.peer.dataWithWelcome = true;
        ASSERT_TRUE( setup.peer.Start() );

        CTlsTestClient client;
        ASSERT_TRUE( Connect( setup, client ) );
        ASSERT_FALSE( client.Connection()->Send( "too early", 9 ) );      // not even connected yet
        ASSERT_TRUE( PumpUntil( PlainReached( client, 8 ), 5000 ) );
        ASSERT_TRUE( client.m_plain == "greeting" );
        ASSERT_TRUE( client.m_events.StartsWith( "Connecting,HandshakeCompleted,Connected,Data," ) );
        ASSERT_TRUE( client.m_events.HasSubstr( "Sent," ) < 0 );          // the refused send was not told as sent
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: the server name of the settings is what the provider is given, the host otherwise" )
    try
    {
        SFakeSetup setup;
        ASSERT_TRUE( setup.peer.Start() );

        CTlsSettings settings;
        settings.SetServerName( "sni.example" );
        settings.AddAlpnProtocol( "http/1.1" );
        settings.SetSessionCacheSize( 3 );
        CTlsTestClient client;
        ASSERT_TRUE( Connect( setup, client, &settings, "127.0.0.1" ) );
        ASSERT_TRUE( PumpUntilTrue( client.m_connected, 5000 ) );
        ASSERT_TRUE( setup.provider->m_lastServerName == "sni.example" );
        ASSERT_TRUE( setup.peer.m_helloName == "sni.example" );
        ASSERT_TRUE( 1 == setup.provider->m_lastSettings.GetAlpnProtocols().size() );
        ASSERT_TRUE( 3 == setup.provider->m_lastSettings.GetSessionCacheSize() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: without settings it is plain TCP and the events of the socket pass" )
    try
    {
        SFakeSetup setup;
        setup.peer.plainUpgrade = true;
        ASSERT_TRUE( setup.peer.Start() );

        CTlsTestClient client;
        client.m_sendOnConnected = "UPGRADE\n";
        ASSERT_TRUE( client.Connection()->ConnectTo( "localhost", setup.peer.GetPort() ) );
        ASSERT_TRUE( PumpUntil( PlainReached( client, 3 ), 5000 ) );
        ASSERT_TRUE( client.m_plain == "OK\n" );
        ASSERT_FALSE( client.Connection()->IsTlsRequested() );
        ASSERT_TRUE( client.m_events == "Connecting,Connected,Sent,Data," );
        ASSERT_TRUE( setup.peer.m_plainLines == "UPGRADE\n" );
        ASSERT_TRUE( 8 == client.m_sentBytes );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestConnectionClose( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: a close sends the close notification before the TCP connection goes, a second close does nothing" )
    try
    {
        SFakeSetup setup;
        ASSERT_TRUE( setup.peer.Start() );

        CTlsTestClient client;
        client.m_sendOnConnected = "x";
        ASSERT_TRUE( Connect( setup, client ) );
        ASSERT_TRUE( PumpUntil( PeerData( setup.peer, 1 ), 5000 ) );

        client.Connection()->Close();
        client.Connection()->Close();
        ASSERT_TRUE( PumpUntil( PeerDisconnects( setup.peer, 1 ), 5000 ) );
        ASSERT_TRUE( setup.peer.m_events == "hello,data,close,disconnect," );
        ASSERT_TRUE( client.m_disconnected );
        ASSERT_FALSE( client.m_gotTlsError );
        ASSERT_FALSE( client.m_gotSocketError );
        ASSERT_FALSE( client.Connection()->Send( "after close", 11 ) );
        ASSERT_FALSE( client.Connection()->IsEstablished() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: a peer that closes the TCP connection without close notification leaves the truncation flag, one that says goodbye does not" )
    try
    {
        {
            SFakeSetup setup;
            setup.peer.closeAfterWelcome = true;
            ASSERT_TRUE( setup.peer.Start() );
            CTlsTestClient client;
            ASSERT_TRUE( Connect( setup, client ) );
            ASSERT_TRUE( PumpUntilTrue( client.m_disconnected, 5000 ) );
            ASSERT_TRUE( client.m_connected );                              // it was a connection, then it ended
            ASSERT_FALSE( client.Connection()->GetSessionInfo().peerClosedCleanly );
            ASSERT_FALSE( client.m_gotTlsError );                           // not an error of the handshake: the consumer decides what truncation means for it
        }
        {
            SFakeSetup setup;
            ASSERT_TRUE( setup.peer.Start() );
            CTlsTestClient client;
            ASSERT_TRUE( Connect( setup, client ) );
            ASSERT_TRUE( PumpUntilTrue( client.m_connected, 5000 ) );
            setup.peer.CloseGracefully();
            ASSERT_TRUE( PumpUntilTrue( client.m_disconnected, 5000 ) );
            ASSERT_TRUE( client.Connection()->GetSessionInfo().peerClosedCleanly );
            ASSERT_FALSE( client.m_gotTlsError );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: the connection can be used again after it ended" )
    try
    {
        SFakeSetup setup;
        setup.peer.echo = true;
        ASSERT_TRUE( setup.peer.Start() );

        CTlsTestClient client;
        client.m_sendOnConnected = "again";
        ASSERT_TRUE( Connect( setup, client ) );
        ASSERT_TRUE( PumpUntil( PlainReached( client, 5 ), 5000 ) );
        client.Connection()->Close();
        ASSERT_TRUE( PumpUntil( PeerDisconnects( setup.peer, 1 ), 5000 ) );

        client.m_plain.Clear();
        ASSERT_TRUE( Connect( setup, client ) );
        ASSERT_TRUE( PumpUntil( PlainReached( client, 5 ), 5000 ) );
        ASSERT_TRUE( client.m_plain == "again" );
        ASSERT_TRUE( 2 == setup.provider->m_sessionsCreated );
        ASSERT_TRUE( client.Connection()->IsEstablished() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: an observer that deletes the connection from inside any of its events leaves nothing behind that is touched" )
    try
    {
        static const char* names[] = { "HandshakeCompleted", "Connected", "Data", "Sent", "Disconnected", "TlsError", "SocketError" };
        for ( size_t i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
        {
            SFakeSetup setup;
            setup.peer.echo = true;
            setup.peer.dataWithWelcome = true;
            const CORE::CString eventName = names[ i ];
            if ( eventName == "TlsError" || eventName == "SocketError" )
                setup.peer.failWith = TLSERR_CERT_UNTRUSTED;
            if ( eventName == "Disconnected" )
                setup.peer.closeAfterWelcome = true;
            ASSERT_TRUE( setup.peer.Start() );

            {
                CTlsTestClient client;
                client.m_deleteOn = eventName;
                client.m_sendOnConnected = "bye";
                ASSERT_TRUE( Connect( setup, client ) );
                PumpUntilTrue( client.m_deleted, 3000 );
                ASSERT_TRUE( client.m_deleted );
                PumpFor( 100 );             // whatever the connection had in flight must find nobody to talk to
            }
            PumpFor( 50 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestConnectionUpgrade( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: StartTls upgrades a connected plain connection, ConnectedEvent is not raised again" )
    try
    {
        SFakeSetup setup;
        setup.peer.plainUpgrade = true;
        setup.peer.echo = true;
        ASSERT_TRUE( setup.peer.Start() );

        CTlsSettings settings;
        CTlsTestClient client;
        client.Connection()->SetProviderRegistry( &setup.registry );
        client.m_sendOnConnected = "UPGRADE\n";
        client.m_upgradeSettings = &settings;
        client.m_upgradeName = "upgrade.example";
        ASSERT_TRUE( client.Connection()->ConnectTo( "127.0.0.1", setup.peer.GetPort() ) );
        ASSERT_TRUE( PumpUntilTrue( client.m_upgradeStarted, 5000 ) );
        ASSERT_TRUE( client.m_upgradeResult );
        ASSERT_TRUE( PumpUntil( HandshakeDone( client ), 5000 ) );

        ASSERT_TRUE( client.Connection()->IsEstablished() );
        ASSERT_TRUE( setup.peer.m_helloName == "upgrade.example" );
        ASSERT_TRUE( client.m_events == "Connecting,Connected,Sent,Data,HandshakeCompleted," );   // one Connected, from the plain stage

        const char text[] = "after the upgrade";
        ASSERT_TRUE( client.Connection()->Send( text, sizeof( text ) - 1 ) );
        ASSERT_TRUE( PumpUntil( PlainReached( client, sizeof( text ) - 1 ), 5000 ) );
        ASSERT_TRUE( client.m_plain == "after the upgrade" );
        ASSERT_TRUE( setup.peer.m_received == "after the upgrade" );
        ASSERT_TRUE( setup.peer.m_plainLines == "UPGRADE\n" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: ciphertext that was read as part of the plain stage is handed to StartTls and used" )
    try
    {
        SFakeSetup setup;
        setup.peer.plainUpgrade = true;
        setup.peer.welcomeWithUpgradeAnswer = true;     // the welcome comes in the same write as "OK": the plain stage reads it
        ASSERT_TRUE( setup.peer.Start() );

        CTlsSettings settings;
        CTlsTestClient client;
        client.Connection()->SetProviderRegistry( &setup.registry );
        client.m_sendOnConnected = "UPGRADE\n";
        client.m_upgradeSettings = &settings;
        client.m_upgradeName = "leftover.example";
        ASSERT_TRUE( client.Connection()->ConnectTo( "127.0.0.1", setup.peer.GetPort() ) );
        ASSERT_TRUE( PumpUntilTrue( client.m_upgradeStarted, 5000 ) );
        ASSERT_TRUE( PumpUntil( HandshakeDone( client ), 5000 ) );
        ASSERT_TRUE( client.Connection()->IsEstablished() );
        ASSERT_FALSE( client.m_gotTlsError );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: StartTls is refused without a connection, on a connection that is already protected, and without a provider" )
    try
    {
        SFakeSetup setup;
        ASSERT_TRUE( setup.peer.Start() );
        CTlsSettings settings;

        CTlsTestClient client;
        client.Connection()->SetProviderRegistry( &setup.registry );
        ASSERT_FALSE( client.Connection()->StartTls( settings, "x" ) );         // nothing connected

        ASSERT_TRUE( Connect( setup, client ) );
        ASSERT_TRUE( PumpUntilTrue( client.m_connected, 5000 ) );
        ASSERT_FALSE( client.Connection()->StartTls( settings, "x" ) );         // already TLS

        CTlsTestClient plain;
        CTlsProviderRegistry empty;
        plain.Connection()->SetProviderRegistry( &empty );
        ASSERT_TRUE( plain.Connection()->ConnectTo( "127.0.0.1", setup.peer.GetPort() ) );
        ASSERT_TRUE( PumpUntilTrue( plain.m_connected, 5000 ) );
        ASSERT_FALSE( plain.Connection()->StartTls( settings, "x" ) );          // no provider
        ASSERT_TRUE( plain.Connection()->GetLastError().errorClass == TLSERR_NO_PROVIDER );
        ASSERT_TRUE( plain.Connection()->IsActive() );                          // the plain connection is left as it was
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestInsecureWarning( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "TlsClientConnection: every connection without verification logs a warning, and one with verification does not" )
    try
    {
        SFakeSetup setup;
        ASSERT_TRUE( setup.peer.Start() );

        TESTFW::CLogTextCapture capture;

        CTlsSettings insecure;
        insecure.DisableVerificationInsecurely();
        for ( int i=0; i<2; ++i )
        {
            CTlsTestClient client;
            ASSERT_TRUE( Connect( setup, client, &insecure ) );
            ASSERT_TRUE( PumpUntilTrue( client.m_connected, 5000 ) );
            ASSERT_TRUE( setup.provider->m_lastSettings.IsVerificationEnabled() == false );
        }
        CORE::CString text = capture.GetText();
        ASSERT_TRUE( text.HasSubstr( "TLS verification is DISABLED for localhost:" + CORE::ToString( setup.peer.GetPort() ) ) >= 0 );
        // once per connection
        Int32 first = text.HasSubstr( "TLS verification is DISABLED" );
        ASSERT_TRUE( first >= 0 && text.HasSubstr( "TLS verification is DISABLED", first + 1 ) > first );

        TESTFW::CLogTextCapture secure;
        CTlsTestClient client;
        ASSERT_TRUE( Connect( setup, client ) );
        ASSERT_TRUE( PumpUntilTrue( client.m_connected, 5000 ) );
        ASSERT_TRUE( secure.GetText().HasSubstr( "DISABLED" ) < 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformTlsLayerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "TlsLayer" );

    TestSettings();
    TestRegistry();
    TestErrorTypes();
    TestConnection();
    TestConnectionErrors();
    TestConnectionTraffic();
    TestConnectionClose();
    TestConnectionUpgrade();
    TestInsecureWarning();
}

/*-------------------------------------------------------------------------*/
