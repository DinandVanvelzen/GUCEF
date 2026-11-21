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

#ifndef GUCEF_ASSETSTORE_CASSETSYNCMARKSTORE_H
#define GUCEF_ASSETSTORE_CASSETSYNCMARKSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKSOURCERECORD_H
#include "gucefASSETSTORE_CAssetPackSourceRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKSOURCERECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKSOURCERECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSYNCMARK_H
#include "gucefASSETSTORE_CAssetSyncMark.h"
#define GUCEF_ASSETSTORE_CASSETSYNCMARK_H
#endif /* GUCEF_ASSETSTORE_CASSETSYNCMARK_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSYNCITEMRECORD_H
#include "gucefASSETSTORE_CAssetSyncItemRecord.h"
#define GUCEF_ASSETSTORE_CASSETSYNCITEMRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETSYNCITEMRECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSYNCITEMDETAILS_H
#include "gucefASSETSTORE_CAssetSyncItemDetails.h"
#define GUCEF_ASSETSTORE_CASSETSYNCITEMDETAILS_H
#endif /* GUCEF_ASSETSTORE_CASSETSYNCITEMDETAILS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAssetCatalogue;

/**
 *  The tables of the library sync in the catalogue (schema version 5, library sync design section 3.2): the store facts of
 *  the packs (pack_sources), the marks of the owner (sync_marks) and the observations of the items (sync_items). Reads of a
 *  store are one query each, by the store id, so the planner never asks per item. On a catalogue below version 5 nothing
 *  is available: reads answer UNSUPPORTED and the sync works without facts. From schema version 6 it also keeps the cache
 *  of the details of the items (sync_item_details); below 6 that cache answers UNSUPPORTED and every plan asks the store.
 *  Thread safe as the catalogue is.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetSyncMarkStore
{
    public:

    explicit CAssetSyncMarkStore( CAssetCatalogue& catalogue );
    ~CAssetSyncMarkStore();

    /** Whether the catalogue has the tables of the library sync */
    bool IsAvailable( void ) const;

    // pack_sources

    TAssetStoreStatus WritePackSource( const CAssetPackSourceRecord& record );
    TAssetStoreStatus GetPackSource( const CORE::CString& packId, CAssetPackSourceRecord& record ) const;
    TAssetStoreStatus GetPackSourcesOfStore( const CORE::CString& sourceId, TAssetPackSourceRecordVector& records ) const;
    TAssetStoreStatus GetPackSourcesOfItem( const CORE::CString& sourceId, const CORE::CString& itemId, TAssetPackSourceRecordVector& records ) const;

    /**
     *  Marks the current packs of the item that the new pack replaces as superseded by it: those of the same format and
     *  those whose format is not known, the new pack not included. Answers how many were marked
     */
    TAssetStoreStatus SupersedeOlderPacks( const CAssetPackSourceRecord& newPack, Int64 nowMs, UInt32& superseded );

    // sync_marks

    TAssetStoreStatus SetMark( const CAssetSyncMark& mark );
    TAssetStoreStatus RemoveMark( const CORE::CString& sourceId, const CORE::CString& itemId );
    TAssetStoreStatus GetMarks( const CORE::CString& sourceId, TAssetSyncMarkVector& marks ) const;

    // sync_items

    TAssetStoreStatus WriteSyncItems( const TAssetSyncItemRecordVector& records );
    TAssetStoreStatus GetSyncItem( const CORE::CString& sourceId, const CORE::CString& itemId, CAssetSyncItemRecord& record ) const;
    TAssetStoreStatus GetSyncItems( const CORE::CString& sourceId, TAssetSyncItemRecordMap& records ) const;

    // sync_item_details (schema version 6)

    /** Whether the catalogue has the cache of the details of the items */
    bool IsDetailsCacheAvailable( void ) const;

    /** Writes (replaces) the cached details of an item */
    TAssetStoreStatus WriteItemDetails( const CAssetSyncItemDetails& details );

    /** The cached details of every item of a store: one query */
    TAssetStoreStatus GetItemDetailsOfStore( const CORE::CString& sourceId, TAssetSyncItemDetailsMap& details ) const;

    /** Removes the cached details of the items of a store that are not in 'keep' (the items of a complete listing); answers how many went */
    TAssetStoreStatus PruneItemDetails( const CORE::CString& sourceId, const CORE::CString::StringSet& keep, UInt32& removed );

    private:

    CAssetSyncMarkStore( const CAssetSyncMarkStore& src );              /**< not implemented */
    CAssetSyncMarkStore& operator=( const CAssetSyncMarkStore& src );   /**< not implemented */

    CAssetCatalogue& m_catalogue;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSYNCMARKSTORE_H ? */
