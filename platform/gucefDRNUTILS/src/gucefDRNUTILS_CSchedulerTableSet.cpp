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

#include "gucefDRNUTILS_CSchedulerTableSet.h"

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

CSchedulerTableSet::CSchedulerTableSet( void )
    : m_jobs()
    , m_lanes()
    , m_runs()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CSchedulerTableSet::IsComplete( void ) const
{GUCEF_TRACE;

    return !m_jobs.IsNULL() && !m_lanes.IsNULL() && !m_runs.IsNULL();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerTableSet::Clear( void )
{GUCEF_TRACE;

    m_jobs.Unlink();
    m_lanes.Unlink();
    m_runs.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerTableSet::SetJobs( const CORE::CIDataTablePtr& jobs )
{GUCEF_TRACE;

    m_jobs = jobs;
}

/*-------------------------------------------------------------------------*/

const CORE::CIDataTablePtr&
CSchedulerTableSet::GetJobs( void ) const
{GUCEF_TRACE;

    return m_jobs;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerTableSet::SetLanes( const CORE::CIDataTablePtr& lanes )
{GUCEF_TRACE;

    m_lanes = lanes;
}

/*-------------------------------------------------------------------------*/

const CORE::CIDataTablePtr&
CSchedulerTableSet::GetLanes( void ) const
{GUCEF_TRACE;

    return m_lanes;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerTableSet::SetRuns( const CORE::CIDataTablePtr& runs )
{GUCEF_TRACE;

    m_runs = runs;
}

/*-------------------------------------------------------------------------*/

const CORE::CIDataTablePtr&
CSchedulerTableSet::GetRuns( void ) const
{GUCEF_TRACE;

    return m_runs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
