/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H
#include "AssetWorkbenchLib_CWorkbenchJobFilter.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H ? */

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

namespace {

const char* const g_groupNames[] = { "all", "active", "running", "queued", "failed", "done", "cancelled", "finished" };

}

/*-------------------------------------------------------------------------*/

CWorkbenchJobFilter::CWorkbenchJobFilter( void )
    : states( STATES_ALL )
    , kind()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobFilter::IsFailed( const ASSETSTORE::CAssetJobEntry& job )
{GUCEF_TRACE;

    // a run that ended with failed assets is a failure for the list, though the service calls it done: it can be retried for those assets
    return ASSETSTORE::ASSETJOB_STATE_FAILED == job.state || ( ASSETSTORE::ASSETJOB_STATE_DONE == job.state && job.IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RETRY ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchJobFilter::DisplayKindOf( const ASSETSTORE::CAssetJobEntry& job )
{GUCEF_TRACE;

    if ( !job.subKind.IsNULLOrEmpty() )
        return job.subKind;
    if ( "pipeline" == job.kind )
        return "convert";
    return job.kind;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchJobFilter::StateTextOf( const ASSETSTORE::CAssetJobEntry& job )
{GUCEF_TRACE;

    if ( !job.stateText.IsNULLOrEmpty() )
        return job.stateText;
    return ASSETSTORE::AssetJobStateToString( job.state );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobFilter::Matches( const ASSETSTORE::CAssetJobEntry& job ) const
{GUCEF_TRACE;

    if ( !kind.IsNULLOrEmpty() && DisplayKindOf( job ) != kind )
        return false;
    switch ( states )
    {
        case STATES_ACTIVE:    return !job.IsEnded();
        case STATES_RUNNING:   return ASSETSTORE::ASSETJOB_STATE_RUNNING == job.state || ASSETSTORE::ASSETJOB_STATE_FINISHING == job.state;
        case STATES_QUEUED:    return ASSETSTORE::ASSETJOB_STATE_QUEUED == job.state;
        case STATES_FAILED:    return IsFailed( job );
        case STATES_DONE:      return ASSETSTORE::ASSETJOB_STATE_DONE == job.state && !IsFailed( job );
        case STATES_CANCELLED: return ASSETSTORE::ASSETJOB_STATE_CANCELLED == job.state;
        case STATES_FINISHED:  return job.IsEnded();
        default:               return true;
    }
}

/*-------------------------------------------------------------------------*/

const char*
CWorkbenchJobFilter::GetStateGroupName( EStateGroup group )
{GUCEF_TRACE;

    return (UInt32) group < (UInt32) STATES_COUNT ? g_groupNames[ group ] : "all";
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobFilter::ParseStateGroup( const CORE::CString& name, EStateGroup& group )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) STATES_COUNT; ++i )
    {
        if ( name.Equals( g_groupNames[ i ], false ) )
        {
            group = (EStateGroup) i;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchJobFilter::GetKnownKinds( CORE::CString::StringVector& kinds )
{GUCEF_TRACE;

    static const char* const names[] = { "convert", "cook", "validate", "index", "analyse", "thumbnail", "open", "presentation", "download", "ingest", "conversion", "upload", "fetch", "verification" };
    for ( UInt32 i=0; i<(UInt32) ( sizeof( names ) / sizeof( names[ 0 ] ) ); ++i )
        kinds.push_back( names[ i ] );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
