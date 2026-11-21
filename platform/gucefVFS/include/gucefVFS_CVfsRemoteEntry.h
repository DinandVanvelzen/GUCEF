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

#ifndef GUCEF_VFS_CVFSREMOTEENTRY_H
#define GUCEF_VFS_CVFSREMOTEENTRY_H

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

/** What a remote place says of a file or a directory: the answer to a stat, and an entry of a listing */
class GUCEF_VFS_PUBLIC_CPP CVfsRemoteEntry
{
    public:

    typedef GUCEF::vector< CVfsRemoteEntry > TVector;

    CORE::CString name;                 /**< the last segment of the path: set in the entries of a listing */
    bool exists;                        /**< false: the remote place says there is nothing at the path */
    bool isDirectory;
    UInt64 size;                        /**< bytes, 0 for a directory */
    CORE::CString revision;             /**< opaque: changes when the content does; identifies a version of the file */
    CVfsDigest digest;                  /**< of the content, when the remote place knows it */
    UInt64 modifiedUtcMs;               /**< milliseconds since the Unix epoch, 0 when unknown */
    bool attributesKnown;               /**< false: the entry of a listing says only that the name is there and what kind it is; size, revision and digest are what an older answer had, and have to be asked again before they are used (default true) */

    CVfsRemoteEntry( void );

    CVfsRemoteEntry( const CVfsRemoteEntry& src );

    CVfsRemoteEntry& operator=( const CVfsRemoteEntry& src );

    /** Whether the other entry is the same thing at the same revision: what a refresh compares to decide that nothing changed */
    bool IsSameAs( const CVfsRemoteEntry& other ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSREMOTEENTRY_H ? */
