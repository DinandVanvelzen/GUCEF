/*
 *  gucefVFS: GUCEF module providing a Virtual File System
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_VFS_CVFSFILESYSTEMACCESS_H
#define GUCEF_VFS_CVFSFILESYSTEMACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_IFILESYSTEMACCESS_H
#include "gucefCORE_IFileSystemAccess.h"
#define GUCEF_CORE_IFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_IFILESYSTEMACCESS_H ? */

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

class CVFS;

/**
 *  The virtual file system as a file system: routes to CVFS (the one of CVfsGlobal unless another is given). Paths are VFS paths
 *  ("/" is the root, directories of mounts and the mount points below them are merged). Directories are made, deleted and moved
 *  in writeable mounts whose archives can do that; places only read-only mounts cover give FILESYSTEMACCESS_READ_ONLY.
 *  The VFS has no special directories of its own.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsFileSystemAccess : public CORE::IFileSystemAccess
{
    public:

    static const CORE::CString DefaultDisplayName;

    explicit CVfsFileSystemAccess( const CORE::CString& displayName = DefaultDisplayName );

    CVfsFileSystemAccess( CVFS& vfs, const CORE::CString& displayName );

    virtual ~CVfsFileSystemAccess() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetRootPath( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsWriteable( const CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TFileSystemAccessResult TryCreateDirectory( const CORE::CString& path, bool createParents ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TFileSystemAccessResult TryDeleteDirectory( const CORE::CString& path, bool deleteContents ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TFileSystemAccessResult TryMoveDirectory( const CORE::CString& oldPath, const CORE::CString& newPath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TFileSystemAccessResult TryMoveFile( const CORE::CString& oldPath, const CORE::CString& newPath, bool overwrite ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TFileSystemAccessResult TryMoveFiles( const CORE::CString::StringVector& filePaths ,
                                                        const CORE::CString& destinationDir         ,
                                                        bool overwrite                              ,
                                                        TFileSystemAccessResultVector* perFileResults ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TFileSystemAccessResult TryRenameFile( const CORE::CString& path, const CORE::CString& newName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TFileSystemAccessResult TryDeleteFile( const CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TFileSystemAccessResult TryGetMetaData( const CORE::CString& path, CORE::CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TFileSystemAccessResult TryGetEntryType( const CORE::CString& path, CORE::TFileSystemEntryType& entryType ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The listing is read here, whole: the iterator is a CORE::CFileSystemEntryListIterator */
    virtual CORE::TFileSystemAccessResult TryGetFileSystemIterator( const CORE::CString& dirPath, CORE::TFileSystemIteratorPtr& iterator ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TFileSystemAccessResult ListAvailableSpecialDirs( CORE::TSpecialDirSet& specialDirs ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TFileSystemAccessResult TryResolveSpecialDir( CORE::TSpecialDirs specialDir, CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;

    /** A VFS path as this class gives them: '/' separators, a '/' first, none at the end (except for the root "/") */
    static CORE::CString ConformPath( const CORE::CString& path );

    private:

    CVfsFileSystemAccess( const CVfsFileSystemAccess& src );
    CVfsFileSystemAccess& operator=( const CVfsFileSystemAccess& src );

    /** A directory of a mount, or one that exists because mount points are below it, or the root */
    bool IsDirectory( const CORE::CString& conformedPath ) const;

    static CORE::CString ParentOf( const CORE::CString& conformedPath );

    CVFS* m_vfs;
    CORE::CString m_displayName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSFILESYSTEMACCESS_H ? */
