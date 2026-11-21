/*
 *  AssetConverterLib: where the pack index of a pack lives and how its files are named
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#include "AssetConverterLib_CPackIndexLocator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CPackIndexLocator::ManifestFileName = "pack-index.json";
const CORE::CString CPackIndexLocator::ShardFolderName = "pack-index.d";
const CORE::CString CPackIndexLocator::UserCacheMountName = "AssetConverterLib/PackIndexCache";
const CORE::CString CPackIndexLocator::UserCacheEnvVar = "GUCEF_PACKINDEX_CACHE";

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexLocator::GetManifestPath( const CORE::CString& indexFolder )
{GUCEF_TRACE;

    return CVfsPaths::Join( indexFolder, ManifestFileName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexLocator::GetShardFolder( const CORE::CString& indexFolder )
{GUCEF_TRACE;

    return CVfsPaths::Join( indexFolder, ShardFolderName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexLocator::GetShardPath( const CORE::CString& indexFolder, const CORE::CString& shardFile )
{GUCEF_TRACE;

    return CVfsPaths::Join( GetShardFolder( indexFolder ), shardFile );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexLocator::GetUserCacheDiskFolder( void )
{GUCEF_TRACE;

    CORE::CString folder = CORE::GetEnv( UserCacheEnvVar );
    if ( !folder.IsNULLOrEmpty() )
        return folder;
#ifdef GUCEF_MSWIN_BUILD
    folder = CORE::GetEnv( "LOCALAPPDATA" );
    if ( !folder.IsNULLOrEmpty() )
    {
        CORE::AppendToPath( folder, "gucef" );
        CORE::AppendToPath( folder, "PackIndex" );
        return folder;
    }
#endif
    folder = CORE::GetEnv( "HOME" );
    if ( folder.IsNULLOrEmpty() )
        folder = CORE::GetEnv( "USERPROFILE" );
    if ( folder.IsNULLOrEmpty() )
        folder = ".";
    CORE::AppendToPath( folder, ".cache" );
    CORE::AppendToPath( folder, "gucef" );
    CORE::AppendToPath( folder, "PackIndex" );
    return folder;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexLocator::MountUserCache( CORE::CString& vfsRoot )
{GUCEF_TRACE;

    // a hidden name: the mount is no folder of the VFS that a person browses
    static const CORE::CString mountPoint = ".PackIndexCache";
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    vfsRoot = CVfsPaths::Normalize( mountPoint );
    const CORE::CString folder = GetUserCacheDiskFolder();
    if ( vfs.IsMountedArchive( mountPoint ) )
        return true;
    if ( !CORE::DirExists( folder ) && !CORE::CreateDirs( folder ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PackIndexLocator: The user cache folder " << folder << " cannot be made" );
        return false;
    }
    if ( !vfs.MountArchive( UserCacheMountName, folder, VFS::CVFS::FileSystemArchiveTypeName, mountPoint, true, false, folder ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PackIndexLocator: The user cache folder " << folder << " cannot be mounted" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexLocator::GetUserCacheFolder( const CORE::CString& vfsCacheRoot, const CORE::CString& packKey )
{GUCEF_TRACE;

    return CVfsPaths::Join( vfsCacheRoot, packKey.SubstrFromRange( 0, packKey.Length() < 24 ? packKey.Length() : 24 ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
