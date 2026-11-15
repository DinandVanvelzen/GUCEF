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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */
#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */
#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETANALYSISTASKCONSUMER_H
#include "gucefASSETSTORE_CAssetAnalysisTaskConsumer.h"
#define GUCEF_ASSETSTORE_CASSETANALYSISTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CASSETANALYSISTASKCONSUMER_H ? */
#include "gucefASSETSTORE_CAssetAnalysisBatch.h"

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

CAssetAnalysisBatch::CAssetAnalysisBatch( CAssetContentAnalyzerRegistry& registry, TAssetRecordVector& assets, const CORE::CString::StringVector& vfsPaths )
    : m_registry( registry )
    , m_assets( assets )
    , m_paths( vfsPaths )
    , m_next( 0 )
    , m_status( ASSETSTORE_STATUS_OK )
    , m_message()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetAnalysisBatch::TakeNext( UInt32& index )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    while ( m_next < (UInt32) m_assets.size() && m_assets[ m_next ].analysed )
        ++m_next;
    if ( m_next >= (UInt32) m_assets.size() )
        return false;
    index = m_next++;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetAnalysisBatch::AnalyzeAsset( UInt32 index )
{GUCEF_TRACE;

    // every index is worked on by one thread: the asset is not shared
    CAssetRecord& asset = m_assets[ index ];
    TAssetContentAnalyzerPtrVector analyzers;
    m_registry.GetAnalyzersFor( asset, analyzers );
    for ( size_t a=0; a<analyzers.size(); ++a )
    {
        TAssetStoreStatus status = ASSETSTORE_STATUS_FAILED;
        try
        {
            status = analyzers[ a ]->Analyze( index < m_paths.size() ? m_paths[ index ] : CORE::CString::Empty, asset );
        }
        catch ( ... )
        {
            status = ASSETSTORE_STATUS_FAILED;
        }
        if ( ASSETSTORE_STATUS_OK == status )
        {
            asset.analyzers.insert( analyzers[ a ]->GetName() + "/" + analyzers[ a ]->GetVersion() );
            continue;
        }
        if ( ASSETSTORE_STATUS_UNSUPPORTED == status )
            continue;

        MT::CScopeMutex lock( m_lock );
        if ( ASSETSTORE_STATUS_OK == m_status )
        {
            m_status = status;
            m_message = "the analyser " + analyzers[ a ]->GetName() + " failed on " + asset.originalPath + " (" + AssetStoreStatusToString( status ) + ")";
        }
        return;
    }
    asset.analysed = true;
}

/*-------------------------------------------------------------------------*/

void
CAssetAnalysisBatch::Work( void )
{GUCEF_TRACE;

    UInt32 index = 0;
    while ( TakeNext( index ) )
        AnalyzeAsset( index );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetAnalysisBatch::Run( const CORE::CString& threadPoolName, UInt32 concurrency )
{GUCEF_TRACE;

    GUCEF::vector< CORE::CFutureResult > futures;
    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( threadPoolName );
    if ( !threadPool.IsNULL() && concurrency > 1 && m_assets.size() > 1 )
    {
        if ( threadPool->GetDesiredMinNrOfWorkerThreads() < concurrency )
            threadPool->SetDesiredMinNrOfWorkerThreads( concurrency );
        const UInt32 tasks = concurrency < (UInt32) m_assets.size() ? concurrency : (UInt32) m_assets.size();
        for ( UInt32 t=0; t<tasks; ++t )
        {
            CAssetAnalysisTaskData* data = GUCEF_NEW CAssetAnalysisTaskData();
            data->batch = this;
            CORE::CFutureResult future = threadPool->QueueTask( CAssetAnalysisTaskConsumer::TaskType, data, GUCEF_NULL, true );
            if ( future.HasNoFuture() )
            {
                GUCEF_DELETE data;
                break;
            }
            futures.push_back( future );
        }
    }

    // the calling thread works as well, then waits for the tasks
    Work();
    for ( size_t f=0; f<futures.size(); ++f )
    {
        while ( !futures[ f ].IsReady() && futures[ f ].HasAFuture() )
            MT::ThreadDelay( 2 );
    }
    return GetStatus();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetAnalysisBatch::GetStatus( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_status;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetAnalysisBatch::GetMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_message;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
