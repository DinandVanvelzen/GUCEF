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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefDRNUTILS_CISchedulerSource.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CISchedulerSource::ChangedEvent = "GUCEF::DRNUTILS::CISchedulerSource::ChangedEvent";

/*-------------------------------------------------------------------------*/

void
CISchedulerSource::RegisterEvents( void )
{GUCEF_TRACE;

    ChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CISchedulerSource::CISchedulerSource( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CISchedulerSource::~CISchedulerSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CISchedulerSource::GetSubmittableTaskTypes( CORE::CString::StringSet& taskTypeNames ) const
{GUCEF_TRACE;

    taskTypeNames.clear();
}

/*-------------------------------------------------------------------------*/

CORE::TJobControlStatus
CISchedulerSource::Submit( const CORE::SJobSubmission& submission, CORE::SJobControlResult& result )
{GUCEF_TRACE;

    result.messages.push_back( "the scheduler accepts no submitted jobs" );
    return CORE::JOBCONTROLSTATUS_FAILED_PRECONDITION;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
