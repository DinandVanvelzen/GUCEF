/*
 *  pluginglueOGRE_TestApp: Test application for the Ogre glue modules (pluginglueOGRE, OgrePlugin_GucefVFS, OgreCodec_GucefIMAGE)
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
#include <stdio.h>

#include "OgreDataStream.h"

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_CIOACCESSTOOGREDATASTREAM_H
#include "pluginglueOGRECOMMON_CIOAccessToOgreDataStream.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_CIOACCESSTOOGREDATASTREAM_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_CIOACCESSTOOGREDATASTREAM_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_CVFSRESOURCETOOGREDATASTREAM_H
#include "pluginglueOGRECOMMON_CVfsResourceToOgreDataStream.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_CVFSRESOURCETOOGREDATASTREAM_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_CVFSRESOURCETOOGREDATASTREAM_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREDATASTREAMTOIOACCESS_H
#include "pluginglueOGRECOMMON_COgreDataStreamToIOAccess.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREDATASTREAMTOIOACCESS_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREDATASTREAMTOIOACCESS_H ? */

#include "TestSupport.h"
#include "TestStreamAdapters.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::OGRECOMMON;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/**
 *  An IO access object of 6 GiB that has no data: reads give zeros up to the size. It is there to see that the
 *  positions of the adapters are 64 bit all the way
 */
class CSparseAccess : public CORE::CIOAccess
{
    public:

    CSparseAccess( UInt64 size ) : CORE::CIOAccess(), m_size( size ), m_pos( 0 ) {}
    virtual ~CSparseAccess() GUCEF_VIRTUAL_OVERRIDE {}

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt64 wanted = (UInt64) esize * elements;
        const UInt64 available = m_size > m_pos ? m_size - m_pos : 0;
        const UInt64 count = wanted < available ? wanted : available;
        memset( dest, 0, (size_t) count );
        m_pos += count;
        return 0 == esize ? 0 : (UInt32) ( count / esize );
    }

    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_pos; }

    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE
    {
        Int64 base = SEEK_SET == origin ? 0 : ( SEEK_CUR == origin ? (Int64) m_pos : (Int64) m_size );
        if ( base + offset < 0 )
            return -1;
        m_pos = (UInt64) ( base + offset );
        return 0;
    }

    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_pos >= m_size; }
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_size; }
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CSparseAccess( m_size ); }

    private:

    UInt64 m_size;
    UInt64 m_pos;
};

/*-------------------------------------------------------------------------*/

/** Data with a pattern that tells every position apart: the byte at position i is i modulo 251 */
static void
FillPattern( CORE::CDynamicBuffer& buffer, UInt32 size )
{GUCEF_TRACE;

    buffer.SetDataSize( size );
    UInt8* data = static_cast< UInt8* >( buffer.GetBufferPtr() );
    for ( UInt32 i=0; i<size; ++i )
        data[ i ] = (UInt8) ( i % 251 );
}

} // anonymous namespace

/*-------------------------------------------------------------------------*/

static void
TestReadSeekAndSize( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Stream adapters: a CIOAccess as Ogre data stream reads, tells, seeks, skips and knows its size and end" )
        try
        {
            CORE::CDynamicBuffer buffer;
            FillPattern( buffer, 1000 );
            CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( buffer ) );
            CIOAccessToOgreDataStream stream( access, "pattern.bin" );

            ASSERT_TRUE( "pattern.bin" == stream.getName() );
            ASSERT_TRUE( stream.isReadable() );
            ASSERT_TRUE( 1000 == stream.size() );
            ASSERT_TRUE( 0 == stream.tell() );
            ASSERT_FALSE( stream.eof() );

            UInt8 bytes[ 20 ];
            ASSERT_TRUE( 10 == stream.read( bytes, 10 ) );
            ASSERT_TRUE( 10 == stream.tell() );
            ASSERT_TRUE( 0 == bytes[ 0 ] && 9 == bytes[ 9 ] );

            stream.seek( 500 );
            ASSERT_TRUE( 500 == stream.tell() );
            ASSERT_TRUE( 20 == stream.read( bytes, 20 ) );
            ASSERT_TRUE( (UInt8) ( 500 % 251 ) == bytes[ 0 ] );
            ASSERT_TRUE( (UInt8) ( 519 % 251 ) == bytes[ 19 ] );

            stream.skip( -20 );
            ASSERT_TRUE( 500 == stream.tell() );
            stream.skip( 100 );
            ASSERT_TRUE( 600 == stream.tell() );
            // never before the start, never past the end
            stream.skip( -10000 );
            ASSERT_TRUE( 0 == stream.tell() );
            stream.skip( 10000 );
            ASSERT_TRUE( 1000 == stream.tell() );
            ASSERT_TRUE( stream.eof() );
            ASSERT_TRUE( 0 == stream.read( bytes, 10 ) );

            // a read at the end that asks for more gives what is there
            stream.seek( 995 );
            ASSERT_TRUE( 5 == stream.read( bytes, 10 ) );
            ASSERT_TRUE( stream.eof() );

            stream.close();
            ASSERT_TRUE( 0 == stream.read( bytes, 10 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestLines( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Stream adapters: readLine, getLine and getAsString of Ogre work on the adapter: delimiters and end of the data" )
        try
        {
            const char* text = "first\nsecond line\r\n\nlast without newline";
            CORE::CDynamicBuffer buffer( text, (UInt32) strlen( text ), false );
            CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( buffer ) );
            CIOAccessToOgreDataStream stream( access, "lines.txt" );

            char line[ 128 ];
            ASSERT_TRUE( 5 == stream.readLine( line, sizeof( line ) - 1, "\n" ) );
            ASSERT_TRUE( 0 == strcmp( line, "first" ) );
            // the delimiter is consumed, the carriage return is part of the data until it is trimmed
            ASSERT_TRUE( "second line" == stream.getLine( true ) );
            ASSERT_TRUE( stream.getLine( false ).empty() );
            ASSERT_FALSE( stream.eof() );
            ASSERT_TRUE( "last without newline" == stream.getLine( true ) );
            ASSERT_TRUE( stream.eof() );

            stream.seek( 0 );
            ASSERT_TRUE( Ogre::String( text ) == stream.getAsString() );

            // skipping a line goes to the first position after its delimiter
            stream.seek( 0 );
            ASSERT_TRUE( stream.skipLine( "\n" ) >= 5 );
            ASSERT_TRUE( 6 == stream.tell() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestWrite( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Stream adapters: writing through the adapter changes the IO access object and the size follows, a read only access refuses" )
        try
        {
            CORE::CDynamicBuffer buffer( true );
            CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( buffer ) );
            CIOAccessToOgreDataStream stream( access, "out.bin" );
            ASSERT_TRUE( stream.isWriteable() );
            ASSERT_TRUE( 0 == stream.size() );

            ASSERT_TRUE( 6 == stream.write( "abcdef", 6 ) );
            ASSERT_TRUE( 6 == stream.size() );
            ASSERT_TRUE( 6 == stream.tell() );
            ASSERT_TRUE( 6 == buffer.GetDataSize() );
            ASSERT_TRUE( 0 == memcmp( buffer.GetConstBufferPtr(), "abcdef", 6 ) );

            stream.seek( 3 );
            ASSERT_TRUE( 3 == stream.write( "XYZ", 3 ) );
            ASSERT_TRUE( 0 == memcmp( buffer.GetConstBufferPtr(), "abcXYZ", 6 ) );

            // a stream of a read only IO access object cannot write
            const CORE::CDynamicBuffer& constBuffer = buffer;
            CORE::IOAccessPtr readOnlyAccess( GUCEF_NEW CORE::CDynamicBufferAccess( constBuffer ) );
            CIOAccessToOgreDataStream readOnlyStream( readOnlyAccess, "ro.bin" );
            ASSERT_FALSE( readOnlyStream.isWriteable() );
            ASSERT_TRUE( 0 == readOnlyStream.write( "no", 2 ) );
            ASSERT_TRUE( 0 == memcmp( buffer.GetConstBufferPtr(), "abcXYZ", 6 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSixtyFourBitPositions( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Stream adapters: positions and sizes beyond 4 GiB are kept in both directions" )
        try
        {
            const UInt64 sixGiB = (UInt64) 6 * 1024 * 1024 * 1024;
            CSparseAccess sparse( sixGiB );

            // Ogre -> its stream interface is size_t: on 64 bit builds all of it fits
            CIOAccessToOgreDataStream stream( sparse, "sparse.bin" );
            if ( sizeof( size_t ) >= 8 )
            {
                ASSERT_TRUE( (size_t) sixGiB == stream.size() );
                const size_t position = (size_t) ( (UInt64) 5 * 1024 * 1024 * 1024 + 3 );
                stream.seek( position );
                ASSERT_TRUE( position == stream.tell() );
                ASSERT_TRUE( position == (size_t) sparse.Tell() );
                UInt8 bytes[ 8 ];
                ASSERT_TRUE( 8 == stream.read( bytes, 8 ) );
                ASSERT_TRUE( position + 8 == stream.tell() );
                stream.skip( -8 );
                ASSERT_TRUE( position == stream.tell() );
                stream.seek( (size_t) sixGiB - 2 );
                ASSERT_FALSE( stream.eof() );
                ASSERT_TRUE( 2 == stream.read( bytes, 8 ) );
                ASSERT_TRUE( stream.eof() );
            }
            else
            {
                // a size that does not fit is "not known", which Ogre allows
                ASSERT_TRUE( 0 == stream.size() );
            }

            // and back: an IO access object over an Ogre stream reports 64 bit positions as it was given
            Ogre::MemoryDataStreamPtr memory = std::make_shared< Ogre::MemoryDataStream >( (size_t) 100 );
            COgreDataStreamToIOAccess access( memory );
            ASSERT_TRUE( 100 == access.GetSize() );
            ASSERT_TRUE( 0 == access.Setpos( 60 ) );
            ASSERT_TRUE( 60 == access.Tell() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSetposFollowsFsetpos( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Stream adapters: Seek and Setpos of the IO access over an Ogre stream return 0 on success as fsetpos does, and an error for places that are not in the stream" )
        try
        {
            Ogre::MemoryDataStreamPtr memory = std::make_shared< Ogre::MemoryDataStream >( (size_t) 100 );
            UInt8* data = static_cast< UInt8* >( memory->getPtr() );
            for ( int i=0; i<100; ++i )
                data[ i ] = (UInt8) i;

            COgreDataStreamToIOAccess access( memory );
            ASSERT_TRUE( access.IsValid() && access.Opened() );
            ASSERT_TRUE( access.IsReadable() );

            ASSERT_TRUE( 0 == access.Setpos( 0 ) );
            ASSERT_TRUE( 0 == access.Setpos( 100 ) );
            ASSERT_TRUE( access.Eof() );
            ASSERT_TRUE( 0 != access.Setpos( 101 ) );
            ASSERT_TRUE( 100 == access.Tell() );
            ASSERT_TRUE( 0 != access.Seek( -1, SEEK_SET ) );
            ASSERT_TRUE( 0 == access.Seek( -10, SEEK_END ) );
            ASSERT_TRUE( 90 == access.Tell() );
            ASSERT_TRUE( 0 == access.Seek( 5, SEEK_CUR ) );
            ASSERT_TRUE( 95 == access.Tell() );
            ASSERT_TRUE( 0 != access.Seek( 6, SEEK_CUR ) );
            ASSERT_TRUE( 0 != access.Seek( 0, 12345 ) );

            // elements are counted as fread() does
            access.Setpos( 0 );
            UInt16 pairs[ 4 ];
            ASSERT_TRUE( 4 == access.Read( pairs, 2, 4 ) );
            ASSERT_TRUE( 8 == access.Tell() );
            ASSERT_TRUE( 8 == access.GetChar() );
            ASSERT_TRUE( 9 == access.Tell() );

            // the data is the data of the stream
            access.Setpos( 50 );
            UInt8 one = 0;
            ASSERT_TRUE( 1 == access.Read( &one, 1, 1 ) );
            ASSERT_TRUE( 50 == one );

            access.Close();
            ASSERT_FALSE( access.IsValid() );
            ASSERT_TRUE( 0 == access.Read( &one, 1, 1 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRoundTrips( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Stream adapters: data survives a trip CIOAccess -> Ogre stream -> CIOAccess -> Ogre stream" )
        try
        {
            CORE::CDynamicBuffer buffer;
            FillPattern( buffer, 4096 );
            CORE::IOAccessPtr origin( GUCEF_NEW CORE::CDynamicBufferAccess( buffer ) );

            Ogre::DataStreamPtr first( new CIOAccessToOgreDataStream( origin, "first" ) );
            CORE::IOAccessPtr middle( GUCEF_NEW COgreDataStreamToIOAccess( first ) );
            Ogre::DataStreamPtr second( new CIOAccessToOgreDataStream( middle, "second" ) );

            ASSERT_TRUE( 4096 == second->size() );
            GUCEF::vector< UInt8 > copy( 4096 );
            ASSERT_TRUE( 4096 == second->read( &copy[ 0 ], 4096 ) );
            ASSERT_TRUE( 0 == memcmp( &copy[ 0 ], buffer.GetConstBufferPtr(), 4096 ) );

            // moving around works through both layers
            second->seek( 1000 );
            ASSERT_TRUE( 1000 == first->tell() );
            ASSERT_TRUE( 1000 == origin->Tell() );
            second->skip( 24 );
            ASSERT_TRUE( 1024 == origin->Tell() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestVfsResourceKeepsHandle( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Stream adapters: the stream of a VFS resource keeps the resource alive and gives the content of the file" )
        try
        {
            const CORE::CString dir = MakeCleanTestDir( "streams_vfs" );
            ASSERT_TRUE( WriteTestFile( dir + "/hello.txt", "hello from the vfs" ) );

            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            ASSERT_TRUE( vfs.AddRoot( dir, "ogre_streams_vfs", false, false, "/ogre_streams_vfs" ) );

            Ogre::DataStreamPtr stream;
            {
                VFS::TBasicVfsResourcePtr resource = vfs.GetFile( "/ogre_streams_vfs/hello.txt", "rb" );
                ASSERT_FALSE( resource.IsNULL() );
                if ( !resource.IsNULL() )
                    stream.reset( new CVfsResourceToOgreDataStream( resource ) );
            }
            // the resource of this scope is gone, the stream still has it
            ASSERT_TRUE( (bool) stream );
            if ( stream )
            {
                ASSERT_TRUE( stream->getName().size() >= 9 && 0 == stream->getName().compare( stream->getName().size() - 9, 9, "hello.txt" ) );
                ASSERT_FALSE( stream->isWriteable() );
                ASSERT_TRUE( 18 == stream->size() );
                ASSERT_TRUE( "hello from the vfs" == stream->getAsString() );
            }

            ASSERT_TRUE( vfs.UnmountArchiveByName( "ogre_streams_vfs" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformStreamAdapterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "pluginglueOGRE.StreamAdapters" );

    TestReadSeekAndSize();
    TestLines();
    TestWrite();
    TestSixtyFourBitPositions();
    TestSetposFollowsFsetpos();
    TestRoundTrips();
    TestVfsResourceKeepsHandle();
}

/*-------------------------------------------------------------------------*/
