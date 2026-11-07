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

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHSIGNING_H
#include "gucefMULTIPLAY_CMatchSigning.h"
#define GUCEF_MULTIPLAY_CMATCHSIGNING_H
#endif /* GUCEF_MULTIPLAY_CMATCHSIGNING_H ? */

#include "gucefMULTIPLAY_CMatchInfo.h"

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

CMatchInfo::CMatchInfo( void )
    : CORE::CIDataNodeSerializable()
    , matchId()
    , title()
    , gameId()
    , gameVersion()
    , rulesetId()
    , contentHash()
    , backend()
    , mode( MATCHMODE_ASYNC )
    , status( MATCHSTATUS_FORMING )
    , decisionPolicy( MATCHDECISIONPOLICY_VOTE )
    , creatorUserId()
    , closedByUserId()
    , createdAt( 0 )
    , participants()
    , settings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchInfo::CMatchInfo( const CMatchInfo& src )
    : CORE::CIDataNodeSerializable( src )
    , matchId( src.matchId )
    , title( src.title )
    , gameId( src.gameId )
    , gameVersion( src.gameVersion )
    , rulesetId( src.rulesetId )
    , contentHash( src.contentHash )
    , backend( src.backend )
    , mode( src.mode )
    , status( src.status )
    , decisionPolicy( src.decisionPolicy )
    , creatorUserId( src.creatorUserId )
    , closedByUserId( src.closedByUserId )
    , createdAt( src.createdAt )
    , participants( src.participants )
    , settings( src.settings )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchInfo::~CMatchInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchInfo&
CMatchInfo::operator=( const CMatchInfo& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        title = src.title;
        gameId = src.gameId;
        gameVersion = src.gameVersion;
        rulesetId = src.rulesetId;
        contentHash = src.contentHash;
        backend = src.backend;
        mode = src.mode;
        status = src.status;
        decisionPolicy = src.decisionPolicy;
        creatorUserId = src.creatorUserId;
        closedByUserId = src.closedByUserId;
        createdAt = src.createdAt;
        participants = src.participants;
        settings = src.settings;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CMatchParticipant*
CMatchInfo::FindParticipant( const CORE::CString& userId )
{GUCEF_TRACE;

    for ( size_t i=0; i<participants.size(); ++i )
        if ( participants[ i ].userId == userId )
            return &participants[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CMatchParticipant*
CMatchInfo::FindParticipant( const CORE::CString& userId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<participants.size(); ++i )
        if ( participants[ i ].userId == userId )
            return &participants[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CMatchParticipant*
CMatchInfo::FindParticipantByKey( const CORE::CString& publicKeyHex ) const
{GUCEF_TRACE;

    CORE::CString lower = publicKeyHex.Lowercase();
    for ( size_t i=0; i<participants.size(); ++i )
        if ( participants[ i ].publicKey.Lowercase() == lower )
            return &participants[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchInfo::GetEligibleVoterCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<participants.size(); ++i )
        if ( participants[ i ].IsEligibleVoter() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchInfo::GetSetting( const CORE::CString& key, const CORE::CString& defaultValue ) const
{GUCEF_TRACE;

    TSettingsMap::const_iterator i = settings.find( key );
    return i == settings.end() ? defaultValue : (*i).second;
}

/*-------------------------------------------------------------------------*/

void
CMatchInfo::SetSetting( const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    settings[ key ] = value;
}

/*-------------------------------------------------------------------------*/

void
CMatchInfo::WriteSigningBytes( CMatchBinaryWriter& writer ) const
{GUCEF_TRACE;

    writer.WriteUInt32( 1 );    // layout version
    writer.WriteString( matchId );
    writer.WriteString( title );
    writer.WriteString( gameId );
    writer.WriteString( gameVersion );
    writer.WriteString( rulesetId );
    writer.WriteString( contentHash );
    writer.WriteString( backend );
    writer.WriteUInt8( (UInt8) mode );
    writer.WriteUInt8( (UInt8) decisionPolicy );
    writer.WriteString( creatorUserId );
    writer.WriteInt64( createdAt );
    writer.WriteUInt32( (UInt32) participants.size() );
    for ( size_t i=0; i<participants.size(); ++i )
        participants[ i ].WriteSigningBytes( writer );
    writer.WriteUInt32( (UInt32) settings.size() );
    TSettingsMap::const_iterator s = settings.begin();
    while ( s != settings.end() )
    {
        writer.WriteString( (*s).first );
        writer.WriteString( (*s).second );
        ++s;
    }
}

/*-------------------------------------------------------------------------*/

void
CMatchInfo::GetSigningBytes( CORE::CDynamicBuffer& bytes ) const
{GUCEF_TRACE;

    bytes.Clear();
    CMatchBinaryWriter writer( bytes );
    WriteSigningBytes( writer );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchInfo::GetDigest( void ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    GetSigningBytes( bytes );
    return CMatchSigning::Sha256Hex( bytes.GetConstBufferPtr(), bytes.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CMatchInfo::IsValid( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !CMatchId::IsValid( matchId ) )
    {
        reason = "The match id is not valid";
        return false;
    }
    if ( title.Length() > MaxTextLength || gameId.Length() > MaxTextLength || gameVersion.Length() > MaxTextLength ||
         rulesetId.Length() > MaxTextLength || backend.Length() > MaxTextLength || contentHash.Length() > MaxContentHashLength )
    {
        reason = "A text of the match is too long";
        return false;
    }
    if ( participants.empty() || participants.size() > MaxParticipants )
    {
        reason = "A match has one to sixteen participants";
        return false;
    }
    if ( settings.size() > MaxSettings )
    {
        reason = "The match has too many settings";
        return false;
    }
    TSettingsMap::const_iterator s = settings.begin();
    while ( s != settings.end() )
    {
        if ( (*s).first.IsNULLOrEmpty() || (*s).first.Length() > MaxSettingKeyLength || (*s).second.Length() > MaxSettingValueLength )
        {
            reason = "A setting of the match is not valid";
            return false;
        }
        ++s;
    }

    bool creatorFound = false;
    for ( size_t i=0; i<participants.size(); ++i )
    {
        if ( !participants[ i ].IsValid( reason ) )
            return false;
        for ( size_t j=0; j<i; ++j )
        {
            if ( participants[ j ].userId == participants[ i ].userId || participants[ j ].slot == participants[ i ].slot )
            {
                reason = "Two participants have the same user id or slot";
                return false;
            }
        }
        if ( participants[ i ].userId == creatorUserId )
            creatorFound = true;
    }
    if ( !creatorFound )
    {
        reason = "The creator is not one of the participants";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchInfo::Serialize( CORE::CDataNode& domRootNode                        ,
                       const CORE::CDataNodeSerializableSettings& serializerSettings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "matchId", matchId );
    domRootNode.SetAttribute( "title", title );
    domRootNode.SetAttribute( "gameId", gameId );
    domRootNode.SetAttribute( "gameVersion", gameVersion );
    domRootNode.SetAttribute( "rulesetId", rulesetId );
    domRootNode.SetAttribute( "contentHash", contentHash );
    domRootNode.SetAttribute( "backend", backend );
    domRootNode.SetAttribute( "mode", CORE::CString( MatchModeToString( mode ) ) );
    domRootNode.SetAttribute( "status", CORE::CString( MatchStatusToString( status ) ) );
    domRootNode.SetAttribute( "decisionPolicy", CORE::CString( MatchDecisionPolicyToString( decisionPolicy ) ) );
    domRootNode.SetAttribute( "creatorUserId", creatorUserId );
    domRootNode.SetAttribute( "closedByUserId", closedByUserId );
    domRootNode.SetAttribute( "createdAt", createdAt );

    CORE::CDataNode* participantsNode = domRootNode.AddChild( "participants", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<participants.size(); ++i )
        participants[ i ].Serialize( *participantsNode->AddChild( "participant", GUCEF_DATATYPE_OBJECT ), serializerSettings );

    CORE::CDataNode* settingsNode = domRootNode.AddChild( "settings", GUCEF_DATATYPE_OBJECT );
    TSettingsMap::const_iterator s = settings.begin();
    while ( s != settings.end() )
    {
        settingsNode->SetAttribute( (*s).first, (*s).second );
        ++s;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchInfo::Deserialize( const CORE::CDataNode& domRootNode                  ,
                         const CORE::CDataNodeSerializableSettings& serializerSettings )
{GUCEF_TRACE;

    matchId = domRootNode.GetAttributeValueOrChildValueByName( "matchId" ).AsString( CORE::CString::Empty, true );
    title = domRootNode.GetAttributeValueOrChildValueByName( "title" ).AsString( CORE::CString::Empty, true );
    gameId = domRootNode.GetAttributeValueOrChildValueByName( "gameId" ).AsString( CORE::CString::Empty, true );
    gameVersion = domRootNode.GetAttributeValueOrChildValueByName( "gameVersion" ).AsString( CORE::CString::Empty, true );
    rulesetId = domRootNode.GetAttributeValueOrChildValueByName( "rulesetId" ).AsString( CORE::CString::Empty, true );
    contentHash = domRootNode.GetAttributeValueOrChildValueByName( "contentHash" ).AsString( CORE::CString::Empty, true );
    backend = domRootNode.GetAttributeValueOrChildValueByName( "backend" ).AsString( CORE::CString::Empty, true );
    mode = StringToMatchMode( domRootNode.GetAttributeValueOrChildValueByName( "mode" ).AsString( CORE::CString::Empty, true ) );
    status = StringToMatchStatus( domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString( CORE::CString::Empty, true ) );
    decisionPolicy = StringToMatchDecisionPolicy( domRootNode.GetAttributeValueOrChildValueByName( "decisionPolicy" ).AsString( CORE::CString::Empty, true ) );
    creatorUserId = domRootNode.GetAttributeValueOrChildValueByName( "creatorUserId" ).AsString( CORE::CString::Empty, true );
    closedByUserId = domRootNode.GetAttributeValueOrChildValueByName( "closedByUserId" ).AsString( CORE::CString::Empty, true );
    createdAt = domRootNode.GetAttributeValueOrChildValueByName( "createdAt" ).AsInt64( 0, true );

    // stored data is not trusted either: a bounded number of participants and settings is read
    participants.clear();
    const CORE::CDataNode* participantsNode = domRootNode.FindChild( "participants" );
    if ( GUCEF_NULL != participantsNode )
    {
        CORE::CDataNode::const_iterator i = participantsNode->ConstBegin();
        while ( i != participantsNode->ConstEnd() && participants.size() <= MaxParticipants )
        {
            CMatchParticipant participant;
            if ( participant.Deserialize( *(*i), serializerSettings ) )
                participants.push_back( participant );
            ++i;
        }
    }

    settings.clear();
    const CORE::CDataNode* settingsNode = domRootNode.FindChild( "settings" );
    if ( GUCEF_NULL != settingsNode )
    {
        CORE::CDataNode::TAttributeMap::const_iterator a = settingsNode->AttributeBegin();
        while ( a != settingsNode->AttributeEnd() && settings.size() <= MaxSettings )
        {
            settings[ (*a).first ] = (*a).second.AsString( CORE::CString::Empty, true );
            ++a;
        }
    }
    return CMatchId::IsValid( matchId );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchInfo::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchInfo( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchInfo::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchInfo";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchInfo::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchInfo::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchInfo";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
