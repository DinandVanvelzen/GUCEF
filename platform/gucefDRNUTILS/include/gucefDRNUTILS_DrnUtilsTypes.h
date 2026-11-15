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

#ifndef GUCEF_DRNUTILS_DRNUTILSTYPES_H
#define GUCEF_DRNUTILS_DRNUTILSTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

/** What a host exposes remotely; nothing by default */
enum EDrnUtilsExposure
{
    DRNUTILS_EXPOSURE_NONE      = 0 ,
    DRNUTILS_EXPOSURE_VIEW      = 1 ,   /**< read methods and tables only */
    DRNUTILS_EXPOSURE_CONTROL   = 2     /**< also the control (and submit) methods, for the actions allowed in the settings */
};
typedef enum EDrnUtilsExposure TDrnUtilsExposure;

/** Outcome of a remote command at the host; the names equal the RPC status names */
enum EDrnUtilsCommandStatus
{
    DRNUTILS_STATUS_OK                  = 0 ,
    DRNUTILS_STATUS_NOT_FOUND           = 1 ,
    DRNUTILS_STATUS_INVALID_ARGUMENT    = 2 ,
    DRNUTILS_STATUS_FAILED_PRECONDITION = 3 ,
    DRNUTILS_STATUS_ABORTED             = 4 ,
    DRNUTILS_STATUS_UNIMPLEMENTED       = 5 ,
    DRNUTILS_STATUS_RESOURCE_EXHAUSTED  = 6 ,
    DRNUTILS_STATUS_INTERNAL            = 7 ,
    DRNUTILS_STATUS_PERMISSION_DENIED   = 8 ,
    DRNUTILS_STATUS_UNAVAILABLE         = 9     /**< client side: no endpoint, no connection */
};
typedef enum EDrnUtilsCommandStatus TDrnUtilsCommandStatus;

/** Control actions of gucef.drnutils.tasks 1.0; numbers equal the contract's TaskControlAction */
enum EDrnUtilsTaskControlAction
{
    DRNUTILS_TASKACTION_UNSPECIFIED = 0 ,
    DRNUTILS_TASKACTION_RESIZE_POOL = 1 ,
    DRNUTILS_TASKACTION_CANCEL_TASK = 2 ,
    DRNUTILS_TASKACTION_STOP_TASK   = 3
    /* 4..7 are reserved for PAUSE_TASK, RESUME_TASK, PAUSE_POOL, RESUME_POOL (deferred) */
};
typedef enum EDrnUtilsTaskControlAction TDrnUtilsTaskControlAction;

GUCEF_DRNUTILS_PUBLIC_CPP const char* DrnUtilsCommandStatusToString( TDrnUtilsCommandStatus status );
GUCEF_DRNUTILS_PUBLIC_CPP const char* DrnUtilsTaskControlActionToString( TDrnUtilsTaskControlAction action );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_DRNUTILSTYPES_H ? */
