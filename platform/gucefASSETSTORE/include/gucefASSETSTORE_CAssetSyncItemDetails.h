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

#ifndef GUCEF_ASSETSTORE_CASSETSYNCITEMDETAILS_H
#define GUCEF_ASSETSTORE_CASSETSYNCITEMDETAILS_H

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

class CAssetStoreItem;

/**
 *  The details of an item of a store as GetItem() gave them to the library sync (a row of the table sync_item_details,
 *  schema version 6): the item without its description, pictures, tags and categories, as JSON text, with the fingerprint
 *  of the library entry the details were asked for (CAssetItemRevision::ComputeEntryFingerprint()) and the time they were
 *  asked. A cache: a plan reuses the details while the entry fingerprint is the same and they are younger than the profile
 *  member detailsMaxAgeDays; losing it only costs requests to the store. Times are UTC milliseconds since the Unix epoch.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetSyncItemDetails
{
    public:

    CORE::CString sourceId;
    CORE::CString itemId;
    CORE::CString entryFingerprint;         /**< of the library entry the details were asked for */
    Int64 fetchedMs;                        /**< when the store gave them */
    CORE::CString details;                  /**< the item as JSON text (CAssetStoreItem::Serialize()) */

    CAssetSyncItemDetails( void );

    /** Takes the item, leaving out what a plan does not need (description, pictures, tags, categories, store attributes) */
    bool SetItem( const CAssetStoreItem& item );

    /** The item of the details; false when the text cannot be read */
    bool GetItem( CAssetStoreItem& item ) const;

    /** Whether the details can stand for the item of the entry at the time: the same entry fingerprint and not older than maxAgeMs */
    bool IsUsable( const CORE::CString& currentEntryFingerprint, Int64 nowMs, Int64 maxAgeMs ) const;
};

typedef GUCEF::map< CORE::CString, CAssetSyncItemDetails > TAssetSyncItemDetailsMap;    /**< by item id */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSYNCITEMDETAILS_H ? */
