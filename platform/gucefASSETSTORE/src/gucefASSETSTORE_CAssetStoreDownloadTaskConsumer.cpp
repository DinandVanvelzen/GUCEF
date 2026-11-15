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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#include "gucefASSETSTORE_CAssetStoreDownloadManager.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H ? */
#include "gucefASSETSTORE_CAssetStoreDownloadTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetStoreDownloadTaskConsumer::TaskType = "GUCEF::ASSETSTORE::CAssetStoreDownloadTaskConsumer";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStoreDownloadTaskConsumer::CAssetStoreDownloadTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreDownloadTaskConsumer::~CAssetStoreDownloadTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreDownloadTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreDownloadTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreDownloadTaskConsumer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CAssetStoreDownloadTaskData* data = const_cast< CAssetStoreDownloadTaskData* >( CORE::CICloneable::CloneableAs< CAssetStoreDownloadTaskData >( task->GetTaskData() ) );
    if ( GUCEF_NULL == data || GUCEF_NULL == data->manager )
        return true;
    CAssetStoreDownloadManager* manager = data->manager;
    data->manager = GUCEF_NULL;     // the manager is told once that the job task ended
    try
    {
        manager->RunJob( data->jobId );
    }
    catch ( ... )
    {
        manager->OnJobException( data->jobId );
    }
    manager->OnJobTaskEnded( data->jobId );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    if ( !task.IsNULL() )
    {
        CAssetStoreDownloadTaskData* data = const_cast< CAssetStoreDownloadTaskData* >( CORE::CICloneable::CloneableAs< CAssetStoreDownloadTaskData >( task->GetTaskData() ) );
        if ( GUCEF_NULL != data && GUCEF_NULL != data->manager )
        {
            CAssetStoreDownloadManager* manager = data->manager;
            data->manager = GUCEF_NULL;
            manager->OnJobTaskEnded( data->jobId );
        }
    }
    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
