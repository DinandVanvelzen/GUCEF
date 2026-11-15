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

#include "gucefASSETSTORE_CAssetStoreLibraryPage.h"

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

CAssetStoreLibraryPage::CAssetStoreLibraryPage( void )
    : CORE::CIDataNodeSerializable()
    , items()
    , nextCursor()
    , hasTotal( false )
    , total( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreLibraryPage::~CAssetStoreLibraryPage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryPage::Clear( void )
{GUCEF_TRACE;

    items.clear();
    nextCursor.Clear();
    hasTotal = false;
    total = 0;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreLibraryPage::Serialize( CORE::CDataNode& domRootNode                        ,
                                   const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "nextCursor", nextCursor );
    if ( hasTotal )
        success = domRootNode.SetAttribute( "total", total ) && success;
    CORE::CDataNode* itemsNode = domRootNode.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == itemsNode )
        return false;
    for ( size_t i=0; i<items.size(); ++i )
    {
        CORE::CDataNode* itemNode = itemsNode->AddChild( "item", GUCEF_DATATYPE_OBJECT );
        success = GUCEF_NULL != itemNode && items[ i ].Serialize( *itemNode, settings ) && success;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreLibraryPage::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                     const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    nextCursor = domRootNode.GetAttributeValueOrChildValueByName( "nextCursor" ).AsString();
    const CORE::CVariant& totalValue = domRootNode.GetAttributeValueOrChildValueByName( "total" );
    hasTotal = totalValue.IsInitialized() && !totalValue.IsNULLOrEmpty();
    total = hasTotal ? totalValue.AsUInt64( 0, true ) : 0;
    const CORE::CDataNode* itemsNode = domRootNode.FindChild( "items" );
    if ( GUCEF_NULL != itemsNode )
    {
        CORE::CDataNode::const_iterator i = itemsNode->ConstBegin();
        while ( i != itemsNode->ConstEnd() )
        {
            CAssetStoreItem item;
            if ( item.Deserialize( *(*i), settings ) )
                items.push_back( item );
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStoreLibraryPage::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetStoreLibraryPage( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreLibraryPage::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreLibraryPage";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreLibraryPage::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreLibraryPage::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetStoreLibraryPage";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
