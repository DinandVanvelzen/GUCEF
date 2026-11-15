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

#ifndef GUCEF_COM_CMCPCALLSCOPE_H
#include "gucefCOM_CMcpCallScope.h"
#define GUCEF_COM_CMCPCALLSCOPE_H
#endif /* GUCEF_COM_CMCPCALLSCOPE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#include "gucefASSETSTORE_CAssetToolUris.h"
#define GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLURIS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLRESOURCETEMPLATE_H
#include "gucefASSETSTORE_CAssetToolResourceTemplate.h"
#define GUCEF_ASSETSTORE_CASSETTOOLRESOURCETEMPLATE_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLRESOURCETEMPLATE_H ? */

#include "gucefASSETSTORE_CAssetToolResource.h"

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

CAssetToolResource::CAssetToolResource( CAssetToolContext& context, EKind kind )
    : COM::CIMcpResource()
    , m_context( context )
    , m_kind( kind )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolResource::~CAssetToolResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetToolResource::GetDescriptor( COM::CMcpResourceDescriptor& descriptor ) const
{GUCEF_TRACE;

    descriptor = COM::CMcpResourceDescriptor();
    if ( KIND_STATUS == m_kind )
    {
        descriptor.uri = CAssetToolUris::Prefix + "server/status";
        descriptor.name = "server-status";
        descriptor.title = "Status of the asset tools";
        descriptor.description = "The service, the enabled tool groups, the roots (ids and kinds), the limits, whether a converter and a renderer are there";
        descriptor.mimeType = "application/json";
    }
    else
    {
        descriptor.uri = CAssetToolUris::ForDoc( "manifest-reference" );
        descriptor.name = "manifest-reference";
        descriptor.title = "Manifest reference";
        descriptor.description = "The members of a pack manifest: presets, rules, overrides, cook profiles, name hints";
        descriptor.mimeType = "text/markdown";
    }
    return true;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecutionMode
CAssetToolResource::GetExecutionMode( void ) const
{GUCEF_TRACE;

    return COM::MCP_EXECUTION_THREAD_POOL;
}

/*-------------------------------------------------------------------------*/

void
CAssetToolResource::GetCacheHints( UInt32& ttlMs, COM::EMcpCacheScope& scope ) const
{GUCEF_TRACE;

    ttlMs = KIND_STATUS == m_kind ? 0 : 60000;
    scope = COM::MCP_CACHESCOPE_PRIVATE;
}

/*-------------------------------------------------------------------------*/

void
CAssetToolResource::BuildStatus( CAssetToolContext& context, CORE::CDataNode& status )
{GUCEF_TRACE;

    status.Clear();
    status.SetNodeType( GUCEF_DATATYPE_OBJECT );
    const CAssetToolSetSettings& settings = context.GetSettings();
    status.SetAttribute( "toolSet", settings.name );
    CORE::CDataNode exposure;
    context.GetExposure().ToDataNode( exposure );
    exposure.SetName( "exposure" );
    status.AddChild( exposure );
    CORE::CDataNode* roots = status.AddChild( "roots", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<settings.roots.size(); ++i )
    {
        CORE::CDataNode* root = roots->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        root->SetAttribute( "id", settings.roots[ i ].id );
        root->SetAttribute( "kind", settings.roots[ i ].kind );
        root->SetAttribute( "writable", settings.roots[ i ].AllowsWrites() );
    }
    CORE::CDataNode* limits = status.AddChild( "limits", GUCEF_DATATYPE_OBJECT );
    limits->SetAttribute( "defaultLimit", settings.defaultLimit );
    limits->SetAttribute( "maxLimit", settings.maxLimit );
    limits->SetAttribute( "maxResultBytes", settings.maxResultBytes );
    limits->SetAttribute( "maxResourceBytes", settings.maxResourceBytes );
    limits->SetAttribute( "defaultWaitMs", settings.defaultWaitMs );
    limits->SetAttribute( "maxWaitMs", settings.maxWaitMs );
    limits->SetAttribute( "confirmWrites", settings.confirmWrites );

    CORE::CDataNode* service = status.AddChild( "service", GUCEF_DATATYPE_OBJECT );
    CIAssetStoreService& backend = context.GetService();
    service->SetAttribute( "type", backend.GetType() );
    service->SetAttribute( "inProcess", backend.IsInProcess() );
    service->SetAttribute( "open", backend.IsOpen() );
    service->SetAttribute( "catalogueAvailable", backend.IsCatalogueAvailable() );
    service->SetAttribute( "message", backend.GetStatusMessage() );
    CAssetServiceInfo info;
    if ( ASSETSTORE_STATUS_OK == backend.GetServiceInfo( info ) )
    {
        service->SetAttribute( "implementationId", info.implementationId );
        service->SetAttribute( "version", info.version );
        service->SetAttribute( "converter", info.converterName );
        service->SetAttribute( "converterProfile", info.converterProfile );
        service->SetAttribute( "hasRenderer", info.hasRenderer );
        CORE::CString::StringVector capabilities( info.capabilities.begin(), info.capabilities.end() );
        service->SetValuesOfChildByName( "capabilities", capabilities );
    }
    COM::CMcpJobBridge* bridge = context.GetJobBridge();
    CORE::CDataNode* jobs = status.AddChild( "jobs", GUCEF_DATATYPE_OBJECT );
    jobs->SetAttribute( "available", GUCEF_NULL != bridge );
    if ( GUCEF_NULL != bridge )
    {
        jobs->SetAttribute( "handles", bridge->GetJobCount() );
        jobs->SetAttribute( "waitingCalls", bridge->GetWaitingCount() );
    }
    status.SetAttribute( "secretsRegistered", context.GetRedaction().GetSecretCount() );
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetToolResource::Read( COM::CMcpRequestContext& context      ,
                          COM::CMcpResourceContents& contents   ,
                          COM::CMcpInputRequired& inputRequired )
{GUCEF_TRACE;

    COM::CMcpCallScope scope( m_context.GetCallGate(), context );
    if ( !scope.IsEntered() )
    {
        context.SetProtocolError( COM::MCP_ERROR_INTERNAL, "backend-unavailable: the asset tools are being stopped" );
        return COM::MCP_EXECUTE_PROTOCOL_ERROR;
    }
    COM::CMcpResourceDescriptor descriptor;
    GetDescriptor( descriptor );
    if ( KIND_STATUS == m_kind )
    {
        CORE::CDataNode status;
        BuildStatus( m_context, status );
        m_context.GetRedaction().RedactTree( status );
        CORE::CString text;
        COM::CJsonRpcEndpoint::ToJsonText( status, text );
        contents.AddText( descriptor.uri, "application/json", text );
        contents.SetCacheHints( 0, COM::MCP_CACHESCOPE_PRIVATE );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    const COM::EMcpExecuteResult outcome = CAssetToolResourceTemplate::ReadUri( m_context, CAssetToolResourceTemplate::KIND_PIPELINE, context, descriptor.uri, contents );
    if ( COM::MCP_EXECUTE_COMPLETE == outcome )
        contents.SetCacheHints( 60000, COM::MCP_CACHESCOPE_PRIVATE );
    return outcome;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
