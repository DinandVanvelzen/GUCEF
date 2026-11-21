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


#ifndef GUCEF_VFS_CVFSASYNCVFSCALL_H
#define GUCEF_VFS_CVFSASYNCVFSCALL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_VFS_CIVFSASYNCCALL_H
#include "gucefVFS_CIVfsAsyncCall.h"
#define GUCEF_VFS_CIVFSASYNCCALL_H
#endif /* GUCEF_VFS_CIVFSASYNCCALL_H ? */

#ifndef GUCEF_VFS_CVFSASYNCRESULT_H
#include "gucefVFS_CVfsAsyncResult.h"
#define GUCEF_VFS_CVFSASYNCRESULT_H
#endif /* GUCEF_VFS_CVFSASYNCRESULT_H ? */

#ifndef GUCEF_VFS_CVFSASYNCCALLGATE_H
#include "gucefVFS_CVfsAsyncCallGate.h"
#define GUCEF_VFS_CVFSASYNCCALLGATE_H
#endif /* GUCEF_VFS_CVFSASYNCCALLGATE_H ? */

#ifndef GUCEF_VFS_CVFSFETCHREQUEST_H
#include "gucefVFS_CVfsFetchRequest.h"
#define GUCEF_VFS_CVFSFETCHREQUEST_H
#endif /* GUCEF_VFS_CVFSFETCHREQUEST_H ? */

#ifndef GUCEF_VFS_CVFSFETCHPROGRESS_H
#include "gucefVFS_CVfsFetchProgress.h"
#define GUCEF_VFS_CVFSFETCHPROGRESS_H
#endif /* GUCEF_VFS_CVFSFETCHPROGRESS_H ? */

#ifndef GUCEF_VFS_CVFSPREFETCHWORK_H
#include "gucefVFS_CVfsPrefetchWork.h"
#define GUCEF_VFS_CVFSPREFETCHWORK_H
#endif /* GUCEF_VFS_CVFSPREFETCHWORK_H ? */

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

class CVFS;

/**
 *  The asynchronous calls of the VFS (CVFS::GetFileMetaDataAsync() and its kind) as one piece of work for a thread of the pool: it
 *  takes the links of the mounts that cover the path while it holds the lock of the VFS, lets go of the lock and then makes
 *  the same loop over the mounts as the synchronous call does, calling remote archives without the lock. Only the members
 *  of the kind of call are used. The call enters the gate of the VFS first: when the VFS has gone it does nothing, and the VFS
 *  does not go while a call is inside.
 *
 *  A prefetch queues helpers of its own kind (KIND_PREFETCHHELPER) that share its list of files; see CVfsPrefetchWork.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsAsyncVfsCall : public CIVfsAsyncCall
{
    public:

    enum EKind
    {
        KIND_GETFILEMETADATA = 0,
        KIND_GETFILELIST,
        KIND_GETDIRLIST,
        KIND_GETFILE,
        KIND_READFILERANGE,
        KIND_FETCHFILE,
        KIND_PREFETCH,
        KIND_PREFETCHHELPER
    };

    // what the kinds of call work from
    CORE::CString path;                         /**< the file, or the location of a listing */
    bool recursive;                             /**< listings */
    bool includePath;                           /**< listings */
    CORE::CString::StringSet filters;           /**< listings */
    UInt32 maxEntries;                          /**< listings */
    UInt64 offset;                              /**< range read */
    UInt32 length;                              /**< range read */
    CVfsFetchRequest request;                   /**< fetches */
    CVfsFetchProgressPtr progress;              /**< fetches */
    CVfsPrefetchWorkPtr work;                   /**< the helpers of a prefetch */

    /** 'result' is where the call puts what it delivers; none for a helper of a prefetch */
    CVfsAsyncVfsCall( CVFS& vfs, const CVfsAsyncCallGatePtr& gate, EKind kind, const CVfsAsyncResultPtr& result );

    virtual ~CVfsAsyncVfsCall() GUCEF_VIRTUAL_OVERRIDE;

    virtual void Run( const CIVfsCancelSource& cancel ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CVfsAsyncVfsCall( const CVfsAsyncVfsCall& src );                    /**< not implemented */
    CVfsAsyncVfsCall& operator=( const CVfsAsyncVfsCall& src );         /**< not implemented */

    void RunKind( const CIVfsCancelSource& stop );
    void RunGetFileMetaData( const CIVfsCancelSource& stop );
    void RunList( const CIVfsCancelSource& stop, bool directories );
    void RunGetFile( const CIVfsCancelSource& stop );
    void RunReadFileRange( const CIVfsCancelSource& stop );
    void RunFetchFile( const CIVfsCancelSource& stop );
    void RunPrefetch( const CIVfsCancelSource& stop );
    void WorkOnFiles( const CIVfsCancelSource& stop, CVfsPrefetchWorkPtr files );

    /** What the call that just failed made of it: the result the remote mount recorded for the thread, else the given one */
    CVfsCallResult FailureOf( CORE::TFileSystemAccessResult fallback, const CORE::CString& message ) const;

    void Cancelled( void );

    private:

    CVFS* m_vfs;
    CVfsAsyncCallGatePtr m_gate;
    EKind m_kind;
    CVfsAsyncResultPtr m_result;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSASYNCVFSCALL_H ? */
