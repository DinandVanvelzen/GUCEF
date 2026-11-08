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

#ifndef GUCEF_COMCORE_CRPCLOOPBACKTRANSPORTLISTENER_H
#define GUCEF_COMCORE_CRPCLOOPBACKTRANSPORTLISTENER_H

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

#ifndef GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#include "gucefCOMCORE_CIRpcTransportListener.h"
#define GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H ? */

#ifndef GUCEF_COMCORE_CRPCPROTOCOLTASK_H
#include "gucefCOMCORE_CRpcProtocolTask.h"
#define GUCEF_COMCORE_CRPCPROTOCOLTASK_H
#endif /* GUCEF_COMCORE_CRPCPROTOCOLTASK_H ? */

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

class CRpcLoopbackTransport;

/*-------------------------------------------------------------------------*/

/**
 *  Listens on "loopback://<name>" in a process-wide table (gucefCOMCORE RPC design 12.1). A Connect() to the name
 *  makes a connected pair; the accepted side runs on the listener's pulse generator and is announced with
 *  TransportAcceptedEvent on that thread, before it delivers any frame.
 *
 *  The notifier has a lock of its own (Lock()): a host subscribes on the thread that adds the listener while the
 *  listener's thread raises TransportAcceptedEvent, which it does holding this lock. Lock order: the pulse
 *  generator's notification lock, this lock, the locks its observers take; m_lock is a leaf.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcLoopbackTransportListener : public CIRpcTransportListener
{
    public:

    CRpcLoopbackTransportListener( void );
    virtual ~CRpcLoopbackTransportListener() GUCEF_VIRTUAL_OVERRIDE;

    void SetPrincipalRoles( const CORE::CString::StringSet& roles );
    void SetMaxFrameSize( UInt32 maxFrameSize );

    virtual const CORE::CString& GetTransportType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Listen( const CORE::CUri& local ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void StopListening( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsListening( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CUri GetListenUri( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The notifier's lock, see the class description */
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Used by CRpcLoopbackTransport::Connect(): makes the accepted side and announces it */
    static bool ConnectTo( const CORE::CString& name, CRpcLoopbackTransport& connecting );

    private:

    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    bool Accept( CRpcLoopbackTransport& connecting );

    CRpcLoopbackTransportListener( const CRpcLoopbackTransportListener& src );                  /**< not implemented */
    CRpcLoopbackTransportListener& operator=( const CRpcLoopbackTransportListener& src );       /**< not implemented */

    mutable MT::CMutex m_notifierLock;     /**< see Lock() */
    mutable MT::CMutex m_lock;
    CORE::CString m_name;
    bool m_listening;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CRpcProtocolTaskPtr m_task;
    CORE::CObserver m_pulseObserver;
    GUCEF::vector< CIRpcTransportPtr > m_pendingAccepts;
    CORE::CString::StringSet m_roles;
    UInt32 m_maxFrameSize;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCLOOPBACKTRANSPORTLISTENER_H ? */
