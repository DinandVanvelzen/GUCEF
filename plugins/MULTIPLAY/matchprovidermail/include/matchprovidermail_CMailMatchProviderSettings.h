/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHPROVIDERSETTINGS_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHPROVIDERSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_COM_CSMTPCLIENTSETTINGS_H
#include "gucefCOM_CSmtpClientSettings.h"
#define GUCEF_COM_CSMTPCLIENTSETTINGS_H
#endif /* GUCEF_COM_CSMTPCLIENTSETTINGS_H ? */

#ifndef GUCEF_COM_CIMAPCLIENTSETTINGS_H
#include "gucefCOM_CImapClientSettings.h"
#define GUCEF_COM_CIMAPCLIENTSETTINGS_H
#endif /* GUCEF_COM_CIMAPCLIENTSETTINGS_H ? */

#ifndef GUCEF_COM_CPOP3CLIENTSETTINGS_H
#include "gucefCOM_CPop3ClientSettings.h"
#define GUCEF_COM_CPOP3CLIENTSETTINGS_H
#endif /* GUCEF_COM_CPOP3CLIENTSETTINGS_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#include "matchprovidermail_macros.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H ? */

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

/** What is done with a mail of a match after it was processed completely; mail that is not of a match is never touched */
enum EMailProcessedAction
{
    MAILPROCESSED_LEAVE          = 0 ,  /**< it stays in the inbox, marked read */
    MAILPROCESSED_COPY_TO_FOLDER = 1 ,  /**< a copy is put in the processed folder, the original stays in the inbox (a folder that is not set means no copy) */
    MAILPROCESSED_MOVE_TO_FOLDER = 2    /**< it is moved to the processed folder, so it is no longer in the inbox */
};
typedef enum EMailProcessedAction TMailProcessedAction;

GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP const char* MailProcessedActionToString( TMailProcessedAction action );

/** @return false when the text is none of LEAVE, COPY_TO_FOLDER and MOVE_TO_FOLDER */
GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP bool TryStringToMailProcessedAction( const CORE::CString& text, TMailProcessedAction& action );

/**
 *  How the mail match provider reaches the mailbox of the player and how it behaves.
 *
 *  Config: the members by the same name as attributes or child values; the children "Smtp", "Imap" and "Pop3" hold the
 *  settings of the three clients (see CMailClientSettings). SaveConfig() never writes a password or a token: the
 *  credentials come from a CIMailCredentialProvider, or are put in the settings of the clients by the application at
 *  run time. Times are milliseconds.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailMatchProviderSettings : public CORE::CIConfigurable
{
    public:

    CORE::CString mailAddress;              /**< the mailbox of the player: where the mail comes from and arrives; part of the signed record of a match */
    CORE::CString displayName;              /**< the name in the From header */
    COM::CSmtpClientSettings smtp;
    COM::CImapClientSettings imap;
    COM::CPop3ClientSettings pop3;
    bool useImap;                           /**< receive over IMAP (true) or over POP3 */
    CORE::CString mailbox;                  /**< the IMAP folder that is watched */
    CORE::CString processedFolder;          /**< IMAP: the folder that processedAction copies or moves processed messages to */
    TMailProcessedAction processedAction;   /**< IMAP: what is done with a message of a match once it was processed. A move uses MOVE (RFC 6851) when the server has it, else a copy, a deleted flag and a UID EXPUNGE of that one message (UIDPLUS), else a copy only */
    bool pop3DeleteAfterProcessing;         /**< POP3: a message of a match that was processed is deleted at the end of the session; other messages never */
    bool markProcessedSeen;                 /**< IMAP: processed messages get the \Seen flag */
    bool useIdle;                           /**< IMAP: wait with IDLE when the server offers it, else poll */
    UInt32 pollIntervalMs;                  /**< polling when there is no IDLE, and POP3 */
    UInt32 outboxResendIntervalMs;          /**< a mail that was sent and not confirmed is sent again after this quiet period */
    UInt32 outboxMaxSendAttempts;           /**< sends of one mail in all; after this many it stays unconfirmed */
    UInt32 sendRetryInitialMs;              /**< the first wait after a temporary failure of the mail server; doubled each time */
    UInt32 sendRetryMaxMs;
    UInt32 reconnectInitialMs;              /**< the first wait before the mailbox connection is tried again; doubled each time */
    UInt32 reconnectMaxMs;
    UInt32 maxContainerBytes;               /**< a turn (container) above this is refused, sent or received; at most CMatchContainer::MaxContainerSize */
    UInt32 maxBodyTextBytes;                /**< the armored text of a turn in a body above this is refused; at most CMatchTextArmor::MaxEncodedTextSize */
    UInt32 maxMessageBytes;                 /**< a message above this is not fetched */
    UInt32 maxControlBytes;                 /**< a signed record, proposal or vote above this is refused */
    UInt32 seenSetSize;                     /**< the number of message ids (and POP3 unique ids) that are remembered */
    bool sendAcknowledgements;              /**< a tiny ACK mail for every turn that arrived, so that the sender can show it as delivered */
    UInt32 nudgeIntervalMs;                 /**< a participant is nudged at most once in this time */
    UInt32 resendRequestIntervalMs;         /**< a missing revision is asked for again after this time */
    UInt32 maxPendingPackages;              /**< packages that arrived ahead of their predecessor and wait for it, per match */
    UInt32 tickMs;                          /**< the interval of the provider's own timer */
    UInt32 introExpiryMs;                   /**< how long a request to introduce oneself is good for */
    UInt32 introResendIntervalMs;           /**< a request or a reply of an introduction is sent again at most this often */
    CORE::CString decisionPolicySettingKey; /**< the game setting that says how the table decides ("BOSS" or anything else for a vote): the draft stage needs it before there is a record. Empty: a vote */
    UInt32 draftDecisionMs;                 /**< how long a proposal of the draft stage (a force start, a kick) is open for votes */
    UInt32 draftApplyGraceMs;               /**< how long the host is given to carry out a decision that passed, before the others show that the host has not done it */
    bool applyDraftDecisions;               /**< the host carries out the decisions of the draft stage that passed by itself; off: only when ApplyDraftDecisions() is called (a host that decides by hand) */
    bool autoJoinIntroduced;                /**< an invitation to a game the player agreed to be introduced to is accepted at once, when its table is the one announced */
    CORE::CString seatsSettingKey;          /**< the game setting that holds the number of seats, empty for none: a table with fewer seats than announced is still the table that was agreed to */
    CORE::CString stateDirectory;           /**< where the provider keeps its files: matches, outbox, mailbox state */
    CORE::CString customScheme;             /**< the scheme of the application link in the mail text, empty for none */
    CORE::CString httpsHost;                /**< the game domain of the https link in the mail text, empty for none */

    CMailMatchProviderSettings( void );

    CMailMatchProviderSettings( const CMailMatchProviderSettings& src );

    virtual ~CMailMatchProviderSettings() GUCEF_VIRTUAL_OVERRIDE;

    CMailMatchProviderSettings& operator=( const CMailMatchProviderSettings& src );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the values are usable together; the reason says what is not */
    bool IsValid( CORE::CString& reason ) const;
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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHPROVIDERSETTINGS_H ? */
