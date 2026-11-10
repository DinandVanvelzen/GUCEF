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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_DRN_CDRNVARIANTCODEC_H
#include "gucefDRN_CDRNVariantCodec.h"
#define GUCEF_DRN_CDRNVARIANTCODEC_H
#endif /* GUCEF_DRN_CDRNVARIANTCODEC_H ? */

#include "gucefMULTIPLAY_CGucefLobbyProtocol.h"

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

const CORE::CString CGucefLobbyProtocol::ServiceName = "gucef";
const CORE::CString CGucefLobbyProtocol::AuthenticatorType = "gucef-lobby";
const CORE::CString CGucefLobbyProtocol::RelayAuthenticatorType = "gucef-lobby-relay";
const CORE::CString CGucefLobbyProtocol::DirectorySessionName = "gucef.directory";
const CORE::CString CGucefLobbyProtocol::LobbySessionPrefix = "gucef.lobby.";
const CORE::CString CGucefLobbyProtocol::GameSessionPrefix = "gucef.game.";

const CORE::CString CGucefLobbyProtocol::ListingsGrid = "listings";
const CORE::CString CGucefLobbyProtocol::RequestsStream = "requests";
const CORE::CString CGucefLobbyProtocol::ChannelStreamPrefix = "chat/";
const CORE::CString CGucefLobbyProtocol::LobbyGrid = "lobby";
const CORE::CString CGucefLobbyProtocol::MembersGrid = "members";
const CORE::CString CGucefLobbyProtocol::SlotsGrid = "slots";
const CORE::CString CGucefLobbyProtocol::SchemaGrid = "schema";
const CORE::CString CGucefLobbyProtocol::ProposalsGrid = "proposals";
const CORE::CString CGucefLobbyProtocol::ChatStream = "chat";
const CORE::CString CGucefLobbyProtocol::CommandsStream = "commands";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CGucefLobbyProtocol::WriteDataTree( DRN::CDRNWireWriter& writer, const CORE::CDataNode& node )
{GUCEF_TRACE;

    writer.WriteString( node.GetName() );
    writer.WriteVarInt32( node.GetNodeType() );
    DRN::CDRNVariantCodec::Write( writer, node.GetValue() );

    writer.WriteVarUInt32( node.GetAttCount() );
    CORE::CDataNode::TAttributeMap::const_iterator a = node.AttributeBegin();
    while ( a != node.AttributeEnd() )
    {
        writer.WriteString( (*a).first );
        DRN::CDRNVariantCodec::Write( writer, (*a).second );
        ++a;
    }

    // The direct children only
    UInt32 childCount = 0;
    CORE::CDataNode::const_iterator c = node.ConstBegin();
    while ( c != node.ConstEnd() )
    {
        ++childCount;
        ++c;
    }
    writer.WriteVarUInt32( childCount );
    c = node.ConstBegin();
    while ( c != node.ConstEnd() )
    {
        WriteDataTree( writer, *(*c) );
        ++c;
    }
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyProtocol::ReadDataTree( DRN::CDRNWireReader& reader, CORE::CDataNode& node, UInt32 maxDepth )
{GUCEF_TRACE;

    if ( 0 == maxDepth )
        return false;

    CORE::CString name;
    Int32 nodeType = 0;
    CORE::CVariant value;
    if ( !reader.ReadString( name ) || !reader.ReadVarInt32( nodeType ) || !DRN::CDRNVariantCodec::Read( reader, value ) )
        return false;
    node.SetName( name );
    node.SetNodeType( nodeType );
    node.SetValue( value );

    UInt32 attributeCount = 0;
    if ( !reader.ReadVarUInt32( attributeCount ) )
        return false;
    for ( UInt32 i=0; i<attributeCount; ++i )
    {
        CORE::CString attributeName;
        CORE::CVariant attributeValue;
        if ( !reader.ReadString( attributeName ) || !DRN::CDRNVariantCodec::Read( reader, attributeValue ) )
            return false;
        node.SetAttribute( attributeName, attributeValue );
    }

    UInt32 childCount = 0;
    if ( !reader.ReadVarUInt32( childCount ) )
        return false;
    for ( UInt32 i=0; i<childCount; ++i )
    {
        CORE::CDataNode* child = node.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL == child || !ReadDataTree( reader, *child, maxDepth - 1 ) )
            return false;
    }
    return !reader.HasFailed();
}

/*-------------------------------------------------------------------------*/

CORE::CVariant
CGucefLobbyProtocol::DataTreeToVariant( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    DRN::CDRNWireWriter writer( buffer );
    WriteDataTree( writer, node );
    CORE::CVariant value;
    value.Set( buffer.GetConstBufferPtr(), writer.GetOffset(), GUCEF_DATATYPE_BINARY_BLOB );
    return value;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyProtocol::VariantToDataTree( const CORE::CVariant& value, CORE::CDataNode& node )
{GUCEF_TRACE;

    if ( !value.IsBinary() || 0 == value.ByteSize( false ) )
        return false;
    DRN::CDRNWireReader reader( value.AsVoidPtr(), value.ByteSize( false ) );
    return ReadDataTree( reader, node );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGucefLobbyProtocol::MakeLobbySessionName( const CORE::CString& lobbyId )
{GUCEF_TRACE;

    return LobbySessionPrefix + lobbyId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGucefLobbyProtocol::MakeGameSessionName( const CORE::CString& lobbyId )
{GUCEF_TRACE;

    return GameSessionPrefix + lobbyId;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyProtocol::SplitMemberKey( const CORE::CString& key, CORE::CString& memberId, CORE::CString& field )
{GUCEF_TRACE;

    Int32 slash = key.HasChar( '/', true );
    if ( slash <= 0 )
        return false;
    memberId = key.SubstrToIndex( (UInt32) slash, true );
    field = key.CutChars( (UInt32) slash + 1, true );
    return !field.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

UInt64
CGucefLobbyProtocol::GetUnixTimeInMs( void )
{GUCEF_TRACE;

    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
