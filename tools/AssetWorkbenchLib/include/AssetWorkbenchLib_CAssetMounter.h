/*
 *  AssetWorkbenchLib: mounting of folders and archives in the VFS and the resource groups of Ogre over them
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

#ifndef ASSETWORKBENCHLIB_CASSETMOUNTER_H
#define ASSETWORKBENCHLIB_CASSETMOUNTER_H

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

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the workbench browses: the folders and archives the user gives with --mount <mountPoint>=<source>, mounted read only in
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
class ASSETWORKBENCHLIB_PUBLIC_CPP CAssetMounter
{
    public:

    struct ASSETWORKBENCHLIB_PUBLIC_CPP SMount
    {
        CORE::CString mountPoint;       /**< the first part of the VFS paths below it */
        CORE::CString source;           /**< the folder, the archive file or the VFS path of the archive file */
        CORE::CString archiveType;      /**< the archive type of the VFS ("FileSystem", "zip", "unitypackage", ...) */
        CORE::CString archiveName;      /**< the name the VFS knows the mount by */
        CORE::CString groupName;        /**< the resource group of Ogre of this mount */
        bool groupReady;
        bool adopted;                   /**< the folder is mounted by someone else; it is not unmounted with the others */
        bool remote;                    /**< the folder is a location (or a pack of it) of a remote store service: its resource group is not made from a listing of everything below it */
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

    /** The spelling of the mounter for a path of the disk: '/' separators, no '/' at the end, variables of the path resolved */
    static CORE::CString NormalizeDiskPath( const CORE::CString& diskPath );

    /**
     *  A mount point for a folder or archive of the disk, made of its name (the extension of an archive is left out, anything
     *  but letters, digits and '_' becomes '_'): not used by a mount or the VFS, and not the name of a folder of the root of the
     *  native file system, so a path of the disk is never taken for a path of a mount. "Shops Pack" gives "Shops_Pack",
     *  and "Shops_Pack_2" when that is taken
     */
    CORE::CString MakeMountPointName( const CORE::CString& diskPath ) const;

    /** The index of the mount that was made of this folder or archive of the disk; -1 when there is none */
    Int32 FindMountBySource( const CORE::CString& diskPath ) const;

    /**
     *  The index of the mount of a folder that holds the path of the disk (the path itself or something below it), with the path
     *  below the mount ("" for the mount itself); -1 when there is none. Archives are not looked in, only folders
     */
    Int32 FindMountOfDiskPath( const CORE::CString& diskPath, CORE::CString& relativePath ) const;

    /**
     *  Mounts a folder or archive of the disk at a mount point of MakeMountPointName(), or finds the mount that holds it already
     *  (the same source); 'mountPoint' tells which. False with a logged reason when it cannot be mounted
     */
    bool MountUnique( const CORE::CString& diskPath, CORE::CString& mountPoint );

    /**
     *  Takes a folder of the VFS that something else mounted (a storage location of a store) in as a mount of the workbench, under the
     *  VFS path it already has: its files can be opened like those of the other mounts. It is not unmounted by UnmountAll(); false
     *  when the path is a mount point already or no folder
     */
    bool Adopt( const CORE::CString& vfsFolder );

    /**
     *  As Adopt() for a folder of a remote store service (ASSETSTORE::CRemoteAssetShares): the resource group of the mount has the mount as a
     *  place and nothing else, a listing of everything below a location of thousands of packs is never made. The packs that are opened bring their
     *  own folders (AdoptRemotePack())
     */
    bool AdoptRemote( const CORE::CString& vfsFolder );

    /**
     *  A pack of a remote store that is opened gets a mount of its own, so a resource group of its own, below the mount of its location: the group
     *  has the folder of the pack and the folders (inside it) that the files of the pack are in, as the catalogue gives them, so a texture is found by
     *  the name a model gives it. The number of folders that were added; 0 when the folder is no mount of this class, or Ogre cannot make the group
     */
    UInt32 AdoptRemotePack( const CORE::CString& packFolder, const CORE::CString::StringVector& folders );

    /** Adds a folder of a mount as a place that resources of its group are found in (for example the folder of cooked files, whose materials name their pictures from there); false when the path is in no mount */
    bool AddResourceLocation( const CORE::CString& vfsFolder );
    /** Takes an adopted mount out of the list of the workbench again and drops its resource group; the folder stays mounted in the VFS. False for a mount that is not adopted */
    bool Forget( const CORE::CString& mountPoint );
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

    /** Drops the resource group of the mount of a path, so that the next use makes it again with the folders as they are then (files were added to the mount, a conversion wrote them); false for a path in no mount */
    bool InvalidateResourceGroup( const CORE::CString& vfsPath );
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

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CASSETMOUNTER_H ? */
