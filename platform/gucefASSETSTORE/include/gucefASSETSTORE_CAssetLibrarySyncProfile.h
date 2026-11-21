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

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPROFILE_H
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPROFILE_H

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
#ifndef GUCEF_ASSETSTORE_CASSETFORMATPOLICY_H
#include "gucefASSETSTORE_CAssetFormatPolicy.h"
#define GUCEF_ASSETSTORE_CASSETFORMATPOLICY_H
#endif /* GUCEF_ASSETSTORE_CASSETFORMATPOLICY_H ? */

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
 *  The settings of the library sync of one store (library sync design, section 6.2): the filters, the order, what a
 *  changed item does, the admission rules of section 5.4 and the retries. Every limit takes 0 for "no maximum" where it
 *  is not a safety rule. The service keeps a profile per store (a later step); a sync job takes the profile of its spec.
 *
 *  Decisions of the owner applied: every format of an item is downloaded (the format policy only orders them), a changed
 *  item is kept as a new pack with the old one superseded (onStale "keep"), an ordered sync runs without an approval step.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetLibrarySyncProfile : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString OnStaleKeep;         /**< "keep": a new pack, the old one superseded, both kept */
    static const CORE::CString OnStaleReport;       /**< "report": counted and listed, nothing queued */
    static const CORE::CString OrderNewest;         /**< "newest" */
    static const CORE::CString OrderOldest;         /**< "oldest" */
    static const CORE::CString OrderSmallest;       /**< "smallest" */
    static const CORE::CString OrderStore;          /**< "store": the order of the listing */

    bool enabled;                                   /**< the store takes part in scheduled syncs (a sync that is ordered runs regardless) */
    CORE::CString schedule;                         /**< when the scheduled sync runs, a cron expression in UTC; empty: no scheduled sync */
    CAssetFormatPolicy formats;                     /**< the order of the formats of an item; with onlyPreferred the others are left out */
    CORE::CString::StringSet listingTypes;          /**< the listing types that are synced, empty for all */
    CORE::CString::StringSet licenceClasses;        /**< the licence classes that are synced, empty for all ("unknown" included) */
    UInt64 maxItemBytes;                            /**< items above are filtered, 0: no maximum */
    UInt64 minItemBytes;                            /**< items below are filtered, 0: no minimum */
    bool skipUnknownSize;                           /**< items whose size is not known are filtered */
    CORE::CString onStale;                          /**< "keep" or "report" */
    CORE::CString order;                            /**< "newest", "oldest", "smallest" or "store" */
    bool adoptExisting;                             /**< packs from before the facts were kept get them from the first run that sees them */
    UInt32 maxSyncAttempts;                         /**< failed downloads of an item before it is left out until a retry of the failed ones */
    UInt32 retryBackoffMinutes;                     /**< the first wait after a failure, doubling up to 24 hours */
    UInt32 maxInFlightPerStore;                     /**< items queued and not ingested at a time; 0: max( 4, 2 x the downloads per store of the manager ) */
    UInt32 reservePercent;                          /**< free space kept on the volume of the location, in percent of its size */
    UInt64 reserveBytes;                            /**< free space kept on the volume of the location; the larger of the two applies */
    Float64 diskFactor;                             /**< the bytes of an item on disk while it is staged and converted, as a factor of its size */
    UInt64 unknownSizeEstimateBytes;                /**< the estimate of an item whose size is not known when the store has no known sizes */
    UInt32 maxItemsPerHour;                         /**< 0: no maximum */
    UInt32 maxItemsPerDay;                          /**< 0: no maximum */
    UInt64 maxBytesPerDay;                          /**< 0: no maximum */
    UInt64 maxStagedBytes;                          /**< downloaded and not ingested, 0: no maximum */
    UInt32 maxPendingIngests;                       /**< downloaded and not ingested, 0: no maximum */
    UInt32 enrichInFlight;                          /**< GetItem calls of the store at a time when the listing is not enough */
    bool indexAfter;                                /**< the run ends with a job index-pack over the packs without an index */
    UInt32 planMaxAgeMinutes;                       /**< a run uses the stored plan of the store (the last "plan") when it is younger than this and made with the same plan members; 0: a run always lists */
    UInt32 detailsMaxAgeDays;                       /**< the details of an item asked of the store are reused by later plans for this many days while its library entry is unchanged; 0: always asked */

    CAssetLibrarySyncProfile( void );
    virtual ~CAssetLibrarySyncProfile() GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the values can be used; the reason when not */
    bool IsValid( CORE::CString& reason ) const;

    /**
     *  A digest of the members that shape a plan: the formats, the filters (listing types, licence classes, sizes), onStale and
     *  adoptExisting. A stored plan is reused only by a run with the same fingerprint; the caps, the window, the order, the
     *  schedule and the two ages are not in it
     */
    CORE::CString GetPlanFingerprint( void ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Members that the node does not have keep their value */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Type check of the result of a future without RTTI (CICloneable::CloneableAs()) */
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPROFILE_H ? */
