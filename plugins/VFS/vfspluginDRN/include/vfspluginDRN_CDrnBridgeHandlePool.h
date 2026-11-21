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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEIOACCESS_H
#include "gucefDRN_CDRNResourceIOAccess.h"
#define GUCEF_DRN_CDRNRESOURCEIOACCESS_H
#endif /* GUCEF_DRN_CDRNRESOURCEIOACCESS_H ? */

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

class CDrnBridgeFileAccess;
class CDrnBridgeHandlePool;

typedef CORE::CTSharedPtr< CDrnBridgeHandlePool, MT::CMutex > CDrnBridgeHandlePoolPtr;

typedef CORE::CTSharedPtr< CDrnBridgeLastResults, MT::CMutex > CDrnBridgeLastResultsPtr;

/**
 *  The DRN handles of the files that are open through one archive. Every open file has a handle of its own (the facility
 *  supports one reading thread per handle), but the serving side allows only a number of open handles per member (32 by
 *  default, shared with whatever else the member opens), so the archive keeps at most maxOpenHandles of them open:
 *  when a file needs a handle and the pool is full the handle of the least recently used file that is not being read
 *  is closed. A file whose handle was closed opens it again when it is read next, for the revision it was opened at
 *  (the open fails with "changed" when the file is another revision by then), and starts with an empty chunk cache.
 *  When every handle is in use by a read the call waits for one to come free, at most the open timeout, then fails as busy.
 *
 *  The pools of all mounts of one member share what that member allows to be open (CDrnBridgeMemberLink): the limit the
 *  member announced is divided between them, so a pool holds at most its share (and never more than maxOpenHandles). A pool
 *  that finds its share used up (or shrunk, because another mount came) closes its own least recently used idle handles
 *  first, and fails as busy only when every handle it has is in use by a read. A member that announces no limit leaves the
 *  pool to maxOpenHandles.
 *
 *  The pool is shared between the archive and its open files, so that a file that outlives the archive's unloading
 *  (the VFS hands out resources that live on) can still be used safely: after Shutdown() it reads nothing.
 *
 *  Threading: the state of the pool and of the files in it (which file holds which handle, whether it is being read)
 *  is guarded by one lock that is held for pointer work only, never while a handle is opened, read or closed.
 *  A file is read by one thread at a time.
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeHandlePool
{
    public:

    CDrnBridgeHandlePool( const CDrnBridgeConnectionPtr& connection      ,
                          const CORE::CString& share                     ,
                          const CDrnBridgeSettings& settings             ,
                          const CDrnBridgeMetadataCachePtr& cache        ,
                          const CDrnBridgeLastResultsPtr& lastResults    );

    ~CDrnBridgeHandlePool();

    /**
     *  Opens a file of the pool's share for reading, pinning it at the revision it has now. NULL when it cannot be opened, with the
     *  reason in 'result'. The file belongs to the caller (it is deleted through the VFS handle that holds it) and holds on to the pool.
     *  With 'expectedRevision' the file is opened for that revision: the serving side is asked for it and the open fails as "changed" when the file
     *  is another revision now (the file is then pinned at that revision, as a file that is opened again after its handle was closed is).
     *  With 'reportsChange' the listener of the pool (SetChangeListener()) is told when the file turns out to have changed, the one who opened it reads the file itself.
     */
    static CDrnBridgeFileAccess* OpenFile( CDrnBridgeHandlePoolPtr pool, const CORE::CString& path, CDrnBridgeResult& result, const CORE::CString& expectedRevision = CORE::CString::Empty, bool reportsChange = false );

    /** Closes every handle that is not being read and refuses all further use: the archive was unloaded */
    void Shutdown( void );

    bool IsShutdown( void ) const;

    /** Who is told when a file opened with 'reportsChange' changed on the serving side; not owned, NULL for none */
    void SetChangeListener( CIDrnBridgeChangeListener* listener );

    const CORE::CString& GetShare( void ) const;

    /** Handles open (or being opened) now, the most there were at once, and how often one was closed to make room or opened again */
    UInt32 GetOpenHandleCount( void ) const;
    UInt32 GetPeakOpenHandleCount( void ) const;
    UInt64 GetEvictionCount( void ) const;
    UInt64 GetReopenCount( void ) const;

    /** The handles this pool may hold open as of the last time one was needed: maxOpenHandles, or its share of what the member announced when that is lower */
    UInt32 GetHandleLimit( void ) const;

    private:

    friend class CDrnBridgeFileAccess;

    CDrnBridgeHandlePool( const CDrnBridgeHandlePool& src );                /**< not implemented */
    CDrnBridgeHandlePool& operator=( const CDrnBridgeHandlePool& src );     /**< not implemented */

    typedef GUCEF::vector< CDrnBridgeFileAccess* > TAccessVector;
    typedef GUCEF::vector< DRN::CDRNResourceIOAccess* > TInnerVector;

    // for the files of this pool

    void Attach( CDrnBridgeFileAccess& access );

    /** Takes the file out of the pool, with the handle it holds */
    void Detach( CDrnBridgeFileAccess& access );

    /**
     *  Makes sure the file has a handle that is open at its revision and marks it as being read: the access to read with, NULL
     *  with the reason in 'result' when that failed. EndUse() has to follow a successful call.
     */
    DRN::CDRNResourceIOAccess* BeginUse( CDrnBridgeFileAccess& access, CDrnBridgeResult& result );

    void EndUse( CDrnBridgeFileAccess& access );

    /** Closes the handle of the file, if it has one that is not being read */
    void DropHandle( CDrnBridgeFileAccess& access );

    /** A read or an open found the file changed: what is known about it is no longer right */
    void NoteChanged( const CORE::CString& path, const CORE::CString& revision, bool report );

    void RecordResult( const CDrnBridgeResult& result );

    static void DeleteInners( TInnerVector& inners );

    CDrnBridgeConnectionPtr m_connection;
    CORE::CString m_share;
    CDrnBridgeSettings m_settings;
    CDrnBridgeMetadataCachePtr m_cache;
    CDrnBridgeLastResultsPtr m_lastResults;

    mutable MT::CMutex m_lock;
    TAccessVector m_accesses;
    UInt32 m_active;                    /**< handles open or being opened */
    UInt32 m_peak;
    UInt64 m_useCounter;
    UInt64 m_evictions;
    UInt64 m_reopens;
    UInt32 m_limit;                     /**< handles this pool may hold now, see GetHandleLimit() */
    bool m_registered;                  /**< takes part in the division of the member's handle limit */
    bool m_shutdown;
    CIDrnBridgeChangeListener* m_changeListener;
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

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEHANDLEPOOL_H ? */
