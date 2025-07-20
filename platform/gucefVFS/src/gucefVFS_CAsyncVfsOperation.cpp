/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <time.h> 

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "gucefVFS_CAsyncVfsOperation.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAsyncVfsOperation::TaskType = "AsyncVfsOperation";
const CORE::CEvent CAsyncVfsOperation::AsyncVfsOperationCompletedEvent = "GUCEF::VFS::CAsyncVfsOperation::AsyncVfsOperationCompletedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void 
CAsyncVfsOperation::RegisterEvents( void )
{GUCEF_TRACE;

    AsyncVfsOperationCompletedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAsyncVfsOperation::CAsyncVfsOperation()
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CAsyncVfsOperation::CAsyncVfsOperation( const CAsyncVfsOperation& src )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncVfsOperation::~CAsyncVfsOperation()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAsyncVfsOperation::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncVfsOperation::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    // This is a simplistic task, no bootstrap is required.
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskStart" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncVfsOperation::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;

    CORE::CDateTime startTime = CORE::CDateTime::NowUTCDateTime();
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::ToString( this ) + "):OnTaskCycle" );

    CORE::CICloneable* opaqueTaskData = task->GetTaskData();
    CAsyncVfsTaskData* syncCallData = static_cast< CAsyncVfsTaskData* >( opaqueTaskData );
    if ( GUCEF_NULL == syncCallData )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation:OnTaskCycle: No task data to operate upon" );
        return true;
    }

    CAsyncVfsTaskResultData syncCallResult;
    syncCallResult.SetTaskData( syncCallData );
    syncCallResult.operationType = syncCallData->operationType;

    switch ( syncCallData->operationType )
    {
        case ASYNCVFSOPERATIONTYPE_MOUNTARCHIVE:
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Async invocation of MountArchive" );
            
            CMountArchiveTaskData* specificSyncCallData = static_cast< CMountArchiveTaskData* >( syncCallData );
            bool success = CVfsGlobal::Instance()->GetVfs().MountArchive( specificSyncCallData->settings );

            syncCallResult.successState = success;
            syncCallResult.durationInMilliSecs = (UInt32) startTime.CORE::CDateTime::GetTimeDifferenceInMillisecondsToNow();

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Completed Async invocation of MountArchive" );

            if ( !NotifyObservers( AsyncVfsOperationCompletedEvent, &syncCallResult ) ) return true;
            break;
        }
        case ASYNCVFSOPERATIONTYPE_STOREDATAASFILE:
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Async invocation of StoreAsFile" );
            
            CStoreAsFileTaskData* specificSyncCallData = static_cast< CStoreAsFileTaskData* >( syncCallData );
            bool success = CVfsGlobal::Instance()->GetVfs().StoreAsFile( specificSyncCallData->filepath       , 
                                                                         specificSyncCallData->data           ,
                                                                         specificSyncCallData->offset         ,
                                                                         specificSyncCallData->overwrite      );
        
            syncCallResult.successState = success;
            syncCallResult.durationInMilliSecs = (UInt32) startTime.CORE::CDateTime::GetTimeDifferenceInMillisecondsToNow();

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Completed Async invocation of StoreAsFile" );
            
            if ( !NotifyObservers( AsyncVfsOperationCompletedEvent, &syncCallResult ) ) return true;
            break;
        }
        case ASYNCVFSOPERATIONTYPE_COPYFILE:
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Async invocation of CopyFile" );
            
            CCopyFileTaskData* specificSyncCallData = static_cast< CCopyFileTaskData* >( syncCallData );
            bool success = CVfsGlobal::Instance()->GetVfs().CopyFile( specificSyncCallData->originalFilepath , 
                                                                      specificSyncCallData->copyFilepath     ,
                                                                      specificSyncCallData->overwrite        );
        
            syncCallResult.successState = success;
            syncCallResult.durationInMilliSecs = (UInt32) startTime.CORE::CDateTime::GetTimeDifferenceInMillisecondsToNow();

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Completed Async invocation of CopyFile" );
            
            if ( !NotifyObservers( AsyncVfsOperationCompletedEvent, &syncCallResult ) ) return true;
            break;
        }
        case ASYNCVFSOPERATIONTYPE_COPYFILECONTENT:
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Async invocation of CopyFileContent" );
            
            CCopyFileContentTaskData* specificSyncCallData = static_cast< CCopyFileContentTaskData* >( syncCallData );
            bool success = CVfsGlobal::Instance()->GetVfs().CopyFileContent( specificSyncCallData->originalFilepath , 
                                                                             specificSyncCallData->copyFilepath     ,
                                                                             specificSyncCallData->overwrite        );
        
            syncCallResult.successState = success;
            syncCallResult.durationInMilliSecs = (UInt32) startTime.CORE::CDateTime::GetTimeDifferenceInMillisecondsToNow();

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Completed Async invocation of CopyFileContent" );
            
            if ( !NotifyObservers( AsyncVfsOperationCompletedEvent, &syncCallResult ) ) return true;
            break;
        }
        case ASYNCVFSOPERATIONTYPE_ENCODEFILE:
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Async invocation of EncodeFile" );
            
            CEncodeFileTaskData* specificSyncCallData = static_cast< CEncodeFileTaskData* >( syncCallData );
            bool success = CVfsGlobal::Instance()->GetVfs().EncodeFile( specificSyncCallData->originalFilepath , 
                                                                        specificSyncCallData->encodedFilepath  ,
                                                                        specificSyncCallData->overwrite        ,
                                                                        specificSyncCallData->codecFamily      ,
                                                                        specificSyncCallData->encodeCodec      );
        
            syncCallResult.successState = success;
            syncCallResult.durationInMilliSecs = (UInt32) startTime.CORE::CDateTime::GetTimeDifferenceInMillisecondsToNow();

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Completed Async invocation of EncodeFile" );
            
            if ( !NotifyObservers( AsyncVfsOperationCompletedEvent, &syncCallResult ) ) return true;
            break;
        }
        case ASYNCVFSOPERATIONTYPE_DECODEFILE:
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Async invocation of DecodeFile" );
            
            CDecodeFileTaskData* specificSyncCallData = static_cast< CDecodeFileTaskData* >( syncCallData );
            bool success = CVfsGlobal::Instance()->GetVfs().DecodeFile( specificSyncCallData->originalFilepath , 
                                                                        specificSyncCallData->decodedFilepath  ,
                                                                        specificSyncCallData->overwrite        ,
                                                                        specificSyncCallData->codecFamily      ,
                                                                        specificSyncCallData->decodeCodec      );
        
            syncCallResult.successState = success;
            syncCallResult.durationInMilliSecs = (UInt32) startTime.CORE::CDateTime::GetTimeDifferenceInMillisecondsToNow();

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Completed Async invocation of DecodeFile" );
            
            if ( !NotifyObservers( AsyncVfsOperationCompletedEvent, &syncCallResult ) ) return true;
            break;
        }
        case ASYNCVFSOPERATIONTYPE_ENCODEDATAASFILE:
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Async invocation of EncodeAsFileAsync" );
            
            CEncodeBufferAsFileTaskData* specificSyncCallData = static_cast< CEncodeBufferAsFileTaskData* >( syncCallData );
            bool success = CVfsGlobal::Instance()->GetVfs().EncodeAsFile( specificSyncCallData->data            , 
                                                                          specificSyncCallData->bufferOffset    , 
                                                                          specificSyncCallData->encodedFilepath ,
                                                                          specificSyncCallData->overwrite       ,
                                                                          specificSyncCallData->codecFamily     ,
                                                                          specificSyncCallData->encodeCodec     );
        
            syncCallResult.successState = success;
            syncCallResult.durationInMilliSecs = (UInt32) startTime.CORE::CDateTime::GetTimeDifferenceInMillisecondsToNow();

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskCycle: Completed Async invocation of EncodeAsFileAsync" );
            
            if ( !NotifyObservers( AsyncVfsOperationCompletedEvent, &syncCallResult ) ) return true;
            break;
        }
        case ASYNCVFSOPERATIONTYPE_UNKNOWN:
        default:
        {
            syncCallResult.successState = false;
            syncCallResult.durationInMilliSecs = (UInt32) startTime.CORE::CDateTime::GetTimeDifferenceInMillisecondsToNow();
            
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation:OnTaskCycle: Invalid or Unsupported operation type specified:" + 
                    CORE::Int32ToString( (Int32) syncCallData->operationType ) );
            break;
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CAsyncVfsOperation::OnTaskEnded( CORE::CTaskPtr task ,
                                 bool wasForced      )
{GUCEF_TRACE;

    // This is a simplistic task, no shutdown or cleanup is required.
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncVfsOperation(" + CORE::PointerToString( this ) + "):OnTaskEnded" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
