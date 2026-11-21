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

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_VFS_CVFSFILESYSTEMACCESS_H
#include "gucefVFS_CVfsFileSystemAccess.h"
#define GUCEF_VFS_CVFSFILESYSTEMACCESS_H
#endif /* GUCEF_VFS_CVFSFILESYSTEMACCESS_H ? */

#ifndef GUCEF_VFS_CVFSURIRESOURCEACCESSOR_H
#include "gucefVFS_CVfsUriResourceAccessor.h"
#define GUCEF_VFS_CVFSURIRESOURCEACCESSOR_H
#endif /* GUCEF_VFS_CVFSURIRESOURCEACCESSOR_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDrnBridgeFixture.h"
#include "TestDrnBridgeBehaviour.h"

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

typedef CORE::CString::StringVector TStrings;

/*-------------------------------------------------------------------------*/

/** The directory a bed of tests makes its files in, below the scratch directory when there is one */
CORE::CString
BedDirectory( const CORE::CString& name )
{
    CORE::CString base = CORE::GetEnv( "GUCEF_TEST_SCRATCH_DIR" );
    if ( base.IsNULLOrEmpty() )
        base = CORE::TempDir();
    CORE::CString dir = CORE::CombinePath( base, "vfsdrn-bed-" + CORE::UInt32ToString( MT::GetProcessID() ) + "-" + name );
    if ( CORE::DirExists( dir ) )
        CORE::DeleteDir( dir, true );
    CORE::CreateDirs( dir );
    return dir;
}

/*-------------------------------------------------------------------------*/

/** The byte at a position of a file that has the given seed */
UInt8
ByteAt( UInt8 seed, UInt64 position )
{
    return (UInt8) ( seed + position * 7 + ( position >> 9 ) );
}

bool
WriteSeeded( const CORE::CString& path, UInt64 size, UInt8 seed )
{
    if ( !CORE::CreatePathDirectories( path ) )
        return false;
    CORE::CFileAccess file( path, "wb" );
    if ( !file.Opened() )
        return false;
    CORE::CDynamicBuffer block( 65536, true );
    UInt64 position = 0;
    while ( position < size )
    {
        const UInt32 count = (UInt32) ( size - position < 65536 ? size - position : 65536 );
        UInt8* out = static_cast< UInt8* >( block.GetBufferPtr() );
        for ( UInt32 i=0; i<count; ++i )
            out[ i ] = ByteAt( seed, position + i );
        if ( count != file.Write( block.GetConstBufferPtr(), 1, count ) )
            return false;
        position += count;
    }
    file.Flush();
    return true;
}

bool
HasSeededContent( const UInt8* data, UInt32 count, UInt64 offset, UInt8 seed )
{
    for ( UInt32 i=0; i<count; ++i )
        if ( data[ i ] != ByteAt( seed, offset + i ) )
            return false;
    return true;
}

/** Reads count bytes at offset through the access and checks them */
bool
ReadSeeded( CORE::CIOAccess& access, UInt64 offset, UInt32 count, UInt8 seed )
{
    CORE::CDynamicBuffer buffer( count, true );
    if ( 0 != access.Setpos( offset ) )
        return false;
    if ( count != access.Read( buffer.GetBufferPtr(), 1, count ) )
        return false;
    return HasSeededContent( static_cast< const UInt8* >( buffer.GetConstBufferPtr() ), count, offset, seed );
}

/** Reads the whole file through the access and checks it */
bool
ReadAllSeeded( CORE::CIOAccess& access, UInt64 size, UInt8 seed )
{
    if ( access.GetSize() != size || 0 != access.Setpos( 0 ) )
        return false;
    CORE::CDynamicBuffer buffer( 20011, true );
    UInt64 position = 0;
    while ( position < size )
    {
        const UInt32 got = access.Read( buffer.GetBufferPtr(), 1, 20011 );
        if ( 0 == got || !HasSeededContent( static_cast< const UInt8* >( buffer.GetConstBufferPtr() ), got, position, seed ) )
            return false;
        position += got;
    }
    return position == size;
}

bool
Contains( const TStrings& names, const CORE::CString& name )
{
    for ( TStrings::const_iterator i = names.begin(); i != names.end(); ++i )
        if ( (*i) == name )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** A directory with files, shared by a serving node, mounted through a connection registered in code, as the archive of a client node */
class CBed
{
    public:

    CORE::CString name;
    CORE::CString root;
    CLoopbackShare* share;
    CDrnBridgeConnectionPtr connection;
    VFS::CVFS::TArchivePtr archive;
    CORE::CString connectionId;

    explicit CBed( const CORE::CString& bedName ) : name( bedName ), root(), share( GUCEF_NULL ), connection(), archive(), connectionId() {}

    ~CBed()
    {
        Stop();
    }

    /** The directory is made first; Start() serves it */
    CORE::CString MakeRoot( void )
    {
        root = BedDirectory( name );
        return root;
    }

    bool Start( const CBridgeMountOptions& options, const DRN::CIDRNResourceProviderPtr& provider = DRN::CIDRNResourceProviderPtr() )
    {
        if ( root.IsNULLOrEmpty() )
            MakeRoot();
        share = GUCEF_NEW CLoopbackShare( "vfsdrn_bed_" + name + "_" + CORE::UInt32ToString( MT::GetProcessID() ) );
        share->SetClientOwnThread( options.clientOwnThread );
        if ( !share->Start( root, provider ) )
            return false;
        CDrnBridgeGlobal::Instance();
        connectionId = share->GetSessionName();
        connection = CDrnBridgeGlobal::Instance()->RegisterConnection( connectionId, share->Client(), share->GetServerPeerId() );
        if ( connection.IsNULL() )
            return false;
        archive = CreateBridgeArchive( "Bed" + name, connectionId + "|" + CLoopbackShare::ShareName, options );
        return !archive.IsNULL();
    }

    void Stop( void )
    {
        archive.Unlink();
        if ( !connectionId.IsNULLOrEmpty() )
        {
            CDrnBridgeGlobal::Instance()->UnregisterConnection( connectionId );
            connectionId.Clear();
        }
        connection.Unlink();
        if ( GUCEF_NULL != share )
        {
            share->Stop();
            GUCEF_DELETE share;
            share = GUCEF_NULL;
        }
        if ( !root.IsNULLOrEmpty() )
        {
            CORE::DeleteDir( root, true );
            root.Clear();
        }
    }

    CDrnBridgeArchive& Bridge( void )
    {
        return *static_cast< CDrnBridgeArchive* >( archive.GetPointerAlways() );
    }

    CORE::CString Path( const CORE::CString& relative ) const
    {
        return CORE::CombinePath( root, relative );
    }

    UInt64 Served( void )
    {
        return share->GetServerStats().namespaceRequestsServed;
    }
};

/*-------------------------------------------------------------------------*/

/** A thread that reads through the same archive as the others, in the way a case of this file needs */
class CPingPongReader : public MT::CActiveObject
{
    public:

    CPingPongReader( VFS::CArchive& archive, const CORE::CString& path, UInt64 size, UInt8 seed, UInt32 rounds )
        : MT::CActiveObject(), m_archive( archive ), m_path( path ), m_size( size ), m_seed( seed ), m_rounds( rounds ), m_done( 0 ), m_failures( 0 ) {}

    virtual ~CPingPongReader() GUCEF_VIRTUAL_OVERRIDE { Deactivate( false, true ); }

    bool IsDone( void ) const { return 0 != m_done.Load(); }
    UInt32 GetFailures( void ) const { return m_failures.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        try
        {
            VFS::TBasicVfsResourcePtr resource = m_archive.GetFile( m_path, "rb", 0, false );
            if ( resource.IsNULL() || resource->GetAccess().IsNULL() )
            {
                m_failures.FetchAdd( 1 );
            }
            else
            {
                CORE::IOAccessPtr access = resource->GetAccess();
                for ( UInt32 i=0; i<m_rounds; ++i )
                {
                    // alternating ends of the file: the handle is needed again after the other thread took it
                    const UInt64 offset = 0 == ( i % 2 ) ? 0 : m_size - 4096;
                    if ( !ReadSeeded( *access, offset, 4096, m_seed ) )
                        m_failures.FetchAdd( 1 );
                    MT::ThreadDelay( 1 );
                }
            }
        }
        catch ( ... )
        {
            m_failures.FetchAdd( 1 );
        }
        m_done.Store( 1 );
        return true;
    }

    private:

    VFS::CArchive& m_archive;
    CORE::CString m_path;
    UInt64 m_size;
    UInt8 m_seed;
    UInt32 m_rounds;
    MT::CTAtomic< UInt32 > m_done;
    MT::CTAtomic< UInt32 > m_failures;
};

/*-------------------------------------------------------------------------*/

/** A policy of the serving node that makes a call of the bridge archive from the node's thread, where a hook of the policy runs */
class CNodeThreadPolicy : public DRN::CDRNDefaultAccessPolicy
{
    public:

    CDrnBridgeArchive* archive;
    bool called;
    bool onNodeThread;
    bool existsAnswer;
    VFS::TBasicVfsResourcePtr openAnswer;
    CDrnBridgeResult existsResult;
    CDrnBridgeResult openResult;

    CNodeThreadPolicy( void ) : DRN::CDRNDefaultAccessPolicy(), archive( GUCEF_NULL ), called( false ), onNodeThread( false ), existsAnswer( true ), openAnswer(), existsResult(), openResult() {}

    virtual bool MayBrowseResourceAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName, bool sharedByProvider ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL != archive && !called )
        {
            called = true;
            onNodeThread = archive->GetConnection()->IsOnNodeThread();
            existsAnswer = archive->FileExists( "never-asked.bin" );
            existsResult = archive->GetLastResult();
            openAnswer = archive->GetFile( "mirror.bin", "rb", 0, false );
            openResult = archive->GetLastResult();
        }
        return DRN::CDRNDefaultAccessPolicy::MayBrowseResourceAs( peerId, auth, providerName, resourceName, sharedByProvider );
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE CASES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
TestMounting( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Mounting: the sources of a mount, the registered connections, and the sources that are refused" )
        CBed bed( "mount" );
        try
        {
            bed.MakeRoot();
            REQUIRE_TRUE( WriteSeeded( bed.Path( "a.bin" ), 1000, 3 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "sub/b.bin" ), 100000, 5 ) );
            CBridgeMountOptions options;
            REQUIRE_TRUE( bed.Start( options ) );

            // the registry of connections: an id is taken once, has no '|', and is found again
            CDrnBridgeGlobal* global = CDrnBridgeGlobal::Instance();
            ASSERT_TRUE( global->RegisterConnection( bed.connectionId, bed.share->Client(), bed.share->GetServerPeerId() ).IsNULL() );
            ASSERT_TRUE( global->RegisterConnection( "", bed.share->Client(), bed.share->GetServerPeerId() ).IsNULL() );
            ASSERT_TRUE( global->RegisterConnection( "a|b", bed.share->Client(), bed.share->GetServerPeerId() ).IsNULL() );
            ASSERT_TRUE( global->FindConnection( bed.connectionId ) == bed.connection );
            ASSERT_TRUE( global->FindConnection( "no such id" ).IsNULL() );
            ASSERT_TRUE( bed.connection->GetId() == bed.connectionId );

            // the archive made through the factory: its type, name, share and that it works
            ASSERT_TRUE( bed.archive->GetType() == "drn" );
            ASSERT_TRUE( bed.archive->GetArchiveName() == "Bedmount" );
            ASSERT_FALSE( bed.archive->IsWriteable() );
            ASSERT_TRUE( bed.Bridge().GetShare() == CLoopbackShare::ShareName );
            ASSERT_TRUE( 1000 == bed.archive->GetFileSize( "a.bin" ) );

            // the registered connection can also be mounted by the typed form, a share that is not shared mounts and just shows nothing
            {
                CDrnBridgeArchive typed;
                ASSERT_TRUE( typed.LoadArchive( bed.connection, "other", CDrnBridgeSettings(), "Typed" ) );
                ASSERT_FALSE( typed.FileExists( "a.bin" ) );
                ASSERT_TRUE( typed.GetLastResult().IsUnreachable() ? false : DRN::DRN_NSRESULT_NO_SUCH_SHARE == typed.GetLastResult().code );
                ASSERT_FALSE( typed.IsHealthy() );
            }
            {
                CDrnBridgeArchive typed;
                ASSERT_FALSE( typed.LoadArchive( CDrnBridgeConnectionPtr(), "files", CDrnBridgeSettings(), "Typed" ) );
                ASSERT_FALSE( typed.LoadArchive( bed.connection, "not a share name", CDrnBridgeSettings(), "Typed" ) );
                ASSERT_FALSE( typed.LoadArchive( bed.connection, "", CDrnBridgeSettings(), "Typed" ) );
                ASSERT_TRUE( DRN::DRN_NSRESULT_INVALID_NAME == typed.GetLastResult().code );
                // an archive that never loaded is not healthy or connected and answers nothing
                ASSERT_FALSE( typed.IsHealthy() );
                ASSERT_FALSE( typed.IsConnected() );
                ASSERT_FALSE( typed.FileExists( "a.bin" ) );
            }

            // the drn:// source: found through the session registry, by "sequencer" and by the node id of the member; the form that the path handling of the settings leaves too
            bed.share->RegisterClientSession();
            CORE::CString sequencerNodeId;
            {
                MT::CScopeMutex lock( bed.share->Client().GetNode().GetLock() );
                DRN::TDRNSessionMemberInfoVector members;
                bed.share->Client().GetMembers( members );
                for ( UInt32 i=0; i<members.size(); ++i )
                    if ( members[ i ].isSequencer )
                        sequencerNodeId = members[ i ].nodeId;
            }
            REQUIRE_TRUE( !sequencerNodeId.IsNULLOrEmpty() );
            const CORE::CString session = bed.share->GetSessionName();
            const char* const goodSources[] = { "drn://%S%/sequencer/files", "drn://%S%/%N%/files", "drn:/%S%/sequencer/files" };
            for ( UInt32 i=0; i<sizeof( goodSources ) / sizeof( goodSources[ 0 ] ); ++i )
            {
                CORE::CString source = CORE::CString( goodSources[ i ] ).ReplaceSubstr( "%S%", session ).ReplaceSubstr( "%N%", sequencerNodeId );
                VFS::CVFS::TArchivePtr archive = CreateBridgeArchive( "ByUri", source, options );
                ASSERT_FALSE( archive.IsNULL() );
                if ( !archive.IsNULL() )
                {
                    ASSERT_TRUE( 1000 == archive->GetFileSize( "a.bin" ) && 100000 == archive->GetFileSize( "sub/b.bin" ) );
                    ASSERT_TRUE( archive->IsConnected() && archive->IsHealthy() );
                }
            }

            // the sources that cannot be mounted: malformed ones (a session that is not registered is not one of them since the session is bound late, see the suite "mounting")
            const char* const badSources[] = { "drn://%S%/sequencer", "drn://%S%/sequencer/files/more", "drn://%S%//files", "no connection|files", "nothing at all", "unregistered id|files", "drn://", "" };
            for ( UInt32 i=0; i<sizeof( badSources ) / sizeof( badSources[ 0 ] ); ++i )
            {
                CORE::CString source = CORE::CString( badSources[ i ] ).ReplaceSubstr( "%S%", session );
                VFS::CVFS::TArchivePtr archive = CreateBridgeArchive( "Bad", source, options );
                if ( !archive.IsNULL() )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeTest: the source \"" << source << "\" was mounted but should not have been" );
                }
                ASSERT_TRUE( archive.IsNULL() );
            }

            // a member that is not in the session: the mount works (it does not need the share now) and says that it cannot be reached
            {
                VFS::CVFS::TArchivePtr archive = CreateBridgeArchive( "Absent", "drn://" + session + "/no such node/files", options );
                ASSERT_FALSE( archive.IsNULL() );
                if ( !archive.IsNULL() )
                {
                    ASSERT_FALSE( archive->IsConnected() );
                    ASSERT_FALSE( archive->FileExists( "a.bin" ) );
                    ASSERT_TRUE( static_cast< CDrnBridgeArchive* >( archive.GetPointerAlways() )->GetLastResult().IsUnreachable() );
                }
            }

            // loading from a resource is not what this archive is
            {
                CDrnBridgeArchive typed;
                ASSERT_FALSE( typed.LoadArchive( "x", VFS::TBasicVfsResourcePtr(), false ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

}; /* anonymous namespace */

namespace {

void
TestChangedFile( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Changed file: an open file whose share changed fails from the next read on, and opening it again gives the new revision" )
        CBed bed( "changed" );
        try
        {
            bed.MakeRoot();
            const UInt64 size = 2 * 1024 * 1024;
            REQUIRE_TRUE( WriteSeeded( bed.Path( "grow.bin" ), size, 11 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "same.bin" ), 5000, 12 ) );

            // a small chunk cache and no read ahead: what is not in the first chunks comes from the serving side
            CBridgeMountOptions options;
            options.chunkCacheChunks = 2;
            options.readAheadChunks = 0;
            options.metadataTtlMs = 60000;
            REQUIRE_TRUE( bed.Start( options ) );
            CDrnBridgeArchive& bridge = bed.Bridge();

            const CORE::CDateTime timeBefore = bridge.GetFileModificationTime( "grow.bin" );
            VFS::TBasicVfsResourcePtr first = bridge.GetFile( "grow.bin", "rb", 0, false );
            REQUIRE_TRUE( !first.IsNULL() );
            CORE::IOAccessPtr access = first->GetAccess();
            REQUIRE_TRUE( !access.IsNULL() );
            ASSERT_TRUE( ReadSeeded( *access, 0, 100, 11 ) );
            ASSERT_TRUE( access->IsValid() );

            // the share changes behind the open file: same size, other bytes, another modification time
            MT::ThreadDelay( 40 );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "grow.bin" ), size, 99 ) );

            // bytes that were read before are still in the chunk cache; the next ones are of another revision and the file fails
            CORE::CDynamicBuffer buffer( 100, true );
            ASSERT_TRUE( 0 == access->Setpos( 1024 * 1024 ) );
            ASSERT_TRUE( 0 == access->Read( buffer.GetBufferPtr(), 1, 100 ) );
            ASSERT_FALSE( access->IsValid() );
            CDrnBridgeResult failure = bridge.GetLastResult();
            ASSERT_TRUE( DRN::DRN_NSRESULT_CHANGED == failure.code );
            ASSERT_FALSE( failure.IsUnreachable() );

            // the file stays failed, also where its bytes were cached
            ASSERT_TRUE( 0 == access->Setpos( 0 ) );
            ASSERT_TRUE( 0 == access->Read( buffer.GetBufferPtr(), 1, 100 ) );
            ASSERT_TRUE( access->GetSize() == size );

            // opening it again is another file: the new revision, with its own modification time and content
            VFS::TBasicVfsResourcePtr second = bridge.GetFile( "grow.bin", "rb", 0, false );
            REQUIRE_TRUE( !second.IsNULL() );
            CORE::IOAccessPtr newAccess = second->GetAccess();
            ASSERT_TRUE( ReadAllSeeded( *newAccess, size, 99 ) );
            ASSERT_TRUE( bridge.GetFileModificationTime( "grow.bin" ) != timeBefore );
            ASSERT_TRUE( bridge.GetLastResult().IsOk() );

            // a file read to its end is not hurt by a change that comes after it
            {
                VFS::TBasicVfsResourcePtr resource = bridge.GetFile( "same.bin", "rb", 0, false );
                REQUIRE_TRUE( !resource.IsNULL() );
                ASSERT_TRUE( ReadAllSeeded( *resource->GetAccess(), 5000, 12 ) );
                MT::ThreadDelay( 40 );
                REQUIRE_TRUE( WriteSeeded( bed.Path( "same.bin" ), 5000, 13 ) );
                ASSERT_TRUE( ReadSeeded( *resource->GetAccess(), 10, 100, 12 ) );
                ASSERT_TRUE( resource->GetAccess()->IsValid() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Changed file: a file whose handle was closed to make room reopens at the revision it was opened at, and fails when that is gone" )
        CBed bed( "changedreopen" );
        try
        {
            bed.MakeRoot();
            REQUIRE_TRUE( WriteSeeded( bed.Path( "a.bin" ), 300000, 21 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "b.bin" ), 300000, 22 ) );

            CBridgeMountOptions options;
            options.maxOpenHandles = 1;
            options.chunkCacheChunks = 1;
            options.readAheadChunks = 0;
            REQUIRE_TRUE( bed.Start( options ) );
            CDrnBridgeArchive& bridge = bed.Bridge();

            VFS::TBasicVfsResourcePtr a = bridge.GetFile( "a.bin", "rb", 0, false );
            REQUIRE_TRUE( !a.IsNULL() );
            ASSERT_TRUE( ReadSeeded( *a->GetAccess(), 0, 1000, 21 ) );
            // b takes the only handle there is: a has none now
            VFS::TBasicVfsResourcePtr b = bridge.GetFile( "b.bin", "rb", 0, false );
            REQUIRE_TRUE( !b.IsNULL() );
            ASSERT_TRUE( 1 == bridge.GetOpenHandleCount() );
            ASSERT_TRUE( bridge.GetHandleEvictionCount() >= 1 );

            // a reopens and carries on where it was: the revision is the same
            ASSERT_TRUE( ReadSeeded( *a->GetAccess(), 200000, 1000, 21 ) );
            ASSERT_TRUE( bridge.GetHandleReopenCount() >= 1 );
            ASSERT_TRUE( ReadSeeded( *b->GetAccess(), 250000, 1000, 22 ) );

            // a changes while it has no handle (b has it): the reopening for the old revision is refused
            MT::ThreadDelay( 40 );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "a.bin" ), 300001, 23 ) );
            CORE::CDynamicBuffer buffer( 100, true );
            ASSERT_TRUE( 0 == a->GetAccess()->Setpos( 100000 ) );
            ASSERT_TRUE( 0 == a->GetAccess()->Read( buffer.GetBufferPtr(), 1, 100 ) );
            ASSERT_FALSE( a->GetAccess()->IsValid() );
            ASSERT_TRUE( bridge.GetLastResult().IsChanged() );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED == bridge.GetLastResult().fileSystemResult );
            // b is not what changed
            ASSERT_TRUE( ReadSeeded( *b->GetAccess(), 1000, 1000, 22 ) );
            ASSERT_TRUE( b->GetAccess()->IsValid() );

            VFS::TBasicVfsResourcePtr again = bridge.GetFile( "a.bin", "rb", 0, false );
            REQUIRE_TRUE( !again.IsNULL() );
            ASSERT_TRUE( ReadAllSeeded( *again->GetAccess(), 300001, 23 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestLinkLoss( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Link loss: the mount stays, what is cached answers, the rest says the share cannot be reached, and a new link revalidates" )
        CBed bed( "linkloss" );
        try
        {
            bed.MakeRoot();
            REQUIRE_TRUE( WriteSeeded( bed.Path( "cached.bin" ), 1000000, 31 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "uncached.bin" ), 1000, 32 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "dir/inside.bin" ), 500, 33 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "dir2/other.bin" ), 600, 34 ) );

            // an expired entry is still what an offline mount knows: a time to live that is short, to see that
            CBridgeMountOptions options;
            options.metadataTtlMs = 300;
            options.negativeTtlMs = 300;
            REQUIRE_TRUE( bed.Start( options ) );
            CDrnBridgeArchive& bridge = bed.Bridge();

            // what is known before the link goes: a file and its description, a directory and its listing, a file that is open
            ASSERT_TRUE( 1000000 == bridge.GetFileSize( "cached.bin" ) );
            const CORE::CDateTime cachedTime = bridge.GetFileModificationTime( "cached.bin" );
            CORE::CString::StringVector names;
            bridge.GetFileList( names, CORE::CString::Empty, "dir", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            ASSERT_TRUE( 1 == names.size() );
            VFS::TBasicVfsResourcePtr open = bridge.GetFile( "cached.bin", "rb", 0, false );
            REQUIRE_TRUE( !open.IsNULL() );
            ASSERT_TRUE( ReadSeeded( *open->GetAccess(), 0, 100, 31 ) );
            ASSERT_TRUE( bridge.IsConnected() && bridge.IsHealthy() );
            MT::ThreadDelay( 400 );

            bed.share->DropClient();

            // the mount stays: not connected, still healthy
            ASSERT_FALSE( bridge.IsConnected() );
            ASSERT_TRUE( bridge.IsHealthy() );
            ASSERT_FALSE( bed.connection->IsConnected() );

            // what the cache holds answers, though its time is over, and says that it is offline
            ASSERT_TRUE( bridge.FileExists( "cached.bin" ) );
            ASSERT_TRUE( 1000000 == bridge.GetFileSize( "cached.bin" ) );
            ASSERT_TRUE( cachedTime == bridge.GetFileModificationTime( "cached.bin" ) );
            CORE::CResourceMetaData metaData;
            ASSERT_TRUE( bridge.GetFileMetaData( "cached.bin", metaData ) );
            ASSERT_TRUE( metaData.hasIsOffline && metaData.isOffline && 1000000 == metaData.resourceSizeInBytes );
            names.clear();
            bridge.GetFileList( names, CORE::CString::Empty, "dir", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            ASSERT_TRUE( 1 == names.size() && "inside.bin" == names[ 0 ] );

            // what the cache does not hold fails, and the reason is that the share cannot be reached
            ASSERT_FALSE( bridge.FileExists( "uncached.bin" ) );
            ASSERT_TRUE( bridge.GetLastResult().IsUnreachable() );
            ASSERT_TRUE( bridge.GetLastResult().message.HasSubstr( "cannot be reached" ) >= 0 );
            ASSERT_TRUE( 0 == bridge.GetFileSize( "uncached.bin" ) );
            ASSERT_TRUE( DRN::DRN_NSRESULT_UNAVAILABLE == bridge.GetLastResult().code );
            VFS::TBasicVfsResourcePtr none = bridge.GetFile( "uncached.bin", "rb", 0, false );
            ASSERT_TRUE( none.IsNULL() );
            ASSERT_TRUE( bridge.GetLastResult().IsUnreachable() );
            names.clear();
            bridge.GetFileList( names, CORE::CString::Empty, "dir2", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            ASSERT_TRUE( names.empty() );
            ASSERT_TRUE( bridge.GetLastResult().IsUnreachable() );

            // a file that is open reads what its chunk cache has and then fails, without hanging
            CORE::CDynamicBuffer buffer( 100, true );
            ASSERT_TRUE( ReadSeeded( *open->GetAccess(), 0, 100, 31 ) );
            ASSERT_TRUE( 0 == open->GetAccess()->Setpos( 900000 ) );
            ASSERT_TRUE( 0 == open->GetAccess()->Read( buffer.GetBufferPtr(), 1, 100 ) );

            // the link is back: the mount carries on, what is expired is checked with its revision, what was not known is asked
            REQUIRE_TRUE( bed.share->RejoinClient() );
            ASSERT_TRUE( bridge.IsConnected() );
            const UInt64 statsBefore = bed.connection->GetStatCallCount();
            ASSERT_TRUE( bridge.FileExists( "cached.bin" ) );
            ASSERT_TRUE( bed.connection->GetStatCallCount() == statsBefore + 1 );
            ASSERT_TRUE( bridge.GetLastResult().IsOk() );
            ASSERT_TRUE( 1000 == bridge.GetFileSize( "uncached.bin" ) );
            names.clear();
            bridge.GetFileList( names, CORE::CString::Empty, "dir2", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            ASSERT_TRUE( 1 == names.size() );
            ASSERT_TRUE( bridge.GetFileMetaData( "cached.bin", metaData ) );
            ASSERT_TRUE( metaData.hasIsOffline && !metaData.isOffline );

            // files that are opened now read; the one that was open before the link was lost lost its handle with the link: it opens again, same revision
            VFS::TBasicVfsResourcePtr fresh = bridge.GetFile( "uncached.bin", "rb", 0, false );
            REQUIRE_TRUE( !fresh.IsNULL() );
            ASSERT_TRUE( ReadAllSeeded( *fresh->GetAccess(), 1000, 32 ) );
            ASSERT_TRUE( ReadSeeded( *open->GetAccess(), 900000, 100, 31 ) );
            ASSERT_TRUE( open->GetAccess()->IsValid() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestNodeThread( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Threads: a blocking call on the thread that works on the node is refused with a clear result, without waiting" )
        CBed bed( "nodethread" );
        CNodeThreadPolicy policy;
        try
        {
            bed.MakeRoot();
            REQUIRE_TRUE( WriteSeeded( bed.Path( "a.bin" ), 100, 41 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "mirror.bin" ), 100, 42 ) );
            CBridgeMountOptions options;
            REQUIRE_TRUE( bed.Start( options ) );

            // the other way round as well: the client node serves a share, an archive on the serving node reads it through its own connection
            CORE::CString clientRoot = BedDirectory( "nodethread_mirror" );
            REQUIRE_TRUE( WriteSeeded( CORE::CombinePath( clientRoot, "mirror.bin" ), 100, 42 ) );
            DRN::CIDRNResourceProviderPtr provider( GUCEF_NEW DRN::CDRNFileSystemAccessResourceProvider( NativeFileSystem(), clientRoot, CORE::CUri( "file://" + clientRoot ) ) );
            REQUIRE_TRUE( bed.share->Client().GetResources().RegisterProvider( "mirror", provider, DRN::CDRNResourceShareRules( DRN::CDRNResourceShareRules::WholeProvider, DRN::CDRNResourceShareRules::AnyRole ) ) );

            CDrnBridgeConnectionPtr back( GUCEF_NEW CDrnBridgeConnection( bed.share->Server(), bed.share->GetClientPeerId() ) );
            CDrnBridgeArchive mirror;
            REQUIRE_TRUE( mirror.LoadArchive( back, "mirror", CDrnBridgeSettings(), "Mirror" ) );

            // on a thread that does not work on the node the mirror works
            ASSERT_FALSE( back->IsOnNodeThread() );
            ASSERT_TRUE( mirror.FileExists( "mirror.bin" ) );

            // the policy of the serving node runs on its thread when the client browses: from there the mirror archive is called
            policy.archive = &mirror;
            bed.share->Server().GetNode().SetAccessPolicy( &policy );
            ASSERT_TRUE( 100 == bed.archive->GetFileSize( "a.bin" ) );

            const UInt64 start = DRN::CDRNClock::NowInMilliseconds();
            while ( !policy.called && DRN::CDRNClock::NowInMilliseconds() - start < 5000 )
                MT::ThreadDelay( 5 );
            bed.share->Server().GetNode().SetAccessPolicy( GUCEF_NULL );

            ASSERT_TRUE( policy.called );
            ASSERT_TRUE( policy.onNodeThread );
            ASSERT_FALSE( policy.existsAnswer );
            ASSERT_TRUE( DRN::DRN_NSRESULT_WRONG_THREAD == policy.existsResult.code );
            ASSERT_TRUE( policy.existsResult.IsUnreachable() );
            ASSERT_TRUE( policy.openAnswer.IsNULL() );
            ASSERT_TRUE( DRN::DRN_NSRESULT_WRONG_THREAD == policy.openResult.code );
            ASSERT_TRUE( policy.existsResult.message.HasSubstr( "another thread" ) >= 0 );

            // and the mirror carries on working on the other threads
            ASSERT_TRUE( mirror.FileExists( "mirror.bin" ) );
            CORE::DeleteDir( clientRoot, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.share->Server().GetNode().SetAccessPolicy( GUCEF_NULL );
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestPumpedClient( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Pumped client: the archive of a client node without a thread of its own, called by the thread that pumps it, stats, lists, reads, fails a changed file, survives a link loss and reconnects" )
        CBed bed( "pumped" );
        try
        {
            bed.MakeRoot();
            const UInt64 size = 1536 * 1024;
            REQUIRE_TRUE( WriteSeeded( bed.Path( "big.bin" ), size, 51 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "grow.bin" ), 2 * 1024 * 1024, 52 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "dir/inside.bin" ), 500, 53 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "dir/other.bin" ), 600, 54 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "dir2/more.bin" ), 700, 55 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "uncached.bin" ), 1000, 56 ) );

            CBridgeMountOptions options;
            options.clientOwnThread = false;
            options.chunkCacheChunks = 2;
            options.readAheadChunks = 0;
            options.metadataTtlMs = 300;
            options.negativeTtlMs = 300;
            REQUIRE_TRUE( bed.Start( options ) );
            CDrnBridgeArchive& bridge = bed.Bridge();
            DRN::CDRNNode& clientNode = bed.share->Client().GetNode();
            REQUIRE_TRUE( !clientNode.IsRunningOwnThread() && clientNode.IsPumpThread() );
            ASSERT_TRUE( !bed.share->Server().GetNode().IsPumpThread() );
            const UInt64 started = DRN::CDRNClock::NowInMilliseconds();

            // stat and list
            ASSERT_TRUE( bridge.FileExists( "big.bin" ) );
            ASSERT_TRUE( size == bridge.GetFileSize( "big.bin" ) );
            ASSERT_FALSE( bridge.FileExists( "nothere.bin" ) );
            ASSERT_TRUE( bridge.GetLastResult().IsOk() || DRN::DRN_NSRESULT_NOT_FOUND == bridge.GetLastResult().code );
            const CORE::CDateTime bigTime = bridge.GetFileModificationTime( "big.bin" );
            CORE::CString::StringVector names;
            bridge.GetFileList( names, CORE::CString::Empty, "dir", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            ASSERT_TRUE( 2 == names.size() && Contains( names, "inside.bin" ) && Contains( names, "other.bin" ) );
            ASSERT_TRUE( bed.connection->GetStatCallCount() + bed.connection->GetListCallCount() > 0 );

            // read: a file of several chunks, whole and at its ends, and an open that waits for the serving side
            VFS::TBasicVfsResourcePtr big = bridge.GetFile( "big.bin", "rb", 0, false );
            REQUIRE_TRUE( !big.IsNULL() );
            ASSERT_TRUE( ReadAllSeeded( *big->GetAccess(), size, 51 ) );
            ASSERT_TRUE( ReadSeeded( *big->GetAccess(), size - 4096, 4096, 51 ) );
            ASSERT_TRUE( ReadSeeded( *big->GetAccess(), 700000, 5000, 51 ) );

            // a changed file: the file that is open fails from the next read on, opening it again gives the new revision
            VFS::TBasicVfsResourcePtr grow = bridge.GetFile( "grow.bin", "rb", 0, false );
            REQUIRE_TRUE( !grow.IsNULL() );
            ASSERT_TRUE( ReadSeeded( *grow->GetAccess(), 0, 100, 52 ) );
            MT::ThreadDelay( 40 );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "grow.bin" ), 2 * 1024 * 1024, 99 ) );
            CORE::CDynamicBuffer buffer( 100, true );
            ASSERT_TRUE( 0 == grow->GetAccess()->Setpos( 1024 * 1024 ) );
            ASSERT_TRUE( 0 == grow->GetAccess()->Read( buffer.GetBufferPtr(), 1, 100 ) );
            ASSERT_FALSE( grow->GetAccess()->IsValid() );
            ASSERT_TRUE( DRN::DRN_NSRESULT_CHANGED == bridge.GetLastResult().code );
            VFS::TBasicVfsResourcePtr again = bridge.GetFile( "grow.bin", "rb", 0, false );
            REQUIRE_TRUE( !again.IsNULL() );
            ASSERT_TRUE( ReadAllSeeded( *again->GetAccess(), 2 * 1024 * 1024, 99 ) );

            // link loss: what the cache holds answers, the rest says the share cannot be reached, nothing waits for a timeout
            ASSERT_TRUE( bridge.IsConnected() );
            ASSERT_TRUE( 1000 == bridge.GetFileSize( "uncached.bin" ) );
            bed.share->DropClient();
            const UInt64 lossStart = DRN::CDRNClock::NowInMilliseconds();
            ASSERT_FALSE( bridge.IsConnected() );
            ASSERT_TRUE( bridge.IsHealthy() );
            ASSERT_FALSE( bridge.FileExists( "dir2/more.bin" ) );
            ASSERT_TRUE( bridge.GetLastResult().IsUnreachable() );
            VFS::TBasicVfsResourcePtr none = bridge.GetFile( "dir2/more.bin", "rb", 0, false );
            ASSERT_TRUE( none.IsNULL() );
            ASSERT_TRUE( DRN::CDRNClock::NowInMilliseconds() - lossStart < 3000 );

            // the link is back: the archive asks again, the file that was open reopens at its revision
            REQUIRE_TRUE( bed.share->RejoinClient() );
            ASSERT_TRUE( bridge.IsConnected() );
            ASSERT_TRUE( 700 == bridge.GetFileSize( "dir2/more.bin" ) );
            ASSERT_TRUE( bigTime == bridge.GetFileModificationTime( "big.bin" ) );
            ASSERT_TRUE( ReadSeeded( *big->GetAccess(), 900000, 100, 51 ) );
            ASSERT_TRUE( big->GetAccess()->IsValid() );

            // no call of the whole case waited for a timeout of half a minute
            ASSERT_TRUE( DRN::CDRNClock::NowInMilliseconds() - started < 20000 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestHandlePool( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Handle pool: many files open at once share a few handles, the least recently used is closed for the next, and every file reads right" )
        CBed bed( "pool" );
        try
        {
            bed.MakeRoot();
            const UInt32 fileCount = 24;
            const UInt64 size = 150000;
            for ( UInt32 i=0; i<fileCount; ++i )
                REQUIRE_TRUE( WriteSeeded( bed.Path( "p" + CORE::UInt32ToString( i ) + ".bin" ), size, (UInt8) ( 50 + i ) ) );

            CBridgeMountOptions options;
            options.maxOpenHandles = 4;
            REQUIRE_TRUE( bed.Start( options ) );
            CDrnBridgeArchive& bridge = bed.Bridge();
            const UInt32 clientPeer = bed.share->GetClientPeerId();

            // all of them open at once: more files than handles
            GUCEF::vector< VFS::TBasicVfsResourcePtr > files;
            for ( UInt32 i=0; i<fileCount; ++i )
            {
                files.push_back( bridge.GetFile( "p" + CORE::UInt32ToString( i ) + ".bin", "rb", 0, false ) );
                REQUIRE_TRUE( !files.back().IsNULL() );
                ASSERT_TRUE( bridge.GetOpenHandleCount() <= 4 );
            }
            ASSERT_TRUE( bridge.GetPeakOpenHandleCount() <= 4 );
            ASSERT_TRUE( bridge.GetHandleEvictionCount() >= fileCount - 4 );

            // each read in turn, twice around: every file needs a handle that another one took
            for ( UInt32 round=0; round<2; ++round )
            {
                for ( UInt32 i=0; i<fileCount; ++i )
                {
                    ASSERT_TRUE( ReadSeeded( *files[ i ]->GetAccess(), round * 70000 + i * 100, 5000, (UInt8) ( 50 + i ) ) );
                    ASSERT_TRUE( bridge.GetOpenHandleCount() <= 4 );
                }
            }
            ASSERT_TRUE( bridge.GetHandleReopenCount() >= fileCount );
            ASSERT_TRUE( bridge.GetPeakOpenHandleCount() <= 4 );

            // the serving side has no more of them open than the archive says
            MT::ThreadDelay( 200 );
            ASSERT_TRUE( bed.share->Server().GetResources().GetOpenHandleCount( clientPeer ) <= 4 );
            ASSERT_TRUE( bed.share->Client().GetResources().GetClientHandleCount() <= 4 );

            // all closed: no handle is held for a file that is closed
            files.clear();
            MT::ThreadDelay( 200 );
            ASSERT_TRUE( 0 == bridge.GetOpenHandleCount() );
            ASSERT_TRUE( 0 == bed.share->Server().GetResources().GetOpenHandleCount( clientPeer ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Handle pool: threads that all need the one handle there is take turns, and every read is right" )
        CBed bed( "poolwait" );
        try
        {
            bed.MakeRoot();
            const UInt64 size = 400000;
            REQUIRE_TRUE( WriteSeeded( bed.Path( "x.bin" ), size, 71 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "y.bin" ), size, 72 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "z.bin" ), size, 73 ) );

            CBridgeMountOptions options;
            options.maxOpenHandles = 1;
            options.chunkCacheChunks = 1;
            options.readAheadChunks = 0;
            REQUIRE_TRUE( bed.Start( options ) );

            CPingPongReader x( bed.Bridge(), "x.bin", size, 71, 150 );
            CPingPongReader y( bed.Bridge(), "y.bin", size, 72, 150 );
            CPingPongReader z( bed.Bridge(), "z.bin", size, 73, 150 );
            REQUIRE_TRUE( x.Activate( GUCEF_NULL, 1, 1 ) && y.Activate( GUCEF_NULL, 1, 1 ) && z.Activate( GUCEF_NULL, 1, 1 ) );
            const UInt64 start = DRN::CDRNClock::NowInMilliseconds();
            while ( ( !x.IsDone() || !y.IsDone() || !z.IsDone() ) && DRN::CDRNClock::NowInMilliseconds() - start < 120000 )
                MT::ThreadDelay( 10 );
            ASSERT_TRUE( x.IsDone() && y.IsDone() && z.IsDone() );
            ASSERT_TRUE( 0 == x.GetFailures() && 0 == y.GetFailures() && 0 == z.GetFailures() );
            ASSERT_TRUE( bed.Bridge().GetPeakOpenHandleCount() <= 1 );
            ASSERT_TRUE( bed.Bridge().GetHandleEvictionCount() > 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestListingCost( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Listing cost: a large directory costs a request per page, a repeat within the time costs none, and after it one small revalidation" )
        CBed bed( "listcost" );
        try
        {
            bed.MakeRoot();
            const UInt32 count = 5000;
            REQUIRE_TRUE( CORE::CreateDirs( bed.Path( "big" ) ) );
            for ( UInt32 i=0; i<count; ++i )
            {
                CORE::CString number = CORE::UInt32ToString( i );
                while ( number.Length() < 5 )
                    number = "0" + number;
                REQUIRE_TRUE( CORE::WriteStringAsTextFile( bed.Path( "big/f" + number + ".dat" ), "x", false ) );
            }

            CBridgeMountOptions options;
            options.metadataTtlMs = 700;
            options.negativeTtlMs = 700;
            REQUIRE_TRUE( bed.Start( options ) );
            CDrnBridgeArchive& bridge = bed.Bridge();

            // the first call of a connection also makes the hello: leave that out of the count
            ASSERT_TRUE( bridge.DirExists( "big" ) );
            UInt64 served = bed.Served();
            UInt64 lists = bed.connection->GetListCallCount();

            CORE::CString::StringVector names;
            bridge.GetFileList( names, CORE::CString::Empty, "big", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            ASSERT_TRUE( count == names.size() );
            ASSERT_TRUE( bed.Served() - served == count / 1000 );
            ASSERT_TRUE( bed.connection->GetListCallCount() - lists == 1 );

            // within the time: no request at all, for the list again and for what the list holds
            served = bed.Served();
            names.clear();
            bridge.GetFileList( names, CORE::CString::Empty, "big", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            ASSERT_TRUE( count == names.size() );
            for ( UInt32 i=0; i<count; i+=50 )
            {
                CORE::CString number = CORE::UInt32ToString( i );
                while ( number.Length() < 5 )
                    number = "0" + number;
                ASSERT_TRUE( bridge.FileExists( "big/f" + number + ".dat" ) );
                ASSERT_TRUE( 1 == bridge.GetFileSize( "big/f" + number + ".dat" ) );
            }
            ASSERT_TRUE( bed.Served() == served );

            // after the time, a directory that did not change: one request, which answers unchanged and carries no entries
            MT::ThreadDelay( 900 );
            served = bed.Served();
            names.clear();
            bridge.GetFileList( names, CORE::CString::Empty, "big", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            ASSERT_TRUE( count == names.size() );
            ASSERT_TRUE( bed.Served() - served == 1 );
            // and what it says about a file is checked on its own, with the revision it has: one more, and then it is current
            served = bed.Served();
            ASSERT_TRUE( bridge.FileExists( "big/f00100.dat" ) );
            ASSERT_TRUE( bed.Served() - served == 1 );
            ASSERT_TRUE( bridge.FileExists( "big/f00100.dat" ) && 1 == bridge.GetFileSize( "big/f00100.dat" ) );
            ASSERT_TRUE( bed.Served() - served == 1 );

            // a directory that changed: the revalidation brings the new list, page by page
            MT::ThreadDelay( 900 );
            MT::ThreadDelay( 20 );
            REQUIRE_TRUE( CORE::WriteStringAsTextFile( bed.Path( "big/f99999.dat" ), "x", false ) );
            served = bed.Served();
            names.clear();
            bridge.GetFileList( names, CORE::CString::Empty, "big", false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            ASSERT_TRUE( count + 1 == names.size() && Contains( names, "f99999.dat" ) );
            ASSERT_TRUE( bed.Served() - served == ( count + 1 + 999 ) / 1000 );
            ASSERT_TRUE( bridge.GetLastResult().IsOk() );

            // no entry costs a request of its own: a recursive list of the whole share is one list per directory
            served = bed.Served();
            names.clear();
            bridge.GetFileList( names, CORE::CString::Empty, CORE::CString::Empty, true, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
            ASSERT_TRUE( count + 1 == names.size() );
            ASSERT_TRUE( bed.Served() - served <= 2 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestThroughTheVfs( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "VFS: mounted with CVFS::MountArchive the share reads through vfs paths, the vfs URI scheme and the VFS as a file system" )
        CBed bed( "vfs" );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        bool mounted = false;
        try
        {
            bed.MakeRoot();
            REQUIRE_TRUE( WriteSeeded( bed.Path( "a.bin" ), 3000, 61 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "sub/b.bin" ), 130000, 62 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "sub/deeper/c.txt" ), 10, 63 ) );
            CBridgeMountOptions options;
            REQUIRE_TRUE( bed.Start( options ) );

            // mounted as an application's configuration would: type, source and mount path
            const VFS::CArchiveSettings settings = MakeBridgeArchiveSettings( "DrnVfsMount", bed.connectionId + "|" + CLoopbackShare::ShareName, "drnremote", options );
            mounted = vfs.MountArchive( settings );
            REQUIRE_TRUE( mounted );

            // vfs paths
            ASSERT_TRUE( vfs.FileExists( "drnremote/a.bin" ) );
            ASSERT_TRUE( 3000 == vfs.GetFileSize( "drnremote/a.bin" ) );
            ASSERT_FALSE( vfs.FileExists( "drnremote/none.bin" ) );
            VFS::TBasicVfsResourcePtr resource = vfs.GetFile( "drnremote/sub/b.bin" );
            REQUIRE_TRUE( !resource.IsNULL() );
            ASSERT_TRUE( ReadAllSeeded( *resource->GetAccess(), 130000, 62 ) );
            resource.Unlink();
            CORE::CString::StringVector names;
            ASSERT_TRUE( vfs.GetFileList( names, CORE::CString( "drnremote/sub" ), false, false, CORE::CString::Empty ) );
            ASSERT_TRUE( 1 == names.size() && "b.bin" == names[ 0 ] );
            names.clear();
            ASSERT_TRUE( vfs.GetFileList( names, CORE::CString( "drnremote" ), true, true, CORE::CString::Empty ) );
            ASSERT_TRUE( 3 == names.size() );
            bool foundDeep = false;
            for ( UInt32 i=0; i<names.size(); ++i )
                foundDeep = foundDeep || names[ i ].HasSubstr( "deeper/c.txt" ) >= 0;
            ASSERT_TRUE( foundDeep );
            names.clear();
            ASSERT_TRUE( vfs.GetDirList( names, CORE::CString( "drnremote" ), false, false, CORE::CString::Empty ) );
            ASSERT_TRUE( 1 == names.size() );

            // the vfs URI scheme
            {
                VFS::CVfsUriResourceAccessor accessor;
                CORE::IOAccessPtr access;
                ASSERT_TRUE( accessor.GetResourceAccess( CORE::CUri( "vfs://drnremote/sub/b.bin" ), access, CORE::CUriResourceAccessor::URI_RESOURCEACCESS_MODE_READ ) );
                REQUIRE_TRUE( !access.IsNULL() );
                ASSERT_TRUE( ReadAllSeeded( *access, 130000, 62 ) );
                CORE::CDynamicBuffer part;
                CORE::CDynamicBufferAccess destination( part );
                ASSERT_TRUE( accessor.GetPartialResource( CORE::CUri( "vfs://drnremote/a.bin" ), 100, 50, destination ) );
                ASSERT_TRUE( 50 == part.GetDataSize() && HasSeededContent( static_cast< const UInt8* >( part.GetConstBufferPtr() ), 50, 100, 61 ) );
            }

            // the VFS as a file system
            {
                CORE::TFileSystemAccessPtr access = ( GUCEF_NEW VFS::CVfsFileSystemAccess() )->CreateSharedPtr();
                CORE::CResourceMetaData metaData;
                ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetMetaData( "/drnremote/a.bin", metaData ) );
                ASSERT_TRUE( metaData.hasResourceSizeInBytes && 3000 == metaData.resourceSizeInBytes );
                CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
                ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryType( "/drnremote/sub", type ) && CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type );
                CORE::TFileSystemIteratorPtr iterator;
                ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetFileSystemIterator( "/drnremote", iterator ) );
                REQUIRE_TRUE( !iterator.IsNULL() );
                TStrings found;
                if ( iterator->FindFirst( "/drnremote" ) )
                {
                    do
                    {
                        found.push_back( iterator->GetResourceName() + ( iterator->IsADirectory() ? "/" : "" ) );
                    }
                    while ( iterator->FindNext() );
                    iterator->FindClose();
                }
                ASSERT_TRUE( 2 == found.size() && Contains( found, "a.bin" ) && Contains( found, "sub/" ) );
                // writing is refused
                ASSERT_TRUE( CORE::FILESYSTEMACCESS_SUCCESS != access->TryDeleteFile( "/drnremote/a.bin" ) );
                ASSERT_TRUE( bed.archive->IsConnected() );
            }
            ASSERT_TRUE( CORE::FileExists( bed.Path( "a.bin" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        if ( mounted )
            vfs.UnmountArchiveByName( "DrnVfsMount" );
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestRefusalsAndRules( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Read only: every call that would change the share is refused with a result that says so, and the share is not touched" )
        CBed bed( "readonly" );
        try
        {
            bed.MakeRoot();
            REQUIRE_TRUE( WriteSeeded( bed.Path( "a.bin" ), 100, 81 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "d/b.bin" ), 100, 82 ) );
            CBridgeMountOptions options;
            REQUIRE_TRUE( bed.Start( options ) );
            CDrnBridgeArchive& bridge = bed.Bridge();

            CORE::CDynamicBuffer data( 10, true );
            data.SetDataSize( 10 );
            ASSERT_FALSE( bridge.IsWriteable() );
            ASSERT_FALSE( bridge.StoreAsFile( "new.bin", data, 0, true ) );
            ASSERT_TRUE( DRN::DRN_NSRESULT_READ_ONLY == bridge.GetLastResult().code );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_READ_ONLY == bridge.GetLastResult().fileSystemResult );
            ASSERT_FALSE( bridge.DeleteFile( "a.bin" ) );
            ASSERT_FALSE( bridge.MoveFile( "a.bin", "c.bin", true ) );
            ASSERT_FALSE( bridge.MoveFileAtomically( "a.bin", "c.bin", true ) );
            ASSERT_FALSE( bridge.CreateDir( "newdir" ) );
            ASSERT_FALSE( bridge.DeleteDir( "d", true ) );
            ASSERT_FALSE( bridge.MoveDir( "d", "e" ) );
            ASSERT_TRUE( DRN::DRN_NSRESULT_READ_ONLY == bridge.GetLastResult().code );
            // opening for anything but reading gives nothing, whatever the mode says
            const char* const modes[] = { "wb", "w", "ab", "a", "r+", "rb+", "w+", "x" };
            for ( UInt32 i=0; i<sizeof( modes ) / sizeof( modes[ 0 ] ); ++i )
            {
                VFS::TBasicVfsResourcePtr resource = bridge.GetFile( "a.bin", modes[ i ], 0, true );
                ASSERT_TRUE( resource.IsNULL() );
            }
            ASSERT_TRUE( bridge.GetFile( "a.bin", GUCEF_NULL, 0, false ).IsNULL() );
            ASSERT_TRUE( bridge.GetFileAs( "a.bin", CORE::CResourceMetaData(), "wb", 0, true ).IsNULL() );
            ASSERT_FALSE( bridge.SetFileMetaData( "a.bin", CORE::CResourceMetaData() ) );
            ASSERT_FALSE( bridge.IsDirectoryWatchingSupported() );

            // a file that is open for reading takes no write
            VFS::TBasicVfsResourcePtr resource = bridge.GetFile( "a.bin", "r", 0, false );
            REQUIRE_TRUE( !resource.IsNULL() );
            ASSERT_FALSE( resource->GetAccess()->IsWriteable() );
            ASSERT_TRUE( 0 == resource->GetAccess()->Write( "xyz", 1, 3 ) );
            ASSERT_TRUE( ReadAllSeeded( *resource->GetAccess(), 100, 81 ) );

            // the share is as it was
            ASSERT_TRUE( CORE::FileExists( bed.Path( "a.bin" ) ) && 100 == CORE::FileSize( bed.Path( "a.bin" ) ) );
            ASSERT_FALSE( CORE::FileExists( bed.Path( "new.bin" ) ) );
            ASSERT_TRUE( CORE::DirExists( bed.Path( "d" ) ) && !CORE::DirExists( bed.Path( "newdir" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Confinement: paths that lead out of the share are refused by the archive, and by the serving side when they reach it" )
        CBed bed( "confine" );
        try
        {
            bed.MakeRoot();
            const CORE::CString outside = BedDirectory( "confine_outside" );
            REQUIRE_TRUE( WriteSeeded( CORE::CombinePath( outside, "secret.bin" ), 100, 91 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "in.bin" ), 100, 92 ) );
            REQUIRE_TRUE( CORE::CreateDirs( bed.Path( "sub" ) ) );
            CBridgeMountOptions options;
            REQUIRE_TRUE( bed.Start( options ) );
            CDrnBridgeArchive& bridge = bed.Bridge();

            // the root of the share here is bed.root: a sibling directory is reached by ".." from inside it
            const CORE::CString up = "../vfsdrn-bed-" + CORE::UInt32ToString( MT::GetProcessID() ) + "-confine_outside/secret.bin";
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( bed.root, up ) ) );
            const char* const paths[] = { "../x", "sub/../../x", "..", "sub/..", "./../x", "sub\\..\\..\\x", "a/../b" };
            for ( UInt32 i=0; i<sizeof( paths ) / sizeof( paths[ 0 ] ); ++i )
            {
                ASSERT_FALSE( bridge.FileExists( paths[ i ] ) );
                ASSERT_FALSE( bridge.DirExists( paths[ i ] ) );
                ASSERT_TRUE( 0 == bridge.GetFileSize( paths[ i ] ) );
                ASSERT_TRUE( bridge.GetFile( paths[ i ], "rb", 0, false ).IsNULL() );
                CORE::CString::StringVector names;
                bridge.GetFileList( names, CORE::CString::Empty, paths[ i ], true, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX );
                ASSERT_TRUE( names.empty() );
            }
            ASSERT_FALSE( bridge.FileExists( up ) );
            ASSERT_TRUE( bridge.GetFile( up, "rb", 0, false ).IsNULL() );
            ASSERT_TRUE( DRN::DRN_NSRESULT_INVALID_NAME == bridge.GetLastResult().code );
            // the archive stays usable, and a name that only has dots in it is a name
            ASSERT_TRUE( bridge.FileExists( "in.bin" ) && bridge.FileExists( "/sub/../in.bin" ) == false );

            // what reaches the serving side without the archive's check is refused there: a request made on the connection itself
            CORE::CString::StringVector requested;
            requested.push_back( up );
            requested.push_back( "sub/../in.bin" );
            requested.push_back( "in.bin" );
            requested.push_back( "/in.bin" );
            requested.push_back( "C:\\x" );
            DRN::CDRNNamespaceClient::TEntryVector answers;
            CDrnBridgeResult result = bed.connection->Stat( CLoopbackShare::ShareName, requested, CORE::CString::StringVector(), answers, 5000 );
            ASSERT_TRUE( result.IsOk() );
            REQUIRE_TRUE( 5 == answers.size() );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == answers[ 0 ].status );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == answers[ 1 ].status );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == answers[ 2 ].status );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == answers[ 3 ].status );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == answers[ 4 ].status );
            DRN::CDRNNamespaceListPage page;
            result = bed.connection->List( CLoopbackShare::ShareName, "..", CORE::CString::Empty, 0, CORE::CString::Empty, page, 5000 );
            ASSERT_FALSE( result.IsOk() );
            ASSERT_TRUE( DRN::DRN_NSRESULT_INVALID_NAME == result.code );
            DRN::CDRNResourceHandlePtr handle;
            result = bed.connection->OpenHandle( CLoopbackShare::ShareName, up, DRN::CDRNResourceTransferOptions(), 5000, handle );
            ASSERT_FALSE( result.IsOk() );
            ASSERT_TRUE( handle.IsNULL() );
            CORE::DeleteDir( outside, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
TestResultsAndSettings( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Last result: each thread has the result of its own last call, and a success clears it" )
        CBed bed( "lastresult" );
        try
        {
            bed.MakeRoot();
            REQUIRE_TRUE( WriteSeeded( bed.Path( "a.bin" ), 100, 1 ) );
            CBridgeMountOptions options;
            REQUIRE_TRUE( bed.Start( options ) );
            CDrnBridgeArchive& bridge = bed.Bridge();

            ASSERT_TRUE( bridge.GetLastResult().IsOk() );
            ASSERT_FALSE( bridge.FileExists( "none.bin" ) );
            CDrnBridgeResult failure = bridge.GetLastResult();
            ASSERT_TRUE( DRN::DRN_NSRESULT_NOT_FOUND == failure.code && CORE::FILESYSTEMACCESS_NOT_FOUND == failure.fileSystemResult );
            ASSERT_TRUE( failure.message.HasSubstr( "none.bin" ) >= 0 );

            // another thread has a result of its own
            class CReader : public MT::CActiveObject
            {
                public:
                CDrnBridgeArchive* archive; bool done; bool sawSuccess; bool sawFailure;
                CReader( CDrnBridgeArchive* a ) : archive( a ), done( false ), sawSuccess( false ), sawFailure( false ) {}
                virtual ~CReader() { Deactivate( false, true ); }
                virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }
                protected:
                virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }
                virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
                {
                    sawSuccess = archive->GetLastResult().IsOk();
                    archive->FileExists( "a.bin" );
                    sawSuccess = sawSuccess && archive->GetLastResult().IsOk();
                    archive->FileExists( "other.bin" );
                    sawFailure = !archive->GetLastResult().IsOk();
                    done = true;
                    return true;
                }
            };
            CReader reader( &bridge );
            REQUIRE_TRUE( reader.Activate( GUCEF_NULL, 1, 1 ) );
            while ( !reader.done )
                MT::ThreadDelay( 5 );
            ASSERT_TRUE( reader.sawSuccess && reader.sawFailure );
            // this thread's result is still its own
            ASSERT_TRUE( DRN::DRN_NSRESULT_NOT_FOUND == bridge.GetLastResult().code );
            ASSERT_TRUE( bridge.FileExists( "a.bin" ) );
            ASSERT_TRUE( bridge.GetLastResult().IsOk() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END
    GUCEF_TESTFW_TESTCASE( "Result mapping: remote situations map onto the remote values of the file system result, the rest keeps its local value" )
        try
        {
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == CDrnBridgeResult( DRN::DRN_NSRESULT_UNAVAILABLE, DRN::DRN_RESSTATUS_LINK_LOST, "" ).fileSystemResult );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED == CDrnBridgeResult( DRN::DRN_NSRESULT_CHANGED, DRN::DRN_RESSTATUS_CHANGED, "" ).fileSystemResult );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER == CDrnBridgeResult( DRN::DRN_NSRESULT_BUSY, DRN::DRN_RESSTATUS_BUSY, "" ).fileSystemResult );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_RESOURCE_CORRUPT == CDrnBridgeResult( DRN::DRN_NSRESULT_INTEGRITY, DRN::DRN_RESSTATUS_DIGEST_MISMATCH, "" ).fileSystemResult );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_REMOTE_PROTOCOL_ERROR == CDrnBridgeResult( DRN::DRN_NSRESULT_INTEGRITY, DRN::DRN_RESSTATUS_PROTOCOL_ERROR, "" ).fileSystemResult );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_TIMEOUT == CDrnBridgeResult( DRN::DRN_NSRESULT_TIMEOUT, DRN::DRN_RESSTATUS_TIMEOUT, "" ).fileSystemResult );
            ASSERT_TRUE( CORE::FILESYSTEMACCESS_NOT_FOUND == CDrnBridgeResult( DRN::DRN_NSRESULT_NOT_FOUND, DRN::DRN_RESSTATUS_NOT_FOUND, "" ).fileSystemResult );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END


    GUCEF_TESTFW_TESTCASE( "Settings: the archive specific settings are read from the configuration of the archive, defaults stay where nothing is said" )
        try
        {
            CDrnBridgeSettings defaults;
            ASSERT_TRUE( 3000 == defaults.metadataTtlMs && 1000 == defaults.negativeTtlMs && 12 == defaults.maxOpenHandles && defaults.serveStaleWhenOffline );
            ASSERT_TRUE( DRN::CDRNResourceTransferOptions::VERIFY_LINK == defaults.verifyMode );

            CORE::CDataNode config;
            config.SetAttribute( "archiveName", CORE::CString( "S" ) );
            config.SetAttribute( "archiveType", CORE::CString( "drn" ) );
            config.SetAttribute( "actualArchivePath", CORE::CString( "x|files" ) );
            const CORE::CString prefix = VFS::CArchiveSettings::ArchiveSpecificSettingsPrefix;
            config.SetAttribute( ( prefix + "metadataTtlMs" ).C_String(), CORE::CString( "0" ) );
            config.SetAttribute( ( prefix + "negativeTtlMs" ).C_String(), CORE::CString( "250" ) );
            config.SetAttribute( ( prefix + "maxCachedEntries" ).C_String(), CORE::CString( "77" ) );
            config.SetAttribute( ( prefix + "serveStaleWhenOffline" ).C_String(), CORE::CString( "false" ) );
            config.SetAttribute( ( prefix + "maxOpenHandles" ).C_String(), CORE::CString( "0" ) );
            config.SetAttribute( ( prefix + "chunkCacheChunks" ).C_String(), CORE::CString( "3" ) );
            config.SetAttribute( ( prefix + "verifyMode" ).C_String(), CORE::CString( "treeIfAvailable" ) );
            config.SetAttribute( ( prefix + "maxWalkedDirectories" ).C_String(), CORE::CString( "9" ) );
            VFS::CArchiveSettings archiveSettings;
            archiveSettings.LoadConfig( config );
            CDrnBridgeSettings settings;
            ASSERT_TRUE( settings.LoadFromArchiveSettings( archiveSettings ) );
            ASSERT_TRUE( 0 == settings.metadataTtlMs && 250 == settings.negativeTtlMs && 77 == settings.maxCachedEntries );
            ASSERT_FALSE( settings.serveStaleWhenOffline );
            ASSERT_TRUE( 1 == settings.maxOpenHandles );    // at least one handle is needed to read
            ASSERT_TRUE( 3 == settings.chunkCacheChunks && 9 == settings.maxWalkedDirectories );
            ASSERT_TRUE( DRN::CDRNResourceTransferOptions::VERIFY_TREE_IF_AVAILABLE == settings.verifyMode );
            ASSERT_TRUE( 4 == settings.readAheadChunks && 15000 == settings.callTimeoutMs );

            // a value that is not one is said and leaves the default
            config.SetAttribute( ( prefix + "verifyMode" ).C_String(), CORE::CString( "whatever" ) );
            VFS::CArchiveSettings other;
            other.LoadConfig( config );
            CDrnBridgeSettings kept;
            ASSERT_FALSE( kept.LoadFromArchiveSettings( other ) );
            ASSERT_TRUE( DRN::CDRNResourceTransferOptions::VERIFY_LINK == kept.verifyMode );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Verification: with the verify mode tree the bytes of a file are checked against the hash tree of the serving side" )
        CBed bed( "verify" );
        try
        {
            bed.MakeRoot();
            const UInt64 size = 400000;
            REQUIRE_TRUE( WriteSeeded( bed.Path( "t.bin" ), size, 5 ) );
            CBridgeMountOptions options;
            options.verifyMode = "tree";
            REQUIRE_TRUE( bed.Start( options ) );
            ASSERT_TRUE( DRN::CDRNResourceTransferOptions::VERIFY_TREE == bed.Bridge().GetSettings().verifyMode );
            const UInt64 verifiedBefore = bed.share->Client().GetResources().GetStats().chunksVerified;
            VFS::TBasicVfsResourcePtr resource = bed.Bridge().GetFile( "t.bin", "rb", 0, false );
            REQUIRE_TRUE( !resource.IsNULL() );
            ASSERT_TRUE( ReadAllSeeded( *resource->GetAccess(), size, 5 ) );
            ASSERT_TRUE( bed.share->Client().GetResources().GetStats().chunksVerified > verifiedBefore || bed.share->Server().GetResources().GetStats().treesBuilt > 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

void
TestMeasurements( void )
{GUCEF_TRACE;

    // not asserted: what a call costs over a loopback link of this build, for the log and the documentation
    GUCEF_TESTFW_TESTCASE( "Measurements: the cost of a stat, of opening a small file and of reading a large one is logged" )
        CBed bed( "measure" );
        try
        {
            bed.MakeRoot();
            const UInt64 size = 32 * 1024 * 1024;
            REQUIRE_TRUE( WriteSeeded( bed.Path( "large.bin" ), size, 3 ) );
            REQUIRE_TRUE( WriteSeeded( bed.Path( "small.bin" ), 100, 4 ) );
            CBridgeMountOptions options;
            options.metadataTtlMs = 0;
            REQUIRE_TRUE( bed.Start( options ) );
            CDrnBridgeArchive& bridge = bed.Bridge();
            ASSERT_TRUE( bridge.FileExists( "small.bin" ) );

            const UInt32 statCount = 300;
            UInt64 start = MT::PrecisionTickCount();
            for ( UInt32 i=0; i<statCount; ++i )
                bridge.FileExists( "small.bin" );
            const Float64 statMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );

            const UInt32 openCount = 100;
            start = MT::PrecisionTickCount();
            for ( UInt32 i=0; i<openCount; ++i )
            {
                VFS::TBasicVfsResourcePtr resource = bridge.GetFile( "small.bin", "rb", 0, false );
                char byte = 0;
                if ( resource.IsNULL() || 1 != resource->GetAccess()->Read( &byte, 1, 1 ) )
                    ERRORHERE;
            }
            const Float64 openMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );

            VFS::TBasicVfsResourcePtr large = bridge.GetFile( "large.bin", "rb", 0, false );
            REQUIRE_TRUE( !large.IsNULL() );
            CORE::CDynamicBuffer buffer( 65536, true );
            UInt64 position = 0;
            start = MT::PrecisionTickCount();
            while ( position < size )
            {
                const UInt32 got = large->GetAccess()->Read( buffer.GetBufferPtr(), 1, 65536 );
                if ( 0 == got )
                    break;
                position += got;
            }
            const Float64 readMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
            ASSERT_TRUE( position == size );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeTest: MEASURED stat " << ( statMs * 1000.0 / statCount ) << " us per call, open and read of a small file " << ( openMs * 1000.0 / openCount )
                << " us, sequential read " << ( ( size / 1048576.0 ) / ( readMs / 1000.0 ) ) << " MiB/s (" << size << " bytes in " << readMs << " ms)" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        bed.Stop();
    GUCEF_TESTFW_TESTCASE_END
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

void
PerformDrnBridgeBehaviourTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnBridge: behaviour" );

    TestMounting();
    TestChangedFile();
    TestLinkLoss();
    TestNodeThread();
    TestPumpedClient();
    TestHandlePool();
    TestListingCost();
    TestThroughTheVfs();
    TestRefusalsAndRules();
    TestResultsAndSettings();
    TestMeasurements();
}

/*-------------------------------------------------------------------------*/
