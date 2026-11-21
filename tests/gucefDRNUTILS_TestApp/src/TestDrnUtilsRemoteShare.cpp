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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#include <exception>

#include "TestDrnUtilsRemoteShare.h"

#ifndef GUCEF__TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF__TEST_FRAMEWORK_H
#endif /* GUCEF__TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GUCEFCORE_H
#include "gucefCORE.h"
#define GUCEF_GUCEFCORE_H
#endif /* GUCEF_GUCEFCORE_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#include "gucefCORE_CNativeFileSystemAccess.h"
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_GUCEFDRNUTILS_H
#include "gucefDRNUTILS.h"
#define GUCEF_GUCEFDRNUTILS_H
#endif /* GUCEF_GUCEFDRNUTILS_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H
#include "vfspluginDRN_CDrnBridgeGlobal.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H ? */

#ifndef GUCEF_TESTRPCSUPPORT_H
#include "TestRpcSupport.h"
#define GUCEF_TESTRPCSUPPORT_H
#endif /* GUCEF_TESTRPCSUPPORT_H ? */

#ifndef GUCEF_TESTDRNUTILSREMOTEWORLD_H
#include "TestDrnUtilsRemoteWorld.h"
#define GUCEF_TESTDRNUTILSREMOTEWORLD_H
#endif /* GUCEF_TESTDRNUTILSREMOTEWORLD_H ? */

#ifndef GUCEF_TESTDRNUTILSDRNGROUP_H
#include "TestDrnUtilsDrnGroup.h"
#define GUCEF_TESTDRNUTILSDRNGROUP_H
#endif /* GUCEF_TESTDRNUTILSDRNGROUP_H ? */

using namespace GUCEF;
using namespace DRNUTILSTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

/*-------------------------------------------------------------------------*/

UInt8
ByteAt( UInt8 seed, UInt64 position )
{
    return (UInt8) ( seed + position * 7 + ( position >> 9 ) );
}

bool
WriteSeeded( const CORE::CString& path, UInt32 size, UInt8 seed )
{
    if ( !CORE::CreatePathDirectories( path ) )
        return false;
    CORE::CFileAccess file( path, "wb" );
    if ( !file.Opened() )
        return false;
    CORE::CDynamicBuffer block( size, true );
    UInt8* out = static_cast< UInt8* >( block.GetBufferPtr() );
    for ( UInt32 i=0; i<size; ++i )
        out[ i ] = ByteAt( seed, i );
    if ( size != file.Write( block.GetConstBufferPtr(), 1, size ) )
        return false;
    file.Flush();
    return true;
}

bool
ReadSeeded( CORE::CIOAccess& access, UInt64 offset, UInt32 count, UInt8 seed )
{
    CORE::CDynamicBuffer buffer( count, true );
    if ( 0 != access.Setpos( offset ) )
        return false;
    if ( count != access.Read( buffer.GetBufferPtr(), 1, count ) )
        return false;
    const UInt8* in = static_cast< const UInt8* >( buffer.GetConstBufferPtr() );
    for ( UInt32 i=0; i<count; ++i )
        if ( in[ i ] != ByteAt( seed, offset + i ) )
            return false;
    return true;
}

/**
 *  A call of the VFS made on a thread of its own while the test thread pumps the pulse generator, as an application makes its reads: the calls of a
 *  mount block for the answer of the service, and the test thread is the thread that drives the pulse generator of the connection
 */
class CVfsProbe : public MT::CActiveObject, public CCondition
{
    public:

    enum EOperation { OPERATION_EXISTS = 0, OPERATION_SIZE, OPERATION_READS };

    VFS::CVFS& vfs;
    EOperation operation;
    CORE::CString path;
    UInt32 expectedSize;
    UInt8 seed;
    bool exists;
    UInt64 size;
    bool reads;
    MT::CTAtomic< UInt32 > finished;

    CVfsProbe( VFS::CVFS& v, EOperation op, const CORE::CString& p, UInt32 expected = 0, UInt8 s = 0 )
        : MT::CActiveObject(), CCondition(), vfs( v ), operation( op ), path( p ), expectedSize( expected ), seed( s ), exists( false ), size( 0 ), reads( false ), finished( 0 ) {}

    virtual ~CVfsProbe() GUCEF_VIRTUAL_OVERRIDE { Deactivate( false, true ); }

    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return 0 != finished.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        try
        {
            if ( OPERATION_EXISTS == operation )
                exists = vfs.FileExists( path );
            else
            if ( OPERATION_SIZE == operation )
                size = vfs.GetFileSize( path );
            else
            {
                VFS::TBasicVfsResourcePtr file = vfs.GetFile( path, "rb", false );
                reads = !file.IsNULL() && !file->GetAccess().IsNULL() && ReadSeeded( *file->GetAccess(), 0, expectedSize, seed );
            }
        }
        catch ( ... )
        {
        }
        finished.Store( 1 );
        return true;
    }
};

/** What the VFS says of a path, asked on a thread of its own while this thread pumps */
bool
VfsExists( VFS::CVFS& vfs, const CORE::CString& path )
{
    CVfsProbe probe( vfs, CVfsProbe::OPERATION_EXISTS, path );
    probe.Activate( GUCEF_NULL );
    PumpUntil( probe, 30000 );
    return probe.exists;
}

UInt64
VfsSize( VFS::CVFS& vfs, const CORE::CString& path )
{
    CVfsProbe probe( vfs, CVfsProbe::OPERATION_SIZE, path );
    probe.Activate( GUCEF_NULL );
    PumpUntil( probe, 30000 );
    return probe.size;
}

/** Whether a file of the VFS reads back with the content of the seed */
bool
VfsFileReads( VFS::CVFS& vfs, const CORE::CString& path, UInt32 size, UInt8 seed )
{
    CVfsProbe probe( vfs, CVfsProbe::OPERATION_READS, path, size, seed );
    probe.Activate( GUCEF_NULL );
    PumpUntil( probe, 30000 );
    return probe.reads;
}

DRN::CDRNSessionRegistry&
Registry( void )
{
    return DRN::CDRNGlobal::Instance()->GetSessionRegistry();
}

/*-------------------------------------------------------------------------*/

/** A remote service host with the share "files" (a directory of the test) and a paired device that has its credentials */
class CShareWorld
{
    public:

    CRemoteWorld world;
    CORE::CString served;
    DRNUTILS::CDeviceCredentials credentials;

    explicit CShareWorld( const CORE::CString& name ) : world( name ), served(), credentials()
    {
        served = CORE::CombinePath( world.directory, "served" );
        CORE::CreateDirs( served );
    }

    bool Start( void )
    {
        if ( !world.StartHost() )
            return false;
        CORE::CString error;
        if ( !world.Pair( "viewer", "viewer", credentials, error ) )
            return false;
        return ServeFiles();
    }

    bool ServeFiles( void )
    {
        DRN::CDRNSession* session = world.host->GetDrnSession();
        if ( GUCEF_NULL == session )
            return false;
        CORE::TFileSystemAccessPtr fileSystem = ( GUCEF_NEW CORE::CNativeFileSystemAccess() )->CreateSharedPtr();
        DRN::CIDRNResourceProviderPtr provider( GUCEF_NEW DRN::CDRNFileSystemAccessResourceProvider( fileSystem, served, CORE::CUri( "file://" + served ) ) );
        return session->GetResources().RegisterProvider( "files", provider, DRN::CDRNResourceShareRules( DRN::CDRNResourceShareRules::WholeProvider, DRN::CDRNResourceShareRules::AnyRole ) );
    }

    CORE::CString Path( const CORE::CString& relative ) const
    {
        return CORE::CombinePath( served, relative );
    }
};

/** A connection to the world, its state events collected */
class CShareClient
{
    public:

    DRNUTILS::CRemoteServiceConnection connection;
    CEventCollector events;

    CShareClient( const CORE::PulseGeneratorPtr& pulse, const DRNUTILS::CRemoteServiceConnectionSettings& settings )
        : connection( pulse, settings ), events()
    {
        connection.Subscribe( &events );
    }

    ~CShareClient()
    {
        connection.Unsubscribe( &events );
    }
};

/** Collects the states a mount reports, and how often an aggregate or a mount event came */
class CShareStateCollector : public CORE::CObserver
{
    public:

    GUCEF::vector< Int32 > states;

    CShareStateCollector( void ) : CORE::CObserver(), states() {}
    virtual ~CShareStateCollector() { SignalUpcomingObserverDestruction(); }

    virtual void OnNotify( CORE::CNotifier*, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( ( DRNUTILS::CRemoteShareMount::StateChangedEvent == eventId || DRNUTILS::CRemoteShareMounts::StateChangedEvent == eventId ) && GUCEF_NULL != eventData )
            states.push_back( (Int32) static_cast< DRNUTILS::TRemoteShareStateEventData* >( eventData )->GetData() );
    }

    bool Saw( DRNUTILS::TRemoteShareState state ) const
    {
        for ( UInt32 i=0; i<states.size(); ++i )
            if ( (Int32) state == states[ i ] )
                return true;
        return false;
    }
};

class CShareIs : public CCondition
{
    public:

    const DRNUTILS::CRemoteShareMount& mount;
    DRNUTILS::TRemoteShareState state;

    CShareIs( const DRNUTILS::CRemoteShareMount& m, DRNUTILS::TRemoteShareState s ) : mount( m ), state( s ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return mount.GetState() == state; }
};

class CSharesAre : public CCondition
{
    public:

    const DRNUTILS::CRemoteShareMounts& mounts;
    DRNUTILS::TRemoteShareState state;

    CSharesAre( const DRNUTILS::CRemoteShareMounts& m, DRNUTILS::TRemoteShareState s ) : mounts( m ), state( s ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return mounts.GetState() == state; }
};

class CEntryIs : public CCondition
{
    public:

    const DRNUTILS::CRemoteShareMounts& mounts;
    UInt32 index;
    DRNUTILS::TRemoteShareState state;

    CEntryIs( const DRNUTILS::CRemoteShareMounts& m, UInt32 i, DRNUTILS::TRemoteShareState s ) : mounts( m ), index( i ), state( s ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return mounts.GetEntryState( index ) == state; }
};

class CSessionRegistered : public CCondition
{
    public:

    CORE::CString name;
    explicit CSessionRegistered( const CORE::CString& sessionName ) : name( sessionName ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL != Registry().Find( name ); }
};

UInt32
CountChildren( const CORE::CDataNode& node, const CORE::CString& name )
{
    UInt32 count = 0;
    for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
        if ( (*i)->GetName() == name )
            ++count;
    return count;
}

DRNUTILS::CRemoteShareMountSettings
ShareSettings( const CORE::CString& mountPoint, UInt32 channel = DRNUTILS::CRemoteShareMountSettings::AutoChannel )
{
    DRNUTILS::CRemoteShareMountSettings settings;
    settings.share = "files";
    settings.mountPoint = mountPoint;
    settings.channel = channel;
    return settings;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE CASES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDrnUtilsRemoteShareTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsRemoteShare" );

    // the archive type is made known to the VFS by the global of the plugin
    VFSPLUGIN::DRNBRIDGE::CDrnBridgeGlobal::Instance();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    GUCEF_TESTFW_TESTCASE( "RemoteShare-1 mounted before the connection is up: WAITING, then ONLINE by itself, the files read through the VFS, registered by name, and Stop() leaves nothing" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CShareWorld shared( "share1" );
        REQUIRE_TRUE( shared.Start() );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "a.bin" ), 3000, 11 ) );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "dir/b.bin" ), 70000, 12 ) );
        const CORE::CString sessionName = shared.credentials.drnSessionName;
        REQUIRE_TRUE( !sessionName.IsNULLOrEmpty() );

        CShareClient client( shared.world.pulse, shared.world.ClientSettings() );
        DRNUTILS::CRemoteShareMount mount( client.connection, ShareSettings( "rs/one" ) );
        CShareStateCollector states;
        mount.Subscribe( &states );
        ASSERT_TRUE( DRNUTILS::REMOTESHARE_IDLE == mount.GetState() );
        ASSERT_FALSE( mount.IsMounted() );

        // started before the connection: mounted, waiting, nothing registered, the share cannot be reached
        CORE::CString error;
        REQUIRE_TRUE( mount.Start( sessionName, error ) );
        ASSERT_TRUE( DRNUTILS::REMOTESHARE_WAITING == mount.GetState() );
        ASSERT_TRUE( mount.IsMounted() );
        ASSERT_TRUE( vfs.IsMountedArchive( "rs/one" ) );
        ASSERT_TRUE( mount.GetAddress() == "drn://" + sessionName + "/sequencer/files" );
        ASSERT_TRUE( 0 == mount.GetChannel() );
        ASSERT_TRUE( GUCEF_NULL == Registry().Find( sessionName ) );
        ASSERT_FALSE( mount.HasRegisteredSession() );
        ASSERT_FALSE( vfs.FileExists( "rs/one/a.bin" ) );
        ASSERT_TRUE( mount.Start( sessionName, error ) == false );            // started already

        // the connection comes up: the session is registered under the name, and the mount is online without being touched
        REQUIRE_TRUE( client.connection.StartConnect( shared.credentials, error ) );
        CShareIs online( mount, DRNUTILS::REMOTESHARE_ONLINE );
        ASSERT_TRUE( PumpUntil( online, 20000 ) );
        ASSERT_TRUE( GUCEF_NULL != Registry().Find( sessionName ) );
        ASSERT_TRUE( Registry().Find( sessionName ) == client.connection.GetDrnSession( 0 ) );
        ASSERT_TRUE( mount.HasRegisteredSession() );
        ASSERT_TRUE( VfsExists( vfs, "rs/one/a.bin" ) );
        ASSERT_TRUE( 70000 == VfsSize( vfs, "rs/one/dir/b.bin" ) );
        ASSERT_TRUE( VfsFileReads( vfs, "rs/one/a.bin", 3000, 11 ) );
        ASSERT_TRUE( VfsFileReads( vfs, "rs/one/dir/b.bin", 70000, 12 ) );
        ASSERT_TRUE( states.Saw( DRNUTILS::REMOTESHARE_WAITING ) && states.Saw( DRNUTILS::REMOTESHARE_ONLINE ) );
        ASSERT_FALSE( states.Saw( DRNUTILS::REMOTESHARE_FAILED ) || states.Saw( DRNUTILS::REMOTESHARE_OFFLINE ) );

        // stopped: unmounted, the registration it made is gone, idle
        mount.Stop();
        ASSERT_TRUE( DRNUTILS::REMOTESHARE_IDLE == mount.GetState() );
        ASSERT_FALSE( mount.IsMounted() );
        ASSERT_FALSE( vfs.IsMountedArchive( "rs/one" ) );
        ASSERT_FALSE( vfs.FileExists( "rs/one/a.bin" ) );
        ASSERT_TRUE( GUCEF_NULL == Registry().Find( sessionName ) );
        ASSERT_FALSE( mount.HasRegisteredSession() );
        ASSERT_TRUE( states.Saw( DRNUTILS::REMOTESHARE_IDLE ) );
        mount.Stop();                                                       // twice: no harm
        mount.Unsubscribe( &states );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShare-2 a lost link: OFFLINE with the mount in place and what is cached answered, the link back: ONLINE again" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CShareWorld shared( "share2" );
        REQUIRE_TRUE( shared.Start() );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "cached.bin" ), 5000, 21 ) );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "other.bin" ), 500, 22 ) );
        const CORE::CString sessionName = shared.credentials.drnSessionName;

        CShareClient client( shared.world.pulse, shared.world.ClientSettings() );
        DRNUTILS::CRemoteShareMountSettings settings = ShareSettings( "rs/two" );
        settings.bridgeSettings.SetAttribute( "metadataTtlMs", CORE::CString( "100" ) );     // an expired entry is still what an offline mount knows
        DRNUTILS::CRemoteShareMount mount( client.connection, settings );
        CShareStateCollector states;
        mount.Subscribe( &states );
        CORE::CString error;
        REQUIRE_TRUE( mount.Start( sessionName, error ) );
        REQUIRE_TRUE( client.connection.StartConnect( shared.credentials, error ) );
        CShareIs online( mount, DRNUTILS::REMOTESHARE_ONLINE );
        REQUIRE_TRUE( PumpUntil( online, 20000 ) );
        ASSERT_TRUE( 5000 == VfsSize( vfs, "rs/two/cached.bin" ) );
        PumpFor( 200 );

        // the link is cut and cannot come back: offline, the mount is still there and answers what it knows, and nothing it does not know
        shared.world.proxy.SetRefuseNew( true );
        shared.world.proxy.Cut();
        CShareIs offline( mount, DRNUTILS::REMOTESHARE_OFFLINE );
        ASSERT_TRUE( PumpUntil( offline, 20000 ) );
        ASSERT_TRUE( mount.IsMounted() );
        ASSERT_TRUE( vfs.IsMountedArchive( "rs/two" ) );
        ASSERT_TRUE( 5000 == vfs.GetFileSize( "rs/two/cached.bin" ) );
        CORE::CResourceMetaData metaData;
        ASSERT_TRUE( vfs.GetFileMetaData( "rs/two/cached.bin", metaData ) );
        ASSERT_TRUE( metaData.hasIsOffline && metaData.isOffline );
        ASSERT_FALSE( vfs.FileExists( "rs/two/other.bin" ) );
        ASSERT_TRUE( states.Saw( DRNUTILS::REMOTESHARE_OFFLINE ) );
        ASSERT_FALSE( states.Saw( DRNUTILS::REMOTESHARE_FAILED ) );

        // the path is open again: the connection reconnects by itself and the mount is online, serving what it did not know before
        shared.world.proxy.SetRefuseNew( false );
        ASSERT_TRUE( PumpUntil( online, 30000 ) );
        ASSERT_TRUE( VfsExists( vfs, "rs/two/other.bin" ) );
        ASSERT_TRUE( VfsFileReads( vfs, "rs/two/other.bin", 500, 22 ) );
        ASSERT_TRUE( mount.HasRegisteredSession() );
        ASSERT_TRUE( Registry().Find( sessionName ) == client.connection.GetDrnSession( 0 ) );
        mount.Stop();
        mount.Unsubscribe( &states );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShare-3 a new session (Close() and StartConnect()): the registration moves to the new session, the mount stays and is online again" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CShareWorld shared( "share3" );
        REQUIRE_TRUE( shared.Start() );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "a.bin" ), 4000, 31 ) );
        const CORE::CString sessionName = shared.credentials.drnSessionName;

        CShareClient client( shared.world.pulse, shared.world.ClientSettings() );
        DRNUTILS::CRemoteShareMount mount( client.connection, ShareSettings( "rs/three" ) );
        CShareStateCollector states;
        mount.Subscribe( &states );
        CORE::CString error;
        REQUIRE_TRUE( mount.Start( sessionName, error ) );
        REQUIRE_TRUE( client.connection.StartConnect( shared.credentials, error ) );
        CShareIs online( mount, DRNUTILS::REMOTESHARE_ONLINE );
        REQUIRE_TRUE( PumpUntil( online, 20000 ) );
        ASSERT_TRUE( VfsFileReads( vfs, "rs/three/a.bin", 4000, 31 ) );
        DRN::CDRNSession* firstSession = client.connection.GetDrnSession( 0 );
        ASSERT_TRUE( Registry().Find( sessionName ) == firstSession );

        // closed: the session is gone, so is its registration, the mount stays and is not reachable
        client.connection.Close( true );
        ASSERT_TRUE( GUCEF_NULL == Registry().Find( sessionName ) );
        CShareIs notOnline( mount, DRNUTILS::REMOTESHARE_OFFLINE );
        ASSERT_TRUE( PumpUntil( notOnline, 10000 ) );
        ASSERT_TRUE( mount.IsMounted() );
        ASSERT_FALSE( mount.HasRegisteredSession() );
        ASSERT_FALSE( vfs.FileExists( "rs/three/never-asked.bin" ) );

        // connected again: a new session, which is the one registered now, the mount is online without a new Start()
        REQUIRE_TRUE( client.connection.StartConnect( shared.credentials, error ) );
        REQUIRE_TRUE( PumpUntil( online, 30000 ) );
        DRN::CDRNSession* secondSession = client.connection.GetDrnSession( 0 );
        ASSERT_TRUE( GUCEF_NULL != secondSession );
        ASSERT_TRUE( Registry().Find( sessionName ) == secondSession );
        ASSERT_TRUE( mount.HasRegisteredSession() );
        ASSERT_TRUE( VfsFileReads( vfs, "rs/three/a.bin", 4000, 31 ) );
        ASSERT_TRUE( states.Saw( DRNUTILS::REMOTESHARE_OFFLINE ) );
        mount.Stop();
        ASSERT_TRUE( GUCEF_NULL == Registry().Find( sessionName ) );
        mount.Unsubscribe( &states );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShare-4 a session name that is taken by another session is refused with a reason, before the mount and after it, and the other session is not touched" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CShareWorld shared( "share4" );
        REQUIRE_TRUE( shared.Start() );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "a.bin" ), 1000, 41 ) );
        const CORE::CString sessionName = shared.credentials.drnSessionName;
        CShareClient client( shared.world.pulse, shared.world.ClientSettings() );

        // another session of that name is registered when the mount starts
        {
            CDrnUtilsGroup foreign( sessionName );
            REQUIRE_TRUE( foreign.Start( 1 ) );
            DRN::CDRNSession* foreignSession = Registry().Find( sessionName );
            REQUIRE_TRUE( foreignSession == &foreign[ 0 ] );

            DRNUTILS::CRemoteShareMount refused( client.connection, ShareSettings( "rs/four" ) );
            CShareStateCollector states;
            refused.Subscribe( &states );
            CORE::CString error;
            ASSERT_FALSE( refused.Start( sessionName, error ) );
            ASSERT_TRUE( error.HasSubstr( "another session" ) >= 0 );
            ASSERT_TRUE( DRNUTILS::REMOTESHARE_FAILED == refused.GetState() );
            ASSERT_TRUE( refused.GetLastError() == error );
            ASSERT_FALSE( refused.IsMounted() );
            ASSERT_FALSE( vfs.IsMountedArchive( "rs/four" ) );
            ASSERT_TRUE( states.Saw( DRNUTILS::REMOTESHARE_FAILED ) );
            ASSERT_TRUE( Registry().Find( sessionName ) == foreignSession );
            refused.Stop();
            ASSERT_TRUE( DRNUTILS::REMOTESHARE_IDLE == refused.GetState() );
            ASSERT_TRUE( Registry().Find( sessionName ) == foreignSession );
            refused.Unsubscribe( &states );
        }
        ASSERT_TRUE( GUCEF_NULL == Registry().Find( sessionName ) );

        // another session of that name is registered after the mount was made, before the session of the connection exists: the connection's session
        // finds the name taken, and the mount, which would show the share of the other session, is taken away
        {
            DRNUTILS::CRemoteShareMount mount( client.connection, ShareSettings( "rs/four" ) );
            CShareStateCollector states;
            mount.Subscribe( &states );
            CORE::CString error;
            REQUIRE_TRUE( mount.Start( sessionName, error ) );
            ASSERT_TRUE( mount.IsMounted() );

            CDrnUtilsGroup foreign( sessionName );
            REQUIRE_TRUE( foreign.Start( 1 ) );
            DRN::CDRNSession* foreignSession = Registry().Find( sessionName );
            REQUIRE_TRUE( foreignSession == &foreign[ 0 ] );

            REQUIRE_TRUE( client.connection.StartConnect( shared.credentials, error ) );
            CShareIs failed( mount, DRNUTILS::REMOTESHARE_FAILED );
            ASSERT_TRUE( PumpUntil( failed, 20000 ) );
            ASSERT_TRUE( mount.GetLastError().HasSubstr( "another session" ) >= 0 );
            ASSERT_FALSE( mount.IsMounted() );
            ASSERT_FALSE( vfs.IsMountedArchive( "rs/four" ) );
            ASSERT_FALSE( mount.HasRegisteredSession() );
            ASSERT_TRUE( Registry().Find( sessionName ) == foreignSession );
            ASSERT_TRUE( Registry().Find( sessionName ) != client.connection.GetDrnSession( 0 ) );

            // stopped: the other session is registered still
            mount.Stop();
            ASSERT_TRUE( Registry().Find( sessionName ) == foreignSession );
            mount.Unsubscribe( &states );
            client.connection.Close( true );
        }
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShare-5 Stop() removes only what the mount made: a session the application registered stays registered, an archive of another name stays mounted, refusals leave the VFS as it was" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CShareWorld shared( "share5" );
        REQUIRE_TRUE( shared.Start() );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "a.bin" ), 1000, 51 ) );
        const CORE::CString sessionName = shared.credentials.drnSessionName;
        CShareClient client( shared.world.pulse, shared.world.ClientSettings() );
        CORE::CString error;

        // a mount point in use by an archive, a share and a mount point that are none: refused, with reasons, nothing changes
        const CORE::CString localDir = CORE::CombinePath( shared.world.directory, "local" );
        REQUIRE_TRUE( WriteSeeded( CORE::CombinePath( localDir, "local.bin" ), 10, 52 ) );
        REQUIRE_TRUE( vfs.AddRoot( localDir, "remoteShareLocal", false, false, "rs/busy" ) );
        {
            DRNUTILS::CRemoteShareMount busy( client.connection, ShareSettings( "rs/busy" ) );
            ASSERT_FALSE( busy.Start( sessionName, error ) );
            ASSERT_TRUE( error.HasSubstr( "in use" ) >= 0 );
            ASSERT_TRUE( DRNUTILS::REMOTESHARE_FAILED == busy.GetState() );
            ASSERT_FALSE( busy.IsMounted() );
            ASSERT_TRUE( vfs.FileExists( "rs/busy/local.bin" ) );
            busy.Stop();
            ASSERT_TRUE( vfs.IsMountedArchive( "rs/busy" ) );                // not the mount's: still there
            ASSERT_TRUE( vfs.FileExists( "rs/busy/local.bin" ) );

            DRNUTILS::CRemoteShareMountSettings noShare = ShareSettings( "rs/five" );
            noShare.share.Clear();
            DRNUTILS::CRemoteShareMount none( client.connection, noShare );
            ASSERT_FALSE( none.Start( sessionName, error ) );
            ASSERT_FALSE( error.IsNULLOrEmpty() );
            ASSERT_FALSE( vfs.IsMountedArchive( "rs/five" ) );

            DRNUTILS::CRemoteShareMount noChannel( client.connection, ShareSettings( "rs/five", 7 ) );
            ASSERT_FALSE( noChannel.Start( sessionName, error ) );
            ASSERT_TRUE( error.HasSubstr( "no channel 7" ) >= 0 );

            DRNUTILS::CRemoteShareMount noName( client.connection, ShareSettings( "rs/five" ) );
            ASSERT_FALSE( noName.Start( error ) );                           // the connection has no credentials yet and none were given
            ASSERT_FALSE( vfs.IsMountedArchive( "rs/five" ) );
        }
        vfs.UnmountArchiveByName( "remoteShareLocal" );

        // the application registered the session of the connection by name itself: the mount uses it, does not own it, and leaves it registered at Stop()
        {
            REQUIRE_TRUE( client.connection.StartConnect( shared.credentials, error ) );
            DRN::CDRNSession* session = client.connection.GetDrnSession( 0 );
            REQUIRE_TRUE( GUCEF_NULL != session );
            REQUIRE_TRUE( Registry().Register( *session ) );

            DRNUTILS::CRemoteShareMount mount( client.connection, ShareSettings( "rs/five" ) );
            REQUIRE_TRUE( mount.Start( sessionName, error ) );
            CShareIs online( mount, DRNUTILS::REMOTESHARE_ONLINE );
            ASSERT_TRUE( PumpUntil( online, 20000 ) );
            ASSERT_FALSE( mount.HasRegisteredSession() );
            ASSERT_TRUE( VfsFileReads( vfs, "rs/five/a.bin", 1000, 51 ) );
            mount.Stop();
            ASSERT_FALSE( vfs.IsMountedArchive( "rs/five" ) );
            ASSERT_TRUE( Registry().Find( sessionName ) == session );          // the application's registration stays
            Registry().Unregister( *session );
        }
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShare-6 the channel: auto is the first bulk channel when there is one, else the control channel; an explicit channel is that channel; the session of that channel is the one registered" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CShareWorld shared( "share6" );
        REQUIRE_TRUE( shared.Start() );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "a.bin" ), 2000, 61 ) );
        const CORE::CString sessionName = shared.credentials.drnSessionName;
        CORE::CString error;

        // a connection with a bulk channel: auto is the bulk channel
        {
            CShareClient client( shared.world.pulse, shared.world.ClientSettings( 1 ) );
            DRNUTILS::CRemoteShareMount autoMount( client.connection, ShareSettings( "rs/six" ) );
            REQUIRE_TRUE( autoMount.Start( sessionName, error ) );
            ASSERT_TRUE( 1 == autoMount.GetChannel() );
            REQUIRE_TRUE( client.connection.StartConnect( shared.credentials, error ) );
            CShareIs online( autoMount, DRNUTILS::REMOTESHARE_ONLINE );
            ASSERT_TRUE( PumpUntil( online, 30000 ) );
            ASSERT_TRUE( client.connection.IsBulkChannelReady( 1 ) );
            ASSERT_TRUE( GUCEF_NULL != client.connection.GetDrnSession( 1 ) );
            ASSERT_TRUE( Registry().Find( sessionName ) == client.connection.GetDrnSession( 1 ) );
            ASSERT_TRUE( Registry().Find( sessionName ) != client.connection.GetDrnSession( 0 ) );
            ASSERT_TRUE( VfsFileReads( vfs, "rs/six/a.bin", 2000, 61 ) );
            autoMount.Stop();
            ASSERT_TRUE( GUCEF_NULL == Registry().Find( sessionName ) );

            // the control channel by its index, on the same connection: its session is the registered one now
            DRNUTILS::CRemoteShareMount controlMount( client.connection, ShareSettings( "rs/six", 0 ) );
            REQUIRE_TRUE( controlMount.Start( sessionName, error ) );
            ASSERT_TRUE( 0 == controlMount.GetChannel() );
            CShareIs controlOnline( controlMount, DRNUTILS::REMOTESHARE_ONLINE );
            ASSERT_TRUE( PumpUntil( controlOnline, 30000 ) );
            ASSERT_TRUE( Registry().Find( sessionName ) == client.connection.GetDrnSession( 0 ) );
            ASSERT_TRUE( VfsFileReads( vfs, "rs/six/a.bin", 2000, 61 ) );

            // the bulk channel's session has the name that the control channel's session holds: the mount of the other channel is refused
            DRNUTILS::CRemoteShareMount bulkMount( client.connection, ShareSettings( "rs/six2", 1 ) );
            ASSERT_FALSE( bulkMount.Start( sessionName, error ) );
            ASSERT_TRUE( error.HasSubstr( "another session" ) >= 0 );
            ASSERT_FALSE( vfs.IsMountedArchive( "rs/six2" ) );
            controlMount.Stop();
            client.connection.Close( true );
        }

        // a connection without bulk channels: auto is the control channel
        {
            CShareClient client( shared.world.pulse, shared.world.ClientSettings( 0 ) );
            DRNUTILS::CRemoteShareMount autoMount( client.connection, ShareSettings( "rs/six" ) );
            REQUIRE_TRUE( autoMount.Start( sessionName, error ) );
            ASSERT_TRUE( 0 == autoMount.GetChannel() );
            REQUIRE_TRUE( client.connection.StartConnect( shared.credentials, error ) );
            CShareIs online( autoMount, DRNUTILS::REMOTESHARE_ONLINE );
            ASSERT_TRUE( PumpUntil( online, 30000 ) );
            ASSERT_TRUE( Registry().Find( sessionName ) == client.connection.GetDrnSession( 0 ) );
            autoMount.Stop();
            client.connection.Close( true );
        }
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShare-7 the settings: a configuration round trip, the bridge settings reach the archive, a bad configuration is refused, SetFailed()" )
    try
    {
        // the round trip of the settings of a share, and of the connection settings
        DRNUTILS::CRemoteShareMountSettings settings = ShareSettings( "rs/seven", 2 );
        settings.service = "slot-a";
        settings.bridgeSettings.SetAttribute( "maxOpenHandles", CORE::CString( "3" ) );
        CORE::CDataNode node;
        REQUIRE_TRUE( settings.SaveConfig( node ) );
        DRNUTILS::CRemoteShareMountSettings loaded;
        REQUIRE_TRUE( loaded.LoadConfig( node ) );
        ASSERT_TRUE( loaded.service == "slot-a" && loaded.share == "files" && loaded.mountPoint == "rs/seven" && 2 == loaded.channel );
        ASSERT_TRUE( loaded.bridgeSettings.GetAttributeValue( "maxOpenHandles", CORE::CVariant::Empty ).AsString() == "3" );
        DRNUTILS::CRemoteShareMountSettings automatic;
        CORE::CDataNode autoNode;
        REQUIRE_TRUE( automatic.SaveConfig( autoNode ) );
        ASSERT_TRUE( autoNode.GetAttributeValue( "channel", CORE::CVariant::Empty ).AsString() == "auto" );
        DRNUTILS::CRemoteShareMountSettings reloaded;
        reloaded.channel = 1;
        REQUIRE_TRUE( reloaded.LoadConfig( autoNode ) );
        ASSERT_TRUE( DRNUTILS::CRemoteShareMountSettings::AutoChannel == reloaded.channel );

        // what is refused
        CORE::CDataNode unknown;
        unknown.SetAttribute( "share", CORE::CString( "files" ) );
        unknown.SetAttribute( "shares", CORE::CString( "typo" ) );
        DRNUTILS::CRemoteShareMountSettings untouched = ShareSettings( "rs/seven" );
        ASSERT_FALSE( untouched.LoadConfig( unknown ) );
        ASSERT_TRUE( untouched.mountPoint == "rs/seven" );
        CORE::CDataNode badChannel;
        badChannel.SetAttribute( "channel", CORE::CString( "two" ) );
        ASSERT_FALSE( untouched.LoadConfig( badChannel ) );
        CORE::CString reason;
        ASSERT_FALSE( DRNUTILS::CRemoteShareMountSettings().IsValid( reason ) );
        ASSERT_FALSE( reason.IsNULLOrEmpty() );

        DRNUTILS::CRemoteServiceConnectionSettings connectionSettings;
        connectionSettings.bulkChannels = 2;
        connectionSettings.clientKind = "viewer";
        connectionSettings.wantedContracts.push_back( "a.b" );
        connectionSettings.reconnectBaseMs = 1234;
        CORE::CDataNode connectionNode;
        REQUIRE_TRUE( connectionSettings.SaveConfig( connectionNode ) );
        DRNUTILS::CRemoteServiceConnectionSettings connectionLoaded;
        REQUIRE_TRUE( connectionLoaded.LoadConfig( connectionNode ) );
        ASSERT_TRUE( 2 == connectionLoaded.bulkChannels && connectionLoaded.clientKind == "viewer" && 1234 == connectionLoaded.reconnectBaseMs );
        ASSERT_TRUE( 1 == connectionLoaded.wantedContracts.size() && connectionLoaded.wantedContracts[ 0 ] == "a.b" );
        CORE::CDataNode connectionBad;
        connectionBad.SetAttribute( "bulkChannel", CORE::CString( "1" ) );
        ASSERT_FALSE( connectionLoaded.LoadConfig( connectionBad ) );
        ASSERT_TRUE( 2 == connectionLoaded.bulkChannels );
        CORE::CDataNode connectionTooMany;
        connectionTooMany.SetAttribute( "bulkChannels", CORE::CString( "3" ) );
        ASSERT_FALSE( connectionLoaded.LoadConfig( connectionTooMany ) );

        // the bridge settings reach the archive, and SetFailed() ends in FAILED until Stop()
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CShareWorld shared( "share7" );
        REQUIRE_TRUE( shared.Start() );
        CShareClient client( shared.world.pulse, shared.world.ClientSettings() );
        DRNUTILS::CRemoteShareMount mount( client.connection, loaded );
        CORE::CString error;
        ASSERT_FALSE( mount.Start( shared.credentials.drnSessionName, error ) );     // channel 2: the connection has no such channel
        DRNUTILS::CRemoteShareMountSettings withBridge = ShareSettings( "rs/seven" );
        withBridge.bridgeSettings.SetAttribute( "maxOpenHandles", CORE::CString( "3" ) );
        withBridge.bridgeSettings.SetAttribute( "metadataTtlMs", CORE::CString( "777" ) );
        DRNUTILS::CRemoteShareMount bridged( client.connection, withBridge );
        REQUIRE_TRUE( bridged.Start( shared.credentials.drnSessionName, error ) );
        VFS::CVFS::TArchivePtr archive = vfs.GetMountedArchive( bridged.GetArchiveName() );
        REQUIRE_TRUE( !archive.IsNULL() );
        VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* bridge = static_cast< VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* >( archive.GetPointerAlways() );
        ASSERT_TRUE( 3 == bridge->GetSettings().maxOpenHandles && 777 == bridge->GetSettings().metadataTtlMs );
        bridged.SetFailed( "the owner says so" );
        ASSERT_TRUE( DRNUTILS::REMOTESHARE_FAILED == bridged.GetState() && bridged.GetLastError() == "the owner says so" );
        ASSERT_FALSE( bridged.Start( shared.credentials.drnSessionName, error ) );
        bridged.Stop();
        ASSERT_TRUE( DRNUTILS::REMOTESHARE_IDLE == bridged.GetState() );
        ASSERT_FALSE( vfs.IsMountedArchive( "rs/seven" ) );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShare-8 the settings of the block cache and the identity of the share pass through the bridge settings: the share is read through the cache of the folder named, under the identity named" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CShareWorld shared( "share8" );
        REQUIRE_TRUE( shared.Start() );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "a.bin" ), 200000, 81 ) );
        const CORE::CString sessionName = shared.credentials.drnSessionName;
        CShareClient client( shared.world.pulse, shared.world.ClientSettings() );

        DRNUTILS::CRemoteShareMountSettings settings = ShareSettings( "rs/eight" );
        settings.bridgeSettings.SetAttribute( "blockCacheFolder", CORE::CombinePath( shared.world.directory, "blockcache" ) );
        settings.bridgeSettings.SetAttribute( "blockCacheMaxBytes", CORE::CString( "3145728" ) );
        settings.bridgeSettings.SetAttribute( "blockCacheBlockBytes", CORE::CString( "65536" ) );
        settings.bridgeSettings.SetAttribute( "cacheIdentity", CORE::CString( "the-shared-share" ) );
        DRNUTILS::CRemoteShareMount mount( client.connection, settings );
        CORE::CString error;
        REQUIRE_TRUE( mount.Start( sessionName, error ) );
        REQUIRE_TRUE( client.connection.StartConnect( shared.credentials, error ) );
        CShareIs online( mount, DRNUTILS::REMOTESHARE_ONLINE );
        REQUIRE_TRUE( PumpUntil( online, 20000 ) );

        VFS::CVFS::TArchivePtr archive = vfs.GetMountedArchive( mount.GetArchiveName() );
        REQUIRE_TRUE( !archive.IsNULL() );
        VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* bridge = static_cast< VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* >( archive.GetPointerAlways() );
        ASSERT_TRUE( bridge->GetCacheIdentity() == "the-shared-share" );
        ASSERT_TRUE( !bridge->GetBlockCache().IsNULL() && 3145728 == bridge->GetBlockCache()->GetMaxBytes() && 65536 == bridge->GetBlockCache()->GetBlockBytes() );
        ASSERT_TRUE( VfsFileReads( vfs, "rs/eight/a.bin", 200000, 81 ) );
        VFS::CVfsRemoteStatistics statistics;
        ASSERT_TRUE( vfs.GetRemoteStatistics( "rs/eight/a.bin", statistics ) );
        ASSERT_TRUE( 3145728 == statistics.maxBytes && statistics.blocksStored >= 3 && statistics.bytesFetched >= 200000 );
        mount.Stop();
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShares-1 one configuration block: two shares of one service make one connection and two mounts, a service that is not paired fails its shares only, the aggregate follows" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CShareWorld shared( "shares1" );
        REQUIRE_TRUE( shared.Start() );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "a.bin" ), 1500, 71 ) );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "sub/b.bin" ), 2500, 72 ) );
        const CORE::CString sessionName = shared.credentials.drnSessionName;

        AUTH::CMemoryCredentialStore store;
        CORE::CString error;
        REQUIRE_TRUE( shared.credentials.SaveToStore( store, "assets", error ) );

        // the configuration: the service "assets" is paired, "ghost" is not; the settings of the service come from its Service child, the share from its Share children
        CORE::CDataNode config;
        CORE::CDataNode* service = config.AddChild( "Service", GUCEF_DATATYPE_OBJECT );
        shared.world.ClientSettings().SaveConfig( *service );
        service->SetAttribute( "slot", CORE::CString( "assets" ) );
        CORE::CDataNode* first = config.AddChild( "Share", GUCEF_DATATYPE_OBJECT );
        first->SetAttribute( "service", CORE::CString( "assets" ) );
        first->SetAttribute( "share", CORE::CString( "files" ) );
        first->SetAttribute( "mountPoint", CORE::CString( "rs/shares/all" ) );
        CORE::CDataNode* second = config.AddChild( "Share", GUCEF_DATATYPE_OBJECT );
        second->SetAttribute( "service", CORE::CString( "assets" ) );
        second->SetAttribute( "share", CORE::CString( "files" ) );
        second->SetAttribute( "mountPoint", CORE::CString( "rs/shares/again" ) );
        second->SetAttribute( "channel", CORE::CString( "0" ) );
        CORE::CDataNode* bridgeNode = second->AddChild( "BridgeSettings", GUCEF_DATATYPE_OBJECT );
        bridgeNode->SetAttribute( "maxOpenHandles", CORE::CString( "2" ) );
        CORE::CDataNode* third = config.AddChild( "Share", GUCEF_DATATYPE_OBJECT );
        third->SetAttribute( "service", CORE::CString( "ghost" ) );
        third->SetAttribute( "share", CORE::CString( "files" ) );
        third->SetAttribute( "mountPoint", CORE::CString( "rs/shares/ghost" ) );

        DRNUTILS::CRemoteShareMounts mounts( shared.world.pulse );
        CShareStateCollector states;
        mounts.Subscribe( &states );
        REQUIRE_TRUE( mounts.LoadConfig( config, error ) );
        ASSERT_TRUE( 3 == mounts.GetEntryCount() && 2 == mounts.GetServiceCount() );
        ASSERT_TRUE( DRNUTILS::REMOTESHARE_IDLE == mounts.GetState() );
        ASSERT_TRUE( GUCEF_NULL == mounts.FindByMountPoint( "rs/shares/all" ) );      // before Start() there are no mounts
        ASSERT_FALSE( mounts.Start( error ) );                                      // no credential store was given yet
        ASSERT_FALSE( error.IsNULLOrEmpty() );
        ASSERT_FALSE( mounts.IsStarted() );
        mounts.SetCredentialStore( &store );
        REQUIRE_TRUE( mounts.Start( error ) );
        ASSERT_FALSE( mounts.Start( error ) );                                      // started already

        // one connection for the two shares of the service, one mount for each share; the one that is not paired failed with its reason, the others go on
        ASSERT_TRUE( GUCEF_NULL != mounts.GetConnection( "assets" ) );
        ASSERT_TRUE( GUCEF_NULL != mounts.GetConnection( "ghost" ) );
        ASSERT_TRUE( mounts.GetConnection( "assets" ) != mounts.GetConnection( "ghost" ) );
        DRNUTILS::CRemoteShareMount* allMount = mounts.FindByMountPoint( "rs/shares/all" );
        DRNUTILS::CRemoteShareMount* againMount = mounts.FindByMountPoint( "/rs/shares/again/" );
        DRNUTILS::CRemoteShareMount* ghostMount = mounts.FindByMountPoint( "rs\\shares\\ghost" );
        REQUIRE_TRUE( GUCEF_NULL != allMount && GUCEF_NULL != againMount && GUCEF_NULL != ghostMount );
        ASSERT_TRUE( allMount != againMount );
        ASSERT_TRUE( allMount == mounts.GetEntryMount( 0 ) && againMount == mounts.GetEntryMount( 1 ) && ghostMount == mounts.GetEntryMount( 2 ) );
        ASSERT_TRUE( DRNUTILS::REMOTESHARE_FAILED == mounts.GetEntryState( 2 ) );
        ASSERT_TRUE( mounts.GetEntryError( 2 ).HasSubstr( "not paired" ) >= 0 );
        ASSERT_FALSE( vfs.IsMountedArchive( "rs/shares/ghost" ) );
        ASSERT_TRUE( vfs.IsMountedArchive( "rs/shares/all" ) && vfs.IsMountedArchive( "rs/shares/again" ) );

        CEntryIs allOnline( mounts, 0, DRNUTILS::REMOTESHARE_ONLINE );
        CEntryIs againOnline( mounts, 1, DRNUTILS::REMOTESHARE_ONLINE );
        ASSERT_TRUE( PumpUntil( allOnline, 30000 ) );
        ASSERT_TRUE( PumpUntil( againOnline, 30000 ) );
        CSharesAre aggregateOnline( mounts, DRNUTILS::REMOTESHARE_ONLINE );
        ASSERT_TRUE( PumpUntil( aggregateOnline, 10000 ) );                         // the failed share does not hold the aggregate back
        ASSERT_TRUE( 2 == mounts.GetStateCount( DRNUTILS::REMOTESHARE_ONLINE ) && 1 == mounts.GetStateCount( DRNUTILS::REMOTESHARE_FAILED ) );
        ASSERT_TRUE( states.Saw( DRNUTILS::REMOTESHARE_ONLINE ) );

        // the files of both mounts read, and one session is registered (by the first of the two)
        ASSERT_TRUE( VfsFileReads( vfs, "rs/shares/all/a.bin", 1500, 71 ) );
        ASSERT_TRUE( VfsFileReads( vfs, "rs/shares/again/sub/b.bin", 2500, 72 ) );
        ASSERT_TRUE( GUCEF_NULL != Registry().Find( sessionName ) );
        ASSERT_TRUE( allMount->HasRegisteredSession() != againMount->HasRegisteredSession() );
        ASSERT_TRUE( 0 == allMount->GetChannel() && 0 == againMount->GetChannel() );
        ASSERT_FALSE( ghostMount->IsMounted() );

        // stopped: everything is unmounted, no registration is left, and Start() again works
        mounts.Stop();
        ASSERT_FALSE( mounts.IsStarted() );
        ASSERT_TRUE( DRNUTILS::REMOTESHARE_IDLE == mounts.GetState() );
        ASSERT_FALSE( vfs.IsMountedArchive( "rs/shares/all" ) || vfs.IsMountedArchive( "rs/shares/again" ) );
        ASSERT_TRUE( GUCEF_NULL == Registry().Find( sessionName ) );
        ASSERT_TRUE( GUCEF_NULL == mounts.GetConnection( "assets" ) );
        mounts.Unsubscribe( &states );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShares-2 the configuration: a round trip, and a bad one is refused with a reason and leaves what was configured" )
    try
    {
        CORE::CDataNode config;
        CORE::CDataNode* service = config.AddChild( "Service", GUCEF_DATATYPE_OBJECT );
        service->SetAttribute( "slot", CORE::CString( "assets" ) );
        service->SetAttribute( "bulkChannels", CORE::CString( "1" ) );
        service->SetAttribute( "clientKind", CORE::CString( "viewer" ) );
        CORE::CDataNode* share = config.AddChild( "Share", GUCEF_DATATYPE_OBJECT );
        share->SetAttribute( "service", CORE::CString( "assets" ) );
        share->SetAttribute( "share", CORE::CString( "textures" ) );
        share->SetAttribute( "mountPoint", CORE::CString( "remote/textures" ) );
        share->SetAttribute( "channel", CORE::CString( "auto" ) );
        CORE::CDataNode* other = config.AddChild( "Share", GUCEF_DATATYPE_OBJECT );
        other->SetAttribute( "service", CORE::CString( "other" ) );
        other->SetAttribute( "share", CORE::CString( "models" ) );
        other->SetAttribute( "mountPoint", CORE::CString( "remote/models" ) );
        other->SetAttribute( "channel", CORE::CString( "1" ) );
        CORE::CDataNode* bridge = other->AddChild( "BridgeSettings", GUCEF_DATATYPE_OBJECT );
        bridge->SetAttribute( "metadataTtlMs", CORE::CString( "500" ) );

        DRNUTILS::CRemoteShareMounts mounts( CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
        CORE::CString error;
        REQUIRE_TRUE( mounts.LoadConfig( config, error ) );
        ASSERT_TRUE( 2 == mounts.GetEntryCount() && 2 == mounts.GetServiceCount() );
        ASSERT_TRUE( mounts.GetEntrySettings( 0 )->share == "textures" && mounts.GetEntrySettings( 1 )->service == "other" && 1 == mounts.GetEntrySettings( 1 )->channel );
        ASSERT_TRUE( DRNUTILS::CRemoteShareMountSettings::AutoChannel == mounts.GetEntrySettings( 0 )->channel );
        ASSERT_TRUE( mounts.GetEntrySettings( 1 )->bridgeSettings.GetAttributeValue( "metadataTtlMs", CORE::CVariant::Empty ).AsString() == "500" );

        // saved and loaded again: the same
        CORE::CDataNode saved;
        REQUIRE_TRUE( mounts.SaveConfig( saved ) );
        DRNUTILS::CRemoteShareMounts again( CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
        REQUIRE_TRUE( again.LoadConfig( saved, error ) );
        ASSERT_TRUE( 2 == again.GetEntryCount() && 2 == again.GetServiceCount() );
        for ( UInt32 i=0; i<2; ++i )
        {
            const DRNUTILS::CRemoteShareMountSettings* a = mounts.GetEntrySettings( i );
            const DRNUTILS::CRemoteShareMountSettings* b = again.GetEntrySettings( i );
            ASSERT_TRUE( a->service == b->service && a->share == b->share && a->mountPoint == b->mountPoint && a->channel == b->channel );
        }
        ASSERT_TRUE( again.GetEntrySettings( 1 )->bridgeSettings.GetAttributeValue( "metadataTtlMs", CORE::CVariant::Empty ).AsString() == "500" );
        CORE::CDataNode savedAgain;
        REQUIRE_TRUE( again.SaveConfig( savedAgain ) );
        ASSERT_TRUE( 1 == CountChildren( saved, "Service" ) && 2 == CountChildren( saved, "Share" ) );          // the one declared service and the two shares
        ASSERT_TRUE( 1 == CountChildren( savedAgain, "Service" ) && 2 == CountChildren( savedAgain, "Share" ) );
        CORE::CDataNode* declared = saved.FindChild( "Service" );
        REQUIRE_TRUE( GUCEF_NULL != declared );
        ASSERT_TRUE( declared->GetAttributeValue( "slot", CORE::CVariant::Empty ).AsString() == "assets" && 1 == declared->GetAttributeValue( "bulkChannels", CORE::CVariant::Empty ).AsUInt32() );

        // bad ones: each is refused with a reason, and the configuration of before stays
        const char* const reasons[] = { "unknown key of a Share", "no service", "no share", "no mount point", "the same mount point twice", "a channel that is none", "a channel beyond the bulk channels", "an unknown child" };
        for ( UInt32 round=0; round<8; ++round )
        {
            CORE::CDataNode bad( config );
            CORE::CDataNode* badShare = bad.FindChild( "Share" );
            REQUIRE_TRUE( GUCEF_NULL != badShare );
            switch ( round )
            {
                case 0: badShare->SetAttribute( "shares", CORE::CString( "typo" ) ); break;
                case 1: badShare->DelAttribute( "service" ); break;
                case 2: badShare->DelAttribute( "share" ); break;
                case 3: badShare->DelAttribute( "mountPoint" ); break;
                case 4: badShare->SetAttribute( "mountPoint", CORE::CString( "/remote/models/" ) ); break;
                case 5: badShare->SetAttribute( "channel", CORE::CString( "first" ) ); break;
                case 6: badShare->SetAttribute( "channel", CORE::CString( "3" ) ); break;
                default: bad.AddChild( "Shares", GUCEF_DATATYPE_OBJECT ); break;
            }
            CORE::CString badError;
            const bool loaded = mounts.LoadConfig( bad, badError );
            if ( loaded )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RemoteShares test: the bad configuration \"" << reasons[ round ] << "\" was accepted" );
            }
            ASSERT_FALSE( loaded );
            ASSERT_FALSE( badError.IsNULLOrEmpty() );
            ASSERT_TRUE( 2 == mounts.GetEntryCount() && mounts.GetEntrySettings( 0 )->share == "textures" );
        }
        CORE::CDataNode twoSlots( config );
        CORE::CDataNode* secondService = twoSlots.AddChild( "Service", GUCEF_DATATYPE_OBJECT );
        secondService->SetAttribute( "slot", CORE::CString( "assets" ) );
        ASSERT_FALSE( mounts.LoadConfig( twoSlots, error ) );
        ASSERT_TRUE( error.HasSubstr( "twice" ) >= 0 );
        CORE::CDataNode noSlot( config );
        noSlot.AddChild( "Service", GUCEF_DATATYPE_OBJECT );
        ASSERT_FALSE( mounts.LoadConfig( noSlot, error ) );
        ASSERT_TRUE( error.HasSubstr( "slot" ) >= 0 );
        CORE::CDataNode badService( config );
        badService.FindChild( "Service" )->SetAttribute( "bulkChannel", CORE::CString( "1" ) );
        ASSERT_FALSE( mounts.LoadConfig( badService, error ) );
        CORE::CDataNode attribute( config );
        attribute.SetAttribute( "mountPoint", CORE::CString( "x" ) );
        ASSERT_FALSE( mounts.LoadConfig( attribute, error ) );
        ASSERT_TRUE( 2 == mounts.GetEntryCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE SHARES OF THE HOST                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A decision that is not ready to say whether a file may be read: it asks the member to come back */
class CTestBusyDecision : public DRNUTILS::CIRemoteShareDecision
{
    public:

    virtual bool CheckRead( const DRNUTILS::CDeviceRecord& /* device */, const CORE::CString& /* share */, const CORE::CString& /* name */, CORE::CString& refusal, bool& retryLater ) GUCEF_VIRTUAL_OVERRIDE
    {
        refusal = "the licence of this file is being looked up";
        retryLater = true;
        return false;
    }
};

/*-------------------------------------------------------------------------*/

/** A decision of a share that says what it was asked and answers from tables of the test */
class CTestShareDecision : public DRNUTILS::CIRemoteShareDecision
{
    public:

    CORE::CString::StringSet readable;
    CORE::CString::StringSet hidden;
    CORE::CString refusalText;
    MT::CTAtomic< UInt32 > reads;
    MT::CTAtomic< UInt32 > browses;
    MT::CTAtomic< UInt32 > lists;
    mutable MT::CMutex lock;
    CORE::CString lastRoles;
    CORE::CString lastDevice;

    CTestShareDecision( void ) : DRNUTILS::CIRemoteShareDecision(), readable(), hidden(), refusalText(), reads( 0 ), browses( 0 ), lists( 0 ), lock(), lastRoles(), lastDevice() {}

    virtual bool MayRead( const DRNUTILS::CDeviceRecord& device, const CORE::CString& /* share */, const CORE::CString& name, CORE::CString& refusal ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++reads;
        Remember( device );
        if ( readable.find( name ) != readable.end() )
            return true;
        refusal = refusalText;
        return false;
    }

    virtual bool MayBrowse( const DRNUTILS::CDeviceRecord& device, const CORE::CString& /* share */, const CORE::CString& /* name */ ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++browses;
        Remember( device );
        return true;
    }

    virtual bool MayListEntry( const DRNUTILS::CDeviceRecord& device, const CORE::CString& /* share */, const CORE::CString& name, bool /* isDirectory */ ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++lists;
        Remember( device );
        return hidden.find( name ) == hidden.end();
    }

    CORE::CString GetLastRoles( void ) const
    {
        MT::CScopeMutex guard( lock );
        return lastRoles;
    }

    private:

    void Remember( const DRNUTILS::CDeviceRecord& device )
    {
        MT::CScopeMutex guard( lock );
        lastRoles = device.GetRolesText();
        lastDevice = device.deviceId;
    }
};

/** A call of the archive of a mount, made on a thread of its own while this thread pumps, with what the call was told */
class CArchiveProbe : public MT::CActiveObject, public CCondition
{
    public:

    enum EOperation { OPERATION_READ = 0, OPERATION_SIZE, OPERATION_LIST, OPERATION_EXISTS, OPERATION_HASH };

    VFS::CVFS::TArchivePtr archive;
    EOperation operation;
    CORE::CString path;
    bool ok;
    UInt64 size;
    CORE::CDynamicBuffer content;
    VFS::CArchive::TStringVector names;
    CORE::CString hash;
    VFSPLUGIN::DRNBRIDGE::CDrnBridgeResult result;
    MT::CTAtomic< UInt32 > finished;

    CArchiveProbe( const VFS::CVFS::TArchivePtr& a, EOperation op, const CORE::CString& p )
        : MT::CActiveObject(), CCondition(), archive( a ), operation( op ), path( p ), ok( false ), size( 0 ), content(), names(), hash(), result(), finished( 0 ) {}

    virtual ~CArchiveProbe() GUCEF_VIRTUAL_OVERRIDE { Deactivate( false, true ); }

    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return 0 != finished.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        try
        {
            VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* bridge = static_cast< VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* >( archive.GetPointerAlways() );
            if ( OPERATION_EXISTS == operation )
                ok = archive->FileExists( path );
            else
            if ( OPERATION_SIZE == operation )
            {
                size = archive->GetFileSize( path );
                ok = true;
            }
            else
            if ( OPERATION_HASH == operation )
            {
                hash = archive->GetFileHash( path );
                ok = !hash.IsNULLOrEmpty();
            }
            else
            if ( OPERATION_LIST == operation )
            {
                archive->GetFileList( names, "", path, false, false );
                VFS::CArchive::TStringVector dirs;
                archive->GetDirList( dirs, "", path, false, false );
                for ( UInt32 i=0; i<dirs.size(); ++i )
                    names.push_back( dirs[ i ] );
                ok = true;
            }
            else
            {
                VFS::TBasicVfsResourcePtr file = archive->GetFile( path, "rb", 0, false );
                if ( !file.IsNULL() && !file->GetAccess().IsNULL() )
                {
                    CORE::CDynamicBuffer piece( 65536, true );
                    ok = true;
                    while ( true )
                    {
                        const UInt32 read = file->GetAccess()->Read( piece.GetBufferPtr(), 1, 65536 );
                        if ( 0 == read )
                            break;
                        content.Append( piece.GetConstBufferPtr(), read );
                    }
                }
            }
            result = bridge->GetLastResult();
        }
        catch ( ... )
        {
        }
        finished.Store( 1 );
        return true;
    }
};

bool
ContentIsSeeded( const CORE::CDynamicBuffer& content, UInt32 size, UInt8 seed )
{
    if ( content.GetDataSize() != size )
        return false;
    const UInt8* in = static_cast< const UInt8* >( content.GetConstBufferPtr() );
    for ( UInt32 i=0; i<size; ++i )
        if ( in[ i ] != ByteAt( seed, i ) )
            return false;
    return true;
}

bool
HasName( const VFS::CArchive::TStringVector& names, const CORE::CString& name )
{
    for ( UInt32 i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return true;
    return false;
}

/** A host with the files of a directory as a share named "files", and paired devices; the share is the world's own, not registered by hand */
class CHostShareWorld
{
    public:

    CRemoteWorld world;
    CORE::CString served;
    DRN::CIDRNResourceProviderPtr provider;
    CTestShareDecision decision;
    DRNUTILS::CDeviceCredentials viewer;

    explicit CHostShareWorld( const CORE::CString& name ) : world( name ), served(), provider(), decision(), viewer()
    {
        served = CORE::CombinePath( world.directory, "served" );
        CORE::CreateDirs( served );
        CORE::TFileSystemAccessPtr fileSystem = ( GUCEF_NEW CORE::CNativeFileSystemAccess() )->CreateSharedPtr();
        provider = DRN::CIDRNResourceProviderPtr( GUCEF_NEW DRN::CDRNFileSystemAccessResourceProvider( fileSystem, served, CORE::CUri( "file://" + served ) ) );
    }

    bool Start( const char* viewerRoles, const char* shareRoles )
    {
        if ( !world.StartHost() )
            return false;
        CORE::CString error;
        if ( !world.Pair( "viewer", viewerRoles, viewer, error ) )
            return false;
        return Register( shareRoles );
    }

    bool Register( const char* shareRoles )
    {
        CORE::CString::StringSet roles;
        CORE::CString::StringVector list = CORE::CString( shareRoles ).ParseElements( ',', false );
        for ( UInt32 i=0; i<list.size(); ++i )
            roles.insert( list[ i ] );
        CORE::CString error;
        return world.host->RegisterShare( "files", provider, roles, &decision, error );
    }

    CORE::CString Path( const CORE::CString& relative ) const { return CORE::CombinePath( served, relative ); }
};

VFS::CVFS::TArchivePtr
ArchiveOf( VFS::CVFS& vfs, const DRNUTILS::CRemoteShareMount& mount )
{
    return vfs.GetMountedArchive( mount.GetArchiveName() );
}

DRNUTILS::CDeviceRecord
MakeShareDevice( const char* id, const char* rolesCsv, DRNUTILS::TDeviceStatus status )
{
    DRNUTILS::CDeviceRecord record;
    record.deviceId = id;
    record.userId = "owner";
    record.status = status;
    CORE::CString::StringVector roles = CORE::CString( rolesCsv ).ParseElements( ',', false );
    for ( UInt32 i=0; i<roles.size(); ++i )
        if ( !roles[ i ].IsNULLOrEmpty() )
            record.roles.insert( roles[ i ] );
    return record;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

void
PerformDrnUtilsRemoteShareHostTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsRemoteShareHost" );

    VFSPLUGIN::DRNBRIDGE::CDrnBridgeGlobal::Instance();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    GUCEF_TESTFW_TESTCASE( "RemoteShareHost-1 policy of the shares: the registry at the call decides (not the roles of the token), the decision has the last word, a name that is no share is left to the facility" )
    try
    {
        DRNUTILS::CDeviceRegistry registry;
        registry.SetInMemory();
        registry.Put( MakeShareDevice( "d-VIEW", "viewer", DRNUTILS::DEVICESTATUS_ACTIVE ) );
        registry.Put( MakeShareDevice( "d-OWNR", "owner,viewer", DRNUTILS::DEVICESTATUS_ACTIVE ) );
        registry.Put( MakeShareDevice( "d-GONE", "viewer", DRNUTILS::DEVICESTATUS_REVOKED ) );
        registry.Put( MakeShareDevice( "d-NONE", "", DRNUTILS::DEVICESTATUS_ACTIVE ) );
        DRNUTILS::CDeviceDrnAccessPolicy policy( registry );
        CTestShareDecision decision;
        decision.readable.insert( "ok.bin" );
        decision.hidden.insert( "secret" );
        decision.refusalText = "this one is not for you";
        CORE::CString::StringSet viewerRoles;
        viewerRoles.insert( "viewer" );
        policy.SetShare( "files", viewerRoles, &decision );
        ASSERT_TRUE( policy.HasShare( "files" ) && !policy.HasShare( "other" ) );

        DRN::CDRNAuthResult view;
        view.userId = "d-VIEW";
        view.role = "owner";                                   // what a token says is not what decides
        CORE::CString refusal;
        ASSERT_TRUE( policy.CheckReadResourceAs( 2, view, "files", "ok.bin", true, refusal ) );
        ASSERT_FALSE( policy.CheckReadResourceAs( 2, view, "files", "no.bin", true, refusal ) );
        ASSERT_TRUE( refusal == "this one is not for you" );
        ASSERT_FALSE( policy.CheckReadResourceAs( 2, view, "files", "ok.bin", false, refusal ) );          // the share rules said no
        ASSERT_TRUE( policy.MayBrowseResourceAs( 2, view, "files", "no.bin", true ) );                     // browsing is not reading
        ASSERT_TRUE( policy.MayListEntryAs( 2, view, "files", "ok.bin", false ) );
        ASSERT_FALSE( policy.MayListEntryAs( 2, view, "files", "secret", true ) );
        ASSERT_TRUE( decision.GetLastRoles() == "viewer" );

        // the form of the read check that can say "ask again": a decision that does not use it is a plain yes or no, whatever the caller had in the flag
        bool retryLater = true;
        refusal.Clear();
        ASSERT_TRUE( policy.DecideReadResourceAs( 2, view, "files", "ok.bin", true, refusal, retryLater ) && !retryLater );
        retryLater = true;
        ASSERT_FALSE( policy.DecideReadResourceAs( 2, view, "files", "no.bin", true, refusal, retryLater ) );
        ASSERT_TRUE( !retryLater && refusal == "this one is not for you" );
        retryLater = true;
        ASSERT_FALSE( policy.DecideReadResourceAs( 2, view, "files", "ok.bin", false, refusal, retryLater ) && !retryLater );
        ASSERT_FALSE( retryLater );
        CTestBusyDecision busy;
        policy.SetShare( "busyfiles", viewerRoles, &busy );
        ASSERT_FALSE( policy.DecideReadResourceAs( 2, view, "busyfiles", "any.bin", true, refusal, retryLater ) );
        ASSERT_TRUE( retryLater && refusal == "the licence of this file is being looked up" );
        ASSERT_FALSE( policy.CheckReadResourceAs( 2, view, "busyfiles", "any.bin", true, refusal ) );     // a caller that cannot be told to ask again has a refusal
        ASSERT_FALSE( policy.MayReadResourceAs( 2, view, "busyfiles", "any.bin", true ) );
        policy.RemoveShare( "busyfiles" );

        // the registry at this call: the role taken away is gone at the next call, whatever the token says
        DRN::CDRNAuthResult owner;
        owner.userId = "d-OWNR";
        owner.role = "viewer";
        ASSERT_TRUE( policy.MayReadResourceAs( 3, owner, "files", "ok.bin", true ) );
        ASSERT_TRUE( decision.GetLastRoles() == "owner,viewer" );
        registry.Put( MakeShareDevice( "d-OWNR", "curator", DRNUTILS::DEVICESTATUS_ACTIVE ) );
        ASSERT_FALSE( policy.MayReadResourceAs( 3, owner, "files", "ok.bin", true ) );
        ASSERT_FALSE( policy.MayBrowseResourceAs( 3, owner, "files", "", true ) );
        registry.Put( MakeShareDevice( "d-OWNR", "viewer", DRNUTILS::DEVICESTATUS_ACTIVE ) );
        ASSERT_TRUE( policy.MayReadResourceAs( 3, owner, "files", "ok.bin", true ) );

        // not active, not known, no role, no user id
        DRN::CDRNAuthResult gone;
        gone.userId = "d-GONE";
        DRN::CDRNAuthResult none;
        none.userId = "d-NONE";
        DRN::CDRNAuthResult stranger;
        stranger.userId = "d-STRANGER";
        DRN::CDRNAuthResult anonymous;
        ASSERT_FALSE( policy.MayReadResourceAs( 4, gone, "files", "ok.bin", true ) || policy.MayBrowseResourceAs( 4, gone, "files", "", true ) || policy.MayListEntryAs( 4, gone, "files", "ok.bin", false ) );
        ASSERT_FALSE( policy.MayReadResourceAs( 5, none, "files", "ok.bin", true ) || policy.MayBrowseResourceAs( 5, none, "files", "", true ) );
        ASSERT_FALSE( policy.MayReadResourceAs( 6, stranger, "files", "ok.bin", true ) || policy.MayBrowseResourceAs( 6, stranger, "files", "", true ) );
        ASSERT_FALSE( policy.MayReadResourceAs( 7, anonymous, "files", "ok.bin", true ) || policy.MayBrowseResourceAs( 7, anonymous, "files", "", true ) );

        // any role ('*'): any active device with a role; no decision: nothing is read
        CORE::CString::StringSet anyRole;
        anyRole.insert( "*" );
        policy.SetShare( "everyone", anyRole, &decision );
        ASSERT_TRUE( policy.MayBrowseResourceAs( 2, view, "everyone", "", true ) );
        ASSERT_FALSE( policy.MayBrowseResourceAs( 5, none, "everyone", "", true ) );
        policy.SetShare( "undecided", anyRole, GUCEF_NULL );
        ASSERT_FALSE( policy.MayReadResourceAs( 2, view, "undecided", "ok.bin", true ) );
        ASSERT_FALSE( policy.MayBrowseResourceAs( 2, view, "undecided", "", true ) );

        // a provider that is no share: what the share rules decided stands, nothing is hidden
        ASSERT_TRUE( policy.MayReadResourceAs( 2, anonymous, "elsewhere", "x", true ) );
        ASSERT_FALSE( policy.MayReadResourceAs( 2, anonymous, "elsewhere", "x", false ) );
        ASSERT_TRUE( policy.MayListEntryAs( 2, anonymous, "elsewhere", "x", false ) );

        // taken away: no decision is asked any more
        const UInt32 asked = decision.reads.Load();
        ASSERT_TRUE( policy.RemoveShare( "files" ) );
        ASSERT_FALSE( policy.RemoveShare( "files" ) );
        ASSERT_TRUE( policy.MayReadResourceAs( 2, view, "files", "ok.bin", true ) );                     // now a provider that is no share
        ASSERT_TRUE( asked == decision.reads.Load() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShareHost-2 RegisterShare on a running host: a mount lists (hidden entries left out), stats, reads what the decision allows, and is refused the rest with the reason the decision gave" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CHostShareWorld shared( "hostshare2" );
        REQUIRE_TRUE( shared.Start( "viewer", "viewer" ) );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "ok.bin" ), 70000, 5 ) );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "no.bin" ), 1234, 6 ) );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "hidden.bin" ), 100, 7 ) );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "dir/inner.bin" ), 300, 8 ) );
        shared.decision.readable.insert( "ok.bin" );
        shared.decision.readable.insert( "dir/inner.bin" );
        shared.decision.hidden.insert( "hidden.bin" );
        shared.decision.readable.insert( "hidden.bin" );             // readable, so that the hiding is what answers
        shared.decision.refusalText = "there is nothing for you in this file";
        ASSERT_TRUE( shared.world.host->HasShare( "files" ) && shared.world.host->IsShareServed( "files" ) );
        CORE::CString error;
        ASSERT_FALSE( shared.world.host->RegisterShare( "files", shared.provider, CORE::CString::StringSet(), &shared.decision, error ) );      // registered already
        ASSERT_FALSE( error.IsNULLOrEmpty() );
        ASSERT_FALSE( shared.world.host->RegisterShare( "not a name/", shared.provider, CORE::CString::StringSet(), &shared.decision, error ) );
        ASSERT_FALSE( shared.world.host->RegisterShare( "empty", DRN::CIDRNResourceProviderPtr(), CORE::CString::StringSet(), &shared.decision, error ) );

        CShareClient client( shared.world.pulse, shared.world.ClientSettings() );
        DRNUTILS::CRemoteShareMount mount( client.connection, ShareSettings( "rs/hs2" ) );
        REQUIRE_TRUE( mount.Start( shared.viewer.drnSessionName, error ) );
        REQUIRE_TRUE( client.connection.StartConnect( shared.viewer, error ) );
        CShareIs online( mount, DRNUTILS::REMOTESHARE_ONLINE );
        REQUIRE_TRUE( PumpUntil( online, 20000 ) );
        VFS::CVFS::TArchivePtr archive = ArchiveOf( vfs, mount );
        REQUIRE_TRUE( !archive.IsNULL() );

        CArchiveProbe list( archive, CArchiveProbe::OPERATION_LIST, "" );
        list.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( list, 30000 ) );
        ASSERT_TRUE( HasName( list.names, "ok.bin" ) && HasName( list.names, "no.bin" ) && HasName( list.names, "dir/" ) );
        ASSERT_FALSE( HasName( list.names, "hidden.bin" ) );

        CArchiveProbe size( archive, CArchiveProbe::OPERATION_SIZE, "no.bin" );
        size.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( size, 30000 ) );
        ASSERT_TRUE( 1234 == size.size );                                       // names and sizes of what may not be read

        CArchiveProbe hiddenSize( archive, CArchiveProbe::OPERATION_EXISTS, "hidden.bin" );
        hiddenSize.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( hiddenSize, 30000 ) );
        ASSERT_FALSE( hiddenSize.ok );

        CArchiveProbe allowed( archive, CArchiveProbe::OPERATION_READ, "ok.bin" );
        allowed.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( allowed, 30000 ) );
        ASSERT_TRUE( allowed.ok && ContentIsSeeded( allowed.content, 70000, 5 ) );

        CArchiveProbe inner( archive, CArchiveProbe::OPERATION_READ, "dir/inner.bin" );
        inner.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( inner, 30000 ) );
        ASSERT_TRUE( inner.ok && ContentIsSeeded( inner.content, 300, 8 ) );

        CArchiveProbe refused( archive, CArchiveProbe::OPERATION_READ, "no.bin" );
        refused.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( refused, 30000 ) );
        ASSERT_FALSE( refused.ok );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_ACCESS_DENIED == refused.result.fileSystemResult );
        ASSERT_TRUE( refused.result.message.HasSubstr( "there is nothing for you in this file" ) >= 0 );

        CArchiveProbe hiddenRead( archive, CArchiveProbe::OPERATION_READ, "hidden.bin" );
        hiddenRead.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( hiddenRead, 30000 ) );
        ASSERT_FALSE( hiddenRead.ok );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == hiddenRead.result.fileSystemResult );

        // the hooks ran with the roles of the registry
        ASSERT_TRUE( shared.decision.GetLastRoles() == "viewer" );
        ASSERT_TRUE( shared.decision.reads.Load() > 0 && shared.decision.browses.Load() > 0 && shared.decision.lists.Load() > 0 );
        mount.Stop();
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShareHost-3 the roles of the registry at the call: a role taken away takes effect at the next call and a role given back works again; a revoked device cannot continue" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CHostShareWorld shared( "hostshare3" );
        REQUIRE_TRUE( shared.Start( "viewer", "viewer" ) );
        for ( UInt32 i=0; i<5; ++i )
            REQUIRE_TRUE( WriteSeeded( shared.Path( "f" + CORE::ToString( i ) + ".bin" ), 2000 + i, (UInt8) ( 40 + i ) ) );
        for ( UInt32 i=0; i<5; ++i )
            shared.decision.readable.insert( "f" + CORE::ToString( i ) + ".bin" );
        CShareClient client( shared.world.pulse, shared.world.ClientSettings() );
        DRNUTILS::CRemoteShareMount mount( client.connection, ShareSettings( "rs/hs3" ) );
        CORE::CString error;
        REQUIRE_TRUE( mount.Start( shared.viewer.drnSessionName, error ) );
        REQUIRE_TRUE( client.connection.StartConnect( shared.viewer, error ) );
        CShareIs online( mount, DRNUTILS::REMOTESHARE_ONLINE );
        REQUIRE_TRUE( PumpUntil( online, 20000 ) );
        VFS::CVFS::TArchivePtr archive = ArchiveOf( vfs, mount );
        REQUIRE_TRUE( !archive.IsNULL() );

        CArchiveProbe first( archive, CArchiveProbe::OPERATION_READ, "f0.bin" );
        first.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( first, 30000 ) );
        ASSERT_TRUE( first.ok && ContentIsSeeded( first.content, 2000, 40 ) );

        // the role of the share is taken away in the registry: the very next read is refused, on the link that is up
        CORE::CString::StringSet other;
        other.insert( "curator" );
        REQUIRE_TRUE( shared.world.host->SetDeviceRoles( shared.viewer.deviceId, other, "test", error ) );
        CArchiveProbe second( archive, CArchiveProbe::OPERATION_READ, "f1.bin" );
        second.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( second, 30000 ) );
        ASSERT_FALSE( second.ok );
        ASSERT_TRUE( CORE::FILESYSTEMACCESS_ACCESS_DENIED == second.result.fileSystemResult );

        // given back: the next read works
        CORE::CString::StringSet viewerRole;
        viewerRole.insert( "viewer" );
        REQUIRE_TRUE( shared.world.host->SetDeviceRoles( shared.viewer.deviceId, viewerRole, "test", error ) );
        CArchiveProbe third( archive, CArchiveProbe::OPERATION_READ, "f2.bin" );
        third.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( third, 30000 ) );
        ASSERT_TRUE( third.ok && ContentIsSeeded( third.content, 2002, 42 ) );

        // revoked: the link goes, the device is not let in again, nothing more is read
        REQUIRE_TRUE( shared.world.host->RevokeDevice( shared.viewer.deviceId, "lost", "test", error ) );
        PumpFor( 1500 );
        CArchiveProbe fourth( archive, CArchiveProbe::OPERATION_READ, "f3.bin" );
        fourth.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( fourth, 30000 ) );
        ASSERT_FALSE( fourth.ok );
        PumpFor( 1500 );
        ASSERT_FALSE( DRNUTILS::REMOTECONNECTION_CONNECTED == client.connection.GetState() );
        ASSERT_FALSE( DRNUTILS::REMOTESHARE_ONLINE == mount.GetState() );
        mount.Stop();
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShareHost-4 Stop() and Start() of the host serve the share again on the new session, and the mount recovers by itself; UnregisterShare() takes it away, and a share registered before Start() is served by it" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CHostShareWorld shared( "hostshare4" );
        REQUIRE_TRUE( shared.Start( "viewer", "viewer" ) );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "a.bin" ), 3000, 61 ) );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "b.bin" ), 3001, 62 ) );
        shared.decision.readable.insert( "a.bin" );
        shared.decision.readable.insert( "b.bin" );
        CShareClient client( shared.world.pulse, shared.world.ClientSettings() );
        DRNUTILS::CRemoteShareMount mount( client.connection, ShareSettings( "rs/hs4" ) );
        CORE::CString error;
        REQUIRE_TRUE( mount.Start( shared.viewer.drnSessionName, error ) );
        REQUIRE_TRUE( client.connection.StartConnect( shared.viewer, error ) );
        CShareIs online( mount, DRNUTILS::REMOTESHARE_ONLINE );
        REQUIRE_TRUE( PumpUntil( online, 20000 ) );
        VFS::CVFS::TArchivePtr archive = ArchiveOf( vfs, mount );
        REQUIRE_TRUE( !archive.IsNULL() );
        CArchiveProbe before( archive, CArchiveProbe::OPERATION_READ, "a.bin" );
        before.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( before, 30000 ) );
        ASSERT_TRUE( before.ok );

        // the host stops: the session is gone with its providers, the share stays registered, not served (the contract of the world's own test service is
        // registered on the RPC host that goes, and again after the start)
        GUCEF_DELETE shared.world.service;
        shared.world.service = GUCEF_NULL;
        shared.world.host->Stop();
        ASSERT_FALSE( shared.world.host->IsRunning() );
        ASSERT_TRUE( shared.world.host->HasShare( "files" ) );
        ASSERT_FALSE( shared.world.host->IsShareServed( "files" ) );
        CShareIs offline( mount, DRNUTILS::REMOTESHARE_OFFLINE );
        ASSERT_TRUE( PumpUntil( offline, 20000 ) );

        // started again: a new DRN session, and the share is on it
        REQUIRE_TRUE( shared.world.host->Start( error ) );
        shared.world.service = GUCEF_NEW CRemoteTestService();
        REQUIRE_TRUE( shared.world.service->Attach( *shared.world.host, shared.world.directory, error ) );
        ASSERT_TRUE( shared.world.host->IsShareServed( "files" ) );
        REQUIRE_TRUE( PumpUntil( online, 40000 ) );
        CArchiveProbe after( archive, CArchiveProbe::OPERATION_READ, "b.bin" );
        after.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( after, 30000 ) );
        ASSERT_TRUE( after.ok && ContentIsSeeded( after.content, 3001, 62 ) );

        // taken away: the decision is not asked again, the share is not there for the mount
        const UInt32 asked = shared.decision.reads.Load();
        ASSERT_TRUE( shared.world.host->UnregisterShare( "files" ) );
        ASSERT_FALSE( shared.world.host->UnregisterShare( "files" ) );
        ASSERT_FALSE( shared.world.host->HasShare( "files" ) || shared.world.host->IsShareServed( "files" ) );
        CArchiveProbe gone( archive, CArchiveProbe::OPERATION_READ, "a.bin" );
        gone.Activate( GUCEF_NULL );
        REQUIRE_TRUE( PumpUntil( gone, 30000 ) );
        ASSERT_FALSE( gone.ok );
        ASSERT_TRUE( asked == shared.decision.reads.Load() );
        mount.Stop();
        client.connection.Close( true );

        // registered while the host does not run: served when it starts
        shared.world.host->Stop();
        REQUIRE_TRUE( shared.Register( "viewer" ) );
        ASSERT_TRUE( shared.world.host->HasShare( "files" ) );
        ASSERT_FALSE( shared.world.host->IsShareServed( "files" ) );
        REQUIRE_TRUE( shared.world.host->Start( error ) );
        ASSERT_TRUE( shared.world.host->IsShareServed( "files" ) );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShareHost-5 two mounts of one connection share the registration of the session (counted): stopping the first leaves the second online, the last one takes the registration away, a session of the application is never counted" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CHostShareWorld shared( "hostshare5" );
        REQUIRE_TRUE( shared.Start( "viewer", "viewer" ) );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "a.bin" ), 2500, 71 ) );
        shared.decision.readable.insert( "a.bin" );
        const CORE::CString sessionName = shared.viewer.drnSessionName;
        CShareClient client( shared.world.pulse, shared.world.ClientSettings() );
        DRNUTILS::CRemoteShareMount first( client.connection, ShareSettings( "rs/hs5a" ) );
        DRNUTILS::CRemoteShareMount second( client.connection, ShareSettings( "rs/hs5b" ) );
        CORE::CString error;
        REQUIRE_TRUE( first.Start( sessionName, error ) );
        REQUIRE_TRUE( second.Start( sessionName, error ) );
        REQUIRE_TRUE( client.connection.StartConnect( shared.viewer, error ) );
        CShareIs firstOnline( first, DRNUTILS::REMOTESHARE_ONLINE );
        CShareIs secondOnline( second, DRNUTILS::REMOTESHARE_ONLINE );
        REQUIRE_TRUE( PumpUntil( firstOnline, 20000 ) );
        REQUIRE_TRUE( PumpUntil( secondOnline, 20000 ) );
        DRN::CDRNSession* session = client.connection.GetDrnSession( 0 );
        REQUIRE_TRUE( GUCEF_NULL != session && Registry().Find( sessionName ) == session );
        ASSERT_TRUE( 2 == DRNUTILS::CRemoteShareMount::GetRegistrationHoldCount( session ) );
        ASSERT_TRUE( first.HoldsRegistration() && second.HoldsRegistration() );
        ASSERT_TRUE( first.HasRegisteredSession() != second.HasRegisteredSession() );

        // the one that made the registration stops first: the other keeps the session, and keeps reading
        DRNUTILS::CRemoteShareMount& maker = first.HasRegisteredSession() ? first : second;
        DRNUTILS::CRemoteShareMount& user = first.HasRegisteredSession() ? second : first;
        maker.Stop();
        ASSERT_TRUE( 1 == DRNUTILS::CRemoteShareMount::GetRegistrationHoldCount( session ) );
        ASSERT_TRUE( Registry().Find( sessionName ) == session );
        ASSERT_FALSE( maker.HoldsRegistration() );
        ASSERT_TRUE( user.HoldsRegistration() );
        ASSERT_TRUE( DRNUTILS::REMOTESHARE_ONLINE == user.GetState() );
        ASSERT_TRUE( VfsFileReads( vfs, user.GetSettings().mountPoint + "/a.bin", 2500, 71 ) );

        // the maker starts again and finds the session registered: it holds a share of it too
        REQUIRE_TRUE( maker.Start( sessionName, error ) );
        CShareIs makerOnline( maker, DRNUTILS::REMOTESHARE_ONLINE );
        REQUIRE_TRUE( PumpUntil( makerOnline, 20000 ) );
        ASSERT_TRUE( 2 == DRNUTILS::CRemoteShareMount::GetRegistrationHoldCount( session ) );
        ASSERT_TRUE( maker.HoldsRegistration() && !maker.HasRegisteredSession() );
        maker.Stop();

        // the last one lets go: the registration is taken away
        user.Stop();
        ASSERT_TRUE( 0 == DRNUTILS::CRemoteShareMount::GetRegistrationHoldCount( session ) );
        ASSERT_TRUE( GUCEF_NULL == Registry().Find( sessionName ) );
        ASSERT_FALSE( user.HoldsRegistration() );

        // a session the application registered is not counted, and stays
        REQUIRE_TRUE( Registry().Register( *session ) );
        DRNUTILS::CRemoteShareMount again( client.connection, ShareSettings( "rs/hs5c" ) );
        REQUIRE_TRUE( again.Start( sessionName, error ) );
        CShareIs againOnline( again, DRNUTILS::REMOTESHARE_ONLINE );
        REQUIRE_TRUE( PumpUntil( againOnline, 20000 ) );
        ASSERT_TRUE( 0 == DRNUTILS::CRemoteShareMount::GetRegistrationHoldCount( session ) );
        ASSERT_FALSE( again.HoldsRegistration() || again.HasRegisteredSession() );
        again.Stop();
        ASSERT_TRUE( Registry().Find( sessionName ) == session );
        Registry().Unregister( *session );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteShareHost-6 two mounts of one connection follow a new session of the connection: both come online again, one registration, one count each" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        CHostShareWorld shared( "hostshare6" );
        REQUIRE_TRUE( shared.Start( "viewer", "viewer" ) );
        REQUIRE_TRUE( WriteSeeded( shared.Path( "a.bin" ), 2600, 81 ) );
        shared.decision.readable.insert( "a.bin" );
        const CORE::CString sessionName = shared.viewer.drnSessionName;
        CShareClient client( shared.world.pulse, shared.world.ClientSettings() );
        DRNUTILS::CRemoteShareMount first( client.connection, ShareSettings( "rs/hs6a" ) );
        DRNUTILS::CRemoteShareMount second( client.connection, ShareSettings( "rs/hs6b" ) );
        CORE::CString error;
        REQUIRE_TRUE( first.Start( sessionName, error ) );
        REQUIRE_TRUE( second.Start( sessionName, error ) );
        REQUIRE_TRUE( client.connection.StartConnect( shared.viewer, error ) );
        CShareIs firstOnline( first, DRNUTILS::REMOTESHARE_ONLINE );
        CShareIs secondOnline( second, DRNUTILS::REMOTESHARE_ONLINE );
        REQUIRE_TRUE( PumpUntil( firstOnline, 20000 ) );
        REQUIRE_TRUE( PumpUntil( secondOnline, 20000 ) );

        client.connection.Close( true );
        ASSERT_TRUE( GUCEF_NULL == Registry().Find( sessionName ) );
        CShareIs firstOffline( first, DRNUTILS::REMOTESHARE_OFFLINE );
        CShareIs secondOffline( second, DRNUTILS::REMOTESHARE_OFFLINE );
        ASSERT_TRUE( PumpUntil( firstOffline, 10000 ) );
        ASSERT_TRUE( PumpUntil( secondOffline, 10000 ) );
        REQUIRE_TRUE( client.connection.StartConnect( shared.viewer, error ) );
        REQUIRE_TRUE( PumpUntil( firstOnline, 30000 ) );
        REQUIRE_TRUE( PumpUntil( secondOnline, 30000 ) );
        DRN::CDRNSession* session = client.connection.GetDrnSession( 0 );
        REQUIRE_TRUE( GUCEF_NULL != session && Registry().Find( sessionName ) == session );
        ASSERT_TRUE( 2 == DRNUTILS::CRemoteShareMount::GetRegistrationHoldCount( session ) );
        ASSERT_TRUE( VfsFileReads( vfs, "rs/hs6a/a.bin", 2600, 81 ) && VfsFileReads( vfs, "rs/hs6b/a.bin", 2600, 81 ) );
        first.Stop();
        second.Stop();
        ASSERT_TRUE( GUCEF_NULL == Registry().Find( sessionName ) );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
