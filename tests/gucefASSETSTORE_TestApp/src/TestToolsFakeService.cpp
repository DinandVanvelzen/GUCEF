/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#include "gucefMT_CScopeMutex.h"
#include "dvcppstringutils.h"
#include "gucefASSETSTORE_CAssetServiceOperations.h"
#include "gucefASSETSTORE_CAssetPipelineOperations.h"

#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */

#include "TestToolsFakeService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

using namespace GUCEF;
using namespace GUCEF::ASSETSTORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CToolsFakeService::CToolsFakeService( void )
    : CFakeAssetStoreService()
    , m_toolsLock()
    , m_pipeline()
    , m_changes()
    , m_pipelineCalls()
    , m_changeCalls()
    , m_lastRequests()
    , m_licences()
    , m_assets()
    , m_packs()
    , m_renders( false )
    , m_renderedTargets( 0 )
    , m_jobSteps( 3 )
    , m_jobFinalState( ASSETJOB_STATE_DONE )
    , m_jobResult( GUCEF_DATATYPE_OBJECT )
    , m_jobUntilCancelled( false )
    , m_jobProgressMessage()
    , m_jobs()
    , m_jobCounter( 0 )
    , m_cancels( 0 )
{
}

/*-------------------------------------------------------------------------*/

CToolsFakeService::~CToolsFakeService()
{
}

/*-------------------------------------------------------------------------*/

void
CToolsFakeService::SetPipelineAnswer( const CORE::CString& operation, TAssetStoreStatus status, const CORE::CDataNode& document,
                                      const CORE::CString& message, const CORE::CString& nextPageToken, const CORE::CString& changeToken )
{
    MT::CScopeMutex lock( m_toolsLock );
    SAnswerDocument answer;
    answer.status = status;
    answer.document.status = status;
    answer.document.document = document;
    answer.document.message = message;
    answer.document.nextPageToken = nextPageToken;
    answer.document.changeToken = changeToken;
    m_pipeline[ operation ] = answer;
}

/*-------------------------------------------------------------------------*/

void
CToolsFakeService::SetChangeAnswer( const CORE::CString& operation, TAssetStoreStatus status, const CORE::CDataNode& document, const CORE::CString& message )
{
    MT::CScopeMutex lock( m_toolsLock );
    SAnswerDocument answer;
    answer.status = status;
    answer.document.status = status;
    answer.document.document = document;
    answer.document.message = message;
    m_changes[ operation ] = answer;
}

/*-------------------------------------------------------------------------*/

void
CToolsFakeService::SetConverterLicence( const CORE::CString& packId, bool conversionAllowed, bool contentAllowed, const CORE::CString& reason )
{
    CORE::CDataNode document( GUCEF_DATATYPE_OBJECT );
    document.SetAttribute( "packId", packId );
    document.SetAttribute( "recordId", packId + ".licence" );
    document.SetAttribute( "recordFound", true );
    document.SetAttribute( "conversionAllowed", conversionAllowed );
    document.SetAttribute( "reason", conversionAllowed ? CORE::CString::Empty : reason );
    document.SetAttribute( "contentExposure", contentAllowed ? "allowed" : "withheld" );
    document.SetAttribute( "exposureReason", contentAllowed ? CORE::CString::Empty : reason );
    SetPipelineAnswer( CAssetPipelineOperations::LicenceCheck + "/" + packId, ASSETSTORE_STATUS_OK, document );
}

/*-------------------------------------------------------------------------*/

void
CToolsFakeService::SetLicence( const CORE::CString& id, const CAssetLicence& licence )
{
    MT::CScopeMutex lock( m_toolsLock );
    m_licences[ id ] = licence;
}

/*-------------------------------------------------------------------------*/

void
CToolsFakeService::SetAsset( const CAssetRecord& asset )
{
    MT::CScopeMutex lock( m_toolsLock );
    m_assets[ asset.assetId ] = asset;
}

/*-------------------------------------------------------------------------*/

void
CToolsFakeService::SetPack( const CAssetPackRecord& pack )
{
    MT::CScopeMutex lock( m_toolsLock );
    m_packs[ pack.packId ] = pack;
}

/*-------------------------------------------------------------------------*/

void
CToolsFakeService::SetRenders( bool renders )
{
    MT::CScopeMutex lock( m_toolsLock );
    m_renders = renders;
}

/*-------------------------------------------------------------------------*/

UInt32
CToolsFakeService::GetRenderedTargetCount( void ) const
{
    MT::CScopeMutex lock( m_toolsLock );
    return m_renderedTargets;
}

/*-------------------------------------------------------------------------*/

void
CToolsFakeService::SetJobScript( UInt32 steps, TAssetJobState finalState, const CORE::CDataNode& resultDocument, bool runUntilCancelled,
                                 const CORE::CString& progressMessage )
{
    MT::CScopeMutex lock( m_toolsLock );
    m_jobSteps = steps;
    m_jobFinalState = finalState;
    m_jobResult = resultDocument;
    m_jobUntilCancelled = runUntilCancelled;
    m_jobProgressMessage = progressMessage;
}

/*-------------------------------------------------------------------------*/

UInt32
CToolsFakeService::CountPipelineCalls( const CORE::CString& operation ) const
{
    MT::CScopeMutex lock( m_toolsLock );
    GUCEF::map< CORE::CString, UInt32 >::const_iterator i = m_pipelineCalls.find( operation );
    return i != m_pipelineCalls.end() ? (*i).second : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CToolsFakeService::CountChangeCalls( const CORE::CString& operation ) const
{
    MT::CScopeMutex lock( m_toolsLock );
    GUCEF::map< CORE::CString, UInt32 >::const_iterator i = m_changeCalls.find( operation );
    return i != m_changeCalls.end() ? (*i).second : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CToolsFakeService::GetStartedJobCount( void ) const
{
    MT::CScopeMutex lock( m_toolsLock );
    return m_jobCounter;
}

/*-------------------------------------------------------------------------*/

UInt32
CToolsFakeService::GetCancelCount( void ) const
{
    MT::CScopeMutex lock( m_toolsLock );
    return m_cancels;
}

/*-------------------------------------------------------------------------*/

bool
CToolsFakeService::GetLastPipelineRequest( const CORE::CString& operation, CAssetPipelineRequest& request ) const
{
    MT::CScopeMutex lock( m_toolsLock );
    GUCEF::map< CORE::CString, CAssetPipelineRequest >::const_iterator i = m_lastRequests.find( operation );
    if ( i == m_lastRequests.end() )
        return false;
    request = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CToolsFakeService::RunPipelineQuery( const CAssetPipelineRequest& request, CAssetServiceDocument& result )
{
    CFakeAssetStoreService::RunPipelineQuery( request, result );
    MT::CScopeMutex lock( m_toolsLock );
    ++m_pipelineCalls[ request.operation ];
    m_lastRequests[ request.operation ] = request;
    result.Clear();
    // a decision per pack first (licence_check/<packId>), then the answer of the operation
    TDocumentMap::const_iterator i = m_pipeline.find( request.operation + "/" + request.packId );
    if ( i == m_pipeline.end() )
        i = m_pipeline.find( request.operation );
    if ( i == m_pipeline.end() )
    {
        result.status = ASSETSTORE_STATUS_UNSUPPORTED;
        result.message = "the fake has no answer for " + request.operation;
        return result.status;
    }
    result = (*i).second.document;
    return (*i).second.status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CToolsFakeService::ApplyPipelineChange( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceDocument& result )
{
    CFakeAssetStoreService::ApplyPipelineChange( commandId, request, result );
    MT::CScopeMutex lock( m_toolsLock );
    ++m_changeCalls[ request.operation ];
    m_lastRequests[ "apply:" + request.operation ] = request;
    result.Clear();
    TDocumentMap::const_iterator i = m_changes.find( request.operation );
    if ( i == m_changes.end() )
    {
        result.status = ASSETSTORE_STATUS_UNSUPPORTED;
        return result.status;
    }
    result = (*i).second.document;
    return (*i).second.status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CToolsFakeService::StartPipelineJob( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceResult& result )
{
    CFakeAssetStoreService::StartPipelineJob( commandId, request, result );
    MT::CScopeMutex lock( m_toolsLock );
    result.Clear();
    // a command id seen before gives the job of that start
    TJobMap::const_iterator seen = m_jobs.find( "cmd:" + commandId );
    if ( !commandId.IsNULLOrEmpty() && seen != m_jobs.end() )
    {
        result.status = ASSETSTORE_STATUS_OK;
        result.id = (*seen).second.jobId;
        result.replayed = true;
        return result.status;
    }
    SJob job;
    job.jobId = "pipeline:pj" + CORE::ToString( ++m_jobCounter );
    job.step = 0;
    job.cancelled = false;
    job.request = request;
    m_jobs[ job.jobId ] = job;
    if ( !commandId.IsNULLOrEmpty() )
        m_jobs[ "cmd:" + commandId ] = job;
    result.status = ASSETSTORE_STATUS_OK;
    result.id = job.jobId;
    return result.status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CToolsFakeService::GetJob( const CORE::CString& jobId, CAssetJobEntry& result )
{
    CFakeAssetStoreService::GetJob( jobId, result );
    MT::CScopeMutex lock( m_toolsLock );
    TJobMap::iterator i = m_jobs.find( jobId );
    if ( i == m_jobs.end() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    SJob& job = (*i).second;
    result.Clear();
    result.jobId = jobId;
    result.kind = "pipeline";
    result.packId = job.request.packId;
    result.detail = job.request.operation;
    if ( job.cancelled )
    {
        result.state = ASSETJOB_STATE_CANCELLED;
        result.message = "cancelled";
        return ASSETSTORE_STATUS_OK;
    }
    if ( job.step < m_jobSteps || m_jobUntilCancelled )
    {
        if ( job.step < m_jobSteps )
            ++job.step;
        result.state = ASSETJOB_STATE_RUNNING;
        result.itemsDone = job.step;
        result.itemsTotal = m_jobSteps;
        result.message = m_jobProgressMessage.IsNULLOrEmpty() ? "asset" + CORE::ToString( job.step ) : m_jobProgressMessage;
        return ASSETSTORE_STATUS_OK;
    }
    result.state = m_jobFinalState;
    result.itemsDone = m_jobSteps;
    result.itemsTotal = m_jobSteps;
    result.message = ASSETJOB_STATE_DONE == m_jobFinalState ? CORE::CString::Empty : m_jobProgressMessage;
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CToolsFakeService::ControlJob( const CORE::CString& commandId, const CORE::CString& jobId, TAssetJobAction action, CAssetServiceResult& result )
{
    CFakeAssetStoreService::ControlJob( commandId, jobId, action, result );
    MT::CScopeMutex lock( m_toolsLock );
    result.Clear();
    TJobMap::iterator i = m_jobs.find( jobId );
    if ( i == m_jobs.end() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    if ( ASSETJOB_ACTION_CANCEL == action )
    {
        (*i).second.cancelled = true;
        ++m_cancels;
    }
    result.status = ASSETSTORE_STATUS_OK;
    return result.status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CToolsFakeService::GetJobResult( const CORE::CString& jobId, const CORE::CString& section, const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result )
{
    CFakeAssetStoreService::GetJobResult( jobId, section, pageToken, maxEntries, result );
    MT::CScopeMutex lock( m_toolsLock );
    result.Clear();
    if ( m_jobs.end() == m_jobs.find( jobId ) )
        return ASSETSTORE_STATUS_NOT_FOUND;
    result.status = ASSETSTORE_STATUS_OK;
    result.jobId = jobId;
    result.document = m_jobResult;
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CToolsFakeService::RenderSheet( const CAssetPipelineRequest& request, CAssetServiceDocument& result )
{
    CFakeAssetStoreService::RenderSheet( request, result );
    MT::CScopeMutex lock( m_toolsLock );
    result.Clear();
    if ( !m_renders )
    {
        result.status = ASSETSTORE_STATUS_UNSUPPORTED;
        result.message = "no renderer is given to the service";
        return result.status;
    }
    const CORE::CDataNode* targets = request.arguments.FindChild( "targets" );
    const UInt32 count = GUCEF_NULL != targets ? targets->GetNrOfDirectChildNodes() : 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CAssetThumbnail image;
        image.kind = "sheet";
        image.status = ASSETSTORE_STATUS_OK;
        MakePng( 16, 16, false, image.encoded );
        result.images.push_back( image );
        ++m_renderedTargets;
    }
    result.status = ASSETSTORE_STATUS_OK;
    return result.status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CToolsFakeService::GetLicence( const CORE::CString& licenceOrPackId, CAssetLicence& result )
{
    CFakeAssetStoreService::GetLicence( licenceOrPackId, result );
    MT::CScopeMutex lock( m_toolsLock );
    GUCEF::map< CORE::CString, CAssetLicence >::const_iterator i = m_licences.find( licenceOrPackId );
    if ( i == m_licences.end() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    result = (*i).second;
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CToolsFakeService::GetAsset( const CORE::CString& assetId, CAssetRecord& result )
{
    CFakeAssetStoreService::GetAsset( assetId, result );
    MT::CScopeMutex lock( m_toolsLock );
    GUCEF::map< CORE::CString, CAssetRecord >::const_iterator i = m_assets.find( assetId );
    if ( i == m_assets.end() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    result = (*i).second;
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CToolsFakeService::GetPack( const CORE::CString& packId, CAssetPackRecord& result )
{
    CFakeAssetStoreService::GetPack( packId, result );
    MT::CScopeMutex lock( m_toolsLock );
    GUCEF::map< CORE::CString, CAssetPackRecord >::const_iterator i = m_packs.find( packId );
    if ( i == m_packs.end() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    result = (*i).second;
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/
