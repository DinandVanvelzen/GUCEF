/*
 *  vfspluginUNITYPACKAGE: GUCEF VFS plugin presenting a Unity package as the project tree it contains
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

#ifndef GUCEF_VFSPLUGIN_UNITYPACKAGE_CUNITYPACKAGEARCHIVE_H
#define GUCEF_VFSPLUGIN_UNITYPACKAGE_CUNITYPACKAGEARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFSPLUGIN_UNITYPACKAGE_MACROS_H
#include "vfspluginUNITYPACKAGE_macros.h"
#define GUCEF_VFSPLUGIN_UNITYPACKAGE_MACROS_H
#endif /* GUCEF_VFSPLUGIN_UNITYPACKAGE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace UNITYPACKAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Read only VFS archive presenting a Unity package as the Unity project tree it contains.
 *
 *  A Unity package is a gzip compressed tar with one folder per asset, named after the asset GUID, holding
 *  the files "pathname" (the project path of the asset), "asset" (the content, absent for folders),
 *  "asset.meta" and optionally "preview.png". The archive presents these as:
 *    <pathname>              the content of "asset"
 *    <pathname>.meta         the content of "asset.meta"
 *    <pathname>.preview.png  the content of "preview.png"
 *  Folders are the directories implied by the pathnames and the GUIDs that have no "asset".
 *  Anything else in the package, such as the icon an Asset Store download adds at the root, is ignored.
 *
 *  Reading strategy: a gzip compressed tar cannot be seeked, so each entry can only be reached by decompressing
 *  the package from its start. At mount the package is decompressed once and indexed: the position of every entry
 *  is recorded and the small pathname entries are read. In that same pass entries up to CacheEntryLimit bytes
 *  (the .meta files, the previews and small assets, which are what a project scan touches) are kept in memory,
 *  up to CacheTotalLimit bytes, so that opening them costs nothing. Other entries are decompressed up to their
 *  position when they are opened. Entries up to LoadOnOpenLimit bytes are loaded into memory at that time so
 *  that the content can be accessed randomly. Larger entries are streamed, where seeking backwards decompresses
 *  from the start again. The cost of opening an uncached entry is therefore linear in its position in the package,
 *  the price of the format. The index is immutable after mounting, which makes the archive safe to use from
 *  multiple threads.
 *
 *  The package can be mounted from a file or, with the VFS resource based mount, from any other resource such as
 *  a file inside another mounted archive.
 */
class CUnityPackageArchive : public VFS::CArchive
{
    public:

    /** Entries up to this size that are read at mount time are kept in memory */
    static const UInt32 CacheEntryLimit = 64 * 1024;

    /** Total number of bytes that are kept in memory for the entries above */
    static const UInt32 CacheTotalLimit = 32 * 1024 * 1024;

    /** Entries up to this size that are not cached are loaded into memory when opened, larger entries are streamed */
    static const UInt32 LoadOnOpenLimit = 256 * 1024 * 1024;

    CUnityPackageArchive( void );

    virtual ~CUnityPackageArchive() GUCEF_VIRTUAL_OVERRIDE;

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
     *  Normalizes a path inside the package: '/' as separator, no leading "./" or '/' and no trailing '/'
     */
    static CORE::CString NormalizePath( const CORE::CString& path );

    private:

    /** a file as presented by the archive */
    struct SEntry
    {
        SEntry( void );

        UInt32 index;                   /**< position of the entry in the tar */
        UInt64 size;
        CORE::CDateTime modificationTime;
        bool hasModificationTime;
        Int32 cacheSlot;                /**< index in the cache or -1 if the content is not kept in memory */
    };
    typedef GUCEF::map< CORE::CString, SEntry > TEntryMap;
    typedef GUCEF::set< CORE::CString > TDirSet;
    typedef GUCEF::vector< CORE::CDynamicBuffer* > TCache;

    /** what the tar holds for a single asset GUID, used while indexing */
    struct SAsset
    {
        SAsset( void );

        CORE::CString pathname;
        bool hasAsset;
        bool hasMeta;
        bool hasPreview;
        SEntry asset;
        SEntry meta;
        SEntry preview;
    };
    typedef GUCEF::map< CORE::CString, SAsset > TAssetMap;

    CUnityPackageArchive( const CUnityPackageArchive& src );            /**< not supported */
    CUnityPackageArchive& operator=( const CUnityPackageArchive& src ); /**< not supported */

    bool BuildIndex( const CORE::CString& path, const CORE::IOAccessPtr& sourceAccess );

    void AddFile( const CORE::CString& path, const SEntry& entry );

    const SEntry* FindEntry( const VFS::CString& filePath ) const;

    void AddParentDirs( const CORE::CString& path );

    TEntryMap m_files;
    TDirSet m_dirs;
    TCache m_cache;
    VFS::CString m_archiveName;
    VFS::CString m_archivePath;
    CORE::CString m_actualArchivePath;
    CORE::IOAccessPtr m_sourceAccess;
    VFS::TBasicVfsResourcePtr m_sourceResource;
    VFS::CString m_type;
    bool m_loaded;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITYPACKAGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_UNITYPACKAGE_CUNITYPACKAGEARCHIVE_H ? */
