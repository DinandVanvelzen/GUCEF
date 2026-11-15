/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CJOBSCHEDULERSOURCE_H
#define GUCEF_DRNUTILS_CJOBSCHEDULERSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULER_H
#include "gucefCORE_CJobScheduler.h"
#define GUCEF_CORE_CJOBSCHEDULER_H
#endif /* GUCEF_CORE_CJOBSCHEDULER_H ? */

#ifndef GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#include "gucefDRNUTILS_CISchedulerSource.h"
#define GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#endif /* GUCEF_DRNUTILS_CISCHEDULERSOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

class CJobSchedulerSourceRelay;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  CISchedulerSource over a CORE::CJobScheduler obtained from the task manager (CTaskManager::GetJobScheduler()).
 *  Turns the scheduler's events into ChangedEvent on the given notification pulse generator.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CJobSchedulerSource : public CISchedulerSource
{
    public:

    CJobSchedulerSource( const CORE::JobSchedulerPtr& scheduler, const CORE::PulseGeneratorPtr& notificationPulseGenerator );

    virtual ~CJobSchedulerSource() GUCEF_VIRTUAL_OVERRIDE;

    CORE::JobSchedulerPtr GetScheduler( void ) const;

    virtual const CORE::CString& GetHostName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetHostInstanceId( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetSnapshot( CORE::CJobSchedulerSnapshot& snapshot, UInt64 changedAfterRevision ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetJobDetail( const CORE::CString& jobId, UInt32 recentRuns, bool includeSourceStatus, SSchedulerJobDetail& detail ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetDeclaredActions( TSchedulerActionInfoVector& actions ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TJobControlStatus Apply( const CORE::SJobControlCommand& command, CORE::SJobControlResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetSubmittableTaskTypes( CORE::CString::StringSet& taskTypeNames ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TJobControlStatus Submit( const CORE::SJobSubmission& submission, CORE::SJobControlResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CNotifier* GetChangeNotifier( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CJobSchedulerSource( const CJobSchedulerSource& src );              /**< not implemented */
    CJobSchedulerSource& operator=( const CJobSchedulerSource& src );   /**< not implemented */

    CORE::JobSchedulerPtr m_scheduler;
    CJobSchedulerSourceRelay* m_relay;     /**< raises ChangedEvent from the scheduler's events */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CJOBSCHEDULERSOURCE_H ? */
