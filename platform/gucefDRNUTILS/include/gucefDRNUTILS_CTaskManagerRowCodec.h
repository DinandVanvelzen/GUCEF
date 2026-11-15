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

#ifndef GUCEF_DRNUTILS_CTASKMANAGERROWCODEC_H
#define GUCEF_DRNUTILS_CTASKMANAGERROWCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H
#include "gucefDRNUTILS_CTaskManagerSample.h"
#define GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H ? */

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
 *  Module-private: the rows of gucef.drnutils.tasks 1.0 (PoolRow, ThreadRow, TaskRow, ScheduledJobRow) and the cells
 *  of the pools, threads, tasks and scheduledjobs grids from a CTaskManagerSample, and back (client, tests)
 */
class CTaskManagerRowCodec
{
    public:

    enum EPoolRowField { POOLROW_NAME = 1, POOLROW_ACTIVEWORKERS = 2, POOLROW_DESIREDMIN = 3, POOLROW_DESIREDMAX = 4, POOLROW_DEDICATED = 5,
                         POOLROW_QUEUED = 6, POOLROW_RUNNING = 7, POOLROW_STARTEDPS = 8, POOLROW_FINISHEDPS = 9, POOLROW_FAILEDPS = 10,
                         POOLROW_CANCELLEDPS = 11, POOLROW_AVGQUEUEWAIT = 12, POOLROW_AVGRUN = 13, POOLROW_UTILISATION = 14,
                         POOLROW_STARTEDTOTAL = 15, POOLROW_FINISHEDTOTAL = 16, POOLROW_FAILEDTOTAL = 17, POOLROW_SCHEDULEDJOBS = 21,
                         POOLROW_CANCELLEDTOTAL = 22 };
    enum EThreadRowField { THREADROW_ID = 1, THREADROW_NAME = 2, THREADROW_POOL = 3, THREADROW_STATUS = 4, THREADROW_TASKID = 5,
                           THREADROW_TASKTYPE = 6, THREADROW_BUSYSINCE = 7, THREADROW_EXTERNAL = 8 };
    enum ETaskRowField { TASKROW_ID = 1, TASKROW_POOL = 2, TASKROW_TASKTYPE = 3, TASKROW_STATUS = 4, TASKROW_EXTRAINFO = 5,
                         TASKROW_QUEUED = 6, TASKROW_STARTED = 7, TASKROW_RUNNINGFOR = 8, TASKROW_THREADID = 9,
                         TASKROW_SCHEDULER = 17, TASKROW_JOBID = 18, TASKROW_RUNID = 19 };
    enum EScheduledJobRowField { SJROW_KEY = 1, SJROW_SCHEDULER = 2, SJROW_JOBID = 3, SJROW_TITLE = 4, SJROW_TASKTYPE = 5,
                                 SJROW_EXECUTIONTARGET = 6, SJROW_POOL = 7, SJROW_LANE = 8, SJROW_STATE = 9, SJROW_NEXTDUE = 10,
                                 SJROW_ACTIVERUNS = 11, SJROW_LASTRUNSTATE = 12, SJROW_LASTRUNENDED = 13, SJROW_ORIGIN = 14 };

    static void PoolToMessage( const CTaskManagerSample::SPool& pool, COMCORE::CRpcMessage& row );
    static void ThreadToMessage( const CTaskManagerSample::SThread& thread, COMCORE::CRpcMessage& row );
    static void TaskToMessage( const CTaskManagerSample::STask& task, COMCORE::CRpcMessage& row );
    static void ScheduledJobToMessage( const CTaskManagerSample::SScheduledJob& job, COMCORE::CRpcMessage& row );

    static void PoolFromMessage( const COMCORE::CRpcMessage& row, CTaskManagerSample::SPool& pool );
    static void TaskFromMessage( const COMCORE::CRpcMessage& row, CTaskManagerSample::STask& task );

    /** Cells without the key column */
    static void PoolToCells( const CTaskManagerSample::SPool& pool, CORE::CVariant::VariantVector& cells );
    static void ThreadToCells( const CTaskManagerSample::SThread& thread, CORE::CVariant::VariantVector& cells );
    static void TaskToCells( const CTaskManagerSample::STask& task, CORE::CVariant::VariantVector& cells );
    static void ScheduledJobToCells( const CTaskManagerSample::SScheduledJob& job, CORE::CVariant::VariantVector& cells );

    private:
    CTaskManagerRowCodec( void );   /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CTASKMANAGERROWCODEC_H ? */
