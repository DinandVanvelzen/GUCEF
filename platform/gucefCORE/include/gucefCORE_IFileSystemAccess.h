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

#ifndef GUCEF_CORE_IFILESYSTEMACCESS_H
#define GUCEF_CORE_IFILESYSTEMACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_EFILESYSTEMACCESSRESULT_H
#include "gucefCORE_EFileSystemAccessResult.h"
#define GUCEF_CORE_EFILESYSTEMACCESSRESULT_H
#endif /* GUCEF_CORE_EFILESYSTEMACCESSRESULT_H ? */

#ifndef GUCEF_CORE_IFILESYSTEMITERATOR_H
#include "gucefCORE_IFileSystemIterator.h"
#define GUCEF_CORE_IFILESYSTEMITERATOR_H
#endif /* GUCEF_CORE_IFILESYSTEMITERATOR_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a path is */
enum EFileSystemEntryType
{
    FILESYSTEMENTRYTYPE_NONE        = 0,    /**< nothing is there */
    FILESYSTEMENTRYTYPE_FILE        = 1,
    FILESYSTEMENTRYTYPE_DIRECTORY   = 2
};
typedef enum EFileSystemEntryType TFileSystemEntryType;

class CFileSystemAccessTaskData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A file system: the operations of browsing and changing files and directories, whatever the file system is (the one of the
 *  operating system, the VFS, ...).
 *
 *  The synchronous operations run on the calling thread. The async variants run the same operation on a thread of a pool of the
 *  task manager: they return FILESYSTEMACCESS_SUCCESS when it was queued and give a future, whose result is read with the
 *  TryGetAsync...() functions once it is ready. A provider is made with GUCEF_NEW and used through TFileSystemAccessPtr
 *  ( ( GUCEF_NEW CNativeFileSystemAccess() )->CreateSharedPtr() ): a queued operation keeps the provider alive until it ended.
 */
class GUCEF_CORE_PUBLIC_CPP IFileSystemAccess : public CTSharedPtrCreator< IFileSystemAccess, MT::CMutex >
{
    public:

    typedef CTSharedPtr< IFileSystemAccess, MT::CMutex >    TFileSystemAccessPtr;
    typedef GUCEF::vector< TFileSystemAccessResult >        TFileSystemAccessResultVector;

    IFileSystemAccess( void );

    virtual ~IFileSystemAccess();

    /** A name for people, for example "This computer" */
    virtual CString GetDisplayName( void ) const = 0;

    /** The path of the top of the file system, where browsing starts */
    virtual CString GetRootPath( void ) const = 0;

    /** Whether something can be written at the path */
    virtual bool IsWriteable( const CString& path ) const = 0;

    virtual TFileSystemAccessResult TryCreateDirectory( const CString& path, bool createParents ) = 0;

    /** FILESYSTEMACCESS_DIRECTORY_NOT_EMPTY when the directory has contents and deleteContents is false */
    virtual TFileSystemAccessResult TryDeleteDirectory( const CString& path, bool deleteContents ) = 0;

    virtual TFileSystemAccessResult TryMoveDirectory( const CString& oldPath, const CString& newPath ) = 0;

    virtual TFileSystemAccessResult TryMoveFile( const CString& oldPath, const CString& newPath, bool overwrite ) = 0;

    /**
     *  Moves files into a directory, keeping their names. FILESYSTEMACCESS_PARTIAL_SUCCESS when some of them could not be
     *  moved; perFileResults (when given) gets the result of each file, in the order of the files.
     */
    virtual TFileSystemAccessResult TryMoveFiles( const CString::StringVector& filePaths     ,
                                                  const CString& destinationDir             ,
                                                  bool overwrite                            ,
                                                  TFileSystemAccessResultVector* perFileResults ) = 0;

    /** Gives a file a new name in the same directory; FILESYSTEMACCESS_INVALID_NAME for a name with a separator */
    virtual TFileSystemAccessResult TryRenameFile( const CString& path, const CString& newName ) = 0;

    virtual TFileSystemAccessResult TryDeleteFile( const CString& path ) = 0;

    virtual TFileSystemAccessResult TryGetMetaData( const CString& path, CResourceMetaData& metaData ) const = 0;

    virtual TFileSystemAccessResult TryGetEntryType( const CString& path, TFileSystemEntryType& entryType ) const = 0;

    /**
     *  An iterator over the entries of the directory, at the interface level of the provider's own iterator.
     *  Walk it with FindFirst( dirPath ) / FindNext()
     */
    virtual TFileSystemAccessResult TryGetFileSystemIterator( const CString& dirPath, TFileSystemIteratorPtr& iterator ) const = 0;

    /** The special directories (home, downloads, ...) the file system knows */
    virtual TFileSystemAccessResult ListAvailableSpecialDirs( TSpecialDirSet& specialDirs ) const = 0;

    /** A special directory as a path of this file system */
    virtual TFileSystemAccessResult TryResolveSpecialDir( TSpecialDirs specialDir, CString& path ) const = 0;

    /*
     *  The async variants. The default implementation runs the synchronous operation on a thread of the pool (the default
     *  pool when no name is given); a provider can replace one with an async operation of its own.
     */

    virtual TFileSystemAccessResult TryCreateDirectoryAsync( const CString& path, bool createParents, CFutureResult& future,
                                                             const CString& threadPoolName = CString::Empty );

    virtual TFileSystemAccessResult TryDeleteDirectoryAsync( const CString& path, bool deleteContents, CFutureResult& future,
                                                             const CString& threadPoolName = CString::Empty );

    virtual TFileSystemAccessResult TryMoveDirectoryAsync( const CString& oldPath, const CString& newPath, CFutureResult& future,
                                                           const CString& threadPoolName = CString::Empty );

    virtual TFileSystemAccessResult TryMoveFileAsync( const CString& oldPath, const CString& newPath, bool overwrite,
                                                      CFutureResult& future, const CString& threadPoolName = CString::Empty );

    virtual TFileSystemAccessResult TryMoveFilesAsync( const CString::StringVector& filePaths, const CString& destinationDir,
                                                       bool overwrite, CFutureResult& future,
                                                       const CString& threadPoolName = CString::Empty );

    virtual TFileSystemAccessResult TryRenameFileAsync( const CString& path, const CString& newName, CFutureResult& future,
                                                        const CString& threadPoolName = CString::Empty );

    virtual TFileSystemAccessResult TryDeleteFileAsync( const CString& path, CFutureResult& future,
                                                        const CString& threadPoolName = CString::Empty );

    virtual TFileSystemAccessResult TryGetMetaDataAsync( const CString& path, CFutureResult& future,
                                                         const CString& threadPoolName = CString::Empty );

    virtual TFileSystemAccessResult TryGetEntryTypeAsync( const CString& path, CFutureResult& future,
                                                          const CString& threadPoolName = CString::Empty );

    /** Reads the whole directory on the pool thread: the iterator of the result is a CFileSystemEntryListIterator */
    virtual TFileSystemAccessResult TryGetFileSystemIteratorAsync( const CString& dirPath, CFutureResult& future,
                                                                   const CString& threadPoolName = CString::Empty );

    /** The result of an async operation; false while it is not ready (result is then left alone) */
    static bool TryGetAsyncResult( const CFutureResult& future, TFileSystemAccessResult& result );

    /** The per file results of TryMoveFilesAsync() */
    static bool TryGetAsyncPerFileResults( const CFutureResult& future, TFileSystemAccessResultVector& perFileResults );

    static bool TryGetAsyncMetaData( const CFutureResult& future, CResourceMetaData& metaData );

    static bool TryGetAsyncEntryType( const CFutureResult& future, TFileSystemEntryType& entryType );

    static bool TryGetAsyncIterator( const CFutureResult& future, TFileSystemIteratorPtr& iterator );

    protected:

    /** Queues an operation for the default async variants; the task data gets the shared pointer to this provider */
    TFileSystemAccessResult QueueAsyncOperation( CFileSystemAccessTaskData& taskData, CFutureResult& future,
                                                 const CString& threadPoolName );

    private:

    IFileSystemAccess( const IFileSystemAccess& src );
    IFileSystemAccess& operator=( const IFileSystemAccess& src );
};

/*-------------------------------------------------------------------------*/

typedef IFileSystemAccess::TFileSystemAccessPtr TFileSystemAccessPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_IFILESYSTEMACCESS_H ? */
