/*
 *  gucefMULTIPLAY_MailProvider_TestApp: test header: a small world of mailboxes for tests that need several people to mail each other
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
 *  Test support: every mailbox of the world has its own SMTP server (the one its owner sends through), IMAP server and POP3
 *  server on local ports. A message accepted by any SMTP server is routed into the IMAP and POP3 mailboxes of its
 *  recipients, the way a mail system delivers. The world can lose, repeat, hold back, reorder and strip messages, and
 *  deliver text that nobody sent. Everything stays on the loopback interface.
 *
 *    GUCEF::TESTFW::CTestEMailWorld world;
 *    GUCEF::TESTFW::CTestEMailBox* alice = world.AddMailbox( "alice@example.invalid", "alice", "alice-pw" );
 *    world.Start();
 *    ... a client sends through alice->smtp, the mail turns up in the mailbox of the recipient ...
 */

#ifndef GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILWORLD_H
#define GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILWORLD_H

#ifdef __cplusplus

#ifndef GUCEF_TEST_SMTP_SERVER_H
#include "gucef_test_smtp_server.h"
#define GUCEF_TEST_SMTP_SERVER_H
#endif /* GUCEF_TEST_SMTP_SERVER_H ? */

#ifndef GUCEF_TEST_IMAP_SERVER_H
#include "gucef_test_imap_server.h"
#define GUCEF_TEST_IMAP_SERVER_H
#endif /* GUCEF_TEST_IMAP_SERVER_H ? */

#ifndef GUCEF_TEST_POP3_SERVER_H
#include "gucef_test_pop3_server.h"
#define GUCEF_TEST_POP3_SERVER_H
#endif /* GUCEF_TEST_POP3_SERVER_H ? */

#ifndef GUCEF_COM_CMIMEPARSER_H
#include "gucefCOM_CMimeParser.h"
#define GUCEF_COM_CMIMEPARSER_H
#endif /* GUCEF_COM_CMIMEPARSER_H ? */

#ifndef GUCEF_COM_CMIMECOMPOSER_H
#include "gucefCOM_CMimeComposer.h"
#define GUCEF_COM_CMIMECOMPOSER_H
#endif /* GUCEF_COM_CMIMECOMPOSER_H ? */

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------*/

/** One mailbox: the address and the three servers of the owner */
class CTestEMailBox
{
    public:

    CORE::CString address;
    CORE::CString user;
    CORE::CString password;
    CTestSmtpServer smtp;
    CTestImapServer imap;
    CTestPop3Server pop3;

    CTestEMailBox( const CORE::CString& mailAddress, const CORE::CString& userName, const CORE::CString& userPassword )
        : address( mailAddress ), user( userName ), password( userPassword ), smtp(), imap(), pop3()
    {
        smtp.authMechanisms = "PLAIN";
        smtp.user = imap.user = pop3.user = userName;
        smtp.password = imap.password = pop3.password = userPassword;
        imap.authMechanisms = "PLAIN";
        pop3.authMechanisms = "PLAIN";
    }
};

/*-------------------------------------------------------------------------*/

class CTestEMailWorld : public CTestSmtpDeliverySink
{
    public:

    CTestEMailWorld( void )
        : m_boxes(), m_dropCount( 0 ), m_dropContaining(), m_duplicateCount( 0 ), m_hold( false ), m_holdAddress(), m_held(), m_stripAttachments( false )
        , m_accepted(), m_copiesDelivered( 0 ), m_pop3Counter( 0 ), m_lock()
    {
    }

    virtual ~CTestEMailWorld()
    {
        Stop();
        for ( size_t i=0; i<m_boxes.size(); ++i )
            delete m_boxes[ i ];
        m_boxes.clear();
    }

    /** Adds a mailbox; only before Start() */
    CTestEMailBox* AddMailbox( const CORE::CString& address, const CORE::CString& user, const CORE::CString& password )
    {
        CTestEMailBox* box = new CTestEMailBox( address, user, password );
        box->smtp.deliverySink = this;
        m_boxes.push_back( box );
        return box;
    }

    CTestEMailBox* Find( const CORE::CString& address )
    {
        const CORE::CString wanted = address.Lowercase();
        for ( size_t i=0; i<m_boxes.size(); ++i )
            if ( m_boxes[ i ]->address.Lowercase() == wanted )
                return m_boxes[ i ];
        return NULL;
    }

    bool Start( void )
    {
        for ( size_t i=0; i<m_boxes.size(); ++i )
            if ( !m_boxes[ i ]->smtp.Start() || !m_boxes[ i ]->imap.Start() || !m_boxes[ i ]->pop3.Start() )
                return false;
        return true;
    }

    void Stop( void )
    {
        for ( size_t i=0; i<m_boxes.size(); ++i )
        {
            m_boxes[ i ]->smtp.Stop();
            m_boxes[ i ]->imap.Stop();
            m_boxes[ i ]->pop3.Stop();
        }
    }

    /* ---- what the mail system does wrong ---- */

    /** The next 'count' messages whose text contains 'containing' (any message when empty) are accepted by SMTP and never delivered */
    void DropNext( UInt32 count, const CORE::CString& containing = CORE::CString::Empty )
    {
        MT::CScopeMutex lock( m_lock );
        m_dropCount = count;
        m_dropContaining = containing;
    }

    /** The next 'count' messages are delivered twice */
    void DuplicateNext( UInt32 count ) { MT::CScopeMutex lock( m_lock ); m_duplicateCount = count; }

    /** While set, accepted messages wait instead of being delivered; with an address only the copies for that mailbox wait */
    void SetHold( bool hold, const CORE::CString& onlyForAddress = CORE::CString::Empty )
    {
        MT::CScopeMutex lock( m_lock );
        m_hold = hold;
        m_holdAddress = onlyForAddress.Lowercase();
    }

    UInt32 GetHeldCount( void ) { MT::CScopeMutex lock( m_lock ); return (UInt32) m_held.size(); }

    /** Delivers what was held back, in the order it was accepted or in the opposite order */
    void ReleaseHeld( bool reverse )
    {
        GUCEF::vector< SHeld > held;
        {
            MT::CScopeMutex lock( m_lock );
            held.swap( m_held );
        }
        if ( reverse )
            for ( size_t i=held.size(); i>0; --i )
                Deliver( held[ i - 1 ].recipients, held[ i - 1 ].data, true );
        else
            for ( size_t i=0; i<held.size(); ++i )
                Deliver( held[ i ].recipients, held[ i ].data, true );
    }

    /** A mail system that removes the attachments of every message it delivers */
    void SetStripAttachments( bool strip ) { MT::CScopeMutex lock( m_lock ); m_stripAttachments = strip; }

    /** Puts text into the mailbox of the address as a message that nobody sent through SMTP */
    bool DeliverTo( const CORE::CString& address, const CORE::CString& data )
    {
        GUCEF::vector< CORE::CString > recipients;
        recipients.push_back( address );
        return Deliver( recipients, data, true );       // put in directly: not subject to what is held back
    }

    /* ---- what happened ---- */

    UInt32 GetAcceptedCount( void ) { MT::CScopeMutex lock( m_lock ); return (UInt32) m_accepted.size(); }

    UInt32 GetDeliveredCopies( void ) { MT::CScopeMutex lock( m_lock ); return m_copiesDelivered; }

    /** The data of every message SMTP accepted, whatever became of it */
    GUCEF::vector< CORE::CString > GetAccepted( void ) { MT::CScopeMutex lock( m_lock ); return m_accepted; }

    virtual void OnSmtpAccepted( CTestSmtpServer* origin, const CORE::CString& mailFrom, const GUCEF::vector< CORE::CString >& recipients, const CORE::CString& data ) GUCEF_VIRTUAL_OVERRIDE
    {
        bool drop = false;
        bool duplicate = false;
        {
            MT::CScopeMutex lock( m_lock );
            m_accepted.push_back( data );
            if ( m_dropCount > 0 && ( m_dropContaining.IsNULLOrEmpty() || data.HasSubstr( m_dropContaining ) >= 0 ) )
            {
                --m_dropCount;
                drop = true;
            }
            else
            if ( m_duplicateCount > 0 )
            {
                --m_duplicateCount;
                duplicate = true;
            }
        }
        if ( drop )
            return;
        Deliver( recipients, data );
        if ( duplicate )
            Deliver( recipients, data );
    }

    private:

    struct SHeld
    {
        GUCEF::vector< CORE::CString > recipients;
        CORE::CString data;
    };

    bool Deliver( const GUCEF::vector< CORE::CString >& recipients, const CORE::CString& original, bool ignoreHold = false )
    {
        CORE::CString data = original;
        bool strip = false;
        {
            MT::CScopeMutex lock( m_lock );
            strip = m_stripAttachments;
        }
        if ( strip )
            data = StripAttachments( data );

        bool any = false;
        for ( size_t i=0; i<recipients.size(); ++i )
        {
            CTestEMailBox* box = Find( recipients[ i ] );
            if ( NULL == box )
                continue;
            if ( !ignoreHold )
            {
                MT::CScopeMutex lock( m_lock );
                if ( m_hold && ( m_holdAddress.IsNULLOrEmpty() || m_holdAddress == box->address.Lowercase() ) )
                {
                    SHeld held;
                    held.recipients.push_back( box->address );
                    held.data = data;
                    m_held.push_back( held );
                    continue;
                }
            }
            box->imap.AddMessage( data );

            // the POP3 maildrop is not guarded: it is only added to while no session reads it
            for ( UInt32 waited=0; waited<3000 && box->pop3.GetLiveConnections() > 0; ++waited )
                MT::ThreadDelay( 1 );
            {
                MT::CScopeMutex lock( m_lock );
                box->pop3.AddMessage( "uid-" + CORE::UInt32ToString( ++m_pop3Counter ), data );
                ++m_copiesDelivered;
            }
            any = true;
        }
        return any;
    }

    static CORE::CString StripAttachments( const CORE::CString& data )
    {
        COM::CMimeParser parser;
        COM::CEMailMessage message;
        if ( !parser.ParseMessage( data.C_String(), data.ByteSize() - 1, message ) || message.attachments.empty() )
            return data;
        message.attachments.clear();
        COM::CMimeComposer composer;
        CORE::CDynamicBuffer out;
        CORE::CString error;
        if ( !composer.Compose( message, out, error ) )
            return data;
        return CORE::CString( static_cast< const char* >( out.GetConstBufferPtr() ), out.GetDataSize() );
    }

    private:

    GUCEF::vector< CTestEMailBox* > m_boxes;
    UInt32 m_dropCount;
    CORE::CString m_dropContaining;
    UInt32 m_duplicateCount;
    bool m_hold;
    CORE::CString m_holdAddress;
    GUCEF::vector< SHeld > m_held;
    bool m_stripAttachments;
    GUCEF::vector< CORE::CString > m_accepted;
    UInt32 m_copiesDelivered;
    UInt32 m_pop3Counter;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

} /* namespace TESTFW */
} /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILWORLD_H ? */
