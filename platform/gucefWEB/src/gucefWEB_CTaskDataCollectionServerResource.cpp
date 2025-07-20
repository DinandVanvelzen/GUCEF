/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CDATANODESERIALIZABLEDATANODE_H
#include "gucefCORE_CDataNodeSerializableDataNode.h"
#define GUCEF_CORE_CDATANODESERIALIZABLEDATANODE_H
#endif /* GUCEF_CORE_CDATANODESERIALIZABLEDATANODE_H ? */

#ifndef GUCEF_WEB_CDATANODESERIALIZABLEHTTPSERVERRESOURCE_H
#include "gucefWEB_CDataNodeSerializableHttpServerResource.h"
#define GUCEF_WEB_CDATANODESERIALIZABLEHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CDATANODESERIALIZABLEHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CTDATANODESERIALIZABLEHTTPSERVERRESOURCEEXPANSION_H
#include "gucefWEB_CTDataNodeSerializableHttpServerResourceExpansion.h"
#define GUCEF_WEB_CTDATANODESERIALIZABLEHTTPSERVERRESOURCEEXPANSION_H
#endif /* GUCEF_WEB_CTDATANODESERIALIZABLEHTTPSERVERRESOURCEEXPANSION_H ? */

#include "gucefWEB_CTaskDataCollectionServerResource.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTaskDataCollectionServerResource::CTaskDataCollectionServerResource( void )
    : CCodecBasedHTTPServerResource()
{GUCEF_TRACE;

    m_allowCreate = true;
}

/*-------------------------------------------------------------------------*/

CTaskDataCollectionServerResource::~CTaskDataCollectionServerResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTaskDataCollectionServerResource::TCreateState 
CTaskDataCollectionServerResource::CreateResource( const CString& resourcePath                   ,
                                                   const CORE::CString& transactionID            ,
                                                   const CORE::CDataNode& input                  ,
                                                   const CORE::CString& representation           ,
                                                   const CORE::CString& params                   ,
                                                   THTTPServerResourcePtr& resourceOutput        ,
                                                   TStringVector& supportedRepresentationsOutput )
{GUCEF_TRACE;
    
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    CORE::CTaskPtr task;
    CORE::TTaskStatus taskStatus = taskManager.StartOrQueueTask( input, &task );
    if ( !CORE::TaskStatusIsAnError( taskStatus ) )
    {
        CDataNodeSerializableHttpServerResourcePtr taskDataResource = CDataNodeSerializableHttpServerResourcePtr( GUCEF_NEW CTDataNodeSerializableHttpServerResourceExpansion< CORE::CDataNodeSerializableDataNode >() ); 
        CORE::CDataNodeSerializableDataNode* taskData = static_cast< CORE::CDataNodeSerializableDataNode* >( taskDataResource->GetSerializable() );
        if ( GUCEF_NULL != taskData )
        {
            // Echo back a copy of the task data in the form that it was actually accepted / persisted
            CORE::CDataNodeSerializableSettings serializerSettings;
            if ( task->GetSerializedTaskDataCopy( *taskData, serializerSettings ) )
            {
                resourceOutput = taskDataResource.StaticCast< CIHTTPServerResource >();
                supportedRepresentationsOutput = m_serializationReps;
            }            
        }
        return TCreateState::CREATESTATE_CREATED;
    }
    else
    {
        switch ( taskStatus )
        {
            case CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID: 
                return TCreateState::CREATESTATE_DESERIALIZATIONFAILED;

            case CORE::TTaskStatus::TASKSTATUS_TASKTYPE_INVALID: 
                return TCreateState::CREATESTATE_NOTSUPPORTED;

            default:
            case CORE::TTaskStatus::TASKSTATUS_UNDEFINED:
            case CORE::TTaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE: 
            case CORE::TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED:
            case CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED:
            case CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED:
                return TCreateState::CREATESTATE_FAILED; 
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
