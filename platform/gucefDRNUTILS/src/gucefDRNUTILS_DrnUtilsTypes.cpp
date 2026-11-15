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

#include "gucefDRNUTILS_DrnUtilsTypes.h"

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

const char*
DrnUtilsCommandStatusToString( TDrnUtilsCommandStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case DRNUTILS_STATUS_OK: return "OK";
        case DRNUTILS_STATUS_NOT_FOUND: return "NOT_FOUND";
        case DRNUTILS_STATUS_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case DRNUTILS_STATUS_FAILED_PRECONDITION: return "FAILED_PRECONDITION";
        case DRNUTILS_STATUS_ABORTED: return "ABORTED";
        case DRNUTILS_STATUS_UNIMPLEMENTED: return "UNIMPLEMENTED";
        case DRNUTILS_STATUS_RESOURCE_EXHAUSTED: return "RESOURCE_EXHAUSTED";
        case DRNUTILS_STATUS_PERMISSION_DENIED: return "PERMISSION_DENIED";
        case DRNUTILS_STATUS_UNAVAILABLE: return "UNAVAILABLE";
        default: return "INTERNAL";
    }
}

/*-------------------------------------------------------------------------*/

const char*
DrnUtilsTaskControlActionToString( TDrnUtilsTaskControlAction action )
{GUCEF_TRACE;

    switch ( action )
    {
        case DRNUTILS_TASKACTION_RESIZE_POOL: return "RESIZE_POOL";
        case DRNUTILS_TASKACTION_CANCEL_TASK: return "CANCEL_TASK";
        case DRNUTILS_TASKACTION_STOP_TASK: return "STOP_TASK";
        default: return "UNSPECIFIED";
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
