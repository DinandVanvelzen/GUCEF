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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBRESULTBUILDER_H
#include "gucefASSETSTORE_CAssetToolJobResultBuilder.h"
#define GUCEF_ASSETSTORE_CASSETTOOLJOBRESULTBUILDER_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBRESULTBUILDER_H ? */

#include "gucefASSETSTORE_CAssetJobTool.h"

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

/** "o<offset>" */
bool
ParseCursor( const CORE::CString& cursor, UInt32& offset )
{GUCEF_TRACE;

    offset = 0;
    if ( cursor.IsNULLOrEmpty() )
        return true;
    if ( cursor.Length() < 2 || 'o' != cursor[ 0 ] )
        return false;
    const CORE::CString digits = cursor.CutChars( 1, true );
    for ( UInt32 i=0; i<digits.Length(); ++i )
        if ( digits[ i ] < '0' || digits[ i ] > '9' )
            return false;
    offset = CORE::StringToUInt32( digits );
    return true;
}

/** A page of the entries of an array node into "items" of the result */
void
PageList( const CORE::CDataNode* list, UInt32 offset, UInt32 limit, CORE::CDataNode& page )
{GUCEF_TRACE;

    CORE::CDataNode* items = page.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    const UInt32 total = GUCEF_NULL != list ? list->GetNrOfDirectChildNodes() : 0;
    for ( UInt32 i=offset; i<total && i<offset+limit; ++i )
        items->AddChild( *list->GetChildAtIndex( i ) );
    page.SetAttribute( "total", total );
    if ( offset + limit < total )
        page.SetAttribute( "nextCursor", "o" + CORE::ToString( offset + limit ) );
    else
        page.SetAttribute( "nextCursor", CORE::CVariant() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetJobTool::CAssetJobTool( CAssetToolContext& context, const CORE::CString& name )
    : CAssetToolBase( context, name )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetJobTool::~CAssetJobTool()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetJobTool::Lookup( COM::CMcpJobBridge& bridge, const COM::CMcpPrincipal& principal, const CORE::CString& jobId,
                       COM::CMcpJobInfo& job, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    switch ( bridge.GetJob( principal, jobId, job ) )
    {
        case COM::CMcpJobBridge::JOB_FOUND:
            return true;
        case COM::CMcpJobBridge::JOB_EXPIRED:
            m_context.SetError( result, "job-expired", "The job ended too long ago: its handle expired. Start the job again for a new result", false );
            return false;
        default:
            m_context.SetError( result, "job-unknown", "There is no such job of this caller: check the jobId (job_list)", false );
            return false;
    }
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetJobTool::Compute( COM::CMcpRequestContext& context      ,
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
    const COM::CMcpPrincipal& principal = context.GetPrincipal();
    const CORE::CString jobId = GetStringArgument( arguments, "jobId" );

    if ( "job_status" == m_name )
    {
        COM::CMcpJobInfo job;
        if ( !Lookup( *bridge, principal, jobId, job, result ) )
            return COM::MCP_EXECUTE_COMPLETE;
        CORE::CDataNode object;
        CAssetToolJobResultBuilder::BuildJobObject( m_context, job, true, object );
        if ( !job.IsFinal() )
            object.SetAttribute( "hint", "The job goes on: poll job_status again." );
        SetStructured( result, object );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    if ( "job_cancel" == m_name )
    {
        COM::CMcpJobInfo job;
        CORE::CString error;
        if ( !Lookup( *bridge, principal, jobId, job, result ) )
            return COM::MCP_EXECUTE_COMPLETE;
        if ( COM::CMcpJobBridge::JOB_FOUND != bridge->CancelJob( principal, jobId, job, error ) )
        {
            m_context.SetError( result, "job-unknown", "There is no such job of this caller", false );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        CORE::CDataNode object;
        CAssetToolJobResultBuilder::BuildJobObject( m_context, job, true, object );
        if ( !job.IsFinal() )
            object.SetAttribute( "hint", "The job was asked to stop: poll job_status until it is cancelled." );
        SetStructured( result, object );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    UInt32 offset = 0;
    if ( !ParseCursor( GetStringArgument( arguments, "cursor" ), offset ) )
    {
        m_context.SetError( result, "invalid-arguments", "The cursor is not one this tool gave", false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    const UInt32 limit = m_context.GetPageLimit( GetUInt32Argument( arguments, "limit", 0 ) );

    if ( "job_list" == m_name )
    {
        COM::TMcpJobInfoVector jobs;
        bridge->ListJobs( principal, jobs, GetStringArgument( arguments, "kind" ), GetStringArgument( arguments, "state" ) );
        const CORE::CString packId = GetStringArgument( arguments, "packId" );
        CORE::CDataNode all( "all", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<jobs.size(); ++i )
        {
            if ( !packId.IsNULLOrEmpty() && m_context.GetJobPack( jobs[ i ].jobId ) != packId )
                continue;
            CORE::CDataNode object;
            CAssetToolJobResultBuilder::BuildJobObject( m_context, jobs[ i ], false, object );
            all.AddChild( object );
        }
        CORE::CDataNode page( GUCEF_DATATYPE_OBJECT );
        PageList( &all, offset, limit, page );
        SetStructured( result, page );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    // job_result
    COM::CMcpJobInfo job;
    if ( !Lookup( *bridge, principal, jobId, job, result ) )
        return COM::MCP_EXECUTE_COMPLETE;
    CORE::CString section = GetStringArgument( arguments, "section" );
    if ( section.IsNULLOrEmpty() )
        section = "summary";
    CORE::CDataNode page( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode object;
    CAssetToolJobResultBuilder::BuildJobObject( m_context, job, false, object );
    object.SetName( "job" );
    page.AddChild( object );
    page.SetAttribute( "section", section );
    CORE::CString failure;
    const CORE::CString serviceJobId = CAssetToolJobResultBuilder::GetServiceJobId( job, failure );
    if ( !job.IsFinal() || serviceJobId.IsNULLOrEmpty() )
    {
        page.SetAttribute( "hint", job.IsFinal() ? CORE::CString( "The job ended without a result document" ) : CORE::CString( "The job has not ended: poll job_status" ) );
        if ( !failure.IsNULLOrEmpty() )
            page.SetAttribute( "message", failure );
        SetStructured( result, page );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    CAssetServiceDocument document;
    const TAssetStoreStatus status = m_context.GetService().GetJobResult( serviceJobId, section, CORE::CString::Empty, 0, document );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        m_context.SetServiceError( result, status, document.message, &document.document );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    if ( "summary" == section )
    {
        CORE::CDataNode summary;
        CAssetToolJobResultBuilder::MakeSummary( document.document, summary );
        summary.SetName( "summary" );
        page.AddChild( summary );
        AddResourceLinks( document.document, result );
    }
    else
    {
        PageList( document.document.FindChild( section ), offset, limit, page );
    }
    SetStructured( result, page );
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
