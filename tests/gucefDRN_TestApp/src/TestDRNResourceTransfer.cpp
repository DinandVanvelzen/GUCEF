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

#include <exception>

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
#include <unistd.h>
#endif

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDRNTestUtils.h"
#include "TestDRNGroup.h"
#include "TestDRNResourceTransfer.h"

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

static const UInt32 ChunkSize = DRN::DRN_RESOURCE_CHUNK_SIZE;
static const UInt32 BlockSize = 1024 * 1024;

/*-------------------------------------------------------------------------*/

UInt64
NowMs( void )
{
    return DRN::CDRNClock::NowInMilliseconds();
}

/*-------------------------------------------------------------------------*/

/** The content of every test resource: a function of the offset and a seed, so nothing has to be held in memory */
inline UInt64
SynthWord( UInt64 wordIndex, UInt32 seed )
{
    UInt64 x = wordIndex * (UInt64) 0x9E3779B97F4A7C15ULL + (UInt64) seed * (UInt64) 0xD6E8FEB86659FD93ULL + 1;
    x ^= x >> 31;
    x *= (UInt64) 0xBF58476D1CE4E5B9ULL;
    x ^= x >> 29;
    return x;
}

/*-------------------------------------------------------------------------*/

void
SynthFill( UInt64 offset, UInt32 size, UInt32 seed, UInt8* dest )
{
    for ( UInt32 i=0; i<size; ++i )
    {
        UInt64 at = offset + i;
        dest[ i ] = (UInt8) ( SynthWord( at >> 3, seed ) >> ( 8 * ( at & 7 ) ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
BufferMatchesSynth( const void* data, UInt32 size, UInt64 offset, UInt32 seed )
{
    const UInt8* bytes = static_cast< const UInt8* >( data );
    for ( UInt32 i=0; i<size; ++i )
    {
        UInt64 at = offset + i;
        if ( bytes[ i ] != (UInt8) ( SynthWord( at >> 3, seed ) >> ( 8 * ( at & 7 ) ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
WriteSynthFile( const CORE::CString& path, UInt64 size, UInt32 seed )
{
    CORE::CFileAccess file;
    if ( !file.Open( path, "wb" ) )
        return false;
    CORE::CDynamicBuffer block( BlockSize, true );
    for ( UInt64 offset=0; offset<size; )
    {
        UInt32 n = size - offset < BlockSize ? (UInt32) ( size - offset ) : BlockSize;
        SynthFill( offset, n, seed, static_cast< UInt8* >( block.GetBufferPtr() ) );
        if ( n != file.Write( block.GetConstBufferPtr(), 1, n ) )
            return false;
        offset += n;
    }
    file.Close();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
FileMatchesSynth( const CORE::CString& path, UInt64 size, UInt32 seed )
{
    if ( !CORE::FileExists( path ) || CORE::FileSize( path ) != size )
        return false;
    CORE::CFileAccess file;
    if ( !file.Open( path, "rb" ) )
        return false;
    CORE::CDynamicBuffer block( BlockSize, true );
    for ( UInt64 offset=0; offset<size; )
    {
        UInt32 n = size - offset < BlockSize ? (UInt32) ( size - offset ) : BlockSize;
        if ( n != file.Read( block.GetBufferPtr(), 1, n ) || !BufferMatchesSynth( block.GetConstBufferPtr(), n, offset, seed ) )
            return false;
        offset += n;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
TestDirectory( const CORE::CString& name )
{
    CORE::CString dir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "drnres_test" );
    dir = CORE::CombinePath( dir, name );
    CORE::CreateDirs( dir );
    return dir;
}

/*-------------------------------------------------------------------------*/

void
RemoveTestDirectories( void )
{
    CORE::CString dir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "drnres_test" );
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

class CTransferFinished : public CTestCondition
{
    public:
    DRN::CDRNResourceTransfer* m_transfer;
    explicit CTransferFinished( DRN::CDRNResourceTransfer* transfer ) : m_transfer( transfer ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_transfer->IsFinished(); }
};

bool
WaitFinished( DRN::CDRNResourceTransfer* transfer, UInt32 timeoutMs = 60000 )
{
    CTransferFinished condition( transfer );
    return PumpUntil( condition, timeoutMs );
}

/*-------------------------------------------------------------------------*/

class CBytesDone : public CTestCondition
{
    public:
    DRN::CDRNResourceTransfer* m_transfer;
    UInt64 m_bytes;
    CBytesDone( DRN::CDRNResourceTransfer* transfer, UInt64 bytes ) : m_transfer( transfer ), m_bytes( bytes ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_transfer->GetBytesDone() >= m_bytes || m_transfer->IsFinished(); }
};

/*-------------------------------------------------------------------------*/

/** Counts the events of a transfer */
class CTransferWatcher : public CORE::CObserver
{
    public:

    UInt32 opened;
    UInt32 progress;
    UInt32 completed;
    UInt32 failed;
    DRN::TDRNResourceStatus lastStatus;

    CTransferWatcher( void ) : CORE::CObserver(), opened( 0 ), progress( 0 ), completed( 0 ), failed( 0 ), lastStatus( DRN::DRN_RESSTATUS_OK ) {}
    virtual ~CTransferWatcher() { SignalUpcomingObserverDestruction(); }

    protected:

    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( DRN::CDRNResourceTransfer::OpenedEvent == eventid )
            ++opened;
        else if ( DRN::CDRNResourceTransfer::ProgressEvent == eventid )
            ++progress;
        else if ( DRN::CDRNResourceTransfer::CompletedEvent == eventid )
            ++completed;
        else if ( DRN::CDRNResourceTransfer::FailedEvent == eventid )
        {
            ++failed;
            if ( GUCEF_NULL != eventdata )
                lastStatus = static_cast< DRN::CDRNResourceTransferResultEventData* >( eventdata )->GetData().status;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** A resource of any size whose bytes are made as they are read: offsets beyond 4 GiB without disk space */
class CSynthProvider : public DRN::CIDRNResourceProvider
{
    public:

    UInt64 m_size;
    UInt32 m_seed;

    CSynthProvider( UInt64 size, UInt32 seed ) : DRN::CIDRNResourceProvider(), m_size( size ), m_seed( seed ) {}

    virtual DRN::TDRNResourceStatus Resolve( const CORE::CString& name, DRN::CDRNResourceInfo& info ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( name != "huge.bin" )
            return DRN::DRN_RESSTATUS_NOT_FOUND;
        info.name = name;
        info.size = m_size;
        info.modifiedUtcMs = 1;
        info.revision = DRN::CDRNResourceInfo::MakeRevision( m_size, 1 );
        return DRN::DRN_RESSTATUS_OK;
    }

    virtual DRN::TDRNResourceStatus Read( const CORE::CString& /* name */, const DRN::CDRNResourceInfo& /* resolved */, UInt64 offset, UInt32 size, void* destination, UInt32& bytesRead ) GUCEF_VIRTUAL_OVERRIDE
    {
        bytesRead = 0;
        if ( offset > m_size )
            return DRN::DRN_RESSTATUS_OUT_OF_RANGE;
        UInt32 n = m_size - offset < size ? (UInt32) ( m_size - offset ) : size;
        SynthFill( offset, n, m_seed, static_cast< UInt8* >( destination ) );
        bytesRead = n;
        return DRN::DRN_RESSTATUS_OK;
    }

    virtual CORE::CString GetDescription( void ) const GUCEF_VIRTUAL_OVERRIDE { return "synthetic"; }
};

/*-------------------------------------------------------------------------*/

/** Files of a directory, one chunk of which comes out with a flipped byte for a number of reads; keeps the right tree */
class CCorruptingProvider : public DRN::CDRNFileSystemResourceProvider
{
    public:

    UInt64 m_badChunk;
    UInt32 m_corruptionsLeft;
    UInt32 m_corruptions;
    MT::CMutex m_lock;

    CCorruptingProvider( const CORE::CString& root, UInt64 badChunk, UInt32 corruptions )
        : DRN::CDRNFileSystemResourceProvider( root ), m_badChunk( badChunk ), m_corruptionsLeft( corruptions ), m_corruptions( 0 ), m_lock() {}

    virtual DRN::TDRNResourceStatus Read( const CORE::CString& name, const DRN::CDRNResourceInfo& resolved, UInt64 offset, UInt32 size, void* destination, UInt32& bytesRead ) GUCEF_VIRTUAL_OVERRIDE
    {
        DRN::TDRNResourceStatus status = DRN::CDRNFileSystemResourceProvider::Read( name, resolved, offset, size, destination, bytesRead );
        UInt64 badOffset = m_badChunk * ChunkSize + 100;
        if ( DRN::DRN_RESSTATUS_OK == status && badOffset >= offset && badOffset < offset + bytesRead )
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_corruptionsLeft > 0 )
            {
                --m_corruptionsLeft;
                ++m_corruptions;
                static_cast< UInt8* >( destination )[ badOffset - offset ] ^= 0x5A;
            }
        }
        return status;
    }

    /** The tree of the true content, so that the tree does not come from a corrupted read */
    virtual bool GetHashTree( const CORE::CString& name, const DRN::CDRNResourceInfo& resolved, DRN::CDRNResourceHashTree& tree ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CDynamicBuffer content;
        if ( !content.LoadContentFromFile( GetPathOf( name ) ) || content.GetDataSize() != resolved.size )
            return false;
        DRN::CDRNResourceHasher hasher;
        if ( !tree.Reset( resolved.size ) )
            return false;
        for ( UInt64 i=0; i<tree.GetLeafCount(); ++i )
        {
            UInt8 leaf[ 32 ];
            hasher.HashLeaf( static_cast< const UInt8* >( content.GetConstBufferPtr() ) + i * ChunkSize, resolved.GetChunkBytes( i ), leaf );
            tree.SetLeafHash( i, leaf );
        }
        return tree.Finish( hasher );
    }
};

/*-------------------------------------------------------------------------*/

class CNoResourcesPolicy : public DRN::CDRNDefaultAccessPolicy
{
    public:

    virtual bool MayReadResourceAs( UInt32 /* peerId */, const DRN::CDRNAuthResult& /* auth */, const CORE::CString& providerName, const CORE::CString& resourceName,
                                    bool sharedByProvider ) GUCEF_VIRTUAL_OVERRIDE
    {
        // a provider the application shares by policy alone, and a name it keeps back whatever the provider says
        if ( providerName == "lobby" )
            return true;
        return sharedByProvider && resourceName != "secret.bin";
    }
};

/*-------------------------------------------------------------------------*/

/** The tree root after RFC 9162 section 2.1.1, recursive and with the built in SHA-256: independent of the class tested */
void
ReferenceRoot( const GUCEF::vector< CORE::CDynamicBuffer >& leaves, UInt32 from, UInt32 to, UInt8* out )
{
    UInt32 n = to - from;
    if ( 0 == n )
    {
        CORE::CSha256::Hash( GUCEF_NULL, 0, out );
        return;
    }
    if ( 1 == n )
    {
        memcpy( out, leaves[ from ].GetConstBufferPtr(), 32 );
        return;
    }
    UInt32 k = 1;
    while ( k * 2 < n )
        k *= 2;
    UInt8 node[ 65 ];
    node[ 0 ] = 0x01;
    ReferenceRoot( leaves, from, from + k, node + 1 );
    ReferenceRoot( leaves, from + k, to, node + 33 );
    CORE::CSha256::Hash( node, sizeof( node ), out );
}

/*-------------------------------------------------------------------------*/

/** A session of two nodes on tcp: member 0 the host that serves, member 1 the requester */
bool
StartPair( CGroup& group )
{
    return group.Start( 2 );
}

/*-------------------------------------------------------------------------*/

/** Every name of the provider, to every member */
DRN::CDRNResourceShareRules
ShareAll( void )
{
    return DRN::CDRNResourceShareRules( DRN::CDRNResourceShareRules::WholeProvider, DRN::CDRNResourceShareRules::AnyRole );
}

/*-------------------------------------------------------------------------*/

DRN::CIDRNResourceProviderPtr
FileProvider( const CORE::CString& root, bool followSymlinks = false )
{
    return DRN::CIDRNResourceProviderPtr( GUCEF_NEW DRN::CDRNFileSystemResourceProvider( root, followSymlinks ) );
}

/*-------------------------------------------------------------------------*/

DRN::CIDRNResourceSinkPtr
FileSink( const CORE::CString& path )
{
    return DRN::CIDRNResourceSinkPtr( GUCEF_NEW DRN::CDRNFileResourceSink( path ) );
}

/*-------------------------------------------------------------------------*/

/** Fetches a resource of the host into a file and waits; the status */
DRN::TDRNResourceStatus
FetchFile( CGroup& group, const CORE::CString& provider, const CORE::CString& name, const CORE::CString& target,
           const DRN::CDRNResourceTransferOptions& options, UInt32 timeoutMs = 60000 )
{
    DRN::CDRNResourceSessionComponent& requester = group[ 1 ].GetResources();
    DRN::CDRNResourceTransfer* transfer = requester.Fetch( LocalPeerOf( group[ 0 ] ), provider, name, FileSink( target ), options );
    if ( !WaitFinished( transfer, timeoutMs ) )
        return DRN::DRN_RESSTATUS_TIMEOUT;
    DRN::TDRNResourceStatus status = transfer->GetResult().status;
    requester.Release( transfer );
    return status;
}

/*-------------------------------------------------------------------------*/

/** Reads a window of a resource of the host into memory and waits */
DRN::TDRNResourceStatus
ReadWindow( CGroup& group, const CORE::CString& provider, const CORE::CString& name, UInt64 offset, UInt64 length,
            CORE::CDynamicBuffer& data, const DRN::CDRNResourceTransferOptions& options )
{
    DRN::CDRNResourceSessionComponent& requester = group[ 1 ].GetResources();
    DRN::CDRNMemoryResourceSink* sink = GUCEF_NEW DRN::CDRNMemoryResourceSink();
    DRN::CIDRNResourceSinkPtr sinkPtr( sink );
    DRN::CDRNResourceTransfer* transfer = requester.Read( LocalPeerOf( group[ 0 ] ), provider, name, offset, length, sinkPtr, options );
    if ( !WaitFinished( transfer, 30000 ) )
        return DRN::DRN_RESSTATUS_TIMEOUT;
    DRN::TDRNResourceStatus status = transfer->GetResult().status;
    if ( DRN::DRN_RESSTATUS_OK == status )
        sink->TakeData( data );
    requester.Release( transfer );
    return status;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNResourceTransferOptions
TreeOptions( void )
{
    DRN::CDRNResourceTransferOptions options;
    options.verifyMode = DRN::CDRNResourceTransferOptions::VERIFY_TREE;
    return options;
}

/*-------------------------------------------------------------------------*/

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNResourceTransferTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DRN RESOURCE TRANSFER TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.Resources" );

    RemoveTestDirectories();

    GUCEF_TESTFW_TESTCASE( "Resources: the hash tree has the root of RFC 9162 for 0 to 17 leaves, every audit path proves its leaf, a changed leaf, index or size does not" )
    try
    {
        DRN::CDRNResourceHasher hasher;
        UInt32 counts[] = { 0, 1, 2, 3, 4, 5, 7, 8, 9, 16, 17 };
        for ( UInt32 c=0; c<sizeof( counts ) / sizeof( counts[ 0 ] ); ++c )
        {
            UInt32 leafCount = counts[ c ];
            UInt64 size = 0 == leafCount ? 0 : (UInt64) ( leafCount - 1 ) * ChunkSize + 1000;
            CORE::CDynamicBuffer content( (UInt32) size + 1, true );
            content.SetDataSize( (UInt32) size );
            SynthFill( 0, (UInt32) size, 3 + c, static_cast< UInt8* >( content.GetBufferPtr() ) );

            DRN::CDRNResourceHashTree tree;
            REQUIRE_TRUE( tree.Reset( size ) && leafCount == tree.GetLeafCount() );
            GUCEF::vector< CORE::CDynamicBuffer > leaves;
            DRN::CDRNResourceInfo info;
            info.size = size;
            for ( UInt32 i=0; i<leafCount; ++i )
            {
                UInt8 leaf[ 32 ];
                hasher.HashLeaf( static_cast< const UInt8* >( content.GetConstBufferPtr() ) + (UInt64) i * ChunkSize, info.GetChunkBytes( i ), leaf );
                REQUIRE_TRUE( tree.SetLeafHash( i, leaf ) );

                // the reference leaf: SHA-256( 0x00 || chunk ) with the built in SHA-256
                CORE::CSha256 sha;
                UInt8 prefix = 0;
                sha.Update( &prefix, 1 );
                sha.Update( static_cast< const UInt8* >( content.GetConstBufferPtr() ) + (UInt64) i * ChunkSize, info.GetChunkBytes( i ) );
                CORE::CDynamicBuffer reference( 32, true );
                reference.SetDataSize( 32 );
                sha.Final( static_cast< UInt8* >( reference.GetBufferPtr() ) );
                ASSERT_TRUE( 0 == memcmp( reference.GetConstBufferPtr(), leaf, 32 ) );
                leaves.push_back( reference );
            }
            REQUIRE_TRUE( tree.Finish( hasher ) );
            UInt8 root[ 32 ];
            UInt8 expectedRoot[ 32 ];
            REQUIRE_TRUE( tree.GetRoot( root ) );
            ReferenceRoot( leaves, 0, leafCount, expectedRoot );
            ASSERT_TRUE( 0 == memcmp( root, expectedRoot, 32 ) );

            for ( UInt32 i=0; i<leafCount; ++i )
            {
                UInt8 path[ DRN::CDRNResourceHashTree::MaxAuditPathLength * 32 ];
                UInt32 length = tree.GetAuditPath( i, path );
                const UInt8* leaf = static_cast< const UInt8* >( leaves[ i ].GetConstBufferPtr() );
                ASSERT_TRUE( DRN::CDRNResourceHashTree::VerifyAuditPath( hasher, i, leafCount, leaf, path, length, root ) );
                UInt8 changed[ 32 ];
                memcpy( changed, leaf, 32 );
                changed[ 5 ] ^= 1;
                ASSERT_FALSE( DRN::CDRNResourceHashTree::VerifyAuditPath( hasher, i, leafCount, changed, path, length, root ) );
                if ( leafCount > 1 )
                {
                    ASSERT_FALSE( DRN::CDRNResourceHashTree::VerifyAuditPath( hasher, ( i + 1 ) % leafCount, leafCount, leaf, path, length, root ) );
                }
            }
            UInt8 digestA[ 32 ];
            UInt8 digestB[ 32 ];
            DRN::CDRNResourceHashTree::ComputeDigest( hasher, size, root, digestA );
            DRN::CDRNResourceHashTree::ComputeDigest( hasher, size + 1, root, digestB );
            ASSERT_TRUE( 0 != memcmp( digestA, digestB, 32 ) );
            UInt8 digestC[ 32 ];
            ASSERT_TRUE( tree.GetDigest( hasher, digestC ) && 0 == memcmp( digestA, digestC, 32 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: names that leave the root, name a drive, a stream or a device, or hold control characters are refused; provider names are checked" )
    try
    {
        const char* good[] = { "a", "a/b", "dir/sub/file.bin", "..a", "a..", ".hidden", "a b/c d", "n\xC3\xA9" };
        const char* bad[] = { "", "/abs", "a/", "a//b", ".", "..", "../x", "a/../b", "a/./b", "a\\b", "C:x", "dir/c:", "a\tb", "a\nb" };
        for ( UInt32 i=0; i<sizeof( good ) / sizeof( good[ 0 ] ); ++i )
            ASSERT_TRUE( DRN::CDRNResourceName::IsValidResourceName( good[ i ] ) );
        for ( UInt32 i=0; i<sizeof( bad ) / sizeof( bad[ 0 ] ); ++i )
            ASSERT_FALSE( DRN::CDRNResourceName::IsValidResourceName( bad[ i ] ) );
        CORE::CString withNul( "ab\0cd", 5 );
        ASSERT_FALSE( DRN::CDRNResourceName::IsValidResourceName( withNul ) );
        CORE::CString tooLong;
        for ( UInt32 i=0; i<1025; ++i )
            tooLong += "x";
        ASSERT_FALSE( DRN::CDRNResourceName::IsValidResourceName( tooLong ) );
        ASSERT_TRUE( DRN::CDRNResourceName::IsValidProviderName( "packs.v2_x-y" ) );
        ASSERT_FALSE( DRN::CDRNResourceName::IsValidProviderName( "" ) );
        ASSERT_FALSE( DRN::CDRNResourceName::IsValidProviderName( "a/b" ) );

        // the file system provider checks again: no name of a requester reaches outside its root
        CORE::CString root = TestDirectory( "names" );
        CORE::CString inside = CORE::CombinePath( root, "inside.bin" );
        REQUIRE_TRUE( WriteSynthFile( inside, 5000, 1 ) );
        DRN::CDRNFileSystemResourceProvider provider( root );
        DRN::CDRNResourceInfo info;
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == provider.Resolve( "inside.bin", info ) && 5000 == info.size );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == provider.Resolve( "../names/inside.bin", info ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == provider.Resolve( "/etc/passwd", info ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == provider.Resolve( "missing.bin", info ) );
        CORE::CreateDirs( CORE::CombinePath( root, "adir" ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == provider.Resolve( "adir", info ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: the partial state round trips, a changed byte or another version is not taken" )
    try
    {
        DRN::CDRNResourceInfo info;
        info.name = "a/b.bin";
        info.size = (UInt64) 5 * 1024 * 1024 * 1024 + 77;
        info.modifiedUtcMs = 123456;
        info.revision = "r1";
        info.hasTreeRoot = true;
        memset( info.treeRoot, 0xAB, 32 );
        DRN::CDRNResourcePartialState state;
        REQUIRE_TRUE( state.Reset( "packs", info.name, info ) );
        state.SetHeld( 0, true );
        state.SetHeld( 9, true );
        state.SetHeld( state.GetChunkCount() - 1, true );
        ASSERT_TRUE( 3 == state.GetHeldCount() && 1 == state.FindFirstMissing( 0 ) && 9 == state.FindFirstHeld( 1 ) );
        ASSERT_TRUE( state.GetHeldBytes() == 2 * (UInt64) ChunkSize + info.GetChunkBytes( state.GetChunkCount() - 1 ) );
        CORE::CDynamicBuffer stored;
        state.Serialize( stored );
        DRN::CDRNResourcePartialState loaded;
        REQUIRE_TRUE( loaded.Deserialize( stored ) );
        ASSERT_TRUE( loaded.IsOf( "packs", info.name, info ) && 3 == loaded.GetHeldCount() && loaded.IsHeld( 9 ) && !loaded.IsHeld( 8 ) );
        DRN::CDRNResourceInfo other( info );
        other.revision = "r2";
        ASSERT_FALSE( loaded.IsOf( "packs", info.name, other ) );
        static_cast< UInt8* >( stored.GetBufferPtr() )[ 20 ] ^= 1;
        ASSERT_FALSE( loaded.Deserialize( stored ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: a file arrives whole, checked by the link and by the hash tree, with its events; an empty file too" )
    try
    {
        CORE::CString root = TestDirectory( "whole" );
        UInt64 size = 5 * ChunkSize + 1234;
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "small.bin" ), size, 7 ) );
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "empty.bin" ), 0, 7 ) );
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_whole", "tcp" );
        REQUIRE_TRUE( StartPair( group ) );
        REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
        ASSERT_FALSE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );

        CORE::CString target = CORE::CombinePath( TestDirectory( "whole_out" ), "small_link.bin" );
        DRN::CDRNResourceSessionComponent& requester = group[ 1 ].GetResources();
        CTransferWatcher watcher;
        DRN::CDRNResourceTransfer* transfer = requester.Fetch( LocalPeerOf( group[ 0 ] ), "files", "small.bin", FileSink( target ) );
        watcher.SubscribeTo( transfer );
        REQUIRE_TRUE( WaitFinished( transfer ) );
        ASSERT_TRUE( transfer->GetResult().IsOk() );
        ASSERT_TRUE( FileMatchesSynth( target, size, 7 ) );
        ASSERT_FALSE( CORE::FileExists( target + ".drnpart" ) || CORE::FileExists( target + ".drnstate" ) );
        ASSERT_TRUE( size == transfer->GetBytesDone() && size == transfer->GetBytesReceived() && !transfer->GetInfo().hasTreeRoot );
        PumpFor( 50 );
        ASSERT_TRUE( 1 == watcher.opened && 1 == watcher.completed && 0 == watcher.failed && watcher.progress > 0 );
        ASSERT_TRUE( requester.Release( transfer ) );

        CORE::CString treeTarget = CORE::CombinePath( TestDirectory( "whole_out" ), "small_tree.bin" );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == FetchFile( group, "files", "small.bin", treeTarget, TreeOptions() ) );
        ASSERT_TRUE( FileMatchesSynth( treeTarget, size, 7 ) );
        DRN::CDRNResourceTransferStats stats = requester.GetStats();
        ASSERT_TRUE( stats.chunksVerified >= 6 && 0 == stats.verificationFailures );
        ASSERT_TRUE( group[ 0 ].GetResources().GetStats().treesBuilt >= 1 );

        // the digest the application pinned: right and wrong
        DRN::CDRNResourceHasher hasher;
        DRN::CDRNResourceHashTree tree;
        CORE::CDynamicBuffer content;
        REQUIRE_TRUE( content.LoadContentFromFile( CORE::CombinePath( root, "small.bin" ) ) );
        REQUIRE_TRUE( tree.Reset( size ) );
        DRN::CDRNResourceInfo info;
        info.size = size;
        for ( UInt64 i=0; i<tree.GetLeafCount(); ++i )
        {
            UInt8 leaf[ 32 ];
            hasher.HashLeaf( static_cast< const UInt8* >( content.GetConstBufferPtr() ) + i * ChunkSize, info.GetChunkBytes( i ), leaf );
            tree.SetLeafHash( i, leaf );
        }
        REQUIRE_TRUE( tree.Finish( hasher ) );
        DRN::CDRNResourceTransferOptions pinned;
        pinned.hasTreeDigest = true;
        REQUIRE_TRUE( tree.GetDigest( hasher, pinned.treeDigest ) );
        CORE::CString pinnedTarget = CORE::CombinePath( TestDirectory( "whole_out" ), "small_pinned.bin" );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == FetchFile( group, "files", "small.bin", pinnedTarget, pinned ) );
        pinned.treeDigest[ 0 ] ^= 1;
        ASSERT_TRUE( DRN::DRN_RESSTATUS_DIGEST_MISMATCH == FetchFile( group, "files", "small.bin", pinnedTarget + ".2", pinned ) );

        // the SHA-256 of the whole file, checked at the end
        DRN::CDRNResourceTransferOptions withSha;
        withSha.hasSha256 = true;
        CORE::CSha256::Hash( content.GetConstBufferPtr(), content.GetDataSize(), withSha.sha256 );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == FetchFile( group, "files", "small.bin", pinnedTarget + ".3", withSha ) );
        withSha.sha256[ 3 ] ^= 1;
        ASSERT_TRUE( DRN::DRN_RESSTATUS_DIGEST_MISMATCH == FetchFile( group, "files", "small.bin", pinnedTarget + ".4", withSha ) );
        ASSERT_FALSE( CORE::FileExists( pinnedTarget + ".4" ) );

        CORE::CString emptyTarget = CORE::CombinePath( TestDirectory( "whole_out" ), "empty.bin" );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == FetchFile( group, "files", "empty.bin", emptyTarget, TreeOptions() ) );
        ASSERT_TRUE( CORE::FileExists( emptyTarget ) && 0 == CORE::FileSize( emptyTarget ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: ranged reads, aligned, unaligned, one byte, the last partial chunk, empty and beyond the end; streamed in order into a CIOAccess" )
    try
    {
        CORE::CString root = TestDirectory( "ranges" );
        UInt64 size = 10 * ChunkSize + 4321;
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "r.bin" ), size, 11 ) );
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_ranges", "tcp" );
        REQUIRE_TRUE( StartPair( group ) );
        REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );

        UInt64 windows[][ 2 ] = { { 0, ChunkSize }, { 100, 70000 }, { ChunkSize - 1, 1 }, { ChunkSize - 7, 14 }, { size - 4321, 4321 },
                                  { size - 10, 10 }, { 3 * ChunkSize + 5, 5 * ChunkSize }, { 0, size } };
        for ( UInt32 w=0; w<sizeof( windows ) / sizeof( windows[ 0 ] ); ++w )
        {
            for ( UInt32 mode=0; mode<2; ++mode )
            {
                CORE::CDynamicBuffer data;
                DRN::CDRNResourceTransferOptions options = 0 == mode ? DRN::CDRNResourceTransferOptions() : TreeOptions();
                DRN::TDRNResourceStatus status = ReadWindow( group, "files", "r.bin", windows[ w ][ 0 ], windows[ w ][ 1 ], data, options );
                ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == status );
                ASSERT_TRUE( data.GetDataSize() == windows[ w ][ 1 ] && BufferMatchesSynth( data.GetConstBufferPtr(), data.GetDataSize(), windows[ w ][ 0 ], 11 ) );
            }
        }
        CORE::CDynamicBuffer data;
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == ReadWindow( group, "files", "r.bin", 500, 0, data, TreeOptions() ) && 0 == data.GetDataSize() );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OUT_OF_RANGE == ReadWindow( group, "files", "r.bin", size - 5, 10, data, DRN::CDRNResourceTransferOptions() ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OUT_OF_RANGE == ReadWindow( group, "files", "r.bin", size + 1, 1, data, DRN::CDRNResourceTransferOptions() ) );

        // in order, into a stream
        CORE::CDynamicBuffer streamed;
        CORE::CDynamicBufferAccess access( streamed );
        DRN::CDRNIOAccessResourceSink* sink = GUCEF_NEW DRN::CDRNIOAccessResourceSink( access );
        DRN::CIDRNResourceSinkPtr sinkPtr( sink );
        DRN::CDRNResourceTransfer* transfer = group[ 1 ].GetResources().Read( LocalPeerOf( group[ 0 ] ), "files", "r.bin", 1000, 300000, sinkPtr, TreeOptions() );
        REQUIRE_TRUE( WaitFinished( transfer ) );
        ASSERT_TRUE( transfer->GetResult().IsOk() && 300000 == sink->GetBytesWritten() );
        ASSERT_TRUE( 300000 == streamed.GetDataSize() && BufferMatchesSynth( streamed.GetConstBufferPtr(), 300000, 1000, 11 ) );
        ASSERT_TRUE( group[ 1 ].GetResources().Release( transfer ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: a synchronous ReadRange on the caller's thread with nodes on threads of their own; refused on a thread that works on the node" )
    try
    {
        CORE::CString root = TestDirectory( "sync" );
        UInt64 size = 4 * ChunkSize + 99;
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "s.bin" ), size, 13 ) );
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_sync", "tcp" );
        group.nodeSettings.ownThread = true;
        REQUIRE_TRUE( StartPair( group ) );
        REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
        CORE::CDynamicBuffer data;
        DRN::CDRNResourceTransferResult result = group[ 1 ].GetResources().ReadRange( LocalPeerOf( group[ 0 ] ), "files", "s.bin", 777, 150000, data, TreeOptions(), 20000 );
        ASSERT_TRUE( result.IsOk() && 150000 == data.GetDataSize() && BufferMatchesSynth( data.GetConstBufferPtr(), 150000, 777, 13 ) );
        result = group[ 1 ].GetResources().ReadRange( LocalPeerOf( group[ 0 ] ), "files", "missing.bin", 0, 10, data, TreeOptions(), 20000 );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == result.status && 0 == data.GetDataSize() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: offsets beyond 4 GiB of a 6 GiB synthetic resource are read right; a tree is required for it only when asked" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_huge", "tcp" );
        REQUIRE_TRUE( StartPair( group ) );
        UInt64 size = (UInt64) 6 * 1024 * 1024 * 1024 + 12345;
        REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "synth", DRN::CIDRNResourceProviderPtr( GUCEF_NEW CSynthProvider( size, 17 ) ), ShareAll() ) );
        UInt64 offsets[] = { (UInt64) 5 * 1024 * 1024 * 1024 + 12345, (UInt64) 4 * 1024 * 1024 * 1024 - 1000, size - 300000 };
        for ( UInt32 i=0; i<3; ++i )
        {
            CORE::CDynamicBuffer data;
            ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == ReadWindow( group, "synth", "huge.bin", offsets[ i ], 300000, data, DRN::CDRNResourceTransferOptions() ) );
            ASSERT_TRUE( 300000 == data.GetDataSize() && BufferMatchesSynth( data.GetConstBufferPtr(), 300000, offsets[ i ], 17 ) );
        }
        // above maxTreeResourceBytes: no tree is built, VERIFY_TREE refuses, VERIFY_TREE_IF_AVAILABLE reads without
        CORE::CDynamicBuffer data;
        ASSERT_TRUE( DRN::DRN_RESSTATUS_NO_TREE == ReadWindow( group, "synth", "huge.bin", offsets[ 0 ], 1000, data, TreeOptions() ) );
        DRN::CDRNResourceTransferOptions ifAvailable;
        ifAvailable.verifyMode = DRN::CDRNResourceTransferOptions::VERIFY_TREE_IF_AVAILABLE;
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == ReadWindow( group, "synth", "huge.bin", offsets[ 0 ], 1000, data, ifAvailable ) );
        ASSERT_TRUE( BufferMatchesSynth( data.GetConstBufferPtr(), 1000, offsets[ 0 ], 17 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: a 256 MiB file arrives whole and verified while both sides hold no more than their bounds in memory" )
    try
    {
        CORE::CString root = TestDirectory( "large" );
        UInt64 size = (UInt64) 256 * 1024 * 1024 + 777;
        CORE::CString source = CORE::CombinePath( root, "large.bin" );
        REQUIRE_TRUE( WriteSynthFile( source, size, 19 ) );
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_large", "tcp" );
        REQUIRE_TRUE( StartPair( group ) );
        REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
        CORE::CString target = CORE::CombinePath( TestDirectory( "large_out" ), "large.bin" );
        UInt64 start = NowMs();
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == FetchFile( group, "files", "large.bin", target, TreeOptions(), 180000 ) );
        UInt64 took = NowMs() - start;
        ASSERT_TRUE( FileMatchesSynth( target, size, 19 ) );
        DRN::CDRNResourceTransferStats server = group[ 0 ].GetResources().GetStats();
        DRN::CDRNResourceTransferStats client = group[ 1 ].GetResources().GetStats();
        DRN::CDRNResourceTransferSettings settings = group[ 0 ].GetResources().GetSettings();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN resources: 256 MiB verified in " << took << " ms, server peak " << server.peakServerBufferedBytes <<
                   " bytes, requester peak " << client.peakClientBufferedBytes << " bytes" );
        ASSERT_TRUE( server.peakServerBufferedBytes <= settings.maxBufferedBytes );
        ASSERT_TRUE( server.peakServerBufferedBytes <= (UInt64) ( settings.readAheadChunks + 1 ) * ChunkSize );
        ASSERT_TRUE( client.peakClientBufferedBytes <= (UInt64) DRN::CDRNResourceTransferOptions().windowChunks * ChunkSize );
        ASSERT_TRUE( 0 == server.serverBufferedBytes && 0 == client.clientBufferedBytes );
        ASSERT_TRUE( client.chunksVerified >= DRN::CDRNResourceInfo::ChunkCountFor( size ) );
        CORE::DeleteFile( source );
        CORE::DeleteFile( target );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: a fetch cut off by a lost link resumes in a new session with the chunks it held, and after a restart of the requester from its persisted state" )
    try
    {
        CORE::CString root = TestDirectory( "resume" );
        UInt64 size = (UInt64) 24 * 1024 * 1024 + 99;
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "r.bin" ), size, 23 ) );
        CORE::CString target = CORE::CombinePath( TestDirectory( "resume_out" ), "r.bin" );
        DRN::CDRNResourceTransferSettings slow;
        slow.maxDownloadBytesPerSecond = 4 * 1024 * 1024;
        slow.stateSaveIntervalMs = 100;

        // 1: the serving node goes away mid-way
        UInt64 firstPart = 0;
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_resume1", "tcp" );
            REQUIRE_TRUE( StartPair( group ) );
            REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
            group[ 1 ].GetResources().SetSettings( slow );
            DRN::CDRNResourceTransfer* transfer = group[ 1 ].GetResources().Fetch( LocalPeerOf( group[ 0 ] ), "files", "r.bin", FileSink( target ), TreeOptions() );
            CBytesDone quarter( transfer, size / 4 );
            REQUIRE_TRUE( PumpUntil( quarter, 30000 ) );
            PumpFor( 300 );
            group.nodes[ 0 ]->Stop();
            REQUIRE_TRUE( WaitFinished( transfer, 30000 ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_LINK_LOST == transfer->GetResult().status );
            firstPart = transfer->GetBytesDone();
            ASSERT_TRUE( firstPart >= size / 4 && firstPart < size );
            ASSERT_TRUE( CORE::FileExists( target + ".drnstate" ) && CORE::FileExists( target + ".drnpart" ) );
            group[ 1 ].GetResources().Release( transfer );
        }

        // 2: another session: the chunks held are not fetched again
        UInt64 resumedOnce = 0;
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_resume2", "tcp" );
            REQUIRE_TRUE( StartPair( group ) );
            REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
            group[ 1 ].GetResources().SetSettings( slow );
            DRN::CDRNResourceTransfer* transfer = group[ 1 ].GetResources().Fetch( LocalPeerOf( group[ 0 ] ), "files", "r.bin", FileSink( target ), TreeOptions() );
            CBytesDone half( transfer, ( size * 3 ) / 5 );
            REQUIRE_TRUE( PumpUntil( half, 30000 ) );
            resumedOnce = transfer->GetBytesResumed();
            ASSERT_TRUE( resumedOnce + 2 * 1024 * 1024 >= firstPart && resumedOnce > 0 );
            PumpFor( 300 );

            // 3: the requester stops (a restart): its node goes, the state on disk stays
            group.nodes[ 1 ]->Stop();
            PumpFor( 500 );
        }
        ASSERT_TRUE( CORE::FileExists( target + ".drnstate" ) );
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_resume3", "tcp" );
            REQUIRE_TRUE( StartPair( group ) );
            REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
            DRN::CDRNResourceTransfer* transfer = group[ 1 ].GetResources().Fetch( LocalPeerOf( group[ 0 ] ), "files", "r.bin", FileSink( target ), TreeOptions() );
            REQUIRE_TRUE( WaitFinished( transfer, 60000 ) );
            ASSERT_TRUE( transfer->GetResult().IsOk() );
            ASSERT_TRUE( transfer->GetBytesResumed() >= ( size * 3 ) / 5 - 2 * 1024 * 1024 );
            ASSERT_TRUE( transfer->GetBytesReceived() + transfer->GetBytesResumed() == size );
            group[ 1 ].GetResources().Release( transfer );
        }
        ASSERT_TRUE( FileMatchesSynth( target, size, 23 ) );
        ASSERT_FALSE( CORE::FileExists( target + ".drnstate" ) || CORE::FileExists( target + ".drnpart" ) );

        // a partial file of another revision is not mixed in: the fetch starts over
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "r.bin" ), size, 29 ) );
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_resume4", "tcp" );
            REQUIRE_TRUE( StartPair( group ) );
            REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
            group[ 1 ].GetResources().SetSettings( slow );
            DRN::CDRNResourceTransfer* transfer = group[ 1 ].GetResources().Fetch( LocalPeerOf( group[ 0 ] ), "files", "r.bin", FileSink( target ), DRN::CDRNResourceTransferOptions() );
            CBytesDone some( transfer, size / 5 );
            REQUIRE_TRUE( PumpUntil( some, 30000 ) );
            transfer->Cancel();
            REQUIRE_TRUE( WaitFinished( transfer, 30000 ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_CANCELLED == transfer->GetResult().status );
            group[ 1 ].GetResources().Release( transfer );
        }
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "r.bin" ), size, 31 ) );
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_resume5", "tcp" );
            REQUIRE_TRUE( StartPair( group ) );
            REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
            DRN::CDRNResourceTransferOptions strict;
            strict.restartIfChanged = false;
            DRN::CDRNResourceTransfer* transfer = group[ 1 ].GetResources().Fetch( LocalPeerOf( group[ 0 ] ), "files", "r.bin", FileSink( target ), strict );
            REQUIRE_TRUE( WaitFinished( transfer, 30000 ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_CHANGED == transfer->GetResult().status );
            group[ 1 ].GetResources().Release( transfer );
            transfer = group[ 1 ].GetResources().Fetch( LocalPeerOf( group[ 0 ] ), "files", "r.bin", FileSink( target ), DRN::CDRNResourceTransferOptions() );
            REQUIRE_TRUE( WaitFinished( transfer, 60000 ) );
            ASSERT_TRUE( transfer->GetResult().IsOk() && 0 == transfer->GetBytesResumed() );
            group[ 1 ].GetResources().Release( transfer );
        }
        ASSERT_TRUE( FileMatchesSynth( target, size, 31 ) );
        CORE::DeleteFile( target );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: a chunk that arrives corrupted fails its audit path and is fetched again; one that stays corrupted ends with DIGEST_MISMATCH" )
    try
    {
        CORE::CString root = TestDirectory( "corrupt" );
        UInt64 size = 20 * ChunkSize + 5;
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "c.bin" ), size, 37 ) );
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_corrupt", "tcp" );
        REQUIRE_TRUE( StartPair( group ) );
        CCorruptingProvider* once = GUCEF_NEW CCorruptingProvider( root, 7, 1 );
        REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "once", DRN::CIDRNResourceProviderPtr( once ), ShareAll() ) );
        CCorruptingProvider* always = GUCEF_NEW CCorruptingProvider( root, 3, 1000 );
        REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "always", DRN::CIDRNResourceProviderPtr( always ), ShareAll() ) );

        CORE::CString target = CORE::CombinePath( TestDirectory( "corrupt_out" ), "c.bin" );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == FetchFile( group, "once", "c.bin", target, TreeOptions() ) );
        ASSERT_TRUE( FileMatchesSynth( target, size, 37 ) );
        ASSERT_TRUE( 1 == once->m_corruptions && group[ 1 ].GetResources().GetStats().verificationFailures >= 1 );

        CORE::CString target2 = CORE::CombinePath( TestDirectory( "corrupt_out" ), "c2.bin" );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_DIGEST_MISMATCH == FetchFile( group, "always", "c.bin", target2, TreeOptions() ) );
        ASSERT_TRUE( always->m_corruptions >= 4 );

        // without the tree the link is trusted: the corruption of the serving side passes, as the option says
        CORE::CDynamicBuffer data;
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == ReadWindow( group, "always", "c.bin", 3 * ChunkSize, ChunkSize, data, DRN::CDRNResourceTransferOptions() ) );
        ASSERT_FALSE( BufferMatchesSynth( data.GetConstBufferPtr(), data.GetDataSize(), 3 * ChunkSize, 37 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: refusals: no provider, no resource, a role or the access policy forbids, a changed revision, too large for memory, a link outside the root, a malformed open" )
    try
    {
        CORE::CString root = TestDirectory( "refusals" );
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "f.bin" ), 3000, 41 ) );
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "secret.bin" ), 3000, 41 ) );
        CORE::CString outside = CORE::CombinePath( TestDirectory( "refusals_outside" ), "o.bin" );
        REQUIRE_TRUE( WriteSynthFile( outside, 3000, 43 ) );
        bool haveLinks = false;
        #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
        haveLinks = 0 == ::symlink( outside.C_String(), CORE::CombinePath( root, "escape.bin" ).C_String() ) &&
                    0 == ::symlink( CORE::CombinePath( root, "f.bin" ).C_String(), CORE::CombinePath( root, "alias.bin" ).C_String() );
        #endif

        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_refusals", "tcp" );
        REQUIRE_TRUE( StartPair( group ) );
        CNoResourcesPolicy policy;
        group.nodes[ 0 ]->SetAccessPolicy( &policy );
        DRN::CDRNResourceSessionComponent& server = group[ 0 ].GetResources();
        REQUIRE_TRUE( server.RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
        REQUIRE_TRUE( server.RegisterProvider( "follow", FileProvider( root, true ), ShareAll() ) );
        REQUIRE_TRUE( server.RegisterProvider( "admin", FileProvider( root ), DRN::CDRNResourceShareRules( DRN::CDRNResourceShareRules::WholeProvider, "admin" ) ) );

        CORE::CDynamicBuffer data;
        DRN::CDRNResourceTransferOptions plain;
        ASSERT_TRUE( DRN::DRN_RESSTATUS_NO_SUCH_PROVIDER == ReadWindow( group, "nothere", "f.bin", 0, 10, data, plain ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == ReadWindow( group, "files", "nothere.bin", 0, 10, data, plain ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == ReadWindow( group, "admin", "f.bin", 0, 10, data, plain ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == ReadWindow( group, "files", "secret.bin", 0, 10, data, plain ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == ReadWindow( group, "files", "../refusals/f.bin", 0, 10, data, plain ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == ReadWindow( group, "files", "/etc/passwd", 0, 10, data, plain ) );
        DRN::CDRNResourceTransferOptions revision;
        revision.expectedRevision = "not-this-one";
        ASSERT_TRUE( DRN::DRN_RESSTATUS_CHANGED == ReadWindow( group, "files", "f.bin", 0, 10, data, revision ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == ReadWindow( group, "files", "f.bin", 0, 10, data, plain ) );

        // a memory target that is too small: refused before anything is fetched
        DRN::CDRNResourceTransfer* transfer = group[ 1 ].GetResources().Read( LocalPeerOf( group[ 0 ] ), "files", "f.bin", 0, 3000,
                                                                              DRN::CIDRNResourceSinkPtr( GUCEF_NEW DRN::CDRNMemoryResourceSink( 1000 ) ), plain );
        REQUIRE_TRUE( WaitFinished( transfer ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_TOO_LARGE == transfer->GetResult().status && 0 == transfer->GetBytesReceived() );
        group[ 1 ].GetResources().Release( transfer );

        if ( haveLinks )
        {
            ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == ReadWindow( group, "files", "escape.bin", 0, 10, data, plain ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == ReadWindow( group, "files", "alias.bin", 0, 10, data, plain ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == ReadWindow( group, "follow", "escape.bin", 0, 10, data, plain ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == ReadWindow( group, "follow", "alias.bin", 0, 10, data, plain ) );
            ASSERT_TRUE( BufferMatchesSynth( data.GetConstBufferPtr(), 10, 0, 41 ) );
        }

        // an open that bypasses the requester's own checks: the serving side refuses it, nothing stays open
        UInt64 refusedBefore = server.GetStats().opensRefused;
        CORE::CDynamicBuffer open;
        DRN::CDRNWireWriter writer( open );
        writer.WriteVarUInt32( 999999 );
        writer.WriteString( "files" );
        writer.WriteString( "../refusals_outside/o.bin" );
        writer.WriteVarUInt32( 0 );
        writer.WriteString( CORE::CString::Empty );
        {
            MT::CScopeMutex lock( group[ 1 ].GetNode().GetLock() );
            DRN::CDRNPeerLinkPtr link = group[ 1 ].GetLinkToMember( LocalPeerOf( group[ 0 ] ) );
            REQUIRE_TRUE( !link.IsNULL() );
            REQUIRE_TRUE( link->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_RESOURCE, DRN::DRN_RES_OPEN, open.GetConstBufferPtr(), writer.GetOffset() ) );
        }
        UInt32 requesterPeer = LocalPeerOf( group[ 1 ] );
        PumpFor( 300 );
        ASSERT_TRUE( server.GetStats().opensRefused == refusedBefore + 1 && 0 == server.GetOpenHandleCount( requesterPeer ) );
        group.nodes[ 0 ]->SetAccessPolicy( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: concurrent fetches over one link share it fairly; a limit of open resources answers BUSY; a cancelled fetch keeps its part and ends the serving side's read" )
    try
    {
        CORE::CString root = TestDirectory( "fair" );
        UInt64 size = (UInt64) 8 * 1024 * 1024;
        for ( UInt32 i=0; i<3; ++i )
            REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "f" + CORE::ToString( i ) + ".bin" ), size, 50 + i ) );
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_fair", "tcp" );
        REQUIRE_TRUE( StartPair( group ) );
        REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
        DRN::CDRNResourceTransferSettings slow;
        slow.maxDownloadBytesPerSecond = 12 * 1024 * 1024;
        group[ 1 ].GetResources().SetSettings( slow );

        CORE::CString outDir = TestDirectory( "fair_out" );
        DRN::CDRNResourceTransfer* transfers[ 3 ];
        for ( UInt32 i=0; i<3; ++i )
            transfers[ i ] = group[ 1 ].GetResources().Fetch( LocalPeerOf( group[ 0 ] ), "files", "f" + CORE::ToString( i ) + ".bin",
                                                              FileSink( CORE::CombinePath( outDir, "f" + CORE::ToString( i ) + ".bin" ) ) );
        UInt64 start = NowMs();
        UInt64 leastAtFirst = 0;
        bool firstSeen = false;
        while ( NowMs() - start < 60000 )
        {
            PumpFor( 5 );
            bool all = true;
            for ( UInt32 i=0; i<3; ++i )
            {
                if ( !transfers[ i ]->IsFinished() )
                {
                    all = false;
                    continue;
                }
                if ( !firstSeen )
                {
                    firstSeen = true;
                    leastAtFirst = size;
                    for ( UInt32 j=0; j<3; ++j )
                        if ( transfers[ j ]->GetBytesDone() < leastAtFirst )
                            leastAtFirst = transfers[ j ]->GetBytesDone();
                }
            }
            if ( all )
                break;
        }
        for ( UInt32 i=0; i<3; ++i )
        {
            ASSERT_TRUE( transfers[ i ]->IsFinished() && transfers[ i ]->GetResult().IsOk() );
            ASSERT_TRUE( FileMatchesSynth( CORE::CombinePath( outDir, "f" + CORE::ToString( i ) + ".bin" ), size, 50 + i ) );
            group[ 1 ].GetResources().Release( transfers[ i ] );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN resources: when the first of three fetches completed the least advanced had " << leastAtFirst << " of " << size << " bytes" );
        ASSERT_TRUE( leastAtFirst >= ( size * 3 ) / 4 );

        // one open resource per member: the second is BUSY
        slow.maxDownloadBytesPerSecond = 2 * 1024 * 1024;
        group[ 1 ].GetResources().SetSettings( slow );
        DRN::CDRNResourceTransferSettings limited;
        limited.maxOpenHandlesPerPeer = 1;
        group[ 0 ].GetResources().SetSettings( limited );
        DRN::CDRNResourceTransfer* first = group[ 1 ].GetResources().Fetch( LocalPeerOf( group[ 0 ] ), "files", "f0.bin", FileSink( CORE::CombinePath( outDir, "busy0.bin" ) ) );
        CBytesDone started( first, 1 );
        REQUIRE_TRUE( PumpUntil( started, 20000 ) );
        DRN::CDRNResourceTransfer* second = group[ 1 ].GetResources().Fetch( LocalPeerOf( group[ 0 ] ), "files", "f1.bin", FileSink( CORE::CombinePath( outDir, "busy1.bin" ) ) );
        REQUIRE_TRUE( WaitFinished( second, 20000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_BUSY == second->GetResult().status && DRN::CDRNResourceTransferResult::IsRetryable( second->GetResult().status ) );
        group[ 1 ].GetResources().Release( second );

        // cancel: the part stays for a resume, the serving side lets go of the resource
        first->Cancel();
        REQUIRE_TRUE( WaitFinished( first, 20000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_CANCELLED == first->GetResult().status );
        ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( outDir, "busy0.bin.drnstate" ) ) );
        group[ 1 ].GetResources().Release( first );
        UInt32 requesterPeer = LocalPeerOf( group[ 1 ] );
        UInt64 waitStart = NowMs();
        while ( 0 != group[ 0 ].GetResources().GetOpenHandleCount( requesterPeer ) && NowMs() - waitStart < 5000 )
            PumpFor( 5 );
        ASSERT_TRUE( 0 == group[ 0 ].GetResources().GetOpenHandleCount( requesterPeer ) );

        // a cancel that discards removes the part
        DRN::CDRNResourceTransfer* third = group[ 1 ].GetResources().Fetch( LocalPeerOf( group[ 0 ] ), "files", "f2.bin", FileSink( CORE::CombinePath( outDir, "busy2.bin" ) ) );
        CBytesDone some( third, 1 );
        REQUIRE_TRUE( PumpUntil( some, 20000 ) );
        third->Cancel( true );
        REQUIRE_TRUE( WaitFinished( third, 20000 ) );
        ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( outDir, "busy2.bin.drnstate" ) ) || CORE::FileExists( CORE::CombinePath( outDir, "busy2.bin.drnpart" ) ) );
        group[ 1 ].GetResources().Release( third );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: over plain links only when both sides allow it (INSECURE_LINK otherwise); a member of protocol 2.1 is UNSUPPORTED; over a datagram channel" )
    try
    {
        CORE::CString root = TestDirectory( "links" );
        UInt64 size = 3 * ChunkSize + 17;
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "l.bin" ), size, 61 ) );
        CORE::CString outDir = TestDirectory( "links_out" );
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_plain", "tcp" );
            group.nodeSettings.securityPolicy.requireEncryption = false;
            group.nodeSettings.securityPolicy.ciphers.clear();
            group.sessionSettings.securityPolicy.requireEncryption = false;
            group.sessionSettings.securityPolicy.ciphers.clear();
            REQUIRE_TRUE( StartPair( group ) );
            REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_INSECURE_LINK == FetchFile( group, "files", "l.bin", CORE::CombinePath( outDir, "plain0.bin" ), TreeOptions() ) );
            DRN::CDRNResourceTransferSettings allow;
            allow.allowUnprotectedLinks = true;
            group[ 1 ].GetResources().SetSettings( allow );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_INSECURE_LINK == FetchFile( group, "files", "l.bin", CORE::CombinePath( outDir, "plain1.bin" ), TreeOptions() ) );
            group[ 0 ].GetResources().SetSettings( allow );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == FetchFile( group, "files", "l.bin", CORE::CombinePath( outDir, "plain2.bin" ), TreeOptions() ) );
            ASSERT_TRUE( FileMatchesSynth( CORE::CombinePath( outDir, "plain2.bin" ), size, 61 ) );
        }
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_old", "tcp" );
            group.nodeSettings.protocolMinor = DRN::DRN_PROTOCOL_MINOR_RPC;
            REQUIRE_TRUE( StartPair( group ) );
            REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_UNSUPPORTED == FetchFile( group, "files", "l.bin", CORE::CombinePath( outDir, "old.bin" ), TreeOptions() ) );
            ASSERT_TRUE( 0 == group[ 0 ].GetNode().GetUnhandledMessageCount() );
        }
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_udp", "udp" );
            REQUIRE_TRUE( StartPair( group ) );
            REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == FetchFile( group, "files", "l.bin", CORE::CombinePath( outDir, "udp.bin" ), TreeOptions() ) );
            ASSERT_TRUE( FileMatchesSynth( CORE::CombinePath( outDir, "udp.bin" ), size, 61 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: nothing is shared by default; an allow-listed subtree is, for the allowed roles only; outside it and traversal stay refused; the policy may share by itself" )
    try
    {
        CORE::CString root = TestDirectory( "share" );
        CORE::CreateDirs( CORE::CombinePath( root, "maps" ) );
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( CORE::CombinePath( root, "maps" ), "a.bin" ), 5000, 81 ) );
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "other.bin" ), 5000, 83 ) );
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "mapsx.bin" ), 5000, 85 ) );
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_share", "tcp" );
        REQUIRE_TRUE( StartPair( group ) );
        CNoResourcesPolicy policy;
        DRN::CDRNResourceSessionComponent& server = group[ 0 ].GetResources();
        REQUIRE_TRUE( server.RegisterProvider( "nothing", FileProvider( root ) ) );
        REQUIRE_TRUE( server.RegisterProvider( "maps", FileProvider( root ), DRN::CDRNResourceShareRules( "maps", DRN::CDRNResourceShareRules::AnyRole ) ) );
        REQUIRE_TRUE( server.RegisterProvider( "adminmaps", FileProvider( root ), DRN::CDRNResourceShareRules( "maps", "admin" ) ) );
        REQUIRE_TRUE( server.RegisterProvider( "lobby", FileProvider( root ) ) );
        ASSERT_FALSE( server.RegisterProvider( "badrules", FileProvider( root ), DRN::CDRNResourceShareRules( "../x", DRN::CDRNResourceShareRules::AnyRole ) ) );

        CORE::CDynamicBuffer data;
        DRN::CDRNResourceTransferOptions plain;
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == ReadWindow( group, "nothing", "other.bin", 0, 10, data, plain ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == ReadWindow( group, "nothing", "maps/a.bin", 0, 10, data, plain ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == ReadWindow( group, "maps", "maps/a.bin", 0, 10, data, plain ) );
        ASSERT_TRUE( BufferMatchesSynth( data.GetConstBufferPtr(), 10, 0, 81 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == ReadWindow( group, "maps", "other.bin", 0, 10, data, plain ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == ReadWindow( group, "maps", "mapsx.bin", 0, 10, data, plain ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == ReadWindow( group, "maps", "maps", 0, 10, data, plain ) );     // shared, but a folder
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == ReadWindow( group, "adminmaps", "maps/a.bin", 0, 10, data, plain ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == ReadWindow( group, "maps", "maps/../other.bin", 0, 10, data, plain ) );

        // the policy shares "lobby" by itself (an explicit transfer of the host), the default policy shares nothing more
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == ReadWindow( group, "lobby", "other.bin", 0, 10, data, plain ) );
        group.nodes[ 0 ]->SetAccessPolicy( &policy );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == ReadWindow( group, "lobby", "other.bin", 0, 10, data, plain ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == ReadWindow( group, "nothing", "other.bin", 0, 10, data, plain ) );

        // a traversal that bypasses the requester's checks: the serving side refuses it before the share rules
        UInt64 refusedBefore = server.GetStats().opensRefused;
        CORE::CDynamicBuffer open;
        DRN::CDRNWireWriter writer( open );
        writer.WriteVarUInt32( 888888 );
        writer.WriteString( "maps" );
        writer.WriteString( "maps/../other.bin" );
        writer.WriteVarUInt32( 0 );
        writer.WriteString( CORE::CString::Empty );
        {
            MT::CScopeMutex lock( group[ 1 ].GetNode().GetLock() );
            DRN::CDRNPeerLinkPtr link = group[ 1 ].GetLinkToMember( LocalPeerOf( group[ 0 ] ) );
            REQUIRE_TRUE( !link.IsNULL() );
            REQUIRE_TRUE( link->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_RESOURCE, DRN::DRN_RES_OPEN, open.GetConstBufferPtr(), writer.GetOffset() ) );
        }
        PumpFor( 300 );
        ASSERT_TRUE( server.GetStats().opensRefused == refusedBefore + 1 && 0 == server.GetOpenHandleCount( LocalPeerOf( group[ 1 ] ) ) );
        group.nodes[ 0 ]->SetAccessPolicy( GUCEF_NULL );

        // the share rules themselves
        DRN::CDRNResourceShareRules rules( "maps", "viewer" );
        ASSERT_TRUE( rules.Allows( "viewer", "maps/x/y.bin" ) && !rules.Allows( "other", "maps/a.bin" ) && !rules.Allows( "viewer", "map" ) );
        ASSERT_FALSE( DRN::CDRNResourceShareRules().Allows( "", "a.bin" ) );
        // a member that carries several roles (comma separated) matches on any of them
        ASSERT_TRUE( rules.Allows( "other,viewer", "maps/a.bin" ) && rules.Allows( "viewer,editor", "maps/a.bin" ) && !rules.Allows( "other,editor", "maps/a.bin" ) );
        ASSERT_TRUE( DRN::CDRNResourceShareRules( "maps", "*" ).Allows( "a,b", "maps/a.bin" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: hash trees kept in a configured tree cache directory are loaded in a later run, a changed file builds again, the directory is pruned" )
    try
    {
        CORE::CString root = TestDirectory( "treecache" );
        CORE::CString treeDir = TestDirectory( "treecache_trees" );
        CORE::CString file = CORE::CombinePath( root, "t.bin" );
        UInt64 size = 12 * ChunkSize + 5;
        REQUIRE_TRUE( WriteSynthFile( file, size, 91 ) );
        DRN::CDRNResourceTransferSettings withDir;
        withDir.treeCacheDirectory = treeDir;
        CORE::CString outDir = TestDirectory( "treecache_out" );
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_trees1", "tcp" );
            REQUIRE_TRUE( StartPair( group ) );
            group[ 0 ].GetResources().SetSettings( withDir );
            REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == FetchFile( group, "files", "t.bin", CORE::CombinePath( outDir, "t1.bin" ), TreeOptions() ) );
            DRN::CDRNResourceTransferStats stats = group[ 0 ].GetResources().GetStats();
            ASSERT_TRUE( 1 == stats.treesBuilt && 0 == stats.treesLoaded );
        }
        DRN::CDRNResourceInfo info;
        DRN::CDRNFileSystemResourceProvider direct( root );
        REQUIRE_TRUE( DRN::DRN_RESSTATUS_OK == direct.Resolve( "t.bin", info ) );
        CORE::CString treeFile = DRN::CDRNResourceTreeStore::GetTreeFilePath( treeDir, "files", info );
        ASSERT_TRUE( CORE::FileExists( treeFile ) );
        ASSERT_FALSE( CORE::FileExists( file + ".drntree" ) );
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_trees2", "tcp" );
            REQUIRE_TRUE( StartPair( group ) );
            group[ 0 ].GetResources().SetSettings( withDir );
            REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == FetchFile( group, "files", "t.bin", CORE::CombinePath( outDir, "t2.bin" ), TreeOptions() ) );
            ASSERT_TRUE( FileMatchesSynth( CORE::CombinePath( outDir, "t2.bin" ), size, 91 ) );
            DRN::CDRNResourceTransferStats stats = group[ 0 ].GetResources().GetStats();
            ASSERT_TRUE( 0 == stats.treesBuilt && 1 == stats.treesLoaded );

            // another content: another revision, the kept tree does not fit, a new one is built
            MT::PrecisionDelay( 20 );
            REQUIRE_TRUE( WriteSynthFile( file, size, 93 ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == FetchFile( group, "files", "t.bin", CORE::CombinePath( outDir, "t3.bin" ), TreeOptions() ) );
            ASSERT_TRUE( FileMatchesSynth( CORE::CombinePath( outDir, "t3.bin" ), size, 93 ) );
            ASSERT_TRUE( 1 == group[ 0 ].GetResources().GetStats().treesBuilt );
        }

        // a damaged tree file is not taken
        DRN::CDRNResourceInfo changed;
        REQUIRE_TRUE( DRN::DRN_RESSTATUS_OK == direct.Resolve( "t.bin", changed ) );
        CORE::CString changedTree = DRN::CDRNResourceTreeStore::GetTreeFilePath( treeDir, "files", changed );
        CORE::CDynamicBuffer stored;
        REQUIRE_TRUE( stored.LoadContentFromFile( changedTree ) );
        DRN::CDRNResourceHashTree loaded;
        ASSERT_TRUE( DRN::CDRNResourceTreeStore::Load( treeDir, "files", changed, loaded ) && loaded.GetLeafCount() == 13 );
        static_cast< UInt8* >( stored.GetBufferPtr() )[ 40 ] ^= 1;
        REQUIRE_TRUE( stored.WriteContentToFile( changedTree ) );
        ASSERT_FALSE( DRN::CDRNResourceTreeStore::Load( treeDir, "files", changed, loaded ) );

        // pruning: down to the size, the oldest first
        DRN::CDRNResourceTreeStore::Prune( treeDir, 0, 0 );
        ASSERT_FALSE( CORE::FileExists( treeFile ) || CORE::FileExists( changedTree ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: one open handle serves many windows: only the first request opens the resource; random access through CDRNResourceIOAccess with Seek, Read, Tell, Eof, GetChar, and a generic CIOAccess consumer" )
    try
    {
        CORE::CString root = TestDirectory( "handles" );
        UInt64 size = 40 * ChunkSize + 777;
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "h.bin" ), size, 101 ) );
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_handles", "tcp" );
        group.nodeSettings.ownThread = true;
        REQUIRE_TRUE( StartPair( group ) );
        DRN::CDRNResourceSessionComponent& server = group[ 0 ].GetResources();
        DRN::CDRNResourceSessionComponent& requester = group[ 1 ].GetResources();
        REQUIRE_TRUE( server.RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
        UInt32 serverPeer = LocalPeerOf( group[ 0 ] );
        UInt32 requesterPeer = LocalPeerOf( group[ 1 ] );

        UInt64 opensBefore = server.GetStats().opensServed;
        DRN::CDRNResourceHandlePtr handle = requester.OpenHandle( serverPeer, "files", "h.bin", TreeOptions() );
        REQUIRE_TRUE( handle->WaitOpen( 20000 ) );
        DRN::CDRNResourceInfo info = handle->GetInfo();
        ASSERT_TRUE( size == info.size && info.hasTreeRoot && !info.revision.IsNULLOrEmpty() && 0 != info.modifiedUtcMs );
        UInt64 offsets[] = { 0, 70000, size - 10, 5 * ChunkSize - 3, 33 * ChunkSize + 1, 1 };
        UInt32 lengths[] = { 100, 200000, 10, 6, 65536, 1 };
        for ( UInt32 round=0; round<3; ++round )
        {
            for ( UInt32 i=0; i<6; ++i )
            {
                CORE::CDynamicBuffer data;
                DRN::CDRNResourceTransferResult result = handle->ReadRange( offsets[ i ], lengths[ i ], data );
                ASSERT_TRUE( result.IsOk() && data.GetDataSize() == lengths[ i ] && BufferMatchesSynth( data.GetConstBufferPtr(), lengths[ i ], offsets[ i ], 101 ) );
            }
        }
        ASSERT_TRUE( server.GetStats().opensServed == opensBefore + 1 && 1 == handle->GetOpenCount() && 18 == handle->GetReadCount() );
        ASSERT_TRUE( 1 == server.GetOpenHandleCount( requesterPeer ) && 1 == requester.GetClientHandleCount() );

        // a CIOAccess over the handle: random access as a file
        DRN::CDRNResourceIOAccess access( handle );
        access.Open();
        REQUIRE_TRUE( access.Opened() && access.IsReadable() && !access.IsWriteable() && size == access.GetSize() );
        UInt8 bytes[ 1000 ];
        ASSERT_TRUE( 0 == access.Seek( (Int64) ( 20 * ChunkSize - 500 ), SEEK_SET ) && 1000 == access.Read( bytes, 1, 1000 ) );
        ASSERT_TRUE( BufferMatchesSynth( bytes, 1000, 20 * ChunkSize - 500, 101 ) && access.Tell() == 20 * ChunkSize + 500 );
        ASSERT_TRUE( 0 == access.Seek( -1000, SEEK_CUR ) && 250 == access.Read( bytes, 4, 250 ) && BufferMatchesSynth( bytes, 1000, 20 * ChunkSize - 500, 101 ) );
        ASSERT_TRUE( 0 == access.Seek( -5, SEEK_END ) && 2 == access.Read( bytes, 2, 3 ) && access.Tell() == size - 1 && !access.Eof() );
        ASSERT_TRUE( BufferMatchesSynth( bytes, 4, size - 5, 101 ) );
        UInt8 last = (UInt8) access.GetChar();
        ASSERT_TRUE( BufferMatchesSynth( &last, 1, size - 1, 101 ) && access.Eof() && 0 == access.Read( bytes, 1, 10 ) );
        ASSERT_TRUE( 0 == access.Setpos( size + 100 ) && access.Eof() && 0 == access.Read( bytes, 1, 1 ) );
        ASSERT_TRUE( 0 != access.Seek( -1, SEEK_SET ) );
        UInt64 hitsBefore = access.GetCacheHits();
        ASSERT_TRUE( 0 == access.Setpos( 20 * ChunkSize ) && 10 == access.Read( bytes, 1, 10 ) && access.GetCacheHits() > hitsBefore );

        // a generic consumer of CIOAccess copies the whole resource; sequential reads go ahead in windows
        access.Setpos( 0 );
        CORE::CDynamicBuffer copy;
        CORE::CDynamicBufferAccess copyAccess( copy );
        UInt64 copied = copyAccess.Write( access );
        ASSERT_TRUE( size == copied && size == copy.GetDataSize() && BufferMatchesSynth( copy.GetConstBufferPtr(), copy.GetDataSize(), 0, 101 ) );
        ASSERT_TRUE( server.GetStats().opensServed == opensBefore + 1 && 1 == handle->GetOpenCount() );
        ASSERT_TRUE( handle->GetReadCount() < 18 + 41 );

        // the access lets go of its reference, the caller's keeps the handle; the last one closes it
        access.Close();
        ASSERT_TRUE( handle->IsOpen() );
        handle.Unlink();
        UInt64 waitStart = NowMs();
        while ( 0 != server.GetOpenHandleCount( requesterPeer ) && NowMs() - waitStart < 5000 )
            PumpFor( 5 );
        ASSERT_TRUE( 0 == server.GetOpenHandleCount( requesterPeer ) && 0 == requester.GetClientHandleCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: handles side by side, a limit of open resources (BUSY), an idle handle closed by the serving side and opened again, a corrupted chunk read again, and handles ended by a lost link" )
    try
    {
        CORE::CString root = TestDirectory( "handles2" );
        UInt64 size = 6 * ChunkSize + 3;
        for ( UInt32 i=0; i<3; ++i )
            REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "f" + CORE::ToString( i ) + ".bin" ), size, 110 + i ) );
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_handles2", "tcp" );
        group.nodeSettings.ownThread = true;
        REQUIRE_TRUE( StartPair( group ) );
        DRN::CDRNResourceSessionComponent& server = group[ 0 ].GetResources();
        DRN::CDRNResourceSessionComponent& requester = group[ 1 ].GetResources();
        REQUIRE_TRUE( server.RegisterProvider( "files", FileProvider( root ), ShareAll() ) );
        CCorruptingProvider* corrupting = GUCEF_NEW CCorruptingProvider( root, 2, 1 );
        REQUIRE_TRUE( server.RegisterProvider( "once", DRN::CIDRNResourceProviderPtr( corrupting ), ShareAll() ) );
        UInt32 serverPeer = LocalPeerOf( group[ 0 ] );
        UInt32 requesterPeer = LocalPeerOf( group[ 1 ] );
        DRN::CDRNResourceTransferSettings limits;
        limits.maxOpenHandlesPerPeer = 3;
        limits.handleIdleTimeoutMs = 300;
        server.SetSettings( limits );

        // three side by side, interleaved
        DRN::CDRNResourceHandlePtr handles[ 3 ];
        for ( UInt32 i=0; i<3; ++i )
            handles[ i ] = requester.OpenHandle( serverPeer, "files", "f" + CORE::ToString( i ) + ".bin" );
        for ( UInt32 i=0; i<3; ++i )
            REQUIRE_TRUE( handles[ i ]->WaitOpen( 20000 ) );
        ASSERT_TRUE( 3 == server.GetOpenHandleCount( requesterPeer ) );
        for ( UInt32 round=0; round<4; ++round )
        {
            for ( UInt32 i=0; i<3; ++i )
            {
                CORE::CDynamicBuffer data;
                UInt64 offset = round * ChunkSize + 11 * i;
                ASSERT_TRUE( handles[ i ]->ReadRange( offset, 3000, data ).IsOk() && BufferMatchesSynth( data.GetConstBufferPtr(), 3000, offset, 110 + i ) );
            }
        }

        // a fourth is refused
        DRN::CDRNResourceHandlePtr fourth = requester.OpenHandle( serverPeer, "once", "f0.bin", TreeOptions() );
        ASSERT_FALSE( fourth->WaitOpen( 20000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_BUSY == fourth->GetResult().status );
        handles[ 2 ].Unlink();
        UInt64 waitStart = NowMs();
        while ( 3 <= server.GetOpenHandleCount( requesterPeer ) && NowMs() - waitStart < 5000 )
            PumpFor( 5 );

        // the corrupted chunk: checked on the reading thread, read again
        fourth = requester.OpenHandle( serverPeer, "once", "f0.bin", TreeOptions() );
        REQUIRE_TRUE( fourth->WaitOpen( 20000 ) );
        CORE::CDynamicBuffer data;
        ASSERT_TRUE( fourth->ReadRange( 0, (UInt32) size, data ).IsOk() && BufferMatchesSynth( data.GetConstBufferPtr(), (UInt32) size, 0, 110 ) );
        ASSERT_TRUE( 1 == corrupting->m_corruptions );

        // idle: the serving side closes the handle, the next read opens it again for the same revision
        UInt64 expiredBefore = server.GetStats().handlesExpired;
        waitStart = NowMs();
        while ( server.GetStats().handlesExpired < expiredBefore + 2 && NowMs() - waitStart < 5000 )
            PumpFor( 20 );
        ASSERT_TRUE( server.GetStats().handlesExpired >= expiredBefore + 2 );
        ASSERT_TRUE( handles[ 0 ]->ReadRange( 100, 1000, data ).IsOk() && BufferMatchesSynth( data.GetConstBufferPtr(), 1000, 100, 110 ) );
        ASSERT_TRUE( 2 == handles[ 0 ]->GetOpenCount() );

        // the link goes: every handle ends with LINK_LOST, reads fail at once
        group.nodes[ 0 ]->Stop();
        waitStart = NowMs();
        while ( handles[ 1 ]->IsOpen() && NowMs() - waitStart < 10000 )
            PumpFor( 5 );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_LINK_LOST == handles[ 1 ]->GetResult().status );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_LINK_LOST == handles[ 0 ]->ReadRange( 0, 10, data ).status );
        ASSERT_TRUE( 0 == requester.GetClientHandleCount() );
        DRN::CDRNResourceIOAccess access( handles[ 1 ] );
        access.Open();
        ASSERT_FALSE( access.Opened() );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_LINK_LOST == access.GetLastResult().status );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resources: a subtree of the virtual file system is served through the vfs URI scheme" )
    try
    {
        CORE::CString root = TestDirectory( "vfs" );
        UInt64 size = 2 * ChunkSize + 333;
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( root, "v.bin" ), size, 67 ) );
        CORE::CreateDirs( CORE::CombinePath( root, "sub" ) );
        REQUIRE_TRUE( WriteSynthFile( CORE::CombinePath( CORE::CombinePath( root, "sub" ), "w.bin" ), 1000, 71 ) );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        REQUIRE_TRUE( vfs.AddRoot( root, "DrnResourceTest", false, false, "drnres" ) );

        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "res_vfs", "tcp" );
        REQUIRE_TRUE( StartPair( group ) );
        REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "vfs", DRN::CIDRNResourceProviderPtr( GUCEF_NEW DRN::CDRNUriResourceProvider( CORE::CUri( "vfs://drnres" ) ) ), ShareAll() ) );
        CORE::CString target = CORE::CombinePath( TestDirectory( "vfs_out" ), "v.bin" );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == FetchFile( group, "vfs", "v.bin", target, TreeOptions() ) );
        ASSERT_TRUE( FileMatchesSynth( target, size, 67 ) );
        CORE::CDynamicBuffer data;
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == ReadWindow( group, "vfs", "sub/w.bin", 10, 900, data, DRN::CDRNResourceTransferOptions() ) );
        ASSERT_TRUE( 900 == data.GetDataSize() && BufferMatchesSynth( data.GetConstBufferPtr(), 900, 10, 71 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == ReadWindow( group, "vfs", "nothere.bin", 0, 10, data, DRN::CDRNResourceTransferOptions() ) );
        vfs.UnmountArchiveByName( "DrnResourceTest" );

        // the same provider class over the "file" scheme
        REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "fileuri", DRN::CIDRNResourceProviderPtr( GUCEF_NEW DRN::CDRNUriResourceProvider( CORE::CUri( "file://" + root ) ) ), ShareAll() ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == ReadWindow( group, "fileuri", "sub/w.bin", 10, 900, data, TreeOptions() ) );
        ASSERT_TRUE( 900 == data.GetDataSize() && BufferMatchesSynth( data.GetConstBufferPtr(), 900, 10, 71 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    RemoveTestDirectories();
}

/*-------------------------------------------------------------------------*/
