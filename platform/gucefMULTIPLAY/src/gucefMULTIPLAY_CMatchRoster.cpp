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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHSIGNING_H
#include "gucefMULTIPLAY_CMatchSigning.h"
#define GUCEF_MULTIPLAY_CMATCHSIGNING_H
#endif /* GUCEF_MULTIPLAY_CMATCHSIGNING_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#include "gucefMULTIPLAY_CMatchRoster.h"

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

CMatchRoster::CMatchRoster( void )
    : CORE::CIDataNodeSerializable()
    , matchId()
    , revision( 0 )
    , hostUserId()
    , hostPublicKey()
    , decisionPolicy( MATCHDECISIONPOLICY_VOTE )
    , settingsDigest()
    , createdAt( 0 )
    , entries()
    , appliedDigests()
    , evidence()
    , signature()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchRoster::CMatchRoster( const CMatchRoster& src )
    : CORE::CIDataNodeSerializable( src )
    , matchId( src.matchId )
    , revision( src.revision )
    , hostUserId( src.hostUserId )
    , hostPublicKey( src.hostPublicKey )
    , decisionPolicy( src.decisionPolicy )
    , settingsDigest( src.settingsDigest )
    , createdAt( src.createdAt )
    , entries( src.entries )
    , appliedDigests( src.appliedDigests )
    , evidence( src.evidence )
    , signature( src.signature )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchRoster::~CMatchRoster()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchRoster&
CMatchRoster::operator=( const CMatchRoster& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        revision = src.revision;
        hostUserId = src.hostUserId;
        hostPublicKey = src.hostPublicKey;
        decisionPolicy = src.decisionPolicy;
        settingsDigest = src.settingsDigest;
        createdAt = src.createdAt;
        entries = src.entries;
        appliedDigests = src.appliedDigests;
        evidence = src.evidence;
        signature = src.signature;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CMatchRoster::WriteSigningBytes( CMatchBinaryWriter& writer ) const
{GUCEF_TRACE;

    writer.WriteString( matchId );
    writer.WriteUInt32( revision );
    writer.WriteString( hostUserId );
    writer.WriteString( hostPublicKey.Lowercase() );
    writer.WriteUInt8( (UInt8) decisionPolicy );
    writer.WriteString( settingsDigest );
    writer.WriteInt64( createdAt );
    writer.WriteUInt8( (UInt8) ( entries.size() > 255 ? 255 : entries.size() ) );
    for ( size_t i=0; i<entries.size() && i<255; ++i )
        entries[ i ].WriteSigningBytes( writer );
    writer.WriteUInt8( (UInt8) ( appliedDigests.size() > 255 ? 255 : appliedDigests.size() ) );
    for ( size_t i=0; i<appliedDigests.size() && i<255; ++i )
        writer.WriteString( appliedDigests[ i ] );

    // the evidence is what the host counted: the proposal and the signatures of the votes, which are signed by their voters themselves
    writer.WriteUInt8( (UInt8) ( evidence.size() > 255 ? 255 : evidence.size() ) );
    for ( size_t i=0; i<evidence.size() && i<255; ++i )
    {
        writer.WriteString( evidence[ i ].proposal.GetDigest() );
        writer.WriteUInt8( (UInt8) ( evidence[ i ].votes.size() > 255 ? 255 : evidence[ i ].votes.size() ) );
        for ( size_t v=0; v<evidence[ i ].votes.size() && v<255; ++v )
            writer.WriteString( evidence[ i ].votes[ v ].signature );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMatchRoster::Sign( const CPlayerIdentity& host )
{GUCEF_TRACE;

    signature.Clear();
    if ( !host.HasKeyPair() )
        return false;
    hostUserId = host.userId;
    hostPublicKey = host.publicKey;
    CORE::CDynamicBuffer bytes;
    CMatchBinaryWriter writer( bytes );
    WriteSigningBytes( writer );
    return CMatchSigning::Sign( host, CMatchSigning::KindRoster, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CMatchRoster::IsWellFormed( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !CMatchId::IsValid( matchId ) || 0 == revision )
    {
        reason = "The roster has no valid match id or revision";
        return false;
    }
    if ( entries.empty() || entries.size() > MaxEntries || evidence.size() > MaxEvidence || appliedDigests.size() > MaxAppliedDigests )
    {
        reason = "The roster has too many or too few entries, decisions or evidence";
        return false;
    }
    if ( !CMatchSigning::IsHexOfLength( hostPublicKey, CMatchSigning::PublicKeyHexSize ) || hostUserId != CPlayerIdentity::MakeUserIdFromHex( hostPublicKey ) )
    {
        reason = "The host of the roster has no valid key";
        return false;
    }
    if ( !settingsDigest.IsNULLOrEmpty() && !CMatchSigning::IsHexOfLength( settingsDigest, CMatchSigning::Sha256HexSize ) )
    {
        reason = "The digest of the settings is not valid";
        return false;
    }
    CORE::CString::StringSet userIds, contacts, keys;
    for ( size_t i=0; i<entries.size(); ++i )
    {
        const CMatchRosterEntry& entry = entries[ i ];
        if ( entry.displayName.Length() > MaxNameBytes || entry.contact.IsNULLOrEmpty() || entry.contact.Length() > CMatchParticipant::MaxContactLength ||
             entry.statusCode.Length() > CMatchParticipant::MaxStatusCodeLength )
        {
            reason = "A participant of the roster has a name, contact or status code that is not valid";
            return false;
        }
        const bool keyed = MATCHPARTICIPANT_ACTIVE == entry.status || MATCHPARTICIPANT_SPECTATOR == entry.status;
        if ( !keyed && MATCHPARTICIPANT_INVITED != entry.status && MATCHPARTICIPANT_DECLINED != entry.status )
        {
            reason = "A participant of the roster has a status a roster does not hold";
            return false;
        }
        if ( keyed )
        {
            if ( !CMatchSigning::IsHexOfLength( entry.publicKey, CMatchSigning::PublicKeyHexSize ) || entry.userId != CPlayerIdentity::MakeUserIdFromHex( entry.publicKey ) )
            {
                reason = "A participant of the roster has a key that is not the key of its user id";
                return false;
            }
            if ( !keys.insert( entry.publicKey.Lowercase() ).second )
            {
                reason = "A key is listed twice";
                return false;
            }
        }
        else
        if ( entry.HasKey() || !CMatchRosterEntry::IsPendingUserId( entry.userId ) )
        {
            reason = "A participant who has not introduced itself has a key";
            return false;
        }
        if ( !userIds.insert( entry.userId ).second || !contacts.insert( entry.contact.Lowercase() ).second )
        {
            reason = "A participant or a contact is listed twice";
            return false;
        }
    }
    if ( entries[ 0 ].userId != hostUserId || entries[ 0 ].publicKey.Lowercase() != hostPublicKey.Lowercase() || MATCHPARTICIPANT_ACTIVE != entries[ 0 ].status )
    {
        reason = "The host is not the first participant of the roster";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchRoster::VerifySignature( void ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    CMatchBinaryWriter writer( bytes );
    WriteSigningBytes( writer );
    return CMatchSigning::Verify( hostPublicKey, CMatchSigning::KindRoster, matchId, bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CMatchRoster::Verify( const CORE::CString& expectedHostKey, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !IsWellFormed( reason ) )
        return false;
    if ( !expectedHostKey.IsNULLOrEmpty() && expectedHostKey.Lowercase() != hostPublicKey.Lowercase() )
    {
        reason = "The roster is not signed by the host of the game";
        return false;
    }
    if ( !VerifySignature() )
    {
        reason = "The roster is not signed by its host";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchRoster::GetDigest( void ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    CMatchBinaryWriter writer( bytes );
    WriteSigningBytes( writer );
    writer.WriteString( signature );
    return CMatchSigning::Sha256Hex( bytes.GetConstBufferPtr(), bytes.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchRoster::GetContentDigest( void ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    CMatchBinaryWriter writer( bytes );
    writer.WriteString( matchId );
    writer.WriteString( hostUserId );
    writer.WriteUInt8( (UInt8) decisionPolicy );
    writer.WriteString( settingsDigest );
    writer.WriteUInt32( (UInt32) entries.size() );
    for ( size_t i=0; i<entries.size(); ++i )
        entries[ i ].WriteSigningBytes( writer );
    writer.WriteUInt32( (UInt32) appliedDigests.size() );
    for ( size_t i=0; i<appliedDigests.size(); ++i )
        writer.WriteString( appliedDigests[ i ] );
    return CMatchSigning::Sha256Hex( bytes.GetConstBufferPtr(), bytes.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

const CMatchRosterEntry*
CMatchRoster::FindEntry( const CORE::CString& userId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<entries.size(); ++i )
        if ( entries[ i ].userId == userId )
            return &entries[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CMatchRosterEntry*
CMatchRoster::FindEntryByContact( const CORE::CString& contact ) const
{GUCEF_TRACE;

    const CORE::CString lower = contact.Lowercase();
    for ( size_t i=0; i<entries.size(); ++i )
        if ( entries[ i ].contact.Lowercase() == lower )
            return &entries[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchRoster::CountVoters( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<entries.size(); ++i )
        if ( MATCHPARTICIPANT_ACTIVE == entries[ i ].status )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
CMatchRoster::ToMatchInfo( CMatchInfo& info ) const
{GUCEF_TRACE;

    info = CMatchInfo();
    info.matchId = matchId;
    info.status = MATCHSTATUS_INITIALIZING;
    info.mode = MATCHMODE_ASYNC;
    info.decisionPolicy = decisionPolicy;
    info.creatorUserId = hostUserId;
    info.createdAt = createdAt;
    for ( size_t i=0; i<entries.size(); ++i )
    {
        CMatchParticipant participant;
        participant.userId = entries[ i ].userId;
        participant.displayName = entries[ i ].displayName;
        participant.slot = (UInt32) i;
        participant.publicKey = entries[ i ].publicKey;
        participant.status = entries[ i ].status;
        participant.contact = entries[ i ].contact;
        participant.boss = entries[ i ].boss;
        participant.statusCode = entries[ i ].statusCode;
        info.participants.push_back( participant );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchRoster::MakeSettingsDigest( const CMatchInfo::TSettingsMap& settings )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    CMatchBinaryWriter writer( bytes );
    writer.WriteUInt32( (UInt32) settings.size() );
    for ( CMatchInfo::TSettingsMap::const_iterator s = settings.begin(); s != settings.end(); ++s )
    {
        writer.WriteString( (*s).first );
        writer.WriteString( (*s).second );
    }
    return CMatchSigning::Sha256Hex( bytes.GetConstBufferPtr(), bytes.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CMatchRoster::Serialize( CORE::CDataNode& domRootNode                        ,
                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "matchId", matchId );
    domRootNode.SetAttribute( "revision", revision );
    domRootNode.SetAttribute( "hostUserId", hostUserId );
    domRootNode.SetAttribute( "hostPublicKey", hostPublicKey );
    domRootNode.SetAttribute( "decisionPolicy", CORE::CString( MatchDecisionPolicyToString( decisionPolicy ) ) );
    domRootNode.SetAttribute( "settingsDigest", settingsDigest );
    domRootNode.SetAttribute( "createdAt", createdAt );
    domRootNode.SetAttribute( "signature", signature );
    CORE::CDataNode* list = domRootNode.AddChild( "entries", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<entries.size(); ++i )
        entries[ i ].Serialize( *list->AddChild( "entry", GUCEF_DATATYPE_OBJECT ) );
    list = domRootNode.AddChild( "applied", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<appliedDigests.size(); ++i )
        list->AddChild( "digest", GUCEF_DATATYPE_OBJECT )->SetAttribute( "value", appliedDigests[ i ] );
    list = domRootNode.AddChild( "evidence", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<evidence.size(); ++i )
        evidence[ i ].Serialize( *list->AddChild( "item", GUCEF_DATATYPE_OBJECT ), settings );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchRoster::Deserialize( const CORE::CDataNode& domRootNode                  ,
                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CMatchRoster();
    matchId = domRootNode.GetAttributeValueOrChildValueByName( "matchId" ).AsString( CORE::CString::Empty, true );
    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsUInt32( 0, true );
    hostUserId = domRootNode.GetAttributeValueOrChildValueByName( "hostUserId" ).AsString( CORE::CString::Empty, true );
    hostPublicKey = domRootNode.GetAttributeValueOrChildValueByName( "hostPublicKey" ).AsString( CORE::CString::Empty, true );
    decisionPolicy = StringToMatchDecisionPolicy( domRootNode.GetAttributeValueOrChildValueByName( "decisionPolicy" ).AsString( CORE::CString::Empty, true ) );
    settingsDigest = domRootNode.GetAttributeValueOrChildValueByName( "settingsDigest" ).AsString( CORE::CString::Empty, true );
    createdAt = domRootNode.GetAttributeValueOrChildValueByName( "createdAt" ).AsInt64( 0, true );
    signature = domRootNode.GetAttributeValueOrChildValueByName( "signature" ).AsString( CORE::CString::Empty, true );

    // received and stored data is not trusted: at most one more than allowed is read, so that the checks of the roster refuse it
    const CORE::CDataNode* list = domRootNode.FindChild( "entries" );
    if ( GUCEF_NULL != list )
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd() && entries.size() <= MaxEntries; ++i )
        {
            CMatchRosterEntry entry;
            if ( !entry.Deserialize( *(*i) ) )
                return false;
            entries.push_back( entry );
        }
    list = domRootNode.FindChild( "applied" );
    if ( GUCEF_NULL != list )
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd() && appliedDigests.size() <= MaxAppliedDigests; ++i )
            appliedDigests.push_back( (*i)->GetAttributeValueOrChildValueByName( "value" ).AsString( CORE::CString::Empty, true ) );
    list = domRootNode.FindChild( "evidence" );
    if ( GUCEF_NULL != list )
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd() && evidence.size() <= MaxEvidence; ++i )
        {
            CMatchDecisionEvidence item;
            if ( !item.Deserialize( *(*i), settings ) )
                return false;
            evidence.push_back( item );
        }
    return CMatchId::IsValid( matchId );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchRoster::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchRoster( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchRoster::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchRoster";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchRoster::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchRoster::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchRoster";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
