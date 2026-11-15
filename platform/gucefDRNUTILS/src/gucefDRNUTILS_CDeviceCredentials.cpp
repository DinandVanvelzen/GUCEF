/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CDeviceCredentials.h"

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINT_H
#include "gucefDRN_CDRNEndpoint.h"
#define GUCEF_DRN_CDRNENDPOINT_H
#endif /* GUCEF_DRN_CDRNENDPOINT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CDeviceCredentials::StoreProvider = "gucef.drnutils.devices";

/*-------------------------------------------------------------------------*/

namespace {

CORE::CString
AsEndpointUri( const CORE::CString& endpoint )
{GUCEF_TRACE;

    if ( 0 <= endpoint.HasSubstr( "://" ) )
        return endpoint;
    return "tcp://" + endpoint;
}

/*-------------------------------------------------------------------------*/

bool
GetJsonCodec( CORE::CDStoreCodecRegistry::TDStoreCodecPtr& codec, CORE::CString& error )
{GUCEF_TRACE;

    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) || codec.IsNULL() )
    {
        error = "the JSON data tree codec is not available (is dstorepluginJSONPARSER loaded?)";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CopyChildInto( const CORE::CDataNode& source, const char* name, CORE::CDataNode& target )
{GUCEF_TRACE;

    target.Clear();
    const CORE::CDataNode* child = source.FindChild( name );
    if ( GUCEF_NULL != child )
        target.Copy( *child );
}

}

/*-------------------------------------------------------------------------*/

CDeviceCredentials::CDeviceCredentials( void )
    : serviceName()
    , serviceId()
    , drnSessionName()
    , deviceId()
    , deviceKind()
    , roles()
    , deviceSeedHex()
    , devicePublicKeyHex()
    , token()
    , tokenExpiresUtcMs( 0 )
    , serverStaticKeyHex()
    , issuerPublicKeyHex()
    , sessionEndpoints()
    , securityPolicy()
    , nextSecurityPolicy()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceCredentials::IsComplete( void ) const
{GUCEF_TRACE;

    return !serviceName.IsNULLOrEmpty() && !drnSessionName.IsNULLOrEmpty() && !deviceId.IsNULLOrEmpty() &&
           64 == deviceSeedHex.Reveal().Length() && 64 == devicePublicKeyHex.Length() && !token.Reveal().IsNULLOrEmpty() &&
           64 == serverStaticKeyHex.Length() && !sessionEndpoints.empty();
}

/*-------------------------------------------------------------------------*/

bool
CDeviceCredentials::GetEndpointList( DRN::CDRNEndpointList& endpoints, const CORE::CString& hostOverride ) const
{GUCEF_TRACE;

    endpoints.Clear();
    CORE::CString::StringVector candidates;
    if ( !hostOverride.IsNULLOrEmpty() )
        candidates.push_back( hostOverride );
    for ( UInt32 i=0; i<sessionEndpoints.size(); ++i )
        candidates.push_back( sessionEndpoints[ i ] );

    for ( UInt32 i=0; i<candidates.size(); ++i )
    {
        DRN::CDRNEndpoint endpoint;
        if ( !endpoint.FromString( AsEndpointUri( candidates[ i ].Trim( true ).Trim( false ) ) ) || !endpoint.IsValid() )
            continue;
        // the static key of the service is pinned: a node that cannot prove it is refused
        if ( !serverStaticKeyHex.IsNULLOrEmpty() )
            endpoint.GetParams().Set( "key", CORE::CVariant( serverStaticKeyHex.Lowercase() ) );
        endpoints.Add( endpoint );
    }
    return !endpoints.IsEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CDeviceCredentials::UpdateSessionEndpoints( const CORE::CString::StringVector& endpoints )
{GUCEF_TRACE;

    if ( endpoints.empty() || endpoints == sessionEndpoints )
        return false;
    sessionEndpoints = endpoints;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDeviceCredentials::ToDataNode( CORE::CDataNode& node, bool includeSecrets ) const
{GUCEF_TRACE;

    node.Clear();
    node.SetName( "deviceCredentials" );
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "version", (UInt32) 1 );
    node.SetAttribute( "serviceName", serviceName );
    node.SetAttribute( "serviceId", serviceId );
    node.SetAttribute( "drnSessionName", drnSessionName );
    node.SetAttribute( "deviceId", deviceId );
    node.SetAttribute( "deviceKind", deviceKind );
    CORE::CDataNode* roleList = node.AddChild( "roles", GUCEF_DATATYPE_ARRAY );
    CORE::CString::StringSet::const_iterator r = roles.begin();
    for ( ; r != roles.end(); ++r )
        roleList->AddValueAsChild( *r );
    node.SetAttribute( "devicePublicKey", devicePublicKeyHex );
    node.SetAttribute( "tokenExpiresUtcMs", tokenExpiresUtcMs );
    node.SetAttribute( "serverStaticKey", serverStaticKeyHex );
    node.SetAttribute( "issuerPublicKey", issuerPublicKeyHex );
    CORE::CDataNode* endpointList = node.AddChild( "sessionEndpoints", GUCEF_DATATYPE_ARRAY );
    for ( UInt32 i=0; i<sessionEndpoints.size(); ++i )
        endpointList->AddValueAsChild( sessionEndpoints[ i ] );
    CORE::CDataNode* policy = node.AddChild( "securityPolicy", GUCEF_DATATYPE_OBJECT );
    policy->Copy( securityPolicy );
    policy->SetName( "securityPolicy" );
    if ( 0 != nextSecurityPolicy.GetNrOfChildNodes() || 0 != nextSecurityPolicy.GetAttCount() )
    {
        CORE::CDataNode* next = node.AddChild( "nextSecurityPolicy", GUCEF_DATATYPE_OBJECT );
        next->Copy( nextSecurityPolicy );
        next->SetName( "nextSecurityPolicy" );
    }
    if ( includeSecrets )
    {
        node.SetAttribute( "deviceSeed", deviceSeedHex.Reveal() );
        node.SetAttribute( "token", token.Reveal() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDeviceCredentials::FromDataNode( const CORE::CDataNode& node, CORE::CString& error )
{GUCEF_TRACE;

    CDeviceCredentials read;
    read.serviceName = node.GetAttributeValueOrChildValueByName( "serviceName" ).AsString();
    read.serviceId = node.GetAttributeValueOrChildValueByName( "serviceId" ).AsString();
    read.drnSessionName = node.GetAttributeValueOrChildValueByName( "drnSessionName" ).AsString();
    read.deviceId = node.GetAttributeValueOrChildValueByName( "deviceId" ).AsString();
    read.deviceKind = node.GetAttributeValueOrChildValueByName( "deviceKind" ).AsString();
    const CORE::CDataNode* roleList = node.FindChild( "roles" );
    if ( GUCEF_NULL != roleList )
    {
        CORE::CDataNode::const_iterator i = roleList->ConstBegin();
        for ( ; i != roleList->ConstEnd(); ++i )
            read.roles.insert( (*i)->GetValue().AsString() );
    }
    read.devicePublicKeyHex = node.GetAttributeValueOrChildValueByName( "devicePublicKey" ).AsString().Lowercase();
    read.tokenExpiresUtcMs = node.GetAttributeValueOrChildValueByName( "tokenExpiresUtcMs" ).AsUInt64( 0 );
    read.serverStaticKeyHex = node.GetAttributeValueOrChildValueByName( "serverStaticKey" ).AsString().Lowercase();
    read.issuerPublicKeyHex = node.GetAttributeValueOrChildValueByName( "issuerPublicKey" ).AsString().Lowercase();
    const CORE::CDataNode* endpointList = node.FindChild( "sessionEndpoints" );
    if ( GUCEF_NULL != endpointList )
    {
        CORE::CDataNode::const_iterator i = endpointList->ConstBegin();
        for ( ; i != endpointList->ConstEnd(); ++i )
            read.sessionEndpoints.push_back( (*i)->GetValue().AsString() );
    }
    CopyChildInto( node, "securityPolicy", read.securityPolicy );
    CopyChildInto( node, "nextSecurityPolicy", read.nextSecurityPolicy );
    read.deviceSeedHex.Set( node.GetAttributeValueOrChildValueByName( "deviceSeed" ).AsString() );
    read.token.Set( node.GetAttributeValueOrChildValueByName( "token" ).AsString() );
    if ( read.deviceId.IsNULLOrEmpty() || read.serviceName.IsNULLOrEmpty() )
    {
        error = "the device credentials have no device id or service name";
        return false;
    }
    *this = read;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceCredentials::SaveToStore( AUTH::ICredentialStore& store, const CORE::CString& slot, CORE::CString& error ) const
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GetJsonCodec( codec, error ) )
        return false;
    CORE::CDataNode node;
    ToDataNode( node, true );
    CORE::CDynamicBuffer buffer;
    {
        CORE::CDynamicBufferAccess access( buffer );
        if ( !codec->StoreDataTree( &node, &access ) )
        {
            error = "the device credentials cannot be encoded";
            return false;
        }
    }
    CORE::CString text;
    text.Set( buffer.AsConstTypePtr< char >(), buffer.GetDataSize() );
    buffer.Clear( false );
    node.Clear();
    AUTH::CSecretString secret( text );
    text.Clear();
    if ( !store.Store( StoreProvider, slot, secret ) )
    {
        error = "the credential store did not keep the device credentials";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceCredentials::LoadFromStore( AUTH::ICredentialStore& store, const CORE::CString& slot, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GetJsonCodec( codec, error ) )
        return false;
    AUTH::CSecretString secret;
    if ( !store.Load( StoreProvider, slot, secret ) )
    {
        error = "the credential store has no device credentials in the slot " + slot;
        return false;
    }
    const CORE::CString& text = secret.Reveal();
    CORE::CDynamicBuffer buffer;
    buffer.Append( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 );
    CORE::CDataNode node;
    bool parsed = false;
    {
        CORE::CDynamicBufferAccess access( buffer );
        parsed = codec->BuildDataTree( &node, &access );
    }
    buffer.Clear( false );
    if ( !parsed )
    {
        error = "the device credentials in the slot " + slot + " cannot be read";
        return false;
    }
    // the document may be the root or its only child, depending on the codec
    const CORE::CDataNode* root = &node;
    if ( node.GetAttributeValueOrChildValueByName( "deviceId" ).AsString().IsNULLOrEmpty() && 1 == node.GetNrOfChildNodes() )
        root = *node.ConstBegin();
    bool ok = FromDataNode( *root, error );
    node.Clear();
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceCredentials::RemoveFromStore( AUTH::ICredentialStore& store, const CORE::CString& slot )
{GUCEF_TRACE;

    return store.Remove( StoreProvider, slot );
}

/*-------------------------------------------------------------------------*/

void
CDeviceCredentials::Wipe( void )
{GUCEF_TRACE;

    deviceSeedHex.Clear();
    token.Clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
