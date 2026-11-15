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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#include "gucefASSETSTORE_CAssetStoreDownloadManager.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H ? */
#include "gucefASSETSTORE_CAssetStoreTransferContext.h"

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

CAssetStoreTransferContext::CAssetStoreTransferContext( CAssetStoreDownloadManager& manager ,
                                                        const CORE::CString& jobId          ,
                                                        UInt32 fileIndex                    ,
                                                        const CORE::CString& partFilePath   ,
                                                        UInt64 resumeOffset                 ,
                                                        UInt64 maxBytesPerSecond            )
    : m_manager( manager )
    , m_jobId( jobId )
    , m_fileIndex( fileIndex )
    , m_partFilePath( partFilePath )
    , m_resumeOffset( resumeOffset )
    , m_maxBytesPerSecond( maxBytesPerSecond )
    , m_startTicks( MT::PrecisionTickCount() )
    , m_retryAfterInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreTransferContext::GetJobId( void ) const
{GUCEF_TRACE;

    return m_jobId;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreTransferContext::GetPartFilePath( void ) const
{GUCEF_TRACE;

    return m_partFilePath;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetStoreTransferContext::GetResumeOffset( void ) const
{GUCEF_TRACE;

    return m_resumeOffset;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferContext::IsStopRequested( void ) const
{GUCEF_TRACE;

    return m_manager.IsJobStopRequested( m_jobId );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferContext::ReportFileBytes( UInt64 bytesInFile )
{GUCEF_TRACE;

    m_manager.OnTransferProgress( m_jobId, m_fileIndex, bytesInFile );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferContext::Throttle( UInt64 bytesInFile )
{GUCEF_TRACE;

    if ( 0 == m_maxBytesPerSecond || bytesInFile <= m_resumeOffset )
        return;
    const UInt64 resolution = MT::PrecisionTimerResolution();
    if ( 0 == resolution )
        return;
    const UInt64 expectedMs = ( bytesInFile - m_resumeOffset ) * 1000 / m_maxBytesPerSecond;
    for ( ;; )
    {
        const UInt64 elapsedMs = ( MT::PrecisionTickCount() - m_startTicks ) * 1000 / resolution;
        if ( elapsedMs >= expectedMs || IsStopRequested() )
            return;
        const UInt64 waitMs = expectedMs - elapsedMs;
        MT::ThreadDelay( waitMs > 10 ? 10 : (UInt32) waitMs );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferContext::SetRetryAfterInMs( UInt32 delayInMs )
{GUCEF_TRACE;

    m_retryAfterInMs = delayInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreTransferContext::GetRetryAfterInMs( void ) const
{GUCEF_TRACE;

    return m_retryAfterInMs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
