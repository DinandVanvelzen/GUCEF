/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKCONSUMER_H
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKCONSUMER_H

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

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The run of the job "sync-library.<store>" of the job scheduler of a service (library sync design, section 6.4): it starts
 *  the pipeline job "sync-library" of the store with the profile the service keeps for it, marked as scheduled, and ends.
 *  The sync itself is a job of the service with its own progress; a sync of the store that runs already is not started
 *  twice (the service answers ALREADY_EXISTS, which ends the run as done). A service that is no longer open ends the run as
 *  failed with the reason.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetLibrarySyncScheduleTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;            /**< "GUCEF::ASSETSTORE::AssetLibrarySyncScheduledRun" */

    CAssetLibrarySyncScheduleTaskConsumer( void );
    virtual ~CAssetLibrarySyncScheduleTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAssetLibrarySyncScheduleTaskConsumer( const CAssetLibrarySyncScheduleTaskConsumer& src );              /**< not implemented */
    CAssetLibrarySyncScheduleTaskConsumer& operator=( const CAssetLibrarySyncScheduleTaskConsumer& src );   /**< not implemented */
};

typedef CORE::CTFactory< CORE::CTaskConsumer, CAssetLibrarySyncScheduleTaskConsumer, MT::CMutex > TAssetLibrarySyncScheduleTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKCONSUMER_H ? */
