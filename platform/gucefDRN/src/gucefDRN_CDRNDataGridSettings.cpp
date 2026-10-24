/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CDRNDataGridSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNDataGridSettings::ClassTypeName = "GUCEF::DRN::CDRNDataGridSettings";

/*-------------------------------------------------------------------------*/

CDRNDataGridSettings::CDRNDataGridSettings( void )
    : CORE::CIConfigurable()
    , name()
    , authority( DRN_AUTHORITY_HOST )
    , reliable( false )
    , reliableKeys()
    , sendRateHz( 20 )
    , maxBytesPerTick( 0 )
    , deltaAgainstAck( true )
    , capacity( 64 )
    , defaultQuantisation()
    , quantisation()
    , notify( DRN_NOTIFY_PER_CHANGE )
    , priority( 1.0f )
    , snapshotOnSubscribe( true )
    , votePolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataGridSettings::CDRNDataGridSettings( const CDRNDataGridSettings& src )
    : CORE::CIConfigurable( src )
    , name( src.name )
    , authority( src.authority )
    , reliable( src.reliable )
    , reliableKeys( src.reliableKeys )
    , sendRateHz( src.sendRateHz )
    , maxBytesPerTick( src.maxBytesPerTick )
    , deltaAgainstAck( src.deltaAgainstAck )
    , capacity( src.capacity )
    , defaultQuantisation( src.defaultQuantisation )
    , quantisation( src.quantisation )
    , notify( src.notify )
    , priority( src.priority )
    , snapshotOnSubscribe( src.snapshotOnSubscribe )
    , votePolicy( src.votePolicy )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataGridSettings::~CDRNDataGridSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataGridSettings&
CDRNDataGridSettings::operator=( const CDRNDataGridSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        name = src.name;
        authority = src.authority;
        reliable = src.reliable;
        reliableKeys = src.reliableKeys;
        sendRateHz = src.sendRateHz;
        maxBytesPerTick = src.maxBytesPerTick;
        deltaAgainstAck = src.deltaAgainstAck;
        capacity = src.capacity;
        defaultQuantisation = src.defaultQuantisation;
        quantisation = src.quantisation;
        notify = src.notify;
        priority = src.priority;
        snapshotOnSubscribe = src.snapshotOnSubscribe;
        votePolicy = src.votePolicy;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const CDRNQuantisation&
CDRNDataGridSettings::GetQuantisation( const CORE::CVariant& key ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<quantisation.size(); ++i )
        if ( CDRNVariantCodec::IsSame( quantisation[ i ].key, key ) )
            return quantisation[ i ].quantisation;
    return defaultQuantisation;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGridSettings::SetQuantisation( const CORE::CVariant& key, const CDRNQuantisation& keyQuantisation )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<quantisation.size(); ++i )
    {
        if ( CDRNVariantCodec::IsSame( quantisation[ i ].key, key ) )
        {
            quantisation[ i ].quantisation = keyQuantisation;
            return;
        }
    }
    CKeyQuantisation entry;
    entry.key = key;
    entry.quantisation = keyQuantisation;
    quantisation.push_back( entry );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridSettings::IsReliableKey( const CORE::CVariant& key ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<reliableKeys.size(); ++i )
        if ( CDRNVariantCodec::IsSame( reliableKeys[ i ], key ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNDataGridSettings::AuthorityToString( TDRNAuthority value )
{GUCEF_TRACE;

    switch ( value )
    {
        case DRN_AUTHORITY_OWNER: return "OWNER";
        case DRN_AUTHORITY_PROPOSAL_VOTE: return "PROPOSAL_VOTE";
        default: return "HOST";
    }
}

/*-------------------------------------------------------------------------*/

TDRNAuthority
CDRNDataGridSettings::StringToAuthority( const CORE::CString& value, TDRNAuthority defaultAuthority )
{GUCEF_TRACE;

    CORE::CString lower = value.Lowercase();
    if ( lower == "host" )
        return DRN_AUTHORITY_HOST;
    if ( lower == "owner" )
        return DRN_AUTHORITY_OWNER;
    if ( lower == "proposal_vote" )
        return DRN_AUTHORITY_PROPOSAL_VOTE;
    return defaultAuthority;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNDataGridSettings::NotifyModeToString( TDRNNotifyMode mode )
{GUCEF_TRACE;

    switch ( mode )
    {
        case DRN_NOTIFY_PER_PACKET: return "PER_PACKET";
        case DRN_NOTIFY_POLL: return "POLL";
        default: return "PER_CHANGE";
    }
}

/*-------------------------------------------------------------------------*/

TDRNNotifyMode
CDRNDataGridSettings::StringToNotifyMode( const CORE::CString& mode, TDRNNotifyMode defaultMode )
{GUCEF_TRACE;

    CORE::CString lower = mode.Lowercase();
    if ( lower == "per_change" )
        return DRN_NOTIFY_PER_CHANGE;
    if ( lower == "per_packet" )
        return DRN_NOTIFY_PER_PACKET;
    if ( lower == "poll" )
        return DRN_NOTIFY_POLL;
    return defaultMode;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "name", name ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "authority", AuthorityToString( authority ) ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "reliable", reliable ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "sendRateHz", sendRateHz ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxBytesPerTick", maxBytesPerTick ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "deltaAgainstAck", deltaAgainstAck ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "capacity", capacity ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "floatPrecision", defaultQuantisation.floatPrecision ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "intBits", defaultQuantisation.intBits ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "notify", NotifyModeToString( notify ) ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "priority", priority ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "snapshotOnSubscribe", snapshotOnSubscribe ) && totalSuccess;

    for ( UInt32 i=0; i<reliableKeys.size(); ++i )
        totalSuccess = GUCEF_NULL != cfg.AddChildWithValue( "ReliableKey", reliableKeys[ i ] ) && totalSuccess;
    for ( UInt32 i=0; i<quantisation.size(); ++i )
    {
        CORE::CDataNode* child = cfg.AddChild( "Quantisation" );
        if ( GUCEF_NULL == child )
        {
            totalSuccess = false;
            continue;
        }
        CORE::CDataNode* keyNode = child->AddChildWithValue( "Key", quantisation[ i ].key );
        totalSuccess = GUCEF_NULL != keyNode && totalSuccess;
        totalSuccess = child->SetAttribute( "floatPrecision", quantisation[ i ].quantisation.floatPrecision ) && totalSuccess;
        totalSuccess = child->SetAttribute( "intBits", quantisation[ i ].quantisation.intBits ) && totalSuccess;
    }
    CORE::CDataNode* voteNode = cfg.FindOrAddChild( "VotePolicy" );
    totalSuccess = GUCEF_NULL != voteNode && votePolicy.SaveConfig( *voteNode ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGridSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    name = cfg.GetAttributeValueOrChildValueByName( "name" ).AsString( name, true );
    authority = StringToAuthority( cfg.GetAttributeValueOrChildValueByName( "authority" ).AsString( AuthorityToString( authority ), true ), authority );
    reliable = cfg.GetAttributeValueOrChildValueByName( "reliable" ).AsBool( reliable, true );
    sendRateHz = cfg.GetAttributeValueOrChildValueByName( "sendRateHz" ).AsUInt32( sendRateHz, true );
    maxBytesPerTick = cfg.GetAttributeValueOrChildValueByName( "maxBytesPerTick" ).AsUInt32( maxBytesPerTick, true );
    deltaAgainstAck = cfg.GetAttributeValueOrChildValueByName( "deltaAgainstAck" ).AsBool( deltaAgainstAck, true );
    capacity = cfg.GetAttributeValueOrChildValueByName( "capacity" ).AsUInt32( capacity, true );
    defaultQuantisation.floatPrecision = cfg.GetAttributeValueOrChildValueByName( "floatPrecision" ).AsInt32( defaultQuantisation.floatPrecision, true );
    defaultQuantisation.intBits = cfg.GetAttributeValueOrChildValueByName( "intBits" ).AsUInt32( defaultQuantisation.intBits, true );
    notify = StringToNotifyMode( cfg.GetAttributeValueOrChildValueByName( "notify" ).AsString( NotifyModeToString( notify ), true ), notify );
    priority = cfg.GetAttributeValueOrChildValueByName( "priority" ).AsFloat32( priority, true );
    snapshotOnSubscribe = cfg.GetAttributeValueOrChildValueByName( "snapshotOnSubscribe" ).AsBool( snapshotOnSubscribe, true );

    reliableKeys.clear();
    quantisation.clear();
    CORE::CDataNode::const_iterator i = cfg.ConstBegin();
    while ( i != cfg.ConstEnd() )
    {
        const CORE::CDataNode* child = (*i);
        if ( GUCEF_NULL != child )
        {
            if ( child->GetName() == "ReliableKey" )
            {
                reliableKeys.push_back( child->GetValue() );
            }
            else
            if ( child->GetName() == "Quantisation" )
            {
                CKeyQuantisation entry;
                const CORE::CDataNode* keyNode = child->FindChild( "Key" );
                if ( GUCEF_NULL != keyNode )
                    entry.key = keyNode->GetValue();
                entry.quantisation.floatPrecision = child->GetAttributeValueOrChildValueByName( "floatPrecision" ).AsInt32( -1, true );
                entry.quantisation.intBits = child->GetAttributeValueOrChildValueByName( "intBits" ).AsUInt32( 0, true );
                quantisation.push_back( entry );
            }
            else
            if ( child->GetName() == "VotePolicy" )
            {
                votePolicy.LoadConfig( *child );
            }
        }
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNDataGridSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
