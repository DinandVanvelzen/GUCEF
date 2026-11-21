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

#ifndef GUCEF_VFS_CIREMOTEARCHIVE_H
#define GUCEF_VFS_CIREMOTEARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_VFS_CVFSDIGEST_H
#include "gucefVFS_CVfsDigest.h"
#define GUCEF_VFS_CVFSDIGEST_H
#endif /* GUCEF_VFS_CVFSDIGEST_H ? */

#ifndef GUCEF_VFS_CVFSCALLRESULT_H
#include "gucefVFS_CVfsCallResult.h"
#define GUCEF_VFS_CVFSCALLRESULT_H
#endif /* GUCEF_VFS_CVFSCALLRESULT_H ? */

#ifndef GUCEF_VFS_CVFSCACHESTATE_H
#include "gucefVFS_CVfsCacheState.h"
#define GUCEF_VFS_CVFSCACHESTATE_H
#endif /* GUCEF_VFS_CVFSCACHESTATE_H ? */

#ifndef GUCEF_VFS_CVFSFETCHREQUEST_H
#include "gucefVFS_CVfsFetchRequest.h"
#define GUCEF_VFS_CVFSFETCHREQUEST_H
#endif /* GUCEF_VFS_CVFSFETCHREQUEST_H ? */

#ifndef GUCEF_VFS_CVFSFETCHPROGRESS_H
#include "gucefVFS_CVfsFetchProgress.h"
#define GUCEF_VFS_CVFSFETCHPROGRESS_H
#endif /* GUCEF_VFS_CVFSFETCHPROGRESS_H ? */

#ifndef GUCEF_VFS_CVFSREMOTESTATISTICS_H
#include "gucefVFS_CVfsRemoteStatistics.h"
#define GUCEF_VFS_CVFSREMOTESTATISTICS_H
#endif /* GUCEF_VFS_CVFSREMOTESTATISTICS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What an archive of a remote or slow place offers beyond the CArchive interface: the reason of the last failure, the
 *  state of what it knows and holds locally, digest hints, keeping files offline, fetching whole files and its statistics.
 *  An archive that has it returns it from CArchive::GetRemoteArchiveInterface(); every other archive returns nothing there
 *  and costs nothing.
 *
 *  A mount of an archive with this interface is a remote mount to the VFS: when a call on it ends because the data is not
 *  here yet or the link is down (FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE, FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING)
 *  the VFS does not try the mounts below it, which could answer with another file of the same path.
 *
 *  Paths are relative to the archive. The calls that say 'the calling thread' keep their state per thread.
 */
class GUCEF_VFS_PUBLIC_CPP CIRemoteArchive
{
    public:

    virtual ~CIRemoteArchive() {}

    /** The result of the latest call of the calling thread on the archive, a success when the call went well or none was made */
    virtual CVfsCallResult GetLastResult( void ) const = 0;

    enum EMetadataState
    {
        METADATA_UNKNOWN = 0,           /**< nothing is known of the path */
        METADATA_FRESH,                 /**< known, and not older than the time to live */
        METADATA_STALE                  /**< known, older than the time to live: answered and checked again, or only when the link is down */
    };

    /** What the archive knows of a file or a directory in memory */
    virtual EMetadataState GetMetadataState( const CORE::CString& path ) const = 0;

    /** Forgets what the archive knows of every path: the next call asks the remote place */
    virtual void InvalidateMetadata( void ) = 0;

    /** Forgets what the archive knows of one path: its record, its listing when it is a directory, and the listing of its parent */
    virtual void InvalidateMetadata( const CORE::CString& path ) = 0;

    /**
     *  Gives the archive a digest the application knows of a file (a catalogue has it). The revision it belongs to may be
     *  empty for any revision. The archive answers it when the remote place knows none, and checks a complete copy in its
     *  cache against it once; a copy that does not match is emptied and the call records FILESYSTEMACCESS_RESOURCE_CORRUPT.
     *  The hint is the application's to keep right: ForgetFileDigests() or a new hint when its catalogue moves on.
     */
    virtual void NoteFileDigest( const CORE::CString& path, const CVfsDigest& digest, const CORE::CString& revision = CORE::CString::Empty ) = 0;

    virtual void ForgetFileDigests( void ) = 0;

    /** What the cache holds of the file; false when it holds nothing of it */
    virtual bool GetFileCacheState( const CORE::CString& path, CVfsCacheState& state ) const = 0;

    /** Keeps the file offline (the cache does not evict it) or lets it go; pinning needs a cache entry: the file was read or fetched before */
    virtual bool SetFilePinned( const CORE::CString& path, bool pinned ) = 0;

    /** Fetches every block of the file into the cache, on the calling thread. 'options' has the pin and digest verification flags */
    virtual CVfsCallResult FetchFile( const CORE::CString& path, const CVfsFetchRequest& options, CVfsFetchProgress& progress ) = 0;

    /** Fetches the listing of a directory and, when asked, of those below it (at most 'maxDirectories') into the metadata of the archive */
    virtual CVfsCallResult FetchDirectory( const CORE::CString& dir, bool recursive, UInt32 maxDirectories, CVfsFetchProgress& progress ) = 0;

    virtual void GetRemoteStatistics( CVfsRemoteStatistics& statistics ) const = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CIREMOTEARCHIVE_H ? */
