/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_IFILESYSTEMACCESS_H
#include "gucefCORE_IFileSystemAccess.h"
#define GUCEF_CORE_IFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_IFILESYSTEMACCESS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The file system of the operating system: everything its file API reaches, local disks as well as network shares and other
 *  mounts. Routes to the CORE file functions; their bool results are mapped to result codes by checks made before each call (a
 *  call that still fails gives FILESYSTEMACCESS_FAILED). Paths go through RelativePath(), so $TEMPDIR$ and the like work.
 *  The root is "/" on Linux; on Windows the root "" lists the drives.
 */
class GUCEF_CORE_PUBLIC_CPP CNativeFileSystemAccess : public IFileSystemAccess
{
    public:

    static const CString DefaultDisplayName;

    explicit CNativeFileSystemAccess( const CString& displayName = DefaultDisplayName );

    virtual ~CNativeFileSystemAccess() GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetRootPath( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsWriteable( const CString& path ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TFileSystemAccessResult TryCreateDirectory( const CString& path, bool createParents ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TFileSystemAccessResult TryDeleteDirectory( const CString& path, bool deleteContents ) GUCEF_VIRTUAL_OVERRIDE;

    /** Moves through CORE::MoveFile, whose rename also moves directories; the directories of the new path are made */
    virtual TFileSystemAccessResult TryMoveDirectory( const CString& oldPath, const CString& newPath ) GUCEF_VIRTUAL_OVERRIDE;

    /** The directories of the new path are made, as CORE::MoveFile does */
    virtual TFileSystemAccessResult TryMoveFile( const CString& oldPath, const CString& newPath, bool overwrite ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TFileSystemAccessResult TryMoveFiles( const CString::StringVector& filePaths     ,
                                                  const CString& destinationDir             ,
                                                  bool overwrite                            ,
                                                  TFileSystemAccessResultVector* perFileResults ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TFileSystemAccessResult TryRenameFile( const CString& path, const CString& newName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TFileSystemAccessResult TryDeleteFile( const CString& path ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TFileSystemAccessResult TryGetMetaData( const CString& path, CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TFileSystemAccessResult TryGetEntryType( const CString& path, TFileSystemEntryType& entryType ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TFileSystemAccessResult TryGetFileSystemIterator( const CString& dirPath, TFileSystemIteratorPtr& iterator ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TFileSystemAccessResult ListAvailableSpecialDirs( TSpecialDirSet& specialDirs ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TFileSystemAccessResult TryResolveSpecialDir( TSpecialDirs specialDir, CString& path ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Whether a name can be the name of a file or directory: not empty, not "." or "..", no directory separator */
    static bool IsValidEntryName( const CString& name );

    private:

    CNativeFileSystemAccess( const CNativeFileSystemAccess& src );
    CNativeFileSystemAccess& operator=( const CNativeFileSystemAccess& src );

    /** Whether the directory is read-only as far as the metadata of the platform tells (Windows) */
    static bool IsReadOnlyDir( const CString& dirPath );

    CString m_displayName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */
