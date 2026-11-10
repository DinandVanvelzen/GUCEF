/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHSYSTEMCLOCK_H
#include "gucefMULTIPLAY_CMatchSystemClock.h"
#define GUCEF_MULTIPLAY_CMATCHSYSTEMCLOCK_H
#endif /* GUCEF_MULTIPLAY_CMATCHSYSTEMCLOCK_H ? */

#include "gucefMULTIPLAY_CMatchBook.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CMatchBook::MatchBookChangedEvent = "GUCEF::MULTIPLAY::CMatchBook::MatchBookChangedEvent";
const CORE::CEvent CMatchBook::YourTurnEvent = "GUCEF::MULTIPLAY::CMatchBook::YourTurnEvent";
const CORE::CEvent CMatchBook::MatchFinishedEvent = "GUCEF::MULTIPLAY::CMatchBook::MatchFinishedEvent";
const CORE::CEvent CMatchBook::InviteReceivedEvent = "GUCEF::MULTIPLAY::CMatchBook::InviteReceivedEvent";
const CORE::CEvent CMatchBook::ProviderProblemEvent = "GUCEF::MULTIPLAY::CMatchBook::ProviderProblemEvent";

#define GUCEF_MULTIPLAY_MATCHBOOK_DEFAULT_TICK_MS       1000

enum EEntryFlag
{
    ENTRYFLAG_MUTED    = 0 ,
    ENTRYFLAG_PINNED   = 1 ,
    ENTRYFLAG_ARCHIVED = 2
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
InfoDiffers( const CMatchInfo& a, const CMatchInfo& b )
{GUCEF_TRACE;

    if ( a.status != b.status || a.closedByUserId != b.closedByUserId || a.GetDigest() != b.GetDigest() || a.participants.size() != b.participants.size() )
        return true;
    for ( size_t i=0; i<a.participants.size(); ++i )
        if ( !( a.participants[ i ] == b.participants[ i ] ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

static bool
IsFinishedStatus( TMatchStatus status )
{GUCEF_TRACE;

    return MATCHSTATUS_FINISHED == status || MATCHSTATUS_ABANDONED == status || MATCHSTATUS_CANCELLED == status;
}

/*-------------------------------------------------------------------------*/

/** Whether the match is over for the player: the signed preview says so, or the table closed it before it began */
static bool
IsClosed( const CMatchEntry& entry )
{GUCEF_TRACE;

    return IsFinishedStatus( entry.preview.status ) || ( entry.hasInfo && MATCHSTATUS_CANCELLED == entry.info.status );
}

/*-------------------------------------------------------------------------*/

/** pinned first, then the nearest deadline (none last), then the latest activity first, then the id for a stable order */
static bool
EntryComesBefore( const CMatchEntry& a, const CMatchEntry& b )
{GUCEF_TRACE;

    if ( a.pinned != b.pinned )
        return a.pinned;
    const Int64 noDeadline = 0x7FFFFFFFFFFFFFFFLL;
    Int64 deadlineA = 0 == a.preview.deadline ? noDeadline : a.preview.deadline;
    Int64 deadlineB = 0 == b.preview.deadline ? noDeadline : b.preview.deadline;
    if ( deadlineA != deadlineB )
        return deadlineA < deadlineB;
    if ( a.preview.lastActivity != b.preview.lastActivity )
        return a.preview.lastActivity > b.preview.lastActivity;
    return a.preview.matchId < b.preview.matchId;
}

/** The order of EntryComesBefore() for pointers to entries */
class CEntryPointerComesBefore
{
    public:

    bool operator()( const CMatchEntry* a, const CMatchEntry* b ) const
    {
        return EntryComesBefore( *a, *b );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMatchBook::RegisterEvents( void )
{GUCEF_TRACE;

    MatchBookChangedEvent.Initialize();
    YourTurnEvent.Initialize();
    MatchFinishedEvent.Initialize();
    InviteReceivedEvent.Initialize();
    ProviderProblemEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CMatchBook::CMatchBook( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_providers()
    , m_entries()
    , m_pendingSubmits()
    , m_offlineProviders()
    , m_store()
    , m_localUserId()
    , m_openMatchId()
    , m_clock( GUCEF_NULL )
    , m_timer( GUCEF_MULTIPLAY_MATCHBOOK_DEFAULT_TICK_MS )
    , m_fastIntervalMs( DefaultFastIntervalMs )
    , m_slowIntervalMs( DefaultSlowIntervalMs )
    , m_lastFastRefresh( 0 )
    , m_lastSlowRefresh( 0 )
    , m_started( false )
    , m_background( false )
    , m_dataLock()
{GUCEF_TRACE;

    RegisterEvents();
    CIMatchProvider::RegisterEvents();
    TEventCallback timerCallback( this, &CMatchBook::OnTimerUpdate );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, timerCallback );
}

/*-------------------------------------------------------------------------*/

CMatchBook::~CMatchBook()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_timer.SetEnabled( false );
    m_providers.clear();
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::SetLocalUserId( const CORE::CString& userId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_localUserId = userId;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchBook::GetLocalUserId( void ) const
{GUCEF_TRACE;

    return m_localUserId;
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::SetClock( const CIMatchClock* clock )
{GUCEF_TRACE;

    m_clock = clock;
}

/*-------------------------------------------------------------------------*/

Int64
CMatchBook::GetNowMs( void ) const
{GUCEF_TRACE;

    return ( GUCEF_NULL != m_clock ? *m_clock : static_cast< const CIMatchClock& >( CMatchSystemClock::Instance() ) ).GetNowMs();
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::SetStoreDirectory( const CORE::CString& directory )
{GUCEF_TRACE;

    m_store.SetDirectory( directory );
}

/*-------------------------------------------------------------------------*/

CMatchStore&
CMatchBook::GetStore( void )
{GUCEF_TRACE;

    return m_store;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::Persist( const CMatchEntry& entry )
{GUCEF_TRACE;

    if ( m_store.GetDirectory().IsNULLOrEmpty() )
        return true;
    return m_store.Save( entry );
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::LoadFromStore( void )
{GUCEF_TRACE;

    if ( m_store.GetDirectory().IsNULLOrEmpty() )
        return false;

    TMatchEntryVector loaded;
    UInt32 skipped = 0;
    if ( !m_store.LoadAll( loaded, &skipped ) )
        return false;
    if ( skipped > 0 )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MatchBook: " << skipped << " files in the match directory are not valid records and were left alone" );

    TPendingEventVector events;
    {
        MT::CScopeMutex lock( m_dataLock );
        for ( size_t i=0; i<loaded.size(); ++i )
        {
            if ( m_entries.find( loaded[ i ].GetMatchId() ) != m_entries.end() )
                continue;
            m_entries[ loaded[ i ].GetMatchId() ] = loaded[ i ];
            TPendingEvent e = TPendingEvent();
            e.hasChange = true;
            e.changeKind = MATCHBOOKCHANGE_ADDED;
            e.matchId = loaded[ i ].GetMatchId();
            events.push_back( e );
        }
    }
    Raise( events );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::SubscribeToProvider( CIMatchProvider* provider )
{GUCEF_TRACE;

    TEventCallback addedCallback( this, &CMatchBook::OnMatchAdded );
    SubscribeTo( provider, CIMatchProvider::MatchAddedEvent, addedCallback );
    TEventCallback updatedCallback( this, &CMatchBook::OnMatchUpdated );
    SubscribeTo( provider, CIMatchProvider::MatchUpdatedEvent, updatedCallback );
    TEventCallback removedCallback( this, &CMatchBook::OnMatchRemoved );
    SubscribeTo( provider, CIMatchProvider::MatchRemovedEvent, removedCallback );
    TEventCallback submitCallback( this, &CMatchBook::OnSubmitResult );
    SubscribeTo( provider, CIMatchProvider::SubmitResultEvent, submitCallback );
    TEventCallback stateCallback( this, &CMatchBook::OnProviderState );
    SubscribeTo( provider, CIMatchProvider::ProviderStateEvent, stateCallback );
    TEventCallback errorCallback( this, &CMatchBook::OnProviderError );
    SubscribeTo( provider, CIMatchProvider::ProviderErrorEvent, errorCallback );
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::AddProvider( CIMatchProviderPtr provider )
{GUCEF_TRACE;

    if ( provider.IsNULL() || provider->GetProviderName().IsNULLOrEmpty() )
        return false;
    for ( size_t i=0; i<m_providers.size(); ++i )
        if ( m_providers[ i ].GetPointerAlways() == provider.GetPointerAlways() || m_providers[ i ]->GetProviderName() == provider->GetProviderName() )
            return false;

    m_providers.push_back( provider );
    SubscribeToProvider( provider.GetPointerAlways() );
    if ( m_started )
        provider->Start();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::AddProvider( const CORE::CString& providerType, const CORE::CDataNode* config )
{GUCEF_TRACE;

    CIMatchProviderPtr provider = CMultiplayGlobal::Instance()->GetMatchProviderFactory().Create( providerType );
    if ( provider.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MatchBook: There is no match provider \"" << providerType << "\"" );
        return false;
    }
    if ( GUCEF_NULL != config && !provider->LoadConfig( *config ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MatchBook: The match provider \"" << providerType << "\" does not accept its config" );
        return false;
    }
    return AddProvider( provider );
}

/*-------------------------------------------------------------------------*/

const CMatchBook::TProviderVector&
CMatchBook::GetProviders( void ) const
{GUCEF_TRACE;

    return m_providers;
}

/*-------------------------------------------------------------------------*/

CIMatchProviderPtr
CMatchBook::FindProvider( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_providers.size(); ++i )
        if ( m_providers[ i ]->GetProviderName() == providerName )
            return m_providers[ i ];
    return CIMatchProviderPtr();
}

/*-------------------------------------------------------------------------*/

CIMatchProviderPtr
CMatchBook::GetProviderOfMatch( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    CORE::CString providerName;
    {
        MT::CScopeMutex lock( m_dataLock );
        TEntryMap::const_iterator i = m_entries.find( matchId );
        if ( i == m_entries.end() )
            return CIMatchProviderPtr();
        providerName = (*i).second.providerName;
    }
    return FindProvider( providerName );
}

/*-------------------------------------------------------------------------*/

CIMatchProvider*
CMatchBook::FindProviderByNotifier( CORE::CNotifier* notifier ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_providers.size(); ++i )
    {
        CIMatchProviderPtr provider = m_providers[ i ];
        CIMatchProvider* providerPtr = provider.GetPointerAlways();
        if ( static_cast< CORE::CNotifier* >( providerPtr ) == notifier )
            return providerPtr;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::Start( void )
{GUCEF_TRACE;

    if ( m_started )
        return true;
    m_started = true;
    for ( size_t i=0; i<m_providers.size(); ++i )
        if ( !m_providers[ i ]->IsStarted() )
            m_providers[ i ]->Start();

    m_lastFastRefresh = m_lastSlowRefresh = GetNowMs();
    m_timer.SetEnabled( true );
    RefreshAll();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::Stop( void )
{GUCEF_TRACE;

    m_started = false;
    m_timer.SetEnabled( false );
    for ( size_t i=0; i<m_providers.size(); ++i )
        m_providers[ i ]->Stop();
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::IsStarted( void ) const
{GUCEF_TRACE;

    return m_started;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::RefreshAll( void )
{GUCEF_TRACE;

    bool any = false;
    for ( size_t i=0; i<m_providers.size(); ++i )
    {
        if ( m_providers[ i ]->IsStarted() && m_providers[ i ]->HasCapability( MATCHCAPABILITY_PREVIEW_POLL ) )
            any = m_providers[ i ]->RefreshAll() || any;
    }
    return any;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::Refresh( const CORE::CString& matchId )
{GUCEF_TRACE;

    CIMatchProviderPtr provider = GetProviderOfMatch( matchId );
    if ( provider.IsNULL() || !provider->IsStarted() || !provider->HasCapability( MATCHCAPABILITY_PREVIEW_POLL ) )
        return false;
    return provider->Refresh( matchId );
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::SetBackground( bool background )
{GUCEF_TRACE;

    m_background = background;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::IsBackground( void ) const
{GUCEF_TRACE;

    return m_background;
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::Resume( void )
{GUCEF_TRACE;

    m_background = false;
    if ( m_started )
    {
        m_lastFastRefresh = m_lastSlowRefresh = GetNowMs();
        RefreshAll();
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::SetIntervals( UInt32 fastIntervalMs, UInt32 slowIntervalMs )
{GUCEF_TRACE;

    m_fastIntervalMs = fastIntervalMs;
    m_slowIntervalMs = slowIntervalMs;
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::Update( void )
{GUCEF_TRACE;

    if ( !m_started || m_background )
        return;

    Int64 now = GetNowMs();
    CORE::CString openMatchId = m_openMatchId;
    if ( !openMatchId.IsNULLOrEmpty() && now - m_lastFastRefresh >= (Int64) m_fastIntervalMs )
    {
        m_lastFastRefresh = now;
        Refresh( openMatchId );
    }
    if ( now - m_lastSlowRefresh >= (Int64) m_slowIntervalMs )
    {
        m_lastSlowRefresh = now;
        RefreshAll();
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::OnTimerUpdate( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Update();
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::IsYourTurn( const CMatchEntry& entry ) const
{GUCEF_TRACE;

    if ( m_localUserId.IsNULLOrEmpty() || MATCHSTATUS_ACTIVE != entry.preview.status || !entry.preview.IsCurrent( m_localUserId ) || IsClosed( entry ) )
        return false;
    if ( IsForming( entry ) )
        return false;                                       // not everybody answered the invitation yet, or the players are still to introduce themselves: nobody plays
    if ( entry.hasInfo )
    {
        const CMatchParticipant* me = entry.info.FindParticipant( m_localUserId );
        if ( GUCEF_NULL != me && MATCHPARTICIPANT_ACTIVE != me->status )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::IsForming( const CMatchEntry& entry ) const
{GUCEF_TRACE;

    return MATCHSTATUS_FORMING == entry.preview.status || MATCHSTATUS_INITIALIZING == entry.preview.status ||
           ( entry.hasInfo && ( MATCHSTATUS_FORMING == entry.info.status || MATCHSTATUS_INITIALIZING == entry.info.status ) );
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::IsInvited( const CMatchEntry& entry ) const
{GUCEF_TRACE;

    if ( m_localUserId.IsNULLOrEmpty() || !entry.hasInfo )
        return false;
    const CMatchParticipant* me = entry.info.FindParticipant( m_localUserId );
    return GUCEF_NULL != me && MATCHPARTICIPANT_INVITED == me->status;
}

/*-------------------------------------------------------------------------*/

TMatchGroup
CMatchBook::GetGroupOf( const CMatchEntry& entry ) const
{GUCEF_TRACE;

    if ( IsClosed( entry ) )
        return MATCHGROUP_FINISHED;
    if ( IsForming( entry ) || IsInvited( entry ) )
        return MATCHGROUP_FORMING_OR_INVITED;
    return IsYourTurn( entry ) ? MATCHGROUP_YOUR_TURN : MATCHGROUP_THEIR_TURN;
}

/*-------------------------------------------------------------------------*/

TMatchEntryState
CMatchBook::GetStateOf( const CMatchEntry& entry ) const
{GUCEF_TRACE;

    if ( entry.hasInfo && MATCHSTATUS_CANCELLED == entry.info.status )
        return MATCHENTRYSTATE_CANCELLED;
    if ( MATCHSTATUS_ABANDONED == entry.preview.status )
        return MATCHENTRYSTATE_ABANDONED;
    if ( MATCHSTATUS_FINISHED == entry.preview.status )
        return MATCHENTRYSTATE_FINISHED;
    if ( !m_localUserId.IsNULLOrEmpty() && entry.hasInfo )
    {
        const CMatchParticipant* me = entry.info.FindParticipant( m_localUserId );
        if ( GUCEF_NULL != me && me->IsSpectator() )
            return me->IsIncompatible() ? MATCHENTRYSTATE_INCOMPATIBLE : MATCHENTRYSTATE_SPECTATING;
    }
    if ( IsInvited( entry ) )
        return MATCHENTRYSTATE_INVITED;
    if ( MATCHSTATUS_INITIALIZING == entry.preview.status || ( entry.hasInfo && MATCHSTATUS_INITIALIZING == entry.info.status ) )
        return MATCHENTRYSTATE_INITIALIZING;
    if ( MATCHSTATUS_FORMING == entry.preview.status || ( entry.hasInfo && MATCHSTATUS_FORMING == entry.info.status ) )
        return MATCHENTRYSTATE_FORMING;
    return IsYourTurn( entry ) ? MATCHENTRYSTATE_YOUR_TURN : MATCHENTRYSTATE_THEIR_TURN;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::MergeUpdate( const CMatchUpdate& update, TPendingEventVector& events )
{GUCEF_TRACE;

    const CMatchPreview& incoming = update.preview;
    if ( !incoming.IsWithinLimits() || ( update.hasInfo && update.info.matchId != incoming.matchId ) )
        return false;

    MT::CScopeMutex lock( m_dataLock );

    const Int64 now = GetNowMs();
    TEntryMap::iterator existing = m_entries.find( incoming.matchId );
    const bool isNew = existing == m_entries.end();
    CMatchEntry entry;
    if ( !isNew )
        entry = (*existing).second;

    const bool wasFinished = !isNew && IsFinishedStatus( entry.preview.status );
    const bool wasInvited = !isNew && IsInvited( entry );
    bool previewAccepted = false;
    bool changed = false;

    if ( isNew )
    {
        entry.preview = incoming;
        previewAccepted = true;
        changed = true;
    }
    else
    if ( incoming.revision > entry.preview.revision )
    {
        entry.preview = incoming;
        previewAccepted = true;
        changed = true;
    }

    if ( entry.providerName.IsNULLOrEmpty() && !update.providerName.IsNULLOrEmpty() )
        entry.providerName = update.providerName;
    if ( update.hasInfo && ( !entry.hasInfo || InfoDiffers( entry.info, update.info ) ) )
    {
        entry.info = update.info;
        entry.hasInfo = true;
        changed = true;
    }
    if ( !update.cursor.IsNULLOrEmpty() )
        entry.cursor = update.cursor;

    bool yourTurnNow = false;
    if ( previewAccepted )
    {
        // the match that is open is seen the moment its revision arrives
        if ( entry.preview.matchId == m_openMatchId )
        {
            entry.lastSeenRevision = entry.preview.revision;
            entry.unseen = false;
        }
        else
            entry.unseen = entry.preview.revision > entry.lastSeenRevision;
    }

    yourTurnNow = IsYourTurn( entry );
    if ( yourTurnNow )
    {
        if ( 0 == entry.yourTurnSince )
        {
            CIMatchProviderPtr provider = FindProvider( entry.providerName );
            bool trustedClock = !provider.IsNULL() && provider->HasCapability( MATCHCAPABILITY_SERVER_CLOCK ) && entry.preview.lastActivity > 0;
            entry.yourTurnSince = trustedClock ? entry.preview.lastActivity : now;
            changed = true;
        }
    }
    else
    if ( 0 != entry.yourTurnSince )
    {
        entry.yourTurnSince = 0;
        changed = true;
    }

    TPendingEvent pending = TPendingEvent();
    pending.matchId = entry.preview.matchId;
    pending.revision = entry.preview.revision;
    if ( yourTurnNow && entry.preview.revision > entry.notifiedRevision )
    {
        entry.notifiedRevision = entry.preview.revision;
        pending.yourTurn = true;
    }
    pending.finished = previewAccepted && !isNew && !wasFinished && IsFinishedStatus( entry.preview.status );
    pending.invite = ( isNew || !wasInvited ) && IsInvited( entry );
    pending.hasChange = changed;
    pending.changeKind = isNew ? MATCHBOOKCHANGE_ADDED : MATCHBOOKCHANGE_UPDATED;

    m_entries[ entry.preview.matchId ] = entry;
    Persist( entry );
    if ( pending.hasChange || pending.yourTurn || pending.finished || pending.invite )
        events.push_back( pending );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::Raise( const TPendingEventVector& events )
{GUCEF_TRACE;

    for ( size_t i=0; i<events.size(); ++i )
    {
        const TPendingEvent& e = events[ i ];
        if ( e.hasChange )
        {
            TMatchBookChangedEventData change( e.matchId, e.changeKind );
            if ( !NotifyObservers( MatchBookChangedEvent, &change ) )
                return;
        }
        if ( e.invite )
        {
            TMatchIdEventData data( e.matchId );
            if ( !NotifyObservers( InviteReceivedEvent, &data ) )
                return;
        }
        if ( e.yourTurn )
        {
            TYourTurnEventData data( e.matchId, e.revision );
            if ( !NotifyObservers( YourTurnEvent, &data ) )
                return;
        }
        if ( e.finished )
        {
            TMatchIdEventData data( e.matchId );
            if ( !NotifyObservers( MatchFinishedEvent, &data ) )
                return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::OnMatchAdded( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CMatchUpdate* update = CORE::CICloneable::CloneableAs< CMatchUpdate >( eventData );
    if ( GUCEF_NULL == update )
        return;
    TPendingEventVector events;
    MergeUpdate( *update, events );
    Raise( events );
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::OnMatchUpdated( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CMatchUpdate* update = CORE::CICloneable::CloneableAs< CMatchUpdate >( eventData );
    if ( GUCEF_NULL == update )
        return;
    TPendingEventVector events;
    MergeUpdate( *update, events );
    Raise( events );
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::OnMatchRemoved( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* matchId = static_cast< const CORE::TCloneableString* >( eventData );
    CIMatchProvider* provider = FindProviderByNotifier( notifier );
    if ( GUCEF_NULL == matchId || GUCEF_NULL == provider )
        return;

    // only the provider that carries a match can take it away
    {
        MT::CScopeMutex lock( m_dataLock );
        TEntryMap::iterator i = m_entries.find( matchId->GetData() );
        if ( i == m_entries.end() || (*i).second.providerName != provider->GetProviderName() )
            return;
    }
    RemoveMatch( matchId->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::OnSubmitResult( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CMatchOperationResult* result = CORE::CICloneable::CloneableAs< CMatchOperationResult >( eventData );
    CIMatchProvider* provider = FindProviderByNotifier( notifier );
    if ( GUCEF_NULL == result || GUCEF_NULL == provider )
        return;

    CMatchUpdate update;
    update.providerName = provider->GetProviderName();
    bool haveUpdate = false;
    {
        MT::CScopeMutex lock( m_dataLock );
        TPreviewMap::iterator pending = m_pendingSubmits.find( result->matchId );
        if ( MATCHOPERATION_OK == result->status )
        {
            if ( pending != m_pendingSubmits.end() )
            {
                update.preview = (*pending).second;
                haveUpdate = true;
            }
            else
            if ( result->hasPreview )
            {
                update.preview = result->preview;
                haveUpdate = true;
            }
        }
        else
        if ( MATCHOPERATION_STALE == result->status && result->hasPreview )
        {
            update.preview = result->preview;
            haveUpdate = true;
        }
        if ( pending != m_pendingSubmits.end() )
            m_pendingSubmits.erase( pending );
    }
    if ( !haveUpdate )
        return;

    TPendingEventVector events;
    MergeUpdate( update, events );
    Raise( events );
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::OnProviderState( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CMatchProviderState* state = CORE::CICloneable::CloneableAs< CMatchProviderState >( eventData );
    CIMatchProvider* provider = FindProviderByNotifier( notifier );
    if ( GUCEF_NULL == state || GUCEF_NULL == provider )
        return;

    if ( !state->online )
    {
        m_offlineProviders.insert( provider->GetProviderName() );
        CMatchProviderState problem( *state );
        NotifyObservers( ProviderProblemEvent, &problem );
    }
    else
    if ( m_offlineProviders.erase( provider->GetProviderName() ) > 0 && m_started && provider->IsStarted() && provider->HasCapability( MATCHCAPABILITY_PREVIEW_POLL ) )
    {
        // back online: what happened meanwhile is found by asking again
        provider->RefreshAll();
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::OnProviderError( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* description = static_cast< const CORE::TCloneableString* >( eventData );
    CIMatchProvider* provider = FindProviderByNotifier( notifier );
    if ( GUCEF_NULL == description || GUCEF_NULL == provider )
        return;
    CMatchProviderState problem;
    problem.online = true;
    problem.providerName = provider->GetProviderName();
    problem.description = description->GetData();
    NotifyObservers( ProviderProblemEvent, &problem );
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::OpenMatch( const CORE::CString& matchId )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_dataLock );
        TEntryMap::iterator i = m_entries.find( matchId );
        if ( i == m_entries.end() )
            return false;
        m_openMatchId = matchId;
        CMatchEntry& entry = (*i).second;
        entry.lastSeenRevision = entry.preview.revision;
        entry.unseen = false;
        Persist( entry );
    }
    m_lastFastRefresh = GetNowMs();
    TMatchBookChangedEventData change( matchId, MATCHBOOKCHANGE_OPENED );
    NotifyObservers( MatchBookChangedEvent, &change );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::CloseMatch( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_openMatchId.Clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchBook::GetOpenMatchId( void ) const
{GUCEF_TRACE;

    return m_openMatchId;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::MarkSeen( const CORE::CString& matchId )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_dataLock );
        TEntryMap::iterator i = m_entries.find( matchId );
        if ( i == m_entries.end() )
            return false;
        CMatchEntry& entry = (*i).second;
        if ( !entry.unseen && entry.lastSeenRevision == entry.preview.revision )
            return true;
        entry.lastSeenRevision = entry.preview.revision;
        entry.unseen = false;
        Persist( entry );
    }
    TMatchBookChangedEventData change( matchId, MATCHBOOKCHANGE_FLAGS );
    NotifyObservers( MatchBookChangedEvent, &change );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::ChangeFlag( const CORE::CString& matchId, int flag, bool value )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_dataLock );
        TEntryMap::iterator i = m_entries.find( matchId );
        if ( i == m_entries.end() )
            return false;
        CMatchEntry& entry = (*i).second;
        bool* target = ENTRYFLAG_MUTED == flag ? &entry.muted : ( ENTRYFLAG_PINNED == flag ? &entry.pinned : &entry.archived );
        if ( *target == value )
            return true;
        *target = value;
        Persist( entry );
    }
    TMatchBookChangedEventData change( matchId, MATCHBOOKCHANGE_FLAGS );
    NotifyObservers( MatchBookChangedEvent, &change );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::SetMuted( const CORE::CString& matchId, bool muted )
{GUCEF_TRACE;

    return ChangeFlag( matchId, ENTRYFLAG_MUTED, muted );
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::SetPinned( const CORE::CString& matchId, bool pinned )
{GUCEF_TRACE;

    return ChangeFlag( matchId, ENTRYFLAG_PINNED, pinned );
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::SetArchived( const CORE::CString& matchId, bool archived )
{GUCEF_TRACE;

    return ChangeFlag( matchId, ENTRYFLAG_ARCHIVED, archived );
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::SetLocalTitle( const CORE::CString& matchId, const CORE::CString& title )
{GUCEF_TRACE;

    // the name comes from a user interface: no control characters, no blanks around it, a bounded size cut between characters
    GUCEF::vector< char > bytes;
    const unsigned char* source = (const unsigned char*) title.C_String();
    while ( 0 != *source )
    {
        const unsigned char lead = *source;
        const UInt32 size = lead < 0x80 ? 1 : ( 0xC0 == ( lead & 0xE0 ) ? 2 : ( 0xE0 == ( lead & 0xF0 ) ? 3 : ( 0xF0 == ( lead & 0xF8 ) ? 4 : 0 ) ) );
        if ( 0 == size )
        {
            ++source;       // a byte that does not start a character
            continue;
        }
        UInt32 present = 1;
        while ( present < size && 0x80 == ( source[ present ] & 0xC0 ) )
            ++present;
        if ( present < size )
            break;          // a character cut short
        if ( size > 1 || ( lead >= 0x20 && 0x7F != lead ) )
        {
            if ( bytes.size() + size > MaxLocalTitleBytes )
                break;
            bytes.insert( bytes.end(), source, source + size );
        }
        source += size;
    }
    bytes.push_back( 0 );
    CORE::CString clean( &bytes[ 0 ] );
    clean = clean.Trim( true ).Trim( false );

    {
        MT::CScopeMutex lock( m_dataLock );
        TEntryMap::iterator i = m_entries.find( matchId );
        if ( i == m_entries.end() )
            return false;
        CMatchEntry& entry = (*i).second;
        if ( entry.localTitle == clean )
            return true;
        entry.localTitle = clean;
        Persist( entry );
    }
    TMatchBookChangedEventData change( matchId, MATCHBOOKCHANGE_FLAGS );
    NotifyObservers( MatchBookChangedEvent, &change );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::RemoveMatch( const CORE::CString& matchId )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_dataLock );
        TEntryMap::iterator i = m_entries.find( matchId );
        if ( i == m_entries.end() )
            return false;
        m_entries.erase( i );
        m_pendingSubmits.erase( matchId );
        if ( m_openMatchId == matchId )
            m_openMatchId.Clear();
        m_store.Remove( matchId );
    }
    TMatchBookChangedEventData change( matchId, MATCHBOOKCHANGE_REMOVED );
    NotifyObservers( MatchBookChangedEvent, &change );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::GetEntry( const CORE::CString& matchId, CMatchEntry& entry ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    TEntryMap::const_iterator i = m_entries.find( matchId );
    if ( i == m_entries.end() )
        return false;
    entry = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchBook::GetEntryCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::GetEntries( TMatchEntryVector& entries ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    entries.reserve( entries.size() + m_entries.size() );
    TEntryMap::const_iterator i = m_entries.begin();
    while ( i != m_entries.end() )
    {
        entries.push_back( (*i).second );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchBook::GetGroup( TMatchGroup group, TMatchEntryVector& entries, bool includeArchived ) const
{GUCEF_TRACE;

    // the entries are ordered by pointer and copied once, in order, so that many matches cost no more than their copies
    MT::CScopeMutex lock( m_dataLock );
    GUCEF::vector< const CMatchEntry* > selected;
    TEntryMap::const_iterator i = m_entries.begin();
    while ( i != m_entries.end() )
    {
        const CMatchEntry& entry = (*i).second;
        if ( ( includeArchived || !entry.archived ) && GetGroupOf( entry ) == group )
            selected.push_back( &entry );
        ++i;
    }
    std::sort( selected.begin(), selected.end(), CEntryPointerComesBefore() );

    entries.reserve( entries.size() + selected.size() );
    for ( size_t a=0; a<selected.size(); ++a )
        entries.push_back( *selected[ a ] );
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchBook::GetYourTurnCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    UInt32 count = 0;
    TEntryMap::const_iterator i = m_entries.begin();
    while ( i != m_entries.end() )
    {
        if ( !(*i).second.archived && MATCHGROUP_YOUR_TURN == GetGroupOf( (*i).second ) )
            ++count;
        ++i;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::GetNextYourTurn( const CORE::CString& exceptMatchId, CORE::CString& matchId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    const CMatchEntry* best = GUCEF_NULL;
    TEntryMap::const_iterator i = m_entries.begin();
    while ( i != m_entries.end() )
    {
        const CMatchEntry& entry = (*i).second;
        if ( entry.preview.matchId != exceptMatchId && !entry.archived && MATCHGROUP_YOUR_TURN == GetGroupOf( entry ) &&
             ( GUCEF_NULL == best || entry.yourTurnSince < best->yourTurnSince ) )
            best = &entry;
        ++i;
    }
    if ( GUCEF_NULL == best )
        return false;
    matchId = best->preview.matchId;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::Submit( const CORE::CString& matchId, const CMatchPreview& preview, const CMatchPackage& package, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CIMatchProviderPtr provider = GetProviderOfMatch( matchId );
    if ( provider.IsNULL() || preview.matchId != matchId )
        return false;

    UInt32 expectedRevision = 0;
    {
        MT::CScopeMutex lock( m_dataLock );
        TEntryMap::const_iterator i = m_entries.find( matchId );
        if ( i != m_entries.end() )
            expectedRevision = (*i).second.preview.revision;
        m_pendingSubmits[ matchId ] = preview;
    }
    if ( !provider->Submit( matchId, expectedRevision, preview, package, future ) )
    {
        MT::CScopeMutex lock( m_dataLock );
        m_pendingSubmits.erase( matchId );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::Fetch( const CORE::CString& matchId, UInt32 revision, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CIMatchProviderPtr provider = GetProviderOfMatch( matchId );
    return !provider.IsNULL() && provider->Fetch( matchId, revision, future );
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::CreateMatch( const CORE::CString& providerName, const CMatchRecord& record, const CMatchPackage& firstPackage, const CMatchPreview& firstPreview, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CIMatchProviderPtr provider = FindProvider( providerName );
    return !provider.IsNULL() && provider->CreateMatch( record, firstPackage, firstPreview, future );
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "storeDirectory", m_store.GetDirectory() );
    config.SetAttribute( "storeCodec", m_store.GetCodecName() );
    config.SetAttribute( "fastIntervalMs", m_fastIntervalMs );
    config.SetAttribute( "slowIntervalMs", m_slowIntervalMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBook::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CORE::CString directory = config.GetAttributeValueOrChildValueByName( "storeDirectory" ).AsString( m_store.GetDirectory(), true );
    CORE::CString codec = config.GetAttributeValueOrChildValueByName( "storeCodec" ).AsString( m_store.GetCodecName(), true );
    m_store.SetDirectory( directory );
    m_store.SetCodecName( codec );
    m_fastIntervalMs = config.GetAttributeValueOrChildValueByName( "fastIntervalMs" ).AsUInt32( m_fastIntervalMs, true );
    m_slowIntervalMs = config.GetAttributeValueOrChildValueByName( "slowIntervalMs" ).AsUInt32( m_slowIntervalMs, true );
    UInt32 tickMs = config.GetAttributeValueOrChildValueByName( "tickMs" ).AsUInt32( 0, true );
    if ( tickMs > 0 )
        m_timer.SetInterval( tickMs );

    bool allOk = true;
    CORE::CDataNode::const_iterator i = config.ConstBegin();
    while ( i != config.ConstEnd() )
    {
        const CORE::CDataNode* child = (*i);
        if ( child->GetName() == "Provider" )
        {
            CORE::CString type = child->GetAttributeValueOrChildValueByName( "type" ).AsString( CORE::CString::Empty, true );
            allOk = AddProvider( type, child->FindChild( "Config" ) ) && allOk;
        }
        ++i;
    }
    return allOk;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchBook::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchBook";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
