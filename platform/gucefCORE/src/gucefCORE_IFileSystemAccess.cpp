/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CASYNCFILESYSTEMACCESSOPERATION_H
#include "gucefCORE_CAsyncFileSystemAccessOperation.h"
#define GUCEF_CORE_CASYNCFILESYSTEMACCESSOPERATION_H
#endif /* GUCEF_CORE_CASYNCFILESYSTEMACCESSOPERATION_H ? */

#include "gucefCORE_IFileSystemAccess.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The data of the ended task of a future, NULL while it is not ready or when the future does not carry file system access data */
const CFileSystemAccessTaskData*
GetEndedTaskData( const CFutureResult& future, CTaskPtr& task )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return GUCEF_NULL;
    task = future.GetResult();
    if ( task.IsNULL() )
        return GUCEF_NULL;
    return CICloneable::CloneableAs< CFileSystemAccessTaskData >( task->GetTaskData() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

IFileSystemAccess::IFileSystemAccess( void )
    : CTSharedPtrCreator< IFileSystemAccess, MT::CMutex >( this )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

IFileSystemAccess::~IFileSystemAccess()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
IFileSystemAccess::QueueAsyncOperation( CFileSystemAccessTaskData& taskData, CFutureResult& future, const CString& threadPoolName )
{GUCEF_TRACE;

    try
    {
        taskData.access = CreateSharedPtr();
        CTaskManager& taskManager = CCoreGlobal::Instance()->GetTaskManager();
        ThreadPoolPtr threadPool = taskManager.GetOrCreateThreadPool( threadPoolName.IsNULLOrEmpty() ? CTaskManager::DefaultThreadPoolName : threadPoolName );
        if ( !threadPool.IsNULL() )
        {
            future = threadPool->QueueTask( CAsyncFileSystemAccessOperation::TaskType, &taskData );
            if ( future.HasAFuture() )
                return FILESYSTEMACCESS_SUCCESS;
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_IMPORTANT, "FileSystemAccess:QueueAsyncOperation: Failed to queue the operation: " << e.what() );
    }
    future = CFutureResult( TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
    return FILESYSTEMACCESS_QUEUEING_FAILED;
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
IFileSystemAccess::TryCreateDirectoryAsync( const CString& path, bool createParents, CFutureResult& future, const CString& threadPoolName )
{GUCEF_TRACE;

    CFileSystemAccessTaskData taskData;
    taskData.operation = FILESYSTEMACCESSOPERATION_CREATE_DIRECTORY;
    taskData.path = path;
    taskData.flag = createParents;
    return QueueAsyncOperation( taskData, future, threadPoolName );
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
IFileSystemAccess::TryDeleteDirectoryAsync( const CString& path, bool deleteContents, CFutureResult& future, const CString& threadPoolName )
{GUCEF_TRACE;

    CFileSystemAccessTaskData taskData;
    taskData.operation = FILESYSTEMACCESSOPERATION_DELETE_DIRECTORY;
    taskData.path = path;
    taskData.flag = deleteContents;
    return QueueAsyncOperation( taskData, future, threadPoolName );
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
IFileSystemAccess::TryMoveDirectoryAsync( const CString& oldPath, const CString& newPath, CFutureResult& future, const CString& threadPoolName )
{GUCEF_TRACE;

    CFileSystemAccessTaskData taskData;
    taskData.operation = FILESYSTEMACCESSOPERATION_MOVE_DIRECTORY;
    taskData.path = oldPath;
    taskData.secondPath = newPath;
    return QueueAsyncOperation( taskData, future, threadPoolName );
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
IFileSystemAccess::TryMoveFileAsync( const CString& oldPath, const CString& newPath, bool overwrite, CFutureResult& future,
                                     const CString& threadPoolName )
{GUCEF_TRACE;

    CFileSystemAccessTaskData taskData;
    taskData.operation = FILESYSTEMACCESSOPERATION_MOVE_FILE;
    taskData.path = oldPath;
    taskData.secondPath = newPath;
    taskData.flag = overwrite;
    return QueueAsyncOperation( taskData, future, threadPoolName );
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
IFileSystemAccess::TryMoveFilesAsync( const CString::StringVector& filePaths, const CString& destinationDir, bool overwrite,
                                      CFutureResult& future, const CString& threadPoolName )
{GUCEF_TRACE;

    CFileSystemAccessTaskData taskData;
    taskData.operation = FILESYSTEMACCESSOPERATION_MOVE_FILES;
    taskData.paths = filePaths;
    taskData.path = destinationDir;
    taskData.flag = overwrite;
    return QueueAsyncOperation( taskData, future, threadPoolName );
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
IFileSystemAccess::TryRenameFileAsync( const CString& path, const CString& newName, CFutureResult& future, const CString& threadPoolName )
{GUCEF_TRACE;

    CFileSystemAccessTaskData taskData;
    taskData.operation = FILESYSTEMACCESSOPERATION_RENAME_FILE;
    taskData.path = path;
    taskData.secondPath = newName;
    return QueueAsyncOperation( taskData, future, threadPoolName );
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
IFileSystemAccess::TryDeleteFileAsync( const CString& path, CFutureResult& future, const CString& threadPoolName )
{GUCEF_TRACE;

    CFileSystemAccessTaskData taskData;
    taskData.operation = FILESYSTEMACCESSOPERATION_DELETE_FILE;
    taskData.path = path;
    return QueueAsyncOperation( taskData, future, threadPoolName );
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
IFileSystemAccess::TryGetMetaDataAsync( const CString& path, CFutureResult& future, const CString& threadPoolName )
{GUCEF_TRACE;

    CFileSystemAccessTaskData taskData;
    taskData.operation = FILESYSTEMACCESSOPERATION_GET_METADATA;
    taskData.path = path;
    return QueueAsyncOperation( taskData, future, threadPoolName );
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
IFileSystemAccess::TryGetEntryTypeAsync( const CString& path, CFutureResult& future, const CString& threadPoolName )
{GUCEF_TRACE;

    CFileSystemAccessTaskData taskData;
    taskData.operation = FILESYSTEMACCESSOPERATION_GET_ENTRY_TYPE;
    taskData.path = path;
    return QueueAsyncOperation( taskData, future, threadPoolName );
}

/*-------------------------------------------------------------------------*/

TFileSystemAccessResult
IFileSystemAccess::TryGetFileSystemIteratorAsync( const CString& dirPath, CFutureResult& future, const CString& threadPoolName )
{GUCEF_TRACE;

    CFileSystemAccessTaskData taskData;
    taskData.operation = FILESYSTEMACCESSOPERATION_GET_ITERATOR;
    taskData.path = dirPath;
    return QueueAsyncOperation( taskData, future, threadPoolName );
}

/*-------------------------------------------------------------------------*/

bool
IFileSystemAccess::TryGetAsyncResult( const CFutureResult& future, TFileSystemAccessResult& result )
{GUCEF_TRACE;

    if ( future.HasNoFuture() )
    {
        // a future that never was: the operation could not be queued
        result = FILESYSTEMACCESS_QUEUEING_FAILED;
        return true;
    }
    CTaskPtr task;
    const CFileSystemAccessTaskData* data = GetEndedTaskData( future, task );
    if ( task.IsNULL() )
        return false;
    if ( TTaskStatus::TASKSTATUS_CANCELLED == task->GetTaskStatus() )
        result = FILESYSTEMACCESS_CANCELLED;
    else
    if ( TTaskStatus::TASKSTATUS_QUEUEING_FAILED == task->GetTaskStatus() )
        result = FILESYSTEMACCESS_QUEUEING_FAILED;
    else
    if ( GUCEF_NULL == data )
        result = FILESYSTEMACCESS_FAILED;
    else
        result = data->result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
IFileSystemAccess::TryGetAsyncPerFileResults( const CFutureResult& future, TFileSystemAccessResultVector& perFileResults )
{GUCEF_TRACE;

    CTaskPtr task;
    const CFileSystemAccessTaskData* data = GetEndedTaskData( future, task );
    if ( GUCEF_NULL == data )
        return false;
    perFileResults = data->perFileResults;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
IFileSystemAccess::TryGetAsyncMetaData( const CFutureResult& future, CResourceMetaData& metaData )
{GUCEF_TRACE;

    CTaskPtr task;
    const CFileSystemAccessTaskData* data = GetEndedTaskData( future, task );
    if ( GUCEF_NULL == data || FILESYSTEMACCESS_SUCCESS != data->result )
        return false;
    metaData = data->metaData;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
IFileSystemAccess::TryGetAsyncEntryType( const CFutureResult& future, TFileSystemEntryType& entryType )
{GUCEF_TRACE;

    CTaskPtr task;
    const CFileSystemAccessTaskData* data = GetEndedTaskData( future, task );
    if ( GUCEF_NULL == data || FILESYSTEMACCESS_SUCCESS != data->result )
        return false;
    entryType = data->entryType;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
IFileSystemAccess::TryGetAsyncIterator( const CFutureResult& future, TFileSystemIteratorPtr& iterator )
{GUCEF_TRACE;

    CTaskPtr task;
    const CFileSystemAccessTaskData* data = GetEndedTaskData( future, task );
    if ( GUCEF_NULL == data || FILESYSTEMACCESS_SUCCESS != data->result || data->iterator.IsNULL() )
        return false;
    iterator = data->iterator;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
