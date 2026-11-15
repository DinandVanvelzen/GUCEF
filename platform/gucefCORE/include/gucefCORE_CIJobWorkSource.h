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

#ifndef GUCEF_CORE_CIJOBWORKSOURCE_H
#define GUCEF_CORE_CIJOBWORKSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CJOBWORKITEM_H
#include "gucefCORE_CJobWorkItem.h"
#define GUCEF_CORE_CJOBWORKITEM_H
#endif /* GUCEF_CORE_CJOBWORKITEM_H ? */

#ifndef GUCEF_CORE_CJOBRUNRECORD_H
#include "gucefCORE_CJobRunRecord.h"
#define GUCEF_CORE_CJOBRUNRECORD_H
#endif /* GUCEF_CORE_CJOBRUNRECORD_H ? */

#ifndef GUCEF_CORE_CJOBCUSTOMACTION_H
#include "gucefCORE_CJobCustomAction.h"
#define GUCEF_CORE_CJOBCUSTOMACTION_H
#endif /* GUCEF_CORE_CJOBCUSTOMACTION_H ? */

#ifndef GUCEF_CORE_CJOBCONTROLCOMMAND_H
#include "gucefCORE_CJobControlCommand.h"
#define GUCEF_CORE_CJOBCONTROLCOMMAND_H
#endif /* GUCEF_CORE_CJOBCONTROLCOMMAND_H ? */

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

class CIJobWorkItemSink;

/*-------------------------------------------------------------------------*/

/**
 *  A planner that decides what is due (value-driven work), behind a job with trigger WORK_SOURCE
 *  (CJobScheduler::AddWorkSourceJob()). It is the only abstraction the scheduler adds beside tasks: its work items are
 *  (task type, payload) pairs where possible, and every run of an item is a task of the task manager.
 *  Evaluate() and OnItemFinished() run on the dispatcher thread, outside the scheduler lock; the source keeps a leaf
 *  lock of its own.
 */
class GUCEF_CORE_PUBLIC_CPP CIJobWorkSource
{
    public:

    CIJobWorkSource( void );

    virtual ~CIJobWorkSource();

    /** Puts the due items in the sink; sets nextWakeUtcMs to the next instant worth evaluating again (0 = none) */
    virtual void Evaluate( Int64 nowUtcMs, UInt64 nowMonotonicMs, CIJobWorkItemSink& dispatch, Int64& nextWakeUtcMs ) = 0;

    /** Every outcome of every item's run, including SKIPPED and CANCELLED */
    virtual void OnItemFinished( const SJobWorkItem& item, const SJobRunRecord& run, Int64 nowUtcMs ) = 0;

    /** A run-now of a lane the source uses: clear own waits and backoffs of that lane. Default: nothing */
    virtual void OnRunNow( const CString& laneName, Int64 nowUtcMs );

    /** For the views (GetJob with includeSourceStatus) */
    virtual void GetStatus( CDataNode& status, Int64 nowUtcMs, bool includeQueue ) const = 0;

    /** Default: nothing to keep */
    virtual bool SaveState( CDataNode& state ) const;

    /** Default: nothing to restore */
    virtual bool LoadState( const CDataNode& state, CString& error );

    /** The custom actions the source declares. Default: none */
    virtual void GetCustomActions( TJobCustomActionVector& actions ) const;

    /** Applies a declared custom action on the calling thread. Default: JOBCONTROLSTATUS_UNIMPLEMENTED */
    virtual TJobControlStatus ApplyCustomAction( const SJobControlCommand& command, CDataNode& detail, CString& message );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIJOBWORKSOURCE_H ? */
