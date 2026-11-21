/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_VFS_CVFSREMOTEARCHIVE_H
#define GUCEF_VFS_CVFSREMOTEARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CARCHIVE_H
#include "gucefVFS_CArchive.h"
#define GUCEF_VFS_CARCHIVE_H
#endif /* GUCEF_VFS_CARCHIVE_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEARCHIVECORE_H
#include "gucefVFS_CVfsRemoteArchiveCore.h"
#define GUCEF_VFS_CVFSREMOTEARCHIVECORE_H
#endif /* GUCEF_VFS_CVFSREMOTEARCHIVECORE_H ? */

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

/**
 *  A read only archive of a remote or slow place that gets its behaviour from CVfsRemoteArchiveCore: the answers of the
 *  CArchive calls, the cached reads, the miss rule of the calling thread, the background fetches, offline serving and the
 *  events. What a derived class has to give is the remote place (CIVfsRemoteBackend) and the loading of the archive: it makes
 *  the core with CVfsRemoteArchiveCore::Create() and hands it to AttachCore().
 *
 *  The archive raises the events of the core as its own (CRemoteArchiveEvents), which is what the VFS subscribes to.
 *  UnloadArchive() drops the background jobs and ends the events; files that are still open fail from then on.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsRemoteArchive : public CArchive
{
    public:

    CVfsRemoteArchive( void );

    virtual ~CVfsRemoteArchive() GUCEF_VIRTUAL_OVERRIDE;

    virtual TBasicVfsResourcePtr GetFile( const CString& file          ,
                                          const char* mode = "rb"      ,
                                          const UInt32 memLoadSize = 0 ,
                                          const bool overwrite = false ) GUCEF_VIRTUAL_OVERRIDE;

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
                              UInt32 maxListEntries = GUCEF_UINT32MAX                         ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetDirList( TStringVector& outputList                                       ,
                             const CString& mountLocation                                    ,
                             const CString& archiveLocation                                  ,
                             bool recursive = false                                          ,
                             bool includeParentDirInName = false                             ,
                             const CString::StringSet& nameFilters = CString::EmptyStringSet ,
                             UInt32 maxListEntries = GUCEF_UINT32MAX                         ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DeleteFile( const CString& filePath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool MoveFile( const CString& oldFilePath ,
                           const CString& newFilePath ,
                           const bool overwrite       ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FileExists( const CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DirExists( const CString& dirPath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetFileSize( const CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetFileMetaData( const CString& filePath           ,
                                  CORE::CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetFileHash( const CString& file ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CVfsDigest GetFileDigest( const CString& file ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CDateTime GetFileModificationTime( const CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetArchiveName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UnloadArchive( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsHealthy( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CIRemoteArchive* GetRemoteArchiveInterface( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyObject( CVFSHandle* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The core that gives the archive its behaviour; empty before AttachCore() and after UnloadArchive() */
    CVfsRemoteArchiveCorePtr GetCore( void ) const;

    protected:

    /** Takes the core, subscribes to its events (raised again as events of the archive) and names the archive */
    void AttachCore( const CVfsRemoteArchiveCorePtr& core, const CORE::CString& archiveName );

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CVfsRemoteArchive( const CVfsRemoteArchive& src );                  /**< not implemented */
    CVfsRemoteArchive& operator=( const CVfsRemoteArchive& src );       /**< not implemented */

    private:

    CVfsRemoteArchiveCorePtr m_core;
    CVfsRemoteArchiveCorePtr m_unloadedCore;                    /**< the core after UnloadArchive(): kept until the archive goes, because the interface that GetRemoteArchiveInterface() hands out is a plain pointer to it */
    CORE::CString m_archiveName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSREMOTEARCHIVE_H ? */
