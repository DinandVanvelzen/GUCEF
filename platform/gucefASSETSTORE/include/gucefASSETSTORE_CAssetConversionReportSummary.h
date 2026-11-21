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

#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONREPORTSUMMARY_H
#define GUCEF_ASSETSTORE_CASSETCONVERSIONREPORTSUMMARY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
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
 *  What a report of the converter ("report.json" of the intermediate folder of a pack, written by the pipeline of the converter
 *  through the converter hook) says per asset, read as a generic JSON tree: the member "assets" is an array of objects with "asset"
 *  (the path of the asset in the pack) and "status" (converted, approximated, skipped, failed, cached, refused); the member "counts"
 *  has the counts per status. A report without the list of assets gives the counts only. The store does not depend on the
 *  converter: this is the one place that reads its reports.
 *
 *  The regeneration of the outputs of a pack (CAssetPackRegeneration) compares the summary before and after a run (Compare()):
 *  which assets now convert (improved), which no longer do as well (regressed), which still fail; and a run of a part of a pack
 *  merges its report into that of the pack (MergeReportFiles()), so the report keeps describing every asset.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetConversionReportSummary
{
    public:

    typedef GUCEF::map< CORE::CString, CORE::CString > TStatusMap;     /**< the status per asset path */
    typedef GUCEF::map< CORE::CString, UInt32 > TCountMap;              /**< the count per status */

    bool found;                         /**< a report was read */
    CORE::CString reportPath;           /**< the VFS path it was read from */
    TStatusMap assets;
    TCountMap counts;                   /**< from the assets, or from the member "counts" of a report without assets */

    CAssetConversionReportSummary( void );

    void Clear( void );

    /** Reads a report; false when there is none or it is no JSON object (found is false then) */
    bool Load( const CORE::CString& reportVfsPath );

    /** The count of a status ("converted", ...) */
    UInt32 GetCount( const CORE::CString& status ) const;

    /** The count of every status together */
    UInt32 GetTotal( void ) const;

    /** The assets of 'newer' replace those of this summary; the counts are made again */
    void Merge( const CAssetConversionReportSummary& newer );

    /** The paths of the assets whose status is one of 'statuses' */
    void GetAssetsWithStatus( const CORE::CString::StringSet& statuses, CORE::CString::StringVector& paths ) const;

    /** The counts as members of the node ("converted", "approximated", "skipped", "failed", "cached", "refused", "total"), and "found" */
    void SerializeCounts( CORE::CDataNode& node ) const;

    /** How good a status is: converted and cached 3, approximated 2, skipped 1, failed and refused 0, none (not in the report) -1 */
    static Int32 Rank( const CORE::CString& status );

    /**
     *  The changes from 'before' to 'after' as children of 'changes': arrays "improved" (an asset that converts better than it did, or
     *  is new and converts), "regressed" (worse than it was, or gone), "stillFailing" (failed or refused in both), each an array of
     *  { "asset", "before", "after" } of at most maxListed entries, with the full counts as members "improvedCount",
     *  "regressedCount", "stillFailingCount", "unchangedCount". The counts are also answered.
     */
    static void Compare( const CAssetConversionReportSummary& before, const CAssetConversionReportSummary& after, UInt32 maxListed,
                         CORE::CDataNode& changes, UInt32& improved, UInt32& regressed );

    /**
     *  Writes to targetVfsPath the report of olderVfsPath in which the entries of the assets that the report of newerVfsPath has are
     *  replaced by those (a run of a part of the pack), with the counts made again; the other members are those of the newer
     *  report. Without an older report the newer one is written as it is. False when the newer report cannot be read or the
     *  target cannot be written
     */
    static bool MergeReportFiles( const CORE::CString& olderVfsPath, const CORE::CString& newerVfsPath, const CORE::CString& targetVfsPath );

    /** The names of the statuses of a report, in the order of the converter; 'count' is set */
    static const char* const* GetStatusNames( UInt32& count );

    private:

    void CountAssets( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONREPORTSUMMARY_H ? */
