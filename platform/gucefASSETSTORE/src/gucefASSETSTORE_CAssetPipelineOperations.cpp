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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefASSETSTORE_CAssetPipelineOperations.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetPipelineOperations::PackList = "pack_list";
const CORE::CString CAssetPipelineOperations::PackGet = "pack_get";
const CORE::CString CAssetPipelineOperations::PackAssets = "pack_assets";
const CORE::CString CAssetPipelineOperations::AssetReport = "asset_report";
const CORE::CString CAssetPipelineOperations::PackActivity = "pack_activity";
const CORE::CString CAssetPipelineOperations::ExplainReimport = "asset_explain_reimport";
const CORE::CString CAssetPipelineOperations::CookOutputs = "pack_cook_outputs";
const CORE::CString CAssetPipelineOperations::PipelinePlugins = "pipeline_plugins";
const CORE::CString CAssetPipelineOperations::LicenceCheck = "licence_check";
const CORE::CString CAssetPipelineOperations::LicenceDraftRecord = "licence_draft_record";
const CORE::CString CAssetPipelineOperations::ConvertPlan = "convert_plan";
const CORE::CString CAssetPipelineOperations::ConvertPack = "convert_pack";
const CORE::CString CAssetPipelineOperations::ConvertFile = "convert_file";
const CORE::CString CAssetPipelineOperations::CookPack = "cook_pack";
const CORE::CString CAssetPipelineOperations::ValidatePack = "validate_pack";
const CORE::CString CAssetPipelineOperations::CompareRoutes = "compare_routes";
const CORE::CString CAssetPipelineOperations::ManifestPropose = "manifest_propose";
const CORE::CString CAssetPipelineOperations::ManifestCreate = "manifest_create";
const CORE::CString CAssetPipelineOperations::ManifestEdit = "manifest_edit";
const CORE::CString CAssetPipelineOperations::OverrideWrite = "override_write";
const CORE::CString CAssetPipelineOperations::OverrideRemove = "override_remove";
const CORE::CString CAssetPipelineOperations::MaterialMapSuggest = "material_map_suggest";
const CORE::CString CAssetPipelineOperations::ReadResource = "read_resource";
const CORE::CString CAssetPipelineOperations::RenderSheet = "render_sheet";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CAssetPipelineOperations::IsJob( const CORE::CString& operation )
{GUCEF_TRACE;

    return ConvertPack == operation || ConvertFile == operation || CookPack == operation || ValidatePack == operation;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipelineOperations::IsWrite( const CORE::CString& operation )
{GUCEF_TRACE;

    return ManifestCreate == operation || ManifestEdit == operation || OverrideWrite == operation || OverrideRemove == operation;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipelineOperations::IsRender( const CORE::CString& operation )
{GUCEF_TRACE;

    return RenderSheet == operation;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipelineOperations::IsKnown( const CORE::CString& operation )
{GUCEF_TRACE;

    return PackList == operation ||
           PackGet == operation ||
           PackAssets == operation ||
           AssetReport == operation ||
           PackActivity == operation ||
           ExplainReimport == operation ||
           CookOutputs == operation ||
           PipelinePlugins == operation ||
           LicenceCheck == operation ||
           LicenceDraftRecord == operation ||
           ConvertPlan == operation ||
           ConvertPack == operation ||
           ConvertFile == operation ||
           CookPack == operation ||
           ValidatePack == operation ||
           CompareRoutes == operation ||
           ManifestPropose == operation ||
           ManifestCreate == operation ||
           ManifestEdit == operation ||
           OverrideWrite == operation ||
           OverrideRemove == operation ||
           MaterialMapSuggest == operation ||
           ReadResource == operation ||
           RenderSheet == operation;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
