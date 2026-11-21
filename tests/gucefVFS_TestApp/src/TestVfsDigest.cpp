/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVfsBlockCacheUtils.h"
#include "TestVfsDigest.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEST ARCHIVE                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** An archive that has a hash string and does not override GetFileDigest(): what the default of CArchive gives */
class CHashOnlyArchive : public VFS::CArchive
{
    public:

    CHashOnlyArchive( void ) : VFS::CArchive(), m_name( "hashonly" ), m_hash( "AbCdEf-0123" ) {}
    virtual ~CHashOnlyArchive() GUCEF_VIRTUAL_OVERRIDE {}

    virtual VFS::TBasicVfsResourcePtr GetFile( const VFS::CString&, const char*, const VFS::UInt32, const bool ) GUCEF_VIRTUAL_OVERRIDE { return VFS::TBasicVfsResourcePtr(); }
    virtual bool StoreAsFile( const CORE::CString&, const CORE::CDynamicBuffer&, const CORE::UInt64, const bool ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual void GetFileList( TStringVector&, const VFS::CString&, const VFS::CString&, bool, bool, const VFS::CString::StringSet&, VFS::UInt32 ) const GUCEF_VIRTUAL_OVERRIDE {}
    virtual void GetDirList( TStringVector&, const VFS::CString&, const VFS::CString&, bool, bool, const VFS::CString::StringSet&, VFS::UInt32 ) const GUCEF_VIRTUAL_OVERRIDE {}
    virtual bool DeleteFile( const VFS::CString& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool MoveFile( const VFS::CString&, const VFS::CString&, const bool ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool FileExists( const VFS::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE { return filePath == "present.bin"; }
    virtual bool DirExists( const VFS::CString& ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual VFS::UInt64 GetFileSize( const VFS::CString& ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual bool GetFileMetaData( const VFS::CString&, CORE::CResourceMetaData& ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual VFS::CString GetFileHash( const VFS::CString& file ) const GUCEF_VIRTUAL_OVERRIDE { return FileExists( file ) ? m_hash : VFS::CString::Empty; }
    virtual CORE::CDateTime GetFileModificationTime( const VFS::CString& ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::CDateTime(); }
    virtual const VFS::CString& GetArchiveName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_name; }
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool LoadArchive( const VFS::CArchiveSettings& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool LoadArchive( const VFS::CString&, VFS::TBasicVfsResourcePtr, const bool ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool UnloadArchive( void ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual const VFS::CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_name; }
    virtual void DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE { delete objectToBeDestroyed; }

    private:

    VFS::CString m_name;
    VFS::CString m_hash;
};

/** The 48 character form of CORE::MD5ToString() (16 decimal numbers of 3 digits) as the 32 hexadecimal characters of a digest */
CORE::CString
TripletsToHex( const CORE::CString& triplets )
{
    CORE::CString hex;
    if ( 48 != triplets.Length() )
        return hex;
    for ( UInt32 i=0; i<16; ++i )
    {
        char group[ 4 ];
        memcpy( group, triplets.C_String() + i * 3, 3 );
        group[ 3 ] = '\0';
        const UInt8 byte = (UInt8) atoi( group );
        hex += CORE::Base16Encode( &byte, 1 ).Lowercase();
    }
    return hex;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVfsDigestTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsDigest" );

    GUCEF_TESTFW_TESTCASE( "CVfsDigest: the value of a known algorithm is lower case hexadecimal without surrounding white space, an unknown algorithm keeps what it was given" )
    try
    {
        const VFS::CVfsDigest md5( VFS::CVfsDigest::ALGORITHM_MD5, "  900150983CD24FB0D6963F7D28E17F72\r\n" );
        ASSERT_TRUE( md5.value == "900150983cd24fb0d6963f7d28e17f72" );
        ASSERT_TRUE( md5.IsWellFormed() );
        ASSERT_FALSE( md5.IsEmpty() );

        // a value of the wrong form is kept (not repaired) and says so
        const VFS::CVfsDigest shortOne( VFS::CVfsDigest::ALGORITHM_SHA1, "abcd" );
        ASSERT_TRUE( shortOne.value == "abcd" );
        ASSERT_FALSE( shortOne.IsWellFormed() );
        const VFS::CVfsDigest notHex( VFS::CVfsDigest::ALGORITHM_MD5, "900150983cd24fb0d6963f7d28e17fzz" );
        ASSERT_FALSE( notHex.IsWellFormed() );
        const VFS::CVfsDigest longOne( VFS::CVfsDigest::ALGORITHM_MD5, "900150983cd24fb0d6963f7d28e17f72ab" );
        ASSERT_FALSE( longOne.IsWellFormed() );

        const VFS::CVfsDigest unknown( VFS::CVfsDigest::ALGORITHM_UNKNOWN, "AbCdEf-0123" );
        ASSERT_TRUE( unknown.value == "AbCdEf-0123" );
        ASSERT_FALSE( unknown.IsWellFormed() );
        ASSERT_FALSE( unknown.IsEmpty() );

        const VFS::CVfsDigest none;
        ASSERT_TRUE( none.IsEmpty() );
        ASSERT_TRUE( VFS::CVfsDigest::ALGORITHM_UNKNOWN == none.algorithm );
        ASSERT_FALSE( none.IsWellFormed() );

        VFS::CVfsDigest copy( md5 );
        ASSERT_TRUE( copy.algorithm == md5.algorithm && copy.value == md5.value );
        copy = unknown;
        ASSERT_TRUE( copy.algorithm == VFS::CVfsDigest::ALGORITHM_UNKNOWN && copy.value == "AbCdEf-0123" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CVfsDigest: Matches() needs two well formed digests of one known algorithm with the same value" )
    try
    {
        const VFS::CVfsDigest a( VFS::CVfsDigest::ALGORITHM_SHA256, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" );
        const VFS::CVfsDigest sameUpper( VFS::CVfsDigest::ALGORITHM_SHA256, "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD" );
        const VFS::CVfsDigest other( VFS::CVfsDigest::ALGORITHM_SHA256, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ae" );
        ASSERT_TRUE( a.Matches( a ) );
        ASSERT_TRUE( a.Matches( sameUpper ) );
        ASSERT_TRUE( sameUpper.Matches( a ) );
        ASSERT_FALSE( a.Matches( other ) );

        // the same characters under another algorithm are not the same digest
        const VFS::CVfsDigest md5Like( VFS::CVfsDigest::ALGORITHM_MD5, "ba7816bf8f01cfea414140de5dae2223" );
        const VFS::CVfsDigest sha1Like( VFS::CVfsDigest::ALGORITHM_SHA1, "ba7816bf8f01cfea414140de5dae2223b00361a3" );
        ASSERT_FALSE( md5Like.Matches( sha1Like ) );
        ASSERT_FALSE( md5Like.Matches( a ) );

        // unknown algorithms never match, not even themselves
        const VFS::CVfsDigest unknownA( VFS::CVfsDigest::ALGORITHM_UNKNOWN, "same" );
        const VFS::CVfsDigest unknownB( VFS::CVfsDigest::ALGORITHM_UNKNOWN, "same" );
        ASSERT_FALSE( unknownA.Matches( unknownB ) );
        ASSERT_FALSE( unknownA.Matches( unknownA ) );
        ASSERT_FALSE( VFS::CVfsDigest().Matches( VFS::CVfsDigest() ) );

        // a digest of the wrong form matches nothing
        const VFS::CVfsDigest bad1( VFS::CVfsDigest::ALGORITHM_MD5, "abc" );
        ASSERT_FALSE( bad1.Matches( bad1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CVfsDigest: names, lengths, bytes and back" )
    try
    {
        ASSERT_TRUE( 32 == VFS::CVfsDigest::GetHexLength( VFS::CVfsDigest::ALGORITHM_MD5 ) );
        ASSERT_TRUE( 40 == VFS::CVfsDigest::GetHexLength( VFS::CVfsDigest::ALGORITHM_SHA1 ) );
        ASSERT_TRUE( 64 == VFS::CVfsDigest::GetHexLength( VFS::CVfsDigest::ALGORITHM_SHA256 ) );
        ASSERT_TRUE( 0 == VFS::CVfsDigest::GetHexLength( VFS::CVfsDigest::ALGORITHM_UNKNOWN ) );
        ASSERT_TRUE( 16 == VFS::CVfsDigest::GetByteLength( VFS::CVfsDigest::ALGORITHM_MD5 ) );
        ASSERT_TRUE( 20 == VFS::CVfsDigest::GetByteLength( VFS::CVfsDigest::ALGORITHM_SHA1 ) );
        ASSERT_TRUE( 32 == VFS::CVfsDigest::GetByteLength( VFS::CVfsDigest::ALGORITHM_SHA256 ) );

        ASSERT_TRUE( 0 == strcmp( "md5", VFS::CVfsDigest::GetAlgorithmName( VFS::CVfsDigest::ALGORITHM_MD5 ) ) );
        ASSERT_TRUE( 0 == strcmp( "sha1", VFS::CVfsDigest::GetAlgorithmName( VFS::CVfsDigest::ALGORITHM_SHA1 ) ) );
        ASSERT_TRUE( 0 == strcmp( "sha256", VFS::CVfsDigest::GetAlgorithmName( VFS::CVfsDigest::ALGORITHM_SHA256 ) ) );
        ASSERT_TRUE( 0 == strcmp( "unknown", VFS::CVfsDigest::GetAlgorithmName( VFS::CVfsDigest::ALGORITHM_UNKNOWN ) ) );
        ASSERT_TRUE( VFS::CVfsDigest::ALGORITHM_MD5 == VFS::CVfsDigest::GetAlgorithmFromName( "MD5" ) );
        ASSERT_TRUE( VFS::CVfsDigest::ALGORITHM_SHA1 == VFS::CVfsDigest::GetAlgorithmFromName( "SHA-1" ) );
        ASSERT_TRUE( VFS::CVfsDigest::ALGORITHM_SHA256 == VFS::CVfsDigest::GetAlgorithmFromName( " sha256 " ) );
        ASSERT_TRUE( VFS::CVfsDigest::ALGORITHM_UNKNOWN == VFS::CVfsDigest::GetAlgorithmFromName( "crc32" ) );

        const UInt8 raw[ 16 ] = { 0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0, 0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72 };
        const VFS::CVfsDigest fromBytes = VFS::CVfsDigest::FromBytes( VFS::CVfsDigest::ALGORITHM_MD5, raw, 16 );
        ASSERT_TRUE( fromBytes.value == "900150983cd24fb0d6963f7d28e17f72" );
        UInt8 back[ 32 ];
        UInt32 backCount = 0;
        ASSERT_TRUE( fromBytes.ToBytes( back, backCount ) );
        ASSERT_TRUE( 16 == backCount && 0 == memcmp( back, raw, 16 ) );

        // the wrong number of bytes for the algorithm is no digest
        ASSERT_TRUE( VFS::CVfsDigest::FromBytes( VFS::CVfsDigest::ALGORITHM_SHA1, raw, 16 ).IsEmpty() );
        ASSERT_TRUE( VFS::CVfsDigest::FromBytes( VFS::CVfsDigest::ALGORITHM_UNKNOWN, raw, 16 ).IsEmpty() );
        ASSERT_FALSE( VFS::CVfsDigest( VFS::CVfsDigest::ALGORITHM_MD5, "xyz" ).ToBytes( back, backCount ) );
        ASSERT_TRUE( 0 == backCount );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CVfsDigestHasher: the three algorithms give the published values, in one piece and in parts" )
    try
    {
        // RFC 1321, FIPS 180 test vectors of "abc" and of the empty message
        ASSERT_TRUE( VFS::CVfsDigestHasher::Compute( VFS::CVfsDigest::ALGORITHM_MD5, "abc", 3 ).value == "900150983cd24fb0d6963f7d28e17f72" );
        ASSERT_TRUE( VFS::CVfsDigestHasher::Compute( VFS::CVfsDigest::ALGORITHM_SHA1, "abc", 3 ).value == "a9993e364706816aba3e25717850c26c9cd0d89d" );
        ASSERT_TRUE( VFS::CVfsDigestHasher::Compute( VFS::CVfsDigest::ALGORITHM_SHA256, "abc", 3 ).value == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" );
        ASSERT_TRUE( VFS::CVfsDigestHasher::Compute( VFS::CVfsDigest::ALGORITHM_MD5, "", 0 ).value == "d41d8cd98f00b204e9800998ecf8427e" );
        ASSERT_TRUE( VFS::CVfsDigestHasher::Compute( VFS::CVfsDigest::ALGORITHM_SHA1, "", 0 ).value == "da39a3ee5e6b4b0d3255bfef95601890afd80709" );
        ASSERT_TRUE( VFS::CVfsDigestHasher::Compute( VFS::CVfsDigest::ALGORITHM_SHA256, "", 0 ).value == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" );

        // parts of every size give what one piece gives
        const UInt32 algorithms[ 3 ] = { VFS::CVfsDigest::ALGORITHM_MD5, VFS::CVfsDigest::ALGORITHM_SHA1, VFS::CVfsDigest::ALGORITHM_SHA256 };
        GUCEF::vector< UInt8 > data;
        data.resize( 5000 );
        BlockCacheTest::FillPattern( 3, 0, &data[ 0 ], 5000 );
        for ( UInt32 a=0; a<3; ++a )
        {
            const VFS::CVfsDigest whole = VFS::CVfsDigestHasher::Compute( (VFS::CVfsDigest::EAlgorithm) algorithms[ a ], &data[ 0 ], 5000 );
            ASSERT_TRUE( whole.IsWellFormed() );
            for ( UInt32 step = 1; step <= 4999; step += 311 )
            {
                VFS::CVfsDigestHasher hasher( (VFS::CVfsDigest::EAlgorithm) algorithms[ a ] );
                for ( UInt32 at = 0; at < 5000; at += step )
                    hasher.Update( &data[ 0 ] + at, at + step <= 5000 ? step : 5000 - at );
                ASSERT_TRUE( hasher.Final().Matches( whole ) );
            }
        }

        // an unknown algorithm hashes nothing
        VFS::CVfsDigestHasher unknown( VFS::CVfsDigest::ALGORITHM_UNKNOWN );
        ASSERT_FALSE( unknown.IsSupported() );
        unknown.Update( "abc", 3 );
        ASSERT_TRUE( unknown.Final().IsEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileDigest: the file system archive names MD5, the value is the 32 character MD5 of the content and the one of GetFileHash(); an absent file has none" )
    try
    {
        CBlockCacheTestFolder folder( "digestfs" );
        CORE::CDynamicBuffer content;
        content.SetBufferSize( 70000 );
        BlockCacheTest::FillPattern( 11, 0, content.GetBufferPtr(), 70000 );
        content.SetDataSize( 70000 );
        ASSERT_TRUE( BlockCacheTest::WriteWholeFile( folder.GetChildPath( "data.bin" ), content ) );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( folder.GetChildPath( "abc.txt" ), "abc" ) );
        CORE::CFileAccess empty( folder.GetChildPath( "empty.bin" ), "wb" );
        empty.Close();

        VFS::CFileSystemArchive archive;
        VFS::CArchiveSettings settings;
        settings.SetArchiveName( "DigestFs" );
        settings.SetActualArchivePath( folder.GetPath() );
        settings.SetArchivePath( folder.GetPath() );
        settings.SetWriteableRequested( false );
        ASSERT_TRUE( archive.LoadArchive( settings ) );

        const VFS::CVfsDigest digest = archive.GetFileDigest( "data.bin" );
        ASSERT_TRUE( VFS::CVfsDigest::ALGORITHM_MD5 == digest.algorithm );
        ASSERT_TRUE( digest.IsWellFormed() );
        ASSERT_TRUE( digest.Matches( BlockCacheTest::PatternDigest( VFS::CVfsDigest::ALGORITHM_MD5, 11, 70000 ) ) );
        ASSERT_TRUE( digest.value == TripletsToHex( archive.GetFileHash( "data.bin" ) ) );

        // the published MD5 of "abc" and of nothing
        ASSERT_TRUE( archive.GetFileDigest( "abc.txt" ).value == "900150983cd24fb0d6963f7d28e17f72" );
        const VFS::CVfsDigest emptyDigest = archive.GetFileDigest( "empty.bin" );
        ASSERT_TRUE( VFS::CVfsDigest::ALGORITHM_MD5 == emptyDigest.algorithm && emptyDigest.value == "d41d8cd98f00b204e9800998ecf8427e" );

        // not there: no digest at all, unknown algorithm included
        const VFS::CVfsDigest absent = archive.GetFileDigest( "absent.bin" );
        ASSERT_TRUE( absent.IsEmpty() && VFS::CVfsDigest::ALGORITHM_UNKNOWN == absent.algorithm );
        ASSERT_TRUE( archive.GetFileDigest( "nested/absent.bin" ).IsEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GetFileDigest: an archive that does not override it answers the hash string with an unknown algorithm" )
    try
    {
        CHashOnlyArchive archive;
        const VFS::CVfsDigest digest = archive.GetFileDigest( "present.bin" );
        ASSERT_TRUE( VFS::CVfsDigest::ALGORITHM_UNKNOWN == digest.algorithm );
        ASSERT_TRUE( digest.value == "AbCdEf-0123" );
        ASSERT_FALSE( digest.Matches( digest ) );
        ASSERT_TRUE( archive.GetFileDigest( "absent.bin" ).IsEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CVFS::GetFileDigest: the first mount that has the file answers, false with no digest for an absent file" )
    try
    {
        CBlockCacheTestFolder folder( "digestvfs" );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( folder.GetChildPath( "abc.txt" ), "abc" ) );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        ASSERT_TRUE( vfs.AddRoot( folder.GetPath(), "DigestVfsTest", false, false, "/digestvfstest/" ) );

        VFS::CVfsDigest digest;
        ASSERT_TRUE( vfs.GetFileDigest( "/digestvfstest/abc.txt", digest ) );
        ASSERT_TRUE( VFS::CVfsDigest::ALGORITHM_MD5 == digest.algorithm && digest.value == "900150983cd24fb0d6963f7d28e17f72" );

        VFS::CVfsDigest other( VFS::CVfsDigest::ALGORITHM_SHA1, "left over from before" );
        ASSERT_FALSE( vfs.GetFileDigest( "/digestvfstest/absent.txt", other ) );
        ASSERT_TRUE( other.IsEmpty() );
        ASSERT_FALSE( vfs.GetFileDigest( "/nomountatall/abc.txt", other ) );

        ASSERT_TRUE( vfs.UnmountArchiveByName( "DigestVfsTest" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Result value REMOTE_NOT_YET_AVAILABLE: 65, its own name, not a success, known to the VFS call result" )
    try
    {
        ASSERT_TRUE( 65 == (int) CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE );
        ASSERT_TRUE( CORE::CString( "FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE" ) == CORE::FileSystemAccessResultToString( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE ) );
        ASSERT_FALSE( CORE::FileSystemAccessSucceeded( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE ) );

        // none of its neighbours has its name or its number
        const CORE::TFileSystemAccessResult neighbours[] = { CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING, CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED,
                                                             CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER, CORE::FILESYSTEMACCESS_RESOURCE_CORRUPT,
                                                             CORE::FILESYSTEMACCESS_REMOTE_PROTOCOL_ERROR, CORE::FILESYSTEMACCESS_FAILED };
        for ( UInt32 n=0; n<sizeof( neighbours ) / sizeof( neighbours[ 0 ] ); ++n )
        {
            ASSERT_TRUE( neighbours[ n ] != CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE );
            ASSERT_TRUE( CORE::FileSystemAccessResultToString( neighbours[ n ] ) != CORE::FileSystemAccessResultToString( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE ) );
        }

        // the mapping of the VFS call result: temporary answers are the three that a later call or an arrival event resolves
        ASSERT_TRUE( VFS::CVfsCallResult().IsSuccess() );
        ASSERT_FALSE( VFS::CVfsCallResult().IsTemporary() );
        const VFS::CVfsCallResult pending( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE, "fetch started", true );
        ASSERT_FALSE( pending.IsSuccess() );
        ASSERT_TRUE( pending.IsTemporary() );
        ASSERT_TRUE( pending.fetchStarted );
        ASSERT_TRUE( pending.message == "fetch started" );
        ASSERT_TRUE( pending.ToString().HasSubstr( "FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE" ) >= 0 && pending.ToString().HasSubstr( "fetch started" ) >= 0 );
        ASSERT_TRUE( VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING ).IsTemporary() );
        ASSERT_TRUE( VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER ).IsTemporary() );
        ASSERT_FALSE( VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_NOT_FOUND ).IsTemporary() );
        ASSERT_FALSE( VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_RESOURCE_CORRUPT ).IsTemporary() );
        ASSERT_FALSE( VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED ).IsTemporary() );
        VFS::CVfsCallResult assigned;
        assigned = pending;
        ASSERT_TRUE( assigned.result == pending.result && assigned.fetchStarted && assigned.message == pending.message );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
