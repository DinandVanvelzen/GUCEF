/*
 *  gucefDRNUTILS_TestApp: GUCEF test application for gucefDRNUTILS
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

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICESET_H
#include "gucefCOMCORE_CRpcServiceSet.h"
#define GUCEF_COMCORE_CRPCSERVICESET_H
#endif /* GUCEF_COMCORE_CRPCSERVICESET_H ? */

#ifndef GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H
#include "gucefCOMCORE_CRpcLoopbackTransport.h"
#define GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H
#endif /* GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLRESULT_H
#include "gucefCOMCORE_CRpcCallResult.h"
#define GUCEF_COMCORE_CRPCCALLRESULT_H
#endif /* GUCEF_COMCORE_CRPCCALLRESULT_H ? */

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCLOSSYTRANSPORT_H
#include "TestRpcLossyTransport.h"
#define GUCEF_COMCORE_TESTAPP_TESTRPCLOSSYTRANSPORT_H
#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCLOSSYTRANSPORT_H ? */

namespace DRNUTILSTEST {

using namespace GUCEF;

/** Pulses the global pulse generator (driven by the test thread) for the given time */
inline void
PumpFor( UInt32 milliseconds )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    UInt64 start = MT::PrecisionTickCount();
    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    if ( 0 == ticksPerMs )
        ticksPerMs = 1;
    do
    {
        pulseGenerator->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
    while ( ( MT::PrecisionTickCount() - start ) / ticksPerMs < milliseconds );
}

/** A condition the pump helpers wait for */
class CCondition
{
    public:
    virtual ~CCondition() {}
    virtual bool IsMet( void ) = 0;
};

/** Pulses until the condition is met or the time is up */
inline bool
PumpUntil( CCondition& condition, UInt32 timeoutMs )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    UInt64 start = MT::PrecisionTickCount();
    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    if ( 0 == ticksPerMs )
        ticksPerMs = 1;
    while ( !condition.IsMet() )
    {
        pulseGenerator->ForceDirectPulse();
        if ( condition.IsMet() )
            return true;
        if ( ( MT::PrecisionTickCount() - start ) / ticksPerMs > timeoutMs )
            return false;
        MT::PrecisionDelay( 1 );
    }
    return true;
}

/** Waits for a counter to reach a value */
class CCounterAtLeast : public CCondition
{
    public:
    const UInt64& m_counter;
    UInt64 m_expected;
    CCounterAtLeast( const UInt64& counter, UInt64 expected ) : m_counter( counter ), m_expected( expected ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_counter >= m_expected; }
};

/*-------------------------------------------------------------------------*/

/**
 *  Two RPC endpoints on loopback, both on the global pulse generator driven by the test thread: the server shares the
 *  given service set (where the hosts register), the client wants the DRNUTILS contracts. The server side transport is
 *  wrapped so that the principal the server sees can be set (roles, transport name, attributes).
 */
class CDrnUtilsRpcWorld
{
    public:

    CORE::PulseGeneratorPtr pulseGenerator;
    TThreadId previousDriver;
    COMCORE::CRpcServiceSetPtr services;
    COMCORE::CRpcEndpointSettings clientSettings;
    COMCORE::CRpcEndpointSettings serverSettings;
    COMCORE::CRpcEndpointPtr client;
    COMCORE::CRpcEndpointPtr server;
    COMCORE::CIRpcTransportPtr clientTransport;
    COMCORE::CIRpcTransportPtr serverTransport;
    CRpcLossyTransport* serverWire;
    CRpcLossyTransport* clientWire;

    CDrnUtilsRpcWorld( void )
        : pulseGenerator(), previousDriver( 0 ), services( COMCORE::CRpcServiceSet::Create() ), clientSettings(), serverSettings()
        , client(), server(), clientTransport(), serverTransport(), serverWire( GUCEF_NULL ), clientWire( GUCEF_NULL )
    {
        pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        previousDriver = pulseGenerator->GetPulseDriverThreadId();
        pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        clientSettings.nodeName = "viewer";
        serverSettings.nodeName = "node";
    }

    ~CDrnUtilsRpcWorld()
    {
        Disconnect();
        pulseGenerator->SetPulseDriverThreadId( previousDriver );
    }

    void Disconnect( void )
    {
        if ( !client.IsNULL() )
            client->Close( false );
        if ( !server.IsNULL() )
            server->Close( false );
        PumpFor( 20 );
        client.Unlink();
        server.Unlink();
        clientTransport.Unlink();
        serverTransport.Unlink();
        serverWire = GUCEF_NULL;
        clientWire = GUCEF_NULL;
        PumpFor( 5 );
    }

    /** The principal the server sees: loopback (secure), authenticated, with the roles */
    static COMCORE::CRpcPrincipal Principal( const CORE::CString& subject, const char* roles, const CORE::CString& transport = "loopback" )
    {
        COMCORE::CRpcPrincipal principal( transport, subject, true );
        CORE::CString::StringVector list = CORE::CString( roles ).ParseElements( ',', false );
        for ( UInt32 i=0; i<list.size(); ++i )
            principal.roles.insert( list[ i ].Trim( true ).Trim( false ) );
        return principal;
    }

    /** A new session between the endpoints; the server sees the given principal. Waits until both contracts are bound */
    bool Connect( const COMCORE::CRpcPrincipal& principal, bool wantTasks = true, bool wantScheduler = true )
    {
        Disconnect();
        COMCORE::CIRpcTransportPtr a;
        COMCORE::CIRpcTransportPtr b;
        if ( !COMCORE::CRpcLoopbackTransport::CreatePair( pulseGenerator, pulseGenerator, a, b ) )
            return false;
        clientTransport = CRpcLossyTransport::Wrap( a, &clientWire );
        serverTransport = CRpcLossyTransport::Wrap( b, &serverWire );
        serverWire->hasPrincipalOverride = true;
        serverWire->principalOverride = principal;
        server = COMCORE::CRpcEndpoint::Create( pulseGenerator, serverSettings );
        client = COMCORE::CRpcEndpoint::Create( pulseGenerator, clientSettings );
        server->SetServiceSet( services );
        if ( wantScheduler )
            client->AddWantedContract( "gucef.drnutils.scheduler", 1 );
        if ( wantTasks )
            client->AddWantedContract( "gucef.drnutils.tasks", 1 );
        if ( !server->Start( serverTransport, false ) || !client->Start( clientTransport, true ) )
            return false;
        return PumpUntilBound( wantScheduler ? "gucef.drnutils.scheduler" : "gucef.drnutils.tasks", 3000 );
    }

    bool PumpUntilBound( const char* name, UInt32 timeoutMs )
    {
        class CBound : public CCondition
        {
            public:
            CDrnUtilsRpcWorld& w;
            const char* n;
            CBound( CDrnUtilsRpcWorld& world, const char* contract ) : w( world ), n( contract ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return w.client->IsContractBound( n, 1 ); }
        };
        CBound bound( *this, name );
        bool ok = PumpUntil( bound, timeoutMs );
        PumpFor( 5 );
        return ok;
    }

    COMCORE::CRpcMessage NewRequest( const CORE::CString& path )
    {
        COMCORE::CRpcMethodRef ref;
        COMCORE::CRpcStatus status;
        if ( !client->ResolveMethod( path, ref, status ) )
            return COMCORE::CRpcMessage();
        return client->NewRequest( ref );
    }

    /** A call through the async API, pumped until it ends */
    COMCORE::CRpcCallResult Call( const CORE::CString& path, const COMCORE::CRpcMessage& request,
                                  const COMCORE::CRpcCallOptions& options = COMCORE::CRpcCallOptions(), UInt32 timeoutMs = 5000 )
    {
        COMCORE::CRpcCallResult result;
        CORE::CFutureResult future = client->CallAsync( path, request, options );
        class CReady : public CCondition
        {
            public:
            CORE::CFutureResult& f;
            explicit CReady( CORE::CFutureResult& future ) : f( future ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return f.IsReady() || f.HasNoFuture(); }
        };
        CReady ready( future );
        if ( !PumpUntil( ready, timeoutMs ) )
        {
            result.status.Set( COMCORE::RPC_STATUS_DEADLINE_EXCEEDED, "test timeout" );
            future.RequestCancellation();
            return result;
        }
        COMCORE::CRpcEndpoint::TryGetCallResult( future, result );
        return result;
    }

    /** Opens a server stream */
    COMCORE::CIRpcStreamPtr OpenStream( const CORE::CString& path, const COMCORE::CRpcMessage& request )
    {
        COMCORE::CRpcMethodRef ref;
        COMCORE::CRpcStatus status;
        if ( !client->ResolveMethod( path, ref, status ) )
            return COMCORE::CIRpcStreamPtr();
        return client->OpenStream( ref, &request );
    }
};

/*-------------------------------------------------------------------------*/

/** Collects the messages of a client stream as they arrive (MessageEvent), and its end */
class CStreamRecorder : public CORE::CObserver
{
    public:
    GUCEF::vector< COMCORE::CRpcMessage > messages;
    UInt64 count;
    bool ended;
    COMCORE::CRpcStatus status;
    CStreamRecorder( void ) : CORE::CObserver(), messages(), count( 0 ), ended( false ), status() {}
    virtual ~CStreamRecorder() { SignalUpcomingObserverDestruction(); }
    void Watch( const COMCORE::CIRpcStreamPtr& stream )
    {
        COMCORE::CIRpcStreamPtr watched( stream );
        watched->Subscribe( this );
    }
    protected:
    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( COMCORE::CIRpcStream::MessageEvent == eventid && GUCEF_NULL != eventdata )
        {
            messages.push_back( static_cast< COMCORE::CRpcStreamMessageEventData* >( eventdata )->message );
            ++count;
        }
        else
        if ( COMCORE::CIRpcStream::StatusEvent == eventid )
        {
            ended = true;
            if ( GUCEF_NULL != eventdata )
                status = *static_cast< COMCORE::CRpcStatus* >( eventdata );
        }
    }
};

}; /* namespace DRNUTILSTEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H ? */
