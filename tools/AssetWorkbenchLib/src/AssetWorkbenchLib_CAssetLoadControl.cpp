/*
 *  AssetWorkbenchLib: what a load job sees of its request while it prepares on a thread of a pool: progress and cancellation
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

#include "AssetWorkbenchLib_CAssetLoadControl.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetLoadControl::CAssetLoadControl( GUI::CActivityService* activity, UInt32 activityId, const CORE::CTask* task )
    : m_activity( activity )
    , m_activityId( activityId )
    , m_task( task )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetLoadControl::IsCancelled( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_task && m_task->IsCancellationRequested() )
        return true;
    return GUCEF_NULL != m_activity && 0 != m_activityId && m_activity->IsCancelRequested( m_activityId );
}

/*-------------------------------------------------------------------------*/

void
CAssetLoadControl::SetProgress( Float32 fraction )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_activity && 0 != m_activityId )
        m_activity->SetProgress( m_activityId, fraction );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
