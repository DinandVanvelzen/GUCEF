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

#ifndef GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H
#define GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAssetStoreDownloadManager;

/**
 *  What a transfer of a download job works with, on the thread of the job: the ".part" file it writes (a VFS path),
 *  the offset to resume from (the bytes the file has already), whether it has to stop (the job is paused, cancelled or
 *  the manager stops), where it reports the bytes the file has, and the throttle of the settings.
 *  Given to CIAssetStoreTransferSource::RunTransfer() and used by the HTTP transfers of the download manager.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreTransferContext
{
    public:

    CAssetStoreTransferContext( CAssetStoreDownloadManager& manager ,
                                const CORE::CString& jobId          ,
                                UInt32 fileIndex                    ,
                                const CORE::CString& partFilePath   ,
                                UInt64 resumeOffset                 ,
                                UInt64 maxBytesPerSecond            );

    const CORE::CString& GetJobId( void ) const;

    /** The VFS path of the ".part" file */
    const CORE::CString& GetPartFilePath( void ) const;

    /** The bytes the ".part" file had when the transfer started */
    UInt64 GetResumeOffset( void ) const;

    /** True when the transfer has to stop as soon as it can */
    bool IsStopRequested( void ) const;

    /** Reports how many bytes the ".part" file has now */
    void ReportFileBytes( UInt64 bytesInFile );

    /** Waits as long as the throttle of the settings asks for the bytes the file has now; returns at once without one */
    void Throttle( UInt64 bytesInFile );

    /** A delay the store asked for (Retry-After), for the next attempt */
    void SetRetryAfterInMs( UInt32 delayInMs );
    UInt32 GetRetryAfterInMs( void ) const;

    private:

    CAssetStoreTransferContext( const CAssetStoreTransferContext& src );              /**< not implemented */
    CAssetStoreTransferContext& operator=( const CAssetStoreTransferContext& src );   /**< not implemented */

    private:

    CAssetStoreDownloadManager& m_manager;
    CORE::CString m_jobId;
    UInt32 m_fileIndex;
    CORE::CString m_partFilePath;
    UInt64 m_resumeOffset;
    UInt64 m_maxBytesPerSecond;
    UInt64 m_startTicks;
    UInt32 m_retryAfterInMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H ? */
