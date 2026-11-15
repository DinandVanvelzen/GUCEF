/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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
#include "gucefASSETSTORE_CAssetServiceInfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetServiceInfo::CAssetServiceInfo( void )
    : CORE::CIDataNodeSerializable()
    , implementationId()
    , version()
    , inProcess( false )
    , capabilities()
    , roles()
    , maxPageEntries( 0 )
    , catalogueRevision( 0 )
    , keyringState()
    , locations()
    , stores()
    , sources()
    , converterName()
    , converterProfile()
    , hasRenderer( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetServiceInfo::~CAssetServiceInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetServiceInfo::Clear( void )
{GUCEF_TRACE;

    *this = CAssetServiceInfo();
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceInfo::HasCapability( const CORE::CString& capability ) const
{GUCEF_TRACE;

    return capabilities.find( capability ) != capabilities.end();
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceInfo::Serialize( CORE::CDataNode& domRootNode                        ,
                             const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "implementationId", implementationId ) && success;
    success = domRootNode.SetAttribute( "version", version ) && success;
    success = domRootNode.SetAttribute( "inProcess", inProcess ) && success;
    success = domRootNode.SetValuesOfChildByName( "capabilities", capabilities ) && success;
    success = domRootNode.SetValuesOfChildByName( "roles", roles ) && success;
    success = domRootNode.SetAttribute( "maxPageEntries", maxPageEntries ) && success;
    success = domRootNode.SetAttribute( "catalogueRevision", catalogueRevision ) && success;
    success = domRootNode.SetAttribute( "keyringState", keyringState ) && success;
    CORE::CDataNode* locationsNode = domRootNode.AddChild( "locations", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; GUCEF_NULL != locationsNode && i<locations.size(); ++i )
        success = locations[ i ].Serialize( *locationsNode->AddChild( "location", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    CORE::CDataNode* storesNode = domRootNode.AddChild( "stores", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; GUCEF_NULL != storesNode && i<stores.size(); ++i )
        success = stores[ i ].Serialize( *storesNode->AddChild( "store", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    CORE::CDataNode* sourcesNode = domRootNode.AddChild( "sources", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; GUCEF_NULL != sourcesNode && i<sources.size(); ++i )
        success = sources[ i ].Serialize( *sourcesNode->AddChild( "source", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = domRootNode.SetAttribute( "converterName", converterName ) && success;
    success = domRootNode.SetAttribute( "converterProfile", converterProfile ) && success;
    success = domRootNode.SetAttribute( "hasRenderer", hasRenderer ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceInfo::Deserialize( const CORE::CDataNode& domRootNode                  ,
                               const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    implementationId = domRootNode.GetAttributeValueOrChildValueByName( "implementationId" ).AsString( implementationId );
    version = domRootNode.GetAttributeValueOrChildValueByName( "version" ).AsString( version );
    inProcess = domRootNode.GetAttributeValueOrChildValueByName( "inProcess" ).AsBool( inProcess, true );
    domRootNode.GetValuesOfChildByName( "capabilities", capabilities );
    domRootNode.GetValuesOfChildByName( "roles", roles );
    maxPageEntries = domRootNode.GetAttributeValueOrChildValueByName( "maxPageEntries" ).AsUInt32( maxPageEntries, true );
    catalogueRevision = domRootNode.GetAttributeValueOrChildValueByName( "catalogueRevision" ).AsUInt64( catalogueRevision, true );
    keyringState = domRootNode.GetAttributeValueOrChildValueByName( "keyringState" ).AsString( keyringState );
    const CORE::CDataNode* locationsNode = domRootNode.FindChild( "locations" );
    if ( GUCEF_NULL != locationsNode )
    {
        CORE::CDataNode::const_iterator i = locationsNode->ConstBegin();
        while ( i != locationsNode->ConstEnd() )
        {
            CAssetLocationInfo entry;
            if ( entry.Deserialize( *(*i), settings ) )
                locations.push_back( entry );
            ++i;
        }
    }
    const CORE::CDataNode* storesNode = domRootNode.FindChild( "stores" );
    if ( GUCEF_NULL != storesNode )
    {
        CORE::CDataNode::const_iterator i = storesNode->ConstBegin();
        while ( i != storesNode->ConstEnd() )
        {
            CAssetStoreInfo entry;
            if ( entry.Deserialize( *(*i), settings ) )
                stores.push_back( entry );
            ++i;
        }
    }
    const CORE::CDataNode* sourcesNode = domRootNode.FindChild( "sources" );
    if ( GUCEF_NULL != sourcesNode )
    {
        CORE::CDataNode::const_iterator i = sourcesNode->ConstBegin();
        while ( i != sourcesNode->ConstEnd() )
        {
            CAssetSourcePolicy entry;
            if ( entry.Deserialize( *(*i), settings ) )
                sources.push_back( entry );
            ++i;
        }
    }
    converterName = domRootNode.GetAttributeValueOrChildValueByName( "converterName" ).AsString( converterName );
    converterProfile = domRootNode.GetAttributeValueOrChildValueByName( "converterProfile" ).AsString( converterProfile );
    hasRenderer = domRootNode.GetAttributeValueOrChildValueByName( "hasRenderer" ).AsBool( hasRenderer, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetServiceInfo::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetServiceInfo( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetServiceInfo::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetServiceInfo::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetServiceInfo";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetServiceInfo::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetServiceInfo";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
