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

#ifndef GUCEF_COM_CMCPTOOLREGISTRY_H
#include "gucefCOM_CMcpToolRegistry.h"
#define GUCEF_COM_CMCPTOOLREGISTRY_H
#endif /* GUCEF_COM_CMCPTOOLREGISTRY_H ? */

#ifndef GUCEF_COM_CMCPREQUESTCONTEXT_H
#include "gucefCOM_CMcpRequestContext.h"
#define GUCEF_COM_CMCPREQUESTCONTEXT_H
#endif /* GUCEF_COM_CMCPREQUESTCONTEXT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLCATALOGUE_H
#include "gucefASSETSTORE_CAssetToolCatalogue.h"
#define GUCEF_ASSETSTORE_CASSETTOOLCATALOGUE_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLCATALOGUE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#include "gucefASSETSTORE_CAssetToolUris.h"
#define GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLURIS_H ? */

#include "gucefASSETSTORE_CAssetToolsAccessPolicy.h"

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

CAssetToolsAccessPolicy::CAssetToolsAccessPolicy( CAssetToolContext& context )
    : COM::CIMcpAccessPolicy()
    , m_context( context )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolsAccessPolicy::~CAssetToolsAccessPolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsAccessPolicy::IsToolVisible( const CAssetToolsExposure& exposure, const CORE::CString& name, COM::EMcpToolEffect effect )
{GUCEF_TRACE;

    if ( !exposure.mcpEnabled || exposure.IsDenied( name ) )
        return false;
    const CORE::CString group = CAssetToolCatalogue::GetGroup( name );
    if ( group.IsNULLOrEmpty() )
        return COM::MCP_TOOLEFFECT_READ_ONLY == effect || exposure.IsAllowedWriteTool( name );
    if ( CAssetToolsExposure::GroupRead == group )
        return true;
    if ( "job_cancel" == name )
        return exposure.convertEnabled || exposure.storeEnabled || exposure.IsAllowedWriteTool( name );
    return exposure.IsGroupEnabled( group ) || exposure.IsAllowedWriteTool( name );
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsAccessPolicy::IsVisibilityPrincipalDependent( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsAccessPolicy::IsToolVisible( const COM::CMcpPrincipal& principal, const COM::CMcpToolEntry& tool ) const
{GUCEF_TRACE;

    return IsToolVisible( m_context.GetExposure(), tool.descriptor.name, tool.descriptor.effect );
}

/*-------------------------------------------------------------------------*/

COM::EMcpAccessDecision
CAssetToolsAccessPolicy::AuthorizeToolCall( const COM::CMcpRequestContext& context ,
                                            const COM::CMcpToolEntry& tool          ,
                                            const CORE::CDataNode& arguments        ,
                                            CORE::CString& reason                   ) const
{GUCEF_TRACE;

    const CAssetToolsExposure exposure = m_context.GetExposure();
    if ( !IsToolVisible( exposure, tool.descriptor.name, tool.descriptor.effect ) )
    {
        reason = "the tool is not enabled";
        return COM::MCP_ACCESS_HIDE;
    }
    if ( "job_cancel" == tool.descriptor.name )
    {
        COM::CMcpJobBridge* bridge = m_context.GetJobBridge();
        const CORE::CString jobId = arguments.GetAttributeValueOrChildValueByName( "jobId" ).AsString( CORE::CString::Empty, true );
        COM::CMcpJobInfo job;
        if ( GUCEF_NULL != bridge && COM::CMcpJobBridge::JOB_FOUND == bridge->GetJob( context.GetPrincipal(), jobId, job ) )
        {
            const CORE::CString group = CAssetToolCatalogue::GetGroupOfJobKind( job.kind );
            if ( !exposure.IsGroupEnabled( group ) )
            {
                reason = "a job of the kind " + job.kind + " is cancelled with the tool group " + group + ", which is not enabled";
                return COM::MCP_ACCESS_DENY;
            }
        }
    }
    return COM::MCP_ACCESS_ALLOW;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsAccessPolicy::IsResourceVisible( const COM::CMcpPrincipal& principal, const CORE::CString& uri ) const
{GUCEF_TRACE;

    return m_context.GetExposure().mcpEnabled;
}

/*-------------------------------------------------------------------------*/

COM::EMcpAccessDecision
CAssetToolsAccessPolicy::AuthorizeResourceRead( const COM::CMcpRequestContext& context, const CORE::CString& uri, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !m_context.GetExposure().mcpEnabled )
    {
        reason = "MCP is not enabled";
        return COM::MCP_ACCESS_HIDE;
    }
    CORE::CString::StringVector segments;
    if ( !CAssetToolUris::Split( uri, segments ) )
        return COM::MCP_ACCESS_ALLOW;
    // content: the pictures of a store asset and contact sheets
    if ( 4 == segments.size() && "store" == segments[ 0 ] && "assets" == segments[ 1 ] && "thumbnail" == segments[ 3 ] )
    {
        if ( !m_context.GetGate().IsStoreAssetAllowed( segments[ 2 ], reason ) )
        {
            reason = "content-withheld: " + reason;
            return COM::MCP_ACCESS_DENY;
        }
        return COM::MCP_ACCESS_ALLOW;
    }
    if ( !segments.empty() && "sheets" == segments[ 0 ] )
    {
        reason = "content-withheld: contact sheets are only given inline by render_sheet, which checks the licence of every target";
        return COM::MCP_ACCESS_DENY;
    }
    return COM::MCP_ACCESS_ALLOW;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsAccessPolicy::IsPromptVisible( const COM::CMcpPrincipal& principal, const COM::CMcpPromptEntry& prompt ) const
{GUCEF_TRACE;

    return m_context.GetExposure().mcpEnabled;
}

/*-------------------------------------------------------------------------*/

COM::EMcpAccessDecision
CAssetToolsAccessPolicy::AuthorizePromptGet( const COM::CMcpRequestContext& context, const COM::CMcpPromptEntry& prompt, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !m_context.GetExposure().mcpEnabled )
    {
        reason = "MCP is not enabled";
        return COM::MCP_ACCESS_HIDE;
    }
    return COM::MCP_ACCESS_ALLOW;
}

/*-------------------------------------------------------------------------*/

COM::EMcpAccessDecision
CAssetToolsAccessPolicy::AuthorizeSubscription( const COM::CMcpRequestContext& context, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !m_context.GetExposure().mcpEnabled )
    {
        reason = "MCP is not enabled";
        return COM::MCP_ACCESS_DENY;
    }
    return COM::MCP_ACCESS_ALLOW;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
