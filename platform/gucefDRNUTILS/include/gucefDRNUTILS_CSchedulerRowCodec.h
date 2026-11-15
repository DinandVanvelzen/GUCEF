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

#ifndef GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H
#define GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CJOBSTATUS_H
#include "gucefCORE_CJobStatus.h"
#define GUCEF_CORE_CJOBSTATUS_H
#endif /* GUCEF_CORE_CJOBSTATUS_H ? */

#ifndef GUCEF_CORE_CJOBLANESTATUS_H
#include "gucefCORE_CJobLaneStatus.h"
#define GUCEF_CORE_CJOBLANESTATUS_H
#endif /* GUCEF_CORE_CJOBLANESTATUS_H ? */

#ifndef GUCEF_CORE_CJOBRUNRECORD_H
#include "gucefCORE_CJobRunRecord.h"
#define GUCEF_CORE_CJOBRUNRECORD_H
#endif /* GUCEF_CORE_CJOBRUNRECORD_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

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
 *  Module-private: the rows of gucef.drnutils.scheduler 1.0 (JobRow, LaneRow, RunRow, ActionInfo) and the cells of
 *  the jobs, lanes and runs grids from the scheduler's snapshot rows, and back (for the client). Texts are cut to
 *  their bounds so that a published row stays below the 8 KB of the publisher. Times: contract timestamps are
 *  microseconds in a message and UTC milliseconds everywhere else.
 */
class CSchedulerRowCodec
{
    public:

    /** Field numbers of the contract */
    enum EJobRowField { JOBROW_JOBID = 1, JOBROW_TITLE = 2, JOBROW_LANE = 3, JOBROW_STATE = 4, JOBROW_PRIORITY = 5, JOBROW_TRIGGER = 6,
                        JOBROW_NEXTDUE = 7, JOBROW_WAITINGFOR = 8, JOBROW_WAITINGUNTIL = 9, JOBROW_LASTRUNID = 12, JOBROW_LASTRUNSTATE = 13,
                        JOBROW_LASTRUNENDED = 14, JOBROW_RUNCOUNT = 15, JOBROW_FAILURECOUNT = 16, JOBROW_CONSECUTIVEFAILURES = 17,
                        JOBROW_REVISION = 18, JOBROW_ALLOWEDACTIONS = 19, JOBROW_TASKTYPE = 20, JOBROW_EXECUTIONTARGET = 21,
                        JOBROW_THREADPOOL = 22, JOBROW_ORIGIN = 23, JOBROW_SUBMITTEDBY = 24, JOBROW_UNAVAILABLEREASON = 25, JOBROW_ACTIVERUNS = 26 };
    enum ELaneRowField { LANEROW_LANE = 1, LANEROW_MODE = 2, LANEROW_STATE = 3, LANEROW_INFLIGHT = 4, LANEROW_MAXCONCURRENT = 5,
                         LANEROW_QUEUEDBYPRIORITY = 6, LANEROW_WAITINGFOR = 7, LANEROW_WAITINGUNTIL = 8, LANEROW_UNAVAILABLEREASON = 10,
                         LANEROW_HELDBY = 11, LANEROW_THROTTLE = 12, LANEROW_REVISION = 13 };
    enum ERunRowField { RUNROW_RUNID = 1, RUNROW_JOBID = 2, RUNROW_ITEMLABEL = 3, RUNROW_CAUSE = 4, RUNROW_STATE = 5, RUNROW_SCHEDULED = 6,
                        RUNROW_STARTED = 7, RUNROW_ENDED = 8, RUNROW_ATTEMPT = 9, RUNROW_PROGRESSDONE = 10, RUNROW_PROGRESSTOTAL = 11,
                        RUNROW_NOTE = 12, RUNROW_MESSAGE = 13, RUNROW_REQUESTEDBY = 14, RUNROW_TASKID = 15, RUNROW_REVISION = 16,
                        RUNROW_TASKTYPE = 19, RUNROW_EXECUTIONTARGET = 20, RUNROW_THREADPOOL = 21 };
    enum EActionInfoField { ACTIONINFO_ACTION = 1, ACTIONINFO_TARGETKIND = 2, ACTIONINFO_CUSTOMNAME = 3, ACTIONINFO_TITLE = 4, ACTIONINFO_ARGUMENTNAMES = 5 };

    static const UInt32 MaxIdBytes = 256;
    static const UInt32 MaxTextBytes = 512;
    static const char* RedactedText;        /**< what replaces notes and messages of sensitive jobs */
    static const UInt32 MaxThrottleColumns = 32;

    static const char* JobOriginToString( CORE::TJobOrigin origin );

    /** Cuts a text to at most maxBytes bytes of UTF-8 (whole code points) */
    static CORE::CString Bound( const CORE::CString& text, UInt32 maxBytes );

    /** A cell of a timestamp(ms,utc) column: null for 0 */
    static CORE::CVariant TimestampCell( Int64 utcMs );

    static void JobToMessage( const CORE::SJobStatus& job, COMCORE::CRpcMessage& row );
    static void LaneToMessage( const CORE::SJobLaneStatus& lane, COMCORE::CRpcMessage& row );
    static void RunToMessage( const CORE::SJobRunRecord& run, COMCORE::CRpcMessage& row );
    static void ActionToMessage( const SSchedulerActionInfo& action, COMCORE::CRpcMessage& row );

    static void JobFromMessage( const COMCORE::CRpcMessage& row, CORE::SJobStatus& job );
    static void LaneFromMessage( const COMCORE::CRpcMessage& row, CORE::SJobLaneStatus& lane );
    static void RunFromMessage( const COMCORE::CRpcMessage& row, CORE::SJobRunRecord& run );
    static void ActionFromMessage( const COMCORE::CRpcMessage& row, SSchedulerActionInfo& action );

    /** The cells of the jobs grid without the key column (jobId) */
    static void JobToCells( const CORE::SJobStatus& job, CORE::CVariant::VariantVector& cells );
    /** The cells of the lanes grid without the key column (lane), without the throttle columns */
    static void LaneToCells( const CORE::SJobLaneStatus& lane, CORE::CVariant::VariantVector& cells );
    /** The cells of the runs grid without the key column (runId) */
    static void RunToCells( const CORE::SJobRunRecord& run, CORE::CVariant::VariantVector& cells );

    /** The throttle status of a lane as flat name = text pairs ("throttle.<name>"), at most MaxThrottleColumns */
    static void FlattenThrottle( const CORE::CDataNode& throttleStatus, CORE::CString::StringMap& values );

    private:
    CSchedulerRowCodec( void );     /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H ? */
