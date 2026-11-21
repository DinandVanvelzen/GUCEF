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

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H

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

/**
 *  The names of the operations of the converter pipeline and of the render path that the asset store service hands to
 *  the converter hook (CIAssetConverterHook::RunPipelineOperation()) and to the render hook, as the 'operation' of a
 *  CAssetPipelineRequest. The names are those of the tools of the asset tool set that use them. The service starts the
 *  operations that are jobs as jobs of its own, previews a write unless the request asks to apply it, and hands the
 *  operations that render to the render hook.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetPipelineOperations
{
    public:

    static const CORE::CString PackList;             /**< "pack_list" */
    static const CORE::CString PackGet;              /**< "pack_get" */
    static const CORE::CString PackAssets;           /**< "pack_assets" */
    static const CORE::CString AssetReport;          /**< "asset_report" */
    static const CORE::CString PackActivity;         /**< "pack_activity" */
    static const CORE::CString ExplainReimport;      /**< "asset_explain_reimport" */
    static const CORE::CString CookOutputs;          /**< "pack_cook_outputs" */
    static const CORE::CString PipelinePlugins;      /**< "pipeline_plugins" */
    static const CORE::CString LicenceCheck;         /**< "licence_check" */
    static const CORE::CString LicenceDraftRecord;   /**< "licence_draft_record" */
    static const CORE::CString ConvertPlan;          /**< "convert_plan" */
    static const CORE::CString ConvertPack;          /**< "convert_pack": a job */
    static const CORE::CString ConvertFile;          /**< "convert_file": a job */
    static const CORE::CString CookPack;             /**< "cook_pack": a job */
    static const CORE::CString ValidatePack;         /**< "validate_pack": a job */
    static const CORE::CString IndexPack;            /**< "index-pack": a job of the service itself (not of the converter): loads or builds the pack index of a pack, or of every pack with the argument "fromStorage" or "missing" */
    static const CORE::CString SyncLibrary;          /**< "sync-library": a job of the service itself (not of the converter): the library sync of a store, its spec (CAssetLibrarySyncSpec) in the arguments */
    static const CORE::CString CompareRoutes;        /**< "compare_routes" */
    static const CORE::CString ManifestPropose;      /**< "manifest_propose" */
    static const CORE::CString ManifestCreate;       /**< "manifest_create": a write */
    static const CORE::CString ManifestEdit;         /**< "manifest_edit": a write */
    static const CORE::CString OverrideWrite;        /**< "override_write": a write */
    static const CORE::CString OverrideRemove;       /**< "override_remove": a write */
    static const CORE::CString MaterialMapSuggest;   /**< "material_map_suggest" */
    static const CORE::CString ReadResource;         /**< "read_resource": the argument "uri" names the resource */
    static const CORE::CString RenderSheet;          /**< "render_sheet": carried out by the render hook */

    /** Whether the operation runs as a job: convert_pack, convert_file, cook_pack, validate_pack, index-pack and sync-library (the last two are carried out by the service itself) */
    static bool IsJob( const CORE::CString& operation );

    /** Whether the operation is a job that the service carries out itself, not the converter: index-pack, sync-library */
    static bool IsServiceJob( const CORE::CString& operation );

    /** Whether the operation writes (previewed unless applied): manifest_create, manifest_edit, override_write, override_remove */
    static bool IsWrite( const CORE::CString& operation );

    /** Whether the operation renders: render_sheet */
    static bool IsRender( const CORE::CString& operation );

    /** Whether the name is one of the operations */
    static bool IsKnown( const CORE::CString& operation );

    private:

    CAssetPipelineOperations( void );                                       /**< not implemented: names only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */
