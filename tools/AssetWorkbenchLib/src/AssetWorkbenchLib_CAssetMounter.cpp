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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <Ogre.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#include "AssetWorkbenchLib_CAssetTypeResolver.h"
#define ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#endif /* ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H ? */

#include "AssetWorkbenchLib_CAssetMounter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const OgreArchiveType = "GucefVFS";
const UInt32 MaxListedFiles = 0xFFFFFFFFU;

/** The folder part of a path, without the file name; empty for a file in the root */
CORE::CString
FolderOf( const CORE::CString& path )
{
    const Int32 slash = path.HasChar( '/', false );
    return slash <= 0 ? CORE::CString::Empty : path.SubstrFromRange( 0, (UInt32) slash );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetMounter::SMount::SMount( void )
    : mountPoint()
    , source()
    , archiveType()
    , archiveName()
    , groupName()
    , groupReady( false )
    , adopted( false )
    , remote( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetMounter::CAssetMounter( void )
    : m_mounts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetMounter::~CAssetMounter()
{GUCEF_TRACE;

    UnmountAll();
}

/*-------------------------------------------------------------------------*/

bool
CAssetMounter::ParseSpec( const CORE::CString& text, CORE::CString& mountPoint, CORE::CString& source )
{GUCEF_TRACE;

    const Int32 equals = text.HasChar( '=', true );
    if ( equals <= 0 || equals+1 >= (Int32) text.Length() )
        return false;
    mountPoint = NormalizePath( text.SubstrFromRange( 0, (UInt32) equals ) );
    source = text.SubstrFromRange( (UInt32) equals+1, text.Length() );
    return !mountPoint.IsNULLOrEmpty() && !source.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetMounter::NormalizePath( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CString path = vfsPath.ReplaceChar( '\\', '/' );
    while ( path.Length() > 0 && '/' == path[ 0 ] )
        path = path.CutChars( 1, true );
    while ( path.Length() > 0 && '/' == path[ path.Length()-1 ] )
        path = path.CutChars( 1, false );
    return path;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetMounter::GetArchiveType( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString extension = CAssetTypeResolver::GetExtension( path );
    if ( extension.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    if ( "gz" == extension && path.Lowercase().HasSubstr( ".tar.gz", false ) >= 0 )
        extension = "tar.gz";

    VFS::CVFS::TAbstractArchiveFactory::TKeySet types;
    VFS::CVfsGlobal::Instance()->GetVfs().GetListOfSupportedArchiveTypes( types );
    VFS::CVFS::TAbstractArchiveFactory::TKeySet::const_iterator i = types.begin();
    while ( i != types.end() )
    {
        if ( (*i) == extension )
            return extension;
        ++i;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CAssetMounter::Mount( const CORE::CString& mountPointText, const CORE::CString& source )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString mountPoint = NormalizePath( mountPointText );
    if ( mountPoint.IsNULLOrEmpty() || source.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetMounter: A mount needs a mount point and a source" );
        return false;
    }
    for ( size_t i=0; i<m_mounts.size(); ++i )
    {
        if ( m_mounts[ i ].mountPoint == mountPoint )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetMounter: The mount point " << mountPoint << " is in use" );
            return false;
        }
    }

    SMount mount;
    mount.mountPoint = mountPoint;
    mount.source = source;
    mount.archiveName = "AssetWorkbench/" + mountPoint;
    mount.groupName = "AssetWorkbench/" + mountPoint;

    const CORE::CString nativePath = CORE::RelativePath( source );
    bool mounted = false;
    if ( CORE::DirExists( nativePath ) )
    {
        // a folder: read only, with the path the VFS reads from named so that it is not taken for a VFS path
        mount.archiveType = VFS::CVFS::FileSystemArchiveTypeName;
        mounted = vfs.MountArchive( mount.archiveName, nativePath, mount.archiveType, mountPoint, false, false, nativePath );
    }
    else
    if ( CORE::FileExists( nativePath ) )
    {
        mount.archiveType = GetArchiveType( nativePath );
        if ( mount.archiveType.IsNULLOrEmpty() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetMounter: " << source << " is no archive that the VFS can read (is the plugin of its format loaded?)" );
            return false;
        }
        mounted = vfs.MountArchive( mount.archiveName, nativePath, mount.archiveType, mountPoint, false, false, nativePath );
    }
    else
    {
        // not on the disk: the VFS path of an archive in a mount that was made before
        const CORE::CString vfsPath = NormalizePath( source );
        mount.archiveType = GetArchiveType( vfsPath );
        if ( vfs.FileExists( vfsPath ) && !mount.archiveType.IsNULLOrEmpty() )
        {
            VFS::TBasicVfsResourcePtr resource = vfs.GetFile( vfsPath, "rb", false );
            if ( !resource.IsNULL() )
                mounted = vfs.MountArchive( mount.archiveName, resource, mount.archiveType, mountPoint, false );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetMounter: " << source << " is neither a folder, a file nor an archive in a mount" );
            return false;
        }
    }

    if ( !mounted )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetMounter: " << source << " cannot be mounted at " << mountPoint << " as " << mount.archiveType );
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetMounter: Mounted " << source << " at " << mountPoint << " as " << mount.archiveType );
    m_mounts.push_back( mount );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetMounter::NormalizeDiskPath( const CORE::CString& diskPath )
{GUCEF_TRACE;

    CORE::CString path = CORE::RelativePath( diskPath ).ReplaceChar( '\\', '/' );
    while ( path.Length() > 1 && '/' == path[ path.Length()-1 ] )
        path = path.CutChars( 1, false );
    return path;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetMounter::MakeMountPointName( const CORE::CString& diskPath ) const
{GUCEF_TRACE;

    const CORE::CString path = NormalizeDiskPath( diskPath );
    const Int32 slash = path.HasChar( '/', false );
    CORE::CString name = slash < 0 ? path : path.SubstrFromRange( (UInt32) slash+1, path.Length() );

    // an archive is named without its extension
    const CORE::CString archiveType = GetArchiveType( path );
    if ( !archiveType.IsNULLOrEmpty() && name.Length() > archiveType.Length()+1 )
        name = name.CutChars( archiveType.Length()+1, false );

    // letters, digits and '_': anything else, in runs, is one '_'
    CORE::CString base;
    bool pendingSeparator = false;
    for ( UInt32 i=0; i<name.Length(); ++i )
    {
        const char c = name[ i ];
        const bool valid = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' );
        if ( valid )
        {
            if ( pendingSeparator && !base.IsNULLOrEmpty() )
                base += "_";
            pendingSeparator = false;
            base += c;
        }
        else
            pendingSeparator = true;
    }
    if ( base.IsNULLOrEmpty() )
        base = "mount";

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( UInt32 attempt=1; attempt<100000; ++attempt )
    {
        const CORE::CString candidate = 1 == attempt ? base : base + "_" + CORE::UInt32ToString( attempt );
        bool taken = false;
        for ( size_t m=0; m<m_mounts.size() && !taken; ++m )
            taken = m_mounts[ m ].mountPoint.Equals( candidate, false );
        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
        taken = taken || CORE::DirExists( "/" + candidate );
        #endif
        if ( !taken && !vfs.DirExists( candidate ) && !vfs.FileExists( candidate ) )
            return candidate;
    }
    return base;
}

/*-------------------------------------------------------------------------*/

Int32
CAssetMounter::FindMountBySource( const CORE::CString& diskPath ) const
{GUCEF_TRACE;

    const CORE::CString path = NormalizeDiskPath( diskPath );
    for ( size_t i=0; i<m_mounts.size(); ++i )
        if ( !m_mounts[ i ].adopted && NormalizeDiskPath( m_mounts[ i ].source ) == path )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CAssetMounter::FindMountOfDiskPath( const CORE::CString& diskPath, CORE::CString& relativePath ) const
{GUCEF_TRACE;

    const CORE::CString path = NormalizeDiskPath( diskPath );
    Int32 best = -1;
    UInt32 bestLength = 0;
    for ( size_t i=0; i<m_mounts.size(); ++i )
    {
        if ( m_mounts[ i ].adopted || VFS::CVFS::FileSystemArchiveTypeName != m_mounts[ i ].archiveType )
            continue;
        const CORE::CString folder = NormalizeDiskPath( m_mounts[ i ].source );
        if ( !CORE::DirExists( folder ) || path.Length() < folder.Length() || path.SubstrFromRange( 0, folder.Length() ) != folder )
            continue;
        const bool isRoot = folder.Length() > 0 && '/' == folder[ folder.Length()-1 ];
        if ( path.Length() > folder.Length() && !isRoot && '/' != path[ folder.Length() ] )
            continue;
        if ( best < 0 || folder.Length() > bestLength )
        {
            best = (Int32) i;
            bestLength = folder.Length();
            relativePath = path.Length() > folder.Length() ? path.CutChars( isRoot ? folder.Length() : folder.Length()+1, true ) : CORE::CString::Empty;
        }
    }
    return best;
}

/*-------------------------------------------------------------------------*/

bool
CAssetMounter::MountUnique( const CORE::CString& diskPath, CORE::CString& mountPoint )
{GUCEF_TRACE;

    const Int32 existing = FindMountBySource( diskPath );
    if ( existing >= 0 )
    {
        mountPoint = m_mounts[ existing ].mountPoint;
        return true;
    }
    const CORE::CString name = MakeMountPointName( diskPath );
    if ( !Mount( name, NormalizeDiskPath( diskPath ) ) )
        return false;
    mountPoint = name;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetMounter::Adopt( const CORE::CString& vfsFolder )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString mountPoint = NormalizePath( vfsFolder );
    if ( mountPoint.IsNULLOrEmpty() || !vfs.DirExists( mountPoint ) )
        return false;
    for ( size_t i=0; i<m_mounts.size(); ++i )
        if ( m_mounts[ i ].mountPoint == mountPoint )
            return false;
    SMount mount;
    mount.mountPoint = mountPoint;
    mount.source = mountPoint;
    mount.archiveType = VFS::CVFS::FileSystemArchiveTypeName;
    mount.groupName = "AssetWorkbench/" + mountPoint;
    mount.adopted = true;
    m_mounts.push_back( mount );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetMounter: Adopted the mounted folder " << mountPoint );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetMounter::AdoptRemote( const CORE::CString& vfsFolder )
{GUCEF_TRACE;

    const CORE::CString mountPoint = NormalizePath( vfsFolder );
    if ( !Adopt( mountPoint ) )
    {
        // adopted already: it is marked as remote all the same
        for ( size_t i=0; i<m_mounts.size(); ++i )
            if ( m_mounts[ i ].mountPoint == mountPoint )
                m_mounts[ i ].remote = true;
        return false;
    }
    for ( size_t i=0; i<m_mounts.size(); ++i )
        if ( m_mounts[ i ].mountPoint == mountPoint )
            m_mounts[ i ].remote = true;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetMounter::AdoptRemotePack( const CORE::CString& packFolder, const CORE::CString::StringVector& folders )
{GUCEF_TRACE;

    const CORE::CString mountPoint = NormalizePath( packFolder );
    if ( mountPoint.IsNULLOrEmpty() )
        return 0;
    bool known = false;
    for ( size_t i=0; i<m_mounts.size(); ++i )
        if ( m_mounts[ i ].mountPoint == mountPoint )
            known = true;
    if ( !known )
    {
        SMount mount;
        mount.mountPoint = mountPoint;
        mount.source = mountPoint;
        mount.archiveType = VFS::CVFS::FileSystemArchiveTypeName;
        mount.groupName = "AssetWorkbench/" + mountPoint;
        mount.adopted = true;
        mount.remote = true;
        m_mounts.push_back( mount );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetMounter: Adopted the remote pack folder " << mountPoint << " as a mount of its own" );
    }
    if ( !EnsureResourceGroup( mountPoint ) )
        return 0;
    UInt32 added = 0;
    for ( size_t f=0; f<folders.size(); ++f )
    {
        const CORE::CString folder = NormalizePath( folders[ f ] );
        if ( folder == mountPoint || 0 == folder.HasSubstr( mountPoint + "/", true ) )
            if ( AddResourceLocation( folder ) )
                ++added;
    }

    // and the folders of the pack that hold files that a model names (pictures, material libraries): the lists of the pack are known to the
    // cache by now (the transfer that fetched the files read them), so this does not wait for the service
    VFS::CVFS::TStringVector files;
    VFS::CVfsGlobal::Instance()->GetVfs().GetFileList( files, mountPoint, true, true, CORE::CString::Empty, MaxListedFiles );
    GUCEF::set< CORE::CString > listed;
    for ( size_t i=0; i<files.size(); ++i )
    {
        const CORE::CString file = NormalizePath( files[ i ] );
        const CORE::CString folder = FolderOf( file );
        if ( folder.IsNULLOrEmpty() || folder == mountPoint || 0 != folder.HasSubstr( mountPoint + "/", true ) )
            continue;
        if ( listed.insert( folder ).second && AddResourceLocation( folder ) )
            ++added;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetMounter: The remote pack " << mountPoint << " has " << added << " of " << (UInt32) folders.size() << " folders as places of its resource group" );
    return added;
}

/*-------------------------------------------------------------------------*/

bool
CAssetMounter::AddResourceLocation( const CORE::CString& vfsFolder )
{GUCEF_TRACE;

    CORE::CString relative;
    const CORE::CString folder = NormalizePath( vfsFolder );
    const CORE::CString group = GetResourceGroup( folder, relative );
    if ( group.IsNULLOrEmpty() )
        return false;
    try
    {
        Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
        if ( !groups.resourceLocationExists( folder.STL_String(), group.STL_String() ) )
            groups.addResourceLocation( folder.STL_String(), OgreArchiveType, group.STL_String(), true );
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetMounter: The place " << folder << " cannot be added: " << e.getDescription() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetMounter::Forget( const CORE::CString& mountPoint )
{GUCEF_TRACE;

    const CORE::CString name = NormalizePath( mountPoint );
    for ( size_t i=0; i<m_mounts.size(); ++i )
    {
        if ( m_mounts[ i ].mountPoint == name && m_mounts[ i ].adopted )
        {
            DropResourceGroup( m_mounts[ i ] );
            m_mounts.erase( m_mounts.begin() + i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetMounter::MountAll( const CORE::CString::StringVector& specs )
{GUCEF_TRACE;

    UInt32 mounted = 0;
    for ( size_t i=0; i<specs.size(); ++i )
    {
        CORE::CString mountPoint, source;
        if ( !ParseSpec( specs[ i ], mountPoint, source ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetMounter: \"" << specs[ i ] << "\" is no mount, which is <mountPoint>=<folder, archive or VFS path of an archive>" );
            continue;
        }
        if ( Mount( mountPoint, source ) )
            ++mounted;
    }
    return mounted;
}

/*-------------------------------------------------------------------------*/

void
CAssetMounter::DropResourceGroup( SMount& mount )
{GUCEF_TRACE;

    if ( !mount.groupReady )
        return;
    mount.groupReady = false;
    if ( GUCEF_NULL == Ogre::Root::getSingletonPtr() || GUCEF_NULL == Ogre::ResourceGroupManager::getSingletonPtr() )
        return;
    try
    {
        Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
        if ( groups.resourceGroupExists( mount.groupName.STL_String() ) )
            groups.destroyResourceGroup( mount.groupName.STL_String() );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CAssetMounter: The resource group " << mount.groupName << " cannot be dropped: " << e.getFullDescription() );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetMounter::UnmountAll( void )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    while ( !m_mounts.empty() )
    {
        SMount& mount = m_mounts.back();
        DropResourceGroup( mount );
        if ( !mount.adopted )
            vfs.UnmountArchiveByName( mount.archiveName );
        m_mounts.pop_back();
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetMounter::GetMountCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_mounts.size();
}

/*-------------------------------------------------------------------------*/

const CAssetMounter::SMount&
CAssetMounter::GetMount( UInt32 index ) const
{GUCEF_TRACE;

    return m_mounts[ index ];
}

/*-------------------------------------------------------------------------*/

bool
CAssetMounter::FindMount( const CORE::CString& vfsPath, SMount& mount, CORE::CString& relativePath ) const
{GUCEF_TRACE;

    const CORE::CString path = NormalizePath( vfsPath );
    // the longest mount point that starts the path, as mount points can be nested
    Int32 best = -1;
    for ( size_t i=0; i<m_mounts.size(); ++i )
    {
        const CORE::CString& mountPoint = m_mounts[ i ].mountPoint;
        if ( path.Length() < mountPoint.Length() || path.SubstrFromRange( 0, mountPoint.Length() ) != mountPoint )
            continue;
        if ( path.Length() > mountPoint.Length() && '/' != path[ mountPoint.Length() ] )
            continue;
        if ( best < 0 || mountPoint.Length() > m_mounts[ best ].mountPoint.Length() )
            best = (Int32) i;
    }
    if ( best < 0 )
        return false;
    mount = m_mounts[ best ];
    relativePath = path.Length() > mount.mountPoint.Length() ? path.CutChars( mount.mountPoint.Length()+1, true ) : CORE::CString::Empty;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetMounter::EnsureResourceGroup( const CORE::CString& mountPoint )
{GUCEF_TRACE;

    SMount* mount = GUCEF_NULL;
    for ( size_t i=0; i<m_mounts.size(); ++i )
        if ( m_mounts[ i ].mountPoint == mountPoint )
            mount = &m_mounts[ i ];
    if ( GUCEF_NULL == mount )
        return false;
    if ( mount->groupReady )
        return true;
    if ( GUCEF_NULL == Ogre::Root::getSingletonPtr() || GUCEF_NULL == Ogre::ResourceGroupManager::getSingletonPtr() )
        return false;

    try
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
        if ( groups.resourceGroupExists( mount->groupName.STL_String() ) )
            groups.destroyResourceGroup( mount->groupName.STL_String() );
        groups.createResourceGroup( mount->groupName.STL_String(), false );

        // the mount as a whole: names relative to it
        groups.addResourceLocation( mount->mountPoint.STL_String(), OgreArchiveType, mount->groupName.STL_String(), true );

        // and the folders with assets, for the names that a model gives the files next to it; a mount of a remote store service is not listed: the packs
        // that are opened add their folders (AdoptRemotePack())
        VFS::CVFS::TStringVector files;
        if ( !mount->remote )
            vfs.GetFileList( files, mount->mountPoint, true, true, CORE::CString::Empty, MaxListedFiles );
        GUCEF::set< CORE::CString > folders;
        for ( size_t i=0; i<files.size(); ++i )
        {
            CORE::CString file = NormalizePath( files[ i ] );
            if ( file.Length() > mount->mountPoint.Length() && file.SubstrFromRange( 0, mount->mountPoint.Length() ) == mount->mountPoint && '/' == file[ mount->mountPoint.Length() ] )
                file = file.CutChars( mount->mountPoint.Length()+1, true );
            const CORE::CString folder = FolderOf( file );
            if ( !folder.IsNULLOrEmpty() && CAssetTypeResolver::HasView( file ) )
                folders.insert( folder );
        }
        GUCEF::set< CORE::CString >::const_iterator f = folders.begin();
        while ( f != folders.end() )
        {
            groups.addResourceLocation( ( mount->mountPoint + "/" + *f ).STL_String(), OgreArchiveType, mount->groupName.STL_String(), false );
            ++f;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CAssetMounter: Resource group " << mount->groupName << " has the mount and " << (UInt32) folders.size() << " folders with assets" );
        mount->groupReady = true;
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetMounter: The resource group of " << mountPoint << " cannot be made: " << e.getFullDescription() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetMounter::InvalidateResourceGroup( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    SMount found;
    CORE::CString relative;
    if ( !FindMount( vfsPath, found, relative ) )
        return false;
    for ( size_t i=0; i<m_mounts.size(); ++i )
        if ( m_mounts[ i ].mountPoint == found.mountPoint )
            DropResourceGroup( m_mounts[ i ] );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetMounter::GetResourceGroup( const CORE::CString& vfsPath, CORE::CString& relativePath )
{GUCEF_TRACE;

    SMount mount;
    if ( !FindMount( vfsPath, mount, relativePath ) || !EnsureResourceGroup( mount.mountPoint ) )
        return CORE::CString::Empty;
    return mount.groupName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
