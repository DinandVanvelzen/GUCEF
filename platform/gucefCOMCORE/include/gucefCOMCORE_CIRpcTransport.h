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

#ifndef GUCEF_COMCORE_CIRPCTRANSPORT_H
#define GUCEF_COMCORE_CIRPCTRANSPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_COMCORE_RPCTYPES_H
#include "gucefCOMCORE_RpcTypes.h"
#define GUCEF_COMCORE_RPCTYPES_H
#endif /* GUCEF_COMCORE_RPCTYPES_H ? */

#ifndef GUCEF_COMCORE_CRPCTRANSPORTPROPERTIES_H
#include "gucefCOMCORE_CRpcTransportProperties.h"
#define GUCEF_COMCORE_CRPCTRANSPORTPROPERTIES_H
#endif /* GUCEF_COMCORE_CRPCTRANSPORTPROPERTIES_H ? */

#ifndef GUCEF_COMCORE_CRPCPRINCIPAL_H
#include "gucefCOMCORE_CRpcPrincipal.h"
#define GUCEF_COMCORE_CRPCPRINCIPAL_H
#endif /* GUCEF_COMCORE_CRPCPRINCIPAL_H ? */

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

#ifndef GUCEF_COMCORE_CRPCFRAMEEVENTDATA_H
#include "gucefCOMCORE_CRpcFrameEventData.h"
#define GUCEF_COMCORE_CRPCFRAMEEVENTDATA_H
#endif /* GUCEF_COMCORE_CRPCFRAMEEVENTDATA_H ? */

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
 *  Moves whole RPC frames between two parties (gucefCOMCORE RPC design 10.2). Implementations: "loopback"
 *  (gucefCOMCORE), "tcp" (gucefCOM), "drn" (gucefDRN); more register into CComCoreGlobal::GetRpcTransportFactory().
 *
 *  Threading contract, which every implementation keeps:
 *      - Events are raised on the thread of GetPulseGenerator() (the protocol thread), synchronously, in order.
 *        FrameReceivedEvent carries CRpcFrameEventData that links to the transport's receive buffer: valid during
 *        the notification only. The consumer (the endpoint) handles the frame in that call.
 *      - SendFrame() may be called from any thread. It takes only a leaf lock of the transport and never calls
 *        out (no event, no callback) from inside, so a caller holding its own lock cannot deadlock with a
 *        delivering protocol thread.
 *      - A transport that needs a thread of its own obtains it through the gucefCORE task manager (a dedicated
 *        task, CThreadPool::SetupSingularTask() / StartTask(), using the delegator's pulse generator); no raw threads.
 *      - The notifier has a lock of its own: Lock() and Unlock() are overridden with a mutex of the transport. An
 *        endpoint subscribes to the transport on the thread that starts it (Start(), Rebind()) while the protocol
 *        thread raises events; CNotifier takes that lock for both, so a subscription waits for an event that is
 *        being delivered instead of changing the observers under it. Lock order: the protocol pulse generator's
 *        notification lock (and a lock its environment holds while it pulses, as a drn node's), this lock, the
 *        endpoint lock, the transport's leaf locks. Subscriptions of the transport itself (to its pulse generator)
 *        go through a member observer, so they never hold this lock.
 *      - Whoever lets go of a transport inside one of its events keeps it until a later pulse of its thread: a
 *        notifier outlives its own notification (CRpcHost does so for the accepted transports it refuses).
 */
class GUCEF_COMCORE_EXPORT_CPP CIRpcTransport : public CORE::CObservingNotifier ,
                                                public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent ConnectedEvent;
    static const CORE::CEvent DisconnectedEvent;    /**< CRpcStatus as event data: why */
    static const CORE::CEvent FrameReceivedEvent;   /**< CRpcFrameEventData: the frame (linked buffer) and its protection class */
    static const CORE::CEvent WritableEvent;        /**< SendFrame() would accept again */
    static const CORE::CEvent TransportErrorEvent;  /**< TCloneableString: a description */

    static void RegisterEvents( void );

    CIRpcTransport( void );
    virtual ~CIRpcTransport() GUCEF_VIRTUAL_OVERRIDE;

    /** "loopback", "tcp", "drn", ... the name the type is registered with in the factory */
    virtual const CORE::CString& GetTransportType( void ) const = 0;

    /** Connecting side; accepted transports are connected already. ConnectedEvent follows (it may follow at once) */
    virtual bool Connect( const CORE::CUri& remote ) = 0;

    /** Closes the transport; DisconnectedEvent follows on the protocol thread unless it was closed already */
    virtual void Close( void ) = 0;

    virtual bool IsConnected( void ) const = 0;

    /**
     *  Sends one whole frame. False when closed or when the transport cannot take more now (WritableEvent follows).
     *  The protection class is honoured by transports with hasFrameProtectionClasses; others send everything at
     *  their own level.
     */
    virtual bool SendFrame( const void* frame, UInt32 size, TRpcProtectionClass protection ) = 0;

    virtual const CRpcTransportProperties& GetProperties( void ) const = 0;

    /** Set by the transport, never from a frame */
    virtual const CRpcPrincipal& GetPeerPrincipal( void ) const = 0;

    /** The protocol thread: frames are received and events raised on it */
    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const = 0;

    virtual CORE::CUri GetRemoteUri( void ) const = 0;

    /**
     *  Before Connect(): the pulse generator the transport should run on. A transport made by the factory without
     *  one runs on a dedicated task of the task manager. Transports whose thread is given by their own environment
     *  (drn: the node) ignore it and return false; GetPulseGenerator() gives that thread once Connect() succeeded,
     *  which is what CRpcEndpoint::Connect( uri ) then runs on.
     */
    virtual bool SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator );

    /**
     *  The endpoint refused a frame of this transport for its protection class: a frame that must be CONFIDENTIAL
     *  arrived as PUBLIC (design 14.3). The endpoint ends the session with ABORT PERMISSION_DENIED; this tells the
     *  transport, for its own anomaly reporting. Called on the protocol thread, outside the endpoint lock, once per
     *  refusal. Default: nothing.
     */
    virtual void OnProtectionDowngradeRefused( TRpcProtectionClass required, TRpcProtectionClass arrivedWith );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CIRpcTransport( const CIRpcTransport& src );                    /**< not implemented */
    CIRpcTransport& operator=( const CIRpcTransport& src );         /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CIRpcTransport, MT::CMutex >    CRpcTransportFactory;
typedef CRpcTransportFactory::TProductPtr                                        CIRpcTransportPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIRPCTRANSPORT_H ? */
