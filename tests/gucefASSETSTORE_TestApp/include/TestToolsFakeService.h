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

#ifndef TESTTOOLSFAKESERVICE_H
#define TESTTOOLSFAKESERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef TESTFAKEASSETSTORESERVICE_H
#include "TestFakeAssetStoreService.h"
#define TESTFAKEASSETSTORESERVICE_H
#endif /* TESTFAKEASSETSTORESERVICE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The fake service of the tests of the asset tool set: CFakeAssetStoreService (every call recorded, answers scripted
 *  per service operation) with what the tools need beyond it: answers per pipeline operation (RunPipelineQuery() and
 *  ApplyPipelineChange() by request.operation), licences, assets and packs by id, rendered sheets, and pipeline jobs that
 *  step on every GetJob() (progress), end in a scripted state with a scripted result document, or run until they are
 *  cancelled (ControlJob( CANCEL )).
 */
class CToolsFakeService : public CFakeAssetStoreService
{
    public:

    CToolsFakeService( void );

    virtual ~CToolsFakeService();

    void SetPipelineAnswer( const GUCEF::CORE::CString& operation, GUCEF::ASSETSTORE::TAssetStoreStatus status, const GUCEF::CORE::CDataNode& document,
                            const GUCEF::CORE::CString& message = GUCEF::CORE::CString::Empty, const GUCEF::CORE::CString& nextPageToken = GUCEF::CORE::CString::Empty,
                            const GUCEF::CORE::CString& changeToken = GUCEF::CORE::CString::Empty );

    void SetChangeAnswer( const GUCEF::CORE::CString& operation, GUCEF::ASSETSTORE::TAssetStoreStatus status, const GUCEF::CORE::CDataNode& document,
                          const GUCEF::CORE::CString& message = GUCEF::CORE::CString::Empty );

    /** The licence decision of licence_check for a converter pack: allowed, or withheld with the reason */
    void SetConverterLicence( const GUCEF::CORE::CString& packId, bool conversionAllowed, bool contentAllowed, const GUCEF::CORE::CString& reason );

    void SetLicence( const GUCEF::CORE::CString& id, const GUCEF::ASSETSTORE::CAssetLicence& licence );
    void SetAsset( const GUCEF::ASSETSTORE::CAssetRecord& asset );
    void SetPack( const GUCEF::ASSETSTORE::CAssetPackRecord& pack );

    /** RenderSheet() answers OK with a PNG per target it is given (true), or UNSUPPORTED (false, the default) */
    void SetRenders( bool renders );
    GUCEF::UInt32 GetRenderedTargetCount( void ) const;

    /** The pipeline jobs: 'steps' GetJob() calls of progress, then the final state; or until cancelled */
    void SetJobScript( GUCEF::UInt32 steps, GUCEF::ASSETSTORE::TAssetJobState finalState, const GUCEF::CORE::CDataNode& resultDocument,
                       bool runUntilCancelled, const GUCEF::CORE::CString& progressMessage = GUCEF::CORE::CString::Empty );

    GUCEF::UInt32 CountPipelineCalls( const GUCEF::CORE::CString& operation ) const;
    GUCEF::UInt32 CountChangeCalls( const GUCEF::CORE::CString& operation ) const;
    GUCEF::UInt32 GetStartedJobCount( void ) const;
    GUCEF::UInt32 GetCancelCount( void ) const;
    bool GetLastPipelineRequest( const GUCEF::CORE::CString& operation, GUCEF::ASSETSTORE::CAssetPipelineRequest& request ) const;

    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RunPipelineQuery( const GUCEF::ASSETSTORE::CAssetPipelineRequest& request, GUCEF::ASSETSTORE::CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ApplyPipelineChange( const GUCEF::CORE::CString& commandId, const GUCEF::ASSETSTORE::CAssetPipelineRequest& request, GUCEF::ASSETSTORE::CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus StartPipelineJob( const GUCEF::CORE::CString& commandId, const GUCEF::ASSETSTORE::CAssetPipelineRequest& request, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetJob( const GUCEF::CORE::CString& jobId, GUCEF::ASSETSTORE::CAssetJobEntry& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ControlJob( const GUCEF::CORE::CString& commandId, const GUCEF::CORE::CString& jobId, GUCEF::ASSETSTORE::TAssetJobAction action, GUCEF::ASSETSTORE::CAssetServiceResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetJobResult( const GUCEF::CORE::CString& jobId, const GUCEF::CORE::CString& section, const GUCEF::CORE::CString& pageToken, GUCEF::UInt32 maxEntries, GUCEF::ASSETSTORE::CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RenderSheet( const GUCEF::ASSETSTORE::CAssetPipelineRequest& request, GUCEF::ASSETSTORE::CAssetServiceDocument& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetLicence( const GUCEF::CORE::CString& licenceOrPackId, GUCEF::ASSETSTORE::CAssetLicence& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetAsset( const GUCEF::CORE::CString& assetId, GUCEF::ASSETSTORE::CAssetRecord& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetPack( const GUCEF::CORE::CString& packId, GUCEF::ASSETSTORE::CAssetPackRecord& result ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SAnswerDocument
    {
        GUCEF::ASSETSTORE::TAssetStoreStatus status;
        GUCEF::ASSETSTORE::CAssetServiceDocument document;
    };
    typedef GUCEF::map< GUCEF::CORE::CString, SAnswerDocument > TDocumentMap;

    struct SJob
    {
        GUCEF::CORE::CString jobId;
        GUCEF::UInt32 step;
        bool cancelled;
        GUCEF::ASSETSTORE::CAssetPipelineRequest request;
    };
    typedef GUCEF::map< GUCEF::CORE::CString, SJob > TJobMap;

    CToolsFakeService( const CToolsFakeService& src );
    CToolsFakeService& operator=( const CToolsFakeService& src );

    mutable GUCEF::MT::CMutex m_toolsLock;
    TDocumentMap m_pipeline;
    TDocumentMap m_changes;
    GUCEF::map< GUCEF::CORE::CString, GUCEF::UInt32 > m_pipelineCalls;
    GUCEF::map< GUCEF::CORE::CString, GUCEF::UInt32 > m_changeCalls;
    GUCEF::map< GUCEF::CORE::CString, GUCEF::ASSETSTORE::CAssetPipelineRequest > m_lastRequests;
    GUCEF::map< GUCEF::CORE::CString, GUCEF::ASSETSTORE::CAssetLicence > m_licences;
    GUCEF::map< GUCEF::CORE::CString, GUCEF::ASSETSTORE::CAssetRecord > m_assets;
    GUCEF::map< GUCEF::CORE::CString, GUCEF::ASSETSTORE::CAssetPackRecord > m_packs;
    bool m_renders;
    GUCEF::UInt32 m_renderedTargets;
    GUCEF::UInt32 m_jobSteps;
    GUCEF::ASSETSTORE::TAssetJobState m_jobFinalState;
    GUCEF::CORE::CDataNode m_jobResult;
    bool m_jobUntilCancelled;
    GUCEF::CORE::CString m_jobProgressMessage;
    TJobMap m_jobs;
    GUCEF::UInt32 m_jobCounter;
    GUCEF::UInt32 m_cancels;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTTOOLSFAKESERVICE_H ? */
