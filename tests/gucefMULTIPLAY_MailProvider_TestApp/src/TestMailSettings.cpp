/*
 *  gucefMULTIPLAY_MailProvider_TestApp: Application for testing the mail match provider of gucefMULTIPLAY
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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#include "matchprovidermail_CEMailMatchProvider.h"
#include "TestMailPump.h"
#include "TestMailSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::MULTIPLAYPLUGIN;

#define ERRORHERE       MAIL_ERRORHERE
#define ASSERT_TRUE(t)  MAIL_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) MAIL_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The data tree as the JSON text it is stored as: what a config file would hold */
bool
NodeToText( const CORE::CDataNode& node, CORE::CString& text )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) || codec.IsNULL() )
        return false;
    CORE::CDynamicBuffer buffer;
    CORE::CDynamicBufferAccess access( &buffer, false );
    if ( !codec->StoreDataTree( &node, &access ) )
        return false;
    text = buffer.GetDataSize() > 0 ? CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() ) : CORE::CString();
    return true;
}

/*-------------------------------------------------------------------------*/

MAIL::CEMailMatchProviderSettings
MakeFullSettings( void )
{GUCEF_TRACE;

    MAIL::CEMailMatchProviderSettings s;
    s.mailAddress = "alice@mail.test";
    s.displayName = "Alice";
    s.smtp.host = "smtp.mail.test";
    s.smtp.port = 2525;
    s.smtp.tlsMode = COM::MAILTLS_IMPLICIT;
    s.smtp.userName = "alice";
    s.smtp.password = "SMTP-SECRET-0001";
    s.imap.host = "imap.mail.test";
    s.imap.tlsMode = COM::MAILTLS_STARTTLS;
    s.imap.userName = "alice";
    s.imap.password = "IMAP-SECRET-0002";
    s.imap.idleRenewInMs = 1234567;
    s.pop3.host = "pop.mail.test";
    s.pop3.userName = "alice";
    s.pop3.password = "POP3-SECRET-0003";
    s.useImap = false;
    s.mailbox = "Games";
    s.processedFolder = "Games/Done";
    s.processedAction = MAIL::MAILPROCESSED_MOVE_TO_FOLDER;
    s.pop3DeleteAfterProcessing = true;
    s.markProcessedSeen = false;
    s.useIdle = false;
    s.pollIntervalMs = 31000;
    s.outboxResendIntervalMs = 7200000;
    s.outboxMaxSendAttempts = 7;
    s.sendRetryInitialMs = 2000;
    s.sendRetryMaxMs = 600000;
    s.reconnectInitialMs = 1500;
    s.reconnectMaxMs = 90000;
    s.maxContainerBytes = 1500000;
    s.maxBodyTextBytes = 300000;
    s.maxMessageBytes = 2000000;
    s.maxControlBytes = 100000;
    s.seenSetSize = 777;
    s.sendAcknowledgements = false;
    s.nudgeIntervalMs = 3600000;
    s.resendRequestIntervalMs = 120000;
    s.maxPendingPackages = 5;
    s.tickMs = 250;
    s.introExpiryMs = 3 * 24 * 60 * 60 * 1000;
    s.introResendIntervalMs = 600000;
    s.autoJoinIntroduced = false;
    s.seatsSettingKey = "seats";
    s.decisionPolicySettingKey = "policy";
    s.draftDecisionMs = 5000;
    s.draftApplyGraceMs = 7000;
    s.applyDraftDecisions = false;
    s.stateDirectory = "/var/games/state";
    s.customScheme = "redux";
    s.httpsHost = "games.example.invalid";
    return s;
}

} /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailSettingsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "matchprovidermail: settings" );

    GUCEF_TESTFW_TESTCASE( "Settings: the defaults are the ones the design names and are not usable before an account is given" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Settings: the defaults are the ones the design names and are not usable before an account is given" ), "not the case that was asked for" );
    try
    {
        MAIL::CEMailMatchProviderSettings s;
        ASSERT_TRUE( s.useImap && s.useIdle && s.markProcessedSeen && s.sendAcknowledgements );
        ASSERT_TRUE( CORE::CString( "INBOX" ) == s.mailbox && s.processedFolder.IsNULLOrEmpty() );
        // introductions: two weeks, an hour between requests, join at once
        ASSERT_TRUE( 14ULL * 24 * 60 * 60 * 1000 == s.introExpiryMs && 3600000 == s.introResendIntervalMs && s.autoJoinIntroduced && s.seatsSettingKey.IsNULLOrEmpty() );
        // the decisions of the draft stage: three days to vote, a day for the host to carry out, carried out by itself, a vote unless the game says otherwise
        ASSERT_TRUE( 3ULL * 24 * 60 * 60 * 1000 == s.draftDecisionMs && 24ULL * 60 * 60 * 1000 == s.draftApplyGraceMs && s.applyDraftDecisions && s.decisionPolicySettingKey.IsNULLOrEmpty() );
        // what is done with processed mail: a copy (as always), and POP3 mail is not deleted
        ASSERT_TRUE( MAIL::MAILPROCESSED_COPY_TO_FOLDER == s.processedAction && !s.pop3DeleteAfterProcessing );
        ASSERT_TRUE( MULTIPLAY::CMatchContainer::MaxContainerSize == s.maxContainerBytes && MULTIPLAY::CMatchTextArmor::MaxEncodedTextSize == s.maxBodyTextBytes );
        ASSERT_TRUE( 86400000 == s.nudgeIntervalMs && s.seenSetSize >= 1000 && s.outboxResendIntervalMs >= 3600000 );
        CORE::CString reason;
        ASSERT_FALSE( s.IsValid( reason ) );
        ASSERT_TRUE( MakeFullSettings().IsValid( reason ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: what is saved is read back, the servers, the security, the folders, the intervals and the caps" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Settings: what is saved is read back, the servers, the security, the folders, the intervals and the caps" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "There is no json codec" );
        const MAIL::CEMailMatchProviderSettings s = MakeFullSettings();
        CORE::CDataNode node( "Config" );
        ASSERT_TRUE( s.SaveConfig( node ) );
        MAIL::CEMailMatchProviderSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( node ) );
        ASSERT_TRUE( loaded.mailAddress == s.mailAddress && loaded.displayName == s.displayName && loaded.useImap == s.useImap && loaded.mailbox == s.mailbox && loaded.processedFolder == s.processedFolder );
        ASSERT_TRUE( MAIL::MAILPROCESSED_MOVE_TO_FOLDER == loaded.processedAction && loaded.pop3DeleteAfterProcessing );
        ASSERT_TRUE( loaded.markProcessedSeen == s.markProcessedSeen && loaded.useIdle == s.useIdle && loaded.pollIntervalMs == s.pollIntervalMs );
        ASSERT_TRUE( loaded.outboxResendIntervalMs == s.outboxResendIntervalMs && loaded.outboxMaxSendAttempts == s.outboxMaxSendAttempts );
        ASSERT_TRUE( loaded.sendRetryInitialMs == s.sendRetryInitialMs && loaded.sendRetryMaxMs == s.sendRetryMaxMs && loaded.reconnectInitialMs == s.reconnectInitialMs && loaded.reconnectMaxMs == s.reconnectMaxMs );
        ASSERT_TRUE( loaded.maxContainerBytes == s.maxContainerBytes && loaded.maxBodyTextBytes == s.maxBodyTextBytes && loaded.maxMessageBytes == s.maxMessageBytes && loaded.maxControlBytes == s.maxControlBytes );
        ASSERT_TRUE( loaded.seenSetSize == s.seenSetSize && loaded.sendAcknowledgements == s.sendAcknowledgements && loaded.nudgeIntervalMs == s.nudgeIntervalMs );
        ASSERT_TRUE( loaded.resendRequestIntervalMs == s.resendRequestIntervalMs && loaded.maxPendingPackages == s.maxPendingPackages && loaded.tickMs == s.tickMs );
        ASSERT_TRUE( loaded.introExpiryMs == s.introExpiryMs && loaded.introResendIntervalMs == s.introResendIntervalMs && !loaded.autoJoinIntroduced && "seats" == loaded.seatsSettingKey );
        ASSERT_TRUE( 5000 == loaded.draftDecisionMs && 7000 == loaded.draftApplyGraceMs && !loaded.applyDraftDecisions && "policy" == loaded.decisionPolicySettingKey );
        ASSERT_TRUE( loaded.stateDirectory == s.stateDirectory && loaded.customScheme == s.customScheme && loaded.httpsHost == s.httpsHost );
        ASSERT_TRUE( loaded.smtp.host == s.smtp.host && loaded.smtp.port == s.smtp.port && COM::MAILTLS_IMPLICIT == loaded.smtp.tlsMode && loaded.smtp.userName == s.smtp.userName );
        ASSERT_TRUE( loaded.imap.host == s.imap.host && COM::MAILTLS_STARTTLS == loaded.imap.tlsMode && loaded.imap.idleRenewInMs == s.imap.idleRenewInMs );
        ASSERT_TRUE( loaded.pop3.host == s.pop3.host && loaded.pop3.userName == s.pop3.userName );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: a password or a token is never in what is saved, whatever the settings of the clients say" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Settings: a password or a token is never in what is saved, whatever the settings of the clients say" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "There is no json codec" );
        MAIL::CEMailMatchProviderSettings s = MakeFullSettings();
        s.smtp.saveSecrets = s.imap.saveSecrets = s.pop3.saveSecrets = true;
        s.imap.passwordIsToken = true;
        CORE::CDataNode node( "Config" );
        ASSERT_TRUE( s.SaveConfig( node ) );
        CORE::CString text;
        ASSERT_TRUE( NodeToText( node, text ) && text.Length() > 100 );
        ASSERT_TRUE( text.HasSubstr( "SMTP-SECRET-0001" ) < 0 && text.HasSubstr( "IMAP-SECRET-0002" ) < 0 && text.HasSubstr( "POP3-SECRET-0003" ) < 0 );
        ASSERT_TRUE( text.HasSubstr( "smtp.mail.test" ) >= 0 );
        // the settings that were saved from are not changed by saving
        ASSERT_TRUE( s.smtp.password == "SMTP-SECRET-0001" && s.smtp.saveSecrets );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: values that cannot work are refused and the settings stay as they were" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Settings: values that cannot work are refused and the settings stay as they were" ), "not the case that was asked for" );
    try
    {
        const MAIL::CEMailMatchProviderSettings original = MakeFullSettings();
        const char* badNames[]  = { "tickMs", "pollIntervalMs", "maxContainerBytes", "maxBodyTextBytes", "maxMessageBytes", "seenSetSize", "mailAddress", "sendRetryInitialMs", "outboxResendIntervalMs", "introExpiryMs", "introResendIntervalMs" };
        const char* badValues[] = { "0",      "0",              "99999999",          "99999999",         "0",               "0",           "not an address\r\nBcc: x@y.z", "0", "0", "0", "0" };
        for ( int i=0; i<11; ++i )
        {
            CORE::CDataNode node( "Config" );
            ASSERT_TRUE( original.SaveConfig( node ) );
            node.SetAttribute( badNames[ i ], badValues[ i ] );
            MAIL::CEMailMatchProviderSettings target = original;
            ASSERT_FALSE( target.LoadConfig( node ) );
            ASSERT_TRUE( target.tickMs == original.tickMs && target.mailAddress == original.mailAddress && target.maxContainerBytes == original.maxContainerBytes && target.seenSetSize == original.seenSetSize );
        }
        // the retry maximum cannot be below the first wait
        CORE::CDataNode node( "Config" );
        ASSERT_TRUE( original.SaveConfig( node ) );
        node.SetAttribute( "sendRetryInitialMs", "5000" );
        node.SetAttribute( "sendRetryMaxMs", "1000" );
        MAIL::CEMailMatchProviderSettings target;
        ASSERT_FALSE( target.LoadConfig( node ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: the action for processed mail has three values, an unknown one is refused, and a move needs a folder" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Settings: the action for processed mail has three values, an unknown one is refused, and a move needs a folder" ), "not the case that was asked for" );
    try
    {
        const char* names[] = { "LEAVE", "COPY_TO_FOLDER", "MOVE_TO_FOLDER" };
        const MAIL::TMailProcessedAction actions[] = { MAIL::MAILPROCESSED_LEAVE, MAIL::MAILPROCESSED_COPY_TO_FOLDER, MAIL::MAILPROCESSED_MOVE_TO_FOLDER };
        for ( int i=0; i<3; ++i )
        {
            ASSERT_TRUE( CORE::CString( names[ i ] ) == MAIL::MailProcessedActionToString( actions[ i ] ) );
            MAIL::TMailProcessedAction parsed = MAIL::MAILPROCESSED_LEAVE;
            ASSERT_TRUE( MAIL::TryStringToMailProcessedAction( names[ i ], parsed ) && actions[ i ] == parsed );
            MAIL::CEMailMatchProviderSettings s = MakeFullSettings();
            s.processedAction = actions[ i ];
            CORE::CDataNode node( "Config" );
            ASSERT_TRUE( s.SaveConfig( node ) );
            MAIL::CEMailMatchProviderSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( node ) && actions[ i ] == loaded.processedAction );
        }
        MAIL::TMailProcessedAction ignored = MAIL::MAILPROCESSED_LEAVE;
        ASSERT_FALSE( MAIL::TryStringToMailProcessedAction( "DELETE", ignored ) || MAIL::TryStringToMailProcessedAction( "", ignored ) || MAIL::TryStringToMailProcessedAction( "move_to_folder", ignored ) );

        // an unknown value in the config: refused, the settings stay as they were
        MAIL::CEMailMatchProviderSettings original = MakeFullSettings();
        CORE::CDataNode node( "Config" );
        ASSERT_TRUE( original.SaveConfig( node ) );
        node.SetAttribute( "processedAction", "DELETE_EVERYTHING" );
        MAIL::CEMailMatchProviderSettings target = original;
        target.processedAction = MAIL::MAILPROCESSED_LEAVE;
        ASSERT_FALSE( target.LoadConfig( node ) );
        ASSERT_TRUE( MAIL::MAILPROCESSED_LEAVE == target.processedAction );

        // a config without the value keeps the default, so that a config from before still loads
        CORE::CDataNode oldNode( "Config" );
        ASSERT_TRUE( original.SaveConfig( oldNode ) );
        oldNode.DelAttribute( "processedAction" );
        oldNode.DelAttribute( "pop3DeleteAfterProcessing" );
        MAIL::CEMailMatchProviderSettings fresh;
        ASSERT_TRUE( fresh.LoadConfig( oldNode ) && MAIL::MAILPROCESSED_COPY_TO_FOLDER == fresh.processedAction && !fresh.pop3DeleteAfterProcessing );

        // a move needs a folder; a copy without one is as it always was (no copy), leaving needs none
        CORE::CString reason;
        MAIL::CEMailMatchProviderSettings s = MakeFullSettings();
        s.useImap = true;
        s.processedAction = MAIL::MAILPROCESSED_MOVE_TO_FOLDER;
        s.processedFolder.Clear();
        ASSERT_FALSE( s.IsValid( reason ) || reason.IsNULLOrEmpty() );
        s.processedAction = MAIL::MAILPROCESSED_COPY_TO_FOLDER;
        ASSERT_TRUE( s.IsValid( reason ) );
        s.processedAction = MAIL::MAILPROCESSED_LEAVE;
        ASSERT_TRUE( s.IsValid( reason ) );
        // the same name for the watched folder and the target would move mail onto itself
        s.processedAction = MAIL::MAILPROCESSED_MOVE_TO_FOLDER;
        s.processedFolder = s.mailbox;
        ASSERT_FALSE( s.IsValid( reason ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: an account is complete when it has an address, a server for sending and one for receiving and a place for the files" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Settings: an account is complete when it has an address, a server for sending and one for receiving and a place for the files" ), "not the case that was asked for" );
    try
    {
        CORE::CString reason;
        ASSERT_TRUE( MakeFullSettings().IsValid( reason ) );
        MAIL::CEMailMatchProviderSettings s = MakeFullSettings();
        s.mailAddress.Clear();
        ASSERT_FALSE( s.IsValid( reason ) || reason.IsNULLOrEmpty() );
        s = MakeFullSettings(); s.smtp.host.Clear();
        ASSERT_FALSE( s.IsValid( reason ) );
        s = MakeFullSettings(); s.useImap = true; s.imap.host.Clear();
        ASSERT_FALSE( s.IsValid( reason ) );
        s = MakeFullSettings(); s.useImap = false; s.pop3.host.Clear();
        ASSERT_FALSE( s.IsValid( reason ) );
        s = MakeFullSettings(); s.useImap = true; s.mailbox.Clear();
        ASSERT_FALSE( s.IsValid( reason ) );
        s = MakeFullSettings(); s.stateDirectory.Clear();
        ASSERT_FALSE( s.IsValid( reason ) );
        s = MakeFullSettings(); s.maxContainerBytes = MULTIPLAY::CMatchContainer::MaxContainerSize + 1;
        ASSERT_FALSE( s.IsValid( reason ) );
        // the reason never repeats the address
        s = MakeFullSettings(); s.smtp.host.Clear();
        s.IsValid( reason );
        ASSERT_TRUE( reason.HasSubstr( "alice@mail.test" ) < 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: the provider is configured through its config and the config it writes is its settings" )
    GUCEF_TESTFW_SKIP_IF( !MailTestWanted( "Settings: the provider is configured through its config and the config it writes is its settings" ), "not the case that was asked for" );
    try
    {
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "There is no json codec" );
        MAIL::CEMailMatchProvider provider;
        CORE::CDataNode node( "Config" );
        ASSERT_TRUE( MakeFullSettings().SaveConfig( node ) );
        ASSERT_TRUE( provider.LoadConfig( node ) );
        ASSERT_TRUE( provider.GetSettings().mailAddress == "alice@mail.test" && 777 == provider.GetSettings().seenSetSize );
        CORE::CDataNode written( "Config" );
        ASSERT_TRUE( provider.SaveConfig( written ) );
        MAIL::CEMailMatchProviderSettings again;
        ASSERT_TRUE( again.LoadConfig( written ) && again.mailAddress == "alice@mail.test" && again.httpsHost == "games.example.invalid" );

        // through the factory the plugin registered
        MULTIPLAY::CIMatchProviderPtr made = MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().Create( "mail" );
        ASSERT_FALSE( made.IsNULL() );
        ASSERT_TRUE( made->LoadConfig( node ) );
        ASSERT_TRUE( CORE::CString( "mail" ) == made->GetProviderName() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
