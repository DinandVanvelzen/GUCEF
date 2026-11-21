/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNRESOURCEHANDLE_H
#define GUCEF_DRN_CDRNRESOURCEHANDLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEINFO_H
#include "gucefDRN_CDRNResourceInfo.h"
#define GUCEF_DRN_CDRNRESOURCEINFO_H
#endif /* GUCEF_DRN_CDRNRESOURCEINFO_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEHASHER_H
#include "gucefDRN_CDRNResourceHasher.h"
#define GUCEF_DRN_CDRNRESOURCEHASHER_H
#endif /* GUCEF_DRN_CDRNRESOURCEHASHER_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFEROPTIONS_H
#include "gucefDRN_CDRNResourceTransferOptions.h"
#define GUCEF_DRN_CDRNRESOURCETRANSFEROPTIONS_H
#endif /* GUCEF_DRN_CDRNRESOURCETRANSFEROPTIONS_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFERRESULT_H
#include "gucefDRN_CDRNResourceTransferResult.h"
#define GUCEF_DRN_CDRNRESOURCETRANSFERRESULT_H
#endif /* GUCEF_DRN_CDRNRESOURCETRANSFERRESULT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNResourceSessionComponent;
class CDRNNode;

/**
 *  A resource of a member held open: the requesting side of an open handle. The first request opens the resource
 *  (OPEN, INFO: size, modification time, revision, tree root); every read after it is a window on the same handle,
 *  without opening again. Made by CDRNResourceSessionComponent::OpenHandle(); CDRNResourceIOAccess puts a CIOAccess on
 *  it, so code that takes a CIOAccess (a VFS resource, a decoder) reads a remote resource through it.
 *
 *  The handle lives until it is closed (Close(), the last reference going), the link to the member or the session
 *  ends (LINK_LOST), or the serving side refuses it. When the serving side closed it for being idle, the next read
 *  opens it again once, for the same revision (CHANGED when the resource changed meanwhile).
 *
 *  Reads block the calling thread, which must not be a thread that works on the node, nor one whose pulse drives a
 *  node without a thread of its own. Chunks are checked against the hash tree on the calling thread when the options
 *  ask for it. One reading thread at a time per handle; the node's thread delivers into it under its own lock.
 */
class GUCEF_DRN_EXPORT_CPP CDRNResourceHandle
{
    public:

    enum TState
    {
        STATE_OPENING   ,
        STATE_OPEN      ,
        STATE_CLOSED        /**< closed, lost or refused: GetResult() says which */
    };

    ~CDRNResourceHandle();

    /** Waits on this thread until the resource is open or refused; false on time out or failure (see GetResult()) */
    bool WaitOpen( UInt32 timeoutInMs );

    TState GetState( void ) const;
    bool IsOpen( void ) const;
    CDRNResourceTransferResult GetResult( void ) const;

    /** The resource as the serving side resolved it, once open */
    CDRNResourceInfo GetInfo( void ) const;

    const CDRNResourceTransferOptions& GetOptions( void ) const;

    UInt32 GetPeerId( void ) const;
    const CORE::CString& GetProviderName( void ) const;
    const CORE::CString& GetResourceName( void ) const;

    /**
     *  Reads the chunks [firstChunk, firstChunk + count) of the resource, each checked against the hash tree when the
     *  options ask for it: chunks[ i ] holds chunk firstChunk + i. Blocks this thread, at most the idle timeout of the
     *  options per window.
     */
    CDRNResourceTransferResult ReadChunks( UInt64 firstChunk, UInt32 count, GUCEF::vector< CORE::CDynamicBuffer >& chunks );

    /** Reads [offset, offset + length) into 'data' (any offset, at most 2 GiB) through ReadChunks() */
    CDRNResourceTransferResult ReadRange( UInt64 offset, UInt32 length, CORE::CDynamicBuffer& data );

    /** Closes the handle: CLOSE to the serving side; reads that wait end with CLOSED */
    void Close( void );

    /** Windows read through the handle, and how often it was opened (1 unless it was opened again after being idle) */
    UInt32 GetReadCount( void ) const;
    UInt32 GetOpenCount( void ) const;

    private:

    friend class CDRNResourceSessionComponent;

    class CArrived;
    class CWaitCondition;
    friend class CWaitCondition;

    typedef GUCEF::map< UInt64, CArrived* > TArrivedMap;

    CDRNResourceHandle( CDRNResourceSessionComponent& component, UInt32 peerId, UInt32 handleId, const CORE::CString& providerName,
                        const CORE::CString& resourceName, const CDRNResourceTransferOptions& options );
    CDRNResourceHandle( const CDRNResourceHandle& src );               /**< not implemented */
    CDRNResourceHandle& operator=( const CDRNResourceHandle& src );    /**< not implemented */

    // node lock held, from the component
    bool SendOpen( void );
    void OnInfo( TDRNResourceStatus status, const CORE::CString& message, const CDRNResourceInfo& info );
    void OnData( UInt32 readId, UInt64 chunk, const UInt8* path, UInt32 pathLength, const UInt8* data, UInt32 dataSize );
    void OnReadEnd( UInt32 readId, TDRNResourceStatus status, const CORE::CString& message );
    void OnLost( TDRNResourceStatus status, const CORE::CString& message );
    void Detach( void );

    bool WaitFor( bool forOpen, UInt32 timeoutInMs );
    bool IsWaitOver( bool forOpen ) const;
    void ClearArrived( void );
    CDRNResourceTransferResult ReadWindow( UInt64 firstChunk, UInt32 count, GUCEF::vector< CORE::CDynamicBuffer >& chunks, bool& reopened );
    CDRNResourceTransferResult ReadWindowRaw( UInt64 chunk, GUCEF::vector< CORE::CDynamicBuffer >& single, bool& reopened );
    CDRNResourceTransferResult Reopen( void );

    mutable MT::CMutex m_lock;          /**< leaf lock: taken under the node lock, never the other way around */
    MT::CSemaphore m_signal;
    CDRNResourceSessionComponent* m_component;
    CDRNNode* m_node;
    UInt32 m_peerId;
    UInt32 m_handleId;
    CORE::CString m_providerName;
    CORE::CString m_resourceName;
    CDRNResourceTransferOptions m_options;
    TState m_state;
    CDRNResourceTransferResult m_result;
    CDRNResourceInfo m_info;
    bool m_useTree;
    UInt32 m_readId;
    UInt32 m_nextReadId;
    UInt64 m_readFirst;
    UInt32 m_readCount;
    UInt32 m_readOutstanding;
    TDRNResourceStatus m_readStatus;
    CORE::CString m_readMessage;
    TArrivedMap m_arrived;
    UInt32 m_readCountTotal;
    UInt32 m_openCount;
    CDRNResourceHasher m_hasher;        /**< the reading thread's */
};

typedef CORE::CTSharedPtr< CDRNResourceHandle, MT::CMutex > CDRNResourceHandlePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCEHANDLE_H ? */
