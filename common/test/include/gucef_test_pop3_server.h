/*
 *  gucef common header: header-only fake POP3 server for tests
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
 *  Test support: a POP3 server on a local port (see gucef_test_tcp_server.h): the transaction of RFC 1939 (STAT, LIST, UIDL,
 *  RETR, TOP, DELE, RSET, NOOP, QUIT with the update of the maildrop at QUIT), CAPA, STLS, USER and PASS, APOP and AUTH with
 *  PLAIN, LOGIN, XOAUTH2 and SCRAM-SHA-256, dot stuffing of the messages. Hostile modes make it misbehave on purpose.
 */

#ifndef GUCEF_TEST_POP3_SERVER_H
#define GUCEF_TEST_POP3_SERVER_H

#ifdef __cplusplus

#ifndef GUCEF_TEST_TCP_SERVER_H
#include "gucef_test_tcp_server.h"
#define GUCEF_TEST_TCP_SERVER_H
#endif /* GUCEF_TEST_TCP_SERVER_H ? */

#ifndef GUCEF_TEST_MAIL_AUTH_H
#include "gucef_test_mail_auth.h"
#define GUCEF_TEST_MAIL_AUTH_H
#endif /* GUCEF_TEST_MAIL_AUTH_H ? */

#ifndef GUCEF_CORE_DVMD5UTILS_H
#include "dvmd5utils.h"
#define GUCEF_CORE_DVMD5UTILS_H
#endif /* GUCEF_CORE_DVMD5UTILS_H ? */

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------*/

class CTestPop3Server : public CTestTcpServer
{
    public:

    struct SMessage
    {
        CORE::CString uid;
        CORE::CString data;             /**< lines ended with CR LF, the way the message is stored */
    };
    typedef struct SMessage TMessage;

    enum EHostile
    {
        HOSTILE_NONE = 0,
        HOSTILE_BAD_GREETING,           /**< a line that is no POP3 response */
        HOSTILE_ENDLESS_RETR,           /**< RETR that never ends */
        HOSTILE_GIANT_STATUS,           /**< a status line of 2 MiB */
        HOSTILE_BAD_UIDL,               /**< a UIDL entry that is not valid */
        HOSTILE_DROP_DURING_RETR,       /**< the connection ends in the middle of a message */
        HOSTILE_INJECT_AFTER_STLS,      /**< text in plain in the write of the answer to STLS */
        HOSTILE_ENDLESS_LIST            /**< a multi line list that never ends */
    };

    CORE::CString greeting;
    CORE::CString apopTimestamp;        /**< like <1896.697170952@pop.test>, empty: no APOP */
    bool supportCapa;
    bool supportUidl;
    bool supportTop;
    bool offerStls;
    bool allowUserPass;
    bool saslOnlyAfterTls;
    CORE::CString authMechanisms;       /**< "PLAIN XOAUTH2 SCRAM-SHA-256", empty: none */
    CORE::CString user;
    CORE::CString password;
    CORE::CString token;
    GUCEF::vector< TMessage > messages;
    CORE::UInt32 replyDelayInMs;
    CORE::UInt32 dropAfterCommands;
    EHostile hostile;

    CTestPop3Server( void )
        : CTestTcpServer()
        , greeting( "POP3 server ready" )
        , apopTimestamp()
        , supportCapa( true )
        , supportUidl( true )
        , supportTop( true )
        , offerStls( false )
        , allowUserPass( true )
        , saslOnlyAfterTls( false )
        , authMechanisms()
        , user( "alice" )
        , password( "s3cret-p@ss" )
        , token( "ya29.token-of-alice" )
        , messages()
        , replyDelayInMs( 0 )
        , dropAfterCommands( 0 )
        , hostile( HOSTILE_NONE )
        , m_commands()
        , m_committedDeletes( 0 )
        , m_lock()
    {
    }

    virtual ~CTestPop3Server()
    {
        Stop();
    }

    void AddMessage( const CORE::CString& uid, const CORE::CString& data )
    {
        TMessage message;
        message.uid = uid;
        message.data = data;
        MT::CScopeMutex lock( m_lock );
        messages.push_back( message );
    }

    /** The maildrop as it is now. 'messages' is the maildrop itself: a test that reads it directly does so while no session is running (see WaitForFinishedConnections()) */
    GUCEF::vector< TMessage > GetMessages( void ) { MT::CScopeMutex lock( m_lock ); return messages; }

    GUCEF::vector< CORE::CString > GetCommands( void ) { MT::CScopeMutex lock( m_lock ); return m_commands; }

    /** How many messages were removed from the maildrop by sessions that ended with QUIT */
    CORE::UInt32 GetCommittedDeletes( void ) { MT::CScopeMutex lock( m_lock ); return m_committedDeletes; }

    /** Waits, without a fixed sleep, until a command that starts with 'prefix' was received (a QUIT after the close of the client, ...), or the time is up */
    bool WaitForCommand( const char* prefix, CORE::UInt32 timeoutInMs )
    {
        return WaitFor( CHasCommand( this, prefix ), timeoutInMs );
    }

    private:

    struct CHasCommand
    {
        CHasCommand( CTestPop3Server* server, const char* prefix ) : m_server( server ), m_prefix( prefix ) {}
        bool operator()( void ) const
        {
            const GUCEF::vector< CORE::CString > commands = m_server->GetCommands();
            for ( size_t i=0; i<commands.size(); ++i )
                if ( commands[ i ].StartsWith( m_prefix ) )
                    return true;
            return false;
        }
        CTestPop3Server* m_server;
        const char* m_prefix;
    };

    public:

    protected:

    virtual void Serve( CTestServerConnection& c ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( HOSTILE_BAD_GREETING == hostile )
        {
            c.WriteLine( "HTTP/1.1 400 Bad Request" );
            c.Sleep( 2000 );
            return;
        }
        if ( HOSTILE_GIANT_STATUS == hostile )
        {
            c.WriteText( "+OK " );
            GUCEF::vector< char > filler( 65536, 'x' );
            for ( int i=0; i<32 && !c.IsClosed(); ++i )
                c.Write( &filler[ 0 ], (CORE::UInt32) filler.size() );
            c.Sleep( 5000 );
            return;
        }
        c.WriteLine( "+OK " + greeting + ( apopTimestamp.IsNULLOrEmpty() ? CORE::CString() : " " + apopTimestamp ) );

        // the maildrop is locked for the session, the way POP3 has it: the session works on what was there when it started
        GUCEF::vector< TMessage > mailbox = GetMessages();
        bool tls = c.IsTls();
        bool authenticated = false;
        CORE::CString pendingUser;
        GUCEF::set< size_t > deleted;
        CORE::UInt32 commandCount = 0;
        CORE::CString line;

        while ( c.ReadLine( line, 20000 ) )
        {
            ++commandCount;
            const CORE::CString upper = line.Uppercase();
            {
                MT::CScopeMutex lock( m_lock );
                // the lines of the login are kept without their secrets
                m_commands.push_back( upper.StartsWith( "PASS" ) || upper.StartsWith( "AUTH" ) || upper.StartsWith( "APOP" ) ? upper.SubstrToChar( ' ', true ) : line );
            }
            SignalChanged();
            if ( dropAfterCommands > 0 && commandCount > dropAfterCommands )
                return;
            c.Sleep( replyDelayInMs );

            if ( upper == "CAPA" )
            {
                if ( !supportCapa )
                {
                    c.WriteLine( "-ERR Unknown command" );
                    continue;
                }
                c.WriteLine( "+OK Capability list follows" );
                if ( supportTop ) c.WriteLine( "TOP" );
                if ( supportUidl ) c.WriteLine( "UIDL" );
                if ( allowUserPass ) c.WriteLine( "USER" );
                if ( !authMechanisms.IsNULLOrEmpty() && ( !saslOnlyAfterTls || tls ) ) c.WriteLine( "SASL " + authMechanisms );
                if ( offerStls && !tls ) c.WriteLine( "STLS" );
                c.WriteLine( "RESP-CODES" );
                c.WriteLine( "IMPLEMENTATION GucefTest" );
                if ( HOSTILE_ENDLESS_LIST == hostile )
                {
                    for ( int i=0; i<200000 && !c.IsClosed(); ++i )
                        c.WriteLine( "X-FILLER" );
                    c.Sleep( 2000 );
                    return;
                }
                c.WriteLine( "." );
            }
            else
            if ( upper == "STLS" )
            {
                if ( !offerStls || tls || !IsTlsAvailable() )
                {
                    c.WriteLine( "-ERR STLS not available" );
                    continue;
                }
                if ( HOSTILE_INJECT_AFTER_STLS == hostile )
                    c.WriteText( "+OK Begin TLS negotiation\r\n+OK injected in plain text\r\n" );
                else
                    c.WriteLine( "+OK Begin TLS negotiation" );
                if ( !c.StartTls() )
                    return;
                tls = true;
            }
            else
            if ( !authenticated )
            {
                if ( upper.StartsWith( "USER " ) && allowUserPass )
                {
                    pendingUser = line.SubstrFromRange( 5, line.Length() );
                    c.WriteLine( "+OK User accepted" );
                }
                else
                if ( upper.StartsWith( "PASS " ) && allowUserPass )
                {
                    if ( pendingUser == user && line.SubstrFromRange( 5, line.Length() ) == password )
                    {
                        authenticated = true;
                        c.WriteLine( "+OK Logged in" );
                    }
                    else
                        c.WriteLine( "-ERR [AUTH] Authentication failed" );
                }
                else
                if ( upper.StartsWith( "APOP " ) && !apopTimestamp.IsNULLOrEmpty() )
                {
                    CORE::CString::StringVector words = line.ParseElements( ' ', false );
                    UInt8 digest[ 16 ];
                    CORE::md5fromstring( ( apopTimestamp + password ).C_String(), digest );
                    if ( words.size() == 3 && words[ 1 ] == user && words[ 2 ].Equals( CORE::Base16Encode( digest, 16 ), false ) )
                    {
                        authenticated = true;
                        c.WriteLine( "+OK Logged in" );
                    }
                    else
                        c.WriteLine( "-ERR [AUTH] Authentication failed" );
                }
                else
                if ( upper.StartsWith( "AUTH" ) )
                {
                    if ( authMechanisms.IsNULLOrEmpty() || ( saslOnlyAfterTls && !tls ) )
                    {
                        c.WriteLine( "-ERR AUTH not available" );
                        continue;
                    }
                    authenticated = Authenticate( c, line );
                }
                else
                if ( upper == "QUIT" )
                {
                    c.WriteLine( "+OK Bye" );
                    return;
                }
                else
                    c.WriteLine( "-ERR Authenticate first" );
            }
            else
            if ( upper == "STAT" )
            {
                CORE::UInt32 count = 0, total = 0;
                for ( size_t i=0; i<mailbox.size(); ++i )
                    if ( deleted.find( i ) == deleted.end() ) { ++count; total += mailbox[ i ].data.ByteSize() - 1; }
                c.WriteLine( "+OK " + CORE::UInt32ToString( count ) + " " + CORE::UInt32ToString( total ) );
            }
            else
            if ( upper.StartsWith( "LIST" ) || ( upper.StartsWith( "UIDL" ) && supportUidl ) )
            {
                const bool isUidl = upper.StartsWith( "UIDL" );
                CORE::CString::StringVector words = line.ParseElements( ' ', false );
                if ( words.size() > 1 )
                {
                    const size_t index = (size_t) atol( words[ 1 ].C_String() );
                    if ( index < 1 || index > mailbox.size() || deleted.find( index - 1 ) != deleted.end() )
                    {
                        c.WriteLine( "-ERR No such message" );
                        continue;
                    }
                    c.WriteLine( "+OK " + words[ 1 ] + " " + ( isUidl ? mailbox[ index - 1 ].uid : CORE::UInt32ToString( mailbox[ index - 1 ].data.ByteSize() - 1 ) ) );
                    continue;
                }
                c.WriteLine( "+OK" );
                for ( size_t i=0; i<mailbox.size(); ++i )
                {
                    if ( deleted.find( i ) != deleted.end() )
                        continue;
                    if ( isUidl && HOSTILE_BAD_UIDL == hostile )
                    {
                        c.WriteLine( CORE::UInt32ToString( (CORE::UInt32) i + 1 ) + " uid with spaces in it" );
                        continue;
                    }
                    c.WriteLine( CORE::UInt32ToString( (CORE::UInt32) i + 1 ) + " " + ( isUidl ? mailbox[ i ].uid : CORE::UInt32ToString( mailbox[ i ].data.ByteSize() - 1 ) ) );
                }
                c.WriteLine( "." );
            }
            else
            if ( upper.StartsWith( "RETR " ) || ( upper.StartsWith( "TOP " ) && supportTop ) )
            {
                CORE::CString::StringVector words = line.ParseElements( ' ', false );
                const size_t index = words.size() > 1 ? (size_t) atol( words[ 1 ].C_String() ) : 0;
                if ( index < 1 || index > mailbox.size() || deleted.find( index - 1 ) != deleted.end() )
                {
                    c.WriteLine( "-ERR No such message" );
                    continue;
                }
                CORE::CString data = mailbox[ index - 1 ].data;
                if ( upper.StartsWith( "TOP " ) )
                {
                    const size_t lines = words.size() > 2 ? (size_t) atol( words[ 2 ].C_String() ) : 0;
                    const Int32 end = data.HasSubstr( "\r\n\r\n" );
                    if ( end >= 0 )
                    {
                        CORE::CString headers = data.SubstrToIndex( (CORE::UInt32) end + 4, true );
                        CORE::CString body = data.SubstrFromRange( (CORE::UInt32) end + 4, data.Length() );
                        CORE::CString::StringVector bodyLines = body.ParseElements( '\n', false );
                        for ( size_t i=0; i<lines && i<bodyLines.size(); ++i )
                            headers += bodyLines[ i ] + ( bodyLines[ i ].EndsWith( "\r" ) ? "\n" : "\r\n" );
                        data = headers;
                    }
                }
                c.WriteLine( "+OK message follows" );
                if ( HOSTILE_ENDLESS_RETR == hostile )
                {
                    for ( int i=0; i<200000 && !c.IsClosed(); ++i )
                        c.WriteLine( "Subject: filler filler filler filler filler filler filler filler filler" );
                    c.Sleep( 2000 );
                    return;
                }
                if ( HOSTILE_DROP_DURING_RETR == hostile )
                {
                    c.WriteText( data.SubstrToIndex( data.Length() / 2, true ) );
                    return;
                }
                // dot stuffing, and the message ends with a line break
                GUCEF::vector< char > out;
                const char* p = data.C_String();
                const CORE::UInt32 size = data.ByteSize() > 0 ? data.ByteSize() - 1 : 0;
                bool lineStart = true;
                for ( CORE::UInt32 i=0; i<size; ++i )
                {
                    if ( lineStart && '.' == p[ i ] )
                        out.push_back( '.' );
                    out.push_back( p[ i ] );
                    lineStart = '\n' == p[ i ];
                }
                if ( !lineStart )
                {
                    out.push_back( '\r' );
                    out.push_back( '\n' );
                }
                out.push_back( '.' );
                out.push_back( '\r' );
                out.push_back( '\n' );
                c.Write( &out[ 0 ], (CORE::UInt32) out.size() );
            }
            else
            if ( upper.StartsWith( "DELE " ) )
            {
                const size_t index = (size_t) atol( line.SubstrFromRange( 5, line.Length() ).C_String() );
                if ( index < 1 || index > mailbox.size() || deleted.find( index - 1 ) != deleted.end() )
                {
                    c.WriteLine( "-ERR No such message" );
                    continue;
                }
                deleted.insert( index - 1 );
                c.WriteLine( "+OK Marked to be deleted" );
            }
            else
            if ( upper == "RSET" )
            {
                deleted.clear();
                c.WriteLine( "+OK Reset" );
            }
            else
            if ( upper == "NOOP" )
                c.WriteLine( "+OK" );
            else
            if ( upper == "QUIT" )
            {
                {
                    // the maildrop is updated: what was added after the start of the session stays
                    MT::CScopeMutex lock( m_lock );
                    m_committedDeletes += (CORE::UInt32) deleted.size();
                    GUCEF::vector< TMessage > remaining;
                    for ( size_t i=0; i<messages.size(); ++i )
                        if ( i >= mailbox.size() || deleted.find( i ) == deleted.end() )
                            remaining.push_back( messages[ i ] );
                    messages = remaining;
                }
                SignalChanged();
                c.WriteLine( "+OK Bye" );
                return;
            }
            else
                c.WriteLine( "-ERR Command not recognized" );
        }
    }

    private:

    bool Authenticate( CTestServerConnection& c, const CORE::CString& line )
    {
        CORE::CString::StringVector words = line.ParseElements( ' ', false );
        if ( words.size() < 2 )
        {
            c.WriteLine( "+OK" );
            return false;
        }
        const CORE::CString mechanism = words[ 1 ].Uppercase();
        CORE::CString initial = words.size() > 2 ? words[ 2 ] : CORE::CString();

        if ( mechanism == "PLAIN" )
        {
            if ( initial.IsNULLOrEmpty() )
            {
                c.WriteLine( "+ " );
                if ( !c.ReadLine( initial, 10000 ) )
                    return false;
            }
            CORE::CString authzid, name, secret;
            if ( SplitPlain( initial, authzid, name, secret ) && name == user && secret == password )
            {
                c.WriteLine( "+OK Logged in" );
                return true;
            }
        }
        else
        if ( mechanism == "XOAUTH2" )
        {
            if ( initial.IsNULLOrEmpty() )
            {
                c.WriteLine( "+ " );
                if ( !c.ReadLine( initial, 10000 ) )
                    return false;
            }
            CORE::CString name, presented;
            if ( SplitXOAuth2( initial, name, presented ) && name == user && presented == token )
            {
                c.WriteLine( "+OK Logged in" );
                return true;
            }
            const CORE::CString json = "{\"status\":\"401\"}";
            c.WriteLine( "+ " + CORE::Base64Encode( json.C_String(), json.ByteSize() - 1 ) );
            CORE::CString ignored;
            c.ReadLine( ignored, 10000 );
        }
        else
        if ( mechanism == "SCRAM-SHA-256" )
        {
            CTestScramServer scram( user, password );
            if ( initial.IsNULLOrEmpty() )
            {
                c.WriteLine( "+ " );
                if ( !c.ReadLine( initial, 10000 ) )
                    return false;
            }
            CORE::CString serverFirst, serverFinal, clientFinal;
            if ( scram.ProcessFirst( DecodeBase64Text( initial ), serverFirst ) )
            {
                c.WriteLine( "+ " + CORE::Base64Encode( serverFirst.C_String(), serverFirst.ByteSize() - 1 ) );
                if ( !c.ReadLine( clientFinal, 10000 ) )
                    return false;
                if ( scram.ProcessFinal( DecodeBase64Text( clientFinal ), serverFinal ) )
                {
                    c.WriteLine( "+ " + CORE::Base64Encode( serverFinal.C_String(), serverFinal.ByteSize() - 1 ) );
                    CORE::CString ignored;
                    if ( !c.ReadLine( ignored, 10000 ) )
                        return false;
                    c.WriteLine( "+OK Logged in" );
                    return true;
                }
            }
        }
        c.WriteLine( "-ERR [AUTH] Authentication failed" );
        return false;
    }

    private:

    GUCEF::vector< CORE::CString > m_commands;
    CORE::UInt32 m_committedDeletes;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

} /* namespace TESTFW */
} /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_POP3_SERVER_H ? */
