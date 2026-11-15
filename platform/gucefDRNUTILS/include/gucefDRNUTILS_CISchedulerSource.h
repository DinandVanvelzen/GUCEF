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

#ifndef GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#define GUCEF_DRNUTILS_CISCHEDULERSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULERSNAPSHOT_H
#include "gucefCORE_CJobSchedulerSnapshot.h"
#define GUCEF_CORE_CJOBSCHEDULERSNAPSHOT_H
#endif /* GUCEF_CORE_CJOBSCHEDULERSNAPSHOT_H ? */

#ifndef GUCEF_CORE_CJOBCONTROLCOMMAND_H
#include "gucefCORE_CJobControlCommand.h"
#define GUCEF_CORE_CJOBCONTROLCOMMAND_H
#endif /* GUCEF_CORE_CJOBCONTROLCOMMAND_H ? */

#ifndef GUCEF_CORE_CJOBCONTROLRESULT_H
#include "gucefCORE_CJobControlResult.h"
#define GUCEF_CORE_CJOBCONTROLRESULT_H
#endif /* GUCEF_CORE_CJOBCONTROLRESULT_H ? */

#ifndef GUCEF_CORE_CJOBSUBMISSION_H
#include "gucefCORE_CJobSubmission.h"
#define GUCEF_CORE_CJOBSUBMISSION_H
#endif /* GUCEF_CORE_CJOBSUBMISSION_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERACTIONINFO_H
#include "gucefDRNUTILS_CSchedulerActionInfo.h"
#define GUCEF_DRNUTILS_CSCHEDULERACTIONINFO_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERACTIONINFO_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERJOBDETAIL_H
#include "gucefDRNUTILS_CSchedulerJobDetail.h"
#define GUCEF_DRNUTILS_CSCHEDULERJOBDETAIL_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERJOBDETAIL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The seam between the remote layer and a scheduler: CSchedulerHost, the publishers and the snapshot tables work
 *  against this interface, so a service that keeps a scheduler of its own and the generic CORE::CJobScheduler
 *  (CJobSchedulerSource) are viewed, controlled and fed submitted jobs the same way.
 *  Apply() and Submit() run on the caller's thread with no lock of the remote layer held.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CISchedulerSource
{
    public:

    /** Raised on the service's notification thread by the notifier of GetChangeNotifier(); data TChangedEventData (the revision) */
    static const CORE::CEvent ChangedEvent;

    typedef CORE::TCloneableUInt64 TChangedEventData;

    static void RegisterEvents( void );

    CISchedulerSource( void );

    virtual ~CISchedulerSource();

    virtual const CORE::CString& GetHostName( void ) const = 0;

    virtual UInt64 GetHostInstanceId( void ) const = 0;

    /** A copy; with changedAfterRevision only the rows changed after it */
    virtual void GetSnapshot( CORE::CJobSchedulerSnapshot& snapshot, UInt64 changedAfterRevision ) const = 0;

    virtual bool GetJobDetail( const CORE::CString& jobId, UInt32 recentRuns, bool includeSourceStatus, SSchedulerJobDetail& detail ) const = 0;

    /** Every action the source supports (generic and custom); the host filters them by settings and roles */
    virtual void GetDeclaredActions( TSchedulerActionInfoVector& actions ) const = 0;

    virtual CORE::TJobControlStatus Apply( const CORE::SJobControlCommand& command, CORE::SJobControlResult& result ) = 0;

    /** The task types that may be submitted as data; none by default */
    virtual void GetSubmittableTaskTypes( CORE::CString::StringSet& taskTypeNames ) const;

    /** Default: FAILED_PRECONDITION (the source accepts no submissions) */
    virtual CORE::TJobControlStatus Submit( const CORE::SJobSubmission& submission, CORE::SJobControlResult& result );

    virtual CORE::CNotifier* GetChangeNotifier( void ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CISCHEDULERSOURCE_H ? */
