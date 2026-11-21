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

#ifndef GUCEF_TESTDRNUTILSREMOTEWORLD_H
#define GUCEF_TESTDRNUTILSREMOTEWORLD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUCEFCORE_H
#include "gucefCORE.h"
#define GUCEF_GUCEFCORE_H
#endif /* GUCEF_GUCEFCORE_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#include "gucefCOMCORE_CRpcContractBuilder.h"
#define GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTBUILDER_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#include "gucefCOMCORE_CRpcContractRegistry.h"
#define GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICEIMPL_H
#include "gucefCOMCORE_CRpcServiceImpl.h"
#define GUCEF_COMCORE_CRPCSERVICEIMPL_H
#endif /* GUCEF_COMCORE_CRPCSERVICEIMPL_H ? */

#ifndef GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#include "gucefCOMCORE_CTRpcMethodHandler.h"
#define GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#endif /* GUCEF_COMCORE_CTRPCMETHODHANDLER_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLRESULT_H
#include "gucefCOMCORE_CRpcCallResult.h"
#define GUCEF_COMCORE_CRPCCALLRESULT_H
#endif /* GUCEF_COMCORE_CRPCCALLRESULT_H ? */

#ifndef GUCEF_GUCEFDRNUTILS_H
#include "gucefDRNUTILS.h"
#define GUCEF_GUCEFDRNUTILS_H
#endif /* GUCEF_GUCEFDRNUTILS_H ? */

#ifndef GUCEF__TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF__TEST_UNIQUE_H
#endif /* GUCEF__TEST_UNIQUE_H ? */

#ifndef GUCEF_TESTDRNUTILSRPCWORLD_H
#include "TestDrnUtilsRpcWorld.h"
#define GUCEF_TESTDRNUTILSRPCWORLD_H
#endif /* GUCEF_TESTDRNUTILSRPCWORLD_H ? */

#ifndef GUCEF_TESTDRNUTILSDRNGROUP_H
#include "TestDrnUtilsDrnGroup.h"
#define GUCEF_TESTDRNUTILSDRNGROUP_H
#endif /* GUCEF_TESTDRNUTILSDRNGROUP_H ? */

#ifndef GUCEF_TESTDRNUTILSTCPPROXY_H
#include "TestDrnUtilsTcpProxy.h"
#define GUCEF_TESTDRNUTILSTCPPROXY_H
#endif /* GUCEF_TESTDRNUTILSTCPPROXY_H ? */

namespace DRNUTILSTEST {

using namespace GUCEF;

/** A wall clock that the test can move ahead, as after a sleep of the machine */
class CTestShiftedUtcClock : public CORE::CIUtcClock
{
    public:

    Int64 offsetMs;

    CTestShiftedUtcClock( void ) : CORE::CIUtcClock(), offsetMs( 0 ), m_system() {}
    virtual Int64 GetUtcTimeInMs( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_system.GetUtcTimeInMs() + offsetMs; }

    private:

    CORE::CSystemUtcClock m_system;
};

/*-------------------------------------------------------------------------*/

inline UInt64
RemoteTestNowMs( void )
{
    return DRNUTILS::CRemoteServiceHost::NowMs();
}

/*-------------------------------------------------------------------------*/

/** Waits for a condition, pumping the global pulse generator that the test thread drives */
template < class T >
class CMemberCondition : public CCondition
{
    public:

    typedef bool ( T::*TPredicate )( void ) const;
    const T& object;
    TPredicate predicate;

    CMemberCondition( const T& o, TPredicate p ) : object( o ), predicate( p ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return ( object.*predicate )(); }
};

/** Waits until a connection is in a state */
class CConnectionState : public CCondition
{
    public:

    const DRNUTILS::CRemoteServiceConnection& connection;
    DRNUTILS::TRemoteConnectionState state;

    CConnectionState( const DRNUTILS::CRemoteServiceConnection& c, DRNUTILS::TRemoteConnectionState s ) : connection( c ), state( s ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return connection.GetState() == state; }
};

/** Waits until a future is ready */
class CFutureReady : public CCondition
{
    public:

    CORE::CFutureResult& future;

    explicit CFutureReady( CORE::CFutureResult& f ) : future( f ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return future.IsReady() || future.HasNoFuture(); }
};

/*-------------------------------------------------------------------------*/

/**
 *  The contract of the products' side in the tests, gucef.drnutils.test.remote 1.0: Commands (a mutation with a
 *  command id, a slow call) and Uploads (staging over the bulk channel)
 */
inline COMCORE::CRpcContractPtr
RemoteTestContract( void )
{
    COMCORE::CRpcContractRegistry& registry = COMCORE::CComCoreGlobal::Instance()->GetRpcContractRegistry();
    COMCORE::CRpcContractPtr contract = registry.FindLatest( "gucef.drnutils.test.remote", 1 );
    if ( !contract.IsNULL() )
        return contract;
    COMCORE::CRpcContractBuilder builder( "gucef.drnutils.test.remote", 1, 0 );
    builder.AddType( "ApplyRequest" );
    builder.AddField( "ApplyRequest", 1, "commandId", "string" );
    builder.AddField( "ApplyRequest", 2, "value", "string" );
    builder.AddType( "ApplyResponse" );
    builder.AddField( "ApplyResponse", 1, "applied", "bool" );
    builder.AddField( "ApplyResponse", 2, "replayed", "bool" );
    builder.AddField( "ApplyResponse", 3, "revision", "uint64" );
    builder.AddType( "SlowRequest" );
    builder.AddField( "SlowRequest", 1, "delayMs", "uint32" );
    builder.AddType( "SlowResponse" );
    builder.AddField( "SlowResponse", 1, "done", "bool" );
    builder.AddType( "BeginRequest" );
    builder.AddField( "BeginRequest", 1, "name", "string" );
    builder.AddField( "BeginRequest", 2, "paths", "string", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "BeginRequest", 3, "sizes", "uint64", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "BeginRequest", 4, "digests", "string", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddType( "UploadAnswer" );
    builder.AddField( "UploadAnswer", 1, "uploadId", "string" );
    builder.AddField( "UploadAnswer", 2, "detail", "string" );
    builder.AddField( "UploadAnswer", 3, "ok", "bool" );
    builder.AddField( "UploadAnswer", 4, "receivedBytes", "uint64" );
    builder.AddField( "UploadAnswer", 5, "ranges", "string", COMCORE::RPC_CARDINALITY_REPEATED );
    builder.AddField( "UploadAnswer", 6, "channel", "string" );
    builder.AddType( "WriteRequest" );
    builder.AddField( "WriteRequest", 1, "uploadId", "string" );
    builder.AddField( "WriteRequest", 2, "fileIndex", "uint32" );
    builder.AddField( "WriteRequest", 3, "offset", "uint64" );
    builder.AddField( "WriteRequest", 4, "data", "bytes" );
    builder.AddType( "UploadRef" );
    builder.AddField( "UploadRef", 1, "uploadId", "string" );
    builder.AddService( "Commands", 1 );
    builder.AddMethod( "Commands", 1, "Apply", COMCORE::RPC_METHODKIND_UNARY, "ApplyRequest", "ApplyResponse", 0, 30000 );
    builder.AddMethod( "Commands", 2, "Slow", COMCORE::RPC_METHODKIND_UNARY, "SlowRequest", "SlowResponse", 0, 60000 );
    builder.AddService( "Uploads", 2 );
    builder.AddMethod( "Uploads", 1, "Begin", COMCORE::RPC_METHODKIND_UNARY, "BeginRequest", "UploadAnswer", 0, 30000 );
    builder.AddMethod( "Uploads", 2, "Write", COMCORE::RPC_METHODKIND_UNARY, "WriteRequest", "UploadAnswer", 0, 30000 );
    builder.AddMethod( "Uploads", 3, "Query", COMCORE::RPC_METHODKIND_UNARY, "UploadRef", "UploadAnswer", GUCEF_RPC_METHODFLAG_IDEMPOTENT, 30000 );
    builder.AddMethod( "Uploads", 4, "Commit", COMCORE::RPC_METHODKIND_UNARY, "UploadRef", "UploadAnswer", 0, 30000 );
    CORE::CString error;
    contract = builder.Build( error );
    if ( contract.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteTestContract: " << error );
        return contract;
    }
    registry.Register( contract, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_BUILTIN );
    return contract;
}

/*-------------------------------------------------------------------------*/

/**
 *  The product side of the tests: a mutation with a command id answered at most once (the outcome cache in memory and
 *  the command journal on disk), a call that answers late, and uploads into a CUploadStaging. Registered on a host
 */
class CRemoteTestService : public CORE::CObservingNotifier
{
    public:

    DRNUTILS::CRemoteServiceHost* host;
    DRNUTILS::CCommandOutcomeCache cache;
    DRNUTILS::CCommandJournal journal;
    DRNUTILS::CUploadStaging staging;
    UInt64 revision;
    UInt32 applied;
    UInt32 replayedFromCache;
    UInt32 replayedFromJournal;
    UInt32 bulkWrites;
    UInt32 controlWrites;

    CRemoteTestService( void )
        : CORE::CObservingNotifier(), host( GUCEF_NULL ), cache(), journal(), staging(), revision( 0 ), applied( 0 ), replayedFromCache( 0 )
        , replayedFromJournal( 0 ), bulkWrites( 0 ), controlWrites( 0 ), m_commands(), m_uploads(), m_delayed(), m_timer()
    {
        typedef COMCORE::CTRpcMethodHandler< CRemoteTestService > THandler;
        m_commands.BindMethod( "Apply", GUCEF_NEW THandler( this, &CRemoteTestService::OnApply ) );
        m_commands.BindMethod( "Slow", GUCEF_NEW THandler( this, &CRemoteTestService::OnSlow ) );
        m_uploads.BindMethod( "Begin", GUCEF_NEW THandler( this, &CRemoteTestService::OnBegin ) );
        m_uploads.BindMethod( "Write", GUCEF_NEW THandler( this, &CRemoteTestService::OnWrite ) );
        m_uploads.BindMethod( "Query", GUCEF_NEW THandler( this, &CRemoteTestService::OnQuery ) );
        m_uploads.BindMethod( "Commit", GUCEF_NEW THandler( this, &CRemoteTestService::OnCommit ) );
    }

    virtual ~CRemoteTestService()
    {
        m_timer.SetEnabled( false );
        UnsubscribeAllFromObserver();
        m_delayed.clear();
    }

    bool Attach( DRNUTILS::CRemoteServiceHost& serviceHost, const CORE::CString& directory, CORE::CString& error )
    {
        host = &serviceHost;
        COMCORE::CRpcContractPtr contract = RemoteTestContract();
        if ( contract.IsNULL() || !serviceHost.RegisterService( contract, "Commands", &m_commands ) || !serviceHost.RegisterService( contract, "Uploads", &m_uploads ) )
        {
            error = "the test services cannot be registered";
            return false;
        }
        serviceHost.GetAccessRules().AddRule( "gucef.drnutils.test.remote.*", "", "" );
        UInt8 key[ 32 ];
        memset( key, 0x42, sizeof( key ) );
        journal.SetKey( key, sizeof( key ) );
        if ( !journal.Open( CORE::CombinePath( directory, "commands" ), error ) )
            return false;
        DRNUTILS::CUploadStaging::SSettings settings;
        settings.directory = CORE::CombinePath( directory, "staging" );
        settings.maxBytesPerSubject = 64 * 1024 * 1024;
        if ( !staging.Open( settings, error ) )
            return false;
        m_timer.SetPulseGenerator( serviceHost.GetPulseGenerator() );
        m_timer.SetInterval( 10 );
        typedef CORE::CTEventHandlerFunctor< CRemoteTestService > TEventCallback;
        TEventCallback onTick( this, &CRemoteTestService::OnTick );
        SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, onTick );
        m_timer.SetEnabled( true );
        return true;
    }

    UInt32 GetDelayedCount( void ) const { return (UInt32) m_delayed.size(); }

    /** Slow calls made with delayMs 0 wait until this releases them (answered at the next tick); how many it released */
    UInt32 ReleaseHeld( void )
    {
        UInt32 released = 0;
        for ( UInt32 i=0; i<m_delayed.size(); ++i )
        {
            if ( HeldDueMs() == m_delayed[ i ].dueMs )
            {
                m_delayed[ i ].dueMs = 0;
                ++released;
            }
        }
        return released;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString name = "DRNUTILSTEST::CRemoteTestService";
        return name;
    }

    private:

    /** The due time of a held call: never, until ReleaseHeld() */
    static UInt64 HeldDueMs( void ) { return ~( (UInt64) 0 ); }

    struct SDelayed
    {
        COMCORE::CRpcServerCallPtr call;
        UInt64 dueMs;
    };

    COMCORE::TRpcHandlerResult OnApply( COMCORE::CRpcServerCall& call )
    {
        DRNUTILS::CRemoteServicePrincipal principal;
        if ( !host->GetCallPrincipal( call, principal ) )
        {
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, "no device" ) );
            return COMCORE::RPC_HANDLER_DONE;
        }
        if ( host->RefuseIfDraining( call ) )
            return COMCORE::RPC_HANDLER_DONE;
        CORE::CString commandId;
        CORE::CString reason;
        if ( !DRNUTILS::CCommandOutcomeCache::GetCommandId( call, 1, commandId, reason ) )
        {
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INVALID_ARGUMENT, reason ) );
            return COMCORE::RPC_HANDLER_DONE;
        }
        const CORE::CString cacheHash = cache.HashRequest( call.GetRequest(), 1 );
        const CORE::CString journalHash = journal.HashRequest( call.GetRequest(), 1 );
        DRNUTILS::CCommandOutcomeCache::SOutcome kept;
        DRNUTILS::CCommandOutcomeCache::ELookup lookup = cache.Find( principal.deviceId, commandId, cacheHash, RemoteTestNowMs(), kept );
        if ( DRNUTILS::CCommandOutcomeCache::COMMAND_REPLAY == lookup )
        {
            ++replayedFromCache;
            DRNUTILS::CCommandOutcomeCache::AnswerReplay( call, kept, 2 );
            return COMCORE::RPC_HANDLER_DONE;
        }
        if ( DRNUTILS::CCommandOutcomeCache::COMMAND_REUSED == lookup )
        {
            DRNUTILS::CCommandOutcomeCache::RefuseReused( call, commandId );
            return COMCORE::RPC_HANDLER_DONE;
        }
        DRNUTILS::CCommandJournal::SEntry entry;
        lookup = journal.Lookup( principal.deviceId, commandId, journalHash, entry );
        if ( DRNUTILS::CCommandOutcomeCache::COMMAND_REPLAY == lookup )
        {
            ++replayedFromJournal;
            COMCORE::CRpcMessage response = call.NewResponse();
            response.SetBool( 1, true );
            response.SetBool( 2, true );
            response.SetUInt64( 3, entry.outcome.GetAttributeValueOrChildValueByName( "revision" ).AsUInt64( 0, true ) );
            call.Reply( response );
            return COMCORE::RPC_HANDLER_DONE;
        }
        if ( DRNUTILS::CCommandOutcomeCache::COMMAND_REUSED == lookup )
        {
            DRNUTILS::CCommandOutcomeCache::RefuseReused( call, commandId );
            return COMCORE::RPC_HANDLER_DONE;
        }
        ++revision;
        ++applied;
        COMCORE::CRpcMessage response = call.NewResponse();
        response.SetBool( 1, true );
        response.SetBool( 2, false );
        response.SetUInt64( 3, revision );
        DRNUTILS::CCommandOutcomeCache::SOutcome outcome;
        DRNUTILS::CCommandOutcomeCache::MakeOutcome( COMCORE::CRpcStatus::Ok, &response, outcome );
        cache.Remember( principal.deviceId, commandId, cacheHash, RemoteTestNowMs(), outcome );
        entry = DRNUTILS::CCommandJournal::SEntry();
        entry.atUtcMs = DRNUTILS::CRemoteServiceHost::NowUtcMs();
        entry.subject = principal.deviceId;
        entry.commandId = commandId;
        entry.requestHash = journalHash;
        entry.method = "gucef.drnutils.test.remote.Commands/Apply";
        entry.code = COMCORE::RPC_STATUS_OK;
        entry.outcome.SetName( "outcome" );
        entry.outcome.SetAttribute( "revision", revision );
        CORE::CString error;
        journal.Record( entry, error );
        call.Reply( response );
        return COMCORE::RPC_HANDLER_DONE;
    }

    COMCORE::TRpcHandlerResult OnSlow( COMCORE::CRpcServerCall& call )
    {
        SDelayed delayed;
        delayed.call = call.GetSharedPtr();
        const UInt32 delayMs = call.GetRequest().GetUInt32( 1 );
        delayed.dueMs = 0 == delayMs ? HeldDueMs() : RemoteTestNowMs() + delayMs;
        m_delayed.push_back( delayed );
        host->AddDrainHold();
        return COMCORE::RPC_HANDLER_DEFERRED;
    }

    void OnTick( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
    {
        const UInt64 now = RemoteTestNowMs();
        for ( UInt32 i=0; i<m_delayed.size(); )
        {
            if ( m_delayed[ i ].dueMs <= now )
            {
                COMCORE::CRpcMessage response = m_delayed[ i ].call->NewResponse();
                response.SetBool( 1, true );
                m_delayed[ i ].call->Reply( response );
                m_delayed.erase( m_delayed.begin() + i );
                host->ReleaseDrainHold();
            }
            else
            {
                ++i;
            }
        }
    }

    COMCORE::TRpcHandlerResult OnBegin( COMCORE::CRpcServerCall& call )
    {
        DRNUTILS::CRemoteServicePrincipal principal;
        if ( !host->GetCallPrincipal( call, principal ) )
        {
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, "no device" ) );
            return COMCORE::RPC_HANDLER_DONE;
        }
        const COMCORE::CRpcMessage& request = call.GetRequest();
        DRNUTILS::CUploadStaging::TFileSpecVector files;
        for ( UInt32 i=0; i<request.GetCount( 2 ); ++i )
        {
            DRNUTILS::CUploadStaging::SFileSpec spec;
            CORE::CVariant value;
            request.GetAt( 2, i, value );
            spec.relativePath = value.AsString();
            request.GetAt( 3, i, value );
            spec.size = value.AsUInt64();
            request.GetAt( 4, i, value );
            spec.sha256Hex = value.AsString();
            files.push_back( spec );
        }
        CORE::CString uploadId;
        CORE::CString detail;
        CORE::CString message;
        COMCORE::CRpcMessage response = call.NewResponse();
        bool ok = staging.Begin( principal.deviceId, request.GetString( 1 ), files, DRNUTILS::CRemoteServiceHost::NowUtcMs(), uploadId, detail, message );
        response.SetBool( 3, ok );
        response.SetString( 1, uploadId );
        response.SetString( 2, detail );
        response.SetString( 6, principal.channel );
        call.Reply( response );
        return COMCORE::RPC_HANDLER_DONE;
    }

    COMCORE::TRpcHandlerResult OnWrite( COMCORE::CRpcServerCall& call )
    {
        DRNUTILS::CRemoteServicePrincipal principal;
        if ( !host->GetCallPrincipal( call, principal ) )
        {
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, "no device" ) );
            return COMCORE::RPC_HANDLER_DONE;
        }
        if ( "bulk" == principal.channel )
            ++bulkWrites;
        else
            ++controlWrites;
        const COMCORE::CRpcMessage& request = call.GetRequest();
        CORE::CDynamicBuffer data;
        request.GetBytes( 4, data );
        CORE::CString detail;
        CORE::CString message;
        bool ok = staging.WriteBlock( principal.deviceId, request.GetString( 1 ), request.GetUInt32( 2 ), request.GetUInt64( 3 ),
                                      data.GetConstBufferPtr(), data.GetDataSize(), DRNUTILS::CRemoteServiceHost::NowUtcMs(), detail, message );
        COMCORE::CRpcMessage response = call.NewResponse();
        response.SetBool( 3, ok );
        response.SetString( 2, detail );
        response.SetString( 6, principal.channel );
        call.Reply( response );
        return COMCORE::RPC_HANDLER_DONE;
    }

    COMCORE::TRpcHandlerResult OnQuery( COMCORE::CRpcServerCall& call )
    {
        DRNUTILS::CRemoteServicePrincipal principal;
        host->GetCallPrincipal( call, principal );
        DRNUTILS::CUploadStaging::SUploadInfo info;
        COMCORE::CRpcMessage response = call.NewResponse();
        bool ok = staging.Query( principal.deviceId, call.GetRequest().GetString( 1 ), info );
        response.SetBool( 3, ok );
        response.SetUInt64( 4, info.receivedBytes );
        for ( UInt32 i=0; i<info.received.size(); ++i )
            response.Add( 5, CORE::CVariant( info.received[ i ].ToString() ) );
        call.Reply( response );
        return COMCORE::RPC_HANDLER_DONE;
    }

    COMCORE::TRpcHandlerResult OnCommit( COMCORE::CRpcServerCall& call )
    {
        DRNUTILS::CRemoteServicePrincipal principal;
        host->GetCallPrincipal( call, principal );
        CORE::CString staged;
        CORE::CString detail;
        CORE::CString message;
        COMCORE::CRpcMessage response = call.NewResponse();
        bool ok = staging.Commit( principal.deviceId, call.GetRequest().GetString( 1 ), staged, detail, message );
        response.SetBool( 3, ok );
        response.SetString( 2, detail );
        call.Reply( response );
        return COMCORE::RPC_HANDLER_DONE;
    }

    COMCORE::CRpcServiceImpl m_commands;
    COMCORE::CRpcServiceImpl m_uploads;
    GUCEF::vector< SDelayed > m_delayed;
    CORE::CTimer m_timer;
};

/*-------------------------------------------------------------------------*/

/**
 *  A remote service host on the loopback (its DRN node on a thread of its own, everything else on the global pulse
 *  generator the test thread drives), with a fault proxy in front of its session port, a test service, pairing of
 *  devices and clients that connect through the proxy
 */
class CRemoteWorld
{
    public:

    CORE::PulseGeneratorPtr pulse;
    TThreadId previousDriver;
    DRNUTILS::CRemoteServiceHostSettings settings;
    DRNUTILS::CRemoteServiceHost* host;
    CRemoteTestService* service;
    CTestTcpFaultProxy proxy;
    CORE::CString directory;

    explicit CRemoteWorld( const CORE::CString& name )
        : pulse(), previousDriver( 0 ), settings(), host( GUCEF_NULL ), service( GUCEF_NULL ), proxy(), directory()
    {
        pulse = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        previousDriver = pulse->GetPulseDriverThreadId();
        pulse->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        directory = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( directory, "remoteservice_" + name );
        CORE::DeleteDir( directory, true );
        CORE::CreateDirs( directory );

        // short times for the tests; keys fixed so a restarted host is the same service
        settings.serviceName = "gucef.test.remote";
        settings.drnSessionName = "remote-test";
        settings.sessionPort = TESTFW::FindFreeTcpAndUdpPort( 0 );
        settings.bindAddress = "127.0.0.1";
        settings.linkTimeoutMs = 1500;
        settings.echoIntervalMs = 250;
        settings.tickIntervalMs = 20;
        settings.drainSeconds = 1;
        settings.expectedBackSeconds = 2;
        settings.staticPrivateKeyHex = "5a1b2c3d4e5f60718293a4b5c6d7e8f90112233445566778899aabbccddeeff0";
        settings.issuerSeedHex = "7766554433221100ffeeddccbbaa99887766554433221100ffeeddccbbaa9988";
    }

    ~CRemoteWorld()
    {
        StopHost();
        proxy.Stop();
        PumpFor( 20 );
        pulse->SetPulseDriverThreadId( previousDriver );
    }

    bool StartHost( void )
    {
        EnsureCryptoCodecPluginLoaded();
        RpcTestJsonCodecAvailable();
        // the proxy first: the host tells its clients the proxy, so no client ever bypasses the faults
        if ( 0 == proxy.GetPort() && !proxy.Start( pulse, settings.sessionPort ) )
            return false;
        settings.sessionEndpoints.clear();
        settings.sessionEndpoints.push_back( "tcp://" + proxy.GetHostAndPort() );
        host = GUCEF_NEW DRNUTILS::CRemoteServiceHost( pulse, settings );
        host->SetStateDirectory( CORE::CombinePath( directory, "state" ) );
        CORE::CString error;
        if ( !host->Start( error ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RemoteWorld: the host does not start: " << error );
            return false;
        }
        service = GUCEF_NEW CRemoteTestService();
        if ( !service->Attach( *host, directory, error ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RemoteWorld: the test service does not start: " << error );
            return false;
        }
        return true;
    }

    void StopHost( void )
    {
        if ( GUCEF_NULL != service )
        {
            GUCEF_DELETE service;
            service = GUCEF_NULL;
        }
        if ( GUCEF_NULL != host )
        {
            host->Stop();
            PumpFor( 20 );
            GUCEF_DELETE host;
            host = GUCEF_NULL;
        }
    }

    /** Pairs a device through a window of the host; with 'confirm' the observer of the client confirms the fingerprint */
    bool Pair( const CORE::CString& kind, const char* roles, DRNUTILS::CDeviceCredentials& credentials, CORE::CString& error,
               const CORE::CString& codeOverride = CORE::CString::Empty, DRNUTILS::CDevicePairingClient::TPairingFailure* failure = GUCEF_NULL )
    {
        DRNUTILS::CPairingWindowRequest window;
        window.deviceKind = kind;
        CORE::CString::StringVector list = CORE::CString( roles ).ParseElements( ',', false );
        for ( UInt32 i=0; i<list.size(); ++i )
            window.roles.insert( list[ i ] );
        CORE::CString windowId;
        CORE::CString code;
        UInt64 expires = 0;
        if ( !host->IsPairingWindowOpen() && !host->OpenPairingWindow( window, DRNUTILS::CRemoteServiceHost::LocalAdmin, windowId, code, expires, error ) )
            return false;
        return PairWithCode( codeOverride.IsNULLOrEmpty() ? code : codeOverride, credentials, error, failure );
    }

    bool PairWithCode( const CORE::CString& code, DRNUTILS::CDeviceCredentials& credentials, CORE::CString& error,
                       DRNUTILS::CDevicePairingClient::TPairingFailure* failure = GUCEF_NULL )
    {
        DRNUTILS::CDevicePairingClient client( pulse );
        DRNUTILS::CDevicePairingClient::SRequest request;
        request.pairingEndpoint = "tcp://127.0.0.1:" + CORE::ToString( host->GetPairingPort() );
        request.code = code;
        request.serviceName = settings.serviceName;
        request.drnSessionName = settings.drnSessionName;
        request.clientKind = "test";
        request.deviceName = "test device";
        request.platform = "linux";
        request.appVersion = "1.0";
        request.connectTimeoutMs = 5000;
        request.confirmationTimeoutMs = 10000;
        CConfirmer confirmer( *this );
        client.Subscribe( &confirmer );
        bool paired = client.Pair( request, credentials, error );
        client.Unsubscribe( &confirmer );
        if ( GUCEF_NULL != failure )
            *failure = client.GetLastFailure();
        return paired;
    }

    /** A client of the world through the proxy */
    DRNUTILS::CRemoteServiceConnectionSettings ClientSettings( UInt32 bulkChannels = 0 ) const
    {
        DRNUTILS::CRemoteServiceConnectionSettings client;
        client.clientKind = "test";
        client.clientVersion = "1.0";
        client.wantedContracts.push_back( "gucef.drnutils.test.remote" );
        client.bulkContracts.push_back( "gucef.drnutils.test.remote" );
        client.bulkChannels = bulkChannels;
        client.hostOverride = proxy.GetHostAndPort();
        client.connectTimeoutMs = 10000;
        client.linkTimeoutMs = 1500;
        client.echoIntervalMs = 250;
        client.reconnectBaseMs = 200;
        client.reconnectMaxMs = 2000;
        return client;
    }

    bool WaitFor( const DRNUTILS::CRemoteServiceConnection& connection, DRNUTILS::TRemoteConnectionState state, UInt32 timeoutMs )
    {
        CConnectionState condition( connection, state );
        return PumpUntil( condition, timeoutMs );
    }

    /** Confirms a waiting pairing whose fingerprint is the one the client shows */
    class CConfirmer : public CORE::CObserver
    {
        public:

        CRemoteWorld& world;
        CORE::CString fingerprint;
        UInt32 confirmed;

        explicit CConfirmer( CRemoteWorld& w ) : CORE::CObserver(), world( w ), fingerprint(), confirmed( 0 ) {}
        virtual ~CConfirmer() { SignalUpcomingObserverDestruction(); }

        virtual void OnNotify( CORE::CNotifier*, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
        {
            if ( DRNUTILS::CDevicePairingClient::FingerprintEvent == eventId && GUCEF_NULL != eventData )
                fingerprint = static_cast< CORE::TCloneableString* >( eventData )->GetData();
            else
            if ( DRNUTILS::CDevicePairingClient::AwaitingConfirmationEvent == eventId )
            {
                DRNUTILS::CDeviceRegistry::TDeviceRecordVector pending;
                world.host->GetPendingPairings( pending );
                for ( UInt32 i=0; i<pending.size(); ++i )
                {
                    CORE::CString error;
                    if ( world.host->ConfirmPairing( pending[ i ].deviceId, fingerprint, true, "test-owner", error ) )
                        ++confirmed;
                }
            }
        }
    };
};

/*-------------------------------------------------------------------------*/

/** Collects the session events of a connection */
class CEventCollector : public CORE::CObserver
{
    public:

    GUCEF::vector< UInt64 > sequences;
    GUCEF::vector< CORE::CString > kinds;
    UInt32 resyncs;
    UInt32 replaced;
    UInt32 skews;
    UInt32 stopping;                            /**< ServiceStoppingEvent of the connection */
    GUCEF::vector< Int32 > states;              /**< the states the connection entered, in order */
    GUCEF::vector< UInt64 > stateTimesMs;       /**< when it entered them (RemoteTestNowMs(), taken in the event, on the connection's thread) */

    CEventCollector( void ) : CORE::CObserver(), sequences(), kinds(), resyncs( 0 ), replaced( 0 ), skews( 0 ), stopping( 0 ), states(), stateTimesMs() {}

    /** The time the connection entered the state at or after the index of states, 0 when it did not (yet); 'index' receives its index */
    UInt64 StateEnteredAt( DRNUTILS::TRemoteConnectionState state, UInt32 fromIndex, UInt32* index = GUCEF_NULL ) const
    {
        for ( UInt32 i=fromIndex; i<states.size(); ++i )
        {
            if ( (Int32) state == states[ i ] )
            {
                if ( GUCEF_NULL != index )
                    *index = i;
                return stateTimesMs[ i ];
            }
        }
        return 0;
    }
    virtual ~CEventCollector() { SignalUpcomingObserverDestruction(); }

    bool InOrderWithoutDuplicates( void ) const
    {
        for ( UInt32 i=1; i<sequences.size(); ++i )
        {
            if ( sequences[ i ] <= sequences[ i-1 ] )
                return false;
        }
        return true;
    }

    virtual void OnNotify( CORE::CNotifier*, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( DRNUTILS::CRemoteServiceConnection::SessionEventEvent == eventId && GUCEF_NULL != eventData )
        {
            const COMCORE::CRpcMessage& event = static_cast< DRNUTILS::TRemoteMessageEventData* >( eventData )->GetData();
            sequences.push_back( event.GetUInt64( 1 ) );
            kinds.push_back( event.GetString( 2 ) );
        }
        else if ( DRNUTILS::CRemoteServiceConnection::ResyncEvent == eventId )
            ++resyncs;
        else if ( DRNUTILS::CRemoteServiceConnection::SessionReplacedEvent == eventId )
            ++replaced;
        else if ( DRNUTILS::CRemoteServiceConnection::ClockSkewEvent == eventId )
            ++skews;
        else if ( DRNUTILS::CRemoteServiceConnection::ServiceStoppingEvent == eventId )
            ++stopping;
        else if ( DRNUTILS::CRemoteServiceConnection::StateChangedEvent == eventId && GUCEF_NULL != eventData )
        {
            states.push_back( (Int32) static_cast< DRNUTILS::TRemoteConnectionStateEventData* >( eventData )->GetData() );
            stateTimesMs.push_back( RemoteTestNowMs() );
        }
    }
};

}; /* namespace DRNUTILSTEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_TESTDRNUTILSREMOTEWORLD_H ? */
