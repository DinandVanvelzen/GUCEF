/*
 *  vfspluginDRN_TestApp: tests of the DRN VFS plugin
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

#include <exception>

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "vfspluginDRN_CDrnBridgeArchive.h"
#include "vfspluginDRN_CDrnBridgeConnection.h"
#include "vfspluginDRN_CDrnBridgeGlobal.h"
#include "vfspluginDRN_CDrnBridgeMemberLink.h"

#include "TestVfsBlockCacheUtils.h"
#include "TestDrnBridgeFixture.h"
#include "TestDrnBridgeLimits.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace DRNBRIDGETEST;
using namespace VFSPLUGIN::DRNBRIDGE;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

static const UInt32 FileBytes = 70000;
static const UInt32 FileCount = 20;

/*-------------------------------------------------------------------------*/

UInt64
NowMs( void )
{
    return DRN::CDRNClock::NowInMilliseconds();
}

bool
HasText( const CORE::CString& text, const CORE::CString& part )
{
    return text.HasSubstr( part ) >= 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
BedDirectory( const CORE::CString& name )
{
    CORE::CString base = CORE::GetEnv( "GUCEF_TEST_SCRATCH_DIR" );
    if ( base.IsNULLOrEmpty() )
        base = CORE::TempDir();
    CORE::CString dir = CORE::CombinePath( base, "vfsdrn-limits-" + CORE::UInt32ToString( MT::GetProcessID() ) + "-" + name );
    if ( CORE::DirExists( dir ) )
        CORE::DeleteDir( dir, true );
    CORE::CreateDirs( dir );
    return dir;
}

/*-------------------------------------------------------------------------*/

UInt8
ByteAt( UInt32 file, UInt64 position )
{
    return (UInt8) ( file * 13 + position * 7 + ( position >> 9 ) );
}

bool
WriteSeeded( const CORE::CString& path, UInt32 file, UInt32 size )
{
    CORE::CFileAccess out( path, "wb" );
    if ( !out.Opened() )
        return false;
    CORE::CDynamicBuffer block( size + 1, true );
    UInt8* bytes = static_cast< UInt8* >( block.GetBufferPtr() );
    for ( UInt32 i=0; i<size; ++i )
        bytes[ i ] = ByteAt( file, i );
    bool ok = size == out.Write( block.GetConstBufferPtr(), 1, size );
    out.Flush();
    return ok;
}

/** Reads count bytes at offset through the access and checks them */
bool
ReadChecked( CORE::CIOAccess& access, UInt32 file, UInt64 offset, UInt32 count )
{
    CORE::CDynamicBuffer buffer( count, true );
    if ( 0 != access.Setpos( offset ) || count != access.Read( buffer.GetBufferPtr(), 1, count ) )
        return false;
    const UInt8* bytes = static_cast< const UInt8* >( buffer.GetConstBufferPtr() );
    for ( UInt32 i=0; i<count; ++i )
        if ( bytes[ i ] != ByteAt( file, offset + i ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** A share whose stats take their time */
class CSlowStatProvider : public DRN::CDRNFileSystemAccessResourceProvider
{
    public:

    UInt32 m_delayMs;
    UInt32 m_listDelayMs;

    CSlowStatProvider( const CORE::CString& root, UInt32 delayMs, UInt32 listDelayMs = 0 )
        : DRN::CDRNFileSystemAccessResourceProvider( NativeFileSystem(), root, CORE::CUri( "file://" + root ) ), m_delayMs( delayMs ), m_listDelayMs( listDelayMs ) {}

    virtual DRN::TDRNResourceStatus OpenEnumerator( const CORE::CString& dirName, DRN::CIDRNNamespaceEnumeratorPtr& enumerator ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 0 != m_listDelayMs )
            MT::PrecisionDelay( m_listDelayMs );
        return DRN::CDRNFileSystemAccessResourceProvider::OpenEnumerator( dirName, enumerator );
    }

    virtual DRN::TDRNResourceStatus Stat( const CORE::CString& name, DRN::CDRNNamespaceEntry& entry ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 0 != m_delayMs )
            MT::PrecisionDelay( m_delayMs );
        return DRN::CDRNFileSystemAccessResourceProvider::Stat( name, entry );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  A policy of the serving node that answers the read of some names with retry-later until they are ready (the data of a share that is being got
 *  ready), refuses some for good, and counts what it was asked
 */
class CWarmingPolicy : public DRN::CDRNDefaultAccessPolicy
{
    public:

    MT::CTAtomic< UInt32 > askedBusy;
    MT::CTAtomic< UInt32 > askedRefused;

    CWarmingPolicy( void ) : DRN::CDRNDefaultAccessPolicy(), askedBusy( 0 ), askedRefused( 0 ), m_lock(), m_busyUntil(), m_refused() {}

    /** The read of the name is answered busy for this long from now on */
    void BusyFor( const CORE::CString& name, UInt32 durationMs )
    {
        MT::CScopeMutex lock( m_lock );
        m_busyUntil[ name ] = NowMs() + durationMs;
    }

    void Ready( const CORE::CString& name )
    {
        MT::CScopeMutex lock( m_lock );
        m_busyUntil.erase( name );
    }

    void RefuseFor( const CORE::CString& name )
    {
        MT::CScopeMutex lock( m_lock );
        m_refused.insert( name );
    }

    virtual bool DecideReadResourceAs( UInt32 /* peerId */, const DRN::CDRNAuthResult& /* auth */, const CORE::CString& /* providerName */, const CORE::CString& resourceName,
                                       bool sharedByProvider, CORE::CString& refusal, bool& retryLater ) GUCEF_VIRTUAL_OVERRIDE
    {
        retryLater = false;
        MT::CScopeMutex lock( m_lock );
        if ( m_refused.find( resourceName ) != m_refused.end() )
        {
            askedRefused.FetchAdd( 1 );
            refusal = "this file is for the owner of the share";
            return false;
        }
        GUCEF::map< CORE::CString, UInt64 >::const_iterator i = m_busyUntil.find( resourceName );
        if ( i != m_busyUntil.end() && NowMs() < (*i).second )
        {
            askedBusy.FetchAdd( 1 );
            refusal = "the licence of this file is being looked up, ask again in a moment";
            retryLater = true;
            return false;
        }
        return sharedByProvider;
    }

    private:

    MT::CMutex m_lock;
    GUCEF::map< CORE::CString, UInt64 > m_busyUntil;
    CORE::CString::StringSet m_refused;
};

/*-------------------------------------------------------------------------*/

/** A directory of files, served by a node over a loopback link to a client node */
class CLimitBed
{
    public:

    CORE::CString root;
    CLoopbackShare* share;

    explicit CLimitBed( const CORE::CString& name ) : root( BedDirectory( name ) ), share( GUCEF_NULL ), m_name( name ) {}

    ~CLimitBed()
    {
        Stop();
    }

    bool Start( UInt32 statDelayMs = 0, UInt32 listDelayMs = 0 )
    {
        for ( UInt32 i=0; i<FileCount; ++i )
            if ( !WriteSeeded( CORE::CombinePath( root, "f" + CORE::ToString( i ) + ".bin" ), i, FileBytes ) )
                return false;
        share = GUCEF_NEW CLoopbackShare( "vfsdrn_lim_" + m_name + "_" + CORE::UInt32ToString( MT::GetProcessID() ) );
        DRN::CIDRNResourceProviderPtr provider( GUCEF_NEW CSlowStatProvider( root, statDelayMs, listDelayMs ) );
        if ( !share->Start( root, provider ) )
            return false;
        CDrnBridgeGlobal::Instance();
        return true;
    }

    void Stop( void )
    {
        if ( GUCEF_NULL != share )
        {
            share->Stop();
            GUCEF_DELETE share;
            share = GUCEF_NULL;
        }
        if ( !root.IsNULLOrEmpty() && CORE::DirExists( root ) )
            CORE::DeleteDir( root, true );
    }

    DRN::CDRNResourceSessionComponent& Server( void ) { return share->Server().GetResources(); }

    /** A connection to the serving member as "sequencer": the way a drn:// source names it, so that every mount of it is the same (session, member) */
    CDrnBridgeConnectionPtr MakeConnection( void )
    {
        return CDrnBridgeConnectionPtr( GUCEF_NEW CDrnBridgeConnection( share->Client(), CDrnBridgeConnection::SequencerMemberName ) );
    }

    private:

    CORE::CString m_name;
};

/*-------------------------------------------------------------------------*/

/** One mount of the share, by the typed route, with a connection of its own */
class CMount
{
    public:

    CDrnBridgeConnectionPtr connection;
    CDrnBridgeArchive* archive;

    CMount( void ) : connection(), archive( GUCEF_NULL ) {}

    ~CMount()
    {
        Unload();
    }

    bool Load( CLimitBed& bed, const CDrnBridgeSettings& settings, const CORE::CString& name )
    {
        connection = bed.MakeConnection();
        archive = GUCEF_NEW CDrnBridgeArchive();
        return archive->LoadArchive( connection, CLoopbackShare::ShareName, settings, name );
    }

    void Unload( void )
    {
        if ( GUCEF_NULL != archive )
        {
            archive->UnloadArchive();
            GUCEF_DELETE archive;
            archive = GUCEF_NULL;
        }
        connection.Unlink();
    }
};

/*-------------------------------------------------------------------------*/

CDrnBridgeSettings
CacheOffSettings( void )
{
    CDrnBridgeSettings settings;
    settings.metadataTtlMs = 0;
    settings.negativeTtlMs = 0;
    return settings;
}

/*-------------------------------------------------------------------------*/

/** Calls the archive in a loop on a thread of its own: stat of files, or the opening and reading of files */
class CArchiveWorker : public MT::CActiveObject
{
    public:

    enum TWork { WORK_STAT, WORK_OPEN_AND_READ, WORK_LIST };

    CArchiveWorker( CDrnBridgeArchive& archive, TWork work, UInt32 rounds, const MT::CTAtomic< bool >* stop = GUCEF_NULL )
        : MT::CActiveObject(), m_archive( archive ), m_work( work ), m_rounds( rounds ), m_stop( stop ), m_done( false ), m_calls( 0 ), m_failures( 0 ), m_retryLater( 0 ), m_maxLatencyMs( 0 )
    {}

    virtual ~CArchiveWorker() GUCEF_VIRTUAL_OVERRIDE { Deactivate( false, true ); }

    bool IsDone( void ) const { return m_done.Load(); }
    UInt32 GetCalls( void ) const { return m_calls.Load(); }
    UInt32 GetFailures( void ) const { return m_failures.Load(); }
    UInt32 GetRetryLater( void ) const { return m_retryLater.Load(); }
    UInt64 GetMaxLatencyMs( void ) const { return m_maxLatencyMs.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        try
        {
            if ( WORK_STAT == m_work )
                DoStats();
            else
            if ( WORK_LIST == m_work )
                DoLists();
            else
                DoOpenAndRead();
        }
        catch ( ... )
        {
            m_failures.FetchAdd( 1 );
        }
        m_done.Store( true );
        return true;
    }

    private:

    void NoteLatency( UInt64 startMs )
    {
        UInt64 took = NowMs() - startMs;
        if ( took > m_maxLatencyMs.Load() )
            m_maxLatencyMs.Store( took );
    }

    void DoStats( void )
    {
        for ( UInt32 i=0; ( 0 == m_rounds || i<m_rounds ) && ( GUCEF_NULL == m_stop || !m_stop->Load() ); ++i )
        {
            UInt64 start = NowMs();
            UInt64 size = m_archive.GetFileSize( "f" + CORE::ToString( i % FileCount ) + ".bin" );
            NoteLatency( start );
            m_calls.FetchAdd( 1 );
            if ( FileBytes != size )
                m_failures.FetchAdd( 1 );
        }
    }

    void DoLists( void )
    {
        for ( UInt32 i=0; ( 0 == m_rounds || i<m_rounds ) && ( GUCEF_NULL == m_stop || !m_stop->Load() ); ++i )
        {
            UInt64 start = NowMs();
            VFS::CArchive::TStringVector names;
            m_archive.GetFileList( names, "", "", false, false );
            NoteLatency( start );
            m_calls.FetchAdd( 1 );
            if ( FileCount != names.size() )
                m_failures.FetchAdd( 1 );
        }
    }

    void DoOpenAndRead( void )
    {
        // every file stays open: the handles of the mount are used by the files in turn, as many as the mount is allowed to hold
        GUCEF::vector< VFS::TBasicVfsResourcePtr > files;
        for ( UInt32 i=0; i<FileCount; ++i )
        {
            VFS::TBasicVfsResourcePtr file = m_archive.GetFile( "f" + CORE::ToString( i ) + ".bin", "rb", 0, false );
            m_calls.FetchAdd( 1 );
            if ( file.IsNULL() || file->GetAccess().IsNULL() )
            {
                m_failures.FetchAdd( 1 );
                NoteRetryLater();
            }
            files.push_back( file );
        }
        for ( UInt32 round=0; round<m_rounds; ++round )
        {
            for ( UInt32 i=0; i<FileCount; ++i )
            {
                if ( files[ i ].IsNULL() || files[ i ]->GetAccess().IsNULL() )
                    continue;
                CORE::IOAccessPtr access = files[ i ]->GetAccess();
                m_calls.FetchAdd( 1 );
                if ( !ReadChecked( *access, i, ( round * 5000 ) % ( FileBytes - 4096 ), 4096 ) )
                {
                    m_failures.FetchAdd( 1 );
                    NoteRetryLater();
                }
            }
        }
    }

    void NoteRetryLater( void )
    {
        // the last result of this thread says whether the call ended as "try again later"
        if ( CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER == m_archive.GetLastResult().fileSystemResult )
            m_retryLater.FetchAdd( 1 );
    }

    CDrnBridgeArchive& m_archive;
    TWork m_work;
    UInt32 m_rounds;
    const MT::CTAtomic< bool >* m_stop;
    MT::CTAtomic< bool > m_done;
    MT::CTAtomic< UInt32 > m_calls;
    MT::CTAtomic< UInt32 > m_failures;
    MT::CTAtomic< UInt32 > m_retryLater;
    MT::CTAtomic< UInt64 > m_maxLatencyMs;
};

/*-------------------------------------------------------------------------*/

/** Keeps the highest of what the serving side counts of one kind while a test runs */
class CCounterSampler : public MT::CActiveObject
{
    public:

    CCounterSampler( DRN::CDRNResourceSessionComponent& server, bool handles )
        : MT::CActiveObject(), m_server( server ), m_handles( handles ), m_stop( false ), m_highest( 0 ) {}

    virtual ~CCounterSampler() GUCEF_VIRTUAL_OVERRIDE { Stop(); }

    void Stop( void )
    {
        m_stop.Store( true );
        Deactivate( false, true );
    }

    UInt32 GetHighest( void ) const { return m_highest.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        while ( !m_stop.Load() )
        {
            UInt32 value = m_handles ? m_server.GetOpenHandleCount( 0 ) : m_server.GetNamespaceServingCount( 0 );
            if ( value > m_highest.Load() )
                m_highest.Store( value );
            MT::PrecisionDelay( 1 );
        }
        return true;
    }

    private:

    DRN::CDRNResourceSessionComponent& m_server;
    bool m_handles;
    MT::CTAtomic< bool > m_stop;
    MT::CTAtomic< UInt32 > m_highest;
};

/*-------------------------------------------------------------------------*/

/** Lets go of a handle of the test's own after a while: another user of the member's budget that ends */
class CLetGo : public MT::CActiveObject
{
    public:

    CLetGo( DRN::CDRNResourceHandlePtr& handle, UInt32 afterMs ) : MT::CActiveObject(), m_handle( handle ), m_afterMs( afterMs ) {}

    virtual ~CLetGo() GUCEF_VIRTUAL_OVERRIDE { Deactivate( false, true ); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::PrecisionDelay( m_afterMs );
        m_handle.Unlink();
        return true;
    }

    private:

    DRN::CDRNResourceHandlePtr& m_handle;
    UInt32 m_afterMs;
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDrnBridgeLimitTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DRN BRIDGE LIMIT TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "vfspluginDRN.Limits" );

    GUCEF_TESTFW_TESTCASE( "Limits: a read that the serving side answers with retry-later while its data is got ready is tried again by the bridge with the default settings until it is served; a refusal that is final is immediate and is not tried again; retries that run out end as retry-later, and the next call is served" )
    try
    {
        CLimitBed bed( "warming" );
        REQUIRE_TRUE( bed.Start() );
        CWarmingPolicy policy;
        bed.share->Server().GetNode().SetAccessPolicy( &policy );
        try
        {
            CDrnBridgeSettings settings;
            ASSERT_TRUE( settings.busyRetries >= 8 && 25 == settings.busyBackoffMs );
            CMount mount;
            REQUIRE_TRUE( mount.Load( bed, settings, "Warm" ) );

            // the data is ready after 1.5 s: more than the first retries wait, well within what the default waits
            policy.BusyFor( "f3.bin", 1500 );
            const UInt64 retriesBefore = mount.connection->GetBusyRetryCount();
            UInt64 begin = NowMs();
            VFS::TBasicVfsResourcePtr file = mount.archive->GetFile( "f3.bin", "rb", 0, false );
            UInt64 took = NowMs() - begin;
            REQUIRE_TRUE( !file.IsNULL() && !file->GetAccess().IsNULL() );
            ASSERT_TRUE( ReadChecked( *file->GetAccess(), 3, 0, 2000 ) );
            ASSERT_TRUE( took >= 1400 && took < 6000 );
            ASSERT_TRUE( mount.connection->GetBusyRetryCount() - retriesBefore >= 5 );
            ASSERT_TRUE( policy.askedBusy.Load() >= 6 );
            file.Unlink();

            // a refusal that is a refusal: at once, with the words of the serving side, no retry
            policy.RefuseFor( "f4.bin" );
            const UInt64 retriesRefused = mount.connection->GetBusyRetryCount();
            begin = NowMs();
            file = mount.archive->GetFile( "f4.bin", "rb", 0, false );
            took = NowMs() - begin;
            ASSERT_TRUE( file.IsNULL() && took < 400 );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_ACCESS_DENIED == mount.archive->GetLastResult().fileSystemResult );
            ASSERT_TRUE( HasText( mount.archive->GetLastResult().message, "for the owner of the share" ) );
            ASSERT_TRUE( retriesRefused == mount.connection->GetBusyRetryCount() );

            // retries that are used up: the retry-later result with the words of the serving side, and the call after the data is ready is served
            CDrnBridgeSettings brief;
            brief.busyRetries = 2;
            brief.busyBackoffMs = 10;
            CMount quick;
            REQUIRE_TRUE( quick.Load( bed, brief, "Brief" ) );
            policy.BusyFor( "f5.bin", 60000 );
            begin = NowMs();
            file = quick.archive->GetFile( "f5.bin", "rb", 0, false );
            took = NowMs() - begin;
            ASSERT_TRUE( file.IsNULL() && took < 1000 );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER == quick.archive->GetLastResult().fileSystemResult );
            ASSERT_TRUE( HasText( quick.archive->GetLastResult().message, "being looked up" ) );
            ASSERT_TRUE( 2 == quick.connection->GetBusyRetryCount() );
            policy.Ready( "f5.bin" );
            file = quick.archive->GetFile( "f5.bin", "rb", 0, false );
            ASSERT_TRUE( !file.IsNULL() && !file->GetAccess().IsNULL() && ReadChecked( *file->GetAccess(), 5, 0, 2000 ) );
            file.Unlink();
        }
        catch ( ... )
        {
            bed.share->Server().GetNode().SetAccessPolicy( GUCEF_NULL );
            throw;
        }
        bed.share->Server().GetNode().SetAccessPolicy( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: the listing and the stat that the serving side refuses as busy are tried again like the open, and the caller gets whole answers" )
    try
    {
        CLimitBed bed( "listbusy" );
        REQUIRE_TRUE( bed.Start( 30, 30 ) );
        CDrnBridgeSettings seed;
        CMount first;
        REQUIRE_TRUE( first.Load( bed, seed, "Seed" ) );
        ASSERT_TRUE( FileBytes == first.archive->GetFileSize( "f0.bin" ) );

        // one request of the namespace at a time is allowed, the client does not throttle itself: the others are refused as busy
        DRN::CDRNResourceTransferSettings tight = bed.Server().GetSettings();
        tight.maxNamespaceRequestsPerPeer = 1;
        bed.Server().SetSettings( tight );
        CDrnBridgeSettings patient = CacheOffSettings();
        patient.busyRetries = 12;
        patient.busyBackoffMs = 20;
        CMount mount;
        REQUIRE_TRUE( mount.Load( bed, patient, "Lists" ) );
        {
            CDrnBridgeMemberLinkPtr link = mount.connection->GetMemberLink();
            REQUIRE_TRUE( !link.IsNULL() );
            link->GetClient()->SetSelfThrottle( false );
        }
        const UInt64 retriesBefore = mount.connection->GetBusyRetryCount();
        GUCEF::vector< CArchiveWorker* > workers;
        for ( UInt32 i=0; i<4; ++i )
            workers.push_back( GUCEF_NEW CArchiveWorker( *mount.archive, i < 2 ? CArchiveWorker::WORK_LIST : CArchiveWorker::WORK_STAT, 5 ) );
        for ( UInt32 i=0; i<4; ++i )
            REQUIRE_TRUE( workers[ i ]->Activate( GUCEF_NULL, 0, 0 ) );
        UInt32 failures = 0;
        UInt32 calls = 0;
        const UInt64 start = NowMs();
        for ( UInt32 i=0; i<4; ++i )
        {
            while ( !workers[ i ]->IsDone() && NowMs() - start < 60000 )
                MT::PrecisionDelay( 5 );
            failures += workers[ i ]->GetFailures();
            calls += workers[ i ]->GetCalls();
            workers[ i ]->Deactivate( false, true );
            GUCEF_DELETE workers[ i ];
        }
        ASSERT_TRUE( 0 == failures && 20 == calls );
        ASSERT_TRUE( mount.connection->GetBusyRetryCount() - retriesBefore > 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: a thread that must not wait is never held by the retries: while the serving side keeps a read busy, every call of a marked thread returns within 100 ms, the retries run on the pool, and the bytes arrive without a failure" )
    try
    {
        CLimitBed bed( "nowaitbusy" );
        REQUIRE_TRUE( bed.Start() );
        CBlockCacheTestFolder cacheFolder( "nowaitbusy-cache" );
        CWarmingPolicy policy;
        bed.share->Server().GetNode().SetAccessPolicy( &policy );
        try
        {
            CDrnBridgeSettings settings;
            settings.blockCacheFolder = cacheFolder.GetPath();
            settings.blockCacheMaxBytes = 8 * 1024 * 1024;
            settings.blockCacheBlockBytes = 65536;
            settings.cacheIdentity = "nowaitbusy";
            settings.fetchThreadPool = "VfsRemoteOpsBusy";
            CMount mount;
            REQUIRE_TRUE( mount.Load( bed, settings, "NoWait" ) );

            policy.BusyFor( "f6.bin", 1500 );
            const UInt64 begin = NowMs();
            UInt64 slowest = 0;
            UInt64 calls = 0;
            bool served = false;
            {
                VFS::CVfsThreadPolicy rule( VFS::MISSACTION_FAIL_AND_FETCH );
                VFS::CVfsThreadPolicyScope marked( rule );
                while ( !served && NowMs() - begin < 30000 )
                {
                    UInt64 t = NowMs();
                    VFS::TBasicVfsResourcePtr file = mount.archive->GetFile( "f6.bin", "rb", 0, false );
                    if ( !file.IsNULL() && !file->GetAccess().IsNULL() )
                    {
                        CORE::CDynamicBuffer buffer( 2000, true );
                        file->GetAccess()->Setpos( 0 );
                        const UInt32 got = file->GetAccess()->Read( buffer.GetBufferPtr(), 1, 2000 );
                        served = 2000 == got && ReadChecked( *file->GetAccess(), 6, 0, 2000 );
                    }
                    const UInt64 took = NowMs() - t;
                    if ( took > slowest )
                        slowest = took;
                    ++calls;
                    if ( !served )
                        MT::PrecisionDelay( 10 );
                }
            }
            ASSERT_TRUE( served );
            ASSERT_TRUE( slowest < 100 );
            ASSERT_TRUE( calls > 5 );
            ASSERT_TRUE( NowMs() - begin >= 1400 );                          // the bytes came when the data was ready, not before
            ASSERT_TRUE( mount.connection->GetBusyRetryCount() >= 3 );       // the retries were made, by the pool
            ASSERT_TRUE( policy.askedBusy.Load() >= 3 );
        }
        catch ( ... )
        {
            bed.share->Server().GetNode().SetAccessPolicy( GUCEF_NULL );
            throw;
        }
        bed.share->Server().GetNode().SetAccessPolicy( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: three mounts of the same member share one client and are released with the last mount" )
    try
    {
        CLimitBed bed( "share" );
        REQUIRE_TRUE( bed.Start() );
        CDrnBridgeGlobal* global = CDrnBridgeGlobal::Instance();
        ASSERT_TRUE( 0 == global->GetMemberLinkCount() );

        CMount mounts[ 3 ];
        for ( UInt32 i=0; i<3; ++i )
            REQUIRE_TRUE( mounts[ i ].Load( bed, CDrnBridgeSettings(), "Share" + CORE::ToString( i ) ) );
        ASSERT_TRUE( 1 == global->GetMemberLinkCount() );

        // every mount works through the one link: a call of each, then the open of a file of each
        for ( UInt32 i=0; i<3; ++i )
        {
            ASSERT_TRUE( FileBytes == mounts[ i ].archive->GetFileSize( "f" + CORE::ToString( i ) + ".bin" ) );
            VFS::TBasicVfsResourcePtr file = mounts[ i ].archive->GetFile( "f" + CORE::ToString( i ) + ".bin", "rb", 0, false );
            ASSERT_TRUE( !file.IsNULL() && !file->GetAccess().IsNULL() && ReadChecked( *file->GetAccess(), i, 100, 1000 ) );
        }
        {
            CDrnBridgeMemberLinkPtr first = mounts[ 0 ].connection->GetMemberLink();
            CDrnBridgeMemberLinkPtr second = mounts[ 1 ].connection->GetMemberLink();
            CDrnBridgeMemberLinkPtr third = mounts[ 2 ].connection->GetMemberLink();
            ASSERT_TRUE( !first.IsNULL() && first.GetPointerAlways() == second.GetPointerAlways() && first.GetPointerAlways() == third.GetPointerAlways() );
            ASSERT_TRUE( 1 == first->GetClientsMade() && 3 == first->GetHandleUserCount() && first->HasAnnouncedLimits() );
            ASSERT_TRUE( !first->GetClient().IsNULL() && first->GetClient()->GetRequestLimit() > 0 );
            ASSERT_TRUE( mounts[ 0 ].connection->GetStatCallCount() + mounts[ 0 ].connection->GetListCallCount() > 0 );
        }

        // a mount that goes leaves the others the link; the last one takes it with it
        mounts[ 0 ].Unload();
        ASSERT_TRUE( 1 == global->GetMemberLinkCount() );
        {
            CDrnBridgeMemberLinkPtr link = mounts[ 1 ].connection->GetMemberLink();
            ASSERT_TRUE( 2 == link->GetHandleUserCount() );
        }
        mounts[ 1 ].Unload();
        ASSERT_TRUE( 1 == global->GetMemberLinkCount() );
        mounts[ 2 ].Unload();
        ASSERT_TRUE( 0 == global->GetMemberLinkCount() );

        // mounting again makes a new one, and nothing of the open files is left on the serving side
        {
            CMount again;
            REQUIRE_TRUE( again.Load( bed, CDrnBridgeSettings(), "Again" ) );
            ASSERT_TRUE( FileBytes == again.archive->GetFileSize( "f1.bin" ) && 1 == global->GetMemberLinkCount() );
        }
        ASSERT_TRUE( 0 == global->GetMemberLinkCount() );
        UInt64 start = NowMs();
        while ( 0 != bed.Server().GetOpenHandleCount( 0 ) && NowMs() - start < 10000 )
            MT::PrecisionDelay( 5 );
        ASSERT_TRUE( 0 == bed.Server().GetOpenHandleCount( 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: a mount that makes many calls at once does not starve a mount that makes few: the request budget of the member is shared first come first served and nobody is refused" )
    try
    {
        CLimitBed bed( "fair" );
        REQUIRE_TRUE( bed.Start( 40 ) );
        DRN::CDRNResourceTransferSettings server = bed.Server().GetSettings();
        server.maxNamespaceRequestsPerPeer = 2;
        bed.Server().SetSettings( server );
        DRN::CDRNResourceTransferStats before = bed.Server().GetStats();

        CMount heavy;
        CMount light;
        REQUIRE_TRUE( heavy.Load( bed, CacheOffSettings(), "Heavy" ) && light.Load( bed, CacheOffSettings(), "Light" ) );
        ASSERT_TRUE( FileBytes == light.archive->GetFileSize( "f0.bin" ) );
        UInt64 heavyStart = heavy.connection->GetStatCallCount() + heavy.connection->GetListCallCount();

        MT::CTAtomic< bool > stop( false );
        GUCEF::vector< CArchiveWorker* > workers;
        for ( UInt32 i=0; i<8; ++i )
            workers.push_back( GUCEF_NEW CArchiveWorker( *heavy.archive, CArchiveWorker::WORK_STAT, 0, &stop ) );
        for ( UInt32 i=0; i<workers.size(); ++i )
            REQUIRE_TRUE( workers[ i ]->Activate( GUCEF_NULL, 0, 0 ) );
        MT::PrecisionDelay( 200 );

        // the light mount's calls go through while the heavy one keeps eight callers waiting
        UInt32 lightFailures = 0;
        UInt64 worst = 0;
        UInt64 total = 0;
        for ( UInt32 i=0; i<12; ++i )
        {
            UInt64 begin = NowMs();
            if ( FileBytes != light.archive->GetFileSize( "f" + CORE::ToString( 1 + i % 5 ) + ".bin" ) )
                ++lightFailures;
            UInt64 took = NowMs() - begin;
            total += took;
            if ( took > worst )
                worst = took;
        }
        UInt64 heavyCallsDuring = heavy.connection->GetStatCallCount() + heavy.connection->GetListCallCount() - heavyStart;
        stop.Store( true );
        UInt32 heavyFailures = 0;
        for ( UInt32 i=0; i<workers.size(); ++i )
        {
            heavyFailures += workers[ i ]->GetFailures();
            workers[ i ]->Deactivate( false, true );
            GUCEF_DELETE workers[ i ];
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN bridge limits: the light mount's 12 calls took at most " << worst << " ms, " << ( total / 12 ) << " ms on average, while the heavy mount made " << heavyCallsDuring << " calls" );
        ASSERT_TRUE( 0 == lightFailures && 0 == heavyFailures );
        ASSERT_TRUE( worst < 3000 && heavyCallsDuring >= 10 );

        // one budget: the server was never over its limit, nothing was refused or retried
        DRN::CDRNResourceTransferStats after = bed.Server().GetStats();
        ASSERT_TRUE( after.refusedByMemberLimit == before.refusedByMemberLimit && after.namespaceRequestsRefused == before.namespaceRequestsRefused );
        ASSERT_TRUE( 0 == heavy.connection->GetBusyRetryCount() && 0 == light.connection->GetBusyRetryCount() );
        {
            CDrnBridgeMemberLinkPtr link = light.connection->GetMemberLink();
            ASSERT_TRUE( 2 == link->GetClient()->GetPeakRequestsInFlight() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: three mounts under a per-member limit of 8 open handles run a workload with many open files and get no retry-later result: the limit is divided between them" )
    try
    {
        CLimitBed bed( "handles" );
        REQUIRE_TRUE( bed.Start() );
        DRN::CDRNResourceTransferSettings server = bed.Server().GetSettings();
        server.maxOpenHandlesPerPeer = 8;
        bed.Server().SetSettings( server );
        DRN::CDRNResourceTransferStats before = bed.Server().GetStats();

        CMount mounts[ 3 ];
        for ( UInt32 i=0; i<3; ++i )
            REQUIRE_TRUE( mounts[ i ].Load( bed, CDrnBridgeSettings(), "Handles" + CORE::ToString( i ) ) );

        CCounterSampler sampler( bed.Server(), true );
        REQUIRE_TRUE( sampler.Activate( GUCEF_NULL, 0, 0 ) );
        GUCEF::vector< CArchiveWorker* > workers;
        for ( UInt32 i=0; i<3; ++i )
            workers.push_back( GUCEF_NEW CArchiveWorker( *mounts[ i ].archive, CArchiveWorker::WORK_OPEN_AND_READ, 3 ) );
        for ( UInt32 i=0; i<3; ++i )
            REQUIRE_TRUE( workers[ i ]->Activate( GUCEF_NULL, 0, 0 ) );
        UInt64 start = NowMs();
        for ( UInt32 i=0; i<3; ++i )
            while ( !workers[ i ]->IsDone() && NowMs() - start < 120000 )
                MT::PrecisionDelay( 5 );
        UInt32 failures = 0;
        UInt32 retryLater = 0;
        for ( UInt32 i=0; i<3; ++i )
        {
            ASSERT_TRUE( workers[ i ]->IsDone() );
            failures += workers[ i ]->GetFailures();
            retryLater += workers[ i ]->GetRetryLater();
        }
        sampler.Stop();
        ASSERT_TRUE( 0 == failures && 0 == retryLater );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN bridge limits: three mounts, per-member limit 8: the serving side held at most " << sampler.GetHighest() << " handles, the mounts hold " <<
                   mounts[ 0 ].archive->GetHandleLimit() << ", " << mounts[ 1 ].archive->GetHandleLimit() << " and " << mounts[ 2 ].archive->GetHandleLimit() <<
                   ", evictions " << mounts[ 0 ].archive->GetHandleEvictionCount() + mounts[ 1 ].archive->GetHandleEvictionCount() + mounts[ 2 ].archive->GetHandleEvictionCount() );

        // the announced limit 8 is divided 3, 3 and 2; the serving side never saw more than 8 and refused nothing; nobody had to retry
        ASSERT_TRUE( 3 == mounts[ 0 ].archive->GetHandleLimit() && 3 == mounts[ 1 ].archive->GetHandleLimit() && 2 == mounts[ 2 ].archive->GetHandleLimit() );
        ASSERT_TRUE( sampler.GetHighest() <= 8 && sampler.GetHighest() >= 2 );
        DRN::CDRNResourceTransferStats after = bed.Server().GetStats();
        ASSERT_TRUE( after.refusedByMemberLimit == before.refusedByMemberLimit && after.opensRefused == before.opensRefused );
        for ( UInt32 i=0; i<3; ++i )
        {
            ASSERT_TRUE( 0 == mounts[ i ].connection->GetBusyRetryCount() );
            ASSERT_TRUE( mounts[ i ].archive->GetPeakOpenHandleCount() <= mounts[ i ].archive->GetHandleLimit() );
            ASSERT_TRUE( mounts[ i ].archive->GetHandleEvictionCount() > 0 );
        }
        for ( UInt32 i=0; i<3; ++i )
        {
            workers[ i ]->Deactivate( false, true );
            GUCEF_DELETE workers[ i ];
        }

        // a mount that goes gives its share to the others
        mounts[ 2 ].Unload();
        {
            VFS::TBasicVfsResourcePtr file = mounts[ 0 ].archive->GetFile( "f1.bin", "rb", 0, false );
            ASSERT_TRUE( !file.IsNULL() );
            ASSERT_TRUE( 4 == mounts[ 0 ].archive->GetHandleLimit() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: a refusal beyond what the serving side announced is tried again with a backoff and succeeds when room is made, or ends as retry-later after the configured retries" )
    try
    {
        CLimitBed bed( "busy" );
        REQUIRE_TRUE( bed.Start( 30 ) );

        // the hello is answered while the limits are the defaults: the mounts learn "32"
        CDrnBridgeSettings settings;
        settings.busyRetries = 3;
        settings.busyBackoffMs = 10;
        CMount mount;
        REQUIRE_TRUE( mount.Load( bed, settings, "Busy" ) );
        ASSERT_TRUE( FileBytes == mount.archive->GetFileSize( "f0.bin" ) );

        // then the limit drops, and others hold what is left of it: handles of the test's own (another user of the member's budget)
        DRN::CDRNResourceTransferSettings limits = bed.Server().GetSettings();
        limits.maxOpenHandlesPerPeer = 2;
        bed.Server().SetSettings( limits );
        DRN::CDRNResourceHandlePtr others[ 2 ];
        for ( UInt32 i=0; i<2; ++i )
        {
            others[ i ] = bed.share->Client().GetResources().OpenHandle( bed.share->GetServerPeerId(), CLoopbackShare::ShareName, "f" + CORE::ToString( 10 + i ) + ".bin" );
            REQUIRE_TRUE( others[ i ]->WaitOpen( 10000 ) );
        }

        // retries used up: the result is the retry-later one, after the waits (10 + 20 + 40 ms)
        {
            UInt64 retriesBefore = mount.connection->GetBusyRetryCount();
            UInt64 begin = NowMs();
            VFS::TBasicVfsResourcePtr file = mount.archive->GetFile( "f5.bin", "rb", 0, false );
            UInt64 took = NowMs() - begin;
            ASSERT_TRUE( file.IsNULL() );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER == mount.archive->GetLastResult().fileSystemResult );
            ASSERT_TRUE( 3 == mount.connection->GetBusyRetryCount() - retriesBefore );
            ASSERT_TRUE( took >= 60 );
        }

        // no retries configured: refused at once
        {
            CDrnBridgeSettings none;
            none.busyRetries = 0;
            CMount quick;
            REQUIRE_TRUE( quick.Load( bed, none, "Quick" ) );
            VFS::TBasicVfsResourcePtr file = quick.archive->GetFile( "f6.bin", "rb", 0, false );
            ASSERT_TRUE( file.IsNULL() && 0 == quick.connection->GetBusyRetryCount() );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER == quick.archive->GetLastResult().fileSystemResult );
        }

        // room is made while the retries wait: the open then goes through
        {
            CDrnBridgeSettings patient;
            patient.busyRetries = 8;
            patient.busyBackoffMs = 25;
            CMount waiting;
            REQUIRE_TRUE( waiting.Load( bed, patient, "Patient" ) );
            CLetGo letGo( others[ 0 ], 150 );
            REQUIRE_TRUE( letGo.Activate( GUCEF_NULL, 0, 0 ) );
            VFS::TBasicVfsResourcePtr file = waiting.archive->GetFile( "f7.bin", "rb", 0, false );
            ASSERT_TRUE( !file.IsNULL() && !file->GetAccess().IsNULL() && ReadChecked( *file->GetAccess(), 7, 0, 2000 ) );
            ASSERT_TRUE( waiting.connection->GetBusyRetryCount() >= 1 );
        }
        others[ 0 ].Unlink();
        others[ 1 ].Unlink();

        // a namespace call refused for a limit it could not know (the client's own throttle is off) is tried again as well
        {
            DRN::CDRNResourceTransferSettings tight = bed.Server().GetSettings();
            tight.maxOpenHandlesPerPeer = 32;
            tight.maxNamespaceRequestsPerPeer = 1;
            bed.Server().SetSettings( tight );
            CDrnBridgeSettings patient = CacheOffSettings();
            patient.busyRetries = 12;
            patient.busyBackoffMs = 20;
            CMount stats;
            REQUIRE_TRUE( stats.Load( bed, patient, "Stats" ) );
            ASSERT_TRUE( FileBytes == stats.archive->GetFileSize( "f0.bin" ) );
            {
                CDrnBridgeMemberLinkPtr link = stats.connection->GetMemberLink();
                link->GetClient()->SetSelfThrottle( false );
            }
            GUCEF::vector< CArchiveWorker* > workers;
            for ( UInt32 i=0; i<4; ++i )
                workers.push_back( GUCEF_NEW CArchiveWorker( *stats.archive, CArchiveWorker::WORK_STAT, 6 ) );
            for ( UInt32 i=0; i<4; ++i )
                REQUIRE_TRUE( workers[ i ]->Activate( GUCEF_NULL, 0, 0 ) );
            UInt32 failures = 0;
            UInt64 start = NowMs();
            for ( UInt32 i=0; i<4; ++i )
            {
                while ( !workers[ i ]->IsDone() && NowMs() - start < 60000 )
                    MT::PrecisionDelay( 5 );
                failures += workers[ i ]->GetFailures();
                workers[ i ]->Deactivate( false, true );
                GUCEF_DELETE workers[ i ];
            }
            ASSERT_TRUE( 0 == failures );
            ASSERT_TRUE( stats.connection->GetBusyRetryCount() > 0 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
