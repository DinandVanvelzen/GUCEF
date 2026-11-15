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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMER_H
#include "gucefASSETSTORE_CAssetToolJobTaskConsumer.h"
#define GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#include "gucefASSETSTORE_CAssetToolUris.h"
#define GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLURIS_H ? */

#include "gucefASSETSTORE_CAssetToolJobResultBuilder.h"

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

CAssetToolJobResultBuilder::CAssetToolJobResultBuilder( CAssetToolContext& context, const CORE::CString& packId )
    : COM::CIMcpJobResultBuilder()
    , m_context( context )
    , m_packId( packId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolJobResultBuilder::~CAssetToolJobResultBuilder()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolJobResultBuilder::GetServiceJobId( const COM::CMcpJobInfo& job, CORE::CString& failure )
{GUCEF_TRACE;

    failure.Clear();
    if ( !job.hasRun )
        return CORE::CString::Empty;
    const CORE::CString& reference = job.run.resultDocument;
    if ( 0 == reference.HasSubstr( CAssetToolJobTaskConsumer::ResultReferencePrefix, true ) )
        return reference.CutChars( CAssetToolJobTaskConsumer::ResultReferencePrefix.Length(), true );
    if ( 0 == reference.HasSubstr( CAssetToolJobTaskConsumer::FailureReferencePrefix, true ) )
        failure = reference.CutChars( CAssetToolJobTaskConsumer::FailureReferencePrefix.Length(), true );
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CAssetToolJobResultBuilder::MakeSummary( const CORE::CDataNode& document, CORE::CDataNode& summary )
{GUCEF_TRACE;

    summary.Clear();
    summary.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode::TAttributeMap::const_iterator a = document.AttributeBegin();
    while ( a != document.AttributeEnd() )
    {
        summary.SetAttribute( (*a).first, (*a).second );
        ++a;
    }
    CORE::CDataNode::const_iterator c = document.ConstBegin();
    while ( c != document.ConstEnd() )
    {
        const CORE::CString& name = (*c)->GetName();
        if ( "failures" != name && "approximations" != name && "assets" != name && "links" != name )
            summary.AddChild( *(*c) );
        ++c;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetToolJobResultBuilder::BuildJobObject( CAssetToolContext& context, const COM::CMcpJobInfo& job, bool withSummary, CORE::CDataNode& object )
{GUCEF_TRACE;

    job.ToDataNode( object );
    CORE::CString failure;
    const CORE::CString serviceJobId = GetServiceJobId( job, failure );
    CORE::CString packId = context.GetJobPack( job.jobId );

    // the state of the job of the service decides once the run ended SUCCEEDED
    CORE::CString serviceMessage;
    if ( COM::CMcpJobInfo::StateCompleted == job.state )
    {
        if ( !failure.IsNULLOrEmpty() )
        {
            object.SetAttribute( "state", COM::CMcpJobInfo::StateFailed );
            serviceMessage = failure;
        }
        else
        if ( !serviceJobId.IsNULLOrEmpty() )
        {
            CAssetJobEntry entry;
            if ( ASSETSTORE_STATUS_OK == context.GetService().GetJob( serviceJobId, entry ) )
            {
                if ( ASSETJOB_STATE_FAILED == entry.state )
                    object.SetAttribute( "state", COM::CMcpJobInfo::StateFailed );
                else
                if ( ASSETJOB_STATE_CANCELLED == entry.state )
                    object.SetAttribute( "state", COM::CMcpJobInfo::StateCancelled );
                serviceMessage = entry.message;
                if ( packId.IsNULLOrEmpty() )
                    packId = entry.packId;
            }
        }
    }
    if ( !serviceMessage.IsNULLOrEmpty() )
        object.SetAttribute( "message", serviceMessage );
    if ( packId.IsNULLOrEmpty() )
        object.SetAttribute( "packId", CORE::CVariant() );
    else
        object.SetAttribute( "packId", packId );
    object.SetAttribute( "resultUri", CAssetToolUris::ForJobResult( job.jobId ) );

    if ( withSummary && job.IsFinal() )
    {
        if ( !serviceJobId.IsNULLOrEmpty() )
        {
            CAssetServiceDocument document;
            if ( ASSETSTORE_STATUS_OK == context.GetService().GetJobResult( serviceJobId, "summary", CORE::CString::Empty, 0, document ) )
            {
                CORE::CDataNode summary;
                MakeSummary( document.document, summary );
                summary.SetName( "summary" );
                object.AddChild( summary );
                return;
            }
        }
        if ( !failure.IsNULLOrEmpty() )
        {
            CORE::CDataNode* summary = object.AddChild( "summary", GUCEF_DATATYPE_OBJECT );
            const Int32 colon = failure.HasChar( ':' );
            summary->SetAttribute( "errorCode", colon > 0 ? failure.SubstrToIndex( (UInt32) colon, true ) : CORE::CString( "failed" ) );
            summary->SetAttribute( "message", colon > 0 ? failure.CutChars( (UInt32) colon + 1, true ) : failure );
            return;
        }
        object.SetAttribute( "summary", CORE::CVariant() );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetToolJobResultBuilder::BuildJobResult( const COM::CMcpJobInfo& job, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    m_context.RememberJobPack( job.jobId, m_packId );
    CORE::CDataNode object;
    BuildJobObject( m_context, job, true, object );
    if ( !job.IsFinal() )
        object.SetAttribute( "hint", "The job goes on: poll job_status with the jobId, read job_result when it ended. Only job_cancel stops it." );
    result.Clear();
    result.SetStructuredContent( object, false );
    CAssetToolContext::TContentTagVector none;
    m_context.FinishResult( result, none );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
