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

#ifndef GUCEF_COM_CMCPSCHEMABUILDER_H
#include "gucefCOM_CMcpSchemaBuilder.h"
#define GUCEF_COM_CMCPSCHEMABUILDER_H
#endif /* GUCEF_COM_CMCPSCHEMABUILDER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEQUERYTOOL_H
#include "gucefASSETSTORE_CAssetPipelineQueryTool.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEQUERYTOOL_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEQUERYTOOL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#include "gucefASSETSTORE_CAssetToolUris.h"
#define GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLURIS_H ? */

#include "gucefASSETSTORE_CAssetPipelineChangeTool.h"

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

const CORE::CString CAssetPipelineChangeTool::ConfirmationKey = "confirm";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetPipelineChangeTool::CAssetPipelineChangeTool( CAssetToolContext& context, const CORE::CString& name )
    : CAssetToolBase( context, name )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPipelineChangeTool::~CAssetPipelineChangeTool()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetPipelineChangeTool::Confirm( CAssetToolContext& context              ,
                                   COM::CMcpRequestContext& request        ,
                                   const CORE::CString& toolName           ,
                                   const CORE::CString& what               ,
                                   COM::CMcpToolResult& result             ,
                                   COM::CMcpInputRequired& inputRequired   ,
                                   bool& confirmed                         )
{GUCEF_TRACE;

    confirmed = false;
    const CORE::CString& mode = context.GetSettings().confirmWrites;
    if ( CAssetToolSetSettings::ConfirmNever == mode )
    {
        confirmed = true;
        return COM::MCP_EXECUTE_COMPLETE;
    }

    // the retry with the answer of the form
    if ( request.HasInputResponses() && request.HasRequestState() )
    {
        if ( !context.UseConfirmationNonce( request.GetStateNonce() ) )
        {
            context.SetError( result, "not-confirmed", "This confirmation was used already: apply again to be asked again", false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        if ( toolName != request.GetRequestState().GetAttributeValueOrChildValueByName( "tool" ).AsString( CORE::CString::Empty, true ) )
        {
            context.SetError( result, "not-confirmed", "The confirmation belongs to another tool", false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        const CORE::CDataNode* answer = request.GetInputResponses().FindChild( ConfirmationKey );
        const CORE::CString action = GUCEF_NULL != answer ? answer->GetAttributeValueOrChildValueByName( "action" ).AsString( CORE::CString::Empty, true ) : CORE::CString::Empty;
        const CORE::CDataNode* content = GUCEF_NULL != answer ? answer->FindChild( "content" ) : GUCEF_NULL;
        const bool yes = GUCEF_NULL != content && content->GetAttributeValueOrChildValueByName( ConfirmationKey ).AsBool( false, true );
        if ( "accept" != action || !yes )
        {
            context.SetError( result, "not-confirmed", "accept" == action ? CORE::CString( "The owner did not confirm the change: nothing was written" ) :
                ( "decline" == action ? CORE::CString( "The owner declined the change: nothing was written" ) : CORE::CString( "The confirmation was cancelled: nothing was written" ) ), false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        confirmed = true;
        return COM::MCP_EXECUTE_COMPLETE;
    }

    if ( request.HasClientCapability( "elicitation/form" ) )
    {
        CORE::CDataNode schema = COM::CMcpSchemaBuilder::Object();
        CORE::CDataNode confirm = COM::CMcpSchemaBuilder::Boolean( "Write the change" );
        confirm.SetAttribute( "title", "Apply" );
        COM::CMcpSchemaBuilder::AddProperty( schema, ConfirmationKey, confirm, true );
        inputRequired.AddFormElicitation( ConfirmationKey, what, schema );
        CORE::CDataNode state( GUCEF_DATATYPE_OBJECT );
        state.SetAttribute( "tool", toolName );
        inputRequired.SetState( state );
        inputRequired.SetStateNonce();
        return COM::MCP_EXECUTE_INPUT_REQUIRED;
    }
    if ( CAssetToolSetSettings::ConfirmAlways == mode )
    {
        context.SetError( result, "not-confirmed", "This server confirms every write with a form, which this client cannot show: nothing was written", false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    confirmed = true;
    return COM::MCP_EXECUTE_COMPLETE;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetPipelineChangeTool::Compute( COM::CMcpRequestContext& context      ,
                                   const CORE::CDataNode& arguments      ,
                                   COM::CMcpToolResult& result           ,
                                   COM::CMcpInputRequired& inputRequired ,
                                   SCallState& state                     )
{GUCEF_TRACE;

    if ( !CAssetPipelineQueryTool::CheckPaths( m_context, arguments, result ) )
        return COM::MCP_EXECUTE_COMPLETE;
    CAssetPipelineRequest request;
    CAssetPipelineQueryTool::BuildRequest( m_context, m_name, arguments, context.GetPrincipal(), request );
    const CORE::CString& subject = context.GetPrincipal().subject;

    if ( !request.apply )
    {
        CAssetServiceDocument answer;
        const TAssetStoreStatus status = m_context.GetService().RunPipelineQuery( request, answer );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            m_context.SetServiceError( result, status, answer.message, &answer.document );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        CAssetPipelineQueryTool::SetDocumentResult( answer, result );
        if ( !answer.changeToken.IsNULLOrEmpty() )
            result.structuredContent.SetAttribute( "changeToken", answer.changeToken );
        result.structuredContent.SetAttribute( "written", false );
        result.structuredContent.SetAttribute( "hint", "Nothing is written yet. Show the diff to the owner; apply with apply true and this changeToken, with the same arguments." );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    if ( request.changeToken.IsNULLOrEmpty() )
    {
        m_context.SetError( result, "change-token-invalid", "An apply needs the changeToken of a preview: call without apply first", false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    const CORE::CString requestKey = GetStringArgument( arguments, "requestKey" );
    const CORE::CString digest = CAssetToolContext::DigestArguments( arguments );
    if ( !requestKey.IsNULLOrEmpty() )
    {
        CORE::CDataNode outcome;
        const CORE::TCommandIdempotencyLookup found = m_context.FindWrite( subject, m_name, requestKey, digest, outcome );
        if ( CORE::COMMANDIDEMPOTENCY_HIT == found )
        {
            SetStructured( result, outcome );
            result.structuredContent.SetAttribute( "replayed", true );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        if ( CORE::COMMANDIDEMPOTENCY_CONFLICT == found )
        {
            m_context.SetError( result, "invalid-arguments", "The requestKey was used before with other arguments: use a new key", false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
    }

    bool confirmed = false;
    const CORE::CString what = "Write the change of " + m_name + " to " + ( request.packId.IsNULLOrEmpty() ? GetStringArgument( arguments, "target" ) : request.packId ) +
        ( request.assetPath.IsNULLOrEmpty() ? CORE::CString::Empty : " / " + request.assetPath ) + "?";
    const COM::EMcpExecuteResult confirmation = Confirm( m_context, context, m_name, what, result, inputRequired, confirmed );
    if ( !confirmed )
        return confirmation;

    CORE::CString nextAllowed;
    if ( !m_context.TakeWriteQuota( subject, nextAllowed ) )
    {
        m_context.SetError( result, "busy", "The writes of this hour are used up; the next one is allowed at " + nextAllowed, true );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    CAssetServiceDocument answer;
    const CORE::CString commandId = CAssetToolContext::MakeCommandId( subject, m_name, requestKey );
    const TAssetStoreStatus status = m_context.GetService().ApplyPipelineChange( commandId, request, answer );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        m_context.SetServiceError( result, status, answer.message, &answer.document );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    CAssetPipelineQueryTool::SetDocumentResult( answer, result );
    if ( GUCEF_NULL == result.structuredContent.GetAttribute( "written" ) )
        result.structuredContent.SetAttribute( "written", true );
    if ( !requestKey.IsNULLOrEmpty() )
        m_context.StoreWrite( subject, m_name, requestKey, digest, result.structuredContent );

    // the resources that changed
    if ( !request.packId.IsNULLOrEmpty() )
    {
        if ( CAssetPipelineOperations::ManifestEdit == m_name )
            m_context.NotifyResourceUpdated( CAssetToolUris::ForPack( request.packId, "manifest" ) );
        else
            m_context.NotifyResourceUpdated( CAssetToolUris::ForAsset( request.packId, request.assetPath, "override" ) );
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
