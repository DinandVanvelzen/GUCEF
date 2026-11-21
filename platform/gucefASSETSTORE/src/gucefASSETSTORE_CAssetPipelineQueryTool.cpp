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

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#include "gucefASSETSTORE_CAssetRootedPath.h"
#define GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#endif /* GUCEF_ASSETSTORE_CASSETROOTEDPATH_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETRENDERSHEETTOOL_H
#include "gucefASSETSTORE_CAssetRenderSheetTool.h"
#define GUCEF_ASSETSTORE_CASSETRENDERSHEETTOOL_H
#endif /* GUCEF_ASSETSTORE_CASSETRENDERSHEETTOOL_H ? */

#include "gucefASSETSTORE_CAssetPipelineQueryTool.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
CheckAssetPath( CAssetToolContext& context, const CORE::CString& assetPath, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    CORE::CString problem;
    if ( !assetPath.IsNULLOrEmpty() && !CAssetRootedPath::IsSafeRelativePath( assetPath, false, true, problem ) )
    {
        context.SetError( result, "path-refused", "The asset path is refused: " + problem, false );
        return false;
    }
    return true;
}

bool
CheckRootedPath( CAssetToolContext& context, const CORE::CString& path, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return true;
    CAssetRootedPath rooted;
    CORE::CString problem;
    if ( !rooted.Parse( path, problem ) )
    {
        context.SetError( result, "path-refused", "The path is refused: " + problem, false );
        return false;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetPipelineQueryTool::CAssetPipelineQueryTool( CAssetToolContext& context, const CORE::CString& name )
    : CAssetToolBase( context, name )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPipelineQueryTool::~CAssetPipelineQueryTool()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetPipelineQueryTool::BuildRequest( CAssetToolContext& context                ,
                                       const CORE::CString& operation            ,
                                       const CORE::CDataNode& arguments          ,
                                       const COM::CMcpPrincipal& principal       ,
                                       CAssetPipelineRequest& request            )
{GUCEF_TRACE;

    request.Clear();
    request.operation = operation;
    request.packId = GetStringArgument( arguments, "packId" );
    request.assetPath = GetStringArgument( arguments, "assetPath" );
    request.sourcePath = GetStringArgument( arguments, "sourcePath" );
    request.storeId = GetStringArgument( arguments, "storeId" );
    request.itemId = GetStringArgument( arguments, "itemId" );
    request.storeAssetId = GetStringArgument( arguments, "storeAssetId" );
    GetStringListArgument( arguments, "assets", request.assets );
    GetStringListArgument( arguments, CAssetPipelineOperations::CookPack == operation ? "targets" : "cookTargets", request.cookTargets );
    GetStringListArgument( arguments, "profiles", request.profiles );
    request.section = GetStringArgument( arguments, "section" );
    request.force = GetBoolArgument( arguments, "force", false );
    request.reimport = GetBoolArgument( arguments, "reimport", false );
    request.apply = GetBoolArgument( arguments, "apply", false );
    request.changeToken = GetStringArgument( arguments, "changeToken" );
    request.pageToken = GetStringArgument( arguments, "cursor" );
    request.maxEntries = context.GetPageLimit( GetUInt32Argument( arguments, "limit", 0 ) );
    request.arguments = arguments;
    request.arguments.SetName( "arguments" );
    request.arguments.SetNodeType( GUCEF_DATATYPE_OBJECT );
    request.arguments.SetAttribute( "principal", principal.subject );
    const CAssetToolsExposure exposure = context.GetExposure();
    request.arguments.SetAttribute( "exposePurchaseDetails", exposure.exposePurchaseDetails );
    request.arguments.SetAttribute( "maxUntrustedTextChars", context.GetUntrustedTextChars() );
    if ( CAssetPipelineOperations::IsReprocess( operation ) || CAssetPipelineOperations::ReprocessPlan == operation )
    {
        // the reprocessing of a store pack: a machine caller, whatever the arguments say; the limits of the service apply to it
        request.arguments.SetAttribute( "machine", true );
        request.arguments.SetAttribute( "caller", "mcp:" + principal.subject );
        if ( request.packId.IsNULLOrEmpty() )
            request.packId = GetStringArgument( arguments, "storePackId" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipelineQueryTool::CheckPaths( CAssetToolContext& context, const CORE::CDataNode& arguments, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    if ( !CheckAssetPath( context, GetStringArgument( arguments, "assetPath" ), result ) )
        return false;
    if ( !CheckRootedPath( context, GetStringArgument( arguments, "sourcePath" ), result ) )
        return false;
    if ( !CheckRootedPath( context, GetStringArgument( arguments, "target" ), result ) )
        return false;
    const CORE::CDataNode* routes = arguments.FindChild( "routes" );
    if ( GUCEF_NULL != routes )
    {
        CORE::CDataNode::const_iterator i = routes->ConstBegin();
        while ( i != routes->ConstEnd() )
        {
            if ( !CheckAssetPath( context, GetStringArgument( *(*i), "assetPath" ), result ) )
                return false;
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetPipelineQueryTool::SetDocumentResult( const CAssetServiceDocument& answer, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    CORE::CDataNode document( answer.document );
    document.SetName( CORE::CString::Empty );
    document.SetNodeType( GUCEF_DATATYPE_OBJECT );
    if ( !answer.nextPageToken.IsNULLOrEmpty() )
        document.SetAttribute( "nextCursor", answer.nextPageToken );
    else
    if ( GUCEF_NULL != document.FindChild( "items" ) )
        SetNull( document, "nextCursor" );
    if ( answer.truncated )
        document.SetAttribute( "truncated", true );
    SetStructured( result, document );
    AddResourceLinks( answer.document, result );
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetPipelineQueryTool::Compute( COM::CMcpRequestContext& context      ,
                                  const CORE::CDataNode& arguments      ,
                                  COM::CMcpToolResult& result           ,
                                  COM::CMcpInputRequired& inputRequired ,
                                  SCallState& state                     )
{GUCEF_TRACE;

    if ( !CheckPaths( m_context, arguments, result ) )
        return COM::MCP_EXECUTE_COMPLETE;
    if ( ( CAssetPipelineOperations::LicenceCheck == m_name || CAssetPipelineOperations::LicenceDraftRecord == m_name ) &&
         GetStringArgument( arguments, "packId" ).IsNULLOrEmpty() && GetStringArgument( arguments, "sourcePath" ).IsNULLOrEmpty() )
    {
        m_context.SetError( result, "invalid-arguments", "Give packId or sourcePath", false );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    CAssetPipelineRequest request;
    BuildRequest( m_context, m_name, arguments, context.GetPrincipal(), request );

    // the product and licence names of an item of an online store, for the draft of a licence record
    CORE::CString storeItemProblem;
    if ( CAssetPipelineOperations::LicenceDraftRecord == m_name && !request.storeId.IsNULLOrEmpty() && !request.itemId.IsNULLOrEmpty() )
    {
        CAssetStoreItem item;
        const TAssetStoreStatus itemStatus = m_context.GetService().GetStoreItem( request.storeId, request.itemId, item );
        if ( ASSETSTORE_STATUS_OK == itemStatus )
        {
            CORE::CDataNode* storeItem = request.arguments.AddChild( "storeItem", GUCEF_DATATYPE_OBJECT );
            storeItem->SetAttribute( "title", item.title );
            storeItem->SetAttribute( "seller", item.seller );
            storeItem->SetAttribute( "licenceName", item.licenceName );
            storeItem->SetAttribute( "licenceUrl", item.licenceUrl );
            CORE::CString::StringVector restrictions( item.licenceRestrictions.begin(), item.licenceRestrictions.end() );
            storeItem->SetValuesOfChildByName( "restrictions", restrictions );
        }
        else
        {
            storeItemProblem = "the item of the store is not available (" + CORE::CString( AssetStoreStatusToString( itemStatus ) ) + "): the draft has no store details";
        }
    }

    CAssetServiceDocument answer;
    const TAssetStoreStatus status = m_context.GetService().RunPipelineQuery( request, answer );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        m_context.SetServiceError( result, status, answer.message, &answer.document );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    SetDocumentResult( answer, result );
    if ( !storeItemProblem.IsNULLOrEmpty() )
        result.structuredContent.SetAttribute( "storeItemProblem", storeItemProblem );

    // contact sheets of the routes, through the gate
    if ( CAssetPipelineOperations::CompareRoutes == m_name && GetBoolArgument( arguments, "render", false ) )
    {
        CAssetRenderSheetTool::TTargetVector targets;
        const CORE::CDataNode* routes = arguments.FindChild( "routes" );
        if ( GUCEF_NULL != routes )
        {
            CORE::CDataNode::const_iterator i = routes->ConstBegin();
            while ( i != routes->ConstEnd() )
            {
                CAssetRenderSheetTool::STarget target;
                target.label = GetStringArgument( *(*i), "label" );
                target.packId = GetStringArgument( *(*i), "packId" );
                target.assetPath = GetStringArgument( *(*i), "assetPath" );
                target.stage = GetStringArgument( *(*i), "stage" );
                if ( target.stage.IsNULLOrEmpty() )
                    target.stage = "intermediate";
                target.profile = GetStringArgument( *(*i), "profile" );
                targets.push_back( target );
                ++i;
            }
        }
        CORE::CDataNode outcomes;
        bool supported = true;
        CAssetRenderSheetTool::RenderTargets( m_context, targets, 256, result, state.tags, outcomes, supported );
        CORE::CDataNode* renders = result.structuredContent.AddChild( outcomes );
        renders->SetName( "renders" );
        result.structuredContent.SetAttribute( "renderSupported", supported );
    }
    if ( CAssetPipelineOperations::MaterialMapSuggest == m_name && GetBoolArgument( arguments, "withTextures", false ) )
    {
        CAssetRenderSheetTool::TTargetVector targets;
        CAssetRenderSheetTool::STarget target;
        target.label = "textures of " + request.packId;
        target.packId = request.packId;
        target.stage = "source";
        targets.push_back( target );
        CORE::CDataNode outcomes;
        bool supported = true;
        CAssetRenderSheetTool::RenderTargets( m_context, targets, 256, result, state.tags, outcomes, supported );
        CORE::CDataNode* textures = result.structuredContent.AddChild( outcomes );
        textures->SetName( "textureSheets" );
        result.structuredContent.SetAttribute( "renderSupported", supported );
    }
    return COM::MCP_EXECUTE_COMPLETE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
