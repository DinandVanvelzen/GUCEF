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

#ifndef GUCEF_VFS_CVFSREMOTEPATH_H
#define GUCEF_VFS_CVFSREMOTEPATH_H

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
 *  The paths of a remote place as the generic remote archive layer keeps them: relative to the share, '/' as the
 *  separator, no leading or trailing separator, "" for the root. Works on the bytes of the UTF-8 text, so names with any
 *  character are cut at the separator only.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsRemotePath
{
    public:

    /** Turns '\' into '/', takes away leading and trailing separators and repeated ones */
    static CORE::CString Normalize( const CORE::CString& path );

    /** The directory a path is in, "" for a path of one segment and for the root */
    static CORE::CString ParentOf( const CORE::CString& path );

    /** The last segment of a path */
    static CORE::CString LeafOf( const CORE::CString& path );

    /** The path of an entry in a directory; the directory may be "" */
    static CORE::CString Join( const CORE::CString& dirPath, const CORE::CString& name );

    /** Whether 'path' is 'dirPath' itself or below it ("" holds every path) */
    static bool IsSameOrBelow( const CORE::CString& dirPath, const CORE::CString& path );

    /** The number of bytes of the text, without the terminator that CString::ByteSize() counts */
    static UInt32 GetByteCount( const CORE::CString& text );

    private:

    CVfsRemotePath( void );             /**< not implemented: only static members */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSREMOTEPATH_H ? */
