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

#ifndef GUCEF_VFS_CIVFSBLOCKSOURCE_H
#define GUCEF_VFS_CIVFSBLOCKSOURCE_H

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
 *  Where the bytes that a cache misses come from: one file of a remote place at one revision. The remote archive makes one
 *  per file when it is needed and gives it to the cached access (CVfsCachedIOAccess) or a background job, which asks for
 *  runs of bytes. It is used by one thread at a time and its calls may block on the remote place.
 */
class GUCEF_VFS_PUBLIC_CPP CIVfsBlockSource
{
    public:

    virtual ~CIVfsBlockSource() {}

    /**
     *  Reads 'length' bytes starting at 'offset' of the file into 'buffer'. 'bytesRead' tells how many bytes were delivered,
     *  which is all of them for a success. A result that is not a success may still have delivered a first part (the link
     *  dropped in the middle of a run). FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED says that the file is not at the revision
     *  of the source any more.
     */
    virtual CVfsCallResult FetchRange( UInt64 offset, UInt32 length, void* buffer, UInt32& bytesRead ) = 0;
};

typedef CORE::CTSharedPtr< CIVfsBlockSource, MT::CMutex > CIVfsBlockSourcePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CIVFSBLOCKSOURCE_H ? */
