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

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEQUERYTOOL_H
#define GUCEF_ASSETSTORE_CASSETPIPELINEQUERYTOOL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMCPPRINCIPAL_H
#include "gucefCOM_CMcpPrincipal.h"
#define GUCEF_COM_CMCPPRINCIPAL_H
#endif /* GUCEF_COM_CMCPPRINCIPAL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#include "gucefASSETSTORE_CAssetPipelineRequest.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLBASE_H
#include "gucefASSETSTORE_CAssetToolBase.h"
#define GUCEF_ASSETSTORE_CASSETTOOLBASE_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLBASE_H ? */

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
 *  The read-only operations of the converter pipeline as tools: pack_list, pack_get, pack_assets, asset_report,
 *  pack_activity, asset_explain_reimport, pack_cook_outputs, pipeline_plugins, licence_check, licence_draft_record,
 *  convert_plan, compare_routes, manifest_propose, material_map_suggest. Each is CIAssetStoreService::RunPipelineQuery()
 *  with the operation of the same name (CAssetPipelineOperations), carried out by the converter hook of the service.
 *
 *  BuildRequest() is the one mapping of tool arguments onto a CAssetPipelineRequest, shared with the change and job
 *  tools: the members the request has (packId, assetPath, sourcePath, storeId, itemId, storeAssetId, assets,
 *  cookTargets, profiles, section, force, reimport; cursor and limit as the page), and all arguments in
 *  request.arguments with "principal" (the subject: change tokens are bound to it), "exposePurchaseDetails" and
 *  "maxUntrustedTextChars" of the exposure in force.
 *
 *  Beyond the pass-through: paths are checked at this layer too (rooted path syntax, safe asset paths), so a bad path
 *  never reaches the service; licence_draft_record copies the item of an online store (storeId, itemId) into
 *  arguments.storeItem; compare_routes with render and material_map_suggest with withTextures render through the
 *  content gate (CAssetRenderSheetTool::RenderTargets()).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetPipelineQueryTool : public CAssetToolBase
{
    public:

    CAssetPipelineQueryTool( CAssetToolContext& context, const CORE::CString& name );

    virtual ~CAssetPipelineQueryTool() GUCEF_VIRTUAL_OVERRIDE;

    static void BuildRequest( CAssetToolContext& context                ,
                              const CORE::CString& operation            ,
                              const CORE::CDataNode& arguments          ,
                              const COM::CMcpPrincipal& principal       ,
                              CAssetPipelineRequest& request            );

    /** The path checks of this layer; false with an error result (path-refused, invalid-arguments) */
    static bool CheckPaths( CAssetToolContext& context, const CORE::CDataNode& arguments, COM::CMcpToolResult& result );

    /** The document of a service answer as the structured result, with nextCursor and the resource links */
    static void SetDocumentResult( const CAssetServiceDocument& answer, COM::CMcpToolResult& result );

    protected:

    virtual COM::EMcpExecuteResult Compute( COM::CMcpRequestContext& context      ,
                                            const CORE::CDataNode& arguments      ,
                                            COM::CMcpToolResult& result           ,
                                            COM::CMcpInputRequired& inputRequired ,
                                            SCallState& state                     ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEQUERYTOOL_H ? */
