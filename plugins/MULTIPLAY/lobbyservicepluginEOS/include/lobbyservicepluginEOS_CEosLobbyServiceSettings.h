/*
 *  lobbyservicepluginEOS: gucefMULTIPLAY lobby service plugin for lobbies on Epic Online Services
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

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICESETTINGS_H
#define GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CLOBBYRULES_H
#include "gucefMULTIPLAY_CLobbyRules.h"
#define GUCEF_MULTIPLAY_CLOBBYRULES_H
#endif /* GUCEF_MULTIPLAY_CLOBBYRULES_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYCODEC_H
#include "lobbyservicepluginEOS_CEosLobbyCodec.h"
#define GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYCODEC_H
#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYCODEC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of the "eos" lobby service, the config of the service (section of the app, or the "ServiceConfig" of
 *  the lobby-service listing provider). Settings are per game, not per player.
 *
 *    bucketId                    the bucket of EOS the lobbies of the game live in (EOS_Lobby search is per bucket);
 *                                default "gucef-<game id>" ("gucef" without a game id)
 *    permission                  public (found by search, joined by id), presence (EOS_LPL_JOINVIAPRESENCE) or invite
 *                                (EOS_LPL_INVITEONLY): the permission level of a lobby; default: public for a public lobby,
 *                                invite otherwise
 *    presenceEnabled             bPresenceEnabled of the lobby, default false
 *    mirrorSettings              ',' separated keys of settings that are mirrored as search attributes s_<key>
 *    spectatorSlots              places for spectators next to the places for players (default 8)
 *    secondInMs                  what a second of the countdown and of a vote lasts (default 1000)
 *    requestTimeoutInMs          how long a request to the SDK may take (default 15000)
 *    joinTimeoutInMs             how long joining may take, up to the acceptance by the owner (default 15000)
 *    joinGraceInMs               how long the owner waits for the name and the password proof of a member that joined (default 5000)
 *    departureGraceInMs          how long a member that was refused or kicked has to leave by itself, and the members of a lobby
 *                                that is closed have to read why, before the owner has EOS remove it or destroy the lobby;
 *                                a member that leaves in time needs no kick (default 2000)
 *    launchTimeoutInMs           how long a member waits for its launch token from the owner (default 15000)
 *    tokenLifetimeInSecs         validity of the tokens of a game (default 3600)
 *    updatesPerSecond, updateBurst              rate limit of the owner's UpdateLobby calls (default 4 and 8)
 *    memberRequestsPerSecond, memberRequestBurst   rate limit of the requests of one member that the owner takes (default 5 and 10)
 *    tickIntervalInMs            how often the platform is ticked while the service has work (default 20)
 *    browseMaxResults            the most lobbies a browse returns (default 50)
 *    relayControl                allow (default), none or force: the relay control of the P2P interface
 *    allowUnsignedLaunch         start a game without signed tokens when there is no signature codec (default false)
 *    Limits                      child: the limits of the lobby interface (CEosLobbyLimits)
 *    Rules                       child: the rules of the lobbies the owner runs (MULTIPLAY::CLobbyRules: minPlayers, requireAllReady,
 *                                allowForceStart, startByVote, countdownInSecs, voteDurationInSecs); the maximum of players and the
 *                                spectators are the lobby's own. A game sets them in its config, they are the game's, not a player's
 */
class GUCEF_MULTIPLAYPLUGIN_EOS_EXPORT_CPP CEosLobbyServiceSettings : public CORE::CIConfigurable
{
    public:

    CORE::CString bucketId;
    CORE::CString permission;
    bool presenceEnabled;
    CORE::CString::StringVector mirrorSettings;
    UInt32 spectatorSlots;
    UInt32 secondInMs;
    UInt32 requestTimeoutInMs;
    UInt32 joinTimeoutInMs;
    UInt32 joinGraceInMs;
    UInt32 departureGraceInMs;
    UInt32 launchTimeoutInMs;
    UInt32 tokenLifetimeInSecs;
    UInt32 updatesPerSecond;
    UInt32 updateBurst;
    UInt32 memberRequestsPerSecond;
    UInt32 memberRequestBurst;
    UInt32 tickIntervalInMs;
    UInt32 browseMaxResults;
    PLUGINGLUE::EOS::EEosRelayControl relayControl;
    bool allowUnsignedLaunch;
    CEosLobbyLimits limits;
    MULTIPLAY::CLobbyRules rules;

    CEosLobbyServiceSettings( void );

    CEosLobbyServiceSettings( const CEosLobbyServiceSettings& src );

    virtual ~CEosLobbyServiceSettings();

    CEosLobbyServiceSettings& operator=( const CEosLobbyServiceSettings& src );

    /** The bucket for lobbies of a game */
    CORE::CString GetBucketId( const CORE::CString& gameId ) const;

    /** Whether the key of a setting is one that is mirrored */
    bool IsMirrored( const CORE::CString& key ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICESETTINGS_H ? */
