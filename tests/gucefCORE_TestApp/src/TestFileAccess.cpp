/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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
#include <stdio.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CMFILEACCESS_H
#include "CMFileAccess.h"
#define GUCEF_CORE_CMFILEACCESS_H
#endif /* GUCEF_CORE_CMFILEACCESS_H ? */

#ifndef GUCEF_CORE_CSUBIOACCESS_H
#include "gucefCORE_CSubIOAccess.h"
#define GUCEF_CORE_CSUBIOACCESS_H
#endif /* GUCEF_CORE_CSUBIOACCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #include <io.h>
    #include <fcntl.h>
    #include <sys/stat.h>
#else
    #include <unistd.h>
    #include <fcntl.h>
#endif

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#include "TestFileAccess.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static int
OpenDescriptorForReading( const CORE::CString& path )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return _open( path.C_String(), _O_RDONLY | _O_BINARY );
    #else
    return open( path.C_String(), O_RDONLY );
    #endif
}

/*-------------------------------------------------------------------------*/

static void
CloseDescriptor( int fd )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    _close( fd );
    #else
    close( fd );
    #endif
}

/*-------------------------------------------------------------------------*/

void
PerformFileAccessTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING FileAccess TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "FileAccess" );

    const char* payload = "0123456789abcdefghijklmnopqrstuvwxyz";
    const UInt32 payloadSize = (UInt32) strlen( payload );

    TESTFW::CScopedTestDir scopedDir( "gucefCORE_TestApp_FileAccess" );
    CORE::CString dir = scopedDir.GetPath();
    CORE::CString filePath = CORE::CombinePath( dir, "payload.bin" );
    {
        CORE::CFileAccess writer( filePath, "wb" );
        writer.Write( payload, 1, payloadSize );
    }

    GUCEF_TESTFW_TESTCASE( "Test: A file is read through a file descriptor that the access object owns" )
        try
        {
            int fd = OpenDescriptorForReading( filePath );
            ASSERT_TRUE( fd >= 0 );

            CORE::CFileAccess access;
            ASSERT_TRUE( access.OpenFromFileDescriptor( fd, "rb", true ) );
            ASSERT_TRUE( access.Opened() );
            ASSERT_TRUE( access.IsReadable() );
            ASSERT_FALSE( access.IsWriteable() );
            ASSERT_TRUE( access.IsValid() );
            ASSERT_TRUE( access.GetSize() == payloadSize );

            char buffer[ 64 ];
            memset( buffer, 0, sizeof( buffer ) );
            ASSERT_TRUE( access.Read( buffer, 1, 10 ) == 10 );
            ASSERT_TRUE( 0 == memcmp( buffer, payload, 10 ) );
            ASSERT_TRUE( access.Tell() == 10 );

            // size queries must not disturb the position
            ASSERT_TRUE( access.GetSize() == payloadSize );
            ASSERT_TRUE( access.Tell() == 10 );

            ASSERT_TRUE( 0 == access.Setpos( 30 ) );
            ASSERT_TRUE( access.Read( buffer, 1, 6 ) == 6 );
            ASSERT_TRUE( 0 == memcmp( buffer, payload + 30, 6 ) );
            ASSERT_TRUE( access.Tell() == payloadSize );

            ASSERT_TRUE( 0 == access.Seek( -5, SEEK_END ) );
            ASSERT_TRUE( access.Read( buffer, 1, 5 ) == 5 );
            ASSERT_TRUE( 0 == memcmp( buffer, payload + payloadSize - 5, 5 ) );

            // No path means no independent clone
            ASSERT_TRUE( GUCEF_NULL == access.Clone() );

            access.Close();
            ASSERT_FALSE( access.Opened() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A descriptor that is borrowed stays open after the access object closes" )
        try
        {
            int fd = OpenDescriptorForReading( filePath );
            ASSERT_TRUE( fd >= 0 );
            {
                CORE::CFileAccess access;
                ASSERT_TRUE( access.OpenFromFileDescriptor( fd, "rb", false ) );
                ASSERT_TRUE( access.GetSize() == payloadSize );
                char c = 0;
                ASSERT_TRUE( access.Read( &c, 1, 1 ) == 1 );
                ASSERT_TRUE( c == payload[ 0 ] );
            }

            // The original descriptor must still be usable
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
            ASSERT_TRUE( _lseek( fd, 0, SEEK_SET ) >= 0 );
            #else
            ASSERT_TRUE( lseek( fd, 0, SEEK_SET ) >= 0 );
            #endif
            CloseDescriptor( fd );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Opening an invalid descriptor fails and leaves the access object closed" )
        try
        {
            CORE::CFileAccess access;
            ASSERT_FALSE( access.OpenFromFileDescriptor( -1, "rb", true ) );
            ASSERT_FALSE( access.Opened() );
            ASSERT_FALSE( access.IsValid() );
            ASSERT_TRUE( access.GetSize() == 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A named file keeps working with the 64 bit capable positioning" )
        try
        {
            CORE::CFileAccess access( filePath, "rb" );
            ASSERT_TRUE( access.Opened() );
            ASSERT_TRUE( access.GetSize() == payloadSize );
            ASSERT_TRUE( 0 == access.Setpos( 20 ) );
            ASSERT_TRUE( access.Tell() == 20 );
            char c = 0;
            ASSERT_TRUE( access.Read( &c, 1, 1 ) == 1 );
            ASSERT_TRUE( c == payload[ 20 ] );
            ASSERT_TRUE( 0 == access.Seek( 2, SEEK_CUR ) );
            ASSERT_TRUE( access.Tell() == 23 );
            CORE::CICloneable* clone = access.Clone();
            ASSERT_FALSE( GUCEF_NULL == clone );
            delete clone;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: GetSize gives the size of the open file, also after its path was given to another file, and counts what was written" )
        try
        {
            CORE::CString heldPath = CORE::CombinePath( dir, "held.bin" );
            CORE::CString otherPath = CORE::CombinePath( dir, "other.bin" );
            {
                CORE::CFileAccess writer( heldPath, "wb" );
                ASSERT_TRUE( payloadSize == writer.Write( payload, 1, payloadSize ) );
                ASSERT_TRUE( payloadSize == writer.GetSize() );     // written, not yet flushed
                ASSERT_TRUE( 1 == writer.Write( payload, 1, 1 ) && payloadSize + 1 == writer.GetSize() );
            }
            {
                CORE::CFileAccess other( otherPath, "wb" );
                ASSERT_TRUE( 5 == other.Write( payload, 1, 5 ) );
            }

            CORE::CFileAccess reader( heldPath, "rb" );
            ASSERT_TRUE( reader.Opened() && payloadSize + 1 == reader.GetSize() );
            ASSERT_TRUE( 0 == reader.Setpos( 3 ) );
            #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
            // POSIX: the path now names another file of 5 bytes, the open file is the one of payloadSize + 1 bytes
            ASSERT_TRUE( CORE::MoveFile( otherPath, heldPath, true ) );
            ASSERT_TRUE( 5 == CORE::FileSize( heldPath ) );
            ASSERT_TRUE( payloadSize + 1 == reader.GetSize() );
            ASSERT_TRUE( 3 == reader.Tell() );      // asking the size does not move the position
            #endif
            reader.Close();
            ASSERT_TRUE( 0 == reader.GetSize() );
            CORE::DeleteFile( heldPath );
            CORE::DeleteFile( otherPath );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A memory block is read through the memory access classes" )
        try
        {
            CORE::CMFileAccess memAccess( payload, payloadSize );
            ASSERT_TRUE( memAccess.GetSize() == payloadSize );
            char buffer[ 16 ];
            ASSERT_TRUE( 0 == memAccess.Setpos( 10 ) );
            ASSERT_TRUE( memAccess.Read( buffer, 1, 8 ) == 8 );
            ASSERT_TRUE( 0 == memcmp( buffer, payload + 10, 8 ) );
            ASSERT_TRUE( memAccess.Tell() == 18 );

            // owning memory: a dynamic buffer that lives and dies with the access object
            CORE::CDynamicBuffer* owned = GUCEF_NEW CORE::CDynamicBuffer();
            owned->CopyFrom( payloadSize, payload );
            CORE::CDynamicBufferAccess ownedAccess( owned, true );
            ASSERT_TRUE( ownedAccess.GetSize() == payloadSize );
            ASSERT_TRUE( ownedAccess.Read( buffer, 1, 4 ) == 4 );
            ASSERT_TRUE( 0 == memcmp( buffer, payload, 4 ) );

            // non owning view on external memory
            CORE::CDynamicBufferAccess linkedAccess;
            linkedAccess.LinkTo( payload, payloadSize );
            ASSERT_TRUE( linkedAccess.GetSize() == payloadSize );
            ASSERT_TRUE( linkedAccess.Read( buffer, 1, 3 ) == 3 );
            ASSERT_TRUE( 0 == memcmp( buffer, payload, 3 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Positioning a dynamic buffer access behaves as fsetpos() and fseek() on a file" )
        try
        {
            CORE::CDynamicBufferAccess access;
            access.LinkTo( payload, payloadSize );
            ASSERT_TRUE( 0 == access.Setpos( 10 ) );                    // 0 on success, it used to return the new position
            ASSERT_TRUE( 10 == access.Tell() );
            ASSERT_TRUE( 0 == access.Setpos( payloadSize ) );
            ASSERT_TRUE( payloadSize == access.Tell() );

            // beyond the end is a valid position, there is just nothing to read
            char buffer[ 8 ];
            ASSERT_TRUE( 0 == access.Setpos( payloadSize + 5 ) );
            ASSERT_TRUE( payloadSize + 5 == access.Tell() );
            ASSERT_TRUE( access.Eof() );
            ASSERT_TRUE( 0 == access.Read( buffer, 1, 4 ) );
            ASSERT_TRUE( 0 == access.GetChar() );
            ASSERT_TRUE( access.ReadLine().IsNULLOrEmpty() );
            ASSERT_TRUE( payloadSize + 5 == access.Tell() );            // reading there does not move it back

            // fseek() alike: relative to the start, the position and the end, never before the start
            ASSERT_TRUE( 0 == access.Seek( 3, SEEK_SET ) && 3 == access.Tell() );
            ASSERT_TRUE( 0 == access.Seek( 2, SEEK_CUR ) && 5 == access.Tell() );
            ASSERT_TRUE( 0 == access.Seek( -2, SEEK_END ) && payloadSize - 2 == access.Tell() );
            ASSERT_TRUE( 0 == access.Seek( 4, SEEK_END ) && payloadSize + 4 == access.Tell() );
            ASSERT_TRUE( 0 != access.Seek( -1, SEEK_SET ) );
            ASSERT_TRUE( payloadSize + 4 == access.Tell() );            // a failed seek leaves the position alone

            // the C view hands out the same
            ASSERT_TRUE( 0 == access.CStyleAccess()->setpos( access.CStyleAccess(), 7 ) );
            ASSERT_TRUE( 7 == access.Tell() );

            // writing beyond the data fills the gap with zeros, as a file does
            CORE::CDynamicBuffer data;
            CORE::CDynamicBufferAccess writer( data );
            ASSERT_TRUE( 3 == writer.Write( "abc", 1, 3 ) );
            ASSERT_TRUE( 0 == writer.Setpos( 600 ) );
            ASSERT_TRUE( 2 == writer.Write( "yz", 1, 2 ) );
            ASSERT_TRUE( 602 == data.GetDataSize() );
            const char* bytes = static_cast< const char* >( data.GetConstBufferPtr() );
            bool gapIsZero = true;
            for ( UInt32 i=3; i<600; ++i )
                gapIsZero = gapIsZero && 0 == bytes[ i ];
            ASSERT_TRUE( gapIsZero );
            ASSERT_TRUE( 0 == memcmp( bytes, "abc", 3 ) && 0 == memcmp( bytes + 600, "yz", 2 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A window on another IO access reads only its own range" )
        try
        {
            CORE::IOAccessPtr parent( GUCEF_NEW CORE::CMFileAccess( payload, payloadSize ) );
            CORE::CSubIOAccess window( parent, 10, 8 );
            ASSERT_TRUE( window.Opened() );
            ASSERT_TRUE( window.IsValid() );
            ASSERT_TRUE( window.IsReadable() );
            ASSERT_FALSE( window.IsWriteable() );
            ASSERT_TRUE( window.GetSize() == 8 );
            ASSERT_TRUE( window.GetOffset() == 10 );
            ASSERT_TRUE( window.Tell() == 0 );

            char buffer[ 32 ];
            memset( buffer, 0, sizeof( buffer ) );
            ASSERT_TRUE( window.Read( buffer, 1, 4 ) == 4 );
            ASSERT_TRUE( 0 == memcmp( buffer, payload + 10, 4 ) );
            ASSERT_TRUE( window.Tell() == 4 );
            ASSERT_FALSE( window.Eof() );

            // asking for more than is left returns what is left, never data outside of the window
            ASSERT_TRUE( window.Read( buffer, 1, 20 ) == 4 );
            ASSERT_TRUE( 0 == memcmp( buffer, payload + 14, 4 ) );
            ASSERT_TRUE( window.Eof() );
            ASSERT_TRUE( window.Read( buffer, 1, 1 ) == 0 );

            // partial elements are not returned
            ASSERT_TRUE( 0 == window.Setpos( 5 ) );
            ASSERT_TRUE( window.Read( buffer, 2, 5 ) == 1 );
            ASSERT_TRUE( window.Tell() == 7 );

            // seeking
            ASSERT_TRUE( 0 == window.Seek( 2, SEEK_SET ) );
            ASSERT_TRUE( window.GetChar() == payload[ 12 ] );
            ASSERT_TRUE( 0 == window.Seek( 2, SEEK_CUR ) );
            ASSERT_TRUE( window.GetChar() == payload[ 15 ] );
            ASSERT_TRUE( 0 == window.Seek( -1, SEEK_END ) );
            ASSERT_TRUE( window.GetChar() == payload[ 17 ] );
            // as fseek() and fsetpos(): beyond the end is a position, with nothing to read; before the start fails
            ASSERT_TRUE( 0 == window.Seek( 100, SEEK_SET ) );
            ASSERT_TRUE( window.Tell() == 100 && window.Eof() );
            ASSERT_TRUE( window.Read( buffer, 1, 1 ) == 0 );
            ASSERT_TRUE( 0 == window.Setpos( 9 ) );
            ASSERT_TRUE( window.Tell() == 9 );
            ASSERT_TRUE( window.Read( buffer, 1, 1 ) == 0 );
            ASSERT_TRUE( 0 != window.Seek( -100, SEEK_CUR ) );
            ASSERT_TRUE( window.Tell() == 9 ); // failed seeks leave the position alone

            ASSERT_TRUE( window.Write( buffer, 1, 1 ) == 0 );

            window.Close();
            ASSERT_FALSE( window.Opened() );
            ASSERT_FALSE( window.IsValid() );
            ASSERT_TRUE( window.Read( buffer, 1, 1 ) == 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Windows sharing one parent keep their own position and nest" )
        try
        {
            CORE::IOAccessPtr parent( GUCEF_NEW CORE::CMFileAccess( payload, payloadSize ) );
            CORE::CSubIOAccess a( parent, 0, 10 );
            CORE::CSubIOAccess b( parent, 10, 10 );
            char x = 0, y = 0;
            ASSERT_TRUE( a.Read( &x, 1, 1 ) == 1 );
            ASSERT_TRUE( b.Read( &y, 1, 1 ) == 1 );
            ASSERT_TRUE( x == payload[ 0 ] );
            ASSERT_TRUE( y == payload[ 10 ] );
            ASSERT_TRUE( a.Read( &x, 1, 1 ) == 1 );
            ASSERT_TRUE( b.Read( &y, 1, 1 ) == 1 );
            ASSERT_TRUE( x == payload[ 1 ] );
            ASSERT_TRUE( y == payload[ 11 ] );

            // a window in a window
            CORE::IOAccessPtr outer( GUCEF_NEW CORE::CSubIOAccess( parent, 5, 20 ) );
            CORE::CSubIOAccess inner( outer, 5, 5 );
            char buffer[ 8 ];
            ASSERT_TRUE( inner.Read( buffer, 1, 5 ) == 5 );
            ASSERT_TRUE( 0 == memcmp( buffer, payload + 10, 5 ) );

            // a clone has its own position
            CORE::CICloneable* cloneBase = a.Clone();
            ASSERT_FALSE( GUCEF_NULL == cloneBase );
            CORE::CSubIOAccess* clone = static_cast< CORE::CSubIOAccess* >( cloneBase );
            ASSERT_TRUE( clone->Tell() == 0 );
            ASSERT_TRUE( clone->GetSize() == 10 );
            delete clone;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A window that does not fit its parent is refused" )
        try
        {
            CORE::IOAccessPtr parent( GUCEF_NEW CORE::CMFileAccess( payload, payloadSize ) );
            CORE::CSubIOAccess window;
            ASSERT_FALSE( window.Load( parent, payloadSize - 2, 10 ) );
            ASSERT_FALSE( window.Load( parent, payloadSize + 1, 0 ) );
            ASSERT_FALSE( window.Load( parent, 0xFFFFFFFFFFFFFFF0ULL, 0x20 ) ); // would overflow when added
            ASSERT_FALSE( window.Load( CORE::IOAccessPtr(), 0, 1 ) );
            ASSERT_FALSE( window.Opened() );
            ASSERT_TRUE( window.Load( parent, payloadSize - 2, 2 ) ); // exactly up to the end is fine
            ASSERT_TRUE( window.Load( parent, payloadSize, 0 ) );     // empty window at the end is fine
            ASSERT_TRUE( window.Eof() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::DeleteFile( filePath );
}

/*-------------------------------------------------------------------------*/
