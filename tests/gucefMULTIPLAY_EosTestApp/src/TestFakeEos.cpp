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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORM_H
#include "pluginglueEOS_CEosPlatform.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORM_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORM_H ? */

#include "TestFakeEos.h"

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::EOS;

namespace EosFake {

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE WORLD                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

SFailures::SFailures( void )
    : create( EOSRESULT_SUCCESS ), update( EOSRESULT_SUCCESS ), join( EOSRESULT_SUCCESS ), search( EOSRESULT_SUCCESS )
    , kick( EOSRESULT_SUCCESS ), promote( EOSRESULT_SUCCESS ), updateCount( 0 ), neverAnswer( false ), noConnection( false )
{
}

/*-------------------------------------------------------------------------*/

SFakeLobby::SFakeLobby( void )
    : id(), owner(), bucketId(), maxMembers( 4 ), permission( EOSLOBBYPERM_PUBLIC_ADVERTISED ), allowInvites( true ), enableJoinById( true )
    , disableHostMigration( false ), rejoinAfterKickRequiresInvite( true ), attributes(), members(), kicked(), invited()
{
}

/*-------------------------------------------------------------------------*/

SNetworkModel::SNetworkModel( void )
    : lossRate( 0 ), duplicateRate( 0 ), reorderRate( 0 ), latencyMs( 0 ), reorderMs( 20 ), relayExtraMs( 15 ), directBlocked( false ), pendingTimeoutMs( 10000 )
{
}

/*-------------------------------------------------------------------------*/

CFakeEosWorld::CFakeEosWorld( void )
    : maxAttributes( EosDefaultMaxLobbyAttributes ), maxAttributeLength( EosDefaultMaxAttributeLength ), maxKeyLength( EosDefaultMaxAttributeKeyLength )
    , maxMembers( EosDefaultMaxLobbyMembers ), network(), packetsSent( 0 ), packetsDelivered( 0 ), packetsDropped( 0 ), packetsDuplicated( 0 )
    , relayedConnections( 0 ), directConnections( 0 ), refusedConnections( 0 ), largestPacket( 0 ), m_connections(), m_peers(), m_lobbies(), m_nextLobby( 0 ), m_random( 0x2545F491u )
{
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeEosWorld::NextRandom( void )
{
    m_random ^= m_random << 13;
    m_random ^= m_random >> 17;
    m_random ^= m_random << 5;
    return m_random;
}

/*-------------------------------------------------------------------------*/

double
CFakeEosWorld::NextRandomUnit( void )
{
    return ( NextRandom() & 0xFFFFFF ) / 16777216.0;
}

/*-------------------------------------------------------------------------*/

void
CFakeEosWorld::AddPeer( CFakeEosPeer* peer )
{
    m_peers.push_back( peer );
}

/*-------------------------------------------------------------------------*/

void
CFakeEosWorld::RemovePeer( CFakeEosPeer* peer )
{
    for ( size_t i=0; i<m_peers.size(); ++i )
        if ( m_peers[ i ] == peer )
        {
            m_peers.erase( m_peers.begin() + i );
            return;
        }
}

/*-------------------------------------------------------------------------*/

CFakeEosPeer*
CFakeEosWorld::FindPeer( const CORE::CString& puid ) const
{
    for ( size_t i=0; i<m_peers.size(); ++i )
        if ( m_peers[ i ]->puid == puid )
            return m_peers[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

SFakeLobby*
CFakeEosWorld::FindLobby( const CORE::CString& lobbyId )
{
    GUCEF::map< CORE::CString, SFakeLobby >::iterator i = m_lobbies.find( lobbyId );
    return i == m_lobbies.end() ? GUCEF_NULL : &(*i).second;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeEosWorld::GetLobbyCount( void ) const
{
    return (UInt32) m_lobbies.size();
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CFakeEosWorld::GetLobbyIds( void ) const
{
    CORE::CString::StringVector ids;
    for ( GUCEF::map< CORE::CString, SFakeLobby >::const_iterator i = m_lobbies.begin(); i != m_lobbies.end(); ++i )
        ids.push_back( (*i).first );
    return ids;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeEosWorld::GetConnectionCount( void ) const
{
    return (UInt32) m_connections.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeEosWorld::GetEstablishedConnectionCount( void ) const
{
    UInt32 count = 0;
    for ( size_t i=0; i<m_connections.size(); ++i )
        if ( m_connections[ i ].established )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

static void
SetAttribute( TEosAttributeVector& target, const SEosAttribute& attribute )
{
    for ( size_t i=0; i<target.size(); ++i )
    {
        if ( target[ i ].key == attribute.key )
        {
            target[ i ] = attribute;
            return;
        }
    }
    target.push_back( attribute );
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosWorld::ForgeLobbyAttribute( const CORE::CString& lobbyId, const SEosAttribute& attribute )
{
    SFakeLobby* lobby = FindLobby( lobbyId );
    if ( GUCEF_NULL == lobby )
        return false;
    SetAttribute( lobby->attributes, attribute );
    for ( size_t i=0; i<lobby->members.size(); ++i )
    {
        CFakeEosPeer* peer = FindPeer( lobby->members[ i ].puid );
        if ( GUCEF_NULL != peer )
        {
            SEosLobbyEvent event;
            event.kind = EOSLOBBYEVENT_LOBBY_UPDATED;
            event.result = EOSRESULT_SUCCESS;
            event.lobbyId = lobbyId;
            peer->QueueLobbyEvent( event );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosWorld::ForgeMemberAttribute( const CORE::CString& lobbyId, const CORE::CString& puid, const SEosAttribute& attribute )
{
    SFakeLobby* lobby = FindLobby( lobbyId );
    if ( GUCEF_NULL == lobby )
        return false;
    for ( size_t i=0; i<lobby->members.size(); ++i )
    {
        if ( lobby->members[ i ].puid == puid )
        {
            SetAttribute( lobby->members[ i ].attributes, attribute );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE PEER                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

CFakeEosPeer::CFakeEosPeer( CFakeEosWorld& w, const CORE::CString& id, const CORE::CString& epic )
    : world( w ), puid( id ), epicAccountId( epic )
    , sdkAvailable( true ), platformOk( true ), platformError(), platformReady( false ), createPlatformCalls( 0 ), releaseCalls( 0 ), tickCalls( 0 )
    , configProductId(), configClientSecret()
    , failures(), latencyMs( 0 ), echoToUpdater( false ), relayControl( EOSRELAY_ALLOW_RELAYS )
    , updateCalls( 0 ), searchCalls( 0 ), createCalls( 0 ), joinCalls( 0 ), leaveCalls( 0 ), destroyCalls( 0 ), kickCalls( 0 ), promoteCalls( 0 ), inviteCalls( 0 )
    , largestUpdateAttributes( 0 ), packetsSent( 0 ), packetsReceived( 0 ), acceptCalls( 0 ), closeCalls( 0 ), lastSearchBucket(), lastSearch(), lastCreate()
    , m_lobbyEvents(), m_p2pEvents(), m_inbox(), m_authEvents(), m_neverAnswered( 0 ), m_failedUpdates( 0 )
{
    world.AddPeer( this );
}

/*-------------------------------------------------------------------------*/

CFakeEosPeer::~CFakeEosPeer()
{
    world.RemovePeer( this );
}

/*-------------------------------------------------------------------------*/

UInt64
CFakeEosPeer::Now( void ) const
{
    return CEosPlatform::GetMonotonicTimeInMs();
}

/*-------------------------------------------------------------------------*/

SEosPlatformShim
CFakeEosPeer::PlatformShim( void )
{
    SEosPlatformShim shim;
    shim.context = this;
    shim.isSdkAvailable = &CFakeEosPeer::IsSdkAvailable;
    shim.createPlatform = &CFakeEosPeer::CreatePlatform;
    shim.releasePlatform = &CFakeEosPeer::ReleasePlatform;
    shim.isPlatformReady = &CFakeEosPeer::IsPlatformReady;
    shim.tick = &CFakeEosPeer::Tick;
    return shim;
}

/*-------------------------------------------------------------------------*/

SEosLobbyShim
CFakeEosPeer::LobbyShim( void )
{
    SEosLobbyShim shim;
    shim.platform = PlatformShim();
    shim.context = this;
    shim.createLobby = &CFakeEosPeer::CreateLobby;
    shim.updateLobby = &CFakeEosPeer::UpdateLobby;
    shim.joinLobbyById = &CFakeEosPeer::JoinLobbyById;
    shim.leaveLobby = &CFakeEosPeer::LeaveLobby;
    shim.destroyLobby = &CFakeEosPeer::DestroyLobby;
    shim.kickMember = &CFakeEosPeer::KickMember;
    shim.promoteMember = &CFakeEosPeer::PromoteMember;
    shim.sendInvite = &CFakeEosPeer::SendInvite;
    shim.searchLobbies = &CFakeEosPeer::SearchLobbies;
    shim.copyLobbyDetails = &CFakeEosPeer::CopyLobbyDetails;
    shim.pollEvent = &CFakeEosPeer::PollLobbyEvent;
    return shim;
}

/*-------------------------------------------------------------------------*/

SEosP2PShim
CFakeEosPeer::P2PShim( void )
{
    SEosP2PShim shim;
    shim.platform = PlatformShim();
    shim.context = this;
    shim.sendPacket = &CFakeEosPeer::SendPacket;
    shim.receivePacket = &CFakeEosPeer::ReceivePacket;
    shim.acceptConnection = &CFakeEosPeer::AcceptConnection;
    shim.closeConnection = &CFakeEosPeer::CloseConnection;
    shim.closeConnections = &CFakeEosPeer::CloseConnections;
    shim.pollEvent = &CFakeEosPeer::PollP2PEvent;
    shim.setRelayControl = &CFakeEosPeer::SetRelayControl;
    shim.getNatType = &CFakeEosPeer::GetNatType;
    shim.setPacketQueueSize = &CFakeEosPeer::SetPacketQueueSize;
    shim.setLocalUser = &CFakeEosPeer::SetLocalUser;
    return shim;
}

/*-------------------------------------------------------------------------*/

AUTHPLUGIN::EOS::TEosShim
CFakeEosPeer::AuthShim( void )
{
    AUTHPLUGIN::EOS::TEosShim shim;
    SEosPlatformShim platform = PlatformShim();
    static_cast< SEosPlatformShim& >( shim ) = platform;
    shim.pollEvent = &CFakeEosPeer::PollAuthEvent;
    shim.authLogin = &CFakeEosPeer::AuthLogin;
    shim.authCopyIdToken = &CFakeEosPeer::AuthCopyIdToken;
    shim.connectLogin = &CFakeEosPeer::ConnectLogin;
    shim.connectCreateUser = &CFakeEosPeer::ConnectCreateUser;
    shim.connectLinkAccount = &CFakeEosPeer::ConnectLinkAccount;
    shim.authLogout = &CFakeEosPeer::AuthLogout;
    return shim;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeEosPeer::GetPendingRequestCount( void ) const
{
    UInt32 count = m_neverAnswered;
    for ( size_t i=0; i<m_lobbyEvents.size(); ++i )
        if ( 0 != m_lobbyEvents[ i ].event.requestId )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
CFakeEosPeer::DropOffTheNetwork( void )
{
    failures.noConnection = true;
    CORE::CString::StringVector ids = world.GetLobbyIds();
    for ( size_t i=0; i<ids.size(); ++i )
    {
        SFakeLobby* lobby = world.FindLobby( ids[ i ] );
        if ( GUCEF_NULL == lobby )
            continue;
        for ( size_t m=0; m<lobby->members.size(); ++m )
        {
            if ( lobby->members[ m ].puid == puid )
            {
                RemoveMember( *lobby, puid, EOSMEMBERSTATUS_DISCONNECTED );
                break;
            }
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE PLATFORM                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

bool CFakeEosPeer::IsSdkAvailable( void* context ) { return Self( context )->sdkAvailable; }

bool
CFakeEosPeer::CreatePlatform( void* context, const SEosPlatformConfig* config, char* errorText, UInt32 errorTextSize )
{
    CFakeEosPeer* self = Self( context );
    ++self->createPlatformCalls;
    if ( !self->platformOk )
    {
        strncpy( errorText, self->platformError.IsNULLOrEmpty() ? "fake: platform failed" : self->platformError.C_String(), errorTextSize - 1 );
        errorText[ errorTextSize - 1 ] = '\0';
        return false;
    }
    self->configProductId = config->productId;
    self->configClientSecret = config->clientSecret;
    self->platformReady = true;
    return true;
}

void CFakeEosPeer::ReleasePlatform( void* context ) { CFakeEosPeer* self = Self( context ); ++self->releaseCalls; self->platformReady = false; }
bool CFakeEosPeer::IsPlatformReady( void* context ) { return Self( context )->platformReady; }

void
CFakeEosPeer::Tick( void* context )
{
    CFakeEosPeer* self = Self( context );
    ++self->tickCalls;
    self->ExpireConnections();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      REQUESTS                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CFakeEosPeer::QueueLobbyEvent( const SEosLobbyEvent& event )
{
    SLobbyEntry entry;
    entry.dueInMs = Now() + latencyMs;
    entry.event = event;
    m_lobbyEvents.push_back( entry );
}

/*-------------------------------------------------------------------------*/

void
CFakeEosPeer::QueueEnd( UInt32 requestId, EEosLobbyEventKind kind, EEosResult result, const CORE::CString& lobbyId )
{
    SEosLobbyEvent event;
    event.requestId = requestId;
    event.kind = kind;
    event.result = result;
    event.sdkResultCode = EOSRESULT_SUCCESS == result ? 0 : 1;
    event.lobbyId = lobbyId;
    QueueLobbyEvent( event );
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::Preempt( UInt32 requestId, EEosLobbyEventKind kind, const CORE::CString& lobbyId, EEosResult overrideResult )
{
    if ( failures.neverAnswer )
    {
        ++m_neverAnswered;
        return true;
    }
    if ( failures.noConnection )
    {
        QueueEnd( requestId, kind, EOSRESULT_NO_CONNECTION, lobbyId );
        return true;
    }
    if ( EOSRESULT_SUCCESS != overrideResult )
    {
        QueueEnd( requestId, kind, overrideResult, lobbyId );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CFakeEosPeer::NotifyMembers( SFakeLobby& lobby, const CORE::CString& except, SEosLobbyEvent event )
{
    event.requestId = 0;
    event.lobbyId = lobby.id;
    event.result = EOSRESULT_SUCCESS;
    for ( size_t i=0; i<lobby.members.size(); ++i )
    {
        if ( lobby.members[ i ].puid == except )
            continue;
        CFakeEosPeer* peer = world.FindPeer( lobby.members[ i ].puid );
        if ( GUCEF_NULL != peer )
            peer->QueueLobbyEvent( event );
    }
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::CreateLobby( void* context, UInt32 requestId, const SEosLobbyCreateOptions* options )
{
    CFakeEosPeer* self = Self( context );
    ++self->createCalls;
    self->lastCreate = *options;
    if ( self->Preempt( requestId, EOSLOBBYEVENT_CREATE, CORE::CString::Empty, self->failures.create ) )
        return true;
    if ( !self->platformReady || options->localUser != self->puid || options->maxMembers < 1 || options->maxMembers > self->world.maxMembers )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_CREATE, EOSRESULT_INVALID_PARAMETERS, CORE::CString::Empty );
        return true;
    }

    SFakeLobby lobby;
    UInt32 idWords[ 4 ];
    idWords[ 0 ] = self->world.NextRandom();
    idWords[ 1 ] = self->world.NextRandom();
    idWords[ 2 ] = self->world.NextRandom();
    idWords[ 3 ] = ++self->world.m_nextLobby;
    UInt8 idBytes[ 16 ];
    for ( UInt32 w = 0; w < 4; ++w )
    {
        idBytes[ w * 4 ] = (UInt8) ( idWords[ w ] >> 24 );
        idBytes[ w * 4 + 1 ] = (UInt8) ( idWords[ w ] >> 16 );
        idBytes[ w * 4 + 2 ] = (UInt8) ( idWords[ w ] >> 8 );
        idBytes[ w * 4 + 3 ] = (UInt8) idWords[ w ];
    }
    lobby.id = CORE::Base16Encode( idBytes, 16 ).Lowercase();
    lobby.owner = self->puid;
    lobby.bucketId = options->bucketId;
    lobby.maxMembers = options->maxMembers;
    lobby.permission = options->permission;
    lobby.allowInvites = options->allowInvites;
    lobby.enableJoinById = options->enableJoinById;
    lobby.disableHostMigration = options->disableHostMigration;
    lobby.rejoinAfterKickRequiresInvite = options->rejoinAfterKickRequiresInvite;
    SFakeMember member;
    member.puid = self->puid;
    lobby.members.push_back( member );
    self->world.m_lobbies[ lobby.id ] = lobby;
    self->QueueEnd( requestId, EOSLOBBYEVENT_CREATE, EOSRESULT_SUCCESS, lobby.id );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::ApplyAttributes( TEosAttributeVector& target, const TEosAttributeVector& set, const CORE::CString::StringVector& remove, EEosResult& result )
{
    TEosAttributeVector copy = target;
    for ( size_t i=0; i<set.size(); ++i )
    {
        if ( set[ i ].key.ByteSize() - 1 > world.maxKeyLength || set[ i ].key.ByteSize() < 2 )
        {
            result = EOSRESULT_LIMIT_EXCEEDED;
            return false;
        }
        if ( EOSATTRIBUTE_STRING == set[ i ].type && set[ i ].stringValue.ByteSize() - 1 > world.maxAttributeLength )
        {
            result = EOSRESULT_LIMIT_EXCEEDED;
            return false;
        }
        SetAttribute( copy, set[ i ] );
    }
    for ( size_t i=0; i<remove.size(); ++i )
        for ( size_t n=0; n<copy.size(); ++n )
            if ( copy[ n ].key == remove[ i ] )
            {
                copy.erase( copy.begin() + n );
                break;
            }
    if ( copy.size() > world.maxAttributes )
    {
        result = EOSRESULT_LIMIT_EXCEEDED;
        return false;
    }
    target = copy;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::UpdateLobby( void* context, UInt32 requestId, const SEosLobbyUpdateOptions* options )
{
    CFakeEosPeer* self = Self( context );
    ++self->updateCalls;
    if ( options->setAttributes.size() > self->largestUpdateAttributes )
        self->largestUpdateAttributes = (UInt32) options->setAttributes.size();

    EEosResult injected = EOSRESULT_SUCCESS;
    if ( EOSRESULT_SUCCESS != self->failures.update )
    {
        if ( 0 == self->failures.updateCount )
            injected = self->failures.update;
        else
        if ( self->m_failedUpdates < self->failures.updateCount )
        {
            ++self->m_failedUpdates;
            injected = self->failures.update;
        }
    }
    if ( self->Preempt( requestId, EOSLOBBYEVENT_UPDATE, options->lobbyId, injected ) )
        return true;

    SFakeLobby* lobby = self->world.FindLobby( options->lobbyId );
    SFakeMember* member = GUCEF_NULL;
    if ( GUCEF_NULL != lobby )
        for ( size_t i=0; i<lobby->members.size(); ++i )
            if ( lobby->members[ i ].puid == options->localUser )
                member = &lobby->members[ i ];
    if ( GUCEF_NULL == lobby || GUCEF_NULL == member || options->localUser != self->puid )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_UPDATE, EOSRESULT_NOT_FOUND, options->lobbyId );
        return true;
    }

    bool lobbyChange = options->setPermission || options->setMaxMembers || options->setAllowInvites || !options->setAttributes.empty() || !options->removeAttributes.empty();
    if ( lobbyChange && lobby->owner != self->puid )
    {
        // Only the owner changes the lobby: nothing of the update is done
        self->QueueEnd( requestId, EOSLOBBYEVENT_UPDATE, EOSRESULT_NO_PERMISSION, options->lobbyId );
        return true;
    }

    // All or nothing
    SFakeLobby changed = *lobby;
    SFakeMember* changedMember = GUCEF_NULL;
    for ( size_t i=0; i<changed.members.size(); ++i )
        if ( changed.members[ i ].puid == self->puid )
            changedMember = &changed.members[ i ];
    EEosResult result = EOSRESULT_SUCCESS;
    if ( !self->ApplyAttributes( changed.attributes, options->setAttributes, options->removeAttributes, result ) ||
         !self->ApplyAttributes( changedMember->attributes, options->setMemberAttributes, options->removeMemberAttributes, result ) )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_UPDATE, result, options->lobbyId );
        return true;
    }
    if ( options->setPermission )
        changed.permission = options->permission;
    if ( options->setMaxMembers )
        changed.maxMembers = options->maxMembers;
    if ( options->setAllowInvites )
        changed.allowInvites = options->allowInvites;
    *lobby = changed;

    if ( lobbyChange )
    {
        SEosLobbyEvent event;
        event.kind = EOSLOBBYEVENT_LOBBY_UPDATED;
        self->NotifyMembers( *lobby, self->echoToUpdater ? CORE::CString::Empty : self->puid, event );
    }
    if ( !options->setMemberAttributes.empty() || !options->removeMemberAttributes.empty() )
    {
        SEosLobbyEvent event;
        event.kind = EOSLOBBYEVENT_MEMBER_UPDATED;
        event.userId = self->puid;
        self->NotifyMembers( *lobby, self->echoToUpdater ? CORE::CString::Empty : self->puid, event );
    }
    self->QueueEnd( requestId, EOSLOBBYEVENT_UPDATE, EOSRESULT_SUCCESS, options->lobbyId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::JoinLobbyById( void* context, UInt32 requestId, const char* lobbyId, const char* localUser, bool presenceEnabled )
{
    CFakeEosPeer* self = Self( context );
    ++self->joinCalls;
    if ( self->Preempt( requestId, EOSLOBBYEVENT_JOIN, lobbyId, self->failures.join ) )
        return true;

    SFakeLobby* lobby = self->world.FindLobby( lobbyId );
    if ( GUCEF_NULL == lobby || !self->platformReady || self->puid != localUser )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_JOIN, EOSRESULT_NOT_FOUND, lobbyId );
        return true;
    }
    bool invited = lobby->invited.find( self->puid ) != lobby->invited.end();
    bool wasKicked = lobby->kicked.find( self->puid ) != lobby->kicked.end();
    if ( ( !lobby->enableJoinById && !invited ) || ( wasKicked && lobby->rejoinAfterKickRequiresInvite && !invited ) )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_JOIN, EOSRESULT_NO_PERMISSION, lobbyId );
        return true;
    }
    for ( size_t i=0; i<lobby->members.size(); ++i )
    {
        if ( lobby->members[ i ].puid == self->puid )
        {
            self->QueueEnd( requestId, EOSLOBBYEVENT_JOIN, EOSRESULT_ALREADY_PENDING, lobbyId );
            return true;
        }
    }
    if ( lobby->members.size() >= lobby->maxMembers )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_JOIN, EOSRESULT_LOBBY_FULL, lobbyId );
        return true;
    }

    SFakeMember member;
    member.puid = self->puid;
    lobby->members.push_back( member );
    SEosLobbyEvent event;
    event.kind = EOSLOBBYEVENT_MEMBER_STATUS;
    event.status = EOSMEMBERSTATUS_JOINED;
    event.userId = self->puid;
    self->NotifyMembers( *lobby, self->puid, event );
    self->QueueEnd( requestId, EOSLOBBYEVENT_JOIN, EOSRESULT_SUCCESS, lobbyId );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFakeEosPeer::DestroyLobbyNow( SFakeLobby& lobby )
{
    // Everybody still in it is told that it is closed
    for ( size_t i=0; i<lobby.members.size(); ++i )
    {
        CFakeEosPeer* peer = world.FindPeer( lobby.members[ i ].puid );
        if ( GUCEF_NULL == peer )
            continue;
        SEosLobbyEvent event;
        event.kind = EOSLOBBYEVENT_MEMBER_STATUS;
        event.status = EOSMEMBERSTATUS_CLOSED;
        event.userId = lobby.members[ i ].puid;
        event.lobbyId = lobby.id;
        event.result = EOSRESULT_SUCCESS;
        peer->QueueLobbyEvent( event );
    }
    world.m_lobbies.erase( lobby.id );
}

/*-------------------------------------------------------------------------*/

void
CFakeEosPeer::RemoveMember( SFakeLobby& lobby, const CORE::CString& user, EEosMemberStatus reason )
{
    bool wasOwner = lobby.owner == user;
    for ( size_t i=0; i<lobby.members.size(); ++i )
    {
        if ( lobby.members[ i ].puid == user )
        {
            lobby.members.erase( lobby.members.begin() + i );
            break;
        }
    }

    // The one that is kicked is told too
    if ( EOSMEMBERSTATUS_KICKED == reason )
    {
        CFakeEosPeer* target = world.FindPeer( user );
        if ( GUCEF_NULL != target )
        {
            SEosLobbyEvent kicked;
            kicked.kind = EOSLOBBYEVENT_MEMBER_STATUS;
            kicked.status = EOSMEMBERSTATUS_KICKED;
            kicked.userId = user;
            kicked.lobbyId = lobby.id;
            kicked.result = EOSRESULT_SUCCESS;
            target->QueueLobbyEvent( kicked );
        }
    }

    if ( lobby.members.empty() )
    {
        world.m_lobbies.erase( lobby.id );
        return;
    }
    if ( wasOwner && lobby.disableHostMigration )
    {
        DestroyLobbyNow( lobby );
        return;
    }

    SEosLobbyEvent left;
    left.kind = EOSLOBBYEVENT_MEMBER_STATUS;
    left.status = reason;
    left.userId = user;
    NotifyMembers( lobby, CORE::CString::Empty, left );
    if ( wasOwner )
    {
        lobby.owner = lobby.members[ 0 ].puid;
        SEosLobbyEvent promoted;
        promoted.kind = EOSLOBBYEVENT_MEMBER_STATUS;
        promoted.status = EOSMEMBERSTATUS_PROMOTED;
        promoted.userId = lobby.owner;
        NotifyMembers( lobby, CORE::CString::Empty, promoted );
    }
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::LeaveLobby( void* context, UInt32 requestId, const char* lobbyId, const char* localUser )
{
    CFakeEosPeer* self = Self( context );
    ++self->leaveCalls;
    if ( self->Preempt( requestId, EOSLOBBYEVENT_LEAVE, lobbyId, EOSRESULT_SUCCESS ) )
        return true;
    SFakeLobby* lobby = self->world.FindLobby( lobbyId );
    bool member = false;
    if ( GUCEF_NULL != lobby )
        for ( size_t i=0; i<lobby->members.size(); ++i )
            member = member || lobby->members[ i ].puid == localUser;
    if ( !member )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_LEAVE, EOSRESULT_NOT_FOUND, lobbyId );
        return true;
    }
    self->RemoveMember( *lobby, localUser, EOSMEMBERSTATUS_LEFT );
    self->QueueEnd( requestId, EOSLOBBYEVENT_LEAVE, EOSRESULT_SUCCESS, lobbyId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::DestroyLobby( void* context, UInt32 requestId, const char* lobbyId, const char* localUser )
{
    CFakeEosPeer* self = Self( context );
    ++self->destroyCalls;
    if ( self->Preempt( requestId, EOSLOBBYEVENT_DESTROY, lobbyId, EOSRESULT_SUCCESS ) )
        return true;
    SFakeLobby* lobby = self->world.FindLobby( lobbyId );
    if ( GUCEF_NULL == lobby )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_DESTROY, EOSRESULT_NOT_FOUND, lobbyId );
        return true;
    }
    if ( lobby->owner != localUser )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_DESTROY, EOSRESULT_NO_PERMISSION, lobbyId );
        return true;
    }
    CORE::CString id = lobbyId;
    self->DestroyLobbyNow( *lobby );
    self->QueueEnd( requestId, EOSLOBBYEVENT_DESTROY, EOSRESULT_SUCCESS, id );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::KickMember( void* context, UInt32 requestId, const char* lobbyId, const char* localUser, const char* targetUser )
{
    CFakeEosPeer* self = Self( context );
    ++self->kickCalls;
    if ( self->Preempt( requestId, EOSLOBBYEVENT_KICK, lobbyId, self->failures.kick ) )
        return true;
    SFakeLobby* lobby = self->world.FindLobby( lobbyId );
    if ( GUCEF_NULL == lobby )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_KICK, EOSRESULT_NOT_FOUND, lobbyId );
        return true;
    }
    if ( lobby->owner != localUser || CORE::CString( targetUser ) == localUser )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_KICK, EOSRESULT_NO_PERMISSION, lobbyId );
        return true;
    }
    bool member = false;
    for ( size_t i=0; i<lobby->members.size(); ++i )
        member = member || lobby->members[ i ].puid == targetUser;
    if ( !member )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_KICK, EOSRESULT_NOT_FOUND, lobbyId );
        return true;
    }
    lobby->kicked.insert( targetUser );
    lobby->invited.erase( targetUser );
    self->RemoveMember( *lobby, targetUser, EOSMEMBERSTATUS_KICKED );
    self->QueueEnd( requestId, EOSLOBBYEVENT_KICK, EOSRESULT_SUCCESS, lobbyId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::PromoteMember( void* context, UInt32 requestId, const char* lobbyId, const char* localUser, const char* targetUser )
{
    CFakeEosPeer* self = Self( context );
    ++self->promoteCalls;
    if ( self->Preempt( requestId, EOSLOBBYEVENT_PROMOTE, lobbyId, self->failures.promote ) )
        return true;
    SFakeLobby* lobby = self->world.FindLobby( lobbyId );
    if ( GUCEF_NULL == lobby )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_PROMOTE, EOSRESULT_NOT_FOUND, lobbyId );
        return true;
    }
    bool member = false;
    for ( size_t i=0; i<lobby->members.size(); ++i )
        member = member || lobby->members[ i ].puid == targetUser;
    if ( lobby->owner != localUser || !member )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_PROMOTE, lobby->owner != localUser ? EOSRESULT_NO_PERMISSION : EOSRESULT_NOT_FOUND, lobbyId );
        return true;
    }
    lobby->owner = targetUser;
    SEosLobbyEvent promoted;
    promoted.kind = EOSLOBBYEVENT_MEMBER_STATUS;
    promoted.status = EOSMEMBERSTATUS_PROMOTED;
    promoted.userId = targetUser;
    self->NotifyMembers( *lobby, CORE::CString::Empty, promoted );
    self->QueueEnd( requestId, EOSLOBBYEVENT_PROMOTE, EOSRESULT_SUCCESS, lobbyId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::SendInvite( void* context, UInt32 requestId, const char* lobbyId, const char* localUser, const char* targetUser )
{
    CFakeEosPeer* self = Self( context );
    ++self->inviteCalls;
    if ( self->Preempt( requestId, EOSLOBBYEVENT_SEND_INVITE, lobbyId, EOSRESULT_SUCCESS ) )
        return true;
    SFakeLobby* lobby = self->world.FindLobby( lobbyId );
    CFakeEosPeer* target = self->world.FindPeer( targetUser );
    bool member = false;
    if ( GUCEF_NULL != lobby )
        for ( size_t i=0; i<lobby->members.size(); ++i )
            member = member || lobby->members[ i ].puid == localUser;
    if ( !member || GUCEF_NULL == target || !lobby->allowInvites )
    {
        self->QueueEnd( requestId, EOSLOBBYEVENT_SEND_INVITE, member ? EOSRESULT_NOT_FOUND : EOSRESULT_NO_PERMISSION, lobbyId );
        return true;
    }
    lobby->invited.insert( targetUser );
    SEosLobbyEvent event;
    event.kind = EOSLOBBYEVENT_INVITE_RECEIVED;
    event.result = EOSRESULT_SUCCESS;
    event.lobbyId = lobbyId;
    event.senderId = localUser;
    target->QueueLobbyEvent( event );
    self->QueueEnd( requestId, EOSLOBBYEVENT_SEND_INVITE, EOSRESULT_SUCCESS, lobbyId );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFakeEosPeer::FillDetails( const SFakeLobby& lobby, SEosLobbyDetails& details, bool member ) const
{
    details.lobbyId = lobby.id;
    details.ownerPuid = lobby.owner;
    details.bucketId = lobby.bucketId;
    details.maxMembers = lobby.maxMembers;
    details.availableSlots = lobby.maxMembers > lobby.members.size() ? lobby.maxMembers - (UInt32) lobby.members.size() : 0;
    details.permission = lobby.permission;
    details.allowInvites = lobby.allowInvites;
    for ( size_t i=0; i<lobby.attributes.size(); ++i )
        if ( member || EOSVISIBILITY_PUBLIC == lobby.attributes[ i ].visibility )
            details.attributes.push_back( lobby.attributes[ i ] );
    for ( size_t i=0; i<lobby.members.size(); ++i )
    {
        SEosLobbyMember copy;
        copy.puid = lobby.members[ i ].puid;
        if ( member )
            copy.attributes = lobby.members[ i ].attributes;
        details.members.push_back( copy );
    }
}

/*-------------------------------------------------------------------------*/

static bool
Compare( const SEosAttribute& attribute, const SEosLobbySearchParameter& parameter )
{
    if ( attribute.type != parameter.type )
        return false;
    int order = 0;
    if ( EOSATTRIBUTE_INT64 == parameter.type )
        order = attribute.intValue < parameter.intValue ? -1 : ( attribute.intValue > parameter.intValue ? 1 : 0 );
    else
        order = attribute.stringValue == parameter.stringValue ? 0 : ( attribute.stringValue < parameter.stringValue ? -1 : 1 );
    switch ( parameter.comparison )
    {
        case EOSCOMPARE_EQUAL :                 return 0 == order;
        case EOSCOMPARE_NOT_EQUAL :             return 0 != order;
        case EOSCOMPARE_GREATER_THAN :          return order > 0;
        case EOSCOMPARE_GREATER_THAN_OR_EQUAL : return order >= 0;
        case EOSCOMPARE_LESS_THAN :             return order < 0;
        case EOSCOMPARE_LESS_THAN_OR_EQUAL :    return order <= 0;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::SearchLobbies( void* context, UInt32 requestId, const SEosLobbySearchOptions* options )
{
    CFakeEosPeer* self = Self( context );
    ++self->searchCalls;
    self->lastSearch = *options;
    self->lastSearchBucket = options->bucketId;
    if ( self->Preempt( requestId, EOSLOBBYEVENT_SEARCH, CORE::CString::Empty, self->failures.search ) )
        return true;

    SEosLobbyEvent event;
    event.requestId = requestId;
    event.kind = EOSLOBBYEVENT_SEARCH;
    event.result = EOSRESULT_SUCCESS;
    CORE::CString::StringVector ids = self->world.GetLobbyIds();
    for ( size_t i=0; i<ids.size() && event.found.size() < options->maxResults; ++i )
    {
        SFakeLobby* lobby = self->world.FindLobby( ids[ i ] );
        bool isMember = false;
        for ( size_t m=0; m<lobby->members.size(); ++m )
            isMember = isMember || lobby->members[ m ].puid == options->localUser;
        bool visible = EOSLOBBYPERM_PUBLIC_ADVERTISED == lobby->permission;
        if ( !options->lobbyId.IsNULLOrEmpty() )
            visible = ( EOSLOBBYPERM_INVITE_ONLY != lobby->permission || isMember ) && lobby->id == options->lobbyId;
        if ( !visible )
            continue;
        if ( !options->targetUser.IsNULLOrEmpty() )
        {
            bool targetIn = false;
            for ( size_t m=0; m<lobby->members.size(); ++m )
                targetIn = targetIn || lobby->members[ m ].puid == options->targetUser;
            if ( !targetIn )
                continue;
        }
        if ( !options->bucketId.IsNULLOrEmpty() && lobby->bucketId != options->bucketId )
            continue;
        if ( options->minSlotsAvailable >= 0 && lobby->maxMembers < lobby->members.size() + (UInt32) options->minSlotsAvailable )
            continue;
        bool matches = true;
        for ( size_t p=0; matches && p<options->parameters.size(); ++p )
        {
            const SEosAttribute* attribute = GUCEF_NULL;
            for ( size_t a=0; a<lobby->attributes.size(); ++a )
                if ( lobby->attributes[ a ].key == options->parameters[ p ].key && EOSVISIBILITY_PUBLIC == lobby->attributes[ a ].visibility )
                    attribute = &lobby->attributes[ a ];
            matches = GUCEF_NULL != attribute && Compare( *attribute, options->parameters[ p ] );
        }
        if ( !matches )
            continue;
        SEosLobbyDetails details;
        self->FillDetails( *lobby, details, false );
        event.found.push_back( details );
    }
    self->QueueLobbyEvent( event );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::CopyLobbyDetails( void* context, const char* lobbyId, const char* localUser, SEosLobbyDetails* details )
{
    CFakeEosPeer* self = Self( context );
    SFakeLobby* lobby = self->world.FindLobby( lobbyId );
    if ( GUCEF_NULL == lobby || self->failures.noConnection )
        return false;
    for ( size_t i=0; i<lobby->members.size(); ++i )
    {
        if ( lobby->members[ i ].puid == localUser )
        {
            self->FillDetails( *lobby, *details, true );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::PollLobbyEvent( void* context, SEosLobbyEvent* event )
{
    CFakeEosPeer* self = Self( context );
    UInt64 now = self->Now();
    for ( size_t i=0; i<self->m_lobbyEvents.size(); ++i )
    {
        if ( self->m_lobbyEvents[ i ].dueInMs <= now )
        {
            *event = self->m_lobbyEvents[ i ].event;
            self->m_lobbyEvents.erase( self->m_lobbyEvents.begin() + i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      P2P                                                                //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CFakeEosPeer::QueueP2PEvent( const SEosP2PEvent& event )
{
    SP2PEntry entry;
    entry.dueInMs = Now() + latencyMs + world.network.latencyMs;
    entry.event = event;
    m_p2pEvents.push_back( entry );
}

/*-------------------------------------------------------------------------*/

static void
FillEvent( SEosP2PEvent& event, EEosP2PEventKind kind, const CORE::CString& local, const CORE::CString& remote, const CORE::CString& socketName )
{
    event.kind = kind;
    strncpy( event.localUser, local.C_String(), sizeof( event.localUser ) - 1 );
    event.localUser[ sizeof( event.localUser ) - 1 ] = 0;
    strncpy( event.remoteUser, remote.C_String(), sizeof( event.remoteUser ) - 1 );
    event.remoteUser[ sizeof( event.remoteUser ) - 1 ] = 0;
    strncpy( event.socketName, socketName.C_String(), sizeof( event.socketName ) - 1 );
    event.socketName[ sizeof( event.socketName ) - 1 ] = 0;
}

/*-------------------------------------------------------------------------*/

void
CFakeEosPeer::QueuePacket( CFakeEosPeer& to, SConnection& connection, UInt8 channel, const void* data, UInt32 size, EEosPacketReliability reliability )
{
    CFakeEosWorld& w = world;
    ++w.packetsSent;
    ++packetsSent;
    if ( size > w.largestPacket )
        w.largestPacket = size;

    bool reliable = EOSRELIABILITY_UNRELIABLE_UNORDERED != reliability;
    if ( !reliable && w.NextRandomUnit() < w.network.lossRate )
    {
        ++w.packetsDropped;
        return;
    }
    UInt32 copies = 1;
    if ( !reliable && w.NextRandomUnit() < w.network.duplicateRate )
    {
        copies = 2;
        ++w.packetsDuplicated;
    }
    bool toOther = connection.initiator == puid;
    UInt64& lastDue = toOther ? connection.lastDueToOther : connection.lastDueToInitiator;
    for ( UInt32 c=0; c<copies; ++c )
    {
        SFakePacket packet;
        packet.from = puid;
        packet.socketName = connection.socketName;
        packet.channel = channel;
        packet.data.assign( static_cast< const UInt8* >( data ), static_cast< const UInt8* >( data ) + size );
        UInt64 due = Now() + w.network.latencyMs + ( connection.relayed ? w.network.relayExtraMs : 0 );
        if ( !reliable && w.NextRandomUnit() < w.network.reorderRate )
            due += 1 + ( w.NextRandom() % ( w.network.reorderMs > 0 ? w.network.reorderMs : 1 ) );
        if ( reliable && due < lastDue )
            due = lastDue;
        if ( reliable )
            lastDue = due;
        packet.dueInMs = due;

        // Sorted by the time it arrives, packets of the same time in the order they were sent
        size_t at = to.m_inbox.size();
        while ( at > 0 && to.m_inbox[ at - 1 ].dueInMs > due )
            --at;
        to.m_inbox.insert( to.m_inbox.begin() + at, packet );
    }
}

/*-------------------------------------------------------------------------*/

static SConnection*
FindConnection( GUCEF::vector< SConnection >& connections, const CORE::CString& a, const CORE::CString& b, const CORE::CString& socketName )
{
    for ( size_t i=0; i<connections.size(); ++i )
    {
        SConnection& c = connections[ i ];
        if ( c.socketName == socketName && ( ( c.initiator == a && c.other == b ) || ( c.initiator == b && c.other == a ) ) )
            return &c;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

EEosResult
CFakeEosPeer::SendPacket( void* context, const char* localUser, const char* remoteUser, const char* socketName, UInt8 channel,
                          const void* data, UInt32 size, EEosPacketReliability reliability, bool allowDelayedDelivery )
{
    CFakeEosPeer* self = Self( context );
    CFakeEosWorld& w = self->world;
    if ( !self->platformReady || self->puid != localUser )
        return EOSRESULT_NOT_LOGGED_IN;
    if ( size > EosP2PMaxPacketSize || GUCEF_NULL == data || strlen( socketName ) > EosP2PMaxSocketNameLength )
        return EOSRESULT_INVALID_PARAMETERS;
    if ( self->failures.noConnection )
        return EOSRESULT_NO_CONNECTION;

    CORE::CString remote = remoteUser;
    CORE::CString socket = socketName;
    CFakeEosPeer* to = w.FindPeer( remote );
    SConnection* connection = FindConnection( w.m_connections, self->puid, remote, socket );
    if ( GUCEF_NULL == connection )
    {
        // A packet opens the connection: the other side has to accept it
        SConnection fresh;
        fresh.initiator = self->puid;
        fresh.other = remote;
        fresh.socketName = socket;
        fresh.requestedInMs = self->Now();
        w.m_connections.push_back( fresh );
        connection = &w.m_connections.back();
        if ( GUCEF_NULL != to && to->platformReady )
        {
            SEosP2PEvent event;
            FillEvent( event, EOSP2PEVENT_CONNECTION_REQUEST, remote, self->puid, socket );
            to->QueueP2PEvent( event );
        }
    }

    if ( !connection->established )
    {
        if ( connection->pending.size() >= 64 )
            return EOSRESULT_LIMIT_EXCEEDED;
        SFakePacket packet;
        packet.from = self->puid;
        packet.socketName = socket;
        packet.channel = channel;
        packet.data.assign( static_cast< const UInt8* >( data ), static_cast< const UInt8* >( data ) + size );
        packet.dueInMs = reliability;
        connection->pending.push_back( packet );
        ++self->packetsSent;
        return EOSRESULT_SUCCESS;
    }
    if ( GUCEF_NULL == to )
        return EOSRESULT_NOT_FOUND;
    self->QueuePacket( *to, *connection, channel, data, size, reliability );
    return EOSRESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::ReceivePacket( void* context, const char* localUser, UInt32 bufferSize, void* buffer, UInt32* size, char* remoteUser, char* socketName, UInt8* channel )
{
    CFakeEosPeer* self = Self( context );
    CFakeEosWorld& w = self->world;
    UInt64 now = self->Now();
    while ( !self->m_inbox.empty() && self->m_inbox[ 0 ].dueInMs <= now )
    {
        SFakePacket packet = self->m_inbox[ 0 ];
        self->m_inbox.erase( self->m_inbox.begin() );

        // Whatever is in flight when the connection closes does not arrive
        SConnection* connection = FindConnection( w.m_connections, self->puid, packet.from, packet.socketName );
        if ( GUCEF_NULL == connection || !connection->established || packet.data.size() > bufferSize )
        {
            ++w.packetsDropped;
            continue;
        }
        memcpy( buffer, &packet.data[ 0 ], packet.data.size() );
        *size = (UInt32) packet.data.size();
        *channel = packet.channel;
        strncpy( remoteUser, packet.from.C_String(), EosP2PUserIdBufferSize - 1 );
        remoteUser[ EosP2PUserIdBufferSize - 1 ] = 0;
        strncpy( socketName, packet.socketName.C_String(), EosP2PSocketBufferSize - 1 );
        socketName[ EosP2PSocketBufferSize - 1 ] = 0;
        ++w.packetsDelivered;
        ++self->packetsReceived;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

EEosResult
CFakeEosPeer::AcceptConnection( void* context, const char* localUser, const char* remoteUser, const char* socketName )
{
    CFakeEosPeer* self = Self( context );
    CFakeEosWorld& w = self->world;
    ++self->acceptCalls;
    SConnection* connection = FindConnection( w.m_connections, self->puid, remoteUser, socketName );
    if ( GUCEF_NULL == connection || connection->established || connection->initiator != remoteUser )
        return EOSRESULT_NOT_FOUND;

    CFakeEosPeer* initiator = w.FindPeer( connection->initiator );
    bool relayNeeded = w.network.directBlocked || EOSRELAY_FORCE_RELAYS == self->relayControl || ( GUCEF_NULL != initiator && EOSRELAY_FORCE_RELAYS == initiator->relayControl );
    bool relayForbidden = EOSRELAY_NO_RELAYS == self->relayControl || ( GUCEF_NULL != initiator && EOSRELAY_NO_RELAYS == initiator->relayControl );
    CORE::CString initiatorId = connection->initiator;
    CORE::CString socket = connection->socketName;
    if ( relayNeeded && relayForbidden )
    {
        // No way through: the connection fails for both
        ++w.refusedConnections;
        if ( GUCEF_NULL != initiator )
        {
            SEosP2PEvent failed;
            FillEvent( failed, EOSP2PEVENT_CONNECTION_CLOSED, initiatorId, self->puid, socket );
            failed.closeReason = EOSCLOSE_CONNECTION_FAILED;
            initiator->QueueP2PEvent( failed );
        }
        SEosP2PEvent failedHere;
        FillEvent( failedHere, EOSP2PEVENT_CONNECTION_CLOSED, self->puid, initiatorId, socket );
        failedHere.closeReason = EOSCLOSE_CONNECTION_FAILED;
        self->QueueP2PEvent( failedHere );
        for ( size_t i=0; i<w.m_connections.size(); ++i )
            if ( &w.m_connections[ i ] == connection )
            {
                w.m_connections.erase( w.m_connections.begin() + i );
                break;
            }
        return EOSRESULT_SUCCESS;
    }

    connection->established = true;
    connection->relayed = relayNeeded;
    if ( relayNeeded )
        ++w.relayedConnections;
    else
        ++w.directConnections;

    SEosP2PEvent established;
    FillEvent( established, EOSP2PEVENT_CONNECTION_ESTABLISHED, self->puid, initiatorId, socket );
    established.networkType = relayNeeded ? EOSNETWORK_RELAYED : EOSNETWORK_DIRECT;
    self->QueueP2PEvent( established );
    if ( GUCEF_NULL != initiator )
    {
        SEosP2PEvent establishedThere;
        FillEvent( establishedThere, EOSP2PEVENT_CONNECTION_ESTABLISHED, initiatorId, self->puid, socket );
        establishedThere.networkType = established.networkType;
        initiator->QueueP2PEvent( establishedThere );
    }

    // What waited for the connection goes now
    GUCEF::vector< SFakePacket > pending = connection->pending;
    connection->pending.clear();
    for ( size_t i=0; i<pending.size() && GUCEF_NULL != initiator; ++i )
        initiator->QueuePacket( *self, *connection, pending[ i ].channel, &pending[ i ].data[ 0 ], (UInt32) pending[ i ].data.size(), (EEosPacketReliability) pending[ i ].dueInMs );
    return EOSRESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

EEosResult
CFakeEosPeer::CloseConnection( void* context, const char* localUser, const char* remoteUser, const char* socketName )
{
    CFakeEosPeer* self = Self( context );
    CFakeEosWorld& w = self->world;
    ++self->closeCalls;
    SConnection* connection = FindConnection( w.m_connections, self->puid, remoteUser, socketName );
    if ( GUCEF_NULL == connection )
        return EOSRESULT_NOT_FOUND;
    CFakeEosPeer* other = w.FindPeer( remoteUser );
    CORE::CString socket = socketName;
    for ( size_t i=0; i<w.m_connections.size(); ++i )
        if ( &w.m_connections[ i ] == connection )
        {
            w.m_connections.erase( w.m_connections.begin() + i );
            break;
        }
    if ( GUCEF_NULL != other )
    {
        SEosP2PEvent closed;
        FillEvent( closed, EOSP2PEVENT_CONNECTION_CLOSED, remoteUser, self->puid, socket );
        closed.closeReason = EOSCLOSE_CLOSED_BY_PEER;
        other->QueueP2PEvent( closed );
    }
    return EOSRESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

EEosResult
CFakeEosPeer::CloseConnections( void* context, const char* localUser, const char* socketName )
{
    CFakeEosPeer* self = Self( context );
    CFakeEosWorld& w = self->world;
    bool found = true;
    while ( found )
    {
        found = false;
        for ( size_t i=0; i<w.m_connections.size(); ++i )
        {
            SConnection& c = w.m_connections[ i ];
            if ( c.socketName == socketName && ( c.initiator == self->puid || c.other == self->puid ) )
            {
                CORE::CString remote = c.initiator == self->puid ? c.other : c.initiator;
                CloseConnection( context, localUser, remote.C_String(), socketName );
                found = true;
                break;
            }
        }
    }
    return EOSRESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

void
CFakeEosPeer::ExpireConnections( void )
{
    CFakeEosWorld& w = world;
    UInt64 now = Now();
    for ( size_t i=0; i<w.m_connections.size(); )
    {
        SConnection& c = w.m_connections[ i ];
        if ( !c.established && now >= c.requestedInMs + w.network.pendingTimeoutMs )
        {
            CFakeEosPeer* initiator = w.FindPeer( c.initiator );
            CFakeEosPeer* other = w.FindPeer( c.other );
            if ( GUCEF_NULL != initiator )
            {
                SEosP2PEvent timedOut;
                FillEvent( timedOut, EOSP2PEVENT_CONNECTION_CLOSED, c.initiator, c.other, c.socketName );
                timedOut.closeReason = EOSCLOSE_TIMED_OUT;
                initiator->QueueP2PEvent( timedOut );
            }
            if ( GUCEF_NULL != other )
            {
                SEosP2PEvent timedOut;
                FillEvent( timedOut, EOSP2PEVENT_CONNECTION_CLOSED, c.other, c.initiator, c.socketName );
                timedOut.closeReason = EOSCLOSE_TIMED_OUT;
                other->QueueP2PEvent( timedOut );
            }
            w.m_connections.erase( w.m_connections.begin() + i );
        }
        else
        {
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::PollP2PEvent( void* context, SEosP2PEvent* event )
{
    CFakeEosPeer* self = Self( context );
    UInt64 now = self->Now();
    for ( size_t i=0; i<self->m_p2pEvents.size(); ++i )
    {
        if ( self->m_p2pEvents[ i ].dueInMs <= now )
        {
            *event = self->m_p2pEvents[ i ].event;
            self->m_p2pEvents.erase( self->m_p2pEvents.begin() + i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

EEosResult
CFakeEosPeer::SetRelayControl( void* context, EEosRelayControl control )
{
    Self( context )->relayControl = control;
    return EOSRESULT_SUCCESS;
}

EEosNatType CFakeEosPeer::GetNatType( void* context ) { return Self( context )->world.network.directBlocked ? EOSNAT_STRICT : EOSNAT_OPEN; }
EEosResult CFakeEosPeer::SetPacketQueueSize( void* context, UInt64 incoming, UInt64 outgoing ) { return EOSRESULT_SUCCESS; }
void CFakeEosPeer::SetLocalUser( void* context, const char* localUser ) {}

/*-------------------------------------------------------------------------//
//                                                                         //
//      AUTH                                                               //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CFakeEosPeer::PollAuthEvent( void* context, AUTHPLUGIN::EOS::SEosEvent* event )
{
    CFakeEosPeer* self = Self( context );
    UInt64 now = self->Now();
    for ( size_t i=0; i<self->m_authEvents.size(); ++i )
    {
        if ( self->m_authEvents[ i ].dueInMs <= now )
        {
            *event = self->m_authEvents[ i ].event;
            self->m_authEvents.erase( self->m_authEvents.begin() + i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFakeEosPeer::AuthLogin( void* context, UInt32 requestId, AUTHPLUGIN::EOS::EEosLoginType type )
{
    CFakeEosPeer* self = Self( context );
    SAuthEntry entry;
    entry.dueInMs = self->Now();
    entry.event.requestId = requestId;
    entry.event.kind = AUTHPLUGIN::EOS::EOSEVENT_AUTH_LOGIN;
    entry.event.outcome = AUTHPLUGIN::EOS::EOSOUTCOME_SUCCESS;
    strncpy( entry.event.epicAccountId, self->epicAccountId.C_String(), sizeof( entry.event.epicAccountId ) - 1 );
    self->m_authEvents.push_back( entry );
    return true;
}

bool
CFakeEosPeer::AuthCopyIdToken( void* context, const char* account, char* buffer, UInt32 bufferSize )
{
    static const char token[] = "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJ4In0.FAKE-EOS-ID-TOKEN-SIGNATURE-DO-NOT-LOG";
    if ( bufferSize < sizeof( token ) )
        return false;
    strcpy( buffer, token );
    return true;
}

bool
CFakeEosPeer::ConnectLogin( void* context, UInt32 requestId, const char* idToken )
{
    CFakeEosPeer* self = Self( context );
    SAuthEntry entry;
    entry.dueInMs = self->Now();
    entry.event.requestId = requestId;
    entry.event.kind = AUTHPLUGIN::EOS::EOSEVENT_CONNECT_LOGIN;
    entry.event.outcome = AUTHPLUGIN::EOS::EOSOUTCOME_SUCCESS;
    strncpy( entry.event.productUserId, self->puid.C_String(), sizeof( entry.event.productUserId ) - 1 );
    self->m_authEvents.push_back( entry );
    return true;
}

bool CFakeEosPeer::ConnectCreateUser( void* context, UInt32 requestId ) { return false; }
bool CFakeEosPeer::ConnectLinkAccount( void* context, UInt32 requestId, const char* productUserId ) { return false; }
bool CFakeEosPeer::AuthLogout( void* context, UInt32 requestId, const char* epicAccountId, bool deletePersistentAuth ) { return true; }

}; /* namespace EosFake */
