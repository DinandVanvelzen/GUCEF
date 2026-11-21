/*
 *  AssetWorkbenchLib: the pool task that prepares an asset for an open
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

#ifndef ASSETWORKBENCHLIB_CASSETLOADTASKCONSUMER_H
#define ASSETWORKBENCHLIB_CASSETLOADTASKCONSUMER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETLOADTASKDATA_H
#include "AssetWorkbenchLib_CAssetLoadTaskData.h"
#define ASSETWORKBENCHLIB_CASSETLOADTASKDATA_H
#endif /* ASSETWORKBENCHLIB_CASSETLOADTASKDATA_H ? */

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
 *  Runs CIAssetLoadJob::Prepare() of the CAssetLoadTaskData on a thread of a pool, for CAssetLoader. A request to cancel the task
 *  reaches the job through its CAssetLoadControl. The task data stays after the task: it holds the outcome.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CAssetLoadTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CAssetLoadTaskConsumer( void );

    virtual ~CAssetLoadTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAssetLoadTaskConsumer( const CAssetLoadTaskConsumer& src );
    CAssetLoadTaskConsumer& operator=( const CAssetLoadTaskConsumer& src );
};

typedef CORE::CTFactory< CORE::CTaskConsumer, CAssetLoadTaskConsumer, MT::CMutex > TAssetLoadTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CASSETLOADTASKCONSUMER_H ? */
