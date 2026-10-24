/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_MULTIPLAY_CLOBBYSTATE_H
#define GUCEF_MULTIPLAY_CLOBBYSTATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

enum ELobbyPhase
{
    LOBBYPHASE_OPEN      = 0 ,
    LOBBYPHASE_COUNTDOWN = 1 ,
    LOBBYPHASE_LAUNCHING = 2 ,
    LOBBYPHASE_IN_GAME   = 3 ,
    LOBBYPHASE_CLOSED    = 4
};
typedef enum ELobbyPhase TLobbyPhase;

enum ELobbyMemberRole
{
    LOBBYMEMBERROLE_BOSS      = 0 ,
    LOBBYMEMBERROLE_PLAYER    = 1 ,
    LOBBYMEMBERROLE_SPECTATOR = 2
};
typedef enum ELobbyMemberRole TLobbyMemberRole;

enum ELobbySlotKind
{
    LOBBYSLOTKIND_OPEN   = 0 ,
    LOBBYSLOTKIND_CLOSED = 1 ,
    LOBBYSLOTKIND_HUMAN  = 2 ,
    LOBBYSLOTKIND_AI     = 3
};
typedef enum ELobbySlotKind TLobbySlotKind;

/** How the game connects once it starts */
enum EGameTopology
{
    GAMETOPOLOGY_CLIENT_SERVER = 0 ,     /**< the host (or a chosen member) is the server */
    GAMETOPOLOGY_PEER_TO_PEER  = 1
};
typedef enum EGameTopology TGameTopology;

GUCEF_MULTIPLAY_EXPORT_CPP const char* LobbyPhaseToString( TLobbyPhase phase );
GUCEF_MULTIPLAY_EXPORT_CPP TLobbyPhase StringToLobbyPhase( const CORE::CString& phase );
GUCEF_MULTIPLAY_EXPORT_CPP const char* LobbyMemberRoleToString( TLobbyMemberRole role );
GUCEF_MULTIPLAY_EXPORT_CPP TLobbyMemberRole StringToLobbyMemberRole( const CORE::CString& role );
GUCEF_MULTIPLAY_EXPORT_CPP const char* LobbySlotKindToString( TLobbySlotKind kind );
GUCEF_MULTIPLAY_EXPORT_CPP TLobbySlotKind StringToLobbySlotKind( const CORE::CString& kind );
GUCEF_MULTIPLAY_EXPORT_CPP const char* GameTopologyToString( TGameTopology topology );
GUCEF_MULTIPLAY_EXPORT_CPP TGameTopology StringToGameTopology( const CORE::CString& topology );

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A member of a lobby */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbyMember
{
    public:

    CORE::CString id;
    CORE::CString name;
    TLobbyMemberRole role;
    bool ready;
    Int32 slot;                 /**< the index of the slot, -1 for none */
    Int32 latencyMs;            /**< -1 when unknown */
    UInt32 joinOrder;           /**< lower joined earlier: the boss passes to the longest present player */

    CLobbyMember( void );

    bool IsSpectator( void ) const;

    void Serialize( CORE::CDataNode& node ) const;

    void Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------*/

/** A place in the game: a team position that a human or an AI fills */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbySlot
{
    public:

    UInt32 index;
    UInt32 team;
    CORE::CString faction;
    CORE::CString colour;
    TLobbySlotKind kind;
    CORE::CString aiType;
    CORE::CString memberId;     /**< the member in it, for a HUMAN slot */

    CLobbySlot( void );

    bool IsFree( void ) const;

    void Serialize( CORE::CDataNode& node ) const;

    void Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------*/

/** A vote that runs in a lobby */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbyVote
{
    public:

    CORE::CString id;               /**< empty when no vote runs */
    CORE::CString what;             /**< "setting" or "start" */
    CORE::CString proposer;         /**< member id */
    CORE::CString key;              /**< the setting voted on */
    CORE::CString value;            /**< the value proposed for it */
    UInt32 yes;
    UInt32 no;
    UInt32 needed;                  /**< the yes votes that pass it */
    CORE::CDateTime deadline;
    CORE::CString::StringSet voters;

    CLobbyVote( void );

    bool IsActive( void ) const;

    void Clear( void );

    void Serialize( CORE::CDataNode& node ) const;

    void Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------*/

/**
 *  Everything a lobby is at a moment, the same for every member
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbyState : public CORE::CIDataNodeSerializable
{
    public:

    typedef GUCEF::vector< CLobbyMember > TMemberVector;
    typedef GUCEF::vector< CLobbySlot > TSlotVector;

    CORE::CString lobbyId;
    CORE::CString name;
    TLobbyPhase phase;
    CORE::CString bossId;
    TMemberVector members;
    TSlotVector slots;
    CORE::CValueList settings;
    CLobbyVote activeVote;
    CORE::CString chatBackend;
    CORE::CString chatChannel;

    CLobbyState( void );

    CLobbyState( const CLobbyState& src );

    virtual ~CLobbyState();

    CLobbyState& operator=( const CLobbyState& src );

    CLobbyMember* FindMember( const CORE::CString& memberId );
    const CLobbyMember* FindMember( const CORE::CString& memberId ) const;

    CLobbySlot* FindSlot( UInt32 index );
    const CLobbySlot* FindSlot( UInt32 index ) const;

    /** The slot the member is in, if any */
    CLobbySlot* FindSlotOfMember( const CORE::CString& memberId );
    const CLobbySlot* FindSlotOfMember( const CORE::CString& memberId ) const;

    /** Members that play, the boss included */
    UInt32 GetPlayerCount( void ) const;

    UInt32 GetSpectatorCount( void ) const;

    /** The value of a setting, empty when it has none */
    CORE::CString GetSetting( const CORE::CString& key ) const;

    void SetSetting( const CORE::CString& key, const CORE::CString& value );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CLOBBYSTATE_H ? */
