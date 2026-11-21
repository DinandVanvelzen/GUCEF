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


#ifndef GUCEF_VFS_CVFSASYNCRESULT_H
#define GUCEF_VFS_CVFSASYNCRESULT_H

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

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CRESOURCEMETADATA_H
#include "gucefCORE_CResourceMetaData.h"
#define GUCEF_CORE_CRESOURCEMETADATA_H
#endif /* GUCEF_CORE_CRESOURCEMETADATA_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFSCALLRESULT_H
#include "gucefVFS_CVfsCallResult.h"
#define GUCEF_VFS_CVFSCALLRESULT_H
#endif /* GUCEF_VFS_CVFSCALLRESULT_H ? */

#ifndef GUCEF_VFS_CVFSDIGEST_H
#include "gucefVFS_CVfsDigest.h"
#define GUCEF_VFS_CVFSDIGEST_H
#endif /* GUCEF_VFS_CVFSDIGEST_H ? */

#ifndef GUCEF_VFS_CVFSHANDLE_H
#include "gucefVFS_CVFSHandle.h"
#define GUCEF_VFS_CVFSHANDLE_H
#endif /* GUCEF_VFS_CVFSHANDLE_H ? */

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
 *  What an asynchronous VFS call delivers. The caller gets the future of the call and a shared pointer to one of these:
 *  the call fills it in on its pool thread and ends the future afterwards, so the caller reads it once the future is
 *  ready (never while the call runs). Only the members of the kind of call that was made are set: the metadata call sets
 *  'exists', 'metaData' and 'digest', the listings 'names', the open call 'resource', the range read 'data', the fetches 'bytesDone'.
 *
 *  'result' is a success when the call did what was asked and the reason otherwise, in the terms of the file system
 *  access layer (the result of the remote mount that failed the call when there was one). A call that did not run at all
 *  (cancelled before a thread took it, or the VFS went away first) leaves it at FILESYSTEMACCESS_CANCELLED, which is what
 *  it is made with.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsAsyncResult
{
    public:

    CVfsCallResult result;                      /**< set before the future ends */
    bool exists;                                /**< metadata: the file is there */
    CORE::CResourceMetaData metaData;           /**< metadata */
    CVfsDigest digest;                          /**< metadata: the digest the remote place gave, when it is a remote file and one is known */
    CORE::CString::StringVector names;          /**< the listings */
    TBasicVfsResourcePtr resource;              /**< the open call: the file opened for reading, usable on any thread */
    CORE::CDynamicBuffer data;                  /**< the range read: the bytes, as many as there were, not more than asked for */
    UInt64 bytesDone;                           /**< the range read: the bytes read; the fetches: the bytes that were added to what the cache held */

    /** Made as a call that has not run: result FILESYSTEMACCESS_CANCELLED, nothing else set */
    CVfsAsyncResult( void );

    private:

    CVfsAsyncResult( const CVfsAsyncResult& src );                  /**< not implemented */
    CVfsAsyncResult& operator=( const CVfsAsyncResult& src );       /**< not implemented */
};

typedef CORE::CTSharedPtr< CVfsAsyncResult, MT::CMutex > CVfsAsyncResultPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSASYNCRESULT_H ? */
