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

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H

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
#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCITEM_H
#include "gucefASSETSTORE_CAssetLibrarySyncItem.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCITEM_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCITEM_H ? */

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
 *  The plan of a library sync (library sync design, section 5.3): the counts per state and reason over the listing, the
 *  estimate of the bytes it downloads, the verdict on the storage and the lines of the items. It is never kept as the
 *  truth: the next plan gives the same decisions for the same listing, packs and marks. Serialize() writes the counts and
 *  the facts of the listing; the lines are given in pages by the result of the job (GetJobResult(), section "items").
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetLibrarySyncPlan : public CORE::CIDataNodeSerializable
{
    public:

    typedef GUCEF::map< CORE::CString, UInt32 > TCountMap;

    CORE::CString storeId;
    CORE::CString mode;
    UInt32 listed;                                  /**< distinct items of the listing */
    UInt32 pages;                                   /**< pages of the listing read */
    bool listingComplete;                           /**< every page was read, the last one ended the listing */
    TCountMap counts;                               /**< per state name of CAssetLibrarySyncItem, "removed-upstream", "needs-licence-review", "revalidated" */
    TCountMap skipped;                              /**< per reason of the items that are not queued and not current */
    UInt32 queuedItems;                             /**< items a run downloads */
    UInt32 queuedDownloads;                         /**< their formats: one download each */
    UInt64 estimatedBytes;                          /**< of the queued formats */
    UInt32 sizeUnknownItems;                        /**< queued items whose size is an estimate */
    UInt64 storageFreeBytes;                        /**< of the volume of the location, 0 when not known */
    UInt64 storageTotalBytes;
    UInt64 storageReserveBytes;
    CORE::CString storageVerdict;                   /**< "fits", "does-not-fit" or "unknown" */
    TAssetLibrarySyncItemVector items;

    CAssetLibrarySyncPlan( void );
    virtual ~CAssetLibrarySyncPlan() GUCEF_VIRTUAL_OVERRIDE;

    /** Every member back to its default */
    void Clear( void );

    /** The count of a name, 0 when there is none */
    UInt32 GetCount( const CORE::CString& name ) const;

    void AddCount( const CORE::CString& name, UInt32 amount = 1 );
    void AddSkipped( const CORE::CString& reason );

    /** The counts in words: "3 missing, 2 current" */
    CORE::CString Describe( void ) const;

    /** The counts, the listing and the storage; not the lines of the items */
    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H ? */
