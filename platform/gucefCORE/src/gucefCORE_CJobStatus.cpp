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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCORE_CJobStatus.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SJobStatus::SJobStatus( void )
    : jobId()
    , title()
    , description()
    , taskTypeName()
    , laneName()
    , state( JOBSTATE_UNSPECIFIED )
    , priority( 1 )
    , triggerKind( JOBTRIGGER_UNSPECIFIED )
    , triggerText()
    , executionTarget( JOBEXECUTIONTARGET_POOL )
    , threadPoolName()
    , origin( JOBORIGIN_CODE )
    , submittedBy()
    , nextDueUtcMs( 0 )
    , waitingFor()
    , waitingUntilUtcMs( 0 )
    , unavailableReason()
    , lastRunId( 0 )
    , lastRunState( JOBRUNSTATE_UNSPECIFIED )
    , lastRunEndedUtcMs( 0 )
    , runCount( 0 )
    , failureCount( 0 )
    , consecutiveFailures( 0 )
    , activeRunCount( 0 )
    , flags( JOBFLAG_NONE )
    , enabled( true )
    , hasWorkSource( false )
    , remoteActions()
    , remoteCustomActions()
    , customActions()
    , payload()
    , revision( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
