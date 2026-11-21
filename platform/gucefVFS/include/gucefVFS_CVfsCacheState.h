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

#ifndef GUCEF_VFS_CVFSCACHESTATE_H
#define GUCEF_VFS_CVFSCACHESTATE_H

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

/** What the cache of a remote archive holds of one file: the answer of CIRemoteArchive::GetFileCacheState() */
class GUCEF_VFS_PUBLIC_CPP CVfsCacheState
{
    public:

    bool cached;                        /**< at least one block is held */
    bool complete;                      /**< every block is held */
    bool pinned;                        /**< kept offline */
    bool superseded;                    /**< a newer revision of the file is known */
    bool staleRevision;                 /**< the state is that of an older, complete, pinned revision that stands in for the current one, which is not held completely */
    bool digestVerified;                /**< the content was hashed and has 'digest' */
    UInt64 fileSize;
    UInt64 bytesHeld;
    CORE::CString revision;
    CVfsDigest digest;

    CVfsCacheState( void );

    CVfsCacheState( const CVfsCacheState& src );

    CVfsCacheState& operator=( const CVfsCacheState& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSCACHESTATE_H ? */
