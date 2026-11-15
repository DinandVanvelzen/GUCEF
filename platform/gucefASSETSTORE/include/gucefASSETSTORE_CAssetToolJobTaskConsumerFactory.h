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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMERFACTORY_H
#define GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMERFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMER_H
#include "gucefASSETSTORE_CAssetToolJobTaskConsumer.h"
#define GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMER_H ? */

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
 *  The task consumer factory of the jobs of one asset tool set: every consumer it makes works for the context of that
 *  tool set (its service, its redaction), so two tool sets in one process (tests, a host with two services) do not
 *  share a global. Registered with the task manager under the task type name of the tool set by CAssetToolSet.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolJobTaskConsumerFactory : public CORE::CTFactory< CORE::CTaskConsumer, CAssetToolJobTaskConsumer, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< CORE::CTaskConsumer, CAssetToolJobTaskConsumer, MT::CMutex >    TBaseFactory;

    CAssetToolJobTaskConsumerFactory( CAssetToolContext* context, const CORE::CString& taskTypeName );

    virtual ~CAssetToolJobTaskConsumerFactory() GUCEF_VIRTUAL_OVERRIDE;

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAssetToolJobTaskConsumerFactory( const CAssetToolJobTaskConsumerFactory& src );              /**< not implemented: the base cannot be copied */
    CAssetToolJobTaskConsumerFactory& operator=( const CAssetToolJobTaskConsumerFactory& src );   /**< not implemented */

    CAssetToolContext* m_context;
    CORE::CString m_taskTypeName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMERFACTORY_H ? */
