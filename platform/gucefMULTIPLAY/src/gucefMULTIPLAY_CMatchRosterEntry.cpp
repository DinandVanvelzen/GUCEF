/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHSIGNING_H
#include "gucefMULTIPLAY_CMatchSigning.h"
#define GUCEF_MULTIPLAY_CMATCHSIGNING_H
#endif /* GUCEF_MULTIPLAY_CMATCHSIGNING_H ? */

#include "gucefMULTIPLAY_CMatchRosterEntry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* PendingPrefix = "pending-";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchRosterEntry::CMatchRosterEntry( void )
    : userId()
    , displayName()
    , contact()
    , publicKey()
    , status( MATCHPARTICIPANT_INVITED )
    , boss( false )
    , statusCode()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMatchRosterEntry::operator==( const CMatchRosterEntry& other ) const
{GUCEF_TRACE;

    return userId == other.userId && displayName == other.displayName && contact == other.contact && publicKey == other.publicKey &&
           status == other.status && boss == other.boss && statusCode == other.statusCode;
}

/*-------------------------------------------------------------------------*/

bool
CMatchRosterEntry::HasKey( void ) const
{GUCEF_TRACE;

    return !publicKey.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchRosterEntry::MakePendingUserId( const CORE::CString& matchId, const CORE::CString& contact )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    CMatchBinaryWriter writer( bytes );
    writer.WriteString( matchId );
    writer.WriteString( contact.Lowercase() );
    const CORE::CString hash = CMatchSigning::Sha256Hex( bytes.GetConstBufferPtr(), bytes.GetDataSize() );
    return CORE::CString( PendingPrefix ) + hash.SubstrToIndex( 24, true );
}

/*-------------------------------------------------------------------------*/

bool
CMatchRosterEntry::IsPendingUserId( const CORE::CString& userId )
{GUCEF_TRACE;

    return userId.Length() == 8 + 24 && userId.StartsWith( PendingPrefix );
}

/*-------------------------------------------------------------------------*/

void
CMatchRosterEntry::WriteSigningBytes( CMatchBinaryWriter& writer ) const
{GUCEF_TRACE;

    writer.WriteString( userId );
    writer.WriteString( displayName );
    writer.WriteString( contact );
    writer.WriteString( publicKey.Lowercase() );
    writer.WriteUInt8( (UInt8) status );
    writer.WriteBool( boss );
    writer.WriteString( statusCode );
}

/*-------------------------------------------------------------------------*/

void
CMatchRosterEntry::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "userId", userId );
    node.SetAttribute( "displayName", displayName );
    node.SetAttribute( "contact", contact );
    node.SetAttribute( "publicKey", publicKey );
    node.SetAttribute( "status", CORE::CString( MatchParticipantStatusToString( status ) ) );
    node.SetAttribute( "boss", boss );
    node.SetAttribute( "statusCode", statusCode );
}

/*-------------------------------------------------------------------------*/

bool
CMatchRosterEntry::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    userId = node.GetAttributeValueOrChildValueByName( "userId" ).AsString( CORE::CString::Empty, true );
    displayName = node.GetAttributeValueOrChildValueByName( "displayName" ).AsString( CORE::CString::Empty, true );
    contact = node.GetAttributeValueOrChildValueByName( "contact" ).AsString( CORE::CString::Empty, true );
    publicKey = node.GetAttributeValueOrChildValueByName( "publicKey" ).AsString( CORE::CString::Empty, true );
    status = StringToMatchParticipantStatus( node.GetAttributeValueOrChildValueByName( "status" ).AsString( CORE::CString::Empty, true ) );
    boss = node.GetAttributeValueOrChildValueByName( "boss" ).AsBool( false, true );
    statusCode = node.GetAttributeValueOrChildValueByName( "statusCode" ).AsString( CORE::CString::Empty, true );
    return !userId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
