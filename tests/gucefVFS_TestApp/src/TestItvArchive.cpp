/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestArchiveUtils.h"
#include "TestItvArchive.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace TestArchiveUtils;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt16 TestRecordType = 7;

struct SItvTestResource
{
    Int32 id;
    CORE::CString content;
};

/*-------------------------------------------------------------------------*/

static void
PutLittleEndian32( CORE::CDynamicBuffer& buffer, UInt32 value )
{
    // explicit byte order, the test must not depend on the platform it runs on
    const UInt8 bytes[ 4 ] = { (UInt8) ( value & 0xFF ), (UInt8) ( ( value >> 8 ) & 0xFF ), (UInt8) ( ( value >> 16 ) & 0xFF ), (UInt8) ( ( value >> 24 ) & 0xFF ) };
    buffer.Append( bytes, 4 );
}

/*-------------------------------------------------------------------------*/

static bool
WriteItvArchive( const CORE::CString& idxPath, const CORE::CString& resPath, const SItvTestResource* resources, UInt32 count )
{
    CORE::CDynamicBuffer idx;
    const UInt8 type[ 2 ] = { (UInt8) ( TestRecordType & 0xFF ), (UInt8) ( TestRecordType >> 8 ) };
    idx.Append( type, 2 );

    CORE::CDynamicBuffer res;
    for ( UInt32 i=0; i<count; ++i )
    {
        PutLittleEndian32( idx, (UInt32) resources[ i ].id );
        PutLittleEndian32( idx, res.GetDataSize() );
        res.Append( resources[ i ].content.C_String(), resources[ i ].content.Length() );
    }

    CORE::CFileAccess idxFile( idxPath, "wb" );
    CORE::CFileAccess resFile( resPath, "wb" );
    return idxFile.Opened() && resFile.Opened() &&
           idxFile.Write( idx.GetConstBufferPtr(), 1, idx.GetDataSize() ) == idx.GetDataSize() &&
           resFile.Write( res.GetConstBufferPtr(), 1, res.GetDataSize() ) == res.GetDataSize();
}

/*-------------------------------------------------------------------------*/

static void
CheckItvArchiveContent( VFS::CVFS& vfs, const CORE::CString& mountPoint, const SItvTestResource* resources, UInt32 count )
{
    VFS::CVFS::TStringVector files;
    vfs.GetFileList( files, mountPoint, false, false, CORE::CString::EmptyStringSet );
    ASSERT_TRUE( files.size() == count );

    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CString name = CORE::Int32ToString( resources[ i ].id ) + ".itv" + CORE::UInt32ToString( TestRecordType );
        CORE::CString path = mountPoint + "/" + name;
        ASSERT_TRUE( vfs.FileExists( path ) );
        ASSERT_TRUE( vfs.GetFileSize( path ) == resources[ i ].content.Length() );

        CORE::CDynamicBuffer loaded;
        ASSERT_TRUE( vfs.LoadFile( loaded, path, "rb" ) );
        ASSERT_TRUE( loaded.GetDataSize() == resources[ i ].content.Length() );
        ASSERT_TRUE( 0 == memcmp( loaded.GetConstBufferPtr(), resources[ i ].content.C_String(), resources[ i ].content.Length() ) );
    }

    ASSERT_FALSE( vfs.FileExists( mountPoint + "/99999.itv" + CORE::UInt32ToString( TestRecordType ) ) );
    ASSERT_FALSE( vfs.FileExists( mountPoint + "/10.itv9" ) ); // wrong record type
}

/*-------------------------------------------------------------------------*/

void
PerformItvArchiveTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING ITV archive plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "ItvArchive" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    bool pluginLoaded = LoadVfsPlugin( "vfspluginITV" );

    CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_ItvArchive" );
    CORE::CreateDirs( dir );
    vfs.AddRoot( dir, "ItvTempDir", false, false, "itvtemp" );

    // One resource each for being served from memory, being served as a window on the data file, and
    // one at the very end of the file, which has no next index entry to tell its size
    SItvTestResource resources[ 3 ];
    resources[ 0 ].id = 10;
    resources[ 0 ].content = "0123456789";
    resources[ 1 ].id = 20;
    for ( UInt32 i=0; i<3000; ++i )
        resources[ 1 ].content += (char) ( 'a' + ( i % 26 ) );
    resources[ 2 ].id = 30;
    resources[ 2 ].content = "tail!";

    const CORE::CString idxPath = CORE::CombinePath( dir, "test.IDX" );
    const CORE::CString resPath = CORE::CombinePath( dir, "test.RES" );
    const bool written = WriteItvArchive( idxPath, resPath, resources, 3 );

    GUCEF_TESTFW_TESTCASE( "Test: The plugin provides the archive types" )
        try
        {
            ASSERT_TRUE( pluginLoaded );
            ASSERT_TRUE( written );
            VFS::CVFS::TAbstractArchiveFactory::TKeySet readerTypes;
            vfs.GetListOfSupportedArchiveTypes( readerTypes );
            ASSERT_TRUE( readerTypes.find( "res" ) != readerTypes.end() );
            ASSERT_TRUE( readerTypes.find( "idx" ) != readerTypes.end() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An archive is mounted from a path and reads back" )
        try
        {
            ASSERT_TRUE( vfs.MountArchive( "itvByPath", "itvtemp/test.RES", "res", "/itvpath", false, false ) );
            CheckItvArchiveContent( vfs, "/itvpath", resources, 3 );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "itvByPath" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An archive is mounted from the opened resource data file, finding the index next to it" )
        try
        {
            VFS::TBasicVfsResourcePtr resource = vfs.GetFile( "itvtemp/test.RES", "rb" );
            ASSERT_FALSE( resource.IsNULL() );
            if ( !resource.IsNULL() )
            {
                ASSERT_FALSE( resource->GetVfsPath().IsNULLOrEmpty() );
                ASSERT_TRUE( vfs.MountArchive( "itvByRes", resource, "res", "/itvres", false ) );
                CheckItvArchiveContent( vfs, "/itvres", resources, 3 );
                ASSERT_TRUE( vfs.UnmountArchiveByName( "itvByRes" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An archive is mounted from the opened index file, finding the data next to it" )
        try
        {
            VFS::TBasicVfsResourcePtr resource = vfs.GetFile( "itvtemp/test.IDX", "rb" );
            ASSERT_FALSE( resource.IsNULL() );
            if ( !resource.IsNULL() )
            {
                ASSERT_TRUE( vfs.MountArchive( "itvByIdx", resource, "idx", "/itvidx", false ) );
                CheckItvArchiveContent( vfs, "/itvidx", resources, 3 );
                ASSERT_TRUE( vfs.UnmountArchiveByName( "itvByIdx" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Mounting fails cleanly when the companion file is missing or the resource is not part of an archive" )
        try
        {
            const CORE::CString lonelyPath = CORE::CombinePath( dir, "lonely.RES" );
            {
                CORE::CFileAccess lonely( lonelyPath, "wb" );
                lonely.Write( "x", 1, 1 );
                CORE::CFileAccess other( CORE::CombinePath( dir, "other.txt" ), "wb" );
                other.Write( "x", 1, 1 );
            }
            VFS::TBasicVfsResourcePtr lonelyResource = vfs.GetFile( "itvtemp/lonely.RES", "rb" );
            ASSERT_FALSE( lonelyResource.IsNULL() );
            ASSERT_FALSE( vfs.MountArchive( "itvLonely", lonelyResource, "res", "/itvlonely", false ) );

            VFS::TBasicVfsResourcePtr otherResource = vfs.GetFile( "itvtemp/other.txt", "rb" );
            ASSERT_FALSE( otherResource.IsNULL() );
            ASSERT_FALSE( vfs.MountArchive( "itvOther", otherResource, "res", "/itvother", false ) );

            VFS::TBasicVfsResourcePtr resource = vfs.GetFile( "itvtemp/test.RES", "rb" );
            ASSERT_FALSE( resource.IsNULL() );
            ASSERT_FALSE( vfs.MountArchive( "itvWritable", resource, "res", "/itvwritable", true ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
