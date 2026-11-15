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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERCONTRACTS_H
#include "gucefDRNUTILS_CSchedulerContracts.h"
#define GUCEF_DRNUTILS_CSCHEDULERCONTRACTS_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERCONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERCONTRACTS_H
#include "gucefDRNUTILS_CTaskManagerContracts.h"
#define GUCEF_DRNUTILS_CTASKMANAGERCONTRACTS_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERCONTRACTS_H ? */

#include "gucefDRNUTILS_CDrnUtilsAccessRules.h"

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

void
CDrnUtilsAccessRules::AddSchedulerRules( COMCORE::CRpcRoleAccessPolicy& policy, const CDrnUtilsRoleMap& roles )
{GUCEF_TRACE;

    const CORE::CString service = CORE::CString( CSchedulerContracts::ContractName ) + "." + CSchedulerContracts::ServiceName + "/";
    CORE::CString::StringSet viewRoles = roles.GetRoles( CDrnUtilsRoleMap::SchedulerViewRole );
    const char* viewMethods[ 6 ] = { "GetInfo", "ListJobs", "GetJob", "ListLanes", "ListRuns", "Watch" };
    for ( UInt32 m=0; m<6; ++m )
        policy.AddRule( service + viewMethods[ m ], viewRoles );
    // Control carries REMOVE_JOB, which is the submit role's: the host refines per action
    CORE::CString::StringSet controlRoles = roles.GetRoles( CDrnUtilsRoleMap::SchedulerControlRole );
    CORE::CString::StringSet submitRoles = roles.GetRoles( CDrnUtilsRoleMap::SchedulerSubmitRole );
    CORE::CString::StringSet controlOrSubmit( controlRoles );
    controlOrSubmit.insert( submitRoles.begin(), submitRoles.end() );
    policy.AddRule( service + "Control", controlOrSubmit );
    policy.AddRule( service + "SubmitJob", submitRoles );
}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsAccessRules::AddTaskManagerRules( COMCORE::CRpcRoleAccessPolicy& policy, const CDrnUtilsRoleMap& roles )
{GUCEF_TRACE;

    const CORE::CString service = CORE::CString( CTaskManagerContracts::ContractName ) + "." + CTaskManagerContracts::ServiceName + "/";
    CORE::CString::StringSet viewRoles = roles.GetRoles( CDrnUtilsRoleMap::TasksViewRole );
    const char* viewMethods[ 6 ] = { "GetInfo", "ListPools", "ListThreads", "ListTasks", "Watch", "ListScheduledJobs" };
    for ( UInt32 m=0; m<6; ++m )
        policy.AddRule( service + viewMethods[ m ], viewRoles );
    policy.AddRule( service + "Control", roles.GetRoles( CDrnUtilsRoleMap::TasksControlRole ) );
}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsAccessRules::AddTo( COMCORE::CRpcRoleAccessPolicy& policy, const CDrnUtilsRoleMap& roles )
{GUCEF_TRACE;

    AddSchedulerRules( policy, roles );
    AddTaskManagerRules( policy, roles );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
