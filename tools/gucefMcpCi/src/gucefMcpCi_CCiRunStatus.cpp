/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpCi_CCiRunStatus.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiRunStatus::CJob::CJob( void )
    : id()
    , state( STATE_UNKNOWN )
    , elapsedSecs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCiRunStatus::CCiRunStatus( void )
    : runId()
    , ciId()
    , commit()
    , title()
    , state( STATE_UNKNOWN )
    , elapsedSecs( 0 )
    , jobs()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CCiRunStatus::IsFinal( EState state )
{GUCEF_TRACE;

    return STATE_SUCCEEDED == state || STATE_FAILED == state || STATE_CANCELLED == state;
}

/*-------------------------------------------------------------------------*/

bool
CCiRunStatus::IsFinished( void ) const
{GUCEF_TRACE;

    return IsFinal( state );
}

/*-------------------------------------------------------------------------*/

UInt32
CCiRunStatus::CountJobs( EState jobState ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<jobs.size(); ++i )
        if ( jobs[ i ].state == jobState )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiRunStatus::ListJobs( EState jobState, UInt32 maxIds ) const
{GUCEF_TRACE;

    CORE::CString text;
    UInt32 listed = 0;
    UInt32 more = 0;
    for ( size_t i=0; i<jobs.size(); ++i )
    {
        if ( jobs[ i ].state != jobState )
            continue;
        if ( listed >= maxIds )
        {
            ++more;
            continue;
        }
        if ( listed > 0 )
            text += ", ";
        text += jobs[ i ].id;
        ++listed;
    }
    if ( more > 0 )
        text += " and " + CORE::UInt32ToString( more ) + " more";
    return text;
}

/*-------------------------------------------------------------------------*/

const char*
CCiRunStatus::StateToString( EState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case STATE_QUEUED:      return "queued";
        case STATE_RUNNING:     return "running";
        case STATE_SUCCEEDED:   return "succeeded";
        case STATE_FAILED:      return "failed";
        case STATE_CANCELLED:   return "cancelled";
        default:                return "unknown";
    }
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
