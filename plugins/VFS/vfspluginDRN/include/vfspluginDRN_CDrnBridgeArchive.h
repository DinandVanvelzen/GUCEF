/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEARCHIVE_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEARCHIVE_H
#include "gucefVFS_CVfsRemoteArchive.h"
#define GUCEF_VFS_CVFSREMOTEARCHIVE_H
#endif /* GUCEF_VFS_CVFSREMOTEARCHIVE_H ? */

#ifndef GUCEF_VFS_CVFSFETCHSCHEDULER_H
#include "gucefVFS_CVfsFetchScheduler.h"
#define GUCEF_VFS_CVFSFETCHSCHEDULER_H
#endif /* GUCEF_VFS_CVFSFETCHSCHEDULER_H ? */

#ifndef GUCEF_VFS_CVFSHANDLE_H
#include "gucefVFS_CVFSHandle.h"
#define GUCEF_VFS_CVFSHANDLE_H
#endif /* GUCEF_VFS_CVFSHANDLE_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H
#include "vfspluginDRN_CDrnBridgeSettings.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGECONNECTION_H
#include "vfspluginDRN_CDrnBridgeConnection.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGECONNECTION_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGECONNECTION_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEBACKEND_H
#include "vfspluginDRN_CDrnBridgeBackend.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEBACKEND_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEBACKEND_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMETADATACACHE_H
#include "vfspluginDRN_CDrnBridgeMetadataCache.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMETADATACACHE_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMETADATACACHE_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H
#include "vfspluginDRN_CDrnBridgeHandlePool.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGELASTRESULTS_H
#include "vfspluginDRN_CDrnBridgeLastResults.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGELASTRESULTS_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGELASTRESULTS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A network drive between two GUCEF processes: a read only VFS archive that presents a share of another node of a DRN
 *  session. Archive type "drn".
 *
 *  What it is made of. The archive is a VFS::CVfsRemoteArchive: the behaviour that every remote or slow archive shares comes from the generic
 *  layer of gucefVFS (VFS::CVfsRemoteArchiveCore): the rule of the calling thread for a miss (VFS::CVfsThreadPolicy), background fetches that end in an
 *  event, the persistent block cache with offline serving, pins, digest hints, whole file fetches, statistics and the events (VFS::CRemoteArchiveEvents).
 *  What the share itself can do is the backend of that layer (CDrnBridgeBackend): the names (lists, existence, sizes, times, revisions, digests the share
 *  knows) come from the namespace operations of the serving node (DRN::CDRNNamespaceClient), the bytes come through the long lived handles of the
 *  resource transfer facility (CDrnBridgeFileAccess, CDrnBridgeHandlePool, used raw by CDrnBridgeBlockSource: one handle per file that is being read, the
 *  least recently used one closed for the next, a file that changed on the serving side ends as "changed"). A metadata cache with a time to live and revision
 *  based revalidation keeps the number of calls down (CDrnBridgeMetadataCache). Without a block cache the archive reads through the facility's chunk
 *  cache as before. This version is read only: IsWriteable() is false and every call that would change the share is refused (the result says read only).
 *
 *  Opening a file takes no handle: a file is opened from the metadata that is known (size and revision), pinned at the revision that metadata had, which is
 *  no older than metadataTtlMs (or the last InvalidateMetadata()); the handle is opened when a block is missing and has to be fetched, for that revision, and
 *  the share answers "changed" when it is another one by then. With metadataTtlMs at 0 the metadata is asked at every open.
 *
 *  The block cache, what a thread that must not wait does and the events are described in VFS::CVfsRemoteArchiveCore; the settings that switch them on are
 *  those of CDrnBridgeSettings (blockCacheFolder, cacheIdentity, ...) and are in the README.
 *
 *  What it does not do: discover, connect or authenticate. The application provides the DRN session, as the session itself
 *  (through the registry of sessions, or as an object registered by id, or directly through the typed LoadArchive()
 *  below) and the archive uses the direct link to one member of it.
 *
 *  Mounting from configuration: type "drn" and, as the actual path (or path) of the archive, one of
 *    drn://<session name>/<member>/<share>   (also as drn:/<session name>/<member>/<share>, the form the path handling of the archive settings makes of it)
 *        the session is found by its name in the DRN session registry (CDRNGlobal::GetSessionRegistry()), the member is the
 *        node id of a member or "sequencer" for the host of the session, the share is the name of the provider the member
 *        shares; a member is looked up each time it is used, so a member that left and came back is found again.
 *    <connection id>|<share>
 *        a connection that the application registered under an id (CDrnBridgeGlobal::RegisterConnection()), for
 *        applications that hold the session and the peer id themselves.
 *  The settings (cache times, number of handles, ...) are archive specific settings, see CDrnBridgeSettings and the README.
 *
 *  Paths. Every path is relative to the share, '/' or '\' separated; a leading or trailing separator makes no difference.
 *  A path that has a ".." part is refused here without asking, and the serving side refuses them too. The lists give what
 *  CFileSystemArchive gives: names for a list of files, names with a trailing '/' for a list of directories, entries
 *  relative to the listed location for a recursive list, and with the path included mountLocation + '/' + the path relative
 *  to the share; maxListEntries is honoured. Name filters are applied as CVFS::FilterValidation() does.
 *
 *  When the share cannot be reached the archive stays mounted: IsConnected() is false, what the metadata cache holds is
 *  answered (marked as offline in the metadata of a file), everything else fails and says that the share cannot be reached
 *  (GetLastResult()). When the link is back the next call asks again; entries that have to be checked are checked with the
 *  revision the cache has, which costs no more than an "unchanged" answer when they did not change.
 *
 *  Why a call failed is in GetLastResult(): the result of the latest call of the calling thread. Blocking calls are made on the calling thread and
 *  refused on a thread that works on the node of the session (they could never be answered); that is logged once.
 *
 *  Files. A file opened through GetFile() is pinned at the revision it had when it was opened: when it changes on the serving
 *  side that opening of it fails from the next read on, and opening it again reads the new one.
 *  The memLoadSize parameter has no meaning here (the facility's chunk cache is the memory a file uses).
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeArchive : public VFS::CVfsRemoteArchive
{
    public:

    CDrnBridgeArchive( void );

    virtual ~CDrnBridgeArchive() GUCEF_VIRTUAL_OVERRIDE;

    virtual VFS::TBasicVfsResourcePtr GetFile( const VFS::CString& file          ,
                                               const char* mode = "rb"           ,
                                               const VFS::UInt32 memLoadSize = 0 ,
                                               const bool overwrite = false      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StoreAsFile( const CORE::CString& filepath    ,
                              const CORE::CDynamicBuffer& data ,
                              const CORE::UInt64 offset        ,
                              const bool overwrite             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetFileList( TStringVector& outputList                                                 ,
                              const VFS::CString& mountLocation                                         ,
                              const VFS::CString& archiveLocation                                       ,
                              bool recursive = false                                                    ,
                              bool includePathInFilename = false                                        ,
                              const CORE::CString::StringSet& nameFilters = CORE::CString::EmptyStringSet ,
                              UInt32 maxListEntries = GUCEF_UINT32MAX                                   ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetDirList( TStringVector& outputList                                                 ,
                             const VFS::CString& mountLocation                                         ,
                             const VFS::CString& archiveLocation                                       ,
                             bool recursive = false                                                    ,
                             bool includeParentDirInName = false                                       ,
                             const CORE::CString::StringSet& nameFilters = CORE::CString::EmptyStringSet ,
                             UInt32 maxListEntries = GUCEF_UINT32MAX                                   ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DeleteFile( const VFS::CString& filePath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool MoveFile( const VFS::CString& oldFilePath ,
                           const VFS::CString& newFilePath ,
                           const bool overwrite            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool MoveFileAtomically( const VFS::CString& oldFilePath ,
                                     const VFS::CString& newFilePath ,
                                     const bool overwrite            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FileExists( const VFS::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DirExists( const VFS::CString& dirPath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual VFS::UInt64 GetFileSize( const VFS::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetFileMetaData( const VFS::CString& filePath      ,
                                  CORE::CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The SHA-256 of the file as hexadecimal text (upper case, as CORE::Base16Encode() gives it) when the share knows it (a provider that has a digest at hand
     *  gives it with the entry), else the empty string: nothing is ever read or computed on either side to answer this call.
     *  The algorithm is not the MD5 that CFileSystemArchive answers with: there is no contract on the algorithm across archive types. GetFileDigest() names it.
     */
    virtual VFS::CString GetFileHash( const VFS::CString& file ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The digest that the share knows (SHA-256, lower case, the algorithm named), else a digest that the application gave with NoteFileDigest(), else the
     *  digest that the block cache verified for the file; empty when there is none. Nothing is computed for this call, on either side.
     */
    virtual VFS::CVfsDigest GetFileDigest( const VFS::CString& file ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CDateTime GetFileModificationTime( const VFS::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const VFS::CString& GetArchiveName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Mounts from the source in the settings, see the class description */
    virtual bool LoadArchive( const VFS::CArchiveSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    /** Not supported: the share is a place on another node, not a resource: always false */
    virtual bool LoadArchive( const VFS::CString& archiveName       ,
                              VFS::TBasicVfsResourcePtr vfsResource ,
                              const bool writeableRequest           ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The typed form of mounting, for applications that hold the connection: the share of the member the connection stands for.
     *  Does not need the share to be reachable at that moment. False for a share name that cannot be one or a connection that is NULL.
     */
    bool LoadArchive( const CDrnBridgeConnectionPtr& connection ,
                      const CORE::CString& share                ,
                      const CDrnBridgeSettings& settings        ,
                      const CORE::CString& archiveName          );

    virtual bool UnloadArchive( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const VFS::CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsHealthy( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the share can be reached now: there is an operational direct link to the member that serves it */
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CreateDir( const VFS::CString& dirPath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DeleteDir( const VFS::CString& dirPath, bool deleteContents ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool MoveDir( const VFS::CString& oldDirPath, const VFS::CString& newDirPath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Why the latest call of the calling thread did not do what was asked: a success when it did. The calls of the archive answer
     *  with a bool, a size or an empty value; this has the reason (not found, access denied, the share cannot be reached, changed, ...)
     *  in the forms described in CDrnBridgeResult.
     */
    CDrnBridgeResult GetLastResult( void ) const;

    /** Forgets what the metadata cache holds, all of it or what is known of one path that was created, removed or changed: the path itself, its listing when it is a directory and the listing of the directory it is in */
    void InvalidateMetadata( void );
    void InvalidateMetadata( const CORE::CString& path );

    const CORE::CString& GetShare( void ) const;
    const CDrnBridgeSettings& GetSettings( void ) const;
    const CDrnBridgeConnectionPtr& GetConnection( void ) const;

    /** Handles of the pool: open now, the most there were, closed to make room, opened again for a file read after that */
    UInt32 GetOpenHandleCount( void ) const;
    UInt32 GetPeakOpenHandleCount( void ) const;
    UInt64 GetHandleEvictionCount( void ) const;
    UInt64 GetHandleReopenCount( void ) const;

    /** The handles this mount may hold open now: its own setting or its share of what the member announced when that is lower */
    UInt32 GetHandleLimit( void ) const;

    /** Records the metadata cache holds */
    UInt32 GetCachedEntryCount( void ) const;

    /** The backend that the generic layer works on: the share, its metadata cache and its handle pool */
    const CDrnBridgeBackendPtr& GetBackend( void ) const;

    /** The block cache that this archive reads through, empty when it has none */
    VFS::CVfsBlockCachePtr GetBlockCache( void ) const;

    /** The name of the share in the block cache: the setting cacheIdentity, or the address when that is empty */
    const CORE::CString& GetCacheIdentity( void ) const;

    /** The scheduler of the background fetches of this archive, NULL when it is not loaded */
    VFS::CVfsFetchScheduler* GetFetchScheduler( void ) const;

    /** Runs of bytes asked of the share and the bytes that came, since the archive was loaded */
    UInt64 GetFetchCallCount( void ) const;
    UInt64 GetBytesFetched( void ) const;

    /** The path as the archive uses it: '/' separated, no leading, trailing or repeated separators, "." parts gone; false when it has a ".." part */
    static bool NormalizePath( const CORE::CString& path, CORE::CString& normalized );

    /**
     *  Takes the form drn://<session name>/<member>/<share> apart (also the form drn:/<session name>/... that the path handling of
     *  the archive settings leaves); false, with the reason in 'why', when it is not of that form or a part is empty
     */
    static bool ParseDrnAddress( const CORE::CString& source, CORE::CString& sessionName, CORE::CString& member, CORE::CString& share, CORE::CString& why );

    private:

    CDrnBridgeArchive( const CDrnBridgeArchive& src );                  /**< not implemented */
    CDrnBridgeArchive& operator=( const CDrnBridgeArchive& src );       /**< not implemented */

    /** Gives the connection a source string asks for, with the share: both forms of the class description */
    bool ResolveSource( const CORE::CString& source, CDrnBridgeConnectionPtr& connection, CORE::CString& share, CORE::CString& why ) const;

    bool IsLoaded( void ) const;

    const CDrnBridgeResult& Record( const CDrnBridgeResult& result ) const;

    bool Refuse( const char* what ) const;

    /** What a path is, as the generic layer says it: one look at the metadata, with the result of the call in the form of the bridge */
    enum EKind { KIND_FAILED = 0, KIND_ABSENT, KIND_FILE, KIND_DIRECTORY };
    EKind Classify( const CORE::CString& normalizedPath, CORE::CResourceMetaData& metaData ) const;

    /** Ends a call: what the generic layer made of it is recorded as the result of the call, when the backend recorded no failure of its own */
    void Settle( void ) const;

    /** The name of the share in the block cache when the settings give none */
    CORE::CString MakeDefaultIdentity( void ) const;

    CORE::CString m_archiveName;
    CORE::CString m_archivePath;
    CORE::CString m_share;
    CORE::CString m_identity;
    CDrnBridgeSettings m_settings;
    mutable CDrnBridgeConnectionPtr m_connection;       /**< the objects behind the archive are shared and thread safe, the calls on them are not changes of the archive */
    CDrnBridgeBackendPtr m_backend;
    mutable CDrnBridgeLastResultsPtr m_lastResults;
    VFS::CVfsFetchScheduler* m_scheduler;
    MT::CTAtomic< UInt32 > m_loaded;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEARCHIVE_H ? */
