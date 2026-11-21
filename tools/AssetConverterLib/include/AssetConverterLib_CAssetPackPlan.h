/*
 *  AssetConverterLib: the plan of a run: what it would convert, keep, cook or refuse, and why, without writing anything
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

#ifndef ASSETCONVERTERLIB_CASSETPACKPLAN_H
#define ASSETCONVERTERLIB_CASSETPACKPLAN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETPLANENTRY_H
#include "AssetConverterLib_CAssetPlanEntry.h"
#define ASSETCONVERTERLIB_CASSETPLANENTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLANENTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The plan of a run of a pack (CAssetPipeline::PlanPack()), a dry run: the licence decision with the restriction
 *  flags of the record, the mode, the asset filter and the cook targets of the run, and per asset what the run would
 *  do and why, with the effective settings and the importer. Nothing is written to make it. A run with the same
 *  manifest and options that follows (with nothing changed in between) does for every asset what the plan says, and
 *  records the same reason in its activity log.
 *
 *  Saved as JSON ("<intermediate>/plan.json" by convention, or where the caller wants it).
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetPackPlan
{
    public:

    typedef GUCEF::vector< CAssetPlanEntry > TEntryVector;

    CORE::CString packId;
    TPipelineMode mode;
    CORE::CString licenceRecordId;
    bool conversionAllowed;
    CORE::CString licenceReason;                    /**< why the conversion is refused; empty when it is allowed */
    CORE::CString::StringSet restrictions;          /**< the restriction flags of the licence record (CLicenceRecord::restrictions) */
    CORE::CString::StringVector assetFilter;
    CORE::CString::StringVector cookTargets;
    CORE::CString intermediateDir;
    CORE::CString lockHolder;                       /**< who holds the lock of the intermediate now; empty when nobody */
    CORE::CString indexNote;                        /**< what the pack index would do and why (the decision of the work gate); empty when the index is not asked for */
    UInt32 filteredOut;                             /**< assets of the pack the asset filter leaves out of this run */
    CORE::CString::StringVector problems;           /**< what keeps the run from starting (no importer, no cooker, ...) */
    TEntryVector entries;

    CAssetPackPlan( void );

    /** The number of assets with the action */
    UInt32 GetCount( TPlanAction action ) const;

    const CAssetPlanEntry* FindEntry( const CORE::CString& assetPath ) const;

    /** Whether the run could start: allowed, without problems */
    bool IsRunnable( void ) const;

    void Clear( void );

    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );

    bool SaveJson( const CORE::CString& vfsPath ) const;
    bool LoadJson( const CORE::CString& vfsPath );

    /** A readable summary: the pack, the decision, the counts per action, and per asset that is not cached the action and the reason */
    CORE::CString ToSummaryText( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETPACKPLAN_H ? */
