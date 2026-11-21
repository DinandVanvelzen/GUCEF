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
#include <algorithm>
#include <stdio.h>

#include <exception>

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
#include <unistd.h>
#endif

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#include "gucefCORE_CNativeFileSystemAccess.h"
#include "gucefCORE_CTaskManager.h"
#include "gucefCORE_CIPulseGeneratorDriver.h"
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

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

#ifndef GUCEF_VFS_CVFSFILESYSTEMACCESS_H
#include "gucefVFS_CVfsFileSystemAccess.h"
#define GUCEF_VFS_CVFSFILESYSTEMACCESS_H
#endif /* GUCEF_VFS_CVFSFILESYSTEMACCESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDRNTestUtils.h"
#include "TestDRNGroup.h"
#include "TestDRNNamespace.h"

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
typedef CORE::CString::StringSet TStringSet;

/*-------------------------------------------------------------------------*/

CORE::CString
TestDirectory( const CORE::CString& name )
{
    CORE::CString dir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "drnns_test" );
    dir = CORE::CombinePath( dir, name );
    CORE::CreateDirs( dir );
    return dir;
}

/*-------------------------------------------------------------------------*/

void
RemoveTestDirectories( void )
{
    CORE::CString dir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "drnns_test" );
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
WriteFile( const CORE::CString& path, UInt32 size, UInt8 seed = 1 )
{
    CORE::CFileAccess file;
    if ( !file.Open( path, "wb" ) )
        return false;
    CORE::CDynamicBuffer block( size + 1, true );
    block.SetDataSize( size );
    for ( UInt32 i=0; i<size; ++i )
        static_cast< UInt8* >( block.GetBufferPtr() )[ i ] = (UInt8) ( seed + i * 7 );
    bool ok = 0 == size || size == file.Write( block.GetConstBufferPtr(), 1, size );
    file.Close();
    return ok;
}

/*-------------------------------------------------------------------------*/

/** A file whose name has characters CORE's path functions would take for separators: as the operating system takes them */
bool
WriteRawNamed( const CORE::CString& path, UInt32 size )
{
    FILE* file = ::fopen( path.C_String(), "wb" );
    if ( GUCEF_NULL == file )
        return false;
    for ( UInt32 i=0; i<size; ++i )
        ::fputc( (int) ( i & 0xFF ), file );
    ::fclose( file );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Removes the files WriteRawNamed() and the links of a test made, which CORE's DeleteDir cannot name */
void
RemoveOddFiles( const CORE::CString& root )
{
    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    const char* names[] = { "/bad:name.bin", "/back\\slash.bin", "/tab\there.bin", "/link.bin" };
    for ( UInt32 i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
        ::unlink( ( root + names[ i ] ).C_String() );
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::CString
Join( const CORE::CString& dir, const CORE::CString& name )
{
    return CORE::CombinePath( dir, name );
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessPtr
NativeFileSystem( void )
{
    return ( GUCEF_NEW CORE::CNativeFileSystemAccess() )->CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

DRN::CIDRNResourceProviderPtr
NsProvider( const CORE::CString& root, const DRN::CDRNNamespaceShareSettings& settings = DRN::CDRNNamespaceShareSettings() )
{
    return DRN::CIDRNResourceProviderPtr( GUCEF_NEW DRN::CDRNFileSystemAccessResourceProvider( NativeFileSystem(), root, CORE::CUri( "file://" + root ), settings ) );
}

/*-------------------------------------------------------------------------*/

DRN::CDRNResourceShareRules
ShareAll( void )
{
    return DRN::CDRNResourceShareRules( DRN::CDRNResourceShareRules::WholeProvider, DRN::CDRNResourceShareRules::AnyRole );
}

/*-------------------------------------------------------------------------*/

TStringSet
NamesOf( const TEntries& entries )
{
    TStringSet names;
    for ( UInt32 i=0; i<entries.size(); ++i )
        names.insert( entries[ i ].name );
    return names;
}

/*-------------------------------------------------------------------------*/

const DRN::CDRNNamespaceEntry*
FindEntry( const TEntries& entries, const CORE::CString& name )
{
    for ( UInt32 i=0; i<entries.size(); ++i )
        if ( entries[ i ].name == name )
            return &entries[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

/** The number in a name like f0123.bin */
UInt32
NumberOf( const CORE::CString& name )
{
    UInt32 number = 0;
    const char* text = name.C_String();
    for ( UInt32 i=1; text[ i ] >= '0' && text[ i ] <= '9'; ++i )
        number = number * 10 + (UInt32) ( text[ i ] - '0' );
    return number;
}

/*-------------------------------------------------------------------------*/

/** Reads the rest of a listing, so that it gives its place back */
void
FinishListing( DRN::CDRNNamespaceClient& client, const CORE::CString& share, const CORE::CString& dir, DRN::CDRNNamespaceListPage& page )
{
    UInt32 guard = 0;
    while ( !page.IsLast() && guard++ < 1000 )
    {
        CORE::CString cursor = page.nextCursor;
        if ( !client.List( share, dir, cursor, 0, CORE::CString::Empty, page ).IsOk() )
            return;
    }
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

/** A policy of the tests: hides, refuses and counts, and can sleep or call a client on the node's thread */
class CNsPolicy : public DRN::CDRNDefaultAccessPolicy
{
    public:

    TStringSet hidden;                  /**< entries MayListEntryAs() hides, by their name in the share */
    TStringSet readDenied;              /**< names MayReadResourceAs() refuses */
    TStringSet browseDenied;            /**< names MayBrowseResourceAs() refuses */
    bool browseIgnoresRead;             /**< MayBrowseResourceAs() decides by the share rules alone: names may be seen that cannot be read */
    UInt32 browseCalls;
    UInt32 sleepOnBrowseCall;           /**< the call (counted from 1) that sleeps, 0 = none */
    CORE::CString sleepName;            /**< when not empty, the calls counted are those for this name */
    UInt32 namedCalls;
    UInt32 sleepMs;
    DRN::CDRNNamespaceClient* nestedClient;
    DRN::CDRNNamespaceResult nestedResult;

    CNsPolicy( void ) : DRN::CDRNDefaultAccessPolicy(), hidden(), readDenied(), browseDenied(), browseIgnoresRead( false ), browseCalls( 0 ),
                        sleepOnBrowseCall( 0 ), sleepName(), namedCalls( 0 ), sleepMs( 0 ), nestedClient( GUCEF_NULL ), nestedResult() {}

    virtual bool MayReadResourceAs( UInt32 /* peerId */, const DRN::CDRNAuthResult& /* auth */, const CORE::CString& /* providerName */,
                                    const CORE::CString& resourceName, bool sharedByProvider ) GUCEF_VIRTUAL_OVERRIDE
    {
        return sharedByProvider && readDenied.find( resourceName ) == readDenied.end();
    }

    virtual bool MayBrowseResourceAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName,
                                      bool sharedByProvider ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++browseCalls;
        if ( !sleepName.IsNULLOrEmpty() )
        {
            if ( resourceName == sleepName && 0 != sleepOnBrowseCall && ++namedCalls == sleepOnBrowseCall )
                MT::PrecisionDelay( sleepMs );
        }
        else
        if ( 0 != sleepOnBrowseCall && browseCalls == sleepOnBrowseCall )
            MT::PrecisionDelay( sleepMs );
        if ( GUCEF_NULL != nestedClient )
        {
            GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
            nestedResult = nestedClient->Shares( shares, 500 );
        }
        if ( browseDenied.find( resourceName ) != browseDenied.end() )
            return false;
        if ( browseIgnoresRead )
            return sharedByProvider;
        return MayReadResourceAs( peerId, auth, providerName, resourceName, sharedByProvider );
    }

    virtual bool MayListEntryAs( UInt32 /* peerId */, const DRN::CDRNAuthResult& /* auth */, const CORE::CString& /* providerName */,
                                 const CORE::CString& entryName, bool /* isDirectory */ ) GUCEF_VIRTUAL_OVERRIDE
    {
        return hidden.find( entryName ) == hidden.end();
    }
};

/*-------------------------------------------------------------------------*/

/** A policy of the tests that gives a reason for one refusal of a read and none for another */
class CReasonPolicy : public DRN::CDRNDefaultAccessPolicy
{
    public:

    CReasonPolicy( void ) : DRN::CDRNDefaultAccessPolicy() {}

    virtual bool CheckReadResourceAs( UInt32 /* peerId */, const DRN::CDRNAuthResult& /* auth */, const CORE::CString& /* providerName */, const CORE::CString& resourceName,
                                      bool sharedByProvider, CORE::CString& refusal ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( "locked.bin" == resourceName )
        {
            refusal = "this file is for the owner of the share";
            return false;
        }
        return sharedByProvider && "plain.bin" != resourceName;
    }
};

/*-------------------------------------------------------------------------*/

/** A policy of the tests that answers "ask again" for one file until it is told the data is ready, and refuses another for good */
class CRetryPolicy : public DRN::CDRNDefaultAccessPolicy
{
    public:

    MT::CTAtomic< UInt32 > askedBusy;
    MT::CTAtomic< bool > ready;

    CRetryPolicy( void ) : DRN::CDRNDefaultAccessPolicy(), askedBusy( 0 ), ready( false ) {}

    virtual bool DecideReadResourceAs( UInt32 /* peerId */, const DRN::CDRNAuthResult& /* auth */, const CORE::CString& /* providerName */, const CORE::CString& resourceName,
                                       bool sharedByProvider, CORE::CString& refusal, bool& retryLater ) GUCEF_VIRTUAL_OVERRIDE
    {
        retryLater = false;
        if ( "locked.bin" == resourceName )
        {
            refusal = "this file is for the owner of the share";
            return false;
        }
        if ( "warming.bin" == resourceName && !ready.Load() )
        {
            askedBusy.FetchAdd( 1 );
            refusal = "the data of this file is being got ready";
            retryLater = true;
            return false;
        }
        return sharedByProvider;
    }
};

/*-------------------------------------------------------------------------*/

/** A share whose files have a digest the provider knows, and which counts the reads (nothing may be read to describe) */
class CKnownShaProvider : public DRN::CDRNFileSystemAccessResourceProvider
{
    public:

    MT::CMutex m_lock;
    UInt32 m_reads;

    CKnownShaProvider( const CORE::CString& root )
        : DRN::CDRNFileSystemAccessResourceProvider( NativeFileSystem(), root, CORE::CUri( "file://" + root ) ), m_lock(), m_reads( 0 ) {}

    virtual DRN::TDRNResourceStatus Read( const CORE::CString& name, const DRN::CDRNResourceInfo& resolved, UInt64 offset, UInt32 size, void* destination, UInt32& bytesRead ) GUCEF_VIRTUAL_OVERRIDE
    {
        {
            MT::CScopeMutex lock( m_lock );
            ++m_reads;
        }
        return DRN::CDRNFileSystemAccessResourceProvider::Read( name, resolved, offset, size, destination, bytesRead );
    }

    UInt32 GetReads( void )
    {
        MT::CScopeMutex lock( m_lock );
        return m_reads;
    }

    static void Digest( UInt8* digest )
    {
        for ( UInt32 i=0; i<32; ++i )
            digest[ i ] = (UInt8) ( 0xA0 + i );
    }

    protected:

    virtual bool GetKnownSha256( const CORE::CString& name, const DRN::CDRNNamespaceEntry& /* entry */, UInt8* digest ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( name.EndsWith( "known.bin" ) )
        {
            Digest( digest );
            return true;
        }
        return false;
    }
};

/*-------------------------------------------------------------------------*/

/** A share whose STAT takes its time */
class CSlowProvider : public DRN::CDRNFileSystemAccessResourceProvider
{
    public:

    UInt32 m_delayMs;

    CSlowProvider( const CORE::CString& root, UInt32 delayMs )
        : DRN::CDRNFileSystemAccessResourceProvider( NativeFileSystem(), root, CORE::CUri( "file://" + root ) ), m_delayMs( delayMs ) {}

    virtual DRN::TDRNResourceStatus Stat( const CORE::CString& name, DRN::CDRNNamespaceEntry& entry ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::PrecisionDelay( m_delayMs );
        return DRN::CDRNFileSystemAccessResourceProvider::Stat( name, entry );
    }
};

/*-------------------------------------------------------------------------*/

/** A pair of nodes of a session with their own threads, where the host serves and the member browses */
class CNsFixture
{
    public:

    CGroup group;
    UInt32 serverPeer;

    explicit CNsFixture( const CORE::CString& sessionName, const CORE::CString& channel = "tcp" )
        : group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, sessionName, channel ), serverPeer( 0 )
    {
        group.nodeSettings.ownThread = true;
    }

    bool Start( void )
    {
        if ( !group.Start( 2 ) )
            return false;
        serverPeer = LocalPeerOf( group[ 0 ] );
        return true;
    }

    DRN::CDRNResourceSessionComponent& Server( void ) { return group[ 0 ].GetResources(); }
    DRN::CDRNResourceSessionComponent& Member( void ) { return group[ 1 ].GetResources(); }
};

/*-------------------------------------------------------------------------*/

/** A host with token authentication and members that join with a role of their own */
class CRoleFixture
{
    public:

    DRN::CDRNNode hostNode;
    DRN::CDRNSession* host;
    DRN::CDRNEndpoint endpoint;
    DRN::CDRNSessionSettings sessionSettings;
    GUCEF::vector< DRN::CDRNNode* > memberNodes;
    GUCEF::vector< DRN::CDRNSession* > members;

    CRoleFixture( void ) : hostNode(), host( GUCEF_NULL ), endpoint(), sessionSettings(), memberNodes(), members() {}

    ~CRoleFixture()
    {
        for ( UInt32 i=0; i<memberNodes.size(); ++i )
        {
            memberNodes[ i ]->Stop();
            GUCEF_DELETE memberNodes[ i ];
        }
        hostNode.SetAccessPolicy( GUCEF_NULL );
        hostNode.Stop();
    }

    static DRN::CDRNNodeSettings NodeSettings( void )
    {
        DRN::CDRNNodeSettings settings;
        settings.echoIntervalMs = 20;
        settings.handshakeTimeoutMs = 2000;
        settings.connectTimeoutMs = 2000;
        settings.pulseIntervalMs = 1;
        settings.ownThread = true;
        return settings;
    }

    bool Start( void )
    {
        DRN::CDRNNodeSettings hostSettings = NodeSettings();
        hostSettings.auth.authenticatorType = "token";
        hostSettings.auth.SetSetting( "key", "ns key" );
        if ( !hostNode.SetSettings( hostSettings ) )
            return false;
        endpoint = LoopbackEndpoint( "tcp", NextTestPort() );
        if ( !hostNode.Listen( endpoint ) )
            return false;
        host = hostNode.CreateSession();
        sessionSettings.sessionName = "ns_roles";
        return host->Host( sessionSettings );
    }

    /** A member with this role string (several roles comma separated); its index */
    Int32 AddMember( const char* role )
    {
        DRN::CDRNNode* node = GUCEF_NEW DRN::CDRNNode();
        memberNodes.push_back( node );
        if ( !node->SetSettings( NodeSettings() ) )
            return -1;
        DRN::CDRNSession* session = node->CreateSession();
        members.push_back( session );
        DRN::CDRNAuthSettings credentials;
        credentials.authenticatorType = "token";
        credentials.SetSetting( "token", DRN::CDRNTokenAuthenticator::IssueToken( "ns key", "ns_roles", "user", role, DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + 600 ) );
        if ( !session->Join( DRN::CDRNEndpointList( endpoint ), sessionSettings, &credentials ) )
            return -1;
        UInt64 start = DRN::CDRNClock::NowInMilliseconds();
        while ( DRN::CDRNClock::NowInMilliseconds() - start < 8000 )
        {
            PumpFor( 5 );
            MT::CScopeMutex lock( session->GetNode().GetLock() );
            if ( session->IsActive() && session->GetMemberCount() >= members.size() + 1 )
                return (Int32) members.size() - 1;
        }
        return -1;
    }
};

/*-------------------------------------------------------------------------*/

/** What a forged namespace request of the member came back with */
class CForged
{
    public:

    DRN::TDRNResourceStatus status;
    CORE::CString message;
    CORE::CDynamicBuffer body;
    CForged( void ) : status( DRN::DRN_RESSTATUS_TIMEOUT ), message(), body() {}
};

/** Sends a hand made request, whose body (after the request id) the caller wrote, and waits for its answer */
CForged
Forge( CNsFixture& fixture, UInt32 opcode, const CORE::CDynamicBuffer& afterId, UInt32 timeoutMs = 5000 )
{
    CForged forged;
    UInt32 id = fixture.Member().GetNextNamespaceRequestId();
    CORE::CDynamicBuffer body;
    DRN::CDRNWireWriter writer( body );
    writer.WriteVarUInt32( id );
    if ( afterId.GetDataSize() > 0 )
        writer.WriteBytes( afterId.GetConstBufferPtr(), afterId.GetDataSize() );
    forged.status = fixture.Member().CallNamespace( fixture.serverPeer, opcode, id, body, timeoutMs, forged.message, forged.body );
    return forged;
}

/** A STAT of one path that does not go through the client, so that the checks of the client are not what refuses it */
DRN::TDRNResourceStatus
ForgeStatOne( CNsFixture& fixture, const CORE::CString& share, const CORE::CString& path )
{
    UInt32 id = fixture.Member().GetNextNamespaceRequestId();
    CORE::CDynamicBuffer body;
    DRN::CDRNWireWriter writer( body );
    DRN::CDRNNamespaceCodec::WriteStatRequestHeader( writer, id, share, 1 );
    DRN::CDRNNamespaceCodec::WriteStatRequestItem( writer, path, CORE::CString::Empty );
    CORE::CString message;
    CORE::CDynamicBuffer answer;
    DRN::TDRNResourceStatus status = fixture.Member().CallNamespace( fixture.serverPeer, DRN::DRN_RES_NS_STAT, id, body, 5000, message, answer );
    if ( DRN::DRN_RESSTATUS_OK != status )
        return status;
    DRN::CDRNWireReader reader( answer );
    UInt32 count = 0;
    DRN::CDRNNamespaceEntry entry;
    if ( !DRN::CDRNNamespaceCodec::ReadCount( reader, 1, 1, count ) || 1 != count || !DRN::CDRNNamespaceCodec::ReadStatItem( reader, entry ) )
        return DRN::DRN_RESSTATUS_PROTOCOL_ERROR;
    return entry.status;
}

DRN::TDRNResourceStatus
ForgeListOne( CNsFixture& fixture, const CORE::CString& share, const CORE::CString& dir, const CORE::CString& cursor = CORE::CString::Empty, UInt32 maxEntries = 0 )
{
    UInt32 id = fixture.Member().GetNextNamespaceRequestId();
    CORE::CDynamicBuffer body;
    DRN::CDRNWireWriter writer( body );
    DRN::CDRNNamespaceCodec::WriteListRequest( writer, id, share, dir, cursor, maxEntries, CORE::CString::Empty );
    CORE::CString message;
    CORE::CDynamicBuffer answer;
    return fixture.Member().CallNamespace( fixture.serverPeer, DRN::DRN_RES_NS_LIST, id, body, 5000, message, answer );
}

/*-------------------------------------------------------------------------*/

/** A directory with a number of files and some structure */
void
MakeTree( const CORE::CString& root, UInt32 bigCount )
{
    REQUIRE_TRUE( WriteFile( Join( root, "a.bin" ), 1000, 3 ) );
    REQUIRE_TRUE( WriteFile( Join( root, "empty.bin" ), 0 ) );
    REQUIRE_TRUE( WriteFile( Join( root, "known.bin" ), 70000, 5 ) );
    CORE::CreateDirs( Join( root, "sub" ) );
    REQUIRE_TRUE( WriteFile( Join( Join( root, "sub" ), "x.bin" ), 10, 7 ) );
    REQUIRE_TRUE( WriteFile( Join( Join( root, "sub" ), "y.bin" ), 20, 9 ) );
    CORE::CreateDirs( Join( root, "sub/deep/er" ) );
    REQUIRE_TRUE( WriteFile( Join( root, "sub/deep/er/z.bin" ), 30, 11 ) );
    CORE::CreateDirs( Join( root, "emptydir" ) );
    CORE::CString big = Join( root, "big" );
    CORE::CreateDirs( big );
    for ( UInt32 i=0; i<bigCount; ++i )
    {
        CORE::CString number = CORE::ToString( i );
        while ( number.Length() < 4 )
            number = "0" + number;
        REQUIRE_TRUE( WriteFile( Join( big, "f" + number + ".bin" ), i % 50, (UInt8) i ) );
    }
}

/*-------------------------------------------------------------------------*/

/** Records the calling thread as the thread that drives the application's pulse generator, as the main loop of an application does */
class CPumpThreadScope
{
    public:

    explicit CPumpThreadScope( TThreadId driverThreadId = MT::GetCurrentThreadID() )
        : m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() ), m_previous( m_pulseGenerator->GetPulseDriverThreadId() )
    {
        m_pulseGenerator->SetPulseDriverThreadId( driverThreadId );
    }

    ~CPumpThreadScope()
    {
        m_pulseGenerator->SetPulseDriverThreadId( m_previous );
    }

    private:

    CPumpThreadScope( const CPumpThreadScope& src );
    CPumpThreadScope& operator=( const CPumpThreadScope& src );

    CORE::PulseGeneratorPtr m_pulseGenerator;
    TThreadId m_previous;
};

/*-------------------------------------------------------------------------*/

/** The condition of a test that is never met */
class CNeverCondition : public DRN::CIDRNWaitCondition
{
    public:
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return false; }
};

/*-------------------------------------------------------------------------*/

/** What a pumped node's session is made of: the session of the serving member waits for the clients */
class CPumpMembership : public CTestCondition
{
    public:

    CPumpMembership( DRN::CDRNSession& a, DRN::CDRNSession& b ) : m_a( a ), m_b( b ) {}

    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        return IsComplete( m_a ) && IsComplete( m_b );
    }

    private:

    static bool IsComplete( DRN::CDRNSession& session )
    {
        MT::CScopeMutex lock( session.GetNode().GetLock() );
        return session.IsActive() && 2 == session.GetMemberCount();
    }

    CPumpMembership& operator=( const CPumpMembership& src );
    DRN::CDRNSession& m_a;
    DRN::CDRNSession& m_b;
};

/*-------------------------------------------------------------------------*/

/**
 *  A serving node on a thread of its own and a client node that has none: the thread of the test is the one that drives
 *  the application's pulse generator, so it pumps the client node (CPumpThreadScope), and makes the calls
 */
class CPumpFixture
{
    public:

    DRN::CDRNNode serverNode;
    DRN::CDRNNode clientNode;
    DRN::CDRNSession* server;
    DRN::CDRNSession* client;
    DRN::CDRNEndpoint endpoint;
    DRN::CDRNSessionSettings sessionSettings;
    UInt32 serverPeer;
    bool clientOwnThread;
    bool serverOwnThread;

    explicit CPumpFixture( const CORE::CString& sessionName, bool ownThreadForClient = false, bool ownThreadForServer = true )
        : serverNode(), clientNode(), server( GUCEF_NULL ), client( GUCEF_NULL ), endpoint(), sessionSettings(), serverPeer( 0 ), clientOwnThread( ownThreadForClient ), serverOwnThread( ownThreadForServer )
    {
        sessionSettings.sessionName = sessionName;
        sessionSettings.topology = DRN::DRN_TOPOLOGY_CLIENT_SERVER;
    }

    ~CPumpFixture()
    {
        clientNode.Stop();
        serverNode.Stop();
    }

    DRN::CDRNNodeSettings NodeSettings( bool ownThread ) const
    {
        DRN::CDRNNodeSettings settings;
        settings.echoIntervalMs = 50;
        settings.pulseIntervalMs = 1;
        settings.ownThread = ownThread;
        return settings;
    }

    /** Starts the serving node and makes the client session, joined when 'join' */
    bool Start( bool join = true )
    {
        if ( !serverNode.SetSettings( NodeSettings( serverOwnThread ) ) || !clientNode.SetSettings( NodeSettings( clientOwnThread ) ) )
            return false;
        endpoint = LoopbackEndpoint( "tcp", NextTestPort() );
        if ( !serverNode.Listen( endpoint ) )
            return false;
        server = serverNode.CreateSession();
        client = clientNode.CreateSession();
        if ( !server->Host( sessionSettings ) )
            return false;
        {
            MT::CScopeMutex lock( serverNode.GetLock() );
            serverPeer = server->GetLocalPeerId();
        }
        return !join || Join();
    }

    bool Join( void )
    {
        DRN::CDRNSessionSettings joinSettings;
        joinSettings.sessionName = sessionSettings.sessionName;
        if ( !client->Join( DRN::CDRNEndpointList( endpoint ), joinSettings ) )
            return false;
        CPumpMembership condition( *server, *client );
        return PumpUntil( condition, 15000 );
    }

    DRN::CDRNResourceSessionComponent& Server( void ) { return server->GetResources(); }
    DRN::CDRNResourceSessionComponent& Member( void ) { return client->GetResources(); }
};

/*-------------------------------------------------------------------------*/

/** A call made from a handler the client node delivers, which is work of the node */
class CInHandlerProbe : public CORE::CObserver
{
    public:

    DRN::CDRNNamespaceClient* client;
    DRN::CDRNSession* session;
    UInt32 peerId;
    UInt32 calls;
    bool working;
    bool pumpThread;
    DRN::CDRNNamespaceResult namespaceResult;
    DRN::CDRNResourceTransferResult readResult;
    DRN::CDRNResourceTransferResult openResult;
    bool pumpUntilMet;
    UInt64 pumpUntilMs;
    UInt64 namespaceMs;

    CInHandlerProbe( void )
        : CORE::CObserver(), client( GUCEF_NULL ), session( GUCEF_NULL ), peerId( 0 ), calls( 0 ), working( false ), pumpThread( true )
        , namespaceResult(), readResult(), openResult(), pumpUntilMet( true ), pumpUntilMs( 0 ), namespaceMs( 0 )
    {}

    virtual ~CInHandlerProbe() { SignalUpcomingObserverDestruction(); }

    protected:

    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* /* eventdata */ = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( DRN::CDRNNode::LinkOperationalEvent != eventid || GUCEF_NULL == client || 0 != calls )
            return;
        ++calls;
        DRN::CDRNNode& node = session->GetNode();
        working = node.IsWorkingOnThisThread();
        pumpThread = node.IsPumpThread();
        UInt64 start = DRN::CDRNClock::NowInMilliseconds();
        DRN::CDRNNamespaceInfo info;
        namespaceResult = client->Negotiate( info, 3000 );
        namespaceMs = DRN::CDRNClock::NowInMilliseconds() - start;
        CORE::CDynamicBuffer data;
        readResult = session->GetResources().ReadRange( peerId, "files", "a.bin", 0, 10, data, DRN::CDRNResourceTransferOptions(), 3000 );
        CNeverCondition never;
        start = DRN::CDRNClock::NowInMilliseconds();
        pumpUntilMet = node.PumpUntil( never, 1500 );
        pumpUntilMs = DRN::CDRNClock::NowInMilliseconds() - start;
    }
};

/*-------------------------------------------------------------------------*/

/** Makes a blocking call on a thread of its own: not the thread that drives the application's pulse generator */
class CForeignCaller : public MT::CActiveObject
{
    public:

    CForeignCaller( DRN::CDRNNode& node, DRN::CDRNNamespaceClient& client )
        : MT::CActiveObject(), m_node( node ), m_client( client ), m_done( false ), m_pumpThread( true ), m_code( (UInt32) DRN::DRN_NSRESULT_OK ), m_elapsedMs( 0 )
    {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_pumpThread.Store( m_node.IsPumpThread() );
        UInt64 start = DRN::CDRNClock::NowInMilliseconds();
        GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
        m_code.Store( (UInt32) m_client.Shares( shares, 400 ).code );
        m_elapsedMs.Store( DRN::CDRNClock::NowInMilliseconds() - start );
        m_done.Store( true );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool IsDone( void ) const { return m_done.Load(); }
    bool WasPumpThread( void ) const { return m_pumpThread.Load(); }
    UInt32 GetCode( void ) const { return m_code.Load(); }
    UInt64 GetElapsedMs( void ) const { return m_elapsedMs.Load(); }

    private:

    DRN::CDRNNode& m_node;
    DRN::CDRNNamespaceClient& m_client;
    MT::CTAtomic< bool > m_done;
    MT::CTAtomic< bool > m_pumpThread;
    MT::CTAtomic< UInt32 > m_code;
    MT::CTAtomic< UInt64 > m_elapsedMs;
};

/*-------------------------------------------------------------------------*/

/** Whether bytes are the content WriteFile() gave a file of that seed, from an offset */
bool
MatchesPattern( const void* data, UInt32 size, UInt64 offset, UInt8 seed )
{
    const UInt8* bytes = static_cast< const UInt8* >( data );
    for ( UInt32 i=0; i<size; ++i )
        if ( bytes[ i ] != (UInt8) ( seed + ( offset + i ) * 7 ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** The options of the calls of a test that must not wait for the default of half a minute when a site is not assisted */
DRN::CDRNResourceTransferOptions
ShortOptions( UInt32 timeoutMs )
{
    DRN::CDRNResourceTransferOptions options;
    options.openTimeoutMs = timeoutMs;
    options.idleTimeoutMs = timeoutMs;
    return options;
}

/** A pulse generator driver that counts the immediate pulse requests it gets, and how many of those came when the watched future was ready */
class CWakeCountingDriver : public CORE::CIPulseGeneratorDriver
{
    public:

    MT::CTAtomic< UInt32 > wakes;
    MT::CTAtomic< UInt32 > wakesWhileReady;
    MT::CTAtomic< const CORE::CFutureResult* > watched;
    MT::CTAtomic< TThreadId > driverThreadId;       /**< the thread that drives the pulse generator, as a main loop reports it; 0 for none */

    CWakeCountingDriver( void ) : CORE::CIPulseGeneratorDriver(), wakes( 0 ), wakesWhileReady( 0 ), watched( GUCEF_NULL ), driverThreadId( 0 ) {}
    virtual ~CWakeCountingDriver() {}

    virtual void RequestImmediatePulse( CORE::CPulseGenerator& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CORE::CFutureResult* future = watched.Load();
        if ( GUCEF_NULL != future && future->IsReady() )
            wakesWhileReady.FetchAdd( 1 );
        wakes.FetchAdd( 1 );
    }
    virtual void RequestPulsesPerImmediatePulseRequest( CORE::CPulseGenerator&, const Int32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void RequestPeriodicPulses( CORE::CPulseGenerator&, const UInt32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void RequestPulseInterval( CORE::CPulseGenerator&, const UInt32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void RequestStopOfPeriodicUpdates( CORE::CPulseGenerator& ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual TThreadId GetPulseDriverThreadId( void ) const GUCEF_VIRTUAL_OVERRIDE { return driverThreadId.Load(); }
};

/*-------------------------------------------------------------------------*/

/** A job of the pool that waits at a gate until the test opens it, then counts itself */
class CWakeProbeJob : public DRN::CIDRNResourceIoJob
{
    public:

    MT::CTAtomic< UInt32 > gateOpen;
    MT::CTAtomic< UInt32 > executed;

    CWakeProbeJob( void ) : DRN::CIDRNResourceIoJob(), gateOpen( 0 ), executed( 0 ) {}
    virtual ~CWakeProbeJob() {}

    virtual void Execute( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 waited=0; waited<5000 && 0 == gateOpen.Load(); ++waited )
            MT::ThreadDelay( 1 );
        executed.FetchAdd( 1 );
    }
};

/** Counts the pulses of a pulse generator: the cycles of a node, which pulses its work pulse generator once per cycle */
class CCycleCounter : public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt32 > cycles;

    CCycleCounter( void ) : CORE::CObserver(), cycles( 0 ) {}
    virtual ~CCycleCounter() { UnsubscribeAllFromObserver(); }

    void Watch( CORE::PulseGeneratorPtr pulseGenerator )
    {
        CORE::CTEventHandlerFunctor< CCycleCounter > callback( this, &CCycleCounter::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, callback );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CCycleCounter";
        return classTypeName;
    }

    private:

    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        cycles.FetchAdd( 1 );
    }
};

/**
 *  The thread that pumps a serving node that has no thread of its own, as the main loop of an application does: it works the
 *  node (CDRNNode::PumpOnce()) until it is stopped. The application's pulse generator of the node has a driver that counts the
 *  immediate pulse requests, which is what a main loop with a wake-up of its own would answer
 */
class CNodePumpThread : public MT::CActiveObject
{
    public:

    CNodePumpThread( DRN::CDRNNode& node, CWakeCountingDriver& driver ) : MT::CActiveObject(), m_node( node ), m_driver( driver ) {}
    virtual ~CNodePumpThread() {}

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_driver.driverThreadId.Store( MT::GetCurrentThreadID() );
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_node.PumpOnce();
        return false;
    }

    private:

    DRN::CDRNNode& m_node;
    CWakeCountingDriver& m_driver;
};

/*-------------------------------------------------------------------------*/

/**
 *  Two nodes, each with an application pulse generator of its own whose driver counts the immediate pulse requests. The serving
 *  node has no thread of its own and is pumped by a thread of its own; the client node has a thread of its own, or is pumped the
 *  same way when asked for
 */
class CWakePumpFixture
{
    public:

    CWakeCountingDriver driver;                 /**< of the serving node */
    CWakeCountingDriver clientDriver;           /**< of the client node */
    CORE::PulseGeneratorPtr appPulseGenerator;
    CORE::PulseGeneratorPtr clientAppPulseGenerator;
    DRN::CDRNNode serverNode;
    DRN::CDRNNode clientNode;
    DRN::CDRNSession* server;
    DRN::CDRNSession* client;
    DRN::CDRNEndpoint endpoint;
    UInt32 serverPeer;
    CNodePumpThread* pump;
    CNodePumpThread* clientPump;
    bool pumpClient;

    explicit CWakePumpFixture( const CORE::CString& sessionName, bool pumpTheClient = false )
        : driver()
        , clientDriver()
        , appPulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
        , clientAppPulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
        , serverNode( appPulseGenerator )
        , clientNode( clientAppPulseGenerator )
        , server( GUCEF_NULL ), client( GUCEF_NULL ), endpoint(), serverPeer( 0 ), pump( GUCEF_NULL ), clientPump( GUCEF_NULL ), pumpClient( pumpTheClient )
        , m_sessionName( sessionName )
    {
        appPulseGenerator->SetPulseGeneratorDriver( &driver );
        clientAppPulseGenerator->SetPulseGeneratorDriver( &clientDriver );
        driver.wakes.Store( 0 );
        clientDriver.wakes.Store( 0 );
    }

    ~CWakePumpFixture()
    {
        if ( GUCEF_NULL != clientPump )
        {
            clientPump->Deactivate( false, true );
            GUCEF_DELETE clientPump;
        }
        if ( GUCEF_NULL != pump )
        {
            pump->Deactivate( false, true );
            GUCEF_DELETE pump;
        }
        clientNode.Stop();
        serverNode.Stop();
        clientAppPulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
        appPulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
    }

    static bool StartPump( CNodePumpThread*& thread, DRN::CDRNNode& node, CWakeCountingDriver& nodeDriver )
    {
        thread = GUCEF_NEW CNodePumpThread( node, nodeDriver );
        if ( !thread->Activate( GUCEF_NULL, 0, 0 ) )
            return false;
        for ( UInt32 i=0; i<1000 && 0 == nodeDriver.GetPulseDriverThreadId(); ++i )
            MT::ThreadDelay( 1 );
        return 0 != nodeDriver.GetPulseDriverThreadId();
    }

    bool Start( void )
    {
        DRN::CDRNNodeSettings serverSettings;
        serverSettings.echoIntervalMs = 50;
        serverSettings.pulseIntervalMs = 1;
        serverSettings.ownThread = false;
        DRN::CDRNNodeSettings clientSettings = serverSettings;
        clientSettings.ownThread = !pumpClient;
        if ( !serverNode.SetSettings( serverSettings ) || !clientNode.SetSettings( clientSettings ) )
            return false;
        if ( !StartPump( pump, serverNode, driver ) )
            return false;
        if ( pumpClient && !StartPump( clientPump, clientNode, clientDriver ) )
            return false;
        endpoint = LoopbackEndpoint( "tcp", NextTestPort() );
        if ( !serverNode.Listen( endpoint ) )
            return false;
        server = serverNode.CreateSession();
        client = clientNode.CreateSession();
        DRN::CDRNSessionSettings sessionSettings;
        sessionSettings.sessionName = m_sessionName;
        sessionSettings.topology = DRN::DRN_TOPOLOGY_CLIENT_SERVER;
        if ( !server->Host( sessionSettings ) )
            return false;
        {
            MT::CScopeMutex lock( serverNode.GetLock() );
            serverPeer = server->GetLocalPeerId();
        }
        if ( !client->Join( DRN::CDRNEndpointList( endpoint ), sessionSettings ) )
            return false;
        CPumpMembership condition( *server, *client );
        return PumpUntil( condition, 15000 );
    }

    DRN::CDRNResourceSessionComponent& Server( void ) { return server->GetResources(); }
    DRN::CDRNResourceSessionComponent& Member( void ) { return client->GetResources(); }

    private:

    CORE::CString m_sessionName;
};

/** Asks the pulse generator it watches for an immediate pulse through its driver from within every pulse of it, as work of the node that is pulsed does */
class CCycleRequester : public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt32 > requests;

    explicit CCycleRequester( const CORE::PulseGeneratorPtr& pulseGenerator ) : CORE::CObserver(), requests( 0 ), m_pulseGenerator( pulseGenerator )
    {
        CORE::CTEventHandlerFunctor< CCycleRequester > callback( this, &CCycleRequester::OnPulse );
        SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, callback );
    }

    virtual ~CCycleRequester() { UnsubscribeAllFromObserver(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CCycleRequester";
        return classTypeName;
    }

    private:

    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        requests.FetchAdd( 1 );
        m_pulseGenerator->RequestImmediatePulseThroughDriver();
    }

    CORE::PulseGeneratorPtr m_pulseGenerator;
};

}; /* anonymous namespace */


/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNNamespaceTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DRN NAMESPACE TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.Namespace" );

    RemoveTestDirectories();

    GUCEF_TESTFW_TESTCASE( "Namespace: the codec writes and reads every message, refuses anything cut short, any entry that could climb out of a directory and counts that no bytes can hold" )
    try
    {
        {
            CORE::CDynamicBuffer buffer;
            DRN::CDRNWireWriter writer( buffer );
            DRN::CDRNNamespaceCodec::WriteListRequest( writer, 77, "share", "dir/x", "12.3", 50, "rev" );
            UInt32 id = 0, maxEntries = 0;
            CORE::CString share, dir, cursor, revision;
            DRN::CDRNWireReader reader( buffer );
            REQUIRE_TRUE( DRN::CDRNNamespaceCodec::ReadListRequest( reader, id, share, dir, cursor, maxEntries, revision ) );
            ASSERT_TRUE( 77 == id && "share" == share && "dir/x" == dir && "12.3" == cursor && 50 == maxEntries && "rev" == revision && 0 == reader.GetRemaining() );
            for ( UInt32 cut=0; cut<buffer.GetDataSize(); ++cut )
            {
                DRN::CDRNWireReader shortReader( buffer.GetConstBufferPtr(), cut );
                ASSERT_FALSE( DRN::CDRNNamespaceCodec::ReadListRequest( shortReader, id, share, dir, cursor, maxEntries, revision ) );
            }
        }
        {
            CORE::CDynamicBuffer buffer;
            DRN::CDRNWireWriter writer( buffer );
            DRN::CDRNNamespaceCodec::WriteStatRequestHeader( writer, 5, "s", 2 );
            DRN::CDRNNamespaceCodec::WriteStatRequestItem( writer, "a/b", "r1" );
            DRN::CDRNNamespaceCodec::WriteStatRequestItem( writer, "", "" );
            DRN::CDRNWireReader reader( buffer );
            UInt32 id = 0, count = 0;
            CORE::CString share, path, revision;
            REQUIRE_TRUE( DRN::CDRNNamespaceCodec::ReadStatRequestHeader( reader, id, share, count ) && 5 == id && "s" == share && 2 == count );
            REQUIRE_TRUE( DRN::CDRNNamespaceCodec::ReadStatRequestItem( reader, path, revision ) && "a/b" == path && "r1" == revision );
            REQUIRE_TRUE( DRN::CDRNNamespaceCodec::ReadStatRequestItem( reader, path, revision ) && path.IsNULLOrEmpty() && revision.IsNULLOrEmpty() );
            ASSERT_FALSE( DRN::CDRNNamespaceCodec::ReadStatRequestItem( reader, path, revision ) );
        }
        {
            CORE::CDynamicBuffer buffer;
            DRN::CDRNWireWriter writer( buffer );
            DRN::CDRNNamespaceCodec::WriteHelloRequest( writer, 1, 4, 9 );
            DRN::CDRNNamespaceInfo info;
            info.version = 1; info.capabilities = 7; info.maxStatPaths = 11; info.maxPageEntries = 22; info.maxAnswerBytes = 33333; info.listingLifetimeMs = 44;
            DRN::CDRNNamespaceShareInfo share;
            share.name = "pack"; share.capabilities = 7; share.flags = 1; share.maxStatPaths = 3; share.maxPageEntries = 5;
            DRN::CDRNNamespaceCodec::WriteShare( writer, share );
            // the hello body is the last of an answer: what follows its six numbers is the extension
            DRN::CDRNNamespaceCodec::WriteHelloBody( writer, info );
            DRN::CDRNWireReader reader( buffer );
            UInt32 id = 0, version = 0, caps = 0;
            REQUIRE_TRUE( DRN::CDRNNamespaceCodec::ReadHelloRequest( reader, id, version, caps ) && 1 == id && 4 == version && 9 == caps );
            DRN::CDRNNamespaceShareInfo readShare;
            REQUIRE_TRUE( DRN::CDRNNamespaceCodec::ReadShare( reader, readShare ) );
            DRN::CDRNNamespaceInfo read;
            REQUIRE_TRUE( DRN::CDRNNamespaceCodec::ReadHelloBody( reader, read ) );
            ASSERT_TRUE( 1 == read.version && 7 == read.capabilities && 11 == read.maxStatPaths && 22 == read.maxPageEntries && 33333 == read.maxAnswerBytes && 44 == read.listingLifetimeMs );
            ASSERT_FALSE( read.limitsAnnounced );
            ASSERT_TRUE( "pack" == readShare.name && 7 == readShare.capabilities && 1 == readShare.flags && 3 == readShare.maxStatPaths && 5 == readShare.maxPageEntries );
            ASSERT_TRUE( readShare.CanList() && readShare.CanStat() && readShare.IsReadOnly() );
        }
        {
            // an entry with its digest, and the entries a client must not take
            DRN::CDRNNamespaceEntry entry;
            entry.name = "n\xC3\xA9.bin";
            entry.kind = DRN::DRN_NS_KIND_FILE;
            entry.size = (UInt64) 5 * 1024 * 1024 * 1024 + 3;
            entry.modifiedUtcMs = 1700000000123ULL;
            entry.flags = DRN::DRN_NS_ENTRYFLAG_READ_ONLY;
            entry.revision = "r-9";
            entry.hasSha256 = true;
            for ( UInt32 i=0; i<32; ++i )
                entry.sha256[ i ] = (UInt8) i;
            CORE::CDynamicBuffer buffer;
            DRN::CDRNWireWriter writer( buffer );
            DRN::CDRNNamespaceCodec::WriteListEntry( writer, entry );
            DRN::CDRNWireReader reader( buffer );
            DRN::CDRNNamespaceEntry read;
            REQUIRE_TRUE( DRN::CDRNNamespaceCodec::ReadListEntry( reader, read ) );
            ASSERT_TRUE( read.name == entry.name && read.IsFile() && read.size == entry.size && read.modifiedUtcMs == entry.modifiedUtcMs && "r-9" == read.revision );
            ASSERT_TRUE( read.hasSha256 && 0 == memcmp( read.sha256, entry.sha256, 32 ) && 0 != ( read.flags & DRN::DRN_NS_ENTRYFLAG_SHA256 ) && 0 != ( read.flags & DRN::DRN_NS_ENTRYFLAG_READ_ONLY ) );
            for ( UInt32 cut=0; cut<buffer.GetDataSize(); ++cut )
            {
                DRN::CDRNWireReader shortReader( buffer.GetConstBufferPtr(), cut );
                DRN::CDRNNamespaceEntry cutEntry;
                ASSERT_FALSE( DRN::CDRNNamespaceCodec::ReadListEntry( shortReader, cutEntry ) );
            }
            const char* badNames[] = { "..", "../x", "a/b", "/abs", "a\\b", "c:", "a\tb", "." };
            for ( UInt32 i=0; i<sizeof( badNames ) / sizeof( badNames[ 0 ] ); ++i )
            {
                DRN::CDRNNamespaceEntry bad( entry );
                bad.name = badNames[ i ];
                CORE::CDynamicBuffer badBuffer;
                DRN::CDRNWireWriter badWriter( badBuffer );
                DRN::CDRNNamespaceCodec::WriteListEntry( badWriter, bad );
                DRN::CDRNWireReader badReader( badBuffer );
                DRN::CDRNNamespaceEntry readBad;
                ASSERT_FALSE( DRN::CDRNNamespaceCodec::ReadListEntry( badReader, readBad ) );
            }
            DRN::CDRNNamespaceEntry kindBad( entry );
            kindBad.kind = 9;
            CORE::CDynamicBuffer kindBuffer;
            DRN::CDRNWireWriter kindWriter( kindBuffer );
            DRN::CDRNNamespaceCodec::WriteListEntry( kindWriter, kindBad );
            DRN::CDRNWireReader kindReader( kindBuffer );
            DRN::CDRNNamespaceEntry readKind;
            ASSERT_FALSE( DRN::CDRNNamespaceCodec::ReadListEntry( kindReader, readKind ) );
            DRN::CDRNNamespaceEntry longRevision( entry );
            for ( UInt32 i=0; i<400; ++i )
                longRevision.revision += "r";
            CORE::CDynamicBuffer revBuffer;
            DRN::CDRNWireWriter revWriter( revBuffer );
            DRN::CDRNNamespaceCodec::WriteListEntry( revWriter, longRevision );
            DRN::CDRNWireReader revReader( revBuffer );
            DRN::CDRNNamespaceEntry readRev;
            ASSERT_FALSE( DRN::CDRNNamespaceCodec::ReadListEntry( revReader, readRev ) );
        }
        {
            // a count that the bytes left cannot hold is refused before anything is made of it
            CORE::CDynamicBuffer buffer;
            DRN::CDRNWireWriter writer( buffer );
            writer.WriteVarUInt32( 4000000 );
            writer.WriteUInt8( 1 );
            DRN::CDRNWireReader reader( buffer );
            UInt32 count = 0;
            ASSERT_FALSE( DRN::CDRNNamespaceCodec::ReadCount( reader, 1, 0xFFFFFF, count ) );
            CORE::CDynamicBuffer ok;
            DRN::CDRNWireWriter okWriter( ok );
            okWriter.WriteVarUInt32( 2 );
            okWriter.WriteUInt8( 1 );
            okWriter.WriteUInt8( 1 );
            DRN::CDRNWireReader okReader( ok );
            ASSERT_TRUE( DRN::CDRNNamespaceCodec::ReadCount( okReader, 1, 10, count ) && 2 == count );
            DRN::CDRNWireReader limitReader( ok );
            ASSERT_FALSE( DRN::CDRNNamespaceCodec::ReadCount( limitReader, 1, 1, count ) );

            // a status of the future is an error of the serving side, not a crash
            CORE::CDynamicBuffer answer;
            DRN::CDRNWireWriter answerWriter( answer );
            answerWriter.WriteVarUInt32( 3 );
            answerWriter.WriteVarUInt32( DRN::DRN_RES_NS_STAT );
            answerWriter.WriteVarUInt32( 999 );
            DRN::CDRNNamespaceCodec::WriteText( answerWriter, "x" );
            DRN::CDRNWireReader answerReader( answer );
            UInt32 id = 0, kind = 0;
            DRN::TDRNResourceStatus status = DRN::DRN_RESSTATUS_OK;
            CORE::CString message;
            REQUIRE_TRUE( DRN::CDRNNamespaceCodec::ReadAnswerHeader( answerReader, id, kind, status, message ) );
            ASSERT_TRUE( 3 == id && DRN::DRN_RES_NS_STAT == kind && DRN::DRN_RESSTATUS_IO_ERROR == status && "x" == message );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: share rules say what may be browsed: the root and the way to a shared subtree, what lies in it, nothing beside it; roles may be several" )
    try
    {
        DRN::CDRNResourceShareRules rules( "maps/pub", "viewer" );
        ASSERT_TRUE( rules.AllowsBrowsingDirectory( "" ) && rules.AllowsBrowsingDirectory( "maps" ) && rules.AllowsBrowsingDirectory( "maps/pub" ) && rules.AllowsBrowsingDirectory( "maps/pub/x" ) );
        ASSERT_FALSE( rules.AllowsBrowsingDirectory( "maps/priv" ) || rules.AllowsBrowsingDirectory( "map" ) || rules.AllowsBrowsingDirectory( "other" ) || rules.AllowsBrowsingDirectory( "maps/pubx" ) );
        ASSERT_TRUE( rules.ShowsEntry( "maps", true ) && rules.ShowsEntry( "maps/pub", true ) && rules.ShowsEntry( "maps/pub/a.bin", false ) );
        ASSERT_FALSE( rules.ShowsEntry( "maps", false ) || rules.ShowsEntry( "maps/priv", true ) || rules.ShowsEntry( "maps/a.bin", false ) || rules.ShowsEntry( "../maps", true ) );
        ASSERT_TRUE( rules.AllowsBrowsing( "viewer", "" ) && rules.AllowsBrowsing( "x,viewer", "maps" ) && !rules.AllowsBrowsing( "x,y", "maps" ) );
        ASSERT_FALSE( DRN::CDRNResourceShareRules().AllowsBrowsingDirectory( "" ) );
        DRN::CDRNResourceShareRules whole( DRN::CDRNResourceShareRules::WholeProvider, "*" );
        ASSERT_TRUE( whole.AllowsBrowsing( "", "" ) && whole.AllowsBrowsing( "a,b", "x/y" ) && whole.ShowsEntry( "x/y/z.bin", false ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: hello negotiates version and limits; shares, stat and list answer through the client, blocking and as futures" )
    try
    {
        CORE::CString root = TestDirectory( "main" );
        MakeTree( root, 20 );
        CNsFixture fixture( "ns_main" );
        REQUIRE_TRUE( fixture.Start() );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );

        DRN::CDRNNamespaceInfo none;
        ASSERT_FALSE( client.GetNegotiatedInfo( none ) );
        DRN::CDRNNamespaceInfo info;
        DRN::CDRNNamespaceResult result = client.Negotiate( info );
        REQUIRE_TRUE( result.IsOk() );
        ASSERT_TRUE( 1 == info.version && 256 == info.maxStatPaths && 1000 == info.maxPageEntries && 512 * 1024 == info.maxAnswerBytes && 60000 == info.listingLifetimeMs );
        ASSERT_TRUE( 0 != ( info.capabilities & DRN::DRN_NS_CAP_LIST ) && 0 != ( info.capabilities & DRN::DRN_NS_CAP_STAT ) && 0 != ( info.capabilities & DRN::DRN_NS_CAP_READ ) );
        ASSERT_TRUE( 0 == ( info.capabilities & ( DRN::DRN_NS_CAP_WRITE | DRN::DRN_NS_CAP_CREATE | DRN::DRN_NS_CAP_DELETE | DRN::DRN_NS_CAP_RENAME | DRN::DRN_NS_CAP_MKDIR ) ) );
        DRN::CDRNNamespaceInfo cached;
        ASSERT_TRUE( client.GetNegotiatedInfo( cached ) && cached.maxStatPaths == info.maxStatPaths );

        GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
        result = client.Shares( shares );
        REQUIRE_TRUE( result.IsOk() && 1 == shares.size() );
        ASSERT_TRUE( "files" == shares[ 0 ].name && shares[ 0 ].CanList() && shares[ 0 ].CanStat() && shares[ 0 ].IsReadOnly() && 0 == shares[ 0 ].maxStatPaths );

        // stat: files, directories, the root, what is not there, what is not a name
        TStrings paths;
        paths.push_back( "a.bin" ); paths.push_back( "empty.bin" ); paths.push_back( "sub" ); paths.push_back( "" ); paths.push_back( "nothere.bin" );
        paths.push_back( "sub/x.bin" ); paths.push_back( "../x" ); paths.push_back( "sub/deep/er" );
        TEntries stats;
        result = client.Stat( "files", paths, TStrings(), stats );
        REQUIRE_TRUE( result.IsOk() && paths.size() == stats.size() );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == stats[ 0 ].status && stats[ 0 ].IsFile() && 1000 == stats[ 0 ].size && "a.bin" == stats[ 0 ].name && stats[ 0 ].modifiedUtcMs > 1000000000000ULL );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == stats[ 1 ].status && 0 == stats[ 1 ].size && stats[ 1 ].IsFile() );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == stats[ 2 ].status && stats[ 2 ].IsDirectory() && 0 == stats[ 2 ].size );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == stats[ 3 ].status && stats[ 3 ].IsDirectory() && stats[ 3 ].name.IsNULLOrEmpty() );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == stats[ 4 ].status );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == stats[ 5 ].status && 10 == stats[ 5 ].size );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == stats[ 6 ].status );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == stats[ 7 ].status && stats[ 7 ].IsDirectory() );
        ASSERT_TRUE( stats[ 0 ].revision == DRN::CDRNResourceInfo::MakeRevision( 1000, stats[ 0 ].modifiedUtcMs ) );

        // list: one directory, with the metadata of every entry
        DRN::CDRNNamespaceListPage page;
        result = client.List( "files", "sub", CORE::CString::Empty, 0, CORE::CString::Empty, page );
            REQUIRE_TRUE( result.IsOk() && page.IsLast() && 3 == page.entries.size() );
        TStringSet subNames = NamesOf( page.entries );
        ASSERT_TRUE( subNames.count( "x.bin" ) && subNames.count( "y.bin" ) && subNames.count( "deep" ) );
        const DRN::CDRNNamespaceEntry* y = FindEntry( page.entries, "y.bin" );
        ASSERT_TRUE( GUCEF_NULL != y && 20 == y->size && y->IsFile() );
        ASSERT_TRUE( FindEntry( page.entries, "deep" )->IsDirectory() );
        result = client.List( "files", "a.bin", CORE::CString::Empty, 0, CORE::CString::Empty, page );
        ASSERT_TRUE( DRN::DRN_NSRESULT_NOT_A_DIRECTORY == result.code && page.entries.empty() );
        result = client.List( "files", "nothere", CORE::CString::Empty, 0, CORE::CString::Empty, page );
        ASSERT_TRUE( DRN::DRN_NSRESULT_NOT_FOUND == result.code );
        result = client.List( "files", "emptydir", CORE::CString::Empty, 0, CORE::CString::Empty, page );
        ASSERT_TRUE( result.IsOk() && page.entries.empty() && page.IsLast() );
        result = client.Stat( "nothere", paths, TStrings(), stats );
        ASSERT_TRUE( DRN::DRN_NSRESULT_NO_SUCH_SHARE == result.code );
        result = client.List( "nothere", "", CORE::CString::Empty, 0, CORE::CString::Empty, page );
        ASSERT_TRUE( DRN::DRN_NSRESULT_NO_SUCH_SHARE == result.code );

        // the same as futures
        DRN::CDRNNamespaceResponsePtr negotiated, sharesResponse, statResponse, listResponse, allResponse;
        CORE::CFutureResult f1 = client.NegotiateAsync( negotiated );
        CORE::CFutureResult f2 = client.SharesAsync( sharesResponse );
        CORE::CFutureResult f3 = client.StatAsync( "files", paths, TStrings(), statResponse );
        CORE::CFutureResult f4 = client.ListAsync( "files", "sub", CORE::CString::Empty, 0, CORE::CString::Empty, listResponse );
        CORE::CFutureResult f5 = client.ListAllAsync( "files", "big", allResponse );
        f1.Await( 15000 ); f2.Await( 15000 ); f3.Await( 15000 ); f4.Await( 15000 ); f5.Await( 15000 );
        ASSERT_TRUE( negotiated->result.IsOk() && 256 == negotiated->info.maxStatPaths );
        ASSERT_TRUE( sharesResponse->result.IsOk() && 1 == sharesResponse->shares.size() );
        ASSERT_TRUE( statResponse->result.IsOk() && paths.size() == statResponse->entries.size() && 1000 == statResponse->entries[ 0 ].size );
        ASSERT_TRUE( listResponse->result.IsOk() && 3 == listResponse->page.entries.size() );
        ASSERT_TRUE( allResponse->result.IsOk() && 20 == allResponse->entries.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a directory of several hundred entries is listed in pages by cursor, the pages follow each other without repeating, listings side by side do not mix, the listing ends with the last page" )
    try
    {
        CORE::CString root = TestDirectory( "paging" );
        MakeTree( root, 730 );
        CNsFixture fixture( "ns_paging" );
        REQUIRE_TRUE( fixture.Start() );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );

        TStringSet seen;
        CORE::CString cursor;
        UInt32 pages = 0;
        UInt32 total = 0;
        DRN::CDRNNamespaceListPage page;
        CORE::CString firstRevision;
        while ( true )
        {
            DRN::CDRNNamespaceResult result = client.List( "files", "big", cursor, 100, CORE::CString::Empty, page );
                REQUIRE_TRUE( result.IsOk() );
            ++pages;
            if ( 1 == pages )
            {
                firstRevision = page.dirRevision;
                ASSERT_FALSE( page.IsLast() );
                ASSERT_TRUE( 1 == fixture.Server().GetNamespaceListingCount( 0 ) );
            }
            ASSERT_TRUE( page.entries.size() <= 100 );
            ASSERT_TRUE( page.dirRevision == firstRevision );
            for ( UInt32 i=0; i<page.entries.size(); ++i )
            {
                ASSERT_TRUE( seen.insert( page.entries[ i ].name ).second );
                ASSERT_TRUE( page.entries[ i ].IsFile() && page.entries[ i ].name.StartsWith( "f" ) && page.entries[ i ].name.EndsWith( ".bin" ) );
                UInt32 number = NumberOf( page.entries[ i ].name );
                ASSERT_TRUE( number < 730 && page.entries[ i ].size == number % 50 );
            }
            total += (UInt32) page.entries.size();
            if ( page.IsLast() )
                break;
            cursor = page.nextCursor;
        }
        ASSERT_TRUE( 730 == total && 730 == seen.size() && 8 == pages );

        // the listing is gone with its last page
        ASSERT_TRUE( 0 == fixture.Server().GetNamespaceListingCount( 0 ) );

        // two listings at the same time, each with its own cursor
        DRN::CDRNNamespaceListPage a, b;
        REQUIRE_TRUE( client.List( "files", "big", CORE::CString::Empty, 10, CORE::CString::Empty, a ).IsOk() );
        REQUIRE_TRUE( client.List( "files", "sub", CORE::CString::Empty, 1, CORE::CString::Empty, b ).IsOk() );
        ASSERT_TRUE( 2 == fixture.Server().GetNamespaceListingCount( 0 ) && 1 == b.entries.size() && !b.IsLast() );
        DRN::CDRNNamespaceListPage a2, b2;
        REQUIRE_TRUE( client.List( "files", "big", a.nextCursor, 10, CORE::CString::Empty, a2 ).IsOk() );
        REQUIRE_TRUE( client.List( "files", "sub", b.nextCursor, 10, CORE::CString::Empty, b2 ).IsOk() );
        ASSERT_TRUE( 10 == a2.entries.size() && 2 == b2.entries.size() && b2.IsLast() );
        TStringSet firstTwenty = NamesOf( a.entries );
        for ( UInt32 i=0; i<a2.entries.size(); ++i )
            ASSERT_TRUE( firstTwenty.insert( a2.entries[ i ].name ).second );

        // a cursor is the next page only once, of the listing it was made for, in the directory it was made for
        DRN::CDRNNamespaceListPage again;
        ASSERT_TRUE( DRN::DRN_NSRESULT_CURSOR_EXPIRED == client.List( "files", "big", a.nextCursor, 10, CORE::CString::Empty, again ).code );
        ASSERT_TRUE( DRN::DRN_NSRESULT_CURSOR_EXPIRED == client.List( "files", "sub", a2.nextCursor, 10, CORE::CString::Empty, again ).code );
        ASSERT_TRUE( DRN::DRN_NSRESULT_INTEGRITY == client.List( "files", "big", "not a cursor", 10, CORE::CString::Empty, again ).code );
        ASSERT_TRUE( DRN::DRN_NSRESULT_CURSOR_EXPIRED == client.List( "files", "big", "99999.1", 10, CORE::CString::Empty, again ).code );

        // everything at once by following the cursors, with a bound
        TEntries all;
        CORE::CString revision;
        bool changed = true;
        DRN::CDRNNamespaceResult listAll = client.ListAll( "files", "big", all, revision, changed, 1000, 64 );
        ASSERT_TRUE( listAll.IsOk() && 730 == all.size() && !changed && revision == firstRevision );
        TEntries bounded;
        listAll = client.ListAll( "files", "big", bounded, revision, changed, 300, 100 );
        ASSERT_TRUE( DRN::DRN_NSRESULT_TOO_LARGE == listAll.code && 300 == bounded.size() );
        ASSERT_TRUE( 3 >= fixture.Server().GetNamespaceListingCount( 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a listing nobody continues is forgotten after its lifetime and the cursor then restarts cleanly; a listing that expires while ListAll reads it is started over" )
    try
    {
        CORE::CString root = TestDirectory( "expiry" );
        MakeTree( root, 300 );
        CNsFixture fixture( "ns_expiry" );
        REQUIRE_TRUE( fixture.Start() );
        DRN::CDRNResourceTransferSettings settings;
        settings.namespaceListingLifetimeMs = 300;
        fixture.Server().SetSettings( settings );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );

        DRN::CDRNNamespaceListPage page;
        REQUIRE_TRUE( client.List( "files", "big", CORE::CString::Empty, 50, CORE::CString::Empty, page ).IsOk() );
        CORE::CString stale = page.nextCursor;
        ASSERT_TRUE( 1 == fixture.Server().GetNamespaceListingCount( 0 ) );
        PumpFor( 800 );
        ASSERT_TRUE( 0 == fixture.Server().GetNamespaceListingCount( 0 ) );
        DRN::CDRNNamespaceResult result = client.List( "files", "big", stale, 50, CORE::CString::Empty, page );
        ASSERT_TRUE( DRN::DRN_NSRESULT_CURSOR_EXPIRED == result.code && page.entries.empty() );
        ASSERT_TRUE( fixture.Server().GetStats().namespaceListingsExpired >= 1 );

        // started over from the first page, it reads on
        REQUIRE_TRUE( client.List( "files", "big", CORE::CString::Empty, 50, CORE::CString::Empty, page ).IsOk() );
        ASSERT_TRUE( 50 == page.entries.size() );

        // the second page of the next listing is asked for after the lifetime: the policy of the serving side holds up that request
        CNsPolicy policy;
        policy.sleepName = "big";
        policy.sleepOnBrowseCall = 2;          // the request of the second page of the first listing
        policy.sleepMs = 450;
        fixture.group.nodes[ 0 ]->SetAccessPolicy( &policy );
        TEntries all;
        CORE::CString revision;
        bool changed = false;
        result = client.ListAll( "files", "big", all, revision, changed, 1000, 100, 2 );
        ASSERT_TRUE( result.IsOk() && 300 == all.size() );
        ASSERT_TRUE( NamesOf( all ).size() == 300 );
        ASSERT_TRUE( policy.browseCalls >= 5 );

        // without restarts allowed it gives up with the code of the listing that went
        policy.namedCalls = 0;
        result = client.ListAll( "files", "big", all, revision, changed, 1000, 100, 0 );
        ASSERT_TRUE( DRN::DRN_NSRESULT_CURSOR_EXPIRED == result.code );
        fixture.group.nodes[ 0 ]->SetAccessPolicy( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: the revision of an entry in STAT and LIST is the one OPEN checks; a changed file has another and the old one fails the open" )
    try
    {
        CORE::CString root = TestDirectory( "revision" );
        MakeTree( root, 5 );
        CNsFixture fixture( "ns_revision" );
        REQUIRE_TRUE( fixture.Start() );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );

        TEntries stats;
        REQUIRE_TRUE( client.Stat( "files", Strings1( "sub/x.bin" ), TStrings(), stats ).IsOk() && 1 == stats.size() );
        DRN::CDRNNamespaceListPage page;
        REQUIRE_TRUE( client.List( "files", "sub", CORE::CString::Empty, 0, CORE::CString::Empty, page ).IsOk() );
        const DRN::CDRNNamespaceEntry* listed = FindEntry( page.entries, "x.bin" );
        REQUIRE_TRUE( GUCEF_NULL != listed );
        ASSERT_TRUE( listed->revision == stats[ 0 ].revision && listed->size == stats[ 0 ].size && listed->modifiedUtcMs == stats[ 0 ].modifiedUtcMs );

        DRN::CDRNResourceHandlePtr handle = fixture.Member().OpenHandle( fixture.serverPeer, "files", "sub/x.bin", DRN::CDRNResourceTransferOptions() );
        REQUIRE_TRUE( handle->WaitOpen( 10000 ) );
        DRN::CDRNResourceInfo opened = handle->GetInfo();
        ASSERT_TRUE( opened.revision == stats[ 0 ].revision && opened.size == stats[ 0 ].size && opened.modifiedUtcMs == stats[ 0 ].modifiedUtcMs );
        handle->Close();

        // the same for a file that was listed in a directory of many files and one in the root
        TEntries bigStats;
        TStrings bigPaths;
        bigPaths.push_back( "big/f0003.bin" ); bigPaths.push_back( "a.bin" ); bigPaths.push_back( "known.bin" );
        REQUIRE_TRUE( client.Stat( "files", bigPaths, TStrings(), bigStats ).IsOk() );
        TEntries bigList;
        CORE::CString rev;
        bool changedFlag = false;
        REQUIRE_TRUE( client.ListAll( "files", "big", bigList, rev, changedFlag ).IsOk() );
        ASSERT_TRUE( bigList.size() == 5 && FindEntry( bigList, "f0003.bin" )->revision == bigStats[ 0 ].revision );
        for ( UInt32 i=0; i<bigPaths.size(); ++i )
        {
            DRN::CDRNResourceHandlePtr h = fixture.Member().OpenHandle( fixture.serverPeer, "files", bigPaths[ i ], DRN::CDRNResourceTransferOptions() );
            REQUIRE_TRUE( h->WaitOpen( 10000 ) );
            ASSERT_TRUE( h->GetInfo().revision == bigStats[ i ].revision );
            h->Close();
        }

        // the revision a caller holds is passed to the open: the file as it is now is accepted; after a change the old revision fails
        DRN::CDRNResourceTransferOptions expect;
        expect.expectedRevision = stats[ 0 ].revision;
        DRN::CDRNResourceHandlePtr good = fixture.Member().OpenHandle( fixture.serverPeer, "files", "sub/x.bin", expect );
        ASSERT_TRUE( good->WaitOpen( 10000 ) );
        good->Close();
        MT::PrecisionDelay( 30 );
        REQUIRE_TRUE( WriteFile( Join( Join( root, "sub" ), "x.bin" ), 11, 7 ) );
        DRN::CDRNResourceHandlePtr changed = fixture.Member().OpenHandle( fixture.serverPeer, "files", "sub/x.bin", expect );
        ASSERT_FALSE( changed->WaitOpen( 10000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_CHANGED == changed->GetResult().status );
        TEntries after;
        REQUIRE_TRUE( client.Stat( "files", Strings1( "sub/x.bin" ), TStrings(), after ).IsOk() );
        ASSERT_TRUE( after[ 0 ].revision != stats[ 0 ].revision && 11 == after[ 0 ].size );
        DRN::CDRNResourceTransferOptions fresh;
        fresh.expectedRevision = after[ 0 ].revision;
        DRN::CDRNResourceHandlePtr now = fixture.Member().OpenHandle( fixture.serverPeer, "files", "sub/x.bin", fresh );
        ASSERT_TRUE( now->WaitOpen( 10000 ) );
        now->Close();

        // revisions the caller holds: STAT and LIST answer UNCHANGED without a body
        TStrings known;
        known.push_back( after[ 0 ].revision );
        known.push_back( "other" );
        TStrings two;
        two.push_back( "sub/x.bin" );
        two.push_back( "sub/y.bin" );
        TEntries conditional;
        REQUIRE_TRUE( client.Stat( "files", two, known, conditional ).IsOk() && 2 == conditional.size() );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_UNCHANGED == conditional[ 0 ].status && DRN::DRN_RESSTATUS_OK == conditional[ 1 ].status && 20 == conditional[ 1 ].size );

        DRN::CDRNNamespaceListPage current, unchanged;
        REQUIRE_TRUE( client.List( "files", "sub", CORE::CString::Empty, 0, CORE::CString::Empty, current ).IsOk() );
        ASSERT_FALSE( current.dirRevision.IsNULLOrEmpty() );
        DRN::CDRNNamespaceResult unchangedResult = client.List( "files", "sub", CORE::CString::Empty, 0, current.dirRevision, unchanged );
        ASSERT_TRUE( unchangedResult.IsUnchanged() && unchanged.unchanged && unchanged.entries.empty() && unchanged.dirRevision == current.dirRevision );
        MT::PrecisionDelay( 30 );
        REQUIRE_TRUE( WriteFile( Join( Join( root, "sub" ), "new.bin" ), 5, 1 ) );
        DRN::CDRNNamespaceListPage changedPage;
        REQUIRE_TRUE( client.List( "files", "sub", CORE::CString::Empty, 0, current.dirRevision, changedPage ).IsOk() );
        ASSERT_TRUE( !changedPage.unchanged && 4 == changedPage.entries.size() && changedPage.dirRevision != current.dirRevision && GUCEF_NULL != FindEntry( changedPage.entries, "new.bin" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a digest is reported only where the provider knew it already, nothing is read to describe a file, and the open reports the same digest" )
    try
    {
        CORE::CString root = TestDirectory( "sha" );
        MakeTree( root, 3 );
        CNsFixture fixture( "ns_sha" );
        REQUIRE_TRUE( fixture.Start() );
        CKnownShaProvider* provider = GUCEF_NEW CKnownShaProvider( root );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", DRN::CIDRNResourceProviderPtr( provider ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );

        TStrings paths;
        paths.push_back( "known.bin" ); paths.push_back( "a.bin" ); paths.push_back( "sub" );
        TEntries stats;
        REQUIRE_TRUE( client.Stat( "files", paths, TStrings(), stats ).IsOk() );
        UInt8 digest[ 32 ];
        CKnownShaProvider::Digest( digest );
        ASSERT_TRUE( stats[ 0 ].hasSha256 && 0 == memcmp( stats[ 0 ].sha256, digest, 32 ) && !stats[ 1 ].hasSha256 && !stats[ 2 ].hasSha256 );
        TEntries all;
        CORE::CString rev;
        bool flag = false;
        REQUIRE_TRUE( client.ListAll( "files", "", all, rev, flag ).IsOk() );
        ASSERT_TRUE( FindEntry( all, "known.bin" )->hasSha256 && !FindEntry( all, "a.bin" )->hasSha256 && !FindEntry( all, "sub" )->hasSha256 );
        ASSERT_TRUE( 0 == provider->GetReads() );

        DRN::CDRNResourceHandlePtr handle = fixture.Member().OpenHandle( fixture.serverPeer, "files", "known.bin", DRN::CDRNResourceTransferOptions() );
        REQUIRE_TRUE( handle->WaitOpen( 10000 ) );
        ASSERT_TRUE( handle->GetInfo().hasSha256 && 0 == memcmp( handle->GetInfo().sha256, digest, 32 ) );
        handle->Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: shares are filtered by the role of the member: one role, several roles in one member, every role, no role, nobody; what a role may not browse is forbidden" )
    try
    {
        CORE::CString root = TestDirectory( "roles" );
        MakeTree( root, 3 );
        CRoleFixture fixture;
        REQUIRE_TRUE( fixture.Start() );
        DRN::CDRNResourceSessionComponent& server = fixture.host->GetResources();
        REQUIRE_TRUE( server.RegisterProvider( "viewers", NsProvider( root ), DRN::CDRNResourceShareRules( DRN::CDRNResourceShareRules::WholeProvider, "viewer" ) ) );
        REQUIRE_TRUE( server.RegisterProvider( "editors", NsProvider( root ), DRN::CDRNResourceShareRules( DRN::CDRNResourceShareRules::WholeProvider, "editor" ) ) );
        REQUIRE_TRUE( server.RegisterProvider( "everyone", NsProvider( root ), ShareAll() ) );
        REQUIRE_TRUE( server.RegisterProvider( "nobody", NsProvider( root ), DRN::CDRNResourceShareRules() ) );
        DRN::CDRNResourceShareRules multi( DRN::CDRNResourceShareRules::WholeProvider, "viewer" );
        multi.AllowRole( "admin" );
        REQUIRE_TRUE( server.RegisterProvider( "multi", NsProvider( root ), multi ) );
        REQUIRE_TRUE( server.RegisterProvider( "bytesonly", DRN::CIDRNResourceProviderPtr( GUCEF_NEW DRN::CDRNFileSystemResourceProvider( root ) ), ShareAll() ) );

        const char* roles[] = { "viewer", "viewer,editor", "other", "admin" };
        const char* expected[] = { "bytesonly everyone multi viewers", "bytesonly editors everyone multi viewers", "bytesonly everyone", "bytesonly everyone multi" };
        for ( UInt32 r=0; r<4; ++r )
        {
            Int32 member = fixture.AddMember( roles[ r ] );
            REQUIRE_TRUE( member >= 0 );
            DRN::CDRNNamespaceClient client( *fixture.members[ member ], LocalPeerOf( *fixture.host ) );
            GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
            REQUIRE_TRUE( client.Shares( shares ).IsOk() );
            CORE::CString names;
            for ( UInt32 i=0; i<shares.size(); ++i )
                names += ( i > 0 ? " " : "" ) + shares[ i ].name;
            ASSERT_TRUE( names == expected[ r ] );
            for ( UInt32 i=0; i<shares.size(); ++i )
            {
                // a byte provider is read from only, a share with a namespace can be browsed
                ASSERT_TRUE( 0 != ( shares[ i ].capabilities & DRN::DRN_NS_CAP_READ ) );
                ASSERT_TRUE( ( "bytesonly" == shares[ i ].name ) == !shares[ i ].CanList() );
            }

            DRN::CDRNNamespaceListPage page;
            DRN::CDRNNamespaceResult viewers = client.List( "viewers", "", CORE::CString::Empty, 0, CORE::CString::Empty, page );
            DRN::CDRNNamespaceResult editors = client.List( "editors", "sub", CORE::CString::Empty, 0, CORE::CString::Empty, page );
            DRN::CDRNNamespaceResult everyone = client.List( "everyone", "", CORE::CString::Empty, 0, CORE::CString::Empty, page );
            DRN::CDRNNamespaceResult nobody = client.List( "nobody", "", CORE::CString::Empty, 0, CORE::CString::Empty, page );
            DRN::CDRNNamespaceResult bytes = client.List( "bytesonly", "", CORE::CString::Empty, 0, CORE::CString::Empty, page );
            bool isViewer = 0 == r || 1 == r;
            bool isEditor = 1 == r;
            ASSERT_TRUE( isViewer == viewers.IsOk() && ( isViewer || DRN::DRN_NSRESULT_ACCESS_DENIED == viewers.code ) );
            ASSERT_TRUE( isEditor == editors.IsOk() && ( isEditor || DRN::DRN_NSRESULT_ACCESS_DENIED == editors.code ) );
            ASSERT_TRUE( everyone.IsOk() );
            ASSERT_TRUE( DRN::DRN_NSRESULT_ACCESS_DENIED == nobody.code );
            ASSERT_TRUE( DRN::DRN_NSRESULT_UNSUPPORTED == bytes.code );
            TEntries stats;
            REQUIRE_TRUE( client.Stat( "viewers", Strings1( "a.bin" ), TStrings(), stats ).IsOk() && 1 == stats.size() );
            ASSERT_TRUE( ( isViewer ? DRN::DRN_RESSTATUS_OK : DRN::DRN_RESSTATUS_FORBIDDEN ) == stats[ 0 ].status );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a share of a subtree shows the way to it and what is in it, nothing beside it; names that are not shared are forbidden before they are resolved" )
    try
    {
        CORE::CString root = TestDirectory( "subtree" );
        CORE::CreateDirs( Join( root, "maps/pub/inner" ) );
        CORE::CreateDirs( Join( root, "maps/priv" ) );
        CORE::CreateDirs( Join( root, "mapsx" ) );
        REQUIRE_TRUE( WriteFile( Join( root, "maps/pub/a.bin" ), 10 ) );
        REQUIRE_TRUE( WriteFile( Join( root, "maps/pub/inner/b.bin" ), 11 ) );
        REQUIRE_TRUE( WriteFile( Join( root, "maps/priv/c.bin" ), 12 ) );
        REQUIRE_TRUE( WriteFile( Join( root, "maps/loose.bin" ), 13 ) );
        REQUIRE_TRUE( WriteFile( Join( root, "top.bin" ), 14 ) );
        CNsFixture fixture( "ns_subtree" );
        REQUIRE_TRUE( fixture.Start() );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "pub", NsProvider( root ), DRN::CDRNResourceShareRules( "maps/pub", "*" ) ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );

        TEntries entries;
        CORE::CString rev;
        bool flag = false;
        REQUIRE_TRUE( client.ListAll( "pub", "", entries, rev, flag ).IsOk() );
        ASSERT_TRUE( 1 == entries.size() && "maps" == entries[ 0 ].name && entries[ 0 ].IsDirectory() );
        REQUIRE_TRUE( client.ListAll( "pub", "maps", entries, rev, flag ).IsOk() );
        ASSERT_TRUE( 1 == entries.size() && "pub" == entries[ 0 ].name );
        REQUIRE_TRUE( client.ListAll( "pub", "maps/pub", entries, rev, flag ).IsOk() );
        TStringSet names = NamesOf( entries );
        ASSERT_TRUE( 2 == names.size() && names.count( "a.bin" ) && names.count( "inner" ) );
        REQUIRE_TRUE( client.ListAll( "pub", "maps/pub/inner", entries, rev, flag ).IsOk() );
        ASSERT_TRUE( 1 == entries.size() && "b.bin" == entries[ 0 ].name );

        DRN::CDRNNamespaceListPage page;
        ASSERT_TRUE( DRN::DRN_NSRESULT_ACCESS_DENIED == client.List( "pub", "maps/priv", CORE::CString::Empty, 0, CORE::CString::Empty, page ).code );
        ASSERT_TRUE( DRN::DRN_NSRESULT_ACCESS_DENIED == client.List( "pub", "mapsx", CORE::CString::Empty, 0, CORE::CString::Empty, page ).code );
        ASSERT_TRUE( DRN::DRN_NSRESULT_ACCESS_DENIED == client.List( "pub", "sub", CORE::CString::Empty, 0, CORE::CString::Empty, page ).code );

        TStrings paths;
        paths.push_back( "" ); paths.push_back( "maps" ); paths.push_back( "maps/pub/a.bin" ); paths.push_back( "maps/priv/c.bin" ); paths.push_back( "maps/loose.bin" );
        paths.push_back( "top.bin" ); paths.push_back( "mapsx" ); paths.push_back( "maps/pub/missing.bin" ); paths.push_back( "nothere/deeper" );
        TEntries stats;
        REQUIRE_TRUE( client.Stat( "pub", paths, TStrings(), stats ).IsOk() && paths.size() == stats.size() );
        DRN::TDRNResourceStatus expected[] = { DRN::DRN_RESSTATUS_OK, DRN::DRN_RESSTATUS_OK, DRN::DRN_RESSTATUS_OK, DRN::DRN_RESSTATUS_FORBIDDEN, DRN::DRN_RESSTATUS_FORBIDDEN,
                                               DRN::DRN_RESSTATUS_FORBIDDEN, DRN::DRN_RESSTATUS_FORBIDDEN, DRN::DRN_RESSTATUS_NOT_FOUND, DRN::DRN_RESSTATUS_FORBIDDEN };
        for ( UInt32 i=0; i<paths.size(); ++i )
            ASSERT_TRUE( expected[ i ] == stats[ i ].status );
        // a name above the subtree is a directory that can be seen, not a file that can be read
        DRN::CDRNResourceHandlePtr handle = fixture.Member().OpenHandle( fixture.serverPeer, "pub", "maps/loose.bin", DRN::CDRNResourceTransferOptions() );
        ASSERT_FALSE( handle->WaitOpen( 10000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == handle->GetResult().status );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: the access policy hides entries from listings, stat and open alike, may let a name be seen that cannot be read, and decides before anything is resolved" )
    try
    {
        CORE::CString root = TestDirectory( "policy" );
        MakeTree( root, 3 );
        REQUIRE_TRUE( WriteFile( Join( root, "secret.bin" ), 50 ) );
        REQUIRE_TRUE( WriteFile( Join( root, "locked.bin" ), 51 ) );
        CORE::CreateDirs( Join( root, "hidden/inside" ) );
        REQUIRE_TRUE( WriteFile( Join( root, "hidden/h.bin" ), 52 ) );
        REQUIRE_TRUE( WriteFile( Join( root, "hidden/inside/i.bin" ), 53 ) );
        CNsFixture fixture( "ns_policy" );
        REQUIRE_TRUE( fixture.Start() );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );
        CNsPolicy policy;
        policy.hidden.insert( "secret.bin" );
        policy.hidden.insert( "hidden" );
        policy.readDenied.insert( "locked.bin" );
        policy.browseDenied.insert( "emptydir" );
        fixture.group.nodes[ 0 ]->SetAccessPolicy( &policy );

        TEntries entries;
        CORE::CString rev;
        bool flag = false;
        UInt64 hiddenBefore = fixture.Server().GetStats().namespaceEntriesHidden;
        REQUIRE_TRUE( client.ListAll( "files", "", entries, rev, flag ).IsOk() );
        TStringSet names = NamesOf( entries );
        // hidden by the policy: secret.bin, hidden; not browsable: emptydir; a name that cannot be read is not shown either (the policy's browse follows its read)
        ASSERT_TRUE( !names.count( "secret.bin" ) && !names.count( "hidden" ) && !names.count( "emptydir" ) && !names.count( "locked.bin" ) );
        ASSERT_TRUE( names.count( "a.bin" ) && names.count( "sub" ) && names.count( "big" ) );
        ASSERT_TRUE( fixture.Server().GetStats().namespaceEntriesHidden == hiddenBefore + 4 );

        TStrings paths;
        paths.push_back( "secret.bin" ); paths.push_back( "hidden" ); paths.push_back( "hidden/h.bin" ); paths.push_back( "hidden/inside/i.bin" );
        paths.push_back( "locked.bin" ); paths.push_back( "emptydir" ); paths.push_back( "a.bin" );
        TEntries stats;
        REQUIRE_TRUE( client.Stat( "files", paths, TStrings(), stats ).IsOk() );
        DRN::TDRNResourceStatus expected[] = { DRN::DRN_RESSTATUS_NOT_FOUND, DRN::DRN_RESSTATUS_NOT_FOUND, DRN::DRN_RESSTATUS_NOT_FOUND, DRN::DRN_RESSTATUS_NOT_FOUND,
                                               DRN::DRN_RESSTATUS_FORBIDDEN, DRN::DRN_RESSTATUS_FORBIDDEN, DRN::DRN_RESSTATUS_OK };
        for ( UInt32 i=0; i<paths.size(); ++i )
            ASSERT_TRUE( expected[ i ] == stats[ i ].status );
        DRN::CDRNNamespaceListPage page;
        ASSERT_TRUE( DRN::DRN_NSRESULT_NOT_FOUND == client.List( "files", "hidden", CORE::CString::Empty, 0, CORE::CString::Empty, page ).code );
        ASSERT_TRUE( DRN::DRN_NSRESULT_NOT_FOUND == client.List( "files", "hidden/inside", CORE::CString::Empty, 0, CORE::CString::Empty, page ).code );
        ASSERT_TRUE( DRN::DRN_NSRESULT_ACCESS_DENIED == client.List( "files", "emptydir", CORE::CString::Empty, 0, CORE::CString::Empty, page ).code );

        // OPEN honours it: a hidden entry is not there, a name that is not shared is forbidden
        DRN::CDRNResourceHandlePtr hiddenOpen = fixture.Member().OpenHandle( fixture.serverPeer, "files", "secret.bin", DRN::CDRNResourceTransferOptions() );
        ASSERT_FALSE( hiddenOpen->WaitOpen( 10000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == hiddenOpen->GetResult().status );
        DRN::CDRNResourceHandlePtr deepOpen = fixture.Member().OpenHandle( fixture.serverPeer, "files", "hidden/inside/i.bin", DRN::CDRNResourceTransferOptions() );
        ASSERT_FALSE( deepOpen->WaitOpen( 10000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == deepOpen->GetResult().status );
        DRN::CDRNResourceHandlePtr lockedOpen = fixture.Member().OpenHandle( fixture.serverPeer, "files", "locked.bin", DRN::CDRNResourceTransferOptions() );
        ASSERT_FALSE( lockedOpen->WaitOpen( 10000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == lockedOpen->GetResult().status );

        // a policy that lets a name be seen without letting it be read: seen and statted, not opened
        policy.browseIgnoresRead = true;
        policy.browseDenied.clear();
        entries.clear();
        REQUIRE_TRUE( client.ListAll( "files", "", entries, rev, flag ).IsOk() );
        ASSERT_TRUE( FindEntry( entries, "locked.bin" ) != GUCEF_NULL && FindEntry( entries, "emptydir" ) != GUCEF_NULL && FindEntry( entries, "secret.bin" ) == GUCEF_NULL );
        REQUIRE_TRUE( client.Stat( "files", Strings1( "locked.bin" ), TStrings(), stats ).IsOk() );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == stats[ 0 ].status && 51 == stats[ 0 ].size );
        lockedOpen = fixture.Member().OpenHandle( fixture.serverPeer, "files", "locked.bin", DRN::CDRNResourceTransferOptions() );
        ASSERT_FALSE( lockedOpen->WaitOpen( 10000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == lockedOpen->GetResult().status );

        // nothing is shared by a policy that says yes where the rules say no, for the namespace as for bytes
        DRN::CDRNResourceShareRules none;
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "none", NsProvider( root ), none ) );
        ASSERT_TRUE( DRN::DRN_NSRESULT_ACCESS_DENIED == client.List( "none", "", CORE::CString::Empty, 0, CORE::CString::Empty, page ).code );
        fixture.group.nodes[ 0 ]->SetAccessPolicy( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a policy that gives the reason of a refused read (CheckReadResourceAs) has it told to the member; a policy that gives none leaves the words of the facility" )
    try
    {
        CORE::CString root = TestDirectory( "refusal" );
        MakeTree( root, 3 );
        REQUIRE_TRUE( WriteFile( Join( root, "locked.bin" ), 51 ) );
        REQUIRE_TRUE( WriteFile( Join( root, "plain.bin" ), 52 ) );
        CNsFixture fixture( "ns_refusal" );
        REQUIRE_TRUE( fixture.Start() );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        CReasonPolicy policy;
        fixture.group.nodes[ 0 ]->SetAccessPolicy( &policy );

        DRN::CDRNResourceHandlePtr locked = fixture.Member().OpenHandle( fixture.serverPeer, "files", "locked.bin", DRN::CDRNResourceTransferOptions() );
        ASSERT_FALSE( locked->WaitOpen( 10000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == locked->GetResult().status );
        ASSERT_TRUE( locked->GetResult().message == "this file is for the owner of the share" );
        DRN::CDRNResourceHandlePtr plain = fixture.Member().OpenHandle( fixture.serverPeer, "files", "plain.bin", DRN::CDRNResourceTransferOptions() );
        ASSERT_FALSE( plain->WaitOpen( 10000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == plain->GetResult().status );
        ASSERT_TRUE( plain->GetResult().message == "not shared with this member" );
        DRN::CDRNResourceHandlePtr allowed = fixture.Member().OpenHandle( fixture.serverPeer, "files", "a.bin", DRN::CDRNResourceTransferOptions() );
        ASSERT_TRUE( allowed->WaitOpen( 10000 ) );
        fixture.group.nodes[ 0 ]->SetAccessPolicy( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a policy can answer a read with retry-later: the member gets BUSY with the words of the policy and no handle, asks again and is served once the policy is ready; a refusal that is none of that stays FORBIDDEN with its reason; names are still seen while a read is busy; a policy that does not use it is unchanged" )
    try
    {
        CORE::CString root = TestDirectory( "retrylater" );
        MakeTree( root, 3 );
        REQUIRE_TRUE( WriteFile( Join( root, "warming.bin" ), 53 ) );
        REQUIRE_TRUE( WriteFile( Join( root, "locked.bin" ), 54 ) );
        CNsFixture fixture( "ns_retrylater" );
        REQUIRE_TRUE( fixture.Start() );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        CRetryPolicy policy;
        fixture.group.nodes[ 0 ]->SetAccessPolicy( &policy );

        // busy: no refusal, the words are the policy's, and it can be told apart from the refusal of the next file
        DRN::CDRNResourceHandlePtr warming = fixture.Member().OpenHandle( fixture.serverPeer, "files", "warming.bin", DRN::CDRNResourceTransferOptions() );
        ASSERT_FALSE( warming->WaitOpen( 10000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_BUSY == warming->GetResult().status );
        ASSERT_TRUE( warming->GetResult().message == "the data of this file is being got ready" );
        DRN::CDRNResourceHandlePtr locked = fixture.Member().OpenHandle( fixture.serverPeer, "files", "locked.bin", DRN::CDRNResourceTransferOptions() );
        ASSERT_FALSE( locked->WaitOpen( 10000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_FORBIDDEN == locked->GetResult().status );
        ASSERT_TRUE( locked->GetResult().message == "this file is for the owner of the share" );
        ASSERT_TRUE( 1 == policy.askedBusy.Load() );

        // its name and size are seen meanwhile
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );
        TEntries stats;
        REQUIRE_TRUE( client.Stat( "files", Strings1( "warming.bin" ), TStrings(), stats ).IsOk() && 1 == stats.size() );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == stats[ 0 ].status && stats[ 0 ].size > 0 );

        // asked again after the policy is ready: served, and the same handle id is usable again (nothing was left open by the busy answer)
        policy.ready.Store( true );
        DRN::CDRNResourceHandlePtr again = fixture.Member().OpenHandle( fixture.serverPeer, "files", "warming.bin", DRN::CDRNResourceTransferOptions() );
        ASSERT_TRUE( again->WaitOpen( 10000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == again->GetResult().status );
        again->Close();
        fixture.group.nodes[ 0 ]->SetAccessPolicy( GUCEF_NULL );

        // the default of the policy base class: retry-later is false whatever it was, and the refusal is the one of CheckReadResourceAs()
        CReasonPolicy reasons;
        DRN::CDRNAuthResult auth;
        CORE::CString refusal;
        bool retryLater = true;
        ASSERT_FALSE( reasons.DecideReadResourceAs( 1, auth, "files", "locked.bin", true, refusal, retryLater ) );
        ASSERT_TRUE( !retryLater && refusal == "this file is for the owner of the share" );
        retryLater = true;
        refusal.Clear();
        ASSERT_TRUE( reasons.DecideReadResourceAs( 1, auth, "files", "other.bin", true, refusal, retryLater ) );
        ASSERT_TRUE( !retryLater && refusal.IsNULLOrEmpty() );
        DRN::CDRNDefaultAccessPolicy plainPolicy;
        retryLater = true;
        ASSERT_FALSE( plainPolicy.DecideReadResourceAs( 1, auth, "files", "x.bin", false, refusal, retryLater ) );
        ASSERT_FALSE( retryLater );
        ASSERT_TRUE( plainPolicy.DecideReadResourceAs( 1, auth, "files", "x.bin", true, refusal, retryLater ) && !retryLater );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: names that climb out of the share, name a drive, hold a backslash, a control character, a NUL or are too long are refused by the serving side before a provider sees them" )
    try
    {
        CORE::CString root = TestDirectory( "confine" );
        CORE::CString inner = Join( root, "inner" );
        CORE::CreateDirs( Join( inner, "d" ) );
        REQUIRE_TRUE( WriteFile( Join( root, "outside.bin" ), 10 ) );
        REQUIRE_TRUE( WriteFile( Join( inner, "in.bin" ), 10 ) );
        CNsFixture fixture( "ns_confine" );
        REQUIRE_TRUE( fixture.Start() );
        // the share is "inner": its parent holds a file that no name may reach
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( inner ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );

        CORE::CString withNul( "ab\0cd", 5 );
        CORE::CString tooLong;
        for ( UInt32 i=0; i<1025; ++i )
            tooLong += "x";
        CORE::CString manySegments;
        for ( UInt32 i=0; i<600; ++i )
            manySegments += "a/";
        const CORE::CString attacks[] = {
            "..", "../outside.bin", "d/../../outside.bin", "a/../../b", "/etc", "/etc/passwd", "/", "d/..", "./in.bin", "d/./x", "d//x", "in.bin/", "d/", "a\\b", "..\\outside.bin",
            "C:x", "C:\\x", "C:/x", "c:", "in.bin:stream", "a\tb", "a\nb", "a\x01" "b", "a\x7f" "b", withNul, tooLong, manySegments, "\\\\server\\share"
        };
        UInt32 attackCount = sizeof( attacks ) / sizeof( attacks[ 0 ] );
        for ( UInt32 i=0; i<attackCount; ++i )
        {
            ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == ForgeStatOne( fixture, "files", attacks[ i ] ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == ForgeListOne( fixture, "files", attacks[ i ] ) );
        }
        // through the client as well: the member refuses nothing by itself, the serving side does
        TStrings paths;
        for ( UInt32 i=0; i<attackCount; ++i )
            paths.push_back( attacks[ i ] );
        TEntries stats;
        REQUIRE_TRUE( client.Stat( "files", paths, TStrings(), stats ).IsOk() && attackCount == stats.size() );
        for ( UInt32 i=0; i<attackCount; ++i )
            ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == stats[ i ].status );
        DRN::CDRNNamespaceListPage page;
        ASSERT_TRUE( DRN::DRN_NSRESULT_INVALID_NAME == client.List( "files", "../", CORE::CString::Empty, 0, CORE::CString::Empty, page ).code );
        ASSERT_TRUE( DRN::DRN_NSRESULT_INVALID_NAME == client.List( "files", "d/../..", CORE::CString::Empty, 0, CORE::CString::Empty, page ).code );

        // the provider checks again, used without the component
        DRN::CDRNFileSystemAccessResourceProvider direct( NativeFileSystem(), inner, CORE::CUri( "file://" + inner ) );
        DRN::CDRNNamespaceEntry entry;
        DRN::CDRNResourceInfo info;
        for ( UInt32 i=0; i<attackCount; ++i )
        {
            ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == direct.Stat( attacks[ i ], entry ) );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == direct.Resolve( attacks[ i ], info ) );
            DRN::CIDRNNamespaceEnumeratorPtr enumerator;
            ASSERT_TRUE( DRN::DRN_RESSTATUS_INVALID_NAME == direct.OpenEnumerator( attacks[ i ], enumerator ) );
        }
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == direct.Stat( "in.bin", entry ) && 10 == entry.size );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == direct.Stat( "outside.bin", entry ) );

        // nothing of the outside is in the listing of the root
        TEntries all;
        CORE::CString rev;
        bool flag = false;
        REQUIRE_TRUE( client.ListAll( "files", "", all, rev, flag ).IsOk() );
        ASSERT_TRUE( 2 == all.size() && FindEntry( all, "in.bin" ) != GUCEF_NULL && FindEntry( all, "d" ) != GUCEF_NULL && FindEntry( all, "outside.bin" ) == GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: entries whose names the resource rules refuse are left out of listings and counted; links are left out unless the share follows them" )
    try
    {
        CORE::CString root = TestDirectory( "invalid" );
        RemoveOddFiles( root );
        REQUIRE_TRUE( WriteFile( Join( root, "good.bin" ), 5 ) );
        REQUIRE_TRUE( WriteRawNamed( root + "/bad:name.bin", 5 ) );
        REQUIRE_TRUE( WriteRawNamed( root + "/back\\slash.bin", 5 ) );
        REQUIRE_TRUE( WriteRawNamed( root + "/tab\there.bin", 5 ) );
        REQUIRE_TRUE( WriteFile( Join( root, "n\xC3\xA9.bin" ), 5 ) );
        REQUIRE_TRUE( WriteFile( Join( root, "..dots.bin" ), 5 ) );

        // a path that is too long for the rules once the name is added: a directory of 1000 bytes in the name, an entry of 40 more
        CORE::CString deepName;
        for ( UInt32 level=0; level<5; ++level )
        {
            CORE::CString part;
            for ( UInt32 i=0; i<199; ++i )
                part += "d";
            deepName += ( level > 0 ? "/" : "" ) + part;
        }
        CORE::CString deepDir = Join( root, deepName );
        CORE::CreateDirs( deepDir );
        CORE::CString longEntry;
        for ( UInt32 i=0; i<40; ++i )
            longEntry += "e";
        REQUIRE_TRUE( WriteFile( Join( deepDir, "short.bin" ), 5 ) );
        REQUIRE_TRUE( WriteFile( Join( deepDir, longEntry + ".bin" ), 5 ) );
        bool haveLinks = false;
        #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
        haveLinks = 0 == ::symlink( Join( root, "good.bin" ).C_String(), Join( root, "link.bin" ).C_String() );
        #endif

        CNsFixture fixture( "ns_invalid" );
        REQUIRE_TRUE( fixture.Start() );
        DRN::CDRNFileSystemAccessResourceProvider* provider = GUCEF_NEW DRN::CDRNFileSystemAccessResourceProvider( NativeFileSystem(), root, CORE::CUri( "file://" + root ) );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", DRN::CIDRNResourceProviderPtr( provider ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );

        UInt64 invalidBefore = fixture.Server().GetStats().namespaceEntriesInvalidName;
        TEntries entries;
        CORE::CString rev;
        bool flag = false;
        REQUIRE_TRUE( client.ListAll( "files", "", entries, rev, flag ).IsOk() );
        TStringSet names = NamesOf( entries );
        ASSERT_TRUE( names.count( "good.bin" ) && names.count( "n\xC3\xA9.bin" ) && names.count( "..dots.bin" ) && names.count( deepName.SubstrToChar( '/', true ) ) );
        ASSERT_FALSE( names.count( "bad:name.bin" ) || names.count( "back\\slash.bin" ) || names.count( "tab\there.bin" ) );
        // the colon and the control character are counted; a name with a backslash is not seen at all by the iterator of the operating system's
        // file system access (CORE's path functions take the backslash for a separator), so it is neither listed nor counted
        ASSERT_TRUE( fixture.Server().GetStats().namespaceEntriesInvalidName == invalidBefore + 2 );
        if ( haveLinks )
        {
            ASSERT_FALSE( names.count( "link.bin" ) );
            ASSERT_TRUE( 1 == provider->GetSkippedSymlinkCount() );
        }

        // an entry the member is told of can be opened by that name
        const DRN::CDRNNamespaceEntry* utf8 = FindEntry( entries, "n\xC3\xA9.bin" );
        REQUIRE_TRUE( GUCEF_NULL != utf8 );
        DRN::CDRNResourceHandlePtr handle = fixture.Member().OpenHandle( fixture.serverPeer, "files", utf8->name, DRN::CDRNResourceTransferOptions() );
        ASSERT_TRUE( handle->WaitOpen( 10000 ) && handle->GetInfo().revision == utf8->revision );
        handle->Close();

        // the deep directory: the entry that would make its name too long is counted, the other is listed
        invalidBefore = fixture.Server().GetStats().namespaceEntriesInvalidName;
        REQUIRE_TRUE( client.ListAll( "files", deepName, entries, rev, flag ).IsOk() );
        ASSERT_TRUE( 1 == entries.size() && "short.bin" == entries[ 0 ].name );
        ASSERT_TRUE( fixture.Server().GetStats().namespaceEntriesInvalidName == invalidBefore + 1 );

        if ( haveLinks )
        {
            DRN::CDRNNamespaceShareSettings follow;
            follow.followSymlinks = true;
            REQUIRE_TRUE( fixture.Server().RegisterProvider( "follow", NsProvider( root, follow ), ShareAll() ) );
            REQUIRE_TRUE( client.ListAll( "follow", "", entries, rev, flag ).IsOk() );
            ASSERT_TRUE( FindEntry( entries, "link.bin" ) != GUCEF_NULL && 5 == FindEntry( entries, "link.bin" )->size );
        }
        RemoveOddFiles( root );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a share is read-only: requests that would change it are answered READ_ONLY, the share advertises no write capability" )
    try
    {
        CORE::CString root = TestDirectory( "readonly" );
        MakeTree( root, 2 );
        CNsFixture fixture( "ns_readonly" );
        REQUIRE_TRUE( fixture.Start() );
        DRN::CDRNNamespaceShareSettings settings;
        settings.readOnly = false;     // not honoured in this version: nothing can be changed through DRN
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root, settings ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );
        GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
        REQUIRE_TRUE( client.Shares( shares ).IsOk() && 1 == shares.size() );
        ASSERT_TRUE( shares[ 0 ].IsReadOnly() && 0 == ( shares[ 0 ].capabilities & ( DRN::DRN_NS_CAP_WRITE | DRN::DRN_NS_CAP_CREATE | DRN::DRN_NS_CAP_DELETE | DRN::DRN_NS_CAP_RENAME | DRN::DRN_NS_CAP_MKDIR ) ) );

        UInt64 refusedBefore = fixture.Server().GetStats().namespaceRequestsRefused;
        UInt32 refused = 0;
        for ( UInt32 opcode=DRN::DRN_RES_NS_MUTATION_FIRST; opcode<=DRN::DRN_RES_NS_MUTATION_LAST; ++opcode )
        {
            CORE::CDynamicBuffer body;
            DRN::CDRNWireWriter writer( body );
            DRN::CDRNNamespaceCodec::WriteText( writer, "files" );
            DRN::CDRNNamespaceCodec::WriteText( writer, "a.bin" );
            CForged forged = Forge( fixture, opcode, body );
            ASSERT_TRUE( DRN::DRN_RESSTATUS_READ_ONLY == forged.status );
            ++refused;
        }
        ASSERT_TRUE( fixture.Server().GetStats().namespaceRequestsRefused == refusedBefore + refused );
        TEntries stats;
        REQUIRE_TRUE( client.Stat( "files", Strings1( "a.bin" ), TStrings(), stats ).IsOk() && 1000 == stats[ 0 ].size );
        ASSERT_TRUE( CORE::FileExists( Join( root, "a.bin" ) ) && 1000 == CORE::FileSize( Join( root, "a.bin" ) ) );

        // the numbers that are kept for later are answered, not dropped
        const UInt32 reserved[] = { 20, 21, 22, 23, 48, 55, 63 };
        for ( UInt32 i=0; i<sizeof( reserved ) / sizeof( reserved[ 0 ] ); ++i )
            ASSERT_TRUE( DRN::DRN_RESSTATUS_UNSUPPORTED == Forge( fixture, reserved[ i ], CORE::CDynamicBuffer() ).status );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: limits refuse with statuses: too many listings, an oversized page, an oversized batch, too many requests at once; the client stays within them by itself" )
    try
    {
        CORE::CString root = TestDirectory( "limits" );
        MakeTree( root, 400 );
        CNsFixture fixture( "ns_limits" );
        REQUIRE_TRUE( fixture.Start() );
        DRN::CDRNResourceTransferSettings settings;
        settings.maxNamespaceStatPaths = 8;
        settings.maxNamespacePageEntries = 60;
        settings.maxNamespaceListingsPerPeer = 2;
        settings.maxNamespaceRequestsPerPeer = 2;
        fixture.Server().SetSettings( settings );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        CSlowProvider* slow = GUCEF_NEW CSlowProvider( root, 400 );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "slow", DRN::CIDRNResourceProviderPtr( slow ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );

        DRN::CDRNNamespaceInfo info;
        REQUIRE_TRUE( client.Negotiate( info ).IsOk() );
        ASSERT_TRUE( 8 == info.maxStatPaths && 60 == info.maxPageEntries );

        // forged requests beyond the limits
        {
            UInt32 id = fixture.Member().GetNextNamespaceRequestId();
            CORE::CDynamicBuffer body;
            DRN::CDRNWireWriter writer( body );
            DRN::CDRNNamespaceCodec::WriteStatRequestHeader( writer, id, "files", 9 );
            for ( UInt32 i=0; i<9; ++i )
                DRN::CDRNNamespaceCodec::WriteStatRequestItem( writer, "a.bin", CORE::CString::Empty );
            CORE::CString message;
            CORE::CDynamicBuffer answer;
            ASSERT_TRUE( DRN::DRN_RESSTATUS_TOO_LARGE == fixture.Member().CallNamespace( fixture.serverPeer, DRN::DRN_RES_NS_STAT, id, body, 5000, message, answer ) );
        }
        ASSERT_TRUE( DRN::DRN_RESSTATUS_TOO_LARGE == ForgeListOne( fixture, "files", "big", CORE::CString::Empty, 61 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_TOO_LARGE == ForgeListOne( fixture, "files", "big", CORE::CString::Empty, 0xFFFFFFFF ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == ForgeListOne( fixture, "files", "emptydir", CORE::CString::Empty, 60 ) );

        // the client splits a long list into batches of the negotiated size and asks pages of at most the negotiated size
        TStrings paths;
        for ( UInt32 i=0; i<60; ++i )
            paths.push_back( 0 == i % 2 ? "a.bin" : "sub/x.bin" );
        UInt64 servedBefore = fixture.Server().GetStats().namespaceRequestsServed;
        TEntries stats;
        REQUIRE_TRUE( client.Stat( "files", paths, TStrings(), stats ).IsOk() && 60 == stats.size() );
        for ( UInt32 i=0; i<60; ++i )
            ASSERT_TRUE( DRN::DRN_RESSTATUS_OK == stats[ i ].status && ( 0 == i % 2 ? 1000 : 10 ) == stats[ i ].size && stats[ i ].name == paths[ i ] );
        ASSERT_TRUE( fixture.Server().GetStats().namespaceRequestsServed == servedBefore + 8 );
        DRN::CDRNNamespaceListPage page;
        REQUIRE_TRUE( client.List( "files", "big", CORE::CString::Empty, 5000, CORE::CString::Empty, page ).IsOk() );
        ASSERT_TRUE( 60 == page.entries.size() );
        FinishListing( client, "files", "big", page );
        TEntries all;
        CORE::CString rev;
        bool flag = false;
        REQUIRE_TRUE( client.ListAll( "files", "big", all, rev, flag ).IsOk() && 400 == all.size() );

        // a share with a limit of its own that is stricter than the serving side's: refused, the client shrinks and goes on
        DRN::CDRNNamespaceShareSettings strict;
        strict.maxStatPaths = 3;
        strict.maxPageEntries = 25;
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "strict", NsProvider( root, strict ), ShareAll() ) );
        GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
        REQUIRE_TRUE( client.Shares( shares ).IsOk() );
        bool found = false;
        for ( UInt32 i=0; i<shares.size(); ++i )
            if ( "strict" == shares[ i ].name )
                found = 3 == shares[ i ].maxStatPaths && 25 == shares[ i ].maxPageEntries;
        ASSERT_TRUE( found );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_TOO_LARGE == ForgeListOne( fixture, "strict", "big", CORE::CString::Empty, 26 ) );
        REQUIRE_TRUE( client.Stat( "strict", paths, TStrings(), stats ).IsOk() && 60 == stats.size() );
        REQUIRE_TRUE( client.List( "strict", "big", CORE::CString::Empty, 50, CORE::CString::Empty, page ).IsOk() );
        ASSERT_TRUE( 25 == page.entries.size() );
        FinishListing( client, "strict", "big", page );
        REQUIRE_TRUE( client.ListAll( "strict", "big", all, rev, flag, 1000, 50 ).IsOk() && 400 == all.size() );

        // too many listings under way: the third is BUSY, a finished one gives its place back
        DRN::CDRNNamespaceListPage l1, l2, l3;
        ASSERT_TRUE( 0 == fixture.Server().GetNamespaceListingCount( 0 ) );
        REQUIRE_TRUE( client.List( "files", "big", CORE::CString::Empty, 10, CORE::CString::Empty, l1 ).IsOk() );
        REQUIRE_TRUE( client.List( "files", "sub", CORE::CString::Empty, 1, CORE::CString::Empty, l2 ).IsOk() );
        ASSERT_TRUE( 2 == fixture.Server().GetNamespaceListingCount( LocalPeerOf( fixture.group[ 1 ] ) ) );
        DRN::CDRNNamespaceResult busy = client.List( "files", "sub", CORE::CString::Empty, 1, CORE::CString::Empty, l3 );
        ASSERT_TRUE( DRN::DRN_NSRESULT_BUSY == busy.code && busy.IsRetryable() );
        DRN::CDRNNamespaceListPage finish;
        REQUIRE_TRUE( client.List( "files", "sub", l2.nextCursor, 10, CORE::CString::Empty, finish ).IsOk() && finish.IsLast() );
        REQUIRE_TRUE( client.List( "files", "sub", CORE::CString::Empty, 1, CORE::CString::Empty, l3 ).IsOk() );

        // too many requests being worked on at once: some of them are BUSY, none is lost (the client's own throttle is
        // off here: with it on, which is the default, none of them is refused, see the limits suite)
        client.SetSelfThrottle( false );
        DRN::CDRNNamespaceResponsePtr responses[ 6 ];
        GUCEF::vector< CORE::CFutureResult > futures;
        for ( UInt32 i=0; i<6; ++i )
            futures.push_back( client.StatAsync( "slow", Strings1( "a.bin" ), TStrings(), responses[ i ], 15000 ) );
        UInt32 busyCount = 0, okCount = 0;
        for ( UInt32 i=0; i<6; ++i )
        {
            futures[ i ].Await( 20000 );
            if ( DRN::DRN_NSRESULT_BUSY == responses[ i ]->result.code )
                ++busyCount;
            else
            if ( responses[ i ]->result.IsOk() && 1000 == responses[ i ]->entries[ 0 ].size )
                ++okCount;
        }
        ASSERT_TRUE( busyCount >= 1 && okCount >= 2 && busyCount + okCount == 6 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a member of protocol 2.2 and a provider without a namespace get a clear refusal, never a closed link; plain links are refused unless both sides allow them" )
    try
    {
        CORE::CString root = TestDirectory( "oldpeer" );
        MakeTree( root, 3 );
        {
            CNsFixture fixture( "ns_old" );
            fixture.group.nodeSettings.protocolMinor = DRN::DRN_PROTOCOL_MINOR_RESOURCE;
            REQUIRE_TRUE( fixture.Start() );
            REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
            DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );
            GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
            DRN::CDRNNamespaceResult result = client.Shares( shares );
            ASSERT_TRUE( DRN::DRN_NSRESULT_UNSUPPORTED == result.code && DRN::DRN_RESSTATUS_UNSUPPORTED == result.status && !result.message.IsNULLOrEmpty() );
            DRN::CDRNNamespaceInfo info;
            ASSERT_TRUE( DRN::DRN_NSRESULT_UNSUPPORTED == client.Negotiate( info ).code );
            DRN::CDRNNamespaceListPage page;
            ASSERT_TRUE( DRN::DRN_NSRESULT_UNSUPPORTED == client.List( "files", "", CORE::CString::Empty, 0, CORE::CString::Empty, page ).code );
            ASSERT_TRUE( 0 == fixture.Member().GetNamespaceRequestCount() );

            // a request that arrives anyway (a node that sends without asking) is answered by the node that is below 2.3, and nothing is closed
            UInt32 id = fixture.Member().GetNextNamespaceRequestId();
            CORE::CDynamicBuffer body;
            DRN::CDRNWireWriter writer( body );
            DRN::CDRNNamespaceCodec::WriteSharesRequest( writer, id );
            {
                MT::CScopeMutex lock( fixture.group[ 1 ].GetNode().GetLock() );
                DRN::CDRNPeerLinkPtr link = fixture.group[ 1 ].GetLinkToMember( fixture.serverPeer );
                REQUIRE_TRUE( !link.IsNULL() );
                REQUIRE_TRUE( link->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_RESOURCE, DRN::DRN_RES_NS_SHARES, body.GetConstBufferPtr(), writer.GetOffset() ) );
            }
            PumpFor( 300 );
            ASSERT_TRUE( fixture.Server().GetStats().namespaceRequestsRefused >= 1 );
            ASSERT_TRUE( fixture.Member().GetStats().namespaceAnswersDropped >= 1 );
            ASSERT_TRUE( 0 == fixture.group[ 0 ].GetNode().GetUnhandledMessageCount() && 0 == fixture.group[ 1 ].GetNode().GetUnhandledMessageCount() );
            {
                MT::CScopeMutex lock( fixture.group[ 1 ].GetNode().GetLock() );
                DRN::CDRNPeerLinkPtr link = fixture.group[ 1 ].GetLinkToMember( fixture.serverPeer );
                ASSERT_TRUE( !link.IsNULL() && link->IsOperational() );
            }

            // the bytes of the resource facility work with it
            DRN::CDRNResourceHandlePtr handle = fixture.Member().OpenHandle( fixture.serverPeer, "files", "a.bin", DRN::CDRNResourceTransferOptions() );
            ASSERT_TRUE( handle->WaitOpen( 10000 ) && 1000 == handle->GetInfo().size );
            handle->Close();
        }
        {
            // a provider of bytes only: a refusal naming the share, the link is fine
            CNsFixture fixture( "ns_bytes" );
            REQUIRE_TRUE( fixture.Start() );
            REQUIRE_TRUE( fixture.Server().RegisterProvider( "bytes", DRN::CIDRNResourceProviderPtr( GUCEF_NEW DRN::CDRNFileSystemResourceProvider( root ) ), ShareAll() ) );
            DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );
            DRN::CDRNNamespaceListPage page;
            DRN::CDRNNamespaceResult result = client.List( "bytes", "", CORE::CString::Empty, 0, CORE::CString::Empty, page );
            ASSERT_TRUE( DRN::DRN_NSRESULT_UNSUPPORTED == result.code && DRN::DRN_RESSTATUS_UNSUPPORTED == result.status );
            TEntries stats;
            REQUIRE_TRUE( client.Stat( "bytes", Strings1( "a.bin" ), TStrings(), stats ).code == DRN::DRN_NSRESULT_UNSUPPORTED );
            GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
            REQUIRE_TRUE( client.Shares( shares ).IsOk() && 1 == shares.size() && !shares[ 0 ].CanList() && !shares[ 0 ].CanStat() );
        }
        {
            CNsFixture fixture( "ns_plain" );
            fixture.group.nodeSettings.securityPolicy.requireEncryption = false;
            fixture.group.nodeSettings.securityPolicy.ciphers.clear();
            fixture.group.sessionSettings.securityPolicy.requireEncryption = false;
            fixture.group.sessionSettings.securityPolicy.ciphers.clear();
            REQUIRE_TRUE( fixture.Start() );
            REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
            DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );
            GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
            ASSERT_TRUE( DRN::DRN_NSRESULT_ACCESS_DENIED == client.Shares( shares ).code );
            DRN::CDRNResourceTransferSettings allow;
            allow.allowUnprotectedLinks = true;
            fixture.Member().SetSettings( allow );
            ASSERT_TRUE( DRN::DRN_NSRESULT_ACCESS_DENIED == client.Shares( shares ).code );     // refused by the serving side now
            ASSERT_TRUE( fixture.Server().GetStats().namespaceRequestsRefused >= 1 );
            fixture.Server().SetSettings( allow );
            ASSERT_TRUE( client.Shares( shares ).IsOk() && 1 == shares.size() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a blocking call on a thread that works on the node is refused at once; the futures work with nodes that have no thread of their own" )
    try
    {
        CORE::CString root = TestDirectory( "threads" );
        MakeTree( root, 3 );
        {
            CNsFixture fixture( "ns_threads" );
            REQUIRE_TRUE( fixture.Start() );
            REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
            DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );

            // the serving node's thread calls a client of its own session: from the access policy, which runs on that thread
            DRN::CDRNNamespaceClient onServerThread( fixture.group[ 0 ], LocalPeerOf( fixture.group[ 1 ] ) );
            CNsPolicy policy;
            policy.nestedClient = &onServerThread;
            fixture.group.nodes[ 0 ]->SetAccessPolicy( &policy );
            GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
            ASSERT_TRUE( client.Shares( shares ).IsOk() && 1 == shares.size() );
            ASSERT_TRUE( policy.browseCalls >= 1 );
            ASSERT_TRUE( DRN::DRN_NSRESULT_WRONG_THREAD == policy.nestedResult.code && !policy.nestedResult.message.IsNULLOrEmpty() );
            ASSERT_TRUE( 0 == fixture.Server().GetNamespaceRequestCount() );
            fixture.group.nodes[ 0 ]->SetAccessPolicy( GUCEF_NULL );
        }
        {
            // nodes without a thread of their own: the application's thread pumps them, so a blocking call there is no use (it would
            // wait for itself); the futures run on the pool and the pump delivers
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "ns_nothread", "tcp" );
            REQUIRE_TRUE( group.Start( 2 ) );
            REQUIRE_TRUE( group[ 0 ].GetResources().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
            DRN::CDRNNamespaceClient client( group[ 1 ], LocalPeerOf( group[ 0 ] ) );
            DRN::CDRNNamespaceResponsePtr shares, listing;
            CORE::CFutureResult f1 = client.SharesAsync( shares );
            CORE::CFutureResult f2 = client.ListAllAsync( "files", "sub", listing );
            UInt64 start = DRN::CDRNClock::NowInMilliseconds();
            while ( ( !f1.IsReady() || !f2.IsReady() ) && DRN::CDRNClock::NowInMilliseconds() - start < 15000 )
                PumpFor( 5 );
            REQUIRE_TRUE( f1.IsReady() && f2.IsReady() );
            ASSERT_TRUE( shares->result.IsOk() && 1 == shares->shares.size() );
            ASSERT_TRUE( listing->result.IsOk() && 3 == listing->entries.size() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a link lost between pages ends the listing with UNAVAILABLE, and a request under way when the link goes is failed at once" )
    try
    {
        CORE::CString root = TestDirectory( "linkloss" );
        MakeTree( root, 300 );
        {
            CNsFixture fixture( "ns_loss1" );
            REQUIRE_TRUE( fixture.Start() );
            REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
            DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );
            DRN::CDRNNamespaceListPage page;
            REQUIRE_TRUE( client.List( "files", "big", CORE::CString::Empty, 50, CORE::CString::Empty, page ).IsOk() && !page.IsLast() );
            REQUIRE_TRUE( 1 == fixture.Server().GetNamespaceListingCount( 0 ) );
            fixture.group.nodes[ 0 ]->Stop();
            PumpFor( 400 );
            DRN::CDRNNamespaceResult result = client.List( "files", "big", page.nextCursor, 50, CORE::CString::Empty, page );
            ASSERT_TRUE( DRN::DRN_NSRESULT_UNAVAILABLE == result.code && result.IsRetryable() );
            TEntries all;
            CORE::CString rev;
            bool flag = false;
            ASSERT_TRUE( DRN::DRN_NSRESULT_UNAVAILABLE == client.ListAll( "files", "big", all, rev, flag ).code );
            ASSERT_TRUE( 0 == fixture.Server().GetNamespaceListingCount( 0 ) );
        }
        {
            CNsFixture fixture( "ns_loss2" );
            REQUIRE_TRUE( fixture.Start() );
            REQUIRE_TRUE( fixture.Server().RegisterProvider( "slow", DRN::CIDRNResourceProviderPtr( GUCEF_NEW CSlowProvider( root, 2000 ) ), ShareAll() ) );
            DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );
            DRN::CDRNNamespaceInfo info;
            REQUIRE_TRUE( client.Negotiate( info ).IsOk() );
            DRN::CDRNNamespaceResponsePtr response;
            CORE::CFutureResult future = client.StatAsync( "slow", Strings1( "a.bin" ), TStrings(), response, 20000 );
            PumpFor( 400 );
            ASSERT_TRUE( 1 == fixture.Member().GetNamespaceRequestCount() );
            UInt64 start = DRN::CDRNClock::NowInMilliseconds();
            fixture.group.nodes[ 0 ]->Stop();
            future.Await( 20000 );
            ASSERT_TRUE( DRN::DRN_NSRESULT_UNAVAILABLE == response->result.code );
            ASSERT_TRUE( DRN::CDRNClock::NowInMilliseconds() - start < 8000 );
            ASSERT_TRUE( 0 == fixture.Member().GetNamespaceRequestCount() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: random bodies for every opcode of the namespace never crash the serving side, never close the link and are answered or counted" )
    try
    {
        CORE::CString root = TestDirectory( "fuzz" );
        MakeTree( root, 30 );
        CNsFixture fixture( "ns_fuzz" );
        REQUIRE_TRUE( fixture.Start() );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );

        UInt32 state = 0x2545F491;
        UInt32 answered = 0;
        const UInt32 opcodes[] = { 16, 17, 18, 19, 20, 21, 23, 32, 33, 40, 47, 48, 55, 63 };
        for ( UInt32 o=0; o<sizeof( opcodes ) / sizeof( opcodes[ 0 ] ); ++o )
        {
            for ( UInt32 round=0; round<120; ++round )
            {
                CORE::CDynamicBuffer garbage;
                DRN::CDRNWireWriter writer( garbage );
                UInt32 shape = round % 4;
                if ( 1 == shape && ( 18 == opcodes[ o ] || 19 == opcodes[ o ] ) )
                    DRN::CDRNNamespaceCodec::WriteText( writer, "files" );      // a share that exists, then noise
                UInt32 length = 0;
                state ^= state << 13; state ^= state >> 17; state ^= state << 5;
                length = state % 90;
                for ( UInt32 i=0; i<length; ++i )
                {
                    state ^= state << 13; state ^= state >> 17; state ^= state << 5;
                    UInt8 byte = (UInt8) ( state >> 11 );
                    if ( 2 == shape )
                        byte = 0 == ( state & 3 ) ? 0xFF : (UInt8) ( byte & 0x07 );      // long varints and small numbers
                    writer.WriteUInt8( byte );
                }
                CForged forged = Forge( fixture, opcodes[ o ], garbage, 5000 );
                ASSERT_TRUE( DRN::DRN_RESSTATUS_TIMEOUT != forged.status && DRN::DRN_RESSTATUS_LINK_LOST != forged.status );
                ++answered;
            }
        }

        // every request cut short at every length is answered PROTOCOL_ERROR; the whole of it is answered
        for ( UInt32 kind=0; kind<3; ++kind )
        {
            UInt32 sampleId = 100;
            CORE::CDynamicBuffer full;
            DRN::CDRNWireWriter fullWriter( full );
            UInt32 opcode = DRN::DRN_RES_NS_HELLO;
            if ( 0 == kind )
            {
                DRN::CDRNNamespaceCodec::WriteHelloRequest( fullWriter, sampleId, 1, 0 );
            }
            else
            if ( 1 == kind )
            {
                opcode = DRN::DRN_RES_NS_STAT;
                DRN::CDRNNamespaceCodec::WriteStatRequestHeader( fullWriter, sampleId, "files", 2 );
                DRN::CDRNNamespaceCodec::WriteStatRequestItem( fullWriter, "a.bin", "r" );
                DRN::CDRNNamespaceCodec::WriteStatRequestItem( fullWriter, "sub", "" );
            }
            else
            {
                opcode = DRN::DRN_RES_NS_LIST;
                DRN::CDRNNamespaceCodec::WriteListRequest( fullWriter, sampleId, "files", "sub", "", 5, "r" );
            }
            UInt32 idBytes = DRN::CDRNWireWriter::GetVarUInt32Size( sampleId );
            UInt32 rest = full.GetDataSize() - idBytes;
            for ( UInt32 cut=0; cut<=rest; ++cut )
            {
                CORE::CDynamicBuffer part;
                if ( cut > 0 )
                {
                    REQUIRE_TRUE( part.SetBufferSize( cut, false, true ) );
                    part.SetDataSize( cut );
                    memcpy( part.GetBufferPtr(), static_cast< const UInt8* >( full.GetConstBufferPtr() ) + idBytes, cut );
                }
                CForged forged = Forge( fixture, opcode, part );
                ASSERT_TRUE( ( cut == rest ? DRN::DRN_RESSTATUS_OK : DRN::DRN_RESSTATUS_PROTOCOL_ERROR ) == forged.status );
            }
        }

        // messages that have no answer: no request id, answers sent to a serving side, numbers of nothing
        const UInt32 silent[] = { 24, 25, 28, 31, 64, 100, 9, 12, 15 };
        for ( UInt32 o=0; o<sizeof( silent ) / sizeof( silent[ 0 ] ); ++o )
        {
            for ( UInt32 round=0; round<40; ++round )
            {
                CORE::CDynamicBuffer garbage;
                DRN::CDRNWireWriter writer( garbage );
                state ^= state << 13; state ^= state >> 17; state ^= state << 5;
                UInt32 length = round % 10 == 0 ? 0 : state % 70;
                for ( UInt32 i=0; i<length; ++i )
                {
                    state ^= state << 13; state ^= state >> 17; state ^= state << 5;
                    writer.WriteUInt8( (UInt8) ( state >> 9 ) );
                }
                MT::CScopeMutex lock( fixture.group[ 1 ].GetNode().GetLock() );
                DRN::CDRNPeerLinkPtr link = fixture.group[ 1 ].GetLinkToMember( fixture.serverPeer );
                REQUIRE_TRUE( !link.IsNULL() );
                link->Send( DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_RESOURCE, silent[ o ], garbage.GetConstBufferPtr(), garbage.GetDataSize() );
            }
        }
        PumpFor( 500 );

        // the link is up on both sides, the serving side counted what it could not read, and it still works
        {
            MT::CScopeMutex lockA( fixture.group[ 0 ].GetNode().GetLock() );
            DRN::CDRNPeerLinkPtr linkA = fixture.group[ 0 ].GetLinkToMember( LocalPeerOf( fixture.group[ 1 ] ) );
            ASSERT_TRUE( !linkA.IsNULL() && linkA->IsOperational() );
        }
        {
            MT::CScopeMutex lockB( fixture.group[ 1 ].GetNode().GetLock() );
            DRN::CDRNPeerLinkPtr linkB = fixture.group[ 1 ].GetLinkToMember( fixture.serverPeer );
            ASSERT_TRUE( !linkB.IsNULL() && linkB->IsOperational() );
        }
        DRN::CDRNResourceTransferStats stats = fixture.Server().GetStats();
        ASSERT_TRUE( stats.namespaceMalformed > 0 && stats.droppedMessages > 0 && stats.namespaceRequestsRefused > 0 );
        ASSERT_TRUE( 0 == fixture.group[ 0 ].GetNode().GetUnhandledMessageCount() );
        GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
        ASSERT_TRUE( client.Shares( shares ).IsOk() && 1 == shares.size() );
        TEntries all;
        CORE::CString rev;
        bool flag = false;
        ASSERT_TRUE( client.ListAll( "files", "big", all, rev, flag ).IsOk() && 30 == all.size() );
        ASSERT_TRUE( answered == sizeof( opcodes ) / sizeof( opcodes[ 0 ] ) * 120 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN namespace fuzz: " << answered << " answered, serving side counted " << stats.namespaceMalformed << " malformed and " << stats.droppedMessages << " dropped messages" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a share over the virtual file system lists, stats and opens with the same revisions" )
    try
    {
        CORE::CString root = TestDirectory( "vfs" );
        MakeTree( root, 60 );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        REQUIRE_TRUE( vfs.AddRoot( root, "DrnNamespaceTest", false, false, "drnns" ) );
        CNsFixture fixture( "ns_vfs" );
        REQUIRE_TRUE( fixture.Start() );
        CORE::TFileSystemAccessPtr vfsAccess = ( GUCEF_NEW VFS::CVfsFileSystemAccess() )->CreateSharedPtr();
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "vfs", DRN::CIDRNResourceProviderPtr( GUCEF_NEW DRN::CDRNFileSystemAccessResourceProvider( vfsAccess, "/drnns", CORE::CUri( "vfs://drnns" ) ) ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );

        TEntries root_entries;
        CORE::CString rev;
        bool flag = false;
        REQUIRE_TRUE( client.ListAll( "vfs", "", root_entries, rev, flag ).IsOk() );
        TStringSet names = NamesOf( root_entries );
        ASSERT_TRUE( names.count( "a.bin" ) && names.count( "sub" ) && names.count( "big" ) && names.count( "known.bin" ) );
        const DRN::CDRNNamespaceEntry* a = FindEntry( root_entries, "a.bin" );
        REQUIRE_TRUE( GUCEF_NULL != a );
        ASSERT_TRUE( a->IsFile() && 1000 == a->size );
        ASSERT_TRUE( FindEntry( root_entries, "sub" )->IsDirectory() );
        TEntries big;
        REQUIRE_TRUE( client.ListAll( "vfs", "big", big, rev, flag, 1000, 25 ).IsOk() && 60 == big.size() );
        TEntries stats;
        REQUIRE_TRUE( client.Stat( "vfs", Strings1( "a.bin" ), TStrings(), stats ).IsOk() && DRN::DRN_RESSTATUS_OK == stats[ 0 ].status );
        ASSERT_TRUE( stats[ 0 ].revision == a->revision && stats[ 0 ].size == 1000 );
        DRN::CDRNResourceHandlePtr handle = fixture.Member().OpenHandle( fixture.serverPeer, "vfs", "a.bin", DRN::CDRNResourceTransferOptions() );
        REQUIRE_TRUE( handle->WaitOpen( 10000 ) );
        ASSERT_TRUE( handle->GetInfo().revision == a->revision );
        CORE::CDynamicBuffer data;
        REQUIRE_TRUE( handle->ReadRange( 10, 100, data ).IsOk() && 100 == data.GetDataSize() );
        const UInt8* bytes = static_cast< const UInt8* >( data.GetConstBufferPtr() );
        bool same = true;
        for ( UInt32 i=0; i<100; ++i )
            same = same && bytes[ i ] == (UInt8) ( 3 + ( 10 + i ) * 7 );
        ASSERT_TRUE( same );
        handle->Close();
        DRN::CDRNNamespaceListPage missingPage;
        ASSERT_TRUE( DRN::DRN_NSRESULT_NOT_FOUND == client.List( "vfs", "nothere", CORE::CString::Empty, 0, CORE::CString::Empty, missingPage ).code );
        vfs.UnmountArchiveByName( "DrnNamespaceTest" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: the thread that pumps a node without a thread of its own makes blocking calls that are answered: shares, stat, paged list, handle open and sequential read, transfers" )
    try
    {
        CORE::CString root = TestDirectory( "pump" );
        MakeTree( root, 120 );
        REQUIRE_TRUE( WriteFile( Join( root, "multi.bin" ), 200000, 9 ) );
        CPumpFixture fixture( "ns_pump" );
        REQUIRE_TRUE( fixture.Start() );
        CPumpThreadScope pumpScope;
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        REQUIRE_TRUE( fixture.clientNode.IsPumpThread() );
        ASSERT_TRUE( !fixture.serverNode.IsPumpThread() );
        UInt64 start = DRN::CDRNClock::NowInMilliseconds();

        // the namespace: calls that wait for an answer of the member
        DRN::CDRNNamespaceClient client( *fixture.client, fixture.serverPeer );
        client.SetDefaultTimeoutMs( 4000 );
        GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
        REQUIRE_TRUE( client.Shares( shares ).IsOk() && 1 == shares.size() );
        TStrings paths;
        paths.push_back( "a.bin" ); paths.push_back( "known.bin" ); paths.push_back( "multi.bin" ); paths.push_back( "missing.bin" );
        TEntries stats;
        REQUIRE_TRUE( client.Stat( "files", paths, TStrings(), stats ).IsOk() && 4 == stats.size() );
        ASSERT_TRUE( 1000 == stats[ 0 ].size && 70000 == stats[ 1 ].size && 200000 == stats[ 2 ].size && DRN::DRN_RESSTATUS_NOT_FOUND == stats[ 3 ].status );
        DRN::CDRNNamespaceListPage page;
        REQUIRE_TRUE( client.List( "files", "big", CORE::CString::Empty, 50, CORE::CString::Empty, page ).IsOk() && 50 == page.entries.size() && !page.IsLast() );
        UInt32 pages = 1;
        UInt32 listed = (UInt32) page.entries.size();
        while ( !page.IsLast() && pages < 20 )
        {
            CORE::CString cursor = page.nextCursor;
            REQUIRE_TRUE( client.List( "files", "big", cursor, 50, CORE::CString::Empty, page ).IsOk() );
            listed += (UInt32) page.entries.size();
            ++pages;
        }
        ASSERT_TRUE( 120 == listed && 3 == pages );
        TEntries all;
        CORE::CString dirRevision;
        bool changedDuringListing = false;
        ASSERT_TRUE( client.ListAll( "files", "big", all, dirRevision, changedDuringListing ).IsOk() && 120 == all.size() );

        // a handle: the open waits for the serving side, a sequential read of a file of several chunks waits for each window
        DRN::CDRNResourceHandlePtr handle = fixture.Member().OpenHandle( fixture.serverPeer, "files", "multi.bin", ShortOptions( 4000 ) );
        REQUIRE_TRUE( handle->WaitOpen( 4000 ) );
        ASSERT_TRUE( 200000 == handle->GetInfo().size );
        DRN::CDRNResourceIOAccess access( handle );
        access.Open();
        REQUIRE_TRUE( access.Opened() && 200000 == access.GetSize() );
        UInt8 block[ 30000 ];
        UInt64 position = 0;
        bool sequentialGood = true;
        while ( position < 200000 )
        {
            UInt32 want = 200000 - position < sizeof( block ) ? (UInt32) ( 200000 - position ) : (UInt32) sizeof( block );
            if ( want != access.Read( block, 1, want ) || !MatchesPattern( block, want, position, 9 ) )
            {
                sequentialGood = false;
                break;
            }
            position += want;
        }
        ASSERT_TRUE( sequentialGood && 200000 == position );
        CORE::CDynamicBuffer window;
        ASSERT_TRUE( handle->ReadRange( 70000, 100000, window ).IsOk() && 100000 == window.GetDataSize() && MatchesPattern( window.GetConstBufferPtr(), 100000, 70000, 9 ) );
        access.Close();
        handle->Close();

        // a transfer: waits for the resource to arrive in its sink
        DRN::CDRNMemoryResourceSink* sink = GUCEF_NEW DRN::CDRNMemoryResourceSink();
        DRN::CIDRNResourceSinkPtr sinkPtr( sink );
        DRN::CDRNResourceTransfer* transfer = fixture.Member().Fetch( fixture.serverPeer, "files", "known.bin", sinkPtr, ShortOptions( 4000 ) );
        REQUIRE_TRUE( GUCEF_NULL != transfer );
        ASSERT_TRUE( transfer->WaitForCompletion( 4000 ) && transfer->GetResult().IsOk() );
        CORE::CDynamicBuffer fetched;
        sink->TakeData( fetched );
        ASSERT_TRUE( 70000 == fetched.GetDataSize() && MatchesPattern( fetched.GetConstBufferPtr(), 70000, 0, 5 ) );
        fixture.Member().Release( transfer );

        // a window of a resource into memory: a transfer and its wait in one call
        CORE::CDynamicBuffer ranged;
        DRN::CDRNResourceTransferResult result = fixture.Member().ReadRange( fixture.serverPeer, "files", "multi.bin", 777, 150000, ranged, ShortOptions( 4000 ), 4000 );
        ASSERT_TRUE( result.IsOk() && 150000 == ranged.GetDataSize() && MatchesPattern( ranged.GetConstBufferPtr(), 150000, 777, 9 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_NOT_FOUND == fixture.Member().ReadRange( fixture.serverPeer, "files", "nothere.bin", 0, 10, ranged, ShortOptions( 4000 ), 4000 ).status );

        // the calls did not wait for their timeouts
        ASSERT_TRUE( DRN::CDRNClock::NowInMilliseconds() - start < 12000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a server that never answers: every blocking call of the pump thread ends at its timeout, none hangs, and the node answers again once the server does" )
    try
    {
        CORE::CString root = TestDirectory( "pumpsilent" );
        MakeTree( root, 5 );
        CPumpFixture fixture( "ns_pump_silent" );
        REQUIRE_TRUE( fixture.Start() );
        CPumpThreadScope pumpScope;
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( *fixture.client, fixture.serverPeer );
        DRN::CDRNNamespaceInfo info;
        REQUIRE_TRUE( client.Negotiate( info, 4000 ).IsOk() );
        DRN::CDRNResourceTransferOptions windowOptions = ShortOptions( 4000 );
        windowOptions.idleTimeoutMs = 300;
        DRN::CDRNResourceHandlePtr openHandle = fixture.Member().OpenHandle( fixture.serverPeer, "files", "known.bin", windowOptions );
        REQUIRE_TRUE( openHandle->WaitOpen( 4000 ) );

        {
            // the serving node is held: its thread cannot take its node lock, nothing is answered
            MT::CScopeMutex held( fixture.serverNode.GetLock() );

            UInt64 start = DRN::CDRNClock::NowInMilliseconds();
            GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
            DRN::CDRNNamespaceResult nsResult = client.Shares( shares, 300 );
            UInt64 elapsed = DRN::CDRNClock::NowInMilliseconds() - start;
            ASSERT_TRUE( DRN::DRN_NSRESULT_TIMEOUT == nsResult.code );
            ASSERT_TRUE( elapsed >= 250 && elapsed < 2500 );

            start = DRN::CDRNClock::NowInMilliseconds();
            CORE::CDynamicBuffer data;
            DRN::CDRNResourceTransferResult readResult = fixture.Member().ReadRange( fixture.serverPeer, "files", "known.bin", 0, 1000, data, ShortOptions( 300 ), 300 );
            elapsed = DRN::CDRNClock::NowInMilliseconds() - start;
            ASSERT_TRUE( DRN::DRN_RESSTATUS_TIMEOUT == readResult.status );
            ASSERT_TRUE( elapsed >= 250 && elapsed < 4000 );

            start = DRN::CDRNClock::NowInMilliseconds();
            DRN::CDRNResourceHandlePtr silent = fixture.Member().OpenHandle( fixture.serverPeer, "files", "a.bin", ShortOptions( 300 ) );
            ASSERT_FALSE( silent->WaitOpen( 300 ) );
            elapsed = DRN::CDRNClock::NowInMilliseconds() - start;
            ASSERT_TRUE( DRN::DRN_RESSTATUS_TIMEOUT == silent->GetResult().status );
            ASSERT_TRUE( elapsed >= 250 && elapsed < 2500 );
            silent->Close();

            // a window of a handle that is open: the wait for the data
            start = DRN::CDRNClock::NowInMilliseconds();
            DRN::CDRNResourceTransferResult windowResult = openHandle->ReadRange( 0, 1000, data );
            elapsed = DRN::CDRNClock::NowInMilliseconds() - start;
            ASSERT_TRUE( !windowResult.IsOk() && DRN::DRN_RESSTATUS_TIMEOUT == windowResult.status );
            ASSERT_TRUE( elapsed >= 250 && elapsed < 4000 );

            start = DRN::CDRNClock::NowInMilliseconds();
            DRN::CDRNMemoryResourceSink* sink = GUCEF_NEW DRN::CDRNMemoryResourceSink();
            DRN::CIDRNResourceSinkPtr sinkPtr( sink );
            DRN::CDRNResourceTransfer* transfer = fixture.Member().Fetch( fixture.serverPeer, "files", "known.bin", sinkPtr, ShortOptions( 4000 ) );
            REQUIRE_TRUE( GUCEF_NULL != transfer );
            ASSERT_FALSE( transfer->WaitForCompletion( 300 ) );
            elapsed = DRN::CDRNClock::NowInMilliseconds() - start;
            ASSERT_TRUE( elapsed >= 250 && elapsed < 2500 );
            transfer->Cancel();
            fixture.Member().Release( transfer );
        }

        // the server works again: the same client, the same links
        client.SetDefaultTimeoutMs( 4000 );
        GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
        ASSERT_TRUE( client.Shares( shares ).IsOk() && 1 == shares.size() );
        openHandle->Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a pump thread that calls before its link is up gets a clean failure at once, not a wait for the timeout" )
    try
    {
        CORE::CString root = TestDirectory( "pumpnolink" );
        MakeTree( root, 5 );
        CPumpFixture fixture( "ns_pump_nolink" );
        REQUIRE_TRUE( fixture.Start( false ) );
        CPumpThreadScope pumpScope;
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( *fixture.client, fixture.serverPeer );

        // the client session has not joined: there is no link to the member
        UInt64 start = DRN::CDRNClock::NowInMilliseconds();
        DRN::CDRNNamespaceInfo info;
        DRN::CDRNNamespaceResult nsResult = client.Negotiate( info, 8000 );
        ASSERT_TRUE( !nsResult.IsOk() && !nsResult.message.IsNULLOrEmpty() );
        ASSERT_TRUE( DRN::CDRNClock::NowInMilliseconds() - start < 1000 );
        start = DRN::CDRNClock::NowInMilliseconds();
        CORE::CDynamicBuffer data;
        // a session that has not joined does not tick, so a transfer does not start: it ends at its timeout, cleanly
        DRN::CDRNResourceTransferResult readResult = fixture.Member().ReadRange( fixture.serverPeer, "files", "a.bin", 0, 10, data, ShortOptions( 8000 ), 400 );
        ASSERT_TRUE( !readResult.IsOk() && DRN::DRN_RESSTATUS_TIMEOUT == readResult.status );
        UInt64 elapsed = DRN::CDRNClock::NowInMilliseconds() - start;
        ASSERT_TRUE( elapsed >= 350 && elapsed < 3000 );
        start = DRN::CDRNClock::NowInMilliseconds();
        DRN::CDRNResourceHandlePtr handle = fixture.Member().OpenHandle( fixture.serverPeer, "files", "a.bin", ShortOptions( 8000 ) );
        ASSERT_FALSE( handle->WaitOpen( 8000 ) );
        ASSERT_TRUE( DRN::CDRNClock::NowInMilliseconds() - start < 1000 );
        ASSERT_TRUE( !handle->GetResult().IsOk() && !handle->GetResult().message.IsNULLOrEmpty() );

        // and the link comes up afterwards: the same objects serve
        REQUIRE_TRUE( fixture.Join() );
        client.SetDefaultTimeoutMs( 4000 );
        ASSERT_TRUE( client.Negotiate( info ).IsOk() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a call from inside work of the pumped node (a handler it delivers) is still refused, and nothing is pumped there" )
    try
    {
        CORE::CString root = TestDirectory( "pumprefuse" );
        MakeTree( root, 5 );
        CPumpFixture fixture( "ns_pump_refuse" );
        REQUIRE_TRUE( fixture.Start( false ) );
        CPumpThreadScope pumpScope;
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( *fixture.client, fixture.serverPeer );
        CInHandlerProbe probe;
        probe.client = &client;
        probe.session = fixture.client;
        probe.peerId = fixture.serverPeer;
        probe.SubscribeTo( &fixture.clientNode, DRN::CDRNNode::LinkOperationalEvent );

        // the link becomes operational in a cycle of the client node, which delivers the event on this very thread
        REQUIRE_TRUE( fixture.Join() );
        REQUIRE_TRUE( 1 == probe.calls );
        ASSERT_TRUE( probe.working );
        ASSERT_TRUE( !probe.pumpThread );
        ASSERT_TRUE( DRN::DRN_NSRESULT_WRONG_THREAD == probe.namespaceResult.code );
        ASSERT_TRUE( probe.namespaceMs < 500 );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_TIMEOUT == probe.readResult.status );
        ASSERT_TRUE( !probe.pumpUntilMet && probe.pumpUntilMs < 500 );

        // outside of the handler the same thread is the pump again
        ASSERT_TRUE( fixture.clientNode.IsPumpThread() );
        client.SetDefaultTimeoutMs( 4000 );
        DRN::CDRNNamespaceInfo info;
        ASSERT_TRUE( client.Negotiate( info ).IsOk() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: two nodes without a thread of their own on the one pump thread: the caller works its own node only, so a call to the other ends at its timeout" )
    try
    {
        CORE::CString root = TestDirectory( "pumptwo" );
        MakeTree( root, 5 );
        CPumpFixture fixture( "ns_pump_two", false, false );
        REQUIRE_TRUE( fixture.Start() );
        CPumpThreadScope pumpScope;
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        REQUIRE_TRUE( fixture.clientNode.IsPumpThread() && fixture.serverNode.IsPumpThread() );
        DRN::CDRNNamespaceClient client( *fixture.client, fixture.serverPeer );
        UInt64 start = DRN::CDRNClock::NowInMilliseconds();
        GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
        DRN::CDRNNamespaceResult nsResult = client.Shares( shares, 400 );
        UInt64 elapsed = DRN::CDRNClock::NowInMilliseconds() - start;
        ASSERT_TRUE( DRN::DRN_NSRESULT_TIMEOUT == nsResult.code );
        ASSERT_TRUE( elapsed >= 350 && elapsed < 3000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Namespace: a node with a thread of its own is never pumped by the caller, whatever thread that is; a thread that is not the pump thread waits for the pump" )
    try
    {
        CORE::CString root = TestDirectory( "pumpown" );
        MakeTree( root, 5 );
        {
            // the calling thread drives the application's pulse generator, the client node works on a thread of its own
            CPumpFixture fixture( "ns_pump_own", true );
            REQUIRE_TRUE( fixture.Start() );
            CPumpThreadScope pumpScope;
            REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
            ASSERT_TRUE( fixture.clientNode.IsRunningOwnThread() );
            ASSERT_TRUE( !fixture.clientNode.IsPumpThread() && !fixture.serverNode.IsPumpThread() );
            DRN::CDRNNamespaceClient client( *fixture.client, fixture.serverPeer );
            client.SetDefaultTimeoutMs( 4000 );
            GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
            ASSERT_TRUE( client.Shares( shares ).IsOk() && 1 == shares.size() );
            CORE::CDynamicBuffer data;
            ASSERT_TRUE( fixture.Member().ReadRange( fixture.serverPeer, "files", "known.bin", 0, 70000, data, ShortOptions( 4000 ), 4000 ).IsOk() && 70000 == data.GetDataSize() );

            // a condition that is never met is checked once for a thread that is not the pump thread, nothing is waited for
            CNeverCondition never;
            UInt64 start = DRN::CDRNClock::NowInMilliseconds();
            ASSERT_FALSE( fixture.clientNode.PumpUntil( never, 1500 ) );
            ASSERT_TRUE( DRN::CDRNClock::NowInMilliseconds() - start < 500 );
        }
        {
            // another thread than the one that drives the application's pulse generator makes the call while that thread does not pump:
            // it is not the pump thread and waits for a pump that does not work the node, ending at its timeout
            CPumpFixture fixture( "ns_pump_other" );
            REQUIRE_TRUE( fixture.Start() );
            CPumpThreadScope pumpScope;
            REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
            REQUIRE_TRUE( fixture.clientNode.IsPumpThread() );
            DRN::CDRNNamespaceClient client( *fixture.client, fixture.serverPeer );
            CForeignCaller caller( fixture.clientNode, client );
            REQUIRE_TRUE( caller.Activate( GUCEF_NULL, 0, 0 ) );
            UInt64 start = DRN::CDRNClock::NowInMilliseconds();
            while ( !caller.IsDone() && DRN::CDRNClock::NowInMilliseconds() - start < 15000 )
                MT::PrecisionDelay( 5 );
            REQUIRE_TRUE( caller.IsDone() );
            caller.Deactivate( false, true );
            ASSERT_TRUE( !caller.WasPumpThread() );
            ASSERT_TRUE( (UInt32) DRN::DRN_NSRESULT_TIMEOUT == caller.GetCode() );
            ASSERT_TRUE( caller.GetElapsedMs() >= 350 && caller.GetElapsedMs() < 3000 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // The node that waits for a job of the pool is woken once the job's future is ready: not from inside the job, not twice, and not
    // for a job that nobody waits for; a job that outlives the pulse generator it names does no harm
    GUCEF_TESTFW_TESTCASE( "Resource IO jobs: the pulse generator of the waiting node is asked for an immediate pulse once per job, after the future of the job is ready; no wake without a pulse generator; a job that outlives its pulse generator wakes nothing" )
    try
    {
        const CORE::CString poolName = "gucefDRN_TestApp.IoWake";
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( poolName );
        REQUIRE_TRUE( !pool.IsNULL() );
        pool->SetDesiredMaxTotalNrOfThreads( 2 );
        pool->SetDesiredMinNrOfWorkerThreads( 2 );
        for ( UInt32 i=0; i<500 && 2 != pool->GetActiveNrOfWorkerThreads(); ++i )
            MT::ThreadDelay( 5 );
        REQUIRE_TRUE( 2 == pool->GetActiveNrOfWorkerThreads() );

        CWakeCountingDriver driver;
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
        pulseGenerator->SetPulseGeneratorDriver( &driver );
        driver.wakes.Store( 0 );        // a driver that is set may be asked for a pulse right away

        // once per job, when the future is ready: the job waits at its gate while the test watches its future
        const UInt32 jobCount = 50;
        for ( UInt32 i=0; i<jobCount; ++i )
        {
            CWakeProbeJob* job = GUCEF_NEW CWakeProbeJob();
            DRN::CIDRNResourceIoJobPtr jobPtr( job );
            CORE::CFutureResult future = DRN::CDRNResourceIoTaskConsumer::Queue( poolName, jobPtr, pulseGenerator );
            REQUIRE_TRUE( future.HasAFuture() );
            driver.watched.Store( &future );
            ASSERT_TRUE( 0 == driver.wakes.Load() - i );
            job->gateOpen.Store( 1 );
            for ( UInt32 waited=0; waited<2000 && driver.wakes.Load() < i + 1; ++waited )
                MT::ThreadDelay( 1 );
            driver.watched.Store( GUCEF_NULL );
            ASSERT_TRUE( future.IsReady() );
            ASSERT_TRUE( 1 == job->executed.Load() );
            ASSERT_TRUE( driver.wakes.Load() == i + 1 );
        }
        MT::ThreadDelay( 50 );
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "ResourceIoJobs: " << jobCount << " jobs gave " << driver.wakes.Load() << " wakes, " << driver.wakesWhileReady.Load() << " of them with the future ready" );
        ASSERT_TRUE( jobCount == driver.wakes.Load() );
        ASSERT_TRUE( jobCount == driver.wakesWhileReady.Load() );

        // a job queued without a pulse generator wakes nobody
        {
            CWakeProbeJob* job = GUCEF_NEW CWakeProbeJob();
            job->gateOpen.Store( 1 );
            DRN::CIDRNResourceIoJobPtr jobPtr( job );
            CORE::CFutureResult future = DRN::CDRNResourceIoTaskConsumer::Queue( poolName, jobPtr );
            REQUIRE_TRUE( future.HasAFuture() );
            ASSERT_TRUE( future.Await( 2000 ).IsReady() );
            DRN::CIDRNResourceIoJobPtr jobPtr2( GUCEF_NEW CWakeProbeJob() );
            static_cast< CWakeProbeJob* >( jobPtr2.GetPointerAlways() )->gateOpen.Store( 1 );
            CORE::CFutureResult future2 = DRN::CDRNResourceIoTaskConsumer::Queue( poolName, jobPtr2, CORE::PulseGeneratorPtr() );
            REQUIRE_TRUE( future2.HasAFuture() );
            ASSERT_TRUE( future2.Await( 2000 ).IsReady() );
            MT::ThreadDelay( 20 );
            ASSERT_TRUE( jobCount == driver.wakes.Load() );
        }

        // the node is gone while its job still runs: the job holds the pulse generator, which has no driver any more
        {
            CWakeCountingDriver goneDriver;
            CORE::PulseGeneratorPtr gonePulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
            gonePulseGenerator->SetPulseGeneratorDriver( &goneDriver );
            goneDriver.wakes.Store( 0 );
            CWakeProbeJob* job = GUCEF_NEW CWakeProbeJob();
            DRN::CIDRNResourceIoJobPtr jobPtr( job );
            CORE::CFutureResult future = DRN::CDRNResourceIoTaskConsumer::Queue( poolName, jobPtr, gonePulseGenerator );
            REQUIRE_TRUE( future.HasAFuture() );
            gonePulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
            gonePulseGenerator.Unlink();
            job->gateOpen.Store( 1 );
            ASSERT_TRUE( future.Await( 2000 ).IsReady() );
            MT::ThreadDelay( 20 );
            ASSERT_TRUE( 1 == job->executed.Load() );
            ASSERT_TRUE( 0 == goneDriver.wakes.Load() );
        }

        pulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
        ASSERT_TRUE( pool->RequestAllThreadsToStop( true, false, 5000 ) );
        taskManager.UnregisterThreadPool( poolName );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // The serving node on a thread of its own with a slow cycle: what it does between cycles is woken by traffic and by the jobs of its
    // pool. The answer of a stat comes from a job, so a stat is answered at once only when the end of the job wakes the node
    GUCEF_TESTFW_TESTCASE( "Namespace: a node on a thread of its own that cycles slowly answers a stat as soon as the job of the pool ended, the wake is one per job and no cycle storm follows" )
    try
    {
        CORE::CString root = TestDirectory( "wake_own" );
        MakeTree( root, 5 );
        CNsFixture fixture( "ns_wake_own" );
        fixture.group.nodeSettings.pulseIntervalMs = 200;
        fixture.group.nodeSettings.idlePulseIntervalMs = 200;
        fixture.group.nodeSettings.echoIntervalMs = 5000;
        REQUIRE_TRUE( fixture.Start() );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( fixture.group[ 1 ], fixture.serverPeer );
        client.SetDefaultTimeoutMs( 4000 );

        TStrings paths;
        paths.push_back( "a.bin" );
        TEntries stats;
        REQUIRE_TRUE( client.Stat( "files", paths, TStrings(), stats ).IsOk() );
        MT::ThreadDelay( 300 );

        CCycleCounter cycles;
        cycles.Watch( fixture.group.nodes[ 0 ]->GetWorkPulseGenerator() );
        const UInt32 calls = 40;
        GUCEF::vector< Float64 > latencies;
        for ( UInt32 i=0; i<calls; ++i )
        {
            MT::ThreadDelay( 7 );
            const UInt64 startTicks = MT::PrecisionTickCount();
            stats.clear();
            REQUIRE_TRUE( client.Stat( "files", paths, TStrings(), stats ).IsOk() && 1 == stats.size() );
            latencies.push_back( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks ) );
        }
        std::sort( latencies.begin(), latencies.end() );
        const Float64 median = latencies[ latencies.size() / 2 ];
        const Float64 worst = latencies.back();
        const UInt32 cycleCount = cycles.cycles.Load();
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "NodeWake: " << calls << " stats against a serving node that cycles every 200 ms: median " << median << " ms, worst " << worst << " ms, serving node cycles " << cycleCount );
        // the wake ends the wait at once; without it a stat waits for the next cycle of the node, 200 ms apart (about 50 ms when the calls are spread over the cycle)
        ASSERT_TRUE( median < 12.0 );
        ASSERT_TRUE( worst < 100.0 );
        // a wake for the request and one for the job: no cycle storm
        ASSERT_TRUE( cycleCount >= calls && cycleCount <= calls * 5 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // The same for a serving node without a thread of its own: its jobs ask the application's pulse generator for a pulse through the
    // driver of the node, and the thread that pumps the node answers at once (the stat needs a job, the share list does not)
    GUCEF_TESTFW_TESTCASE( "Namespace: a pumped serving node answers a stat within the pump's cycle; the end of every job of the pool asks the application's pulse generator for a pulse, once" )
    try
    {
        CORE::CString root = TestDirectory( "wake_pumped" );
        MakeTree( root, 5 );
        CWakePumpFixture fixture( "ns_wake_pumped" );
        REQUIRE_TRUE( fixture.Start() );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        DRN::CDRNNamespaceClient client( *fixture.client, fixture.serverPeer );
        client.SetDefaultTimeoutMs( 4000 );

        TStrings paths;
        paths.push_back( "a.bin" );
        TEntries stats;
        GUCEF::vector< DRN::CDRNNamespaceShareInfo > shares;
        REQUIRE_TRUE( client.Stat( "files", paths, TStrings(), stats ).IsOk() );
        REQUIRE_TRUE( client.Shares( shares ).IsOk() );
        MT::ThreadDelay( 100 );

        // calls that need no job of the pool, for what else asks the pulse generator while the node answers
        const UInt32 calls = 40;
        const UInt32 baseline = fixture.driver.wakes.Load();
        for ( UInt32 i=0; i<calls; ++i )
        {
            MT::ThreadDelay( 3 );
            REQUIRE_TRUE( client.Shares( shares ).IsOk() );
        }
        MT::ThreadDelay( 50 );
        const UInt32 wakesWithoutJobs = fixture.driver.wakes.Load() - baseline;

        GUCEF::vector< Float64 > latencies;
        const UInt32 beforeStats = fixture.driver.wakes.Load();
        for ( UInt32 i=0; i<calls; ++i )
        {
            MT::ThreadDelay( 3 );
            const UInt64 startTicks = MT::PrecisionTickCount();
            stats.clear();
            REQUIRE_TRUE( client.Stat( "files", paths, TStrings(), stats ).IsOk() && 1 == stats.size() );
            latencies.push_back( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks ) );
        }
        MT::ThreadDelay( 50 );
        const UInt32 wakesWithJobs = fixture.driver.wakes.Load() - beforeStats;
        std::sort( latencies.begin(), latencies.end() );
        const Float64 median = latencies[ latencies.size() / 2 ];
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "NodeWake: " << calls << " stats against a pumped serving node: median " << median << " ms, worst " << latencies.back() << " ms; pulse requests of the application's pulse generator "
                   << wakesWithJobs << " with a job per call, " << wakesWithoutJobs << " without" );
        ASSERT_TRUE( median < 8.0 );
        ASSERT_TRUE( wakesWithJobs >= wakesWithoutJobs + calls );
        ASSERT_TRUE( wakesWithJobs <= wakesWithoutJobs + calls + calls / 4 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // The job of a serving node that is gone when the job ends: the wake finds a pulse generator without a driver, and nothing else of the node
    GUCEF_TESTFW_TESTCASE( "Namespace: a job of the pool that ends after its serving node was stopped and destroyed wakes nothing and touches nothing of the node" )
    try
    {
        CORE::CString root = TestDirectory( "wake_gone" );
        MakeTree( root, 5 );
        DRN::CDRNNodeSettings settings;
        settings.echoIntervalMs = 50;
        settings.pulseIntervalMs = 1;
        settings.ownThread = true;
        DRN::CDRNNode* serverNode = GUCEF_NEW DRN::CDRNNode();
        DRN::CDRNNode clientNode;
        REQUIRE_TRUE( serverNode->SetSettings( settings ) && clientNode.SetSettings( settings ) );
        DRN::CDRNEndpoint endpoint = LoopbackEndpoint( "tcp", NextTestPort() );
        REQUIRE_TRUE( serverNode->Listen( endpoint ) );
        DRN::CDRNSession* server = serverNode->CreateSession();
        DRN::CDRNSession* member = clientNode.CreateSession();
        DRN::CDRNSessionSettings sessionSettings;
        sessionSettings.sessionName = "ns_wake_gone";
        sessionSettings.topology = DRN::DRN_TOPOLOGY_CLIENT_SERVER;
        REQUIRE_TRUE( server->Host( sessionSettings ) );
        UInt32 serverPeer = 0;
        {
            MT::CScopeMutex lock( serverNode->GetLock() );
            serverPeer = server->GetLocalPeerId();
        }
        REQUIRE_TRUE( member->Join( DRN::CDRNEndpointList( endpoint ), sessionSettings ) );
        CPumpMembership membership( *server, *member );
        REQUIRE_TRUE( PumpUntil( membership, 15000 ) );
        REQUIRE_TRUE( server->GetResources().RegisterProvider( "slow", DRN::CIDRNResourceProviderPtr( GUCEF_NEW CSlowProvider( root, 400 ) ), ShareAll() ) );
        CORE::PulseGeneratorPtr workPulseGenerator = serverNode->GetWorkPulseGenerator();

        {
            DRN::CDRNNamespaceClient client( *member, serverPeer );
            client.SetDefaultTimeoutMs( 800 );
            TStrings paths;
            paths.push_back( "a.bin" );
            DRN::CDRNNamespaceResponsePtr response;
            CORE::CFutureResult future = client.StatAsync( "slow", paths, TStrings(), response, 800 );
            REQUIRE_TRUE( future.HasAFuture() );

            // the serving side has its job under way (it takes 400 ms): the node goes
            MT::ThreadDelay( 120 );
            serverNode->Stop();
            GUCEF_DELETE serverNode;
            serverNode = GUCEF_NULL;
            ASSERT_TRUE( GUCEF_NULL == workPulseGenerator->GetPulseGeneratorDriver() );

            // the call ends (the link is gone), the job ends later: nothing of the node is left to touch
            ASSERT_TRUE( future.Await( 5000 ).IsReady() );
            MT::ThreadDelay( 600 );
        }
        clientNode.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // Every job the facility queues names the node that waits for it: serving side (stat, list, open, tree, read) and requesting side
    // (prepare, drain, finish, abort of a transfer). A job that does not wake its node leaves it waiting for its next cycle
    GUCEF_TESTFW_TESTCASE( "Namespace: every job of the pool that the facility queues, on the serving and on the requesting side, asks the pulse generator of its node for exactly one pulse when it ends" )
    try
    {
        CORE::CString root = TestDirectory( "wake_sites" );
        MakeTree( root, 30 );
        REQUIRE_TRUE( WriteFile( Join( root, "multi.bin" ), 600000, 9 ) );
        REQUIRE_TRUE( WriteFile( Join( root, "huge.bin" ), 24 * 1024 * 1024, 4 ) );
        CWakePumpFixture fixture( "ns_wake_sites", true );
        REQUIRE_TRUE( fixture.Start() );
        REQUIRE_TRUE( fixture.Server().RegisterProvider( "files", NsProvider( root ), ShareAll() ) );
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( CORE::CTaskManager::DefaultThreadPoolName );
        REQUIRE_TRUE( !pool.IsNULL() );
        DRN::CDRNNamespaceClient client( *fixture.client, fixture.serverPeer );
        client.SetDefaultTimeoutMs( 4000 );
        MT::ThreadDelay( 100 );

        CORE::CThreadPoolCounters before;
        pool->GetCounters( before );
        const UInt32 wakesBefore = fixture.driver.wakes.Load() + fixture.clientDriver.wakes.Load();

        // the namespace
        TStrings paths;
        paths.push_back( "a.bin" ); paths.push_back( "multi.bin" );
        TEntries stats;
        REQUIRE_TRUE( client.Stat( "files", paths, TStrings(), stats ).IsOk() && 2 == stats.size() );
        DRN::CDRNNamespaceListPage page;
        REQUIRE_TRUE( client.List( "files", "big", CORE::CString::Empty, 10, CORE::CString::Empty, page ).IsOk() && 10 == page.entries.size() );

        // a handle and the windows of it
        DRN::CDRNResourceHandlePtr handle = fixture.Member().OpenHandle( fixture.serverPeer, "files", "multi.bin", ShortOptions( 4000 ) );
        REQUIRE_TRUE( handle->WaitOpen( 4000 ) );
        CORE::CDynamicBuffer window;
        ASSERT_TRUE( handle->ReadRange( 70000, 300000, window ).IsOk() && 300000 == window.GetDataSize() && MatchesPattern( window.GetConstBufferPtr(), 300000, 70000, 9 ) );
        handle->Close();

        // a transfer into memory
        DRN::CDRNMemoryResourceSink* sink = GUCEF_NEW DRN::CDRNMemoryResourceSink();
        DRN::CIDRNResourceSinkPtr sinkPtr( sink );
        DRN::CDRNResourceTransferOptions treeOptions = ShortOptions( 4000 );
        treeOptions.verifyMode = DRN::CDRNResourceTransferOptions::VERIFY_TREE_IF_AVAILABLE;       // the serving side builds the hash tree on the pool
        DRN::CDRNResourceTransfer* transfer = fixture.Member().Fetch( fixture.serverPeer, "files", "multi.bin", sinkPtr, treeOptions );
        REQUIRE_TRUE( GUCEF_NULL != transfer );
        ASSERT_TRUE( transfer->WaitForCompletion( 8000 ) && transfer->GetResult().IsOk() );
        CORE::CDynamicBuffer fetched;
        sink->TakeData( fetched );
        ASSERT_TRUE( 600000 == fetched.GetDataSize() );
        fixture.Member().Release( transfer );

        // a transfer that is given up while it runs
        DRN::CDRNMemoryResourceSink* abortedSink = GUCEF_NEW DRN::CDRNMemoryResourceSink();
        DRN::CIDRNResourceSinkPtr abortedSinkPtr( abortedSink );
        DRN::CDRNResourceTransfer* aborted = fixture.Member().Fetch( fixture.serverPeer, "files", "huge.bin", abortedSinkPtr, ShortOptions( 4000 ) );
        REQUIRE_TRUE( GUCEF_NULL != aborted );
        for ( UInt32 waited=0; waited<4000 && aborted->GetBytesReceived() == 0 && !aborted->IsFinished(); ++waited )
            MT::ThreadDelay( 1 );
        ASSERT_TRUE( !aborted->IsFinished() );
        aborted->Cancel( true );
        ASSERT_TRUE( aborted->WaitForCompletion( 8000 ) );
        ASSERT_TRUE( DRN::DRN_RESSTATUS_CANCELLED == aborted->GetResult().status );
        fixture.Member().Release( aborted );

        // everything that was queued has ended: its pulse request came
        for ( UInt32 stable=0, waited=0; stable<20 && waited<5000; ++waited )
        {
            MT::ThreadDelay( 5 );
            stable = ( 0 == pool->GetRunningTaskCount() && 0 == pool->GetNrOfQueuedTasks() ) ? stable + 1 : 0;
        }
        CORE::CThreadPoolCounters after;
        pool->GetCounters( after );
        const UInt64 queued = after.GetQueuedTotal() - before.GetQueuedTotal();
        const UInt64 finished = after.GetFinishedTotal() - before.GetFinishedTotal();
        const UInt32 serverWakes = fixture.driver.wakes.Load();
        const UInt32 clientWakes = fixture.clientDriver.wakes.Load();
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "NodeWake: facility jobs queued " << queued << ", ended " << finished << "; pulse requests of the serving node " << serverWakes << ", of the requesting node " << clientWakes
                   << " (before the calls " << wakesBefore << ")" );
        ASSERT_TRUE( queued >= 10 );
        ASSERT_TRUE( queued == finished );
        ASSERT_TRUE( clientWakes > 0 && serverWakes > 0 );
        ASSERT_TRUE( (UInt64) ( serverWakes + clientWakes - wakesBefore ) == finished );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // The thread that pumps a node without a thread of its own yields about a millisecond after every cycle (CDRNNode::PumpOnce()); a
    // wake of another thread ends the yield, work of the node itself does not
    GUCEF_TESTFW_TESTCASE( "Node: the yield of the pump thread after its cycle lasts about a millisecond, a wake-up (WakeOwnThread() or a pulse request through the driver of the node) ends it at once, a request made inside the node's own work does not" )
    try
    {
        DRN::CDRNNode node;
        DRN::CDRNNodeSettings settings;
        settings.pulseIntervalMs = 1;
        settings.ownThread = false;
        REQUIRE_TRUE( node.SetSettings( settings ) );
        CPumpThreadScope pumpScope;
        REQUIRE_TRUE( node.IsPumpThread() );
        CORE::PulseGeneratorPtr workPulseGenerator = node.GetWorkPulseGenerator();
        REQUIRE_TRUE( !workPulseGenerator.IsNULL() );
        const UInt32 rounds = 100;
        node.PumpOnce();

        UInt64 startTicks = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<rounds; ++i )
            node.PumpOnce();
        const Float64 plainMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks );

        startTicks = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<rounds; ++i )
        {
            node.WakeOwnThread();
            node.PumpOnce();
        }
        const Float64 wokenMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks );

        startTicks = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<rounds; ++i )
        {
            workPulseGenerator->RequestImmediatePulseThroughDriver();
            node.PumpOnce();
        }
        const Float64 requestedMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks );

        // a handler of the node's own pulse that asks for one more pulse: part of the work, the yield stays
        CCycleRequester requester( workPulseGenerator );
        node.PumpOnce();
        startTicks = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<rounds; ++i )
            node.PumpOnce();
        const Float64 insideMs = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - startTicks );
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "NodeWake: " << rounds << " cycles of a pump thread took " << plainMs << " ms, with a wake before each " << wokenMs << " ms, with a pulse request through the driver before each "
                   << requestedMs << " ms, with a request made inside the node's work " << insideMs << " ms" );
        ASSERT_TRUE( plainMs >= (Float64) rounds * 0.9 );
        ASSERT_TRUE( wokenMs < plainMs / 3.0 );
        ASSERT_TRUE( requestedMs < plainMs / 3.0 );
        ASSERT_TRUE( insideMs >= (Float64) rounds * 0.9 );
        ASSERT_TRUE( requester.requests.Load() >= rounds );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    RemoveTestDirectories();
}

/*-------------------------------------------------------------------------*/
