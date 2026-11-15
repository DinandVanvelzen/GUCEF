/*
 *  gucefDRNUTILS_TestApp: GUCEF test application
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

#ifndef GUCEF_TESTDRNUTILSTCPPROXY_H
#define GUCEF_TESTDRNUTILSTCPPROXY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_CORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_CORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_CORE_CTCPSERVERCONNECTION_H
#include "CTCPServerConnection.h"
#define GUCEF_CORE_CTCPSERVERCONNECTION_H
#endif /* GUCEF_CORE_CTCPSERVERCONNECTION_H ? */

#ifndef GUCEF_CORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_CORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_CORE_CTCPCLIENTSOCKET_H ? */

namespace DRNUTILSTEST {

using namespace GUCEF;

/**
 *  A TCP proxy on the loopback for fault injection between a client and a service (the network design's test plan
 *  12.1): every connection to the proxy is forwarded to the target port, and the faults are switched at any time:
 *      Cut()                       closes both sides of every forwarded connection (a reset: the link is gone at once)
 *      SetBlackHole( true )        keeps the sockets but forwards nothing in either direction (a silent path)
 *      SetDelayMs( ms )            holds every chunk for that long before it is forwarded
 *      SetThrottle( bytesPerSec )  forwards at most that many bytes a second in each direction (0: no limit)
 *      SetRefuseNew( true )        accepts new connections and closes them at once (a service that is not there)
 *  Everything runs on the pulse generator given to Start() (the test thread pumps it). Not thread safe.
 */
class CTestTcpFaultProxy : public CORE::CObserver
{
    public:

    struct SStats
    {
        UInt32 accepted;
        UInt32 cuts;
        UInt64 forwardedBytes;
        UInt64 droppedBytes;

        SStats( void ) : accepted( 0 ), cuts( 0 ), forwardedBytes( 0 ), droppedBytes( 0 ) {}
    };

    CTestTcpFaultProxy( void )
        : CORE::CObserver(), m_pulse(), m_server( GUCEF_NULL ), m_targetPort( 0 ), m_pairs(), m_doomed(), m_blackHole( false )
        , m_refuseNew( false ), m_delayMs( 0 ), m_throttle( 0 ), m_budgetUp( 0 ), m_budgetDown( 0 ), m_lastBudgetMs( 0 ), m_stats()
    {}

    virtual ~CTestTcpFaultProxy()
    {
        Stop();
        SignalUpcomingObserverDestruction();
    }

    bool Start( const CORE::PulseGeneratorPtr& pulse, UInt16 targetPort )
    {
        m_pulse = pulse;
        m_targetPort = targetPort;
        m_server = GUCEF_NEW COMCORE::CTCPServerSocket( pulse, false, 64 );
        m_server->Subscribe( this );
        CORE::PulseGeneratorPtr p( pulse );
        p->Subscribe( this, CORE::CPulseGenerator::PulseEvent );
        if ( !m_server->ListenOnPort( 0 ) )
            return false;
        return true;
    }

    void Stop( void )
    {
        Cut();
        ReapDoomed();
        if ( GUCEF_NULL != m_server )
        {
            m_server->Unsubscribe( this );
            m_server->Close();
            GUCEF_DELETE m_server;
            m_server = GUCEF_NULL;
        }
        if ( !m_pulse.IsNULL() )
            m_pulse->Unsubscribe( this );
    }

    UInt16 GetPort( void ) const { return GUCEF_NULL != m_server ? m_server->GetBoundPort() : 0; }
    CORE::CString GetHostAndPort( void ) const { return "127.0.0.1:" + CORE::ToString( GetPort() ); }

    void Cut( void )
    {
        for ( UInt32 i=0; i<m_pairs.size(); ++i )
            ClosePair( i );
        m_pairs.clear();
        ++m_stats.cuts;
    }

    void SetBlackHole( bool blackHole ) { m_blackHole = blackHole; }
    void SetRefuseNew( bool refuse ) { m_refuseNew = refuse; }
    void SetDelayMs( UInt32 delayMs ) { m_delayMs = delayMs; }
    void SetThrottle( UInt32 bytesPerSecond ) { m_throttle = bytesPerSecond; }
    UInt32 GetPairCount( void ) const { return (UInt32) m_pairs.size(); }
    const SStats& GetStats( void ) const { return m_stats; }

    protected:

    struct SChunk
    {
        CORE::CDynamicBuffer data;
        UInt64 dueMs;
    };
    typedef GUCEF::vector< SChunk > TChunkVector;

    struct SPair
    {
        COMCORE::CTCPServerConnection* inbound;
        COMCORE::CTCPClientSocket* outbound;
        bool outboundUp;
        TChunkVector toTarget;
        TChunkVector toClient;
    };
    typedef GUCEF::vector< SPair > TPairVector;

    static UInt64 NowMs( void )
    {
        UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
        return MT::PrecisionTickCount() / ( 0 == ticksPerMs ? 1 : ticksPerMs );
    }

    void ClosePair( UInt32 index )
    {
        SPair& pair = m_pairs[ index ];
        if ( GUCEF_NULL != pair.inbound )
        {
            pair.inbound->Unsubscribe( this );
            pair.inbound->Close();
            pair.inbound = GUCEF_NULL;
        }
        if ( GUCEF_NULL != pair.outbound )
        {
            pair.outbound->Unsubscribe( this );
            pair.outbound->Close();
            m_doomed.push_back( pair.outbound );
            pair.outbound = GUCEF_NULL;
        }
    }

    void ReapDoomed( void )
    {
        for ( UInt32 i=0; i<m_doomed.size(); ++i )
            GUCEF_DELETE m_doomed[ i ];
        m_doomed.clear();
    }

    Int32 FindPair( CORE::CNotifier* notifier, bool& fromClient ) const
    {
        for ( UInt32 i=0; i<m_pairs.size(); ++i )
        {
            if ( static_cast< CORE::CNotifier* >( m_pairs[ i ].inbound ) == notifier ) { fromClient = true; return (Int32) i; }
            if ( static_cast< CORE::CNotifier* >( m_pairs[ i ].outbound ) == notifier ) { fromClient = false; return (Int32) i; }
        }
        return -1;
    }

    void Queue( SPair& pair, bool toTarget, const CORE::CDynamicBuffer& data )
    {
        if ( m_blackHole )
        {
            m_stats.droppedBytes += data.GetDataSize();
            return;
        }
        SChunk chunk;
        chunk.data = data;
        chunk.dueMs = NowMs() + m_delayMs;
        ( toTarget ? pair.toTarget : pair.toClient ).push_back( chunk );
    }

    UInt32 Flush( TChunkVector& chunks, COMCORE::CTCPConnection* to, UInt64 now, UInt64& budget )
    {
        UInt32 sent = 0;
        while ( !chunks.empty() && GUCEF_NULL != to && chunks.front().dueMs <= now )
        {
            SChunk& chunk = chunks.front();
            UInt32 size = chunk.data.GetDataSize();
            if ( 0 != m_throttle )
            {
                if ( 0 == budget )
                    break;
                if ( size > budget )
                {
                    // send what the budget allows, keep the rest
                    to->Send( chunk.data.GetConstBufferPtr(), (UInt32) budget );
                    m_stats.forwardedBytes += budget;
                    CORE::CDynamicBuffer rest;
                    rest.Append( static_cast< const UInt8* >( chunk.data.GetConstBufferPtr() ) + budget, size - (UInt32) budget );
                    chunk.data = rest;
                    budget = 0;
                    break;
                }
                budget -= size;
            }
            to->Send( chunk.data.GetConstBufferPtr(), size );
            m_stats.forwardedBytes += size;
            sent += size;
            chunks.erase( chunks.begin() );
        }
        return sent;
    }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CORE::CPulseGenerator::PulseEvent == eventId )
        {
            ReapDoomed();
            UInt64 now = NowMs();
            if ( 0 != m_throttle && now - m_lastBudgetMs >= 10 )
            {
                UInt64 add = (UInt64) m_throttle * ( now - m_lastBudgetMs ) / 1000;
                m_budgetUp = add > m_throttle ? m_throttle : add;
                m_budgetDown = m_budgetUp;
                m_lastBudgetMs = now;
            }
            for ( UInt32 i=0; i<m_pairs.size(); ++i )
            {
                SPair& pair = m_pairs[ i ];
                if ( pair.outboundUp )
                    Flush( pair.toTarget, pair.outbound, now, m_budgetUp );
                Flush( pair.toClient, pair.inbound, now, m_budgetDown );
            }
            return;
        }
        if ( notifier == m_server )
        {
            if ( COMCORE::CTCPServerSocket::ClientConnectedEvent == eventId && GUCEF_NULL != eventData )
            {
                COMCORE::CTCPServerSocket::TConnectionInfo& info = static_cast< COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventData )->GetData();
                if ( GUCEF_NULL == info.connection )
                    return;
                ++m_stats.accepted;
                if ( m_refuseNew )
                {
                    info.connection->Close();
                    return;
                }
                SPair pair;
                pair.inbound = info.connection;
                pair.outbound = GUCEF_NEW COMCORE::CTCPClientSocket( m_pulse, false );
                pair.outboundUp = false;
                pair.inbound->Subscribe( this );
                pair.outbound->Subscribe( this );
                m_pairs.push_back( pair );
                m_pairs.back().outbound->ConnectTo( "127.0.0.1", m_targetPort, false );
            }
            return;
        }
        bool fromClient = false;
        Int32 index = FindPair( notifier, fromClient );
        if ( index < 0 )
            return;
        SPair& pair = m_pairs[ index ];
        if ( COMCORE::CTCPConnection::DataRecievedEvent == eventId && GUCEF_NULL != eventData )
        {
            const CORE::CDynamicBuffer& data = static_cast< COMCORE::CTCPConnection::TDataRecievedEventData* >( eventData )->GetData();
            Queue( pair, fromClient, data );
        }
        else
        if ( COMCORE::CTCPConnection::ConnectedEvent == eventId && !fromClient )
        {
            pair.outboundUp = true;
        }
        else
        if ( COMCORE::CTCPConnection::DisconnectedEvent == eventId || COMCORE::CTCPConnection::SocketErrorEvent == eventId )
        {
            // the server socket reuses its connection objects: the late event of a connection that went before is not about this pair
            if ( fromClient && GUCEF_NULL != pair.inbound && pair.inbound->IsActive() && COMCORE::CTCPConnection::DisconnectedEvent == eventId )
                return;
            ClosePair( (UInt32) index );
            m_pairs.erase( m_pairs.begin() + index );
        }
    }

    CORE::PulseGeneratorPtr m_pulse;
    COMCORE::CTCPServerSocket* m_server;
    UInt16 m_targetPort;
    TPairVector m_pairs;
    GUCEF::vector< COMCORE::CTCPClientSocket* > m_doomed;
    bool m_blackHole;
    bool m_refuseNew;
    UInt32 m_delayMs;
    UInt32 m_throttle;
    UInt64 m_budgetUp;
    UInt64 m_budgetDown;
    UInt64 m_lastBudgetMs;
    SStats m_stats;
};

}; /* namespace DRNUTILSTEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_TESTDRNUTILSTCPPROXY_H ? */
