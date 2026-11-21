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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>

#include <exception>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#include "gucefCORE_CNativeFileSystemAccess.h"
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDRNTestUtils.h"
#include "TestDRNGroup.h"
#include "TestDRNLimits.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace DRNTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

typedef GUCEF::vector< DRN::CDRNNamespaceEntry > TEntries;
typedef CORE::CString::StringVector TStrings;

static const char* const LimitsPoolName = "GUCEF.DRNTEST.Limits";
static const UInt32 ChunkBytes = DRN::DRN_RESOURCE_CHUNK_SIZE;

/*-------------------------------------------------------------------------*/

UInt64
NowMs( void )
{
    return DRN::CDRNClock::NowInMilliseconds();
}

/*-------------------------------------------------------------------------*/

CORE::CString
TestDirectory( const CORE::CString& name )
{
    CORE::CString dir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "drnlim_test" );
    dir = CORE::CombinePath( dir, name );
    CORE::CreateDirs( dir );
    return dir;
}

/*-------------------------------------------------------------------------*/

void
RemoveTestDirectories( void )
{
    CORE::CString dir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "drnlim_test" );
    if ( CORE::DirExists( dir ) )
        CORE::DeleteDir( dir, true );
}

/*-------------------------------------------------------------------------*/

UInt32
LocalPeerOf( DRN::CDRNSession& session )
{
    MT::CScopeMutex lock( session.GetNode().GetLock() );
    return session.GetLocalPeerId();
}

/*-------------------------------------------------------------------------*/

bool
WriteFile( const CORE::CString& path, UInt32 size )
{
    CORE::CFileAccess file;
    if ( !file.Open( path, "wb" ) )
        return false;
    CORE::CDynamicBuffer block( size + 1, true );
    block.SetDataSize( size );
    for ( UInt32 i=0; i<size; ++i )
        static_cast< UInt8* >( block.GetBufferPtr() )[ i ] = (UInt8) ( 3 + i * 7 );
    bool ok = 0 == size || size == file.Write( block.GetConstBufferPtr(), 1, size );
    file.Close();
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
Contains( const CORE::CString& text, const CORE::CString& part )
{
    return GUCEF_NULL != strstr( text.C_String(), part.C_String() );
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessPtr
NativeFileSystem( void )
{
    return ( GUCEF_NEW CORE::CNativeFileSystemAccess() )->CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

DRN::CDRNResourceShareRules
ShareAll( void )
{
    return DRN::CDRNResourceShareRules( DRN::CDRNResourceShareRules::WholeProvider, DRN::CDRNResourceShareRules::AnyRole );
}

/*-------------------------------------------------------------------------*/

TStrings
Strings1( const char* a )
{
    TStrings result;
    result.push_back( a );
    return result;
}

/*-------------------------------------------------------------------------*/

/** A pool for the provider work of the serving side with enough threads for the jobs a test holds blocked at once */
void
EnsureLimitsPool( void )
{
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( LimitsPoolName );
    if ( !pool.IsNULL() && pool->GetDesiredMinNrOfWorkerThreads() < 24 )
        pool->SetDesiredMinNrOfWorkerThreads( 24 );
}

/*-------------------------------------------------------------------------*/

/** The settings of the serving side of these tests: every limit off, so that a test sets exactly the ones it is about */
DRN::CDRNResourceTransferSettings
BaseSettings( void )
{
    DRN::CDRNResourceTransferSettings settings;
    settings.threadPoolName = LimitsPoolName;
    settings.maxOpenHandlesPerPeer = 0;
    settings.maxOpenHandles = 0;
    settings.maxActiveReadsPerPeer = 0;
    settings.maxActiveReads = 0;
    settings.maxNamespaceRequestsPerPeer = 0;
    settings.maxNamespaceRequests = 0;
    settings.maxNamespaceListingsPerPeer = 0;
    settings.maxNamespaceListings = 0;
    settings.maxUploadBytesPerSecond = 0;
    settings.maxUploadBytesPerSecondPerPeer = 0;
    settings.maxBufferedBytesPerPeer = 0;
    settings.handleIdleTimeoutMs = 0;
    return settings;
}

/*-------------------------------------------------------------------------*/

/** A share whose reads and stats can be held, made slow or made to fail, and that counts the stats under way */
class CGateProvider : public DRN::CDRNFileSystemAccessResourceProvider
{
    public:

    MT::CMutex m_lock;
    bool m_blockReads;
    bool m_blockStats;
    bool m_failReads;
    UInt32 m_statDelayMs;
    UInt32 m_statsInside;
    UInt32 m_statsPeak;

    explicit CGateProvider( const CORE::CString& root )
        : DRN::CDRNFileSystemAccessResourceProvider( NativeFileSystem(), root, CORE::CUri( "file://" + root ) )
        , m_lock(), m_blockReads( false ), m_blockStats( false ), m_failReads( false ), m_statDelayMs( 0 ), m_statsInside( 0 ), m_statsPeak( 0 )
    {}

    void Block( bool reads, bool stats )
    {
        MT::CScopeMutex lock( m_lock );
        m_blockReads = reads;
        m_blockStats = stats;
    }

    void Open( void ) { Block( false, false ); }

    void SetFailReads( bool fail )
    {
        MT::CScopeMutex lock( m_lock );
        m_failReads = fail;
    }

    void SetStatDelay( UInt32 delayMs )
    {
        MT::CScopeMutex lock( m_lock );
        m_statDelayMs = delayMs;
    }

    UInt32 GetStatsPeak( void )
    {
        MT::CScopeMutex lock( m_lock );
        return m_statsPeak;
    }

    void ResetStatsPeak( void )
    {
        MT::CScopeMutex lock( m_lock );
        m_statsPeak = m_statsInside;
    }

    virtual DRN::TDRNResourceStatus Read( const CORE::CString& name, const DRN::CDRNResourceInfo& resolved, UInt64 offset, UInt32 size, void* destination, UInt32& bytesRead ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 waited=0; waited<60000 && IsBlocked( true ); waited+=2 )
            MT::PrecisionDelay( 2 );
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_failReads )
                return DRN::DRN_RESSTATUS_IO_ERROR;
        }
        return DRN::CDRNFileSystemAccessResourceProvider::Read( name, resolved, offset, size, destination, bytesRead );
    }

    virtual DRN::TDRNResourceStatus Stat( const CORE::CString& name, DRN::CDRNNamespaceEntry& entry ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 delay = 0;
        {
            MT::CScopeMutex lock( m_lock );
            ++m_statsInside;
            if ( m_statsInside > m_statsPeak )
                m_statsPeak = m_statsInside;
            delay = m_statDelayMs;
        }
        for ( UInt32 waited=0; waited<60000 && IsBlocked( false ); waited+=2 )
            MT::PrecisionDelay( 2 );
        if ( 0 != delay )
            MT::PrecisionDelay( delay );
        DRN::TDRNResourceStatus status = DRN::CDRNFileSystemAccessResourceProvider::Stat( name, entry );
        {
            MT::CScopeMutex lock( m_lock );
            --m_statsInside;
        }
        return status;
    }

    private:

    bool IsBlocked( bool reads )
    {
        MT::CScopeMutex lock( m_lock );
        return reads ? m_blockReads : m_blockStats;
    }
};

/*-------------------------------------------------------------------------*/

/** A job that runs on a thread of its own, once */
class CJob
{
    public:
    virtual ~CJob() {}
    virtual void Run( void ) = 0;
};

/*-------------------------------------------------------------------------*/

class CJobThread : public MT::CActiveObject
{
    public:

    explicit CJobThread( CJob& job ) : MT::CActiveObject(), m_job( job ), m_done( false ) {}

    virtual ~CJobThread() { Deactivate( false, true ); }

    bool Start( void ) { return Activate( GUCEF_NULL, 0, 0 ); }

    bool IsDone( void ) const { return m_done.Load(); }

    /** Waits for the end of the job; false when it did not end in time */
    bool Join( UInt32 timeoutMs )
    {
        UInt64 start = NowMs();
        while ( !IsDone() && NowMs() - start < timeoutMs )
            MT::PrecisionDelay( 2 );
        return IsDone();
    }

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_job.Run();
        m_done.Store( true );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    CJob& m_job;
    MT::CTAtomic< bool > m_done;
};

/*-------------------------------------------------------------------------*/

/** What a test learns of a call: whether it was refused as BUSY (and the words), or how it ended */
class CCallOutcome
{
    public:

    MT::CMutex lock;
    bool done;
    bool ok;
    bool busy;
    bool timedOut;
    CORE::CString message;

    CCallOutcome( void ) : lock(), done( false ), ok( false ), busy( false ), timedOut( false ), message() {}

    void Set( bool isOk, bool isBusy, bool isTimedOut, const CORE::CString& text )
    {
        MT::CScopeMutex scope( lock );
        ok = isOk;
        busy = isBusy;
        timedOut = isTimedOut;
        message = text;
        done = true;
    }

    bool IsDone( void )
    {
        MT::CScopeMutex scope( lock );
        return done;
    }
};

/*-------------------------------------------------------------------------*/

/** One read of a handle: ReadRange() of the first bytes */
class CReadJob : public CJob
{
    public:

    DRN::CDRNResourceHandlePtr handle;
    UInt32 length;
    CCallOutcome outcome;

    explicit CReadJob( const DRN::CDRNResourceHandlePtr& h, UInt32 bytes = 1000 ) : handle( h ), length( bytes ), outcome() {}

    virtual void Run( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CDynamicBuffer data;
        DRN::CDRNResourceTransferResult result = handle->ReadRange( 0, length, data );
        outcome.Set( result.IsOk(), DRN::DRN_RESSTATUS_BUSY == result.status, DRN::DRN_RESSTATUS_TIMEOUT == result.status, result.message );
    }
};

/*-------------------------------------------------------------------------*/

/** One STAT of a share through a client */
class CStatJob : public CJob
{
    public:

    DRN::CDRNNamespaceClient& client;
    CORE::CString share;
    UInt32 repeat;
    UInt32 timeoutMs;
    UInt32 okCount;
    UInt32 busyCount;
    UInt32 timeoutCount;
    UInt32 otherCount;
    MT::CTAtomic< UInt32 >* orderCounter;
    UInt32 finishedAs;
    CCallOutcome outcome;

    CStatJob( DRN::CDRNNamespaceClient& c, const CORE::CString& s, UInt32 times = 1, UInt32 timeout = 30000 )
        : client( c ), share( s ), repeat( times ), timeoutMs( timeout ), okCount( 0 ), busyCount( 0 ), timeoutCount( 0 ), otherCount( 0 )
        , orderCounter( GUCEF_NULL ), finishedAs( 0 ), outcome()
    {}

    virtual void Run( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CString lastMessage;
        bool lastBusy = false;
        bool lastOk = false;
        bool lastTimeout = false;
        for ( UInt32 i=0; i<repeat; ++i )
        {
            TEntries entries;
            DRN::CDRNNamespaceResult result = client.Stat( share, Strings1( "f0.bin" ), TStrings(), entries, timeoutMs );
            lastMessage = result.message;
            lastOk = result.IsOk();
            lastBusy = DRN::DRN_NSRESULT_BUSY == result.code;
            lastTimeout = DRN::DRN_NSRESULT_TIMEOUT == result.code;
            if ( lastOk )
                ++okCount;
            else
            if ( lastBusy )
                ++busyCount;
            else
            if ( lastTimeout )
                ++timeoutCount;
            else
                ++otherCount;
        }
        if ( GUCEF_NULL != orderCounter )
            finishedAs = orderCounter->FetchAdd( 1 );
        outcome.Set( lastOk, lastBusy, lastTimeout, lastMessage );
    }
};

/*-------------------------------------------------------------------------*/

/** ListAll() of a directory through a client */
class CListAllJob : public CJob
{
    public:

    DRN::CDRNNamespaceClient& client;
    CORE::CString dir;
    UInt32 pageEntries;
    UInt32 timeoutMs;
    UInt32 entryCount;
    CCallOutcome outcome;

    CListAllJob( DRN::CDRNNamespaceClient& c, const CORE::CString& d, UInt32 page, UInt32 timeout = 30000 )
        : client( c ), dir( d ), pageEntries( page ), timeoutMs( timeout ), entryCount( 0 ), outcome() {}

    virtual void Run( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TEntries entries;
        CORE::CString revision;
        bool changed = false;
        DRN::CDRNNamespaceResult result = client.ListAll( "files", dir, entries, revision, changed, 1000000, pageEntries, 2, timeoutMs );
        entryCount = (UInt32) entries.size();
        outcome.Set( result.IsOk(), DRN::DRN_NSRESULT_BUSY == result.code, DRN::DRN_NSRESULT_TIMEOUT == result.code, result.message );
    }
};

/*-------------------------------------------------------------------------*/

/** Samples a counter of the serving side while a test runs, and keeps the highest */
class CSampler : public CJob
{
    public:

    DRN::CDRNResourceSessionComponent& server;
    bool listings;
    MT::CTAtomic< bool > stop;
    UInt32 highest;

    CSampler( DRN::CDRNResourceSessionComponent& s, bool countListings ) : server( s ), listings( countListings ), stop( false ), highest( 0 ) {}

    virtual void Run( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        while ( !stop.Load() )
        {
            UInt32 value = listings ? server.GetNamespaceListingCount( 0 ) : server.GetNamespaceServingCount( 0 );
            if ( value > highest )
                highest = value;
            MT::PrecisionDelay( 1 );
        }
    }
};

/*-------------------------------------------------------------------------*/

/** A session of a host and members, the host serving the shares "files" (plain) and "gate" (reads and stats held at will) */
class CLimitFixture
{
    public:

    CGroup group;
    CORE::CString root;
    UInt32 serverPeer;
    UInt32 memberCount;
    CGateProvider* gate;
    GUCEF::vector< DRN::CDRNNamespaceClient* > clients;

    CLimitFixture( const CORE::CString& name, UInt32 members = 2 )
        : group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, name, "tcp" ), root(), serverPeer( 0 ), memberCount( members ), gate( GUCEF_NULL ), clients()
    {
        group.nodeSettings.ownThread = true;
    }

    ~CLimitFixture()
    {
        if ( GUCEF_NULL != gate )
            gate->Open();
        for ( UInt32 i=0; i<clients.size(); ++i )
            GUCEF_DELETE clients[ i ];
        clients.clear();
        group.Stop();
    }

    bool Start( void )
    {
        EnsureLimitsPool();
        root = TestDirectory( "root" );
        if ( !CORE::FileExists( CORE::CombinePath( root, "f0.bin" ) ) )
        {
            for ( UInt32 i=0; i<10; ++i )
                if ( !WriteFile( CORE::CombinePath( root, "f" + CORE::ToString( i ) + ".bin" ), 3 * ChunkBytes + 5 ) )
                    return false;
            if ( !WriteFile( CORE::CombinePath( root, "u.bin" ), 24 * ChunkBytes ) )
                return false;
            CORE::CString big = CORE::CombinePath( root, "big" );
            CORE::CreateDirs( big );
            for ( UInt32 i=0; i<200; ++i )
                if ( !WriteFile( CORE::CombinePath( big, "b" + CORE::ToString( 1000 + i ) + ".bin" ), 10 ) )
                    return false;
        }
        if ( !group.Start( 1 + memberCount ) )
            return false;
        serverPeer = LocalPeerOf( group[ 0 ] );
        DRN::CDRNResourceTransferSettings settings = BaseSettings();
        Server().SetSettings( settings );
        DRN::CIDRNResourceProviderPtr plain( GUCEF_NEW DRN::CDRNFileSystemAccessResourceProvider( NativeFileSystem(), root, CORE::CUri( "file://" + root ) ) );
        gate = GUCEF_NEW CGateProvider( root );
        if ( !Server().RegisterProvider( "files", plain, ShareAll() ) || !Server().RegisterProvider( "gate", DRN::CIDRNResourceProviderPtr( gate ), ShareAll() ) )
            return false;
        for ( UInt32 m=1; m<=memberCount; ++m )
        {
            DRN::CDRNNamespaceClient* client = GUCEF_NEW DRN::CDRNNamespaceClient( group[ m ], serverPeer );
            clients.push_back( client );
        }
        return true;
    }

    DRN::CDRNResourceSessionComponent& Server( void ) { return group[ 0 ].GetResources(); }
    DRN::CDRNResourceSessionComponent& Member( UInt32 member ) { return group[ member ].GetResources(); }
    UInt32 PeerOf( UInt32 member ) { return LocalPeerOf( group[ member ] ); }
    DRN::CDRNNamespaceClient& Client( UInt32 member ) { return *clients[ member - 1 ]; }

    bool AllCountersZero( void )
    {
        DRN::CDRNResourceSessionComponent& server = Server();
        return 0 == server.GetOpenHandleCount( 0 ) && 0 == server.GetActiveReadCount( 0 ) && 0 == server.GetNamespaceServingCount( 0 ) &&
               0 == server.GetNamespaceListingCount( 0 ) && 0 == server.GetBufferedBytes( 0 );
    }

    /** Waits for every counter of the serving side to be back at zero */
    bool WaitAllZero( UInt32 timeoutMs = 10000 )
    {
        UInt64 start = NowMs();
        while ( !AllCountersZero() && NowMs() - start < timeoutMs )
            MT::PrecisionDelay( 5 );
        return AllCountersZero();
    }
};

/*-------------------------------------------------------------------------*/

/** One kind of what the serving side counts and limits, taken one unit at a time on behalf of a member and given back all at once */
class CProbe
{
    public:

    CLimitFixture& fixture;
    CORE::CString lastWhy;

    explicit CProbe( CLimitFixture& f ) : fixture( f ), lastWhy() {}
    virtual ~CProbe() {}

    virtual const char* GetName( void ) const = 0;

    /** Sets the two limits of this kind (0 = none) on the serving side, every other limit off */
    virtual DRN::CDRNResourceTransferSettings Configure( UInt32 serverMax, UInt32 memberMax ) const = 0;

    /** Takes one unit as the member; false when the serving side refused it as BUSY (lastWhy has its words) */
    virtual bool Take( UInt32 member ) = 0;

    /** Gives back everything that was taken, in the normal way */
    virtual void GiveBackAll( void ) = 0;

    /** What the serving side counts of this kind, of one member or of all (peer 0) */
    virtual UInt32 Held( UInt32 peerId ) = 0;

    void Apply( UInt32 serverMax, UInt32 memberMax )
    {
        fixture.Server().SetSettings( Configure( serverMax, memberMax ) );
    }

    bool WaitHeld( UInt32 peerId, UInt32 expected, UInt32 timeoutMs = 10000 )
    {
        UInt64 start = NowMs();
        while ( Held( peerId ) != expected && NowMs() - start < timeoutMs )
            MT::PrecisionDelay( 5 );
        return Held( peerId ) == expected;
    }
};

/*-------------------------------------------------------------------------*/

class COpenProbe : public CProbe
{
    public:

    GUCEF::vector< DRN::CDRNResourceHandlePtr > handles;
    UInt32 counter;

    explicit COpenProbe( CLimitFixture& f ) : CProbe( f ), handles(), counter( 0 ) {}

    virtual const char* GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "open handles"; }

    virtual DRN::CDRNResourceTransferSettings Configure( UInt32 serverMax, UInt32 memberMax ) const GUCEF_VIRTUAL_OVERRIDE
    {
        DRN::CDRNResourceTransferSettings settings = BaseSettings();
        settings.maxOpenHandles = serverMax;
        settings.maxOpenHandlesPerPeer = memberMax;
        return settings;
    }

    virtual bool Take( UInt32 member ) GUCEF_VIRTUAL_OVERRIDE
    {
        DRN::CDRNResourceHandlePtr handle = fixture.Member( member ).OpenHandle( fixture.serverPeer, "files", "f" + CORE::ToString( counter++ % 10 ) + ".bin" );
        if ( handle->WaitOpen( 10000 ) )
        {
            handles.push_back( handle );
            return true;
        }
        lastWhy = handle->GetResult().message;
        return false;
    }

    virtual void GiveBackAll( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        handles.clear();
    }

    virtual UInt32 Held( UInt32 peerId ) GUCEF_VIRTUAL_OVERRIDE
    {
        return fixture.Server().GetOpenHandleCount( peerId );
    }
};

/*-------------------------------------------------------------------------*/

class CReadProbe : public CProbe
{
    public:

    GUCEF::vector< DRN::CDRNResourceHandlePtr > handles;
    GUCEF::vector< CReadJob* > jobs;
    GUCEF::vector< CJobThread* > threads;
    UInt32 counter;

    explicit CReadProbe( CLimitFixture& f ) : CProbe( f ), handles(), jobs(), threads(), counter( 0 ) {}

    virtual ~CReadProbe() { GiveBackAll(); }

    virtual const char* GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "reads"; }

    virtual DRN::CDRNResourceTransferSettings Configure( UInt32 serverMax, UInt32 memberMax ) const GUCEF_VIRTUAL_OVERRIDE
    {
        DRN::CDRNResourceTransferSettings settings = BaseSettings();
        settings.maxActiveReads = serverMax;
        settings.maxActiveReadsPerPeer = memberMax;
        return settings;
    }

    virtual bool Take( UInt32 member ) GUCEF_VIRTUAL_OVERRIDE
    {
        fixture.gate->Block( true, false );
        DRN::CDRNResourceHandlePtr handle = fixture.Member( member ).OpenHandle( fixture.serverPeer, "gate", "f" + CORE::ToString( counter++ % 10 ) + ".bin" );
        if ( !handle->WaitOpen( 10000 ) )
        {
            lastWhy = handle->GetResult().message;
            return false;
        }
        UInt32 before = fixture.Server().GetActiveReadCount( 0 );
        CReadJob* job = GUCEF_NEW CReadJob( handle );
        CJobThread* thread = GUCEF_NEW CJobThread( *job );
        handles.push_back( handle );
        jobs.push_back( job );
        threads.push_back( thread );
        if ( !thread->Start() )
            return false;

        // taken: the serving side counts one more read, and the read stays; refused: the call ended at once with BUSY
        UInt64 start = NowMs();
        while ( NowMs() - start < 10000 )
        {
            if ( fixture.Server().GetActiveReadCount( 0 ) > before )
                return true;
            if ( job->outcome.IsDone() )
            {
                lastWhy = job->outcome.message;
                return false;
            }
            MT::PrecisionDelay( 2 );
        }
        return false;
    }

    virtual void GiveBackAll( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        fixture.gate->Open();
        for ( UInt32 i=0; i<threads.size(); ++i )
        {
            threads[ i ]->Join( 20000 );
            GUCEF_DELETE threads[ i ];
            GUCEF_DELETE jobs[ i ];
        }
        threads.clear();
        jobs.clear();
        handles.clear();
    }

    virtual UInt32 Held( UInt32 peerId ) GUCEF_VIRTUAL_OVERRIDE
    {
        return fixture.Server().GetActiveReadCount( peerId );
    }
};

/*-------------------------------------------------------------------------*/

class CRequestProbe : public CProbe
{
    public:

    GUCEF::vector< CStatJob* > jobs;
    GUCEF::vector< CJobThread* > threads;

    explicit CRequestProbe( CLimitFixture& f ) : CProbe( f ), jobs(), threads()
    {
        // the client's own throttle is off: it is the serving side that is under test here
        for ( UInt32 m=1; m<=fixture.memberCount; ++m )
            fixture.Client( m ).SetSelfThrottle( false );
    }

    virtual ~CRequestProbe() { GiveBackAll(); }

    virtual const char* GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "namespace requests"; }

    virtual DRN::CDRNResourceTransferSettings Configure( UInt32 serverMax, UInt32 memberMax ) const GUCEF_VIRTUAL_OVERRIDE
    {
        DRN::CDRNResourceTransferSettings settings = BaseSettings();
        settings.maxNamespaceRequests = serverMax;
        settings.maxNamespaceRequestsPerPeer = memberMax;
        return settings;
    }

    virtual bool Take( UInt32 member ) GUCEF_VIRTUAL_OVERRIDE
    {
        fixture.gate->Block( false, true );
        UInt32 before = fixture.Server().GetNamespaceServingCount( 0 );
        CStatJob* job = GUCEF_NEW CStatJob( fixture.Client( member ), "gate", 1, 60000 );
        CJobThread* thread = GUCEF_NEW CJobThread( *job );
        jobs.push_back( job );
        threads.push_back( thread );
        if ( !thread->Start() )
            return false;
        UInt64 start = NowMs();
        while ( NowMs() - start < 10000 )
        {
            if ( fixture.Server().GetNamespaceServingCount( 0 ) > before )
                return true;
            if ( job->outcome.IsDone() )
            {
                lastWhy = job->outcome.message;
                return false;
            }
            MT::PrecisionDelay( 2 );
        }
        return false;
    }

    virtual void GiveBackAll( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        fixture.gate->Open();
        for ( UInt32 i=0; i<threads.size(); ++i )
        {
            threads[ i ]->Join( 20000 );
            GUCEF_DELETE threads[ i ];
            GUCEF_DELETE jobs[ i ];
        }
        threads.clear();
        jobs.clear();
    }

    virtual UInt32 Held( UInt32 peerId ) GUCEF_VIRTUAL_OVERRIDE
    {
        return fixture.Server().GetNamespaceServingCount( peerId );
    }
};

/*-------------------------------------------------------------------------*/

class CListingProbe : public CProbe
{
    public:

    class CHeld
    {
        public:
        UInt32 member;
        CORE::CString cursor;
    };

    GUCEF::vector< CHeld > held;

    explicit CListingProbe( CLimitFixture& f ) : CProbe( f ), held()
    {
        for ( UInt32 m=1; m<=fixture.memberCount; ++m )
            fixture.Client( m ).SetSelfThrottle( false );
    }

    virtual const char* GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "listings"; }

    virtual DRN::CDRNResourceTransferSettings Configure( UInt32 serverMax, UInt32 memberMax ) const GUCEF_VIRTUAL_OVERRIDE
    {
        DRN::CDRNResourceTransferSettings settings = BaseSettings();
        settings.maxNamespaceListings = serverMax;
        settings.maxNamespaceListingsPerPeer = memberMax;
        return settings;
    }

    virtual bool Take( UInt32 member ) GUCEF_VIRTUAL_OVERRIDE
    {
        DRN::CDRNNamespaceListPage page;
        DRN::CDRNNamespaceResult result = fixture.Client( member ).List( "files", "big", CORE::CString::Empty, 5, CORE::CString::Empty, page );
        if ( !result.IsOk() )
        {
            lastWhy = result.message;
            return false;
        }
        CHeld entry;
        entry.member = member;
        entry.cursor = page.nextCursor;
        held.push_back( entry );
        return true;
    }

    virtual void GiveBackAll( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        // a listing gives its place back with its last page
        for ( UInt32 i=0; i<held.size(); ++i )
        {
            DRN::CDRNNamespaceListPage page;
            page.nextCursor = held[ i ].cursor;
            UInt32 guard = 0;
            while ( !page.nextCursor.IsNULLOrEmpty() && guard++ < 1000 )
            {
                CORE::CString cursor = page.nextCursor;
                if ( !fixture.Client( held[ i ].member ).List( "files", "big", cursor, 0, CORE::CString::Empty, page ).IsOk() )
                    break;
            }
        }
        held.clear();
    }

    virtual UInt32 Held( UInt32 peerId ) GUCEF_VIRTUAL_OVERRIDE
    {
        return fixture.Server().GetNamespaceListingCount( peerId );
    }
};

/*-------------------------------------------------------------------------*/

/** What the tests of a probe have in common: the refusal is BUSY-worded and names the tier */
bool
RefusedBy( CProbe& probe, UInt32 member, const char* tier, UInt32 limit )
{
    if ( probe.Take( member ) )
        return false;
    return Contains( probe.lastWhy, CORE::CString( tier ) + " limit of " + CORE::ToString( limit ) + " " );
}

/*-------------------------------------------------------------------------*/

/** A policy of the tests: lowers or raises the limits of chosen members and counts how often it was asked */
class CMapPolicy : public DRN::CIDRNPeerLimitPolicy
{
    public:

    MT::CMutex lock;
    GUCEF::map< UInt32, DRN::CDRNPeerLimits > overrides;
    GUCEF::map< UInt32, UInt32 > calls;
    DRN::CDRNPeerLimits lastConfigured;

    CMapPolicy( void ) : lock(), overrides(), calls(), lastConfigured() {}

    void Set( UInt32 peerId, const DRN::CDRNPeerLimits& limits )
    {
        MT::CScopeMutex scope( lock );
        overrides[ peerId ] = limits;
    }

    UInt32 Calls( UInt32 peerId )
    {
        MT::CScopeMutex scope( lock );
        return calls[ peerId ];
    }

    virtual void GetEffectiveLimits( UInt32 peerId, const DRN::CDRNAuthResult& auth, const DRN::CDRNPeerLimits& configured, DRN::CDRNPeerLimits& effective ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex scope( lock );
        ++calls[ peerId ];
        lastConfigured = configured;
        GUCEF::map< UInt32, DRN::CDRNPeerLimits >::iterator i = overrides.find( peerId );
        if ( i != overrides.end() )
            effective = (*i).second;
        else
            DRN::CIDRNPeerLimitPolicy::GetEffectiveLimits( peerId, auth, configured, effective );
    }
};

/*-------------------------------------------------------------------------*/

/** The reasons a call to the pool ended, for a test of the pool: reads a whole file through a handle and times it */
class CDownloadJob : public CJob
{
    public:

    DRN::CDRNResourceHandlePtr handle;
    UInt32 length;
    UInt64 elapsedMs;
    bool ok;

    CDownloadJob( const DRN::CDRNResourceHandlePtr& h, UInt32 bytes ) : handle( h ), length( bytes ), elapsedMs( 0 ), ok( false ) {}

    virtual void Run( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CDynamicBuffer data;
        UInt64 start = NowMs();
        DRN::CDRNResourceTransferResult result = handle->ReadRange( 0, length, data );
        elapsedMs = NowMs() - start;
        ok = result.IsOk() && data.GetDataSize() == length;
    }
};

/*-------------------------------------------------------------------------*/

/** Every member of the session takes units of a probe until refused, up to a number */
UInt32
TakeUntilRefused( CProbe& probe, UInt32 member, UInt32 most )
{
    UInt32 taken = 0;
    while ( taken < most && probe.Take( member ) )
        ++taken;
    return taken;
}

/*-------------------------------------------------------------------------*/

/** The two tiers, for one kind of limit */
void
TwoTierScenarios( CProbe& probe, CLimitFixture& fx )
{
    const UInt32 a = 1;
    const UInt32 b = 2;
    DRN::CDRNResourceSessionComponent& server = fx.Server();
    const char* name = probe.GetName();

    // (a) the server-wide limit alone: two members together reach it, neither alone does; the refusal names the server-wide limit
    probe.Apply( 2, 0 );
    {
        DRN::CDRNResourceTransferStats before = server.GetStats();
        ASSERT_TRUE( probe.Take( a ) );
        ASSERT_TRUE( probe.Take( b ) );
        ASSERT_TRUE( 2 == probe.Held( 0 ) && 1 == probe.Held( fx.PeerOf( a ) ) && 1 == probe.Held( fx.PeerOf( b ) ) );
        ASSERT_TRUE( RefusedBy( probe, a, "server-wide", 2 ) );
        ASSERT_TRUE( RefusedBy( probe, b, "server-wide", 2 ) );
        DRN::CDRNResourceTransferStats after = server.GetStats();
        ASSERT_TRUE( after.refusedByServerLimit == before.refusedByServerLimit + 2 && after.refusedByMemberLimit == before.refusedByMemberLimit );
        probe.GiveBackAll();
        ASSERT_TRUE( probe.WaitHeld( 0, 0 ) );

        // one member alone is within the limit with what it takes
        ASSERT_TRUE( probe.Take( a ) );
        ASSERT_TRUE( probe.Take( a ) );
        ASSERT_TRUE( RefusedBy( probe, a, "server-wide", 2 ) );
        ASSERT_TRUE( RefusedBy( probe, b, "server-wide", 2 ) );
        probe.GiveBackAll();
        ASSERT_TRUE( probe.WaitHeld( 0, 0 ) );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN limits: " << name << ", server-wide limit alone checked" );

    // (b) the member limit alone: one member is refused, the other is not; the refusal names the member limit
    probe.Apply( 0, 1 );
    {
        DRN::CDRNResourceTransferStats before = server.GetStats();
        ASSERT_TRUE( probe.Take( a ) );
        ASSERT_TRUE( RefusedBy( probe, a, "per-member", 1 ) );
        ASSERT_TRUE( probe.Take( b ) );
        ASSERT_TRUE( RefusedBy( probe, b, "per-member", 1 ) );
        DRN::CDRNResourceTransferStats after = server.GetStats();
        ASSERT_TRUE( after.refusedByMemberLimit == before.refusedByMemberLimit + 2 && after.refusedByServerLimit == before.refusedByServerLimit );
        probe.GiveBackAll();
        ASSERT_TRUE( probe.WaitHeld( 0, 0 ) );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN limits: " << name << ", member limit alone checked" );

    // (c) both set: the lower one wins, in both directions
    probe.Apply( 2, 3 );
    {
        ASSERT_TRUE( probe.Take( a ) );
        ASSERT_TRUE( probe.Take( b ) );
        ASSERT_TRUE( RefusedBy( probe, a, "server-wide", 2 ) );
        probe.GiveBackAll();
        ASSERT_TRUE( probe.WaitHeld( 0, 0 ) );
    }
    probe.Apply( 3, 1 );
    {
        ASSERT_TRUE( probe.Take( a ) );
        ASSERT_TRUE( RefusedBy( probe, a, "per-member", 1 ) );
        ASSERT_TRUE( probe.Take( b ) );
        ASSERT_TRUE( 2 == probe.Held( 0 ) );
        probe.GiveBackAll();
        ASSERT_TRUE( probe.WaitHeld( 0, 0 ) );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN limits: " << name << ", both tiers checked" );

    // (d) 0 is no maximum, for both
    probe.Apply( 0, 0 );
    {
        DRN::CDRNResourceTransferStats before = server.GetStats();
        ASSERT_TRUE( 6 == TakeUntilRefused( probe, a, 6 ) );
        ASSERT_TRUE( 6 == TakeUntilRefused( probe, b, 6 ) );
        ASSERT_TRUE( 12 == probe.Held( 0 ) );
        DRN::CDRNResourceTransferStats after = server.GetStats();
        ASSERT_TRUE( after.refusedByMemberLimit == before.refusedByMemberLimit && after.refusedByServerLimit == before.refusedByServerLimit );
        probe.GiveBackAll();
        ASSERT_TRUE( probe.WaitHeld( 0, 0 ) );
    }

    // (e) normal close: every counter is back at zero
    ASSERT_TRUE( fx.WaitAllZero() );
}

/*-------------------------------------------------------------------------*/

/** What the serving side does with the units of a member that goes: link lost, or leaving the session */
void
GoneScenario( CProbe& probe, CLimitFixture& fx, bool leave )
{
    const UInt32 a = 1;
    const UInt32 b = 2;
    probe.Apply( 0, 0 );
    ASSERT_TRUE( probe.Take( a ) );
    ASSERT_TRUE( probe.Take( a ) );
    ASSERT_TRUE( probe.Take( b ) );
    ASSERT_TRUE( 3 == probe.Held( 0 ) );
    UInt32 peerA = fx.PeerOf( a );
    if ( leave )
        fx.group[ a ].Leave();
    else
        fx.group.nodes[ a ]->Stop();
    ASSERT_TRUE( probe.WaitHeld( peerA, 0, 20000 ) );
    ASSERT_TRUE( 1 == probe.Held( 0 ) && 1 == probe.Held( fx.PeerOf( b ) ) );
    probe.GiveBackAll();
    ASSERT_TRUE( fx.WaitAllZero( 20000 ) );
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNLimitTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DRN LIMIT TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.Limits" );

    RemoveTestDirectories();

    GUCEF_TESTFW_TESTCASE( "Limits: the settings have bounded server-wide defaults beside the per-member ones, load and save as they are, and an old configuration keeps working" )
    try
    {
        DRN::CDRNResourceTransferSettings defaults;
        ASSERT_TRUE( 4096 == defaults.maxOpenHandles && 256 == defaults.maxActiveReads && 512 == defaults.maxNamespaceRequests && 256 == defaults.maxNamespaceListings );
        ASSERT_TRUE( 32 == defaults.maxOpenHandlesPerPeer && 8 == defaults.maxActiveReadsPerPeer && 16 == defaults.maxNamespaceRequestsPerPeer && 8 == defaults.maxNamespaceListingsPerPeer );
        ASSERT_TRUE( 0 == defaults.maxUploadBytesPerSecond && 0 == defaults.maxUploadBytesPerSecondPerPeer && 0 == defaults.maxBufferedBytesPerPeer && defaults.announceLimits );

        DRN::CDRNResourceTransferSettings custom;
        custom.maxOpenHandles = 11; custom.maxOpenHandlesPerPeer = 3; custom.maxActiveReads = 12; custom.maxActiveReadsPerPeer = 4;
        custom.maxNamespaceRequests = 13; custom.maxNamespaceRequestsPerPeer = 5; custom.maxNamespaceListings = 14; custom.maxNamespaceListingsPerPeer = 6;
        custom.maxUploadBytesPerSecond = 1500000; custom.maxUploadBytesPerSecondPerPeer = 700000; custom.maxBufferedBytesPerPeer = 900000; custom.announceLimits = false;
        CORE::CDataNode node;
        ASSERT_TRUE( custom.SaveConfig( node ) );
        DRN::CDRNResourceTransferSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( node ) );
        ASSERT_TRUE( 11 == loaded.maxOpenHandles && 3 == loaded.maxOpenHandlesPerPeer && 12 == loaded.maxActiveReads && 4 == loaded.maxActiveReadsPerPeer );
        ASSERT_TRUE( 13 == loaded.maxNamespaceRequests && 5 == loaded.maxNamespaceRequestsPerPeer && 14 == loaded.maxNamespaceListings && 6 == loaded.maxNamespaceListingsPerPeer );
        ASSERT_TRUE( 1500000 == loaded.maxUploadBytesPerSecond && 700000 == loaded.maxUploadBytesPerSecondPerPeer && 900000 == loaded.maxBufferedBytesPerPeer && !loaded.announceLimits );

        // a configuration of before the server-wide limits existed: what it names is read, the rest is the default
        CORE::CDataNode old;
        old.SetAttribute( "maxOpenHandlesPerPeer", 7 );
        old.SetAttribute( "maxNamespaceRequestsPerPeer", 9 );
        DRN::CDRNResourceTransferSettings fromOld;
        ASSERT_TRUE( fromOld.LoadConfig( old ) );
        ASSERT_TRUE( 7 == fromOld.maxOpenHandlesPerPeer && 9 == fromOld.maxNamespaceRequestsPerPeer );
        ASSERT_TRUE( 4096 == fromOld.maxOpenHandles && 256 == fromOld.maxActiveReads && 512 == fromOld.maxNamespaceRequests && 256 == fromOld.maxNamespaceListings && fromOld.announceLimits );

        // the limits are local decisions: the security policy, whose hash both ends compare, holds none of them
        DRN::CDRNSecurityPolicy policy;
        CORE::CDataNode policyNode;
        policy.SaveConfig( policyNode );
        ASSERT_FALSE( policyNode.IsAttribute( "maxOpenHandles" ) || policyNode.IsAttribute( "maxOpenHandlesPerPeer" ) || policyNode.IsAttribute( "maxUploadBytesPerSecondPerPeer" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: open handles, reads, namespace requests and listings each have a server-wide and a per-member limit that apply independently; the lower wins; every counter returns to zero" )
    try
    {
        CLimitFixture fixture( "lim_tiers" );
        REQUIRE_TRUE( fixture.Start() );
        {
            COpenProbe probe( fixture );
            TwoTierScenarios( probe, fixture );
        }
        {
            CReadProbe probe( fixture );
            TwoTierScenarios( probe, fixture );
        }
        {
            CRequestProbe probe( fixture );
            TwoTierScenarios( probe, fixture );
        }
        {
            CListingProbe probe( fixture );
            TwoTierScenarios( probe, fixture );
        }
        ASSERT_TRUE( fixture.WaitAllZero() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: the counters of every kind return to zero when the link of a member is lost under load" )
    try
    {
        for ( UInt32 kind=0; kind<4; ++kind )
        {
            CLimitFixture fixture( "lim_lost" + CORE::ToString( kind ) );
            REQUIRE_TRUE( fixture.Start() );
            if ( 0 == kind )
            {
                COpenProbe probe( fixture );
                GoneScenario( probe, fixture, false );
            }
            else
            if ( 1 == kind )
            {
                CReadProbe probe( fixture );
                GoneScenario( probe, fixture, false );
            }
            else
            if ( 2 == kind )
            {
                CRequestProbe probe( fixture );
                GoneScenario( probe, fixture, false );
            }
            else
            {
                CListingProbe probe( fixture );
                GoneScenario( probe, fixture, false );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: the counters of every kind return to zero when a member leaves the session under load" )
    try
    {
        for ( UInt32 kind=0; kind<4; ++kind )
        {
            CLimitFixture fixture( "lim_left" + CORE::ToString( kind ) );
            REQUIRE_TRUE( fixture.Start() );
            if ( 0 == kind )
            {
                COpenProbe probe( fixture );
                GoneScenario( probe, fixture, true );
            }
            else
            if ( 1 == kind )
            {
                CReadProbe probe( fixture );
                GoneScenario( probe, fixture, true );
            }
            else
            if ( 2 == kind )
            {
                CRequestProbe probe( fixture );
                GoneScenario( probe, fixture, true );
            }
            else
            {
                CListingProbe probe( fixture );
                GoneScenario( probe, fixture, true );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: a handle that is idle too long, a listing nobody continues, a read that was cancelled and a read that failed give their places back" )
    try
    {
        CLimitFixture fixture( "lim_expiry" );
        REQUIRE_TRUE( fixture.Start() );
        DRN::CDRNResourceSessionComponent& server = fixture.Server();

        // idle timeout of handles
        {
            COpenProbe probe( fixture );
            DRN::CDRNResourceTransferSettings settings = probe.Configure( 0, 0 );
            settings.handleIdleTimeoutMs = 300;
            server.SetSettings( settings );
            UInt64 expiredBefore = server.GetStats().handlesExpired;
            ASSERT_TRUE( probe.Take( 1 ) && probe.Take( 1 ) && probe.Take( 2 ) );
            ASSERT_TRUE( probe.WaitHeld( 0, 0, 15000 ) );
            ASSERT_TRUE( server.GetStats().handlesExpired >= expiredBefore + 3 );
            probe.GiveBackAll();
        }

        // expiry of listings: the lifetime ends them, and so does finishing them
        {
            CListingProbe probe( fixture );
            DRN::CDRNResourceTransferSettings settings = probe.Configure( 0, 0 );
            settings.namespaceListingLifetimeMs = 300;
            server.SetSettings( settings );
            UInt64 expiredBefore = server.GetStats().namespaceListingsExpired;
            ASSERT_TRUE( probe.Take( 1 ) && probe.Take( 1 ) && probe.Take( 2 ) );
            ASSERT_TRUE( probe.WaitHeld( 0, 0, 15000 ) );
            ASSERT_TRUE( server.GetStats().namespaceListingsExpired >= expiredBefore + 3 );
            probe.GiveBackAll();
            settings.namespaceListingLifetimeMs = 60000;
            server.SetSettings( settings );
            ASSERT_TRUE( probe.Take( 1 ) && 1 == probe.Held( 0 ) );
            probe.GiveBackAll();
            ASSERT_TRUE( probe.WaitHeld( 0, 0 ) );
        }

        // a cancelled read: the transfer ends, the serving side lets go of the read and the handle at once
        {
            server.SetSettings( BaseSettings() );
            fixture.gate->Block( true, false );
            DRN::CDRNResourceSessionComponent& requester = fixture.Member( 1 );
            DRN::CDRNResourceTransfer* transfer = requester.Fetch( fixture.serverPeer, "gate", "f1.bin", DRN::CIDRNResourceSinkPtr( GUCEF_NEW DRN::CDRNMemoryResourceSink() ) );
            UInt64 start = NowMs();
            while ( 0 == server.GetActiveReadCount( 0 ) && NowMs() - start < 10000 )
                MT::PrecisionDelay( 2 );
            ASSERT_TRUE( 1 == server.GetActiveReadCount( 0 ) && 1 == server.GetOpenHandleCount( 0 ) );
            transfer->Cancel();
            start = NowMs();
            while ( !transfer->IsFinished() && NowMs() - start < 10000 )
                MT::PrecisionDelay( 2 );
            ASSERT_TRUE( transfer->IsFinished() );
            requester.Release( transfer );
            start = NowMs();
            while ( ( 0 != server.GetActiveReadCount( 0 ) || 0 != server.GetOpenHandleCount( 0 ) ) && NowMs() - start < 10000 )
                MT::PrecisionDelay( 2 );
            ASSERT_TRUE( 0 == server.GetActiveReadCount( 0 ) && 0 == server.GetOpenHandleCount( 0 ) );
            fixture.gate->Open();
        }

        // a read that fails in the provider: the read, its bytes and its handle are given back
        {
            fixture.gate->SetFailReads( true );
            DRN::CDRNResourceHandlePtr handle = fixture.Member( 1 ).OpenHandle( fixture.serverPeer, "gate", "f2.bin" );
            REQUIRE_TRUE( handle->WaitOpen( 10000 ) );
            CORE::CDynamicBuffer data;
            ASSERT_FALSE( handle->ReadRange( 0, 1000, data ).IsOk() );
            ASSERT_TRUE( 0 == server.GetActiveReadCount( 0 ) );
            handle.Unlink();
            fixture.gate->SetFailReads( false );
        }
        ASSERT_TRUE( fixture.WaitAllZero() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: the policy is consulted per member and changes the limits of that member only; the server-wide limits are never changed by it" )
    try
    {
        CLimitFixture fixture( "lim_policy" );
        REQUIRE_TRUE( fixture.Start() );
        DRN::CDRNResourceSessionComponent& server = fixture.Server();
        COpenProbe probe( fixture );
        const UInt32 a = 1;
        const UInt32 b = 2;
        UInt32 peerA = fixture.PeerOf( a );
        UInt32 peerB = fixture.PeerOf( b );

        // configured: 2 per member, 3 together
        server.SetSettings( probe.Configure( 3, 2 ) );
        DRN::CDRNPeerLimits effective = server.GetEffectivePeerLimits( peerA );
        ASSERT_TRUE( 2 == effective.maxOpenHandles );

        CMapPolicy policy;
        DRN::CDRNPeerLimits lowered = server.GetEffectivePeerLimits( peerA );
        lowered.maxOpenHandles = 1;
        DRN::CDRNPeerLimits raised = lowered;
        raised.maxOpenHandles = 100;
        policy.Set( peerA, lowered );
        policy.Set( peerB, raised );
        server.SetPeerLimitPolicy( &policy );

        // A is held to 1 though the configured value is 2; B is held to 3, the server-wide limit, though the policy gave it 100
        ASSERT_TRUE( probe.Take( a ) );
        ASSERT_TRUE( RefusedBy( probe, a, "per-member", 1 ) );
        ASSERT_TRUE( probe.Take( b ) && probe.Take( b ) );
        ASSERT_TRUE( RefusedBy( probe, b, "server-wide", 3 ) );
        ASSERT_TRUE( 3 == probe.Held( 0 ) );
        ASSERT_TRUE( policy.lastConfigured.maxOpenHandles == 2 );

        // the policy is asked per member and the answer is kept: many requests, a few questions
        for ( UInt32 i=0; i<10; ++i )
            probe.Take( a );
        ASSERT_TRUE( policy.Calls( peerA ) >= 1 && policy.Calls( peerA ) <= 3 && policy.Calls( peerB ) >= 1 && policy.Calls( peerB ) <= 3 );

        // what the hello tells each member is the lower of its own effective limit and the server-wide one
        {
            DRN::CDRNNamespaceInfo infoA;
            DRN::CDRNNamespaceInfo infoB;
            ASSERT_TRUE( fixture.Client( a ).Negotiate( infoA ).IsOk() && fixture.Client( b ).Negotiate( infoB ).IsOk() );
            ASSERT_TRUE( infoA.limitsAnnounced && 1 == infoA.maxOpenHandles && infoB.limitsAnnounced && 3 == infoB.maxOpenHandles );
        }

        // a change of the policy's mind applies at once after RefreshPeerLimits(), to that member only
        probe.GiveBackAll();
        ASSERT_TRUE( probe.WaitHeld( 0, 0 ) );
        lowered.maxOpenHandles = 3;
        policy.Set( peerA, lowered );
        server.RefreshPeerLimits( peerA );
        ASSERT_TRUE( 3 == server.GetEffectivePeerLimits( peerA ).maxOpenHandles && 100 == server.GetEffectivePeerLimits( peerB ).maxOpenHandles );
        ASSERT_TRUE( probe.Take( a ) && probe.Take( a ) && probe.Take( a ) );
        ASSERT_TRUE( RefusedBy( probe, a, "per-member", 3 ) );
        ASSERT_TRUE( RefusedBy( probe, b, "server-wide", 3 ) );
        probe.GiveBackAll();
        ASSERT_TRUE( probe.WaitHeld( 0, 0 ) );

        // without a policy the configured values are the limits again
        server.SetPeerLimitPolicy( GUCEF_NULL );
        ASSERT_TRUE( 2 == server.GetEffectivePeerLimits( peerA ).maxOpenHandles && 2 == server.GetEffectivePeerLimits( peerB ).maxOpenHandles );
        ASSERT_TRUE( probe.Take( a ) && probe.Take( a ) );
        ASSERT_TRUE( RefusedBy( probe, a, "per-member", 2 ) );
        probe.GiveBackAll();
        ASSERT_TRUE( fixture.WaitAllZero() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: the upload rate of a member is held to its own limit while another member, which the policy gave none, is not; the buffered bytes of a member stay within its limit" )
    try
    {
        CLimitFixture fixture( "lim_rate" );
        REQUIRE_TRUE( fixture.Start() );
        DRN::CDRNResourceSessionComponent& server = fixture.Server();
        const UInt32 a = 1;
        const UInt32 b = 2;
        UInt32 peerA = fixture.PeerOf( a );
        UInt32 peerB = fixture.PeerOf( b );
        const UInt32 length = 24 * ChunkBytes;

        // 512 KiB/s per member, nothing server-wide; the policy leaves A with that and gives B no limit
        DRN::CDRNResourceTransferSettings settings = BaseSettings();
        settings.maxUploadBytesPerSecondPerPeer = 512 * 1024;
        fixture.Server().SetSettings( settings );
        CMapPolicy policy;
        DRN::CDRNPeerLimits unlimited = server.GetEffectivePeerLimits( peerB );
        unlimited.maxUploadBytesPerSecond = 0;
        policy.Set( peerB, unlimited );
        server.SetPeerLimitPolicy( &policy );
        ASSERT_TRUE( 512 * 1024 == server.GetEffectivePeerLimits( peerA ).maxUploadBytesPerSecond && 0 == server.GetEffectivePeerLimits( peerB ).maxUploadBytesPerSecond );

        DRN::CDRNResourceHandlePtr handleA = fixture.Member( a ).OpenHandle( fixture.serverPeer, "files", "u.bin" );
        DRN::CDRNResourceHandlePtr handleB = fixture.Member( b ).OpenHandle( fixture.serverPeer, "files", "u.bin" );
        REQUIRE_TRUE( handleA->WaitOpen( 10000 ) && handleB->WaitOpen( 10000 ) );
        CDownloadJob jobA( handleA, length );
        CDownloadJob jobB( handleB, length );
        CJobThread threadA( jobA );
        CJobThread threadB( jobB );
        REQUIRE_TRUE( threadA.Start() && threadB.Start() );
        REQUIRE_TRUE( threadA.Join( 60000 ) && threadB.Join( 60000 ) );
        ASSERT_TRUE( jobA.ok && jobB.ok );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN limits: 1.5 MiB at 512 KiB/s per member took " << jobA.elapsedMs << " ms for the held member, " << jobB.elapsedMs << " ms for the other" );

        // the burst of the limiter is a quarter second of the rate (at least four chunks): what remains takes at least this long; generous tolerances
        ASSERT_TRUE( jobA.elapsedMs >= 1500 );
        ASSERT_TRUE( jobB.elapsedMs * 2 < jobA.elapsedMs );

        jobA.handle.Unlink();
        jobB.handle.Unlink();
        handleA.Unlink();
        handleB.Unlink();
        ASSERT_TRUE( fixture.WaitAllZero() );
        server.SetPeerLimitPolicy( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: the buffered bytes of a member stay within its own limit, and exceed it where there is none" )
    try
    {
        CLimitFixture fixture( "lim_buffer" );
        REQUIRE_TRUE( fixture.Start() );
        DRN::CDRNResourceSessionComponent& server = fixture.Server();
        const UInt32 a = 1;
        const UInt32 length = 24 * ChunkBytes;
        DRN::CDRNResourceTransferSettings buffered = BaseSettings();
        buffered.maxBufferedBytesPerPeer = 2 * ChunkBytes;
        server.SetSettings( buffered );
        {
            DRN::CDRNResourceHandlePtr handle = fixture.Member( a ).OpenHandle( fixture.serverPeer, "files", "u.bin" );
            REQUIRE_TRUE( handle->WaitOpen( 10000 ) );
            CDownloadJob job( handle, length );
            CJobThread thread( job );
            REQUIRE_TRUE( thread.Start() && thread.Join( 60000 ) );
            ASSERT_TRUE( job.ok );
        }
        ASSERT_TRUE( server.GetStats().peakServerBufferedBytes <= 2 * ChunkBytes && server.GetStats().peakServerBufferedBytes > 0 );
        buffered.maxBufferedBytesPerPeer = 0;
        server.SetSettings( buffered );
        {
            DRN::CDRNResourceHandlePtr handle = fixture.Member( a ).OpenHandle( fixture.serverPeer, "files", "u.bin" );
            REQUIRE_TRUE( handle->WaitOpen( 10000 ) );
            CDownloadJob job( handle, length );
            CJobThread thread( job );
            REQUIRE_TRUE( thread.Start() && thread.Join( 60000 ) );
            ASSERT_TRUE( job.ok );
        }
        ASSERT_TRUE( server.GetStats().peakServerBufferedBytes > 2 * ChunkBytes );
        ASSERT_TRUE( fixture.WaitAllZero() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: the hello announces the lower of both tiers for the asking member, an answer without the extension is accepted and means unknown, a malformed extension is refused" )
    try
    {
        CLimitFixture fixture( "lim_hello" );
        REQUIRE_TRUE( fixture.Start() );
        DRN::CDRNResourceSessionComponent& server = fixture.Server();
        DRN::CDRNNamespaceClient& client = fixture.Client( 1 );

        // the server-wide value is the lower for some, the member's for others, 0 for none
        DRN::CDRNResourceTransferSettings settings = BaseSettings();
        settings.maxOpenHandles = 10;               settings.maxOpenHandlesPerPeer = 4;
        settings.maxActiveReads = 0;                settings.maxActiveReadsPerPeer = 3;
        settings.maxNamespaceRequests = 6;          settings.maxNamespaceRequestsPerPeer = 0;
        settings.maxNamespaceListings = 2;          settings.maxNamespaceListingsPerPeer = 5;
        settings.maxUploadBytesPerSecond = 1000000; settings.maxUploadBytesPerSecondPerPeer = 500000;
        server.SetSettings( settings );
        DRN::CDRNNamespaceInfo info;
        REQUIRE_TRUE( client.Negotiate( info ).IsOk() );
        ASSERT_TRUE( info.limitsAnnounced && 4 == info.maxOpenHandles && 3 == info.maxActiveReads && 6 == info.maxNamespaceRequests && 2 == info.maxNamespaceListings && 500000 == info.maxUploadBytesPerSecond );
        ASSERT_TRUE( 6 == client.GetRequestLimit() && 2 == client.GetListingLimit() && 4 == client.GetOpenHandleLimit() && 3 == client.GetActiveReadLimit() && 500000 == client.GetUploadRateLimit() );
        ASSERT_TRUE( 1 == info.version );

        settings.maxOpenHandles = 2;                settings.maxOpenHandlesPerPeer = 9;
        settings.maxUploadBytesPerSecond = 0;       settings.maxUploadBytesPerSecondPerPeer = 0;
        server.SetSettings( settings );
        REQUIRE_TRUE( client.Negotiate( info ).IsOk() );
        ASSERT_TRUE( 2 == info.maxOpenHandles && 0 == info.maxUploadBytesPerSecond );

        // the page and answer limits that were there stay
        ASSERT_TRUE( info.maxStatPaths > 0 && info.maxPageEntries > 0 && info.maxAnswerBytes > 0 && info.listingLifetimeMs > 0 );

        // a serving side that does not announce: the limits are unknown, the client does not throttle itself
        settings.announceLimits = false;
        server.SetSettings( settings );
        DRN::CDRNNamespaceClient fresh( fixture.group[ 2 ], fixture.serverPeer );
        REQUIRE_TRUE( fresh.Negotiate( info ).IsOk() );
        ASSERT_TRUE( !info.limitsAnnounced && 0 == info.maxOpenHandles && 0 == info.maxNamespaceRequests && 0 == info.maxNamespaceListings );
        ASSERT_TRUE( 0 == fresh.GetRequestLimit() && 0 == fresh.GetListingLimit() );

        // the codec: the body without the extension, the old reader that stops after the six numbers, and every cut of the extended body
        {
            CORE::CDynamicBuffer plain;
            DRN::CDRNWireWriter plainWriter( plain );
            DRN::CDRNNamespaceInfo written;
            written.version = 1; written.capabilities = 7; written.maxStatPaths = 11; written.maxPageEntries = 22; written.maxAnswerBytes = 33333; written.listingLifetimeMs = 44;
            DRN::CDRNNamespaceCodec::WriteHelloBody( plainWriter, written );
            const UInt32 plainSize = plainWriter.GetOffset();

            CORE::CDynamicBuffer extended;
            DRN::CDRNWireWriter extendedWriter( extended );
            written.limitsAnnounced = true; written.maxOpenHandles = 5; written.maxActiveReads = 6; written.maxNamespaceRequests = 7; written.maxNamespaceListings = 8; written.maxUploadBytesPerSecond = 9000000;
            DRN::CDRNNamespaceCodec::WriteHelloBody( extendedWriter, written );
            const UInt32 extendedSize = extendedWriter.GetOffset();
            ASSERT_TRUE( extendedSize > plainSize );
            ASSERT_TRUE( 0 == memcmp( plain.GetConstBufferPtr(), extended.GetConstBufferPtr(), plainSize ) );

            // a reader of before the extension reads the six numbers and has what it needs
            {
                DRN::CDRNWireReader old( extended.GetConstBufferPtr(), extendedSize );
                UInt32 numbers[ 6 ];
                bool all = true;
                for ( UInt32 i=0; i<6; ++i )
                    all = old.ReadVarUInt32( numbers[ i ] ) && all;
                ASSERT_TRUE( all && 1 == numbers[ 0 ] && 7 == numbers[ 1 ] && 11 == numbers[ 2 ] && 22 == numbers[ 3 ] && 33333 == numbers[ 4 ] && 44 == numbers[ 5 ] );
            }
            // a reader of now reads the old form as unknown
            {
                DRN::CDRNWireReader reader( plain.GetConstBufferPtr(), plainSize );
                DRN::CDRNNamespaceInfo read;
                read.maxOpenHandles = 77;
                ASSERT_TRUE( DRN::CDRNNamespaceCodec::ReadHelloBody( reader, read ) );
                ASSERT_TRUE( !read.limitsAnnounced && 0 == read.maxOpenHandles && 0 == read.maxUploadBytesPerSecond && 22 == read.maxPageEntries );
            }
            {
                DRN::CDRNWireReader reader( extended.GetConstBufferPtr(), extendedSize );
                DRN::CDRNNamespaceInfo read;
                ASSERT_TRUE( DRN::CDRNNamespaceCodec::ReadHelloBody( reader, read ) );
                ASSERT_TRUE( read.limitsAnnounced && 5 == read.maxOpenHandles && 6 == read.maxActiveReads && 7 == read.maxNamespaceRequests && 8 == read.maxNamespaceListings && 9000000 == read.maxUploadBytesPerSecond );
            }

            // every cut: inside the six numbers or inside the extension it is refused, after the six numbers it is the old form
            for ( UInt32 cut=0; cut<=extendedSize; ++cut )
            {
                DRN::CDRNWireReader reader( extended.GetConstBufferPtr(), cut );
                DRN::CDRNNamespaceInfo read;
                bool ok = DRN::CDRNNamespaceCodec::ReadHelloBody( reader, read );
                if ( cut < plainSize )
                    ASSERT_FALSE( ok );
                else
                if ( cut == plainSize )
                    ASSERT_TRUE( ok && !read.limitsAnnounced );
                else
                if ( cut < extendedSize )
                    ASSERT_FALSE( ok );
                else
                    ASSERT_TRUE( ok && read.limitsAnnounced );
            }

            // an extension of a later version (more numbers, more after the block) is read for what is known; the block that lies about its size or its numbers is refused
            {
                CORE::CDynamicBuffer later;
                DRN::CDRNWireWriter laterWriter( later );
                laterWriter.WriteBytes( plain.GetConstBufferPtr(), plainSize );
                CORE::CDynamicBuffer block;
                DRN::CDRNWireWriter blockWriter( block );
                blockWriter.WriteVarUInt32( 7 );
                for ( UInt32 i=1; i<=7; ++i )
                    blockWriter.WriteVarUInt32( i * 10 );
                laterWriter.WriteSizedBytes( block.GetConstBufferPtr(), blockWriter.GetOffset() );
                laterWriter.WriteVarUInt32( 12345 );
                DRN::CDRNWireReader reader( later.GetConstBufferPtr(), laterWriter.GetOffset() );
                DRN::CDRNNamespaceInfo read;
                ASSERT_TRUE( DRN::CDRNNamespaceCodec::ReadHelloBody( reader, read ) && read.limitsAnnounced && 10 == read.maxOpenHandles && 50 == read.maxUploadBytesPerSecond );
            }
            {
                // fewer numbers than this version knows: the others are unknown
                CORE::CDynamicBuffer fewer;
                DRN::CDRNWireWriter fewerWriter( fewer );
                fewerWriter.WriteBytes( plain.GetConstBufferPtr(), plainSize );
                CORE::CDynamicBuffer block;
                DRN::CDRNWireWriter blockWriter( block );
                blockWriter.WriteVarUInt32( 2 );
                blockWriter.WriteVarUInt32( 4 );
                blockWriter.WriteVarUInt32( 3 );
                fewerWriter.WriteSizedBytes( block.GetConstBufferPtr(), blockWriter.GetOffset() );
                DRN::CDRNWireReader reader( fewer.GetConstBufferPtr(), fewerWriter.GetOffset() );
                DRN::CDRNNamespaceInfo read;
                ASSERT_TRUE( DRN::CDRNNamespaceCodec::ReadHelloBody( reader, read ) && read.limitsAnnounced && 4 == read.maxOpenHandles && 3 == read.maxActiveReads && 0 == read.maxNamespaceRequests );
            }
            {
                // a block that says 5 numbers and holds 2; a block longer than what is left; an empty block
                CORE::CDynamicBuffer liar;
                DRN::CDRNWireWriter liarWriter( liar );
                liarWriter.WriteBytes( plain.GetConstBufferPtr(), plainSize );
                CORE::CDynamicBuffer block;
                DRN::CDRNWireWriter blockWriter( block );
                blockWriter.WriteVarUInt32( 5 );
                blockWriter.WriteVarUInt32( 1 );
                blockWriter.WriteVarUInt32( 2 );
                liarWriter.WriteSizedBytes( block.GetConstBufferPtr(), blockWriter.GetOffset() );
                DRN::CDRNWireReader reader( liar.GetConstBufferPtr(), liarWriter.GetOffset() );
                DRN::CDRNNamespaceInfo read;
                ASSERT_FALSE( DRN::CDRNNamespaceCodec::ReadHelloBody( reader, read ) );

                CORE::CDynamicBuffer tooLong;
                DRN::CDRNWireWriter tooLongWriter( tooLong );
                tooLongWriter.WriteBytes( plain.GetConstBufferPtr(), plainSize );
                tooLongWriter.WriteVarUInt32( 1000 );
                tooLongWriter.WriteVarUInt32( 1 );
                DRN::CDRNWireReader reader2( tooLong.GetConstBufferPtr(), tooLongWriter.GetOffset() );
                ASSERT_FALSE( DRN::CDRNNamespaceCodec::ReadHelloBody( reader2, read ) );

                CORE::CDynamicBuffer empty;
                DRN::CDRNWireWriter emptyWriter( empty );
                emptyWriter.WriteBytes( plain.GetConstBufferPtr(), plainSize );
                emptyWriter.WriteVarUInt32( 0 );
                DRN::CDRNWireReader reader3( empty.GetConstBufferPtr(), emptyWriter.GetOffset() );
                ASSERT_FALSE( DRN::CDRNNamespaceCodec::ReadHelloBody( reader3, read ) );
            }

            // random bodies after a valid beginning never crash and never read past their end
            srand( 12345 );
            for ( UInt32 round=0; round<500; ++round )
            {
                CORE::CDynamicBuffer noise;
                DRN::CDRNWireWriter noiseWriter( noise );
                noiseWriter.WriteBytes( plain.GetConstBufferPtr(), plainSize );
                UInt32 extra = (UInt32) ( rand() % 24 );
                for ( UInt32 i=0; i<extra; ++i )
                    noiseWriter.WriteUInt8( (UInt8) ( rand() & 0xFF ) );
                DRN::CDRNWireReader reader( noise.GetConstBufferPtr(), noiseWriter.GetOffset() );
                DRN::CDRNNamespaceInfo read;
                DRN::CDRNNamespaceCodec::ReadHelloBody( reader, read );
                ASSERT_TRUE( reader.GetPosition() <= noiseWriter.GetOffset() );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: a client with an announced request limit of 2 and 8 callers never has more than 2 requests under way, none is refused BUSY; without the announcement or without the throttle the server refuses" )
    try
    {
        CLimitFixture fixture( "lim_throttle" );
        REQUIRE_TRUE( fixture.Start() );
        DRN::CDRNResourceSessionComponent& server = fixture.Server();
        fixture.gate->SetStatDelay( 100 );
        DRN::CDRNResourceTransferSettings settings = BaseSettings();
        settings.maxNamespaceRequestsPerPeer = 2;
        server.SetSettings( settings );

        // announced: 8 callers, 3 calls each
        {
            DRN::CDRNNamespaceClient& client = fixture.Client( 1 );
            DRN::CDRNNamespaceInfo info;
            REQUIRE_TRUE( client.Negotiate( info ).IsOk() && 2 == client.GetRequestLimit() );
            DRN::CDRNResourceTransferStats before = server.GetStats();
            fixture.gate->ResetStatsPeak();
            CSampler sampler( server, false );
            CJobThread samplerThread( sampler );
            REQUIRE_TRUE( samplerThread.Start() );
            GUCEF::vector< CStatJob* > jobs;
            GUCEF::vector< CJobThread* > threads;
            for ( UInt32 i=0; i<8; ++i )
            {
                jobs.push_back( GUCEF_NEW CStatJob( client, "gate", 3 ) );
                threads.push_back( GUCEF_NEW CJobThread( *jobs.back() ) );
            }
            for ( UInt32 i=0; i<8; ++i )
                REQUIRE_TRUE( threads[ i ]->Start() );
            UInt32 ok = 0, busy = 0, other = 0;
            for ( UInt32 i=0; i<8; ++i )
            {
                REQUIRE_TRUE( threads[ i ]->Join( 60000 ) );
                ok += jobs[ i ]->okCount;
                busy += jobs[ i ]->busyCount;
                other += jobs[ i ]->timeoutCount + jobs[ i ]->otherCount;
                GUCEF_DELETE threads[ i ];
                GUCEF_DELETE jobs[ i ];
            }
            sampler.stop.Store( true );
            samplerThread.Join( 5000 );
            DRN::CDRNResourceTransferStats after = server.GetStats();
            ASSERT_TRUE( 24 == ok && 0 == busy && 0 == other );
            ASSERT_TRUE( after.refusedByMemberLimit == before.refusedByMemberLimit && after.namespaceRequestsRefused == before.namespaceRequestsRefused );
            ASSERT_TRUE( sampler.highest <= 2 && fixture.gate->GetStatsPeak() <= 2 );
            ASSERT_TRUE( 2 == client.GetPeakRequestsInFlight() && 0 == client.GetRequestsInFlight() && 0 == client.GetRequestsWaiting() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN limits: 8 callers, announced limit 2: the serving side saw at most " << sampler.highest << " requests at once, the provider " << fixture.gate->GetStatsPeak() );
        }

        // the client's throttle off: the same load runs into the member limit
        {
            DRN::CDRNNamespaceClient& client = fixture.Client( 2 );
            client.SetSelfThrottle( false );
            GUCEF::vector< CStatJob* > jobs;
            GUCEF::vector< CJobThread* > threads;
            for ( UInt32 i=0; i<8; ++i )
            {
                jobs.push_back( GUCEF_NEW CStatJob( client, "gate", 3 ) );
                threads.push_back( GUCEF_NEW CJobThread( *jobs.back() ) );
            }
            for ( UInt32 i=0; i<8; ++i )
                REQUIRE_TRUE( threads[ i ]->Start() );
            UInt32 busy = 0;
            for ( UInt32 i=0; i<8; ++i )
            {
                REQUIRE_TRUE( threads[ i ]->Join( 60000 ) );
                busy += jobs[ i ]->busyCount;
                GUCEF_DELETE threads[ i ];
                GUCEF_DELETE jobs[ i ];
            }
            ASSERT_TRUE( busy > 0 );
        }

        // a serving side that does not announce: the client has no limit to keep to and meets the refusals as before
        {
            settings.announceLimits = false;
            server.SetSettings( settings );
            DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );
            DRN::CDRNNamespaceInfo info;
            REQUIRE_TRUE( client.Negotiate( info ).IsOk() && !info.limitsAnnounced && 0 == client.GetRequestLimit() );
            GUCEF::vector< CStatJob* > jobs;
            GUCEF::vector< CJobThread* > threads;
            for ( UInt32 i=0; i<8; ++i )
            {
                jobs.push_back( GUCEF_NEW CStatJob( client, "gate", 3 ) );
                threads.push_back( GUCEF_NEW CJobThread( *jobs.back() ) );
            }
            for ( UInt32 i=0; i<8; ++i )
                REQUIRE_TRUE( threads[ i ]->Start() );
            UInt32 busy = 0;
            for ( UInt32 i=0; i<8; ++i )
            {
                REQUIRE_TRUE( threads[ i ]->Join( 60000 ) );
                busy += jobs[ i ]->busyCount;
                GUCEF_DELETE threads[ i ];
                GUCEF_DELETE jobs[ i ];
            }
            ASSERT_TRUE( busy > 0 );
        }
        ASSERT_TRUE( fixture.WaitAllZero() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: ListAll keeps within the announced listing limit with 6 callers and none is refused; queued callers go in first come first served; a caller that waits too long ends with the timeout result" )
    try
    {
        CLimitFixture fixture( "lim_listing" );
        REQUIRE_TRUE( fixture.Start() );
        DRN::CDRNResourceSessionComponent& server = fixture.Server();
        DRN::CDRNResourceTransferSettings settings = BaseSettings();
        settings.maxNamespaceListingsPerPeer = 2;
        server.SetSettings( settings );
        {
            DRN::CDRNNamespaceClient& client = fixture.Client( 1 );
            DRN::CDRNNamespaceInfo info;
            REQUIRE_TRUE( client.Negotiate( info ).IsOk() && 2 == client.GetListingLimit() );
            DRN::CDRNResourceTransferStats before = server.GetStats();
            CSampler sampler( server, true );
            CJobThread samplerThread( sampler );
            REQUIRE_TRUE( samplerThread.Start() );
            GUCEF::vector< CListAllJob* > jobs;
            GUCEF::vector< CJobThread* > threads;
            for ( UInt32 i=0; i<6; ++i )
            {
                jobs.push_back( GUCEF_NEW CListAllJob( client, "big", 10 ) );
                threads.push_back( GUCEF_NEW CJobThread( *jobs.back() ) );
            }
            for ( UInt32 i=0; i<6; ++i )
                REQUIRE_TRUE( threads[ i ]->Start() );
            UInt32 complete = 0, busy = 0;
            for ( UInt32 i=0; i<6; ++i )
            {
                REQUIRE_TRUE( threads[ i ]->Join( 60000 ) );
                if ( jobs[ i ]->outcome.ok && 200 == jobs[ i ]->entryCount )
                    ++complete;
                if ( jobs[ i ]->outcome.busy )
                    ++busy;
                GUCEF_DELETE threads[ i ];
                GUCEF_DELETE jobs[ i ];
            }
            sampler.stop.Store( true );
            samplerThread.Join( 5000 );
            ASSERT_TRUE( 6 == complete && 0 == busy );
            ASSERT_TRUE( sampler.highest >= 1 && sampler.highest <= 2 && client.GetPeakListingsInFlight() <= 2 );
            ASSERT_TRUE( server.GetStats().refusedByMemberLimit == before.refusedByMemberLimit );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN limits: 6 listings, announced limit 2: the serving side had at most " << sampler.highest << " under way" );
        }

        // a listing that waits for its place for longer than its timeout ends as a timeout
        {
            settings.maxNamespaceListingsPerPeer = 1;
            server.SetSettings( settings );
            DRN::CDRNNamespaceClient& client = fixture.Client( 2 );
            DRN::CDRNNamespaceInfo info;
            REQUIRE_TRUE( client.Negotiate( info ).IsOk() && 1 == client.GetListingLimit() );
            CListAllJob slow( client, "big", 1, 30000 );
            CJobThread slowThread( slow );
            REQUIRE_TRUE( slowThread.Start() );
            UInt64 start = NowMs();
            while ( 1 != client.GetListingsInFlight() && NowMs() - start < 10000 )
                MT::PrecisionDelay( 1 );
            ASSERT_TRUE( 1 == client.GetListingsInFlight() );
            TEntries entries;
            CORE::CString revision;
            bool changed = false;
            UInt64 begin = NowMs();
            DRN::CDRNNamespaceResult result = client.ListAll( "files", "big", entries, revision, changed, 1000000, 10, 2, 30 );
            ASSERT_TRUE( DRN::DRN_NSRESULT_TIMEOUT == result.code && NowMs() - begin < 5000 );
            ASSERT_TRUE( 0 == client.GetListingsWaiting() );
            REQUIRE_TRUE( slowThread.Join( 60000 ) );
            ASSERT_TRUE( slow.outcome.ok && 200 == slow.entryCount );
            ASSERT_TRUE( 0 == client.GetListingsInFlight() );
            REQUIRE_TRUE( client.ListAll( "files", "big", entries, revision, changed, 1000000, 50, 2, 10000 ).IsOk() && 200 == entries.size() );
        }
        ASSERT_TRUE( fixture.WaitAllZero() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: callers wait for the request budget in the order they came, and a caller that waits too long ends with the timeout result and leaves nothing behind" )
    try
    {
        CLimitFixture fixture( "lim_fifo" );
        REQUIRE_TRUE( fixture.Start() );
        DRN::CDRNResourceSessionComponent& server = fixture.Server();
        DRN::CDRNResourceTransferSettings settings = BaseSettings();
        settings.maxNamespaceRequestsPerPeer = 1;
        server.SetSettings( settings );
        fixture.gate->SetStatDelay( 60 );
        DRN::CDRNNamespaceClient& client = fixture.Client( 1 );
        DRN::CDRNNamespaceInfo info;
        REQUIRE_TRUE( client.Negotiate( info ).IsOk() && 1 == client.GetRequestLimit() );

        // five callers, each one starts when the one before it is in the queue: they have to finish in that order
        {
            MT::CTAtomic< UInt32 > order( 0 );
            GUCEF::vector< CStatJob* > jobs;
            GUCEF::vector< CJobThread* > threads;
            for ( UInt32 i=0; i<5; ++i )
            {
                jobs.push_back( GUCEF_NEW CStatJob( client, "gate", 1 ) );
                jobs.back()->orderCounter = &order;
                threads.push_back( GUCEF_NEW CJobThread( *jobs.back() ) );
            }
            for ( UInt32 i=0; i<5; ++i )
            {
                REQUIRE_TRUE( threads[ i ]->Start() );
                UInt64 start = NowMs();
                while ( client.GetRequestsWaiting() + client.GetRequestsInFlight() < i + 1 && NowMs() - start < 10000 )
                    MT::PrecisionDelay( 1 );
                ASSERT_TRUE( client.GetRequestsWaiting() + client.GetRequestsInFlight() >= i + 1 );
            }
            for ( UInt32 i=0; i<5; ++i )
            {
                REQUIRE_TRUE( threads[ i ]->Join( 30000 ) );
                ASSERT_TRUE( jobs[ i ]->outcome.ok && jobs[ i ]->finishedAs == i );
                GUCEF_DELETE threads[ i ];
                GUCEF_DELETE jobs[ i ];
            }
        }

        // the budget is used by a request that is held: another caller gives up at its timeout and is no longer in the queue
        {
            fixture.gate->SetStatDelay( 0 );
            fixture.gate->Block( false, true );
            CStatJob holder( client, "gate", 1 );
            CJobThread holderThread( holder );
            REQUIRE_TRUE( holderThread.Start() );
            UInt64 start = NowMs();
            while ( 0 == server.GetNamespaceServingCount( 0 ) && NowMs() - start < 10000 )
                MT::PrecisionDelay( 1 );
            ASSERT_TRUE( 1 == client.GetRequestsInFlight() );
            UInt64 madeBefore = server.GetStats().namespaceRequestsMade + fixture.Member( 1 ).GetStats().namespaceRequestsMade;
            TEntries entries;
            UInt64 begin = NowMs();
            DRN::CDRNNamespaceResult result = client.Stat( "gate", Strings1( "f1.bin" ), TStrings(), entries, 300 );
            UInt64 waited = NowMs() - begin;
            ASSERT_TRUE( DRN::DRN_NSRESULT_TIMEOUT == result.code && waited >= 250 && waited < 5000 );
            ASSERT_TRUE( 0 == client.GetRequestsWaiting() && 1 == client.GetRequestsInFlight() );
            ASSERT_TRUE( 1 == server.GetNamespaceServingCount( 0 ) );
            ASSERT_TRUE( server.GetStats().namespaceRequestsMade + fixture.Member( 1 ).GetStats().namespaceRequestsMade == madeBefore );
            fixture.gate->Open();
            REQUIRE_TRUE( holderThread.Join( 30000 ) );
            ASSERT_TRUE( holder.outcome.ok && 0 == client.GetRequestsInFlight() && 0 == client.GetRequestsWaiting() );
            REQUIRE_TRUE( client.Stat( "gate", Strings1( "f1.bin" ), TStrings(), entries, 10000 ).IsOk() );
        }
        ASSERT_TRUE( fixture.WaitAllZero() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: the request gate hands out its budget in order, lets a waiter go at its timeout, and lets waiting callers in when the limit grows" )
    try
    {
        DRN::CDRNRequestGate gate( 2 );
        ASSERT_TRUE( gate.Acquire( 0 ) && gate.Acquire( 0 ) );
        ASSERT_FALSE( gate.Acquire( 0 ) );
        UInt64 begin = NowMs();
        ASSERT_FALSE( gate.Acquire( 80 ) );
        ASSERT_TRUE( NowMs() - begin >= 60 && 0 == gate.GetWaiting() && 2 == gate.GetInFlight() );
        gate.Release();
        ASSERT_TRUE( 1 == gate.GetInFlight() && gate.Acquire( 0 ) && 2 == gate.GetPeakInFlight() );

        // growing the limit lets the waiting one in
        class CGateWaiter : public CJob
        {
            public:
            DRN::CDRNRequestGate& gate;
            bool got;
            explicit CGateWaiter( DRN::CDRNRequestGate& g ) : gate( g ), got( false ) {}
            virtual void Run( void ) GUCEF_VIRTUAL_OVERRIDE { got = gate.Acquire( 20000 ); }
        };
        CGateWaiter waiter( gate );
        CJobThread waiterThread( waiter );
        REQUIRE_TRUE( waiterThread.Start() );
        UInt64 start = NowMs();
        while ( 1 != gate.GetWaiting() && NowMs() - start < 10000 )
            MT::PrecisionDelay( 1 );
        ASSERT_TRUE( 1 == gate.GetWaiting() );
        gate.SetLimit( 3 );
        REQUIRE_TRUE( waiterThread.Join( 10000 ) );
        ASSERT_TRUE( waiter.got && 3 == gate.GetInFlight() && 0 == gate.GetWaiting() );

        // 0 is no maximum
        gate.SetLimit( 0 );
        for ( UInt32 i=0; i<20; ++i )
            ASSERT_TRUE( gate.Acquire( 0 ) );
        ASSERT_TRUE( 23 == gate.GetInFlight() );
        for ( UInt32 i=0; i<23; ++i )
            gate.Release();
        ASSERT_TRUE( 0 == gate.GetInFlight() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    RemoveTestDirectories();
}

/*-------------------------------------------------------------------------*/
