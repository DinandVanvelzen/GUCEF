/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTMEMORYCHANNEL_H
#define GUCEF_MULTIPLAY_TESTAPP_TESTMEMORYCHANNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Test channel: a datagram channel between two nodes of the same process, mem://127.0.0.1:<port>.
 *  A sent packet is queued at the other end and delivered on that end's next pulse. The packet buffers are kept and
 *  reused, so the channel itself does not allocate once it has its buffers: what the allocation tests measure is DRN
 *  and the GUCEF event system, not a socket layer.
 */
class CDRNMemoryChannel : public GUCEF::DRN::CIDRNChannel
{
    public:

    CDRNMemoryChannel( void );
    virtual ~CDRNMemoryChannel();

    virtual bool Connect( const GUCEF::DRN::CDRNEndpoint& remote ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendPacket( const void* data, GUCEF::UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const GUCEF::DRN::CDRNChannelCapabilities& GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::DRN::CDRNEndpoint GetRemoteEndpoint( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetChannelType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetPulseGenerator( GUCEF::CORE::PulseGeneratorPtr pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Called by the listener: the accepting end of a connection */
    void Accept( CDRNMemoryChannel* peer, const GUCEF::DRN::CDRNEndpoint& remote );

    protected:

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventid, GUCEF::CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::vector< GUCEF::CORE::CDynamicBuffer* > TBufferVector;

    void Enqueue( const void* data, GUCEF::UInt32 size );
    void Deliver( void );
    void Unlink( void );

    GUCEF::DRN::CDRNChannelCapabilities m_capabilities;
    GUCEF::CORE::PulseGeneratorPtr m_pulseGenerator;
    GUCEF::DRN::CDRNEndpoint m_remote;
    CDRNMemoryChannel* m_peer;
    bool m_connected;
    bool m_announceConnected;
    bool m_announceDisconnected;
    GUCEF::MT::CMutex m_queueLock;
    TBufferVector m_queue;
    GUCEF::UInt32 m_queueHead;
    GUCEF::UInt32 m_queueCount;
    GUCEF::CORE::CDynamicBuffer m_delivering;
    GUCEF::CORE::CDynamicBuffer m_rxView;
    GUCEF::DRN::CDRNPacketEventData m_rxEventData;
};

/*-------------------------------------------------------------------------*/

/** Accepts mem channels on a port number */
class CDRNMemoryChannelListener : public GUCEF::DRN::CIDRNChannelListener
{
    public:

    CDRNMemoryChannelListener( void );
    virtual ~CDRNMemoryChannelListener();

    virtual bool Listen( const GUCEF::DRN::CDRNEndpoint& local ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void StopListening( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsListening( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::DRN::CDRNEndpoint GetListenEndpoint( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetChannelType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetPulseGenerator( GUCEF::CORE::PulseGeneratorPtr pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** A connecting channel arrives: makes its accepting end, NULL when not listening */
    CDRNMemoryChannel* AcceptFrom( CDRNMemoryChannel* connecting );

    protected:

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventid, GUCEF::CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::DRN::CDRNEndpoint m_listenEndpoint;
    GUCEF::CORE::PulseGeneratorPtr m_pulseGenerator;
    GUCEF::vector< GUCEF::DRN::CIDRNChannelPtr > m_pendingAccepts;
    bool m_listening;
};

/*-------------------------------------------------------------------------*/

/** Registers the "mem" channel type with the channel and listener factories of DRN */
void RegisterMemoryChannelFactories( void );
void UnregisterMemoryChannelFactories( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTMEMORYCHANNEL_H ? */
