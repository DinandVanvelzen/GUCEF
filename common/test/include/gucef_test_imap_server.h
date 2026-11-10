/*
 *  gucef common header: header-only fake IMAP server for tests
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
 *  Test support: an IMAP server on a local port (see gucef_test_tcp_server.h) with one mailbox of messages that a test fills:
 *  CAPABILITY, STARTTLS, LOGIN, AUTHENTICATE (PLAIN, XOAUTH2, SCRAM-SHA-256), LIST, SELECT and EXAMINE with UIDVALIDITY and
 *  UIDNEXT, STATUS, SEARCH (ALL, UNSEEN, SEEN, DELETED, FLAGGED, UID ranges, HEADER, SUBJECT, FROM, SINCE), FETCH (UID, FLAGS,
 *  INTERNALDATE, RFC822.SIZE, ENVELOPE, BODYSTRUCTURE and the sections "", HEADER, TEXT and HEADER.FIELDS, partial), STORE,
 *  EXPUNGE, UID EXPUNGE, COPY, MOVE (when allowMove is set), CLOSE, CREATE, DELETE, APPEND, IDLE, NOOP, LOGOUT. Literals of the commands are read, literals of the
 *  responses are written. Hostile modes make it misbehave on purpose.
 */

#ifndef GUCEF_TEST_IMAP_SERVER_H
#define GUCEF_TEST_IMAP_SERVER_H

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

class CTestImapServer : public CTestTcpServer
{
    public:

    struct SMessage
    {
        CORE::UInt32 uid;
        GUCEF::vector< CORE::CString > flags;
        CORE::CString internalDate;     /**< 02-Nov-2020 10:11:12 +0000 */
        CORE::CString data;             /**< the message, lines ended with CR LF */
        CORE::CString envelope;         /**< what ENVELOPE answers, as the server writes it; empty: NIL */
        CORE::CString bodyStructure;    /**< what BODYSTRUCTURE answers; empty: a text part */
    };
    typedef struct SMessage TMessage;

    enum EHostile
    {
        HOSTILE_NONE = 0,
        HOSTILE_HUGE_LITERAL,           /**< a FETCH response that announces a literal of 4 GiB */
        HOSTILE_LARGE_LITERAL,          /**< a FETCH response that announces a literal of 200 MiB */
        HOSTILE_ENDLESS_UNTAGGED,       /**< endless untagged responses after SELECT */
        HOSTILE_DEEP_NESTING,           /**< a FETCH response with lists nested a thousand deep */
        HOSTILE_GIANT_LINE,             /**< a response line of 2 MiB that does not end */
        HOSTILE_LITERAL_STALL,          /**< a literal of 100 bytes of which 10 come, then silence */
        HOSTILE_GARBAGE,                /**< a response that is not IMAP */
        HOSTILE_BYE_ON_SELECT,
        HOSTILE_INJECT_AFTER_STARTTLS,
        HOSTILE_STATUS_TEXT_WITH_BRACES,/**< the text of a tagged OK ends in {5} */
        HOSTILE_WRONG_TAG,              /**< a completion for a tag that was not used */
        HOSTILE_NUMBER_OVERFLOW         /**< an untagged EXISTS with a number above 32 bits */
    };

    CORE::CString greeting;
    bool offerStartTls;
    bool loginDisabledBeforeTls;
    bool allowLogin;
    bool allowIdle;
    bool allowUidPlus;
    bool allowMove;                     /**< MOVE (RFC 6851) is offered and carried out; off by default */
    bool saslIr;
    CORE::CString authMechanisms;       /**< "PLAIN XOAUTH2 SCRAM-SHA-256" */
    CORE::CString user;
    CORE::CString password;
    CORE::CString token;
    bool preauth;                       /**< the greeting is PREAUTH */
    CORE::UInt32 uidValidity;
    GUCEF::vector< CORE::CString > mailboxes;
    CORE::UInt32 replyDelayInMs;
    CORE::UInt32 dropAfterCommands;
    EHostile hostile;

    CTestImapServer( void )
        : CTestTcpServer()
        , greeting( "IMAP4rev1 test server ready" )
        , offerStartTls( false )
        , loginDisabledBeforeTls( false )
        , allowLogin( true )
        , allowIdle( true )
        , allowUidPlus( true )
        , allowMove( false )
        , saslIr( true )
        , authMechanisms()
        , user( "alice" )
        , password( "s3cret-p@ss" )
        , token( "ya29.token-of-alice" )
        , preauth( false )
        , uidValidity( 1000 )
        , mailboxes()
        , replyDelayInMs( 0 )
        , dropAfterCommands( 0 )
        , hostile( HOSTILE_NONE )
        , m_messages()
        , m_nextUid( 1 )
        , m_pending()
        , m_commands()
        , m_appended()
        , m_lock()
    {
        mailboxes.push_back( "INBOX" );
        mailboxes.push_back( "Sent" );
        mailboxes.push_back( "Drafts" );
    }

    virtual ~CTestImapServer()
    {
        Stop();
    }

    /** Adds a message to INBOX; an untagged EXISTS goes to the clients that are idle */
    CORE::UInt32 AddMessage( const CORE::CString& data, const GUCEF::vector< CORE::CString >& flags = GUCEF::vector< CORE::CString >() )
    {
        MT::CScopeMutex lock( m_lock );
        TMessage message;
        message.uid = m_nextUid++;
        message.flags = flags;
        message.internalDate = "02-Nov-2020 10:11:12 +0000";
        message.data = data;
        m_messages.push_back( message );
        m_pending.push_back( CORE::UInt32ToString( (CORE::UInt32) m_messages.size() ) + " EXISTS" );
        return message.uid;
    }

    /** The mailbox is a new one with the same name: the UIDs start again, the messages are gone */
    void ResetMailbox( CORE::UInt32 newUidValidity )
    {
        MT::CScopeMutex lock( m_lock );
        m_messages.clear();
        m_nextUid = 1;
        uidValidity = newUidValidity;
    }

    /** A line for the clients, untagged (without the star): sent while a client is idle and in front of the next completion */
    void PushUntagged( const CORE::CString& text )
    {
        MT::CScopeMutex lock( m_lock );
        m_pending.push_back( text );
    }

    GUCEF::vector< TMessage > GetMessages( void ) { MT::CScopeMutex lock( m_lock ); return m_messages; }

    GUCEF::vector< CORE::CString > GetCommands( void ) { MT::CScopeMutex lock( m_lock ); return m_commands; }

    /** Waits, without a fixed sleep, until a command that contains 'text' was received (the commands start with a tag; a LOGOUT after the close of the client, ...), or the time is up */
    bool WaitForCommand( const char* text, CORE::UInt32 timeoutInMs )
    {
        return WaitFor( CHasCommand( this, text ), timeoutInMs );
    }

    private:

    struct CHasCommand
    {
        CHasCommand( CTestImapServer* server, const char* text ) : m_server( server ), m_text( text ) {}
        bool operator()( void ) const
        {
            const GUCEF::vector< CORE::CString > commands = m_server->GetCommands();
            for ( size_t i=0; i<commands.size(); ++i )
                if ( commands[ i ].HasSubstr( m_text ) >= 0 )
                    return true;
            return false;
        }
        CTestImapServer* m_server;
        const char* m_text;
    };

    public:

    /** The messages that were put in a mailbox other than INBOX by APPEND, COPY or MOVE */
    GUCEF::vector< TMessage > GetFolder( const CORE::CString& name ) { MT::CScopeMutex lock( m_lock ); return m_folders[ name ]; }

    /** What was appended: "mailbox" and the data of every APPEND */
    GUCEF::vector< TMessage > GetAppended( void ) { MT::CScopeMutex lock( m_lock ); return m_appended; }

    protected:

    virtual void Serve( CTestServerConnection& c ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( HOSTILE_GARBAGE == hostile )
        {
            c.WriteLine( "<html>not imap</html>" );
            c.Sleep( 2000 );
            return;
        }
        if ( HOSTILE_GIANT_LINE == hostile )
        {
            c.WriteText( "* OK " );
            GUCEF::vector< char > filler( 65536, 'x' );
            for ( int i=0; i<40 && !c.IsClosed(); ++i )
                c.Write( &filler[ 0 ], (CORE::UInt32) filler.size() );
            c.Sleep( 5000 );
            return;
        }
        c.WriteLine( ( preauth ? "* PREAUTH " : "* OK " ) + greeting );

        bool tls = c.IsTls();
        bool authenticated = preauth;
        bool selected = false;
        bool readOnly = false;
        CORE::UInt32 commandCount = 0;

        for ( ;; )
        {
            CORE::CString tag, command;
            GUCEF::vector< CORE::CString > args;
            CORE::CString rawLine;
            if ( !ReadCommand( c, tag, command, args, rawLine ) )
                return;
            {
                MT::CScopeMutex lock( m_lock );
                // the secrets of a login are not kept
                const CORE::CString upperCommand = command.Uppercase();
                m_commands.push_back( upperCommand == "LOGIN" || upperCommand == "AUTHENTICATE" ? tag + " " + upperCommand : rawLine );
            }
            SignalChanged();
            ++commandCount;
            if ( dropAfterCommands > 0 && commandCount > dropAfterCommands )
                return;
            c.Sleep( replyDelayInMs );

            const CORE::CString cmd = command.Uppercase();
            if ( cmd == "CAPABILITY" )
            {
                c.WriteLine( "* CAPABILITY " + Capabilities( tls, authenticated ) );
                Complete( c, tag, "OK CAPABILITY completed" );
            }
            else
            if ( cmd == "NOOP" )
            {
                FlushPending( c );
                Complete( c, tag, "OK NOOP completed" );
            }
            else
            if ( cmd == "STARTTLS" )
            {
                if ( !offerStartTls || tls || !IsTlsAvailable() )
                {
                    Complete( c, tag, "BAD STARTTLS not available" );
                    continue;
                }
                if ( HOSTILE_INJECT_AFTER_STARTTLS == hostile )
                    c.WriteText( tag + " OK Begin TLS negotiation now\r\n* OK injected in plain text\r\n" );
                else
                    c.WriteLine( tag + " OK Begin TLS negotiation now" );
                if ( !c.StartTls() )
                    return;
                tls = true;
            }
            else
            if ( cmd == "LOGIN" )
            {
                if ( authenticated || !allowLogin )
                {
                    Complete( c, tag, "BAD LOGIN not allowed" );
                    continue;
                }
                if ( loginDisabledBeforeTls && !tls )
                {
                    Complete( c, tag, "NO [PRIVACYREQUIRED] LOGIN is disabled" );
                    continue;
                }
                if ( args.size() >= 2 && args[ 0 ] == user && args[ 1 ] == password )
                {
                    authenticated = true;
                    Complete( c, tag, "OK LOGIN completed" );
                }
                else
                    Complete( c, tag, "NO [AUTHENTICATIONFAILED] Authentication failed" );
            }
            else
            if ( cmd == "AUTHENTICATE" )
            {
                if ( authenticated )
                {
                    Complete( c, tag, "BAD Already authenticated" );
                    continue;
                }
                if ( Authenticate( c, tag, args ) )
                {
                    authenticated = true;
                    Complete( c, tag, "OK AUTHENTICATE completed" );
                }
            }
            else
            if ( !authenticated )
                Complete( c, tag, "NO [AUTHENTICATIONFAILED] Authenticate first" );
            else
            if ( cmd == "LOGOUT" )
            {
                c.WriteLine( "* BYE Logging out" );
                Complete( c, tag, "OK LOGOUT completed" );
                return;
            }
            else
            if ( cmd == "LIST" )
            {
                const CORE::CString pattern = args.size() > 1 ? args[ 1 ] : CORE::CString( "*" );
                for ( size_t i=0; i<mailboxes.size(); ++i )
                    if ( pattern.IsNULLOrEmpty() || pattern == "*" || pattern == "%" || pattern == mailboxes[ i ] )
                        c.WriteLine( "* LIST (\\HasNoChildren) \"/\" \"" + mailboxes[ i ] + "\"" );
                Complete( c, tag, "OK LIST completed" );
            }
            else
            if ( cmd == "SELECT" || cmd == "EXAMINE" )
            {
                const CORE::CString name = args.empty() ? CORE::CString() : args[ 0 ];
                bool known = false;
                for ( size_t i=0; i<mailboxes.size(); ++i )
                    known = known || mailboxes[ i ] == name;
                if ( !known )
                {
                    selected = false;
                    Complete( c, tag, "NO [NONEXISTENT] No such mailbox" );
                    continue;
                }
                if ( HOSTILE_BYE_ON_SELECT == hostile )
                {
                    c.WriteLine( "* BYE Server shutting down" );
                    return;
                }
                selected = true;
                readOnly = cmd == "EXAMINE";
                const bool isInbox = name.Equals( "INBOX", false );
                CORE::UInt32 count = 0;
                {
                    MT::CScopeMutex lock( m_lock );
                    count = isInbox ? (CORE::UInt32) m_messages.size() : 0;
                    m_pending.clear();
                }
                if ( HOSTILE_NUMBER_OVERFLOW == hostile )
                    c.WriteLine( "* 99999999999 EXISTS" );
                c.WriteLine( "* " + CORE::UInt32ToString( count ) + " EXISTS" );
                c.WriteLine( "* 0 RECENT" );
                c.WriteLine( "* OK [UIDVALIDITY " + CORE::UInt32ToString( uidValidity ) + "] UIDs valid" );
                c.WriteLine( "* OK [UIDNEXT " + CORE::UInt32ToString( NextUid() ) + "] Predicted next UID" );
                c.WriteLine( "* FLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft)" );
                c.WriteLine( "* OK [PERMANENTFLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft \\*)] Limited" );
                if ( HOSTILE_ENDLESS_UNTAGGED == hostile )
                {
                    for ( int i=0; i<500000 && !c.IsClosed(); ++i )
                        c.WriteLine( "* " + CORE::Int32ToString( i ) + " EXISTS" );
                    c.Sleep( 2000 );
                    return;
                }
                Complete( c, tag, readOnly ? "OK [READ-ONLY] EXAMINE completed" : "OK [READ-WRITE] SELECT completed" );
            }
            else
            if ( cmd == "STATUS" )
            {
                const CORE::CString name = args.empty() ? CORE::CString() : args[ 0 ];
                CORE::UInt32 count = name.Equals( "INBOX", false ) ? (CORE::UInt32) GetMessages().size() : 0;
                c.WriteLine( "* STATUS \"" + name + "\" (MESSAGES " + CORE::UInt32ToString( count ) + " RECENT 0 UIDNEXT " + CORE::UInt32ToString( NextUid() ) + " UIDVALIDITY " + CORE::UInt32ToString( uidValidity ) + " UNSEEN 0)" );
                Complete( c, tag, "OK STATUS completed" );
            }
            else
            if ( cmd == "CREATE" )
            {
                if ( !args.empty() )
                    mailboxes.push_back( args[ 0 ] );
                Complete( c, tag, "OK CREATE completed" );
            }
            else
            if ( cmd == "DELETE" )
            {
                if ( !args.empty() )
                    for ( size_t i=0; i<mailboxes.size(); ++i )
                        if ( mailboxes[ i ] == args[ 0 ] ) { mailboxes.erase( mailboxes.begin() + i ); break; }
                Complete( c, tag, "OK DELETE completed" );
            }
            else
            if ( cmd == "APPEND" )
            {
                if ( args.size() < 2 )
                {
                    Complete( c, tag, "BAD APPEND needs a mailbox and data" );
                    continue;
                }
                TMessage message;
                message.data = args.back();
                message.internalDate = "02-Nov-2020 10:11:12 +0000";
                for ( size_t i=1; i+1<args.size(); ++i )
                    if ( args[ i ].StartsWith( "\\" ) )
                        message.flags.push_back( args[ i ] );
                CORE::UInt32 uid = 0;
                {
                    MT::CScopeMutex lock( m_lock );
                    message.uid = 0;
                    m_appended.push_back( message );
                    if ( args[ 0 ].Equals( "INBOX", false ) )
                    {
                        message.uid = m_nextUid++;
                        uid = message.uid;
                        m_messages.push_back( message );
                    }
                    else
                    {
                        message.uid = (CORE::UInt32) m_folders[ args[ 0 ] ].size() + 1;
                        m_folders[ args[ 0 ] ].push_back( message );
                    }
                }
                Complete( c, tag, allowUidPlus && 0 != uid ? "OK [APPENDUID " + CORE::UInt32ToString( uidValidity ) + " " + CORE::UInt32ToString( uid ) + "] APPEND completed" : CORE::CString( "OK APPEND completed" ) );
            }
            else
            if ( !selected )
                Complete( c, tag, "BAD No mailbox selected" );
            else
            if ( cmd == "IDLE" )
            {
                if ( !allowIdle )
                {
                    Complete( c, tag, "BAD IDLE not supported" );
                    continue;
                }
                c.WriteLine( "+ idling" );
                CORE::CString line;
                for ( ;; )
                {
                    FlushPending( c );
                    if ( c.ReadLine( line, 100 ) )
                    {
                        {
                            MT::CScopeMutex lock( m_lock );
                            m_commands.push_back( line );
                        }
                        SignalChanged();
                        if ( line.Equals( "DONE", false ) )
                            break;
                    }
                    else
                    if ( c.IsClosed() || c.IsServerStopping() )
                        return;
                }
                Complete( c, tag, "OK IDLE terminated" );
            }
            else
            if ( cmd == "CLOSE" )
            {
                Expunge( c, false, CORE::CString(), false );
                selected = false;
                Complete( c, tag, "OK CLOSE completed" );
            }
            else
            if ( cmd == "EXPUNGE" )
            {
                Expunge( c, true, CORE::CString(), false );
                Complete( c, tag, "OK EXPUNGE completed" );
            }
            else
            if ( cmd == "SEARCH" || cmd == "FETCH" || cmd == "STORE" || cmd == "COPY" || cmd == "MOVE" || cmd == "UID" )
            {
                bool useUid = false;
                GUCEF::vector< CORE::CString > rest = args;
                CORE::CString subcommand = cmd;
                if ( cmd == "UID" )
                {
                    useUid = true;
                    if ( rest.empty() )
                    {
                        Complete( c, tag, "BAD UID needs a command" );
                        continue;
                    }
                    subcommand = rest[ 0 ].Uppercase();
                    rest.erase( rest.begin() );
                }
                if ( subcommand == "SEARCH" ) DoSearch( c, tag, rest, useUid );
                else if ( subcommand == "FETCH" ) DoFetch( c, tag, rest, useUid );
                else if ( subcommand == "STORE" ) DoStore( c, tag, rest, useUid );
                else if ( subcommand == "COPY" ) DoCopy( c, tag, rest, useUid, false );
                else if ( subcommand == "MOVE" && allowMove ) DoCopy( c, tag, rest, useUid, true );
                else if ( subcommand == "EXPUNGE" )
                {
                    Expunge( c, true, rest.empty() ? CORE::CString() : rest[ 0 ], true );
                    Complete( c, tag, "OK UID EXPUNGE completed" );
                }
                else
                    Complete( c, tag, "BAD Unknown command" );
            }
            else
                Complete( c, tag, "BAD Command unknown" );
        }
    }

    private:

    CORE::UInt32 NextUid( void ) { MT::CScopeMutex lock( m_lock ); return m_nextUid; }

    CORE::CString Capabilities( bool tls, bool authenticated )
    {
        CORE::CString caps = "IMAP4rev1";
        if ( offerStartTls && !tls ) caps += " STARTTLS";
        if ( loginDisabledBeforeTls && !tls ) caps += " LOGINDISABLED";
        if ( !authenticated )
        {
            CORE::CString::StringVector mechanisms = authMechanisms.ParseElements( ' ', false );
            for ( size_t i=0; i<mechanisms.size(); ++i )
                caps += " AUTH=" + mechanisms[ i ];
        }
        if ( saslIr ) caps += " SASL-IR";
        if ( allowIdle ) caps += " IDLE";
        if ( allowUidPlus ) caps += " UIDPLUS";
        if ( allowMove ) caps += " MOVE";
        return caps;
    }

    /** What is waiting for the clients, written in front of a completion or while idling */
    void FlushPending( CTestServerConnection& c )
    {
        GUCEF::vector< CORE::CString > pending;
        {
            MT::CScopeMutex lock( m_lock );
            pending.swap( m_pending );
        }
        for ( size_t i=0; i<pending.size(); ++i )
            c.WriteLine( "* " + pending[ i ] );
    }

    void Complete( CTestServerConnection& c, const CORE::CString& tag, const CORE::CString& text )
    {
        FlushPending( c );
        if ( HOSTILE_WRONG_TAG == hostile )
        {
            c.WriteLine( "ZZ99 " + text );
            return;
        }
        if ( HOSTILE_STATUS_TEXT_WITH_BRACES == hostile && text.StartsWith( "OK" ) )
        {
            c.WriteLine( tag + " " + text + " {5}" );
            return;
        }
        c.WriteLine( tag + " " + text );
    }

    /** Reads a command with the literals it has, as words: a quoted string or a literal is one word, parentheses are words of their own */
    bool ReadCommand( CTestServerConnection& c, CORE::CString& tag, CORE::CString& command, GUCEF::vector< CORE::CString >& args, CORE::CString& rawLine )
    {
        GUCEF::vector< CORE::CString > words;
        CORE::CString line;
        rawLine.Clear();
        for ( ;; )
        {
            if ( !c.ReadLine( line, 60000 ) )
                return false;
            rawLine += ( rawLine.IsNULLOrEmpty() ? CORE::CString() : CORE::CString( " " ) ) + line;

            // a literal at the end of the line: {n} or {n+}
            CORE::UInt32 literalSize = 0;
            bool literal = false;
            bool plus = false;
            CORE::CString text = line;
            const CORE::UInt32 size = line.ByteSize() > 0 ? line.ByteSize() - 1 : 0;
            const char* p = line.C_String();
            if ( size > 2 && '}' == p[ size - 1 ] )
            {
                CORE::UInt32 open = size - 2;
                if ( '+' == p[ open ] ) { plus = true; --open; }
                CORE::UInt32 end = open;
                while ( open > 0 && p[ open ] >= '0' && p[ open ] <= '9' ) --open;
                if ( '{' == p[ open ] && open + 1 <= end )
                {
                    literalSize = (CORE::UInt32) atol( p + open + 1 );
                    literal = true;
                    text = CORE::CString( p, open );
                }
            }
            Tokenize( text, words );
            if ( !literal )
                break;
            if ( !plus )
                c.WriteLine( "+ Ready for literal data" );
            CORE::CDynamicBuffer data;
            if ( !c.ReadBytes( literalSize, data, 30000 ) )
                return false;
            words.push_back( data.GetDataSize() > 0 ? CORE::CString( static_cast< const char* >( data.GetConstBufferPtr() ), data.GetDataSize() ) : CORE::CString() );
        }
        if ( words.empty() )
        {
            // an empty line is no command; a client that sends nothing else is not worth waiting for
            return false;
        }
        tag = words[ 0 ];
        command = words.size() > 1 ? words[ 1 ] : CORE::CString();
        args.clear();
        for ( size_t i=2; i<words.size(); ++i )
            args.push_back( words[ i ] );
        return true;
    }

    static void Tokenize( const CORE::CString& text, GUCEF::vector< CORE::CString >& words )
    {
        const char* p = text.C_String();
        const CORE::UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
        CORE::UInt32 i = 0;
        while ( i < size )
        {
            if ( ' ' == p[ i ] ) { ++i; continue; }
            if ( '(' == p[ i ] || ')' == p[ i ] )
            {
                words.push_back( CORE::CString( p + i, 1 ) );
                ++i;
                continue;
            }
            if ( '"' == p[ i ] )
            {
                ++i;
                GUCEF::vector< char > chars;
                while ( i < size && '"' != p[ i ] )
                {
                    if ( '\\' == p[ i ] && i + 1 < size ) ++i;
                    chars.push_back( p[ i++ ] );
                }
                ++i;
                words.push_back( chars.empty() ? CORE::CString() : CORE::CString( &chars[ 0 ], (CORE::UInt32) chars.size() ) );
                continue;
            }
            const CORE::UInt32 start = i;
            int bracket = 0;
            while ( i < size && ( bracket > 0 || ( ' ' != p[ i ] && '(' != p[ i ] && ')' != p[ i ] ) ) )
            {
                if ( '[' == p[ i ] ) ++bracket;
                if ( ']' == p[ i ] && bracket > 0 ) --bracket;
                ++i;
            }
            words.push_back( CORE::CString( p + start, i - start ) );
        }
    }

    bool Authenticate( CTestServerConnection& c, const CORE::CString& tag, const GUCEF::vector< CORE::CString >& args )
    {
        if ( args.empty() )
        {
            Complete( c, tag, "BAD No mechanism" );
            return false;
        }
        const CORE::CString mechanism = args[ 0 ].Uppercase();
        CORE::CString initial = args.size() > 1 ? args[ 1 ] : CORE::CString();
        if ( initial == "=" )
            initial.Clear();

        #define GUCEF_TEST_AUTH_FAILED { Complete( c, tag, "NO [AUTHENTICATIONFAILED] Authentication failed" ); return false; }

        if ( mechanism == "PLAIN" )
        {
            if ( initial.IsNULLOrEmpty() )
            {
                c.WriteLine( "+ " );
                if ( !c.ReadLine( initial, 10000 ) )
                    return false;
            }
            CORE::CString authzid, name, secret;
            if ( initial != "*" && SplitPlain( initial, authzid, name, secret ) && name == user && secret == password )
                return true;
            GUCEF_TEST_AUTH_FAILED
        }
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
                return true;
            const CORE::CString json = "{\"status\":\"400\",\"schemes\":\"Bearer\",\"scope\":\"https://mail.test/\"}";
            c.WriteLine( "+ " + CORE::Base64Encode( json.C_String(), json.ByteSize() - 1 ) );
            CORE::CString ignored;
            c.ReadLine( ignored, 10000 );
            GUCEF_TEST_AUTH_FAILED
        }
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
            if ( !scram.ProcessFirst( DecodeBase64Text( initial ), serverFirst ) )
                GUCEF_TEST_AUTH_FAILED
            c.WriteLine( "+ " + CORE::Base64Encode( serverFirst.C_String(), serverFirst.ByteSize() - 1 ) );
            if ( !c.ReadLine( clientFinal, 10000 ) )
                return false;
            if ( !scram.ProcessFinal( DecodeBase64Text( clientFinal ), serverFinal ) )
                GUCEF_TEST_AUTH_FAILED
            c.WriteLine( "+ " + CORE::Base64Encode( serverFinal.C_String(), serverFinal.ByteSize() - 1 ) );
            CORE::CString ignored;
            if ( !c.ReadLine( ignored, 10000 ) )
                return false;
            return true;
        }
        Complete( c, tag, "NO Mechanism not supported" );
        return false;

        #undef GUCEF_TEST_AUTH_FAILED
    }

    /** The numbers of a set like 1:3,5,7:* for a mailbox with 'maxValue' as its highest number */
    static bool InSet( const CORE::CString& set, CORE::UInt32 value, CORE::UInt32 maxValue )
    {
        CORE::CString::StringVector parts = set.ParseElements( ',', false );
        for ( size_t i=0; i<parts.size(); ++i )
        {
            CORE::UInt32 low = 0, high = 0;
            const Int32 colon = parts[ i ].HasChar( ':' );
            const CORE::CString a = colon >= 0 ? parts[ i ].SubstrToIndex( (CORE::UInt32) colon, true ) : parts[ i ];
            const CORE::CString b = colon >= 0 ? parts[ i ].SubstrFromRange( (CORE::UInt32) colon + 1, parts[ i ].Length() ) : parts[ i ];
            low = a == "*" ? maxValue : (CORE::UInt32) atol( a.C_String() );
            high = b == "*" ? maxValue : (CORE::UInt32) atol( b.C_String() );
            if ( low > high ) { CORE::UInt32 t = low; low = high; high = t; }
            // "n:*" with n above the highest number includes the highest
            if ( b == "*" && low > maxValue ) low = maxValue;
            if ( value >= low && value <= high )
                return true;
        }
        return false;
    }

    static bool HasFlag( const TMessage& message, const CORE::CString& flag )
    {
        for ( size_t i=0; i<message.flags.size(); ++i )
            if ( message.flags[ i ].Equals( flag, false ) )
                return true;
        return false;
    }

    static CORE::CString FlagsText( const TMessage& message )
    {
        CORE::CString text = "(";
        for ( size_t i=0; i<message.flags.size(); ++i )
            text += ( i > 0 ? " " : "" ) + message.flags[ i ];
        return text + ")";
    }

    /** The value of a header of the message, empty when it has none */
    static CORE::CString HeaderOf( const TMessage& message, const CORE::CString& name )
    {
        const CORE::CString headers = HeaderSection( message );
        CORE::CString::StringVector lines = headers.ParseElements( '\n', false );
        CORE::CString value;
        bool found = false;
        for ( size_t i=0; i<lines.size(); ++i )
        {
            CORE::CString l = lines[ i ];
            if ( l.EndsWith( "\r" ) ) l = l.SubstrToIndex( l.Length() - 1, true );
            if ( !l.IsNULLOrEmpty() && ( ' ' == l.C_String()[ 0 ] || '\t' == l.C_String()[ 0 ] ) )
            {
                if ( found ) value += " " + l.Trim( true );
                continue;
            }
            if ( found )
                break;
            const Int32 colon = l.HasChar( ':' );
            if ( colon > 0 && l.SubstrToIndex( (CORE::UInt32) colon, true ).Equals( name, false ) )
            {
                found = true;
                value = l.SubstrFromRange( (CORE::UInt32) colon + 1, l.Length() ).Trim( true ).Trim( false );
            }
        }
        return value;
    }

    static CORE::CString HeaderSection( const TMessage& message )
    {
        const Int32 end = message.data.HasSubstr( "\r\n\r\n" );
        return end >= 0 ? message.data.SubstrToIndex( (CORE::UInt32) end + 4, true ) : message.data;
    }

    static CORE::CString BodySection( const TMessage& message )
    {
        const Int32 end = message.data.HasSubstr( "\r\n\r\n" );
        return end >= 0 ? message.data.SubstrFromRange( (CORE::UInt32) end + 4, message.data.Length() ) : CORE::CString();
    }

    void DoSearch( CTestServerConnection& c, const CORE::CString& tag, const GUCEF::vector< CORE::CString >& args, bool useUid )
    {
        GUCEF::vector< TMessage > messages = GetMessages();
        const CORE::UInt32 maxUid = messages.empty() ? 0 : messages.back().uid;
        CORE::CString results;
        for ( size_t m=0; m<messages.size(); ++m )
        {
            bool match = true;
            for ( size_t i=0; i<args.size() && match; ++i )
            {
                const CORE::CString a = args[ i ].Uppercase();
                if ( a == "CHARSET" ) { ++i; continue; }
                if ( a == "ALL" ) continue;
                if ( a == "UNSEEN" ) match = !HasFlag( messages[ m ], "\\Seen" );
                else if ( a == "SEEN" ) match = HasFlag( messages[ m ], "\\Seen" );
                else if ( a == "DELETED" ) match = HasFlag( messages[ m ], "\\Deleted" );
                else if ( a == "FLAGGED" ) match = HasFlag( messages[ m ], "\\Flagged" );
                else if ( a == "UID" && i + 1 < args.size() ) { match = InSet( args[ i + 1 ], messages[ m ].uid, maxUid ); ++i; }
                else if ( a == "SUBJECT" && i + 1 < args.size() ) { match = HeaderOf( messages[ m ], "Subject" ).Lowercase().HasSubstr( args[ i + 1 ].Lowercase() ) >= 0; ++i; }
                else if ( a == "FROM" && i + 1 < args.size() ) { match = HeaderOf( messages[ m ], "From" ).Lowercase().HasSubstr( args[ i + 1 ].Lowercase() ) >= 0; ++i; }
                else if ( a == "HEADER" && i + 2 < args.size() )
                {
                    const CORE::CString value = HeaderOf( messages[ m ], args[ i + 1 ] );
                    // a header that is not there does not match, an empty text only asks for the header
                    match = HeaderSection( messages[ m ] ).Lowercase().HasSubstr( args[ i + 1 ].Lowercase() + ":" ) >= 0 &&
                            ( args[ i + 2 ].IsNULLOrEmpty() || value.Lowercase().HasSubstr( args[ i + 2 ].Lowercase() ) >= 0 );
                    i += 2;
                }
                else if ( a == "SINCE" && i + 1 < args.size() ) ++i;
            }
            if ( match )
                results += " " + CORE::UInt32ToString( useUid ? messages[ m ].uid : (CORE::UInt32) m + 1 );
        }
        c.WriteLine( "* SEARCH" + results );
        Complete( c, tag, "OK SEARCH completed" );
    }

    void DoFetch( CTestServerConnection& c, const CORE::CString& tag, const GUCEF::vector< CORE::CString >& args, bool useUid )
    {
        if ( args.size() < 2 )
        {
            Complete( c, tag, "BAD FETCH needs a set and items" );
            return;
        }
        // the items: one word or a list in parentheses
        GUCEF::vector< CORE::CString > items;
        for ( size_t i=1; i<args.size(); ++i )
            if ( args[ i ] != "(" && args[ i ] != ")" )
                items.push_back( args[ i ] );

        GUCEF::vector< TMessage > messages = GetMessages();
        GUCEF::vector< CORE::CString > seenUids;
        const CORE::UInt32 maxNumber = useUid ? ( messages.empty() ? 0 : messages.back().uid ) : (CORE::UInt32) messages.size();
        for ( size_t m=0; m<messages.size(); ++m )
        {
            TMessage& message = messages[ m ];
            const CORE::UInt32 number = useUid ? message.uid : (CORE::UInt32) m + 1;
            if ( !InSet( args[ 0 ], number, maxNumber ) )
                continue;

            if ( HOSTILE_HUGE_LITERAL == hostile )
            {
                c.WriteText( "* " + CORE::UInt32ToString( (CORE::UInt32) m + 1 ) + " FETCH (UID " + CORE::UInt32ToString( message.uid ) + " BODY[] {4294967295}\r\n" );
                c.Sleep( 3000 );
                return;
            }
            if ( HOSTILE_LARGE_LITERAL == hostile )
            {
                c.WriteText( "* " + CORE::UInt32ToString( (CORE::UInt32) m + 1 ) + " FETCH (UID " + CORE::UInt32ToString( message.uid ) + " BODY[] {209715200}\r\n" );
                c.Sleep( 3000 );
                return;
            }
            if ( HOSTILE_LITERAL_STALL == hostile )
            {
                c.WriteText( "* 1 FETCH (BODY[] {100}\r\n0123456789" );
                c.Sleep( 10000 );
                return;
            }
            if ( HOSTILE_DEEP_NESTING == hostile )
            {
                GUCEF::vector< char > nest( 1000, '(' );
                c.WriteText( "* 1 FETCH " );
                c.Write( &nest[ 0 ], (CORE::UInt32) nest.size() );
                c.WriteText( "\r\n" );
                continue;
            }

            CORE::CString out = "* " + CORE::UInt32ToString( (CORE::UInt32) m + 1 ) + " FETCH (UID " + CORE::UInt32ToString( message.uid );
            bool setSeen = false;
            for ( size_t i=0; i<items.size(); ++i )
            {
                const CORE::CString item = items[ i ].Uppercase();
                if ( item == "UID" ) continue;
                if ( item == "FLAGS" ) out += " FLAGS " + FlagsText( message );
                else if ( item == "INTERNALDATE" ) out += " INTERNALDATE \"" + message.internalDate + "\"";
                else if ( item == "RFC822.SIZE" ) out += " RFC822.SIZE " + CORE::UInt32ToString( message.data.ByteSize() - 1 );
                else if ( item == "ENVELOPE" ) out += " ENVELOPE " + ( message.envelope.IsNULLOrEmpty() ? CORE::CString( "NIL" ) : message.envelope );
                else if ( item == "BODYSTRUCTURE" ) out += " BODYSTRUCTURE " + ( message.bodyStructure.IsNULLOrEmpty() ? "(\"TEXT\" \"PLAIN\" (\"CHARSET\" \"UTF-8\") NIL NIL \"7BIT\" " + CORE::UInt32ToString( message.data.ByteSize() - 1 ) + " 1)" : message.bodyStructure );
                else if ( item.StartsWith( "BODY[" ) || item.StartsWith( "BODY.PEEK[" ) )
                {
                    const bool peek = item.StartsWith( "BODY.PEEK[" );
                    const Int32 open = items[ i ].HasChar( '[' );
                    const Int32 close = items[ i ].HasChar( ']', false );
                    CORE::CString section = items[ i ].SubstrFromRange( (CORE::UInt32) open + 1, (CORE::UInt32) close ).Uppercase();
                    CORE::CString content;
                    bool nil = false;
                    if ( section.IsNULLOrEmpty() ) content = message.data;
                    else if ( section == "HEADER" ) content = HeaderSection( message );
                    else if ( section == "TEXT" ) content = BodySection( message );
                    else if ( section.StartsWith( "HEADER.FIELDS" ) )
                    {
                        // the lines of the headers that were asked for
                        CORE::CString wanted = section.SubstrFromRange( 13, section.Length() ).Lowercase();
                        CORE::CString::StringVector lines = HeaderSection( message ).ParseElements( '\n', false );
                        for ( size_t l=0; l<lines.size(); ++l )
                        {
                            const Int32 colon = lines[ l ].HasChar( ':' );
                            if ( colon > 0 && wanted.HasSubstr( lines[ l ].SubstrToIndex( (CORE::UInt32) colon, true ).Lowercase() ) >= 0 )
                                content += lines[ l ].EndsWith( "\r" ) ? lines[ l ] + "\n" : lines[ l ] + "\r\n";
                        }
                        content += "\r\n";
                    }
                    else nil = true;

                    // a partial fetch <offset.count>
                    CORE::CString suffix;
                    const CORE::CString after = items[ i ].SubstrFromRange( (CORE::UInt32) close + 1, items[ i ].Length() );
                    if ( !nil && after.StartsWith( "<" ) )
                    {
                        const Int32 dot = after.HasChar( '.' );
                        const CORE::UInt32 offset = (CORE::UInt32) atol( after.SubstrFromRange( 1, (CORE::UInt32) dot ).C_String() );
                        const CORE::UInt32 count = (CORE::UInt32) atol( after.SubstrFromRange( (CORE::UInt32) dot + 1, after.Length() ).C_String() );
                        CORE::CString slice = content.SubstrFromRange( offset < content.Length() ? offset : content.Length(), content.Length() );
                        content = slice.Length() > count ? slice.SubstrToIndex( count, true ) : slice;
                        suffix = "<" + CORE::UInt32ToString( offset ) + ">";
                    }
                    const CORE::CString sectionName = "BODY[" + section + "]" + suffix;
                    if ( nil )
                        out += " " + sectionName + " NIL";
                    else
                        out += " " + sectionName + " {" + CORE::UInt32ToString( content.ByteSize() - 1 ) + "}\r\n" + content;
                    if ( !peek )
                        setSeen = true;
                }
            }
            if ( setSeen && !HasFlag( message, "\\Seen" ) )
            {
                message.flags.push_back( "\\Seen" );
                out += " FLAGS " + FlagsText( message );
                seenUids.push_back( CORE::UInt32ToString( message.uid ) );
            }
            c.WriteLine( out + ")" );
        }

        if ( !seenUids.empty() )
        {
            MT::CScopeMutex lock( m_lock );
            for ( size_t i=0; i<seenUids.size(); ++i )
                for ( size_t m=0; m<m_messages.size(); ++m )
                    if ( CORE::UInt32ToString( m_messages[ m ].uid ) == seenUids[ i ] && !HasFlag( m_messages[ m ], "\\Seen" ) )
                        m_messages[ m ].flags.push_back( "\\Seen" );
        }
        Complete( c, tag, "OK FETCH completed" );
    }

    void DoStore( CTestServerConnection& c, const CORE::CString& tag, const GUCEF::vector< CORE::CString >& args, bool useUid )
    {
        if ( args.size() < 3 )
        {
            Complete( c, tag, "BAD STORE needs a set, an item and flags" );
            return;
        }
        const CORE::CString item = args[ 1 ].Uppercase();
        const bool silent = item.HasSubstr( ".SILENT" ) >= 0;
        const bool add = item.StartsWith( "+" );
        const bool remove = item.StartsWith( "-" );
        GUCEF::vector< CORE::CString > flags;
        for ( size_t i=2; i<args.size(); ++i )
            if ( args[ i ] != "(" && args[ i ] != ")" )
                flags.push_back( args[ i ] );

        GUCEF::vector< CORE::CString > responses;
        {
            MT::CScopeMutex lock( m_lock );
            const CORE::UInt32 maxNumber = useUid ? ( m_messages.empty() ? 0 : m_messages.back().uid ) : (CORE::UInt32) m_messages.size();
            for ( size_t m=0; m<m_messages.size(); ++m )
            {
                TMessage& message = m_messages[ m ];
                if ( !InSet( args[ 0 ], useUid ? message.uid : (CORE::UInt32) m + 1, maxNumber ) )
                    continue;
                if ( !add && !remove )
                    message.flags.clear();
                for ( size_t f=0; f<flags.size(); ++f )
                {
                    if ( remove )
                    {
                        for ( size_t k=0; k<message.flags.size(); ++k )
                            if ( message.flags[ k ].Equals( flags[ f ], false ) ) { message.flags.erase( message.flags.begin() + k ); break; }
                    }
                    else
                    if ( !HasFlag( message, flags[ f ] ) )
                        message.flags.push_back( flags[ f ] );
                }
                if ( !silent )
                    responses.push_back( "* " + CORE::UInt32ToString( (CORE::UInt32) m + 1 ) + " FETCH (" + ( useUid ? "UID " + CORE::UInt32ToString( message.uid ) + " " : CORE::CString() ) + "FLAGS " + FlagsText( message ) + ")" );
            }
        }
        for ( size_t i=0; i<responses.size(); ++i )
            c.WriteLine( responses[ i ] );
        Complete( c, tag, "OK STORE completed" );
    }

    /** COPY and MOVE: the messages of the set go to a folder that exists; the flags go with them. MOVE removes them from INBOX and reports it as expunged */
    void DoCopy( CTestServerConnection& c, const CORE::CString& tag, const GUCEF::vector< CORE::CString >& args, bool useUid, bool move )
    {
        if ( args.size() < 2 )
        {
            Complete( c, tag, "BAD COPY needs a set and a mailbox" );
            return;
        }
        bool known = false;
        for ( size_t i=0; i<mailboxes.size(); ++i )
            known = known || mailboxes[ i ] == args[ 1 ];
        if ( !known || args[ 1 ].Equals( "INBOX", false ) )
        {
            Complete( c, tag, "NO [TRYCREATE] No such mailbox" );
            return;
        }
        GUCEF::vector< CORE::CString > responses;
        CORE::UInt32 firstTarget = 0;
        CORE::UInt32 firstSource = 0;
        {
            MT::CScopeMutex lock( m_lock );
            GUCEF::vector< TMessage >& target = m_folders[ args[ 1 ] ];
            const CORE::UInt32 maxNumber = useUid ? ( m_messages.empty() ? 0 : m_messages.back().uid ) : (CORE::UInt32) m_messages.size();
            for ( size_t m=0; m<m_messages.size(); )
            {
                TMessage& message = m_messages[ m ];
                if ( !InSet( args[ 0 ], useUid ? message.uid : (CORE::UInt32) m + 1, maxNumber ) )
                {
                    ++m;
                    continue;
                }
                TMessage copy = message;
                copy.uid = (CORE::UInt32) target.size() + 1;
                if ( 0 == firstTarget ) { firstTarget = copy.uid; firstSource = message.uid; }
                target.push_back( copy );
                if ( move )
                {
                    responses.push_back( "* " + CORE::UInt32ToString( (CORE::UInt32) m + 1 ) + " EXPUNGE" );
                    m_messages.erase( m_messages.begin() + m );
                }
                else
                    ++m;
            }
        }
        const CORE::CString copyUid = allowUidPlus && 0 != firstTarget ? "[COPYUID " + CORE::UInt32ToString( uidValidity ) + " " + CORE::UInt32ToString( firstSource ) + " " + CORE::UInt32ToString( firstTarget ) + "]" : CORE::CString();
        if ( move )
        {
            if ( !copyUid.IsNULLOrEmpty() )
                c.WriteLine( "* OK " + copyUid );
            for ( size_t i=0; i<responses.size(); ++i )
                c.WriteLine( responses[ i ] );
            Complete( c, tag, "OK MOVE completed" );
        }
        else
            Complete( c, tag, copyUid.IsNULLOrEmpty() ? CORE::CString( "OK COPY completed" ) : "OK " + copyUid + " COPY completed" );
    }

    void Expunge( CTestServerConnection& c, bool report, const CORE::CString& uidSet, bool useSet )
    {
        GUCEF::vector< CORE::CString > lines;
        {
            MT::CScopeMutex lock( m_lock );
            const CORE::UInt32 maxUid = m_messages.empty() ? 0 : m_messages.back().uid;
            CORE::UInt32 removed = 0;
            for ( size_t m=0; m<m_messages.size(); )
            {
                if ( HasFlag( m_messages[ m ], "\\Deleted" ) && ( !useSet || InSet( uidSet, m_messages[ m ].uid, maxUid ) ) )
                {
                    lines.push_back( "* " + CORE::UInt32ToString( (CORE::UInt32) m + 1 ) + " EXPUNGE" );
                    m_messages.erase( m_messages.begin() + m );
                    ++removed;
                }
                else
                    ++m;
            }
        }
        if ( report )
            for ( size_t i=0; i<lines.size(); ++i )
                c.WriteLine( lines[ i ] );
    }

    private:

    GUCEF::vector< TMessage > m_messages;
    CORE::UInt32 m_nextUid;
    GUCEF::vector< CORE::CString > m_pending;
    GUCEF::vector< CORE::CString > m_commands;
    GUCEF::vector< TMessage > m_appended;
    GUCEF::map< CORE::CString, GUCEF::vector< TMessage > > m_folders;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

} /* namespace TESTFW */
} /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_IMAP_SERVER_H ? */
