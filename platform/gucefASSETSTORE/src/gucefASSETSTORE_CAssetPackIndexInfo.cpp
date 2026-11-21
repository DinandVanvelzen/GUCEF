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

#include "gucefASSETSTORE_CAssetPackIndexInfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetPackIndexInfo::StateLoaded = "loaded";
const CORE::CString CAssetPackIndexInfo::StateLoading = "loading";
const CORE::CString CAssetPackIndexInfo::StateStale = "stale";
const CORE::CString CAssetPackIndexInfo::StateFailed = "failed";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetPackIndexInfo::CAssetPackIndexInfo( void )
    : CORE::CIDataNodeSerializable()
    , packId()
    , indexKey()
    , packKey()
    , schemaVersion( 0 )
    , documentPath()
    , documentHash()
    , levels()
    , state()
    , stateReason()
    , stale( false )
    , resourceCount( 0 )
    , noiseCount( 0 )
    , shardCount( 0 )
    , producer()
    , producedMs( 0 )
    , loadedMs( 0 )
    , groups()
    , hasManifest( false )
    , manifest( "manifest", GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPackIndexInfo::~CAssetPackIndexInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetPackIndexInfo::Clear( void )
{GUCEF_TRACE;

    *this = CAssetPackIndexInfo();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackIndexInfo::HasLicenceRestriction( const CORE::CString& flag ) const
{GUCEF_TRACE;

    if ( !hasManifest )
        return false;
    const CORE::CDataNode* licence = manifest.FindChild( "licence" );
    if ( GUCEF_NULL == licence )
        return false;
    CORE::CString::StringVector restrictions;
    licence->GetValuesOfChildByName( "restrictions", restrictions );
    for ( size_t i=0; i<restrictions.size(); ++i )
        if ( restrictions[ i ] == flag )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackIndexInfo::Serialize( CORE::CDataNode& domRootNode                        ,
                                const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetAttribute( "indexKey", indexKey ) && success;
    success = domRootNode.SetAttribute( "packKey", packKey ) && success;
    success = domRootNode.SetAttribute( "schemaVersion", schemaVersion ) && success;
    success = domRootNode.SetAttribute( "documentPath", documentPath ) && success;
    success = domRootNode.SetAttribute( "documentHash", documentHash ) && success;
    success = domRootNode.SetValuesOfChildByName( "levels", levels ) && success;
    success = domRootNode.SetAttribute( "state", state ) && success;
    success = domRootNode.SetAttribute( "stateReason", stateReason ) && success;
    success = domRootNode.SetAttribute( "stale", stale ) && success;
    success = domRootNode.SetAttribute( "resourceCount", resourceCount ) && success;
    success = domRootNode.SetAttribute( "noiseCount", noiseCount ) && success;
    success = domRootNode.SetAttribute( "shardCount", shardCount ) && success;
    success = domRootNode.SetAttribute( "producer", producer ) && success;
    success = domRootNode.SetAttribute( "producedMs", producedMs ) && success;
    success = domRootNode.SetAttribute( "loadedMs", loadedMs ) && success;
    success = domRootNode.SetAttribute( "hasManifest", hasManifest ) && success;
    CORE::CDataNode* groupsNode = domRootNode.AddChild( "groups", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; GUCEF_NULL != groupsNode && i<groups.size(); ++i )
        success = groups[ i ].Serialize( *groupsNode->AddChild( "group", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    if ( hasManifest )
    {
        CORE::CDataNode* manifestNode = domRootNode.AddChild( "manifest", GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL != manifestNode )
        {
            manifestNode->Copy( manifest );
            manifestNode->SetName( "manifest" );
        }
        else
            success = false;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackIndexInfo::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                  const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString();
    indexKey = domRootNode.GetAttributeValueOrChildValueByName( "indexKey" ).AsString();
    packKey = domRootNode.GetAttributeValueOrChildValueByName( "packKey" ).AsString();
    schemaVersion = domRootNode.GetAttributeValueOrChildValueByName( "schemaVersion" ).AsUInt32( 0, true );
    documentPath = domRootNode.GetAttributeValueOrChildValueByName( "documentPath" ).AsString();
    documentHash = domRootNode.GetAttributeValueOrChildValueByName( "documentHash" ).AsString();
    domRootNode.GetValuesOfChildByName( "levels", levels );
    state = domRootNode.GetAttributeValueOrChildValueByName( "state" ).AsString();
    stateReason = domRootNode.GetAttributeValueOrChildValueByName( "stateReason" ).AsString();
    stale = domRootNode.GetAttributeValueOrChildValueByName( "stale" ).AsBool( false, true );
    resourceCount = domRootNode.GetAttributeValueOrChildValueByName( "resourceCount" ).AsUInt64( 0, true );
    noiseCount = domRootNode.GetAttributeValueOrChildValueByName( "noiseCount" ).AsUInt64( 0, true );
    shardCount = domRootNode.GetAttributeValueOrChildValueByName( "shardCount" ).AsUInt32( 0, true );
    producer = domRootNode.GetAttributeValueOrChildValueByName( "producer" ).AsString();
    producedMs = domRootNode.GetAttributeValueOrChildValueByName( "producedMs" ).AsInt64( 0, true );
    loadedMs = domRootNode.GetAttributeValueOrChildValueByName( "loadedMs" ).AsInt64( 0, true );
    const CORE::CDataNode* groupsNode = domRootNode.FindChild( "groups" );
    if ( GUCEF_NULL != groupsNode )
    {
        CORE::CDataNode::const_iterator i = groupsNode->ConstBegin();
        while ( i != groupsNode->ConstEnd() )
        {
            CAssetPackGroup group;
            if ( group.Deserialize( *(*i), settings ) )
                groups.push_back( group );
            ++i;
        }
    }
    const CORE::CDataNode* manifestNode = domRootNode.FindChild( "manifest" );
    hasManifest = GUCEF_NULL != manifestNode;
    if ( hasManifest )
    {
        manifest.Copy( *manifestNode );
        manifest.SetName( "manifest" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetPackIndexInfo::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetPackIndexInfo( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetPackIndexInfo::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetPackIndexInfo::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetPackIndexInfo";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetPackIndexInfo::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetPackIndexInfo";
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
