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

#ifndef GUCEF_DRNUTILS_CSCHEDULERJOBDETAILANSWER_H
#define GUCEF_DRNUTILS_CSCHEDULERJOBDETAILANSWER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_DRNUTILS_DRNUTILSTYPES_H
#include "gucefDRNUTILS_DrnUtilsTypes.h"
#define GUCEF_DRNUTILS_DRNUTILSTYPES_H
#endif /* GUCEF_DRNUTILS_DRNUTILSTYPES_H ? */

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
 *  The answer of Scheduler/GetJob to CSchedulerClient::RequestJobDetail() (JobDetailReceivedEvent): the job row with the
 *  actions this viewer may use, its recent runs, the status of its work source when asked for, its description and, when
 *  the job exposes it, its payload. On a failure only requestId, jobId, status and statusMessage are set
 */
struct GUCEF_DRNUTILS_PUBLIC_CPP SSchedulerJobDetailAnswer
{
    CORE::CString requestId;                        /**< what RequestJobDetail() returned */
    CORE::CString jobId;
    TDrnUtilsCommandStatus status;
    CORE::CString statusMessage;
    SSchedulerJobDetail detail;                     /**< detail.job.description and detail.job.payload as sent */
    TSchedulerActionInfoVector allowedActions;      /**< the actions this viewer may use on the job (JobRow.allowedActions) */

    SSchedulerJobDetailAnswer( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CSCHEDULERJOBDETAILANSWER_H ? */
