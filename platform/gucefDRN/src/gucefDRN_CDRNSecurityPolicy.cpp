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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#include "gucefDRN_CDRNSecurityPolicy.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNSecurityPolicy::ClassTypeName = "GUCEF::DRN::CDRNSecurityPolicy";

/** Version of the canonical encoding, part of what is hashed */
static const UInt8 PolicyEncodingVersion = 2;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString::StringSet
LowercaseSet( const CORE::CString::StringSet& names )
{GUCEF_TRACE;

    CORE::CString::StringSet result;
    CORE::CString::StringSet::const_iterator i = names.begin();
    while ( i != names.end() )
    {
        CORE::CString name = (*i).Lowercase().Trim( true ).Trim( false );
        if ( !name.IsNULLOrEmpty() )
            result.insert( name );
        ++i;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

static CORE::CString::StringSet
ParseNameSet( const CORE::CString& names )
{GUCEF_TRACE;

    CORE::CString::StringVector elements = names.ParseElements( ',', false );
    CORE::CString::StringSet result;
    for ( UInt32 i=0; i<elements.size(); ++i )
    {
        CORE::CString name = elements[ i ].Trim( true ).Trim( false );
        if ( !name.IsNULLOrEmpty() )
            result.insert( name );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

static CORE::CString::StringVector
ParseNameList( const CORE::CString& names )
{GUCEF_TRACE;

    CORE::CString::StringVector elements = names.ParseElements( ',', false );
    CORE::CString::StringVector result;
    for ( UInt32 i=0; i<elements.size(); ++i )
    {
        CORE::CString name = elements[ i ].Trim( true ).Trim( false );
        if ( !name.IsNULLOrEmpty() )
            result.push_back( name );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

static bool
ContainsIgnoringCase( const CORE::CString::StringSet& names, const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString::StringSet::const_iterator i = names.begin();
    while ( i != names.end() )
    {
        if ( (*i).Equals( name, false ) )
            return true;
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** True if every name is one of the allowed ones (ignoring case); an empty allowed set allows any, an empty names set is any */
static bool
IsNameSubset( const CORE::CString::StringSet& names, const CORE::CString::StringSet& allowed, CORE::CString& extra )
{GUCEF_TRACE;

    if ( allowed.empty() )
        return true;
    if ( names.empty() )
    {
        extra = "any";
        return false;
    }
    CORE::CString::StringSet::const_iterator i = names.begin();
    while ( i != names.end() )
    {
        if ( !ContainsIgnoringCase( allowed, *i ) )
        {
            extra = (*i);
            return false;
        }
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
IsLimitWeaker( UInt32 limit, UInt32 minimum, const char* name, CORE::CString& why )
{GUCEF_TRACE;

    if ( 0 == minimum || ( 0 != limit && limit <= minimum ) )
        return false;
    why = CORE::CString( name ) + " " + ( 0 == limit ? CORE::CString( "has no limit" ) : CORE::UInt32ToString( limit ) ) + ", the minimum is " + CORE::UInt32ToString( minimum );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNPolicyLimits::CDRNPolicyLimits( void )
    : maxMessageSize( DRN_MAX_STREAM_PACKET_SIZE )
    , maxPreAuthPacketSize( 16 * 1024 )
    , maxMessagesPerSecond( 20000 )
    , maxBytesPerSecond( 64 * 1024 * 1024 )
    , sustainedLimitSeconds( 5 )
    , maxPendingHandshakes( 256 )
    , maxHandshakesPerAddressPerMinute( 120 )
    , maxFragmentGroups( 64 )
    , maxFragmentBytes( 32 * 1024 * 1024 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDRNPolicyLimits::operator==( const CDRNPolicyLimits& other ) const
{GUCEF_TRACE;

    return maxMessageSize == other.maxMessageSize                                       &&
           maxPreAuthPacketSize == other.maxPreAuthPacketSize                           &&
           maxMessagesPerSecond == other.maxMessagesPerSecond                           &&
           maxBytesPerSecond == other.maxBytesPerSecond                                 &&
           sustainedLimitSeconds == other.sustainedLimitSeconds                         &&
           maxPendingHandshakes == other.maxPendingHandshakes                           &&
           maxHandshakesPerAddressPerMinute == other.maxHandshakesPerAddressPerMinute   &&
           maxFragmentGroups == other.maxFragmentGroups                                 &&
           maxFragmentBytes == other.maxFragmentBytes;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPolicyLimits::operator!=( const CDRNPolicyLimits& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

CDRNAnomalyThresholds::CDRNAnomalyThresholds( void )
    : clockDriftPercent( 5 )
    , clockWindowMs( 2000 )
    , clockSustainMs( 6000 )
    , silenceMs( 1000 )
    , burstPackets( 16 )
    , burstWindowMs( 100 )
    , floodPacketsPerSecond( 10000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDRNAnomalyThresholds::operator==( const CDRNAnomalyThresholds& other ) const
{GUCEF_TRACE;

    return clockDriftPercent == other.clockDriftPercent         &&
           clockWindowMs == other.clockWindowMs                 &&
           clockSustainMs == other.clockSustainMs               &&
           silenceMs == other.silenceMs                         &&
           burstPackets == other.burstPackets                   &&
           burstWindowMs == other.burstWindowMs                 &&
           floodPacketsPerSecond == other.floodPacketsPerSecond;
}

/*-------------------------------------------------------------------------*/

bool
CDRNAnomalyThresholds::operator!=( const CDRNAnomalyThresholds& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

CDRNSecurityPolicy::CDRNSecurityPolicy( void )
    : CORE::CIConfigurable()
    , requireEncryption( true )
    , requireSignedTokens( false )
    , requireTokenPolicyHash( false )
    , ciphers()
    , allowedChannelTypes()
    , relayOnly( false )
    , allowedAuthenticators()
    , statelessCookies( true )
    , limits()
    , anomalies()
{GUCEF_TRACE;

    ciphers.push_back( "xchacha20-poly1305" );
    ciphers.push_back( "chacha20-poly1305" );
    ciphers.push_back( "aes-256-gcm" );
}

/*-------------------------------------------------------------------------*/

CDRNSecurityPolicy::CDRNSecurityPolicy( const CDRNSecurityPolicy& src )
    : CORE::CIConfigurable( src )
    , requireEncryption( src.requireEncryption )
    , requireSignedTokens( src.requireSignedTokens )
    , requireTokenPolicyHash( src.requireTokenPolicyHash )
    , ciphers( src.ciphers )
    , allowedChannelTypes( src.allowedChannelTypes )
    , relayOnly( src.relayOnly )
    , allowedAuthenticators( src.allowedAuthenticators )
    , statelessCookies( src.statelessCookies )
    , limits( src.limits )
    , anomalies( src.anomalies )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNSecurityPolicy::~CDRNSecurityPolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNSecurityPolicy&
CDRNSecurityPolicy::operator=( const CDRNSecurityPolicy& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        requireEncryption = src.requireEncryption;
        requireSignedTokens = src.requireSignedTokens;
        requireTokenPolicyHash = src.requireTokenPolicyHash;
        ciphers = src.ciphers;
        allowedChannelTypes = src.allowedChannelTypes;
        relayOnly = src.relayOnly;
        allowedAuthenticators = src.allowedAuthenticators;
        statelessCookies = src.statelessCookies;
        limits = src.limits;
        anomalies = src.anomalies;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSecurityPolicy::operator==( const CDRNSecurityPolicy& other ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer a;
    CORE::CDynamicBuffer b;
    GetCanonicalEncoding( a );
    other.GetCanonicalEncoding( b );
    return a.GetDataSize() == b.GetDataSize() &&
           0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSecurityPolicy::operator!=( const CDRNSecurityPolicy& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSecurityPolicy::IsWeakerThan( const CDRNSecurityPolicy& minimum, CORE::CString& why ) const
{GUCEF_TRACE;

    why.Clear();
    if ( minimum.requireEncryption && !requireEncryption )
    {
        why = "it does not require encryption";
        return true;
    }
    if ( minimum.requireSignedTokens && !requireSignedTokens )
    {
        why = "it does not require signed tokens";
        return true;
    }
    if ( minimum.requireTokenPolicyHash && !requireTokenPolicyHash )
    {
        why = "it does not require the policy hash in tokens";
        return true;
    }
    if ( minimum.relayOnly && !relayOnly )
    {
        why = "it is not relay only";
        return true;
    }
    if ( minimum.statelessCookies && !statelessCookies )
    {
        why = "it has no stateless cookies";
        return true;
    }
    CORE::CString extra;
    CORE::CString::StringSet cipherSet;
    CORE::CString::StringSet minimumCipherSet;
    for ( UInt32 i=0; i<ciphers.size(); ++i )
        cipherSet.insert( ciphers[ i ] );
    for ( UInt32 i=0; i<minimum.ciphers.size(); ++i )
        minimumCipherSet.insert( minimum.ciphers[ i ] );
    if ( !IsNameSubset( cipherSet, minimumCipherSet, extra ) )
    {
        why = "it allows the cipher " + extra;
        return true;
    }
    if ( !IsNameSubset( allowedChannelTypes, minimum.allowedChannelTypes, extra ) )
    {
        why = "it allows the channel type " + extra;
        return true;
    }
    if ( !IsNameSubset( allowedAuthenticators, minimum.allowedAuthenticators, extra ) )
    {
        why = "it allows the authenticator " + extra;
        return true;
    }
    const CDRNPolicyLimits& minimumLimits = minimum.limits;
    return IsLimitWeaker( limits.maxMessageSize, minimumLimits.maxMessageSize, "maxMessageSize", why ) ||
           IsLimitWeaker( limits.maxPreAuthPacketSize, minimumLimits.maxPreAuthPacketSize, "maxPreAuthPacketSize", why ) ||
           IsLimitWeaker( limits.maxMessagesPerSecond, minimumLimits.maxMessagesPerSecond, "maxMessagesPerSecond", why ) ||
           IsLimitWeaker( limits.maxBytesPerSecond, minimumLimits.maxBytesPerSecond, "maxBytesPerSecond", why ) ||
           IsLimitWeaker( limits.sustainedLimitSeconds, minimumLimits.sustainedLimitSeconds, "sustainedLimitSeconds", why ) ||
           IsLimitWeaker( limits.maxPendingHandshakes, minimumLimits.maxPendingHandshakes, "maxPendingHandshakes", why ) ||
           IsLimitWeaker( limits.maxHandshakesPerAddressPerMinute, minimumLimits.maxHandshakesPerAddressPerMinute, "maxHandshakesPerAddressPerMinute", why ) ||
           IsLimitWeaker( limits.maxFragmentGroups, minimumLimits.maxFragmentGroups, "maxFragmentGroups", why ) ||
           IsLimitWeaker( limits.maxFragmentBytes, minimumLimits.maxFragmentBytes, "maxFragmentBytes", why );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSecurityPolicy::IsChannelTypeAllowed( const CORE::CString& channelType ) const
{GUCEF_TRACE;

    return allowedChannelTypes.empty() || ContainsIgnoringCase( allowedChannelTypes, channelType );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSecurityPolicy::IsAuthenticatorAllowed( const CORE::CString& authenticatorType ) const
{GUCEF_TRACE;

    return allowedAuthenticators.empty() || ContainsIgnoringCase( allowedAuthenticators, authenticatorType );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSecurityPolicy::IsEndpointAllowed( const CDRNEndpoint& endpoint ) const
{GUCEF_TRACE;

    if ( relayOnly && !endpoint.GetChannelType().Equals( CDRNEndpoint::ChannelTypeRelay, false ) )
        return false;
    return IsChannelTypeAllowed( endpoint.GetChannelType() );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSecurityPolicy::IsPacketProtectionAvailable( void ) const
{GUCEF_TRACE;

    if ( !CDRNCrypto::IsKeyAgreementAvailable() )
        return false;
    for ( UInt32 i=0; i<ciphers.size(); ++i )
    {
        if ( CDRNCrypto::IsCipherAvailable( ciphers[ i ] ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSecurityPolicy::CanBeHonoured( CORE::CString& whyNot ) const
{GUCEF_TRACE;

    whyNot.Clear();
    if ( requireEncryption && !IsPacketProtectionAvailable() )
    {
        whyNot = "the security policy requires encryption and there is no \"x25519\" key agreement codec or no codec for any of its ciphers";
        return false;
    }
    if ( requireSignedTokens && !CDRNCrypto::IsSignatureAvailable() )
    {
        whyNot = "the security policy requires signed tokens and there is no \"ed25519\" signature codec";
        return false;
    }
    if ( limits.maxFragmentBytes > 0 && limits.maxMessageSize > limits.maxFragmentBytes )
    {
        whyNot = "the security policy allows messages larger than the fragment bytes limit";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNSecurityPolicy::GetCanonicalEncoding( CORE::CDynamicBuffer& encoding ) const
{GUCEF_TRACE;

    encoding.SetDataSize( 0 );
    CDRNWireWriter writer( encoding );
    writer.WriteString( "DRNSecurityPolicy" );
    writer.WriteUInt8( PolicyEncodingVersion );
    writer.WriteUInt8( requireEncryption ? 1 : 0 );
    writer.WriteUInt8( requireSignedTokens ? 1 : 0 );
    writer.WriteUInt8( requireTokenPolicyHash ? 1 : 0 );
    writer.WriteUInt8( relayOnly ? 1 : 0 );
    writer.WriteUInt8( statelessCookies ? 1 : 0 );

    // the cipher order is a preference and counts, the sets are written sorted
    CORE::CString::StringVector cipherNames;
    for ( UInt32 i=0; i<ciphers.size(); ++i )
    {
        CORE::CString name = ciphers[ i ].Lowercase().Trim( true ).Trim( false );
        if ( !name.IsNULLOrEmpty() )
            cipherNames.push_back( name );
    }
    writer.WriteVarUInt32( (UInt32) cipherNames.size() );
    for ( UInt32 i=0; i<cipherNames.size(); ++i )
        writer.WriteString( cipherNames[ i ] );

    CORE::CString::StringSet channelTypes = LowercaseSet( allowedChannelTypes );
    writer.WriteVarUInt32( (UInt32) channelTypes.size() );
    CORE::CString::StringSet::const_iterator i = channelTypes.begin();
    while ( i != channelTypes.end() )
    {
        writer.WriteString( *i );
        ++i;
    }

    CORE::CString::StringSet authenticators = LowercaseSet( allowedAuthenticators );
    writer.WriteVarUInt32( (UInt32) authenticators.size() );
    i = authenticators.begin();
    while ( i != authenticators.end() )
    {
        writer.WriteString( *i );
        ++i;
    }

    writer.WriteVarUInt32( limits.maxMessageSize );
    writer.WriteVarUInt32( limits.maxPreAuthPacketSize );
    writer.WriteVarUInt32( limits.maxMessagesPerSecond );
    writer.WriteVarUInt32( limits.maxBytesPerSecond );
    writer.WriteVarUInt32( limits.sustainedLimitSeconds );
    writer.WriteVarUInt32( limits.maxPendingHandshakes );
    writer.WriteVarUInt32( limits.maxHandshakesPerAddressPerMinute );
    writer.WriteVarUInt32( limits.maxFragmentGroups );
    writer.WriteVarUInt32( limits.maxFragmentBytes );

    writer.WriteVarUInt32( anomalies.clockDriftPercent );
    writer.WriteVarUInt32( anomalies.clockWindowMs );
    writer.WriteVarUInt32( anomalies.clockSustainMs );
    writer.WriteVarUInt32( anomalies.silenceMs );
    writer.WriteVarUInt32( anomalies.burstPackets );
    writer.WriteVarUInt32( anomalies.burstWindowMs );
    writer.WriteVarUInt32( anomalies.floodPacketsPerSecond );
}

/*-------------------------------------------------------------------------*/

void
CDRNSecurityPolicy::GetPolicyHash( UInt8* hash ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer encoding;
    GetCanonicalEncoding( encoding );
    CDRNCrypto::Sha256( encoding.GetConstBufferPtr(), encoding.GetDataSize(), hash );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNSecurityPolicy::GetPolicyHashString( void ) const
{GUCEF_TRACE;

    static const char hexDigits[] = "0123456789abcdef";
    UInt8 hash[ PolicyHashSize ];
    GetPolicyHash( hash );
    char text[ PolicyHashSize * 2 + 1 ];
    for ( UInt32 i=0; i<PolicyHashSize; ++i )
    {
        text[ i*2 ] = hexDigits[ hash[ i ] >> 4 ];
        text[ i*2 + 1 ] = hexDigits[ hash[ i ] & 0x0F ];
    }
    text[ PolicyHashSize * 2 ] = '\0';
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSecurityPolicy::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "requireEncryption", requireEncryption ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "requireSignedTokens", requireSignedTokens ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "requireTokenPolicyHash", requireTokenPolicyHash ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "ciphers", CORE::CString::Empty.Combine( ciphers, ',' ) ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "allowedChannelTypes", CORE::CString::Empty.Combine( allowedChannelTypes, ',' ) ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "relayOnly", relayOnly ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "allowedAuthenticators", CORE::CString::Empty.Combine( allowedAuthenticators, ',' ) ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "statelessCookies", statelessCookies ) && totalSuccess;

    CORE::CDataNode* limitsNode = cfg.FindOrAddChild( "Limits" );
    if ( GUCEF_NULL == limitsNode )
        return false;
    totalSuccess = limitsNode->SetAttribute( "maxMessageSize", limits.maxMessageSize ) && totalSuccess;
    totalSuccess = limitsNode->SetAttribute( "maxPreAuthPacketSize", limits.maxPreAuthPacketSize ) && totalSuccess;
    totalSuccess = limitsNode->SetAttribute( "maxMessagesPerSecond", limits.maxMessagesPerSecond ) && totalSuccess;
    totalSuccess = limitsNode->SetAttribute( "maxBytesPerSecond", limits.maxBytesPerSecond ) && totalSuccess;
    totalSuccess = limitsNode->SetAttribute( "sustainedLimitSeconds", limits.sustainedLimitSeconds ) && totalSuccess;
    totalSuccess = limitsNode->SetAttribute( "maxPendingHandshakes", limits.maxPendingHandshakes ) && totalSuccess;
    totalSuccess = limitsNode->SetAttribute( "maxHandshakesPerAddressPerMinute", limits.maxHandshakesPerAddressPerMinute ) && totalSuccess;
    totalSuccess = limitsNode->SetAttribute( "maxFragmentGroups", limits.maxFragmentGroups ) && totalSuccess;
    totalSuccess = limitsNode->SetAttribute( "maxFragmentBytes", limits.maxFragmentBytes ) && totalSuccess;

    CORE::CDataNode* anomaliesNode = cfg.FindOrAddChild( "Anomalies" );
    if ( GUCEF_NULL == anomaliesNode )
        return false;
    totalSuccess = anomaliesNode->SetAttribute( "clockDriftPercent", anomalies.clockDriftPercent ) && totalSuccess;
    totalSuccess = anomaliesNode->SetAttribute( "clockWindowMs", anomalies.clockWindowMs ) && totalSuccess;
    totalSuccess = anomaliesNode->SetAttribute( "clockSustainMs", anomalies.clockSustainMs ) && totalSuccess;
    totalSuccess = anomaliesNode->SetAttribute( "silenceMs", anomalies.silenceMs ) && totalSuccess;
    totalSuccess = anomaliesNode->SetAttribute( "burstPackets", anomalies.burstPackets ) && totalSuccess;
    totalSuccess = anomaliesNode->SetAttribute( "burstWindowMs", anomalies.burstWindowMs ) && totalSuccess;
    totalSuccess = anomaliesNode->SetAttribute( "floodPacketsPerSecond", anomalies.floodPacketsPerSecond ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSecurityPolicy::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    requireEncryption = cfg.GetAttributeValueOrChildValueByName( "requireEncryption" ).AsBool( requireEncryption, true );
    requireSignedTokens = cfg.GetAttributeValueOrChildValueByName( "requireSignedTokens" ).AsBool( requireSignedTokens, true );
    requireTokenPolicyHash = cfg.GetAttributeValueOrChildValueByName( "requireTokenPolicyHash" ).AsBool( requireTokenPolicyHash, true );
    const CORE::CVariant& cipherValue = cfg.GetAttributeValueOrChildValueByName( "ciphers" );
    if ( cipherValue.IsInitialized() )
        ciphers = ParseNameList( cipherValue.AsString( CORE::CString::Empty, true ) );
    const CORE::CVariant& channelTypesValue = cfg.GetAttributeValueOrChildValueByName( "allowedChannelTypes" );
    if ( channelTypesValue.IsInitialized() )
        allowedChannelTypes = ParseNameSet( channelTypesValue.AsString( CORE::CString::Empty, true ) );
    relayOnly = cfg.GetAttributeValueOrChildValueByName( "relayOnly" ).AsBool( relayOnly, true );
    const CORE::CVariant& authenticatorsValue = cfg.GetAttributeValueOrChildValueByName( "allowedAuthenticators" );
    if ( authenticatorsValue.IsInitialized() )
        allowedAuthenticators = ParseNameSet( authenticatorsValue.AsString( CORE::CString::Empty, true ) );
    statelessCookies = cfg.GetAttributeValueOrChildValueByName( "statelessCookies" ).AsBool( statelessCookies, true );

    const CORE::CDataNode* limitsNode = cfg.FindChild( "Limits" );
    if ( GUCEF_NULL != limitsNode )
    {
        limits.maxMessageSize = limitsNode->GetAttributeValueOrChildValueByName( "maxMessageSize" ).AsUInt32( limits.maxMessageSize, true );
        limits.maxPreAuthPacketSize = limitsNode->GetAttributeValueOrChildValueByName( "maxPreAuthPacketSize" ).AsUInt32( limits.maxPreAuthPacketSize, true );
        limits.maxMessagesPerSecond = limitsNode->GetAttributeValueOrChildValueByName( "maxMessagesPerSecond" ).AsUInt32( limits.maxMessagesPerSecond, true );
        limits.maxBytesPerSecond = limitsNode->GetAttributeValueOrChildValueByName( "maxBytesPerSecond" ).AsUInt32( limits.maxBytesPerSecond, true );
        limits.sustainedLimitSeconds = limitsNode->GetAttributeValueOrChildValueByName( "sustainedLimitSeconds" ).AsUInt32( limits.sustainedLimitSeconds, true );
        limits.maxPendingHandshakes = limitsNode->GetAttributeValueOrChildValueByName( "maxPendingHandshakes" ).AsUInt32( limits.maxPendingHandshakes, true );
        limits.maxHandshakesPerAddressPerMinute = limitsNode->GetAttributeValueOrChildValueByName( "maxHandshakesPerAddressPerMinute" ).AsUInt32( limits.maxHandshakesPerAddressPerMinute, true );
        limits.maxFragmentGroups = limitsNode->GetAttributeValueOrChildValueByName( "maxFragmentGroups" ).AsUInt32( limits.maxFragmentGroups, true );
        limits.maxFragmentBytes = limitsNode->GetAttributeValueOrChildValueByName( "maxFragmentBytes" ).AsUInt32( limits.maxFragmentBytes, true );
    }

    const CORE::CDataNode* anomaliesNode = cfg.FindChild( "Anomalies" );
    if ( GUCEF_NULL != anomaliesNode )
    {
        anomalies.clockDriftPercent = anomaliesNode->GetAttributeValueOrChildValueByName( "clockDriftPercent" ).AsUInt32( anomalies.clockDriftPercent, true );
        anomalies.clockWindowMs = anomaliesNode->GetAttributeValueOrChildValueByName( "clockWindowMs" ).AsUInt32( anomalies.clockWindowMs, true );
        anomalies.clockSustainMs = anomaliesNode->GetAttributeValueOrChildValueByName( "clockSustainMs" ).AsUInt32( anomalies.clockSustainMs, true );
        anomalies.silenceMs = anomaliesNode->GetAttributeValueOrChildValueByName( "silenceMs" ).AsUInt32( anomalies.silenceMs, true );
        anomalies.burstPackets = anomaliesNode->GetAttributeValueOrChildValueByName( "burstPackets" ).AsUInt32( anomalies.burstPackets, true );
        anomalies.burstWindowMs = anomaliesNode->GetAttributeValueOrChildValueByName( "burstWindowMs" ).AsUInt32( anomalies.burstWindowMs, true );
        anomalies.floodPacketsPerSecond = anomaliesNode->GetAttributeValueOrChildValueByName( "floodPacketsPerSecond" ).AsUInt32( anomalies.floodPacketsPerSecond, true );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNSecurityPolicy::GetClassTypeName( void ) const
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
