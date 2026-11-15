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

#ifndef GUCEF_CORE_CJOBSCHEDULERLANEHOLDSCOPE_H
#define GUCEF_CORE_CJOBSCHEDULERLANEHOLDSCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CJOBSCHEDULER_H
#include "gucefCORE_CJobScheduler.h"
#define GUCEF_CORE_CJOBSCHEDULER_H
#endif /* GUCEF_CORE_CJOBSCHEDULER_H ? */

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

/**
 *  Scope of an exclusive lane hold (CJobScheduler::TryHoldLane() / ReleaseLaneHold()): the hold is released with the
 *  cost given by SetActualCost() when the scope ends.
 */
class GUCEF_CORE_PUBLIC_CPP CJobSchedulerLaneHoldScope
{
    public:

    CJobSchedulerLaneHoldScope( CJobScheduler& scheduler, const CString& laneName, const CString& holder );

    ~CJobSchedulerLaneHoldScope();

    bool IsHeld( void ) const;

    /** Why the hold was refused, when it was */
    const CString& GetRefusal( void ) const;

    /** What the holder spent on the lane, told to its throttle policy on release */
    void SetActualCost( UInt32 actualCost );

    private:

    CJobSchedulerLaneHoldScope( const CJobSchedulerLaneHoldScope& src );            /**< not implemented */
    CJobSchedulerLaneHoldScope& operator=( const CJobSchedulerLaneHoldScope& src ); /**< not implemented */

    CJobScheduler& m_scheduler;
    CString m_laneName;
    CString m_refusal;
    UInt32 m_actualCost;
    bool m_held;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBSCHEDULERLANEHOLDSCOPE_H ? */
