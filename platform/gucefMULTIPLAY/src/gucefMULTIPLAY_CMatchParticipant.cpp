/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHSIGNING_H
#include "gucefMULTIPLAY_CMatchSigning.h"
#define GUCEF_MULTIPLAY_CMATCHSIGNING_H
#endif /* GUCEF_MULTIPLAY_CMATCHSIGNING_H ? */

#include "gucefMULTIPLAY_CMatchParticipant.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchParticipant::CMatchParticipant( void )
    : CORE::CIDataNodeSerializable()
    , userId()
    , displayName()
    , slot( 0 )
    , publicKey()
    , status( MATCHPARTICIPANT_INVITED )
    , outcome( MATCHOUTCOME_NONE )
    , contact()
    , boss( false )
    , statusCode()
    , statusReason()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchParticipant::CMatchParticipant( const CMatchParticipant& src )
    : CORE::CIDataNodeSerializable( src )
    , userId( src.userId )
    , displayName( src.displayName )
    , slot( src.slot )
    , publicKey( src.publicKey )
    , status( src.status )
    , outcome( src.outcome )
    , contact( src.contact )
    , boss( src.boss )
    , statusCode( src.statusCode )
    , statusReason( src.statusReason )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchParticipant::~CMatchParticipant()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchParticipant&
CMatchParticipant::operator=( const CMatchParticipant& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        userId = src.userId;
        displayName = src.displayName;
        slot = src.slot;
        publicKey = src.publicKey;
        status = src.status;
        outcome = src.outcome;
        contact = src.contact;
        boss = src.boss;
        statusCode = src.statusCode;
        statusReason = src.statusReason;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMatchParticipant::operator==( const CMatchParticipant& other ) const
{GUCEF_TRACE;

    return userId == other.userId && displayName == other.displayName && slot == other.slot && publicKey == other.publicKey &&
           status == other.status && outcome == other.outcome && contact == other.contact && boss == other.boss &&
           statusCode == other.statusCode && statusReason == other.statusReason;
}

/*-------------------------------------------------------------------------*/

bool
CMatchParticipant::IsEligibleVoter( void ) const
{GUCEF_TRACE;

    return MATCHPARTICIPANT_ACTIVE == status;
}

/*-------------------------------------------------------------------------*/

bool
CMatchParticipant::IsSpectator( void ) const
{GUCEF_TRACE;

    return MATCHPARTICIPANT_SPECTATOR == status;
}

/*-------------------------------------------------------------------------*/

bool
CMatchParticipant::IsIncompatible( void ) const
{GUCEF_TRACE;

    return MATCHPARTICIPANT_SPECTATOR == status && !statusCode.IsNULLOrEmpty() && statusCode != GUCEF_MULTIPLAY_SPECTATOR_REASON_VOLUNTARY;
}

/*-------------------------------------------------------------------------*/

void
CMatchParticipant::WriteSigningBytes( CMatchBinaryWriter& writer ) const
{GUCEF_TRACE;

    writer.WriteString( userId );
    writer.WriteString( displayName );
    writer.WriteUInt32( slot );
    writer.WriteString( publicKey );
    writer.WriteString( contact );
    writer.WriteBool( boss );
}

/*-------------------------------------------------------------------------*/

bool
CMatchParticipant::IsValid( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !CMatchSigning::IsHexOfLength( publicKey, CMatchSigning::PublicKeyHexSize ) )
    {
        reason = "A participant has no valid public key";
        return false;
    }
    if ( userId.IsNULLOrEmpty() || userId != CPlayerIdentity::MakeUserIdFromHex( publicKey ) )
    {
        reason = "The user id of a participant is not the one of its public key";
        return false;
    }
    if ( displayName.Length() > MaxDisplayNameLength || contact.Length() > MaxContactLength )
    {
        reason = "The name or contact of a participant is too long";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchParticipant::Serialize( CORE::CDataNode& domRootNode                        ,
                              const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "userId", userId );
    domRootNode.SetAttribute( "displayName", displayName );
    domRootNode.SetAttribute( "slot", slot );
    domRootNode.SetAttribute( "publicKey", publicKey );
    domRootNode.SetAttribute( "status", CORE::CString( MatchParticipantStatusToString( status ) ) );
    domRootNode.SetAttribute( "outcome", CORE::CString( MatchOutcomeToString( outcome ) ) );
    domRootNode.SetAttribute( "contact", contact );
    domRootNode.SetAttribute( "boss", boss );
    domRootNode.SetAttribute( "statusCode", statusCode );
    domRootNode.SetAttribute( "statusReason", statusReason );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchParticipant::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    userId = domRootNode.GetAttributeValueOrChildValueByName( "userId" ).AsString( CORE::CString::Empty, true );
    displayName = domRootNode.GetAttributeValueOrChildValueByName( "displayName" ).AsString( CORE::CString::Empty, true );
    slot = domRootNode.GetAttributeValueOrChildValueByName( "slot" ).AsUInt32( 0, true );
    publicKey = domRootNode.GetAttributeValueOrChildValueByName( "publicKey" ).AsString( CORE::CString::Empty, true );
    status = StringToMatchParticipantStatus( domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString( CORE::CString::Empty, true ) );
    outcome = StringToMatchOutcome( domRootNode.GetAttributeValueOrChildValueByName( "outcome" ).AsString( CORE::CString::Empty, true ) );
    contact = domRootNode.GetAttributeValueOrChildValueByName( "contact" ).AsString( CORE::CString::Empty, true );
    boss = domRootNode.GetAttributeValueOrChildValueByName( "boss" ).AsBool( false, true );
    statusCode = domRootNode.GetAttributeValueOrChildValueByName( "statusCode" ).AsString( CORE::CString::Empty, true );
    statusReason = domRootNode.GetAttributeValueOrChildValueByName( "statusReason" ).AsString( CORE::CString::Empty, true );
    if ( statusCode.Length() > MaxStatusCodeLength || statusReason.Length() > MaxStatusReasonLength )
    {
        statusCode.Clear();
        statusReason.Clear();
    }
    return !userId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchParticipant::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchParticipant( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchParticipant::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchParticipant";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchParticipant::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchParticipant::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchParticipant";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
