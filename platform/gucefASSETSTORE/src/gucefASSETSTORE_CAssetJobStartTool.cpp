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

#ifndef GUCEF_CORE_CJOBDEFINITION_H
#include "gucefCORE_CJobDefinition.h"
#define GUCEF_CORE_CJOBDEFINITION_H
#endif /* GUCEF_CORE_CJOBDEFINITION_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#include "gucefASSETSTORE_CAssetRootedPath.h"
#define GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#endif /* GUCEF_ASSETSTORE_CASSETROOTEDPATH_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEQUERYTOOL_H
#include "gucefASSETSTORE_CAssetPipelineQueryTool.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEQUERYTOOL_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEQUERYTOOL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATA_H
#include "gucefASSETSTORE_CAssetToolJobTaskData.h"
#define GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATA_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBRESULTBUILDER_H
#include "gucefASSETSTORE_CAssetToolJobResultBuilder.h"
#define GUCEF_ASSETSTORE_CASSETTOOLJOBRESULTBUILDER_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBRESULTBUILDER_H ? */

#include "gucefASSETSTORE_CAssetJobStartTool.h"

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

const CORE::CString CAssetJobStartTool::LaneConversion = "assets.conversion";
const CORE::CString CAssetJobStartTool::LaneValidation = "assets.validation";
const CORE::CString CAssetJobStartTool::LaneIngest = "assets.ingest";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetJobStartTool::CAssetJobStartTool( CAssetToolContext& context, const CORE::CString& name, const CORE::CString& taskTypeName )
    : CAssetToolBase( context, name )
    , m_taskTypeName( taskTypeName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetJobStartTool::~CAssetJobStartTool()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetJobStartTool::GetLaneOfKind( const CORE::CString& kind )
{GUCEF_TRACE;

    if ( CAssetPipelineOperations::ValidatePack == kind )
        return LaneValidation;
    if ( "store_ingest" == kind || "store_ingest_resume" == kind )
        return LaneIngest;
    return LaneConversion;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetJobStartTool::Compute( COM::CMcpRequestContext& context      ,
                             const CORE::CDataNode& arguments      ,
                             COM::CMcpToolResult& result           ,
                             COM::CMcpInputRequired& inputRequired ,
                             SCallState& state                     )
{GUCEF_TRACE;

    COM::CMcpJobBridge* bridge = m_context.GetJobBridge();
    if ( GUCEF_NULL == bridge )
    {
        m_context.SetError( result, "backend-unavailable", "The jobs of the asset tools are not available", true );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    if ( !CAssetPipelineQueryTool::CheckPaths( m_context, arguments, result ) )
        return COM::MCP_EXECUTE_COMPLETE;
    const COM::CMcpPrincipal& principal = context.GetPrincipal();
    const CORE::CString requestKey = GetStringArgument( arguments, "requestKey" );
    const CORE::CString digest = CAssetToolContext::DigestArguments( arguments );

    CAssetToolJobTaskData data( m_taskTypeName );
    data.kind = m_name;
    data.commandId = requestKey.IsNULLOrEmpty() ? "mcp-" + CAssetToolContext::MakeHandle() : CAssetToolContext::MakeCommandId( principal.subject, m_name, requestKey );
    CORE::CString packId = GetStringArgument( arguments, "packId" );
    CORE::CDataNodeSerializableSettings serializerSettings;

    if ( "store_ingest" == m_name )
    {
        CAssetRootedPath rooted;
        const CAssetToolRoot* root = GUCEF_NULL;
        CORE::CString vfsPath;
        CORE::CString problem;
        if ( !CAssetRootedPath::ParseAndResolve( GetStringArgument( arguments, "path" ), m_context.GetSettings().roots,
                                                 CAssetToolRoot::KindSource + "|" + CAssetToolRoot::KindStaging, rooted, root, vfsPath, problem ) )
        {
            m_context.SetError( result, "path-refused", "The path is refused: " + problem, false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        CAssetIngestRequest request;
        request.sourceId = "disk";
        request.contentPath = vfsPath;
        request.keep = GetStringArgument( arguments, "keep" );
        request.locationId = GetStringArgument( arguments, "locationId" );
        request.duplicatePolicy = GetStringArgument( arguments, "duplicatePolicy" );
        data.request.Clear();
        data.request.SetNodeType( GUCEF_DATATYPE_OBJECT );
        request.Serialize( data.request, serializerSettings );
        packId.Clear();
    }
    else
    if ( "store_ingest_resume" == m_name )
    {
        data.request.Clear();
        data.request.SetNodeType( GUCEF_DATATYPE_OBJECT );
        data.request.SetAttribute( "ingestId", GetStringArgument( arguments, "ingestId" ) );
    }
    else
    {
        CAssetPipelineRequest request;
        CAssetPipelineQueryTool::BuildRequest( m_context, m_name, arguments, principal, request );
        if ( CAssetPipelineOperations::ConvertFile == m_name )
            request.arguments.SetAttribute( "jobKey", CAssetToolContext::Sha256Hex( data.commandId ).SubstrToIndex( 16, true ) );

        // the licence decides before anything is queued (the pipeline checks it again when it runs)
        if ( !request.packId.IsNULLOrEmpty() )
        {
            CAssetPipelineRequest check;
            CAssetPipelineQueryTool::BuildRequest( m_context, CAssetPipelineOperations::LicenceCheck, arguments, principal, check );
            check.operation = CAssetPipelineOperations::LicenceCheck;
            CAssetServiceDocument answer;
            const TAssetStoreStatus status = m_context.GetService().RunPipelineQuery( check, answer );
            if ( ASSETSTORE_STATUS_OK != status )
            {
                m_context.SetServiceError( result, status, answer.message, &answer.document );
                return COM::MCP_EXECUTE_COMPLETE;
            }
            if ( !answer.document.GetAttributeValueOrChildValueByName( "conversionAllowed" ).AsBool( false, true ) )
            {
                const CORE::CString reason = answer.document.GetAttributeValueOrChildValueByName( "reason" ).AsString( CORE::CString::Empty, true );
                m_context.SetError( result, "licence-refused", "The licence record refuses the conversion of the pack" + ( reason.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + reason ) +
                    ". Nothing was queued; the licence record is the owner's to change", false );
                return COM::MCP_EXECUTE_COMPLETE;
            }
        }
        data.request.Clear();
        data.request.SetNodeType( GUCEF_DATATYPE_OBJECT );
        request.Serialize( data.request, serializerSettings );
    }

    CORE::CString nextAllowed;
    if ( !m_context.TakeJobQuota( principal.subject, nextAllowed ) )
    {
        m_context.SetError( result, "busy", "The jobs of today are used up; the next one is allowed at " + nextAllowed, true );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    CORE::CJobDefinition definition;
    definition.SetTaskTypeName( m_taskTypeName );
    definition.SetLaneName( GetLaneOfKind( m_name ) );
    CORE::CDataNode payload;
    data.ToPayload( payload );
    definition.SetPayload( payload );

    COM::CMcpJobInfo job;
    CORE::CString error;
    switch ( bridge->StartJob( principal, m_name, definition, requestKey, digest, job, error ) )
    {
        case COM::CMcpJobBridge::JOBSTART_STARTED:
        case COM::CMcpJobBridge::JOBSTART_REPEATED:
        {
            break;
        }
        case COM::CMcpJobBridge::JOBSTART_KEY_CONFLICT:
        {
            m_context.SetError( result, "invalid-arguments", "The requestKey was used before with other arguments: use a new key", false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        case COM::CMcpJobBridge::JOBSTART_BUSY:
        {
            m_context.SetError( result, "busy", "Too many jobs of this caller are not done yet: wait for one to end. " + error, true );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        default:
        {
            m_context.SetError( result, "failed", "The job could not be started: " + error, false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
    }
    m_context.RememberJobPack( job.jobId, packId );
    UInt32 waitMs = GetUInt32Argument( arguments, "waitMs", m_context.GetSettings().defaultWaitMs );
    if ( waitMs > m_context.GetSettings().maxWaitMs )
        waitMs = m_context.GetSettings().maxWaitMs;
    COM::CIMcpJobResultBuilderPtr builder( GUCEF_NEW CAssetToolJobResultBuilder( m_context, packId ) );
    state.finished = true;
    return bridge->Wait( context, job.jobId, waitMs, result, builder );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
