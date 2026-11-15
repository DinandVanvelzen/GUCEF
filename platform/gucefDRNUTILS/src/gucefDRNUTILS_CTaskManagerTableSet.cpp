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

#include "gucefDRNUTILS_CTaskManagerTableSet.h"

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

CTaskManagerTableSet::CTaskManagerTableSet( void )
    : m_pools()
    , m_threads()
    , m_tasks()
    , m_scheduledJobs()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerTableSet::IsComplete( void ) const
{GUCEF_TRACE;

    return !m_pools.IsNULL() && !m_threads.IsNULL() && !m_tasks.IsNULL() && !m_scheduledJobs.IsNULL();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerTableSet::Clear( void )
{GUCEF_TRACE;

    m_pools.Unlink();
    m_threads.Unlink();
    m_tasks.Unlink();
    m_scheduledJobs.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerTableSet::SetPools( const CORE::CIDataTablePtr& pools )
{GUCEF_TRACE;

    m_pools = pools;
}

/*-------------------------------------------------------------------------*/

const CORE::CIDataTablePtr&
CTaskManagerTableSet::GetPools( void ) const
{GUCEF_TRACE;

    return m_pools;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerTableSet::SetThreads( const CORE::CIDataTablePtr& threads )
{GUCEF_TRACE;

    m_threads = threads;
}

/*-------------------------------------------------------------------------*/

const CORE::CIDataTablePtr&
CTaskManagerTableSet::GetThreads( void ) const
{GUCEF_TRACE;

    return m_threads;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerTableSet::SetTasks( const CORE::CIDataTablePtr& tasks )
{GUCEF_TRACE;

    m_tasks = tasks;
}

/*-------------------------------------------------------------------------*/

const CORE::CIDataTablePtr&
CTaskManagerTableSet::GetTasks( void ) const
{GUCEF_TRACE;

    return m_tasks;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerTableSet::SetScheduledJobs( const CORE::CIDataTablePtr& scheduledJobs )
{GUCEF_TRACE;

    m_scheduledJobs = scheduledJobs;
}

/*-------------------------------------------------------------------------*/

const CORE::CIDataTablePtr&
CTaskManagerTableSet::GetScheduledJobs( void ) const
{GUCEF_TRACE;

    return m_scheduledJobs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
