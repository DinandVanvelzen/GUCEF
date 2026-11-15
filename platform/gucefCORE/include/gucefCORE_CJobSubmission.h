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

#ifndef GUCEF_CORE_CJOBSUBMISSION_H
#define GUCEF_CORE_CJOBSUBMISSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CJOBDEFINITION_H
#include "gucefCORE_CJobDefinition.h"
#define GUCEF_CORE_CJOBDEFINITION_H
#endif /* GUCEF_CORE_CJOBDEFINITION_H ? */

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
 *  A job submitted as data (CJobScheduler::SubmitJob()): a definition that refers to a task type the scheduler allows
 *  for submission, with the payload that task type's data factory understands.
 */
struct GUCEF_CORE_PUBLIC_CPP SJobSubmission
{
    CJobDefinition definition;          /**< Origin, SubmittedBy and SubmittedAtUtcMs are set by the scheduler */
    CString submittedBy;                /**< subject of the principal */
    bool replaceExisting;               /**< replace a submitted job with the same id; a job defined in code is never replaced */
    bool dryRun;                        /**< validate only */
    CString reason;

    SJobSubmission( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBSUBMISSION_H ? */
