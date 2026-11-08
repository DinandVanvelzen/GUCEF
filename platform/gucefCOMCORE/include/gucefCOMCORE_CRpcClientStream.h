/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCCLIENTSTREAM_H
#define GUCEF_COMCORE_CRPCCLIENTSTREAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_COMCORE_CIRPCSTREAM_H
#include "gucefCOMCORE_CIRpcStream.h"
#define GUCEF_COMCORE_CIRPCSTREAM_H
#endif /* GUCEF_COMCORE_CIRPCSTREAM_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINTLINK_H
#include "gucefCOMCORE_CRpcEndpointLink.h"
#define GUCEF_COMCORE_CRPCENDPOINTLINK_H
#endif /* GUCEF_COMCORE_CRPCENDPOINTLINK_H ? */

#ifndef GUCEF_COMCORE_CRPCMETHODREF_H
#include "gucefCOMCORE_CRpcMethodRef.h"
#define GUCEF_COMCORE_CRPCMETHODREF_H
#endif /* GUCEF_COMCORE_CRPCMETHODREF_H ? */

#ifndef GUCEF_COMCORE_RPCEVENTDATA_H
#include "gucefCOMCORE_RpcEventData.h"
#define GUCEF_COMCORE_RPCEVENTDATA_H
#endif /* GUCEF_COMCORE_RPCEVENTDATA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The implementation of CIRpcStream made by CRpcEndpoint::OpenStream(). Messages from the provider are queued for
 *  ReadNext() and raised as MessageEvent; credit is granted as messages are consumed (by an event delivered or a
 *  ReadNext()), or only by GrantCredit() in manual mode.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcClientStream : public CIRpcStream
{
    public:

    virtual ~CRpcClientStream() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Write( const CRpcMessage& message ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool WritesDone( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Cancel( const CRpcStatus& status ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ReadNext( CRpcMessage& message, CRpcStatus& status, UInt32 timeoutMs ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetManualCredit( bool manual ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void GrantCredit( UInt32 messages ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsDone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CRpcStatus GetStatus( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CRpcMessage GetResponse( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetCallRef( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CRpcEndpoint;

    /** Subscribes to itself: made outside the endpoint lock, as subscribing takes the lock this stream's mail is delivered under */
    CRpcClientStream( const CORE::PulseGeneratorPtr& pulseGenerator, const CRpcEndpointLinkPtr& link, const CRpcMethodRef& ref );

    /**
     *  From the endpoint, under its lock: the stream's state changes at once, the events follow once the endpoint lock
     *  is released (RaiseStreamEvent()), as events are never raised under it (design 10.9)
     */
    void RecordMessage( const CRpcMessage& message );
    bool RecordEnd( const CRpcStatus& status, const CRpcMessage* response );   /**< false when the stream had ended already */
    void RaiseStreamEvent( const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void OnInternalMessage( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void Consumed( UInt32 count );

    static const CORE::CEvent InternalMessageEvent;

    mutable MT::CMutex m_lock;
    CRpcEndpointLinkPtr m_link;
    CRpcMethodRef m_ref;
    UInt64 m_callRef;
    UInt32 m_sessionGeneration;
    GUCEF::vector< CRpcMessage > m_queue;
    bool m_readMode;
    bool m_done;
    CRpcStatus m_status;
    CRpcMessage m_response;
    bool m_manualCredit;
    CORE::CObserver m_selfObserver;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCCLIENTSTREAM_H ? */
