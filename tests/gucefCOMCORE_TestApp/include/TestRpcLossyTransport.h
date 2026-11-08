/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the COMCORE module
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

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCLOSSYTRANSPORT_H
#define GUCEF_COMCORE_TESTAPP_TESTRPCLOSSYTRANSPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_COMCORE_CIRPCTRANSPORT_H
#include "gucefCOMCORE_CIRpcTransport.h"
#define GUCEF_COMCORE_CIRPCTRANSPORT_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORT_H ? */

#ifndef GUCEF_COMCORE_RPCPROTOCOL_H
#include "gucefCOMCORE_RpcProtocol.h"
#define GUCEF_COMCORE_RPCPROTOCOL_H
#endif /* GUCEF_COMCORE_RPCPROTOCOL_H ? */

#ifndef GUCEF_COMCORE_CRPCFRAMECODEC_H
#include "gucefCOMCORE_CRpcFrameCodec.h"
#define GUCEF_COMCORE_CRPCFRAMECODEC_H
#endif /* GUCEF_COMCORE_CRPCFRAMECODEC_H ? */

#ifndef GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H
#include "gucefCOMCORE_CRpcDurableRequestTable.h"
#define GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H
#endif /* GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Test decorator of an RPC transport (gucefCOMCORE RPC design 16.1): records every frame sent and received (the
 *  capturing decorator) and, when asked, damages what this side sends: drop, duplicate, reorder, delay with jitter,
 *  disconnect after N frames, and tamperProtection (frames received are reported as PUBLIC). It reports itself
 *  unreliable and unordered when it loses, duplicates or reorders. Header only: gucefCOM_TestApp uses it too.
 *  Its notifier has a lock of its own, as every transport's (gucefCOMCORE RPC design 10.2): it raises the events of
 *  the inner transport on that one's thread while endpoints subscribe on others.
 */
class CRpcLossyTransport : public GUCEF::COMCORE::CIRpcTransport
{
    public:

    struct SFrame
    {
        GUCEF::CORE::CDynamicBuffer bytes;
        GUCEF::COMCORE::TRpcProtectionClass protection;
        GUCEF::UInt8 type;
        GUCEF::UInt64 seq;
    };

    GUCEF::UInt32 dropPermille;
    GUCEF::UInt32 duplicatePermille;
    GUCEF::UInt32 reorderPermille;
    GUCEF::UInt32 delayMs;
    GUCEF::UInt32 jitterMs;
    GUCEF::UInt32 disconnectAfterFrames;        /**< 0 = never */
    bool tamperProtection;
    bool lossyControlFrames;                    /**< damage control frames (HELLO, ACK, ...) too */
    bool blackHole;                             /**< session frames vanish while the transport still claims reliability */
    bool forceInsecure;                         /**< reports isSecure false */
    bool hasPrincipalOverride;
    GUCEF::COMCORE::CRpcPrincipal principalOverride;
    GUCEF::CORE::CString profileName;           /**< reported profile; empty = the inner transport's */
    GUCEF::UInt32 downgradeRefusals;            /**< OnProtectionDowngradeRefused() calls of the endpoint */
    GUCEF::UInt8 dropFrameType;                 /**< frames of this type sent are dropped, dropFrameTypeCount of them */
    GUCEF::UInt32 dropFrameTypeCount;
    bool reportUnreliable;                      /**< reports an unreliable, unordered transport whatever it does */

    static GUCEF::COMCORE::CIRpcTransportPtr Wrap( const GUCEF::COMCORE::CIRpcTransportPtr& inner, CRpcLossyTransport** decorator )
    {
        GUCEF::CORE::CTSharedPtr< CRpcLossyTransport, GUCEF::MT::CMutex > shared( GUCEF_NEW CRpcLossyTransport( inner ) );
        if ( GUCEF_NULL != decorator )
            *decorator = shared.GetPointerAlways();
        GUCEF::COMCORE::CIRpcTransportPtr result;
        result = shared;
        return result;
    }

    explicit CRpcLossyTransport( const GUCEF::COMCORE::CIRpcTransportPtr& inner )
        : GUCEF::COMCORE::CIRpcTransport()
        , dropPermille( 0 ), duplicatePermille( 0 ), reorderPermille( 0 ), delayMs( 0 ), jitterMs( 0 )
        , disconnectAfterFrames( 0 ), tamperProtection( false ), lossyControlFrames( false ), blackHole( false ), forceInsecure( false )
        , hasPrincipalOverride( false ), principalOverride(), profileName(), downgradeRefusals( 0 ), dropFrameType( 0 )
        , dropFrameTypeCount( 0 ), reportUnreliable( false )
        , m_inner( inner ), m_notifierLock(), m_lock(), m_sent(), m_received(), m_held(), m_hasHeld( false ), m_delayed()
        , m_random( 12345 ), m_sentCount( 0 ), m_closeRequested( false ), m_properties(), m_observer()
    {
        typedef GUCEF::CORE::CTEventHandlerFunctor< CRpcLossyTransport > TEventCallback;
        GUCEF::COMCORE::CIRpcTransport* innerPtr = m_inner.GetPointerAlways();
        TEventCallback connected( this, &CRpcLossyTransport::OnInnerConnected );
        TEventCallback disconnected( this, &CRpcLossyTransport::OnInnerDisconnected );
        TEventCallback frame( this, &CRpcLossyTransport::OnInnerFrame );
        TEventCallback writable( this, &CRpcLossyTransport::OnInnerWritable );
        m_observer.SubscribeTo( innerPtr, ConnectedEvent, connected );
        m_observer.SubscribeTo( innerPtr, DisconnectedEvent, disconnected );
        m_observer.SubscribeTo( innerPtr, FrameReceivedEvent, frame );
        m_observer.SubscribeTo( innerPtr, WritableEvent, writable );
        SubscribeToPulses();
    }

    virtual ~CRpcLossyTransport()
    {
        m_observer.UnsubscribeAllFromObserver();
    }

    void SetSeed( GUCEF::UInt32 seed ) { GUCEF::MT::CScopeMutex lock( m_lock ); m_random = seed; }

    /** Frames recorded so far */
    GUCEF::UInt32 GetSentCount( void ) const { GUCEF::MT::CScopeMutex lock( m_lock ); return (GUCEF::UInt32) m_sent.size(); }
    GUCEF::UInt32 GetReceivedCount( void ) const { GUCEF::MT::CScopeMutex lock( m_lock ); return (GUCEF::UInt32) m_received.size(); }
    void GetSent( GUCEF::vector< SFrame >& frames ) const { GUCEF::MT::CScopeMutex lock( m_lock ); frames = m_sent; }
    void GetReceived( GUCEF::vector< SFrame >& frames ) const { GUCEF::MT::CScopeMutex lock( m_lock ); frames = m_received; }
    void ClearCapture( void ) { GUCEF::MT::CScopeMutex lock( m_lock ); m_sent.clear(); m_received.clear(); }

    GUCEF::UInt32 CountSent( GUCEF::UInt8 type ) const
    {
        GUCEF::MT::CScopeMutex lock( m_lock );
        GUCEF::UInt32 count = 0;
        for ( GUCEF::UInt32 i=0; i<m_sent.size(); ++i )
            if ( m_sent[ i ].type == type )
                ++count;
        return count;
    }

    /** Sends of one seq: more than one means a retransmission */
    GUCEF::UInt32 CountSentSeq( GUCEF::UInt64 seq ) const
    {
        GUCEF::MT::CScopeMutex lock( m_lock );
        GUCEF::UInt32 count = 0;
        for ( GUCEF::UInt32 i=0; i<m_sent.size(); ++i )
            if ( m_sent[ i ].seq == seq )
                ++count;
        return count;
    }

    /** Whether frames sent from index on contain the bytes */
    bool SentContains( GUCEF::UInt32 fromIndex, const void* needle, GUCEF::UInt32 size ) const
    {
        GUCEF::MT::CScopeMutex lock( m_lock );
        for ( GUCEF::UInt32 i=fromIndex; i<m_sent.size(); ++i )
        {
            const GUCEF::UInt8* data = static_cast< const GUCEF::UInt8* >( m_sent[ i ].bytes.GetConstBufferPtr() );
            GUCEF::UInt32 length = m_sent[ i ].bytes.GetDataSize();
            for ( GUCEF::UInt32 p=0; p+size<=length; ++p )
                if ( 0 == memcmp( data + p, needle, size ) )
                    return true;
        }
        return false;
    }

    /** Bytes straight to the peer, past the capture: hand-made frames */
    bool SendRaw( const void* frame, GUCEF::UInt32 size ) { return m_inner->SendFrame( frame, size, GUCEF::COMCORE::RPC_PROTECTION_CONFIDENTIAL ); }

    /** Closes the inner transport now: a forced disconnect */
    void ForceDisconnect( void ) { m_inner->Close(); }

    virtual const GUCEF::CORE::CString& GetTransportType( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_inner->GetTransportType(); }
    virtual bool Connect( const GUCEF::CORE::CUri& remote ) GUCEF_VIRTUAL_OVERRIDE { return m_inner->Connect( remote ); }
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE { m_inner->Close(); }
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_inner->IsConnected(); }
    virtual const GUCEF::COMCORE::CRpcPrincipal& GetPeerPrincipal( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return hasPrincipalOverride ? principalOverride : m_inner->GetPeerPrincipal();
    }
    virtual GUCEF::CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_inner->GetPulseGenerator(); }
    virtual GUCEF::CORE::CUri GetRemoteUri( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_inner->GetRemoteUri(); }
    /** Counted, and passed on: the inner transport may report it itself (drn) */
    virtual void OnProtectionDowngradeRefused( GUCEF::COMCORE::TRpcProtectionClass required, GUCEF::COMCORE::TRpcProtectionClass arrivedWith ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++downgradeRefusals;
        m_inner->OnProtectionDowngradeRefused( required, arrivedWith );
    }

    virtual GUCEF::MT::TLockStatus Lock( GUCEF::UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return m_notifierLock.Lock( lockWaitTimeoutInMs );
    }

    virtual GUCEF::MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return m_notifierLock.Unlock();
    }
    virtual bool SetPulseGenerator( const GUCEF::CORE::PulseGeneratorPtr& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE
    {
        bool set = m_inner->SetPulseGenerator( pulseGenerator );
        SubscribeToPulses();
        return set;
    }

    virtual const GUCEF::COMCORE::CRpcTransportProperties& GetProperties( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::MT::CScopeMutex lock( m_lock );
        m_properties = m_inner->GetProperties();
        if ( 0 != dropPermille || 0 != duplicatePermille || 0 != reorderPermille || 0 != jitterMs || reportUnreliable )
        {
            m_properties.isReliable = false;
            m_properties.isOrdered = false;
        }
        if ( tamperProtection )
            m_properties.hasFrameProtectionClasses = true;
        if ( forceInsecure )
            m_properties.isSecure = false;
        if ( !profileName.IsNULLOrEmpty() )
            m_properties.profileName = profileName;
        return m_properties;
    }

    virtual bool SendFrame( const void* frame, GUCEF::UInt32 size, GUCEF::COMCORE::TRpcProtectionClass protection ) GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::MT::CScopeMutex lock( m_lock );
        Record( m_sent, frame, size, protection );
        ++m_sentCount;
        if ( 0 != disconnectAfterFrames && m_sentCount > disconnectAfterFrames )
        {
            // never call out from SendFrame: the close happens at the next pulse
            m_closeRequested = true;
            return true;
        }
        bool control = 0 == m_sent.back().seq;
        if ( blackHole && !control )
            return true;
        if ( 0 != dropFrameTypeCount && m_sent.back().type == dropFrameType )
        {
            --dropFrameTypeCount;
            return true;
        }
        bool damage = !control || lossyControlFrames;
        if ( damage && Chance( dropPermille ) )
            return true;
        if ( damage && ( 0 != delayMs || 0 != jitterMs ) )
        {
            SDelayed delayed;
            delayed.bytes.CopyFrom( size, frame );
            delayed.protection = protection;
            delayed.releaseMs = GUCEF::COMCORE::CRpcDurableRequestTable::NowMs() + delayMs + ( 0 != jitterMs ? Next() % jitterMs : 0 );
            m_delayed.push_back( delayed );
            if ( Chance( duplicatePermille ) )
            {
                delayed.releaseMs += 1 + ( 0 != jitterMs ? Next() % jitterMs : 0 );
                m_delayed.push_back( delayed );
            }
            return true;
        }
        if ( !m_delayed.empty() )
        {
            // a delay line keeps its order: while delayed frames wait (the delay was just turned off, or this frame is
            // not one that is delayed), a frame waits behind them
            SDelayed delayed;
            delayed.bytes.CopyFrom( size, frame );
            delayed.protection = protection;
            delayed.releaseMs = m_delayed.back().releaseMs;
            m_delayed.push_back( delayed );
            return true;
        }
        if ( damage && Chance( reorderPermille ) && !m_hasHeld )
        {
            m_held.CopyFrom( size, frame );
            m_heldProtection = protection;
            m_hasHeld = true;
            return true;
        }
        bool sent = m_inner->SendFrame( frame, size, protection );
        if ( damage && Chance( duplicatePermille ) )
            m_inner->SendFrame( frame, size, protection );
        if ( m_hasHeld )
        {
            m_inner->SendFrame( m_held.GetConstBufferPtr(), m_held.GetDataSize(), m_heldProtection );
            m_hasHeld = false;
        }
        return sent;
    }

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString classTypeName = "CRpcLossyTransport";
        return classTypeName;
    }

    private:

    struct SDelayed
    {
        GUCEF::CORE::CDynamicBuffer bytes;
        GUCEF::COMCORE::TRpcProtectionClass protection;
        GUCEF::UInt64 releaseMs;
    };

    GUCEF::UInt32 Next( void )
    {
        m_random = m_random * 1103515245u + 12345u;
        return ( m_random >> 8 ) & 0xFFFFFF;
    }

    bool Chance( GUCEF::UInt32 permille ) { return 0 != permille && ( Next() % 1000 ) < permille; }

    void Record( GUCEF::vector< SFrame >& list, const void* frame, GUCEF::UInt32 size, GUCEF::COMCORE::TRpcProtectionClass protection )
    {
        SFrame entry;
        entry.bytes.CopyFrom( size, frame );
        entry.protection = protection;
        GUCEF::COMCORE::SRpcFrameHeader header;
        if ( GUCEF::COMCORE::CRpcFrameCodec::ParseHeader( static_cast< const GUCEF::UInt8* >( frame ), size, header ) )
        {
            entry.type = header.type;
            entry.seq = header.seq;
        }
        else
        {
            entry.type = 0;
            entry.seq = 0;
        }
        list.push_back( entry );
    }

    void SubscribeToPulses( void )
    {
        GUCEF::CORE::PulseGeneratorPtr pulseGenerator = m_inner->GetPulseGenerator();
        if ( pulseGenerator.IsNULL() )
            return;
        typedef GUCEF::CORE::CTEventHandlerFunctor< CRpcLossyTransport > TEventCallback;
        TEventCallback pulse( this, &CRpcLossyTransport::OnPulse );
        m_observer.SubscribeTo( pulseGenerator.GetPointerAlways(), GUCEF::CORE::CPulseGenerator::PulseEvent, pulse );
    }

    void OnPulse( GUCEF::CORE::CNotifier*, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* )
    {
        bool close = false;
        {
            GUCEF::MT::CScopeMutex lock( m_lock );
            GUCEF::UInt64 now = GUCEF::COMCORE::CRpcDurableRequestTable::NowMs();
            for ( GUCEF::UInt32 i=0; i<m_delayed.size(); )
            {
                if ( now >= m_delayed[ i ].releaseMs )
                {
                    m_inner->SendFrame( m_delayed[ i ].bytes.GetConstBufferPtr(), m_delayed[ i ].bytes.GetDataSize(), m_delayed[ i ].protection );
                    m_delayed.erase( m_delayed.begin() + i );
                }
                else
                    ++i;
            }
            if ( m_hasHeld && Chance( 300 ) )
            {
                m_inner->SendFrame( m_held.GetConstBufferPtr(), m_held.GetDataSize(), m_heldProtection );
                m_hasHeld = false;
            }
            close = m_closeRequested;
            m_closeRequested = false;
            if ( close )
                disconnectAfterFrames = 0;
        }
        if ( close )
            m_inner->Close();
    }

    void OnInnerConnected( GUCEF::CORE::CNotifier*, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* eventData )
    {
        NotifyObservers( ConnectedEvent, eventData );
    }

    void OnInnerDisconnected( GUCEF::CORE::CNotifier*, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* eventData )
    {
        NotifyObservers( DisconnectedEvent, eventData );
    }

    void OnInnerWritable( GUCEF::CORE::CNotifier*, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* eventData )
    {
        NotifyObservers( WritableEvent, eventData );
    }

    void OnInnerFrame( GUCEF::CORE::CNotifier*, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* eventData )
    {
        GUCEF::COMCORE::CRpcFrameEventData* frame = static_cast< GUCEF::COMCORE::CRpcFrameEventData* >( eventData );
        {
            GUCEF::MT::CScopeMutex lock( m_lock );
            Record( m_received, frame->GetFrame(), frame->GetSize(), frame->GetProtection() );
        }
        if ( tamperProtection )
        {
            GUCEF::COMCORE::CRpcFrameEventData tampered( frame->GetFrame(), frame->GetSize(), GUCEF::COMCORE::RPC_PROTECTION_PUBLIC );
            NotifyObservers( FrameReceivedEvent, &tampered );
            return;
        }
        NotifyObservers( FrameReceivedEvent, eventData );
    }

    GUCEF::COMCORE::CIRpcTransportPtr m_inner;
    mutable GUCEF::MT::CMutex m_notifierLock;
    mutable GUCEF::MT::CMutex m_lock;
    GUCEF::vector< SFrame > m_sent;
    GUCEF::vector< SFrame > m_received;
    GUCEF::CORE::CDynamicBuffer m_held;
    GUCEF::COMCORE::TRpcProtectionClass m_heldProtection;
    bool m_hasHeld;
    GUCEF::vector< SDelayed > m_delayed;
    GUCEF::UInt32 m_random;
    GUCEF::UInt32 m_sentCount;
    bool m_closeRequested;
    mutable GUCEF::COMCORE::CRpcTransportProperties m_properties;
    GUCEF::CORE::CObserver m_observer;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCLOSSYTRANSPORT_H ? */
