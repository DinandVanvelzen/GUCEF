/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CJOBCONTROLRESULT_H
#define GUCEF_CORE_CJOBCONTROLRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CJOBRUNRECORD_H
#include "gucefCORE_CJobRunRecord.h"
#define GUCEF_CORE_CJOBRUNRECORD_H
#endif /* GUCEF_CORE_CJOBRUNRECORD_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Outcome of CJobScheduler::Control() and CJobScheduler::SubmitJob()
 */
struct GUCEF_CORE_PUBLIC_CPP SJobControlResult
{
    UInt64 revision;                    /**< scheduler revision after the command */
    UInt64 runId;                       /**< RUN_NOW: the queued run */
    CString::StringVector messages;     /**< what was done or why not */
    bool hasRun;
    SJobRunRecord run;                  /**< RUN_NOW, CANCEL_RUN: the run as it stands */
    CDataNode detail;                   /**< CUSTOM: what the handler answers */

    SJobControlResult( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBCONTROLRESULT_H ? */
