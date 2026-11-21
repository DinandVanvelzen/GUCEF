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

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKDATA_H
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#include "gucefCORE_CIDataNodeSerializableTaskData.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

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
 *  The task data of a scheduled run of the library sync of a store (library sync design, section 6.4): the payload of the
 *  job "sync-library.<store>" of the job scheduler of a service. 'serviceKey' names the service in this process the run
 *  belongs to (CLocalAssetStoreService::GetScheduleKey()), 'storeId' the store.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetLibrarySyncScheduleTaskData : public CORE::CIDataNodeSerializableTaskData
{
    public:

    CORE::CString serviceKey;
    CORE::CString storeId;

    CAssetLibrarySyncScheduleTaskData( void );
    CAssetLibrarySyncScheduleTaskData( const CAssetLibrarySyncScheduleTaskData& src );
    virtual ~CAssetLibrarySyncScheduleTaskData() GUCEF_VIRTUAL_OVERRIDE;

    /** The payload of a job definition with these members */
    void ToPayload( CORE::CDataNode& payload ) const;

    virtual const CORE::CString& GetTaskTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetThreadPoolName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetOnlyUseExistingThreadPool( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetTaskCanBeQueued( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetTaskId( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** The task data of the scheduled runs, made by the task manager from the payload of the job */
typedef CORE::CTFactory< CORE::CIDataNodeSerializableTaskData, CAssetLibrarySyncScheduleTaskData, MT::CMutex > TAssetLibrarySyncScheduleTaskDataFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKDATA_H ? */
