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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefMULTIPLAY_CMatchAlertRouter.h"

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

#define GUCEF_MULTIPLAY_MATCHALERTROUTER_TICK_MS      1000

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchAlertRouter::CMatchAlertRouter( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_book( GUCEF_NULL )
    , m_sinks()
    , m_clock( GUCEF_NULL )
    , m_timer( GUCEF_MULTIPLAY_MATCHALERTROUTER_TICK_MS )
    , m_mode( MATCHALERTMODE_EVERY_TURN )
    , m_quietEnabled( false )
    , m_quietStart( 22 * 60 )
    , m_quietEnd( 8 * 60 )
    , m_minIntervalMs( 0 )
    , m_coalesceWindowMs( 0 )
    , m_digestMinute( 9 * 60 )
    , m_onlyPinned( false )
    , m_notifiedRevisions()
    , m_waiting()
    , m_held()
    , m_firstHeldAt( 0 )
    , m_lastDeliveryAt( 0 )
    , m_hasDelivered( false )
    , m_digestArmed( false )
    , m_digestArmKnown( false )
    , m_interruptingDeliveries( 0 )
{GUCEF_TRACE;

    TEventCallback timerCallback( this, &CMatchAlertRouter::OnTimerUpdate );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, timerCallback );
}

/*-------------------------------------------------------------------------*/

CMatchAlertRouter::~CMatchAlertRouter()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_timer.SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::SetBook( CMatchBook* book )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_book )
        UnsubscribeFrom( m_book );
    m_book = book;
    if ( GUCEF_NULL == m_book )
        return;

    TEventCallback yourTurnCallback( this, &CMatchAlertRouter::OnYourTurn );
    SubscribeTo( m_book, CMatchBook::YourTurnEvent, yourTurnCallback );
    TEventCallback inviteCallback( this, &CMatchAlertRouter::OnInvite );
    SubscribeTo( m_book, CMatchBook::InviteReceivedEvent, inviteCallback );
    TEventCallback finishedCallback( this, &CMatchAlertRouter::OnFinished );
    SubscribeTo( m_book, CMatchBook::MatchFinishedEvent, finishedCallback );
    TEventCallback changedCallback( this, &CMatchAlertRouter::OnBookChanged );
    SubscribeTo( m_book, CMatchBook::MatchBookChangedEvent, changedCallback );
    m_timer.SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::OnNotify( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the book going away is the one thing here the handlers of the events do not take care of
    if ( GUCEF_NULL != m_book && notifier == static_cast< CORE::CNotifier* >( m_book ) && CORE::CINotifier::DestructionEvent == eventId )
    {
        m_book = GUCEF_NULL;
        m_timer.SetEnabled( false );
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::AddSink( CIMatchAlertSink* sink )
{GUCEF_TRACE;

    if ( GUCEF_NULL == sink )
        return;
    for ( size_t i=0; i<m_sinks.size(); ++i )
        if ( m_sinks[ i ] == sink )
            return;
    m_sinks.push_back( sink );
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::RemoveSink( CIMatchAlertSink* sink )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_sinks.size(); ++i )
    {
        if ( m_sinks[ i ] == sink )
        {
            m_sinks.erase( m_sinks.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::SetClock( const CIMatchClock* clock )
{GUCEF_TRACE;

    m_clock = clock;
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::SetMode( TMatchAlertMode mode )
{GUCEF_TRACE;

    m_mode = mode;
    m_digestArmKnown = false;
}

/*-------------------------------------------------------------------------*/

TMatchAlertMode
CMatchAlertRouter::GetMode( void ) const
{GUCEF_TRACE;

    return m_mode;
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::SetQuietHours( bool enabled, Int32 startMinute, Int32 endMinute )
{GUCEF_TRACE;

    m_quietEnabled = enabled;
    m_quietStart = startMinute;
    m_quietEnd = endMinute;
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::SetMinimumIntervalMs( UInt32 intervalMs )
{GUCEF_TRACE;

    m_minIntervalMs = intervalMs;
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::SetCoalesceWindowMs( UInt32 windowMs )
{GUCEF_TRACE;

    m_coalesceWindowMs = windowMs;
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::SetDigestMinute( Int32 minuteOfDay )
{GUCEF_TRACE;

    m_digestMinute = minuteOfDay;
    m_digestArmKnown = false;
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::SetOnlyPinned( bool onlyPinned )
{GUCEF_TRACE;

    m_onlyPinned = onlyPinned;
}

/*-------------------------------------------------------------------------*/

Int64
CMatchAlertRouter::GetNowMs( void ) const
{GUCEF_TRACE;

    return ( GUCEF_NULL != m_clock ? *m_clock : static_cast< const CIMatchClock& >( CMatchSystemClock::Instance() ) ).GetNowMs();
}

/*-------------------------------------------------------------------------*/

Int32
CMatchAlertRouter::GetMinuteOfDay( void ) const
{GUCEF_TRACE;

    return ( GUCEF_NULL != m_clock ? *m_clock : static_cast< const CIMatchClock& >( CMatchSystemClock::Instance() ) ).GetLocalMinuteOfDay();
}

/*-------------------------------------------------------------------------*/

bool
CMatchAlertRouter::IsQuietNow( void ) const
{GUCEF_TRACE;

    if ( !m_quietEnabled || m_quietStart == m_quietEnd )
        return false;
    Int32 minute = GetMinuteOfDay();
    if ( m_quietStart < m_quietEnd )
        return minute >= m_quietStart && minute < m_quietEnd;
    return minute >= m_quietStart || minute < m_quietEnd;
}

/*-------------------------------------------------------------------------*/

bool
CMatchAlertRouter::MakeMatchAlert( const CORE::CString& matchId, TMatchAlertKind kind, UInt32 revision, CMatchAlert& alert, bool& muted, bool& pinned )
{GUCEF_TRACE;

    CMatchEntry entry;
    if ( GUCEF_NULL == m_book || !m_book->GetEntry( matchId, entry ) )
        return false;

    alert = CMatchAlert();
    alert.kind = kind;
    alert.matchId = matchId;
    alert.revision = revision;
    alert.createdAt = GetNowMs();
    muted = entry.muted;
    pinned = entry.pinned;

    // an invitation is shown without its contents: the sender is not known yet
    if ( MATCHALERTKIND_INVITE != kind && entry.hasInfo )
    {
        alert.title = entry.info.title;
        const CMatchParticipant* mover = entry.info.FindParticipantByKey( entry.preview.issuerKey );
        if ( GUCEF_NULL != mover )
            alert.previousMover = mover->displayName;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::DeliverToPassiveSinks( const CMatchAlert& alert )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_sinks.size(); ++i )
        if ( !m_sinks[ i ]->IsInterrupting() )
            m_sinks[ i ]->Deliver( alert );
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::DeliverToInterruptingSinks( const CMatchAlert& alert )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_sinks.size(); ++i )
    {
        if ( m_sinks[ i ]->IsInterrupting() )
        {
            m_sinks[ i ]->Deliver( alert );
            ++m_interruptingDeliveries;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::Hold( const CMatchAlert& alert )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_held.size(); ++i )
    {
        if ( m_held[ i ].matchId == alert.matchId )
        {
            m_held[ i ] = alert;
            return;
        }
    }
    if ( m_held.empty() )
        m_firstHeldAt = GetNowMs();
    m_held.push_back( alert );
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::Flush( void )
{GUCEF_TRACE;

    if ( m_held.empty() || IsQuietNow() )
        return;
    const Int64 now = GetNowMs();

    // a clock that was set back must not hold match alerts for longer than the windows
    if ( now < m_firstHeldAt )
        m_firstHeldAt = now;
    if ( m_hasDelivered && now < m_lastDeliveryAt )
        m_lastDeliveryAt = now;
    if ( now - m_firstHeldAt < (Int64) m_coalesceWindowMs )
        return;
    if ( m_hasDelivered && now - m_lastDeliveryAt < (Int64) m_minIntervalMs )
        return;

    CMatchAlert toDeliver;
    if ( 1 == m_held.size() )
        toDeliver = m_held[ 0 ];
    else
    {
        toDeliver.kind = MATCHALERTKIND_COALESCED;
        toDeliver.count = (UInt32) m_held.size();
        toDeliver.createdAt = now;
    }
    m_held.clear();
    m_lastDeliveryAt = now;
    m_hasDelivered = true;
    DeliverToInterruptingSinks( toDeliver );
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::SendDigest( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_book )
        return;
    TMatchEntryVector entries;
    m_book->GetGroup( MATCHGROUP_YOUR_TURN, entries, false );
    UInt32 waiting = 0;
    for ( size_t i=0; i<entries.size(); ++i )
        if ( !entries[ i ].muted && ( !m_onlyPinned || entries[ i ].pinned ) )
            ++waiting;
    if ( 0 == waiting )
        return;

    CMatchAlert digest;
    digest.kind = MATCHALERTKIND_DIGEST;
    digest.count = waiting;
    digest.createdAt = GetNowMs();
    m_lastDeliveryAt = digest.createdAt;
    m_hasDelivered = true;
    DeliverToInterruptingSinks( digest );
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::Update( void )
{GUCEF_TRACE;

    Flush();

    if ( MATCHALERTMODE_DAILY_DIGEST != m_mode )
        return;
    Int32 minute = GetMinuteOfDay();
    if ( !m_digestArmKnown )
    {
        // started after the digest time: the next one is tomorrow's
        m_digestArmed = minute < m_digestMinute;
        m_digestArmKnown = true;
    }
    else
    if ( minute < m_digestMinute )
        m_digestArmed = true;

    if ( m_digestArmed && minute >= m_digestMinute && !IsQuietNow() )
    {
        m_digestArmed = false;
        SendDigest();
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::OnTimerUpdate( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Update();
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::OnYourTurn( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CMatchTurnEventData* turn = CORE::CICloneable::CloneableAs< CMatchTurnEventData >( eventData );
    if ( GUCEF_NULL == turn )
        return;

    // a match and revision is notified once
    TRevisionMap::iterator seen = m_notifiedRevisions.find( turn->matchId );
    if ( seen != m_notifiedRevisions.end() && (*seen).second >= turn->revision )
        return;
    m_notifiedRevisions[ turn->matchId ] = turn->revision;

    CMatchAlert alert;
    bool muted = false;
    bool pinned = false;
    if ( !MakeMatchAlert( turn->matchId, MATCHALERTKIND_YOUR_TURN, turn->revision, alert, muted, pinned ) )
        return;
    if ( muted || ( m_onlyPinned && !pinned ) )
        return;

    DeliverToPassiveSinks( alert );

    switch ( m_mode )
    {
        case MATCHALERTMODE_ONCE_UNTIL_I_PLAY:
        {
            if ( m_waiting.find( turn->matchId ) != m_waiting.end() )
                return;
            m_waiting.insert( turn->matchId );
            Hold( alert );
            Flush();
            break;
        }
        case MATCHALERTMODE_DAILY_DIGEST:
        {
            // the digest counts what waits when its time comes
            break;
        }
        default:
        {
            Hold( alert );
            Flush();
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::OnInvite( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* matchId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == matchId )
        return;
    const CORE::CString key = matchId->GetData() + "#invite";
    if ( m_notifiedRevisions.find( key ) != m_notifiedRevisions.end() )
        return;
    m_notifiedRevisions[ key ] = 1;

    CMatchAlert alert;
    bool muted = false;
    bool pinned = false;
    if ( !MakeMatchAlert( matchId->GetData(), MATCHALERTKIND_INVITE, 0, alert, muted, pinned ) || muted )
        return;

    DeliverToPassiveSinks( alert );
    if ( MATCHALERTMODE_DAILY_DIGEST != m_mode )
    {
        Hold( alert );
        Flush();
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::OnFinished( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the end of a match is shown inside the application, it is no reason to interrupt the player
    const CORE::TCloneableString* matchId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == matchId )
        return;
    CMatchAlert alert;
    bool muted = false;
    bool pinned = false;
    if ( MakeMatchAlert( matchId->GetData(), MATCHALERTKIND_FINISHED, 0, alert, muted, pinned ) && !muted )
        DeliverToPassiveSinks( alert );
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::ForgetMatch( const CORE::CString& matchId, bool alsoWaiting )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_held.size(); ++i )
    {
        if ( m_held[ i ].matchId == matchId )
        {
            m_held.erase( m_held.begin() + i );
            break;
        }
    }
    if ( alsoWaiting )
        m_waiting.erase( matchId );
    for ( size_t i=0; i<m_sinks.size(); ++i )
        m_sinks[ i ]->Clear( matchId );
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::ClearForMatch( const CORE::CString& matchId )
{GUCEF_TRACE;

    ForgetMatch( matchId, false );
}

/*-------------------------------------------------------------------------*/

void
CMatchAlertRouter::OnBookChanged( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CMatchBookChange* change = CORE::CICloneable::CloneableAs< CMatchBookChange >( eventData );
    if ( GUCEF_NULL == change || GUCEF_NULL == m_book )
        return;

    switch ( change->kind )
    {
        case MATCHBOOKCHANGE_OPENED:
        {
            // opening a match answers its match alerts; whoever was told once is not told again before they play
            ForgetMatch( change->matchId, false );
            break;
        }
        case MATCHBOOKCHANGE_REMOVED:
        {
            ForgetMatch( change->matchId, true );
            m_notifiedRevisions.erase( change->matchId );
            m_notifiedRevisions.erase( change->matchId + "#invite" );
            break;
        }
        default:
        {
            // played elsewhere, or muted since: what was shown is taken back
            if ( m_notifiedRevisions.find( change->matchId ) == m_notifiedRevisions.end() )
                break;
            CMatchEntry entry;
            if ( !m_book->GetEntry( change->matchId, entry ) )
                break;
            if ( MATCHGROUP_YOUR_TURN != m_book->GetGroupOf( entry ) )
                ForgetMatch( change->matchId, true );
            else
            if ( entry.muted )
                ForgetMatch( change->matchId, false );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchAlertRouter::GetHeldCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_held.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchAlertRouter::GetInterruptingDeliveryCount( void ) const
{GUCEF_TRACE;

    return m_interruptingDeliveries;
}

/*-------------------------------------------------------------------------*/

bool
CMatchAlertRouter::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    static const char* modeNames[] = { "EVERY_TURN", "ONCE_UNTIL_I_PLAY", "DAILY_DIGEST" };
    config.SetAttribute( "mode", CORE::CString( modeNames[ m_mode ] ) );
    config.SetAttribute( "quietHoursEnabled", m_quietEnabled );
    config.SetAttribute( "quietStartMinute", m_quietStart );
    config.SetAttribute( "quietEndMinute", m_quietEnd );
    config.SetAttribute( "minIntervalMs", m_minIntervalMs );
    config.SetAttribute( "coalesceWindowMs", m_coalesceWindowMs );
    config.SetAttribute( "digestMinute", m_digestMinute );
    config.SetAttribute( "onlyPinned", m_onlyPinned );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchAlertRouter::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CORE::CString mode = config.GetAttributeValueOrChildValueByName( "mode" ).AsString( CORE::CString::Empty, true ).Uppercase();
    if ( "ONCE_UNTIL_I_PLAY" == mode )
        m_mode = MATCHALERTMODE_ONCE_UNTIL_I_PLAY;
    else
    if ( "DAILY_DIGEST" == mode )
        m_mode = MATCHALERTMODE_DAILY_DIGEST;
    else
        m_mode = MATCHALERTMODE_EVERY_TURN;
    m_quietEnabled = config.GetAttributeValueOrChildValueByName( "quietHoursEnabled" ).AsBool( m_quietEnabled, true );
    m_quietStart = config.GetAttributeValueOrChildValueByName( "quietStartMinute" ).AsInt32( m_quietStart, true );
    m_quietEnd = config.GetAttributeValueOrChildValueByName( "quietEndMinute" ).AsInt32( m_quietEnd, true );
    m_minIntervalMs = config.GetAttributeValueOrChildValueByName( "minIntervalMs" ).AsUInt32( m_minIntervalMs, true );
    m_coalesceWindowMs = config.GetAttributeValueOrChildValueByName( "coalesceWindowMs" ).AsUInt32( m_coalesceWindowMs, true );
    m_digestMinute = config.GetAttributeValueOrChildValueByName( "digestMinute" ).AsInt32( m_digestMinute, true );
    m_onlyPinned = config.GetAttributeValueOrChildValueByName( "onlyPinned" ).AsBool( m_onlyPinned, true );
    m_digestArmKnown = false;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchAlertRouter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchAlertRouter";
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
