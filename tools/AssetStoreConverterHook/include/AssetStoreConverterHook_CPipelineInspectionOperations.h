/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEINSPECTIONOPERATIONS_H
#define ASSETSTORECONVERTERHOOK_CPIPELINEINSPECTIONOPERATIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H
#include "AssetStoreConverterHook_CPipelineOperationCall.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The read-only operations of the pipeline that inspect converter packs (section 7.1 of the asset tools design):
 *  pack_list, pack_get, pack_assets, asset_report, pack_activity, asset_explain_reimport, pack_cook_outputs and
 *  pipeline_plugins. They read the manifests, the licence records, the report, the activity log, the sidecars, the cache
 *  entries and the reference and size reports of the cooked profiles, and write nothing. Each answers in the document
 *  of the call, see the members named in the design; paths are rooted, texts from the packs are untrusted objects.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPipelineInspectionOperations
{
    public:

    static ASSETSTORE::TAssetStoreStatus PackList( CPipelineOperationCall& call );
    static ASSETSTORE::TAssetStoreStatus PackGet( CPipelineOperationCall& call );
    static ASSETSTORE::TAssetStoreStatus PackAssets( CPipelineOperationCall& call );
    static ASSETSTORE::TAssetStoreStatus AssetReport( CPipelineOperationCall& call );
    static ASSETSTORE::TAssetStoreStatus PackActivity( CPipelineOperationCall& call );
    static ASSETSTORE::TAssetStoreStatus ExplainReimport( CPipelineOperationCall& call );
    static ASSETSTORE::TAssetStoreStatus CookOutputs( CPipelineOperationCall& call );
    static ASSETSTORE::TAssetStoreStatus PipelinePlugins( CPipelineOperationCall& call );

    /** { converted, cached, approximated, failed, refused, skipped } of a report */
    static void AddCounts( const ASSETCONVERTER::CConversionReport& report, CORE::CDataNode& counts );

    /** "lastRun": { endedUtc, counts } of the last run of the pack, or null when it never ran */
    static void AddLastRun( const CConverterPack& pack, CORE::CDataNode& node );

    /** The licence member of a pack: the decision, with the details when asked */
    static void AddLicence( const CPipelineOperationCall& call, const CConverterPack& pack, bool details, CORE::CDataNode& node );

    /** Adds { uri, name, mimeType } to the "links" array of the node */
    static void AddLink( CORE::CDataNode& node, const CORE::CString& uri, const CORE::CString& name, const CORE::CString& mimeType );

    /** Differences between two settings trees as JSON pointers with the values before and after, appended to the array node */
    static void AddSettingsDifferences( const CORE::CDataNode& before, const CORE::CDataNode& after, CORE::CDataNode& list );

    /** The text of a value of a settings tree (attribute or child) for a difference: a scalar as it is, a subtree as JSON */
    static CORE::CString DescribeValue( const CORE::CDataNode* node, const CORE::CVariant* value );

    private:

    CPipelineInspectionOperations( void );      /**< not implemented: static functions only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEINSPECTIONOPERATIONS_H ? */
