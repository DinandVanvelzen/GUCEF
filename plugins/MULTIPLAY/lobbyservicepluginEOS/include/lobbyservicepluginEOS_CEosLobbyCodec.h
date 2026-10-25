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

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYCODEC_H
#define GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#include "gucefMULTIPLAY_CGameListingEntry.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSTATE_H
#include "gucefMULTIPLAY_CLobbyState.h"
#define GUCEF_MULTIPLAY_CLOBBYSTATE_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSTATE_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#include "pluginglueEOS_EosShim.h"
#define GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#endif /* GUCEF_PLUGINGLUE_EOS_EOSSHIM_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_MACROS_H
#include "lobbyservicepluginEOS_macros.h"
#define GUCEF_MULTIPLAYPLUGIN_EOS_MACROS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_MACROS_H ? */

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
 *  What the lobby interface of EOS accepts, as assumed here (the SDK's names are EOS_LOBBY_MAX_LOBBY_MEMBERS,
 *  EOS_LOBBYMODIFICATION_MAX_ATTRIBUTES and EOS_LOBBYMODIFICATION_MAX_ATTRIBUTE_LENGTH: TO CONFIRM against the SDK)
 *  and what the plugin allows itself. The splitting of the data of a lobby over attributes stays inside these limits
 *  and fails with a reason when the data does not fit; a game with another account of Epic that has other limits
 *  sets them in the config of the service.
 *
 *  Config: maxLobbyMembers, maxLobbyAttributes, maxAttributeLength, maxAttributeKeyLength, maxSearchResults, maxBlobBytes
 */
class GUCEF_MULTIPLAYPLUGIN_EOS_EXPORT_CPP CEosLobbyLimits : public CORE::CIConfigurable
{
    public:

    UInt32 maxLobbyMembers;         /**< 64 */
    UInt32 maxLobbyAttributes;      /**< 64 attributes of a lobby */
    UInt32 maxAttributeLength;      /**< 1000 characters of a value */
    UInt32 maxAttributeKeyLength;   /**< 64 characters of a key */
    UInt32 maxSearchResults;        /**< 200 */
    UInt32 maxBlobBytes;            /**< the most that is decoded of data that came from another member, 65536 */

    CEosLobbyLimits( void );

    CEosLobbyLimits( const CEosLobbyLimits& src );

    virtual ~CEosLobbyLimits();

    CEosLobbyLimits& operator=( const CEosLobbyLimits& src );

    bool IsWellFormed( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** The fixed attributes of a lobby that the owner writes: what a search shows without decoding the rest */
class GUCEF_MULTIPLAYPLUGIN_EOS_EXPORT_CPP CEosLobbySummary
{
    public:

    UInt32 wireVersion;
    CORE::CString stamp;                    /**< the Product User Id of whoever wrote the attributes, which has to be the owner of the lobby */
    CORE::CString name;
    CORE::CString hostName;
    CORE::CString gameId;
    CORE::CString gameVersion;
    MULTIPLAY::TGameTopology topology;
    bool passworded;
    CORE::CString passwordSalt;             /**< hex, when passworded */
    bool allowSpectators;
    UInt32 maxPlayers;
    UInt32 spectatorSlots;
    MULTIPLAY::TLobbyPhase phase;
    UInt32 players;
    UInt32 spectators;
    bool isPublic;
    UInt32 revision;                        /**< counts up with every publication of the state of the owner */

    CEosLobbySummary( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  Everything of the lobby data that is carried as attributes: the names of the attributes, the splitting of larger
 *  data over several attributes and putting it back together, and the checks on data that came from another member.
 *
 *  Attributes of a lobby, written by the owner only (the SDK lets nobody else change them), all public:
 *    v      wire version (1)              by     Product User Id of the writer, the owner
 *    nm     name                          hn     name of the host
 *    gid    game id                       gv     game version
 *    topo   topology (0 server, 1 peers)  pw     1 when there is a password      pws   its salt, hex
 *    sp     spectators allowed (0 / 1)    mx     maximum of players              sps   places for spectators
 *    ph     phase                         pl     players                         sc    spectators
 *    pub    public (1) or not (0)         rv     revision of the published state
 *    s_<key>   a setting that is mirrored for searching (the config mirrorSettings)
 *    df_*   the definition of the lobby: the create parameters without the password, and the rules
 *    st_*   the state: members, slots, settings, vote, countdown, the acknowledgements of requests
 *    la_*   the launch description of the game, when it started
 *  Data that is too large for an attribute is a blob: a data tree in the DRN wire form (a version byte first), in
 *  base64, cut in pieces of at most maxAttributeLength characters: <prefix>_n the number of pieces, <prefix>_h the first
 *  16 hex digits of the SHA-256 of the whole text, <prefix>_0 .. the pieces.
 *
 *  Attributes of a member, written by that member (the SDK lets nobody else change them):
 *    n      display name         pwp    proof of the lobby password      sp     1 when joining as a spectator
 *    rdy    "<seq>:<0|1>"        rq     "<seq>|<verb>|<arg>|<arg>" a request to the owner, the sequence counts up
 *  Every other key of a member is ignored.
 */
class GUCEF_MULTIPLAYPLUGIN_EOS_EXPORT_CPP CEosLobbyCodec
{
    public:

    static const UInt32 WireVersion = 1;

    static const CORE::CString KeyVersion;
    static const CORE::CString KeyStamp;
    static const CORE::CString KeyName;
    static const CORE::CString KeyHostName;
    static const CORE::CString KeyGameId;
    static const CORE::CString KeyGameVersion;
    static const CORE::CString KeyTopology;
    static const CORE::CString KeyPassworded;
    static const CORE::CString KeyPasswordSalt;
    static const CORE::CString KeyAllowSpectators;
    static const CORE::CString KeyMaxPlayers;
    static const CORE::CString KeySpectatorSlots;
    static const CORE::CString KeyPhase;
    static const CORE::CString KeyPlayers;
    static const CORE::CString KeySpectators;
    static const CORE::CString KeyPublic;
    static const CORE::CString KeyRevision;
    static const CORE::CString MirrorPrefix;
    static const CORE::CString BlobDefinition;
    static const CORE::CString BlobState;
    static const CORE::CString BlobLaunch;

    static const CORE::CString MemberKeyName;
    static const CORE::CString MemberKeyPasswordProof;
    static const CORE::CString MemberKeySpectator;
    static const CORE::CString MemberKeyReady;
    static const CORE::CString MemberKeyRequest;

    /** The attributes of the lobby, other than blobs, that follow from the summary */
    static void EncodeSummary( const CEosLobbySummary& summary, PLUGINGLUE::EOS::TEosAttributeVector& attributes );

    /**
     *  Reads the fixed attributes of a lobby and checks them: the version is known, the stamp is the owner, every text
     *  and number is within its bounds
     */
    static bool DecodeSummary( const PLUGINGLUE::EOS::TEosAttributeVector& attributes, const CORE::CString& ownerPuid, CEosLobbySummary& summary, CORE::CString& reason );

    /** A data tree as a blob of attributes under the prefix; false with the reason when it does not fit the limits */
    static bool EncodeBlob( const CORE::CString& prefix, const CORE::CDataNode& tree, const CEosLobbyLimits& limits, PLUGINGLUE::EOS::TEosAttributeVector& attributes, CORE::CString& reason );

    /** Puts a blob together from the attributes and reads the tree. The data is not trusted: sizes, the hash and the depth are checked */
    static bool DecodeBlob( const CORE::CString& prefix, const PLUGINGLUE::EOS::TEosAttributeVector& attributes, const CEosLobbyLimits& limits, CORE::CDataNode& tree, CORE::CString& reason );

    /** The keys of the attributes a blob of that prefix is made of, for removing it */
    static void GetBlobKeys( const CORE::CString& prefix, const PLUGINGLUE::EOS::TEosAttributeVector& attributes, CORE::CString::StringVector& keys );

    /** The number of attributes a blob of that many characters takes */
    static UInt32 GetBlobAttributeCount( UInt32 textLength, const CEosLobbyLimits& limits );

    static const PLUGINGLUE::EOS::SEosAttribute* FindAttribute( const PLUGINGLUE::EOS::TEosAttributeVector& attributes, const CORE::CString& key );

    static bool GetString( const PLUGINGLUE::EOS::TEosAttributeVector& attributes, const CORE::CString& key, CORE::CString& value );

    static bool GetInt( const PLUGINGLUE::EOS::TEosAttributeVector& attributes, const CORE::CString& key, Int64& value );

    static PLUGINGLUE::EOS::SEosAttribute MakeString( const CORE::CString& key, const CORE::CString& value );

    static PLUGINGLUE::EOS::SEosAttribute MakeInt( const CORE::CString& key, Int64 value );

    /** Whether a key is one the codec writes: letters, digits and '_', at most maxAttributeKeyLength characters */
    static bool IsValidKey( const CORE::CString& key, const CEosLobbyLimits& limits );

    /** A name or text from another member: control characters out, cut at the limit on a character boundary */
    static CORE::CString SanitizeText( const CORE::CString& text, UInt32 maxBytes );

    /** The lobby ids of EOS are letters and digits */
    static bool IsValidLobbyId( const CORE::CString& lobbyId );

    static bool IsValidUserId( const CORE::CString& userId );

    /** The proof of a member that it knows the password: SHA-256 over a domain text, the salt of the lobby, the user and the password, hex */
    static CORE::CString MakePasswordProof( const CORE::CString& salt, const CORE::CString& userId, const CORE::CString& password );

    /** A random salt, 16 bytes in hex; empty when the system has no randomness */
    static CORE::CString MakeSalt( void );

    /** The name of a socket for a lobby: the kind (2 letters) and 24 hex digits of the SHA-256 of the lobby id, 26 characters */
    static CORE::CString MakeSocketName( const CORE::CString& kind, const CORE::CString& lobbyId );

    /** A listing of a lobby found by a search; false with the reason when the lobby does not hold up */
    static bool MakeListing( const PLUGINGLUE::EOS::SEosLobbyDetails& details, const CEosLobbyLimits& limits, MULTIPLAY::CGameListingEntry& listing, CORE::CString& reason );

    /** Splits "<seq>|<verb>|<a>|<b>" of a request; the parts are cut at 256 bytes, false when it is not one */
    static bool ParseRequest( const CORE::CString& text, UInt32& sequence, CORE::CString& verb, CORE::CString& arg1, CORE::CString& arg2 );

    static CORE::CString MakeRequest( UInt32 sequence, const CORE::CString& verb, const CORE::CString& arg1, const CORE::CString& arg2 );

    /** "<seq>:<0|1>" */
    static bool ParseReady( const CORE::CString& text, UInt32& sequence, bool& ready );

    private:

    CEosLobbyCodec( void );
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

#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYCODEC_H ? */
