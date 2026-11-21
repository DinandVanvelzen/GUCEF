/*
 *  AssetWorkbenchLib: what an asset load job does on a thread of a pool and on the thread of the renderer
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

#ifndef ASSETWORKBENCHLIB_CIASSETLOADJOB_H
#define ASSETWORKBENCHLIB_CIASSETLOADJOB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETLOADCONTROL_H
#include "AssetWorkbenchLib_CAssetLoadControl.h"
#define ASSETWORKBENCHLIB_CASSETLOADCONTROL_H
#endif /* ASSETWORKBENCHLIB_CASSETLOADCONTROL_H ? */

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
 *  The two halves of opening an asset without blocking the interface (see CAssetLoader). Prepare() does what is safe on any
 *  thread and takes the time: reading, decoding, importing; it runs on a thread of a pool and watches the control for
 *  cancellation. Step() then makes what needs the thread of the renderer (the Ogre and GPU objects, the views) in pieces, one
 *  piece per call, so that the loader can keep to a time budget per frame. Discard() lets go of what was made when the result
 *  is not used (cancelled, or another asset was opened meanwhile). Prepare() is not called with Step() or Discard() at the same
 *  time: Discard() of a job that is still preparing waits for it (the loader does that).
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CIAssetLoadJob
{
    public:

    enum EStepResult
    {
        STEP_MORE = 0   ,   /**< another piece follows */
        STEP_DONE       ,   /**< the asset is shown */
        STEP_FAILED         /**< it could not be shown; GetProblem() says why */
    };

    virtual ~CIAssetLoadJob() {}

    /** Whether Prepare() reports a fraction (a bar), else the operation is shown as busy */
    virtual bool ReportsProgress( void ) const { return false; }

    /** On any thread: false when it failed or was cancelled */
    virtual bool Prepare( CAssetLoadControl& control ) = 0;

    /** On the thread of the renderer, after Prepare() succeeded: one piece of the work */
    virtual EStepResult Step( void ) = 0;

    /** On the thread of the renderer: lets go of what Prepare() and Step() made, nothing of it is shown */
    virtual void Discard( void ) = 0;

    /** Why Prepare() or Step() failed */
    virtual CORE::CString GetProblem( void ) const { return CORE::CString::Empty; }
};

typedef CORE::CTSharedPtr< CIAssetLoadJob, MT::CMutex > TAssetLoadJobPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CIASSETLOADJOB_H ? */
