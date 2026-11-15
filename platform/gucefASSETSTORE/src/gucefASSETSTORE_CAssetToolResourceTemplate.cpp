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

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_COM_CMCPREQUESTCONTEXT_H
#include "gucefCOM_CMcpRequestContext.h"
#define GUCEF_COM_CMCPREQUESTCONTEXT_H
#endif /* GUCEF_COM_CMCPREQUESTCONTEXT_H ? */

#ifndef GUCEF_COM_CMCPCALLSCOPE_H
#include "gucefCOM_CMcpCallScope.h"
#define GUCEF_COM_CMCPCALLSCOPE_H
#endif /* GUCEF_COM_CMCPCALLSCOPE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#include "gucefASSETSTORE_CAssetToolUris.h"
#define GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLURIS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBRESULTBUILDER_H
#include "gucefASSETSTORE_CAssetToolJobResultBuilder.h"
#define GUCEF_ASSETSTORE_CASSETTOOLJOBRESULTBUILDER_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBRESULTBUILDER_H ? */

#include "gucefASSETSTORE_CAssetToolResourceTemplate.h"

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

COM::EMcpExecuteResult
Fail( COM::CMcpRequestContext& request, CAssetToolContext& context, Int32 code, const CORE::CString& message )
{GUCEF_TRACE;

    request.SetProtocolError( code, context.GetRedaction().ForResult( message ) );
    return COM::MCP_EXECUTE_PROTOCOL_ERROR;
}

COM::EMcpExecuteResult
AddJson( CAssetToolContext& context, COM::CMcpRequestContext& request, const CORE::CString& uri, CORE::CDataNode& document,
         COM::CMcpResourceContents& contents )
{GUCEF_TRACE;

    context.GetRedaction().RedactTree( document );
    CORE::CString text;
    COM::CJsonRpcEndpoint::ToJsonText( document, text );
    if ( text.Length() > context.GetSettings().maxResourceBytes )
        return Fail( request, context, COM::MCP_ERROR_INTERNAL, "The resource is larger than the limit of a resource read: use the paged tools (pack_assets, job_result) instead" );
    contents.AddText( uri, "application/json", text );
    return COM::MCP_EXECUTE_COMPLETE;
}

/** The protocol error of a service answer */
COM::EMcpExecuteResult
FailService( CAssetToolContext& context, COM::CMcpRequestContext& request, TAssetStoreStatus status, const CORE::CString& message, const CORE::CDataNode* document )
{GUCEF_TRACE;

    CORE::CString code = GUCEF_NULL != document ? document->GetAttributeValueOrChildValueByName( "errorCode" ).AsString( CORE::CString::Empty, true ) : CORE::CString::Empty;
    if ( code.IsNULLOrEmpty() )
        code = CAssetToolContext::GetErrorCodeOfStatus( status );
    const CORE::CString text = code + ": " + ( message.IsNULLOrEmpty() ? CORE::CString( "the resource cannot be read" ) : message );
    if ( "not-found" == code || "pack-unknown" == code || "path-refused" == code || "invalid-arguments" == code || "pack-ambiguous" == code )
        return Fail( request, context, COM::MCP_ERROR_INVALID_PARAMS, text );
    if ( "content-withheld" == code )
        return Fail( request, context, COM::MCP_ERROR_GUCEF_ACCESS_DENIED, text );
    return Fail( request, context, COM::MCP_ERROR_INTERNAL, text );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetToolResourceTemplate::CAssetToolResourceTemplate( CAssetToolContext& context, EKind kind, const CORE::CString& name, const CORE::CString& uriTemplate,
                                                        const CORE::CString& mimeType, const CORE::CString& description )
    : COM::CIMcpResourceTemplate()
    , m_context( context )
    , m_kind( kind )
    , m_name( name )
    , m_uriTemplate( uriTemplate )
    , m_mimeType( mimeType )
    , m_description( description )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolResourceTemplate::~CAssetToolResourceTemplate()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetToolResourceTemplate::GetDescriptor( COM::CMcpResourceTemplateDescriptor& descriptor ) const
{GUCEF_TRACE;

    descriptor = COM::CMcpResourceTemplateDescriptor();
    descriptor.name = m_name;
    descriptor.uriTemplate = m_uriTemplate;
    descriptor.mimeType = m_mimeType;
    descriptor.description = m_description;
    return true;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecutionMode
CAssetToolResourceTemplate::GetExecutionMode( void ) const
{GUCEF_TRACE;

    return COM::MCP_EXECUTION_THREAD_POOL;
}

/*-------------------------------------------------------------------------*/

void
CAssetToolResourceTemplate::GetCacheHints( const CORE::CString& uri, UInt32& ttlMs, COM::EMcpCacheScope& scope ) const
{GUCEF_TRACE;

    scope = COM::MCP_CACHESCOPE_PRIVATE;
    ttlMs = 0 == uri.HasSubstr( CAssetToolUris::Prefix + "docs/", true ) ? 60000 : 0;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetToolResourceTemplate::ReadUri( CAssetToolContext& context              ,
                                     EKind kind                              ,
                                     COM::CMcpRequestContext& request        ,
                                     const CORE::CString& uri                ,
                                     COM::CMcpResourceContents& contents     )
{GUCEF_TRACE;

    CORE::CString::StringVector segments;
    if ( !CAssetToolUris::Split( uri, segments ) )
        return Fail( request, context, COM::MCP_ERROR_INVALID_PARAMS, "not-found: the URI is no assettools URI this server serves" );
    CIAssetStoreService& service = context.GetService();
    CORE::CDataNodeSerializableSettings settings;

    if ( KIND_PIPELINE == kind )
    {
        CAssetPipelineRequest pipeline;
        pipeline.operation = CAssetPipelineOperations::ReadResource;
        pipeline.arguments.SetNodeType( GUCEF_DATATYPE_OBJECT );
        pipeline.arguments.SetAttribute( "uri", uri );
        pipeline.arguments.SetAttribute( "principal", request.GetPrincipal().subject );
        pipeline.arguments.SetAttribute( "exposePurchaseDetails", context.GetExposure().exposePurchaseDetails );
        pipeline.arguments.SetAttribute( "maxResourceBytes", context.GetSettings().maxResourceBytes );
        CAssetServiceDocument answer;
        const TAssetStoreStatus status = service.RunPipelineQuery( pipeline, answer );
        if ( ASSETSTORE_STATUS_OK != status )
            return FailService( context, request, status, answer.message, &answer.document );
        CORE::CString mimeType = answer.document.GetAttributeValueOrChildValueByName( "mimeType" ).AsString( CORE::CString::Empty, true );
        const CORE::CString text = context.GetRedaction().ForResult( answer.document.GetAttributeValueOrChildValueByName( "text" ).AsString( CORE::CString::Empty, true ) );
        if ( text.Length() > context.GetSettings().maxResourceBytes )
            return Fail( request, context, COM::MCP_ERROR_INTERNAL, "too-large: the resource is larger than the limit of a resource read: use the paged tools instead" );
        contents.AddText( uri, mimeType.IsNULLOrEmpty() ? CORE::CString( "application/json" ) : mimeType, text );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    if ( KIND_JOB_RESULT == kind )
    {
        COM::CMcpJobBridge* bridge = context.GetJobBridge();
        COM::CMcpJobInfo job;
        if ( segments.size() < 3 || GUCEF_NULL == bridge || COM::CMcpJobBridge::JOB_FOUND != bridge->GetJob( request.GetPrincipal(), segments[ 1 ], job ) )
            return Fail( request, context, COM::MCP_ERROR_INVALID_PARAMS, "not-found: there is no such job of this caller" );
        CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode object;
        CAssetToolJobResultBuilder::BuildJobObject( context, job, false, object );
        object.SetName( "job" );
        document.AddChild( object );
        CORE::CString failure;
        const CORE::CString serviceJobId = CAssetToolJobResultBuilder::GetServiceJobId( job, failure );
        if ( job.IsFinal() && !serviceJobId.IsNULLOrEmpty() )
        {
            CAssetServiceDocument result;
            const TAssetStoreStatus status = service.GetJobResult( serviceJobId, CORE::CString::Empty, CORE::CString::Empty, 0, result );
            if ( ASSETSTORE_STATUS_OK != status )
                return FailService( context, request, status, result.message, &result.document );
            CORE::CDataNode* resultNode = document.AddChild( result.document );
            resultNode->SetName( "result" );
        }
        return AddJson( context, request, uri, document, contents );
    }

    if ( segments.size() < 3 || "store" != segments[ 0 ] )
        return Fail( request, context, COM::MCP_ERROR_INVALID_PARAMS, "not-found: the URI is no store resource" );
    const CORE::CString& id = segments[ 2 ];

    if ( KIND_STORE_ASSET == kind )
    {
        CAssetRecord asset;
        const TAssetStoreStatus status = service.GetAsset( id, asset );
        if ( ASSETSTORE_STATUS_OK != status )
            return FailService( context, request, status, "there is no asset " + id, GUCEF_NULL );
        CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
        asset.Serialize( document, settings );
        return AddJson( context, request, uri, document, contents );
    }
    if ( KIND_STORE_PACK == kind )
    {
        CAssetPackRecord pack;
        const TAssetStoreStatus status = service.GetPack( id, pack );
        if ( ASSETSTORE_STATUS_OK != status )
            return FailService( context, request, status, "there is no pack " + id, GUCEF_NULL );
        CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
        pack.Serialize( document, settings );
        return AddJson( context, request, uri, document, contents );
    }
    if ( KIND_LICENCE_TEXT == kind )
    {
        CAssetLicence licence;
        const TAssetStoreStatus status = service.GetLicence( id, licence );
        if ( ASSETSTORE_STATUS_OK != status )
            return FailService( context, request, status, "there is no licence " + id, GUCEF_NULL );
        const CORE::CString text = context.GetRedaction().ForResult( licence.text );
        if ( text.Length() > context.GetSettings().maxResourceBytes )
            return Fail( request, context, COM::MCP_ERROR_INTERNAL, "too-large: the licence text is larger than the limit of a resource read" );
        contents.AddText( uri, "text/plain", text );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    // KIND_STORE_THUMBNAIL: content, gated here as well as by the access policy
    CORE::CString reason;
    if ( !context.GetGate().IsStoreAssetAllowed( id, reason ) )
        return Fail( request, context, COM::MCP_ERROR_GUCEF_ACCESS_DENIED, "content-withheld: " + reason );
    CORE::CString::StringVector ids;
    ids.push_back( id );
    CAssetThumbnailPage thumbnails;
    const TAssetStoreStatus status = service.GetThumbnails( ids, 256, thumbnails );
    if ( ASSETSTORE_STATUS_OK != status )
        return FailService( context, request, status, ASSETSTORE_STATUS_UNSUPPORTED == status ? CORE::CString( "this service has no thumbnails yet" ) : CORE::CString::Empty, GUCEF_NULL );
    if ( thumbnails.entries.empty() || ASSETSTORE_STATUS_OK != thumbnails.entries[ 0 ].status || 0 == thumbnails.entries[ 0 ].encoded.GetDataSize() )
        return Fail( request, context, COM::MCP_ERROR_INVALID_PARAMS, "not-found: the asset has no thumbnail" );
    contents.AddBlob( uri, "image/png", thumbnails.entries[ 0 ].encoded );
    return COM::MCP_EXECUTE_COMPLETE;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetToolResourceTemplate::Read( COM::CMcpRequestContext& context      ,
                                  const CORE::CString& uri              ,
                                  const CORE::CValueList& variables     ,
                                  COM::CMcpResourceContents& contents   ,
                                  COM::CMcpInputRequired& inputRequired )
{GUCEF_TRACE;

    COM::CMcpCallScope scope( m_context.GetCallGate(), context );
    if ( !scope.IsEntered() )
        return Fail( context, m_context, COM::MCP_ERROR_INTERNAL, "backend-unavailable: the asset tools are being stopped" );
    try
    {
        COM::EMcpExecuteResult outcome = ReadUri( m_context, m_kind, context, uri, contents );
        if ( COM::MCP_EXECUTE_COMPLETE == outcome )
        {
            UInt32 ttlMs = 0;
            COM::EMcpCacheScope cacheScope = COM::MCP_CACHESCOPE_PRIVATE;
            GetCacheHints( uri, ttlMs, cacheScope );
            contents.SetCacheHints( ttlMs, cacheScope );
        }
        return outcome;
    }
    catch ( ... )
    {
        return Fail( context, m_context, COM::MCP_ERROR_INTERNAL, "failed: the resource could not be read" );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
