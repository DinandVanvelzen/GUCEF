/*
 *  AssetViewerLib: mounting of folders and archives in the VFS and the resource groups of Ogre over them
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

#ifndef ASSETVIEWERLIB_CASSETMOUNTER_H
#define ASSETVIEWERLIB_CASSETMOUNTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETVIEWERLIB_MACROS_H
#include "AssetViewerLib_macros.h"
#define ASSETVIEWERLIB_MACROS_H
#endif /* ASSETVIEWERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the viewer browses: the folders and archives the user gives with --mount <mountPoint>=<source>, mounted read only in
 *  the VFS. The source is a folder, an archive file (zip, unitypackage and the formats of libarchive: by the extension) or the
 *  VFS path of an archive file inside a mount that was made before, which is how a package inside a zip is reached.
 *
 *  Ogre reads the assets through the same mounts: every mount gets a resource group of Ogre with the mount itself as a
 *  location (names relative to the mount, "Assets/Models/House.fbx") and every folder below it that holds assets as a location
 *  of its own (names as a model refers to a texture next to it, "House.png"). The groups are made when an asset of the mount
 *  is first opened (EnsureResourceGroup()), never initialised: the scripts of a group are not read until a view asks for one.
 *
 *  VFS paths in this class have '/' separators, no '/' first and none at the end: "town/Assets/Models/House.fbx".
 */
class ASSETVIEWERLIB_PUBLIC_CPP CAssetMounter
{
    public:

    struct SMount
    {
        CORE::CString mountPoint;       /**< the first part of the VFS paths below it */
        CORE::CString source;           /**< the folder, the archive file or the VFS path of the archive file */
        CORE::CString archiveType;      /**< the archive type of the VFS ("FileSystem", "zip", "unitypackage", ...) */
        CORE::CString archiveName;      /**< the name the VFS knows the mount by */
        CORE::CString groupName;        /**< the resource group of Ogre of this mount */
        bool groupReady;
        SMount( void );
    };
    typedef GUCEF::vector< SMount > TMountVector;

    CAssetMounter( void );

    ~CAssetMounter();

    /** Splits "mountPoint=source"; false for a text without both parts */
    static bool ParseSpec( const CORE::CString& text, CORE::CString& mountPoint, CORE::CString& source );

    /** The path as this class has them: no leading or trailing '/', '/' separators */
    static CORE::CString NormalizePath( const CORE::CString& vfsPath );

    /** The archive type of the VFS for a file by its extension, empty for a file that is no archive the VFS can read */
    static CORE::CString GetArchiveType( const CORE::CString& path );

    /** Mounts a folder, an archive or an archive of a mount; false with a logged reason when that is not possible */
    bool Mount( const CORE::CString& mountPoint, const CORE::CString& source );

    /** Mounts what a list of "mountPoint=source" texts names; the number that was mounted */
    UInt32 MountAll( const CORE::CString::StringVector& specs );

    /** Takes every mount out of the VFS again, the one made last first, and drops the resource groups of Ogre */
    void UnmountAll( void );

    UInt32 GetMountCount( void ) const;
    const SMount& GetMount( UInt32 index ) const;

    /** The mount a VFS path is in and the path below the mount ("" for the mount itself); false for a path outside of all mounts */
    bool FindMount( const CORE::CString& vfsPath, SMount& mount, CORE::CString& relativePath ) const;

    /**
     *  Makes the resource group of the mount of Ogre when that was not done yet (the locations of the mount and of its folders
     *  with assets); false when Ogre cannot do it
     */
    bool EnsureResourceGroup( const CORE::CString& mountPoint );

    /** The name of the resource group of the mount of a path, empty when there is none; makes the group when needed */
    CORE::CString GetResourceGroup( const CORE::CString& vfsPath, CORE::CString& relativePath );

    private:

    CAssetMounter( const CAssetMounter& src );
    CAssetMounter& operator=( const CAssetMounter& src );

    void DropResourceGroup( SMount& mount );

    private:

    TMountVector m_mounts;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CASSETMOUNTER_H ? */
