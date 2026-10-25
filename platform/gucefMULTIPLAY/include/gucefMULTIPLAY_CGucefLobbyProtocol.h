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

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#define GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the built-in `gucef` lobby server and its clients agree on: the names of the sessions, grids and streams,
 *  the requests and the messages, and how data trees travel.
 *
 *  Directory session "gucef.directory" (CLIENT_SERVER, the server is the sequencer):
 *      grid "listings" (HOST): listing id -> listing (a data tree, see WriteDataTree)
 *      stream "requests" of every client: { request id, verb, arguments }
 *      streams "chat/<channel>" of the server: { kind ("msg", "joined", "left"), from, text, time (ms), id }
 *      app messages of the server: REPLY, DIRECT_MESSAGE
 *  Lobby session "gucef.lobby.<lobby id>" (CLIENT_SERVER, the server is the sequencer):
 *      grid "lobby" (HOST): name, phase, boss, countdown, settings/<key>, vote/..., chat/...
 *      grid "members" (HOST): <member id>/<field>; a member writes its own ready, slot, faction, colour, spectator
 *      grid "slots" (HOST): <index>/<field>, count
 *      grid "schema" (HOST): schema -> the settings schema (a data tree)
 *      grid "proposals" (PROPOSAL_VOTE): settings/<key> and start, written by players to start a vote
 *      stream "chat" of the server: { from id, from name, text, time (ms), id }
 *      stream "commands" of every member: { verb, arguments }
 *      app messages of the server: REFUSED, LAUNCH, KICKED, CLOSED
 *  Every write to a grid of the lobby is checked by the server's CLobbyRules (a DRN write validator).
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CGucefLobbyProtocol
{
    public:

    static const CORE::CString ServiceName;             /**< "gucef": the DRN service name and the lobby service name */
    static const CORE::CString AuthenticatorType;       /**< "gucef-lobby" */
    static const CORE::CString RelayAuthenticatorType;  /**< "gucef-lobby-relay": of the relay for the game sessions */
    static const CORE::CString DirectorySessionName;    /**< "gucef.directory" */
    static const CORE::CString LobbySessionPrefix;      /**< "gucef.lobby." */
    static const CORE::CString GameSessionPrefix;       /**< "gucef.game." */

    static const CORE::CString ListingsGrid;            /**< "listings" */
    static const UInt32 MaxDataTreeDepth = 32;          /**< the nesting that ReadDataTree() accepts, the trees of the lobbies are a few levels deep */
    static const CORE::CString RequestsStream;          /**< "requests" */
    static const CORE::CString ChannelStreamPrefix;     /**< "chat/" */
    static const CORE::CString LobbyGrid;               /**< "lobby" */
    static const CORE::CString MembersGrid;             /**< "members" */
    static const CORE::CString SlotsGrid;               /**< "slots" */
    static const CORE::CString SchemaGrid;              /**< "schema" */
    static const CORE::CString ProposalsGrid;           /**< "proposals" */
    static const CORE::CString ChatStream;              /**< "chat" */
    static const CORE::CString CommandsStream;          /**< "commands" */

    /** The first byte of an app message of the server */
    enum EMessageType
    {
        MESSAGE_REPLY           = 1,    /**< { request id, ok, reason, arguments } */
        MESSAGE_DIRECT_MESSAGE  = 2,    /**< { from, text, time, id } */
        MESSAGE_REFUSED         = 3,    /**< { what, reason } */
        MESSAGE_LAUNCH          = 4,    /**< { launch description (a data tree) } */
        MESSAGE_KICKED          = 5,    /**< { reason } */
        MESSAGE_CLOSED          = 6     /**< { reason } */
    };

    /** Writes a data tree: name, type, value, the attributes and the children */
    static void WriteDataTree( DRN::CDRNWireWriter& writer, const CORE::CDataNode& node );

    /**
     *  Reads a data tree written by WriteDataTree(). Trees come from clients and from lobby owners, so the reader
     *  refuses a tree nested deeper than maxDepth levels instead of recursing without end
     */
    static bool ReadDataTree( DRN::CDRNWireReader& reader, CORE::CDataNode& node, UInt32 maxDepth = MaxDataTreeDepth );

    /** A data tree as a binary blob variant, for a grid value */
    static CORE::CVariant DataTreeToVariant( const CORE::CDataNode& node );

    static bool VariantToDataTree( const CORE::CVariant& value, CORE::CDataNode& node );

    static CORE::CString MakeLobbySessionName( const CORE::CString& lobbyId );

    static CORE::CString MakeGameSessionName( const CORE::CString& lobbyId );

    /** Splits "<member id>/<field>" */
    static bool SplitMemberKey( const CORE::CString& key, CORE::CString& memberId, CORE::CString& field );

    /** A UTC time in milliseconds since the Unix epoch */
    static UInt64 GetUnixTimeInMs( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H ? */
