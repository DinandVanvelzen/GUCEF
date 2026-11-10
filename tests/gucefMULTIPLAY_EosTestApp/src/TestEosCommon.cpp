/*
 *  gucefMULTIPLAY_EosTestApp: Application for testing the Epic Online Services plugins of gucefMULTIPLAY, gucefDRN and gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include <stdio.h>

#include "TestEosCommon.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PumpFor( UInt32 durationInMs )
{GUCEF_TRACE;

    for ( UInt32 waited=0; waited<durationInMs; ++waited )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

bool
PumpUntil( CCondition& condition, UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( UInt32 waited=0; waited<timeoutInMs && !condition.IsMet(); ++waited )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return condition.IsMet();
}

/*-------------------------------------------------------------------------*/

bool
PumpUntilReady( const CORE::CFutureResult& future, UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( UInt32 waited=0; waited<timeoutInMs && !future.IsReady(); ++waited )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return future.IsReady();
}

/*-------------------------------------------------------------------------*/

bool
PumpUntilCount( const UInt32& counter, UInt32 value, UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( UInt32 waited=0; waited<timeoutInMs && counter < value; ++waited )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return counter >= value;
}

/*-------------------------------------------------------------------------*/

bool
LoadTestPlugin( const CORE::CString& pluginName, const CORE::CString& group )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, group, true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, group, true );
}

/*-------------------------------------------------------------------------*/

CLogCapture::CLogCapture( void )
    : m_buffer()
    , m_access( m_buffer )
    , m_logger( m_access )
{
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &m_logger );
}

/*-------------------------------------------------------------------------*/

CLogCapture::~CLogCapture()
{
    CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &m_logger );
}

/*-------------------------------------------------------------------------*/

bool
CLogCapture::Contains( const CORE::CString& text )
{
    return Text().HasSubstr( text ) >= 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLogCapture::Text( void )
{
    CORE::CLogStreamScope::FlushLogs();
    CORE::CString all;
    if ( m_buffer.GetDataSize() > 0 )
        all = CORE::CString( m_buffer.AsConstTypePtr< char >(), m_buffer.GetDataSize() );
    return all;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      RECORDERS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CLobbyRecorder::CLobbyRecorder( void )
    : CORE::CObserver()
    , stateChanged( 0 ), paths(), joined(), left(), launches( 0 ), lastLaunch(), closed( 0 ), closedReason(), refusals( 0 ), refusalWhat(), refusalReasons()
    , votesStarted( 0 ), votesEnded( 0 ), lastVotePassed( false ), lastVoteId(), countdowns(), eventThreadId( 0 ), wrongThreadEvents( 0 ), expectedThreadId( MT::GetCurrentTaskID() )
{
}

/*-------------------------------------------------------------------------*/

CLobbyRecorder::~CLobbyRecorder()
{
    SignalUpcomingObserverDestruction();
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRecorder::HasPath( const CORE::CString& path ) const
{
    for ( size_t i=0; i<paths.size(); ++i )
        if ( paths[ i ] == path )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRecorder::HasRefusal( const CORE::CString& what ) const
{
    for ( size_t i=0; i<refusalWhat.size(); ++i )
        if ( refusalWhat[ i ] == what )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CLobbyRecorder::Clear( void )
{
    stateChanged = 0;
    paths.clear();
    joined.clear();
    left.clear();
    launches = 0;
    closed = 0;
    refusals = 0;
    refusalWhat.clear();
    refusalReasons.clear();
    votesStarted = 0;
    votesEnded = 0;
    countdowns.clear();
}

/*-------------------------------------------------------------------------*/

void
CLobbyRecorder::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{
    eventThreadId = MT::GetCurrentTaskID();
    if ( eventThreadId != expectedThreadId )
        ++wrongThreadEvents;

    if ( MULTIPLAY::CILobby::LobbyStateChangedEvent == eventId )
    {
        ++stateChanged;
        const CORE::CString::StringVector& changed = static_cast< MULTIPLAY::CILobby::TLobbyStateChangedEventData* >( eventData )->GetData();
        for ( size_t i=0; i<changed.size(); ++i )
            paths.push_back( changed[ i ] );
    }
    else
    if ( MULTIPLAY::CILobby::MemberJoinedEvent == eventId )
        joined.push_back( static_cast< MULTIPLAY::CILobby::TMemberEventData* >( eventData )->GetData() );
    else
    if ( MULTIPLAY::CILobby::MemberLeftEvent == eventId )
        left.push_back( static_cast< MULTIPLAY::CILobby::TMemberEventData* >( eventData )->GetData() );
    else
    if ( MULTIPLAY::CILobby::LaunchEvent == eventId )
    {
        ++launches;
        lastLaunch = *static_cast< MULTIPLAY::CLaunchDescription* >( eventData );
    }
    else
    if ( MULTIPLAY::CILobby::LobbyClosedEvent == eventId )
    {
        ++closed;
        closedReason = static_cast< MULTIPLAY::CILobby::TLobbyClosedEventData* >( eventData )->GetData();
    }
    else
    if ( MULTIPLAY::CILobby::RequestRefusedEvent == eventId )
    {
        const MULTIPLAY::CLobbyRefusalEventData* refusal = static_cast< MULTIPLAY::CLobbyRefusalEventData* >( eventData );
        ++refusals;
        refusalWhat.push_back( refusal->what );
        refusalReasons.push_back( refusal->reason );
    }
    else
    if ( MULTIPLAY::CILobby::VoteStartedEvent == eventId )
    {
        ++votesStarted;
        lastVoteId = static_cast< MULTIPLAY::CLobbyVoteEventData* >( eventData )->vote.id;
    }
    else
    if ( MULTIPLAY::CILobby::VoteEndedEvent == eventId )
    {
        ++votesEnded;
        lastVotePassed = static_cast< MULTIPLAY::CLobbyVoteEventData* >( eventData )->passed;
    }
    else
    if ( MULTIPLAY::CILobby::CountdownEvent == eventId )
        countdowns.push_back( static_cast< MULTIPLAY::CILobby::TCountdownEventData* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

CServiceRecorder::CServiceRecorder( void )
    : CORE::CObserver()
    , connected( 0 ), disconnected( 0 ), lobbyJoined( 0 ), lastLobby(), invites( 0 ), lastInviteLobby(), lastInviteSender(), watcher( GUCEF_NULL )
{
}

/*-------------------------------------------------------------------------*/

CServiceRecorder::~CServiceRecorder()
{
    SignalUpcomingObserverDestruction();
}

/*-------------------------------------------------------------------------*/

void
CServiceRecorder::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{
    if ( MULTIPLAY::CILobbyService::ServiceConnectedEvent == eventId )
        ++connected;
    else
    if ( MULTIPLAY::CILobbyService::ServiceDisconnectedEvent == eventId )
        ++disconnected;
    else
    if ( MULTIPLAY::CILobbyService::LobbyJoinedEvent == eventId )
    {
        ++lobbyJoined;
        lastLobby = static_cast< MULTIPLAY::CLobbyResultData* >( eventData )->GetData();
        if ( GUCEF_NULL != watcher && !lastLobby.IsNULL() )
            watcher->SubscribeTo( lastLobby.GetPointerAlways() );
    }
    else
    if ( MULTIPLAYPLUGIN::EOS::CEosLobbyService::InviteReceivedEvent == eventId )
    {
        const MULTIPLAYPLUGIN::EOS::CEosInviteEventData* invite = static_cast< MULTIPLAYPLUGIN::EOS::CEosInviteEventData* >( eventData );
        ++invites;
        lastInviteLobby = invite->lobbyId;
        lastInviteSender = invite->senderUserId;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE FIXTURE                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

void
EnsureGlueSettings( void )
{GUCEF_TRACE;

    // The settings of the product that every platform of the tests is made with
    PLUGINGLUE::EOS::CEosPlatformSettings settings;
    settings.clientId = "xyz-client-id";
    settings.clientSecret.Set( "EOS-CLIENT-SECRET-5d8e0c47-DO-NOT-LOG" );
    settings.productId = "product-1";
    settings.sandboxId = "sandbox-1";
    settings.deploymentId = "deployment-1";
    PLUGINGLUE::EOS::CEosGlue::Instance()->SetSettings( settings );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
MakeId( const char* prefix, UInt32 index )
{
    // the index is a small number: one byte of lower case hex
    const UInt8 indexByte = (UInt8) index;
    return CORE::CString( prefix ) + CORE::Base16Encode( &indexByte, 1 ).Lowercase();
}

/*-------------------------------------------------------------------------*/

CEosPeerContext::CEosPeerContext( EosFake::CFakeEosWorld& world, UInt32 index )
    : peer( world, MakeId( "00112233445566778899aabbccdd", 0x10 + index ), MakeId( "0123456789abcdef0123456789ab", 0x10 + index ) )
    , platform( GUCEF_NULL )
    , identity()
    , service()
    , serviceRecorder()
    , reason()
{
    identity.userId = "profile-" + CORE::UInt32ToString( index );
    identity.displayName = "Player " + CORE::UInt32ToString( index );
    EnsureGlueSettings();
    serviceRecorder.SubscribeTo( &service );
    service.SetSettings( CEosFixture::MakeFastSettings() );
}

/*-------------------------------------------------------------------------*/

CEosPeerContext::~CEosPeerContext()
{
    serviceRecorder.UnsubscribeAllFromObserver();
    service.Disconnect();
    if ( GUCEF_NULL != platform )
    {
        platform->ClearLoginState();
        PLUGINGLUE::EOS::CEosGlue::Instance()->ReleasePlatform( platform );
    }
}

/*-------------------------------------------------------------------------*/

bool
CEosPeerContext::Login( void )
{GUCEF_TRACE;

    EnsureGlueSettings();
    PLUGINGLUE::EOS::SEosPlatformShim shim = peer.PlatformShim();
    CORE::CString problem;
    platform = PLUGINGLUE::EOS::CEosGlue::Instance()->AcquirePlatform( &shim, GUCEF_NULL, problem );
    if ( GUCEF_NULL == platform )
    {
        reason = problem;
        return false;
    }
    platform->SetLoginState( peer.epicAccountId, peer.puid );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosPeerContext::Connect( void )
{GUCEF_TRACE;

    PLUGINGLUE::EOS::SEosP2PShim p2p = peer.P2PShim();
    service.SetShim( peer.LobbyShim(), &p2p );
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    bool started = service.Connect( identity, future );
    if ( !PumpUntilReady( future, 3000 ) )
    {
        reason = "no answer";
        return false;
    }
    CORE::CTaskPtr task = future.GetResult( 0 );
    if ( task.IsNULL() || CORE::TTaskStatus::TASKSTATUS_FINISHED != task->GetTaskStatus() )
    {
        reason = task.IsNULL() ? CORE::CString( "no task" ) : task->GetTaskStatusExtraInfo();
        return false;
    }
    return started && service.IsConnected();
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CILobbyPtr
CEosPeerContext::Create( const MULTIPLAY::CLobbyCreateParams& params )
{GUCEF_TRACE;

    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    service.CreateLobby( params, future );
    if ( !PumpUntilReady( future, 5000 ) )
    {
        reason = "no answer";
        return MULTIPLAY::CILobbyPtr();
    }
    MULTIPLAY::CILobbyPtr lobby;
    if ( !MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, lobby ) )
    {
        CORE::CTaskPtr task = future.GetResult( 0 );
        reason = task.IsNULL() ? CORE::CString( "no task" ) : task->GetTaskStatusExtraInfo();
        return MULTIPLAY::CILobbyPtr();
    }
    return lobby;
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CILobbyPtr
CEosPeerContext::Join( const CORE::CString& lobbyId, const CORE::CString& password, bool asSpectator )
{GUCEF_TRACE;

    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    service.JoinLobby( lobbyId, password, asSpectator, future );
    if ( !PumpUntilReady( future, 5000 ) )
    {
        reason = "no answer";
        return MULTIPLAY::CILobbyPtr();
    }
    MULTIPLAY::CILobbyPtr lobby;
    if ( !MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, lobby ) )
    {
        CORE::CTaskPtr task = future.GetResult( 0 );
        reason = task.IsNULL() ? CORE::CString( "no task" ) : task->GetTaskStatusExtraInfo();
        return MULTIPLAY::CILobbyPtr();
    }
    return lobby;
}

/*-------------------------------------------------------------------------*/

MULTIPLAYPLUGIN::EOS::CEosLobbyServiceSettings
CEosFixture::MakeFastSettings( void )
{GUCEF_TRACE;

    MULTIPLAYPLUGIN::EOS::CEosLobbyServiceSettings settings;
    settings.tickIntervalInMs = 2;
    settings.secondInMs = 40;
    settings.requestTimeoutInMs = 2000;
    settings.joinTimeoutInMs = 2000;
    settings.joinGraceInMs = 400;
    settings.departureGraceInMs = 100;
    settings.launchTimeoutInMs = 2000;
    settings.updatesPerSecond = 1000;
    settings.updateBurst = 1000;
    settings.rules.minPlayers = 2;
    settings.rules.countdownInSecs = 2;
    settings.rules.voteDurationInSecs = 2;
    settings.rules.requireAllReady = true;
    settings.rules.allowForceStart = true;
    settings.rules.startByVote = true;
    return settings;
}

/*-------------------------------------------------------------------------*/

CEosFixture::CEosFixture( UInt32 peerCount, bool connect )
    : world()
    , peers()
{
    for ( UInt32 i=0; i<peerCount; ++i )
    {
        CEosPeerContext* context = GUCEF_NEW CEosPeerContext( world, i );
        peers.push_back( context );
        if ( connect )
        {
            context->Login();
            context->Connect();
        }
    }
}

/*-------------------------------------------------------------------------*/

CEosFixture::~CEosFixture()
{
    // The lobbies of the tests go before their services do
    PumpFor( 5 );
    for ( size_t i=0; i<peers.size(); ++i )
        GUCEF_DELETE peers[ i ];
    peers.clear();
}

/*-------------------------------------------------------------------------*/

void
CEosFixture::ApplySettings( const MULTIPLAYPLUGIN::EOS::CEosLobbyServiceSettings& settings )
{
    for ( size_t i=0; i<peers.size(); ++i )
        peers[ i ]->service.SetSettings( settings );
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CLobbyCreateParams
CEosFixture::MakeParams( const CORE::CString& name )
{GUCEF_TRACE;

    MULTIPLAY::CLobbyCreateParams params;
    params.name = name;
    params.gameId = "testgame";
    params.gameVersion = "1.0";
    params.maxPlayers = 4;
    params.allowSpectators = true;
    params.isPublic = true;
    params.topology = MULTIPLAY::GAMETOPOLOGY_CLIENT_SERVER;

    MULTIPLAY::CLobbySettingDef map;
    map.key = "map";
    map.type = MULTIPLAY::CLobbySettingDef::TYPE_ENUM;
    map.options.push_back( "dust" );
    map.options.push_back( "ice" );
    map.defaultValue = "dust";
    map.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
    params.schema.AddSetting( map );
    MULTIPLAY::CLobbySettingDef speed;
    speed.key = "speed";
    speed.type = MULTIPLAY::CLobbySettingDef::TYPE_INT;
    speed.hasRange = true;
    speed.minValue = 1;
    speed.maxValue = 5;
    speed.defaultValue = "3";
    speed.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_VOTE;
    params.schema.AddSetting( speed );
    MULTIPLAY::CLobbySettingDef handicap;
    handicap.key = "handicap";
    handicap.type = MULTIPLAY::CLobbySettingDef::TYPE_INT;
    handicap.hasRange = true;
    handicap.minValue = 0;
    handicap.maxValue = 100;
    handicap.defaultValue = "0";
    handicap.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_OWNER;
    params.schema.AddSetting( handicap );
    MULTIPLAY::CLobbySettingDef note;
    note.key = "note";
    note.type = MULTIPLAY::CLobbySettingDef::TYPE_STRING;
    note.defaultValue = "";
    note.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
    params.schema.AddSetting( note );
    params.schema.factions.push_back( "arm" );
    params.schema.factions.push_back( "core" );
    params.schema.colours.push_back( "red" );
    params.schema.colours.push_back( "blue" );
    params.schema.colours.push_back( "green" );
    params.schema.colours.push_back( "yellow" );
    params.schema.uniqueColours = true;
    return params;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      HELPERS OF THE TESTS                                               //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF::MULTIPLAYPLUGIN::EOS;

CORE::CString
FailureOf( const CORE::CFutureResult& future )
{
    CORE::CTaskPtr task = future.GetResult( 0 );
    return task.IsNULL() ? CORE::CString( "no task" ) : task->GetTaskStatusExtraInfo();
}

/*-------------------------------------------------------------------------*/

bool
Browse( CEosPeerContext& context, const MULTIPLAY::CGameListingFilter& filter, MULTIPLAY::CGameListingList& list, CORE::CString* failure )
{
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    context.service.Browse( filter, future );
    if ( !PumpUntilReady( future, 5000 ) )
        return false;
    if ( !MULTIPLAY::CGameListingList::TryGetFromFuture( future, list ) )
    {
        if ( GUCEF_NULL != failure )
            *failure = FailureOf( future );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
LobbyIdOf( const MULTIPLAY::CILobbyPtr& lobby )
{
    return static_cast< const CEosLobby* >( lobby.GetPointerAlways() )->GetLobbyId();
}

/*-------------------------------------------------------------------------*/

MULTIPLAYPLUGIN::EOS::CEosLobby&
Eos( const MULTIPLAY::CILobbyPtr& lobby )
{
    return *const_cast< CEosLobby* >( static_cast< const CEosLobby* >( lobby.GetPointerAlways() ) );
}

/*-------------------------------------------------------------------------*/

bool
MakePair( CEosFixture& fixture, SPair& pair, const MULTIPLAY::CLobbyCreateParams& params )
{
    pair.host = fixture[ 0 ].Create( params );
    if ( pair.host.IsNULL() )
        return false;
    pair.hostEvents.SubscribeTo( pair.host.GetPointerAlways() );
    pair.guest = fixture[ 1 ].Join( LobbyIdOf( pair.host ), params.password );
    if ( pair.guest.IsNULL() )
        return false;
    pair.guestEvents.SubscribeTo( pair.guest.GetPointerAlways() );
    CMemberCount both( pair.host, 2 );
    return PumpUntil( both, 3000 );
}

/** Whether the text is in an attribute of the lobby or of a member of it */
bool
IsInAnyAttribute( EosFake::SFakeLobby& lobby, const CORE::CString& text )
{
    for ( size_t i=0; i<lobby.attributes.size(); ++i )
        if ( lobby.attributes[ i ].stringValue.HasSubstr( text ) >= 0 || lobby.attributes[ i ].key.HasSubstr( text ) >= 0 )
            return true;
    for ( size_t m=0; m<lobby.members.size(); ++m )
        for ( size_t i=0; i<lobby.members[ m ].attributes.size(); ++i )
            if ( lobby.members[ m ].attributes[ i ].stringValue.HasSubstr( text ) >= 0 )
                return true;
    return false;
}

/*-------------------------------------------------------------------------*/
