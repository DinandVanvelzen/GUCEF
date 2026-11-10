/*
 *  drnchannelpluginEOSP2P: DRN channel plugin that moves DRN packets over the Epic Online Services P2P interface
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

#ifndef GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PSOCKETEVENTQUEUE_H
#define GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PSOCKETEVENTQUEUE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSP2PSOCKET_H
#include "pluginglueEOS_CEosP2PSocket.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSP2PSOCKET_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSP2PSOCKET_H ? */

#ifndef GUCEF_DRNPLUGIN_EOSP2P_MACROS_H
#include "drnchannelpluginEOSP2P_macros.h"
#define GUCEF_DRNPLUGIN_EOSP2P_MACROS_H
#endif /* GUCEF_DRNPLUGIN_EOSP2P_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNPLUGIN {
namespace EOSP2P {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The events of an EOS P2P socket on their way from the tick of the platform to the pulse generator a channel or a
 *  listener runs on. The platform is ticked by the pulse generator of the application, outside the lock of a DRN node;
 *  the channel and the listener belong to the node and take the events in a pulse of the node's pulse generator, which
 *  the node pulses under its lock (gucefDRN design 9.1).
 *
 *  Any thread adds, one thread takes. The lock of the queue is a leaf: nothing is called while it is held. A packet is
 *  copied into an entry that is kept for reuse, so once the queue holds as many entries as one tick brings, nothing is
 *  allocated per packet.
 */
class GUCEF_DRNPLUGIN_EOSP2P_EXPORT_CPP CEosP2PSocketEventQueue
{
    public:

    enum EEventType
    {
        EVENTTYPE_PACKET = 0,
        EVENTTYPE_CONNECTION_ESTABLISHED,
        EVENTTYPE_CONNECTION_CLOSED
    };
    typedef enum EEventType TEventType;

    /** One event: the packet for EVENTTYPE_PACKET, the connection data for the others */
    class GUCEF_DRNPLUGIN_EOSP2P_EXPORT_CPP CQueuedEvent
    {
        public:

        TEventType type;
        PLUGINGLUE::EOS::CEosP2PPacketEventData packet;
        PLUGINGLUE::EOS::CEosP2PConnectionEventData connection;

        CQueuedEvent( void );
    };
    typedef GUCEF::vector< CQueuedEvent* > TQueuedEventVector;

    CEosP2PSocketEventQueue( void );

    ~CEosP2PSocketEventQueue();

    /** Keeps a copy of the packet; true when the queue was empty before, the moment to ask for a pulse */
    bool AddPacket( const PLUGINGLUE::EOS::CEosP2PPacketEventData& packet );

    /** Keeps a copy of the event of a connection; true when the queue was empty before */
    bool AddConnectionEvent( TEventType type, const PLUGINGLUE::EOS::CEosP2PConnectionEventData& data );

    /**
     *  The events that waited, oldest first: the first count entries of the vector returned. For the one thread that
     *  takes the events; the entries stay valid until its next call
     */
    const TQueuedEventVector& TakeAll( UInt32& count );

    /** Forgets the events that wait */
    void Clear( void );

    private:

    CEosP2PSocketEventQueue( const CEosP2PSocketEventQueue& src );
    CEosP2PSocketEventQueue& operator=( const CEosP2PSocketEventQueue& src );

    CQueuedEvent* NextEntry( void );

    private:

    MT::CMutex m_lock;
    TQueuedEventVector m_waiting;           /**< the first m_waitingCount entries wait, the others are kept for reuse */
    UInt32 m_waitingCount;
    TQueuedEventVector m_taken;             /**< what the last TakeAll() handed out */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOSP2P */
}; /* namespace DRNPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PSOCKETEVENTQUEUE_H ? */
