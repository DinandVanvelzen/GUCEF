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

#ifndef GUCEF_VFS_CIVFSREMOTEBACKEND_H
#define GUCEF_VFS_CIVFSREMOTEBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_VFS_CVFSCALLRESULT_H
#include "gucefVFS_CVfsCallResult.h"
#define GUCEF_VFS_CVFSCALLRESULT_H
#endif /* GUCEF_VFS_CVFSCALLRESULT_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEENTRY_H
#include "gucefVFS_CVfsRemoteEntry.h"
#define GUCEF_VFS_CVFSREMOTEENTRY_H
#endif /* GUCEF_VFS_CVFSREMOTEENTRY_H ? */

#ifndef GUCEF_VFS_CIVFSBLOCKSOURCE_H
#include "gucefVFS_CIVfsBlockSource.h"
#define GUCEF_VFS_CIVFSBLOCKSOURCE_H
#endif /* GUCEF_VFS_CIVFSBLOCKSOURCE_H ? */

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
 *  What a remote place gives the generic remote archive layer (CVfsRemoteArchiveCore): whether it can be reached, and
 *  the three things it can be asked, all on the calling thread and all of which may block: the metadata of a path, the
 *  listing of a directory, and a source of bytes for a file at a revision. Everything else a remote archive does
 *  (caching, offline answers, the miss rule of a thread, background fetches, events) the layer does with these.
 *
 *  Paths are relative to the share, '/' separated, without a leading or trailing separator; the share root is "".
 *  The calls may be made from several threads at once.
 */
class GUCEF_VFS_PUBLIC_CPP CIVfsRemoteBackend
{
    public:

    virtual ~CIVfsRemoteBackend() {}

    /** Whether the place can be reached now. A call that answers false does not reach the place at all */
    virtual bool IsLinkUp( void ) const = 0;

    /** The place in words for a message that tells a person why something is not available, like "share x of member y" */
    virtual CORE::CString GetPlaceDescription( void ) const;

    /**
     *  The metadata of a path. A success with entry.exists false says that the place has nothing at the path; a result that
     *  is not a success says that the place could not tell (the link, a timeout, a limit), which is not the same.
     */
    virtual CVfsCallResult Stat( const CORE::CString& path, CVfsRemoteEntry& entry ) = 0;

    /**
     *  The entries directly in a directory (name, kind, size, revision, digest). A success with dirExists false says that
     *  the directory is not there.
     */
    virtual CVfsCallResult List( const CORE::CString& dirPath, CVfsRemoteEntry::TVector& entries, bool& dirExists ) = 0;

    /** A source of the bytes of the file at the revision. An empty pointer with the result that says why when there is none */
    virtual CIVfsBlockSourcePtr OpenBlockSource( const CORE::CString& path, const CORE::CString& revision, CVfsCallResult& result ) = 0;

    /** Adds what only the transport knows (retries, open handles) to the statistics. The default adds nothing */
    virtual void GetBackendStatistics( CVfsRemoteStatistics& statistics ) const;

    /**
     *  The layer drops what it knows of a path, or of everything: a backend that keeps metadata of its own (its own record of stats and
     *  listings) drops that too, so that the next call goes to the place. The default does nothing. The path is relative to the share and normalised.
     */
    virtual void InvalidateMetadata( void );
    virtual void InvalidateMetadata( const CORE::CString& path );
};

typedef CORE::CTSharedPtr< CIVfsRemoteBackend, MT::CMutex > CIVfsRemoteBackendPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CIVFSREMOTEBACKEND_H ? */
