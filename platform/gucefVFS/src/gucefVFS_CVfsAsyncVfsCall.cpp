/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#include <stdexcept>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSASYNCCALLGATESCOPE_H
#include "gucefVFS_CVfsAsyncCallGateScope.h"
#define GUCEF_VFS_CVFSASYNCCALLGATESCOPE_H
#endif /* GUCEF_VFS_CVFSASYNCCALLGATESCOPE_H ? */

#ifndef GUCEF_VFS_CVFSEITHERCANCELSOURCE_H
#include "gucefVFS_CVfsEitherCancelSource.h"
#define GUCEF_VFS_CVFSEITHERCANCELSOURCE_H
#endif /* GUCEF_VFS_CVFSEITHERCANCELSOURCE_H ? */

#ifndef GUCEF_VFS_CVFSFETCHPROGRESSCANCELSCOPE_H
#include "gucefVFS_CVfsFetchProgressCancelScope.h"
#define GUCEF_VFS_CVFSFETCHPROGRESSCANCELSCOPE_H
#endif /* GUCEF_VFS_CVFSFETCHPROGRESSCANCELSCOPE_H ? */

#ifndef GUCEF_VFS_CVFSCACHEDIOACCESS_H
#include "gucefVFS_CVfsCachedIOAccess.h"
#define GUCEF_VFS_CVFSCACHEDIOACCESS_H
#endif /* GUCEF_VFS_CVFSCACHEDIOACCESS_H ? */

#include "gucefVFS_CVfsAsyncVfsCall.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsAsyncVfsCall::CVfsAsyncVfsCall( CVFS& vfs, const CVfsAsyncCallGatePtr& gate, EKind kind, const CVfsAsyncResultPtr& result )
    : CIVfsAsyncCall()
    , path()
    , recursive( false )
    , includePath( false )
    , filters()
    , maxEntries( GUCEF_UINT32MAX )
    , offset( 0 )
    , length( 0 )
    , request()
    , progress()
    , work()
    , m_vfs( &vfs )
    , m_gate( gate )
    , m_kind( kind )
    , m_result( result )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsAsyncVfsCall::~CVfsAsyncVfsCall()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CVfsAsyncVfsCall::Cancelled( void )
{GUCEF_TRACE;

    if ( !m_result.IsNULL() )
        m_result->result = CVfsCallResult( CORE::FILESYSTEMACCESS_CANCELLED, "the call was cancelled" );
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsAsyncVfsCall::FailureOf( CORE::TFileSystemAccessResult fallback, const CORE::CString& message ) const
{GUCEF_TRACE;

    // the remote mount that failed the call said why, for the thread that made it: this one
    CVfsCallResult recorded;
    m_vfs->GetLastResult( recorded );
    if ( !recorded.IsSuccess() )
        return recorded;
    return CVfsCallResult( fallback, message );
}

/*-------------------------------------------------------------------------*/

void
CVfsAsyncVfsCall::Run( const CIVfsCancelSource& cancel )
{GUCEF_TRACE;

    // the VFS is only used while the door is open; once it is shut the VFS may be going and the call does nothing
    CVfsAsyncCallGateScope inside( m_gate );
    if ( !inside.IsEntered() )
    {
        if ( !m_result.IsNULL() )
            m_result->result = CVfsCallResult( CORE::FILESYSTEMACCESS_CANCELLED, "the VFS is shut down" );
        return;
    }

    CVfsEitherCancelSource stop( cancel, *m_gate.GetPointerAlways() );
    try
    {
        RunKind( stop );
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsAsyncVfsCall: The call of \"" << path << "\" threw: " << e.what() );
        if ( !m_result.IsNULL() )
            m_result->result = CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, CORE::CString( "exception: " ) + e.what() );
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsAsyncVfsCall: The call of \"" << path << "\" threw" );
        if ( !m_result.IsNULL() )
            m_result->result = CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "exception" );
    }
}

/*-------------------------------------------------------------------------*/

void
CVfsAsyncVfsCall::RunKind( const CIVfsCancelSource& stop )
{GUCEF_TRACE;

    switch ( m_kind )
    {
        case KIND_GETFILEMETADATA:      RunGetFileMetaData( stop ); break;
        case KIND_GETFILELIST:          RunList( stop, false ); break;
        case KIND_GETDIRLIST:           RunList( stop, true ); break;
        case KIND_GETFILE:              RunGetFile( stop ); break;
        case KIND_READFILERANGE:        RunReadFileRange( stop ); break;
        case KIND_FETCHFILE:            RunFetchFile( stop ); break;
        case KIND_PREFETCH:             RunPrefetch( stop ); break;
        case KIND_PREFETCHHELPER:
        {
            if ( !work.IsNULL() )
            {
                work->HelperStarted();
                try
                {
                    WorkOnFiles( stop, work );
                }
                catch ( ... )
                {
                    work->HelperEnded();
                    throw;
                }
                work->HelperEnded();
            }
            break;
        }
        default:
        {
            if ( !m_result.IsNULL() )
                m_result->result = CVfsCallResult( CORE::FILESYSTEMACCESS_NOT_SUPPORTED, "unknown kind of call" );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CVfsAsyncVfsCall::RunGetFileMetaData( const CIVfsCancelSource& stop )
{GUCEF_TRACE;

    if ( stop.IsCancelRequested() )
    {
        Cancelled();
        return;
    }

    const CORE::CString filepath = m_vfs->ConformVfsFilePath( path );
    m_vfs->m_lastResults.Clear();

    CVFS::TConstMountLinkVector mountLinks;
    m_vfs->SnapshotMounts( filepath, false, mountLinks );

    CORE::CResourceMetaData metaData;
    CVfsDigest digest;
    const bool found = m_vfs->GetFileMetaDataFromMounts( mountLinks, metaData, true, &digest );

    m_result->exists = found;
    m_result->metaData = metaData;
    m_result->digest = digest;
    m_result->result = found ? CVfsCallResult() : FailureOf( CORE::FILESYSTEMACCESS_NOT_FOUND, "\"" + path + "\" is not there" );
}

/*-------------------------------------------------------------------------*/

void
CVfsAsyncVfsCall::RunList( const CIVfsCancelSource& stop, bool directories )
{GUCEF_TRACE;

    if ( stop.IsCancelRequested() )
    {
        Cancelled();
        return;
    }

    const CORE::CString location = m_vfs->ConformVfsDirPath( path );
    m_vfs->m_lastResults.Clear();

    CVFS::TConstMountLinkVector mountLinks;
    m_vfs->SnapshotMounts( location, false, mountLinks );

    CORE::CString::StringVector names;
    const bool complete = m_vfs->ListFromMounts( mountLinks, names, directories, recursive, includePath, filters, maxEntries, true );

    m_result->names = names;
    m_result->result = complete ? CVfsCallResult() : FailureOf( CORE::FILESYSTEMACCESS_FAILED, "the listing of \"" + path + "\" is not complete" );
}

/*-------------------------------------------------------------------------*/

void
CVfsAsyncVfsCall::RunGetFile( const CIVfsCancelSource& stop )
{GUCEF_TRACE;

    if ( stop.IsCancelRequested() )
    {
        Cancelled();
        return;
    }

    const CORE::CString filepath = m_vfs->ConformVfsFilePath( path );
    if ( filepath.IsNULLOrEmpty() )
    {
        m_result->result = CVfsCallResult( CORE::FILESYSTEMACCESS_INVALID_PATH, "the path of the file is empty" );
        return;
    }
    m_vfs->m_lastResults.Clear();

    CVFS::TConstMountLinkVector mountLinks;
    m_vfs->SnapshotMounts( filepath, false, mountLinks );

    TBasicVfsResourcePtr resource = m_vfs->OpenFromMounts( mountLinks, filepath, path, "rb", false, true );
    m_result->resource = resource;
    m_result->result = resource.IsNULL() ? FailureOf( CORE::FILESYSTEMACCESS_NOT_FOUND, "\"" + path + "\" could not be opened" ) : CVfsCallResult();
}

/*-------------------------------------------------------------------------*/

void
CVfsAsyncVfsCall::RunReadFileRange( const CIVfsCancelSource& stop )
{GUCEF_TRACE;

    if ( stop.IsCancelRequested() )
    {
        Cancelled();
        return;
    }

    const UInt32 maxBytes = m_vfs->GetMaxAsyncReadBytes();
    if ( length > maxBytes )
    {
        m_result->result = CVfsCallResult( CORE::FILESYSTEMACCESS_TOO_LONG, "the range of " + CORE::ToString( length ) + " bytes is longer than the " + CORE::ToString( maxBytes ) + " bytes one call reads" );
        return;
    }

    const CORE::CString filepath = m_vfs->ConformVfsFilePath( path );
    if ( filepath.IsNULLOrEmpty() )
    {
        m_result->result = CVfsCallResult( CORE::FILESYSTEMACCESS_INVALID_PATH, "the path of the file is empty" );
        return;
    }
    m_vfs->m_lastResults.Clear();

    CVFS::TConstMountLinkVector mountLinks;
    m_vfs->SnapshotMounts( filepath, false, mountLinks );

    TBasicVfsResourcePtr resource = m_vfs->OpenFromMounts( mountLinks, filepath, path, "rb", false, true );
    if ( resource.IsNULL() )
    {
        m_result->result = FailureOf( CORE::FILESYSTEMACCESS_NOT_FOUND, "\"" + path + "\" could not be opened" );
        return;
    }

    CORE::CIOAccess* access = resource->GetAccess().GetPointerAlways();
    if ( GUCEF_NULL == access )
    {
        m_result->result = CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "\"" + path + "\" has no access to read from" );
        return;
    }

    // the range that is there: a range that starts at or past the end of the file holds no bytes
    const UInt64 fileSize = access->GetSize();
    UInt64 want = length;
    if ( offset >= fileSize )
        want = 0;
    else
    if ( want > fileSize - offset )
        want = fileSize - offset;
    if ( 0 == want )
    {
        m_result->result = CVfsCallResult();
        return;
    }

    if ( stop.IsCancelRequested() )
    {
        Cancelled();
        return;
    }

    m_result->data.SetDataSize( (UInt32) want );
    if ( 0 != access->Setpos( offset ) )
    {
        m_result->data.SetDataSize( 0 );
        m_result->result = CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "\"" + path + "\" could not be positioned at " + CORE::ToString( offset ) );
        return;
    }

    const UInt32 got = access->Read( m_result->data.GetBufferPtr(), 1, (UInt32) want );
    m_result->data.SetDataSize( got );
    m_result->bytesDone = got;
    if ( got == want )
    {
        m_result->result = CVfsCallResult();
        return;
    }

    // a short read before the end of the file is a failure and the access of a remote archive says why
    CVfsCallResult reason;
    #ifdef GUCEF_RTTI_IS_SUPPORTED
    const CVfsCachedIOAccess* cached = dynamic_cast< const CVfsCachedIOAccess* >( access );
    if ( GUCEF_NULL != cached )
        reason = cached->GetLastResult();
    #endif
    if ( reason.IsSuccess() )
        reason = CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "\"" + path + "\": " + CORE::ToString( got ) + " of " + CORE::ToString( want ) + " bytes could be read" );
    m_result->result = reason;
}

/*-------------------------------------------------------------------------*/

void
CVfsAsyncVfsCall::RunFetchFile( const CIVfsCancelSource& stop )
{GUCEF_TRACE;

    if ( stop.IsCancelRequested() || progress.IsNULL() )
    {
        Cancelled();
        return;
    }

    // the cancellation request of the future counts as the cancel flag of the progress for as long as the fetch runs
    CVfsFetchProgressCancelScope cancelScope( progress, &stop );

    const CORE::CString filepath = m_vfs->ConformVfsFilePath( path );
    m_vfs->m_lastResults.Clear();

    CVFS::TConstMountLinkVector mountLinks;
    m_vfs->SnapshotMounts( filepath, false, mountLinks );

    m_result->result = m_vfs->FetchFromMounts( mountLinks, path, request, *progress.GetPointerAlways(), true );
    m_result->bytesDone = progress->bytesDone.Load();
}

/*-------------------------------------------------------------------------*/

void
CVfsAsyncVfsCall::WorkOnFiles( const CIVfsCancelSource& stop, CVfsPrefetchWorkPtr files )
{GUCEF_TRACE;

    if ( files.IsNULL() || progress.IsNULL() )
        return;

    // the files are taken one by one: by the call that runs the prefetch and by the helpers, whoever is free
    CORE::CString file;
    while ( !stop.IsCancelRequested() && !progress->IsCancelRequested() && files->TakeNext( file ) )
    {
        const CORE::CString filepath = m_vfs->ConformVfsFilePath( file );

        CVFS::TConstMountLinkVector mountLinks;
        m_vfs->SnapshotMounts( filepath, false, mountLinks );
        files->NoteEnded( m_vfs->FetchFromMounts( mountLinks, file, request, *progress.GetPointerAlways(), true ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CVfsAsyncVfsCall::RunPrefetch( const CIVfsCancelSource& stop )
{GUCEF_TRACE;

    if ( stop.IsCancelRequested() || progress.IsNULL() )
    {
        Cancelled();
        return;
    }

    CVfsFetchProgressCancelScope cancelScope( progress, &stop );
    m_vfs->m_lastResults.Clear();

    // the listings first: what they put into the metadata of the archives is what the files are then resolved from
    UInt32 directoriesDone = 0;
    UInt32 directoriesFailed = 0;
    bool cancelled = false;
    CVfsCallResult firstFailure;
    for ( size_t i=0; i<request.directories.size(); ++i )
    {
        if ( stop.IsCancelRequested() || progress->IsCancelRequested() )
        {
            cancelled = true;
            break;
        }

        const CORE::CString location = m_vfs->ConformVfsDirPath( request.directories[ i ] );
        CVFS::TConstMountLinkVector mountLinks;
        m_vfs->SnapshotMounts( location, false, mountLinks );
        const CVfsCallResult listed = m_vfs->FetchDirectoryFromMounts( mountLinks, request.directories[ i ], request.recursive, request.maxDirectories, *progress.GetPointerAlways(), true );
        if ( listed.IsSuccess() )
        {
            ++directoriesDone;
        }
        else
        if ( CORE::FILESYSTEMACCESS_CANCELLED == listed.result )
        {
            cancelled = true;
            break;
        }
        else
        {
            ++directoriesFailed;
            if ( firstFailure.IsSuccess() )
                firstFailure = listed;
        }
    }

    // then the files: this call works through the list and gives helpers to the pool, which take files from the same list
    CVfsPrefetchWorkPtr files( GUCEF_NEW CVfsPrefetchWork( request.files ) );
    GUCEF::vector< CORE::CFutureResult > helperFutures;
    UInt32 parallel = request.maxParallelFiles < 1 ? 1 : request.maxParallelFiles;
    if ( parallel > request.files.size() )
        parallel = (UInt32) request.files.size();
    for ( UInt32 n=1; n<parallel && !cancelled; ++n )
    {
        CVfsAsyncVfsCall* helper = GUCEF_NEW CVfsAsyncVfsCall( *m_vfs, m_gate, KIND_PREFETCHHELPER, CVfsAsyncResultPtr() );
        helper->request = request;
        helper->progress = progress;
        helper->work = files;
        helperFutures.push_back( m_vfs->QueueAsyncCall( helper, CVfsAsyncResultPtr() ) );       // a pool without a free thread or room: this call does the files by itself
    }
    if ( !cancelled )
    {
        WorkOnFiles( stop, files );
    }

    // the list is empty: a helper that no thread took has nothing to do and is taken out of the queue of the pool, where a pool that
    // stops would leave it behind. A helper that took a file is waited for
    for ( size_t h=0; h<helperFutures.size(); ++h )
        helperFutures[ h ].RequestCancellation();
    while ( files->GetHelpersRunning() > 0 )
        MT::ThreadDelay( 1 );

    const UInt32 filesFailed = files->GetFilesFailed();
    const UInt32 filesDone = files->GetFilesSucceeded();
    cancelled = cancelled || files->HasCancelled() || ( !request.files.empty() && files->GetFilesTaken() < request.files.size() );
    if ( firstFailure.IsSuccess() )
        firstFailure = files->GetFirstFailure();

    const UInt32 done = directoriesDone + filesDone;
    const UInt32 failed = directoriesFailed + filesFailed;
    m_result->bytesDone = progress->bytesDone.Load();
    if ( cancelled )
        m_result->result = CVfsCallResult( CORE::FILESYSTEMACCESS_CANCELLED, "the prefetch was cancelled, " + CORE::ToString( done ) + " of the listings and files were done" );
    else
    if ( 0 == failed )
        m_result->result = CVfsCallResult();
    else
    if ( 0 == done )
        m_result->result = firstFailure;
    else
        m_result->result = CVfsCallResult( CORE::FILESYSTEMACCESS_PARTIAL_SUCCESS, CORE::ToString( failed ) + " of " + CORE::ToString( done + failed ) + " listings and files failed, the first: " + firstFailure.ToString() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
