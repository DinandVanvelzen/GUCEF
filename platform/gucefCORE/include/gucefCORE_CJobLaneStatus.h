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

#ifndef GUCEF_CORE_CJOBLANESTATUS_H
#define GUCEF_CORE_CJOBLANESTATUS_H

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

/**
 *  How one lane stands, as a row of a CJobSchedulerSnapshot. A copy, taken under the scheduler lock.
 */
struct GUCEF_CORE_PUBLIC_CPP SJobLaneStatus
{
    typedef GUCEF::vector< UInt32 > TUInt32Vector;

    CString laneName;
    CString mode;
    CString::StringVector declaredModes;
    CString state;                      /**< "available", "unavailable", "held", "waiting", "disabled" */
    UInt32 inFlight;
    UInt32 maxConcurrent;
    TUInt32Vector queuedByPriority;     /**< queued runs per priority 0..3 */
    CString waitingFor;
    Int64 waitingUntilUtcMs;
    bool available;
    CString unavailableReason;
    CString heldBy;
    Int64 heldSinceUtcMs;
    CDataNode throttleStatus;           /**< CIJobThrottlePolicy::GetStatus() */
    UInt64 revision;

    SJobLaneStatus( void );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< SJobLaneStatus > TJobLaneStatusVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBLANESTATUS_H ? */
