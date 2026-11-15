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

#ifndef GUCEF_DRNUTILS_CDRNUTILSROLEMAP_H
#define GUCEF_DRNUTILS_CDRNUTILSROLEMAP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_JOBSCHEDULERTYPES_H
#include "gucefCORE_JobSchedulerTypes.h"
#define GUCEF_CORE_JOBSCHEDULERTYPES_H
#endif /* GUCEF_CORE_JOBSCHEDULERTYPES_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Maps the generic role names of gucefDRNUTILS to the role names of a service (the investment service maps the view
 *  roles to "viewer" and the control and submit roles to "admin"), and refines roles per control action.
 *  A generic role without a mapping maps to itself.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDrnUtilsRoleMap
{
    public:

    static const char* SchedulerViewRole;       /**< "scheduler.view": GetInfo, ListJobs, GetJob, ListLanes, ListRuns, Watch */
    static const char* SchedulerControlRole;    /**< "scheduler.control": Control */
    static const char* SchedulerSubmitRole;     /**< "scheduler.submit": SubmitJob, and Control REMOVE_JOB of submitted jobs */
    static const char* TasksViewRole;           /**< "tasks.view": GetInfo, ListPools, ListThreads, ListTasks, ListScheduledJobs, Watch */
    static const char* TasksControlRole;        /**< "tasks.control": Control */

    CDrnUtilsRoleMap( void );

    /** The service's roles for a generic role */
    void SetRoles( const CORE::CString& genericRole, const CORE::CString::StringSet& serviceRoles );

    /** The service's roles of the generic role; the generic role itself when not mapped */
    CORE::CString::StringSet GetRoles( const CORE::CString& genericRole ) const;

    /** An action that needs other roles than the method's (RUN_NOW at priority 0, SET_LANE_MODE, privileged custom actions) */
    void SetControlRolesForAction( CORE::TJobControlAction action, const CORE::CString::StringSet& serviceRoles );

    /** Empty when the action has no refinement */
    CORE::CString::StringSet GetControlRolesForAction( CORE::TJobControlAction action ) const;

    /** Whether one of the principal's roles is one of the service roles of the generic role */
    bool HasRole( const CORE::CString::StringSet& principalRoles, const CORE::CString& genericRole ) const;

    private:

    typedef GUCEF::map< CORE::CString, CORE::CString::StringSet > TRoleMap;
    typedef GUCEF::map< Int32, CORE::CString::StringSet > TActionRoleMap;

    TRoleMap m_roles;
    TActionRoleMap m_actionRoles;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDRNUTILSROLEMAP_H ? */
