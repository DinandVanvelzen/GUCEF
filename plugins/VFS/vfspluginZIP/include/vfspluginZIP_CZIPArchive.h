/*
 *  vfspluginZIP: Generic GUCEF VFS plugin for ZIP archives
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef VFSPLUGIN_ZIP_CZIPARCHIVE_H
#define VFSPLUGIN_ZIP_CZIPARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFSPLUGIN_ZIP_MACROS_H
#include "vfspluginZIP_macros.h"
#define GUCEF_VFSPLUGIN_ZIP_MACROS_H
#endif /* GUCEF_VFSPLUGIN_ZIP_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

// Forward declaration for zziplib to avoid header file dependency.
typedef struct zzip_dir		ZZIP_DIR;
typedef struct zzip_file	ZZIP_FILE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace ZIP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CZIPArchive : public VFS::CArchive
{
    public:
    
    static const VFS::CString ZipArchiveTypeName;
    
    CZIPArchive( void );
    
    virtual ~CZIPArchive();
    
    virtual VFS::TBasicVfsResourcePtr GetFile( const VFS::CString& file          ,
                                               const char* mode = "rb"           ,
                                               const VFS::UInt32 memLoadSize = 0 ,
                                               const bool overwrite = false      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual VFS::TBasicVfsResourcePtr GetFileAs( const VFS::CString& file                ,
                                                 const CORE::CResourceMetaData& metaData ,
                                                 const char* mode = "wb"                 ,
                                                 const UInt32 memLoadSize = 0            ,
                                                 const bool overwrite = false            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StoreAsFile( const CORE::CString& filepath    ,
                              const CORE::CDynamicBuffer& data ,
                              const CORE::UInt64 offset        ,
                              const bool overwrite             ) GUCEF_VIRTUAL_OVERRIDE;
                                  
    virtual void GetFileList( TStringVector& outputList                                                   ,
                              const CORE::CString& mountLocation                                          , 
                              const CORE::CString& archiveLocation                                        ,
                              bool recursive = false                                                      ,
                              bool includePathInFilename = false                                          ,
                              const CORE::CString::StringSet& nameFilters = CORE::CString::EmptyStringSet ,
                              UInt32 maxListEntries = GUCEF_UINT16MAX                                     ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetDirList( TStringVector& outputList                                                   ,
                             const CORE::CString& mountLocation                                          , 
                             const CORE::CString& archiveLocation                                        ,
                             bool recursive = false                                                      ,
                             bool includeParentDirInName = false                                         ,
                             const CORE::CString::StringSet& nameFilters = CORE::CString::EmptyStringSet ,
                             UInt32 maxListEntries = GUCEF_UINT16MAX                                     ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool DeleteFile( const VFS::CString& filePath ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool MoveFile( const VFS::CString& oldFilePath ,
                           const VFS::CString& newFilePath ,
                           const bool overwrite            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetFileMetaData( const VFS::CString& filePath      ,
                                  CORE::CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FileExists( const VFS::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DirExists( const VFS::CString& dirPath ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual VFS::UInt64 GetFileSize( const VFS::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual VFS::CString GetFileHash( const VFS::CString& file ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual CORE::CDateTime GetFileModificationTime( const VFS::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual const VFS::CString& GetArchiveName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool LoadArchive( const VFS::CArchiveSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
                              
    virtual bool LoadArchive( const VFS::CString& archiveName       ,
                              VFS::TBasicVfsResourcePtr vfsResource ,
                              const bool writeableRequest           ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool UnloadArchive( void ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual const VFS::CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SZipEntry 
    {
        VFS::CString path;
        size_t compressedSize;
        size_t uncompressedSize;
        bool isDir;
    };
    typedef struct SZipEntry TZipEntry;
    
    CZIPArchive( const CZIPArchive& src );
    CZIPArchive& operator=( const CZIPArchive& src );

    void CheckZzipError( int zzipError                 ,
                         const VFS::CString& operation ) const;

    CORE::IOAccessPtr
    LoadFile( const VFS::CString& file      ,
              const VFS::UInt32 memLoadSize ) const;
    
    private:
    
    typedef std::vector< TZipEntry > TZipEntryVector;
    typedef CORE::CString::StringSet TDirSet;
    
    VFS::CString m_archiveName;
    VFS::CString m_archivePath;
    ZZIP_DIR* m_zipRoot;
    MT::CMutex m_zipLock;           /**< the open files share the handle and read position of m_zipRoot: one zziplib call at a time */
    TZipEntryVector m_fileList; 
    TDirSet m_dirs;                 /**< every folder: those with an entry of their own and those only in the paths of files */
    TDirSet m_lowercaseDirs;        /**< the same in lower case, as names are matched ignoring case */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ZIP */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* VFSPLUGIN_ZIP_CZIPARCHIVE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 04-05-2005 :
        - Dinand: Initial version.

-----------------------------------------------------------------------------*/
