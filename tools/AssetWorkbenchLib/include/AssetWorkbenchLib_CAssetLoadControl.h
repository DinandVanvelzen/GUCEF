/*
 *  AssetWorkbenchLib: what a load job sees of its request while it prepares on a thread of a pool: progress and cancellation
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

#ifndef ASSETWORKBENCHLIB_CASSETLOADCONTROL_H
#define ASSETWORKBENCHLIB_CASSETLOADCONTROL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CACTIVITYSERVICE_H
#include "gucefGUI_CActivityService.h"
#define GUCEF_GUI_CACTIVITYSERVICE_H
#endif /* GUCEF_GUI_CACTIVITYSERVICE_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What CIAssetLoadJob::Prepare() gets: whether the request was cancelled (by the user through the activity service, or because
 *  something else was opened since) and a way to report the fraction done. Both are safe on the thread of the job, and both may
 *  be used by a job that runs on the calling thread (CAssetLoader::Run()), where nothing cancels it.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CAssetLoadControl
{
    public:

    /** The service and the operation of the request may be NULL / 0 and the task NULL: then nothing is cancelled and progress goes nowhere */
    CAssetLoadControl( GUI::CActivityService* activity, UInt32 activityId, const CORE::CTask* task );

    /** A job that sees this stops as soon as it can: its result is not used */
    bool IsCancelled( void ) const;

    /** Between 0 and 1; shown only when the job says it ReportsProgress() */
    void SetProgress( Float32 fraction );

    private:

    GUI::CActivityService* m_activity;
    UInt32 m_activityId;
    const CORE::CTask* m_task;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CASSETLOADCONTROL_H ? */
