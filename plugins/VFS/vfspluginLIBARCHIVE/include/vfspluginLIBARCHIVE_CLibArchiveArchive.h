/*
 *  vfspluginLIBARCHIVE: Generic GUCEF VFS plugin for archives handled by libarchive
 *  Copyright (C) 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_VFSPLUGIN_LIBARCHIVE_CLIBARCHIVEARCHIVE_H
#define GUCEF_VFSPLUGIN_LIBARCHIVE_CLIBARCHIVEARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <set>

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H
#include "vfspluginLIBARCHIVE_macros.h"
#define GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H
#endif /* GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

// Forward declaration to avoid a header file dependency on libarchive
struct archive;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace LIBARCHIVE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Read only VFS archive for all archive formats libarchive can read: tar (ustar, pax, GNU), zip including ZIP64,
 *  7-Zip, RAR, ISO 9660, cpio, cab, lha, xar, ar and more, optionally compressed with gzip.
 *  The format is detected from the content. When mounted the archive is indexed once, after which each file
 *  is streamed from the archive when it is opened.
 */
class CLibArchiveArchive : public VFS::CArchive
{
    public:

    CLibArchiveArchive( void );

    virtual ~CLibArchiveArchive() GUCEF_VIRTUAL_OVERRIDE;

    virtual VFS::TBasicVfsResourcePtr GetFile( const VFS::CString& file          ,
                                               const char* mode = "rb"           ,
                                               const VFS::UInt32 memLoadSize = 0 ,
                                               const bool overwrite = false      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StoreAsFile( const CORE::CString& filepath    ,
                              const CORE::CDynamicBuffer& data ,
                              const CORE::UInt64 offset        ,
                              const bool overwrite             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetFileList( TStringVector& outputList                                       ,
                              const VFS::CString& mountLocation                               ,
                              const VFS::CString& archiveLocation                             ,
                              bool recursive = false                                          ,
                              bool includePathInFilename = false                              ,
                              const CORE::CString::StringSet& nameFilters = CORE::CString::EmptyStringSet ,
                              UInt32 maxListEntries = GUCEF_UINT16MAX                         ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetDirList( TStringVector& outputList                                       ,
                             const VFS::CString& mountLocation                               ,
                             const VFS::CString& archiveLocation                             ,
                             bool recursive = false                                          ,
                             bool includeParentDirInName = false                             ,
                             const CORE::CString::StringSet& nameFilters = CORE::CString::EmptyStringSet ,
                             UInt32 maxListEntries = GUCEF_UINT16MAX                         ) const GUCEF_VIRTUAL_OVERRIDE;

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

    /**
     *  Opens the archive at the given path for reading with all formats and filters libarchive supports
     *  @return the archive handle, to be released with archive_read_free(), or NULL on failure
     */
    static struct archive* OpenForReading( const CORE::CString& path );

    /**
     *  Normalizes a path inside an archive: '/' as separator, no leading "./" or '/' and no trailing '/'
     */
    static CORE::CString NormalizePath( const CORE::CString& path );

    private:

    struct SEntry
    {
        UInt32 index;                   /**< position of the entry in the archive */
        UInt64 size;
        CORE::CDateTime modificationTime;
        bool hasModificationTime;
    };
    typedef std::map< CORE::CString, SEntry > TEntryMap;
    typedef std::set< CORE::CString > TDirSet;

    CLibArchiveArchive( const CLibArchiveArchive& src );            /**< not supported */
    CLibArchiveArchive& operator=( const CLibArchiveArchive& src ); /**< not supported */

    const SEntry* FindEntry( const VFS::CString& filePath ) const;

    void AddParentDirs( const CORE::CString& path );

    TEntryMap m_files;
    TDirSet m_dirs;
    VFS::CString m_archiveName;
    VFS::CString m_archivePath;
    CORE::CString m_actualArchivePath;
    VFS::CString m_type;
    bool m_loaded;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LIBARCHIVE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_LIBARCHIVE_CLIBARCHIVEARCHIVE_H ? */
