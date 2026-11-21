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

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETSHARES_H
#define GUCEF_ASSETSTORE_CREMOTEASSETSHARES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_EFILESYSTEMACCESSRESULT_H
#include "gucefCORE_EFileSystemAccessResult.h"
#define GUCEF_CORE_EFILESYSTEMACCESSRESULT_H
#endif /* GUCEF_CORE_EFILESYSTEMACCESSRESULT_H ? */

#ifndef GUCEF_VFS_CVFSREMOTESTATISTICS_H
#include "gucefVFS_CVfsRemoteStatistics.h"
#define GUCEF_VFS_CVFSREMOTESTATISTICS_H
#endif /* GUCEF_VFS_CVFSREMOTESTATISTICS_H ? */

#ifndef GUCEF_ASSETSTORE_CIREMOTEFILESOURCE_H
#include "gucefASSETSTORE_CIRemoteFileSource.h"
#define GUCEF_ASSETSTORE_CIREMOTEFILESOURCE_H
#endif /* GUCEF_ASSETSTORE_CIREMOTEFILESOURCE_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEFILESSETTINGS_H
#include "gucefASSETSTORE_CRemoteFilesSettings.h"
#define GUCEF_ASSETSTORE_CREMOTEFILESSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CREMOTEFILESSETTINGS_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

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

namespace DRNUTILS {
class CRemoteShareMount;
}

namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The files of a remote asset store service on this computer: the share of the service (one share of the root of its primary store, named by the
 *  setting shareName of the files settings, "assets" by default) mounted into the VFS as an archive of the type "drn" with a CRemoteShareMount, so that
 *  every VFS path that the catalogue gives opens on this computer unchanged (below a prefix, for a process that has the service in it too). What the
 *  files do is what the generic remote archive of the VFS does: sizes, kinds and lists are asked for the paths that are looked at (with a bounded
 *  cost), a read fetches only the blocks it touches into the block cache, a thread that is marked with CVfsThreadPolicy never waits for the service
 *  (what is not here is fetched in the background and announced with the events of the VFS: FileArrivedEvent, ListingArrivedEvent,
 *  FetchFailedEvent and ConnectionStateChangedEvent, with the VFS path of the mount in front), the files that are kept offline stay in the cache,
 *  and a file whose blocks are in the cache opens and reads while the service cannot be reached. The plugin of the VFS for DRN has to be loaded (or
 *  linked and its global made): Open() fails with that reason when the VFS does not know the archive type "drn".
 *
 *  What this class adds is what only an asset store knows: which share holds which location (the share is the longest common folder of the
 *  roots of the locations, or one share per location, named by its id, when the roots have nothing in common: the layout that the host of the
 *  service serves), the mapping of the paths of the service to the paths of this computer, the digests that the catalogue gave (hints for the
 *  verification of the files), the forgetting of what is known of a folder when the catalogue changes, the settings of the files of the previous client
 *  mapped onto the settings of the archive and its cache, and the transfers (CRemoteFileTransfer: file sets, keep offline, download to a folder).
 *
 *  Threads. Every call is thread safe. A call that reaches the service waits for it on the calling thread, unless the thread has the rule
 *  FAIL_AND_FETCH in CVfsThreadPolicyRegistry (the thread of the interface): there it answers from what is held and starts the fetch of the rest.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CRemoteAssetShares : public CORE::CObservingNotifier, public CIRemoteFileSource
{
    public:

    static const CORE::CEvent StateChangedEvent;        /**< no data: a share went online or offline (see IsOffline()) */
    static const CORE::CString DefaultShareName;        /**< "assets" */

    static void RegisterEvents( void );

    /** One share of the service as it is mounted on this computer */
    struct GUCEF_ASSETSTORE_PUBLIC_CPP SShare
    {
        CORE::CString name;                 /**< the name the service shares the files under */
        CORE::CString serviceRoot;          /**< the VFS root of the service that the share is the root of */
        CORE::CString mountPoint;           /**< the VFS path of this computer the share is mounted at */
        SShare( void );
    };
    typedef GUCEF::vector< SShare > TShareVector;

    /**
     *  The shares of the locations that a service reports: one share named 'shareName' of the longest common folder of the roots of all the locations
     *  (when that exists and a location is online), else one share per online location, named by the id of the location, of its root. The shares
     *  are mounted at 'mountPrefix' + "/" + their root (at their root when there is no prefix)
     */
    static void MakeShareLayout( const CAssetLocationPage& locations, const CORE::CString& shareName, const CORE::CString& mountPrefix, TShareVector& shares );

    /** The longest common part, by whole folder names, of the roots; empty when there is none */
    static CORE::CString FindCommonRoot( const CORE::CString::StringVector& roots );

    /** The archive specific settings of the mount of the files (the names of the README of the VFS plugin for DRN) for the settings of the files */
    static void MakeBridgeSettings( const CRemoteFilesSettings& settings, const CORE::CString& cacheIdentity, CORE::CDataNode& bridgeSettings );

    /** The service is not owned and has to outlive this object. It has to be a remote service (CRemoteAssetStoreService): its connection carries the share */
    CRemoteAssetShares( CIAssetStoreService& service, const CRemoteFilesSettings& settings );

    virtual ~CRemoteAssetShares() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Mounts the shares of the locations that the service reports now ('mountPrefix' in front of the paths of the VFS), starts following the catalogue of the
     *  service and, at the end, puts the settings of the cache to use. False with a logged reason when nothing can be mounted
     */
    bool Open( const CORE::CString& mountPrefix = CORE::CString::Empty );

    /** Cancels the transfers that run, unmounts the shares and lets go of the cache */
    void Close( void );

    bool IsOpen( void ) const;

    const CRemoteFilesSettings& GetSettings( void ) const;
    CIAssetStoreService& GetService( void ) const;

    /** The shares that are mounted */
    void GetShares( TShareVector& shares ) const;

    /** The mount of a share (NULL for an index that is not there); the state of the share and the reason of a failure are asked of it */
    DRNUTILS::CRemoteShareMount* GetMount( UInt32 index ) const;

    /** Whether a path of this computer (a VFS path) is below a mount of this object */
    bool Contains( const CORE::CString& vfsPath ) const;

    /** The VFS roots of the locations of the service that the mounted shares cover, as the service has them */
    void GetLocationRoots( CORE::CString::StringVector& serviceRoots ) const;

    /** Whether the root of a location of the service is covered by a mounted share */
    bool IsMounted( const CORE::CString& serviceRoot ) const;

    virtual bool ToServicePath( const CORE::CString& clientPath, CORE::CString& servicePath ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString ToClientPath( const CORE::CString& servicePath ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Size and kind of a path (from what is held, else asked of the service by a thread that may wait). False for a path that is not there or not known yet */
    virtual bool GetFileInfo( const CORE::CString& servicePath, SFileInfo& info ) GUCEF_VIRTUAL_OVERRIDE;

    /** The names in a folder; false when the list is not known (a thread that must not wait: it is asked for) or the folder is not there */
    virtual bool GetFolderEntries( const CORE::CString& serviceFolder, CORE::CString::StringVector& dirs, CORE::CString::StringVector& files, bool* truncated = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    /** Starts the list of a folder in the background (on any thread, which never waits for it); ListingArrivedEvent of the VFS tells when it is in */
    void RequestFolder( const CORE::CString& serviceFolder );

    /** Whether the list of a folder is held (nothing is asked of the service) */
    bool IsFolderKnown( const CORE::CString& serviceFolder ) const;

    /** Remembers the SHA-256 of a file; the archive checks the file against it when it is complete */
    virtual void NoteDigest( const CORE::CString& servicePath, const CORE::CString& digest ) GUCEF_VIRTUAL_OVERRIDE;

    /** Remembers the digests that the catalogue gave (the file set of an asset) */
    void NoteFiles( const GUCEF::vector< CAssetFileRecord >& files );

    /** The SHA-256 (lower case) that is known of a file, from the share or from the catalogue; false when it is not known (nothing is asked of the service) */
    bool GetKnownDigest( const CORE::CString& servicePath, CORE::CString& digest ) const;

    /** Forgets what is known of files, folders and digests (a change of the catalogue, files that were written meanwhile) */
    void ForgetMetadata( void );

    virtual bool IsOffline( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsFileCached( const CORE::CString& servicePath, UInt64 size ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void UnpinFile( const CORE::CString& servicePath ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus FetchWholeFile( const CORE::CString& servicePath, bool pin, const MT::CTAtomic< bool >* cancel, MT::CTAtomic< UInt64 >* done, CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus ReadFileRange( const CORE::CString& servicePath, UInt64 offset, UInt32 length, CORE::CDynamicBuffer& data, CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Starts a transfer (CRemoteFileTransfer) as a task of a thread pool; the future ends with it (its status and message are those of
     *  the transfer). The progress is read from the transfer object. Close() cancels the transfers that run and waits for them
     */
    CORE::CFutureResult StartTransfer( const TRemoteFileTransferPtr& transfer, const CORE::CString& threadPoolName = CORE::CString::Empty );

    virtual void TransferEnded( const TRemoteFileTransferPtr& transfer ) GUCEF_VIRTUAL_OVERRIDE;

    UInt32 GetRunningTransferCount( void ) const;

    /**
     *  Whether this object fetched the whole file itself (FetchWholeFile()) since the last time this was asked: what arrives of such a file is not
     *  what a read that missed asked for. True once per fetch
     */
    bool TakeWholeFetched( const CORE::CString& servicePath );

    /** What the first share (the one that holds the files) says of its remote archive: the counters of the calls, the fetches and the cache */
    bool GetStatistics( VFS::CVfsRemoteStatistics& statistics ) const;

    /** A message in plain words for a status of a failed call about a file ('vfsMessage' is the reason that the VFS gave) */
    static CORE::CString DescribeFailure( const CORE::CString& servicePath, TAssetStoreStatus status, const CORE::CString& vfsMessage );

    /** The status of a transfer that stands for the result of a call of the VFS */
    static TAssetStoreStatus StatusOfResult( CORE::TFileSystemAccessResult result );

    private:

    CRemoteAssetShares( const CRemoteAssetShares& src );
    CRemoteAssetShares& operator=( const CRemoteAssetShares& src );

    struct SMounted
    {
        SShare share;
        DRNUTILS::CRemoteShareMount* mount;
        SMounted( void );
    };
    typedef GUCEF::vector< SMounted > TMountedVector;
    typedef GUCEF::vector< TRemoteFileTransferPtr > TTransferVector;

    void OnCatalogueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnShareStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    CORE::CString CacheIdentityOf( const CORE::CString& share ) const;

    typedef CORE::CTEventHandlerFunctor< CRemoteAssetShares > TEventCallback;

    private:

    CIAssetStoreService& m_service;
    CRemoteFilesSettings m_settings;
    bool m_open;
    TMountedVector m_mounted;
    CORE::CString::StringSet m_wholeFetched;           /**< the service paths of the files that FetchWholeFile() was asked for, until TakeWholeFetched() takes them */
    CORE::CString::StringVector m_locationRoots;       /**< the roots of the online locations that the shares cover, as the service had them at Open() */
    TTransferVector m_transfers;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CREMOTEASSETSHARES_H ? */
