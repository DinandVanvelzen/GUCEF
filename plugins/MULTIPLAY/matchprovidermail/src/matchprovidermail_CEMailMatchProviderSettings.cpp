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


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MULTIPLAY_H
#include "gucefMULTIPLAY.h"
#define GUCEF_MULTIPLAY_H
#endif /* GUCEF_MULTIPLAY_H ? */

#include "gucefCOM_CEMailAddress.h"
#include "matchprovidermail_CEMailMatchProviderSettings.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


const char*
MailProcessedActionToString( TMailProcessedAction action )
{GUCEF_TRACE;

    switch ( action )
    {
        case MAILPROCESSED_LEAVE:           return "LEAVE";
        case MAILPROCESSED_COPY_TO_FOLDER:  return "COPY_TO_FOLDER";
        case MAILPROCESSED_MOVE_TO_FOLDER:  return "MOVE_TO_FOLDER";
        default:                            return "COPY_TO_FOLDER";
    }
}

/*-------------------------------------------------------------------------*/

bool
TryStringToMailProcessedAction( const CORE::CString& text, TMailProcessedAction& action )
{GUCEF_TRACE;

    if ( text == "LEAVE" )
        action = MAILPROCESSED_LEAVE;
    else
    if ( text == "COPY_TO_FOLDER" )
        action = MAILPROCESSED_COPY_TO_FOLDER;
    else
    if ( text == "MOVE_TO_FOLDER" )
        action = MAILPROCESSED_MOVE_TO_FOLDER;
    else
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

CEMailMatchProviderSettings::CEMailMatchProviderSettings( void )
    : CORE::CIConfigurable()
    , mailAddress()
    , displayName()
    , smtp()
    , imap()
    , pop3()
    , useImap( true )
    , mailbox( "INBOX" )
    , processedFolder()
    , processedAction( MAILPROCESSED_COPY_TO_FOLDER )
    , pop3DeleteAfterProcessing( false )
    , markProcessedSeen( true )
    , useIdle( true )
    , pollIntervalMs( 60000 )
    , outboxResendIntervalMs( 6 * 60 * 60 * 1000 )
    , outboxMaxSendAttempts( 20 )
    , sendRetryInitialMs( 10000 )
    , sendRetryMaxMs( 15 * 60 * 1000 )
    , reconnectInitialMs( 5000 )
    , reconnectMaxMs( 5 * 60 * 1000 )
    , maxContainerBytes( MULTIPLAY::CMatchContainer::MaxContainerSize )
    , maxBodyTextBytes( MULTIPLAY::CMatchTextArmor::MaxEncodedTextSize )
    , maxMessageBytes( 3 * 1024 * 1024 )
    , maxControlBytes( 256 * 1024 )
    , seenSetSize( 5000 )
    , sendAcknowledgements( true )
    , nudgeIntervalMs( 24 * 60 * 60 * 1000 )
    , resendRequestIntervalMs( 10 * 60 * 1000 )
    , maxPendingPackages( 8 )
    , tickMs( 1000 )
    , introExpiryMs( 14 * 24 * 60 * 60 * 1000 )
    , introResendIntervalMs( 60 * 60 * 1000 )
    , decisionPolicySettingKey()
    , draftDecisionMs( 3 * 24 * 60 * 60 * 1000 )
    , draftApplyGraceMs( 24 * 60 * 60 * 1000 )
    , applyDraftDecisions( true )
    , autoJoinIntroduced( true )
    , seatsSettingKey()
    , stateDirectory()
    , customScheme()
    , httpsHost()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailMatchProviderSettings::CEMailMatchProviderSettings( const CEMailMatchProviderSettings& src )
    : CORE::CIConfigurable( src )
    , mailAddress( src.mailAddress )
    , displayName( src.displayName )
    , smtp( src.smtp )
    , imap( src.imap )
    , pop3( src.pop3 )
    , useImap( src.useImap )
    , mailbox( src.mailbox )
    , processedFolder( src.processedFolder )
    , processedAction( src.processedAction )
    , pop3DeleteAfterProcessing( src.pop3DeleteAfterProcessing )
    , markProcessedSeen( src.markProcessedSeen )
    , useIdle( src.useIdle )
    , pollIntervalMs( src.pollIntervalMs )
    , outboxResendIntervalMs( src.outboxResendIntervalMs )
    , outboxMaxSendAttempts( src.outboxMaxSendAttempts )
    , sendRetryInitialMs( src.sendRetryInitialMs )
    , sendRetryMaxMs( src.sendRetryMaxMs )
    , reconnectInitialMs( src.reconnectInitialMs )
    , reconnectMaxMs( src.reconnectMaxMs )
    , maxContainerBytes( src.maxContainerBytes )
    , maxBodyTextBytes( src.maxBodyTextBytes )
    , maxMessageBytes( src.maxMessageBytes )
    , maxControlBytes( src.maxControlBytes )
    , seenSetSize( src.seenSetSize )
    , sendAcknowledgements( src.sendAcknowledgements )
    , nudgeIntervalMs( src.nudgeIntervalMs )
    , resendRequestIntervalMs( src.resendRequestIntervalMs )
    , maxPendingPackages( src.maxPendingPackages )
    , tickMs( src.tickMs )
    , introExpiryMs( src.introExpiryMs )
    , introResendIntervalMs( src.introResendIntervalMs )
    , decisionPolicySettingKey( src.decisionPolicySettingKey )
    , draftDecisionMs( src.draftDecisionMs )
    , draftApplyGraceMs( src.draftApplyGraceMs )
    , applyDraftDecisions( src.applyDraftDecisions )
    , autoJoinIntroduced( src.autoJoinIntroduced )
    , seatsSettingKey( src.seatsSettingKey )
    , stateDirectory( src.stateDirectory )
    , customScheme( src.customScheme )
    , httpsHost( src.httpsHost )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailMatchProviderSettings::~CEMailMatchProviderSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailMatchProviderSettings&
CEMailMatchProviderSettings::operator=( const CEMailMatchProviderSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        mailAddress = src.mailAddress;
        displayName = src.displayName;
        smtp = src.smtp;
        imap = src.imap;
        pop3 = src.pop3;
        useImap = src.useImap;
        mailbox = src.mailbox;
        processedFolder = src.processedFolder;
        processedAction = src.processedAction;
        pop3DeleteAfterProcessing = src.pop3DeleteAfterProcessing;
        markProcessedSeen = src.markProcessedSeen;
        useIdle = src.useIdle;
        pollIntervalMs = src.pollIntervalMs;
        outboxResendIntervalMs = src.outboxResendIntervalMs;
        outboxMaxSendAttempts = src.outboxMaxSendAttempts;
        sendRetryInitialMs = src.sendRetryInitialMs;
        sendRetryMaxMs = src.sendRetryMaxMs;
        reconnectInitialMs = src.reconnectInitialMs;
        reconnectMaxMs = src.reconnectMaxMs;
        maxContainerBytes = src.maxContainerBytes;
        maxBodyTextBytes = src.maxBodyTextBytes;
        maxMessageBytes = src.maxMessageBytes;
        maxControlBytes = src.maxControlBytes;
        seenSetSize = src.seenSetSize;
        sendAcknowledgements = src.sendAcknowledgements;
        nudgeIntervalMs = src.nudgeIntervalMs;
        resendRequestIntervalMs = src.resendRequestIntervalMs;
        maxPendingPackages = src.maxPendingPackages;
        tickMs = src.tickMs;
        introExpiryMs = src.introExpiryMs;
        introResendIntervalMs = src.introResendIntervalMs;
        decisionPolicySettingKey = src.decisionPolicySettingKey;
        draftDecisionMs = src.draftDecisionMs;
        draftApplyGraceMs = src.draftApplyGraceMs;
        applyDraftDecisions = src.applyDraftDecisions;
        autoJoinIntroduced = src.autoJoinIntroduced;
        seatsSettingKey = src.seatsSettingKey;
        stateDirectory = src.stateDirectory;
        customScheme = src.customScheme;
        httpsHost = src.httpsHost;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProviderSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "mailAddress", mailAddress );
    config.SetAttribute( "displayName", displayName );
    config.SetAttribute( "useImap", useImap );
    config.SetAttribute( "mailbox", mailbox );
    config.SetAttribute( "processedFolder", processedFolder );
    config.SetAttribute( "processedAction", MailProcessedActionToString( processedAction ) );
    config.SetAttribute( "pop3DeleteAfterProcessing", pop3DeleteAfterProcessing );
    config.SetAttribute( "markProcessedSeen", markProcessedSeen );
    config.SetAttribute( "useIdle", useIdle );
    config.SetAttribute( "pollIntervalMs", pollIntervalMs );
    config.SetAttribute( "outboxResendIntervalMs", outboxResendIntervalMs );
    config.SetAttribute( "outboxMaxSendAttempts", outboxMaxSendAttempts );
    config.SetAttribute( "sendRetryInitialMs", sendRetryInitialMs );
    config.SetAttribute( "sendRetryMaxMs", sendRetryMaxMs );
    config.SetAttribute( "reconnectInitialMs", reconnectInitialMs );
    config.SetAttribute( "reconnectMaxMs", reconnectMaxMs );
    config.SetAttribute( "maxContainerBytes", maxContainerBytes );
    config.SetAttribute( "maxBodyTextBytes", maxBodyTextBytes );
    config.SetAttribute( "maxMessageBytes", maxMessageBytes );
    config.SetAttribute( "maxControlBytes", maxControlBytes );
    config.SetAttribute( "seenSetSize", seenSetSize );
    config.SetAttribute( "sendAcknowledgements", sendAcknowledgements );
    config.SetAttribute( "nudgeIntervalMs", nudgeIntervalMs );
    config.SetAttribute( "resendRequestIntervalMs", resendRequestIntervalMs );
    config.SetAttribute( "maxPendingPackages", maxPendingPackages );
    config.SetAttribute( "tickMs", tickMs );
    config.SetAttribute( "introExpiryMs", introExpiryMs );
    config.SetAttribute( "introResendIntervalMs", introResendIntervalMs );
    config.SetAttribute( "decisionPolicySettingKey", decisionPolicySettingKey );
    config.SetAttribute( "draftDecisionMs", draftDecisionMs );
    config.SetAttribute( "draftApplyGraceMs", draftApplyGraceMs );
    config.SetAttribute( "applyDraftDecisions", applyDraftDecisions );
    config.SetAttribute( "autoJoinIntroduced", autoJoinIntroduced );
    config.SetAttribute( "seatsSettingKey", seatsSettingKey );
    config.SetAttribute( "stateDirectory", stateDirectory );
    config.SetAttribute( "customScheme", customScheme );
    config.SetAttribute( "httpsHost", httpsHost );

    // the passwords and tokens are never written, whatever the settings of the clients say
    COM::CSmtpClientSettings smtpCopy( smtp );
    COM::CImapClientSettings imapCopy( imap );
    COM::CPop3ClientSettings pop3Copy( pop3 );
    smtpCopy.saveSecrets = imapCopy.saveSecrets = pop3Copy.saveSecrets = false;
    smtpCopy.password.Clear();
    imapCopy.password.Clear();
    pop3Copy.password.Clear();
    return smtpCopy.SaveConfig( *config.AddChild( "Smtp", GUCEF_DATATYPE_OBJECT ) ) &&
           imapCopy.SaveConfig( *config.AddChild( "Imap", GUCEF_DATATYPE_OBJECT ) ) &&
           pop3Copy.SaveConfig( *config.AddChild( "Pop3", GUCEF_DATATYPE_OBJECT ) );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProviderSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CEMailMatchProviderSettings loaded( *this );
    loaded.mailAddress = config.GetAttributeValueOrChildValueByName( "mailAddress" ).AsString( mailAddress, true );
    loaded.displayName = config.GetAttributeValueOrChildValueByName( "displayName" ).AsString( displayName, true );
    loaded.useImap = config.GetAttributeValueOrChildValueByName( "useImap" ).AsBool( useImap, true );
    loaded.mailbox = config.GetAttributeValueOrChildValueByName( "mailbox" ).AsString( mailbox, true );
    loaded.processedFolder = config.GetAttributeValueOrChildValueByName( "processedFolder" ).AsString( processedFolder, true );
    loaded.pop3DeleteAfterProcessing = config.GetAttributeValueOrChildValueByName( "pop3DeleteAfterProcessing" ).AsBool( pop3DeleteAfterProcessing, true );
    const CORE::CString actionText = config.GetAttributeValueOrChildValueByName( "processedAction" ).AsString( CORE::CString::Empty, true );
    if ( !actionText.IsNULLOrEmpty() && !TryStringToMailProcessedAction( actionText, loaded.processedAction ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProviderSettings: The action for processed mail is not known, the settings are not used" );
        return false;
    }
    loaded.markProcessedSeen = config.GetAttributeValueOrChildValueByName( "markProcessedSeen" ).AsBool( markProcessedSeen, true );
    loaded.useIdle = config.GetAttributeValueOrChildValueByName( "useIdle" ).AsBool( useIdle, true );
    loaded.pollIntervalMs = config.GetAttributeValueOrChildValueByName( "pollIntervalMs" ).AsUInt32( pollIntervalMs, true );
    loaded.outboxResendIntervalMs = config.GetAttributeValueOrChildValueByName( "outboxResendIntervalMs" ).AsUInt32( outboxResendIntervalMs, true );
    loaded.outboxMaxSendAttempts = config.GetAttributeValueOrChildValueByName( "outboxMaxSendAttempts" ).AsUInt32( outboxMaxSendAttempts, true );
    loaded.sendRetryInitialMs = config.GetAttributeValueOrChildValueByName( "sendRetryInitialMs" ).AsUInt32( sendRetryInitialMs, true );
    loaded.sendRetryMaxMs = config.GetAttributeValueOrChildValueByName( "sendRetryMaxMs" ).AsUInt32( sendRetryMaxMs, true );
    loaded.reconnectInitialMs = config.GetAttributeValueOrChildValueByName( "reconnectInitialMs" ).AsUInt32( reconnectInitialMs, true );
    loaded.reconnectMaxMs = config.GetAttributeValueOrChildValueByName( "reconnectMaxMs" ).AsUInt32( reconnectMaxMs, true );
    loaded.maxContainerBytes = config.GetAttributeValueOrChildValueByName( "maxContainerBytes" ).AsUInt32( maxContainerBytes, true );
    loaded.maxBodyTextBytes = config.GetAttributeValueOrChildValueByName( "maxBodyTextBytes" ).AsUInt32( maxBodyTextBytes, true );
    loaded.maxMessageBytes = config.GetAttributeValueOrChildValueByName( "maxMessageBytes" ).AsUInt32( maxMessageBytes, true );
    loaded.maxControlBytes = config.GetAttributeValueOrChildValueByName( "maxControlBytes" ).AsUInt32( maxControlBytes, true );
    loaded.seenSetSize = config.GetAttributeValueOrChildValueByName( "seenSetSize" ).AsUInt32( seenSetSize, true );
    loaded.sendAcknowledgements = config.GetAttributeValueOrChildValueByName( "sendAcknowledgements" ).AsBool( sendAcknowledgements, true );
    loaded.nudgeIntervalMs = config.GetAttributeValueOrChildValueByName( "nudgeIntervalMs" ).AsUInt32( nudgeIntervalMs, true );
    loaded.resendRequestIntervalMs = config.GetAttributeValueOrChildValueByName( "resendRequestIntervalMs" ).AsUInt32( resendRequestIntervalMs, true );
    loaded.maxPendingPackages = config.GetAttributeValueOrChildValueByName( "maxPendingPackages" ).AsUInt32( maxPendingPackages, true );
    loaded.tickMs = config.GetAttributeValueOrChildValueByName( "tickMs" ).AsUInt32( tickMs, true );
    loaded.introExpiryMs = config.GetAttributeValueOrChildValueByName( "introExpiryMs" ).AsUInt32( introExpiryMs, true );
    loaded.introResendIntervalMs = config.GetAttributeValueOrChildValueByName( "introResendIntervalMs" ).AsUInt32( introResendIntervalMs, true );
    loaded.decisionPolicySettingKey = config.GetAttributeValueOrChildValueByName( "decisionPolicySettingKey" ).AsString( decisionPolicySettingKey, true );
    loaded.draftDecisionMs = config.GetAttributeValueOrChildValueByName( "draftDecisionMs" ).AsUInt32( draftDecisionMs, true );
    loaded.draftApplyGraceMs = config.GetAttributeValueOrChildValueByName( "draftApplyGraceMs" ).AsUInt32( draftApplyGraceMs, true );
    loaded.applyDraftDecisions = config.GetAttributeValueOrChildValueByName( "applyDraftDecisions" ).AsBool( applyDraftDecisions, true );
    loaded.autoJoinIntroduced = config.GetAttributeValueOrChildValueByName( "autoJoinIntroduced" ).AsBool( autoJoinIntroduced, true );
    loaded.seatsSettingKey = config.GetAttributeValueOrChildValueByName( "seatsSettingKey" ).AsString( seatsSettingKey, true );
    loaded.stateDirectory = config.GetAttributeValueOrChildValueByName( "stateDirectory" ).AsString( stateDirectory, true );
    loaded.customScheme = config.GetAttributeValueOrChildValueByName( "customScheme" ).AsString( customScheme, true );
    loaded.httpsHost = config.GetAttributeValueOrChildValueByName( "httpsHost" ).AsString( httpsHost, true );

    const CORE::CDataNode* smtpNode = config.FindChild( "Smtp" );
    const CORE::CDataNode* imapNode = config.FindChild( "Imap" );
    const CORE::CDataNode* pop3Node = config.FindChild( "Pop3" );
    if ( ( GUCEF_NULL != smtpNode && !loaded.smtp.LoadConfig( *smtpNode ) ) ||
         ( GUCEF_NULL != imapNode && !loaded.imap.LoadConfig( *imapNode ) ) ||
         ( GUCEF_NULL != pop3Node && !loaded.pop3.LoadConfig( *pop3Node ) ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProviderSettings: The settings of a mail server are not valid, the settings are not used" );
        return false;
    }

    // what cannot work is refused; whether the account is complete is IsValid()
    if ( 0 == loaded.introExpiryMs || 0 == loaded.introResendIntervalMs || 0 == loaded.draftDecisionMs || 0 == loaded.draftApplyGraceMs || 0 == loaded.tickMs || 0 == loaded.pollIntervalMs || 0 == loaded.outboxResendIntervalMs || 0 == loaded.sendRetryInitialMs || 0 == loaded.reconnectInitialMs ||
         loaded.sendRetryMaxMs < loaded.sendRetryInitialMs || loaded.reconnectMaxMs < loaded.reconnectInitialMs ||
         0 == loaded.maxContainerBytes || loaded.maxContainerBytes > MULTIPLAY::CMatchContainer::MaxContainerSize ||
         0 == loaded.maxBodyTextBytes || loaded.maxBodyTextBytes > MULTIPLAY::CMatchTextArmor::MaxEncodedTextSize ||
         0 == loaded.maxMessageBytes || 0 == loaded.maxControlBytes || 0 == loaded.seenSetSize )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProviderSettings: An interval or a size is not usable, the settings are not used" );
        return false;
    }
    if ( !loaded.mailAddress.IsNULLOrEmpty() && !COM::CEMailAddress( loaded.mailAddress ).IsValid() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProviderSettings: The mail address is not valid, the settings are not used" );
        return false;
    }
    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProviderSettings::IsValid( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( mailAddress.IsNULLOrEmpty() || !COM::CEMailAddress( mailAddress ).IsValid() )
        reason = "There is no valid mail address of the player";
    else
    if ( smtp.host.IsNULLOrEmpty() )
        reason = "There is no server for sending mail";
    else
    if ( useImap && imap.host.IsNULLOrEmpty() )
        reason = "There is no IMAP server for receiving mail";
    else
    if ( useImap && mailbox.IsNULLOrEmpty() )
        reason = "There is no folder to watch";
    else
    if ( !useImap && pop3.host.IsNULLOrEmpty() )
        reason = "There is no POP3 server for receiving mail";
    else
    if ( useImap && MAILPROCESSED_MOVE_TO_FOLDER == processedAction && processedFolder.IsNULLOrEmpty() )
        reason = "There is no folder to move processed mail to";
    else
    if ( useImap && MAILPROCESSED_MOVE_TO_FOLDER == processedAction && processedFolder.Equals( mailbox, false ) )
        reason = "The folder for processed mail is the folder that is watched";
    else
    if ( stateDirectory.IsNULLOrEmpty() )
        reason = "There is no directory for the files of the provider";
    else
    if ( 0 == introExpiryMs || 0 == introResendIntervalMs || 0 == draftDecisionMs || 0 == draftApplyGraceMs || 0 == tickMs || 0 == pollIntervalMs || 0 == outboxResendIntervalMs || 0 == sendRetryInitialMs || sendRetryMaxMs < sendRetryInitialMs || 0 == reconnectInitialMs || reconnectMaxMs < reconnectInitialMs )
        reason = "An interval is not usable";
    else
    if ( 0 == maxContainerBytes || maxContainerBytes > MULTIPLAY::CMatchContainer::MaxContainerSize || 0 == maxBodyTextBytes || maxBodyTextBytes > MULTIPLAY::CMatchTextArmor::MaxEncodedTextSize ||
         0 == maxMessageBytes || 0 == maxControlBytes || 0 == seenSetSize )
        reason = "A size limit is not usable";
    else
    {
        reason.Clear();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEMailMatchProviderSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchProviderSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
