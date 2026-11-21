/*
 *  AssetWorkbenchLib: what a thread of a pool is asked to prepare for an open, and what came of it
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

#ifndef ASSETWORKBENCHLIB_CASSETLOADTASKDATA_H
#define ASSETWORKBENCHLIB_CASSETLOADTASKDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef ASSETWORKBENCHLIB_CIASSETLOADJOB_H
#include "AssetWorkbenchLib_CIAssetLoadJob.h"
#define ASSETWORKBENCHLIB_CIASSETLOADJOB_H
#endif /* ASSETWORKBENCHLIB_CIASSETLOADJOB_H ? */

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

/** What CAssetLoader::Begin() hands to the task: the job (shared with the loader) and the operation of the activity service of the request */
class ASSETWORKBENCHLIB_PUBLIC_CPP CAssetLoadTaskData : public CORE::CICloneable
{
    public:

    TAssetLoadJobPtr job;
    GUI::CActivityService* activity;    /**< may be NULL */
    UInt32 activityId;                  /**< 0 for none */
    bool prepared;                      /**< filled when the task ran: Prepare() succeeded */
    bool cancelled;                     /**< filled when the task ran: the control said cancelled at the end */
    CORE::CString problem;              /**< filled when the task ran, when Prepare() threw */

    CAssetLoadTaskData( void );

    CAssetLoadTaskData( const CAssetLoadTaskData& src );

    virtual ~CAssetLoadTaskData() GUCEF_VIRTUAL_OVERRIDE;

    CAssetLoadTaskData& operator=( const CAssetLoadTaskData& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CASSETLOADTASKDATA_H ? */
