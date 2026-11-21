/*
 *  AssetWorkbenchLib: the pool task that pairs with, or opens, a remote asset store service
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

#ifndef ASSETWORKBENCHLIB_CSTORECONNECTIONTASKCONSUMER_H
#define ASSETWORKBENCHLIB_CSTORECONNECTIONTASKCONSUMER_H

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

#ifndef ASSETWORKBENCHLIB_CSTORECONNECTIONTASKDATA_H
#include "AssetWorkbenchLib_CStoreConnectionTaskData.h"
#define ASSETWORKBENCHLIB_CSTORECONNECTIONTASKDATA_H
#endif /* ASSETWORKBENCHLIB_CSTORECONNECTIONTASKDATA_H ? */

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
 *  Runs the CStoreConnectionTaskData of CWorkbenchStoreConnection on a thread of a pool: the pairing (CDevicePairingClient, which
 *  works on a pulse generator of its own) or the opening of the remote service, which waits for the connection while the thread that
 *  drives the notification pulse generator goes on. What came of it is in the task data.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CStoreConnectionTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CStoreConnectionTaskConsumer( void );

    virtual ~CStoreConnectionTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The words for a pairing that failed: what a person reads, with what to do about it */
    static CORE::CString DescribePairingFailure( Int32 failure, const CORE::CString& detail );

    private:

    CStoreConnectionTaskConsumer( const CStoreConnectionTaskConsumer& src );
    CStoreConnectionTaskConsumer& operator=( const CStoreConnectionTaskConsumer& src );

    void Pair( CStoreConnectionTaskData& data );
    void Connect( CStoreConnectionTaskData& data );
};

typedef CORE::CTFactory< CORE::CTaskConsumer, CStoreConnectionTaskConsumer, MT::CMutex > TStoreConnectionTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CSTORECONNECTIONTASKCONSUMER_H ? */
