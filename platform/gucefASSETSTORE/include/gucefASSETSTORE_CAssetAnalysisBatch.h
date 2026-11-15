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

#ifndef GUCEF_ASSETSTORE_CASSETANALYSISBATCH_H
#define GUCEF_ASSETSTORE_CASSETANALYSISBATCH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H
#include "gucefASSETSTORE_CAssetContentAnalyzerRegistry.h"
#define GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H ? */
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

/**
 *  The content analysis of the assets of one ingest (section 7.3), as pool tasks bounded in number: Run() queues at
 *  most 'concurrency' tasks of CAssetAnalysisTaskConsumer that take the assets one by one, waits for them on the calling thread
 *  and analyses what is left there. Assets that were analysed before (a resumed ingest) are skipped. Every analyser that
 *  wants an asset looks at it; an asset is analysed when none of them failed. The first failure is kept.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetAnalysisBatch
{
    public:

    /** The assets and the VFS paths of their files, index by index; both outlive the batch */
    CAssetAnalysisBatch( CAssetContentAnalyzerRegistry& registry, TAssetRecordVector& assets, const CORE::CString::StringVector& vfsPaths );

    TAssetStoreStatus Run( const CORE::CString& threadPoolName, UInt32 concurrency );

    /** Analyses assets until none is left; what the tasks call */
    void Work( void );

    TAssetStoreStatus GetStatus( void ) const;
    CORE::CString GetMessage( void ) const;

    private:

    bool TakeNext( UInt32& index );
    void AnalyzeAsset( UInt32 index );

    CAssetAnalysisBatch( const CAssetAnalysisBatch& src );              /**< not implemented */
    CAssetAnalysisBatch& operator=( const CAssetAnalysisBatch& src );   /**< not implemented */

    private:

    CAssetContentAnalyzerRegistry& m_registry;
    TAssetRecordVector& m_assets;
    const CORE::CString::StringVector& m_paths;
    UInt32 m_next;
    TAssetStoreStatus m_status;
    CORE::CString m_message;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETANALYSISBATCH_H ? */
