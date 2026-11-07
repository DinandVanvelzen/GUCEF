/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINBOX_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINBOX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_COM_CIMAPCLIENT_H
#include "gucefCOM_CImapClient.h"
#define GUCEF_COM_CIMAPCLIENT_H
#endif /* GUCEF_COM_CIMAPCLIENT_H ? */

#ifndef GUCEF_COM_CPOP3CLIENT_H
#include "gucefCOM_CPop3Client.h"
#define GUCEF_COM_CPOP3CLIENT_H
#endif /* GUCEF_COM_CPOP3CLIENT_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHCODEC_H
#include "matchprovidermail_CMailMatchCodec.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHCODEC_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHCODEC_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHPROVIDERSETTINGS_H
#include "matchprovidermail_CMailMatchProviderSettings.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHPROVIDERSETTINGS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHPROVIDERSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The data of CMailInbox::MessageEvent: a mail of a match that was read completely */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailInboxMessage : public CORE::CICloneable
{
    public:

    CMailIncoming incoming;
    COM::CMailMessage message;
    UInt32 rawSize;

    CMailInboxMessage( void );
    CMailInboxMessage( const CMailInboxMessage& src );
    virtual ~CMailInboxMessage() GUCEF_VIRTUAL_OVERRIDE;
    CMailInboxMessage& operator=( const CMailInboxMessage& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/** The data of CMailInbox::RefusedEvent: a mail that was not read, and why. No addresses, no content */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailInboxRefusal : public CORE::CICloneable
{
    public:

    CORE::CString matchId;
    UInt32 revision;
    TMailMatchKind kind;
    CORE::CString reason;

    CMailInboxRefusal( void );
    CMailInboxRefusal( const CMailInboxRefusal& src );
    virtual ~CMailInboxRefusal() GUCEF_VIRTUAL_OVERRIDE;
    CMailInboxRefusal& operator=( const CMailInboxRefusal& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/** The data of CMailInbox::NoticeEvent: something about the mailbox that the player should read. Plain words, no addresses, no content */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailInboxNotice : public CORE::CICloneable
{
    public:

    CORE::CString code;                 /**< MOVE_NOT_POSSIBLE, MOVE_FAILED, DELETE_FAILED */
    CORE::CString description;

    CMailInboxNotice( void );
    CMailInboxNotice( const CMailInboxNotice& src );
    virtual ~CMailInboxNotice() GUCEF_VIRTUAL_OVERRIDE;
    CMailInboxNotice& operator=( const CMailInboxNotice& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  Watches the mailbox of the player for the mail of matches. Only mail that has the header X-Redux-Match is ever asked for
 *  (the search is done by the server); of that mail the headers are read first, and only a mail of a known protocol that is
 *  not our own, not automatic and not read before, and not above the size cap, is read completely. Nothing is deleted.
 *
 *  IMAP: a persistent connection selects the folder, remembers its UIDVALIDITY and the highest UID that was handled,
 *  waits with IDLE (renewed by the client before the server gives up) or, when the server has no IDLE or it is switched
 *  off, checks every poll interval. A new UIDVALIDITY makes the UIDs mean nothing: the UID position starts again and the set
 *  of message ids, which is what de-duplicates, stays. Processed messages get \Seen and then, by the setting processedAction,
 *  stay, are copied to a folder or are moved to it. A move is one MOVE command when the server has the extension; else a copy,
 *  the deleted flag on that one UID and a UID EXPUNGE of that UID (UIDPLUS), so that no other message that carries the deleted
 *  flag can go; else only the copy is made and the player is told once. Only a message of a match that was processed completely
 *  is ever copied, moved or deleted. A step that fails leaves the message where it is.
 *
 *  POP3: every poll interval a session lists the unique ids, reads the headers (TOP) of the new ones and the whole of
 *  those that are for a match, and ends. The unique ids that were seen are kept, bounded.
 *
 *  The position (UIDVALIDITY, UID) and the sets are written to a file after every handled message. A connection that fails
 *  is tried again later with a growing wait. Time comes from Update().
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailInbox : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent MessageEvent;     /**< TMessageEventData: handled completely before the message is marked as processed */
    static const CORE::CEvent RefusedEvent;     /**< TRefusedEventData */
    static const CORE::CEvent StateEvent;       /**< TStateEventData: online or offline */
    static const CORE::CEvent NoticeEvent;      /**< TNoticeEventData: a move or a deletion that could not be done */
    typedef CMailInboxMessage TMessageEventData;
    typedef CMailInboxRefusal TRefusedEventData;
    typedef CMailInboxNotice TNoticeEventData;
    typedef MULTIPLAY::CMatchProviderState TStateEventData;

    static void RegisterEvents( void );

    CMailInbox( void );

    virtual ~CMailInbox() GUCEF_VIRTUAL_OVERRIDE;

    /** The settings and the credential provider are not owned and have to outlive the inbox */
    void Configure( const CMailMatchProviderSettings* settings, COM::CIMailCredentialProvider* credentials, const CORE::CString& statePath );

    bool Start( Int64 nowMs );

    void Stop( void );

    bool IsStarted( void ) const;

    bool IsOnline( void ) const;

    /** Checks the mailbox as soon as the connection allows */
    void CheckNow( void );

    /** The timer of the provider: reconnects, polls, renews */
    void Update( Int64 nowMs );

    /** The UIDVALIDITY and UID position, for a display or a test */
    UInt32 GetUidValidity( void ) const;
    UInt32 GetLastUid( void ) const;
    UInt32 GetSeenCount( void ) const;
    UInt32 GetHandledCount( void ) const;
    bool IsIdling( void ) const;

    private:

    typedef CORE::CTEventHandlerFunctor< CMailInbox > TEventCallback;

    enum EStep
    {
        STEP_NONE = 0,
        STEP_SELECT,
        STEP_SEARCH,
        STEP_FETCH_HEADER,
        STEP_FETCH_BODY,
        STEP_STORE,
        STEP_APPEND,
        STEP_COPY,                  /**< a COPY of a message that is being moved */
        STEP_MOVE,                  /**< the MOVE command */
        STEP_FLAG_DELETED,          /**< \Deleted on the one UID that was copied */
        STEP_EXPUNGE_UID,           /**< UID EXPUNGE of that UID */
        STEP_UNFLAG_DELETED,        /**< the flag is taken off again when the expunge failed */
        STEP_IDLE,
        STEP_ENDING_IDLE,
        STEP_NOOP,
        STEP_POP_UIDL,
        STEP_POP_LIST,
        STEP_POP_TOP,
        STEP_POP_RETR,
        STEP_POP_DELE
    };

    CMailInbox( const CMailInbox& src );
    CMailInbox& operator=( const CMailInbox& src );

    bool LoadState( void );
    bool SaveState( void );
    bool IsSeen( const CORE::CString& id ) const;
    void AddSeen( const CORE::CString& id );
    void Connect( Int64 nowMs );
    void Disconnect( void );
    void ScheduleReconnect( Int64 nowMs );
    void SetOnline( bool online, const char* description, bool tellAlways = false );

    // IMAP
    void OnImapReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnImapCommand( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnImapIdleUpdate( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnClientError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnClientDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void ImapStartCycle( void );
    void ImapNextUid( void );
    void ImapHandleHeader( const COM::CImapFetchedItem& item );
    void ImapHandleBody( const COM::CImapFetchedItem& item );
    void ImapFinishMessage( bool markSeen, bool processed );
    void ImapProcessedOrNext( void );
    bool ImapStartProcessedAction( void );
    void ImapMoveFailed( void );
    void Notice( const char* code, const char* description );
    void ImapWait( void );

    // POP3
    void OnPop3Ready( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPop3Command( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void Pop3NextMessage( void );
    void Pop3EndSession( void );

    // both
    bool DecideOnHeaders( const COM::CMailHeaders& headers, UInt32 size, CMailIncoming& incoming, bool& markAsSeen );
    bool HandleCompleteMessage( const CMailIncoming& incoming, const void* data, UInt32 size );

    private:

    const CMailMatchProviderSettings* m_settings;
    COM::CIMailCredentialProvider* m_credentials;
    CORE::CString m_statePath;

    bool m_started;
    bool m_online;
    Int64 m_nowMs;
    Int64 m_nextConnectAt;
    Int64 m_nextPollAt;
    Int64 m_backoffMs;
    bool m_checkRequested;

    COM::CImapClient* m_imap;
    COM::CPop3Client* m_pop3;
    GUCEF::vector< void* > m_retired;
    GUCEF::vector< bool > m_retiredIsImap;
    COM::CImapClientSettings m_imapSettings;
    COM::CPop3ClientSettings m_pop3Settings;
    EStep m_step;
    UInt32 m_requestId;
    bool m_supportsIdle;
    UInt32 m_existsAtSearch;                        /**< the number of messages the server had when the last search was asked: a change after it is mail the search did not see */

    // position and sets, written to the state file
    UInt32 m_uidValidity;
    UInt32 m_lastUid;
    GUCEF::vector< CORE::CString > m_seenOrder;
    CORE::CString::StringSet m_seen;
    UInt32 m_handled;

    // the cycle
    GUCEF::vector< UInt32 > m_uids;
    size_t m_uidIndex;
    UInt32 m_currentUid;
    CMailIncoming m_currentIncoming;
    UInt32 m_currentSize;
    bool m_currentInteresting;
    bool m_actionPending;                           /**< the message was processed completely: processedAction is carried out for it */
    bool m_moveDeletes;                             /**< the copy that is made is the first half of a move: the original is deleted after it */
    bool m_degradedNoticeGiven;                     /**< the player was told that the server cannot move */
    CORE::CDynamicBuffer m_currentRaw;
    GUCEF::vector< CORE::CString > m_popUids;       /**< POP3: unique ids of this session, index = message number - 1 */
    GUCEF::vector< UInt32 > m_popSizes;
    size_t m_popIndex;
    CORE::CString m_popCurrentUid;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINBOX_H ? */
