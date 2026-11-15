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
#include "gucefASSETSTORE_CAssetSavedSearch.h"

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

CAssetSavedSearch::CAssetSavedSearch( void )
    : CORE::CIDataNodeSerializable()
    , searchId()
    , name()
    , query()
    , precanned( false )
    , sortOrder( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSavedSearch::~CAssetSavedSearch()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetSavedSearch::Clear( void )
{GUCEF_TRACE;

    *this = CAssetSavedSearch();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSavedSearch::Serialize( CORE::CDataNode& domRootNode                        ,
                              const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "searchId", searchId ) && success;
    success = domRootNode.SetAttribute( "name", name ) && success;
    CORE::CDataNode* queryNode = domRootNode.AddChild( "query", GUCEF_DATATYPE_OBJECT );
    success = GUCEF_NULL != queryNode && query.Serialize( *queryNode, settings ) && success;
    success = domRootNode.SetAttribute( "precanned", precanned ) && success;
    success = domRootNode.SetAttribute( "sortOrder", sortOrder ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSavedSearch::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    searchId = domRootNode.GetAttributeValueOrChildValueByName( "searchId" ).AsString( searchId );
    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( name );
    const CORE::CDataNode* queryNode = domRootNode.FindChild( "query" );
    if ( GUCEF_NULL != queryNode )
        query.Deserialize( *queryNode, settings );
    precanned = domRootNode.GetAttributeValueOrChildValueByName( "precanned" ).AsBool( precanned, true );
    sortOrder = domRootNode.GetAttributeValueOrChildValueByName( "sortOrder" ).AsInt64( sortOrder, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetSavedSearch::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetSavedSearch( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSavedSearch::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSavedSearch::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetSavedSearch";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSavedSearch::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetSavedSearch";
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
