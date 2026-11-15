/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_COM_CMCPSCHEMABUILDER_H
#include "gucefCOM_CMcpSchemaBuilder.h"
#define GUCEF_COM_CMCPSCHEMABUILDER_H
#endif /* GUCEF_COM_CMCPSCHEMABUILDER_H ? */

#include "gucefCOM_CMcpJobInfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CMcpJobInfo::StateQueued = "queued";
const CORE::CString CMcpJobInfo::StateWorking = "working";
const CORE::CString CMcpJobInfo::StateCompleted = "completed";
const CORE::CString CMcpJobInfo::StateFailed = "failed";
const CORE::CString CMcpJobInfo::StateCancelled = "cancelled";
const CORE::CString CMcpJobInfo::StateInterrupted = "interrupted";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpJobInfo::CMcpJobInfo( void )
    : jobId()
    , kind()
    , subject()
    , state( StateQueued )
    , hasRun( false )
    , run()
    , createdUtcMs( 0 )
    , expiresUtcMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMcpJobInfo::IsFinal( void ) const
{GUCEF_TRACE;

    return IsFinalState( state );
}

/*-------------------------------------------------------------------------*/

bool
CMcpJobInfo::IsFinalState( const CORE::CString& state )
{GUCEF_TRACE;

    return StateCompleted == state || StateFailed == state || StateCancelled == state || StateInterrupted == state;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpJobInfo::StateOfRun( CORE::TJobRunState runState )
{GUCEF_TRACE;

    switch ( runState )
    {
        case CORE::JOBRUNSTATE_RUNNING:     { return StateWorking; }
        case CORE::JOBRUNSTATE_SUCCEEDED:   { return StateCompleted; }
        case CORE::JOBRUNSTATE_FAILED:
        case CORE::JOBRUNSTATE_TIMED_OUT:
        case CORE::JOBRUNSTATE_SKIPPED:     { return StateFailed; }
        case CORE::JOBRUNSTATE_CANCELLED:   { return StateCancelled; }
        case CORE::JOBRUNSTATE_QUEUED:
        case CORE::JOBRUNSTATE_WAITING:
        default:                            { return StateQueued; }
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMcpJobInfo::UtcMsToText( Int64 utcMs )
{GUCEF_TRACE;

    if ( utcMs <= 0 )
        return CORE::CString::Empty;
    CORE::CDateTime time;
    time.FromUnixEpochBasedTicksInMillisecs( (UInt64) utcMs );
    return time.ToIso8601DateTimeString( true, true );
}

/*-------------------------------------------------------------------------*/

namespace {

void
SetTimeOrNull( CORE::CDataNode& node, const char* name, Int64 utcMs )
{GUCEF_TRACE;

    if ( utcMs > 0 )
        node.SetAttribute( name, CMcpJobInfo::UtcMsToText( utcMs ) );
    else
        node.SetAttribute( name, CORE::CVariant() );
}

}

/*-------------------------------------------------------------------------*/

void
CMcpJobInfo::ToDataNode( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.Clear();
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "jobId", jobId );
    node.SetAttribute( "kind", kind );
    node.SetAttribute( "state", state );
    SetTimeOrNull( node, "createdUtc", createdUtcMs );
    SetTimeOrNull( node, "startedUtc", hasRun ? run.startedUtcMs : 0 );
    SetTimeOrNull( node, "endedUtc", hasRun ? run.endedUtcMs : 0 );
    CORE::CDataNode* progress = node.AddChild( "progress", GUCEF_DATATYPE_OBJECT );
    progress->SetAttribute( "done", hasRun ? run.progressDone : (UInt64) 0 );
    if ( hasRun && run.progressTotal > 0 )
        progress->SetAttribute( "total", run.progressTotal );
    else
        progress->SetAttribute( "total", CORE::CVariant() );
    if ( hasRun && !run.note.IsNULLOrEmpty() )
        progress->SetAttribute( "message", run.note );
    else
        progress->SetAttribute( "message", CORE::CVariant() );
    if ( hasRun && !run.outcomeMessage.IsNULLOrEmpty() )
        node.SetAttribute( "message", run.outcomeMessage );
    else
        node.SetAttribute( "message", CORE::CVariant() );
    SetTimeOrNull( node, "expiresUtc", expiresUtcMs );
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode
CMcpJobInfo::GetSchema( void )
{GUCEF_TRACE;

    CORE::CString::StringVector states;
    states.push_back( StateQueued );
    states.push_back( StateWorking );
    states.push_back( StateCompleted );
    states.push_back( StateFailed );
    states.push_back( StateCancelled );
    states.push_back( StateInterrupted );

    CORE::CDataNode progress = CMcpSchemaBuilder::Object();
    CMcpSchemaBuilder::AddProperty( progress, "done", CMcpSchemaBuilder::UnboundedInteger(), true );
    CMcpSchemaBuilder::AddProperty( progress, "total", CMcpSchemaBuilder::Nullable( CMcpSchemaBuilder::UnboundedInteger() ), true );
    CMcpSchemaBuilder::AddProperty( progress, "message", CMcpSchemaBuilder::Nullable( CMcpSchemaBuilder::String() ), true );

    CORE::CDataNode schema = CMcpSchemaBuilder::Object( "A job: poll it with its jobId until its state is final" );
    CMcpSchemaBuilder::AddProperty( schema, "jobId", CMcpSchemaBuilder::String( 1, 64 ), true );
    CMcpSchemaBuilder::AddProperty( schema, "kind", CMcpSchemaBuilder::String(), true );
    CMcpSchemaBuilder::AddProperty( schema, "state", CMcpSchemaBuilder::Enum( states ), true );
    CMcpSchemaBuilder::AddProperty( schema, "createdUtc", CMcpSchemaBuilder::Nullable( CMcpSchemaBuilder::String() ), true );
    CMcpSchemaBuilder::AddProperty( schema, "startedUtc", CMcpSchemaBuilder::Nullable( CMcpSchemaBuilder::String() ), true );
    CMcpSchemaBuilder::AddProperty( schema, "endedUtc", CMcpSchemaBuilder::Nullable( CMcpSchemaBuilder::String() ), true );
    CMcpSchemaBuilder::AddProperty( schema, "progress", progress, true );
    CMcpSchemaBuilder::AddProperty( schema, "message", CMcpSchemaBuilder::Nullable( CMcpSchemaBuilder::String() ), true );
    CMcpSchemaBuilder::AddProperty( schema, "expiresUtc", CMcpSchemaBuilder::Nullable( CMcpSchemaBuilder::String() ), true );
    return schema;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
