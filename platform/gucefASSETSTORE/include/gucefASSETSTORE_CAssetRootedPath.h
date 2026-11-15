/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
 *
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

#ifndef GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#define GUCEF_ASSETSTORE_CASSETROOTEDPATH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLROOT_H
#include "gucefASSETSTORE_CAssetToolRoot.h"
#define GUCEF_ASSETSTORE_CASSETTOOLROOT_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLROOT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A path as the asset tools take and give it (section 5.2 of the asset tools design): "<rootId>:<relative path>",
 *  for example "packs:Some Pack/source.zip", against the configured roots (CAssetToolRoot). Results never hold paths
 *  of the disk: rooted paths, pack relative asset paths or store location relative paths only.
 *
 *  Parse() refuses: a root id that does not match [a-z][a-z0-9-]{0,31}; in the relative part an absolute path, a drive
 *  letter, a ".." or "." segment, a backslash, a NUL or another control character, an empty segment, a percent sign
 *  (no decoding happens here: a "%2e%2e" is refused rather than read as a name). Resolve() also refuses a root that is
 *  not configured, a root of a kind the tool does not take, and a path that leads out of its root on the file system
 *  underneath (a symbolic link inside the root that points elsewhere).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetRootedPath
{
    public:

    CORE::CString rootId;
    CORE::CString relativePath;     /**< '/' separated, without a leading '/'; empty for the root itself */

    CAssetRootedPath( void );

    /** Takes the text apart and checks both parts; false with the reason when the text is refused */
    bool Parse( const CORE::CString& text, CORE::CString& problem );

    /** "<rootId>:<relativePath>" */
    CORE::CString ToString( void ) const;

    /** The rooted path of a VFS path below the folder of a root; false when the path is not below it */
    static bool FromVfsPath( const CAssetToolRoot& root, const CORE::CString& vfsPath, CAssetRootedPath& rooted );

    /** Whether the id matches [a-z][a-z0-9-]{0,31} */
    static bool IsValidRootId( const CORE::CString& rootId );

    /**
     *  Whether a relative path is safe as described above. Used for the relative part of a rooted path and for pack
     *  relative asset paths. 'allowEmpty': whether an empty path (the folder itself) is accepted.
     *  'refuseLeadingDash': an asset path that starts with '-' is refused (it could be read as an option of a program)
     */
    static bool IsSafeRelativePath( const CORE::CString& path, bool allowEmpty, bool refuseLeadingDash, CORE::CString& problem );

    /**
     *  Resolves the rooted path against the roots: the root must exist and be of one of the kinds (a list separated
     *  by '|', empty: any kind), and the path must stay inside the root on the file system underneath. 'vfsPath' is
     *  the path of the VFS to open
     */
    bool Resolve( const TAssetToolRootVector& roots   ,
                  const CORE::CString& kinds          ,
                  const CAssetToolRoot*& root         ,
                  CORE::CString& vfsPath              ,
                  CORE::CString& problem              ) const;

    /** Parse() and Resolve() in one */
    static bool ParseAndResolve( const CORE::CString& text            ,
                                 const TAssetToolRootVector& roots    ,
                                 const CORE::CString& kinds           ,
                                 CAssetRootedPath& rooted             ,
                                 const CAssetToolRoot*& root          ,
                                 CORE::CString& vfsPath               ,
                                 CORE::CString& problem               );

    /**
     *  Whether the relative path of a root stays inside the folder of the root on the file system underneath (the
     *  diskPath of the root), with every symbolic link followed. The deepest part of the path that exists is
     *  resolved, so a file that is still to be written is checked by its folder. True for a root without a diskPath
     *  (an archive or a memory mount has no links). On platforms without symbolic link resolution the paths are
     *  compared as they are.
     */
    static bool IsInsideRootOnDisk( const CAssetToolRoot& root, const CORE::CString& relativePath, CORE::CString& problem );

    /** Joins a VFS folder and a relative path with one '/' */
    static CORE::CString JoinVfsPath( const CORE::CString& folder, const CORE::CString& relative );

    /** The VFS path with '/' separators, without a trailing '/', with one leading '/' */
    static CORE::CString NormalizeVfsPath( const CORE::CString& vfsPath );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETROOTEDPATH_H ? */
