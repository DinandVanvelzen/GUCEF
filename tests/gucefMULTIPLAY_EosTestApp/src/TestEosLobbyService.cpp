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


#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "authpluginEOS_CEosAuthProvider.h"
#include "gucefMULTIPLAY_CLobbyServiceListingProvider.h"
#include "gucefMULTIPLAY_CMultiplayGlobal.h"

#include "TestEosCommon.h"
#include "TestEosLobby.h"

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::EOS;
using namespace GUCEF::MULTIPLAYPLUGIN::EOS;
using namespace EosFake;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAtLeast : public CCondition
{
    public:
    const UInt32& counter;
    UInt32 value;
    CAtLeast( const UInt32& c, UInt32 v ) : counter( c ), value( v ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return counter >= value; }
};

class CRefusedCount : public CCondition
{
    public:
    CLobbyRecorder& recorder;
    UInt32 count;
    CRefusedCount( CLobbyRecorder& r, UInt32 c ) : recorder( r ), count( c ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return recorder.refusals >= count; }
};

class CClosedCount : public CCondition
{
    public:
    CLobbyRecorder& recorder;
    UInt32 count;
    CClosedCount( CLobbyRecorder& r, UInt32 c ) : recorder( r ), count( c ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return recorder.closed >= count; }
};

class CLobbyPermissionIs : public CCondition
{
    public:
    CFakeEosWorld& world;
    CORE::CString lobbyId;
    EEosLobbyPermission permission;
    CLobbyPermissionIs( CFakeEosWorld& w, const CORE::CString& l, EEosLobbyPermission p ) : world( w ), lobbyId( l ), permission( p ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        SFakeLobby* lobby = world.FindLobby( lobbyId );
        return GUCEF_NULL != lobby && lobby->permission == permission;
    }
};

class CListingCounter : public CORE::CObserver
{
    public:
    UInt32 added;
    UInt32 removed;
    UInt32 errors;
    CListingCounter( void ) : CORE::CObserver(), added( 0 ), removed( 0 ), errors( 0 ) {}
    protected:
    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MULTIPLAY::CIGameListingProvider::ListingAddedEvent == eventId ) ++added;
        else
        if ( MULTIPLAY::CIGameListingProvider::ListingRemovedEvent == eventId ) ++removed;
        else
        if ( MULTIPLAY::CIGameListingProvider::ProviderErrorEvent == eventId ) ++errors;
    }
};

class CNoteIs : public CCondition
{
    public:
    MULTIPLAY::CILobbyPtr lobby;
    UInt32 length;
    CNoteIs( MULTIPLAY::CILobbyPtr l, UInt32 n ) : lobby( l ), length( n ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return lobby->GetState().GetSetting( "note" ).Length() == length; }
};

class CLobbyCountIs : public CCondition
{
    public:
    CFakeEosWorld& world;
    UInt32 count;
    CLobbyCountIs( CFakeEosWorld& w, UInt32 c ) : world( w ), count( c ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return world.GetLobbyCount() == count; }
};

static CORE::CString
Repeated( char c, UInt32 count )
{
    GUCEF::vector< char > chars( count + 1, c );
    chars[ count ] = '\0';
    return CORE::CString( &chars[ 0 ] );
}

/** The smallest number of bytes a lobby may hold of a blob for the tree to be written */
static UInt32
BlobSizeOf( const CORE::CDataNode& tree )
{
    UInt32 low = 1;
    UInt32 high = 1048576;
    while ( low < high )
    {
        UInt32 middle = ( low + high ) / 2;
        CEosLobbyLimits limits;
        limits.maxBlobBytes = middle;
        limits.maxLobbyAttributes = 4096;
        TEosAttributeVector attributes;
        CORE::CString reason;
        if ( CEosLobbyCodec::EncodeBlob( "st", tree, limits, attributes, reason ) )
            high = middle;
        else
            low = middle + 1;
    }
    return low;
}

/** A schema with many settings: a lobby definition that does not fit one attribute */
static MULTIPLAY::CLobbyCreateParams
MakeBigParams( UInt32 settings, UInt32 options )
{
    MULTIPLAY::CLobbyCreateParams params = CEosFixture::MakeParams( "Big lobby" );
    for ( UInt32 i=0; i<settings; ++i )
    {
        MULTIPLAY::CLobbySettingDef setting;
        setting.key = "option" + CORE::UInt32ToString( i );
        setting.type = MULTIPLAY::CLobbySettingDef::TYPE_ENUM;
        for ( UInt32 n=0; n<options; ++n )
            setting.options.push_back( "value-number-" + CORE::UInt32ToString( n ) + "-of-the-setting-" + CORE::UInt32ToString( i ) );
        setting.defaultValue = setting.options[ 0 ];
        setting.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
        params.schema.AddSetting( setting );
    }
    return params;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformEosLobbyServiceTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING EOS lobby service limits and errors TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "EosLobbyService" );

    GUCEF_TESTFW_TESTCASE( "Test: The data of a lobby that is larger than an attribute is split over attributes that each fit, and a member reads it back whole" )
        try
        {
            CEosFixture fixture( 2 );
            MULTIPLAY::CLobbyCreateParams params = MakeBigParams( 12, 6 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, params ) );
            SFakeLobby* fake = fixture.world.FindLobby( LobbyIdOf( pair.host ) );
            REQUIRE_TRUE( GUCEF_NULL != fake );

            // many pieces, every one within the limits of the SDK, and the whole within the number of attributes of a lobby
            Int64 pieces = 0;
            ASSERT_TRUE( CEosLobbyCodec::GetInt( fake->attributes, "df_n", pieces ) && pieces >= 3 );
            ASSERT_TRUE( fake->attributes.size() <= fixture.world.maxAttributes && fixture.peers[ 0 ]->peer.largestUpdateAttributes <= fixture.world.maxAttributes );
            for ( size_t i=0; i<fake->attributes.size(); ++i )
            {
                ASSERT_TRUE( fake->attributes[ i ].key.Length() <= fixture.world.maxKeyLength );
                if ( EOSATTRIBUTE_STRING == fake->attributes[ i ].type )
                    ASSERT_TRUE( fake->attributes[ i ].stringValue.Length() <= fixture.world.maxAttributeLength );
            }

            // the member has the schema of the owner
            ASSERT_TRUE( pair.guest->GetSchema().settings.size() == params.schema.settings.size() );
            ASSERT_TRUE( pair.guest->GetSchema().settings.back().key == params.schema.settings.back().key );
            ASSERT_TRUE( pair.guest->GetState().GetSetting( "option11" ) == params.schema.settings.back().options[ 0 ] );

            // and what the owner changes in it
            ASSERT_TRUE( pair.host->SetSetting( "option3", params.schema.settings[ 3 + 4 ].options[ 5 ] ) );
            CSettingIs seen( pair.guest, "option3", params.schema.settings[ 3 + 4 ].options[ 5 ] );
            ASSERT_TRUE( PumpUntil( seen ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A lobby whose data does not fit the attributes of a lobby is not made: it fails with the reason and nothing is left of it" )
        try
        {
            CEosFixture fixture( 1 );
            MULTIPLAY::CLobbyCreateParams params = MakeBigParams( 150, 8 );
            MULTIPLAY::CILobbyPtr lobby = fixture[ 0 ].Create( params );
            ASSERT_TRUE( lobby.IsNULL() );
            ASSERT_TRUE( fixture[ 0 ].reason.HasSubstr( "attributes" ) >= 0 || fixture[ 0 ].reason.HasSubstr( "more than" ) >= 0 );
            CLobbyCountIs none( fixture.world, 0 );
            ASSERT_TRUE( PumpUntil( none ) );
            // the dry run of the publication refused it before EOS was asked to make anything
            ASSERT_TRUE( 0 == fixture[ 0 ].peer.createCalls && 0 == fixture[ 0 ].peer.destroyCalls );
            ASSERT_TRUE( 0 == fixture[ 0 ].peer.GetPendingRequestCount() );

            // the service goes on
            MULTIPLAY::CILobbyPtr fine = fixture[ 0 ].Create( CEosFixture::MakeParams() );
            ASSERT_TRUE( !fine.IsNULL() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A setting that grows the state over several attributes is carried, shrinking it takes the pieces away, and one that does not fit is refused and changes nothing" )
        try
        {
            CEosFixture fixture( 2 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            SFakeLobby* fake = fixture.world.FindLobby( LobbyIdOf( pair.host ) );
            size_t base = fake->attributes.size();
            Int64 basePieces = 0;
            ASSERT_TRUE( CEosLobbyCodec::GetInt( fake->attributes, "st_n", basePieces ) );

            // a note of 3000 characters: more pieces
            ASSERT_TRUE( pair.host->SetSetting( "note", Repeated( 'n', 3000 ) ) );
            CNoteIs grown( pair.guest, 3000 );
            ASSERT_TRUE( PumpUntil( grown ) );
            Int64 pieces = 0;
            ASSERT_TRUE( CEosLobbyCodec::GetInt( fake->attributes, "st_n", pieces ) && pieces > basePieces );
            ASSERT_TRUE( fake->attributes.size() > base );
            for ( size_t i=0; i<fake->attributes.size(); ++i )
                if ( EOSATTRIBUTE_STRING == fake->attributes[ i ].type )
                    ASSERT_TRUE( fake->attributes[ i ].stringValue.Length() <= fixture.world.maxAttributeLength );

            // short again: the pieces that are not needed are gone from the lobby of EOS
            ASSERT_TRUE( pair.host->SetSetting( "note", "short" ) );
            CNoteIs shrunk( pair.guest, 5 );
            ASSERT_TRUE( PumpUntil( shrunk ) );
            PumpFor( 50 );
            ASSERT_TRUE( CEosLobbyCodec::GetInt( fake->attributes, "st_n", pieces ) && pieces == basePieces );
            ASSERT_TRUE( fake->attributes.size() == base );
            ASSERT_TRUE( GUCEF_NULL == CEosLobbyCodec::FindAttribute( fake->attributes, CORE::CString( "st_" ) + CORE::Int64ToString( basePieces ) ) );

            // 70000 characters do not fit: refused, with the reason, nothing changed, and the lobby goes on
            pair.hostEvents.Clear();
            ASSERT_TRUE( pair.host->SetSetting( "note", Repeated( 'x', 70000 ) ) );
            CRefusedCount refused( pair.hostEvents, 1 );
            ASSERT_TRUE( PumpUntil( refused ) );
            ASSERT_TRUE( pair.hostEvents.refusalReasons[ 0 ].HasSubstr( "does not fit" ) >= 0 );
            ASSERT_TRUE( pair.host->GetState().GetSetting( "note" ) == "short" );
            ASSERT_TRUE( pair.host->SetSetting( "map", "ice" ) );
            CSettingIs stillWorks( pair.guest, "map", "ice" );
            ASSERT_TRUE( PumpUntil( stillWorks ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A member that would make the data of the lobby too large to hold is not taken in: it is put out with the reason" )
        try
        {
            CEosFixture fixture( 3 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            CORE::CString lobbyId = LobbyIdOf( pair.host );

            // the state of the lobby is as large as it may be for two members
            ASSERT_TRUE( pair.host->SetSetting( "note", Repeated( 'n', 1000 ) ) );
            CNoteIs noted( pair.guest, 1000 );
            ASSERT_TRUE( PumpUntil( noted ) );
            SFakeLobby* fake = fixture.world.FindLobby( lobbyId );
            CORE::CDataNode tree;
            CORE::CString reason;
            CEosLobbyLimits wide;
            ASSERT_TRUE( CEosLobbyCodec::DecodeBlob( "st", fake->attributes, wide, tree, reason ) );
            UInt32 size = BlobSizeOf( tree );
            CEosLobbyServiceSettings settings = CEosFixture::MakeFastSettings();
            settings.limits.maxBlobBytes = size + 8;
            ASSERT_TRUE( fixture[ 0 ].service.SetSettings( settings ) );

            CLobbyRecorder thirdEvents;
            fixture[ 2 ].serviceRecorder.watcher = &thirdEvents;
            MULTIPLAY::CILobbyPtr third = fixture[ 2 ].Join( lobbyId );
            fixture[ 2 ].serviceRecorder.watcher = GUCEF_NULL;
            // it is let in by EOS and put out by the owner, either at the join or right after
            if ( !third.IsNULL() )
            {
                CClosedCount closed( thirdEvents, 1 );
                ASSERT_TRUE( PumpUntil( closed, 6000 ) );
                ASSERT_TRUE( thirdEvents.closedReason.HasSubstr( "no room" ) >= 0 );
            }
            else
            {
                // the reason of the refusal needs room in the lobby too: when there is none the joiner does not hear it and its join times out
                ASSERT_TRUE( fixture[ 2 ].reason.HasSubstr( "no room" ) >= 0 || fixture[ 2 ].reason.HasSubstr( "did not answer in time" ) >= 0 );
            }
            CEosMembers two( fixture.world, lobbyId, 2 );
            ASSERT_TRUE( PumpUntil( two, 6000 ) );
            ASSERT_TRUE( 2 == pair.host->GetState().members.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Updates of the lobby are held to a rate: many quick changes are coalesced and the last one wins, and an update carries only what changed" )
        try
        {
            CEosFixture fixture( 2 );
            CEosLobbyServiceSettings settings = CEosFixture::MakeFastSettings();
            settings.updatesPerSecond = 4;
            settings.updateBurst = 2;
            settings.secondInMs = 1000;
            fixture[ 0 ].service.SetSettings( settings );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            PumpFor( 400 );
            UInt32 before = fixture[ 0 ].peer.updateCalls;

            // thirty changes in a quick row
            for ( UInt32 i=1; i<=30; ++i )
            {
                ASSERT_TRUE( pair.host->SetSetting( "note", "note number " + CORE::UInt32ToString( i ) ) );
                PumpFor( 2 );
            }
            ASSERT_TRUE( fixture[ 0 ].peer.updateCalls - before <= 8 );
            CSettingIs last( pair.guest, "note", "note number 30" );
            ASSERT_TRUE( PumpUntil( last, 8000 ) );
            ASSERT_TRUE( fixture[ 0 ].peer.updateCalls - before < 30 );
            ASSERT_TRUE( pair.host->GetState().GetSetting( "note" ) == "note number 30" );

            // an update of a small change is small: not the whole lobby again
            PumpFor( 1200 );
            fixture[ 0 ].peer.largestUpdateAttributes = 0;
            ASSERT_TRUE( pair.host->SetSetting( "map", "ice" ) );
            CSettingIs seen( pair.guest, "map", "ice" );
            ASSERT_TRUE( PumpUntil( seen, 8000 ) );
            SFakeLobby* fake = fixture.world.FindLobby( LobbyIdOf( pair.host ) );
            ASSERT_TRUE( fixture[ 0 ].peer.largestUpdateAttributes > 0 && fixture[ 0 ].peer.largestUpdateAttributes < fake->attributes.size() / 2 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A refused update is tried again with everything in it, and a lobby that cannot be published at all says so" )
        try
        {
            CEosFixture fixture( 2 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );

            // two refusals and then it works: the change gets through
            fixture[ 0 ].peer.failures.update = EOSRESULT_LIMIT_EXCEEDED;
            fixture[ 0 ].peer.failures.updateCount = 2;
            ASSERT_TRUE( pair.host->SetSetting( "map", "ice" ) );
            CSettingIs seen( pair.guest, "map", "ice" );
            ASSERT_TRUE( PumpUntil( seen, 15000 ) );

            // a lobby that cannot be made because the first publication fails
            fixture[ 0 ].peer.failures.update = EOSRESULT_LIMIT_EXCEEDED;
            fixture[ 0 ].peer.failures.updateCount = 0;
            MULTIPLAY::CILobbyPtr lobby = fixture[ 0 ].Create( CEosFixture::MakeParams( "Second" ) );
            ASSERT_TRUE( lobby.IsNULL() );
            ASSERT_TRUE( fixture[ 0 ].reason.HasSubstr( "could not be published" ) >= 0 );
            fixture[ 0 ].peer.failures.update = EOSRESULT_SUCCESS;
            CLobbyCountIs one( fixture.world, 1 );
            ASSERT_TRUE( PumpUntil( one ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Creating, joining and browsing fail with the reason of the SDK, and the futures of them end" )
        try
        {
            CEosFixture fixture( 2 );
            MULTIPLAY::CILobbyPtr host = fixture[ 0 ].Create( CEosFixture::MakeParams() );
            REQUIRE_TRUE( !host.IsNULL() );
            CORE::CString lobbyId = LobbyIdOf( host );

            fixture[ 1 ].peer.failures.create = EOSRESULT_NO_PERMISSION;
            ASSERT_TRUE( fixture[ 1 ].Create( CEosFixture::MakeParams() ).IsNULL() );
            ASSERT_TRUE( fixture[ 1 ].reason.HasSubstr( "ermission" ) >= 0 );
            fixture[ 1 ].peer.failures.create = EOSRESULT_SUCCESS;

            // a lobby that is not there, an id that is no id, one that is full
            ASSERT_TRUE( fixture[ 1 ].Join( "0123456789abcdef0123456789abcdef" ).IsNULL() );
            ASSERT_TRUE( fixture[ 1 ].reason.HasSubstr( "not found" ) >= 0 || fixture[ 1 ].reason.HasSubstr( "ot found" ) >= 0 );
            ASSERT_TRUE( fixture[ 1 ].Join( "../not an id" ).IsNULL() );
            ASSERT_FALSE( fixture[ 1 ].reason.IsNULLOrEmpty() );
            fixture[ 1 ].peer.failures.join = EOSRESULT_LOBBY_FULL;
            ASSERT_TRUE( fixture[ 1 ].Join( lobbyId ).IsNULL() );
            ASSERT_TRUE( fixture[ 1 ].reason.HasSubstr( "full" ) >= 0 );
            fixture[ 1 ].peer.failures.join = EOSRESULT_SUCCESS;

            // browse
            fixture[ 1 ].peer.failures.search = EOSRESULT_NO_CONNECTION;
            MULTIPLAY::CGameListingFilter filter;
            filter.gameId = "testgame";
            MULTIPLAY::CGameListingList list;
            CORE::CString failure;
            ASSERT_FALSE( Browse( fixture[ 1 ], filter, list, &failure ) );
            ASSERT_FALSE( failure.IsNULLOrEmpty() );
            fixture[ 1 ].peer.failures.search = EOSRESULT_SUCCESS;
            ASSERT_TRUE( Browse( fixture[ 1 ], filter, list ) && 1 == list.listings.size() );

            // nothing was left under way
            PumpFor( 50 );
            ASSERT_TRUE( 0 == fixture[ 1 ].peer.GetPendingRequestCount() && 1 == fixture.world.GetLobbyCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A request that the SDK does not answer in time fails with a timeout; the lobby that was made late is destroyed again" )
        try
        {
            CEosFixture fixture( 2 );
            CEosLobbyServiceSettings settings = CEosFixture::MakeFastSettings();
            settings.requestTimeoutInMs = 300;
            settings.joinTimeoutInMs = 300;
            fixture[ 0 ].service.SetSettings( settings );
            fixture[ 1 ].service.SetSettings( settings );

            // the answer of creating comes after the time is up
            fixture[ 0 ].peer.latencyMs = 700;
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( fixture[ 0 ].service.CreateLobby( CEosFixture::MakeParams(), future ) );
            ASSERT_TRUE( PumpUntilReady( future, 3000 ) );
            ASSERT_TRUE( FailureOf( future ).HasSubstr( "before the SDK answered" ) >= 0 || FailureOf( future ).HasSubstr( "timed out" ) >= 0 || FailureOf( future ).HasSubstr( "time" ) >= 0 );
            MULTIPLAY::CILobbyPtr none;
            ASSERT_FALSE( MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, none ) );

            // the lobby that EOS made after all goes again
            CLobbyCountIs gone( fixture.world, 0 );
            ASSERT_TRUE( PumpUntil( gone, 5000 ) );
            ASSERT_TRUE( 0 == fixture[ 0 ].serviceRecorder.lobbyJoined );
            PumpFor( 800 );
            ASSERT_TRUE( 0 == fixture[ 0 ].peer.GetPendingRequestCount() );
            fixture[ 0 ].peer.latencyMs = 0;

            // the SDK does not answer at all
            fixture[ 1 ].peer.failures.neverAnswer = true;
            CORE::CFutureResult browse( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            MULTIPLAY::CGameListingFilter filter;
            filter.gameId = "testgame";
            ASSERT_TRUE( fixture[ 1 ].service.Browse( filter, browse ) );
            ASSERT_TRUE( PumpUntilReady( browse, 3000 ) );
            ASSERT_FALSE( FailureOf( browse ).IsNULLOrEmpty() );
            fixture[ 1 ].peer.failures.neverAnswer = false;

            // the service still works
            MULTIPLAY::CILobbyPtr fine = fixture[ 1 ].Create( CEosFixture::MakeParams() );
            ASSERT_TRUE( !fine.IsNULL() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A request that is cancelled ends cancelled, and what it made in the meantime is cleaned up" )
        try
        {
            CEosFixture fixture( 2 );
            MULTIPLAY::CILobbyPtr host = fixture[ 0 ].Create( CEosFixture::MakeParams() );
            REQUIRE_TRUE( !host.IsNULL() );

            fixture[ 1 ].peer.latencyMs = 100;
            CORE::CFutureResult join( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( fixture[ 1 ].service.JoinLobby( LobbyIdOf( host ), CORE::CString::Empty, false, join ) );
            ASSERT_TRUE( join.RequestCancellation() );
            ASSERT_TRUE( PumpUntilReady( join, 3000 ) );
            MULTIPLAY::CILobbyPtr none;
            ASSERT_FALSE( MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( join, none ) );
            CEosMembers onlyHost( fixture.world, LobbyIdOf( host ), 1 );
            ASSERT_TRUE( PumpUntil( onlyHost, 5000 ) );
            PumpFor( 300 );
            ASSERT_TRUE( 0 == fixture[ 1 ].serviceRecorder.lobbyJoined && 1 == host->GetState().members.size() );

            CORE::CFutureResult create( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( fixture[ 1 ].service.CreateLobby( CEosFixture::MakeParams(), create ) );
            ASSERT_TRUE( create.RequestCancellation() );
            ASSERT_TRUE( PumpUntilReady( create, 3000 ) );
            CLobbyCountIs onlyOne( fixture.world, 1 );
            ASSERT_TRUE( PumpUntil( onlyOne, 5000 ) );
            fixture[ 1 ].peer.latencyMs = 0;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A service that is disconnected or destroyed with requests under way ends every one of them, and leaves nothing behind in EOS" )
        try
        {
            CEosFixture fixture( 2 );
            MULTIPLAY::CILobbyPtr host = fixture[ 0 ].Create( CEosFixture::MakeParams() );
            REQUIRE_TRUE( !host.IsNULL() );

            // under way: a create, a join and a browse
            fixture[ 1 ].peer.latencyMs = 200;
            CORE::CFutureResult create( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            CORE::CFutureResult join( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            CORE::CFutureResult browse( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            MULTIPLAY::CGameListingFilter filter;
            filter.gameId = "testgame";
            ASSERT_TRUE( fixture[ 1 ].service.CreateLobby( CEosFixture::MakeParams( "In flight" ), create ) );
            ASSERT_TRUE( fixture[ 1 ].service.JoinLobby( LobbyIdOf( host ), CORE::CString::Empty, false, join ) );
            ASSERT_TRUE( fixture[ 1 ].service.Browse( filter, browse ) );
            ASSERT_FALSE( create.IsReady() || join.IsReady() || browse.IsReady() );
            fixture[ 1 ].service.Disconnect();
            ASSERT_TRUE( create.IsReady() && join.IsReady() && browse.IsReady() );
            ASSERT_TRUE( FailureOf( create ).HasSubstr( "disconnected" ) >= 0 && FailureOf( join ).HasSubstr( "disconnected" ) >= 0 );
            ASSERT_FALSE( fixture[ 1 ].service.IsConnected() );
            ASSERT_TRUE( 1 == fixture[ 1 ].serviceRecorder.disconnected );

            // the join is undone: the local user is not left in the lobby that it was joining
            PumpFor( 600 );
            CEosMembers onlyHost( fixture.world, LobbyIdOf( host ), 1 );
            ASSERT_TRUE( PumpUntil( onlyHost, 5000 ) );
            fixture[ 1 ].peer.latencyMs = 0;

            // a service that is destroyed with a lobby of its own and a request under way
            {
                CEosPeerContext extra( fixture.world, 2 );
                ASSERT_TRUE( extra.Login() && extra.Connect() );
                MULTIPLAY::CILobbyPtr own = extra.Create( CEosFixture::MakeParams( "Short lived" ) );
                ASSERT_TRUE( !own.IsNULL() );
                extra.peer.latencyMs = 200;
                CORE::CFutureResult pending( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( extra.service.JoinLobby( LobbyIdOf( host ), CORE::CString::Empty, false, pending ) );
                // the lobby object that the application still holds goes on without a service
                CLobbyRecorder watched;
                watched.SubscribeTo( own.GetPointerAlways() );
                extra.service.Disconnect();
                ASSERT_TRUE( pending.IsReady() && 1 == watched.closed );
                ASSERT_FALSE( own->Leave() );
                own = MULTIPLAY::CILobbyPtr();
            }
            CEosMembers hostAlone( fixture.world, LobbyIdOf( host ), 1 );
            ASSERT_TRUE( PumpUntil( hostAlone, 5000 ) );
            ASSERT_TRUE( 1 == host->GetState().members.size() );
            ASSERT_TRUE( fixture.world.GetLobbyCount() <= 2 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A player that drops off the network leaves its lobby for the others: the lobby goes on, and the one that was the owner hands it over" )
        try
        {
            CEosFixture fixture( 3 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            CORE::CString lobbyId = LobbyIdOf( pair.host );
            MULTIPLAY::CILobbyPtr third = fixture[ 2 ].Join( lobbyId );
            REQUIRE_TRUE( !third.IsNULL() );
            CMemberCount three( pair.host, 3 );
            ASSERT_TRUE( PumpUntil( three ) );

            // a member: gone from the state of the owner
            fixture[ 2 ].peer.DropOffTheNetwork();
            CMemberCount two( pair.host, 2 );
            ASSERT_TRUE( PumpUntil( two ) );
            CMemberCount twoAtGuest( pair.guest, 2 );
            ASSERT_TRUE( PumpUntil( twoAtGuest ) );

            // the owner: the lobby moves
            fixture[ 0 ].peer.DropOffTheNetwork();
            CBossIs guestIsBoss( pair.guest, fixture[ 1 ].peer.puid );
            ASSERT_TRUE( PumpUntil( guestIsBoss ) );
            ASSERT_TRUE( Eos( pair.guest ).IsOwner() && 1 == pair.guest->GetState().members.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A lobby is listed while it is published and not when it is withdrawn, ReportPlayer is not supported, and an invite reaches the invited" )
        try
        {
            CEosFixture fixture( 2 );
            MULTIPLAY::CILobbyPtr lobby = fixture[ 0 ].Create( CEosFixture::MakeParams( "Listed" ) );
            REQUIRE_TRUE( !lobby.IsNULL() );
            CORE::CString lobbyId = LobbyIdOf( lobby );
            MULTIPLAY::CGameListingFilter filter;
            filter.gameId = "testgame";
            MULTIPLAY::CGameListingList list;
            ASSERT_TRUE( Browse( fixture[ 1 ], filter, list ) && 1 == list.listings.size() );

            ASSERT_TRUE( fixture[ 0 ].service.WithdrawListing( lobbyId ) );
            CLobbyPermissionIs hidden( fixture.world, lobbyId, EOSLOBBYPERM_INVITE_ONLY );
            ASSERT_TRUE( PumpUntil( hidden ) );
            ASSERT_TRUE( Browse( fixture[ 1 ], filter, list ) && 0 == list.listings.size() );

            MULTIPLAY::CGameListingEntry entry;
            entry.listingId = lobbyId;
            ASSERT_TRUE( fixture[ 0 ].service.PublishListing( entry ) );
            CLobbyPermissionIs shown( fixture.world, lobbyId, EOSLOBBYPERM_PUBLIC_ADVERTISED );
            ASSERT_TRUE( PumpUntil( shown ) );
            ASSERT_TRUE( Browse( fixture[ 1 ], filter, list ) && 1 == list.listings.size() );

            // a lobby that is not of this service, and one that is not the local user's
            ASSERT_FALSE( fixture[ 0 ].service.WithdrawListing( "unknown" ) );
            MULTIPLAY::CILobbyPtr joined = fixture[ 1 ].Join( lobbyId );
            REQUIRE_TRUE( !joined.IsNULL() );
            ASSERT_FALSE( fixture[ 1 ].service.WithdrawListing( lobbyId ) );

            // players cannot be reported over EOS
            ASSERT_FALSE( joined->ReportPlayer( fixture[ 0 ].peer.puid, "rude" ) );
            ASSERT_FALSE( fixture[ 1 ].service.GetCapabilities().chat );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Nothing secret is logged: not the client secret, the encryption key, the id token, the password of a lobby, the tokens of the game" )
        try
        {
            ASSERT_TRUE( LoadTestPlugin( "drnchannelpluginEOSP2P", "eosp2p" ) );
            CLogCapture capture;
            {
                EnsureGlueSettings();
                CEosFixture fixture( 3, false );
                CEosPeerContext& host = fixture[ 0 ];
                {
                    // the login of the host is the real auth plugin, with the secrets in its config
                    AUTHPLUGIN::EOS::CEosAuthProvider provider( host.peer.AuthShim() );
                    CORE::CDataNode node( "epic", GUCEF_DATATYPE_OBJECT );
                    node.SetAttribute( "clientId", "xyz-client-id" );
                    node.SetAttribute( "clientSecret", "EOS-CLIENT-SECRET-5d8e0c47-DO-NOT-LOG" );
                    CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
                    parameters->SetAttribute( "productId", "product-1" );
                    parameters->SetAttribute( "sandboxId", "sandbox-1" );
                    parameters->SetAttribute( "deploymentId", "deployment-1" );
                    parameters->SetAttribute( "encryptionKey", "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff" );
                    parameters->SetAttribute( "tickIntervalInMs", 5 );
                    REQUIRE_TRUE( provider.LoadConfig( node ) );
                    CORE::CFutureResult login( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                    REQUIRE_TRUE( provider.TrySilentLogin( login ) && PumpUntilReady( login, 3000 ) );
                    REQUIRE_TRUE( host.Connect() );
                    for ( UInt32 i=1; i<3; ++i )
                        REQUIRE_TRUE( fixture[ i ].Login() && fixture[ i ].Connect() );

                    MULTIPLAY::CLobbyCreateParams params = CEosFixture::MakeParams( "Secret place" );
                    params.password = "open sesame";
                    MULTIPLAY::CILobbyPtr hostLobby = host.Create( params );
                    REQUIRE_TRUE( !hostLobby.IsNULL() );
                    CORE::CString lobbyId = LobbyIdOf( hostLobby );
                    ASSERT_TRUE( fixture[ 1 ].Join( lobbyId, "wrong password" ).IsNULL() );
                    CLobbyRecorder hostEvents, guestEvents;
                    hostEvents.SubscribeTo( hostLobby.GetPointerAlways() );
                    fixture[ 1 ].serviceRecorder.watcher = &guestEvents;
                    MULTIPLAY::CILobbyPtr guest = fixture[ 1 ].Join( lobbyId, "open sesame" );
                    fixture[ 1 ].serviceRecorder.watcher = GUCEF_NULL;
                    REQUIRE_TRUE( !guest.IsNULL() );
                    CMemberCount both( hostLobby, 2 );
                    ASSERT_TRUE( PumpUntil( both ) );
                    guest->SetReady( true );
                    CMemberReady guestReady( hostLobby, fixture[ 1 ].peer.puid, true );
                    ASSERT_TRUE( PumpUntil( guestReady ) );
                    hostLobby->SetReady( true );
                    CMemberReady hostReady( hostLobby, host.peer.puid, true );
                    ASSERT_TRUE( PumpUntil( hostReady ) );
                    ASSERT_TRUE( hostLobby->RequestStart() );
                    CAtLeast hostLaunched( hostEvents.launches, 1 );
                    CAtLeast guestLaunched( guestEvents.launches, 1 );
                    ASSERT_TRUE( PumpUntil( hostLaunched, 10000 ) && PumpUntil( guestLaunched, 10000 ) );

                    // the tokens and the password exist; they are not in the log
                    ASSERT_FALSE( guestEvents.lastLaunch.token.IsNULLOrEmpty() || hostEvents.lastLaunch.token.IsNULLOrEmpty() );
                    ASSERT_FALSE( capture.Contains( guestEvents.lastLaunch.token ) );
                    ASSERT_FALSE( capture.Contains( hostEvents.lastLaunch.token ) );
                    guest = MULTIPLAY::CILobbyPtr();
                    hostLobby = MULTIPLAY::CILobbyPtr();
                }
            }
            ASSERT_FALSE( capture.Contains( "EOS-CLIENT-SECRET" ) );
            ASSERT_FALSE( capture.Contains( "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff" ) );
            ASSERT_FALSE( capture.Contains( "FAKE-EOS-ID-TOKEN" ) );
            ASSERT_FALSE( capture.Contains( "open sesame" ) );
            ASSERT_FALSE( capture.Contains( "wrong password" ) );
            // and there was something to see: the log works
            ASSERT_TRUE( capture.Contains( "EosLobby" ) || capture.Contains( "EosLobbyService" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every event of a lobby comes on the thread that pulses: the application thread" )
        try
        {
            CEosFixture fixture( 2 );
            SPair pair;
            pair.hostEvents.expectedThreadId = MT::GetCurrentTaskID();
            pair.guestEvents.expectedThreadId = MT::GetCurrentTaskID();
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            ASSERT_TRUE( pair.host->SetSetting( "map", "ice" ) );
            ASSERT_TRUE( pair.guest->SetReady( true ) );
            CMemberReady ready( pair.host, fixture[ 1 ].peer.puid, true );
            ASSERT_TRUE( PumpUntil( ready ) );
            ASSERT_TRUE( pair.host->Kick( fixture[ 1 ].peer.puid ) );
            CClosedCount closed( pair.guestEvents, 1 );
            ASSERT_TRUE( PumpUntil( closed ) );
            ASSERT_TRUE( 0 == pair.hostEvents.wrongThreadEvents && 0 == pair.guestEvents.wrongThreadEvents );
            ASSERT_TRUE( pair.hostEvents.stateChanged > 0 && pair.guestEvents.stateChanged > 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The plugin puts the eos service in the factory of lobby services, where the create lobby form and the listing provider find it" )
        try
        {
            MULTIPLAY::CMultiplayGlobal* multiplay = MULTIPLAY::CMultiplayGlobal::Instance();
            ASSERT_TRUE( LoadTestPlugin( "lobbyservicepluginEOS", "eosplugin" ) );
            MULTIPLAY::CLobbyServiceFactory::TKeySet keys;
            multiplay->GetLobbyServiceFactory().ObtainKeySet( keys );
            ASSERT_TRUE( keys.find( "eos" ) != keys.end() );

            MULTIPLAY::CILobbyServicePtr service = multiplay->GetLobbyServiceFactory().Create( "eos" );
            REQUIRE_TRUE( !service.IsNULL() );
            ASSERT_TRUE( service->GetServiceName() == "eos" && !service->IsConnected() );
            CEosLobbyService* eos = static_cast< CEosLobbyService* >( service.GetPointerAlways() );

            // a user, logged in, and the service of the factory connected for it
            CFakeEosWorld world;
            CEosPeerContext player( world, 0 );
            CEosPeerContext other( world, 1 );
            REQUIRE_TRUE( player.Login() && other.Login() );
            PLUGINGLUE::EOS::SEosP2PShim p2p = player.peer.P2PShim();
            eos->SetSettings( CEosFixture::MakeFastSettings() );
            eos->SetShim( player.peer.LobbyShim(), &p2p );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            REQUIRE_TRUE( service->Connect( player.identity, future ) && PumpUntilReady( future, 3000 ) );
            REQUIRE_TRUE( service->IsConnected() );
            REQUIRE_TRUE( other.Connect() );

            // a lobby of another player
            MULTIPLAY::CILobbyPtr otherLobby = other.Create( CEosFixture::MakeParams( "Seen by the listing" ) );
            REQUIRE_TRUE( !otherLobby.IsNULL() );

            // the provider of listings that goes over a lobby service finds it
            MULTIPLAY::CIGameListingProviderPtr provider = multiplay->GetListingProviderFactory().Create( "lobby-service" );
            REQUIRE_TRUE( !provider.IsNULL() );
            MULTIPLAY::CLobbyServiceListingProvider* listing = static_cast< MULTIPLAY::CLobbyServiceListingProvider* >( provider.GetPointerAlways() );
            listing->SetLobbyService( service );
            listing->SetRefreshIntervalInSecs( 0 );
            CListingCounter counter;
            counter.SubscribeTo( provider.GetPointerAlways() );
            MULTIPLAY::CGameListingFilter filter;
            filter.gameId = "testgame";
            ASSERT_TRUE( provider->Start( filter ) );
            CAtLeast added( counter.added, 1 );
            ASSERT_TRUE( PumpUntil( added, 8000 ) );
            MULTIPLAY::CGameListingEntry found;
            ASSERT_TRUE( provider->GetListing( LobbyIdOf( otherLobby ), found ) && found.name == "Seen by the listing" && found.backendName == "eos" );
            ASSERT_TRUE( 1 == found.endpoints.size() && found.endpoints[ 0 ].HasSubstr( "eos-p2p://" ) == 0 );

            // it goes when the lobby does
            CORE::CFutureResult none( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( otherLobby->Leave() );
            otherLobby = MULTIPLAY::CILobbyPtr();
            CLobbyCountIs empty( world, 0 );
            ASSERT_TRUE( PumpUntil( empty, 5000 ) );
            ASSERT_TRUE( provider->Refresh() );
            CAtLeast removed( counter.removed, 1 );
            ASSERT_TRUE( PumpUntil( removed, 8000 ) );
            ASSERT_TRUE( 0 == provider->GetListingCount() );
            provider->Stop();
            counter.UnsubscribeAllFromObserver();

            // an eos service from the factory that is not connected does not hang the provider: it reports
            MULTIPLAY::CLobbyServiceListingProvider byName;
            CORE::CDataNode config;
            config.SetAttribute( "service", "eos" );
            ASSERT_TRUE( byName.LoadConfig( config ) );
            CListingCounter errors;
            errors.SubscribeTo( &byName );
            ASSERT_TRUE( byName.Start( filter ) );
            ASSERT_TRUE( 1 == errors.errors );
            byName.Stop();
            errors.UnsubscribeAllFromObserver();

            service->Disconnect();
            service = MULTIPLAY::CILobbyServicePtr();
            provider = MULTIPLAY::CIGameListingProviderPtr();
            CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "eosplugin" );
            keys.clear();
            multiplay->GetLobbyServiceFactory().ObtainKeySet( keys );
            ASSERT_TRUE( keys.find( "eos" ) == keys.end() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
