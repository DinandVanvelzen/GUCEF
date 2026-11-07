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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILOUTBOX_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILOUTBOX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_COM_CSMTPCLIENT_H
#include "gucefCOM_CSmtpClient.h"
#define GUCEF_COM_CSMTPCLIENT_H
#endif /* GUCEF_COM_CSMTPCLIENT_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Where a mail is on its way to the others */
enum EMailDeliveryState
{
    MAILDELIVERY_NONE      = 0 ,    /**< nothing is known of a mail for this */
    MAILDELIVERY_QUEUED    = 1 ,    /**< in the outbox, not yet accepted by the mail server (a temporary failure keeps it here) */
    MAILDELIVERY_SENT      = 2 ,    /**< the mail server of the player accepted it; no sign yet that it arrived */
    MAILDELIVERY_DELIVERED = 3 ,    /**< every recipient showed that it arrived (acknowledgement, or their next move) */
    MAILDELIVERY_FAILED    = 4      /**< the mail server refused it for good */
};
typedef enum EMailDeliveryState TMailDeliveryState;

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailDeliveryStateToString( TMailDeliveryState state );

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The data of CMailOutbox::DeliveryChangedEvent */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailDeliveryEvent : public CORE::CICloneable
{
    public:

    CORE::CString matchId;
    UInt32 revision;
    TMailMatchKind kind;
    TMailDeliveryState state;
    CORE::CString description;      /**< a phrase of ours: no addresses, no content */

    CMailDeliveryEvent( void );
    CMailDeliveryEvent( const CMailDeliveryEvent& src );
    virtual ~CMailDeliveryEvent() GUCEF_VIRTUAL_OVERRIDE;
    CMailDeliveryEvent& operator=( const CMailDeliveryEvent& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/** One mail waiting to be sent or to be confirmed; stored as one file in the outbox directory */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailOutboxItem
{
    public:

    UInt32 id;
    TMailMatchKind kind;
    CORE::CString matchId;
    UInt32 revision;
    CORE::CString messageId;
    CORE::CString envelopeFrom;
    CORE::CString::StringVector recipients;         /**< addresses of the envelope */
    CORE::CString::StringVector awaiting;           /**< recipients (lower case) that have not shown that the mail arrived */
    CORE::CString packageSha256;
    bool needsConfirmation;                         /**< the item stays until the recipients confirm; else it ends when the server accepted it */
    TMailDeliveryState state;
    UInt32 attempts;
    Int64 createdAt;
    Int64 nextAttemptAt;
    Int64 lastSentAt;
    CORE::CDynamicBuffer message;                   /**< the mail as it goes over the wire */

    CMailOutboxItem( void );

    /** The bytes of the file, a fixed layout */
    bool ToBytes( CORE::CDynamicBuffer& bytes ) const;

    /** Strict reader of the bytes of a file */
    bool FromBytes( const void* data, UInt32 size );
};

/*-------------------------------------------------------------------------*/

/**
 *  The mails the player has made and the mail server has not yet taken, or whose arrival nobody confirmed. Every mail is a file
 *  in the outbox directory (written through a temporary file and a rename), so that a crash or a restart loses
 *  nothing.
 *
 *  Sending: when something is due, an SMTP client connects, hands over the due mails one after another and disconnects. A
 *  temporary failure of the server keeps the mail in the outbox and tries again later (the wait doubles up to a limit); a
 *  refusal for good makes it FAILED, which is reported. A mail that was accepted but not confirmed is sent again after the
 *  quiet period to the recipients that did not confirm, with the same Message-ID, so a copy that arrives twice is ignored by
 *  the receiver. Confirmation is Acknowledge(): an ACK mail, or the next move of the recipient.
 *
 *  Time comes from the caller (Update() is told the time), so a test or a clock of the application drives it.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailOutbox : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent DeliveryChangedEvent;         /**< TDeliveryChangedEventData */
    typedef CMailDeliveryEvent TDeliveryChangedEventData;

    static void RegisterEvents( void );

    CMailOutbox( void );

    virtual ~CMailOutbox() GUCEF_VIRTUAL_OVERRIDE;

    /** The settings and the credential provider are not owned and have to outlive the outbox */
    void Configure( const CMailMatchProviderSettings* settings, COM::CIMailCredentialProvider* credentials, const CORE::CString& directory );

    /** Reads the files of the directory; damaged files are left alone and counted */
    bool Load( UInt32* skipped = GUCEF_NULL );

    /** Takes the item (the id is assigned) and writes its file. @return the id, 0 when the file could not be written */
    UInt32 Enqueue( CMailOutboxItem& item, Int64 nowMs );

    /** Sends what is due; to be called at the interval of the provider's timer */
    void Update( Int64 nowMs );

    /**
     *  The recipient showed that the mails of the match up to the revision arrived (INVITE items are matched with revision 0 and
     *  any revision). The items that have no recipient left to wait for end.
     *  @return the number of items that ended
     */
    UInt32 Acknowledge( const CORE::CString& matchId, const CORE::CString& address, UInt32 upToRevision, bool includeInvites );

    /** Makes the items of the revision of the match due at once. @return the number of items */
    UInt32 ResendNow( const CORE::CString& matchId, UInt32 revision, Int64 nowMs );

    /** The state of the newest item of the revision of the match that carries a turn; MAILDELIVERY_NONE when there is none */
    TMailDeliveryState GetState( const CORE::CString& matchId, UInt32 revision, UInt32* attempts = GUCEF_NULL ) const;

    /** Removes the items of a match, files included */
    void RemoveMatch( const CORE::CString& matchId );

    /** Removes the items of a match that stay until the recipients confirm them (turns, invitations): nothing is sent again. Returns how many */
    UInt32 RemoveUnconfirmed( const CORE::CString& matchId );

    UInt32 GetItemCount( void ) const;

    UInt32 GetUnconfirmedCount( void ) const;

    void GetItems( const CORE::CString& matchId, GUCEF::vector< CMailOutboxItem >& items ) const;

    /** Disconnects from the mail server */
    void Shutdown( void );

    private:

    typedef CORE::CTEventHandlerFunctor< CMailOutbox > TEventCallback;
    typedef GUCEF::map< UInt32, CMailOutboxItem > TItemMap;

    CMailOutbox( const CMailOutbox& src );
    CMailOutbox& operator=( const CMailOutbox& src );

    CORE::CString GetItemPath( UInt32 id ) const;
    bool Persist( const CMailOutboxItem& item );
    void Remove( UInt32 id );
    void RaiseState( const CMailOutboxItem& item, TMailDeliveryState state, const char* description );
    void StartSession( Int64 nowMs );
    void EndSession( void );
    void OnReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSendCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSessionError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSessionDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void FailInFlight( Int64 nowMs, const char* description );
    void SubmitDue( void );
    Int64 BackoffFor( UInt32 failures ) const;
    void ItemFailedTemporarily( CMailOutboxItem& item, const char* description );

    private:

    const CMailMatchProviderSettings* m_settings;
    COM::CIMailCredentialProvider* m_credentials;
    CORE::CString m_directory;
    TItemMap m_items;
    UInt32 m_nextId;
    COM::CSmtpClient* m_client;
    bool m_clientReady;
    bool m_clientFinished;
    Int64 m_nowMs;
    Int64 m_sessionBackoffMs;
    Int64 m_nextSessionAt;
    GUCEF::map< UInt32, UInt32 > m_inFlight;        /**< request id of the client -> item id */
    GUCEF::set< UInt32 > m_sessionItems;            /**< items that were counted as an attempt in the session that is going */
    GUCEF::map< UInt32, UInt32 > m_failures;        /**< failures in a row per item, for the wait before the next try */
    GUCEF::vector< COM::CSmtpClient* > m_retired;
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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILOUTBOX_H ? */
