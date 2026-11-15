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

#ifndef GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#include "gucefDRNUTILS_CISchedulerSource.h"
#define GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#endif /* GUCEF_DRNUTILS_CISCHEDULERSOURCE_H ? */

#include "gucefDRNUTILS_CJobSchedulerSourceRelay.h"

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

CJobSchedulerSourceRelay::CJobSchedulerSourceRelay( const CORE::JobSchedulerPtr& scheduler, const CORE::PulseGeneratorPtr& notificationPulseGenerator )
    : CORE::CTSGNotifier( notificationPulseGenerator, true, false )
    , m_scheduler( scheduler )
{GUCEF_TRACE;

    if ( m_scheduler.IsNULL() )
        return;
    CORE::CNotifier* notifier = m_scheduler.GetPointerAlways();
    TEventCallback changed( this, &CJobSchedulerSourceRelay::OnSchedulerChange );
    SubscribeTo( notifier, CORE::CJobScheduler::JobChangedEvent, changed );
    SubscribeTo( notifier, CORE::CJobScheduler::LaneChangedEvent, changed );
    SubscribeTo( notifier, CORE::CJobScheduler::RunChangedEvent, changed );
    SubscribeTo( notifier, CORE::CJobScheduler::HostStateChangedEvent, changed );
}

/*-------------------------------------------------------------------------*/

CJobSchedulerSourceRelay::~CJobSchedulerSourceRelay()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSourceRelay::OnSchedulerChange( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    UInt64 revision = 0;
    if ( GUCEF_NULL != eventData )
        revision = static_cast< CORE::CJobScheduler::TJobSchedulerChangeEventData* >( eventData )->GetData().revision;
    else
    if ( !m_scheduler.IsNULL() )
        revision = m_scheduler->GetRevision();
    CISchedulerSource::TChangedEventData data( revision );
    NotifyObservers( CISchedulerSource::ChangedEvent, &data );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CJobSchedulerSourceRelay::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNUTILS::CJobSchedulerSourceRelay";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
