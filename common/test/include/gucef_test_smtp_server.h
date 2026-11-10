/*
 *  gucef common header: header-only fake SMTP server for tests
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

/*
 *  Test support: an SMTP server on a local port (see gucef_test_tcp_server.h) that plays the protocol of RFC 5321 with the
 *  extensions that a client uses: EHLO and HELO, STARTTLS (and TLS from the first byte), AUTH PLAIN, LOGIN, XOAUTH2 and
 *  SCRAM-SHA-256, SIZE, PIPELINING, 8BITMIME and SMTPUTF8, per recipient answers and the transparency of the data. It keeps
 *  what it received. The members are the knobs: set them before Start(). Hostile modes make it misbehave on purpose.
 *
 *    GUCEF::TESTFW::CTestSmtpServer server;
 *    server.authMechanisms = "PLAIN";
 *    server.Start();
 *    ... connect a client to server.GetPort() ...
 *    ASSERT_TRUE( 1 == server.GetMessages().size() );
 */

#ifndef GUCEF_TEST_SMTP_SERVER_H
#define GUCEF_TEST_SMTP_SERVER_H

#ifdef __cplusplus

#ifndef GUCEF_TEST_TCP_SERVER_H
#include "gucef_test_tcp_server.h"
#define GUCEF_TEST_TCP_SERVER_H
#endif /* GUCEF_TEST_TCP_SERVER_H ? */

#ifndef GUCEF_TEST_MAIL_AUTH_H
#include "gucef_test_mail_auth.h"
#define GUCEF_TEST_MAIL_AUTH_H
#endif /* GUCEF_TEST_MAIL_AUTH_H ? */

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------*/

class CTestSmtpServer;

/**
 *  Told about every message an SMTP server accepted, on the thread of the connection, before the server answers the end of
 *  the data: a mail world routes the message to the mailboxes of the recipients.
 */
class CTestSmtpDeliverySink
{
    public:

    virtual ~CTestSmtpDeliverySink() {}

    virtual void OnSmtpAccepted( CTestSmtpServer* origin, const CORE::CString& mailFrom, const GUCEF::vector< CORE::CString >& recipients, const CORE::CString& data ) = 0;
};

class CTestSmtpServer : public CTestTcpServer
{
    public:

    /** A message that the server accepted */
    struct SReceived
    {
        CORE::CString mailFrom;
        CORE::CString mailParameters;           /**< what followed the address: SIZE=.. BODY=8BITMIME SMTPUTF8 */
        GUCEF::vector< CORE::CString > recipients;
        CORE::CString data;                     /**< unstuffed, as received */
    };
    typedef struct SReceived TReceived;

    enum EHostile
    {
        HOSTILE_NONE = 0,
        HOSTILE_GIANT_GREETING,         /**< a greeting line of 2 MiB that does not end */
        HOSTILE_ENDLESS_EHLO,           /**< a multi line reply that never ends */
        HOSTILE_BAD_GREETING,           /**< a greeting that is not SMTP */
        HOSTILE_INJECT_AFTER_STARTTLS,  /**< a reply in plain text in the same write as the 220 of STARTTLS */
        HOSTILE_NO_GREETING,            /**< the connection is open and silent */
        HOSTILE_DROP_AFTER_EHLO,        /**< the connection is closed after the answer to EHLO */
        HOSTILE_MIXED_CODES,            /**< a multi line reply with different codes */
        HOSTILE_UNSOLICITED_REPLY       /**< a reply that nobody asked for after the greeting */
    };

    CORE::CString greeting;
    bool supportEhlo;
    bool offerStartTls;
    bool authOnlyAfterTls;
    CORE::CString authMechanisms;       /**< "PLAIN LOGIN XOAUTH2 SCRAM-SHA-256", empty: no AUTH extension */
    CORE::CString user;
    CORE::CString password;
    CORE::CString token;
    bool requireAuth;
    bool pipelining;
    bool eightBitMime;
    bool smtpUtf8;
    CORE::UInt32 sizeLimit;             /**< announced as SIZE and enforced, 0: none */
    GUCEF::set< CORE::CString > rejectRecipients;           /**< 550 */
    GUCEF::set< CORE::CString > tempRejectRecipients;       /**< 450 */
    bool rejectSender;
    CORE::UInt32 dataReplyCode;
    CORE::CString dataReplyText;
    CORE::UInt32 replyDelayInMs;        /**< before every reply */
    CORE::UInt32 greetingDelayInMs;
    CORE::UInt32 dropAfterCommands;     /**< the connection is closed after this many commands, 0: never */
    CORE::UInt32 dataDelayInMs;         /**< before the reply to the end of the data */
    EHostile hostile;
    CTestSmtpDeliverySink* deliverySink;    /**< not owned, may be NULL */

    CTestSmtpServer( void )
        : CTestTcpServer()
        , greeting( "mail.test ESMTP ready" )
        , supportEhlo( true )
        , offerStartTls( false )
        , authOnlyAfterTls( false )
        , authMechanisms()
        , user( "alice" )
        , password( "s3cret-p@ss" )
        , token( "ya29.token-of-alice" )
        , requireAuth( false )
        , pipelining( true )
        , eightBitMime( true )
        , smtpUtf8( false )
        , sizeLimit( 0 )
        , rejectRecipients()
        , tempRejectRecipients()
        , rejectSender( false )
        , dataReplyCode( 250 )
        , dataReplyText( "2.0.0 Ok: queued as 4F2A1" )
        , replyDelayInMs( 0 )
        , greetingDelayInMs( 0 )
        , dropAfterCommands( 0 )
        , dataDelayInMs( 0 )
        , hostile( HOSTILE_NONE )
        , deliverySink( NULL )
        , m_received()
        , m_commands()
        , m_authAttempts( 0 )
        , m_authenticatedSessions( 0 )
        , m_lock()
    {
    }

    virtual ~CTestSmtpServer()
    {
        Stop();
    }

    GUCEF::vector< TReceived > GetMessages( void ) { MT::CScopeMutex lock( m_lock ); return m_received; }

    /** The commands received, including the lines of the authentication exchange as they were sent: for tests that look at what went over the wire */
    GUCEF::vector< CORE::CString > GetCommands( void ) { MT::CScopeMutex lock( m_lock ); return m_commands; }

    CORE::UInt32 GetAuthAttempts( void ) { MT::CScopeMutex lock( m_lock ); return m_authAttempts; }

    CORE::UInt32 GetAuthenticatedSessions( void ) { MT::CScopeMutex lock( m_lock ); return m_authenticatedSessions; }

    /** Waits, without a fixed sleep, until a command that starts with 'prefix' was received (a QUIT after the close of the client, ...), or the time is up */
    bool WaitForCommand( const char* prefix, CORE::UInt32 timeoutInMs )
    {
        return WaitFor( CHasCommand( this, prefix ), timeoutInMs );
    }

    private:

    struct CHasCommand
    {
        CHasCommand( CTestSmtpServer* server, const char* prefix ) : m_server( server ), m_prefix( prefix ) {}
        bool operator()( void ) const
        {
            const GUCEF::vector< CORE::CString > commands = m_server->GetCommands();
            for ( size_t i=0; i<commands.size(); ++i )
                if ( commands[ i ].StartsWith( m_prefix ) )
                    return true;
            return false;
        }
        CTestSmtpServer* m_server;
        const char* m_prefix;
    };

    public:

    protected:

    virtual void Serve( CTestServerConnection& c ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( HOSTILE_NO_GREETING == hostile )
        {
            c.Sleep( 60000 );
            return;
        }
        c.Sleep( greetingDelayInMs );
        if ( HOSTILE_GIANT_GREETING == hostile )
        {
            // a line of 2 MiB that does not end
            c.WriteText( "220 " );
            GUCEF::vector< char > filler( 65536, 'x' );
            for ( int i=0; i<32 && !c.IsClosed(); ++i )
                c.Write( &filler[ 0 ], (CORE::UInt32) filler.size() );
            c.Sleep( 5000 );
            return;
        }
        if ( HOSTILE_BAD_GREETING == hostile )
        {
            c.WriteLine( "HTTP/1.1 400 Bad Request" );
            c.Sleep( 2000 );
            return;
        }
        c.WriteLine( "220 " + greeting );
        if ( HOSTILE_UNSOLICITED_REPLY == hostile )
            c.WriteLine( "250 nobody asked" );

        bool tls = c.IsTls();
        bool authenticated = false;
        CORE::CString mailFrom, mailParameters;
        GUCEF::vector< CORE::CString > recipients;
        bool haveMail = false;
        CORE::UInt32 commandCount = 0;
        CORE::CString line;

        while ( c.ReadLine( line, 20000 ) )
        {
            {
                MT::CScopeMutex lock( m_lock );
                m_commands.push_back( line );
            }
            SignalChanged();
            ++commandCount;
            if ( dropAfterCommands > 0 && commandCount > dropAfterCommands )
                return;
            c.Sleep( replyDelayInMs );

            const CORE::CString upper = line.Uppercase();
            if ( upper.StartsWith( "EHLO" ) || upper.StartsWith( "HELO" ) )
            {
                haveMail = false;
                if ( upper.StartsWith( "EHLO" ) && !supportEhlo )
                {
                    c.WriteLine( "502 5.5.1 EHLO not supported" );
                    continue;
                }
                if ( upper.StartsWith( "HELO" ) )
                {
                    c.WriteLine( "250 " + greeting );
                    continue;
                }
                if ( HOSTILE_MIXED_CODES == hostile )
                {
                    c.WriteText( "250-first\r\n251 second\r\n" );
                    continue;
                }
                GUCEF::vector< CORE::CString > extensions;
                if ( pipelining ) extensions.push_back( "PIPELINING" );
                if ( eightBitMime ) extensions.push_back( "8BITMIME" );
                if ( smtpUtf8 ) extensions.push_back( "SMTPUTF8" );
                extensions.push_back( "ENHANCEDSTATUSCODES" );
                if ( sizeLimit > 0 ) extensions.push_back( "SIZE " + CORE::UInt32ToString( sizeLimit ) );
                if ( offerStartTls && !tls ) extensions.push_back( "STARTTLS" );
                if ( !authMechanisms.IsNULLOrEmpty() && ( !authOnlyAfterTls || tls ) )
                    extensions.push_back( "AUTH " + authMechanisms );

                c.WriteText( ( extensions.empty() ? "250 " : "250-" ) + greeting + "\r\n" );
                for ( size_t i=0; i<extensions.size(); ++i )
                    c.WriteText( ( i + 1 < extensions.size() ? "250-" : "250 " ) + extensions[ i ] + "\r\n" );
                if ( HOSTILE_ENDLESS_EHLO == hostile )
                {
                    for ( int i=0; i<5000 && !c.IsClosed(); ++i )
                        c.WriteLine( "250-X-FILLER-" + CORE::Int32ToString( i ) + " more and more" );
                    c.Sleep( 2000 );
                    return;
                }
                if ( HOSTILE_DROP_AFTER_EHLO == hostile )
                    return;
            }
            else
            if ( upper.StartsWith( "STARTTLS" ) )
            {
                if ( !offerStartTls || tls || !IsTlsAvailable() )
                {
                    c.WriteLine( "502 5.5.1 STARTTLS not available" );
                    continue;
                }
                if ( HOSTILE_INJECT_AFTER_STARTTLS == hostile )
                    c.WriteText( "220 2.0.0 Ready to start TLS\r\n250 2.0.0 injected in plain text\r\n" );
                else
                    c.WriteLine( "220 2.0.0 Ready to start TLS" );
                if ( !c.StartTls() )
                    return;
                tls = true;
                haveMail = false;
                authenticated = false;
            }
            else
            if ( upper.StartsWith( "AUTH" ) )
            {
                if ( authenticated )
                {
                    c.WriteLine( "503 5.5.1 Already authenticated" );
                    continue;
                }
                if ( authMechanisms.IsNULLOrEmpty() || ( authOnlyAfterTls && !tls ) )
                {
                    c.WriteLine( "503 5.5.1 AUTH not available" );
                    continue;
                }
                {
                    MT::CScopeMutex lock( m_lock );
                    ++m_authAttempts;
                    // the credentials of the exchange are not kept in the log of commands
                    m_commands.back() = upper.StartsWith( "AUTH" ) ? CORE::CString( "AUTH " ) + upper.SubstrFromRange( 5, upper.Length() ).SubstrToChar( ' ', true ) : line;
                }
                authenticated = Authenticate( c, line );
                if ( authenticated )
                {
                    MT::CScopeMutex lock( m_lock );
                    ++m_authenticatedSessions;
                }
            }
            else
            if ( upper.StartsWith( "MAIL FROM:" ) )
            {
                if ( requireAuth && !authenticated )
                {
                    c.WriteLine( "530 5.7.0 Authentication required" );
                    continue;
                }
                if ( haveMail )
                {
                    c.WriteLine( "503 5.5.1 Nested MAIL command" );
                    continue;
                }
                CORE::CString rest = line.SubstrFromRange( 10, line.Length() ).Trim( true );
                const Int32 open = rest.HasChar( '<' );
                const Int32 close = rest.HasChar( '>' );
                if ( open != 0 || close < 0 )
                {
                    c.WriteLine( "501 5.5.4 Syntax error in the address" );
                    continue;
                }
                mailFrom = rest.SubstrFromRange( 1, (CORE::UInt32) close );
                mailParameters = rest.SubstrFromRange( (CORE::UInt32) close + 1, rest.Length() ).Trim( true ).Trim( false );
                if ( rejectSender )
                {
                    c.WriteLine( "550 5.7.1 Sender refused" );
                    continue;
                }
                if ( sizeLimit > 0 && mailParameters.HasSubstr( "SIZE=" ) >= 0 )
                {
                    const CORE::CString sizeText = mailParameters.SubstrFromRange( (CORE::UInt32) mailParameters.HasSubstr( "SIZE=" ) + 5, mailParameters.Length() );
                    if ( (CORE::UInt32) atol( sizeText.C_String() ) > sizeLimit )
                    {
                        c.WriteLine( "552 5.3.4 Message size exceeds the limit" );
                        continue;
                    }
                }
                haveMail = true;
                recipients.clear();
                c.WriteLine( "250 2.1.0 Sender ok" );
            }
            else
            if ( upper.StartsWith( "RCPT TO:" ) )
            {
                if ( !haveMail )
                {
                    c.WriteLine( "503 5.5.1 MAIL first" );
                    continue;
                }
                CORE::CString rest = line.SubstrFromRange( 8, line.Length() ).Trim( true );
                const Int32 close = rest.HasChar( '>' );
                if ( !rest.StartsWith( "<" ) || close < 0 )
                {
                    c.WriteLine( "501 5.1.3 Syntax error in the address" );
                    continue;
                }
                const CORE::CString address = rest.SubstrFromRange( 1, (CORE::UInt32) close );
                if ( rejectRecipients.find( address ) != rejectRecipients.end() )
                {
                    c.WriteLine( "550 5.1.1 No such user here" );
                    continue;
                }
                if ( tempRejectRecipients.find( address ) != tempRejectRecipients.end() )
                {
                    c.WriteLine( "450 4.2.0 Mailbox busy, try again later" );
                    continue;
                }
                recipients.push_back( address );
                c.WriteLine( "250 2.1.5 Recipient ok" );
            }
            else
            if ( upper == "DATA" )
            {
                if ( !haveMail || recipients.empty() )
                {
                    c.WriteLine( "503 5.5.1 No valid recipients" );
                    continue;
                }
                c.WriteLine( "354 End data with <CR><LF>.<CR><LF>" );
                CORE::CDynamicBuffer data;
                if ( !c.ReadDotTerminated( data, 30000 ) )
                    return;
                c.Sleep( dataDelayInMs );
                if ( sizeLimit > 0 && data.GetDataSize() > sizeLimit )
                {
                    c.WriteLine( "552 5.3.4 Message too big" );
                    haveMail = false;
                    continue;
                }
                if ( dataReplyCode >= 400 )
                {
                    c.WriteLine( CORE::UInt32ToString( dataReplyCode ) + " " + dataReplyText );
                    haveMail = false;
                    continue;
                }
                TReceived received;
                received.mailFrom = mailFrom;
                received.mailParameters = mailParameters;
                received.recipients = recipients;
                if ( data.GetDataSize() > 0 )
                    received.data = CORE::CString( static_cast< const char* >( data.GetConstBufferPtr() ), data.GetDataSize() );
                {
                    MT::CScopeMutex lock( m_lock );
                    m_received.push_back( received );
                }
                if ( NULL != deliverySink )
                    deliverySink->OnSmtpAccepted( this, received.mailFrom, received.recipients, received.data );
                haveMail = false;
                c.WriteLine( CORE::UInt32ToString( dataReplyCode ) + " " + dataReplyText );
            }
            else
            if ( upper == "RSET" )
            {
                haveMail = false;
                recipients.clear();
                c.WriteLine( "250 2.0.0 Reset" );
            }
            else
            if ( upper == "NOOP" )
                c.WriteLine( "250 2.0.0 Ok" );
            else
            if ( upper == "QUIT" )
            {
                c.WriteLine( "221 2.0.0 Bye" );
                return;
            }
            else
                c.WriteLine( "500 5.5.2 Command not recognized" );
        }
    }

    private:

    bool Authenticate( CTestServerConnection& c, const CORE::CString& line )
    {
        CORE::CString::StringVector words = line.ParseElements( ' ', false );
        if ( words.size() < 2 )
        {
            c.WriteLine( "501 5.5.4 No mechanism" );
            return false;
        }
        const CORE::CString mechanism = words[ 1 ].Uppercase();
        CORE::CString initial = words.size() > 2 ? words[ 2 ] : CORE::CString();
        CORE::CString reply;

        if ( mechanism == "PLAIN" )
        {
            if ( initial.IsNULLOrEmpty() )
            {
                c.WriteLine( "334 " );
                if ( !c.ReadLine( initial, 10000 ) )
                    return false;
            }
            CORE::CString authzid, name, secret;
            if ( initial != "*" && SplitPlain( initial, authzid, name, secret ) && name == user && secret == password )
                return Succeed( c );
            return Fail( c );
        }
        if ( mechanism == "LOGIN" )
        {
            c.WriteLine( "334 VXNlcm5hbWU6" );
            CORE::CString name, secret;
            if ( !c.ReadLine( name, 10000 ) )
                return false;
            c.WriteLine( "334 UGFzc3dvcmQ6" );
            if ( !c.ReadLine( secret, 10000 ) )
                return false;
            if ( DecodeBase64Text( name ) == user && DecodeBase64Text( secret ) == password )
                return Succeed( c );
            return Fail( c );
        }
        if ( mechanism == "XOAUTH2" )
        {
            if ( initial.IsNULLOrEmpty() )
            {
                c.WriteLine( "334 " );
                if ( !c.ReadLine( initial, 10000 ) )
                    return false;
            }
            CORE::CString name, presented;
            if ( SplitXOAuth2( initial, name, presented ) && name == user && presented == token )
                return Succeed( c );
            // the failure is a challenge with JSON in it, answered by the client with an empty line
            const CORE::CString json = "{\"status\":\"401\",\"schemes\":\"Bearer\",\"scope\":\"https://mail.test/\"}";
            c.WriteLine( "334 " + CORE::Base64Encode( json.C_String(), json.ByteSize() - 1 ) );
            CORE::CString ignored;
            c.ReadLine( ignored, 10000 );
            return Fail( c );
        }
        if ( mechanism == "SCRAM-SHA-256" )
        {
            CTestScramServer scram( user, password );
            if ( initial.IsNULLOrEmpty() )
            {
                c.WriteLine( "334 " );
                if ( !c.ReadLine( initial, 10000 ) )
                    return false;
            }
            CORE::CString serverFirst, serverFinal, clientFinal;
            if ( !scram.ProcessFirst( DecodeBase64Text( initial ), serverFirst ) )
                return Fail( c );
            c.WriteLine( "334 " + CORE::Base64Encode( serverFirst.C_String(), serverFirst.ByteSize() - 1 ) );
            if ( !c.ReadLine( clientFinal, 10000 ) )
                return false;
            if ( !scram.ProcessFinal( DecodeBase64Text( clientFinal ), serverFinal ) )
                return Fail( c );
            c.WriteLine( "334 " + CORE::Base64Encode( serverFinal.C_String(), serverFinal.ByteSize() - 1 ) );
            CORE::CString ignored;
            if ( !c.ReadLine( ignored, 10000 ) )
                return false;
            return Succeed( c );
        }
        c.WriteLine( "504 5.5.4 Mechanism not supported" );
        return false;
    }

    bool Succeed( CTestServerConnection& c )
    {
        c.WriteLine( "235 2.7.0 Authentication successful" );
        return true;
    }

    bool Fail( CTestServerConnection& c )
    {
        c.WriteLine( "535 5.7.8 Authentication credentials invalid" );
        return false;
    }

    private:

    GUCEF::vector< TReceived > m_received;
    GUCEF::vector< CORE::CString > m_commands;
    CORE::UInt32 m_authAttempts;
    CORE::UInt32 m_authenticatedSessions;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

} /* namespace TESTFW */
} /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_SMTP_SERVER_H ? */
