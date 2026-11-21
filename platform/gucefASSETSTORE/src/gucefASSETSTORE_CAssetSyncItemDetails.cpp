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
#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#include "gucefASSETSTORE_CAssetStoreItem.h"
#define GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEM_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#include "gucefASSETSTORE_CAssetStoreJson.h"
#define GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREJSON_H ? */

#include "gucefASSETSTORE_CAssetSyncItemDetails.h"

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

CAssetSyncItemDetails::CAssetSyncItemDetails( void )
    : sourceId()
    , itemId()
    , entryFingerprint()
    , fetchedMs( 0 )
    , details()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetSyncItemDetails::SetItem( const CAssetStoreItem& item )
{GUCEF_TRACE;

    // what the plan takes of the details: the formats with their files and sizes, the revision, the times, the availability
    CAssetStoreItem lean( item );
    lean.description.Clear();
    lean.imageUrls.clear();
    lean.thumbnailUrl.Clear();
    lean.tags.clear();
    lean.categories.clear();
    lean.licenceText.Clear();
    lean.storeAttributes = CORE::CDataNode( "storeAttributes", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNodeSerializableSettings settings;
    CORE::CDataNode node( "item", GUCEF_DATATYPE_OBJECT );
    if ( !lean.Serialize( node, settings ) )
        return false;
    itemId = item.itemId;
    return CAssetStoreJson::ToText( node, details );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSyncItemDetails::GetItem( CAssetStoreItem& item ) const
{GUCEF_TRACE;

    item = CAssetStoreItem();
    if ( details.IsNULLOrEmpty() )
        return false;
    CORE::CDataNode node;
    if ( !CAssetStoreJson::FromText( details, node ) )
        return false;
    CORE::CDataNodeSerializableSettings settings;
    if ( !item.Deserialize( node, settings ) || item.itemId.IsNULLOrEmpty() )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSyncItemDetails::IsUsable( const CORE::CString& currentEntryFingerprint, Int64 nowMs, Int64 maxAgeMs ) const
{GUCEF_TRACE;

    if ( details.IsNULLOrEmpty() || entryFingerprint.IsNULLOrEmpty() || entryFingerprint != currentEntryFingerprint || maxAgeMs <= 0 )
        return false;
    return fetchedMs > 0 && fetchedMs <= nowMs + 60000 && nowMs - fetchedMs < maxAgeMs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
