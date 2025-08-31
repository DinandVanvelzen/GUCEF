/*
 *  gucefMT: GUCEF module providing multithreading solutions
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include <cstring>

#include "gucefMT_ThreadStatus.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* 
ThreadStatusToThreadStatusString( TThreadStatus threadStatus )
{GUCEF_TRACE;

    switch ( threadStatus )
    {
        case THREADSTATUS_CREATION_FAILED: return "CREATION_FAILED";
        case THREADSTATUS_STARTUP_FAILED: return "STARTUP_FAILED";
        case THREADSTATUS_RUNNING: return "RUNNING";
        case THREADSTATUS_PAUSED: return "PAUSED";
        case THREADSTATUS_RESUMED: return "RESUMED";
        case THREADSTATUS_STOPPED: return "STOPPED";
        case THREADSTATUS_FINISHED: return "FINISHED";
        case THREADSTATUS_KILLED: return "KILLED";
        case THREADSTATUS_DEADLOCKED: return "DEADLOCKED";

        default:
        case THREADSTATUS_UNDEFINED:
            return "UNDEFINED";
    }
}

/*-------------------------------------------------------------------------*/

TThreadStatus 
ThreadStatusStringToThreadStatus( const char* threadStatusStr )
{GUCEF_TRACE;

    if ( GUCEF_NULL == threadStatusStr )
        return THREADSTATUS_UNDEFINED;

    if ( 0 == std::strcmp( threadStatusStr, "CREATION_FAILED" ) )
        return THREADSTATUS_CREATION_FAILED;

    if ( 0 == std::strcmp( threadStatusStr, "STARTUP_FAILED" ) )
        return THREADSTATUS_STARTUP_FAILED;

    if ( 0 == std::strcmp( threadStatusStr, "RUNNING" ) )
        return THREADSTATUS_RUNNING;

    if ( 0 == std::strcmp( threadStatusStr, "PAUSED" ) )
        return THREADSTATUS_PAUSED;

    if ( 0 == std::strcmp( threadStatusStr, "RESUMED" ) )
        return THREADSTATUS_RESUMED;

    if ( 0 == std::strcmp( threadStatusStr, "STOPPED" ) )
        return THREADSTATUS_STOPPED;

    if ( 0 == std::strcmp( threadStatusStr, "FINISHED" ) )
        return THREADSTATUS_FINISHED;        

    if ( 0 == std::strcmp( threadStatusStr, "KILLED" ) )
        return THREADSTATUS_KILLED;        

    if ( 0 == std::strcmp( threadStatusStr, "DEADLOCKED" ) )
        return THREADSTATUS_DEADLOCKED; 

    return THREADSTATUS_UNDEFINED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
