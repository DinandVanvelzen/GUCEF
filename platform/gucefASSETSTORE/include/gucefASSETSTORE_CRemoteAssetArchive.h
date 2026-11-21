/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETARCHIVE_H
#define GUCEF_ASSETSTORE_CREMOTEASSETARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CARCHIVE_H
#include "gucefVFS_CArchive.h"
#define GUCEF_VFS_CARCHIVE_H
#endif /* GUCEF_VFS_CARCHIVE_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The VFS archive of the type "assetstore" (remote service design 5.4): a storage location of a remote asset store service, read
 *  only. CRemoteAssetFileSystem::MountLocation() mounts it; the files read through the block cache of the file system (CRemoteFileAccess),
 *  the sizes and the lists answer from what the file system knows of the folders, and a hash is the SHA-256 that the catalogue gave
 *  (empty when it is not known). Nothing here waits on the thread that the file system treats as the no-wait thread: what is not
 *  known there is asked for in the background.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CRemoteAssetArchive : public VFS::CArchive
{
    public:

    CRemoteAssetArchive( void );

    virtual ~CRemoteAssetArchive() GUCEF_VIRTUAL_OVERRIDE;

    virtual VFS::TBasicVfsResourcePtr GetFile( const CORE::CString& file    ,
                                               const char* mode = "rb"      ,
                                               const UInt32 memLoadSize = 0 ,
                                               const bool overwrite = false ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StoreAsFile( const CORE::CString& filepath    ,
                              const CORE::CDynamicBuffer& data ,
                              const CORE::UInt64 offset        ,
                              const bool overwrite             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetFileList( TStringVector& outputList                                                 ,
                              const CORE::CString& mountLocation                                        ,
                              const CORE::CString& archiveLocation                                      ,
                              bool recursive = false                                                    ,
                              bool includePathInFilename = false                                        ,
                              const CORE::CString::StringSet& nameFilters = CORE::CString::EmptyStringSet ,
                              UInt32 maxListEntries = GUCEF_UINT16MAX                                   ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetDirList( TStringVector& outputList                                                 ,
                             const CORE::CString& mountLocation                                        ,
                             const CORE::CString& archiveLocation                                      ,
                             bool recursive = false                                                    ,
                             bool includePathInFilename = false                                        ,
                             const CORE::CString::StringSet& nameFilters = CORE::CString::EmptyStringSet ,
                             UInt32 maxListEntries = GUCEF_UINT16MAX                                   ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DeleteFile( const CORE::CString& filePath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool MoveFile( const CORE::CString& oldFilePath ,
                           const CORE::CString& newFilePath ,
                           const bool overwrite             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FileExists( const CORE::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DirExists( const CORE::CString& dirPath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetFileSize( const CORE::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetFileMetaData( const CORE::CString& filePath      ,
                                  CORE::CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetFileHash( const CORE::CString& file ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CDateTime GetFileModificationTime( const CORE::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetArchiveName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadArchive( const VFS::CArchiveSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadArchive( const CORE::CString& archiveName       ,
                              VFS::TBasicVfsResourcePtr vfsResource ,
                              const bool writeableRequest           ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UnloadArchive( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CRemoteAssetArchive( const CRemoteAssetArchive& src );
    CRemoteAssetArchive& operator=( const CRemoteAssetArchive& src );

    CORE::CString ToServicePath( const CORE::CString& archivePath ) const;

    private:

    CORE::CString m_archiveName;
    CORE::CString m_fileSystemId;
    CORE::CString m_serviceRoot;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CREMOTEASSETARCHIVE_H ? */
