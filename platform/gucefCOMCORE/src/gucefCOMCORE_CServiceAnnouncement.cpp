/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCOMCORE_CServiceAnnouncement.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CServiceAnnouncement::ProtocolVersionKey = "pv";
const char* CServiceAnnouncement::KeyFingerprintKey = "kfp";
const UInt32 CServiceAnnouncement::MaxEndpoints = 8;
const UInt32 CServiceAnnouncement::MaxEncodedSizeInBytes = 1400;
const UInt32 CServiceAnnouncement::DefaultTtlInSecs = 30;
const UInt32 CServiceAnnouncement::MinTtlInSecs = 3;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

UInt32
StringBytes( const CORE::CString& str )
{GUCEF_TRACE;

    UInt32 byteSize = str.ByteSize();
    return byteSize > 0 ? byteSize - 1 : 0;
}

/*-------------------------------------------------------------------------*/

bool
IsLabelChar( char c )
{GUCEF_TRACE;

    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '-' == c;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CServiceAnnouncement::CServiceAnnouncement( void )
    : CORE::CIConfigurable()
    , serviceType()
    , instanceId()
    , displayName()
    , endpoints()
    , protocolVersion()
    , keyFingerprint()
    , properties()
    , ttlInSecs( DefaultTtlInSecs )
{GUCEF_TRACE;

    properties.SetAllowMultipleValues( false );
}

/*-------------------------------------------------------------------------*/

CServiceAnnouncement::CServiceAnnouncement( const CServiceAnnouncement& src )
    : CORE::CIConfigurable( src )
    , serviceType( src.serviceType )
    , instanceId( src.instanceId )
    , displayName( src.displayName )
    , endpoints( src.endpoints )
    , protocolVersion( src.protocolVersion )
    , keyFingerprint( src.keyFingerprint )
    , properties( src.properties )
    , ttlInSecs( src.ttlInSecs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceAnnouncement::~CServiceAnnouncement()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceAnnouncement&
CServiceAnnouncement::operator=( const CServiceAnnouncement& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        serviceType = src.serviceType;
        instanceId = src.instanceId;
        displayName = src.displayName;
        endpoints = src.endpoints;
        protocolVersion = src.protocolVersion;
        keyFingerprint = src.keyFingerprint;
        properties = src.properties;
        ttlInSecs = src.ttlInSecs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CServiceAnnouncement::ToEntry( CDiscoveryEntry& entry ) const
{GUCEF_TRACE;

    entry.SetServiceType( serviceType );
    entry.SetApplicationInstanceId( instanceId );
    entry.SetInstanceName( displayName );
    entry.SetEndpoints( endpoints );
    entry.SetTtlInSecs( ttlInSecs );

    CORE::CValueList& attributes = entry.GetAttributes();
    attributes.DeleteAll();
    attributes.SetAllowMultipleValues( false );
    UInt32 keyCount = properties.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
        attributes.Set( properties.GetKey( i ).AsString(), properties.GetValue( i ).AsString() );
    if ( !protocolVersion.IsNULLOrEmpty() )
        attributes.Set( ProtocolVersionKey, protocolVersion );
    if ( !keyFingerprint.IsNULLOrEmpty() )
        attributes.Set( KeyFingerprintKey, keyFingerprint );
}

/*-------------------------------------------------------------------------*/

bool
CServiceAnnouncement::FromEntry( const CIDiscoveryEntry& entry )
{GUCEF_TRACE;

    CORE::CString id;
    entry.GetApplicationInstanceId( id );
    if ( entry.GetServiceType().IsNULLOrEmpty() || id.IsNULLOrEmpty() )
        return false;

    serviceType = entry.GetServiceType();
    instanceId = id;
    displayName = entry.GetInstanceName();
    endpoints = entry.GetEndpoints();
    ttlInSecs = entry.GetTtlInSecs();
    protocolVersion.Clear();
    keyFingerprint.Clear();
    properties.DeleteAll();
    properties.SetAllowMultipleValues( false );

    const CORE::CValueList& attributes = entry.GetAttributes();
    UInt32 keyCount = attributes.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
    {
        CORE::CString key = attributes.GetKey( i ).AsString();
        CORE::CString value = attributes.GetValue( i ).AsString();
        if ( key == ProtocolVersionKey )
            protocolVersion = value;
        else
        if ( key == KeyFingerprintKey )
            keyFingerprint = value;
        else
            properties.Set( key, value );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CServiceAnnouncement::GetEncodedSizeInBytes( void ) const
{GUCEF_TRACE;

    // magic, version and kind; the three strings; ttl; the endpoint count; the attribute count
    UInt32 size = 6 + ( 2 + StringBytes( serviceType ) ) + ( 2 + StringBytes( instanceId ) ) + ( 2 + StringBytes( displayName ) ) + 2 + 2 + 2;
    for ( size_t i=0; i<endpoints.size(); ++i )
        size += 2 + StringBytes( endpoints[ i ] );
    UInt32 keyCount = properties.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
        size += 2 + StringBytes( properties.GetKey( i ).AsString() ) + 2 + StringBytes( properties.GetValue( i ).AsString() );
    if ( !protocolVersion.IsNULLOrEmpty() )
        size += 2 + 2 + StringBytes( protocolVersion ) + 2;
    if ( !keyFingerprint.IsNULLOrEmpty() )
        size += 2 + 3 + 2 + StringBytes( keyFingerprint );
    return size;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAnnouncement::IsValid( CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !IsValidServiceType( serviceType ) )
    {
        reason = "the service type \"" + serviceType + "\" is not of the form _<name>._tcp or _<name>._udp";
        return false;
    }
    if ( instanceId.IsNULLOrEmpty() )
    {
        reason = "the announcement has no instance id";
        return false;
    }
    if ( endpoints.size() > MaxEndpoints )
    {
        reason = "the announcement has " + CORE::ToString( (UInt32) endpoints.size() ) + " endpoints, at most " + CORE::ToString( MaxEndpoints ) + " are allowed";
        return false;
    }
    if ( ttlInSecs < MinTtlInSecs )
    {
        reason = "the TTL of " + CORE::ToString( ttlInSecs ) + " seconds is shorter than " + CORE::ToString( MinTtlInSecs );
        return false;
    }
    UInt32 keyCount = properties.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
    {
        CORE::CString key = properties.GetKey( i ).AsString();
        if ( key.IsNULLOrEmpty() || IsReservedPropertyKey( key ) )
        {
            reason = "the property key \"" + key + "\" is reserved";
            return false;
        }
        if ( IsForbiddenPropertyKey( key ) )
        {
            reason = "the property key \"" + key + "\" looks like it holds a secret, which an announcement must never carry";
            return false;
        }
    }
    UInt32 size = GetEncodedSizeInBytes();
    if ( size > MaxEncodedSizeInBytes )
    {
        reason = "the announcement takes " + CORE::ToString( size ) + " bytes, at most " + CORE::ToString( MaxEncodedSizeInBytes ) + " fit in one datagram";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAnnouncement::HasSameContent( const CServiceAnnouncement& other ) const
{GUCEF_TRACE;

    return serviceType == other.serviceType &&
           instanceId == other.instanceId &&
           displayName == other.displayName &&
           endpoints == other.endpoints &&
           protocolVersion == other.protocolVersion &&
           keyFingerprint == other.keyFingerprint &&
           CDiscoveryEntry::AreAttributesEqual( properties, other.properties );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServiceAnnouncement::GetProtocolMajor( void ) const
{GUCEF_TRACE;

    if ( protocolVersion.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    return protocolVersion.SubstrToChar( '.', true ).Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

UInt16
CServiceAnnouncement::GetPort( const CORE::CString& scheme ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<endpoints.size(); ++i )
    {
        const CORE::CString& endpoint = endpoints[ i ];
        Int32 schemeEnd = endpoint.HasSubstr( "://", true );
        if ( !scheme.IsNULLOrEmpty() && ( schemeEnd < 0 || !endpoint.SubstrToIndex( (UInt32) schemeEnd, true ).Equals( scheme, false ) ) )
            continue;
        Int32 portStart = endpoint.HasChar( ':', false );
        if ( portStart < 0 || portStart == schemeEnd )
            continue;
        UInt32 port = CORE::StringToUInt32( endpoint.CutChars( (UInt32) portStart + 1, true, 0 ), 0 );
        if ( port > 0 && port <= 0xFFFF )
            return (UInt16) port;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServiceAnnouncement::GetKey( void ) const
{GUCEF_TRACE;

    return CIDiscoveryEntry::MakeEntryKey( serviceType, instanceId );
}

/*-------------------------------------------------------------------------*/

bool
CServiceAnnouncement::IsValidServiceType( const CORE::CString& type )
{GUCEF_TRACE;

    CORE::CString lower = type.Lowercase();
    if ( !lower.StartsWith( "_" ) || !( lower.EndsWith( "._tcp" ) || lower.EndsWith( "._udp" ) ) )
        return false;
    UInt32 labelLength = lower.Length() - 1 - 5;
    if ( labelLength < 1 || labelLength > 63 )
        return false;
    const char* text = lower.C_String();
    for ( UInt32 i=1; i<=labelLength; ++i )
    {
        if ( !IsLabelChar( text[ i ] ) )
            return false;
    }
    return '-' != text[ 1 ] && '-' != text[ labelLength ];
}

/*-------------------------------------------------------------------------*/

bool
CServiceAnnouncement::IsReservedPropertyKey( const CORE::CString& key )
{GUCEF_TRACE;

    return key == ProtocolVersionKey || key == KeyFingerprintKey;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAnnouncement::IsForbiddenPropertyKey( const CORE::CString& key )
{GUCEF_TRACE;

    static const char* const SecretWords[] = { "secret", "password", "passwd", "token", "credential", "privatekey", "private_key", "private-key", "apikey", "api_key", "api-key", GUCEF_NULL };
    CORE::CString lower = key.Lowercase();
    for ( UInt32 i=0; GUCEF_NULL != SecretWords[ i ]; ++i )
    {
        if ( lower.HasSubstr( SecretWords[ i ], true ) >= 0 )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServiceAnnouncement::MakeEndpoint( const CORE::CString& scheme, const CORE::CString& host, UInt16 port )
{GUCEF_TRACE;

    return scheme + "://" + ( host.IsNULLOrEmpty() ? CORE::CString( "0.0.0.0" ) : host ) + ":" + CORE::UInt16ToString( port );
}

/*-------------------------------------------------------------------------*/

bool
CServiceAnnouncement::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "serviceType", serviceType );
    config.SetAttribute( "instanceId", instanceId );
    config.SetAttribute( "displayName", displayName );
    config.SetAttribute( "protocolVersion", protocolVersion );
    config.SetAttribute( "keyFingerprint", keyFingerprint );
    config.SetAttribute( "ttlInSecs", ttlInSecs );
    CORE::CDataNode* list = config.AddChild( "endpoints", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<endpoints.size(); ++i )
        list->AddValueAsChild( CORE::CVariant( endpoints[ i ] ) );
    CORE::CDataNode* props = config.AddChild( "properties", GUCEF_DATATYPE_OBJECT );
    UInt32 keyCount = properties.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
        props->SetAttribute( properties.GetKey( i ).AsString(), properties.GetValue( i ).AsString() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAnnouncement::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    serviceType = config.GetAttributeValueOrChildValueByName( "serviceType" ).AsString( serviceType, true );
    instanceId = config.GetAttributeValueOrChildValueByName( "instanceId" ).AsString( instanceId, true );
    displayName = config.GetAttributeValueOrChildValueByName( "displayName" ).AsString( displayName, true );
    protocolVersion = config.GetAttributeValueOrChildValueByName( "protocolVersion" ).AsString( protocolVersion, true );
    keyFingerprint = config.GetAttributeValueOrChildValueByName( "keyFingerprint" ).AsString( keyFingerprint, true );
    ttlInSecs = config.GetAttributeValueOrChildValueByName( "ttlInSecs" ).AsUInt32( ttlInSecs, true );

    const CORE::CDataNode* list = config.FindChild( "endpoints" );
    if ( GUCEF_NULL != list )
    {
        endpoints.clear();
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
        {
            CORE::CString endpoint = (*i)->GetValue().AsString().Trim( true ).Trim( false );
            if ( !endpoint.IsNULLOrEmpty() )
                endpoints.push_back( endpoint );
        }
    }
    const CORE::CDataNode* props = config.FindChild( "properties" );
    if ( GUCEF_NULL != props )
    {
        properties.DeleteAll();
        properties.SetAllowMultipleValues( false );
        for ( CORE::CDataNode::TAttributeMap::const_iterator a = props->AttributeBegin(); a != props->AttributeEnd(); ++a )
            properties.Set( (*a).first, (*a).second.AsString() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServiceAnnouncement::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CServiceAnnouncement";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
