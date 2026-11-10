/*
 *  vfspluginISO9660: Generic GUCEF VFS plugin for ISO 9660 disc images
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

#ifndef GUCEF_VFSPLUGIN_ISO9660_CISO9660ARCHIVE_H
#define GUCEF_VFSPLUGIN_ISO9660_CISO9660ARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_VFSPLUGIN_ISO9660_MACROS_H
#include "vfspluginISO9660_macros.h"
#define GUCEF_VFSPLUGIN_ISO9660_MACROS_H
#endif /* GUCEF_VFSPLUGIN_ISO9660_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace ISO9660 {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Read only archive for ISO 9660 (ECMA-119) disc images, as used on CD-ROM and DVD-ROM.
 *
 *  The image is accessed through an IO access object, so it can come from a file, a mounted disc, a block
 *  of memory, a file descriptor or even from a resource inside another archive. Nothing is extracted: files
 *  are served as windows on the image. Only data in 2048 byte "cooked" sectors is supported. Raw 2352 byte
 *  images (.bin) are mounted through their cue sheet with the "cue" archive type of vfspluginCUEBIN, which reads
 *  the data track with this archive.
 *
 *  Names: when a Joliet volume descriptor is present those names are used (Unicode, long names), otherwise
 *  Rock Ridge names when present, otherwise the plain ISO 9660 names (upper case, with the ";1" version
 *  suffix and a trailing dot removed). Lookups ignore case, listings return the names as stored.
 *
 *  Limitations: files that consist of several non contiguous extents cannot be served, directory relocation
 *  of Rock Ridge (deep directory trees on plain ISO names) is not followed.
 */
class CISO9660Archive : public VFS::CArchive
{
    public:

    static const VFS::CString ISO9660ArchiveTypeName;

    CISO9660Archive( void );

    virtual ~CISO9660Archive();

    virtual VFS::TBasicVfsResourcePtr GetFile( const VFS::CString& file          ,
                                               const char* mode = "rb"           ,
                                               const VFS::UInt32 memLoadSize = 0 ,
                                               const bool overwrite = false      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StoreAsFile( const CORE::CString& filepath    ,
                              const CORE::CDynamicBuffer& data ,
                              const CORE::UInt64 offset        ,
                              const bool overwrite             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetFileList( TStringVector& outputList                                                    ,
                              const CORE::CString& mountLocation                                           ,
                              const CORE::CString& archiveLocation                                         ,
                              bool recursive = false                                                       ,
                              bool includePathInFilename = false                                           ,
                              const CORE::CString::StringSet& nameFilters = CORE::CString::EmptyStringSet  ,
                              UInt32 maxListEntries = GUCEF_UINT16MAX                                      ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetDirList( TStringVector& outputList                                                    ,
                             const CORE::CString& mountLocation                                           ,
                             const CORE::CString& archiveLocation                                         ,
                             bool recursive = false                                                       ,
                             bool includeParentDirInName = false                                          ,
                             const CORE::CString::StringSet& nameFilters = CORE::CString::EmptyStringSet  ,
                             UInt32 maxListEntries = GUCEF_UINT16MAX                                      ) const GUCEF_VIRTUAL_OVERRIDE;

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

    /** Loads the image from the file the settings point to */
    virtual bool LoadArchive( const VFS::CArchiveSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    /** Loads the image from an already opened resource, for example a file inside another archive */
    virtual bool LoadArchive( const VFS::CString& archiveName       ,
                              VFS::TBasicVfsResourcePtr vfsResource ,
                              const bool writeableRequest           ) GUCEF_VIRTUAL_OVERRIDE;

    /** Loads the image from an IO access object. The image stays in use until the archive is unloaded */
    bool LoadArchive( const VFS::CString& archiveName ,
                      const VFS::CString& archivePath ,
                      CORE::IOAccessPtr imageAccess   );

    virtual bool UnloadArchive( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const VFS::CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Volume identifier from the volume descriptor, empty if unloaded */
    const VFS::CString& GetVolumeId( void ) const;

    /** true if the names come from the Joliet extension */
    bool UsesJolietNames( void ) const;

    /** true if the names come from the Rock Ridge extension */
    bool UsesRockRidgeNames( void ) const;

    /** Number of files and directories in the image, not counting the root */
    VFS::UInt32 GetEntryCount( void ) const;

    private:

    CISO9660Archive( const CISO9660Archive& src );
    CISO9660Archive& operator=( const CISO9660Archive& src );

    struct SEntry
    {
        VFS::CString name;              /**< name as stored on the image */
        bool isDir;
        bool readable;                  /**< false for files that cannot be served */
        VFS::UInt32 extent;             /**< first logical block */
        VFS::UInt64 size;               /**< size in bytes */
        CORE::CDateTime modifiedTime;

        SEntry( void ) : isDir( false ), readable( true ), extent( 0 ), size( 0 ), modifiedTime() {}
    };
    typedef struct SEntry TEntry;
    typedef GUCEF::map< VFS::CString, TEntry >          TEntryMap;      /**< key is the normalized path */
    typedef GUCEF::vector< VFS::CString >               TKeyVector;
    typedef GUCEF::map< VFS::CString, TKeyVector >      TDirMap;        /**< key is the normalized path of the dir, value the keys of its children */

    struct SDirToVisit
    {
        VFS::UInt32 extent;
        VFS::UInt32 size;
        VFS::CString path;
    };
    typedef struct SDirToVisit TDirToVisit;

    static VFS::CString NormalizePath( const VFS::CString& path );

    bool ReadBytes( VFS::UInt64 offset, void* dest, VFS::UInt32 byteCount ) const;

    bool ReadVolumeDescriptors( void );

    bool ReadDirectoryTree( VFS::UInt32 rootExtent, VFS::UInt32 rootSize );

    bool ParseDirectory( const TDirToVisit& dir, GUCEF::vector< TDirToVisit >& dirsToVisit );

    VFS::CString ReadRockRidgeName( const VFS::UInt8* systemUse, VFS::UInt32 systemUseSize ) const;

    static VFS::CString DecodeJolietName( const VFS::UInt8* name, VFS::UInt32 nameLength );

    static VFS::CString CleanIsoName( const VFS::UInt8* name, VFS::UInt32 nameLength );

    static CORE::CDateTime DecodeRecordingTime( const VFS::UInt8* sevenBytes );

    CORE::IOAccessPtr GetImageAccessForReading( void ) const;

    CORE::IOAccessPtr OpenEntry( const TEntry& entry, const VFS::UInt32 memLoadSize ) const;

    const TEntry* FindEntry( const VFS::CString& path ) const;

    void AppendDirectoryListing( TStringVector& outputList                     ,
                                 const VFS::CString& dirKey                    ,
                                 const VFS::CString& relativePrefix            ,
                                 const VFS::CString& mountLocation             ,
                                 bool wantDirs                                 ,
                                 bool recursive                                ,
                                 bool includePathInName                        ,
                                 const CORE::CString::StringSet& nameFilters   ,
                                 UInt32 maxListEntries                         ) const;

    private:

    VFS::CString m_archiveName;
    VFS::CString m_archivePath;
    VFS::CString m_volumeId;
    mutable CORE::IOAccessPtr m_image;   /**< mutable: reading moves the position of the image */
    VFS::TBasicVfsResourcePtr m_imageResource;
    VFS::UInt32 m_blockSize;
    bool m_joliet;
    bool m_rockRidge;
    TEntryMap m_entries;
    TDirMap m_dirs;
    mutable MT::CMutex m_imageLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ISO9660 */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_ISO9660_CISO9660ARCHIVE_H ? */
