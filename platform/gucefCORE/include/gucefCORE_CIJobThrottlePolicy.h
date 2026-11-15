/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CIJOBTHROTTLEPOLICY_H
#define GUCEF_CORE_CIJOBTHROTTLEPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_JOBSCHEDULERTYPES_H
#include "gucefCORE_JobSchedulerTypes.h"
#define GUCEF_CORE_JOBSCHEDULERTYPES_H
#endif /* GUCEF_CORE_JOBSCHEDULERTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What the scheduler asks a throttle policy before it starts work on a lane */
struct GUCEF_CORE_PUBLIC_CPP SJobPermitRequest
{
    CString laneName;
    CString jobId;
    CString itemKey;
    TJobRunCause cause;                 /**< HOLD for the use of a lane by a hold holder */
    UInt32 priority;                    /**< 0 = highest */
    UInt32 estimatedCost;               /**< requests, units; 1 by default */

    SJobPermitRequest( void );
};

/*-------------------------------------------------------------------------*/

/** What a throttle policy answers */
struct GUCEF_CORE_PUBLIC_CPP SJobPermitAnswer
{
    bool granted;
    UInt64 waitMs;                      /**< when not granted: the earliest time worth asking again (a lower bound is allowed) */
    CString bindingLimit;               /**< "requestsPerMinute", "dayBudget", "pace", ...: shown as waitingFor */

    SJobPermitAnswer( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  Decides whether work of a lane may start now. Called on the dispatcher thread under the scheduler lock: pure
 *  bookkeeping, no I/O, no notifier, no pulse generator, no other lock than a leaf lock of its own.
 *  Built in: CSlidingWindowThrottlePolicy and CDailyBudgetPacingPolicy (gucefCORE).
 */
class GUCEF_CORE_PUBLIC_CPP CIJobThrottlePolicy
{
    public:

    CIJobThrottlePolicy( void );

    virtual ~CIJobThrottlePolicy();

    virtual void Check( const SJobPermitRequest& request, Int64 nowUtcMs, UInt64 nowMonotonicMs, SJobPermitAnswer& answer ) = 0;

    virtual void OnStarted( const SJobPermitRequest& request, Int64 nowUtcMs, UInt64 nowMonotonicMs ) = 0;

    virtual void OnFinished( const SJobPermitRequest& request, UInt32 actualCost, TJobRunState outcome, UInt64 retryAfterMs, Int64 nowUtcMs ) = 0;

    /** A run-now of the lane: clear own waits where that makes sense. Default: nothing */
    virtual void OnRunNow( Int64 nowUtcMs );

    /** Windows, use, remainder: for the views (flattened into the lane table as throttle.<name> columns) */
    virtual void GetStatus( CDataNode& status, Int64 nowUtcMs ) const = 0;

    /** Default: nothing to keep */
    virtual bool SaveState( CDataNode& state ) const;

    /** Default: nothing to restore */
    virtual bool LoadState( const CDataNode& state, CString& error );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIJOBTHROTTLEPOLICY_H ? */
