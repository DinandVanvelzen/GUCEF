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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMER_H
#define GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

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

class CAssetToolContext;

/**
 *  Carries out a job of the asset tools as a run of the job scheduler, by driving the job of the service (section 6.3):
 *
 *  - OnTaskStart() starts it: StartPipelineJob() for convert_pack, convert_file, cook_pack and validate_pack,
 *    IngestAsync() for store_ingest, ResumeIngestAsync() for store_ingest_resume, with the command id of the job; a
 *    start the service refuses fails the run with "<errorCode>: <message>";
 *  - every cycle looks at the job of the service (GetJob()), reports its progress into the run (redacted) and ends when
 *    the job of the service ended; the run then names the job of the service as its result document
 *    ("assetjob:<service job id>", or "assetjob!<status>:<message>" for an ingest that failed without an id), which
 *    job_result reads with GetJobResult();
 *  - a cancellation of the run cancels the job of the service (ControlJob( CANCEL )) and returns false: the delegator
 *    then ends the task CANCELLED and the run CANCELLED. Returning true would end it FINISHED (SUCCEEDED).
 *
 *  A run that ends while the job of the service failed or was cancelled on its own is SUCCEEDED for the scheduler;
 *  the tools give the state of the job of the service then (CAssetToolJobResultBuilder).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolJobTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString ResultReferencePrefix;   /**< "assetjob:" */
    static const CORE::CString FailureReferencePrefix;  /**< "assetjob!" */

    /** For the factory template only: a task without a context fails at its start */
    CAssetToolJobTaskConsumer( void );

    CAssetToolJobTaskConsumer( CAssetToolContext* context, const CORE::CString& taskTypeName );

    virtual ~CAssetToolJobTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAssetToolJobTaskConsumer( const CAssetToolJobTaskConsumer& src );              /**< not implemented */
    CAssetToolJobTaskConsumer& operator=( const CAssetToolJobTaskConsumer& src );   /**< not implemented */

    bool CycleIngest( CORE::CTaskPtr task );

    /** Tells the job of the service to stop, once */
    void CancelServiceJob( void );

    CAssetToolContext* m_context;
    CORE::CString m_taskTypeName;
    CORE::CString m_kind;
    CORE::CString m_commandId;
    CORE::CString m_serviceJobId;
    bool m_isIngest;
    bool m_reported;
    bool m_cancelSent;
    bool m_serviceEnded;
    CORE::CFutureResult m_future;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMER_H ? */
