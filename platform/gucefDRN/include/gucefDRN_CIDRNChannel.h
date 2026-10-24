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

#ifndef GUCEF_DRN_CIDRNCHANNEL_H
#define GUCEF_DRN_CIDRNCHANNEL_H

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

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINT_H
#include "gucefDRN_CDRNEndpoint.h"
#define GUCEF_DRN_CDRNENDPOINT_H
#endif /* GUCEF_DRN_CDRNENDPOINT_H ? */

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

class CIDRNLatencyMeter;

/*-------------------------------------------------------------------------*/

/**
 *  What a channel's transport offers
 */
class GUCEF_DRN_EXPORT_CPP CDRNChannelCapabilities
{
    public:

    bool isReliable;            /**< the transport itself never loses or reorders packets (TCP, WebSocket) */
    bool isDatagram;            /**< packets travel as datagrams with a size budget */
    UInt32 maxPacketSize;       /**< the budget of a datagram, 0 for streams */
    bool isBrowserCompatible;   /**< a browser can be the other end */
    bool canListen;             /**< channels of this type can be accepted by a listener */

    CDRNChannelCapabilities( void );
};

/*-------------------------------------------------------------------------*/

/** Event data of PacketReceivedEvent: one whole DRN packet, linked to the channel's receive buffer */
typedef CORE::TConstLinkedCloneableBuffer   CDRNPacketEventData;

/** Event data of ChannelErrorEvent: a description */
typedef CORE::TCloneableString              CDRNChannelErrorEventData;

/*-------------------------------------------------------------------------*/

/**
 *  A channel moves DRN packets between two endpoints. It adds nothing to them and interprets nothing:
 *  framing for stream transports is the only thing it does.
 *
 *  Channels run on the pulse generator they are given (SetPulseGenerator() before Connect()), events are sent from
 *  that thread. A channel is not deleted from within its own notifications.
 */
class GUCEF_DRN_EXPORT_CPP CIDRNChannel : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ChannelConnectedEvent;
    static const CORE::CEvent ChannelDisconnectedEvent;
    static const CORE::CEvent PacketReceivedEvent;         /**< CDRNPacketEventData */
    static const CORE::CEvent ChannelErrorEvent;           /**< CDRNChannelErrorEventData */

    static void RegisterEvents( void );

    CIDRNChannel( void );

    virtual ~CIDRNChannel();

    /** Starts connecting, ChannelConnectedEvent follows (possibly from within this call) */
    virtual bool Connect( const CDRNEndpoint& remote ) = 0;

    virtual void Close( void ) = 0;

    virtual bool IsConnected( void ) const = 0;

    /** Sends one whole DRN packet */
    virtual bool SendPacket( const void* data, UInt32 size ) = 0;

    virtual const CDRNChannelCapabilities& GetCapabilities( void ) const = 0;

    virtual CDRNEndpoint GetRemoteEndpoint( void ) const = 0;

    virtual CORE::CString GetChannelType( void ) const = 0;

    /** A channel specific latency meter, or NULL to use the protocol's own */
    virtual CIDRNLatencyMeter* GetLatencyMeter( void );

    /**
     *  The largest packet the channel takes from the peer from now on, for channels that have to collect a packet
     *  before they can hand it over (streams). The link keeps it small until the peer authenticated.
     *  The default does nothing: datagram channels are bounded by their datagrams.
     */
    virtual void SetMaxReceivePacketSize( UInt32 maxPacketSize );

    /** The pulse generator to run on, to be set before Connect() */
    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator ) = 0;

    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const = 0;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CIDRNChannel( const CIDRNChannel& src );
    CIDRNChannel& operator=( const CIDRNChannel& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CIDRNChannel, MT::CMutex >                       CIDRNChannelPtr;
typedef CORE::CTAbstractFactory< CORE::CString, CIDRNChannel, MT::CMutex >  CDRNChannelFactory;
typedef CORE::CTCloneableObj< CIDRNChannelPtr >                             CDRNChannelEventData;

/*-------------------------------------------------------------------------*/

/**
 *  Channels run on the thread of their pulse generator and must be destroyed there: their sockets are pulsed by it.
 *  An object holding a channel that may be let go of on another thread (a link whose last reference is in the hands
 *  of the application) gives the channel to the reaper of its node, which destroys it on the node's thread.
 */
class GUCEF_DRN_EXPORT_CPP CDRNChannelReaper : public CORE::CTSharedObjCreator< CDRNChannelReaper, MT::CMutex >
{
    public:

    CDRNChannelReaper( void );

    virtual ~CDRNChannelReaper();

    /** The thread the channels belong to, 0 when any thread may destroy them */
    void SetOwnerThreadId( UInt32 threadId );

    /** Lets go of the channel: right away on the owner thread, otherwise at the next Reap() */
    void Release( CIDRNChannelPtr& channel );

    /** Destroys the channels let go of on other threads, called on the owner thread */
    void Reap( void );

    private:

    CDRNChannelReaper( const CDRNChannelReaper& src );
    CDRNChannelReaper& operator=( const CDRNChannelReaper& src );

    typedef GUCEF::vector< CIDRNChannelPtr > TChannelVector;

    MT::CMutex m_lock;
    UInt32 m_ownerThreadId;
    TChannelVector m_released;
};

typedef CORE::CTSharedPtr< CDRNChannelReaper, MT::CMutex >                  CDRNChannelReaperPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CIDRNCHANNEL_H ? */
