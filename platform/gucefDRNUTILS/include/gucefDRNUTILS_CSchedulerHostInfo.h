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

#ifndef GUCEF_DRNUTILS_CSCHEDULERHOSTINFO_H
#define GUCEF_DRNUTILS_CSCHEDULERHOSTINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_JOBSCHEDULERTYPES_H
#include "gucefCORE_JobSchedulerTypes.h"
#define GUCEF_CORE_JOBSCHEDULERTYPES_H
#endif /* GUCEF_CORE_JOBSCHEDULERTYPES_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERACTIONINFO_H
#include "gucefDRNUTILS_CSchedulerActionInfo.h"
#define GUCEF_DRNUTILS_CSCHEDULERACTIONINFO_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERACTIONINFO_H ? */

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
 *  What Scheduler/GetInfo answers (contract type HostInfo), as the host builds it and as the client keeps it
 */
struct GUCEF_DRNUTILS_PUBLIC_CPP SSchedulerHostInfo
{
    CORE::CString hostName;
    UInt64 hostInstanceId;
    UInt64 revision;
    Int64 serverTimeUtcMs;
    CORE::CString state;                            /**< JobSchedulerHostStateToString() */
    TSchedulerActionInfoVector allowedActions;      /**< filtered by the viewer's roles */
    CORE::CString::StringMap tableNames;            /**< table name ("jobs", "lanes", "runs") to grid name */
    CORE::CString::StringSet submittableTaskTypes;  /**< empty unless the viewer may submit */
    UInt32 maxSubmitPayloadBytes;

    SSchedulerHostInfo( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CSCHEDULERHOSTINFO_H ? */
