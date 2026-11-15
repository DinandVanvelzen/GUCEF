/*
 *  guidriverMyGUI_TestApp: Application for testing the MyGUI GUI drivers
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

#include "MyGUI_DataManager.h"
#include "MyGUI_DataStreamHolder.h"

#include "guidriverMyGUI_CMyGUIDataManager.h"
#include "guidriverMyGUI_CMyGUIVfsDataStream.h"

#include "guidriverMyGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

const char* const MountName = "mygui_test_data";

bool
WriteTestFile( const CORE::CString& directory, const CORE::CString& name, const char* content, UInt32 size )
{
    CORE::CString path = directory;
    CORE::AppendToPath( path, name );
    CORE::CFileAccess file( path, "wb" );
    return file.IsValid() && size == file.Write( content, 1, size );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMyGUIDataManagerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MyGUI data manager over the VFS" );

    // a directory with files, the only way MyGUI can see them is through the VFS mount
    CORE::CString directory = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( directory, MountName );
    CORE::CreateDirs( directory );
    CORE::CreateDirs( directory + "/sub" );
    const char text[] = "first line\nsecond line\r\nthird";
    const char binary[] = { 'a', 0, 'b', (char) 0xFF, 'c' };
    bool prepared = WriteTestFile( directory, "lines.txt", text, (UInt32) sizeof( text ) - 1 ) &&
                    WriteTestFile( directory, "bytes.bin", binary, (UInt32) sizeof( binary ) ) &&
                    WriteTestFile( directory + "/sub", "skin.xml", "<MyGUI/>", 8 ) &&
                    WriteTestFile( directory + "/sub", "other.layout", "x", 1 );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( prepared )
        prepared = vfs.MountArchive( MountName, directory, VFS::CVFS::FileSystemArchiveTypeName, MountName, false, false, directory );

    GUCEF_TESTFW_TESTCASE( "The test data is mounted in the VFS" )
    ASSERT_TRUE( prepared );
    ASSERT_TRUE( prepared && vfs.FileExists( CORE::CString( MountName ) + "/lines.txt" ) );
    GUCEF_TESTFW_TESTCASE_END
    if ( !prepared )
        return;

    GUCEF_TESTFW_TESTCASE( "Streams give the bytes of a VFS file, the size, lines and the end" )
    try
    {
        guidriverMyGUI::CMyGUIDataManager manager;
        manager.AddSearchPath( MountName );

        MyGUI::IDataStream* stream = manager.getData( "lines.txt" );
        ASSERT_TRUE( GUCEF_NULL != stream );
        if ( GUCEF_NULL != stream )
        {
            ASSERT_TRUE( sizeof( text ) - 1 == stream->size() );
            ASSERT_FALSE( stream->eof() );
            std::string line;
            stream->readline( line );
            ASSERT_TRUE( "first line" == line );
            stream->readline( line );
            ASSERT_TRUE( "second line\r" == line );
            char rest[ 16 ];
            size_t got = stream->read( rest, sizeof( rest ) );
            ASSERT_TRUE( 5 == got && 0 == memcmp( rest, "third", 5 ) );
            ASSERT_TRUE( stream->eof() );
            ASSERT_TRUE( 0 == stream->read( rest, sizeof( rest ) ) );
            manager.freeData( stream );
        }

        // embedded NULs and high bytes survive
        stream = manager.getData( "bytes.bin" );
        ASSERT_TRUE( GUCEF_NULL != stream );
        if ( GUCEF_NULL != stream )
        {
            std::vector< std::byte > all = stream->readAll();
            ASSERT_TRUE( sizeof( binary ) == all.size() && 0 == memcmp( all.data(), binary, sizeof( binary ) ) );
            manager.freeData( stream );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Names are looked up in the search paths in order, then as VFS paths; missing ones are refused" )
    try
    {
        guidriverMyGUI::CMyGUIDataManager manager;
        ASSERT_FALSE( manager.isDataExist( "lines.txt" ) );
        ASSERT_TRUE( GUCEF_NULL == manager.getData( "lines.txt" ) );

        // without a search path the full VFS path works
        ASSERT_TRUE( manager.isDataExist( std::string( MountName ) + "/lines.txt" ) );

        manager.AddSearchPath( MountName );
        manager.AddSearchPath( MountName );
        ASSERT_TRUE( 1 == manager.GetSearchPaths().size() );
        ASSERT_TRUE( manager.isDataExist( "lines.txt" ) );
        ASSERT_FALSE( manager.isDataExist( "skin.xml" ) );
        ASSERT_TRUE( GUCEF_NULL == manager.getData( "nothing.here" ) );

        manager.AddSearchPath( CORE::CString( MountName ) + "/sub" );
        ASSERT_TRUE( manager.isDataExist( "skin.xml" ) );
        ASSERT_TRUE( ( std::string( MountName ) + "/sub/skin.xml" ) == manager.getDataPath( "skin.xml" ) );
        ASSERT_TRUE( ( std::string( MountName ) + "/lines.txt" ) == manager.getDataPath( "lines.txt" ) );
        ASSERT_TRUE( manager.getDataPath( "nothing.here" ).empty() );

        manager.RemoveSearchPath( CORE::CString( MountName ) + "/sub" );
        ASSERT_FALSE( manager.isDataExist( "skin.xml" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "The data manager is the one MyGUI finds, and lists the files that match a pattern" )
    try
    {
        guidriverMyGUI::CMyGUIDataManager manager;
        manager.AddSearchPath( CORE::CString( MountName ) + "/sub" );
        ASSERT_TRUE( &manager == MyGUI::DataManager::getInstancePtr() );

        MyGUI::VectorString layouts = manager.getDataListNames( "*.layout" );
        ASSERT_TRUE( 1 == layouts.size() && ( layouts.empty() || "other.layout" == layouts[ 0 ] ) );
        MyGUI::VectorString all = manager.getDataListNames( "*" );
        ASSERT_TRUE( 2 == all.size() );

        // the holder of MyGUI owns the stream while it lives
        {
            MyGUI::DataStreamHolder holder = MyGUI::DataManager::getInstance().getDataHolder( "skin.xml" );
            ASSERT_TRUE( holder.getData() != GUCEF_NULL );
            if ( holder.getData() != GUCEF_NULL )
                ASSERT_TRUE( "<MyGUI/>" == holder.getData()->readAllText() );
        }
        MyGUI::DataStreamHolder none = MyGUI::DataManager::getInstance().getDataHolder( "missing.xml" );
        ASSERT_TRUE( none.getData() == GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A stream on bytes in memory reads like a file" )
    try
    {
        const char bytes[] = "abc";
        guidriverMyGUI::CMyGUIVfsDataStream stream( bytes, 3 );
        ASSERT_TRUE( stream.IsValid() && 3 == stream.size() && !stream.eof() );
        char buffer[ 2 ];
        ASSERT_TRUE( 2 == stream.read( buffer, 2 ) && 'a' == buffer[ 0 ] && 'b' == buffer[ 1 ] );
        ASSERT_TRUE( 1 == stream.read( buffer, 2 ) && 'c' == buffer[ 0 ] && stream.eof() );

        guidriverMyGUI::CMyGUIVfsDataStream missing( CORE::CString( MountName ) + "/missing" );
        ASSERT_FALSE( missing.IsValid() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    vfs.UnmountArchiveByName( MountName );
}

/*-------------------------------------------------------------------------*/
