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

#ifndef GUCEF_VFS_CVFSFETCHREQUEST_H
#define GUCEF_VFS_CVFSFETCHREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

#ifndef GUCEF_VFS_ETYPES_H
#include "gucefVFS_ETypes.h"
#define GUCEF_VFS_ETYPES_H
#endif /* GUCEF_VFS_ETYPES_H ? */

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

/** What a fetch of files from a remote archive asks for: which files, whether to keep them offline, whether to check them */
class GUCEF_VFS_PUBLIC_CPP CVfsFetchRequest
{
    public:

    CORE::CString::StringVector files;              /**< paths, fetched whole */
    CORE::CString::StringVector directories;        /**< paths of directories whose listings are fetched (and those below them when 'recursive') */
    bool recursive;
    UInt32 maxDirectories;                          /**< the bound of a recursive walk (default 1000) */
    bool pin;                                       /**< keep the files offline: the cache does not evict them */
    bool verifyDigest;                              /**< check a complete file against its digest when one is known (default true) */
    UInt32 maxParallelFiles;                        /**< the files fetched at once by a request of several (default 4) */

    CVfsFetchRequest( void );

    CVfsFetchRequest( const CVfsFetchRequest& src );

    CVfsFetchRequest& operator=( const CVfsFetchRequest& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSFETCHREQUEST_H ? */
