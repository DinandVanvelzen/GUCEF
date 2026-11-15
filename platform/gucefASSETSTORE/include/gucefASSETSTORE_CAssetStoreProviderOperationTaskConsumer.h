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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H
#define GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H

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
#ifndef GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONDATA_H
#include "gucefASSETSTORE_CAssetStoreProviderOperationData.h"
#define GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONDATA_H ? */
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

/** The task consumer that carries out the async operations of CIAssetStoreProvider on a pool thread */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreProviderOperationTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CAssetStoreProviderOperationTaskConsumer( void );
    virtual ~CAssetStoreProviderOperationTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Ends the future of the source as failed with the status, "statusName: message" as its extra info */
    static void FailSource( CORE::CFutureResultSource& source, TAssetStoreStatus status, const CORE::CString& message );
};

typedef CORE::CTFactory< CORE::CTaskConsumer, CAssetStoreProviderOperationTaskConsumer, MT::CMutex > TAssetStoreProviderOperationTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H ? */
