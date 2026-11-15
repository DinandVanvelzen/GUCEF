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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATAFACTORY_H
#define GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATAFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATA_H
#include "gucefASSETSTORE_CAssetToolJobTaskData.h"
#define GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATA_H ? */

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

/** The task data factory of the jobs of one asset tool set: the data it makes carries the task type name of the set */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolJobTaskDataFactory : public CORE::CTFactory< CORE::CIDataNodeSerializableTaskData, CAssetToolJobTaskData, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< CORE::CIDataNodeSerializableTaskData, CAssetToolJobTaskData, MT::CMutex >   TBaseFactory;

    explicit CAssetToolJobTaskDataFactory( const CORE::CString& taskTypeName );

    virtual ~CAssetToolJobTaskDataFactory() GUCEF_VIRTUAL_OVERRIDE;

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAssetToolJobTaskDataFactory( const CAssetToolJobTaskDataFactory& src );              /**< not implemented: the base cannot be copied */
    CAssetToolJobTaskDataFactory& operator=( const CAssetToolJobTaskDataFactory& src );   /**< not implemented */

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

#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATAFACTORY_H ? */
