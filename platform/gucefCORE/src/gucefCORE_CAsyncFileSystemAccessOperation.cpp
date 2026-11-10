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

#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#include "gucefCORE_CFileSystemEntryListIterator.h"
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */

#include "gucefCORE_CAsyncFileSystemAccessOperation.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CAsyncFileSystemAccessOperation::TaskType = "AsyncFileSystemAccessOperation";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFileSystemAccessTaskData::CFileSystemAccessTaskData( void )
    : CICloneable()
    , access()
    , operation( FILESYSTEMACCESSOPERATION_UNKNOWN )
    , path()
    , secondPath()
    , flag( false )
    , paths()
    , result( FILESYSTEMACCESS_FAILED )
    , perFileResults()
    , metaData()
    , entryType( FILESYSTEMENTRYTYPE_NONE )
    , iterator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileSystemAccessTaskData::CFileSystemAccessTaskData( const CFileSystemAccessTaskData& src )
    : CICloneable( src )
    , access( src.access )
    , operation( src.operation )
    , path( src.path )
    , secondPath( src.secondPath )
    , flag( src.flag )
    , paths( src.paths )
    , result( src.result )
    , perFileResults( src.perFileResults )
    , metaData( src.metaData )
    , entryType( src.entryType )
    , iterator( src.iterator )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileSystemAccessTaskData::~CFileSystemAccessTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileSystemAccessTaskData&
CFileSystemAccessTaskData::operator=( const CFileSystemAccessTaskData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        access = src.access;
        operation = src.operation;
        path = src.path;
        secondPath = src.secondPath;
        flag = src.flag;
        paths = src.paths;
        result = src.result;
        perFileResults = src.perFileResults;
        metaData = src.metaData;
        entryType = src.entryType;
        iterator = src.iterator;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CFileSystemAccessTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CFileSystemAccessTaskData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CFileSystemAccessTaskData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CFileSystemAccessTaskData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::CORE::CFileSystemAccessTaskData";
}

/*-------------------------------------------------------------------------*/

CAsyncFileSystemAccessOperation::CAsyncFileSystemAccessOperation( void )
    : CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncFileSystemAccessOperation::~CAsyncFileSystemAccessOperation()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CAsyncFileSystemAccessOperation::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncFileSystemAccessOperation::OnTaskCycle( CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CFileSystemAccessTaskData* data = CICloneable::CloneableAs< CFileSystemAccessTaskData >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AsyncFileSystemAccessOperation: No task data to work on" );
        return true;
    }
    if ( data->access.IsNULL() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AsyncFileSystemAccessOperation: The task has no file system to work on" );
        data->result = FILESYSTEMACCESS_FAILED;
        return true;
    }
    if ( task->IsCancellationRequested() )
    {
        data->result = FILESYSTEMACCESS_CANCELLED;
        return true;
    }

    IFileSystemAccess& access = *data->access;
    switch ( data->operation )
    {
        case FILESYSTEMACCESSOPERATION_CREATE_DIRECTORY:
            data->result = access.TryCreateDirectory( data->path, data->flag );
            break;
        case FILESYSTEMACCESSOPERATION_DELETE_DIRECTORY:
            data->result = access.TryDeleteDirectory( data->path, data->flag );
            break;
        case FILESYSTEMACCESSOPERATION_MOVE_DIRECTORY:
            data->result = access.TryMoveDirectory( data->path, data->secondPath );
            break;
        case FILESYSTEMACCESSOPERATION_MOVE_FILE:
            data->result = access.TryMoveFile( data->path, data->secondPath, data->flag );
            break;
        case FILESYSTEMACCESSOPERATION_MOVE_FILES:
            data->result = access.TryMoveFiles( data->paths, data->path, data->flag, &data->perFileResults );
            break;
        case FILESYSTEMACCESSOPERATION_RENAME_FILE:
            data->result = access.TryRenameFile( data->path, data->secondPath );
            break;
        case FILESYSTEMACCESSOPERATION_DELETE_FILE:
            data->result = access.TryDeleteFile( data->path );
            break;
        case FILESYSTEMACCESSOPERATION_GET_METADATA:
            data->result = access.TryGetMetaData( data->path, data->metaData );
            break;
        case FILESYSTEMACCESSOPERATION_GET_ENTRY_TYPE:
            data->result = access.TryGetEntryType( data->path, data->entryType );
            break;
        case FILESYSTEMACCESSOPERATION_GET_ITERATOR:
        {
            // the whole directory is read here, on the pool thread
            TFileSystemIteratorPtr source;
            data->result = access.TryGetFileSystemIterator( data->path, source );
            if ( FILESYSTEMACCESS_SUCCESS == data->result && !source.IsNULL() )
            {
                CFileSystemEntryListIterator::TEntryVector entries;
                CFileSystemEntryListIterator::TryReadAll( *source, data->path, entries );
                data->iterator = TFileSystemIteratorPtr( GUCEF_NEW CFileSystemEntryListIterator( data->path, entries ) );
            }
            break;
        }
        default:
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AsyncFileSystemAccessOperation: Unknown operation " << (Int32) data->operation );
            data->result = FILESYSTEMACCESS_NOT_SUPPORTED;
            break;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAsyncFileSystemAccessOperation::OnTaskEnded( CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
