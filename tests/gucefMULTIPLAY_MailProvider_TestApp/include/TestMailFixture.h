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


#ifndef GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILFIXTURE_H
#define GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILFIXTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <sstream>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H
#include "../../gucefMULTIPLAY_TestApp/include/TestMatchSupport.h"
#define GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H
#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H ? */

#ifndef GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILWORLD_H
#include "TestMailWorld.h"
#define GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILWORLD_H
#endif /* GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILWORLD_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_COM_CMIMECOMPOSER_H
#include "gucefCOM_CMimeComposer.h"
#define GUCEF_COM_CMIMECOMPOSER_H
#endif /* GUCEF_COM_CMIMECOMPOSER_H ? */

#include "matchprovidermail_CEMailMatchProvider.h"
#include "TestMailPump.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MAILTEST {

namespace MAIL = GUCEF::MULTIPLAYPLUGIN::MAIL;

/*-------------------------------------------------------------------------*/

/**
 *  The game's check, as a game would do it: a package is the payload of the package it follows with one move added. The
 *  payload is text, "game|setup|move|move...". A move that contains CHEAT is not a legal move.
 */
class CGameVerifier : public MULTIPLAY::CIMatchPackageVerifier
{
    public:

    UInt32 calls;
    UInt32 noBaseCount;                 /**< the next so many packages that have a base are answered with NO_BASE */
    bool rejectAll;
    bool rejectFirstOnly;               /**< only a package of revision 1 is rejected (the settings or the seed of the game are not the ones of this client) */
    CORE::CString refusalCode;
    CORE::CString lastReason;
    GUCEF::map< CORE::CString, MULTIPLAY::CIMatchPackageVerifier::TEvidenceVector > evidenceBySha;

    CGameVerifier( void ) : MULTIPLAY::CIMatchPackageVerifier(), calls( 0 ), noBaseCount( 0 ), rejectAll( false ), rejectFirstOnly( false ), refusalCode( "NOT_PLAYABLE" ), lastReason(), evidenceBySha() {}

    virtual CORE::CString GetRefusalCode( void ) const GUCEF_VIRTUAL_OVERRIDE { return refusalCode; }

    virtual MULTIPLAY::TMatchPackageVerdict Verify( const MULTIPLAY::CMatchInfo& info, const MULTIPLAY::CMatchPackage* trusted, const MULTIPLAY::CMatchPackage& next, CORE::CString& reason ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++calls;
        if ( noBaseCount > 0 && GUCEF_NULL != trusted )
        {
            --noBaseCount;
            reason = "The package it follows is not at hand";
            lastReason = reason;
            return MULTIPLAY::MATCHPACKAGE_NO_BASE;
        }
        const CORE::CString text = Text( next );
        if ( rejectAll || ( rejectFirstOnly && 1 == next.revision ) )
        {
            reason = "The game rejects everything";
            lastReason = reason;
            return MULTIPLAY::MATCHPACKAGE_REJECTED;
        }
        if ( !text.StartsWith( "game|setup" ) )
        {
            reason = "This is not a game";
            lastReason = reason;
            return MULTIPLAY::MATCHPACKAGE_REJECTED;
        }
        if ( text.HasSubstr( "CHEAT" ) >= 0 )
        {
            reason = "A move in the log is not legal";
            lastReason = reason;
            return MULTIPLAY::MATCHPACKAGE_REJECTED;
        }
        if ( GUCEF_NULL == trusted )
        {
            if ( next.revision > 1 )
            {
                reason = "The package it follows is not at hand";
                lastReason = reason;
                return MULTIPLAY::MATCHPACKAGE_NO_BASE;
            }
            return MULTIPLAY::MATCHPACKAGE_VERIFIED;
        }
        if ( next.baseRevision != trusted->revision )
        {
            reason = "The package it follows is not at hand";
            lastReason = reason;
            return MULTIPLAY::MATCHPACKAGE_NO_BASE;
        }
        const CORE::CString base = Text( *trusted );
        if ( !text.StartsWith( base + "|" ) )
        {
            reason = "The log does not replay on the package it follows";
            lastReason = reason;
            return MULTIPLAY::MATCHPACKAGE_REJECTED;
        }
        return MULTIPLAY::MATCHPACKAGE_VERIFIED;
    }

    virtual UInt32 GetDecisionEvidence( const MULTIPLAY::CMatchInfo& info, const MULTIPLAY::CMatchPackage& package, TEvidenceVector& evidence ) GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::map< CORE::CString, TEvidenceVector >::const_iterator i = evidenceBySha.find( package.sha256 );
        if ( i == evidenceBySha.end() )
            return 0;
        for ( size_t e=0; e<(*i).second.size(); ++e )
            evidence.push_back( (*i).second[ e ] );
        return (UInt32) (*i).second.size();
    }

    static CORE::CString Text( const MULTIPLAY::CMatchPackage& package )
    {
        return package.payload.GetDataSize() > 0 ? CORE::CString( static_cast< const char* >( package.payload.GetConstBufferPtr() ), package.payload.GetDataSize() ) : CORE::CString();
    }
};

/*-------------------------------------------------------------------------*/

/** Keeps what a provider reports */
class CProviderRecorder : public CORE::CObserver
{
    public:

    struct SUpdate
    {
        CORE::CString matchId;
        UInt32 revision;
        MULTIPLAY::TMatchStatus status;
        bool hasInfo;
        MULTIPLAY::CMatchInfo info;
        MULTIPLAY::CMatchPreview preview;
    };

    GUCEF::vector< SUpdate > added;
    GUCEF::vector< SUpdate > updated;
    CORE::CString::StringVector removed;
    GUCEF::vector< MULTIPLAY::CMatchOperationResult > submitResults;
    GUCEF::vector< bool > states;
    CORE::CString::StringVector errors;
    GUCEF::vector< MAIL::CEMailMatchNotice > refused;
    GUCEF::vector< MAIL::CEMailMatchNotice > equivocations;
    GUCEF::vector< MAIL::CEMailMatchNotice > nudges;
    GUCEF::vector< MAIL::CEMailMatchNotice > decisions;
    GUCEF::vector< MAIL::CEMailMatchNotice > problems;
    GUCEF::vector< MAIL::CEMailMatchNotice > intros;
    GUCEF::vector< MAIL::CEMailMatchNotice > lobby;
    GUCEF::vector< MAIL::CEMailMatchNotice > rosters;
    GUCEF::vector< MAIL::CEMailMatchNotice > draftMessages;
    GUCEF::vector< MAIL::CEMailMatchNotice > draftApplied;
    GUCEF::vector< MAIL::CEMailDeliveryEvent > deliveries;

    CProviderRecorder( void ) : CORE::CObserver() {}

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { static const CORE::CString n = "GUCEF::MAILTEST::CProviderRecorder"; return n; }

    /** Forgets what was reported, as when the application starts again */
    void Clear( void )
    {
        added.clear(); updated.clear(); removed.clear(); submitResults.clear(); states.clear(); errors.clear();
        refused.clear(); equivocations.clear(); nudges.clear(); decisions.clear(); problems.clear(); intros.clear(); lobby.clear(); rosters.clear(); draftMessages.clear(); draftApplied.clear(); deliveries.clear();
    }

    UInt32 CountUpdates( const CORE::CString& matchId, UInt32 revision ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<added.size(); ++i )
            if ( added[ i ].matchId == matchId && added[ i ].revision == revision )
                ++count;
        for ( size_t i=0; i<updated.size(); ++i )
            if ( updated[ i ].matchId == matchId && updated[ i ].revision == revision )
                ++count;
        return count;
    }

    UInt32 CountAdded( const CORE::CString& matchId ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<added.size(); ++i )
            if ( added[ i ].matchId == matchId )
                ++count;
        return count;
    }

    UInt32 CountRefused( const CORE::CString& code ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<refused.size(); ++i )
            if ( refused[ i ].code == code )
                ++count;
        return count;
    }

    bool HasDelivery( const CORE::CString& matchId, UInt32 revision, MAIL::TMailDeliveryState state ) const
    {
        for ( size_t i=0; i<deliveries.size(); ++i )
            if ( deliveries[ i ].matchId == matchId && deliveries[ i ].revision == revision && deliveries[ i ].state == state )
                return true;
        return false;
    }

    /** The newest announced state of the match: false when it was never announced */
    bool GetLast( const CORE::CString& matchId, SUpdate& update ) const
    {
        bool found = false;
        UInt32 best = 0;
        for ( int pass=0; pass<2; ++pass )
        {
            const GUCEF::vector< SUpdate >& list = 0 == pass ? added : updated;
            for ( size_t i=0; i<list.size(); ++i )
                if ( list[ i ].matchId == matchId && ( !found || list[ i ].revision >= best ) )
                {
                    update = list[ i ];
                    best = list[ i ].revision;
                    found = true;
                }
        }
        return found;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MULTIPLAY::CIMatchProvider::MatchAddedEvent == eventId || MULTIPLAY::CIMatchProvider::MatchUpdatedEvent == eventId )
        {
            const MULTIPLAY::CMatchUpdate* data = static_cast< const MULTIPLAY::CMatchUpdate* >( eventData );
            SUpdate update;
            update.matchId = data->preview.matchId;
            update.revision = data->preview.revision;
            update.status = data->hasInfo ? data->info.status : data->preview.status;
            update.hasInfo = data->hasInfo;
            update.info = data->info;
            update.preview = data->preview;
            ( MULTIPLAY::CIMatchProvider::MatchAddedEvent == eventId ? added : updated ).push_back( update );
        }
        else
        if ( MULTIPLAY::CIMatchProvider::MatchRemovedEvent == eventId )
            removed.push_back( static_cast< const CORE::TCloneableString* >( eventData )->GetData() );
        else
        if ( MULTIPLAY::CIMatchProvider::SubmitResultEvent == eventId )
            submitResults.push_back( *static_cast< const MULTIPLAY::CMatchOperationResult* >( eventData ) );
        else
        if ( MULTIPLAY::CIMatchProvider::ProviderStateEvent == eventId )
            states.push_back( static_cast< const MULTIPLAY::CMatchProviderState* >( eventData )->online );
        else
        if ( MULTIPLAY::CIMatchProvider::ProviderErrorEvent == eventId )
            errors.push_back( static_cast< const CORE::TCloneableString* >( eventData )->GetData() );
        else
        if ( MAIL::CEMailMatchProvider::PackageRefusedEvent == eventId )
            refused.push_back( *static_cast< const MAIL::CEMailMatchNotice* >( eventData ) );
        else
        if ( MAIL::CEMailMatchProvider::EquivocationDetectedEvent == eventId )
            equivocations.push_back( *static_cast< const MAIL::CEMailMatchNotice* >( eventData ) );
        else
        if ( MAIL::CEMailMatchProvider::NudgeReceivedEvent == eventId )
            nudges.push_back( *static_cast< const MAIL::CEMailMatchNotice* >( eventData ) );
        else
        if ( MAIL::CEMailMatchProvider::DecisionMessageEvent == eventId )
            decisions.push_back( *static_cast< const MAIL::CEMailMatchNotice* >( eventData ) );
        else
        if ( MAIL::CEMailMatchProvider::MailProblemEvent == eventId )
            problems.push_back( *static_cast< const MAIL::CEMailMatchNotice* >( eventData ) );
        else
        if ( MAIL::CEMailMatchProvider::IntroductionEvent == eventId )
            intros.push_back( *static_cast< const MAIL::CEMailMatchNotice* >( eventData ) );
        else
        if ( MAIL::CEMailMatchProvider::LobbyChangedEvent == eventId )
            lobby.push_back( *static_cast< const MAIL::CEMailMatchNotice* >( eventData ) );
        else
        if ( MAIL::CEMailMatchProvider::RosterChangedEvent == eventId )
            rosters.push_back( *static_cast< const MAIL::CEMailMatchNotice* >( eventData ) );
        else
        if ( MAIL::CEMailMatchProvider::VoteReceivedEvent == eventId )
            draftMessages.push_back( *static_cast< const MAIL::CEMailMatchNotice* >( eventData ) );
        else
        if ( MAIL::CEMailMatchProvider::DraftDecisionAppliedEvent == eventId )
            draftApplied.push_back( *static_cast< const MAIL::CEMailMatchNotice* >( eventData ) );
        else
        if ( MAIL::CEMailMatchProvider::DeliveryStateChangedEvent == eventId )
            deliveries.push_back( *static_cast< const MAIL::CEMailDeliveryEvent* >( eventData ) );
    }
};

/*-------------------------------------------------------------------------*/

struct SPlayer
{
    CORE::CString name;
    MULTIPLAY::CPlayerIdentity identity;
    TESTFW::CTestEMailBox* box;
    CORE::CString address;
    CORE::CString dir;
    MAIL::CEMailMatchProviderSettings settings;
    MULTIPLAY::CIMatchProviderPtr ptr;
    MAIL::CEMailMatchProvider* provider;
    CProviderRecorder recorder;
    CGameVerifier verifier;
    COM::CStaticEMailCredentialProvider credentials;

    SPlayer( void ) : name(), identity(), box( GUCEF_NULL ), address(), dir(), settings(), ptr(), provider( GUCEF_NULL ), recorder(), verifier(), credentials() {}
};

/*-------------------------------------------------------------------------*/

struct SOptions
{
    bool useImap;
    bool useIdle;
    bool acknowledgements;
    bool installVerifier;
    bool allowUnverified;
    bool credentialsFromProvider;       /**< the passwords are not in the settings: a credential provider has them */
    UInt32 maxContainerBytes;
    UInt32 pollIntervalMs;
    UInt32 resendIntervalMs;
    UInt32 seenSetSize;
    UInt32 autoClockMs;                 /**< the manual clock moves this many ms with every step of the pump, for polling that is on the clock */
    bool autoJoinIntroduced;            /**< the provider accepts an invitation of a game its player was introduced to */
    CORE::CString seatsSettingKey;      /**< the game setting that holds the number of seats: a table with fewer seats than announced is still the table that was agreed to */
    CORE::CString decisionPolicySettingKey; /**< the game setting that says how the table decides ("BOSS", anything else is a vote) */

    SOptions( void )
        : useImap( true ), useIdle( true ), acknowledgements( true ), installVerifier( true ), allowUnverified( false ), credentialsFromProvider( false )
        , maxContainerBytes( MULTIPLAY::CMatchContainer::MaxContainerSize ), pollIntervalMs( 500 ), resendIntervalMs( 60000 ), seenSetSize( 5000 ), autoClockMs( 0 ), autoJoinIntroduced( true ), seatsSettingKey(), decisionPolicySettingKey()
    {}
};

/*-------------------------------------------------------------------------*/

/** A table of players, each with an identity, a mailbox of their own and a provider; all on one pulse generator and one manual clock */
class CEMailTable
{
    public:

    TESTFW::CTestEMailWorld world;
    TESTFW::CManualMatchClock clock;
    GUCEF::vector< SPlayer* > players;
    CORE::CString root;
    SOptions options;
    UInt64 lastHookMs;

    CEMailTable( void ) : world(), clock(), players(), root(), options(), lastHookMs( 0 ) {}

    static void ClockHook( void* context )
    {
        CEMailTable* table = static_cast< CEMailTable* >( context );
        const UInt64 now = PumpNowMs();
        if ( now > table->lastHookMs )
            table->clock.AdvanceMs( (Int64)( now - table->lastHookMs ) * table->options.autoClockMs );
        table->lastHookMs = now;
    }

    ~CEMailTable()
    {
        if ( this == GetPumpHook().context )
        {
            GetPumpHook().function = 0;
            GetPumpHook().context = 0;
        }
        for ( size_t i=0; i<players.size(); ++i )
            if ( GUCEF_NULL != players[ i ]->provider )
            {
                players[ i ]->provider->Stop();
                players[ i ]->provider = GUCEF_NULL;
                players[ i ]->ptr = MULTIPLAY::CIMatchProviderPtr();
            }
        for ( size_t i=0; i<players.size(); ++i )
            delete players[ i ];
        players.clear();
    }

    static const char* NameOf( UInt32 index )
    {
        static const char* names[] = { "Alice", "Bob", "Carol", "Dave", "Erin" };
        return index < 5 ? names[ index ] : "Zed";
    }

    /** The settings of a player for their mailbox */
    void FillSettings( SPlayer& p ) const
    {
        MAIL::CEMailMatchProviderSettings& s = p.settings;
        s.mailAddress = p.address;
        s.displayName = p.name;
        s.smtp.host = s.imap.host = s.pop3.host = "127.0.0.1";
        s.smtp.port = p.box->smtp.GetPort();
        s.imap.port = p.box->imap.GetPort();
        s.pop3.port = p.box->pop3.GetPort();
        s.smtp.tlsMode = s.imap.tlsMode = s.pop3.tlsMode = COM::MAILTLS_NONE;
        s.smtp.allowInsecureAuth = s.imap.allowInsecureAuth = s.pop3.allowInsecureAuth = true;
        s.smtp.connectTimeoutInMs = s.imap.connectTimeoutInMs = s.pop3.connectTimeoutInMs = 5000;
        s.smtp.commandTimeoutInMs = s.imap.commandTimeoutInMs = s.pop3.commandTimeoutInMs = 5000;
        s.smtp.heloName = "player.mail.test";
        s.smtp.userName = s.imap.userName = s.pop3.userName = p.box->user;
        if ( !options.credentialsFromProvider )
            s.smtp.password = s.imap.password = s.pop3.password = p.box->password;
        s.useImap = options.useImap;
        s.useIdle = options.useIdle;
        s.sendAcknowledgements = options.acknowledgements;
        s.maxContainerBytes = options.maxContainerBytes;
        s.pollIntervalMs = options.pollIntervalMs;
        s.outboxResendIntervalMs = options.resendIntervalMs;
        s.seenSetSize = options.seenSetSize;
        s.autoJoinIntroduced = options.autoJoinIntroduced;
        s.seatsSettingKey = options.seatsSettingKey;
        s.decisionPolicySettingKey = options.decisionPolicySettingKey;
        s.tickMs = 20;
        s.sendRetryInitialMs = 500;
        s.sendRetryMaxMs = 4000;
        s.reconnectInitialMs = 200;
        s.reconnectMaxMs = 2000;
        s.stateDirectory = p.dir;
        s.customScheme = "redux";
        s.httpsHost = "games.example.invalid";
    }

    bool Setup( UInt32 count, const char* name, const SOptions& opts = SOptions() )
    {
        options = opts;
        root = CORE::CombinePath( CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "mailtemp" ), name );
        CORE::DeleteDir( root, true );
        CORE::CreateDirs( root );
        for ( UInt32 i=0; i<count; ++i )
        {
            SPlayer* p = new SPlayer;
            p->name = NameOf( i );
            if ( !TESTFW::MakeTestIdentity( p->name.C_String(), p->identity ) )
                return false;
            p->address = p->name.Lowercase() + "@mail.test";
            p->box = world.AddMailbox( p->address, p->name.Lowercase(), "pw-" + p->name.Lowercase() + "-Zq81" );
            p->dir = CORE::CombinePath( root, p->name.Lowercase() );
            players.push_back( p );
        }
        if ( !world.Start() )
            return false;
        if ( options.autoClockMs > 0 )
        {
            lastHookMs = PumpNowMs();
            GetPumpHook().function = &CEMailTable::ClockHook;
            GetPumpHook().context = this;
        }
        for ( UInt32 i=0; i<count; ++i )
        {
            SPlayer& p = *players[ i ];
            FillSettings( p );
            p.credentials.SetCredential( COM::MAILCRED_PASSWORD, p.box->user, p.box->password );
            if ( !MakeProvider( p ) )
                return false;
        }
        return true;
    }

    /** A provider of the player made by the factory the plugin registered, with the settings, the identity and the rest */
    bool MakeProvider( SPlayer& p )
    {
        p.ptr = MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().Create( "mail" );
        if ( p.ptr.IsNULL() )
            return false;
        p.provider = static_cast< MAIL::CEMailMatchProvider* >( p.ptr.GetPointerAlways() );
        p.provider->SetIdentity( p.identity );
        p.provider->SetClock( &clock );
        if ( options.installVerifier )
            p.provider->SetPackageVerifier( &p.verifier );
        p.provider->SetAllowUnverifiedPackages( options.allowUnverified );
        if ( options.credentialsFromProvider )
            p.provider->SetCredentialProvider( &p.credentials );
        if ( !p.provider->SetSettings( p.settings ) )
            return false;
        p.recorder.SubscribeTo( p.provider );
        return true;
    }

    /** Stops the provider of a player and makes a new one on the same files and the same mailbox, as after a restart of the application */
    bool Restart( UInt32 index )
    {
        SPlayer& p = *players[ index ];
        p.provider->Stop();
        p.recorder.UnsubscribeFrom( *p.provider );
        p.recorder.Clear();
        p.provider = GUCEF_NULL;
        p.ptr = MULTIPLAY::CIMatchProviderPtr();
        return MakeProvider( p );
    }

    bool StartAll( void )
    {
        for ( size_t i=0; i<players.size(); ++i )
            if ( !players[ i ]->provider->Start() )
                return false;
        return true;
    }

    SPlayer& P( UInt32 index ) { return *players[ index ]; }

    /** Lets the others and the mail servers work for the time */
    void Settle( UInt32 ms ) { PumpFor( ms ); }

    /** Moves the clock the providers see and lets them work */
    void Advance( UInt32 ms, UInt32 settleMs = 200 )
    {
        clock.AdvanceMs( ms );
        PumpFor( settleMs );
    }

    /* ---- matches ---- */

    MULTIPLAY::CMatchRecord MakeRecord( const char* title, MAIL::TMailTransfer transfer, MAIL::TMailDistribution distribution, bool allActive = false )
    {
        GUCEF::vector< MULTIPLAY::CPlayerIdentity > ids;
        for ( size_t i=0; i<players.size(); ++i )
            ids.push_back( players[ i ]->identity );
        MULTIPLAY::CMatchInfo info = TESTFW::MakeTestMatchInfo( ids, title, allActive );
        info.backend = "mail";
        info.status = allActive ? MULTIPLAY::MATCHSTATUS_ACTIVE : MULTIPLAY::MATCHSTATUS_FORMING;
        for ( size_t i=0; i<players.size(); ++i )
            info.participants[ i ].contact = players[ i ]->address;
        info.SetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER, MAIL::MailTransferToString( transfer ) );
        info.SetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_DISTRIBUTION, MAIL::MailDistributionToString( distribution ) );
        MULTIPLAY::CMatchRecord record( info );
        record.Sign( players[ 0 ]->identity );
        return record;
    }

    /** A package of the next revision on the newest turn the player holds (none: the first) with one more move */
    bool MakeTurn( UInt32 player, const CORE::CString& matchId, int nextHolder, const char* move, MULTIPLAY::TMatchStatus status, MULTIPLAY::CMatchPackage& package, MULTIPLAY::CMatchPreview& preview )
    {
        MULTIPLAY::CMatchPackage base;
        MULTIPLAY::CMatchPreview basePreview;
        MULTIPLAY::CMatchPackage* basePtr = GUCEF_NULL;
        const UInt32 trusted = players[ player ]->provider->GetTrustedRevision( matchId );
        CORE::CString text;
        if ( trusted > 0 )
        {
            if ( !players[ player ]->provider->GetStoredTurn( matchId, trusted, basePreview, base ) )
                return false;
            basePtr = &base;
            text = CGameVerifier::Text( base ) + "|" + move;
        }
        else
            text = CORE::CString( "game|setup|" ) + move;
        package = TESTFW::MakeTestPackage( trusted + 1, basePtr, text.C_String() );
        MULTIPLAY::CMatchInfo idOnly;
        idOnly.matchId = matchId;
        preview = TESTFW::MakeTestPreview( idOnly, players[ player ]->identity, package, nextHolder >= 0 ? &players[ nextHolder ]->identity : GUCEF_NULL, status, clock.nowMs );
        return true;
    }

    bool Await( CORE::CFutureResult& future, MULTIPLAY::CMatchOperationResult& result, UInt32 timeoutMs = 5000 )
    {
        bool ready = false;
        PUMP_WAIT( ready, future.IsReady(), timeoutMs );
        return ready && MULTIPLAY::CMatchOperationResult::TryGetFromFuture( future, result );
    }

    bool CreateMatch( UInt32 creator, const MULTIPLAY::CMatchRecord& record, int firstHolder, MULTIPLAY::CMatchOperationResult& result )
    {
        MULTIPLAY::CMatchPackage package = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "game|setup|start" );
        MULTIPLAY::CMatchPreview preview = TESTFW::MakeTestPreview( record.info, players[ creator ]->identity, package, firstHolder >= 0 ? &players[ firstHolder ]->identity : GUCEF_NULL, MULTIPLAY::MATCHSTATUS_ACTIVE, clock.nowMs );
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        if ( !players[ creator ]->provider->CreateMatch( record, package, preview, future ) )
            return false;
        return Await( future, result );
    }

    /** The turn of the player on the newest revision they hold */
    bool PlayTurn( UInt32 player, const CORE::CString& matchId, int nextHolder, const char* move, MULTIPLAY::CMatchOperationResult& result, MULTIPLAY::TMatchStatus status = MULTIPLAY::MATCHSTATUS_ACTIVE )
    {
        MULTIPLAY::CMatchPackage package;
        MULTIPLAY::CMatchPreview preview;
        if ( !MakeTurn( player, matchId, nextHolder, move, status, package, preview ) )
            return false;
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        if ( !players[ player ]->provider->Submit( matchId, package.revision - 1, preview, package, future ) )
            return false;
        return Await( future, result );
    }

    bool Respond( UInt32 player, const CORE::CString& matchId, bool accept, MULTIPLAY::CMatchOperationResult& result )
    {
        MULTIPLAY::CMatchInfo info;
        if ( !players[ player ]->provider->GetMatchInfo( matchId, info ) )
            return false;
        MULTIPLAY::CMatchAcceptance acceptance;
        if ( !acceptance.Sign( players[ player ]->identity, info, accept ) )
            return false;
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        if ( !players[ player ]->provider->Respond( matchId, acceptance, future ) )
            return false;
        return Await( future, result );
    }

    bool WaitTrusted( UInt32 player, const CORE::CString& matchId, UInt32 revision, UInt32 timeoutMs = 8000 )
    {
        bool ok = false;
        PUMP_WAIT( ok, players[ player ]->provider->GetTrustedRevision( matchId ) >= revision, timeoutMs );
        return ok;
    }

    bool WaitAllTrusted( const CORE::CString& matchId, UInt32 revision, UInt32 timeoutMs = 8000 )
    {
        for ( size_t i=0; i<players.size(); ++i )
            if ( !WaitTrusted( (UInt32) i, matchId, revision, timeoutMs ) )
                return false;
        return true;
    }

    bool WaitKnown( UInt32 player, const CORE::CString& matchId, UInt32 timeoutMs = 8000 )
    {
        bool ok = false;
        MULTIPLAY::CMatchInfo info;
        PUMP_WAIT( ok, players[ player ]->provider->GetMatchInfo( matchId, info ), timeoutMs );
        return ok;
    }

    MULTIPLAY::TMatchParticipantStatus StatusOf( UInt32 viewer, const CORE::CString& matchId, UInt32 subject )
    {
        MULTIPLAY::CMatchInfo info;
        if ( players[ viewer ]->provider->GetMatchInfo( matchId, info ) )
        {
            const MULTIPLAY::CMatchParticipant* participant = info.FindParticipant( players[ subject ]->identity.userId );
            if ( GUCEF_NULL != participant )
                return participant->status;
        }
        return MULTIPLAY::MATCHPARTICIPANT_DROPPED;
    }

    MULTIPLAY::TMatchStatus MatchStatusOf( UInt32 viewer, const CORE::CString& matchId )
    {
        MULTIPLAY::CMatchInfo info;
        return players[ viewer ]->provider->GetMatchInfo( matchId, info ) ? info.status : MULTIPLAY::MATCHSTATUS_ABANDONED;
    }

    /** The first player creates a match that all the others accept; returns when everybody sees it as active. Empty text when it failed */
    CORE::CString StartMatch( MAIL::TMailTransfer transfer, MAIL::TMailDistribution distribution, int firstHolder, const char* title = "Friday game" )
    {
        MULTIPLAY::CMatchRecord record = MakeRecord( title, transfer, distribution );
        MULTIPLAY::CMatchOperationResult result;
        if ( !CreateMatch( 0, record, firstHolder, result ) || MULTIPLAY::MATCHOPERATION_OK != result.status )
            return CORE::CString::Empty;
        const CORE::CString matchId = record.info.matchId;
        for ( UInt32 i=1; i<players.size(); ++i )
            if ( !WaitKnown( i, matchId ) )
                return CORE::CString::Empty;
        for ( UInt32 i=1; i<players.size(); ++i )
            if ( !Respond( i, matchId, true, result ) || MULTIPLAY::MATCHOPERATION_OK != result.status )
                return CORE::CString::Empty;
        for ( UInt32 i=0; i<players.size(); ++i )
        {
            bool active = false;
            PUMP_WAIT( active, MULTIPLAY::MATCHSTATUS_ACTIVE == MatchStatusOf( i, matchId ), 8000 );
            if ( !active )
                return CORE::CString::Empty;
        }
        return matchId;
    }

    /* ---- mail made by hand, for what the providers would never send ---- */

    /** A package of the next revision on the newest turn that the player 'baseHolder' holds, signed by 'signer' for 'nextHolder' (-1: nobody, the game is over) */
    bool CraftTurn( const MULTIPLAY::CPlayerIdentity& signer, UInt32 baseHolder, const CORE::CString& matchId, int nextHolder, const char* move, MULTIPLAY::CMatchPackage& package, MULTIPLAY::CMatchPreview& preview, MULTIPLAY::TMatchStatus status = MULTIPLAY::MATCHSTATUS_ACTIVE )
    {
        MULTIPLAY::CMatchPackage base;
        MULTIPLAY::CMatchPreview basePreview;
        const UInt32 trusted = players[ baseHolder ]->provider->GetTrustedRevision( matchId );
        if ( 0 == trusted || !players[ baseHolder ]->provider->GetStoredTurn( matchId, trusted, basePreview, base ) )
            return false;
        const CORE::CString text = CGameVerifier::Text( base ) + "|" + move;
        package = TESTFW::MakeTestPackage( trusted + 1, &base, text.C_String() );
        MULTIPLAY::CMatchInfo idOnly;
        idOnly.matchId = matchId;
        preview = TESTFW::MakeTestPreview( idOnly, signer, package, nextHolder >= 0 ? &players[ nextHolder ]->identity : GUCEF_NULL, status, clock.nowMs );
        return true;
    }

    /** The wire text of a mail with a turn from the address to the addresses, in the form the providers send */
    CORE::CString TurnMailText( const char* fromAddress, const GUCEF::vector< CORE::CString >& to, const CORE::CString& matchId, const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package, MAIL::TMailTransfer transfer, const char* messageId = GUCEF_NULL )
    {
        CORE::CDynamicBuffer container;
        if ( MULTIPLAY::MATCHCONTAINER_OK != MULTIPLAY::CMatchContainer::Encode( preview, package, true, container ) )
            return CORE::CString::Empty;
        return MailTextForData( fromAddress, to, matchId, preview.revision, MAIL::MAILKIND_TURN, container, MAIL::CEMailMatchCodec::ArmorLabelTurn, "r" + CORE::UInt32ToString( preview.revision ), transfer, messageId );
    }

    CORE::CString MailTextForData( const char* fromAddress, const GUCEF::vector< CORE::CString >& to, const CORE::CString& matchId, UInt32 revision, MAIL::TMailMatchKind kind, const CORE::CDynamicBuffer& data, const char* label, const CORE::CString& part, MAIL::TMailTransfer transfer, const char* messageId = GUCEF_NULL )
    {
        static UInt32 counter = 0;
        MAIL::CEMailOutgoing outgoing;
        outgoing.kind = kind;
        outgoing.matchId = matchId;
        outgoing.revision = revision;
        outgoing.turn = revision;
        outgoing.title = "Friday game";
        outgoing.from = COM::CEMailAddress( fromAddress );
        for ( size_t i=0; i<to.size(); ++i )
            outgoing.to.push_back( COM::CEMailAddress( to[ i ] ) );
        outgoing.transfer = transfer;
        outgoing.data = &data;
        outgoing.dataLabel = label;
        outgoing.dataRevisionName = part;
        COM::CEMailMessage message;
        CORE::CString reason;
        if ( !MAIL::CEMailMatchCodec::Compose( outgoing, message, reason ) )
            return CORE::CString::Empty;
        message.messageId = GUCEF_NULL != messageId ? CORE::CString( messageId ) : "<crafted-" + CORE::UInt32ToString( ++counter ) + "@crafted.test>";
        COM::CMimeComposer composer;
        CORE::CDynamicBuffer wire;
        if ( !composer.Compose( message, wire, reason ) )
            return CORE::CString::Empty;
        return CORE::CString( static_cast< const char* >( wire.GetConstBufferPtr() ), wire.GetDataSize() );
    }

    bool Inject( UInt32 toPlayer, const CORE::CString& text )
    {
        return !text.IsNULLOrEmpty() && world.DeliverTo( players[ toPlayer ]->address, text );
    }

    /** What a player's mailbox and outbox are doing, for the notes that explain a failure */
    CORE::CString Diagnose( UInt32 index )
    {
        SPlayer& p = *players[ index ];
        std::ostringstream out;
        const MAIL::CEMailInbox& inbox = p.provider->GetInbox();
        out << p.name.C_String() << ": started " << p.provider->IsStarted() << " online " << inbox.IsOnline() << " idling " << inbox.IsIdling() << " handled " << inbox.GetHandledCount() << " lastUid " << inbox.GetLastUid()
            << " validity " << inbox.GetUidValidity() << " seen " << inbox.GetSeenCount() << " | imap mails " << p.box->imap.GetMessages().size() << " imap connections " << p.box->imap.GetAcceptedConnections()
            << " | outbox items " << p.provider->GetOutbox().GetItemCount() << " unconfirmed " << p.provider->GetOutbox().GetUnconfirmedCount() << " smtp accepted " << p.box->smtp.GetMessages().size()
            << " | problems " << p.recorder.problems.size() << " errors " << p.recorder.errors.size() << " refused " << p.recorder.refused.size();
        GUCEF::vector< CORE::CString > commands = p.box->imap.GetCommands();
        out << " | last commands:";
        for ( size_t i = commands.size() > 8 ? commands.size() - 8 : 0; i<commands.size(); ++i )
            out << " [" << commands[ i ].C_String() << "]";
        return out.str().c_str();
    }

    /** The messages of the mailbox of a player that carry the header of a match */
    UInt32 CountMatchMails( UInt32 player, const char* kind = GUCEF_NULL )
    {
        UInt32 count = 0;
        GUCEF::vector< TESTFW::CTestImapServer::TMessage > messages = players[ player ]->box->imap.GetMessages();
        for ( size_t i=0; i<messages.size(); ++i )
            if ( messages[ i ].data.HasSubstr( "X-Redux-Match:" ) >= 0 && ( GUCEF_NULL == kind || messages[ i ].data.HasSubstr( CORE::CString( "X-Redux-Kind: " ) + kind + "\r\n" ) >= 0 ) )
                ++count;
        return count;
    }
};

} /* namespace MAILTEST */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILFIXTURE_H ? */
