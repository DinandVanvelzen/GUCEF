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

#include "gucefDRNUTILS_CDrnUtilsConfigSupport.h"

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
CDrnUtilsConfigSupport::SaveStringSet( CORE::CDataNode& config, const CORE::CString& name, const CORE::CString::StringSet& values )
{GUCEF_TRACE;

    CORE::CDataNode* list = config.AddChild( name, GUCEF_DATATYPE_ARRAY );
    CORE::CString::StringSet::const_iterator i = values.begin();
    for ( ; i != values.end(); ++i )
        list->AddValueAsChild( *i );
}

/*-------------------------------------------------------------------------*/

bool
CDrnUtilsConfigSupport::LoadStringSet( const CORE::CDataNode& config, const CORE::CString& name, CORE::CString::StringSet& values )
{GUCEF_TRACE;

    const CORE::CDataNode* list = config.FindChild( name );
    if ( GUCEF_NULL == list )
        return false;
    values.clear();
    CORE::CDataNode::const_iterator i = list->ConstBegin();
    for ( ; i != list->ConstEnd(); ++i )
    {
        CORE::CString value = (*i)->GetValue().AsString();
        if ( !value.IsNULLOrEmpty() )
            values.insert( value );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const char*
CDrnUtilsConfigSupport::ExposureToString( TDrnUtilsExposure exposure )
{GUCEF_TRACE;

    switch ( exposure )
    {
        case DRNUTILS_EXPOSURE_VIEW: return "VIEW";
        case DRNUTILS_EXPOSURE_CONTROL: return "CONTROL";
        default: return "NONE";
    }
}

/*-------------------------------------------------------------------------*/

TDrnUtilsExposure
CDrnUtilsConfigSupport::ExposureFromString( const CORE::CString& text, TDrnUtilsExposure defaultIfNeeded )
{GUCEF_TRACE;

    if ( text.Equals( "NONE", false ) )
        return DRNUTILS_EXPOSURE_NONE;
    if ( text.Equals( "VIEW", false ) )
        return DRNUTILS_EXPOSURE_VIEW;
    if ( text.Equals( "CONTROL", false ) )
        return DRNUTILS_EXPOSURE_CONTROL;
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CORE::TJobControlAction
CDrnUtilsConfigSupport::JobControlActionFromString( const CORE::CString& name )
{GUCEF_TRACE;

    for ( Int32 a=CORE::JOBCONTROLACTION_RUN_NOW; a<=CORE::JOBCONTROLACTION_REMOVE_JOB; ++a )
        if ( name.Equals( CORE::JobControlActionToString( (CORE::TJobControlAction) a ), false ) )
            return (CORE::TJobControlAction) a;
    return CORE::JOBCONTROLACTION_UNSPECIFIED;
}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsConfigSupport::SaveRoleMap( CORE::CDataNode& config, const CDrnUtilsRoleMap& roleMap )
{GUCEF_TRACE;

    const char* genericRoles[ 5 ] = { CDrnUtilsRoleMap::SchedulerViewRole, CDrnUtilsRoleMap::SchedulerControlRole, CDrnUtilsRoleMap::SchedulerSubmitRole,
                                      CDrnUtilsRoleMap::TasksViewRole, CDrnUtilsRoleMap::TasksControlRole };
    CORE::CDataNode* roles = config.AddChild( "roles", GUCEF_DATATYPE_OBJECT );
    for ( UInt32 r=0; r<5; ++r )
        SaveStringSet( *roles, genericRoles[ r ], roleMap.GetRoles( genericRoles[ r ] ) );
    CORE::CDataNode* actionRoles = config.AddChild( "actionRoles", GUCEF_DATATYPE_OBJECT );
    for ( Int32 a=CORE::JOBCONTROLACTION_RUN_NOW; a<=CORE::JOBCONTROLACTION_REMOVE_JOB; ++a )
    {
        CORE::CString::StringSet refined = roleMap.GetControlRolesForAction( (CORE::TJobControlAction) a );
        if ( !refined.empty() )
            SaveStringSet( *actionRoles, CORE::JobControlActionToString( (CORE::TJobControlAction) a ), refined );
    }
}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsConfigSupport::LoadRoleMap( const CORE::CDataNode& config, CDrnUtilsRoleMap& roleMap )
{GUCEF_TRACE;

    const CORE::CDataNode* roles = config.FindChild( "roles" );
    if ( GUCEF_NULL != roles )
    {
        CORE::CDataNode::const_iterator i = roles->ConstBegin();
        for ( ; i != roles->ConstEnd(); ++i )
        {
            CORE::CString::StringSet serviceRoles;
            LoadStringSet( *roles, (*i)->GetName(), serviceRoles );
            roleMap.SetRoles( (*i)->GetName(), serviceRoles );
        }
    }
    const CORE::CDataNode* actionRoles = config.FindChild( "actionRoles" );
    if ( GUCEF_NULL != actionRoles )
    {
        CORE::CDataNode::const_iterator i = actionRoles->ConstBegin();
        for ( ; i != actionRoles->ConstEnd(); ++i )
        {
            CORE::TJobControlAction action = JobControlActionFromString( (*i)->GetName() );
            if ( CORE::JOBCONTROLACTION_UNSPECIFIED == action )
                continue;
            CORE::CString::StringSet serviceRoles;
            LoadStringSet( *actionRoles, (*i)->GetName(), serviceRoles );
            roleMap.SetControlRolesForAction( action, serviceRoles );
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
