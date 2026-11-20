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

#ifndef GUCEF_ASSETSTORE_CASSETITEMREVISION_H
#define GUCEF_ASSETSTORE_CASSETITEMREVISION_H

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
#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#include "gucefASSETSTORE_CAssetStoreItem.h"
#define GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEM_H ? */

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
 *  The effective revision of a listed item (library sync design, section 2.5): one value per item, chosen by a fixed ladder
 *  of bases, tagged with its basis so that values of different bases are never compared with each other.
 *
 *  The ladder, first basis with a value wins: "ts" (changedAtMs of the item), "ver" (version), "rev" (revision), "fmt" (a
 *  SHA-256 fingerprint of the formats with their files, when the listing has file sizes), "lst" (a SHA-256 fingerprint of a
 *  fixed whitelist of the listing record) and "unk" (nothing usable). A tagged value is "<basis>:<value>", for example
 *  "ts:1731000000000" or "unk:".
 *
 *  Comparison (stored against listed): different bases are not compared; "ts" is stale only when the listed time is newer;
 *  "ver", "rev" and "fmt" are stale when the values differ; a different "lst" means that the item is to be checked further
 *  (its formats fingerprint decides), never stale by itself; "unk" is never stale. Pure functions, thread safe.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetItemRevision
{
    public:

    static const CORE::CString TimestampBasis;      /**< "ts" */
    static const CORE::CString VersionBasis;        /**< "ver" */
    static const CORE::CString RevisionBasis;       /**< "rev" */
    static const CORE::CString FormatBasis;         /**< "fmt" */
    static const CORE::CString ListingBasis;        /**< "lst" */
    static const CORE::CString UnknownBasis;        /**< "unk" */
    static const CORE::CString EntryChangeHintAttribute;   /**< "entryChangeHint": a store attribute of a library entry that changes when the item does (ComputeEntryFingerprint()) */

    enum EComparison
    {
        COMPARISON_SAME = 0         ,   /**< the stored revision is the listed one (or newer, for a time) */
        COMPARISON_STALE            ,   /**< the listed revision is a newer one: the item changed */
        COMPARISON_CHECK_FURTHER    ,   /**< the listing fingerprints differ: the formats fingerprint decides */
        COMPARISON_OTHER_BASIS      ,   /**< the two values have different bases and are not compared */
        COMPARISON_UNKNOWN              /**< one of the two is empty or of the basis "unk" */
    };
    typedef enum EComparison TComparison;

    /** The tagged effective revision of the item by the ladder */
    static CORE::CString GetEffectiveRevision( const CAssetStoreItem& item );

    /** "<basis>:<value>" */
    static CORE::CString MakeTagged( const CORE::CString& basis, const CORE::CString& value );

    /** The basis of a tagged value, empty for an empty value */
    static CORE::CString GetBasis( const CORE::CString& tagged );

    /** The value of a tagged value without its basis */
    static CORE::CString GetValue( const CORE::CString& tagged );

    /** The time of a tagged value of the basis "ts", 0 for any other */
    static Int64 GetChangedMs( const CORE::CString& tagged );

    /** The SHA-256 fingerprint (hexadecimal) over the sorted formats with their file ids, sizes and digests; empty when the listing has no file sizes */
    static CORE::CString ComputeFormatFingerprint( const CAssetStoreItem& item );

    /** The SHA-256 fingerprint (hexadecimal) over the item id, title, version, the sorted format ids and the listing type; never images, addresses or counters */
    static CORE::CString ComputeListingFingerprint( const CAssetStoreItem& item );

    /**
     *  The SHA-256 fingerprint (hexadecimal) of what the library listing says of an item cheaply: the id, title, version, listing
     *  type, revision token, change and acquisition times, whether it can be downloaded, the sorted format ids with their
     *  names and file counts, and the store attribute EntryChangeHintAttribute a plugin sets from what its entries say that
     *  changes with the item. The details of the item asked of the store are reused while it stays the same
     */
    static CORE::CString ComputeEntryFingerprint( const CAssetStoreItem& item );

    /** Compares a stored revision with a listed one */
    static TComparison Compare( const CORE::CString& stored, const CORE::CString& listed );

    /** The name of a comparison: "same", "stale", "check-further", "other-basis", "unknown" */
    static const char* ComparisonToString( TComparison comparison );

    private:

    CAssetItemRevision( void );     /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETITEMREVISION_H ? */
