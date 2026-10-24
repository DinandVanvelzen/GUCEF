/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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

#ifndef GUCEF_GAMEGUI_TESTAPP_TESTLOBBYFIXTURE_H
#define GUCEF_GAMEGUI_TESTAPP_TESTLOBBYFIXTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "TestLobbyService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A lobby with a boss (alice), two players (bob, carol) and a spectator (dave), each through a client of the
 *  in-memory lobby service, the settings of every type and the rules of the MULTIPLAY tests. A lobby "second" is
 *  20 ms, so countdowns and votes run quickly when the test pumps the pulse generator.
 */
class CLobbyFixture
{
    public:

    GUCEF::MULTIPLAY::CLobbyRules rules;
    GUCEF::MULTIPLAY::CLobbyCreateParams params;
    CTestLobbyService services[ 4 ];
    GUCEF::MULTIPLAY::CILobbyPtr alice;
    GUCEF::MULTIPLAY::CILobbyPtr bob;
    GUCEF::MULTIPLAY::CILobbyPtr carol;
    GUCEF::MULTIPLAY::CILobbyPtr dave;
    GUCEF::CORE::CString lobbyId;

    CLobbyFixture( void )
    {
        params.name = "Test lobby";
        params.gameId = "testgame";
        params.gameVersion = "1.0";
        params.maxPlayers = 4;
        params.allowSpectators = true;
        params.isPublic = false;
        params.topology = GUCEF::MULTIPLAY::GAMETOPOLOGY_CLIENT_SERVER;

        GUCEF::MULTIPLAY::CLobbySettingDef map;
        map.key = "map";
        map.label = "Map";
        map.type = GUCEF::MULTIPLAY::CLobbySettingDef::TYPE_ENUM;
        map.options.push_back( "dust" );
        map.options.push_back( "ice" );
        map.defaultValue = "dust";
        map.changePolicy = GUCEF::MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
        params.schema.AddSetting( map );

        GUCEF::MULTIPLAY::CLobbySettingDef speed;
        speed.key = "speed";
        speed.label = "Game speed";
        speed.type = GUCEF::MULTIPLAY::CLobbySettingDef::TYPE_INT;
        speed.hasRange = true;
        speed.minValue = 1;
        speed.maxValue = 5;
        speed.defaultValue = "3";
        speed.changePolicy = GUCEF::MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_VOTE;
        params.schema.AddSetting( speed );

        GUCEF::MULTIPLAY::CLobbySettingDef handicap;
        handicap.key = "handicap";
        handicap.label = "Handicap";
        handicap.type = GUCEF::MULTIPLAY::CLobbySettingDef::TYPE_INT;
        handicap.hasRange = true;
        handicap.minValue = 0;
        handicap.maxValue = 100;
        handicap.defaultValue = "0";
        handicap.changePolicy = GUCEF::MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_OWNER;
        params.schema.AddSetting( handicap );

        GUCEF::MULTIPLAY::CLobbySettingDef fog;
        fog.key = "fog";
        fog.label = "Fog of war";
        fog.type = GUCEF::MULTIPLAY::CLobbySettingDef::TYPE_BOOL;
        fog.defaultValue = "false";
        fog.changePolicy = GUCEF::MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_ANY;
        params.schema.AddSetting( fog );

        GUCEF::MULTIPLAY::CLobbySettingDef gravity;
        gravity.key = "gravity";
        gravity.label = "Gravity";
        gravity.type = GUCEF::MULTIPLAY::CLobbySettingDef::TYPE_FLOAT;
        gravity.hasRange = true;
        gravity.minValue = 0.5;
        gravity.maxValue = 2.0;
        gravity.step = 0.1;
        gravity.defaultValue = "1.0";
        gravity.group = "Physics";
        gravity.changePolicy = GUCEF::MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
        params.schema.AddSetting( gravity );

        GUCEF::MULTIPLAY::CLobbySettingDef motd;
        motd.key = "motd";
        motd.label = "Message";
        motd.type = GUCEF::MULTIPLAY::CLobbySettingDef::TYPE_STRING;
        motd.defaultValue = "hello";
        motd.group = "Physics";
        motd.changePolicy = GUCEF::MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
        params.schema.AddSetting( motd );

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
        alice = GUCEF::MULTIPLAY::CILobbyPtr();
        bob = GUCEF::MULTIPLAY::CILobbyPtr();
        carol = GUCEF::MULTIPLAY::CILobbyPtr();
        dave = GUCEF::MULTIPLAY::CILobbyPtr();
        CTestLobbyService::Reset();
    }

    bool Connect( GUCEF::UInt32 index, const char* id, const char* name )
    {
        GUCEF::MULTIPLAY::CPlayerIdentity identity;
        identity.userId = id;
        identity.displayName = name;
        CTestLobbyService& service = services[ index ];
        service.SetRules( rules );
        service.SetSecondInMs( 20 );
        GUCEF::CORE::CString::StringVector endpoints;
        endpoints.push_back( "udp://127.0.0.1:" + GUCEF::CORE::ToString( 5001 + index ) );
        service.SetLocalEndpoints( endpoints );
        GUCEF::CORE::CFutureResult future( GUCEF::CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        return service.Connect( identity, future ) && service.IsConnected();
    }

    GUCEF::MULTIPLAY::CILobbyPtr Join( GUCEF::UInt32 index, bool asSpectator )
    {
        GUCEF::CORE::CFutureResult future( GUCEF::CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        GUCEF::MULTIPLAY::CILobbyPtr lobby;
        if ( services[ index ].JoinLobby( lobbyId, GUCEF::CORE::CString::Empty, asSpectator, future ) )
            GUCEF::MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, lobby );
        return lobby;
    }

    /** Alice makes the lobby; the others join when asked to */
    bool Setup( bool withOthers = true )
    {
        if ( !Connect( 0, "alice-id", "Alice" ) || !Connect( 1, "bob-id", "Bob" ) ||
             !Connect( 2, "carol-id", "Carol" ) || !Connect( 3, "dave-id", "Dave" ) )
            return false;

        GUCEF::CORE::CFutureResult future( GUCEF::CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        if ( !services[ 0 ].CreateLobby( params, future ) || !GUCEF::MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, alice ) )
            return false;
        lobbyId = alice->GetState().lobbyId;
        if ( !withOthers )
            return true;

        bob = Join( 1, false );
        carol = Join( 2, false );
        dave = Join( 3, true );
        return !bob.IsNULL() && !carol.IsNULL() && !dave.IsNULL();
    }

    CTestLobbyHost* GetHost( void )
    {
        return CTestLobbyService::FindHost( lobbyId );
    }

    bool AllReady( void )
    {
        return alice->SetReady( true ) && bob->SetReady( true ) && carol->SetReady( true );
    }
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_TESTAPP_TESTLOBBYFIXTURE_H ? */
