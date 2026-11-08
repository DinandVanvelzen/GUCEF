/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/file.h>
#endif

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_CHILD_PROCESS_H
#include "gucef_test_child_process.h"
#define GUCEF_TEST_CHILD_PROCESS_H
#endif /* GUCEF_TEST_CHILD_PROCESS_H ? */

#include "TestPump.h"
#include "TestFiles.h"
#include "TestSigner.h"
#include "TestLinkStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the failure and abandons the test case, for what the rest of it needs */
#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static AUTH::CLinkRecord
MakeRecord( CTestSigner& signer, const CORE::CString& account, UInt64 linkedAt = 1700000000000ULL )
{GUCEF_TRACE;

    AUTH::CAccountRef ref;
    AUTH::CAccountRef::TryParse( account, ref );
    AUTH::CLinkRecord record;
    AUTH::CLinkRecord::Create( signer, ref, AUTH::CAuthCrypto::Sha256Hex( account.C_String(), account.Length() ), linkedAt, record );
    return record;
}

/*-------------------------------------------------------------------------*/

static bool
WriteJsonFile( const CORE::CString& path, const CORE::CDataNode& root )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) )
        return false;
    CORE::CDynamicBuffer buffer;
    CORE::CDynamicBufferAccess access( buffer );
    if ( !codec->StoreDataTree( &root, &access ) )
        return false;
    CORE::CreateDirs( CORE::StripFilename( path ) );
    return CORE::WriteFileAtomically( path, buffer );
}

/*-------------------------------------------------------------------------*/

static bool
ReadJsonFile( const CORE::CString& path, CORE::CDataNode& root )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) )
        return false;
    return codec->BuildDataTree( &root, path );
}

/*-------------------------------------------------------------------------*/

/** The node that holds the "links" of a document the json codec read */
static const CORE::CDataNode*
FindDocument( const CORE::CDataNode& root )
{GUCEF_TRACE;

    if ( GUCEF_NULL != root.FindChild( "links" ) )
        return &root;
    CORE::CDataNode::const_iterator i = root.ConstBegin();
    while ( i != root.ConstEnd() )
    {
        if ( GUCEF_NULL != (*i)->FindChild( "links" ) )
            return *i;
        ++i;
    }
    return &root;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ReadTextFile( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CDynamicBuffer content;
    if ( !content.LoadContentFromFile( path ) || 0 == content.GetDataSize() )
        return CORE::CString::Empty;
    return CORE::CString( content.AsConstTypePtr< char >(), content.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
WriterAccount( const CORE::CString& prefix, UInt32 index )
{GUCEF_TRACE;

    return "google:" + prefix + "-" + CORE::ToString( index );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ScratchAccount( const CORE::CString& prefix, UInt32 index )
{GUCEF_TRACE;

    return "epic:" + prefix + "-" + CORE::ToString( index );
}

/*-------------------------------------------------------------------------*/

/**
 *  What one writer of the contention tests does with one instance of the store, kept for all its changes: links the
 *  accounts google:<prefix>-<i> to a profile of its own, every third one with an account epic:<prefix>-<i> linked and
 *  unlinked again, and resolves its own accounts at the end. Returns the number of changes and reads that did not work
 */
static UInt32
LinkAccounts( const CORE::CString& path, const CORE::CString& prefix, UInt32 count )
{GUCEF_TRACE;

    CTestSigner signer( prefix );
    if ( !signer.IsValid() )
        return count + 1;
    UInt32 failures = 0;
    AUTH::CLocalIdentityLinkStore store( path );
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( AUTH::LINKRESULT_OK != store.Link( MakeRecord( signer, WriterAccount( prefix, i ), 1000 + i ) ) )
            ++failures;
        if ( 0 == i % 3 )
        {
            AUTH::CLinkRecord scratch = MakeRecord( signer, ScratchAccount( prefix, i ), 1000 + i );
            if ( AUTH::LINKRESULT_OK != store.Link( scratch ) )
                ++failures;
            if ( !store.Unlink( scratch.accountRef, signer.GetKeyId() ) )
                ++failures;
        }
    }
    for ( UInt32 i=0; i<count; ++i )
    {
        AUTH::CAccountRef account;
        CORE::CString profile;
        if ( !AUTH::CAccountRef::TryParse( WriterAccount( prefix, i ), account ) || !store.Resolve( account, profile ) || signer.GetKeyId() != profile )
            ++failures;
    }
    return failures;
}

/*-------------------------------------------------------------------------*/

/** Counts the accounts of a writer that a new instance finds linked; scratchCount receives those of its scratch accounts */
static UInt32
CountLinkedAccounts( const CORE::CString& path, const CORE::CString& prefix, UInt32 count, UInt32& scratchCount )
{GUCEF_TRACE;

    AUTH::CLocalIdentityLinkStore store( path );
    UInt32 found = 0;
    scratchCount = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        AUTH::CAccountRef account;
        CORE::CString profile;
        if ( AUTH::CAccountRef::TryParse( WriterAccount( prefix, i ), account ) && store.Resolve( account, profile ) )
            ++found;
        if ( AUTH::CAccountRef::TryParse( ScratchAccount( prefix, i ), account ) && store.Resolve( account, profile ) )
            ++scratchCount;
    }
    return found;
}

/*-------------------------------------------------------------------------*/

/** A thread with an instance of the store of its own that runs LinkAccounts() */
class CLinkWriterThread : public MT::CActiveObject
{
    public:

    CLinkWriterThread( const CORE::CString& path, const CORE::CString& prefix, UInt32 count )
        : MT::CActiveObject(), m_path( path ), m_prefix( prefix ), m_count( count ), m_lock(), m_done( false ), m_failures( 0 )
    {GUCEF_TRACE;
    }

    bool IsDone( void ) const { MT::CScopeMutex lock( m_lock ); return m_done; }
    UInt32 GetFailures( void ) const { MT::CScopeMutex lock( m_lock ); return m_failures; }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        UInt32 failures = LinkAccounts( m_path, m_prefix, m_count );
        MT::CScopeMutex lock( m_lock );
        m_failures = failures;
        m_done = true;
        return true;
    }

    private:

    CORE::CString m_path;
    CORE::CString m_prefix;
    UInt32 m_count;
    mutable MT::CMutex m_lock;
    bool m_done;
    UInt32 m_failures;
};

/*-------------------------------------------------------------------------*/

bool
IsLinkStoreHelperRun( void )
{GUCEF_TRACE;

    return "linkstore-writer" == GetEnvText( "GUCEF_AUTH_TEST_HELPER" );
}

/*-------------------------------------------------------------------------*/

int
RunLinkStoreHelper( void )
{GUCEF_TRACE;

    CORE::CString path = GetEnvText( "GUCEF_AUTH_TEST_STORE_PATH" );
    CORE::CString syncDirectory = GetEnvText( "GUCEF_AUTH_TEST_SYNC_DIR" );
    CORE::CString prefix = GetEnvText( "GUCEF_AUTH_TEST_PREFIX" );
    UInt32 count = CORE::StringToUInt32( GetEnvText( "GUCEF_AUTH_TEST_COUNT" ) );
    if ( path.IsNULLOrEmpty() || syncDirectory.IsNULLOrEmpty() || prefix.IsNULLOrEmpty() || 0 == count )
        return 3;
    if ( !JsonCodecAvailable() || !AUTH::CAuthCrypto::IsSignatureAvailable() )
        return 4;

    // both processes start writing at the same moment: this one says it is ready and waits for the word of the test
    CORE::CString ready = "ready";
    if ( !CORE::WriteFileAtomically( CORE::CombinePath( syncDirectory, "ready-" + prefix ), ready.C_String(), ready.Length() ) )
        return 5;
    if ( !WaitForFile( CORE::CombinePath( syncDirectory, "go" ), 60000 ) )
        return 6;
    return 0 == LinkAccounts( path, prefix, count ) ? 0 : 2;
}

/*-------------------------------------------------------------------------*/

void
PerformLinkStoreTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING identity link store TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "LinkStore" );

    GUCEF_TESTFW_TESTCASE( "Test: A signed link record verifies and any change to it does not" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        try
        {
            CTestSigner signer( "alice" );
            CTestSigner other( "mallory" );
            ASSERT_TRUE( signer.IsValid() && other.IsValid() );
            AUTH::CLinkRecord record = MakeRecord( signer, "google:112233" );
            ASSERT_TRUE( 1 == record.version && "google:112233" == record.accountRef.ToString() );
            ASSERT_TRUE( signer.GetKeyId() == record.profileKeyId && signer.GetPublicKeyHex() == record.publicKey );
            ASSERT_TRUE( 128 == record.signature.Length() && 64 == record.proofDigest.Length() );
            ASSERT_TRUE( record.IsKnownVersion() && record.IsWellFormed() && record.VerifySignature() );

            // every signed field is protected
            { AUTH::CLinkRecord r( record ); r.accountRef = AUTH::CAccountRef( "google", "112234" ); ASSERT_FALSE( r.VerifySignature() ); }
            { AUTH::CLinkRecord r( record ); r.accountRef = AUTH::CAccountRef( "epic", "112233" ); ASSERT_FALSE( r.VerifySignature() ); }
            { AUTH::CLinkRecord r( record ); r.profileKeyId = "bob-0000"; ASSERT_FALSE( r.VerifySignature() ); }
            { AUTH::CLinkRecord r( record ); r.linkedAtMs += 1; ASSERT_FALSE( r.VerifySignature() ); }
            { AUTH::CLinkRecord r( record ); r.proofDigest = AUTH::CAuthCrypto::Sha256Hex( "x", 1 ); ASSERT_FALSE( r.VerifySignature() ); }
            { AUTH::CLinkRecord r( record ); r.proofDigest.Clear(); ASSERT_FALSE( r.VerifySignature() ); }
            { AUTH::CLinkRecord r( record ); r.mergedFrom = "someone"; ASSERT_FALSE( r.VerifySignature() ); }
            { AUTH::CLinkRecord r( record ); r.publicKey = other.GetPublicKeyHex(); ASSERT_FALSE( r.VerifySignature() ); }
            { AUTH::CLinkRecord r( record ); r.signature = record.signature.SubstrToIndex( 126, true ) + ( record.signature.EndsWith( "00" ) ? "01" : "00" ); ASSERT_FALSE( r.VerifySignature() ); }
            { AUTH::CLinkRecord r( record ); r.signature.Clear(); ASSERT_FALSE( r.VerifySignature() ); }
            // a record signed by another key but claiming the profile of the first
            { AUTH::CLinkRecord r( MakeRecord( other, "google:112233" ) ); ASSERT_TRUE( r.VerifySignature() && r.profileKeyId != record.profileKeyId ); }

            // the form of the fields is checked: upper case hex, spaces and new lines have no place
            { AUTH::CLinkRecord r( record ); r.publicKey = record.publicKey.Uppercase(); ASSERT_FALSE( r.IsWellFormed() || r.VerifySignature() ); }
            { AUTH::CLinkRecord r( record ); r.profileKeyId = "ali ce"; ASSERT_FALSE( r.IsWellFormed() ); }
            { AUTH::CLinkRecord r( record ); r.profileKeyId = "alice\nbob"; ASSERT_FALSE( r.IsWellFormed() ); }
            { AUTH::CLinkRecord r( record ); r.accountRef = AUTH::CAccountRef(); ASSERT_FALSE( r.IsWellFormed() ); }

            // a version this code does not know is not verified, whatever it holds
            { AUTH::CLinkRecord r( record ); r.version = 2; ASSERT_FALSE( r.IsKnownVersion() || r.VerifySignature() ); }
            { AUTH::CLinkRecord r( record ); r.version = 0; ASSERT_FALSE( r.IsKnownVersion() ); }

            // the signature is deterministic over the fields: the same inputs give the same signed bytes
            CORE::CDynamicBuffer a;
            CORE::CDynamicBuffer b;
            ASSERT_TRUE( record.GetSignedBytes( a ) && AUTH::CLinkRecord( record ).GetSignedBytes( b ) );
            ASSERT_TRUE( a.GetDataSize() == b.GetDataSize() && 0 == memcmp( a.GetConstBufferPtr(), b.GetConstBufferPtr(), a.GetDataSize() ) );

            // through a data tree
            CORE::CDataNode node( "link", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( record.Serialize( node, CORE::CDataNodeSerializableSettings() ) );
            AUTH::CLinkRecord back;
            ASSERT_TRUE( back.Deserialize( node, CORE::CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( back.VerifySignature() && back.accountRef == record.accountRef && back.linkedAtMs == record.linkedAtMs );
            ASSERT_FALSE( AUTH::CLinkRecord().Deserialize( CORE::CDataNode(), CORE::CDataNodeSerializableSettings() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Creating a record fails without a usable signer or account" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        try
        {
            CTestSigner signer;
            AUTH::CLinkRecord record;
            ASSERT_FALSE( AUTH::CLinkRecord::Create( signer, AUTH::CAccountRef(), CORE::CString::Empty, 1, record ) );
            ASSERT_FALSE( AUTH::CLinkRecord::Create( signer, AUTH::CAccountRef( "epic", "1" ), "not-a-digest", 1, record ) );
            ASSERT_TRUE( AUTH::CLinkRecord::Create( signer, AUTH::CAccountRef( "epic", "1" ), CORE::CString::Empty, 1, record ) && record.VerifySignature() );
            // a record without a proof digest is fine, it says the login had no proof
            ASSERT_TRUE( record.proofDigest.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Linking, resolving, replacing, conflicts and unlinking in a store" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        try
        {
            CTestSigner alice( "alice" );
            CTestSigner bob( "bob" );
            AUTH::CLocalIdentityLinkStore store;
            ASSERT_TRUE( store.GetPath().IsNULLOrEmpty() );

            AUTH::CLinkRecord google = MakeRecord( alice, "google:g-1", 1000 );
            AUTH::CLinkRecord epic = MakeRecord( alice, "epic:765", 2000 );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( google ) );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( epic ) );

            AUTH::TLinkRecordVector links;
            REQUIRE_TRUE( store.GetLinks( alice.GetKeyId(), links ) && 2 == links.size() );
            ASSERT_TRUE( "google:g-1" == links[ 0 ].accountRef.ToString() && "epic:765" == links[ 1 ].accountRef.ToString() );
            ASSERT_TRUE( store.GetLinks( bob.GetKeyId(), links ) && links.empty() );

            CORE::CString profile;
            ASSERT_TRUE( store.Resolve( AUTH::CAccountRef( "epic", "765" ), profile ) && alice.GetKeyId() == profile );
            ASSERT_FALSE( store.Resolve( AUTH::CAccountRef( "epic", "766" ), profile ) );

            // the same profile links the account again with a new proof: replaced, not doubled
            AUTH::CLinkRecord googleAgain = MakeRecord( alice, "google:g-1", 3000 );
            googleAgain.proofDigest = AUTH::CAuthCrypto::Sha256Hex( "new proof", 9 );
            ASSERT_TRUE( googleAgain.Sign( alice ) );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( googleAgain ) );
            ASSERT_TRUE( store.GetLinks( alice.GetKeyId(), links ) && 2 == links.size() && 3000 == links[ 0 ].linkedAtMs && links[ 0 ].proofDigest == googleAgain.proofDigest );

            // another profile cannot take the account, nothing changes
            AUTH::CLinkRecord stolen = MakeRecord( bob, "epic:765" );
            ASSERT_TRUE( AUTH::LINKRESULT_CONFLICT == store.Link( stolen ) );
            ASSERT_TRUE( store.Resolve( AUTH::CAccountRef( "epic", "765" ), profile ) && alice.GetKeyId() == profile );
            ASSERT_TRUE( store.GetLinks( bob.GetKeyId(), links ) && links.empty() );

            // records that are not signed, not well formed, changed or of an unknown version are refused
            AUTH::CLinkRecord unsigned_ = MakeRecord( bob, "epic:abcdef" );
            unsigned_.signature.Clear();
            ASSERT_TRUE( AUTH::LINKRESULT_INVALID_RECORD == store.Link( unsigned_ ) );
            AUTH::CLinkRecord changed = MakeRecord( bob, "epic:abcdef" );
            changed.accountRef = AUTH::CAccountRef( "epic", "fedcba" );
            ASSERT_TRUE( AUTH::LINKRESULT_INVALID_RECORD == store.Link( changed ) );
            AUTH::CLinkRecord future = MakeRecord( bob, "epic:abcdef" );
            future.version = 9;
            ASSERT_TRUE( AUTH::LINKRESULT_INVALID_RECORD == store.Link( future ) );
            ASSERT_TRUE( AUTH::LINKRESULT_INVALID_RECORD == store.Link( AUTH::CLinkRecord() ) );
            ASSERT_FALSE( store.Resolve( AUTH::CAccountRef( "epic", "abcdef" ), profile ) );

            // unlinking is for the profile that owns the link
            ASSERT_TRUE( store.Unlink( AUTH::CAccountRef( "epic", "765" ), bob.GetKeyId() ) );
            ASSERT_TRUE( store.Resolve( AUTH::CAccountRef( "epic", "765" ), profile ) );
            ASSERT_TRUE( store.Unlink( AUTH::CAccountRef( "epic", "765" ), alice.GetKeyId() ) );
            ASSERT_FALSE( store.Resolve( AUTH::CAccountRef( "epic", "765" ), profile ) );
            ASSERT_TRUE( store.Unlink( AUTH::CAccountRef( "epic", "765" ), alice.GetKeyId() ) );   // already gone
            ASSERT_TRUE( store.GetLinks( alice.GetKeyId(), links ) && 1 == links.size() );

            // after the unlink another profile can have the account
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( stolen ) );
            ASSERT_TRUE( store.Resolve( AUTH::CAccountRef( "epic", "765" ), profile ) && bob.GetKeyId() == profile );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Merging the accounts of one profile into another" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        try
        {
            CTestSigner phone( "phone" );
            CTestSigner pc( "pc" );
            AUTH::CLocalIdentityLinkStore store;
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( phone, "google:g-1" ) ) );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( phone, "epic:765" ) ) );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( pc, "epic:e-1" ) ) );

            UInt32 merged = 0;
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Merge( phone.GetKeyId(), pc, &merged ) );
            ASSERT_TRUE( 2 == merged );
            AUTH::TLinkRecordVector links;
            ASSERT_TRUE( store.GetLinks( phone.GetKeyId(), links ) && links.empty() );
            REQUIRE_TRUE( store.GetLinks( pc.GetKeyId(), links ) && 3 == links.size() );
            UInt32 movedCount = 0;
            for ( size_t i=0; i<links.size(); ++i )
            {
                ASSERT_TRUE( links[ i ].VerifySignature() && pc.GetKeyId() == links[ i ].profileKeyId && pc.GetPublicKeyHex() == links[ i ].publicKey );
                if ( phone.GetKeyId() == links[ i ].mergedFrom )
                    ++movedCount;
            }
            ASSERT_TRUE( 2 == movedCount );
            CORE::CString profile;
            ASSERT_TRUE( store.Resolve( AUTH::CAccountRef( "google", "g-1" ), profile ) && pc.GetKeyId() == profile );

            // nothing left to merge, and a profile cannot merge into itself
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Merge( phone.GetKeyId(), pc, &merged ) && 0 == merged );
            ASSERT_TRUE( AUTH::LINKRESULT_INVALID_RECORD == store.Merge( pc.GetKeyId(), pc, &merged ) );
            ASSERT_TRUE( AUTH::LINKRESULT_INVALID_RECORD == store.Merge( CORE::CString::Empty, pc, &merged ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The links in a file survive a restart and the file is written atomically" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "links_restart" );
            CORE::CString path = CORE::CombinePath( dir, "links.json" );
            CTestSigner alice( "alice" );
            {
                AUTH::CLocalIdentityLinkStore store( path );
                ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( alice, "google:g-1", 1000 ) ) );
                ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( alice, "epic:765", 2000 ) ) );
                ASSERT_TRUE( CORE::FileExists( path ) );
            }
            // the file and its lock file, no leftovers of the atomic writes
            CORE::CString::StringVector names = ListDirectory( dir );
            ASSERT_TRUE( 2 == names.size() && HasName( names, "links.json" ) && HasName( names, "links.json.lock" ) );

            AUTH::CLocalIdentityLinkStore again( path );
            AUTH::TLinkRecordVector links;
            REQUIRE_TRUE( again.GetLinks( alice.GetKeyId(), links ) && 2 == links.size() );
            ASSERT_TRUE( "google:g-1" == links[ 0 ].accountRef.ToString() && 1000 == links[ 0 ].linkedAtMs && links[ 0 ].VerifySignature() );
            ASSERT_TRUE( "epic:765" == links[ 1 ].accountRef.ToString() && 2000 == links[ 1 ].linkedAtMs );
            CORE::CString profile;
            ASSERT_TRUE( again.Resolve( AUTH::CAccountRef( "epic", "765" ), profile ) && alice.GetKeyId() == profile );
            ASSERT_TRUE( 0 == again.GetRefusedCount() && 0 == again.GetForeignCount() );

            // an unlink is kept too
            ASSERT_TRUE( again.Unlink( AUTH::CAccountRef( "google", "g-1" ), alice.GetKeyId() ) );
            AUTH::CLocalIdentityLinkStore third( path );
            ASSERT_TRUE( third.GetLinks( alice.GetKeyId(), links ) && 1 == links.size() && "epic:765" == links[ 0 ].accountRef.ToString() );

            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            struct stat info;
            ASSERT_TRUE( 0 == stat( path.C_String(), &info ) && 0600 == ( info.st_mode & 0777 ) );
            #endif

            // the path and the config
            CORE::CDataNode config( "Links", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( third.SaveConfig( config ) );
            AUTH::CLocalIdentityLinkStore configured;
            ASSERT_TRUE( configured.LoadConfig( config ) && configured.GetPath() == third.GetPath() );
            ASSERT_TRUE( configured.GetLinks( alice.GetKeyId(), links ) && 1 == links.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A record changed in the file is refused when the file is read" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "links_tamper" );
            CORE::CString path = CORE::CombinePath( dir, "links.json" );
            CTestSigner alice( "alice" );
            {
                AUTH::CLocalIdentityLinkStore store( path );
                ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( alice, "google:ALICE-SUBJECT-1" ) ) );
                ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( alice, "epic:765" ) ) );
            }

            // the account of the first link is changed to one of somebody else, the signature is left as it was
            CORE::CString text = ReadTextFile( path );
            ASSERT_TRUE( text.HasSubstr( "ALICE-SUBJECT-1" ) >= 0 );
            CORE::CString edited = text.ReplaceSubstr( "ALICE-SUBJECT-1", "ALICE-SUBJECT-2" );
            CORE::WriteFileAtomically( path, edited.C_String(), edited.Length() );

            AUTH::CLocalIdentityLinkStore tampered( path );
            AUTH::TLinkRecordVector links;
            ASSERT_TRUE( tampered.GetLinks( alice.GetKeyId(), links ) );
            ASSERT_TRUE( 1 == links.size() && "epic:765" == links[ 0 ].accountRef.ToString() );
            ASSERT_TRUE( 1 == tampered.GetRefusedCount() );
            CORE::CString profile;
            ASSERT_FALSE( tampered.Resolve( AUTH::CAccountRef( "google", "ALICE-SUBJECT-2" ), profile ) );
            ASSERT_FALSE( tampered.Resolve( AUTH::CAccountRef( "google", "ALICE-SUBJECT-1" ), profile ) );

            // the refused record is dropped when the file is written again
            CTestSigner bob( "bob" );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == tampered.Link( MakeRecord( bob, "epic:e-1" ) ) );
            CORE::CString after = ReadTextFile( path );
            ASSERT_TRUE( after.HasSubstr( "ALICE-SUBJECT" ) < 0 && after.HasSubstr( "e-1" ) >= 0 );

            // a record whose profile key is changed to another key fails as well
            CORE::CString path2 = CORE::CombinePath( dir, "links2.json" );
            {
                AUTH::CLocalIdentityLinkStore store( path2 );
                ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( alice, "google:g-2" ) ) );
            }
            CORE::CString text2 = ReadTextFile( path2 ).ReplaceSubstr( alice.GetKeyId(), bob.GetKeyId() );
            CORE::WriteFileAtomically( path2, text2.C_String(), text2.Length() );
            AUTH::CLocalIdentityLinkStore swapped( path2 );
            ASSERT_TRUE( swapped.GetLinks( bob.GetKeyId(), links ) && links.empty() && 1 == swapped.GetRefusedCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Records of an unknown future version are tolerated and kept" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "links_future" );
            CORE::CString path = CORE::CombinePath( dir, "links.json" );
            CTestSigner alice( "alice" );

            AUTH::CLinkRecord known = MakeRecord( alice, "google:g-1" );
            CORE::CDataNode root( "AuthLinks", GUCEF_DATATYPE_OBJECT );
            root.SetAttribute( "version", (UInt32) 1 );
            CORE::CDataNode* list = root.AddChild( "links", GUCEF_DATATYPE_ARRAY );
            known.Serialize( *list->AddChild( "link", GUCEF_DATATYPE_OBJECT ), CORE::CDataNodeSerializableSettings() );
            CORE::CDataNode* future = list->AddChild( "link", GUCEF_DATATYPE_OBJECT );
            future->SetAttribute( "version", (UInt32) 7 );
            future->SetAttribute( "accountRef", CORE::CString( "epic:from-the-future" ) );
            future->SetAttribute( "profileKeyId", alice.GetKeyId() );
            future->SetAttribute( "attestation", CORE::CString( "a-field-that-does-not-exist-yet" ) );
            ASSERT_TRUE( WriteJsonFile( path, root ) );

            AUTH::CLocalIdentityLinkStore store( path );
            AUTH::TLinkRecordVector links;
            ASSERT_TRUE( store.GetLinks( alice.GetKeyId(), links ) && 1 == links.size() && "google:g-1" == links[ 0 ].accountRef.ToString() );
            ASSERT_TRUE( 1 == store.GetForeignCount() && 0 == store.GetRefusedCount() );
            CORE::CString profile;
            ASSERT_FALSE( store.Resolve( AUTH::CAccountRef( "epic", "from-the-future" ), profile ) );

            // a change by this version writes the record of the future back as it was
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( alice, "epic:e-1" ) ) );
            CORE::CDataNode reread;
            ASSERT_TRUE( ReadJsonFile( path, reread ) );
            const CORE::CDataNode* document = FindDocument( reread );
            const CORE::CDataNode* rereadList = document->FindChild( "links" );
            ASSERT_TRUE( GUCEF_NULL != rereadList && 3 == rereadList->GetNrOfChildNodes() );
            bool foundFuture = false;
            if ( GUCEF_NULL != rereadList )
            {
                CORE::CDataNode::const_iterator i = rereadList->ConstBegin();
                while ( i != rereadList->ConstEnd() )
                {
                    if ( "epic:from-the-future" == (*i)->GetAttributeValueOrChildValueByName( "accountRef" ).AsString() )
                    {
                        foundFuture = 7 == (*i)->GetAttributeValueOrChildValueByName( "version" ).AsUInt32() &&
                                      "a-field-that-does-not-exist-yet" == (*i)->GetAttributeValueOrChildValueByName( "attestation" ).AsString();
                    }
                    ++i;
                }
            }
            ASSERT_TRUE( foundFuture );

            AUTH::CLocalIdentityLinkStore after( path );
            ASSERT_TRUE( after.GetLinks( alice.GetKeyId(), links ) && 2 == links.size() && 1 == after.GetForeignCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A file of a newer format version is read but not changed" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "links_newer" );
            CORE::CString path = CORE::CombinePath( dir, "links.json" );
            CTestSigner alice( "alice" );
            CORE::CDataNode root( "AuthLinks", GUCEF_DATATYPE_OBJECT );
            root.SetAttribute( "version", (UInt32) 5 );
            root.SetAttribute( "somethingNew", CORE::CString( "yes" ) );
            CORE::CDataNode* list = root.AddChild( "links", GUCEF_DATATYPE_ARRAY );
            MakeRecord( alice, "google:g-1" ).Serialize( *list->AddChild( "link", GUCEF_DATATYPE_OBJECT ), CORE::CDataNodeSerializableSettings() );
            ASSERT_TRUE( WriteJsonFile( path, root ) );
            CORE::CString before = ReadTextFile( path );

            AUTH::CLocalIdentityLinkStore store( path );
            AUTH::TLinkRecordVector links;
            ASSERT_TRUE( store.GetLinks( alice.GetKeyId(), links ) && 1 == links.size() );
            ASSERT_TRUE( AUTH::LINKRESULT_STORE_ERROR == store.Link( MakeRecord( alice, "epic:765" ) ) );
            ASSERT_FALSE( store.Unlink( AUTH::CAccountRef( "google", "g-1" ), alice.GetKeyId() ) );
            ASSERT_TRUE( store.GetLinks( alice.GetKeyId(), links ) && 1 == links.size() );
            ASSERT_TRUE( before == ReadTextFile( path ) );

            // the same goes for a file that cannot be read at all
            CORE::CString brokenPath = CORE::CombinePath( dir, "broken.json" );
            CORE::CString garbage = "{ this is not json";
            CORE::WriteFileAtomically( brokenPath, garbage.C_String(), garbage.Length() );
            AUTH::CLocalIdentityLinkStore broken( brokenPath );
            ASSERT_TRUE( AUTH::LINKRESULT_STORE_ERROR == broken.Link( MakeRecord( alice, "epic:765" ) ) );
            ASSERT_TRUE( garbage == ReadTextFile( brokenPath ) );

            // the state follows the file: a file of this version in its place is changed again
            CORE::CDataNode current( "AuthLinks", GUCEF_DATATYPE_OBJECT );
            current.SetAttribute( "version", (UInt32) 1 );
            CORE::CDataNode* currentList = current.AddChild( "links", GUCEF_DATATYPE_ARRAY );
            MakeRecord( alice, "google:g-1" ).Serialize( *currentList->AddChild( "link", GUCEF_DATATYPE_OBJECT ), CORE::CDataNodeSerializableSettings() );
            ASSERT_TRUE( WriteJsonFile( path, current ) );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( alice, "epic:765" ) ) );
            ASSERT_TRUE( store.GetLinks( alice.GetKeyId(), links ) && 2 == links.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Records of the own profile must carry the key of the signer" )
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        try
        {
            CTestSigner alice( "alice" );
            CTestSigner impostor( "impostor" );
            AUTH::CLocalIdentityLinkStore store;
            store.SetSigner( &alice );

            // the impostor makes a record that claims the profile id of alice with a key of its own
            AUTH::CLinkRecord forged = MakeRecord( impostor, "google:g-1" );
            forged.profileKeyId = alice.GetKeyId();
            ASSERT_TRUE( forged.Sign( impostor ) );
            forged.profileKeyId = alice.GetKeyId();
            ASSERT_FALSE( forged.VerifySignature() );
            // a record that verifies with the impostor's own key but names the profile of alice
            AUTH::CLinkRecord claimed;
            claimed.accountRef = AUTH::CAccountRef( "google", "g-1" );
            claimed.linkedAtMs = 5;
            claimed.profileKeyId = alice.GetKeyId();
            claimed.publicKey = impostor.GetPublicKeyHex();
            CORE::CDynamicBuffer bytes;
            ASSERT_TRUE( claimed.GetSignedBytes( bytes ) );
            CORE::CDynamicBuffer signature;
            ASSERT_TRUE( impostor.Sign( bytes.GetConstBufferPtr(), bytes.GetDataSize(), signature ) );
            claimed.signature = AUTH::CAuthCrypto::ToHex( signature.GetConstBufferPtr(), signature.GetDataSize() );
            ASSERT_TRUE( claimed.VerifySignature() );
            ASSERT_TRUE( AUTH::LINKRESULT_INVALID_RECORD == store.Link( claimed ) );

            // the real thing is accepted
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( alice, "google:g-1" ) ) );
            // and other profiles are not bound to this signer
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( impostor, "epic:765" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A store whose file is not there reads as empty and makes nothing for a read or a removal" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "links_absent" );
            CORE::CString path = CORE::CombinePath( dir, "links.json" );
            CTestSigner alice( "alice" );
            AUTH::CLocalIdentityLinkStore store( path );
            AUTH::TLinkRecordVector links;
            CORE::CString profile;
            ASSERT_TRUE( store.GetLinks( alice.GetKeyId(), links ) && links.empty() );
            ASSERT_FALSE( store.Resolve( AUTH::CAccountRef( "google", "g-1" ), profile ) );
            ASSERT_TRUE( store.Unlink( AUTH::CAccountRef( "google", "g-1" ), alice.GetKeyId() ) );
            CTestSigner bob( "bob" );
            UInt32 merged = 1;
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Merge( alice.GetKeyId(), bob, &merged ) && 0 == merged );
            ASSERT_TRUE( 0 == store.GetRefusedCount() && 0 == store.GetForeignCount() );
            ASSERT_FALSE( CORE::DirExists( dir ) );

            // a link makes the directory, the file and its lock file; a file deleted by someone else reads as empty
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( alice, "google:g-1" ) ) );
            ASSERT_TRUE( CORE::FileExists( path ) && CORE::FileExists( store.GetLockPath() ) );
            ASSERT_TRUE( CORE::DeleteFile( path ) );
            ASSERT_FALSE( store.Resolve( AUTH::CAccountRef( "google", "g-1" ), profile ) );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( alice, "epic:765" ) ) );
            ASSERT_TRUE( store.GetLinks( alice.GetKeyId(), links ) && 1 == links.size() && "epic:765" == links[ 0 ].accountRef.ToString() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Two instances on one file see each other's links and unlinks and never undo them" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "links_two_instances" );
            CORE::CString path = CORE::CombinePath( dir, "links.json" );
            CTestSigner alice( "alice" );
            CTestSigner bob( "bob" );
            AUTH::CLocalIdentityLinkStore first( path );
            AUTH::CLocalIdentityLinkStore second( path );
            AUTH::TLinkRecordVector links;
            CORE::CString profile;

            // both have read the file before the other changes it
            ASSERT_TRUE( AUTH::LINKRESULT_OK == first.Link( MakeRecord( alice, "google:g-1", 1000 ) ) );
            ASSERT_TRUE( second.GetLinks( alice.GetKeyId(), links ) && 1 == links.size() );

            // a link by one is seen by the other, and a link by the other keeps it
            ASSERT_TRUE( AUTH::LINKRESULT_OK == first.Link( MakeRecord( alice, "epic:765", 2000 ) ) );
            ASSERT_TRUE( second.Resolve( AUTH::CAccountRef( "epic", "765" ), profile ) && alice.GetKeyId() == profile );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == second.Link( MakeRecord( bob, "google:b-1", 3000 ) ) );
            {
                AUTH::CLocalIdentityLinkStore check( path );
                ASSERT_TRUE( check.GetLinks( alice.GetKeyId(), links ) && 2 == links.size() );
                ASSERT_TRUE( check.GetLinks( bob.GetKeyId(), links ) && 1 == links.size() );
            }
            ASSERT_TRUE( first.Resolve( AUTH::CAccountRef( "google", "b-1" ), profile ) && bob.GetKeyId() == profile );

            // an unlink by one is not undone by a change of the other
            ASSERT_TRUE( first.Unlink( AUTH::CAccountRef( "google", "g-1" ), alice.GetKeyId() ) );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == second.Link( MakeRecord( bob, "google:b-2", 4000 ) ) );
            {
                AUTH::CLocalIdentityLinkStore check( path );
                ASSERT_FALSE( check.Resolve( AUTH::CAccountRef( "google", "g-1" ), profile ) );
                ASSERT_TRUE( check.Resolve( AUTH::CAccountRef( "epic", "765" ), profile ) && alice.GetKeyId() == profile );
                ASSERT_TRUE( check.GetLinks( bob.GetKeyId(), links ) && 2 == links.size() );
            }
            ASSERT_FALSE( second.Resolve( AUTH::CAccountRef( "google", "g-1" ), profile ) );

            // an account that one linked to a profile is a conflict for the other
            ASSERT_TRUE( AUTH::LINKRESULT_OK == first.Link( MakeRecord( alice, "epic:e-9", 5000 ) ) );
            ASSERT_TRUE( AUTH::LINKRESULT_CONFLICT == second.Link( MakeRecord( bob, "epic:e-9", 6000 ) ) );
            {
                AUTH::CLocalIdentityLinkStore check( path );
                ASSERT_TRUE( check.Resolve( AUTH::CAccountRef( "epic", "e-9" ), profile ) && alice.GetKeyId() == profile );
            }

            // a merge by one moves what the other linked, and the other changes the merged links
            ASSERT_TRUE( AUTH::LINKRESULT_OK == second.Link( MakeRecord( alice, "google:a-3", 7000 ) ) );
            UInt32 merged = 0;
            ASSERT_TRUE( AUTH::LINKRESULT_OK == first.Merge( alice.GetKeyId(), bob, &merged ) && 3 == merged );
            ASSERT_TRUE( second.GetLinks( alice.GetKeyId(), links ) && links.empty() );
            ASSERT_TRUE( second.GetLinks( bob.GetKeyId(), links ) && 5 == links.size() );
            ASSERT_TRUE( second.Unlink( AUTH::CAccountRef( "epic", "765" ), bob.GetKeyId() ) );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == first.Link( MakeRecord( alice, "google:a-4", 8000 ) ) );
            AUTH::CLocalIdentityLinkStore check( path );
            ASSERT_TRUE( check.GetLinks( bob.GetKeyId(), links ) && 4 == links.size() );
            ASSERT_TRUE( check.GetLinks( alice.GetKeyId(), links ) && 1 == links.size() && "google:a-4" == links[ 0 ].accountRef.ToString() );
            ASSERT_FALSE( check.Resolve( AUTH::CAccountRef( "epic", "765" ), profile ) );
            ASSERT_TRUE( 0 == check.GetRefusedCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Two threads with an instance each link into one file at the same time without losing a link" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "links_threads" );
            CORE::CString path = CORE::CombinePath( dir, "links.json" );
            const UInt32 AccountCount = 40;
            CTestSigner seed( "seed" );
            {
                AUTH::CLocalIdentityLinkStore store( path );
                ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( seed, "google:seed" ) ) );
            }
            CLinkWriterThread left( path, "left", AccountCount );
            CLinkWriterThread right( path, "right", AccountCount );
            ASSERT_TRUE( left.Activate( GUCEF_NULL, 1, 1 ) );
            ASSERT_TRUE( right.Activate( GUCEF_NULL, 1, 1 ) );
            for ( UInt32 waited=0; waited<240000 && !( left.IsDone() && right.IsDone() ); waited+=5 )
                MT::PrecisionDelay( 5 );
            ASSERT_TRUE( left.IsDone() && right.IsDone() );
            left.Deactivate( false, true );
            right.Deactivate( false, true );
            ASSERT_TRUE( 0 == left.GetFailures() && 0 == right.GetFailures() );

            UInt32 scratch = 0;
            ASSERT_TRUE( AccountCount == CountLinkedAccounts( path, "left", AccountCount, scratch ) && 0 == scratch );
            ASSERT_TRUE( AccountCount == CountLinkedAccounts( path, "right", AccountCount, scratch ) && 0 == scratch );
            AUTH::CLocalIdentityLinkStore check( path );
            CORE::CString profile;
            ASSERT_TRUE( check.Resolve( AUTH::CAccountRef( "google", "seed" ), profile ) && seed.GetKeyId() == profile );
            ASSERT_TRUE( 0 == check.GetRefusedCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A second process links into one file at the same time as this one without losing a link" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_LINUX )
        GUCEF_TESTFW_SKIP( "The test application is started again as a second process on Linux" );
        #endif
        try
        {
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            CORE::CString dir = MakeTestDirectory( "links_process" );
            CORE::CString syncDir = MakeTestDirectory( "links_process_sync" );
            ASSERT_TRUE( CORE::CreateDirs( syncDir ) );
            CORE::CString path = CORE::CombinePath( dir, "links.json" );
            const UInt32 AccountCount = 40;
            CTestSigner seed( "seed" );
            {
                AUTH::CLocalIdentityLinkStore store( path );
                ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( seed, "google:seed" ) ) );
            }

            CORE::CString exePath = GetOwnExecutablePath();
            REQUIRE_TRUE( !exePath.IsNULLOrEmpty() );
            CORE::CProcess child;
            child.SetIsolateFromParent( true );
            CORE::CProcess::TStringMap environment;
            environment[ "GUCEF_AUTH_TEST_HELPER" ] = "linkstore-writer";
            environment[ "GUCEF_AUTH_TEST_STORE_PATH" ] = path;
            environment[ "GUCEF_AUTH_TEST_SYNC_DIR" ] = syncDir;
            environment[ "GUCEF_AUTH_TEST_PREFIX" ] = "child";
            environment[ "GUCEF_AUTH_TEST_COUNT" ] = CORE::ToString( AccountCount );
            environment[ "GUCEF_CONSOLE_LOGGER" ] = "none";
            TESTFW::KeepChildExitCodeUnderSanitizers( environment );     // the exit code is the child's own, its sanitizer reports go to a log
            REQUIRE_TRUE( child.Launch( exePath, CORE::CProcess::TStringVector(), CORE::CString::Empty, environment ) );
            ASSERT_TRUE( WaitForFile( CORE::CombinePath( syncDir, "ready-child" ), 60000 ) );
            CORE::CString go = "go";
            ASSERT_TRUE( CORE::WriteFileAtomically( CORE::CombinePath( syncDir, "go" ), go.C_String(), go.Length() ) );
            UInt32 failures = LinkAccounts( path, "parent", AccountCount );
            Int32 exitCode = -1;
            ASSERT_TRUE( child.WaitForExit( 240000, exitCode ) );
            ASSERT_TRUE( 0 == exitCode );
            ASSERT_TRUE( 0 == failures );

            UInt32 scratch = 0;
            ASSERT_TRUE( AccountCount == CountLinkedAccounts( path, "parent", AccountCount, scratch ) && 0 == scratch );
            ASSERT_TRUE( AccountCount == CountLinkedAccounts( path, "child", AccountCount, scratch ) && 0 == scratch );
            AUTH::CLocalIdentityLinkStore check( path );
            CORE::CString profile;
            ASSERT_TRUE( check.Resolve( AUTH::CAccountRef( "google", "seed" ), profile ) && seed.GetKeyId() == profile );
            ASSERT_TRUE( 0 == check.GetRefusedCount() );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An instance waits for the lock of another one and gives up after the lock timeout without changing anything" )
        GUCEF_TESTFW_SKIP_IF( !JsonCodecAvailable(), "The json codec plugin is not available" );
        GUCEF_TESTFW_SKIP_IF( !AUTH::CAuthCrypto::IsSignatureAvailable(), "The ed25519 codec is not available" );
        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_LINUX )
        GUCEF_TESTFW_SKIP( "The lock is held with flock() by the test on Linux" );
        #endif
        try
        {
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            CORE::CString dir = MakeTestDirectory( "links_lock_timeout" );
            CORE::CString path = CORE::CombinePath( dir, "links.json" );
            CORE::CString lockPath = path + ".lock";
            CTestSigner alice( "alice" );
            AUTH::CLocalIdentityLinkStore store( path );
            ASSERT_TRUE( AUTH::CLocalIdentityLinkStore::DefaultLockTimeoutInMs == store.GetLockTimeoutInMs() );
            ASSERT_TRUE( lockPath == store.GetLockPath() && AUTH::CLocalIdentityLinkStore().GetLockPath().IsNULLOrEmpty() );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( alice, "google:g-1", 1000 ) ) );
            ASSERT_TRUE( CORE::FileExists( lockPath ) );
            struct stat info;
            ASSERT_TRUE( 0 == stat( lockPath.C_String(), &info ) && 0600 == ( info.st_mode & 0777 ) );
            CORE::CString before = ReadTextFile( path );

            // the lock timeout is configuration
            CORE::CDataNode config( "Links", GUCEF_DATATYPE_OBJECT );
            config.SetAttribute( "path", path );
            config.SetAttribute( "lockTimeoutInMs", CORE::CString( "150" ) );
            ASSERT_TRUE( store.LoadConfig( config ) && 150 == store.GetLockTimeoutInMs() );
            CORE::CDataNode saved( "Links", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( store.SaveConfig( saved ) && 150 == saved.GetAttributeValueOrChildValueByName( "lockTimeoutInMs" ).AsUInt32() );

            // another holder of the exclusive lock: a change and a read wait for the timeout, then give up
            int holder = open( lockPath.C_String(), O_RDWR | O_CLOEXEC );
            REQUIRE_TRUE( holder >= 0 && 0 == flock( holder, LOCK_EX ) );
            UInt64 startedAt = AUTH::GetMonotonicTimeInMs();
            ASSERT_TRUE( AUTH::LINKRESULT_STORE_ERROR == store.Link( MakeRecord( alice, "epic:765", 2000 ) ) );
            UInt64 waited = AUTH::GetMonotonicTimeInMs() - startedAt;
            ASSERT_TRUE( waited >= 140 && waited < 5000 );
            ASSERT_FALSE( store.Unlink( AUTH::CAccountRef( "google", "g-1" ), alice.GetKeyId() ) );
            CTestSigner bob( "bob" );
            ASSERT_TRUE( AUTH::LINKRESULT_STORE_ERROR == store.Merge( alice.GetKeyId(), bob ) );
            AUTH::TLinkRecordVector links;
            CORE::CString profile;
            ASSERT_FALSE( store.GetLinks( alice.GetKeyId(), links ) );
            ASSERT_FALSE( store.Resolve( AUTH::CAccountRef( "google", "g-1" ), profile ) );
            ASSERT_TRUE( before == ReadTextFile( path ) );

            // a shared holder: reads go on, a change still waits
            ASSERT_TRUE( 0 == flock( holder, LOCK_SH ) );
            ASSERT_TRUE( store.GetLinks( alice.GetKeyId(), links ) && 1 == links.size() );
            ASSERT_TRUE( store.Resolve( AUTH::CAccountRef( "google", "g-1" ), profile ) && alice.GetKeyId() == profile );
            ASSERT_TRUE( AUTH::LINKRESULT_STORE_ERROR == store.Link( MakeRecord( alice, "epic:765", 2000 ) ) );
            ASSERT_TRUE( before == ReadTextFile( path ) );

            // let go: the change goes through, and the lock file stays
            ASSERT_TRUE( 0 == flock( holder, LOCK_UN ) );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( alice, "epic:765", 2000 ) ) );
            ASSERT_TRUE( store.GetLinks( alice.GetKeyId(), links ) && 2 == links.size() );
            ASSERT_TRUE( store.Unlink( AUTH::CAccountRef( "epic", "765" ), alice.GetKeyId() ) && CORE::FileExists( lockPath ) );
            close( holder );

            // the timeout set in code
            store.SetLockTimeoutInMs( 0 );
            ASSERT_TRUE( 0 == store.GetLockTimeoutInMs() );
            ASSERT_TRUE( AUTH::LINKRESULT_OK == store.Link( MakeRecord( alice, "epic:766", 3000 ) ) );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
