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

#ifndef GUCEF_COM_CMAILCLIENTBASE_H
#include "gucefCOM_CMailClientBase.h"
#define GUCEF_COM_CMAILCLIENTBASE_H
#endif /* GUCEF_COM_CMAILCLIENTBASE_H ? */

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

#ifndef GUCEF_COM_CMAILMESSAGE_H
#include "gucefCOM_CMailMessage.h"
#define GUCEF_COM_CMAILMESSAGE_H
#endif /* GUCEF_COM_CMAILMESSAGE_H ? */

#ifndef GUCEF_COM_CMIMEPARSER_H
#include "gucefCOM_CMimeParser.h"
#define GUCEF_COM_CMIMEPARSER_H
#endif /* GUCEF_COM_CMIMEPARSER_H ? */

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

enum EImapCommand
{
    IMAPCMD_CAPABILITY = 0,
    IMAPCMD_NOOP,
    IMAPCMD_LIST,
    IMAPCMD_SELECT,
    IMAPCMD_EXAMINE,
    IMAPCMD_STATUS,
    IMAPCMD_SEARCH,
    IMAPCMD_FETCH,
    IMAPCMD_STORE,
    IMAPCMD_EXPUNGE,
    IMAPCMD_CLOSE,
    IMAPCMD_CREATE,
    IMAPCMD_DELETE,
    IMAPCMD_APPEND,
    IMAPCMD_IDLE,
    IMAPCMD_COPY,
    IMAPCMD_MOVE
};

/** How STORE changes the flags */
enum EImapStoreMode
{
    IMAPSTORE_ADD = 0,
    IMAPSTORE_REMOVE,
    IMAPSTORE_REPLACE
};

/*-------------------------------------------------------------------------*/

/** One line of the answer to LIST */
class GUCEF_COM_PUBLIC_CPP CImapMailboxEntry
{
    public:

    CORE::CString name;                         /**< UTF-8 */
    CORE::CString hierarchyDelimiter;
    CORE::CString::StringVector attributes;     /**< \HasNoChildren, \Sent, \Trash ... */

    CImapMailboxEntry( void );
};

/** What the server said about a mailbox that was selected or asked about with STATUS */
class GUCEF_COM_PUBLIC_CPP CImapMailboxStatus
{
    public:

    CORE::CString name;
    UInt32 exists;
    UInt32 recent;
    UInt32 unseen;
    UInt32 uidNext;
    UInt32 uidValidity;                         /**< the number that says whether the UIDs of the mailbox are still the ones that were known */
    bool readOnly;
    CORE::CString::StringVector flags;
    CORE::CString::StringVector permanentFlags;

    CImapMailboxStatus( void );
};

/** The outcome of one command */
class GUCEF_COM_PUBLIC_CPP CImapCommandResult : public CORE::CICloneable
{
    public:

    UInt32 requestId;
    EImapCommand command;
    bool ok;                                    /**< the server said OK */
    CORE::CString status;                       /**< OK, NO or BAD, empty when the command was not carried out */
    EMailErrorClass errorClass;
    CORE::CString text;                         /**< the text of the server, or of ours: for a log only after redaction */
    CORE::CString code;                         /**< response code of the completion: TRYCREATE, READ-ONLY, ... */
    GUCEF::vector< CImapMailboxEntry > mailboxes;       /**< LIST */
    CImapMailboxStatus mailbox;                         /**< SELECT, EXAMINE, STATUS */
    bool uidValidityChanged;                            /**< SELECT with an expected UIDVALIDITY that is not the one of the mailbox: the UIDs of before mean nothing */
    GUCEF::vector< UInt32 > numbers;                    /**< SEARCH: sequence numbers or UIDs */
    GUCEF::vector< CImapFetchedItem > fetched;          /**< FETCH, and the flags that STORE reports */
    UInt32 appendUid;                                   /**< APPEND with UIDPLUS */
    UInt32 copyUid;                                     /**< COPY and MOVE with UIDPLUS: the UID of the first message in the target mailbox, 0 when the server did not say */
    CORE::CString::StringVector capabilities;           /**< CAPABILITY */

    CImapCommandResult( void );
    CImapCommandResult( const CImapCommandResult& src );
    virtual ~CImapCommandResult() GUCEF_VIRTUAL_OVERRIDE;
    CImapCommandResult& operator=( const CImapCommandResult& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/** What happened in the mailbox while the client was idle */
class GUCEF_COM_PUBLIC_CPP CImapIdleUpdate : public CORE::CICloneable
{
    public:

    enum EKind
    {
        KIND_EXISTS = 0,        /**< 'number' messages are in the mailbox now */
        KIND_EXPUNGE,           /**< the message with sequence number 'number' is gone */
        KIND_RECENT,
        KIND_FLAGS              /**< the flags of the message with this sequence number changed: 'flags', and the uid when the server says */
    };

    EKind kind;
    UInt32 number;
    bool hasUid;
    UInt32 uid;
    CORE::CString::StringVector flags;

    CImapIdleUpdate( void );
    CImapIdleUpdate( const CImapIdleUpdate& src );
    virtual ~CImapIdleUpdate() GUCEF_VIRTUAL_OVERRIDE;
    CImapIdleUpdate& operator=( const CImapIdleUpdate& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/** What a fetch of new mail does */
class GUCEF_COM_PUBLIC_CPP CImapFetchOptions
{
    public:

    CORE::CString mailbox;                      /**< "INBOX" */
    UInt32 lastUidValidity;                     /**< the UIDVALIDITY of the last fetch, 0 for none */
    UInt32 lastUid;                             /**< the highest UID that was handled by the last fetch, 0 for none */
    UInt32 maxMessages;                         /**< at most this many messages per fetch, 0: all */
    bool markSeen;                              /**< the messages that are fetched become \Seen */
    bool deleteAfterFetch;                      /**< the messages that were fetched are deleted and expunged */
    GUCEF::vector< CORE::CString > headerNames; /**< only messages that have these headers are fetched (the search is done by the server)... */
    GUCEF::vector< CORE::CString > headerTexts; /**< ...with this text in them, one text per name, empty: the header only has to be there */
    CMimeLimits mimeLimits;

    CImapFetchOptions( void );
};

/** A message that a fetch brought */
class GUCEF_COM_PUBLIC_CPP CImapFetchedMessage
{
    public:

    UInt32 uid;
    UInt32 size;
    CORE::CString::StringVector flags;
    CMailMessage message;
    bool parsed;

    CImapFetchedMessage( void );
};

/** What a fetch of new mail found */
class GUCEF_COM_PUBLIC_CPP CImapFetchResult : public CORE::CICloneable
{
    public:

    bool success;
    EMailErrorClass errorClass;
    CORE::CString errorDescription;
    UInt32 uidValidity;
    bool uidValidityChanged;                    /**< the mailbox has other UIDs than the ones lastUid was about: all messages were looked at, none are skipped on the UID */
    UInt32 uidNext;
    UInt32 highestUid;                          /**< the highest UID handled, the lastUid for the next fetch */
    GUCEF::vector< CImapFetchedMessage > messages;

    CImapFetchResult( void );
    CImapFetchResult( const CImapFetchResult& src );
    virtual ~CImapFetchResult() GUCEF_VIRTUAL_OVERRIDE;
    CImapFetchResult& operator=( const CImapFetchResult& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  An IMAP client (RFC 3501, IMAP4rev1, with the parts of IMAP4rev2 that servers of rev1 have too): STARTTLS or TLS from
 *  the start, LOGIN or AUTHENTICATE (PLAIN, LOGIN, SCRAM-SHA-256 and XOAUTH2 through CMailSasl), LIST, SELECT and
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
 *  For one shot use FetchNewSync() (blocks the calling thread) and CMailOperations::FetchNewImapAsync().
 */
class GUCEF_COM_PUBLIC_CPP CImapClient : public CMailClientBase
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
    UInt32 AppendMessage( const CORE::CString& mailbox, const CMailMessage& message, const CORE::CString::StringVector& flags = CORE::CString::StringVector() );

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
    static bool FetchNewSync( const CImapClientSettings& settings, const CImapFetchOptions& options, CImapFetchResult& result, CIMailCredentialProvider* credentialProvider = GUCEF_NULL );

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
    virtual void OnClientFailed( const CMailErrorEventData& error ) GUCEF_VIRTUAL_OVERRIDE;
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
