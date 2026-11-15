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

#include "gucefCORE_CJobSchedulerLaneHoldScope.h"

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

CJobSchedulerLaneHoldScope::CJobSchedulerLaneHoldScope( CJobScheduler& scheduler, const CString& laneName, const CString& holder )
    : m_scheduler( scheduler )
    , m_laneName( laneName )
    , m_refusal()
    , m_actualCost( 0 )
    , m_held( false )
{GUCEF_TRACE;

    m_held = m_scheduler.TryHoldLane( laneName, holder, m_refusal );
}

/*-------------------------------------------------------------------------*/

CJobSchedulerLaneHoldScope::~CJobSchedulerLaneHoldScope()
{GUCEF_TRACE;

    if ( m_held )
        m_scheduler.ReleaseLaneHold( m_laneName, m_actualCost );
}

/*-------------------------------------------------------------------------*/

bool
CJobSchedulerLaneHoldScope::IsHeld( void ) const
{GUCEF_TRACE;

    return m_held;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobSchedulerLaneHoldScope::GetRefusal( void ) const
{GUCEF_TRACE;

    return m_refusal;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerLaneHoldScope::SetActualCost( UInt32 actualCost )
{GUCEF_TRACE;

    m_actualCost = actualCost;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
