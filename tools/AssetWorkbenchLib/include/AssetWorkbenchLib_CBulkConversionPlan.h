/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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

#ifndef ASSETWORKBENCHLIB_CBULKCONVERSIONPLAN_H
#define ASSETWORKBENCHLIB_CBULKCONVERSIONPLAN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#include "gucefASSETSTORE_CAssetServiceDocument.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The plan of a bulk conversion as the user sees it before it starts: what the dry run of the pipeline (the operation convert_plan)
 *  says it would do, per asset the action and its reason, the counts per action, the licence of the pack with its restrictions, the
 *  problems and the cook targets. A pack whose licence does not allow the conversion is refused with the reason: the plan has no
 *  work and cannot be started.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CBulkConversionPlan
{
    public:

    struct SItem
    {
        CORE::CString assetPath;
        CORE::CString action;       /**< import, cached, cook, refused, failed, validate, no intermediate */
        CORE::CString reason;
        CORE::CString detail;
        CORE::CString importer;
        SItem( void );
    };
    typedef GUCEF::vector< SItem > TItemVector;

    CORE::CString packId;
    bool valid;                     /**< the plan was read from an answer of the pipeline */
    bool conversionAllowed;
    CORE::CString refusalReason;
    CORE::CString::StringVector restrictions;
    CORE::CString::StringVector problems;
    CORE::CString::StringVector cookTargets;
    CORE::CString lockHolder;       /**< who holds the intermediate; the run would be refused while that lasts */
    UInt32 importCount;
    UInt32 cachedCount;
    UInt32 cookCount;
    UInt32 refusedCount;
    UInt32 failedCount;
    UInt32 validateCount;
    UInt32 noIntermediateCount;
    UInt32 filteredOutCount;
    UInt64 lastRunAssetTimeMs;      /**< what the assets of the last run took, 0 when not known: the scale of the estimate */
    UInt64 totalItems;              /**< the assets of the plan; the items below are a page of them */
    TItemVector items;

    CBulkConversionPlan( void );

    void Clear( void );

    /** Reads the answer of convert_plan; false when it is no plan */
    bool Parse( const ASSETSTORE::CAssetServiceDocument& answer );

    /** The assets that would be worked on: import, cook and validate */
    UInt32 GetWorkCount( void ) const;

    /** Whether starting the plan makes sense: allowed, not locked and with work */
    bool CanStart( void ) const;

    /** A line for the plan view: "12 to import, 3 cached, 0 refused" with the estimate, or the refusal */
    CORE::CString ToSummaryText( void ) const;

    /** The estimated work of an item in words: "import and export", "cook", "none (up to date)", "refused: <reason>" */
    static CORE::CString DescribeWork( const SItem& item );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CBULKCONVERSIONPLAN_H ? */
