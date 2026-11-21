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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEBACKEND_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTWEAKPTR_H
#include "gucefCORE_CTWeakPtr.h"
#define GUCEF_CORE_CTWEAKPTR_H
#endif /* GUCEF_CORE_CTWEAKPTR_H ? */

#ifndef GUCEF_VFS_CIVFSREMOTEBACKEND_H
#include "gucefVFS_CIVfsRemoteBackend.h"
#define GUCEF_VFS_CIVFSREMOTEBACKEND_H
#endif /* GUCEF_VFS_CIVFSREMOTEBACKEND_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEARCHIVECORE_H
#include "gucefVFS_CVfsRemoteArchiveCore.h"
#define GUCEF_VFS_CVFSREMOTEARCHIVECORE_H
#endif /* GUCEF_VFS_CVFSREMOTEARCHIVECORE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H
#include "vfspluginDRN_CDrnBridgeSettings.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGECONNECTION_H
#include "vfspluginDRN_CDrnBridgeConnection.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGECONNECTION_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGECONNECTION_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMETADATACACHE_H
#include "vfspluginDRN_CDrnBridgeMetadataCache.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMETADATACACHE_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMETADATACACHE_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H
#include "vfspluginDRN_CDrnBridgeHandlePool.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CIDRNBRIDGECHANGELISTENER_H
#include "vfspluginDRN_CIDrnBridgeChangeListener.h"
#define GUCEF_VFSPLUGIN_DRN_CIDRNBRIDGECHANGELISTENER_H
#endif /* GUCEF_VFSPLUGIN_DRN_CIDRNBRIDGECHANGELISTENER_H ? */

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
 *  The share of a member of a DRN session as the place that the generic remote archive layer works on (VFS::CIVfsRemoteBackend): what
 *  the share says of a path (a stat), of a directory (a listing) and the bytes of a file at a revision (CDrnBridgeBlockSource).
 *
 *  The calls are the ones the bridge always made: a stat takes the metadata cache of the plugin first (a fresh record, a record of the
 *  parent listing, else one stat that carries the revision the cache has so that an unchanged answer costs no more than that), a listing
 *  is checked with the revision it has, files are read through the handles of the pool of the archive. They run on the calling thread and
 *  block; which thread may block, what a miss does for a thread that must not wait, what is served while the link is down and what
 *  is kept on disk is the generic layer's affair and is not done here.
 *
 *  The backend records the reason of every call that did not go well in the table of last results of the archive (the form of
 *  CDrnBridgeResult), and tells the generic layer when the link to the member comes or goes (the events of the session and of its node), so
 *  that ConnectionStateChangedEvent is raised once per change.
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeBackend : public VFS::CIVfsRemoteBackend ,
                                                        public CORE::CObserver ,
                                                        public CIDrnBridgeChangeListener
{
    public:

    /**
     *  @param connection   the member of the session that serves the share
     *  @param share        the name of the share
     *  @param settings     the settings of the archive (its handle pool and its metadata cache use them)
     *  @param lastResults  where the reasons of failed calls are recorded, per calling thread
     */
    CDrnBridgeBackend( const CDrnBridgeConnectionPtr& connection   ,
                       const CORE::CString& share                  ,
                       const CDrnBridgeSettings& settings          ,
                       const CDrnBridgeLastResultsPtr& lastResults );

    virtual ~CDrnBridgeBackend() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The layer that is told when the link comes or goes. Not owned: the backend lets go of it in Shutdown(), which the archive calls before it lets go
     *  of the layer, and the events of the link are never answered by holding the layer (a thread that is told of a link must not be the one that ends it).
     */
    void SetLinkListener( VFS::CVfsRemoteArchiveCore* core );

    /** Ends the use of the share: the handles of the pool are closed, what the metadata cache holds is dropped and no event is passed on any more */
    void Shutdown( void );

    // CIVfsRemoteBackend

    virtual bool IsLinkUp( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetPlaceDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual VFS::CVfsCallResult Stat( const CORE::CString& path, VFS::CVfsRemoteEntry& entry ) GUCEF_VIRTUAL_OVERRIDE;
    virtual VFS::CVfsCallResult List( const CORE::CString& dirPath, VFS::CVfsRemoteEntry::TVector& entries, bool& dirExists ) GUCEF_VIRTUAL_OVERRIDE;
    virtual VFS::CIVfsBlockSourcePtr OpenBlockSource( const CORE::CString& path, const CORE::CString& revision, VFS::CVfsCallResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetBackendStatistics( VFS::CVfsRemoteStatistics& statistics ) const GUCEF_VIRTUAL_OVERRIDE;

    /** What the metadata cache of the plugin holds of a path now, fresh or not; false when it holds nothing (flags of the share that the generic layer does not carry, like "hidden") */
    bool PeekEntry( const CORE::CString& path, DRN::CDRNNamespaceEntry& entry ) const;

    /**
     *  The SHA-256 that the share knows of a file, as the entry of the share has it: from the metadata cache when it holds one, else asked of the share when
     *  'mayAsk' (a thread that must not wait does not ask). False, with the reason recorded, when the share has none or the file is not there.
     */
    bool FindShareSha256( const CORE::CString& normalizedPath, bool mayAsk, UInt8* digest ) const;

    /** Notes what a call made of the share's state: an answer that says the share cannot be used as a share is remembered, a success forgets it */
    void NoteOutcome( const CDrnBridgeResult& result ) const;

    /** A file opened by the archive itself changed on the serving side (CIDrnBridgeChangeListener): the generic layer drops what it knows of it and tells its observers */
    virtual void OnFileChanged( const CORE::CString& path, const CORE::CString& revision ) GUCEF_VIRTUAL_OVERRIDE;

    /** Forgets the metadata cache of the plugin: all of it, or what is known of one path (the path, its listing and the listing of its directory) */
    void InvalidateMetadata( void );
    void InvalidateMetadata( const CORE::CString& path );

    const CDrnBridgeConnectionPtr& GetConnection( void ) const;
    const CDrnBridgeMetadataCachePtr& GetMetadataCache( void ) const;
    const CDrnBridgeHandlePoolPtr& GetPool( void ) const;
    const CORE::CString& GetShare( void ) const;

    /** Runs of bytes asked of the share and the bytes that came, since the backend was made */
    UInt64 GetFetchCallCount( void ) const;
    UInt64 GetBytesFetched( void ) const;

    /** Whether the problems of the share are those of a share that cannot be used as one (an integrity, unsupported or no such share answer came last) */
    bool HasProtocolProblem( void ) const;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDrnBridgeBackend( const CDrnBridgeBackend& src );                  /**< not implemented */
    CDrnBridgeBackend& operator=( const CDrnBridgeBackend& src );       /**< not implemented */

    /** What the share says about a path: from the cache or from a stat; 'exists' is the answer, false also when it could not be found out ('result' says which) */
    bool LookupEntry( const CORE::CString& normalizedPath, DRN::CDRNNamespaceEntry& entry, CDrnBridgeResult& result ) const;

    /** The entries of a directory, from the cache or by listing it; NULL when it cannot be listed ('result' says why) */
    CDrnBridgeMetadataCache::TEntryVectorPtr GetListing( const CORE::CString& normalizedDir, CDrnBridgeResult& result, bool& fromTheShare ) const;

    bool FetchListing( const CORE::CString& normalizedDir, DRN::CDRNNamespaceClient::TEntryVector& entries, CORE::CString& dirRevision, bool& complete, CDrnBridgeResult& result ) const;

    /** Follows the session the connection has now: the events of its node and of the session tell when the link comes or goes */
    void EnsureWatching( void ) const;

    /** Tells the layer where the state of the link is now */
    void PassOnLinkState( void ) const;

    /** The layer is told 'up'; when the link went down and the settings do not want expired metadata served while it is, the layer and the plugin forget what they hold in memory */
    void TellLayer( bool up ) const;

    static void ToRemoteEntry( const CORE::CString& name, const DRN::CDRNNamespaceEntry& source, bool attributesKnown, VFS::CVfsRemoteEntry& entry );

    mutable CDrnBridgeConnectionPtr m_connection;         /**< the objects behind the backend are shared and thread safe, the calls on them are not changes of the backend */
    CORE::CString m_share;
    CDrnBridgeSettings m_settings;
    mutable CDrnBridgeMetadataCachePtr m_metadata;
    mutable CDrnBridgeHandlePoolPtr m_pool;
    mutable CDrnBridgeLastResultsPtr m_lastResults;

    mutable MT::CMutex m_watchLock;
    mutable DRN::CDRNSession* m_watched;
    VFS::CVfsRemoteArchiveCore* m_listener;
    MT::CTAtomic< bool > m_shutdown;
    mutable MT::CTAtomic< bool > m_wasUp;

    mutable MT::CTAtomic< UInt32 > m_protocolProblem;
    mutable MT::CTAtomic< UInt64 > m_fetchCalls;
    mutable MT::CTAtomic< UInt64 > m_bytesFetched;
    UInt64 m_statBaseline;
    UInt64 m_listBaseline;
    UInt64 m_openBaseline;
    UInt64 m_busyBaseline;
};

typedef CORE::CTSharedPtr< CDrnBridgeBackend, MT::CMutex > CDrnBridgeBackendPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEBACKEND_H ? */
