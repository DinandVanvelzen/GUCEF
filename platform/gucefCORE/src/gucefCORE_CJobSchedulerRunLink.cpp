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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULER_H
#include "gucefCORE_CJobScheduler.h"
#define GUCEF_CORE_CJOBSCHEDULER_H
#endif /* GUCEF_CORE_CJOBSCHEDULER_H ? */

#include "gucefCORE_CJobSchedulerRunLink.h"

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

CJobSchedulerRunLink::CJobSchedulerRunLink( CJobScheduler* scheduler, const CString& schedulerName )
    : CTSharedPtrCreator< CJobSchedulerRunLink, MT::CMutex >( this )
    , m_lock()
    , m_scheduler( scheduler )
    , m_schedulerName( schedulerName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJobSchedulerRunLink::~CJobSchedulerRunLink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CJobSchedulerRunLink::GetSchedulerName( void ) const
{GUCEF_TRACE;

    return m_schedulerName;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerRunLink::OnRunTaskEnded( UInt64 runId                 ,
                                      UInt32 taskId                ,
                                      TTaskStatus endStatus        ,
                                      const CString& statusExtra   )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( GUCEF_NULL != m_scheduler )
        m_scheduler->OnRunTaskEnded( runId, taskId, endStatus, statusExtra );
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerRunLink::Detach( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_scheduler = GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
