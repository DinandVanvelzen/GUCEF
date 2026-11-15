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

#ifndef GUCEF_CORE_CJOBCONTROLCOMMAND_H
#define GUCEF_CORE_CJOBCONTROLCOMMAND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  A control command for CJobScheduler::Control(). The remote layer (gucefDRNUTILS) checks roles, rate limits and
 *  idempotency before it hands a command to the scheduler; the scheduler only checks the command against its model.
 */
struct GUCEF_CORE_PUBLIC_CPP SJobControlCommand
{
    typedef CString::StringMap TArgumentMap;

    TJobControlAction action;
    TJobControlTargetKind targetKind;
    CString targetId;                   /**< job id, lane name, run id (decimal) or host name */
    CString customName;                 /**< CUSTOM: the action name */
    TArgumentMap arguments;             /**< CUSTOM: the arguments */
    bool hasPriority;                   /**< SET_PRIORITY, and RUN_NOW at a priority other than the job's */
    UInt32 priority;
    CString mode;                       /**< SET_LANE_MODE */
    CString reason;                     /**< free text for the audit trail */
    UInt64 expectedRevision;            /**< 0 = no check; else ABORTED when the target's revision differs */
    bool dryRun;                        /**< check only, change nothing */
    CString requestedBy;                /**< subject of the principal; becomes SJobRunRecord::requestedBy */

    SJobControlCommand( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBCONTROLCOMMAND_H ? */
