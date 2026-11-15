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

#ifndef GUCEF_DRNUTILS_CDRNUTILSAUDITRECORD_H
#define GUCEF_DRNUTILS_CDRNUTILSAUDITRECORD_H

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

/**
 *  One remote command (control, task control or job submission), applied, refused or replayed. Arguments and payloads
 *  are never kept in clear: only the SHA-256 digest of their canonical form.
 */
struct GUCEF_DRNUTILS_PUBLIC_CPP SDrnUtilsAuditRecord
{
    Int64 timeUtcMs;
    CORE::CString hostName;
    CORE::CString subject;
    CORE::CString transport;
    CORE::CString sessionId;            /**< hex */
    CORE::CString contract;             /**< "gucef.drnutils.scheduler" or "gucef.drnutils.tasks" */
    CORE::CString action;               /**< action name, "SUBMIT_JOB" for a submission */
    CORE::CString targetKind;
    CORE::CString targetId;
    CORE::CString customName;
    CORE::CString taskTypeName;         /**< submissions */
    CORE::CString argumentsDigest;      /**< SHA-256 hex of the canonical arguments or payload; empty when there are none */
    CORE::CString outcome;              /**< DrnUtilsCommandStatusToString() */
    CORE::CString reason;               /**< why it was refused, or the operator's reason */
    CORE::CString commandId;
    bool dryRun;
    bool replayed;

    SDrnUtilsAuditRecord( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDRNUTILSAUDITRECORD_H ? */
