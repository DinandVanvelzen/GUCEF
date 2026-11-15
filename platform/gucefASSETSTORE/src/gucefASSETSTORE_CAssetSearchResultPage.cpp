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
#include "gucefASSETSTORE_CAssetSearchResultPage.h"

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

CAssetSearchResultPage::CAssetSearchResultPage( void )
    : CORE::CIDataNodeSerializable()
    , hits()
    , total( 0 )
    , pageIndex( 0 )
    , pageSize( 0 )
    , revision( 0 )
    , facets()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSearchResultPage::~CAssetSearchResultPage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetSearchResultPage::Clear( void )
{GUCEF_TRACE;

    hits.clear();
    total = 0;
    pageIndex = 0;
    pageSize = 0;
    revision = 0;
    facets.clear();
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetSearchResultPage::GetFacetCount( const CORE::CString& facet, const CORE::CString& value ) const
{GUCEF_TRACE;

    TFacetMap::const_iterator f = facets.find( facet );
    if ( f == facets.end() )
        return 0;
    TValueCountMap::const_iterator v = (*f).second.find( value );
    return v == (*f).second.end() ? 0 : (*v).second;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchResultPage::HasHit( const CORE::CString& assetOrPackId ) const
{GUCEF_TRACE;

    for ( size_t h=0; h<hits.size(); ++h )
        if ( hits[ h ].assetId == assetOrPackId || ( hits[ h ].assetId.IsNULLOrEmpty() && hits[ h ].packId == assetOrPackId ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchResultPage::Serialize( CORE::CDataNode& domRootNode                        ,
                                   const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "total", total );
    success = domRootNode.SetAttribute( "pageIndex", pageIndex ) && success;
    success = domRootNode.SetAttribute( "pageSize", pageSize ) && success;
    success = domRootNode.SetAttribute( "revision", revision ) && success;
    CORE::CDataNode* hitsNode = domRootNode.AddChild( "hits", GUCEF_DATATYPE_ARRAY );
    for ( size_t h=0; GUCEF_NULL != hitsNode && h<hits.size(); ++h )
        success = hits[ h ].Serialize( *hitsNode->AddChild( "hit", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    CORE::CDataNode* facetsNode = domRootNode.AddChild( "facets", GUCEF_DATATYPE_OBJECT );
    TFacetMap::const_iterator f = facets.begin();
    while ( GUCEF_NULL != facetsNode && f != facets.end() )
    {
        CORE::CDataNode* facetNode = facetsNode->AddChild( (*f).first, GUCEF_DATATYPE_OBJECT );
        TValueCountMap::const_iterator v = (*f).second.begin();
        while ( v != (*f).second.end() )
        {
            facetNode->SetAttribute( (*v).first, (*v).second );
            ++v;
        }
        ++f;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSearchResultPage::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                     const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    total = domRootNode.GetAttributeValueOrChildValueByName( "total" ).AsUInt64( 0, true );
    pageIndex = domRootNode.GetAttributeValueOrChildValueByName( "pageIndex" ).AsUInt64( 0, true );
    pageSize = domRootNode.GetAttributeValueOrChildValueByName( "pageSize" ).AsUInt64( 0, true );
    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsUInt64( 0, true );
    const CORE::CDataNode* hitsNode = domRootNode.FindChild( "hits" );
    if ( GUCEF_NULL != hitsNode )
    {
        CORE::CDataNode::const_iterator i = hitsNode->ConstBegin();
        while ( i != hitsNode->ConstEnd() )
        {
            CAssetSearchHit hit;
            if ( hit.Deserialize( *(*i), settings ) )
                hits.push_back( hit );
            ++i;
        }
    }
    const CORE::CDataNode* facetsNode = domRootNode.FindChild( "facets" );
    if ( GUCEF_NULL != facetsNode )
    {
        CORE::CDataNode::const_iterator i = facetsNode->ConstBegin();
        while ( i != facetsNode->ConstEnd() )
        {
            TValueCountMap& counts = facets[ (*i)->GetName() ];
            for ( UInt32 a=0; a<(*i)->GetAttCount(); ++a )
                counts[ (*i)->GetAttribute( a )->first ] = (*i)->GetAttribute( a )->second.AsUInt64( 0, true );
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetSearchResultPage::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetSearchResultPage( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSearchResultPage::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetSearchResultPage";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSearchResultPage::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSearchResultPage::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetSearchResultPage";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
