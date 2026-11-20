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

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCITEM_H
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCITEM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
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

/**
 *  One line of the plan of a library sync (library sync design, sections 4.1 and 5.3): a listed item with its state, the
 *  reason of the state, the formats it has in the order they are wanted, the formats that are queued, the estimate of
 *  their size, the stored and the listed revision and the licence class. The state names are the constants below.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetLibrarySyncItem : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString Ignored;             /**< "ignored": a mark of the owner */
    static const CORE::CString NoId;                /**< "no-id": the listing gave no item id */
    static const CORE::CString Filtered;            /**< "filtered": a filter of the profile, the reason names it */
    static const CORE::CString Unavailable;         /**< "unavailable": the account cannot download it now */
    static const CORE::CString InFlight;            /**< "in-flight": queued, downloading or being ingested */
    static const CORE::CString Pinned;              /**< "pinned": a mark of the owner on a pack that exists */
    static const CORE::CString Current;             /**< "current": a pack of every wanted format with the listed revision */
    static const CORE::CString UnknownFreshness;    /**< "unknown-freshness": a pack exists, whether it is the listed version cannot be told */
    static const CORE::CString Stale;               /**< "stale": the store has a newer version; queued as a new pack */
    static const CORE::CString TooLarge;            /**< "too-large": larger than the size guard of the store plugin (known size, or refused by it before); not queued unless the user allows it for the item */
    static const CORE::CString FailedBefore;        /**< "failed-before": too many failed downloads, or waiting for the next attempt */
    static const CORE::CString Missing;             /**< "missing": no pack of a wanted format; queued */

    CORE::CString itemId;
    CORE::CString title;
    CORE::CString state;
    CORE::CString reason;
    CORE::CString::StringVector formats;            /**< the formats of the item, in the order they are wanted */
    CORE::CString::StringVector queuedFormats;      /**< the formats a run downloads */
    UInt64 estimatedBytes;                          /**< of the queued formats (of every format when none is queued) */
    bool sizeKnown;                                 /**< false when the estimate is not from the sizes of the store */
    CORE::CString listedRevision;                   /**< tagged (CAssetItemRevision) */
    CORE::CString storedRevision;                   /**< of the pack the item was compared with, tagged; empty without one */
    CORE::CString licenceClass;
    CORE::CString::StringVector packIds;            /**< the current packs of the item */
    Int64 changedAtMs;
    Int64 acquiredAtMs;
    UInt32 listIndex;                               /**< the place in the listing */

    CAssetLibrarySyncItem( void );
    virtual ~CAssetLibrarySyncItem() GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the item is queued by a run: missing or stale with formats to download */
    bool IsQueued( void ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef GUCEF::vector< CAssetLibrarySyncItem > TAssetLibrarySyncItemVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCITEM_H ? */
