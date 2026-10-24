/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

#ifndef GUCEF_DRN_TESTAPP_TESTDRNLOSSYCHANNEL_H
#define GUCEF_DRN_TESTAPP_TESTDRNLOSSYCHANNEL_H

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
 *  How a CDRNLossyChannel mistreats the packets it sends.
 *  Endpoint parameters: loss, dup, reorder (0..1), delay, jitter (ms), and to play a cheater:
 *  warp (the sendTime of every packet multiplied, a clock running at that speed), holdAfter and holdFor (ms: a lag
 *  switch that holds every packet from holdAfter after connecting for holdFor and then lets them go at once),
 *  tamper (0..1: share of protected packets with a byte of the tag changed), downgrade (1: HELLO offers only its
 *  last cipher, as a man in the middle would make it), recordAuth (1: keeps the first protected packet it sends, the
 *  one with AUTH, for all channels) and injectAuth (1: sends that kept packet instead of its own first protected one)
 */
class CDRNLossySettings
{
    public:

    GUCEF::Float64 lossRate;
    GUCEF::Float64 duplicateRate;
    GUCEF::Float64 reorderRate;         /**< share of packets held back for up to 20 ms extra */
    GUCEF::UInt32 delayMs;
    GUCEF::UInt32 jitterMs;
    GUCEF::Float64 clockRate;
    GUCEF::UInt32 holdAfterMs;
    GUCEF::UInt32 holdForMs;
    GUCEF::Float64 tamperRate;
    bool downgrade;
    bool recordAuth;
    bool injectAuth;

    CDRNLossySettings( void );

    void FromParams( const GUCEF::CORE::CValueList& params );

    bool MakesUnreliable( void ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  Test decorator: wraps a channel of another type and drops, delays, duplicates and reorders the packets it sends.
 *  Channel type "lossy-<inner type>", for example lossy-udp://127.0.0.1:4000?loss=0.2&delay=10
 *  When it loses, duplicates or reorders it reports itself as an unreliable datagram channel, so that a link over
 *  it uses its own reliability even over TCP or a web socket.
 */
class CDRNLossyChannel : public GUCEF::DRN::CIDRNChannel
{
    public:

    /** A connecting channel whose inner channel is of the given type */
    explicit CDRNLossyChannel( const GUCEF::CORE::CString& innerType );

    /** An accepted channel wrapping an accepted inner channel */
    CDRNLossyChannel( const GUCEF::DRN::CIDRNChannelPtr& inner, const CDRNLossySettings& settings, const GUCEF::CORE::CString& innerType );

    virtual ~CDRNLossyChannel();

    virtual bool Connect( const GUCEF::DRN::CDRNEndpoint& remote ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendPacket( const void* data, GUCEF::UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const GUCEF::DRN::CDRNChannelCapabilities& GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::DRN::CDRNEndpoint GetRemoteEndpoint( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetChannelType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetPulseGenerator( GUCEF::CORE::PulseGeneratorPtr pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    GUCEF::UInt32 GetDroppedCount( void ) const;
    GUCEF::UInt32 GetDuplicatedCount( void ) const;
    GUCEF::UInt32 GetTamperedCount( void ) const;
    GUCEF::UInt32 GetMaxSentPacketSize( void ) const;

    /** The packet recordAuth kept, empty when none */
    static const GUCEF::CORE::CDynamicBuffer& GetRecordedAuthPacket( void );
    static void ClearRecordedAuthPacket( void );

    protected:

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventid, GUCEF::CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CDelayedPacket
    {
        public:
        GUCEF::UInt64 dueTime;
        GUCEF::CORE::CDynamicBuffer data;
        CDelayedPacket( void ) : dueTime( 0 ), data() {}
    };

    void UpdateCapabilities( void );
    void SendDue( bool all );
    const void* Warp( const void* data, GUCEF::UInt32& size );
    const void* Downgrade( const void* data, GUCEF::UInt32& size );

    GUCEF::CORE::CString m_innerType;
    GUCEF::DRN::CIDRNChannelPtr m_inner;
    CDRNLossySettings m_settings;
    GUCEF::DRN::CDRNChannelCapabilities m_capabilities;
    GUCEF::CORE::PulseGeneratorPtr m_pulseGenerator;
    GUCEF::vector< CDelayedPacket > m_delayed;
    GUCEF::UInt32 m_dropped;
    GUCEF::UInt32 m_duplicated;
    GUCEF::UInt64 m_holdStart;
    GUCEF::UInt64 m_holdEnd;
    GUCEF::CORE::CDynamicBuffer m_warpBuffer;
    GUCEF::CORE::CDynamicBuffer m_tamperBuffer;
    GUCEF::UInt32 m_tampered;
    GUCEF::UInt32 m_maxSentSize;
    bool m_sentProtected;
};

/*-------------------------------------------------------------------------*/

/** Accepts channels of the inner type and wraps them in CDRNLossyChannel with the settings of the listen endpoint */
class CDRNLossyChannelListener : public GUCEF::DRN::CIDRNChannelListener
{
    public:

    explicit CDRNLossyChannelListener( const GUCEF::CORE::CString& innerType );
    virtual ~CDRNLossyChannelListener();

    virtual bool Listen( const GUCEF::DRN::CDRNEndpoint& local ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void StopListening( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsListening( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::DRN::CDRNEndpoint GetListenEndpoint( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetChannelType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetPulseGenerator( GUCEF::CORE::PulseGeneratorPtr pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventid, GUCEF::CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::CORE::CString m_innerType;
    GUCEF::DRN::CIDRNChannelListenerPtr m_inner;
    CDRNLossySettings m_settings;
    GUCEF::DRN::CDRNEndpoint m_listenEndpoint;
    GUCEF::CORE::PulseGeneratorPtr m_pulseGenerator;
};

/*-------------------------------------------------------------------------*/

/** Registers lossy-tcp, lossy-udp and lossy-websocket with the channel and listener factories of DRN */
void RegisterLossyChannelFactories( void );

void UnregisterLossyChannelFactories( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_TESTAPP_TESTDRNLOSSYCHANNEL_H ? */
