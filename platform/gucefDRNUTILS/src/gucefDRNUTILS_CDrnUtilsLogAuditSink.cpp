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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRNUTILS_CDrnUtilsLogAuditSink.h"

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

CDrnUtilsLogAuditSink::CDrnUtilsLogAuditSink( void )
    : CIDrnUtilsAuditSink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDrnUtilsLogAuditSink::~CDrnUtilsLogAuditSink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsLogAuditSink::OnAuditRecord( const SDrnUtilsAuditRecord& record )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "DrnUtilsAudit: host=" << record.hostName << " subject=" << record.subject
               << " transport=" << record.transport << " contract=" << record.contract << " action=" << record.action
               << " target=" << record.targetKind << ":" << record.targetId << " custom=" << record.customName
               << " taskType=" << record.taskTypeName << " digest=" << record.argumentsDigest
               << " outcome=" << record.outcome << " reason=" << record.reason << " commandId=" << record.commandId
               << " dryRun=" << record.dryRun << " replayed=" << record.replayed );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
