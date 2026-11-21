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

#ifndef GUCEF_DRN_CDRNNAMESPACEREQUEST_H
#define GUCEF_DRN_CDRNNAMESPACEREQUEST_H

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

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

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

/**
 *  A namespace request of this side that waits for its answer (CDRNResourceSessionComponent::CallNamespace()): the node's
 *  thread completes it when the NS_ANSWER arrives, or fails it when the link or session ends; the calling thread waits on
 *  it. Shared between the two, so a request that timed out leaves a late answer harmless. The lock is a leaf lock.
 */
class GUCEF_DRN_EXPORT_CPP CDRNNamespaceRequest
{
    public:

    CDRNNamespaceRequest( UInt32 peerId, UInt32 requestId, UInt32 kind );

    UInt32 GetPeerId( void ) const;
    UInt32 GetRequestId( void ) const;
    UInt32 GetKind( void ) const;

    /** From the node's thread: the answer arrived; 'body' is what follows the answer header */
    void Complete( TDRNResourceStatus status, const CORE::CString& message, const void* body, UInt32 bodySize );

    /** From the node's thread: no answer will come (the link or the session ended, the request was given up) */
    void Fail( TDRNResourceStatus status, const CORE::CString& message );

    bool IsDone( void ) const;

    /** On the calling thread; false when it is not done in time */
    bool Wait( UInt32 timeoutInMs );

    /** Once done: how it ended; the status of the answer, or the one of the failure */
    TDRNResourceStatus GetStatus( void ) const;
    CORE::CString GetMessage( void ) const;

    /** Once done and OK: the body of the answer, owned by the request */
    const CORE::CDynamicBuffer& GetBody( void ) const;

    private:

    CDRNNamespaceRequest( const CDRNNamespaceRequest& src );                /**< not implemented */
    CDRNNamespaceRequest& operator=( const CDRNNamespaceRequest& src );     /**< not implemented */

    mutable MT::CMutex m_lock;
    MT::CSemaphore m_signal;
    UInt32 m_peerId;
    UInt32 m_requestId;
    UInt32 m_kind;
    bool m_done;
    TDRNResourceStatus m_status;
    CORE::CString m_message;
    CORE::CDynamicBuffer m_body;
};

typedef CORE::CTSharedPtr< CDRNNamespaceRequest, MT::CMutex > CDRNNamespaceRequestPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNNAMESPACEREQUEST_H ? */
