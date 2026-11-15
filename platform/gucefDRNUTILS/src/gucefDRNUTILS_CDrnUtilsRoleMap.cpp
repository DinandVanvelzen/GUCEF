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

#include "gucefDRNUTILS_CDrnUtilsRoleMap.h"

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

const char* CDrnUtilsRoleMap::SchedulerViewRole = "scheduler.view";
const char* CDrnUtilsRoleMap::SchedulerControlRole = "scheduler.control";
const char* CDrnUtilsRoleMap::SchedulerSubmitRole = "scheduler.submit";
const char* CDrnUtilsRoleMap::TasksViewRole = "tasks.view";
const char* CDrnUtilsRoleMap::TasksControlRole = "tasks.control";

/*-------------------------------------------------------------------------*/

CDrnUtilsRoleMap::CDrnUtilsRoleMap( void )
    : m_roles()
    , m_actionRoles()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsRoleMap::SetRoles( const CORE::CString& genericRole, const CORE::CString::StringSet& serviceRoles )
{GUCEF_TRACE;

    m_roles[ genericRole ] = serviceRoles;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringSet
CDrnUtilsRoleMap::GetRoles( const CORE::CString& genericRole ) const
{GUCEF_TRACE;

    TRoleMap::const_iterator i = m_roles.find( genericRole );
    if ( i != m_roles.end() )
        return (*i).second;
    CORE::CString::StringSet roles;
    roles.insert( genericRole );
    return roles;
}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsRoleMap::SetControlRolesForAction( CORE::TJobControlAction action, const CORE::CString::StringSet& serviceRoles )
{GUCEF_TRACE;

    m_actionRoles[ (Int32) action ] = serviceRoles;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringSet
CDrnUtilsRoleMap::GetControlRolesForAction( CORE::TJobControlAction action ) const
{GUCEF_TRACE;

    TActionRoleMap::const_iterator i = m_actionRoles.find( (Int32) action );
    if ( i != m_actionRoles.end() )
        return (*i).second;
    return CORE::CString::StringSet();
}

/*-------------------------------------------------------------------------*/

bool
CDrnUtilsRoleMap::HasRole( const CORE::CString::StringSet& principalRoles, const CORE::CString& genericRole ) const
{GUCEF_TRACE;

    CORE::CString::StringSet serviceRoles = GetRoles( genericRole );
    CORE::CString::StringSet::const_iterator i = serviceRoles.begin();
    while ( i != serviceRoles.end() )
    {
        if ( principalRoles.find( (*i) ) != principalRoles.end() )
            return true;
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
