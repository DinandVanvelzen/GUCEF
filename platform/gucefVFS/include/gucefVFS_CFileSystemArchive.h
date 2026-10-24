/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#ifndef GUCEF_VFS_CFILESYSTEMARCHIVE_H
#define GUCEF_VFS_CFILESYSTEMARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_VFS_CARCHIVE_H
#include "gucefVFS_CArchive.h"
#define GUCEF_VFS_CARCHIVE_H
#endif /* GUCEF_VFS_CARCHIVE_H ? */

#ifndef GUCEF_CORE_CFILESYSTEMDIRECTORYWATCHER_H
#include "gucefCORE_CFileSystemDirectoryWatcher.h"
#define GUCEF_CORE_CFILESYSTEMDIRECTORYWATCHER_H
#endif /* GUCEF_CORE_CFILESYSTEMDIRECTORYWATCHER_H ? */

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

class GUCEF_VFS_PUBLIC_CPP CFileSystemArchive : public CArchive
{
    public:
    
    CFileSystemArchive( void );
    
    CFileSystemArchive( const CString& archiveName ,
                        const CString& rootDir     ,
                        const bool writeable       );
    
    virtual ~CFileSystemArchive();
    
    virtual TBasicVfsResourcePtr GetFile( const CString& file          ,
                                          const char* mode = "rb"      ,
                                          const UInt32 memLoadSize = 0 ,
                                          const bool overwrite = false ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TBasicVfsResourcePtr GetFileAs( const CString& file                     ,
                                            const CORE::CResourceMetaData& metaData ,
                                            const char* mode = "wb"                 ,
                                            const UInt32 memLoadSize = 0            ,
                                            const bool overwrite = false            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StoreAsFile( const CORE::CString& filepath    ,
                              const CORE::CDynamicBuffer& data ,
                              const CORE::UInt64 offset        ,
                              const bool overwrite             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetFileList( TStringVector& outputList                                       ,
                              const CString& mountLocation                                    , 
                              const CString& archiveLocation                                  ,
                              bool recursive = false                                          ,
                              bool includePathInFilename = false                              ,
                              const CString::StringSet& nameFilters = CString::EmptyStringSet ,
                              UInt32 maxListEntries = GUCEF_UINT16MAX                         ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetDirList( TStringVector& outputList                                       ,
                             const CString& mountLocation                                    ,  
                             const CString& archiveLocation                                  ,
                             bool recursive = false                                          ,
                             bool includeParentDirInName = false                             ,
                             const CString::StringSet& nameFilters = CString::EmptyStringSet ,
                             UInt32 maxListEntries = GUCEF_UINT16MAX                         ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool DeleteFile( const CString& filePath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool MoveFile( const CString& oldFilePath ,
                           const CString& newFilePath ,
                           const bool overwrite       ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool FileExists( const CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DirExists( const CString& dirPath ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual UInt64 GetFileSize( const CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetFileMetaData( const CString& filePath                 ,
                                  const CORE::CResourceMetaData& metaData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetFileMetaData( const CString& filePath           ,
                                  CORE::CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual CString GetFileHash( const CString& file ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual CORE::CDateTime GetFileModificationTime( const CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual const CString& GetArchiveName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool LoadArchive( const CArchiveSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadArchive( const VFS::CString& archiveName  ,
                              TBasicVfsResourcePtr vfsResource ,
                              const bool writeableRequest      ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool UnloadArchive( void ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual void DestroyObject( CVFSHandle* sharedPointer ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsDirectoryWatchingSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool AddDirToWatch( const CString& dirToWatch       ,
                                const CDirWatchOptions& options ,
                                bool tryToCreatePathIfNotExists ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RemoveDirToWatch( const CString& dirToWatch ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RemoveAllWatches( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetAllWatchedDirs( CString::StringSet& dirs ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsHealthy( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE; 

    /**
     *  member function to allow the archive to resolve special directories
     *  like the user's home directory, the current working directory, etc.
     *  Redirects to O/S filesystem implementation.
     */
    virtual bool TryResolveSpecialDir( CORE::TSpecialDirs dir, CString& resolvedPath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CreateDir( const CString& dirPath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DeleteDir( const CString& dirPath, bool deleteContents ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool MoveDir( const CString& oldDirPath, const CString& newDirPath ) GUCEF_VIRTUAL_OVERRIDE;
    
    const CString& GetRootDir( void ) const;

    protected:

    /**
     *  Event callback member function.
     *
     *  @param notifier the notifier that sent the notification
     *  @param eventid the unique event id for an event
     *  @param eventdata optional notifier defined userdata
     */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;
    
    private:

    /** Whether a path of the archive stays in it: no ".." segment takes it out of the root */
    static bool IsWithinArchive( const CString& archivePath );

    private:

    CORE::CString GetVfsPathForFileSystemPath( const CORE::CString& fsPath ) const;

    void OnDirectoryWatcherEvent( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventid  ,
                                  CORE::CICloneable* eventdata );
    
    void GetListFromRoot( const CORE::CString& actualFsDir            , 
                          const CString& vfsMountLocation             , 
                          const CString& vfsArchiveLocation           ,
                          bool recursive                              , 
                          bool includePathInFilename                  , 
                          const CORE::CString::StringSet& nameFilters , 
                          TStringVector& outputList                   , 
                          bool addFiles                               ,
                          bool addDirs                                ) const;
    
    TBasicVfsResourcePtr LoadFromDisk( const CString& file                     ,
                                       const char* mode                        ,
                                       const CORE::CResourceMetaData* metaData ,
                                       const UInt32 memLoadSize = 0            ,
                                       const bool overwrite = false            );

    private:
    typedef CORE::CTSharedPtr< CORE::CDynamicBuffer, MT::CMutex > TDynamicBufferPtr;
    typedef std::map< CString, TDynamicBufferPtr >                TFileMemCache;
    
    TFileMemCache m_diskCacheList;
    CString m_rootDir;
    CString m_archiveName;
    bool m_writable;
    CORE::CFileSystemDirectoryWatcher m_fsWatcher;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< VFS::CArchive, CFileSystemArchive, MT::CMutex > TFileSystemArchiveFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_VFS_CFILESYSTEMARCHIVE_H ? */
