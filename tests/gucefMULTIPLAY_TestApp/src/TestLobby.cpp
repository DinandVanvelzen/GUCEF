/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPump.h"
#include "TestLobbyService.h"
#include "TestLobbyRecorder.h"
#include "TestLobby.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the failure and abandons the test case, for what the rest of it needs */
#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A lobby with a boss (alice), two players (bob, carol) and a spectator (dave) */
class CLobbyFixture
{
    public:

    MULTIPLAY::CLobbyRules rules;
    MULTIPLAY::CLobbyCreateParams params;
    CTestLobbyService services[ 4 ];
    MULTIPLAY::CILobbyPtr alice;
    MULTIPLAY::CILobbyPtr bob;
    MULTIPLAY::CILobbyPtr carol;
    MULTIPLAY::CILobbyPtr dave;
    CLobbyRecorder aliceEvents;
    CLobbyRecorder bobEvents;
    CLobbyRecorder carolEvents;
    CLobbyRecorder daveEvents;
    CORE::CString lobbyId;

    CLobbyFixture( void )
    {
        params.name = "Test lobby";
        params.gameId = "testgame";
        params.gameVersion = "1.0";
        params.maxPlayers = 4;
        params.allowSpectators = true;
        params.isPublic = false;
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

        params.schema.factions.push_back( "arm" );
        params.schema.factions.push_back( "core" );
        params.schema.colours.push_back( "red" );
        params.schema.colours.push_back( "blue" );
        params.schema.colours.push_back( "green" );
        params.schema.colours.push_back( "yellow" );
        params.schema.uniqueColours = true;

        rules.minPlayers = 2;
        rules.countdownInSecs = 3;
        rules.voteDurationInSecs = 30;
        rules.requireAllReady = true;
        rules.allowForceStart = true;
        rules.startByVote = true;
        rules.closeWhenHostLeaves = false;
    }

    ~CLobbyFixture()
    {
        CTestLobbyService::Reset();
    }

    bool Connect( UInt32 index, const char* id, const char* name )
    {
        MULTIPLAY::CPlayerIdentity identity;
        identity.userId = id;
        identity.displayName = name;
        CTestLobbyService& service = services[ index ];
        service.SetRules( rules );
        service.SetSecondInMs( 20 );
        CORE::CString::StringVector endpoints;
        endpoints.push_back( "udp://127.0.0.1:" + CORE::ToString( 5001 + index ) );
        service.SetLocalEndpoints( endpoints );
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        return service.Connect( identity, future ) && service.IsConnected();
    }

    MULTIPLAY::CILobbyPtr Join( UInt32 index, bool asSpectator )
    {
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        MULTIPLAY::CILobbyPtr lobby;
        if ( services[ index ].JoinLobby( lobbyId, CORE::CString::Empty, asSpectator, future ) )
            MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, lobby );
        return lobby;
    }

    bool Setup( void )
    {
        if ( !Connect( 0, "alice-id", "Alice" ) || !Connect( 1, "bob-id", "Bob" ) ||
             !Connect( 2, "carol-id", "Carol" ) || !Connect( 3, "dave-id", "Dave" ) )
            return false;

        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        if ( !services[ 0 ].CreateLobby( params, future ) || !MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, alice ) )
            return false;
        aliceEvents.Watch( alice );
        lobbyId = alice->GetState().lobbyId;

        bob = Join( 1, false );
        bobEvents.Watch( bob );
        carol = Join( 2, false );
        carolEvents.Watch( carol );
        dave = Join( 3, true );
        daveEvents.Watch( dave );
        return !bob.IsNULL() && !carol.IsNULL() && !dave.IsNULL();
    }

    bool AllReady( void )
    {
        return alice->SetReady( true ) && bob->SetReady( true ) && carol->SetReady( true );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static MULTIPLAY::CLobbyMember
MakeMember( const char* id, MULTIPLAY::TLobbyMemberRole role, Int32 slot, UInt32 joinOrder )
{
    MULTIPLAY::CLobbyMember member;
    member.id = id;
    member.name = id;
    member.role = role;
    member.slot = slot;
    member.joinOrder = joinOrder;
    return member;
}

/*-------------------------------------------------------------------------*/

void
PerformLobbyTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING lobby TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Lobby" );

    TTestLobbyServiceFactory testServiceFactory;
    MULTIPLAY::CMultiplayGlobal::Instance()->GetLobbyServiceFactory().RegisterConcreteFactory( CTestLobbyService::ServiceName, &testServiceFactory );

    GUCEF_TESTFW_TESTCASE( "Test: Setting definitions check types, ranges, steps and options" )
        try
        {
            CORE::CString reason;
            MULTIPLAY::CLobbySettingDef def;
            def.key = "speed";
            def.type = MULTIPLAY::CLobbySettingDef::TYPE_INT;
            def.hasRange = true;
            def.minValue = 1;
            def.maxValue = 9;
            def.step = 2;
            ASSERT_TRUE( def.IsValidValue( "3", reason ) );
            ASSERT_FALSE( def.IsValidValue( "4", reason ) );        // not a step from the minimum
            ASSERT_FALSE( def.IsValidValue( "11", reason ) );       // out of range
            ASSERT_FALSE( def.IsValidValue( "fast", reason ) );
            ASSERT_FALSE( def.IsValidValue( "3.5", reason ) );

            def.type = MULTIPLAY::CLobbySettingDef::TYPE_FLOAT;
            def.step = 0;
            ASSERT_TRUE( def.IsValidValue( "3.5", reason ) );

            def.type = MULTIPLAY::CLobbySettingDef::TYPE_BOOL;
            ASSERT_TRUE( def.IsValidValue( "true", reason ) );
            ASSERT_FALSE( def.IsValidValue( "maybe", reason ) );

            def.type = MULTIPLAY::CLobbySettingDef::TYPE_ENUM;
            def.options.push_back( "dust" );
            ASSERT_TRUE( def.IsValidValue( "dust", reason ) );
            ASSERT_FALSE( def.IsValidValue( "lava", reason ) );
            ASSERT_FALSE( reason.IsNULLOrEmpty() );

            // The schema survives its config
            CLobbyFixture fixture;
            CORE::CDataNode config;
            ASSERT_TRUE( fixture.params.SaveConfig( config ) );
            MULTIPLAY::CLobbyCreateParams loaded;
            ASSERT_TRUE( loaded.LoadConfig( config ) );
            ASSERT_TRUE( 3 == loaded.schema.settings.size() );
            const MULTIPLAY::CLobbySettingDef* speed = loaded.schema.FindSetting( "speed" );
            ASSERT_TRUE( GUCEF_NULL != speed );
            if ( GUCEF_NULL != speed )
            {
                ASSERT_TRUE( speed->hasRange && 1 == speed->minValue && 5 == speed->maxValue );
                ASSERT_TRUE( MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_VOTE == speed->changePolicy );
            }
            ASSERT_TRUE( 2 == loaded.schema.factions.size() );
            ASSERT_TRUE( 4 == loaded.schema.colours.size() && loaded.schema.uniqueColours );
            ASSERT_TRUE( "testgame" == loaded.gameId && 4 == loaded.maxPlayers );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A setting has a unit and can be gated by another setting; both survive the config and the JSON text, and the rules keep checking the value" )
        try
        {
            // nothing is set by default
            MULTIPLAY::CLobbySettingDef plain;
            ASSERT_TRUE( plain.unit.IsNULLOrEmpty() && plain.enabledByKey.IsNULLOrEmpty() && plain.enabledByValue.IsNULLOrEmpty() );
            ASSERT_FALSE( plain.HasEnabledBy() );

            CLobbyFixture fixture;
            MULTIPLAY::CLobbySettingDef timer;
            timer.key = "timer";
            timer.type = MULTIPLAY::CLobbySettingDef::TYPE_BOOL;
            timer.defaultValue = "true";
            timer.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_VOTE;
            fixture.params.schema.AddSetting( timer );

            MULTIPLAY::CLobbySettingDef seconds;
            seconds.key = "seconds";
            seconds.type = MULTIPLAY::CLobbySettingDef::TYPE_INT;
            seconds.hasRange = true;
            seconds.minValue = 15;
            seconds.maxValue = 3600;
            seconds.step = 5;
            seconds.defaultValue = "180";
            seconds.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_VOTE;
            seconds.unit = "s";
            seconds.enabledByKey = "timer";
            seconds.enabledByValue = "true";
            ASSERT_TRUE( seconds.HasEnabledBy() );
            fixture.params.schema.AddSetting( seconds );

            // the dependency is about the value of the other setting, in the form of its type
            ASSERT_TRUE( seconds.IsEnabledByValue( timer, "true" ) );
            ASSERT_TRUE( seconds.IsEnabledByValue( timer, "1" ) );
            ASSERT_FALSE( seconds.IsEnabledByValue( timer, "false" ) );
            ASSERT_FALSE( seconds.IsEnabledByValue( timer, "0" ) );
            ASSERT_TRUE( plain.IsEnabledByValue( timer, "false" ) );
            ASSERT_TRUE( timer.IsSameValue( "TRUE", "true" ) );
            MULTIPLAY::CLobbySettingDef number;
            number.type = MULTIPLAY::CLobbySettingDef::TYPE_INT;
            ASSERT_TRUE( number.IsSameValue( "4", "4.0" ) && !number.IsSameValue( "4", "5" ) );
            MULTIPLAY::CLobbySettingDef text;
            ASSERT_TRUE( text.IsSameValue( "ice", "ice" ) && !text.IsSameValue( "ice", "Ice" ) );

            // the schema keeps both through its config
            CORE::CDataNode config;
            ASSERT_TRUE( fixture.params.SaveConfig( config ) );
            MULTIPLAY::CLobbyCreateParams loaded;
            ASSERT_TRUE( loaded.LoadConfig( config ) );
            const MULTIPLAY::CLobbySettingDef* secondsCopy = loaded.schema.FindSetting( "seconds" );
            ASSERT_TRUE( GUCEF_NULL != secondsCopy );
            if ( GUCEF_NULL != secondsCopy )
            {
                ASSERT_TRUE( "s" == secondsCopy->unit && "timer" == secondsCopy->enabledByKey && "true" == secondsCopy->enabledByValue );
                ASSERT_TRUE( secondsCopy->hasRange && 15 == secondsCopy->minValue && 3600 == secondsCopy->maxValue && 5 == secondsCopy->step );
            }
            const MULTIPLAY::CLobbySettingDef* timerCopy = loaded.schema.FindSetting( "timer" );
            ASSERT_TRUE( GUCEF_NULL != timerCopy && timerCopy->unit.IsNULLOrEmpty() && !timerCopy->HasEnabledBy() );

            // and through a JSON text of the config
            if ( JsonCodecAvailable() )
            {
                CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
                ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) );
                CORE::CDataNode root( "params", GUCEF_DATATYPE_OBJECT );
                ASSERT_TRUE( fixture.params.SaveConfig( root ) );
                CORE::CDynamicBuffer buffer;
                CORE::CDynamicBufferAccess access( buffer );
                ASSERT_TRUE( codec->StoreDataTree( &root, &access ) );
                access.Setpos( 0 );
                CORE::CDataNode fromJson;
                ASSERT_TRUE( codec->BuildDataTree( &fromJson, &access ) );
                const CORE::CDataNode* document = &fromJson;
                if ( GUCEF_NULL == fromJson.FindChild( "schema" ) && 1 == fromJson.GetNrOfChildNodes() )
                    document = *fromJson.ConstBegin();
                MULTIPLAY::CLobbyCreateParams viaJson;
                ASSERT_TRUE( viaJson.LoadConfig( *document ) );
                const MULTIPLAY::CLobbySettingDef* secondsJson = viaJson.schema.FindSetting( "seconds" );
                ASSERT_TRUE( GUCEF_NULL != secondsJson && "s" == secondsJson->unit && "timer" == secondsJson->enabledByKey && "true" == secondsJson->enabledByValue );
            }

            // the server keeps checking the value of a gated setting, whatever the setting it depends on is
            MULTIPLAY::CLobbyState state;
            MULTIPLAY::CLobbySlot slot;
            state.slots.push_back( slot );
            fixture.rules.ApplyJoin( state, "a", "A", false );
            bool needsVote = false;
            CORE::CString reason;
            const MULTIPLAY::CLobbySettingsSchema& schema = fixture.params.schema;
            ASSERT_TRUE( fixture.rules.CheckSetting( state, schema, "a", "timer", "false", needsVote, reason ) );
            ASSERT_TRUE( fixture.rules.CheckSetting( state, schema, "a", "seconds", "60", needsVote, reason ) );
            ASSERT_FALSE( fixture.rules.CheckSetting( state, schema, "a", "seconds", "7", needsVote, reason ) );
            ASSERT_FALSE( fixture.rules.CheckSetting( state, schema, "a", "seconds", "5000", needsVote, reason ) );
            ASSERT_TRUE( reason.Length() > 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The rules decide on settings by policy, on slots, colours, votes and who is boss next" )
        try
        {
            CLobbyFixture fixture;
            MULTIPLAY::CLobbyRules& rules = fixture.rules;
            const MULTIPLAY::CLobbySettingsSchema& schema = fixture.params.schema;

            MULTIPLAY::CLobbyState state;
            for ( UInt32 i=0; i<4; ++i )
            {
                MULTIPLAY::CLobbySlot slot;
                slot.index = i;
                state.slots.push_back( slot );
            }
            rules.ApplyJoin( state, "a", "A", false );
            rules.ApplyJoin( state, "b", "B", false );
            rules.ApplyJoin( state, "c", "C", false );
            rules.ApplyJoin( state, "s", "S", true );
            ASSERT_TRUE( "a" == state.bossId );
            ASSERT_TRUE( 3 == state.GetPlayerCount() && 1 == state.GetSpectatorCount() );
            ASSERT_TRUE( 1 == state.FindMember( "b" )->slot );
            ASSERT_TRUE( -1 == state.FindMember( "s" )->slot );

            bool needsVote = false;
            CORE::CString reason;
            ASSERT_TRUE( rules.CheckSetting( state, schema, "a", "map", "ice", needsVote, reason ) && !needsVote );
            ASSERT_FALSE( rules.CheckSetting( state, schema, "b", "map", "ice", needsVote, reason ) );
            ASSERT_TRUE( rules.CheckSetting( state, schema, "b", "speed", "4", needsVote, reason ) && needsVote );
            ASSERT_TRUE( rules.CheckSetting( state, schema, "a", "speed", "4", needsVote, reason ) && !needsVote );
            ASSERT_FALSE( rules.CheckSetting( state, schema, "a", "speed", "9", needsVote, reason ) );
            ASSERT_TRUE( rules.CheckSetting( state, schema, "b", "b/handicap", "20", needsVote, reason ) && !needsVote );
            ASSERT_FALSE( rules.CheckSetting( state, schema, "b", "a/handicap", "20", needsVote, reason ) );
            ASSERT_FALSE( rules.CheckSetting( state, schema, "b", "handicap", "20", needsVote, reason ) );
            ASSERT_FALSE( rules.CheckSetting( state, schema, "a", "nosuch", "1", needsVote, reason ) );
            ASSERT_FALSE( rules.CheckProposal( state, schema, "s", "speed", "4", reason ) );   // spectators do not propose

            ASSERT_FALSE( rules.CheckSlotRequest( state, "b", 0, reason ) );                   // taken
            ASSERT_TRUE( rules.CheckSlotRequest( state, "b", 3, reason ) );
            ASSERT_FALSE( rules.CheckSlotRequest( state, "b", 7, reason ) );                   // no such slot
            ASSERT_TRUE( rules.CheckColour( state, schema, "a", "red", reason ) );
            state.FindSlotOfMember( "a" )->colour = "red";
            ASSERT_FALSE( rules.CheckColour( state, schema, "b", "red", reason ) );            // unique
            ASSERT_FALSE( rules.CheckColour( state, schema, "b", "pink", reason ) );           // not offered
            ASSERT_FALSE( rules.CheckColour( state, schema, "s", "blue", reason ) );           // no slot
            ASSERT_TRUE( rules.CheckFaction( state, schema, "b", "core", reason ) );
            ASSERT_FALSE( rules.CheckFaction( state, schema, "b", "zerg", reason ) );
            ASSERT_FALSE( rules.CheckSpectator( state, "a", true, reason ) );                  // the boss plays
            ASSERT_FALSE( rules.CheckSlotKind( state, "b", 3, MULTIPLAY::LOBBYSLOTKIND_AI, reason ) );
            ASSERT_TRUE( rules.CheckSlotKind( state, "a", 3, MULTIPLAY::LOBBYSLOTKIND_AI, reason ) );
            ASSERT_FALSE( rules.CheckSlotKind( state, "a", 1, MULTIPLAY::LOBBYSLOTKIND_CLOSED, reason ) );   // b is in it
            ASSERT_FALSE( rules.CheckReady( state, "s", reason ) );

            // Starting: all ready, enough players
            ASSERT_FALSE( rules.IsStartAllowed( state, reason ) );
            state.FindMember( "a" )->ready = true;
            state.FindMember( "b" )->ready = true;
            state.FindMember( "c" )->ready = true;
            ASSERT_TRUE( rules.IsStartAllowed( state, reason ) );
            ASSERT_TRUE( rules.CheckStart( state, "b", false, needsVote, reason ) && needsVote );
            ASSERT_FALSE( rules.CheckStart( state, "b", true, needsVote, reason ) );           // only the boss forces
            ASSERT_FALSE( rules.CheckStart( state, "s", false, needsVote, reason ) );

            // Votes: more than half of the players
            ASSERT_TRUE( 2 == rules.GetVotesNeeded( state ) );
            state.activeVote.id = "v1";
            state.activeVote.needed = 2;
            state.activeVote.yes = 1;
            ASSERT_TRUE( MULTIPLAY::CLobbyRules::VOTEOUTCOME_UNDECIDED == rules.EvaluateVote( state ) );
            state.activeVote.no = 2;
            ASSERT_TRUE( MULTIPLAY::CLobbyRules::VOTEOUTCOME_FAILED == rules.EvaluateVote( state ) );
            state.activeVote.no = 0;
            state.activeVote.yes = 2;
            ASSERT_TRUE( MULTIPLAY::CLobbyRules::VOTEOUTCOME_PASSED == rules.EvaluateVote( state ) );

            // Leaving: the vote of the proposer ends, the boss passes to the longest present player
            state.activeVote.yes = 1;
            state.activeVote.proposer = "a";
            MULTIPLAY::CLobbyRules::TLeaveOutcome outcome;
            rules.ApplyLeave( state, "a", false, outcome );
            ASSERT_TRUE( "b" == outcome.newBossId && "b" == state.bossId );
            ASSERT_TRUE( outcome.voteCancelled && !state.activeVote.IsActive() );
            ASSERT_FALSE( outcome.lobbyClosed );
            ASSERT_TRUE( state.FindSlot( 0 )->IsFree() );
            ASSERT_TRUE( MULTIPLAY::LOBBYMEMBERROLE_BOSS == state.FindMember( "b" )->role );

            // Only a spectator left: it becomes boss, then the lobby closes when empty
            rules.ApplyLeave( state, "b", false, outcome );
            rules.ApplyLeave( state, "c", false, outcome );
            ASSERT_TRUE( "s" == state.bossId );
            rules.ApplyLeave( state, "s", false, outcome );
            ASSERT_TRUE( outcome.lobbyClosed && MULTIPLAY::LOBBYPHASE_CLOSED == state.phase );

            // A LAN host that leaves closes the lobby when the rules say so
            MULTIPLAY::CLobbyState lanState;
            lanState.members.push_back( MakeMember( "h", MULTIPLAY::LOBBYMEMBERROLE_BOSS, -1, 0 ) );
            lanState.members.push_back( MakeMember( "p", MULTIPLAY::LOBBYMEMBERROLE_PLAYER, -1, 1 ) );
            lanState.bossId = "h";
            rules.closeWhenHostLeaves = true;
            rules.ApplyLeave( lanState, "h", true, outcome );
            ASSERT_TRUE( outcome.lobbyClosed );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The lobby state survives a data tree" )
        try
        {
            MULTIPLAY::CLobbyState state;
            state.lobbyId = "lobby-x";
            state.name = "Name";
            state.phase = MULTIPLAY::LOBBYPHASE_COUNTDOWN;
            state.bossId = "a";
            state.members.push_back( MakeMember( "a", MULTIPLAY::LOBBYMEMBERROLE_BOSS, 0, 0 ) );
            state.members.push_back( MakeMember( "s", MULTIPLAY::LOBBYMEMBERROLE_SPECTATOR, -1, 1 ) );
            state.members[ 0 ].latencyMs = 42;
            MULTIPLAY::CLobbySlot slot;
            slot.index = 0;
            slot.team = 1;
            slot.faction = "arm";
            slot.colour = "red";
            slot.kind = MULTIPLAY::LOBBYSLOTKIND_HUMAN;
            slot.memberId = "a";
            state.slots.push_back( slot );
            state.SetSetting( "map", "ice" );
            state.activeVote.id = "vote-1";
            state.activeVote.what = "setting";
            state.activeVote.key = "speed";
            state.activeVote.value = "4";
            state.activeVote.yes = 1;
            state.activeVote.needed = 2;
            state.chatBackend = "irc";
            state.chatChannel = "lobby-x";

            CORE::CDataNode tree;
            ASSERT_TRUE( state.Serialize( tree, CORE::CDataNodeSerializableSettings() ) );
            MULTIPLAY::CLobbyState copy;
            ASSERT_TRUE( copy.Deserialize( tree, CORE::CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( "lobby-x" == copy.lobbyId && MULTIPLAY::LOBBYPHASE_COUNTDOWN == copy.phase && "a" == copy.bossId );
            ASSERT_TRUE( 2 == copy.members.size() && 1 == copy.slots.size() );
            if ( 2 == copy.members.size() && 1 == copy.slots.size() )
            {
                ASSERT_TRUE( 42 == copy.members[ 0 ].latencyMs && copy.members[ 1 ].IsSpectator() );
                ASSERT_TRUE( "red" == copy.slots[ 0 ].colour && "a" == copy.slots[ 0 ].memberId && 1 == copy.slots[ 0 ].team );
            }
            ASSERT_TRUE( "ice" == copy.GetSetting( "map" ) );
            ASSERT_TRUE( copy.activeVote.IsActive() && "speed" == copy.activeVote.key && 2 == copy.activeVote.needed );
            ASSERT_TRUE( "irc" == copy.chatBackend && "lobby-x" == copy.chatChannel );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A boss, two players and a spectator; the boss changes a setting, a player cannot" )
        try
        {
            CLobbyFixture fixture;
            REQUIRE_TRUE( fixture.Setup() );

            // The service made by name is the same kind
            MULTIPLAY::CILobbyServicePtr made = MULTIPLAY::CMultiplayGlobal::Instance()->GetLobbyServiceFactory().Create( CTestLobbyService::ServiceName );
            ASSERT_FALSE( made.IsNULL() );

            const MULTIPLAY::CLobbyState& state = fixture.alice->GetState();
            ASSERT_TRUE( fixture.alice->IsBoss() && !fixture.bob->IsBoss() );
            ASSERT_TRUE( 4 == state.members.size() && 3 == state.GetPlayerCount() && 1 == state.GetSpectatorCount() );
            ASSERT_TRUE( 0 == state.FindMember( "alice-id" )->slot && 1 == state.FindMember( "bob-id" )->slot );
            ASSERT_TRUE( -1 == state.FindMember( "dave-id" )->slot );
            ASSERT_TRUE( 3 == fixture.aliceEvents.joined );
            ASSERT_TRUE( "dust" == state.GetSetting( "map" ) && "3" == state.GetSetting( "speed" ) );

            ASSERT_TRUE( fixture.alice->SetSetting( "map", "ice" ) );
            ASSERT_TRUE( "ice" == fixture.bob->GetState().GetSetting( "map" ) );
            ASSERT_TRUE( fixture.bobEvents.changedPaths.find( "settings/map" ) != fixture.bobEvents.changedPaths.end() );

            fixture.bob->SetSetting( "map", "dust" );
            ASSERT_TRUE( 1 == fixture.bobEvents.refusals && "setting" == fixture.bobEvents.refusedWhat );
            ASSERT_TRUE( "ice" == state.GetSetting( "map" ) );
            ASSERT_TRUE( 0 == fixture.aliceEvents.refusals );     // only the one who asked hears it

            fixture.alice->SetSetting( "map", "lava" );
            ASSERT_TRUE( 1 == fixture.aliceEvents.refusals );

            // Per member settings
            fixture.bob->SetSetting( "bob-id/handicap", "10" );
            ASSERT_TRUE( "10" == state.GetSetting( "bob-id/handicap" ) );
            fixture.bob->SetSetting( "alice-id/handicap", "10" );
            ASSERT_TRUE( 2 == fixture.bobEvents.refusals );

            // A spectator does not get ready, and a full lobby takes no player
            fixture.dave->SetReady( true );
            ASSERT_TRUE( 1 == fixture.daveEvents.refusals );
            CTestLobbyService late;
            MULTIPLAY::CPlayerIdentity erin;
            erin.userId = "erin-id";
            erin.displayName = "Erin";
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( late.Connect( erin, future ) );
            fixture.dave->SetSpectator( false );                   // dave plays: 4 players
            ASSERT_TRUE( 4 == state.GetPlayerCount() );
            ASSERT_TRUE( late.JoinLobby( fixture.lobbyId, CORE::CString::Empty, false, future ) );
            MULTIPLAY::CILobbyPtr erinLobby;
            ASSERT_FALSE( MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, erinLobby ) );
            ASSERT_TRUE( late.JoinLobby( fixture.lobbyId, CORE::CString::Empty, true, future ) );
            ASSERT_TRUE( MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, erinLobby ) );

            // Browsing finds the lobby
            ASSERT_TRUE( late.Browse( MULTIPLAY::CGameListingFilter(), future ) );
            MULTIPLAY::CGameListingList list;
            ASSERT_TRUE( MULTIPLAY::CGameListingList::TryGetFromFuture( future, list ) );
            ASSERT_TRUE( 1 == list.listings.size() );
            if ( 1 == list.listings.size() )
            {
                ASSERT_TRUE( fixture.lobbyId == list.listings[ 0 ].listingId && 4 == list.listings[ 0 ].players );
                ASSERT_TRUE( 1 == list.listings[ 0 ].spectators && "Alice" == list.listings[ 0 ].hostName );
                ASSERT_TRUE( list.listings[ 0 ].IsFull() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A player's change becomes a vote that passes, another proposal fails" )
        try
        {
            CLobbyFixture fixture;
            REQUIRE_TRUE( fixture.Setup() );
            const MULTIPLAY::CLobbyState& state = fixture.alice->GetState();

            fixture.bob->SetSetting( "speed", "5" );
            ASSERT_TRUE( 1 == fixture.aliceEvents.votesStarted && 1 == fixture.daveEvents.votesStarted );
            ASSERT_TRUE( state.activeVote.IsActive() && 2 == state.activeVote.needed && 1 == state.activeVote.yes );
            ASSERT_TRUE( "3" == state.GetSetting( "speed" ) );
            CORE::CString voteId = fixture.carolEvents.lastVote.id;

            fixture.carol->ProposeSetting( "speed", "1" );         // one vote at a time
            ASSERT_TRUE( 1 == fixture.carolEvents.refusals );
            fixture.dave->Vote( voteId, true );                    // spectators do not vote
            ASSERT_TRUE( 1 == fixture.daveEvents.refusals );
            fixture.bob->Vote( voteId, true );                     // the proposer voted already
            ASSERT_TRUE( 1 == fixture.bobEvents.refusals );

            fixture.carol->Vote( voteId, true );
            ASSERT_TRUE( 1 == fixture.aliceEvents.votesEnded && fixture.aliceEvents.lastVotePassed );
            ASSERT_TRUE( "5" == state.GetSetting( "speed" ) && !state.activeVote.IsActive() );

            fixture.carol->ProposeSetting( "speed", "1" );
            voteId = fixture.aliceEvents.lastVote.id;
            fixture.alice->Vote( voteId, false );
            ASSERT_TRUE( 1 == fixture.aliceEvents.votesEnded );     // undecided still
            fixture.bob->Vote( voteId, false );
            ASSERT_TRUE( 2 == fixture.aliceEvents.votesEnded && !fixture.aliceEvents.lastVotePassed );
            ASSERT_TRUE( "5" == state.GetSetting( "speed" ) );

            // The boss sets a vote setting directly
            fixture.alice->SetSetting( "speed", "2" );
            ASSERT_TRUE( "2" == state.GetSetting( "speed" ) && 2 == fixture.aliceEvents.votesStarted );

            // A vote nobody finishes runs out
            fixture.bob->ProposeSetting( "speed", "4" );
            ASSERT_TRUE( state.activeVote.IsActive() );
            ASSERT_TRUE( PumpUntilAtLeast( fixture.bobEvents.votesEnded, 3, 5000 ) );
            ASSERT_FALSE( fixture.bobEvents.lastVotePassed );
            ASSERT_TRUE( "2" == state.GetSetting( "speed" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Slots, factions, colours and slot kinds" )
        try
        {
            CLobbyFixture fixture;
            REQUIRE_TRUE( fixture.Setup() );
            const MULTIPLAY::CLobbyState& state = fixture.alice->GetState();

            fixture.bob->RequestSlot( 0 );
            ASSERT_TRUE( 1 == fixture.bobEvents.refusals && "slot" == fixture.bobEvents.refusedWhat );
            fixture.bob->RequestSlot( 3 );
            ASSERT_TRUE( 3 == state.FindMember( "bob-id" )->slot && state.FindSlot( 1 )->IsFree() );
            ASSERT_TRUE( "bob-id" == state.FindSlot( 3 )->memberId && MULTIPLAY::LOBBYSLOTKIND_HUMAN == state.FindSlot( 3 )->kind );

            fixture.bob->SetFaction( "core" );
            ASSERT_TRUE( "core" == state.FindSlot( 3 )->faction );
            fixture.bob->SetFaction( "zerg" );
            ASSERT_TRUE( 2 == fixture.bobEvents.refusals && "core" == state.FindSlot( 3 )->faction );

            fixture.alice->SetColour( "red" );
            fixture.bob->SetColour( "red" );
            ASSERT_TRUE( 3 == fixture.bobEvents.refusals && "colour" == fixture.bobEvents.refusedWhat );
            fixture.bob->SetColour( "purple" );
            ASSERT_TRUE( 4 == fixture.bobEvents.refusals );
            fixture.bob->SetColour( "blue" );
            ASSERT_TRUE( "red" == state.FindSlot( 0 )->colour && "blue" == state.FindSlot( 3 )->colour );

            fixture.alice->SetSlotKind( 1, MULTIPLAY::LOBBYSLOTKIND_AI, "easy" );
            ASSERT_TRUE( MULTIPLAY::LOBBYSLOTKIND_AI == state.FindSlot( 1 )->kind && "easy" == state.FindSlot( 1 )->aiType );
            fixture.bob->SetSlotKind( 1, MULTIPLAY::LOBBYSLOTKIND_CLOSED, CORE::CString::Empty );
            ASSERT_TRUE( 5 == fixture.bobEvents.refusals && "slotKind" == fixture.bobEvents.refusedWhat );
            fixture.alice->SetSlotKind( 3, MULTIPLAY::LOBBYSLOTKIND_CLOSED, CORE::CString::Empty );
            ASSERT_TRUE( 1 == fixture.aliceEvents.refusals );
            ASSERT_FALSE( state.FindSlot( 1 )->IsFree() );

            fixture.alice->SetSpectator( true );
            ASSERT_TRUE( 2 == fixture.aliceEvents.refusals );
            fixture.carol->SetSpectator( true );
            ASSERT_TRUE( state.FindMember( "carol-id" )->IsSpectator() && state.FindSlot( 2 )->IsFree() );

            fixture.alice->TransferBoss( "dave-id" );               // not to a spectator
            ASSERT_TRUE( 3 == fixture.aliceEvents.refusals );
            fixture.alice->TransferBoss( "bob-id" );
            ASSERT_TRUE( fixture.bob->IsBoss() && !fixture.alice->IsBoss() );

            fixture.bob->Kick( "carol-id" );
            ASSERT_TRUE( 1 == fixture.carolEvents.closed && "You were kicked" == fixture.carolEvents.closeReason );
            ASSERT_TRUE( GUCEF_NULL == state.FindMember( "carol-id" ) );
            ASSERT_FALSE( static_cast< CTestLobby* >( fixture.carol.GetPointerAlways() )->IsMember() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Ready, the countdown and its cancellation, the start vote and the launch for every member" )
        try
        {
            CLobbyFixture fixture;
            REQUIRE_TRUE( fixture.Setup() );
            const MULTIPLAY::CLobbyState& state = fixture.alice->GetState();
            fixture.alice->SetSetting( "map", "ice" );

            fixture.alice->RequestStart();
            ASSERT_TRUE( 1 == fixture.aliceEvents.refusals && "start" == fixture.aliceEvents.refusedWhat );
            fixture.bob->ForceStart();
            ASSERT_TRUE( 1 == fixture.bobEvents.refusals && "forceStart" == fixture.bobEvents.refusedWhat );

            ASSERT_TRUE( fixture.AllReady() );
            fixture.alice->RequestStart();
            ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_COUNTDOWN == state.phase && fixture.bobEvents.SawCountdown( 3 ) );

            // Unready during the countdown stops it
            fixture.bob->SetReady( false );
            ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_OPEN == state.phase && fixture.carolEvents.SawCountdown( -1 ) );
            PumpFor( 150 );
            ASSERT_TRUE( 0 == fixture.aliceEvents.launches );

            // A player starts a vote to start
            fixture.bob->SetReady( true );
            fixture.bob->RequestStart();
            ASSERT_TRUE( 1 == fixture.aliceEvents.votesStarted && "start" == fixture.aliceEvents.lastVote.what );
            fixture.carol->Vote( fixture.carolEvents.lastVote.id, true );
            ASSERT_TRUE( fixture.aliceEvents.lastVotePassed && MULTIPLAY::LOBBYPHASE_COUNTDOWN == state.phase );

            ASSERT_TRUE( PumpUntilAtLeast( fixture.daveEvents.launches, 1, 5000 ) );
            ASSERT_TRUE( fixture.aliceEvents.SawCountdown( 2 ) && fixture.aliceEvents.SawCountdown( 1 ) && fixture.aliceEvents.SawCountdown( 0 ) );
            ASSERT_TRUE( 1 == fixture.aliceEvents.launches && 1 == fixture.bobEvents.launches && 1 == fixture.carolEvents.launches );
            ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_IN_GAME == state.phase );

            // What every member gets
            const MULTIPLAY::CLaunchDescription& launch = fixture.bobEvents.launch;
            ASSERT_TRUE( "testgame" == launch.gameId && "1.0" == launch.gameVersion && fixture.lobbyId == launch.lobbyId );
            ASSERT_TRUE( MULTIPLAY::GAMETOPOLOGY_CLIENT_SERVER == launch.topology && "alice-id" == launch.serverMemberId );
            ASSERT_TRUE( 4 == launch.members.size() && 4 == launch.slots.size() );
            ASSERT_TRUE( "bob-id" == launch.localMemberId && "token:" + fixture.lobbyId + ":bob-id" == launch.token );
            ASSERT_FALSE( launch.spectator );
            ASSERT_TRUE( "ice" == launch.settings.GetValueAlways( "map" ).AsString() );
            const MULTIPLAY::CLaunchMember* carol = launch.FindMember( "carol-id" );
            ASSERT_TRUE( GUCEF_NULL != carol && 1 == carol->endpoints.size() && "udp://127.0.0.1:5003" == carol->endpoints[ 0 ] );
            CORE::CString::StringVector hostEndpoints;
            ASSERT_TRUE( launch.GetHostEndpoints( hostEndpoints ) );
            ASSERT_TRUE( 1 == hostEndpoints.size() && "udp://127.0.0.1:5001" == hostEndpoints[ 0 ] );
            ASSERT_FALSE( launch.IsLocalMemberHost() );
            ASSERT_TRUE( fixture.aliceEvents.launch.IsLocalMemberHost() );
            ASSERT_TRUE( fixture.daveEvents.launch.spectator && "dave-id" == fixture.daveEvents.launch.localMemberId );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A force start, then a spectator joins the game in progress; a player cannot" )
        try
        {
            CLobbyFixture fixture;
            fixture.params.topology = MULTIPLAY::GAMETOPOLOGY_PEER_TO_PEER;
            REQUIRE_TRUE( fixture.Setup() );

            fixture.alice->ForceStart();
            ASSERT_TRUE( 0 == fixture.aliceEvents.refusals );
            ASSERT_TRUE( PumpUntilAtLeast( fixture.aliceEvents.launches, 1, 5000 ) );
            ASSERT_TRUE( "alice-id" == fixture.aliceEvents.launch.sequencerMemberId && fixture.aliceEvents.launch.serverMemberId.IsNULLOrEmpty() );

            CTestLobbyService late;
            MULTIPLAY::CPlayerIdentity erin;
            erin.userId = "erin-id";
            erin.displayName = "Erin";
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( late.Connect( erin, future ) );

            // The listing shows a game in progress
            ASSERT_TRUE( late.Browse( MULTIPLAY::CGameListingFilter(), future ) );
            MULTIPLAY::CGameListingList list;
            ASSERT_TRUE( MULTIPLAY::CGameListingList::TryGetFromFuture( future, list ) && 1 == list.listings.size() );
            if ( 1 == list.listings.size() )
                ASSERT_TRUE( MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS == list.listings[ 0 ].kind );

            ASSERT_TRUE( late.JoinLobby( fixture.lobbyId, CORE::CString::Empty, false, future ) );
            MULTIPLAY::CILobbyPtr erinLobby;
            ASSERT_FALSE( MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, erinLobby ) );

            ASSERT_TRUE( late.JoinLobby( fixture.lobbyId, CORE::CString::Empty, true, future ) );
            ASSERT_TRUE( MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, erinLobby ) );
            ASSERT_TRUE( 4 == fixture.aliceEvents.joined );
            MULTIPLAY::CLaunchDescription erinLaunch = static_cast< CTestLobbyHost* >( CTestLobbyService::FindHost( fixture.lobbyId ) )->MakeLaunchDescription( "erin-id" );
            ASSERT_TRUE( erinLaunch.spectator && 5 == erinLaunch.members.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The boss leaves and the boss passes on; a vote ends with its proposer; a LAN host leaving closes the lobby" )
        try
        {
            {
                CLobbyFixture fixture;
                REQUIRE_TRUE( fixture.Setup() );
                const MULTIPLAY::CLobbyState& state = fixture.bob->GetState();

                fixture.carol->ProposeSetting( "speed", "4" );
                ASSERT_TRUE( state.activeVote.IsActive() );
                fixture.carol->Leave();
                ASSERT_TRUE( 1 == fixture.bobEvents.votesEnded && !fixture.bobEvents.lastVotePassed );
                ASSERT_FALSE( state.activeVote.IsActive() );

                ASSERT_TRUE( fixture.alice->Leave() );
                ASSERT_TRUE( fixture.bob->IsBoss() && "bob-id" == state.bossId );
                ASSERT_TRUE( 2 == fixture.bobEvents.left && 0 == fixture.bobEvents.closed );
                ASSERT_TRUE( fixture.bobEvents.changedPaths.find( "boss" ) != fixture.bobEvents.changedPaths.end() );
                ASSERT_FALSE( static_cast< CTestLobby* >( fixture.alice.GetPointerAlways() )->IsMember() );
                ASSERT_FALSE( fixture.alice->SetReady( true ) );
            }
            {
                CLobbyFixture fixture;
                fixture.rules.closeWhenHostLeaves = true;
                REQUIRE_TRUE( fixture.Setup() );
                fixture.alice->Leave();
                ASSERT_TRUE( 1 == fixture.bobEvents.closed && 1 == fixture.daveEvents.closed );
                ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_CLOSED == fixture.bob->GetState().phase );
                ASSERT_FALSE( static_cast< CTestLobby* >( fixture.bob.GetPointerAlways() )->IsMember() );
            }
            {
                // Dropping the lobby object leaves the lobby
                CLobbyFixture fixture;
                REQUIRE_TRUE( fixture.Setup() );
                fixture.dave = MULTIPLAY::CILobbyPtr();
                ASSERT_TRUE( 3 == fixture.alice->GetState().members.size() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The launch description survives a JSON text" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        try
        {
            MULTIPLAY::CLaunchDescription launch;
            launch.gameId = "testgame";
            launch.gameVersion = "1.0";
            launch.lobbyId = "lobby-9";
            launch.sessionName = "lobby-9-game";
            launch.topology = MULTIPLAY::GAMETOPOLOGY_PEER_TO_PEER;
            launch.sequencerMemberId = "a";
            MULTIPLAY::CLaunchMember member;
            member.id = "a";
            member.name = "Alice";
            member.role = MULTIPLAY::LOBBYMEMBERROLE_BOSS;
            member.slot = 0;
            member.endpoints.push_back( "tcp://10.0.0.1:4000" );
            member.endpoints.push_back( "ws://10.0.0.1:4001/game" );
            launch.members.push_back( member );
            member.id = "s";
            member.name = "Spec";
            member.role = MULTIPLAY::LOBBYMEMBERROLE_SPECTATOR;
            member.slot = -1;
            member.endpoints.clear();
            launch.members.push_back( member );
            launch.settings.Set( "map", "ice" );
            MULTIPLAY::CLobbySlot slot;
            slot.index = 0;
            slot.faction = "arm";
            slot.memberId = "a";
            slot.kind = MULTIPLAY::LOBBYSLOTKIND_HUMAN;
            launch.slots.push_back( slot );
            launch.localMemberId = "s";
            launch.token = "signed-token";
            launch.relay = "udp://relay.example:3478";
            launch.spectator = true;

            CORE::CString json;
            ASSERT_TRUE( launch.ToJsonString( json ) );
            ASSERT_TRUE( 0 <= json.HasSubstr( "signed-token" ) );

            MULTIPLAY::CLaunchDescription copy;
            ASSERT_TRUE( copy.FromJsonString( json ) );
            ASSERT_TRUE( "testgame" == copy.gameId && "lobby-9" == copy.lobbyId && "lobby-9-game" == copy.sessionName );
            ASSERT_TRUE( MULTIPLAY::GAMETOPOLOGY_PEER_TO_PEER == copy.topology && "a" == copy.sequencerMemberId );
            ASSERT_TRUE( 2 == copy.members.size() && 1 == copy.slots.size() );
            if ( 2 == copy.members.size() )
            {
                ASSERT_TRUE( 2 == copy.members[ 0 ].endpoints.size() && "ws://10.0.0.1:4001/game" == copy.members[ 0 ].endpoints[ 1 ] );
                ASSERT_TRUE( MULTIPLAY::LOBBYMEMBERROLE_SPECTATOR == copy.members[ 1 ].role && -1 == copy.members[ 1 ].slot );
                ASSERT_TRUE( copy.members[ 1 ].endpoints.empty() );
            }
            ASSERT_TRUE( "ice" == copy.settings.GetValueAlways( "map" ).AsString() );
            if ( 1 == copy.slots.size() )
                ASSERT_TRUE( "arm" == copy.slots[ 0 ].faction && "a" == copy.slots[ 0 ].memberId );
            ASSERT_TRUE( "s" == copy.localMemberId && "signed-token" == copy.token && "udp://relay.example:3478" == copy.relay );
            ASSERT_TRUE( copy.spectator );
            CORE::CString::StringVector hostEndpoints;
            ASSERT_TRUE( copy.GetHostEndpoints( hostEndpoints ) && 2 == hostEndpoints.size() );

            ASSERT_FALSE( copy.FromJsonString( "this is not json" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CTestLobbyService::Reset();
    MULTIPLAY::CMultiplayGlobal::Instance()->GetLobbyServiceFactory().UnregisterConcreteFactory( CTestLobbyService::ServiceName );
}

/*-------------------------------------------------------------------------*/
