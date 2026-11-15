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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#include "gucefASSETSTORE_CAssetRootedPath.h"
#define GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#endif /* GUCEF_ASSETSTORE_CASSETROOTEDPATH_H ? */

#include "gucefASSETSTORE_CAssetRenderSheetTool.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetRenderSheetTool::STarget::STarget( void )
    : label()
    , packId()
    , assetPath()
    , stage()
    , profile()
    , storeAssetId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetRenderSheetTool::CAssetRenderSheetTool( CAssetToolContext& context, const CORE::CString& name )
    : CAssetToolBase( context, name )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetRenderSheetTool::~CAssetRenderSheetTool()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetRenderSheetTool::RenderTargets( CAssetToolContext& context                  ,
                                      const TTargetVector& targets                ,
                                      UInt32 tile                                 ,
                                      COM::CMcpToolResult& result                 ,
                                      CAssetToolContext::TContentTagVector& tags  ,
                                      CORE::CDataNode& outcomes                   ,
                                      bool& supported                             )
{GUCEF_TRACE;

    outcomes.Clear();
    outcomes.SetNodeType( GUCEF_DATATYPE_ARRAY );
    supported = true;

    // the gate first: a withheld target is never handed to the renderer
    GUCEF::vector< Int32 > allowedIndex;
    CAssetPipelineRequest request;
    request.operation = CAssetPipelineOperations::RenderSheet;
    request.arguments.SetNodeType( GUCEF_DATATYPE_OBJECT );
    request.arguments.SetAttribute( "tile", tile );
    CORE::CDataNode* requestTargets = request.arguments.AddChild( "targets", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<targets.size(); ++i )
    {
        CORE::CDataNode* outcome = outcomes.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        outcome->SetAttribute( "label", targets[ i ].label.IsNULLOrEmpty() ? targets[ i ].assetPath : targets[ i ].label );
        CORE::CString reason;
        if ( !context.GetGate().IsAllowed( targets[ i ].packId, CORE::CString::Empty, targets[ i ].storeAssetId, targets[ i ].stage, reason ) )
        {
            outcome->SetAttribute( "withheld", true );
            outcome->SetAttribute( "reason", reason );
            continue;
        }
        CORE::CDataNode* target = requestTargets->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        if ( !targets[ i ].packId.IsNULLOrEmpty() )
            target->SetAttribute( "packId", targets[ i ].packId );
        if ( !targets[ i ].assetPath.IsNULLOrEmpty() )
            target->SetAttribute( "assetPath", targets[ i ].assetPath );
        if ( !targets[ i ].stage.IsNULLOrEmpty() )
            target->SetAttribute( "stage", targets[ i ].stage );
        if ( !targets[ i ].profile.IsNULLOrEmpty() )
            target->SetAttribute( "profile", targets[ i ].profile );
        if ( !targets[ i ].storeAssetId.IsNULLOrEmpty() )
            target->SetAttribute( "storeAssetId", targets[ i ].storeAssetId );
        allowedIndex.push_back( (Int32) i );
    }
    if ( allowedIndex.empty() )
        return;

    CAssetServiceDocument answer;
    const TAssetStoreStatus status = context.GetService().RenderSheet( request, answer );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        if ( ASSETSTORE_STATUS_UNSUPPORTED == status )
            supported = false;
        const CORE::CString reason = ( ASSETSTORE_STATUS_UNSUPPORTED == status ? CORE::CString( "unsupported: " ) : CORE::CString( "failed: " ) ) +
            ( answer.message.IsNULLOrEmpty() ? CORE::CString( "the service cannot render" ) : answer.message );
        for ( size_t a=0; a<allowedIndex.size(); ++a )
        {
            CORE::CDataNode* outcome = outcomes.GetChildAtIndex( (UInt32) allowedIndex[ a ] );
            if ( GUCEF_NULL != outcome )
            {
                outcome->SetAttribute( "failed", true );
                outcome->SetAttribute( "reason", reason );
            }
        }
        return;
    }

    // a picture per allowed target, in their order
    for ( size_t a=0; a<allowedIndex.size(); ++a )
    {
        const UInt32 index = (UInt32) allowedIndex[ a ];
        CORE::CDataNode* outcome = outcomes.GetChildAtIndex( index );
        if ( GUCEF_NULL == outcome )
            continue;
        if ( a >= answer.images.size() || answer.images[ a ].encoded.GetDataSize() == 0 )
        {
            outcome->SetAttribute( "failed", true );
            outcome->SetAttribute( "reason", "the renderer gave no PNG for the target" );
            continue;
        }
        const CAssetThumbnail& image = answer.images[ a ];
        if ( image.encoded.GetDataSize() > MaxImageBytes )
        {
            outcome->SetAttribute( "failed", true );
            outcome->SetAttribute( "reason", "the sheet is larger than 1 MiB" );
            continue;
        }
        result.AddText( outcome->GetAttributeValueOrChildValueByName( "label" ).AsString() );
        CAssetToolContext::SContentTag tag;
        tag.contentIndex = (UInt32) result.content.size();
        tag.converterPackId = targets[ index ].packId;
        tag.storeAssetId = targets[ index ].storeAssetId;
        tag.stage = targets[ index ].stage;
        tags.push_back( tag );
        result.AddContent( COM::CMcpContent::Image( image.encoded, "image/png" ).SetAudience( true, true ) );
        outcome->SetAttribute( "sheet", (UInt32) a );
    }
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetRenderSheetTool::Compute( COM::CMcpRequestContext& context      ,
                                const CORE::CDataNode& arguments      ,
                                COM::CMcpToolResult& result           ,
                                COM::CMcpInputRequired& inputRequired ,
                                SCallState& state                     )
{GUCEF_TRACE;

    TTargetVector targets;
    const CORE::CDataNode* list = arguments.FindChild( "targets" );
    if ( GUCEF_NULL != list )
    {
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        while ( i != list->ConstEnd() )
        {
            STarget target;
            target.packId = GetStringArgument( *(*i), "packId" );
            target.assetPath = GetStringArgument( *(*i), "assetPath" );
            target.stage = GetStringArgument( *(*i), "stage" );
            target.profile = GetStringArgument( *(*i), "profile" );
            target.storeAssetId = GetStringArgument( *(*i), "storeAssetId" );
            CORE::CString problem;
            if ( target.storeAssetId.IsNULLOrEmpty() && ( target.packId.IsNULLOrEmpty() || target.assetPath.IsNULLOrEmpty() ) )
            {
                m_context.SetError( result, "invalid-arguments", "A target needs packId and assetPath, or storeAssetId", false );
                return COM::MCP_EXECUTE_COMPLETE;
            }
            if ( !target.assetPath.IsNULLOrEmpty() && !CAssetRootedPath::IsSafeRelativePath( target.assetPath, false, true, problem ) )
            {
                m_context.SetError( result, "path-refused", "The asset path is refused: " + problem, false );
                return COM::MCP_EXECUTE_COMPLETE;
            }
            targets.push_back( target );
            ++i;
        }
    }
    const UInt32 tile = GetUInt32Argument( arguments, "tile", 256 );
    CORE::CDataNode outcomes;
    bool supported = true;
    RenderTargets( m_context, targets, tile, result, state.tags, outcomes, supported );
    CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
    document.SetAttribute( "supported", supported );
    document.SetAttribute( "tile", tile );
    CORE::CDataNode* targetList = document.AddChild( outcomes );
    targetList->SetName( "targets" );
    if ( !supported )
        document.SetAttribute( "message", "unsupported: this service has no renderer for contact sheets; the targets that may be shown are listed as failed" );
    SetStructured( result, document );
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
