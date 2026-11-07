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

#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COM_CMIMECOMPOSER_H
#include "gucefCOM_CMimeComposer.h"
#define GUCEF_COM_CMIMECOMPOSER_H
#endif /* GUCEF_COM_CMIMECOMPOSER_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILFILES_H
#include "matchprovidermail_CEMailFiles.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILFILES_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILFILES_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHSYSTEMCLOCK_H
#include "gucefMULTIPLAY_CMatchSystemClock.h"
#define GUCEF_MULTIPLAY_CMATCHSYSTEMCLOCK_H
#endif /* GUCEF_MULTIPLAY_CMATCHSYSTEMCLOCK_H ? */

#include "matchprovidermail_CEMailMatchProvider.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CEMailMatchProvider::ProviderName = "mail";
const CORE::CEvent CEMailMatchProvider::PackageRefusedEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchProvider::PackageRefusedEvent";
const CORE::CEvent CEMailMatchProvider::EquivocationDetectedEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchProvider::EquivocationDetectedEvent";
const CORE::CEvent CEMailMatchProvider::DeliveryStateChangedEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchProvider::DeliveryStateChangedEvent";
const CORE::CEvent CEMailMatchProvider::NudgeReceivedEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchProvider::NudgeReceivedEvent";
const CORE::CEvent CEMailMatchProvider::DecisionMessageEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchProvider::DecisionMessageEvent";
const CORE::CEvent CEMailMatchProvider::MailProblemEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchProvider::MailProblemEvent";
const CORE::CEvent CEMailMatchProvider::LobbyChangedEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchProvider::LobbyChangedEvent";
const CORE::CEvent CEMailMatchProvider::IntroductionEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchProvider::IntroductionEvent";
const CORE::CEvent CEMailMatchProvider::RosterChangedEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchProvider::RosterChangedEvent";
const CORE::CEvent CEMailMatchProvider::VoteReceivedEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchProvider::VoteReceivedEvent";
const CORE::CEvent CEMailMatchProvider::DraftDecisionAppliedEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchProvider::DraftDecisionAppliedEvent";

static const UInt32 g_maxStateFileBytes = 32 * 1024 * 1024;
static const UInt32 g_maxRefusalsKept = 16;
static const UInt32 g_maxOrphans = 32;
static const UInt32 g_maxReferences = 10;
static const UInt32 g_maxRemoved = 1000;
static const UInt32 g_missingRevisionsPerRequest = 8;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CEMailMatchProvider::RegisterEvents( void )
{GUCEF_TRACE;

    MULTIPLAY::CIMatchProvider::RegisterEvents();
    PackageRefusedEvent.Initialize();
    EquivocationDetectedEvent.Initialize();
    DeliveryStateChangedEvent.Initialize();
    NudgeReceivedEvent.Initialize();
    DecisionMessageEvent.Initialize();
    MailProblemEvent.Initialize();
    LobbyChangedEvent.Initialize();
    IntroductionEvent.Initialize();
    RosterChangedEvent.Initialize();
    VoteReceivedEvent.Initialize();
    DraftDecisionAppliedEvent.Initialize();
    CEMailOutbox::RegisterEvents();
    CEMailInbox::RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CEMailMatchProvider::CEMailMatchProvider( void )
    : MULTIPLAY::CIMatchProvider()
    , m_settings()
    , m_identity()
    , m_verifier( GUCEF_NULL )
    , m_allowUnverified( false )
    , m_credentials( GUCEF_NULL )
    , m_clock( GUCEF_NULL )
    , m_link()
    , m_timer( 1000 )
    , m_outbox()
    , m_inbox()
    , m_intros()
    , m_lastIntroCheck( 0 )
    , m_announcedDrafts()
    , m_boards()
    , m_rosterResent()
    , m_matches()
    , m_removed()
    , m_started( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_timer.SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

CEMailMatchProvider::~CEMailMatchProvider()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    if ( m_started )
        Stop();
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::SetIdentity( const MULTIPLAY::CPlayerIdentity& identity )
{GUCEF_TRACE;

    m_identity = identity;
    ConfigureIntroductions();
}
void CEMailMatchProvider::SetPackageVerifier( MULTIPLAY::CIMatchPackageVerifier* verifier ) { m_verifier = verifier; }
void CEMailMatchProvider::SetAllowUnverifiedPackages( bool allow ) { m_allowUnverified = allow; }
void CEMailMatchProvider::SetCredentialProvider( COM::CIEMailCredentialProvider* provider ) { m_credentials = provider; }
void CEMailMatchProvider::SetClock( const MULTIPLAY::CIMatchClock* clock ) { m_clock = clock; }
void CEMailMatchProvider::SetMatchLink( const MULTIPLAY::CMatchLink& link ) { m_link = link; }

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::SetSettings( const CEMailMatchProviderSettings& settings )
{GUCEF_TRACE;

    if ( m_started )
        return false;
    m_settings = settings;
    ConfigureIntroductions();
    return true;
}

/*-------------------------------------------------------------------------*/

const CEMailMatchProviderSettings&
CEMailMatchProvider::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return m_settings.SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    if ( m_started )
        return false;
    return m_settings.LoadConfig( config );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEMailMatchProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool CEMailMatchProvider::IsStarted( void ) const { return m_started; }

UInt32
CEMailMatchProvider::GetCapabilities( void ) const
{GUCEF_TRACE;

    return (UInt32) MULTIPLAY::MATCHCAPABILITY_NUDGE | (UInt32) MULTIPLAY::MATCHCAPABILITY_RESIGN | (UInt32) MULTIPLAY::MATCHCAPABILITY_REMATCH;
}

const CORE::CString& CEMailMatchProvider::GetProviderName( void ) const { return ProviderName; }
const CEMailInbox& CEMailMatchProvider::GetInbox( void ) const { return m_inbox; }
const CEMailOutbox& CEMailMatchProvider::GetOutbox( void ) const { return m_outbox; }

/*-------------------------------------------------------------------------*/

Int64
CEMailMatchProvider::GetNowMs( void ) const
{GUCEF_TRACE;

    return ( GUCEF_NULL != m_clock ? m_clock : &MULTIPLAY::CMatchSystemClock::Instance() )->GetNowMs();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailMatchProvider::GetMatchesDirectory( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( m_settings.stateDirectory, "matches" );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
RevisionFileName( UInt32 revision )
{GUCEF_TRACE;

    CORE::CString number = CORE::UInt32ToString( revision );
    while ( number.Length() < 6 )
        number = "0" + number;
    return "r" + number + ".rdxturn";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailMatchProvider::GetTurnPath( const SMatch& match, UInt32 revision ) const
{GUCEF_TRACE;

    return CORE::CombinePath( match.directory, RevisionFileName( revision ) );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::SaveMatch( SMatch& match )
{GUCEF_TRACE;

    if ( !CEMailFiles::WriteRecord( CORE::CombinePath( match.directory, "state.json" ), match.state ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProvider: The state of a match could not be written" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::SaveTurn( const SMatch& match, UInt32 revision, const void* container, UInt32 size )
{GUCEF_TRACE;

    return CEMailFiles::WriteAtomic( GetTurnPath( match, revision ), container, size );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::LoadTurn( const SMatch& match, UInt32 revision, MULTIPLAY::CMatchContainer& container ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    return CEMailFiles::ReadBounded( GetTurnPath( match, revision ), MULTIPLAY::CMatchContainer::MaxContainerSize, bytes ) && 
           MULTIPLAY::MATCHCONTAINER_OK == container.Decode( bytes.GetConstBufferPtr(), bytes.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

CEMailMatchProvider::SMatch*
CEMailMatchProvider::FindMatch( const CORE::CString& matchId )
{GUCEF_TRACE;

    TMatchMap::iterator i = m_matches.find( matchId );
    return i == m_matches.end() ? GUCEF_NULL : &(*i).second;
}

/*-------------------------------------------------------------------------*/

const CEMailMatchProvider::SMatch*
CEMailMatchProvider::FindMatch( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    TMatchMap::const_iterator i = m_matches.find( matchId );
    return i == m_matches.end() ? GUCEF_NULL : &(*i).second;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::IsRemoved( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    return m_removed.find( matchId ) != m_removed.end();
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::SaveRemoved( void )
{GUCEF_TRACE;

    CORE::CString text;
    for ( CORE::CString::StringSet::const_iterator i = m_removed.begin(); i != m_removed.end(); ++i )
        text += (*i) + "\n";
    return CEMailFiles::WriteAtomic( CORE::CombinePath( m_settings.stateDirectory, "removed.txt" ), text.C_String(), text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1 );
}

/*-------------------------------------------------------------------------*/

CEMailMatchProvider::SMatch*
CEMailMatchProvider::AddMatch( const MULTIPLAY::CMatchRecord& record )
{GUCEF_TRACE;

    SMatch& match = m_matches[ record.info.matchId ];
    match = SMatch();
    match.directory = CORE::CombinePath( GetMatchesDirectory(), record.info.matchId );
    CORE::CreateDirs( match.directory );
    match.state.record = record;
    match.state.hasRecord = true;
    match.state.info = record.info;
    // what the record says about who accepted is not signed and not believed: the creator takes part, the others are invited until they answer
    for ( size_t i=0; i<match.state.info.participants.size(); ++i )
    {
        MULTIPLAY::CMatchParticipant& participant = match.state.info.participants[ i ];
        participant.status = participant.userId == match.state.info.creatorUserId ? MULTIPLAY::MATCHPARTICIPANT_ACTIVE : MULTIPLAY::MATCHPARTICIPANT_INVITED;
        participant.outcome = MULTIPLAY::MATCHOUTCOME_NONE;
    }
    match.state.info.status = MULTIPLAY::MATCHSTATUS_FORMING;
    match.state.lastActivity = GetNowMs();
    return &match;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::LoadMatches( void )
{GUCEF_TRACE;

    m_matches.clear();
    CORE::CString::StringVector names;
    CEMailFiles::ListDirectories( GetMatchesDirectory(), names );
    for ( size_t i=0; i<names.size(); ++i )
    {
        if ( !MULTIPLAY::CMatchId::IsValid( names[ i ] ) )
            continue;
        SMatch match;
        match.directory = CORE::CombinePath( GetMatchesDirectory(), names[ i ] );
        if ( !CEMailFiles::ReadRecord( CORE::CombinePath( match.directory, "state.json" ), match.state, g_maxStateFileBytes ) || match.state.info.matchId != names[ i ] )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProvider: The files of a match are not usable and are left alone" );
            continue;
        }
        if ( match.state.trustedRevision > 0 )
        {
            MULTIPLAY::CMatchContainer container;
            if ( LoadTurn( match, match.state.trustedRevision, container ) )
            {
                match.trustedPreview = container.preview;
                match.trustedPackage = container.package;
                match.hasTrusted = true;
            }
            else
            {
                // the newest turn is lost: the chain starts again from what can be asked of the others
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProvider: The newest turn of a match is missing from its files" );
                match.state.trustedRevision = 0;
            }
        }
        m_matches[ names[ i ] ] = match;
    }
    m_removed.clear();
    CORE::CDynamicBuffer bytes;
    if ( CEMailFiles::ReadBounded( CORE::CombinePath( m_settings.stateDirectory, "removed.txt" ), 1024 * 1024, bytes ) && bytes.GetDataSize() > 0 )
    {
        CORE::CString::StringVector lines = CORE::CString( static_cast< const char* >( bytes.GetConstBufferPtr() ), bytes.GetDataSize() ).ParseElements( '\n', false );
        for ( size_t i=0; i<lines.size() && m_removed.size() < g_maxRemoved; ++i )
            if ( MULTIPLAY::CMatchId::IsValid( lines[ i ] ) )
                m_removed.insert( lines[ i ] );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::Start( void )
{GUCEF_TRACE;

    if ( m_started )
        return true;
    CORE::CString reason;
    if ( !m_settings.IsValid( reason ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProvider: The provider cannot start: " << reason );
        return false;
    }
    if ( !m_identity.HasKeyPair() || !m_identity.IsUserIdOfKey() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProvider: The provider cannot start: the player has no identity with a key pair" );
        return false;
    }
    if ( !CORE::CreateDirs( CORE::CombinePath( m_settings.stateDirectory, "outbox" ) ) || !CORE::CreateDirs( GetMatchesDirectory() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProvider: The provider cannot start: the directory for its files cannot be made" );
        return false;
    }
    LoadMatches();
    ConfigureIntroductions();
    m_intros.Load();
    LoadBoards();
    m_outbox.Configure( &m_settings, m_credentials, CORE::CombinePath( m_settings.stateDirectory, "outbox" ) );
    UInt32 skipped = 0;
    m_outbox.Load( &skipped );
    m_inbox.Configure( &m_settings, m_credentials, CORE::CombinePath( m_settings.stateDirectory, "mailbox.state" ) );

    TEventCallback tickCallback( this, &CEMailMatchProvider::OnTimerTick );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, tickCallback );
    TEventCallback messageCallback( this, &CEMailMatchProvider::OnInboxMessage );
    SubscribeTo( &m_inbox, CEMailInbox::MessageEvent, messageCallback );
    TEventCallback refusedCallback( this, &CEMailMatchProvider::OnInboxRefused );
    SubscribeTo( &m_inbox, CEMailInbox::RefusedEvent, refusedCallback );
    TEventCallback stateCallback( this, &CEMailMatchProvider::OnInboxState );
    SubscribeTo( &m_inbox, CEMailInbox::StateEvent, stateCallback );
    TEventCallback noticeCallback( this, &CEMailMatchProvider::OnInboxNotice );
    SubscribeTo( &m_inbox, CEMailInbox::NoticeEvent, noticeCallback );
    TEventCallback deliveryCallback( this, &CEMailMatchProvider::OnOutboxDelivery );
    SubscribeTo( &m_outbox, CEMailOutbox::DeliveryChangedEvent, deliveryCallback );

    m_started = true;
    m_timer.SetInterval( m_settings.tickMs );
    m_timer.SetEnabled( true );
    m_inbox.Start( GetNowMs() );

    // the matches that were kept are told to the match book again
    for ( TMatchMap::iterator i = m_matches.begin(); i != m_matches.end(); ++i )
        AnnounceMatch( (*i).second );
    GUCEF::vector< CEMailIntroDraft > drafts;
    m_intros.GetDrafts( drafts );
    for ( size_t i=0; i<drafts.size(); ++i )
        AnnounceDraft( drafts[ i ], true );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::Stop( void )
{GUCEF_TRACE;

    if ( !m_started )
        return;
    m_started = false;
    m_timer.SetEnabled( false );
    UnsubscribeFrom( &m_timer );
    UnsubscribeFrom( &m_inbox );
    UnsubscribeFrom( &m_outbox );
    m_inbox.Stop();
    m_outbox.Shutdown();
    for ( TMatchMap::iterator i = m_matches.begin(); i != m_matches.end(); ++i )
        SaveMatch( (*i).second );
    m_intros.Save();
    m_announcedDrafts.clear();
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::OnTimerTick( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_started )
        return;
    const Int64 now = GetNowMs();
    m_outbox.Update( now );
    m_inbox.Update( now );
    CheckIntroductionTimes( now );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::RefreshAll( void )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    m_inbox.CheckNow();
    for ( TMatchMap::iterator i = m_matches.begin(); i != m_matches.end(); ++i )
    {
        MULTIPLAY::CMatchUpdate update;
        MakeUpdate( (*i).second, update );
        ReportMatchUpdated( update );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::Refresh( const CORE::CString& matchId )
{GUCEF_TRACE;

    SMatch* match = FindMatch( matchId );
    if ( !m_started || GUCEF_NULL == match )
        return false;
    m_inbox.CheckNow();
    MULTIPLAY::CMatchUpdate update;
    MakeUpdate( *match, update );
    ReportMatchUpdated( update );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::MakeUpdate( const SMatch& match, MULTIPLAY::CMatchUpdate& update ) const
{GUCEF_TRACE;

    if ( match.hasTrusted )
        update.preview = match.trustedPreview;
    else
    {
        update.preview.matchId = match.state.info.matchId;
        update.preview.revision = 0;
        update.preview.status = match.state.info.status;
        update.preview.lastActivity = match.state.lastActivity;
    }
    update.info = match.state.info;
    update.hasInfo = true;
    update.cursor = CORE::UInt32ToString( m_inbox.GetUidValidity() ) + ":" + CORE::UInt32ToString( m_inbox.GetLastUid() );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::AnnounceMatch( SMatch& match )
{GUCEF_TRACE;

    MULTIPLAY::CMatchUpdate update;
    MakeUpdate( match, update );
    if ( !match.announced )
    {
        match.announced = true;
        ReportMatchAdded( update );
    }
    else
        ReportMatchUpdated( update );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::ReportNotice( const CORE::CEvent& eventId, const CEMailMatchNotice& notice )
{GUCEF_TRACE;

    CEMailMatchNotice data( notice );
    NotifyObservers( eventId, &data );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::Notice( const CORE::CEvent& eventId, const CORE::CString& matchId, UInt32 revision, const char* code, const CORE::CString& description, const CORE::CString& userId, const CORE::CString& proposalId )
{GUCEF_TRACE;

    CEMailMatchNotice notice;
    notice.matchId = matchId;
    notice.revision = revision;
    notice.code = code;
    notice.description = description;
    notice.userId = userId;
    notice.proposalId = proposalId;
    ReportNotice( eventId, notice );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::Succeed( CORE::CFutureResult& future, const CORE::CString& matchId, const MULTIPLAY::CMatchPreview* preview )
{GUCEF_TRACE;

    MULTIPLAY::CMatchOperationResult result;
    result.status = MULTIPLAY::MATCHOPERATION_OK;
    result.matchId = matchId;
    if ( GUCEF_NULL != preview )
    {
        result.preview = *preview;
        result.hasPreview = true;
    }
    CompleteFuture( future, result );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::Reject( CORE::CFutureResult& future, const CORE::CString& matchId, MULTIPLAY::TMatchOperationStatus status, const char* reason )
{GUCEF_TRACE;

    MULTIPLAY::CMatchOperationResult result;
    result.status = status;
    result.matchId = matchId;
    result.reason = reason;
    CompleteFuture( future, result );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::CompleteFuture( CORE::CFutureResult& future, const MULTIPLAY::CMatchOperationResult& result )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    future = source.GetFuture();
    source.Complete( result.Clone(), true );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::UpdateStatus( SMatch& match )
{GUCEF_TRACE;

    MULTIPLAY::CMatchInfo& info = match.state.info;
    bool everybodyIn = !info.participants.empty();
    for ( size_t i=0; i<info.participants.size(); ++i )
        if ( MULTIPLAY::MATCHPARTICIPANT_ACTIVE != info.participants[ i ].status && MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER != info.participants[ i ].status )
            everybodyIn = false;
    if ( MULTIPLAY::MATCHSTATUS_CANCELLED == info.status )
        return;
    if ( match.hasTrusted && MULTIPLAY::MATCHSTATUS_FINISHED == match.trustedPreview.status )
        info.status = MULTIPLAY::MATCHSTATUS_FINISHED;
    else
    if ( everybodyIn || match.state.trustedRevision >= 2 )
        info.status = MULTIPLAY::MATCHSTATUS_ACTIVE;
    else
        info.status = MULTIPLAY::MATCHSTATUS_FORMING;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::GetContact( const SMatch& match, const CORE::CString& userId, COM::CEMailAddress& address ) const
{GUCEF_TRACE;

    const MULTIPLAY::CMatchParticipant* participant = match.state.info.FindParticipant( userId );
    if ( GUCEF_NULL == participant || participant->contact.IsNULLOrEmpty() )
        return false;
    address = COM::CEMailAddress( participant->displayName.Length() <= 64 ? CORE::CString( CEMailMatchCodec::SanitizeTitle( participant->displayName ) ) : CORE::CString(), participant->contact );
    return address.IsValid();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailMatchProvider::GetNameOf( const SMatch& match, const CORE::CString& userId ) const
{GUCEF_TRACE;

    const MULTIPLAY::CMatchParticipant* participant = match.state.info.FindParticipant( userId );
    return GUCEF_NULL == participant ? CORE::CString() : participant->displayName;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::FindParticipantByAddress( const SMatch& match, const CORE::CString& address, CORE::CString& userId ) const
{GUCEF_TRACE;

    const CORE::CString lower = address.Lowercase();
    for ( size_t i=0; i<match.state.info.participants.size(); ++i )
        if ( match.state.info.participants[ i ].contact.Lowercase() == lower )
        {
            userId = match.state.info.participants[ i ].userId;
            return true;
        }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailMatchProvider::BuildLinkText( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    MULTIPLAY::CMatchLink link = m_link;
    if ( !m_settings.customScheme.IsNULLOrEmpty() || !m_settings.httpsHost.IsNULLOrEmpty() )
        link = MULTIPLAY::CMatchLink( m_settings.customScheme, m_settings.httpsHost );
    CORE::CString text;
    const CORE::CString https = link.MakeHttpsLink( matchId );
    const CORE::CString custom = link.MakeCustomLink( matchId );
    if ( !https.IsNULLOrEmpty() )
        text += "Open the game: " + https + "\n";
    if ( !custom.IsNULLOrEmpty() )
        text += "Or in the game: " + custom + "\n";
    if ( text.IsNULLOrEmpty() )
        text = "Open the game and choose the match " + MULTIPLAY::CMatchId::Short( matchId ) + "\n";
    return text;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::FillOutgoingBase( const SMatch& match, TMailMatchKind kind, CEMailOutgoing& outgoing ) const
{GUCEF_TRACE;

    outgoing.kind = kind;
    outgoing.matchId = match.state.info.matchId;
    outgoing.title = match.state.info.title;
    outgoing.from = COM::CEMailAddress( m_settings.displayName.Length() <= 64 ? CEMailMatchCodec::SanitizeTitle( m_settings.displayName ) : CORE::CString(), m_settings.mailAddress );
    outgoing.inReplyTo = match.state.lastMessageId;
    outgoing.references = match.state.references;
    outgoing.linkText = BuildLinkText( match.state.info.matchId );
    outgoing.transfer = CEMailMatchCodec::GetTransfer( match.state.info );
    outgoing.actorName = m_identity.displayName;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::NoteThread( SMatch& match, const CORE::CString& messageId )
{GUCEF_TRACE;

    if ( messageId.IsNULLOrEmpty() )
        return;
    match.state.lastMessageId = messageId;
    match.state.references.push_back( messageId );
    while ( match.state.references.size() > g_maxReferences )
        match.state.references.erase( match.state.references.begin() + 1 );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::QueueMail( SMatch& match, CEMailOutgoing& outgoing, bool needsConfirmation )
{GUCEF_TRACE;

    COM::CEMailMessage message;
    CORE::CString reason;
    if ( !CEMailMatchCodec::Compose( outgoing, message, reason ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProvider: A mail could not be made: " << reason );
        return false;
    }
    message.messageId = CEMailMatchCodec::MakeMessageId( m_settings.mailAddress );
    COM::CMimeComposer composer;
    CEMailOutboxItem item;
    if ( !composer.Compose( message, item.message, reason ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProvider: A mail could not be written: " << reason );
        return false;
    }
    item.kind = outgoing.kind;
    item.matchId = outgoing.matchId;
    item.revision = outgoing.revision;
    item.messageId = message.messageId;
    item.envelopeFrom = m_settings.mailAddress;
    item.packageSha256 = outgoing.packageSha256;
    item.needsConfirmation = needsConfirmation;
    COM::CEMailAddress::TEMailAddressVector all;
    message.GetEnvelopeRecipients( all );
    for ( size_t i=0; i<all.size(); ++i )
        item.recipients.push_back( all[ i ].GetAddress() );
    if ( 0 == m_outbox.Enqueue( item, GetNowMs() ) )
        return false;
    if ( MAILKIND_TURN == outgoing.kind || MAILKIND_FINISH == outgoing.kind || MAILKIND_INVITE == outgoing.kind || MAILKIND_ACCEPT == outgoing.kind || MAILKIND_DECLINE == outgoing.kind )
        NoteThread( match, message.messageId );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The participants a mail goes to: who takes part and is not the player */
void
CEMailMatchProvider::GetAudience( const SMatch& match, bool includeInvited, GUCEF::vector< CORE::CString >& userIds, bool includeSpectators ) const
{GUCEF_TRACE;

    userIds.clear();
    for ( size_t i=0; i<match.state.info.participants.size(); ++i )
    {
        const MULTIPLAY::CMatchParticipant& participant = match.state.info.participants[ i ];
        if ( participant.userId == m_identity.userId )
            continue;
        // a participant that the computer plays now still gets the mails: the person may want to see what happens
        if ( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == participant.status || MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER == participant.status || ( includeInvited && MULTIPLAY::MATCHPARTICIPANT_INVITED == participant.status ) ||
             ( includeSpectators && MULTIPLAY::MATCHPARTICIPANT_SPECTATOR == participant.status ) )
            userIds.push_back( participant.userId );
    }
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::MailTurn( SMatch& match, const MULTIPLAY::CMatchPreview& preview, const CORE::CDynamicBuffer& container, bool useTransfer, TMailTransfer transfer, const CORE::CString& onlyToAddress )
{GUCEF_TRACE;

    // those who were invited and have not answered yet get the turns too: they need the game when they accept
    // and the participants who watch get every turn, whatever the distribution is: they follow the game
    GUCEF::vector< CORE::CString > audience;
    GetAudience( match, true, audience, true );

    COM::CEMailAddress::TEMailAddressVector to, cc;
    if ( !onlyToAddress.IsNULLOrEmpty() )
        to.push_back( COM::CEMailAddress( onlyToAddress ) );
    else
    {
        const bool everybody = MULTIPLAY::MATCHSTATUS_FINISHED == preview.status || MAILDISTRIBUTION_ALL == CEMailMatchCodec::GetDistribution( match.state.info );
        GUCEF::vector< CORE::CString > nextHolders;
        for ( size_t i=0; i<audience.size(); ++i )
            if ( preview.IsCurrent( audience[ i ] ) )
                nextHolders.push_back( audience[ i ] );
        COM::CEMailAddress address;
        for ( size_t i=0; i<nextHolders.size(); ++i )
            if ( GetContact( match, nextHolders[ i ], address ) )
                to.push_back( address );
        for ( size_t i=0; i<audience.size(); ++i )
        {
            const MULTIPLAY::CMatchParticipant* listed = match.state.info.FindParticipant( audience[ i ] );
            const bool watches = GUCEF_NULL != listed && listed->IsSpectator();
            if ( !everybody && !watches )
                continue;
            bool isTo = false;
            for ( size_t h=0; h<nextHolders.size(); ++h )
                isTo = isTo || nextHolders[ h ] == audience[ i ];
            if ( !isTo && GetContact( match, audience[ i ], address ) )
                ( MULTIPLAY::MATCHSTATUS_FINISHED == preview.status ? to : cc ).push_back( address );
        }
        if ( to.empty() && !cc.empty() )
        {
            to = cc;
            cc.clear();
        }
    }
    if ( to.empty() )
        return true;        // nobody else to tell

    CEMailOutgoing outgoing;
    FillOutgoingBase( match, MULTIPLAY::MATCHSTATUS_FINISHED == preview.status ? MAILKIND_FINISH : MAILKIND_TURN, outgoing );
    outgoing.revision = preview.revision;
    outgoing.turn = preview.turn;
    outgoing.to = to;
    outgoing.cc = cc;
    outgoing.packageSha256 = preview.packageSha256;
    outgoing.data = &container;
    outgoing.dataLabel = CEMailMatchCodec::ArmorLabelTurn;
    outgoing.dataRevisionName = "r" + CORE::UInt32ToString( preview.revision );
    if ( useTransfer )
        outgoing.transfer = transfer;
    const CORE::CString issuerUserId = preview.GetIssuerUserId();
    outgoing.actorName = GetNameOf( match, issuerUserId );
    for ( CORE::CString::StringSet::const_iterator c = preview.currentUserIds.begin(); c != preview.currentUserIds.end(); ++c )
        outgoing.nextName += ( outgoing.nextName.IsNULLOrEmpty() ? "" : " and " ) + GetNameOf( match, *c );
    return QueueMail( match, outgoing, onlyToAddress.IsNULLOrEmpty() );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::MailControl( SMatch& match, TMailMatchKind kind, const CORE::CIDataNodeSerializable& data, UInt32 revision, const CORE::CString& dataPart, const CORE::CString& toUserId, const CORE::CString& firstToUserId, bool includeInvited, bool needsConfirmation, bool includeSpectators )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    if ( !CEMailMatchCodec::SerializeToBytes( data, bytes, m_settings.maxControlBytes ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProvider: The data of a mail could not be made" );
        return false;
    }
    GUCEF::vector< CORE::CString > userIds;
    if ( !toUserId.IsNULLOrEmpty() )
        userIds.push_back( toUserId );
    else
        GetAudience( match, includeInvited, userIds, includeSpectators );
    COM::CEMailAddress::TEMailAddressVector to, cc;
    COM::CEMailAddress address;
    for ( size_t i=0; i<userIds.size(); ++i )
    {
        if ( !GetContact( match, userIds[ i ], address ) )
            continue;
        // the one who is addressed first, the others are told along
        if ( ( !firstToUserId.IsNULLOrEmpty() && userIds[ i ] == firstToUserId ) || ( firstToUserId.IsNULLOrEmpty() && to.empty() ) )
            to.push_back( address );
        else
            cc.push_back( address );
    }
    if ( to.empty() && !cc.empty() )
    {
        to.push_back( cc.front() );
        cc.erase( cc.begin() );
    }
    if ( to.empty() )
        return true;
    CEMailOutgoing outgoing;
    FillOutgoingBase( match, kind, outgoing );
    outgoing.revision = revision;
    outgoing.turn = revision;
    outgoing.to = to;
    outgoing.cc = cc;
    outgoing.data = &bytes;
    outgoing.dataLabel = CEMailMatchCodec::ArmorLabelData;
    outgoing.dataRevisionName = dataPart;
    return QueueMail( match, outgoing, needsConfirmation );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::CreateMatch( const MULTIPLAY::CMatchRecord& record, const MULTIPLAY::CMatchPackage& firstPackage, const MULTIPLAY::CMatchPreview& firstPreview, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    const CORE::CString matchId = record.info.matchId;
    CORE::CString reason;
    if ( !record.Verify( reason ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The record of the match is not valid" );
        return true;
    }
    if ( record.info.creatorUserId != m_identity.userId )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The record was not made by the player" );
        return true;
    }
    if ( NULL != FindMatch( matchId ) || IsRemoved( matchId ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The match exists already" );
        return true;
    }
    const MULTIPLAY::CMatchParticipant* me = record.info.FindParticipant( m_identity.userId );
    if ( GUCEF_NULL == me || me->contact.Lowercase() != m_settings.mailAddress.Lowercase() )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The contact of the player in the record is not the mailbox of the provider" );
        return true;
    }
    for ( size_t i=0; i<record.info.participants.size(); ++i )
        if ( !COM::CEMailAddress( record.info.participants[ i ].contact ).IsValid() )
        {
            Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "A participant has no usable mail address" );
            return true;
        }
    CEMailIntroDraft draft;
    const bool fromDraft = m_intros.FindDraft( matchId, draft );
    if ( fromDraft )
    {
        // the table is the one that was introduced: every invited player is a contact of the draft that introduced itself, with this address and this key
        bool matches = record.info.participants.size() >= 2;
        for ( size_t i=0; matches && i<record.info.participants.size(); ++i )
        {
            const MULTIPLAY::CMatchParticipant& participant = record.info.participants[ i ];
            if ( participant.userId == m_identity.userId )
                continue;
            const CEMailIntroContact* contact = draft.FindContact( participant.contact );
            matches = GUCEF_NULL != contact && MAILINTRO_INTRODUCED == contact->state && contact->publicKey == participant.publicKey.Lowercase();
        }
        if ( !matches )
        {
            Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The table is not the one that was introduced" );
            return true;
        }
    }
    TMailTransfer transfer;
    TMailDistribution distribution;
    const CORE::CString transferText = record.info.GetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER );
    const CORE::CString distributionText = record.info.GetSetting( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_DISTRIBUTION );
    if ( ( !transferText.IsNULLOrEmpty() && !TryStringToMailTransfer( transferText, transfer ) ) || ( !distributionText.IsNULLOrEmpty() && !TryStringToMailDistribution( distributionText, distribution ) ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The transfer or the distribution of the match is not known" );
        return true;
    }
    if ( 1 != firstPackage.revision || 1 != firstPreview.revision )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The first turn has to be revision 1" );
        return true;
    }

    SMatch* match = AddMatch( record );
    match->announced = m_announcedDrafts.find( matchId ) != m_announcedDrafts.end();
    ImportDraftStatements( *match );
    if ( !ApplyTurn( *match, firstPreview, firstPackage, true, GUCEF_NULL, CORE::CString::Empty, reason ) )
    {
        m_matches.erase( matchId );
        CORE::DeleteDir( CORE::CombinePath( GetMatchesDirectory(), matchId ), true );
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, reason.C_String() );
        return true;
    }
    // the invitation: the signed record, the first turn, and what a person needs to know
    CORE::CDynamicBuffer container;
    CORE::CDynamicBuffer recordBytes;
    if ( MULTIPLAY::MATCHCONTAINER_OK != MULTIPLAY::CMatchContainer::Encode( firstPreview, firstPackage, true, container ) || !CEMailMatchCodec::SerializeToBytes( record, recordBytes, m_settings.maxControlBytes ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The invitation could not be made" );
        return true;
    }
    for ( size_t i=0; i<match->state.info.participants.size(); ++i )
    {
        const MULTIPLAY::CMatchParticipant& participant = match->state.info.participants[ i ];
        COM::CEMailAddress address;
        if ( participant.userId == m_identity.userId || !GetContact( *match, participant.userId, address ) )
            continue;
        CEMailOutgoing outgoing;
        FillOutgoingBase( *match, MAILKIND_INVITE, outgoing );
        outgoing.revision = 1;
        outgoing.turn = 1;
        outgoing.to.push_back( address );
        outgoing.data = &recordBytes;
        outgoing.dataLabel = CEMailMatchCodec::ArmorLabelData;
        outgoing.dataRevisionName = "record";
        outgoing.extraData = &container;
        outgoing.extraLabel = CEMailMatchCodec::ArmorLabelTurn;
        outgoing.extraRevisionName = "r1";
        outgoing.packageSha256 = firstPackage.sha256;
        QueueMail( *match, outgoing, true );
    }
    SaveMatch( *match );
    if ( fromDraft )
    {
        m_intros.RemoveDraft( matchId );
        m_announcedDrafts.erase( matchId );
    }
    Succeed( future, matchId, &firstPreview );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::Respond( const CORE::CString& matchId, const MULTIPLAY::CMatchAcceptance& acceptance, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_NOT_FOUND, "The match is not known" );
        return true;
    }
    CORE::CString reason;
    if ( MULTIPLAY::MATCHSTATUS_CANCELLED == match->state.info.status )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The match was cancelled" );
        return true;
    }
    MULTIPLAY::CMatchParticipant* me = match->state.info.FindParticipant( m_identity.userId );
    if ( GUCEF_NULL == me || acceptance.userId != m_identity.userId || acceptance.matchId != matchId || !match->state.hasRecord || !acceptance.Verify( match->state.record.info, reason ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The answer is not the signed answer of the player to this record" );
        return true;
    }
    const MULTIPLAY::TMatchParticipantStatus wanted = acceptance.accepted ? MULTIPLAY::MATCHPARTICIPANT_ACTIVE : MULTIPLAY::MATCHPARTICIPANT_DECLINED;
    if ( me->userId == match->state.info.creatorUserId || ( MULTIPLAY::MATCHPARTICIPANT_INVITED != me->status && wanted != me->status ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The player has answered already" );
        return true;
    }
    if ( me->status != wanted )
    {
        me->status = wanted;
        match->state.acceptances.push_back( acceptance );
        UpdateStatus( *match );
        match->state.lastActivity = GetNowMs();
        // the creator first, the other participants are told along
        MailControl( *match, acceptance.accepted ? MAILKIND_ACCEPT : MAILKIND_DECLINE, acceptance, match->state.trustedRevision, "acceptance", CORE::CString::Empty, match->state.info.creatorUserId, true, false, true );
        SaveMatch( *match );
        AnnounceMatch( *match );
    }
    Succeed( future, matchId, GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::Fetch( const CORE::CString& matchId, UInt32 revision, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    const SMatch* match = FindMatch( matchId );
    MULTIPLAY::CMatchContainer container;
    MULTIPLAY::CMatchOperationResult result;
    result.matchId = matchId;
    if ( GUCEF_NULL == match || !LoadTurn( *match, revision, container ) )
        result.status = MULTIPLAY::MATCHOPERATION_NOT_FOUND;
    else
    {
        result.package = container.package;
        result.hasPackage = true;
        result.preview = container.preview;
        result.hasPreview = true;
    }
    CompleteFuture( future, result );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::Submit( const CORE::CString& matchId, UInt32 expectedRevision, const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    MULTIPLAY::CMatchOperationResult result;
    result.matchId = matchId;
    SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
    {
        result.status = MULTIPLAY::MATCHOPERATION_NOT_FOUND;
        result.reason = "The match is not known";
        CompleteFuture( future, result );
        ReportSubmitResult( result );
        return true;
    }
    const UInt32 trusted = match->state.trustedRevision;
    CORE::CString reason;
    bool proceed = false;
    if ( preview.matchId != matchId || preview.revision != package.revision || !package.IsConsistent() || preview.packageSha256 != package.sha256 )
    {
        result.status = MULTIPLAY::MATCHOPERATION_REJECTED;
        result.reason = "The preview and the package do not belong together";
    }
    else
    if ( preview.revision <= trusted )
    {
        // the same package again is safe; another one for a revision that exists means the caller is behind
        GUCEF::map< UInt32, CORE::CString >::const_iterator known = match->state.shaByRevision.find( preview.revision );
        MULTIPLAY::CMatchContainer held;
        if ( known != match->state.shaByRevision.end() && (*known).second == package.sha256 && LoadTurn( *match, preview.revision, held ) )
        {
            result.status = MULTIPLAY::MATCHOPERATION_OK;
            result.preview = held.preview;
            result.hasPreview = true;
        }
        else
        {
            result.status = MULTIPLAY::MATCHOPERATION_STALE;
            result.preview = match->trustedPreview;
            result.hasPreview = match->hasTrusted;
            result.reason = "The match is further than the caller expected";
        }
    }
    else
    if ( expectedRevision < trusted )
    {
        result.status = MULTIPLAY::MATCHOPERATION_STALE;
        result.preview = match->trustedPreview;
        result.hasPreview = match->hasTrusted;
        result.reason = "The match is further than the caller expected";
    }
    else
    if ( expectedRevision > trusted || preview.revision != trusted + 1 )
    {
        result.status = MULTIPLAY::MATCHOPERATION_REJECTED;
        result.reason = "The revision does not follow the newest revision of the match";
    }
    else
    if ( preview.issuerKey.Lowercase() != m_identity.publicKey.Lowercase() )
    {
        result.status = MULTIPLAY::MATCHOPERATION_REJECTED;
        result.reason = "The turn is not signed by the player";
    }
    else
    if ( MULTIPLAY::MATCHSTATUS_CANCELLED == match->state.info.status )
    {
        result.status = MULTIPLAY::MATCHOPERATION_REJECTED;
        result.reason = "The match was cancelled";
    }
    else
    if ( trusted > 0 && MULTIPLAY::MATCHSTATUS_FORMING == match->state.info.status && !HasStarted( *match ) && IsForceStartCovering( *match, package ) )
        proceed = true;             // the table decided to start with the computer in the seats of those who did not answer
    else
    if ( trusted > 0 && MULTIPLAY::MATCHSTATUS_FORMING == match->state.info.status )
    {
        bool anyInvited = false, anyOther = false;
        for ( size_t i=0; i<match->state.info.participants.size(); ++i )
        {
            anyInvited = anyInvited || MULTIPLAY::MATCHPARTICIPANT_INVITED == match->state.info.participants[ i ].status;
            anyOther = anyOther || ( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == match->state.info.participants[ i ].status && match->state.info.participants[ i ].userId != m_identity.userId );
        }
        if ( anyInvited || !anyOther )
        {
            result.status = MULTIPLAY::MATCHOPERATION_REJECTED;
            result.reason = anyInvited ? "Not everybody has answered the invitation yet" : "Nobody has accepted the invitation";
        }
        else
            proceed = true;
    }
    else
        proceed = true;

    CORE::CDynamicBuffer container;
    if ( proceed )
    {
        if ( MULTIPLAY::MATCHCONTAINER_OK != MULTIPLAY::CMatchContainer::Encode( preview, package, true, container ) || container.GetDataSize() > m_settings.maxContainerBytes )
        {
            result.status = MULTIPLAY::MATCHOPERATION_REJECTED;
            result.reason = "The turn is too large to be mailed";
            proceed = false;
        }
        else
        if ( MAILTRANSFER_BODY_TEXT == CEMailMatchCodec::GetTransfer( match->state.info ) )
        {
            CORE::CString armored;
            MULTIPLAY::CMatchTextArmor armor( CEMailMatchCodec::ArmorLabelTurn );
            if ( !armor.Encode( matchId, preview.revision, container.GetConstBufferPtr(), container.GetDataSize(), armored ) || armored.Length() > m_settings.maxBodyTextBytes )
            {
                result.status = MULTIPLAY::MATCHOPERATION_REJECTED;
                result.reason = "The turn is too large to be mailed as text";
                proceed = false;
            }
        }
    }
    if ( proceed )
    {
        if ( !ApplyTurn( *match, preview, package, true, &container, CORE::CString::Empty, reason ) )
        {
            result.status = MULTIPLAY::MATCHOPERATION_REJECTED;
            result.reason = reason;
        }
        else
        {
            result.status = MULTIPLAY::MATCHOPERATION_OK;
            result.preview = preview;
            result.hasPreview = true;
            // the turn is the player's own and applied: the mail is a delivery. The next player gets it, the others too in ALL mode
            if ( !MailTurn( *match, preview, container, false, MAILTRANSFER_ATTACHMENT, CORE::CString::Empty ) )
                MailProblem( matchId, preview.revision, "MAIL", "The mail for the turn could not be made; the turn is applied and can be sent again" );
            SaveMatch( *match );
        }
    }
    CompleteFuture( future, result );
    ReportSubmitResult( result );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::MailProblem( const CORE::CString& matchId, UInt32 revision, const char* code, const CORE::CString& description )
{GUCEF_TRACE;

    Notice( MailProblemEvent, matchId, revision, code, description, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::Resign( const CORE::CString& matchId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_NOT_FOUND, "The match is not known" );
        return true;
    }
    MULTIPLAY::CMatchParticipant* me = match->state.info.FindParticipant( m_identity.userId );
    if ( GUCEF_NULL == me || ( MULTIPLAY::MATCHPARTICIPANT_ACTIVE != me->status && MULTIPLAY::MATCHPARTICIPANT_INVITED != me->status && MULTIPLAY::MATCHPARTICIPANT_SPECTATOR != me->status ) ||
         MULTIPLAY::MATCHSTATUS_FINISHED == match->state.info.status || MULTIPLAY::MATCHSTATUS_CANCELLED == match->state.info.status )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The player cannot leave this match now" );
        return true;
    }
    CEMailSignedStatement statement;
    statement.matchId = matchId;
    statement.statementKind = CEMailSignedStatement::KindResign;
    statement.revision = match->state.trustedRevision;
    statement.createdAt = GetNowMs();
    if ( !statement.Sign( m_identity ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The leave could not be signed" );
        return true;
    }
    me->status = MULTIPLAY::MATCHPARTICIPANT_LEFT;
    me->outcome = MULTIPLAY::MATCHOUTCOME_QUIT;
    match->state.resigns.push_back( statement );
    // everybody who takes part or was invited is told
    MailControl( *match, MAILKIND_RESIGN, statement, match->state.trustedRevision, "resign", CORE::CString::Empty, CORE::CString::Empty, true, false, true );
    match->state.lastActivity = GetNowMs();
    SaveMatch( *match );
    AnnounceMatch( *match );
    Succeed( future, matchId, GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::Nudge( const CORE::CString& matchId, const CORE::CString& userId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_NOT_FOUND, "The match is not known" );
        return true;
    }
    const MULTIPLAY::CMatchParticipant* target = match->state.info.FindParticipant( userId );
    const Int64 now = GetNowMs();
    if ( GUCEF_NULL == target || userId == m_identity.userId || MULTIPLAY::MATCHPARTICIPANT_ACTIVE != target->status )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The player cannot be reminded" );
        return true;
    }
    if ( !match->hasTrusted || MULTIPLAY::MATCHSTATUS_ACTIVE != match->state.info.status || !match->trustedPreview.IsCurrent( userId ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "It is not the turn of that player" );
        return true;
    }
    GUCEF::map< CORE::CString, Int64 >::const_iterator last = match->state.nudgesSent.find( userId );
    if ( last != match->state.nudgesSent.end() && now < (*last).second + (Int64) m_settings.nudgeIntervalMs )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "That player was reminded not long ago" );
        return true;
    }
    CEMailSignedStatement statement;
    statement.matchId = matchId;
    statement.statementKind = CEMailSignedStatement::KindNudge;
    statement.targetUserId = userId;
    statement.revision = match->state.trustedRevision;
    statement.createdAt = now;
    if ( !statement.Sign( m_identity ) || !MailControl( *match, MAILKIND_NUDGE, statement, match->state.trustedRevision, "nudge", userId, CORE::CString::Empty, false, false ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The reminder could not be made" );
        return true;
    }
    match->state.nudgesSent[ userId ] = now;
    SaveMatch( *match );
    Succeed( future, matchId, GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/


bool
CEMailMatchProvider::HasStarted( const SMatch& match ) const
{GUCEF_TRACE;

    // the first turn of the game after the start of the match is revision 2: from then on the decisions that are for the time before it are over
    return match.state.trustedRevision >= 2;
}

/*-------------------------------------------------------------------------*/

/** The words of a reason without control characters, cut on a character boundary at the limit of a participant's status reason */
static CORE::CString
CleanReason( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString clean;
    const char* p = text.C_String();
    const UInt32 length = text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
    for ( UInt32 i=0; i<length; ++i )
    {
        const char c = ( (unsigned char) p[ i ] < 0x20 || 0x7F == (unsigned char) p[ i ] ) ? ' ' : p[ i ];
        clean.Append( &c, 1 );
    }
    if ( clean.ByteSize() - 1 <= MULTIPLAY::CMatchParticipant::MaxStatusReasonLength )
        return clean;
    const char* q = clean.C_String();
    UInt32 cut = MULTIPLAY::CMatchParticipant::MaxStatusReasonLength;
    while ( cut > 0 && 0x80 == ( (unsigned char) q[ cut ] & 0xC0 ) )
        --cut;
    return CORE::CString( q, cut );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::ReportLobbyDecision( const SMatch& match, const MULTIPLAY::CMatchProposal& proposal )
{GUCEF_TRACE;

    const CORE::CString& matchId = match.state.info.matchId;
    switch ( proposal.kind )
    {
        case MULTIPLAY::MATCHDECISION_CANCEL:
            Notice( LobbyChangedEvent, matchId, match.state.trustedRevision, "CANCELLED", "The match was cancelled before it began", proposal.proposerUserId, proposal.proposalId );
            break;
        case MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR:
            if ( proposal.key == GUCEF_MULTIPLAY_SPECTATOR_REASON_VOLUNTARY )
                Notice( LobbyChangedEvent, matchId, match.state.trustedRevision, "PLAYER_SPECTATING", "A participant watches instead of playing", proposal.targetUserId, proposal.proposalId );
            else
                Notice( LobbyChangedEvent, matchId, match.state.trustedRevision, "PLAYER_INCOMPATIBLE", CleanReason( proposal.value ), proposal.targetUserId, proposal.proposalId );
            break;
        case MULTIPLAY::MATCHDECISION_FORCE_START:
            Notice( LobbyChangedEvent, matchId, match.state.trustedRevision, "PLAYERS_REPLACED", "The computer replaces the participants who did not answer", proposal.proposerUserId, proposal.proposalId );
            break;
        case MULTIPLAY::MATCHDECISION_KICK:
            Notice( LobbyChangedEvent, matchId, match.state.trustedRevision, "PLAYER_KICKED", "A participant was removed before the match began", proposal.targetUserId, proposal.proposalId );
            break;
        default:
            break;
    }
}

/*-------------------------------------------------------------------------*/

/** Verifies a decision that is not carried by a turn (a cancel, a participant's own statement) and carries it out; the proposal is kept as evidence for the first turn */
bool
CEMailMatchProvider::ApplyEvidenceOutsideTurn( SMatch& match, const MULTIPLAY::CMatchProposal& proposal, CORE::CString& reason )
{GUCEF_TRACE;

    MULTIPLAY::CMatchDecisionEvidence evidence;
    evidence.proposal = proposal;
    return ApplyEvidenceOutsideTurn( match, evidence, reason );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::ApplyEvidenceOutsideTurn( SMatch& match, const MULTIPLAY::CMatchDecisionEvidence& evidence, CORE::CString& reason )
{GUCEF_TRACE;

    const MULTIPLAY::CMatchProposal& proposal = evidence.proposal;
    const CORE::CString digest = proposal.GetDigest();
    for ( size_t a=0; a<match.state.appliedDecisions.size(); ++a )
        if ( match.state.appliedDecisions[ a ] == digest )
        {
            reason = "known already";
            return false;
        }
    MULTIPLAY::CMatchInfo newInfo = match.state.info;
    if ( !MULTIPLAY::CMatchDecisionVerifier::Verify( match.state.info, evidence, GetNowMs(), reason, HasStarted( match ) ) ||
         !MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( newInfo, proposal, reason ) )
        return false;
    match.state.info = newInfo;
    match.state.appliedDecisions.push_back( digest );
    StoreProposal( match, proposal );
    UpdateStatus( match );
    match.state.lastActivity = GetNowMs();
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  A match decided by vote is closed by the players: every proposal to cancel that passed here, with the votes known here, is carried out. Every machine
 *  counts for itself and arrives at the same result from the same signed proposals and votes (the electorate is in the proposal), so nothing else is mailed.
 */
void
CEMailMatchProvider::ApplyPassedCancels( SMatch& match )
{GUCEF_TRACE;

    if ( MULTIPLAY::MATCHDECISIONPOLICY_BOSS == match.state.info.decisionPolicy || HasStarted( match ) )
        return;
    for ( size_t i=0; i<match.state.proposals.size() && MULTIPLAY::MATCHSTATUS_CANCELLED != match.state.info.status; ++i )
    {
        if ( MULTIPLAY::MATCHDECISION_CANCEL != match.state.proposals[ i ].kind )
            continue;
        MULTIPLAY::CMatchDecisionEvidence evidence;
        if ( !GetDecisionEvidence( match.state.info.matchId, match.state.proposals[ i ].proposalId, evidence ) )
            continue;
        MULTIPLAY::CMatchDecisionResult tally;
        if ( !MULTIPLAY::CMatchDecisionVerifier::Evaluate( match.state.info, evidence, GetNowMs(), tally, false ) || !tally.valid || MULTIPLAY::MATCHVOTEOUTCOME_PASSED != tally.outcome )
            continue;
        CORE::CString reason;
        if ( !ApplyEvidenceOutsideTurn( match, evidence, reason ) )
            continue;
        m_outbox.RemoveUnconfirmed( match.state.info.matchId );
        SaveMatch( match );
        AnnounceMatch( match );
        ReportLobbyDecision( match, evidence.proposal );
    }
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::DeclareSpectator( SMatch& match, const CORE::CString& code, const CORE::CString& reason, CORE::CString& why )
{GUCEF_TRACE;

    const MULTIPLAY::CMatchParticipant* me = match.state.info.FindParticipant( m_identity.userId );
    if ( GUCEF_NULL == me || ( MULTIPLAY::MATCHPARTICIPANT_INVITED != me->status && MULTIPLAY::MATCHPARTICIPANT_ACTIVE != me->status ) )
    {
        why = "The player takes no part in the match that could be given up";
        return false;
    }
    MULTIPLAY::CMatchProposal statement;
    statement.matchId = match.state.info.matchId;
    statement.kind = MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR;
    statement.targetUserId = m_identity.userId;
    statement.key = code;
    statement.value = CleanReason( reason );
    statement.createdAt = GetNowMs();
    if ( !statement.Sign( m_identity ) )
    {
        why = "The statement could not be signed";
        return false;
    }
    if ( !ApplyEvidenceOutsideTurn( match, statement, why ) )
        return false;
    // the creator first, everybody who takes part, was invited or watches is told; the mail stays until the others showed that it arrived
    MailControl( match, MAILKIND_INCOMPATIBLE, statement, match.state.trustedRevision, "spectator", CORE::CString::Empty, match.state.info.creatorUserId, true, true, true );
    SaveMatch( match );
    AnnounceMatch( match );
    ReportLobbyDecision( match, statement );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::ChooseSpectator( const CORE::CString& matchId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_NOT_FOUND, "The match is not known" );
        return true;
    }
    CORE::CString why;
    if ( !DeclareSpectator( *match, GUCEF_MULTIPLAY_SPECTATOR_REASON_VOLUNTARY, CORE::CString::Empty, why ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, why.IsNULLOrEmpty() ? "The player cannot watch this match" : why.C_String() );
        return true;
    }
    Succeed( future, matchId, GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::MailDraftCancel( const CEMailIntroDraft& draft, const CEMailIntroContact& contact, const MULTIPLAY::CMatchProposal& cancel )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    if ( !CEMailMatchCodec::SerializeToBytes( cancel, bytes, m_settings.maxControlBytes ) )
        return false;
    CEMailOutgoing outgoing;
    outgoing.kind = MAILKIND_CANCEL;
    outgoing.matchId = draft.matchId;
    outgoing.revision = 0;
    outgoing.turn = 0;
    outgoing.title = draft.title;
    outgoing.from = COM::CEMailAddress( m_settings.displayName.Length() <= 64 ? CEMailMatchCodec::SanitizeTitle( m_settings.displayName ) : CORE::CString(), m_settings.mailAddress );
    outgoing.to.push_back( COM::CEMailAddress( contact.address ) );
    outgoing.actorName = draft.creatorName;
    outgoing.transfer = MAILTRANSFER_ATTACHMENT;
    outgoing.data = &bytes;
    outgoing.dataLabel = CEMailMatchCodec::ArmorLabelData;
    outgoing.dataRevisionName = "cancel";
    outgoing.note = "Nothing is asked of you any more. If you were asked to introduce yourself, the question is gone.";
    SMatch scratch;
    return QueueMail( scratch, outgoing, false );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::CancelDraft( const CORE::CString& matchId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CEMailIntroDraft draft;
    if ( !m_intros.FindDraft( matchId, draft ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_NOT_FOUND, "The match is not known" );
        return true;
    }
    MULTIPLAY::CMatchProposal cancel;
    cancel.matchId = matchId;
    cancel.kind = MULTIPLAY::MATCHDECISION_CANCEL;
    cancel.createdAt = GetNowMs();
    if ( !cancel.Sign( m_identity ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The decision could not be signed" );
        return true;
    }
    // what was not sent yet is not sent any more; then everybody who was asked is told
    m_outbox.RemoveMatch( matchId );
    for ( size_t i=0; i<draft.contacts.size(); ++i )
        if ( MAILINTRO_DECLINED != draft.contacts[ i ].state && COM::CEMailAddress( draft.contacts[ i ].address ).IsValid() )
            MailDraftCancel( draft, draft.contacts[ i ], cancel );
    m_intros.RemoveDraft( matchId );
    m_announcedDrafts.erase( matchId );
    ForgetBoard( matchId );
    ReportMatchRemoved( matchId );
    Succeed( future, matchId, GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::Cancel( const CORE::CString& matchId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
        return CancelDraft( matchId, future );
    if ( MULTIPLAY::MATCHSTATUS_CANCELLED == match->state.info.status )
    {
        Succeed( future, matchId, GUCEF_NULL );
        return true;
    }
    if ( MULTIPLAY::MATCHDECISIONPOLICY_BOSS != match->state.info.decisionPolicy )
    {
        // as in a lobby that is decided by vote: nobody closes it alone, the creator included; the players who accepted vote on a proposal (Propose)
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "This match is decided by vote: propose to cancel it and the players who accepted vote on it" );
        return true;
    }
    MULTIPLAY::CMatchProposal cancel;
    cancel.matchId = matchId;
    cancel.kind = MULTIPLAY::MATCHDECISION_CANCEL;
    cancel.createdAt = GetNowMs();
    CORE::CString reason;
    if ( !cancel.Sign( m_identity ) || !ApplyEvidenceOutsideTurn( *match, cancel, reason ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, reason.IsNULLOrEmpty() ? "The match cannot be cancelled" : reason.C_String() );
        return true;
    }
    // nothing waits for an answer any more; everybody who was invited, plays or watches is told
    m_outbox.RemoveUnconfirmed( matchId );
    MailControl( *match, MAILKIND_CANCEL, cancel, match->state.trustedRevision, "cancel", CORE::CString::Empty, CORE::CString::Empty, true, false, true );
    SaveMatch( *match );
    AnnounceMatch( *match );
    ReportLobbyDecision( *match, cancel );
    Succeed( future, matchId, GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::HandleCancel( const CORE::CString& matchId, const void* data, UInt32 size )
{GUCEF_TRACE;

    MULTIPLAY::CMatchProposal cancel;
    if ( !CEMailMatchCodec::DeserializeFromBytes( data, size, cancel, m_settings.maxControlBytes ) || cancel.matchId != matchId || MULTIPLAY::MATCHDECISION_CANCEL != cancel.kind )
    {
        MailProblem( matchId, 0, "BAD_CANCEL", "A cancellation is not valid" );
        return;
    }
    SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL != match )
    {
        CORE::CString reason;
        if ( MULTIPLAY::MATCHSTATUS_CANCELLED == match->state.info.status )
            return;                 // the same again
        if ( !ApplyEvidenceOutsideTurn( *match, cancel, reason ) )
        {
            if ( "known already" != reason )
                MailProblem( matchId, 0, "BAD_CANCEL", "A cancellation is not valid" );
            return;
        }
        m_outbox.RemoveUnconfirmed( matchId );
        SaveMatch( *match );
        AnnounceMatch( *match );
        ReportLobbyDecision( *match, cancel );
        return;
    }

    // no record here: the game may be an introduction that waits for an answer of the player
    switch ( m_intros.CancelInbound( matchId, cancel ) )
    {
        case MAILINTRORESULT_OK:
            IntroNotice( "INTRO_CANCELLED", matchId, "The creator gave the game up" );
            break;
        case MAILINTRORESULT_NO_REQUEST:
            KeepOrphan( matchId, (UInt8) MAILKIND_CANCEL, data, size );      // it came before the request or the invitation
            break;
        case MAILINTRORESULT_DUPLICATE:
            break;
        default:
            MailProblem( matchId, 0, "BAD_CANCEL", "A cancellation is not valid" );
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::HandleIncompatible( const CORE::CString& matchId, const void* data, UInt32 size )
{GUCEF_TRACE;

    MULTIPLAY::CMatchProposal statement;
    if ( !CEMailMatchCodec::DeserializeFromBytes( data, size, statement, m_settings.maxControlBytes ) || statement.matchId != matchId ||
         MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR != statement.kind || statement.proposerUserId != statement.targetUserId )
    {
        MailProblem( matchId, 0, "BAD_NOTICE", "A notice of a participant is not valid" );
        return;
    }
    SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
    {
        KeepOrphan( matchId, (UInt8) MAILKIND_INCOMPATIBLE, data, size );
        return;
    }
    if ( statement.proposerUserId == m_identity.userId )
        return;
    // at most a few notices per participant are ever kept: the first decides, the rest are the same again
    UInt32 fromSender = 0;
    for ( size_t i=0; i<match->state.proposals.size(); ++i )
        if ( MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR == match->state.proposals[ i ].kind && match->state.proposals[ i ].proposerUserId == statement.proposerUserId )
            ++fromSender;
    if ( fromSender >= 4 )
        return;
    CORE::CString reason;
    if ( !ApplyEvidenceOutsideTurn( *match, statement, reason ) )
    {
        if ( "known already" != reason )
            MailProblem( matchId, 0, "BAD_NOTICE", "A notice of a participant is not valid" );
        return;
    }
    SaveMatch( *match );
    AnnounceMatch( *match );
    ReportLobbyDecision( *match, statement );
}

/*-------------------------------------------------------------------------*/

/** Whether the package carries a passed force start that leaves nobody at the table who has not answered */
bool
CEMailMatchProvider::IsForceStartCovering( const SMatch& match, const MULTIPLAY::CMatchPackage& package ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_verifier )
        return false;
    MULTIPLAY::CIMatchPackageVerifier::TEvidenceVector evidence;
    m_verifier->GetDecisionEvidence( match.state.info, package, evidence );
    MULTIPLAY::CMatchInfo after = match.state.info;
    bool forced = false;
    for ( size_t e=0; e<evidence.size(); ++e )
    {
        CORE::CString why;
        if ( MULTIPLAY::CMatchDecisionVerifier::IsPreStartKind( evidence[ e ].proposal.kind ) && MULTIPLAY::CMatchDecisionVerifier::Verify( after, evidence[ e ], GetNowMs(), why, HasStarted( match ) ) &&
             MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( after, evidence[ e ].proposal, why ) )
            forced = forced || MULTIPLAY::MATCHDECISION_FORCE_START == evidence[ e ].proposal.kind;
    }
    bool anyInvited = false, anyOther = false;
    for ( size_t i=0; i<after.participants.size(); ++i )
    {
        anyInvited = anyInvited || MULTIPLAY::MATCHPARTICIPANT_INVITED == after.participants[ i ].status;
        anyOther = anyOther || ( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == after.participants[ i ].status && after.participants[ i ].userId != m_identity.userId );
    }
    // the table itself decided to start with the computer in the seats of those who did not answer: that is enough, even when the only other player watches
    return !anyInvited && ( anyOther || forced );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::Remove( const CORE::CString& matchId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
    {
        // a game that waits for introductions goes away with them: nothing is asked of anybody any more
        if ( m_intros.RemoveDraft( matchId ) )
        {
            m_outbox.RemoveMatch( matchId );
            m_announcedDrafts.erase( matchId );
            ForgetBoard( matchId );
            ReportMatchRemoved( matchId );
            Succeed( future, matchId, GUCEF_NULL );
            return true;
        }
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_NOT_FOUND, "The match is not known" );
        return true;
    }
    m_outbox.RemoveMatch( matchId );
    CORE::DeleteDir( match->directory, true );
    m_matches.erase( matchId );
    ForgetBoard( matchId );
    if ( m_removed.size() < g_maxRemoved )
        m_removed.insert( matchId );
    SaveRemoved();
    ReportMatchRemoved( matchId );
    Succeed( future, matchId, GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::Rematch( const CORE::CString& matchId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    SMatch* old = FindMatch( matchId );
    if ( GUCEF_NULL == old )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_NOT_FOUND, "The match is not known" );
        return true;
    }
    if ( MULTIPLAY::MATCHSTATUS_FINISHED != old->state.info.status && MULTIPLAY::MATCHSTATUS_ABANDONED != old->state.info.status )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "A match that is going on has no rematch" );
        return true;
    }
    // the same table and settings; the record is the player's, the others are invited again
    MULTIPLAY::CMatchInfo info = old->state.info;
    info.matchId = MULTIPLAY::CMatchId::Generate();
    info.creatorUserId = m_identity.userId;
    info.status = MULTIPLAY::MATCHSTATUS_FORMING;
    info.createdAt = GetNowMs();
    info.backend = ProviderName;
    for ( size_t i=0; i<info.participants.size(); ++i )
    {
        info.participants[ i ].status = info.participants[ i ].userId == m_identity.userId ? MULTIPLAY::MATCHPARTICIPANT_ACTIVE : MULTIPLAY::MATCHPARTICIPANT_INVITED;
        info.participants[ i ].outcome = MULTIPLAY::MATCHOUTCOME_NONE;
    }
    // a participant that the computer plays or that was dropped is a person again in the new match, with the contact of the record
    if ( old->state.hasRecord )
        for ( size_t i=0; i<info.participants.size(); ++i )
        {
            const MULTIPLAY::CMatchParticipant* original = old->state.record.info.FindParticipant( info.participants[ i ].userId );
            if ( GUCEF_NULL != original )
                info.participants[ i ].contact = original->contact;
        }
    MULTIPLAY::CMatchRecord record( info );
    CORE::CString reason;
    if ( !record.Sign( m_identity ) || !record.Verify( reason ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The record of the rematch could not be made" );
        return true;
    }
    SMatch* match = AddMatch( record );
    CORE::CDynamicBuffer recordBytes;
    if ( !CEMailMatchCodec::SerializeToBytes( record, recordBytes, m_settings.maxControlBytes ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The invitation could not be made" );
        return true;
    }
    for ( size_t i=0; i<match->state.info.participants.size(); ++i )
    {
        const MULTIPLAY::CMatchParticipant& participant = match->state.info.participants[ i ];
        COM::CEMailAddress address;
        if ( participant.userId == m_identity.userId || !GetContact( *match, participant.userId, address ) )
            continue;
        CEMailOutgoing outgoing;
        FillOutgoingBase( *match, MAILKIND_INVITE, outgoing );
        outgoing.to.push_back( address );
        outgoing.data = &recordBytes;
        outgoing.dataLabel = CEMailMatchCodec::ArmorLabelData;
        outgoing.dataRevisionName = "record";
        outgoing.rematchOf = matchId;
        QueueMail( *match, outgoing, true );
    }
    SaveMatch( *match );
    AnnounceMatch( *match );
    MULTIPLAY::CMatchPreview preview;
    MULTIPLAY::CMatchUpdate update;
    MakeUpdate( *match, update );
    Succeed( future, match->state.info.matchId, &update.preview );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::Propose( const CORE::CString& matchId, const MULTIPLAY::CMatchProposal& proposal, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_NOT_FOUND, "The match is not known" );
        return true;
    }
    const MULTIPLAY::CMatchParticipant* proposer = match->state.info.FindParticipant( proposal.proposerUserId );
    if ( proposal.matchId != matchId || proposal.proposerUserId != m_identity.userId || GUCEF_NULL == proposer || MULTIPLAY::MATCHPARTICIPANT_ACTIVE != proposer->status || !proposal.VerifySignature( match->state.info ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The proposal is not the signed proposal of the player" );
        return true;
    }
    // a participant's own statement has its own operation, so has a cancel that a boss decides alone (a cancel of a match decided by vote is a proposal like another);
    // a decision that cannot be valid is not sent to anybody
    const bool cancelByVote = MULTIPLAY::MATCHDECISION_CANCEL == proposal.kind && MULTIPLAY::MATCHDECISIONPOLICY_BOSS != match->state.info.decisionPolicy;
    if ( ( MULTIPLAY::MATCHDECISION_CANCEL == proposal.kind && !cancelByVote ) || ( MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR == proposal.kind && proposal.targetUserId == proposal.proposerUserId ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "This decision is made with its own operation" );
        return true;
    }
    {
        MULTIPLAY::CMatchDecisionEvidence candidate;
        candidate.proposal = proposal;
        MULTIPLAY::CMatchDecisionResult checked;
        MULTIPLAY::CMatchDecisionVerifier::Evaluate( match->state.info, candidate, GetNowMs(), checked, HasStarted( *match ) );
        if ( !checked.valid )
        {
            Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, checked.reason.IsNULLOrEmpty() ? "The proposal is not valid" : checked.reason.C_String() );
            return true;
        }
    }
    if ( !StoreProposal( *match, proposal ) )
    {
        Succeed( future, matchId, GUCEF_NULL );     // known already
        return true;
    }
    // players who have not answered are told as well when the proposal is to close the match: they are not asked to vote, they have to learn that it ended
    MailControl( *match, MAILKIND_PROPOSAL, proposal, match->state.trustedRevision, "proposal", CORE::CString::Empty, CORE::CString::Empty, cancelByVote, false, true );
    ApplyPassedCancels( *match );
    SaveMatch( *match );
    Succeed( future, matchId, GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::StoreProposal( SMatch& match, const MULTIPLAY::CMatchProposal& proposal )
{GUCEF_TRACE;

    for ( size_t i=0; i<match.state.proposals.size(); ++i )
        if ( match.state.proposals[ i ].proposalId == proposal.proposalId )
            return false;
    if ( match.state.proposals.size() >= 64 )
        match.state.proposals.erase( match.state.proposals.begin() );
    match.state.proposals.push_back( proposal );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::StoreVote( SMatch& match, const MULTIPLAY::CMatchVote& vote )
{GUCEF_TRACE;

    const MULTIPLAY::CMatchProposal* proposal = GUCEF_NULL;
    for ( size_t i=0; i<match.state.proposals.size(); ++i )
        if ( match.state.proposals[ i ].proposalId == vote.proposalId )
            proposal = &match.state.proposals[ i ];
    if ( GUCEF_NULL == proposal || proposal->GetDigest() != vote.proposalDigest )
        return false;
    for ( size_t i=0; i<match.state.votes.size(); ++i )
        if ( match.state.votes[ i ].proposalId == vote.proposalId && match.state.votes[ i ].voterUserId == vote.voterUserId )
            return false;
    if ( match.state.votes.size() >= 1024 )
        return false;
    match.state.votes.push_back( vote );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::Vote( const CORE::CString& matchId, const MULTIPLAY::CMatchVote& vote, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_started )
        return false;
    SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_NOT_FOUND, "The match is not known" );
        return true;
    }
    const MULTIPLAY::CMatchParticipant* voter = match->state.info.FindParticipant( vote.voterUserId );
    const MULTIPLAY::CMatchProposal* voted = GUCEF_NULL;
    for ( size_t i=0; i<match->state.proposals.size(); ++i )
        if ( match->state.proposals[ i ].proposalId == vote.proposalId )
            voted = &match->state.proposals[ i ];
    // the voters of a proposal that names its electorate are the ones it names, whatever their status is now
    const bool mayVote = GUCEF_NULL != voter && ( ( GUCEF_NULL != voted && !voted->voterUserIds.empty() ) ? voted->IsInElectorate( vote.voterUserId ) : voter->IsEligibleVoter() );
    if ( vote.matchId != matchId || vote.voterUserId != m_identity.userId || !mayVote || !vote.VerifySignature( match->state.info ) )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "The vote is not the signed vote of the player" );
        return true;
    }
    bool known = GUCEF_NULL != voted;
    if ( !known )
    {
        Reject( future, matchId, MULTIPLAY::MATCHOPERATION_REJECTED, "There is no such proposal" );
        return true;
    }
    if ( StoreVote( *match, vote ) )
    {
        MailControl( *match, MAILKIND_VOTE, vote, match->state.trustedRevision, "vote", CORE::CString::Empty, CORE::CString::Empty, MULTIPLAY::MATCHDECISION_CANCEL == voted->kind, false, true );
        ApplyPassedCancels( *match );
        SaveMatch( *match );
    }
    Succeed( future, matchId, GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::GetProposals( const CORE::CString& matchId, GUCEF::vector< MULTIPLAY::CMatchProposal >& proposals ) const
{GUCEF_TRACE;

    proposals.clear();
    const SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
        return false;
    proposals = match->state.proposals;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::GetDecisionEvidence( const CORE::CString& matchId, const CORE::CString& proposalId, MULTIPLAY::CMatchDecisionEvidence& evidence ) const
{GUCEF_TRACE;

    const SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
        return false;
    bool found = false;
    evidence = MULTIPLAY::CMatchDecisionEvidence();
    for ( size_t i=0; i<match->state.proposals.size(); ++i )
        if ( match->state.proposals[ i ].proposalId == proposalId )
        {
            evidence.proposal = match->state.proposals[ i ];
            found = true;
        }
    if ( !found )
        return false;
    for ( size_t i=0; i<match->state.votes.size(); ++i )
        if ( match->state.votes[ i ].proposalId == proposalId && evidence.votes.size() < MULTIPLAY::CMatchDecisionEvidence::MaxVotes )
            evidence.votes.push_back( match->state.votes[ i ] );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::GetPreStartEvidence( const CORE::CString& matchId, GUCEF::vector< MULTIPLAY::CMatchDecisionEvidence >& evidence ) const
{GUCEF_TRACE;

    evidence.clear();
    const SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match || HasStarted( *match ) || MULTIPLAY::MATCHSTATUS_CANCELLED == match->state.info.status )
        return false;
    const MULTIPLAY::CMatchInfo& info = match->state.info;
    for ( size_t i=0; i<match->state.proposals.size(); ++i )
    {
        const MULTIPLAY::CMatchProposal& proposal = match->state.proposals[ i ];
        if ( !MULTIPLAY::CMatchDecisionVerifier::IsPreStartKind( proposal.kind ) || MULTIPLAY::MATCHDECISION_CANCEL == proposal.kind )
            continue;
        MULTIPLAY::CMatchDecisionEvidence candidate;
        MULTIPLAY::CMatchDecisionResult tally;
        if ( !GetDecisionEvidence( matchId, proposal.proposalId, candidate ) || !MULTIPLAY::CMatchDecisionVerifier::Evaluate( info, candidate, GetNowMs(), tally, false ) ||
             !tally.valid || MULTIPLAY::MATCHVOTEOUTCOME_PASSED != tally.outcome )
            continue;
        // it still applies: a participant who answered since is not replaced, and one who was removed is not removed twice. Nobody who takes part
        // is left out of the electorate (a third party cannot check that; the creator, who carries the evidence, does): a proposal made before somebody
        // accepted is made again
        bool applies = true;
        for ( size_t p=0; p<info.participants.size() && applies; ++p )
            if ( info.participants[ p ].IsEligibleVoter() && !proposal.voterUserIds.empty() && !proposal.IsInElectorate( info.participants[ p ].userId ) )
                applies = false;
        if ( MULTIPLAY::MATCHDECISION_FORCE_START == proposal.kind )
        {
            for ( size_t t=0; t<proposal.targetUserIds.size(); ++t )
            {
                const MULTIPLAY::CMatchParticipant* target = info.FindParticipant( proposal.targetUserIds[ t ] );
                applies = applies && GUCEF_NULL != target && ( MULTIPLAY::MATCHPARTICIPANT_INVITED == target->status || MULTIPLAY::MATCHPARTICIPANT_DECLINED == target->status || MULTIPLAY::MATCHPARTICIPANT_AI_TAKEOVER == target->status );
            }
        }
        else
        {
            const MULTIPLAY::CMatchParticipant* target = info.FindParticipant( proposal.targetUserId );
            applies = applies && GUCEF_NULL != target && MULTIPLAY::MATCHPARTICIPANT_LEFT != target->status;
        }
        if ( applies )
            evidence.push_back( candidate );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::EvaluateDecision( const CORE::CString& matchId, const CORE::CString& proposalId, MULTIPLAY::CMatchDecisionResult& result ) const
{GUCEF_TRACE;

    const SMatch* match = FindMatch( matchId );
    MULTIPLAY::CMatchDecisionEvidence evidence;
    if ( GUCEF_NULL == match || !GetDecisionEvidence( matchId, proposalId, evidence ) )
        return false;
    return MULTIPLAY::CMatchDecisionVerifier::Evaluate( match->state.info, evidence, GetNowMs(), result, HasStarted( *match ) );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::Resend( const CORE::CString& matchId, UInt32 revision )
{GUCEF_TRACE;

    SMatch* match = FindMatch( matchId );
    if ( !m_started || GUCEF_NULL == match || revision < 1 || revision > match->state.trustedRevision )
        return false;
    // mails that are not confirmed yet are sent again; for a turn that was confirmed a new mail is made from the turn that is kept
    if ( m_outbox.ResendNow( matchId, revision, GetNowMs() ) > 0 )
        return true;
    MULTIPLAY::CMatchContainer container;
    CORE::CDynamicBuffer raw;
    if ( !CEMailFiles::ReadBounded( GetTurnPath( *match, revision ), MULTIPLAY::CMatchContainer::MaxContainerSize, raw ) || MULTIPLAY::MATCHCONTAINER_OK != container.Decode( raw.GetConstBufferPtr(), raw.GetDataSize() ) )
        return false;
    return MailTurn( *match, container.preview, raw, false, MAILTRANSFER_ATTACHMENT, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::RequestResend( const CORE::CString& matchId, UInt32 revision, bool wantText )
{GUCEF_TRACE;

    SMatch* match = FindMatch( matchId );
    if ( !m_started || GUCEF_NULL == match || revision < 1 )
        return false;
    GUCEF::vector< CORE::CString > audience;
    GetAudience( *match, false, audience );
    CEMailOutgoing outgoing;
    FillOutgoingBase( *match, MAILKIND_RESEND, outgoing );
    outgoing.revision = revision;
    outgoing.turn = revision;
    COM::CEMailAddress address;
    for ( size_t i=0; i<audience.size(); ++i )
        if ( GetContact( *match, audience[ i ], address ) )
            ( outgoing.to.empty() ? outgoing.to : outgoing.cc ).push_back( address );
    if ( outgoing.to.empty() )
        return false;
    if ( wantText )
        outgoing.wantTransfer = MailTransferToString( MAILTRANSFER_BODY_TEXT );
    match->state.resendRequested[ revision ] = GetNowMs();
    const bool queued = QueueMail( *match, outgoing, false );
    SaveMatch( *match );
    return queued;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::RequestMissing( SMatch& match, UInt32 fromRevision, UInt32 toRevision, bool wantText )
{GUCEF_TRACE;

    const Int64 now = GetNowMs();
    UInt32 asked = 0;
    for ( UInt32 revision=fromRevision; revision<=toRevision && asked < g_missingRevisionsPerRequest; ++revision )
    {
        GUCEF::map< UInt32, Int64 >::const_iterator last = match.state.resendRequested.find( revision );
        if ( last != match.state.resendRequested.end() && now < (*last).second + (Int64) m_settings.resendRequestIntervalMs )
            continue;
        RequestResend( match.state.info.matchId, revision, wantText );
        ++asked;
    }
}

/*-------------------------------------------------------------------------*/

TMailDeliveryState
CEMailMatchProvider::GetDeliveryState( const CORE::CString& matchId, UInt32 revision, UInt32* attempts ) const
{GUCEF_TRACE;

    const TMailDeliveryState state = m_outbox.GetState( matchId, revision, attempts );
    if ( MAILDELIVERY_NONE != state )
        return state;
    const SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL != match && match->state.deliveredRevisions.find( revision ) != match->state.deliveredRevisions.end() )
        return MAILDELIVERY_DELIVERED;
    return MAILDELIVERY_NONE;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::GetMatchInfo( const CORE::CString& matchId, MULTIPLAY::CMatchInfo& info ) const
{GUCEF_TRACE;

    const SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
    {
        CEMailIntroDraft draft;
        if ( !m_intros.FindDraft( matchId, draft ) )
            return false;
        MakeDraftInfo( draft, info );
        return true;
    }
    info = match->state.info;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CEMailMatchProvider::GetTrustedRevision( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    const SMatch* match = FindMatch( matchId );
    return GUCEF_NULL == match ? 0 : match->state.trustedRevision;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::GetStoredTurn( const CORE::CString& matchId, UInt32 revision, MULTIPLAY::CMatchPreview& preview, MULTIPLAY::CMatchPackage& package ) const
{GUCEF_TRACE;

    const SMatch* match = FindMatch( matchId );
    MULTIPLAY::CMatchContainer container;
    if ( GUCEF_NULL == match || !LoadTurn( *match, revision, container ) )
        return false;
    preview = container.preview;
    package = container.package;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CEMailMatchProvider::GetRefusals( const CORE::CString& matchId, TEMailRefusalVector& refusals ) const
{GUCEF_TRACE;

    refusals.clear();
    const SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL != match )
        refusals = match->state.refusals;
    return (UInt32) refusals.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CEMailMatchProvider::GetEquivocations( const CORE::CString& matchId, TEMailEquivocationVector& equivocations ) const
{GUCEF_TRACE;

    equivocations.clear();
    const SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL != match )
        equivocations = match->state.equivocations;
    return (UInt32) equivocations.size();
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::Refuse( SMatch& match, const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package, const void* container, UInt32 containerSize, const char* code, const CORE::CString& reason )
{GUCEF_TRACE;

    // a turn that is refused once is not announced again when it comes again
    for ( size_t i=0; i<match.state.refusals.size(); ++i )
        if ( match.state.refusals[ i ].sha256 == package.sha256 && match.state.refusals[ i ].code == code && match.state.refusals[ i ].revision == preview.revision )
            return;
    CEMailRefusal refusal;
    refusal.revision = preview.revision;
    refusal.sha256 = package.sha256;
    refusal.code = code;
    refusal.reason = reason;
    refusal.at = GetNowMs();
    if ( GUCEF_NULL != container && containerSize > 0 )
    {
        refusal.file = "refused-" + CORE::UInt32ToString( ++match.refusedCounter ) + ".rdxturn";
        if ( !CEMailFiles::WriteAtomic( CORE::CombinePath( match.directory, refusal.file ), container, containerSize ) )
            refusal.file.Clear();
    }
    match.state.refusals.push_back( refusal );
    while ( match.state.refusals.size() > g_maxRefusalsKept )
    {
        if ( !match.state.refusals.front().file.IsNULLOrEmpty() )
            CORE::DeleteFile( CORE::CombinePath( match.directory, match.state.refusals.front().file ) );
        match.state.refusals.erase( match.state.refusals.begin() );
    }
    SaveMatch( match );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProvider: A turn of revision " << preview.revision << " was not applied (" << code << ")" );
    Notice( PackageRefusedEvent, match.state.info.matchId, preview.revision, code, reason, preview.GetIssuerUserId() );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::NoteSignOfLife( SMatch& match, const CORE::CString& userId, UInt32 revision )
{GUCEF_TRACE;

    const MULTIPLAY::CMatchParticipant* participant = match.state.info.FindParticipant( userId );
    if ( GUCEF_NULL != participant && !participant->contact.IsNULLOrEmpty() )
        m_outbox.Acknowledge( match.state.info.matchId, participant->contact, revision, true );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::SendAck( SMatch& match, const MULTIPLAY::CMatchPreview& preview )
{GUCEF_TRACE;

    if ( !m_settings.sendAcknowledgements )
        return;
    const CORE::CString issuer = preview.GetIssuerUserId();
    COM::CEMailAddress address;
    if ( issuer == m_identity.userId || !GetContact( match, issuer, address ) )
        return;
    CEMailOutgoing outgoing;
    FillOutgoingBase( match, MAILKIND_ACK, outgoing );
    outgoing.revision = preview.revision;
    outgoing.turn = preview.turn;
    outgoing.to.push_back( address );
    outgoing.packageSha256 = preview.packageSha256;
    QueueMail( match, outgoing, false );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::IsTurnOfAbsentHolders( const SMatch& match, const MULTIPLAY::CMatchPreview& trusted, const MULTIPLAY::CMatchPreview& next, const MULTIPLAY::CMatchPackage& package ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_verifier || trusted.currentUserIds.empty() )
        return false;
    const MULTIPLAY::CMatchParticipant* signer = match.state.info.FindParticipantByKey( next.issuerKey );
    if ( GUCEF_NULL == signer || !signer->IsEligibleVoter() )
        return false;

    // every participant who is up is handed to the computer or dropped by a decision that passed and that the turn carries
    MULTIPLAY::CIMatchPackageVerifier::TEvidenceVector evidence;
    m_verifier->GetDecisionEvidence( match.state.info, package, evidence );
    const Int64 decisionTime = next.lastActivity > 0 ? next.lastActivity : GetNowMs();
    for ( CORE::CString::StringSet::const_iterator holder = trusted.currentUserIds.begin(); holder != trusted.currentUserIds.end(); ++holder )
    {
        bool covered = false;
        for ( size_t e=0; e<evidence.size() && !covered; ++e )
        {
            CORE::CString why;
            covered = ( MULTIPLAY::MATCHDECISION_HAND_TO_COMPUTER == evidence[ e ].proposal.kind || MULTIPLAY::MATCHDECISION_DROP_PARTICIPANT == evidence[ e ].proposal.kind ) &&
                      evidence[ e ].proposal.targetUserId == *holder && MULTIPLAY::CMatchDecisionVerifier::Verify( match.state.info, evidence[ e ], decisionTime, why );
        }
        if ( !covered )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::ApplyTurn( SMatch& match, const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package, bool isLocal, const CORE::CDynamicBuffer* knownContainer, const CORE::CString& fromAddress, CORE::CString& reason )
{GUCEF_TRACE;

    CORE::CDynamicBuffer container;
    if ( GUCEF_NULL != knownContainer )
        container = *knownContainer;
    else
    if ( MULTIPLAY::MATCHCONTAINER_OK != MULTIPLAY::CMatchContainer::Encode( preview, package, true, container ) )
    {
        reason = "The turn cannot be stored";
        return false;
    }

    const MULTIPLAY::CMatchPreview* trusted = match.hasTrusted ? &match.trustedPreview : GUCEF_NULL;
    const MULTIPLAY::CMatchPackage* trustedPackage = match.hasTrusted ? &match.trustedPackage : GUCEF_NULL;
    CORE::CString why;
    if ( MULTIPLAY::MATCHSTATUS_CANCELLED == match.state.info.status )
    {
        reason = "The match was cancelled";
        if ( !isLocal )
            Refuse( match, preview, package, container.GetConstBufferPtr(), container.GetDataSize(), "CANCELLED", reason );
        return false;
    }
    MULTIPLAY::TMatchChainResult chain = MULTIPLAY::CMatchChainVerifier::VerifySuccessor( match.state.info, trusted, preview, &package, why );
    if ( MULTIPLAY::MATCHCHAIN_NOT_ON_TURN == chain && GUCEF_NULL != trusted && IsTurnOfAbsentHolders( match, *trusted, preview, package ) )
    {
        // the participant who is up does not play: a decision of the table hands the participant to the computer, and the turn that applies it is the turn of any participant
        why.Clear();
        chain = MULTIPLAY::CMatchChainVerifier::VerifySuccessor( match.state.info, trusted, preview, &package, why, true );
    }
    if ( MULTIPLAY::MATCHCHAIN_OK != chain )
    {
        const char* code = "BROKEN_CHAIN";
        switch ( chain )
        {
            case MULTIPLAY::MATCHCHAIN_WRONG_MATCH:
            case MULTIPLAY::MATCHCHAIN_BAD_PREVIEW:     code = "BAD_PREVIEW"; break;
            case MULTIPLAY::MATCHCHAIN_UNKNOWN_SIGNER:  code = "UNKNOWN_SIGNER"; break;
            case MULTIPLAY::MATCHCHAIN_NOT_ON_TURN:     code = "NOT_ON_TURN"; break;
            case MULTIPLAY::MATCHCHAIN_OLD_REVISION:    code = "OLD_REVISION"; break;
            case MULTIPLAY::MATCHCHAIN_BAD_PACKAGE:     code = "BAD_PACKAGE"; break;
            default:                                    break;
        }
        reason = why;
        if ( !isLocal )
            Refuse( match, preview, package, container.GetConstBufferPtr(), container.GetDataSize(), code, why );
        return false;
    }

    // the game replays the turn
    MULTIPLAY::CIMatchPackageVerifier::TEvidenceVector evidence;
    if ( !isLocal )
    {
        if ( GUCEF_NULL == m_verifier )
        {
            if ( !m_allowUnverified )
            {
                reason = "There is no check of the game to judge the turn with";
                Refuse( match, preview, package, container.GetConstBufferPtr(), container.GetDataSize(), "NO_VERIFIER", reason );
                return false;
            }
        }
        else
        {
            const MULTIPLAY::TMatchPackageVerdict verdict = m_verifier->Verify( match.state.info, trustedPackage, package, why );
            if ( MULTIPLAY::MATCHPACKAGE_REJECTED == verdict )
            {
                reason = why;
                Refuse( match, preview, package, container.GetConstBufferPtr(), container.GetDataSize(), "GAME_REJECTED", why );
                // the first turn is signed by the creator and this client cannot play it (settings, seed, version): the player says so, signed, and watches
                if ( 1 == preview.revision )
                {
                    CORE::CString code = m_verifier->GetRefusalCode();
                    CORE::CString declared;
                    DeclareSpectator( match, MULTIPLAY::CMatchDecisionVerifier::IsValidReasonCode( code ) ? code : CORE::CString( "NOT_PLAYABLE" ), why, declared );
                }
                return false;
            }
            if ( MULTIPLAY::MATCHPACKAGE_NO_BASE == verdict )
            {
                // it cannot be judged yet: kept, and the package it follows is asked for
                reason = "The turn waits for the turn it follows";
                KeepPending( match, preview.revision, package.sha256, container );
                RequestMissing( match, preview.revision > 1 ? preview.revision - 1 : 1, preview.revision > 1 ? preview.revision - 1 : 1, false );
                return false;
            }
            m_verifier->GetDecisionEvidence( match.state.info, package, evidence );
        }
    }
    else
    if ( GUCEF_NULL != m_verifier )
        m_verifier->GetDecisionEvidence( match.state.info, package, evidence );

    // the decisions the turn carries out have to be good
    MULTIPLAY::CMatchInfo newInfo = match.state.info;
    GUCEF::vector< CORE::CString > digests;
    const Int64 decisionTime = preview.lastActivity > 0 ? preview.lastActivity : GetNowMs();
    GUCEF::vector< MULTIPLAY::CMatchProposal > lobbyDecisions;
    for ( size_t e=0; e<evidence.size(); ++e )
    {
        const CORE::CString digest = evidence[ e ].proposal.GetDigest();
        bool done = false;
        for ( size_t a=0; a<match.state.appliedDecisions.size(); ++a )
            done = done || match.state.appliedDecisions[ a ] == digest;
        if ( done )
            continue;
        if ( !MULTIPLAY::CMatchDecisionVerifier::Verify( match.state.info, evidence[ e ], decisionTime, why, HasStarted( match ) ) || !MULTIPLAY::CMatchDecisionVerifier::ApplyDecision( newInfo, evidence[ e ].proposal, why ) )
        {
            reason = why.IsNULLOrEmpty() ? CORE::CString( "A decision in the turn did not pass" ) : why;
            if ( !isLocal )
                Refuse( match, preview, package, container.GetConstBufferPtr(), container.GetDataSize(), "BAD_EVIDENCE", reason );
            return false;
        }
        digests.push_back( digest );
        if ( MULTIPLAY::CMatchDecisionVerifier::IsPreStartKind( evidence[ e ].proposal.kind ) )
            lobbyDecisions.push_back( evidence[ e ].proposal );
    }

    // applied
    if ( !SaveTurn( match, preview.revision, container.GetConstBufferPtr(), container.GetDataSize() ) )
    {
        reason = "The turn could not be stored";
        return false;
    }
    match.state.info = newInfo;
    for ( size_t d=0; d<digests.size(); ++d )
        match.state.appliedDecisions.push_back( digests[ d ] );
    match.trustedPreview = preview;
    match.trustedPackage = package;
    match.hasTrusted = true;
    match.state.trustedRevision = preview.revision;
    match.state.shaByRevision[ preview.revision ] = package.sha256;
    match.state.lastActivity = GetNowMs();

    // somebody who plays a turn has accepted
    const CORE::CString issuerUserId = preview.GetIssuerUserId();
    MULTIPLAY::CMatchParticipant* issuer = match.state.info.FindParticipant( issuerUserId );
    if ( GUCEF_NULL != issuer && MULTIPLAY::MATCHPARTICIPANT_INVITED == issuer->status )
        issuer->status = MULTIPLAY::MATCHPARTICIPANT_ACTIVE;
    UpdateStatus( match );
    SaveMatch( match );
    AnnounceMatch( match );
    for ( size_t d=0; d<lobbyDecisions.size(); ++d )
        ReportLobbyDecision( match, lobbyDecisions[ d ] );
    if ( !isLocal && !issuerUserId.IsNULLOrEmpty() )
        NoteSignOfLife( match, issuerUserId, preview.revision );
    if ( !isLocal )
        SendAck( match, preview );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::KeepPending( SMatch& match, UInt32 revision, const CORE::CString& sha256, const CORE::CDynamicBuffer& container )
{GUCEF_TRACE;

    CORE::CString::StringVector names;
    CEMailFiles::ListFiles( match.directory, names );
    UInt32 pending = 0;
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ].StartsWith( "pending-" ) )
            ++pending;
    const CORE::CString name = "pending-" + CORE::UInt32ToString( revision ) + "-" + sha256.SubstrToIndex( 8, true ) + ".rdxturn";
    if ( CORE::FileExists( CORE::CombinePath( match.directory, name ) ) )
        return;
    if ( pending >= m_settings.maxPendingPackages )
    {
        MailProblem( match.state.info.matchId, revision, "PENDING_FULL", "Too many turns wait for an earlier one; this one is dropped and can be asked for again" );
        return;
    }
    CEMailFiles::WriteAtomic( CORE::CombinePath( match.directory, name ), container.GetConstBufferPtr(), container.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::TryApplyPending( SMatch& match )
{GUCEF_TRACE;

    // a turn that waited is tried when the one before it is there; each file once per call
    for ( int round=0; round<64; ++round )
    {
        CORE::CString::StringVector names;
        CEMailFiles::ListFiles( match.directory, names );
        const CORE::CString prefix = "pending-" + CORE::UInt32ToString( match.state.trustedRevision + 1 ) + "-";
        CORE::CString found;
        for ( size_t i=0; i<names.size(); ++i )
            if ( names[ i ].StartsWith( prefix ) )
            {
                found = names[ i ];
                break;
            }
        if ( found.IsNULLOrEmpty() )
            return;
        const CORE::CString path = CORE::CombinePath( match.directory, found );
        CORE::CDynamicBuffer raw;
        MULTIPLAY::CMatchContainer container;
        const UInt32 before = match.state.trustedRevision;
        if ( CEMailFiles::ReadBounded( path, MULTIPLAY::CMatchContainer::MaxContainerSize, raw ) && MULTIPLAY::MATCHCONTAINER_OK == container.Decode( raw.GetConstBufferPtr(), raw.GetDataSize() ) )
        {
            CORE::DeleteFile( path );
            CORE::CString reason;
            ApplyIncoming( match, container, raw, CORE::CString::Empty, false, reason );
        }
        else
            CORE::DeleteFile( path );
        if ( match.state.trustedRevision == before )
            return;
    }
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::ApplyIncoming( SMatch& match, const MULTIPLAY::CMatchContainer& container, const CORE::CDynamicBuffer& raw, const CORE::CString& fromAddress, bool fromPendingCheck, CORE::CString& reason )
{GUCEF_TRACE;

    const MULTIPLAY::CMatchPreview& preview = container.preview;
    const MULTIPLAY::CMatchPackage& package = container.package;
    const UInt32 revision = preview.revision;
    const UInt32 trusted = match.state.trustedRevision;

    if ( revision >= 1 && revision <= trusted )
    {
        // a turn that is held already, or another one for the same revision
        GUCEF::map< UInt32, CORE::CString >::const_iterator known = match.state.shaByRevision.find( revision );
        if ( known != match.state.shaByRevision.end() && (*known).second == package.sha256 )
        {
            // the sender may not have heard that it arrived
            const MULTIPLAY::CMatchParticipant* signer = match.state.info.FindParticipantByKey( preview.issuerKey );
            if ( GUCEF_NULL != signer && preview.VerifySignature() )
            {
                NoteSignOfLife( match, signer->userId, revision );
                SendAck( match, preview );
            }
            if ( !fromPendingCheck )
                TryApplyPending( match );
            return true;
        }
        const MULTIPLAY::CMatchParticipant* signer = match.state.info.FindParticipantByKey( preview.issuerKey );
        MULTIPLAY::CMatchContainer held;
        if ( GUCEF_NULL != signer && LoadTurn( match, revision, held ) && MULTIPLAY::CMatchChainVerifier::IsEquivocation( held.preview, preview ) )
        {
            CEMailEquivocation proof;
            proof.revision = revision;
            proof.issuerKey = preview.issuerKey;
            proof.issuerUserId = signer->userId;
            proof.firstSha256 = held.package.sha256;
            proof.secondSha256 = package.sha256;
            bool seen = false;
            for ( size_t i=0; i<match.state.equivocations.size(); ++i )
                seen = seen || ( match.state.equivocations[ i ].revision == revision && match.state.equivocations[ i ].secondSha256 == proof.secondSha256 );
            if ( !seen )
            {
                proof.file = "refused-" + CORE::UInt32ToString( ++match.refusedCounter ) + ".rdxturn";
                CEMailFiles::WriteAtomic( CORE::CombinePath( match.directory, proof.file ), raw.GetConstBufferPtr(), raw.GetDataSize() );
                match.state.equivocations.push_back( proof );
                reason = "The same participant signed two different turns for one revision";
                SaveMatch( match );
                Notice( EquivocationDetectedEvent, match.state.info.matchId, revision, "EQUIVOCATION", reason, signer->userId );
                Refuse( match, preview, package, GUCEF_NULL, 0, "EQUIVOCATION", reason );
            }
            return false;
        }
        reason = "The revision is older than the newest one that is held";
        Refuse( match, preview, package, raw.GetConstBufferPtr(), raw.GetDataSize(), "OLD_REVISION", reason );
        return false;
    }

    if ( revision == trusted + 1 )
    {
        const bool applied = ApplyTurn( match, preview, package, false, &raw, fromAddress, reason );
        if ( applied )
            TryApplyPending( match );
        return applied;
    }

    // a participant who watches because the game cannot be played here cannot follow the turns and does not ask for the ones before
    const MULTIPLAY::CMatchParticipant* local = match.state.info.FindParticipant( m_identity.userId );
    if ( GUCEF_NULL != local && local->IsIncompatible() )
    {
        reason = "The player watches and cannot follow this game";
        return false;
    }

    // ahead of the chain: it waits, and the turns before it are asked for
    reason = "The turn waits for the turns before it";
    KeepPending( match, revision, package.sha256, raw );
    RequestMissing( match, trusted + 1, revision - 1, false );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::KeepOrphan( const CORE::CString& matchId, UInt8 kind, const void* data, UInt32 size )
{GUCEF_TRACE;

    const CORE::CString directory = CORE::CombinePath( m_settings.stateDirectory, "orphans" );
    CORE::CString::StringVector names;
    CEMailFiles::ListFiles( directory, names );
    if ( names.size() >= g_maxOrphans || size > MULTIPLAY::CMatchContainer::MaxContainerSize )
        return;
    const CORE::CString name = matchId + "-" + MULTIPLAY::CMatchSigning::Sha256Hex( data, size ).SubstrToIndex( 8, true ) + ".orphan";
    CORE::CDynamicBuffer bytes;
    bytes.Append( &kind, 1 );
    bytes.Append( data, size );
    CEMailFiles::WriteAtomic( CORE::CombinePath( directory, name ), bytes.GetConstBufferPtr(), bytes.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::ProcessOrphans( const CORE::CString& matchId )
{GUCEF_TRACE;

    const CORE::CString directory = CORE::CombinePath( m_settings.stateDirectory, "orphans" );
    CORE::CString::StringVector names;
    CEMailFiles::ListFiles( directory, names );
    GUCEF::vector< CORE::CString > mine;
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ].StartsWith( matchId + "-" ) )
            mine.push_back( names[ i ] );
    for ( size_t i=0; i<mine.size(); ++i )
    {
        CORE::CDynamicBuffer bytes;
        const CORE::CString path = CORE::CombinePath( directory, mine[ i ] );
        if ( CEMailFiles::ReadBounded( path, MULTIPLAY::CMatchContainer::MaxContainerSize + 1, bytes ) && bytes.GetDataSize() > 1 )
        {
            const UInt8 kind = *static_cast< const UInt8* >( bytes.GetConstBufferPtr() );
            const UInt8* body = static_cast< const UInt8* >( bytes.GetConstBufferPtr() ) + 1;
            CORE::DeleteFile( path );
            CORE::CString reason;
            if ( MAILKIND_TURN == kind )
                ProcessContainer( body, bytes.GetDataSize() - 1, matchId, CORE::CString::Empty, reason );
            else
                HandleBlob( (TMailMatchKind) kind, matchId, body, bytes.GetDataSize() - 1 );
        }
        else
            CORE::DeleteFile( path );
    }
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::ProcessContainer( const void* data, UInt32 size, const CORE::CString& expectedMatchId, const CORE::CString& fromAddress, CORE::CString& reason )
{GUCEF_TRACE;

    MULTIPLAY::CMatchContainer container;
    const MULTIPLAY::TMatchContainerResult decoded = container.Decode( data, size );
    if ( MULTIPLAY::MATCHCONTAINER_OK != decoded )
    {
        reason = CORE::CString( "The turn cannot be read (" ) + MULTIPLAY::MatchContainerResultToString( decoded ) + ")";
        Notice( PackageRefusedEvent, expectedMatchId, 0, "BAD_CONTAINER", reason, CORE::CString::Empty );
        return false;
    }
    const CORE::CString matchId = container.preview.matchId;
    if ( !expectedMatchId.IsNULLOrEmpty() && expectedMatchId != matchId )
    {
        reason = "The mail and the turn in it are about different matches";
        Notice( PackageRefusedEvent, expectedMatchId, container.preview.revision, "BAD_CONTAINER", reason, CORE::CString::Empty );
        return false;
    }
    if ( IsRemoved( matchId ) )
    {
        reason = "The match was removed here";
        return false;
    }
    SMatch* match = FindMatch( matchId );
    if ( GUCEF_NULL == match )
    {
        KeepOrphan( matchId, (UInt8) MAILKIND_TURN, data, size );
        reason = "The match is not known here yet; the turn is kept until it is";
        return false;
    }
    CORE::CDynamicBuffer raw;
    raw.Append( data, size );
    return ApplyIncoming( *match, container, raw, fromAddress, false, reason );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::OnInboxMessage( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    HandleMail( *static_cast< const CEMailInboxMessage* >( eventData ) );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::OnInboxRefused( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CEMailInboxRefusal* refusal = static_cast< const CEMailInboxRefusal* >( eventData );
    MailProblem( refusal->matchId, refusal->revision, "TOO_LARGE", refusal->reason );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::OnInboxState( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const MULTIPLAY::CMatchProviderState* state = static_cast< const MULTIPLAY::CMatchProviderState* >( eventData );
    ReportProviderState( state->online, state->description );
    if ( !state->online )
        ReportProviderError( state->description );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::OnInboxNotice( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CEMailInboxNotice* notice = static_cast< const CEMailInboxNotice* >( eventData );
    MailProblem( CORE::CString::Empty, 0, notice->code.C_String(), notice->description );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::OnOutboxDelivery( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CEMailDeliveryEvent* delivery = static_cast< const CEMailDeliveryEvent* >( eventData );
    if ( MAILDELIVERY_DELIVERED == delivery->state && ( MAILKIND_TURN == delivery->kind || MAILKIND_FINISH == delivery->kind ) )
    {
        SMatch* match = FindMatch( delivery->matchId );
        if ( GUCEF_NULL != match )
        {
            match->state.deliveredRevisions.insert( delivery->revision );
            SaveMatch( *match );
        }
    }
    CEMailDeliveryEvent copy( *delivery );
    NotifyObservers( DeliveryStateChangedEvent, &copy );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::ReadData( const CEMailInboxMessage& mail, const CORE::CString& label, UInt32 maxBytes, CORE::CDynamicBuffer& data, TMailTransfer& used )
{GUCEF_TRACE;

    const TMailPayloadResult result = CEMailMatchCodec::ExtractData( mail.message, mail.incoming.matchId, label == CEMailMatchCodec::ArmorLabelTurn ? mail.incoming.revision : 0, label, maxBytes, m_settings.maxBodyTextBytes, data, used );
    if ( MAILPAYLOAD_OK == result )
        return true;
    const SMatch* match = FindMatch( mail.incoming.matchId );
    MailProblem( mail.incoming.matchId, mail.incoming.revision, MailPayloadResultToString( result ), "A mail of the match carries no usable data" );
    if ( GUCEF_NULL != match && label == CEMailMatchCodec::ArmorLabelTurn && ( MAILPAYLOAD_NONE == result || MAILPAYLOAD_DAMAGED == result || MAILPAYLOAD_BAD_ENCODING == result ) && mail.incoming.revision > match->state.trustedRevision )
    {
        // the carrier took the data away or damaged the text: the text form is asked for, once in a while
        SMatch* writable = const_cast< SMatch* >( match );
        RequestMissing( *writable, mail.incoming.revision, mail.incoming.revision, true );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::HandleMail( const CEMailInboxMessage& mail )
{GUCEF_TRACE;

    const CEMailIncoming& incoming = mail.incoming;
    if ( IsRemoved( incoming.matchId ) )
        return;
    SMatch* match = FindMatch( incoming.matchId );
    const bool substantive = MAILKIND_TURN == incoming.kind || MAILKIND_FINISH == incoming.kind || MAILKIND_INVITE == incoming.kind || MAILKIND_ACCEPT == incoming.kind || MAILKIND_DECLINE == incoming.kind;
    if ( GUCEF_NULL != match && substantive )
    {
        NoteThread( *match, incoming.messageId );
        SaveMatch( *match );
    }

    switch ( incoming.kind )
    {
        case MAILKIND_INVITE:   HandleInvite( mail ); break;
        case MAILKIND_TURN:
        case MAILKIND_FINISH:   HandleTurnMail( mail ); break;
        case MAILKIND_ACK:      HandleAck( mail ); break;
        case MAILKIND_RESEND:   HandleResendRequest( mail ); break;
        case MAILKIND_INTRO_REQUEST:    HandleIntroRequest( mail ); break;
        case MAILKIND_INTRO_REPLY:      HandleIntroReply( mail ); break;
        case MAILKIND_CANCEL:
        case MAILKIND_INCOMPATIBLE:
        {
            CORE::CDynamicBuffer data;
            TMailTransfer used;
            if ( ReadData( mail, CEMailMatchCodec::ArmorLabelData, m_settings.maxControlBytes, data, used ) )
                HandleBlob( incoming.kind, incoming.matchId, data.GetConstBufferPtr(), data.GetDataSize() );
            break;
        }
        case MAILKIND_ROSTER:
        case MAILKIND_DRAFT_PROPOSAL:
        case MAILKIND_DRAFT_VOTE:
        {
            CORE::CDynamicBuffer data;
            TMailTransfer used;
            if ( !ReadData( mail, CEMailMatchCodec::ArmorLabelData, m_settings.maxControlBytes, data, used ) )
                break;
            if ( MAILKIND_ROSTER == incoming.kind )
                HandleRoster( incoming.matchId, data.GetConstBufferPtr(), data.GetDataSize(), incoming.fromAddress );
            else
            if ( MAILKIND_DRAFT_PROPOSAL == incoming.kind )
                HandleDraftProposal( incoming.matchId, data.GetConstBufferPtr(), data.GetDataSize(), incoming.fromAddress );
            else
                HandleDraftVote( incoming.matchId, data.GetConstBufferPtr(), data.GetDataSize(), incoming.fromAddress );
            break;
        }
        case MAILKIND_ACCEPT:
        case MAILKIND_DECLINE:
        case MAILKIND_NUDGE:
        case MAILKIND_RESIGN:
        case MAILKIND_PROPOSAL:
        case MAILKIND_VOTE:
        {
            CORE::CDynamicBuffer data;
            TMailTransfer used;
            if ( ReadData( mail, CEMailMatchCodec::ArmorLabelData, m_settings.maxControlBytes, data, used ) )
                HandleBlob( incoming.kind, incoming.matchId, data.GetConstBufferPtr(), data.GetDataSize() );
            break;
        }
        default:
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::HandleInvite( const CEMailInboxMessage& mail )
{GUCEF_TRACE;

    CORE::CDynamicBuffer recordBytes;
    TMailTransfer used;
    if ( !ReadData( mail, CEMailMatchCodec::ArmorLabelData, m_settings.maxControlBytes, recordBytes, used ) )
        return;
    MULTIPLAY::CMatchRecord record;
    CORE::CString reason;
    if ( !CEMailMatchCodec::DeserializeFromBytes( recordBytes.GetConstBufferPtr(), recordBytes.GetDataSize(), record, m_settings.maxControlBytes ) || !record.Verify( reason ) || record.info.matchId != mail.incoming.matchId )
    {
        MailProblem( mail.incoming.matchId, 0, "BAD_RECORD", "An invitation does not have a valid signed record" );
        return;
    }
    const MULTIPLAY::CMatchParticipant* me = record.info.FindParticipant( m_identity.userId );
    if ( GUCEF_NULL == me || me->publicKey.Lowercase() != m_identity.publicKey.Lowercase() )
    {
        MailProblem( mail.incoming.matchId, 0, "NOT_FOR_ME", "An invitation is for somebody else" );
        return;
    }
    SMatch* match = FindMatch( record.info.matchId );
    if ( GUCEF_NULL == match )
    {
        match = AddMatch( record );
        NoteThread( *match, mail.incoming.messageId );
        ImportDraftStatements( *match );
        CheckRecordAgainstRoster( *match );
        SaveMatch( *match );
    }
    else
    if ( !match->state.hasRecord || match->state.record.info.GetDigest() != record.info.GetDigest() )
        return;

    // the first turn that comes with the invitation
    CORE::CDynamicBuffer container;
    if ( ReadFirstTurn( mail, container ) )
        ProcessContainer( container.GetConstBufferPtr(), container.GetDataSize(), record.info.matchId, mail.incoming.fromAddress, reason );
    match = FindMatch( record.info.matchId );
    if ( GUCEF_NULL != match )
    {
        AnnounceMatch( *match );
        TryJoinIntroduced( *match, mail );
    }
    ProcessOrphans( record.info.matchId );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::ReadFirstTurn( const CEMailInboxMessage& mail, CORE::CDynamicBuffer& container )
{GUCEF_TRACE;

    TMailTransfer used;
    return MAILPAYLOAD_OK == CEMailMatchCodec::ExtractData( mail.message, mail.incoming.matchId, 1, CEMailMatchCodec::ArmorLabelTurn, m_settings.maxContainerBytes, m_settings.maxBodyTextBytes, container, used );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::HandleTurnMail( const CEMailInboxMessage& mail )
{GUCEF_TRACE;

    CORE::CDynamicBuffer container;
    TMailTransfer used;
    if ( !ReadData( mail, CEMailMatchCodec::ArmorLabelTurn, m_settings.maxContainerBytes, container, used ) )
        return;
    CORE::CString reason;
    ProcessContainer( container.GetConstBufferPtr(), container.GetDataSize(), mail.incoming.matchId, mail.incoming.fromAddress, reason );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::HandleAck( const CEMailInboxMessage& mail )
{GUCEF_TRACE;

    SMatch* match = FindMatch( mail.incoming.matchId );
    if ( GUCEF_NULL == match )
        return;
    CORE::CString userId;
    if ( !FindParticipantByAddress( *match, mail.incoming.fromAddress, userId ) || userId == m_identity.userId )
        return;
    GUCEF::map< UInt32, CORE::CString >::const_iterator known = match->state.shaByRevision.find( mail.incoming.revision );
    if ( known != match->state.shaByRevision.end() && !mail.incoming.packageSha256.IsNULLOrEmpty() && (*known).second != mail.incoming.packageSha256 )
        return;
    m_outbox.Acknowledge( mail.incoming.matchId, mail.incoming.fromAddress, mail.incoming.revision, false );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::HandleResendRequest( const CEMailInboxMessage& mail )
{GUCEF_TRACE;

    SMatch* match = FindMatch( mail.incoming.matchId );
    if ( GUCEF_NULL == match )
        return;
    CORE::CString userId;
    if ( !FindParticipantByAddress( *match, mail.incoming.fromAddress, userId ) || userId == m_identity.userId )
        return;
    const UInt32 revision = mail.incoming.revision;
    if ( match->state.shaByRevision.find( revision ) == match->state.shaByRevision.end() )
        return;
    const Int64 now = GetNowMs();
    const CORE::CString key = CORE::UInt32ToString( revision ) + ":" + mail.incoming.fromAddress;
    GUCEF::map< CORE::CString, Int64 >::const_iterator last = match->state.resendAnswered.find( key );
    if ( last != match->state.resendAnswered.end() && now < (*last).second + (Int64) m_settings.resendRequestIntervalMs )
        return;
    CORE::CDynamicBuffer raw;
    MULTIPLAY::CMatchContainer container;
    if ( !CEMailFiles::ReadBounded( GetTurnPath( *match, revision ), MULTIPLAY::CMatchContainer::MaxContainerSize, raw ) || MULTIPLAY::MATCHCONTAINER_OK != container.Decode( raw.GetConstBufferPtr(), raw.GetDataSize() ) )
        return;
    // the original, signed by whoever played it: it is sent as it is
    TMailTransfer transfer = MAILTRANSFER_ATTACHMENT;
    const bool wanted = !mail.incoming.wantTransfer.IsNULLOrEmpty() && TryStringToMailTransfer( mail.incoming.wantTransfer, transfer );
    match->state.resendAnswered[ key ] = now;
    const MULTIPLAY::CMatchParticipant* requester = match->state.info.FindParticipant( userId );
    MailTurn( *match, container.preview, raw, wanted, transfer, requester->contact );
    SaveMatch( *match );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::HandleBlob( TMailMatchKind kind, const CORE::CString& matchId, const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( MAILKIND_CANCEL == kind )
    {
        HandleCancel( matchId, data, size );
        return;
    }
    if ( MAILKIND_INCOMPATIBLE == kind )
    {
        HandleIncompatible( matchId, data, size );
        return;
    }
    if ( MAILKIND_ROSTER == kind || MAILKIND_DRAFT_PROPOSAL == kind || MAILKIND_DRAFT_VOTE == kind )
    {
        if ( MAILKIND_ROSTER == kind )
            HandleRoster( matchId, data, size, CORE::CString::Empty );
        else
        if ( MAILKIND_DRAFT_PROPOSAL == kind )
            HandleDraftProposal( matchId, data, size, CORE::CString::Empty );
        else
            HandleDraftVote( matchId, data, size, CORE::CString::Empty );
        return;
    }
    SMatch* match = FindMatch( matchId );
    CORE::CString reason;
    CORE::CString userId;
    const Int64 now = GetNowMs();

    if ( MAILKIND_ACCEPT == kind || MAILKIND_DECLINE == kind )
    {
        if ( GUCEF_NULL == match )
        {
            KeepOrphan( matchId, (UInt8) kind, data, size );
            return;
        }
        MULTIPLAY::CMatchAcceptance acceptance;
        if ( !CEMailMatchCodec::DeserializeFromBytes( data, size, acceptance, m_settings.maxControlBytes ) || acceptance.matchId != matchId || !match->state.hasRecord || !acceptance.Verify( match->state.record.info, reason ) )
        {
            MailProblem( matchId, 0, "BAD_ACCEPTANCE", "An answer to an invitation is not valid" );
            return;
        }
        MULTIPLAY::CMatchParticipant* participant = match->state.info.FindParticipant( acceptance.userId );
        const MULTIPLAY::TMatchParticipantStatus wanted = acceptance.accepted ? MULTIPLAY::MATCHPARTICIPANT_ACTIVE : MULTIPLAY::MATCHPARTICIPANT_DECLINED;
        if ( GUCEF_NULL == participant || participant->userId == match->state.info.creatorUserId )
            return;
        // the first answer is the answer
        if ( MULTIPLAY::MATCHPARTICIPANT_INVITED == participant->status )
        {
            participant->status = wanted;
            match->state.acceptances.push_back( acceptance );
            UpdateStatus( *match );
            match->state.lastActivity = now;
            SaveMatch( *match );
            AnnounceMatch( *match );
        }
        NoteSignOfLife( *match, acceptance.userId, 0 );
        return;
    }
    if ( GUCEF_NULL == match )
        return;

    if ( MAILKIND_NUDGE == kind || MAILKIND_RESIGN == kind )
    {
        CEMailSignedStatement statement;
        const char* wantedKind = MAILKIND_NUDGE == kind ? CEMailSignedStatement::KindNudge : CEMailSignedStatement::KindResign;
        if ( !CEMailMatchCodec::DeserializeFromBytes( data, size, statement, m_settings.maxControlBytes ) || statement.statementKind != wantedKind || !statement.Verify( match->state.info, reason ) )
        {
            MailProblem( matchId, 0, "BAD_STATEMENT", "A statement of a participant is not valid" );
            return;
        }
        if ( statement.userId == m_identity.userId )
            return;
        if ( MAILKIND_NUDGE == kind )
        {
            if ( statement.targetUserId != m_identity.userId )
                return;
            GUCEF::map< CORE::CString, Int64 >::const_iterator last = match->state.nudgesReceived.find( statement.userId );
            if ( last != match->state.nudgesReceived.end() && now < (*last).second + (Int64) m_settings.nudgeIntervalMs )
                return;
            match->state.nudgesReceived[ statement.userId ] = now;
            SaveMatch( *match );
            Notice( NudgeReceivedEvent, matchId, statement.revision, "NUDGE", "A participant reminds the player of the turn", statement.userId );
            return;
        }
        MULTIPLAY::CMatchParticipant* participant = match->state.info.FindParticipant( statement.userId );
        if ( GUCEF_NULL != participant && ( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == participant->status || MULTIPLAY::MATCHPARTICIPANT_INVITED == participant->status || MULTIPLAY::MATCHPARTICIPANT_SPECTATOR == participant->status ) )
        {
            participant->status = MULTIPLAY::MATCHPARTICIPANT_LEFT;
            participant->outcome = MULTIPLAY::MATCHOUTCOME_QUIT;
            match->state.resigns.push_back( statement );
            match->state.lastActivity = now;
            SaveMatch( *match );
            AnnounceMatch( *match );
        }
        return;
    }

    if ( MAILKIND_PROPOSAL == kind )
    {
        MULTIPLAY::CMatchProposal proposal;
        if ( !CEMailMatchCodec::DeserializeFromBytes( data, size, proposal, m_settings.maxControlBytes ) || proposal.matchId != matchId || !proposal.VerifySignature( match->state.info ) )
        {
            MailProblem( matchId, 0, "BAD_PROPOSAL", "A proposal is not valid" );
            return;
        }
        const MULTIPLAY::CMatchParticipant* proposer = match->state.info.FindParticipant( proposal.proposerUserId );
        if ( GUCEF_NULL == proposer || proposal.proposerUserId == m_identity.userId || ( proposal.proposerUserId == proposal.targetUserId && MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR == proposal.kind ) )
            return;
        // a proposal that names its electorate is made by one of its voters, one without by a participant who votes now
        if ( proposal.voterUserIds.empty() ? MULTIPLAY::MATCHPARTICIPANT_ACTIVE != proposer->status : !proposal.IsInElectorate( proposal.proposerUserId ) )
            return;
        if ( MULTIPLAY::MATCHDECISION_CANCEL == proposal.kind && MULTIPLAY::MATCHDECISIONPOLICY_BOSS == match->state.info.decisionPolicy )
            return;         // a cancel that a boss decides alone comes as a cancel
        {
            MULTIPLAY::CMatchDecisionEvidence candidate;
            candidate.proposal = proposal;
            MULTIPLAY::CMatchDecisionResult checked;
            MULTIPLAY::CMatchDecisionVerifier::Evaluate( match->state.info, candidate, now, checked, HasStarted( *match ) );
            if ( !checked.valid )
            {
                MailProblem( matchId, 0, "BAD_PROPOSAL", "A proposal is not valid" );
                return;
            }
        }
        if ( StoreProposal( *match, proposal ) )
        {
            ApplyPassedCancels( *match );
            SaveMatch( *match );
            Notice( DecisionMessageEvent, matchId, 0, "PROPOSAL", "A participant proposes a decision", proposal.proposerUserId, proposal.proposalId );
        }
        return;
    }
    if ( MAILKIND_VOTE == kind )
    {
        MULTIPLAY::CMatchVote vote;
        if ( !CEMailMatchCodec::DeserializeFromBytes( data, size, vote, m_settings.maxControlBytes ) || vote.matchId != matchId || !vote.VerifySignature( match->state.info ) )
        {
            MailProblem( matchId, 0, "BAD_VOTE", "A vote is not valid" );
            return;
        }
        const MULTIPLAY::CMatchParticipant* voter = match->state.info.FindParticipant( vote.voterUserId );
        const MULTIPLAY::CMatchProposal* voted = GUCEF_NULL;
        for ( size_t p=0; p<match->state.proposals.size(); ++p )
            if ( match->state.proposals[ p ].proposalId == vote.proposalId )
                voted = &match->state.proposals[ p ];
        if ( GUCEF_NULL == voter || vote.voterUserId == m_identity.userId )
            return;
        if ( ( GUCEF_NULL != voted && !voted->voterUserIds.empty() ) ? !voted->IsInElectorate( vote.voterUserId ) : !voter->IsEligibleVoter() )
            return;
        if ( StoreVote( *match, vote ) )
        {
            ApplyPassedCancels( *match );
            SaveMatch( *match );
            Notice( DecisionMessageEvent, matchId, 0, "VOTE", "A participant voted on a proposal", vote.voterUserId, vote.proposalId );
        }
        return;
    }
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      INTRODUCTION BY ADDRESS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CEMailMatchProvider::GetOwnName( void ) const
{GUCEF_TRACE;

    return m_settings.displayName.IsNULLOrEmpty() ? m_identity.displayName : m_settings.displayName;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::ConfigureIntroductions( void )
{GUCEF_TRACE;

    CEMailIntroLimits limits;
    limits.expiryMs = m_settings.introExpiryMs;
    limits.resendIntervalMs = m_settings.introResendIntervalMs;
    m_intros.Configure( m_identity, m_settings.stateDirectory, limits );
}

/*-------------------------------------------------------------------------*/

CEMailAddressBook& CEMailMatchProvider::GetAddressBook( void ) { return m_intros.GetBook(); }
const CEMailAddressBook& CEMailMatchProvider::GetAddressBook( void ) const { return m_intros.GetBook(); }

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::IntroNotice( const char* code, const CORE::CString& matchId, const CORE::CString& description )
{GUCEF_TRACE;

    Notice( IntroductionEvent, matchId, 0, code, description, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::MakeDraftInfo( const CEMailIntroDraft& draft, MULTIPLAY::CMatchInfo& info ) const
{GUCEF_TRACE;

    info = MULTIPLAY::CMatchInfo();
    info.matchId = draft.matchId;
    info.title = draft.title;
    info.backend = ProviderName;
    info.mode = MULTIPLAY::MATCHMODE_ASYNC;
    info.status = MULTIPLAY::MATCHSTATUS_INITIALIZING;
    info.creatorUserId = m_identity.userId;
    info.createdAt = draft.createdAt;
    MULTIPLAY::CMatchParticipant me;
    me.userId = m_identity.userId;
    me.displayName = draft.creatorName;
    me.publicKey = m_identity.publicKey;
    me.slot = 0;
    me.status = MULTIPLAY::MATCHPARTICIPANT_ACTIVE;
    me.contact = draft.creatorAddress;
    info.participants.push_back( me );
    for ( GUCEF::map< CORE::CString, CORE::CString >::const_iterator s = draft.settings.begin(); s != draft.settings.end(); ++s )
        info.SetSetting( (*s).first, (*s).second );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::AnnounceDraft( const CEMailIntroDraft& draft, bool added )
{GUCEF_TRACE;

    MULTIPLAY::CMatchUpdate update;
    MakeDraftInfo( draft, update.info );
    update.hasInfo = true;
    update.preview.matchId = draft.matchId;
    update.preview.revision = 0;
    update.preview.status = MULTIPLAY::MATCHSTATUS_INITIALIZING;
    update.preview.lastActivity = GetNowMs();
    if ( added && m_announcedDrafts.find( draft.matchId ) == m_announcedDrafts.end() )
    {
        m_announcedDrafts[ draft.matchId ] = true;
        ReportMatchAdded( update );
    }
    else
        ReportMatchUpdated( update );
    PublishDraftRoster( draft );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::CheckIntroductionTimes( Int64 now )
{GUCEF_TRACE;

    if ( now < m_lastIntroCheck + 1000 )
        return;
    m_lastIntroCheck = now;
    GUCEF::vector< CORE::CString > changed;
    m_intros.ExpireDue( now, changed );
    for ( size_t i=0; i<changed.size(); ++i )
    {
        IntroNotice( "INTRO_EXPIRED", changed[ i ], "An introduction was not answered in time" );
        CEMailIntroDraft draft;
        if ( m_intros.FindDraft( changed[ i ], draft ) )
            AnnounceDraft( draft, false );
    }
    m_intros.PurgeExpired( now );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailMatchProvider::GetIntroLinkText( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    CORE::CString text;
    if ( !m_settings.httpsHost.IsNULLOrEmpty() )
        text += "About the game and where to get it: https://" + m_settings.httpsHost + "/\n";
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::MailIntroRequest( const CEMailIntroDraft& draft, const CEMailIntroRequest& request )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    if ( !CEMailMatchCodec::SerializeToBytes( request, bytes, CEMailIntroRequest::MaxDataBytes ) )
        return false;
    CEMailOutgoing outgoing;
    outgoing.kind = MAILKIND_INTRO_REQUEST;
    outgoing.matchId = draft.matchId;
    outgoing.revision = 0;
    outgoing.turn = 0;
    outgoing.title = draft.title;
    outgoing.from = COM::CEMailAddress( m_settings.displayName.Length() <= 64 ? CEMailMatchCodec::SanitizeTitle( m_settings.displayName ) : CORE::CString(), m_settings.mailAddress );
    outgoing.to.push_back( COM::CEMailAddress( request.inviteeAddress ) );
    outgoing.actorName = request.creatorName;
    outgoing.linkText = GetIntroLinkText( draft.matchId );
    TMailTransfer transfer = MAILTRANSFER_ATTACHMENT;
    TryStringToMailTransfer( draft.settings.find( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER ) != draft.settings.end() ? (*draft.settings.find( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER )).second : CORE::CString(), transfer );
    outgoing.transfer = transfer;
    outgoing.data = &bytes;
    outgoing.dataLabel = CEMailMatchCodec::ArmorLabelData;
    outgoing.dataRevisionName = "intro";

    // what a person who has never seen the game needs to know
    const CORE::CString creator = request.creatorName.IsNULLOrEmpty() ? CORE::CString( "A player" ) : request.creatorName;
    CORE::CString text;
    text += creator + " would like to play the game \"" + request.title + "\" with you. These are the players:\n";
    for ( size_t i=0; i<request.parties.size(); ++i )
        text += "  " + request.parties[ i ].name + " <" + request.parties[ i ].address + ">\n";
    text += "The players of this game see each other's mail address: the addresses above are the ones that all of them will see.\n";
    text += "\n";
    text += "Redux is a turn-based game that is played by e-mail. You need the game on your computer to take part";
    text += m_settings.httpsHost.IsNULLOrEmpty() ? ".\n" : ": see the link below.\n";
    text += "When you have it, open Redux and look at the invitations: this introduction is listed there. Accept it to introduce yourself to " + creator + ", or decline it.\n";
    text += "When you accept, " + creator + " and the other players get your name, your mail address and the short fingerprint of your key. Your answer is signed by your key.\n";
    text += "If you do nothing, nothing happens and nothing is lost. The introduction is good for " + CORE::UInt32ToString( (UInt32)( ( request.expiresAt - request.createdAt ) / ( 24 * 60 * 60 * 1000 ) ) ) + " days.\n";
    text += "\n";
    text += "The fingerprint of the key of " + creator + " is " + CEMailIntroFingerprint::Of( request.creatorPublicKey ) + ". Mail alone cannot prove who sent this, so if it matters to you, compare the fingerprint with " + creator + " by another way, a call or a chat.\n";
    outgoing.bodyText = text;
    SMatch scratch;
    return QueueMail( scratch, outgoing, false );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::MailIntroReply( const CEMailInboundIntro& intro, const CEMailIntroReply& reply )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    if ( !CEMailMatchCodec::SerializeToBytes( reply, bytes, CEMailIntroRequest::MaxDataBytes ) )
        return false;
    CEMailOutgoing outgoing;
    outgoing.kind = MAILKIND_INTRO_REPLY;
    outgoing.matchId = intro.request.matchId;
    outgoing.revision = 0;
    outgoing.turn = 0;
    outgoing.title = intro.request.title;
    outgoing.from = COM::CEMailAddress( m_settings.displayName.Length() <= 64 ? CEMailMatchCodec::SanitizeTitle( m_settings.displayName ) : CORE::CString(), m_settings.mailAddress );
    outgoing.to.push_back( COM::CEMailAddress( intro.request.creatorAddress ) );
    outgoing.actorName = reply.accepted ? reply.displayName : GetOwnName();
    outgoing.linkText = GetIntroLinkText( intro.request.matchId );
    outgoing.transfer = MAILTRANSFER_ATTACHMENT;
    outgoing.data = &bytes;
    outgoing.dataLabel = CEMailMatchCodec::ArmorLabelData;
    outgoing.dataRevisionName = "intro";
    outgoing.note = reply.accepted ? CORE::CString( "This is the introduction you asked for: the name, the mail address and the key of the sender." ) : CORE::CString( "The sender does not want to take part." );
    SMatch scratch;
    return QueueMail( scratch, outgoing, false );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::BeginIntroductions( const CORE::CString& matchId, const CORE::CString& title, const GUCEF::map< CORE::CString, CORE::CString >& settings, const GUCEF::vector< CEMailIntroContactInput >& contacts, CORE::CString& reason )
{GUCEF_TRACE;

    if ( !m_started )
    {
        reason = "The mailbox of the player is not working";
        return false;
    }
    const Int64 now = GetNowMs();
    if ( !m_intros.BeginDraft( matchId, title, GetOwnName(), m_settings.mailAddress, settings, contacts, now, reason ) )
        return false;
    CEMailIntroDraft draft;
    m_intros.FindDraft( matchId, draft );
    for ( size_t i=0; i<draft.contacts.size(); ++i )
    {
        if ( MAILINTRO_NOT_ASKED != draft.contacts[ i ].state )
            continue;
        CEMailIntroRequest request;
        if ( MAILINTRORESULT_OK == m_intros.PrepareRequest( matchId, draft.contacts[ i ].address, false, now, request ) )
            MailIntroRequest( draft, request );
    }
    m_intros.FindDraft( matchId, draft );
    AnnounceDraft( draft, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::GetIntroductions( const CORE::CString& matchId, GUCEF::vector< CEMailIntroContact >& contacts ) const
{GUCEF_TRACE;

    CEMailIntroDraft draft;
    if ( !m_intros.FindDraft( matchId, draft ) )
        return false;
    contacts = draft.contacts;
    return true;
}

bool
CEMailMatchProvider::GetIntroductionDraft( const CORE::CString& matchId, CEMailIntroDraft& draft ) const
{GUCEF_TRACE;

    return m_intros.FindDraft( matchId, draft );
}

bool
CEMailMatchProvider::GetIntroductionSummary( const CORE::CString& matchId, CEMailIntroSummary& summary ) const
{GUCEF_TRACE;

    return m_intros.GetSummary( matchId, summary );
}

/*-------------------------------------------------------------------------*/

TMailIntroResult
CEMailMatchProvider::ResendIntroduction( const CORE::CString& matchId, const CORE::CString& address, bool askAgain )
{GUCEF_TRACE;

    if ( !m_started )
        return MAILINTRORESULT_NO_DRAFT;
    CEMailIntroRequest request;
    const TMailIntroResult result = m_intros.PrepareRequest( matchId, address, askAgain, GetNowMs(), request );
    if ( MAILINTRORESULT_OK != result )
        return result;
    CEMailIntroDraft draft;
    if ( !m_intros.FindDraft( matchId, draft ) || !MailIntroRequest( draft, request ) )
        return MAILINTRORESULT_BAD_STATEMENT;
    AnnounceDraft( draft, false );
    return MAILINTRORESULT_OK;
}

/*-------------------------------------------------------------------------*/

TMailIntroResult
CEMailMatchProvider::ConfirmIntroductionKey( const CORE::CString& matchId, const CORE::CString& address, bool accept )
{GUCEF_TRACE;

    const TMailIntroResult result = m_intros.ConfirmKeyChange( matchId, address, accept, GetNowMs() );
    if ( MAILINTRORESULT_OK == result )
    {
        CEMailIntroDraft draft;
        if ( m_intros.FindDraft( matchId, draft ) )
            AnnounceDraft( draft, false );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::GetPendingIntroductions( GUCEF::vector< CEMailInboundIntro >& pending ) const
{GUCEF_TRACE;

    m_intros.GetPending( pending );
}

/*-------------------------------------------------------------------------*/

TMailIntroResult
CEMailMatchProvider::AcceptIntroduction( const CORE::CString& matchId, bool confirmKeyChange )
{GUCEF_TRACE;

    if ( !m_started )
        return MAILINTRORESULT_NO_REQUEST;
    CEMailIntroReply reply;
    const TMailIntroResult result = m_intros.AcceptRequest( matchId, confirmKeyChange, GetOwnName(), GetNowMs(), reply );
    if ( MAILINTRORESULT_OK != result )
        return result;
    CEMailInboundIntro intro;
    if ( !m_intros.FindInbound( matchId, intro ) || !MailIntroReply( intro, reply ) )
        return MAILINTRORESULT_BAD_STATEMENT;
    return MAILINTRORESULT_OK;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::RemoveIntroducedPlayer( const CORE::CString& matchId, const CORE::CString& address, CORE::CString& reason )
{GUCEF_TRACE;

    // as in a lobby: a boss removes a player, under a vote nobody does alone, the host included. The players who accepted vote on a proposal (ProposeInDraft) and the host
    // carries out what passed. A player who declined is not removed by anybody: that entry is only tidied away
    CEMailIntroDraft draft;
    if ( m_started && m_intros.FindDraft( matchId, draft ) && MULTIPLAY::MATCHDECISIONPOLICY_BOSS != GetDraftPolicy( draft ) )
    {
        const CEMailIntroContact* contact = draft.FindContact( address );
        if ( GUCEF_NULL != contact && MAILINTRO_DECLINED != contact->state )
        {
            reason = "This game is decided by vote: propose to remove the player and the players who accepted vote on it";
            return false;
        }
    }
    return RemoveContactInternal( matchId, address, true, reason );
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::TMatchDecisionPolicy
CEMailMatchProvider::GetDraftPolicy( const CEMailIntroDraft& draft ) const
{GUCEF_TRACE;

    GUCEF::map< CORE::CString, CORE::CString >::const_iterator policyValue = m_settings.decisionPolicySettingKey.IsNULLOrEmpty() ? draft.settings.end() : draft.settings.find( m_settings.decisionPolicySettingKey );
    return ( policyValue != draft.settings.end() && (*policyValue).second.Uppercase() == "BOSS" ) ? MULTIPLAY::MATCHDECISIONPOLICY_BOSS : MULTIPLAY::MATCHDECISIONPOLICY_VOTE;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::AddIntroducedPlayer( const CORE::CString& matchId, const CEMailIntroContactInput& player, CORE::CString& reason )
{GUCEF_TRACE;

    if ( !m_started )
    {
        reason = "The mailbox of the player is not working";
        return false;
    }
    if ( !m_intros.AddContact( matchId, player, reason ) )
        return false;

    // a player who is not known by a player code is asked now, with the table as it is
    CEMailIntroDraft draft;
    if ( !m_intros.FindDraft( matchId, draft ) )
        return true;
    const CEMailIntroContact* added = draft.FindContact( player.address.Lowercase() );
    if ( GUCEF_NULL != added && MAILINTRO_NOT_ASKED == added->state )
    {
        CEMailIntroRequest request;
        if ( MAILINTRORESULT_OK == m_intros.PrepareRequest( matchId, added->address, false, GetNowMs(), request ) )
            MailIntroRequest( draft, request );
        m_intros.FindDraft( matchId, draft );
    }
    AnnounceDraft( draft, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::SetIntroductionSetting( const CORE::CString& matchId, const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    if ( !m_started || !m_intros.SetDraftSetting( matchId, key, value ) )
        return false;
    CEMailIntroDraft draft;
    if ( m_intros.FindDraft( matchId, draft ) )
        PublishDraftRoster( draft );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::DismissIntroduction( const CORE::CString& matchId )
{GUCEF_TRACE;

    CEMailInboundIntro intro;
    if ( !m_started || !m_intros.FindInbound( matchId, intro ) || CEMailInboundIntro::STATUS_PENDING == intro.status )
        return false;
    m_intros.ForgetInbound( matchId );
    if ( GUCEF_NULL == FindMatch( matchId ) )
        ForgetBoard( matchId );
    return true;
}

/*-------------------------------------------------------------------------*/

TMailIntroResult
CEMailMatchProvider::DeclineIntroduction( const CORE::CString& matchId )
{GUCEF_TRACE;

    if ( !m_started )
        return MAILINTRORESULT_NO_REQUEST;
    CEMailIntroReply reply;
    const TMailIntroResult result = m_intros.DeclineRequest( matchId, GetNowMs(), reply );
    if ( MAILINTRORESULT_OK != result )
        return result;
    CEMailInboundIntro intro;
    if ( !m_intros.FindInbound( matchId, intro ) || !MailIntroReply( intro, reply ) )
        return MAILINTRORESULT_BAD_STATEMENT;
    return MAILINTRORESULT_OK;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::HandleIntroRequest( const CEMailInboxMessage& mail )
{GUCEF_TRACE;

    CORE::CDynamicBuffer data;
    TMailTransfer used;
    const UInt32 cap = m_settings.maxControlBytes < CEMailIntroRequest::MaxDataBytes ? m_settings.maxControlBytes : CEMailIntroRequest::MaxDataBytes;
    if ( !ReadData( mail, CEMailMatchCodec::ArmorLabelData, cap, data, used ) )
    {
        IntroNotice( "INTRO_REFUSED", mail.incoming.matchId, MailIntroResultToString( MAILINTRORESULT_BAD_STATEMENT ) );
        return;
    }
    CEMailIntroRequest request;
    if ( !CEMailMatchCodec::DeserializeFromBytes( data.GetConstBufferPtr(), data.GetDataSize(), request, cap ) || request.matchId != mail.incoming.matchId )
    {
        IntroNotice( "INTRO_REFUSED", mail.incoming.matchId, MailIntroResultToString( MAILINTRORESULT_BAD_STATEMENT ) );
        return;
    }
    const Int64 now = GetNowMs();
    const TMailIntroResult result = m_intros.HandleRequest( request, mail.incoming.fromAddress, m_settings.mailAddress, NULL != FindMatch( request.matchId ) || IsRemoved( request.matchId ), now );
    switch ( result )
    {
        case MAILINTRORESULT_OK:
            IntroNotice( "INTRO_REQUEST", request.matchId, "A player asks to be introduced" );
            EnsureParticipantBoard( request.matchId );
            ProcessOrphans( request.matchId );         // a cancellation can have come first
            break;
        case MAILINTRORESULT_RESEND_REPLY:
        {
            // the reply of an introduction that was agreed to is made again: the creator did not get it
            CEMailIntroReply reply;
            CEMailInboundIntro intro;
            if ( MAILINTRORESULT_OK == m_intros.RepeatReply( request.matchId, GetOwnName(), now, reply ) && m_intros.FindInbound( request.matchId, intro ) )
                MailIntroReply( intro, reply );
            break;
        }
        case MAILINTRORESULT_DUPLICATE:
        case MAILINTRORESULT_RATE_LIMITED:
            break;
        default:
            IntroNotice( "INTRO_REFUSED", request.matchId, MailIntroResultToString( result ) );
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::HandleIntroReply( const CEMailInboxMessage& mail )
{GUCEF_TRACE;

    CORE::CDynamicBuffer data;
    TMailTransfer used;
    const UInt32 cap = m_settings.maxControlBytes < CEMailIntroRequest::MaxDataBytes ? m_settings.maxControlBytes : CEMailIntroRequest::MaxDataBytes;
    if ( !ReadData( mail, CEMailMatchCodec::ArmorLabelData, cap, data, used ) )
    {
        IntroNotice( "INTRO_REFUSED", mail.incoming.matchId, MailIntroResultToString( MAILINTRORESULT_BAD_STATEMENT ) );
        return;
    }
    CEMailIntroReply reply;
    if ( !CEMailMatchCodec::DeserializeFromBytes( data.GetConstBufferPtr(), data.GetDataSize(), reply, cap ) || reply.matchId != mail.incoming.matchId )
    {
        IntroNotice( "INTRO_REFUSED", mail.incoming.matchId, MailIntroResultToString( MAILINTRORESULT_BAD_STATEMENT ) );
        return;
    }
    CORE::CString matchId;
    const TMailIntroResult result = m_intros.HandleReply( reply, mail.incoming.fromAddress, GetNowMs(), &matchId );
    const char* code = GUCEF_NULL;
    switch ( result )
    {
        case MAILINTRORESULT_OK:            code = "INTRO_INTRODUCED"; break;
        case MAILINTRORESULT_DECLINED:      code = "INTRO_DECLINED"; break;
        case MAILINTRORESULT_KEY_CHANGED:   code = "INTRO_KEY_CHANGED"; break;
        case MAILINTRORESULT_DUPLICATE:     return;
        default:                            code = "INTRO_REFUSED"; break;
    }
    IntroNotice( code, reply.matchId, MailIntroResultToString( result ) );
    CEMailIntroDraft draft;
    if ( !matchId.IsNULLOrEmpty() && m_intros.FindDraft( matchId, draft ) )
        AnnounceDraft( draft, false );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::TryJoinIntroduced( SMatch& match, const CEMailInboxMessage& mail )
{GUCEF_TRACE;

    CEMailInboundIntro intro;
    if ( !m_intros.FindInbound( match.state.info.matchId, intro ) )
        return;
    const CORE::CString matchId = match.state.info.matchId;
    bool join = m_settings.autoJoinIntroduced && CEMailInboundIntro::STATUS_ACCEPTED == intro.status && match.state.hasRecord;
    if ( join )
    {
        // the table is the one that was announced, by the one who announced it, and the key in it is the key of the player
        const MULTIPLAY::CMatchInfo& info = match.state.record.info;
        const MULTIPLAY::CMatchParticipant* me = info.FindParticipant( m_identity.userId );
        GUCEF::vector< CORE::CString > addresses;
        for ( size_t i=0; i<info.participants.size(); ++i )
            addresses.push_back( info.participants[ i ].contact );
        const MULTIPLAY::CMatchParticipant* creator = info.FindParticipant( info.creatorUserId );
        join = GUCEF_NULL != me && GUCEF_NULL != creator && MULTIPLAY::MATCHPARTICIPANT_INVITED == me->status && creator->publicKey.Lowercase() == intro.request.creatorPublicKey.Lowercase() &&
               me->publicKey.Lowercase() == m_identity.publicKey.Lowercase() && m_intros.IsTableAnnounced( matchId, addresses, info.settings, m_settings.seatsSettingKey ) &&
               creator->contact.Lowercase() == mail.incoming.fromAddress.Lowercase();
    }
    m_intros.ForgetInbound( matchId );
    if ( !join )
        return;
    MULTIPLAY::CMatchAcceptance acceptance;
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    if ( acceptance.Sign( m_identity, match.state.record.info, true ) && Respond( matchId, acceptance, future ) )
        IntroNotice( "INTRO_JOINED", matchId, "The player joined the game they were introduced to" );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::ExportToFile( const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package, const CORE::CString& path, CORE::CString& reason )
{GUCEF_TRACE;

    CORE::CDynamicBuffer container;
    if ( MULTIPLAY::MATCHCONTAINER_OK != MULTIPLAY::CMatchContainer::Encode( preview, package, true, container ) )
    {
        reason = "The turn cannot be put in a file";
        return false;
    }
    if ( !CEMailFiles::WriteAtomic( path, container.GetConstBufferPtr(), container.GetDataSize() ) )
    {
        reason = "The file cannot be written";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::ExportToText( const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package, CORE::CString& text, CORE::CString& reason )
{GUCEF_TRACE;

    CORE::CDynamicBuffer container;
    if ( MULTIPLAY::MATCHCONTAINER_OK != MULTIPLAY::CMatchContainer::Encode( preview, package, true, container ) )
    {
        reason = "The turn cannot be put in text";
        return false;
    }
    MULTIPLAY::CMatchTextArmor armor( CEMailMatchCodec::ArmorLabelTurn );
    if ( !armor.Encode( preview.matchId, preview.revision, container.GetConstBufferPtr(), container.GetDataSize(), text ) )
    {
        reason = "The turn is too large to be put in text";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::ImportFile( const CORE::CString& path, CORE::CString& reason )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    if ( !m_started || !CEMailFiles::ReadBounded( path, m_settings.maxContainerBytes, bytes ) || 0 == bytes.GetDataSize() )
    {
        reason = "The file cannot be read or is larger than the limit";
        return false;
    }
    return ProcessContainer( bytes.GetConstBufferPtr(), bytes.GetDataSize(), CORE::CString::Empty, CORE::CString::Empty, reason );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::ImportText( const CORE::CString& text, CORE::CString& reason )
{GUCEF_TRACE;

    COM::CEMailMessage message;
    message.textBody = text;
    CORE::CDynamicBuffer bytes;
    TMailTransfer used;
    const TMailPayloadResult result = CEMailMatchCodec::ExtractData( message, CORE::CString::Empty, 0, CEMailMatchCodec::ArmorLabelTurn, m_settings.maxContainerBytes, m_settings.maxBodyTextBytes, bytes, used );
    if ( !m_started || MAILPAYLOAD_OK != result )
    {
        reason = CORE::CString( "No turn could be read from the text (" ) + MailPayloadResultToString( result ) + ")";
        return false;
    }
    return ProcessContainer( bytes.GetConstBufferPtr(), bytes.GetDataSize(), CORE::CString::Empty, CORE::CString::Empty, reason );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE TABLE AND THE DECISIONS OF A GAME THAT HAS NO RECORD YET       //
//                                                                         //
//-------------------------------------------------------------------------*/

MULTIPLAY::CMatchDraftBoard*
CEMailMatchProvider::FindBoard( const CORE::CString& matchId )
{GUCEF_TRACE;

    TBoardMap::iterator i = m_boards.find( matchId );
    return i == m_boards.end() ? GUCEF_NULL : &(*i).second;
}

const MULTIPLAY::CMatchDraftBoard*
CEMailMatchProvider::FindBoard( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    TBoardMap::const_iterator i = m_boards.find( matchId );
    return i == m_boards.end() ? GUCEF_NULL : &(*i).second;
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CMatchDraftBoard*
CEMailMatchProvider::EnsureHostBoard( const CORE::CString& matchId )
{GUCEF_TRACE;

    MULTIPLAY::CMatchDraftBoard* board = FindBoard( matchId );
    if ( GUCEF_NULL != board )
        return board->IsHost() ? board : GUCEF_NULL;
    MULTIPLAY::CMatchDraftBoard created;
    if ( !created.InitializeAsHost( m_identity, matchId ) )
        return GUCEF_NULL;
    created.SetApplyGraceMs( m_settings.draftApplyGraceMs );
    m_boards[ matchId ] = created;
    return FindBoard( matchId );
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CMatchDraftBoard*
CEMailMatchProvider::EnsureParticipantBoard( const CORE::CString& matchId )
{GUCEF_TRACE;

    MULTIPLAY::CMatchDraftBoard* board = FindBoard( matchId );
    if ( GUCEF_NULL != board )
        return board;
    CEMailInboundIntro intro;
    if ( !m_intros.FindInbound( matchId, intro ) )
        return GUCEF_NULL;
    MULTIPLAY::CMatchDraftBoard created;
    if ( !created.InitializeAsParticipant( matchId, intro.request.creatorUserId, intro.request.creatorPublicKey, m_identity ) )
        return GUCEF_NULL;
    created.SetOwnContact( m_settings.mailAddress );
    created.SetApplyGraceMs( m_settings.draftApplyGraceMs );
    m_boards[ matchId ] = created;
    return FindBoard( matchId );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailMatchProvider::GetBoardsDirectory( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( m_settings.stateDirectory, "drafts" );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::SaveBoard( const CORE::CString& matchId )
{GUCEF_TRACE;

    const MULTIPLAY::CMatchDraftBoard* board = FindBoard( matchId );
    if ( GUCEF_NULL == board || m_settings.stateDirectory.IsNULLOrEmpty() )
        return;
    CORE::CDynamicBuffer bytes;
    if ( !CORE::CreateDirs( GetBoardsDirectory() ) || !CEMailMatchCodec::SerializeToBytes( *board, bytes, g_maxStateFileBytes ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProvider: The table of a game that is being set up could not be stored" );
        return;
    }
    CEMailFiles::WriteAtomic( CORE::CombinePath( GetBoardsDirectory(), matchId + ".board" ), bytes.GetConstBufferPtr(), bytes.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::LoadBoards( void )
{GUCEF_TRACE;

    m_boards.clear();
    if ( m_settings.stateDirectory.IsNULLOrEmpty() )
        return;
    CORE::CString::StringVector names;
    CEMailFiles::ListFiles( GetBoardsDirectory(), names );
    for ( size_t i=0; i<names.size(); ++i )
    {
        if ( !names[ i ].EndsWith( ".board" ) )
            continue;
        const CORE::CString matchId = names[ i ].SubstrToIndex( names[ i ].Length() - 6, true );
        CORE::CDynamicBuffer bytes;
        MULTIPLAY::CMatchDraftBoard board;
        if ( !MULTIPLAY::CMatchId::IsValid( matchId ) || !CEMailFiles::ReadBounded( CORE::CombinePath( GetBoardsDirectory(), names[ i ] ), g_maxStateFileBytes, bytes ) ||
             !CEMailMatchCodec::DeserializeFromBytes( bytes.GetConstBufferPtr(), bytes.GetDataSize(), board, g_maxStateFileBytes ) || board.GetMatchId() != matchId )
            continue;
        board.SetApplyGraceMs( m_settings.draftApplyGraceMs );
        m_boards[ matchId ] = board;
    }
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::ForgetBoard( const CORE::CString& matchId )
{GUCEF_TRACE;

    m_boards.erase( matchId );
    if ( !m_settings.stateDirectory.IsNULLOrEmpty() )
        CORE::DeleteFile( CORE::CombinePath( GetBoardsDirectory(), matchId + ".board" ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailMatchProvider::GetDraftTitle( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    CEMailIntroDraft draft;
    if ( m_intros.FindDraft( matchId, draft ) )
        return draft.title;
    CEMailInboundIntro intro;
    if ( m_intros.FindInbound( matchId, intro ) )
        return intro.request.title;
    const SMatch* match = FindMatch( matchId );
    return GUCEF_NULL != match ? match->state.info.title : CORE::CString();
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::MailDraftData( TMailMatchKind kind, const CORE::CIDataNodeSerializable& data, const CORE::CString& matchId, UInt32 revision, const CORE::CString& dataPart, const GUCEF::vector< CORE::CString >& addresses, const CORE::CString& title, const CORE::CString& actorName, const CORE::CString& note, TMailTransfer transfer )
{GUCEF_TRACE;

    if ( addresses.empty() )
        return true;
    CORE::CDynamicBuffer bytes;
    if ( !CEMailMatchCodec::SerializeToBytes( data, bytes, m_settings.maxControlBytes ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MailMatchProvider: The data of a mail about a game that is being set up could not be made" );
        return false;
    }
    CEMailOutgoing outgoing;
    outgoing.kind = kind;
    outgoing.matchId = matchId;
    outgoing.revision = revision;
    outgoing.turn = 0;
    outgoing.title = title;
    outgoing.from = COM::CEMailAddress( m_settings.displayName.Length() <= 64 ? CEMailMatchCodec::SanitizeTitle( m_settings.displayName ) : CORE::CString(), m_settings.mailAddress );
    outgoing.to.push_back( COM::CEMailAddress( addresses[ 0 ] ) );
    for ( size_t i=1; i<addresses.size(); ++i )
        outgoing.cc.push_back( COM::CEMailAddress( addresses[ i ] ) );
    outgoing.actorName = actorName;
    outgoing.transfer = transfer;
    outgoing.data = &bytes;
    outgoing.dataLabel = CEMailMatchCodec::ArmorLabelData;
    outgoing.dataRevisionName = dataPart;
    outgoing.note = note;
    SMatch scratch;
    return QueueMail( scratch, outgoing, false );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::MailToDraftAudience( TMailMatchKind kind, const CORE::CIDataNodeSerializable& data, const CORE::CString& matchId, const char* dataPart )
{GUCEF_TRACE;

    MULTIPLAY::CMatchRoster roster;
    const MULTIPLAY::CMatchDraftBoard* board = FindBoard( matchId );
    if ( GUCEF_NULL == board || !board->GetRoster( roster ) )
        return false;

    // the host first, then every other player who has not declined
    GUCEF::vector< CORE::CString > addresses;
    for ( size_t i=0; i<roster.entries.size(); ++i )
    {
        const MULTIPLAY::CMatchRosterEntry& entry = roster.entries[ i ];
        if ( entry.userId == m_identity.userId || MULTIPLAY::MATCHPARTICIPANT_DECLINED == entry.status || !COM::CEMailAddress( entry.contact ).IsValid() )
            continue;
        if ( entry.userId == roster.hostUserId )
            addresses.insert( addresses.begin(), entry.contact );
        else
            addresses.push_back( entry.contact );
    }
    TMailTransfer transfer = MAILTRANSFER_ATTACHMENT;
    return MailDraftData( kind, data, matchId, roster.revision, dataPart, addresses, GetDraftTitle( matchId ), GetOwnName(), CORE::CString( "A decision about the table of the game: signed by the player, counted by everybody." ), transfer );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::PublishDraftRoster( const CEMailIntroDraft& draft )
{GUCEF_TRACE;

    if ( !m_started )
        return;
    MULTIPLAY::CMatchDraftBoard* board = EnsureHostBoard( draft.matchId );
    if ( GUCEF_NULL == board )
        return;
    GUCEF::vector< CORE::CString > spectators;
    board->GetAppliedSpectators( spectators );

    MULTIPLAY::TMatchRosterEntryVector entries;
    MULTIPLAY::CMatchRosterEntry host;
    host.userId = m_identity.userId;
    host.publicKey = m_identity.publicKey.Lowercase();
    host.displayName = draft.creatorName;
    host.contact = draft.creatorAddress;
    host.status = MULTIPLAY::MATCHPARTICIPANT_ACTIVE;
    const MULTIPLAY::TMatchDecisionPolicy policy = GetDraftPolicy( draft );
    host.boss = MULTIPLAY::MATCHDECISIONPOLICY_BOSS == policy;
    entries.push_back( host );
    for ( size_t i=0; i<draft.contacts.size(); ++i )
    {
        const CEMailIntroContact& contact = draft.contacts[ i ];
        MULTIPLAY::CMatchRosterEntry entry;
        entry.displayName = ( contact.displayName.IsNULLOrEmpty() ? contact.name : contact.displayName );
        entry.contact = contact.address;
        if ( MAILINTRO_INTRODUCED == contact.state && MULTIPLAY::CMatchSigning::IsHexOfLength( contact.publicKey, MULTIPLAY::CMatchSigning::PublicKeyHexSize ) )
        {
            entry.publicKey = contact.publicKey.Lowercase();
            entry.userId = MULTIPLAY::CPlayerIdentity::MakeUserIdFromHex( entry.publicKey );
            bool watches = false;
            for ( size_t s=0; s<spectators.size(); ++s )
                watches = watches || spectators[ s ] == entry.userId;
            entry.status = watches ? MULTIPLAY::MATCHPARTICIPANT_SPECTATOR : MULTIPLAY::MATCHPARTICIPANT_ACTIVE;
            if ( watches )
                entry.statusCode = GUCEF_MULTIPLAY_SPECTATOR_REASON_VOLUNTARY;
        }
        else
        {
            entry.userId = MULTIPLAY::CMatchRosterEntry::MakePendingUserId( draft.matchId, contact.address );
            entry.status = MAILINTRO_DECLINED == contact.state ? MULTIPLAY::MATCHPARTICIPANT_DECLINED : MULTIPLAY::MATCHPARTICIPANT_INVITED;
        }
        entries.push_back( entry );
    }
    MULTIPLAY::CMatchInfo::TSettingsMap settings;
    for ( GUCEF::map< CORE::CString, CORE::CString >::const_iterator s = draft.settings.begin(); s != draft.settings.end(); ++s )
        settings[ (*s).first ] = (*s).second;

    MULTIPLAY::CMatchRoster roster;
    if ( !board->PublishRoster( m_identity, entries, policy, MULTIPLAY::CMatchRoster::MakeSettingsDigest( settings ), GetNowMs(), roster ) )
        return;
    SaveBoard( draft.matchId );

    // everybody who is still asked or takes part gets the table
    GUCEF::vector< CORE::CString > addresses;
    for ( size_t i=0; i<draft.contacts.size(); ++i )
    {
        const TMailIntroState state = draft.contacts[ i ].state;
        if ( MAILINTRO_DECLINED != state && MAILINTRO_EXPIRED != state && MAILINTRO_REFUSED != state && COM::CEMailAddress( draft.contacts[ i ].address ).IsValid() )
            addresses.push_back( draft.contacts[ i ].address );
    }
    TMailTransfer transfer = MAILTRANSFER_ATTACHMENT;
    GUCEF::map< CORE::CString, CORE::CString >::const_iterator transferValue = draft.settings.find( GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER );
    if ( transferValue != draft.settings.end() )
        TryStringToMailTransfer( (*transferValue).second, transfer );
    MailDraftData( MAILKIND_ROSTER, roster, draft.matchId, roster.revision, "roster", addresses, draft.title, draft.creatorName, CORE::CString( "The table of the game as it is now: who takes part. Signed by the host; the game checks it." ), transfer );
    Notice( RosterChangedEvent, draft.matchId, roster.revision, "ROSTER", CORE::UInt32ToString( roster.revision ), CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::HandleRoster( const CORE::CString& matchId, const void* data, UInt32 size, const CORE::CString& fromAddress )
{GUCEF_TRACE;

    MULTIPLAY::CMatchRoster roster;
    if ( !CEMailMatchCodec::DeserializeFromBytes( data, size, roster, m_settings.maxControlBytes ) || roster.matchId != matchId )
    {
        MailProblem( matchId, 0, "BAD_ROSTER", "A table of a game is not valid" );
        return;
    }
    MULTIPLAY::CMatchDraftBoard* board = EnsureParticipantBoard( matchId );
    if ( GUCEF_NULL == board )
    {
        if ( GUCEF_NULL == FindBoard( matchId ) )
            KeepOrphan( matchId, (UInt8) MAILKIND_ROSTER, data, size );
        return;
    }
    if ( board->IsHost() )
        return;
    switch ( board->AcceptRoster( roster, GetNowMs() ) )
    {
        case MULTIPLAY::MATCHDRAFT_OK:
            SaveBoard( matchId );
            Notice( RosterChangedEvent, matchId, roster.revision, "ROSTER", CORE::UInt32ToString( roster.revision ), CORE::CString::Empty );
            break;
        case MULTIPLAY::MATCHDRAFT_DUPLICATE:
        case MULTIPLAY::MATCHDRAFT_STALE:
            break;
        case MULTIPLAY::MATCHDRAFT_EQUIVOCATION:
            SaveBoard( matchId );
            Notice( RosterChangedEvent, matchId, roster.revision, "ROSTER_EQUIVOCATION", "The host signed two different tables for one step", roster.hostUserId );
            break;
        case MULTIPLAY::MATCHDRAFT_KEY_CHANGED:
            SaveBoard( matchId );
            Notice( RosterChangedEvent, matchId, roster.revision, "ROSTER_KEY_CHANGED", "A table gives a player another key than an earlier one did", CORE::CString::Empty );
            break;
        case MULTIPLAY::MATCHDRAFT_OWN_KEY_MISMATCH:
            Notice( RosterChangedEvent, matchId, roster.revision, "ROSTER_REFUSED", MULTIPLAY::MatchDraftResultToString( MULTIPLAY::MATCHDRAFT_OWN_KEY_MISMATCH ), CORE::CString::Empty );
            break;
        case MULTIPLAY::MATCHDRAFT_UNAUTHORIZED_REMOVAL:
            Notice( RosterChangedEvent, matchId, roster.revision, "ROSTER_REFUSED", MULTIPLAY::MatchDraftResultToString( MULTIPLAY::MATCHDRAFT_UNAUTHORIZED_REMOVAL ), roster.hostUserId );
            break;
        default:
            Notice( RosterChangedEvent, matchId, roster.revision, "ROSTER_REFUSED", "NOT_ACCEPTED", CORE::CString::Empty );
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::HandleDraftProposal( const CORE::CString& matchId, const void* data, UInt32 size, const CORE::CString& fromAddress )
{GUCEF_TRACE;

    MULTIPLAY::CMatchProposal proposal;
    if ( !CEMailMatchCodec::DeserializeFromBytes( data, size, proposal, m_settings.maxControlBytes ) || proposal.matchId != matchId )
    {
        MailProblem( matchId, 0, "BAD_PROPOSAL", "A proposal is not valid" );
        return;
    }
    MULTIPLAY::CMatchDraftBoard* board = FindBoard( matchId );
    if ( GUCEF_NULL == board )
        board = EnsureParticipantBoard( matchId );
    if ( GUCEF_NULL == board )
    {
        KeepOrphan( matchId, (UInt8) MAILKIND_DRAFT_PROPOSAL, data, size );
        return;
    }
    if ( proposal.proposerUserId == m_identity.userId )
        return;
    CORE::CString reason;
    const MULTIPLAY::TMatchDraftResult result = board->AddProposal( proposal, GetNowMs(), reason );
    if ( MULTIPLAY::MATCHDRAFT_OK == result || MULTIPLAY::MATCHDRAFT_PENDING == result )
        SaveBoard( matchId );
    if ( MULTIPLAY::MATCHDRAFT_OK == result )
    {
        Notice( VoteReceivedEvent, matchId, proposal.rosterRevision, "DRAFT_PROPOSAL", "A player proposes a decision about the table", proposal.proposerUserId, proposal.proposalId );
        AfterDraftMessage( matchId );
    }
    else
    if ( MULTIPLAY::MATCHDRAFT_DUPLICATE != result && MULTIPLAY::MATCHDRAFT_PENDING != result && MULTIPLAY::MATCHDRAFT_LIMIT != result )
        MailProblem( matchId, 0, "BAD_PROPOSAL", "A proposal is not valid" );

    // the sender holds an older table than the host does: the host sends the table again, to a player of the game and not often
    if ( board->IsHost() && proposal.rosterRevision < board->GetRevision() && !fromAddress.IsNULLOrEmpty() )
    {
        CEMailIntroDraft draft;
        MULTIPLAY::CMatchRoster roster;
        const CORE::CString key = matchId + "|" + fromAddress.Lowercase();
        const Int64 now = GetNowMs();
        GUCEF::map< CORE::CString, Int64 >::const_iterator last = m_rosterResent.find( key );
        if ( m_intros.FindDraft( matchId, draft ) && GUCEF_NULL != draft.FindContact( fromAddress.Lowercase() ) && MAILINTRO_DECLINED != draft.FindContact( fromAddress.Lowercase() )->state &&
             board->GetRoster( roster ) && ( last == m_rosterResent.end() || now >= (*last).second + (Int64) m_settings.introResendIntervalMs ) )
        {
            m_rosterResent[ key ] = now;
            GUCEF::vector< CORE::CString > to;
            to.push_back( fromAddress.Lowercase() );
            MailDraftData( MAILKIND_ROSTER, roster, matchId, roster.revision, "roster", to, draft.title, draft.creatorName, CORE::CString( "The table of the game as it is now." ), MAILTRANSFER_ATTACHMENT );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::HandleDraftVote( const CORE::CString& matchId, const void* data, UInt32 size, const CORE::CString& fromAddress )
{GUCEF_TRACE;

    MULTIPLAY::CMatchVote vote;
    if ( !CEMailMatchCodec::DeserializeFromBytes( data, size, vote, m_settings.maxControlBytes ) || vote.matchId != matchId )
    {
        MailProblem( matchId, 0, "BAD_VOTE", "A vote is not valid" );
        return;
    }
    MULTIPLAY::CMatchDraftBoard* board = FindBoard( matchId );
    if ( GUCEF_NULL == board )
        board = EnsureParticipantBoard( matchId );
    if ( GUCEF_NULL == board )
    {
        KeepOrphan( matchId, (UInt8) MAILKIND_DRAFT_VOTE, data, size );
        return;
    }
    if ( vote.voterUserId == m_identity.userId )
        return;
    CORE::CString reason;
    const MULTIPLAY::TMatchDraftResult result = board->AddVote( vote, GetNowMs(), reason );
    if ( MULTIPLAY::MATCHDRAFT_OK == result || MULTIPLAY::MATCHDRAFT_PENDING == result )
        SaveBoard( matchId );
    if ( MULTIPLAY::MATCHDRAFT_OK == result )
    {
        Notice( VoteReceivedEvent, matchId, 0, "DRAFT_VOTE", "A player voted on a decision about the table", vote.voterUserId, vote.proposalId );
        AfterDraftMessage( matchId );
    }
    else
    if ( MULTIPLAY::MATCHDRAFT_DUPLICATE != result && MULTIPLAY::MATCHDRAFT_PENDING != result && MULTIPLAY::MATCHDRAFT_LIMIT != result )
        MailProblem( matchId, 0, "BAD_VOTE", "A vote is not valid" );
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::AfterDraftMessage( const CORE::CString& matchId )
{GUCEF_TRACE;

    const MULTIPLAY::CMatchDraftBoard* board = FindBoard( matchId );
    if ( GUCEF_NULL != board && board->IsHost() && m_settings.applyDraftDecisions )
        ApplyDraftDecisions( matchId );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::GetDraftBoard( const CORE::CString& matchId, MULTIPLAY::CMatchDraftBoard& board ) const
{GUCEF_TRACE;

    const MULTIPLAY::CMatchDraftBoard* found = FindBoard( matchId );
    if ( GUCEF_NULL == found )
        return false;
    board = *found;
    return true;
}

bool
CEMailMatchProvider::GetDraftRoster( const CORE::CString& matchId, MULTIPLAY::CMatchRoster& roster ) const
{GUCEF_TRACE;

    const MULTIPLAY::CMatchDraftBoard* found = FindBoard( matchId );
    return GUCEF_NULL != found && found->GetRoster( roster );
}

UInt32
CEMailMatchProvider::GetDraftTallies( const CORE::CString& matchId, GUCEF::vector< MULTIPLAY::CMatchDraftTally >& tallies ) const
{GUCEF_TRACE;

    tallies.clear();
    const MULTIPLAY::CMatchDraftBoard* found = FindBoard( matchId );
    return GUCEF_NULL == found ? 0 : found->GetTallies( GetNowMs(), tallies );
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::ProposeInDraft( const CORE::CString& matchId, MULTIPLAY::TMatchDecisionKind kind, const CORE::CString& targetUserId, CORE::CString& proposalId, CORE::CString& reason )
{GUCEF_TRACE;

    MULTIPLAY::CMatchDraftBoard* board = FindBoard( matchId );
    if ( !m_started || GUCEF_NULL == board )
    {
        reason = "This game has no table here";
        return false;
    }
    MULTIPLAY::CMatchProposal proposal;
    const Int64 now = GetNowMs();
    if ( !board->MakeProposal( m_identity, kind, targetUserId, now, (Int64) m_settings.draftDecisionMs, proposal, reason ) )
        return false;
    const MULTIPLAY::TMatchDraftResult result = board->AddProposal( proposal, now, reason );
    if ( MULTIPLAY::MATCHDRAFT_OK != result && MULTIPLAY::MATCHDRAFT_DUPLICATE != result )
    {
        if ( reason.IsNULLOrEmpty() )
            reason = MULTIPLAY::MatchDraftResultToString( result );
        return false;
    }
    MailToDraftAudience( MAILKIND_DRAFT_PROPOSAL, proposal, matchId, "proposal" );
    SaveBoard( matchId );
    proposalId = proposal.proposalId;
    Notice( VoteReceivedEvent, matchId, proposal.rosterRevision, "DRAFT_PROPOSAL", "The player proposes a decision about the table", m_identity.userId, proposal.proposalId );
    AfterDraftMessage( matchId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::VoteInDraft( const CORE::CString& matchId, const CORE::CString& proposalId, bool yes, CORE::CString& reason )
{GUCEF_TRACE;

    MULTIPLAY::CMatchDraftBoard* board = FindBoard( matchId );
    MULTIPLAY::CMatchProposal proposal;
    if ( !m_started || GUCEF_NULL == board || !board->GetProposal( proposalId, proposal ) )
    {
        reason = "This proposal is not known";
        return false;
    }
    MULTIPLAY::CMatchVote vote;
    if ( board->GetVote( proposalId, m_identity.userId, vote ) )
    {
        // a vote that was cast is sent again, the same
        MailToDraftAudience( MAILKIND_DRAFT_VOTE, vote, matchId, "vote" );
        return true;
    }
    const Int64 now = GetNowMs();
    if ( !vote.Sign( m_identity, proposal, yes, now ) )
    {
        reason = "The vote could not be signed";
        return false;
    }
    const MULTIPLAY::TMatchDraftResult result = board->AddVote( vote, now, reason );
    if ( MULTIPLAY::MATCHDRAFT_OK != result )
    {
        if ( reason.IsNULLOrEmpty() )
            reason = MULTIPLAY::MatchDraftResultToString( result );
        return false;
    }
    MailToDraftAudience( MAILKIND_DRAFT_VOTE, vote, matchId, "vote" );
    SaveBoard( matchId );
    Notice( VoteReceivedEvent, matchId, 0, "DRAFT_VOTE", "The player voted on a decision about the table", m_identity.userId, proposalId );
    AfterDraftMessage( matchId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::ConfirmDraftKeyChange( const CORE::CString& matchId, bool accept )
{GUCEF_TRACE;

    MULTIPLAY::CMatchDraftBoard* board = FindBoard( matchId );
    if ( GUCEF_NULL == board || !board->ConfirmHeldRoster( accept, GetNowMs() ) )
        return false;
    SaveBoard( matchId );
    if ( accept )
        Notice( RosterChangedEvent, matchId, board->GetRevision(), "ROSTER", CORE::UInt32ToString( board->GetRevision() ), CORE::CString::Empty );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailMatchProvider::RemoveContactInternal( const CORE::CString& matchId, const CORE::CString& address, bool lowerSeats, CORE::CString& reason )
{GUCEF_TRACE;

    CEMailIntroDraft draft;
    if ( !m_started || !m_intros.FindDraft( matchId, draft ) )
    {
        reason = "The game does not wait for introductions";
        return false;
    }
    const CEMailIntroContact* contact = draft.FindContact( address );
    if ( GUCEF_NULL == contact )
    {
        reason = "The player is not part of the game";
        return false;
    }
    const CEMailIntroContact removed = *contact;
    const size_t before = draft.contacts.size();
    if ( !m_intros.RemoveContact( matchId, address ) )
    {
        reason = "The game needs at least one other player";
        return false;
    }
    // the table gets a seat less when the number of seats is a setting the provider knows: the players that remain are asked nothing again
    if ( lowerSeats && !m_settings.seatsSettingKey.IsNULLOrEmpty() )
    {
        GUCEF::map< CORE::CString, CORE::CString >::const_iterator seatsValue = draft.settings.find( m_settings.seatsSettingKey );
        const UInt32 seats = seatsValue == draft.settings.end() ? 0 : CORE::StringToUInt32( (*seatsValue).second.C_String(), 0 );
        if ( seats > 2 && seats - 1 >= before )
            m_intros.SetDraftSetting( matchId, m_settings.seatsSettingKey, CORE::UInt32ToString( seats - 1 ) );
    }
    MULTIPLAY::CMatchProposal cancel;
    cancel.matchId = matchId;
    cancel.kind = MULTIPLAY::MATCHDECISION_CANCEL;
    cancel.createdAt = GetNowMs();
    if ( cancel.Sign( m_identity ) )
        MailDraftCancel( draft, removed, cancel );
    CEMailIntroDraft after;
    if ( m_intros.FindDraft( matchId, after ) )
        AnnounceDraft( after, false );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CEMailMatchProvider::ApplyDraftDecisions( const CORE::CString& matchId )
{GUCEF_TRACE;

    MULTIPLAY::CMatchDraftBoard* board = FindBoard( matchId );
    CEMailIntroDraft draft;
    if ( !m_started || GUCEF_NULL == board || !board->IsHost() || !m_intros.FindDraft( matchId, draft ) )
        return 0;
    GUCEF::vector< MULTIPLAY::CMatchDraftTally > passed;
    board->GetPassedForApplication( GetNowMs(), passed );
    UInt32 applied = 0;
    for ( size_t p=0; p<passed.size(); ++p )
    {
        const MULTIPLAY::CMatchDraftTally& tally = passed[ p ];
        if ( !m_intros.FindDraft( matchId, draft ) )
            break;

        // the contacts the decision is about
        GUCEF::vector< CORE::CString > addresses;
        for ( size_t c=0; c<draft.contacts.size(); ++c )
        {
            const CEMailIntroContact& contact = draft.contacts[ c ];
            const bool keyed = MAILINTRO_INTRODUCED == contact.state && MULTIPLAY::CMatchSigning::IsHexOfLength( contact.publicKey, MULTIPLAY::CMatchSigning::PublicKeyHexSize );
            const CORE::CString realId = keyed ? MULTIPLAY::CPlayerIdentity::MakeUserIdFromHex( contact.publicKey.Lowercase() ) : CORE::CString();
            const CORE::CString pendingId = MULTIPLAY::CMatchRosterEntry::MakePendingUserId( matchId, contact.address );
            if ( MULTIPLAY::MATCHDECISION_FORCE_START == tally.kind )
            {
                bool named = false;
                for ( size_t t=0; t<tally.targetUserIds.size(); ++t )
                    named = named || tally.targetUserIds[ t ] == pendingId;
                if ( named && !keyed && MAILINTRO_DECLINED != contact.state )
                    addresses.push_back( contact.address );
            }
            else
            if ( ( keyed && realId == tally.targetUserId ) || pendingId == tally.targetUserId )
                addresses.push_back( contact.address );
        }
        if ( addresses.empty() )
            continue;
        board->MarkApplied( tally.proposalId );
        SaveBoard( matchId );
        CORE::CString reason;
        if ( MULTIPLAY::MATCHDECISION_MOVE_TO_SPECTATOR == tally.kind )
        {
            if ( m_intros.FindDraft( matchId, draft ) )
                PublishDraftRoster( draft );
        }
        else
            for ( size_t a=0; a<addresses.size(); ++a )
                RemoveContactInternal( matchId, addresses[ a ], MULTIPLAY::MATCHDECISION_KICK == tally.kind, reason );
        ++applied;
        Notice( DraftDecisionAppliedEvent, matchId, board->GetRevision(), MULTIPLAY::MatchDecisionKindToString( tally.kind ), "The host carried out a decision about the table", tally.targetUserId, tally.proposalId );
    }
    return applied;
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::ImportDraftStatements( SMatch& match )
{GUCEF_TRACE;

    const MULTIPLAY::CMatchDraftBoard* board = FindBoard( match.state.info.matchId );
    if ( GUCEF_NULL == board )
        return;
    // the participants who chose to watch before the record existed: their own signed statements are decisions of the table, carried by the first turn
    GUCEF::vector< MULTIPLAY::CMatchProposal > statements;
    board->GetSpectatorStatements( statements );
    for ( size_t i=0; i<statements.size(); ++i )
    {
        CORE::CString reason;
        if ( GUCEF_NULL != match.state.info.FindParticipant( statements[ i ].proposerUserId ) )
            ApplyEvidenceOutsideTurn( match, statements[ i ], reason );
    }
}

/*-------------------------------------------------------------------------*/

void
CEMailMatchProvider::CheckRecordAgainstRoster( SMatch& match )
{GUCEF_TRACE;

    const MULTIPLAY::CMatchDraftBoard* board = FindBoard( match.state.info.matchId );
    MULTIPLAY::CMatchRoster roster;
    if ( GUCEF_NULL == board || !match.state.hasRecord || !board->GetRoster( roster ) )
        return;
    bool deviates = false;
    const MULTIPLAY::CMatchInfo& info = match.state.record.info;
    for ( size_t i=0; i<info.participants.size(); ++i )
    {
        const MULTIPLAY::CMatchRosterEntry* entry = roster.FindEntry( info.participants[ i ].userId );
        if ( GUCEF_NULL == entry || ( MULTIPLAY::MATCHPARTICIPANT_ACTIVE != entry->status && MULTIPLAY::MATCHPARTICIPANT_SPECTATOR != entry->status ) )
            deviates = true;
    }
    for ( size_t i=0; i<roster.entries.size(); ++i )
        if ( MULTIPLAY::MATCHPARTICIPANT_ACTIVE == roster.entries[ i ].status && GUCEF_NULL == info.FindParticipant( roster.entries[ i ].userId ) )
            deviates = true;
    if ( deviates )
        Notice( RosterChangedEvent, info.matchId, roster.revision, "RECORD_DEVIATES", "The record of the game is not the table the host published last", info.creatorUserId );
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
