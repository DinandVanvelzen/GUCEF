/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CIMAPCLIENT_H
#define GUCEF_COM_CIMAPCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CEMAILCLIENTBASE_H
#include "gucefCOM_CEMailClientBase.h"
#define GUCEF_COM_CEMAILCLIENTBASE_H
#endif /* GUCEF_COM_CEMAILCLIENTBASE_H ? */

#ifndef GUCEF_COM_CIMAPCLIENTSETTINGS_H
#include "gucefCOM_CImapClientSettings.h"
#define GUCEF_COM_CIMAPCLIENTSETTINGS_H
#endif /* GUCEF_COM_CIMAPCLIENTSETTINGS_H ? */

#ifndef GUCEF_COM_CIMAPRESPONSE_H
#include "gucefCOM_CImapResponse.h"
#define GUCEF_COM_CIMAPRESPONSE_H
#endif /* GUCEF_COM_CIMAPRESPONSE_H ? */

#ifndef GUCEF_COM_CIMAPSEARCHCRITERIA_H
#include "gucefCOM_CImapSearchCriteria.h"
#define GUCEF_COM_CIMAPSEARCHCRITERIA_H
#endif /* GUCEF_COM_CIMAPSEARCHCRITERIA_H ? */

#ifndef GUCEF_COM_CEMAILMESSAGE_H
#include "gucefCOM_CEMailMessage.h"
#define GUCEF_COM_CEMAILMESSAGE_H
#endif /* GUCEF_COM_CEMAILMESSAGE_H ? */

#ifndef GUCEF_COM_CMIMEPARSER_H
#include "gucefCOM_CMimeParser.h"
#define GUCEF_COM_CMIMEPARSER_H
#endif /* GUCEF_COM_CMIMEPARSER_H ? */

#ifndef GUCEF_COM_CIMAPCOMMANDRESULT_H
#include "gucefCOM_CImapCommandResult.h"
#define GUCEF_COM_CIMAPCOMMANDRESULT_H
#endif /* GUCEF_COM_CIMAPCOMMANDRESULT_H ? */

#ifndef GUCEF_COM_CIMAPIDLEUPDATE_H
#include "gucefCOM_CImapIdleUpdate.h"
#define GUCEF_COM_CIMAPIDLEUPDATE_H
#endif /* GUCEF_COM_CIMAPIDLEUPDATE_H ? */

#ifndef GUCEF_COM_CIMAPFETCHRESULT_H
#include "gucefCOM_CImapFetchResult.h"
#define GUCEF_COM_CIMAPFETCHRESULT_H
#endif /* GUCEF_COM_CIMAPFETCHRESULT_H ? */

#ifndef GUCEF_COM_CIMAPFETCHOPTIONS_H
#include "gucefCOM_CImapFetchOptions.h"
#define GUCEF_COM_CIMAPFETCHOPTIONS_H
#endif /* GUCEF_COM_CIMAPFETCHOPTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How STORE changes the flags */
enum EImapStoreMode
{
    IMAPSTORE_ADD = 0,
    IMAPSTORE_REMOVE,
    IMAPSTORE_REPLACE
};

/*-------------------------------------------------------------------------*/

/**
 *  An IMAP client (RFC 3501, IMAP4rev1, with the parts of IMAP4rev2 that servers of rev1 have too): STARTTLS or TLS from
 *  the start, LOGIN or AUTHENTICATE (PLAIN, LOGIN, SCRAM-SHA-256 and XOAUTH2 through CEMailSaslFactory), LIST, SELECT and
 *  EXAMINE with the UIDVALIDITY check, SEARCH, FETCH (envelope, flags, body structure, sections, partial), STORE,
 *  EXPUNGE, UID EXPUNGE (UIDPLUS), COPY, MOVE (RFC 6851), CLOSE, CREATE, DELETE, APPEND and IDLE (RFC 2177).
 *
 *  Commands are given when the client is ready and are sent one at a time, in order; every command raises
 *  CommandCompletedEvent with its result. IDLE holds the connection open and raises IdleUpdateEvent for what happens in
 *  the mailbox; a command that is given while idling ends the IDLE first, and the IDLE starts again when asked.
 *  What goes into a command (mailbox names, search texts, flags) is quoted, sent as a literal or refused: nothing
 *  can end a command and start another.
 *
 *      CImapClient client;
 *      SubscribeTo( &client, CImapClient::ReadyEvent, ... );
 *      client.Connect( settings );
 *      // when ready:  client.Select( "INBOX", false, lastUidValidity );  client.Idle();
 *
 *  For one shot use FetchNewSync() (blocks the calling thread) and CEMailOperations::FetchNewImapAsync().
 */
class GUCEF_COM_PUBLIC_CPP CImapClient : public CEMailClientBase
{
    public:

    static const CORE::CEvent CommandCompletedEvent;    /**< TCommandCompletedEventData */
    static const CORE::CEvent IdleStartedEvent;         /**< the server is waiting for changes */
    static const CORE::CEvent IdleUpdateEvent;          /**< TIdleUpdateEventData */

    typedef CImapCommandResult TCommandCompletedEventData;
    typedef CImapIdleUpdate TIdleUpdateEventData;

    static void RegisterEvents( void );

    CImapClient( void );

    explicit CImapClient( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CImapClient() GUCEF_VIRTUAL_OVERRIDE;

    bool Connect( const CImapClientSettings& settings );

    /** All of these return the id that the result carries, 0 when the command is refused right away: not connected, an argument that cannot be sent (the reason: GetLastRefusal()) */
    UInt32 Capability( void );
    UInt32 Noop( void );
    UInt32 List( const CORE::CString& reference, const CORE::CString& pattern );
    UInt32 Select( const CORE::CString& mailbox, bool readOnly = false, UInt32 expectedUidValidity = 0 );
    UInt32 Status( const CORE::CString& mailbox );
    UInt32 Search( const CImapSearchCriteria& criteria, bool useUid = true );
    UInt32 Fetch( const CORE::CString& sequenceSet, const CImapFetchSpec& spec, bool useUid = true );
    UInt32 Store( const CORE::CString& sequenceSet, EImapStoreMode mode, const CORE::CString::StringVector& flags, bool useUid = true, bool silent = true );
    UInt32 Expunge( void );
    UInt32 UidExpunge( const CORE::CString& uidSet );

    /** Puts a copy of the messages in another mailbox; the originals stay. Mailbox and set are checked like in every command */
    UInt32 Copy( const CORE::CString& sequenceSet, const CORE::CString& mailbox, bool useUid = true );

    /** Moves the messages to another mailbox in one step (RFC 6851); refused (0, GetLastRefusal()) when the server does not offer MOVE, so that a caller can do a copy and a delete of its own */
    UInt32 Move( const CORE::CString& sequenceSet, const CORE::CString& mailbox, bool useUid = true );
    UInt32 Close( void );
    UInt32 CreateMailbox( const CORE::CString& mailbox );
    UInt32 DeleteMailbox( const CORE::CString& mailbox );

    /** Adds a message (bytes of an Internet message with CR LF line ends) to a mailbox. 'internalDate' like 02-Nov-2020 10:11:12 +0000, may be empty */
    UInt32 AppendRaw( const CORE::CString& mailbox, const void* data, UInt32 size, const CORE::CString::StringVector& flags = CORE::CString::StringVector(), const CORE::CString& internalDate = CORE::CString::Empty );
    UInt32 AppendMessage( const CORE::CString& mailbox, const CEMailMessage& message, const CORE::CString::StringVector& flags = CORE::CString::StringVector() );

    /** Waits for changes in the selected mailbox; needs the IDLE capability. The command completes when IdleDone() is called, a command is given, or the server ends it. IdleDone() before the server has confirmed the IDLE ends it as soon as the server has */
    UInt32 Idle( void );

    /** Ends the IDLE */
    void IdleDone( void );

    bool IsIdling( void ) const;

    /** Whether the server offered the capability (IDLE, UIDPLUS, ...) in its last CAPABILITY */
    bool HasCapability( const CORE::CString& name ) const;

    /** The mailbox that is selected, with what the client knows of it */
    const CImapMailboxStatus& GetSelectedMailbox( void ) const;

    bool IsMailboxSelected( void ) const;

    const CORE::CString& GetLastRefusal( void ) const;

    const CImapClientSettings& GetSettings( void ) const;

    virtual const CORE::CString& GetProtocolName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Fetches what is new in a mailbox from the calling thread: connects (on a pulse generator that this thread owns),
     *  selects the mailbox, remembers its UIDVALIDITY, searches the UIDs above lastUid, fetches and parses the messages,
     *  marks or deletes them when asked, and says LOGOUT.
     */
    static bool FetchNewSync( const CImapClientSettings& settings, const CImapFetchOptions& options, CImapFetchResult& result, CIEMailCredentialProvider* credentialProvider = GUCEF_NULL );

    protected:

    virtual bool SendSaslStart( const CORE::CString& mechanism, bool hasInitialResponse, const CORE::CString& base64InitialResponse ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendSaslResponse( const CORE::CString& base64Response ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SendSaslCancel( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SendQuitCommand( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnServerLine( const CORE::CString& line ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnServerBytes( const CORE::CDynamicBuffer& bytes ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsServerLineLoggable( const CORE::CString& line ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnConnectionEstablished( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnTlsEstablished( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnClientFailed( const CEMailErrorEventData& error ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnClientClosed( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnTimerTick( UInt64 nowInMs ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    enum EInternal
    {
        INTERNAL_NONE = 0,
        INTERNAL_CAPABILITY,
        INTERNAL_STARTTLS,
        INTERNAL_AUTHENTICATE,
        INTERNAL_LOGIN,
        INTERNAL_LOGOUT
    };

    enum EPhase
    {
        PHASE_GREETING = 0,
        PHASE_SETUP,                    /**< capabilities, STARTTLS, login */
        PHASE_READY
    };

    struct SCommand
    {
        UInt32 requestId;
        EImapCommand command;
        EInternal internal;
        CORE::CString tag;
        CImapCommandParts parts;
        UInt32 expectedUidValidity;
        CORE::CString mailbox;
        bool silentUse;
        CImapCommandResult result;
    };

    CImapClient( const CImapClient& src );
    CImapClient& operator=( const CImapClient& src );

    void Initialize( void );
    UInt32 Queue( EImapCommand command, const CImapCommandParts& parts, const CORE::CString& mailbox = CORE::CString::Empty, UInt32 expectedUidValidity = 0 );
    void QueueInternal( EInternal internal, const CImapCommandParts& parts );
    void SendNext( void );
    void SendFirstPart( SCommand* command );
    void SendLiteralAndContinue( void );
    CORE::CString NextTag( void );
    void HandleResponse( const CORE::CDynamicBuffer& response );
    void OnGreeting( const CImapResponse& response );
    void OnUntagged( const CImapResponse& response );
    void OnUntaggedStatus( const CImapResponse& response );
    void OnContinuation( const CImapResponse& response );
    void OnTagged( const CImapResponse& response );
    void OnInternalCompleted( const CImapResponse& response );
    void ReadCapabilities( const CORE::CString::StringVector& words );
    void ProceedAfterCapabilities( void );
    void ProceedToLogin( void );
    void LoggedIn( void );
    void CompleteCurrent( const CImapResponse& response );
    void FailAllCommands( EMailErrorClass errorClass, const CORE::CString& description );
    void RaiseIdleUpdate( const CImapIdleUpdate& update );
    void SendDone( bool renewing );

    bool IsIdleCommandPending( void ) const;
    UInt32 Refuse( const char* reason );
    bool IsValidFlag( const CORE::CString& flag ) const;
    static CORE::CString FlagsToText( const CORE::CString::StringVector& flags );

    private:

    CImapClientSettings m_settings;
    EPhase m_phase;
    GUCEF::vector< SCommand* > m_queue;
    SCommand* m_current;
    UInt32 m_tagCounter;
    UInt32 m_literalIndex;                      /**< the literal of the current command that is sent next */
    bool m_awaitingContinuation;
    CORE::CDynamicBuffer m_assembly;            /**< the response that is coming in: lines and literals */
    bool m_assemblyHasLiteral;
    bool m_expectingLiteral;
    UInt32 m_responseCount;
    GUCEF::map< CORE::CString, CORE::CString > m_capabilities;
    CImapMailboxStatus m_selected;
    bool m_mailboxSelected;
    bool m_idling;
    bool m_idleRenewing;
    bool m_idleDoneSent;
    bool m_idleEndWanted;
    UInt64 m_idleStartedAt;
    bool m_loggedIn;
    bool m_logoutSent;
    CORE::CString m_lastRefusal;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPCLIENT_H ? */
