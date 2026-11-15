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

#include "gucefCORE_CJobRunRecord.h"

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

SJobRunRecord::SJobRunRecord( void )
    : runId( 0 )
    , hostInstanceId( 0 )
    , jobId()
    , itemKey()
    , itemLabel()
    , taskTypeName()
    , executionTarget( JOBEXECUTIONTARGET_POOL )
    , threadPoolName()
    , cause( JOBRUNCAUSE_UNSPECIFIED )
    , scheduledUtcMs( 0 )
    , queuedUtcMs( 0 )
    , startedUtcMs( 0 )
    , endedUtcMs( 0 )
    , attempt( 0 )
    , state( JOBRUNSTATE_UNSPECIFIED )
    , progressDone( 0 )
    , progressTotal( 0 )
    , note()
    , outcomeMessage()
    , requestedBy()
    , resultDocument()
    , taskId( 0 )
    , actualCost( 0 )
    , revision( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
SJobRunRecord::Clear( void )
{GUCEF_TRACE;

    *this = SJobRunRecord();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
