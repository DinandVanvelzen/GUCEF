/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDynamicBuffer.h"

using namespace GUCEF;

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

/** Appends count bytes one at a time, returns how often the buffer size changed */
UInt32
AppendBytesOneByOne( CORE::CDynamicBuffer& buffer, UInt32 count )
{
    UInt32 sizeChanges = 0;
    UInt32 lastBufferSize = buffer.GetBufferSize();
    for ( UInt32 i=0; i<count; ++i )
    {
        UInt8 value = (UInt8) ( i % 251 );
        if ( 1 != buffer.Append( &value, 1 ) )
            break;
        if ( buffer.GetBufferSize() != lastBufferSize )
        {
            ++sizeChanges;
            lastBufferSize = buffer.GetBufferSize();
        }
    }
    return sizeChanges;
}

bool
HasPattern( const CORE::CDynamicBuffer& buffer, UInt32 count )
{
    if ( buffer.GetDataSize() != count )
        return false;
    const UInt8* bytes = static_cast< const UInt8* >( buffer.GetConstBufferPtr() );
    for ( UInt32 i=0; i<count; ++i )
        if ( bytes[ i ] != (UInt8) ( i % 251 ) )
            return false;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDynamicBufferTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DynamicBuffer TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "DynamicBuffer" );

    GUCEF_TESTFW_TESTCASE( "DynamicBuffer: Append does not grow a buffer whose auto enlarge is off" )
    try
    {
        CORE::CDynamicBuffer buffer( 4, false );
        ASSERT_TRUE( 4 == buffer.Append( "abcd", 4 ) );
        ASSERT_TRUE( 0 == buffer.Append( "e", 1 ) );
        ASSERT_TRUE( 4 == buffer.GetDataSize() );
        ASSERT_TRUE( 4 == buffer.GetBufferSize() );
        ASSERT_TRUE( 0 == buffer.Append( "e", 1, false ) );         // appending to the buffer rather than the data as well
        ASSERT_TRUE( 4 == buffer.GetBufferSize() );

        buffer.SetAutoEnlarge( true );
        ASSERT_TRUE( 1 == buffer.Append( "e", 1 ) );
        ASSERT_TRUE( 5 == buffer.GetDataSize() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DynamicBuffer: The default growth is exact" )
    try
    {
        CORE::CDynamicBuffer buffer;
        ASSERT_TRUE( CORE::CDynamicBuffer::GROWTHSTRATEGY_EXACT == buffer.GetGrowthStrategy() );
        AppendBytesOneByOne( buffer, 1000 );
        ASSERT_TRUE( HasPattern( buffer, 1000 ) );
        ASSERT_TRUE( 1000 == buffer.GetBufferSize() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DynamicBuffer: Geometric growth makes byte by byte appends grow the buffer rarely" )
    try
    {
        CORE::CDynamicBuffer buffer;
        buffer.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
        const UInt32 count = 1024 * 1024;
        UInt32 sizeChanges = AppendBytesOneByOne( buffer, count );
        ASSERT_TRUE( HasPattern( buffer, count ) );
        ASSERT_TRUE( sizeChanges <= 21 );                           // doubling from 1 byte reaches 1 MiB in 21 steps
        ASSERT_TRUE( buffer.GetBufferSize() >= count && buffer.GetBufferSize() <= 2 * count );

        CORE::CDynamicBuffer slower;
        slower.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC, 50 );
        ASSERT_TRUE( 50 == slower.GetGrowthStrategyParam() );
        slower.SetBufferSize( 100 );
        AppendBytesOneByOne( slower, 101 );
        ASSERT_TRUE( 150 == slower.GetBufferSize() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DynamicBuffer: Block growth grows in multiples of the block size" )
    try
    {
        CORE::CDynamicBuffer buffer;
        buffer.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_BLOCKS, 64 );
        ASSERT_TRUE( 1 == buffer.Append( "x", 1 ) );
        ASSERT_TRUE( 64 == buffer.GetBufferSize() );
        AppendBytesOneByOne( buffer, 64 );
        ASSERT_TRUE( 65 == buffer.GetDataSize() );
        ASSERT_TRUE( 128 == buffer.GetBufferSize() );
        ASSERT_TRUE( 3 == buffer.CopyFrom( 200, 3, "abc" ) );       // CopyFrom grows the same way
        ASSERT_TRUE( 256 == buffer.GetBufferSize() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DynamicBuffer: A buffer access grows its buffer with its own strategy, leaving the buffer's setting as it is" )
    try
    {
        CORE::CDynamicBuffer buffer;
        UInt32 sizeChanges = 0;
        {
            CORE::CDynamicBufferAccess access( buffer, CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
            ASSERT_TRUE( access.HasOwnGrowthStrategy() );
            UInt32 lastBufferSize = buffer.GetBufferSize();
            for ( UInt32 i=0; i<100000; ++i )
            {
                const UInt8 value = (UInt8) ( i % 251 );
                ASSERT_TRUE( 1 == access.Write( &value, 1, 1 ) );
                if ( buffer.GetBufferSize() != lastBufferSize )
                {
                    ++sizeChanges;
                    lastBufferSize = buffer.GetBufferSize();
                }
            }
        }
        ASSERT_TRUE( HasPattern( buffer, 100000 ) );
        ASSERT_TRUE( sizeChanges <= 18 );                                               // doubling from 1 byte passes 100000 at the 18th size
        ASSERT_TRUE( CORE::CDynamicBuffer::GROWTHSTRATEGY_EXACT == buffer.GetGrowthStrategy() );

        // without a strategy of its own the access grows the buffer the buffer's way
        CORE::CDynamicBuffer exact;
        CORE::CDynamicBufferAccess exactAccess( exact );
        ASSERT_FALSE( exactAccess.HasOwnGrowthStrategy() );
        ASSERT_TRUE( 3 == exactAccess.Write( "abc", 1, 3 ) );
        ASSERT_TRUE( 3 == exact.GetBufferSize() );

        exactAccess.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_BLOCKS, 16 );
        ASSERT_TRUE( 1 == exactAccess.Write( "d", 1, 1 ) );
        ASSERT_TRUE( 16 == exact.GetBufferSize() );
        exactAccess.UseBufferGrowthStrategy();
        ASSERT_FALSE( exactAccess.HasOwnGrowthStrategy() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DynamicBuffer: Explicit sizing stays exact and the strategy travels with copies, moves and swaps" )
    try
    {
        CORE::CDynamicBuffer buffer;
        buffer.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
        buffer.SetDataSize( 10 );
        ASSERT_TRUE( 10 == buffer.GetBufferSize() );

        CORE::CDynamicBuffer copy( buffer );
        ASSERT_TRUE( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC == copy.GetGrowthStrategy() );

        CORE::CDynamicBuffer other;
        other.Swap( buffer );
        ASSERT_TRUE( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC == other.GetGrowthStrategy() );
        ASSERT_TRUE( CORE::CDynamicBuffer::GROWTHSTRATEGY_EXACT == buffer.GetGrowthStrategy() );

        #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
        CORE::CDynamicBuffer moved( GUCEF_MOVE( other ) );
        ASSERT_TRUE( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC == moved.GetGrowthStrategy() );
        ASSERT_TRUE( CORE::CDynamicBuffer::GROWTHSTRATEGY_EXACT == other.GetGrowthStrategy() );
        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DynamicBuffer: Text without a terminating null becomes a string of exactly the data, nothing beyond it" )
    try
    {
        // the bytes after the data are there in memory, they must not end up in the string
        CORE::CDynamicBuffer buffer;
        buffer.Append( "abcX", 4 );
        buffer.SetDataSize( 3 );
        CORE::CString text = CORE::ToString( buffer );
        ASSERT_TRUE( text == "abc" );
        ASSERT_TRUE( 3 == text.Length() );

        // text with its null terminator in the data keeps working
        CORE::CDynamicBuffer terminated;
        terminated.Append( "abc", 4 );
        ASSERT_TRUE( CORE::ToString( terminated ) == "abc" );

        // a sized string of exactly the text
        CORE::CString sized( "defY", 3, true );
        ASSERT_TRUE( sized == "def" && 3 == sized.Length() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DynamicBuffer: Downshift moves the data to the front and reads nothing past the data" )
    try
    {
        // 10 bytes of data in a buffer of 16, the 6 bytes after the data are sentinels
        CORE::CDynamicBuffer buffer( 16, false );
        buffer.CopyFrom( 16, "0123456789######" );
        buffer.SetDataSize( 10 );
        ASSERT_TRUE( 16 == buffer.GetBufferSize() );

        ASSERT_TRUE( buffer.Downshift( 3 ) );
        ASSERT_TRUE( 7 == buffer.GetDataSize() );
        ASSERT_TRUE( 0 == memcmp( buffer.GetConstBufferPtr(), "3456789", 7 ) );

        // What was after the data was not read: no sentinel moved into the bytes the data left
        const char* raw = buffer.AsConstTypePtr< char >();
        for ( UInt32 i=7; i<10; ++i )
            ASSERT_TRUE( '#' != raw[ i ] );
        ASSERT_TRUE( 0 == memcmp( raw + 10, "######", 6 ) );

        // A shift larger than half the data, and one of all of it
        ASSERT_TRUE( buffer.Downshift( 5 ) );
        ASSERT_TRUE( 2 == buffer.GetDataSize() && 0 == memcmp( buffer.GetConstBufferPtr(), "89", 2 ) );
        ASSERT_TRUE( buffer.Downshift( 2 ) );
        ASSERT_TRUE( 0 == buffer.GetDataSize() );
        ASSERT_FALSE( buffer.Downshift( 1 ) );

        // Data that fills the buffer exactly: reading past it would leave the allocation
        CORE::CDynamicBuffer full( 8, false );
        full.CopyFrom( 8, "abcdefgh" );
        ASSERT_TRUE( 8 == full.GetDataSize() && 8 == full.GetBufferSize() );
        ASSERT_TRUE( full.Downshift( 3 ) );
        ASSERT_TRUE( 5 == full.GetDataSize() && 0 == memcmp( full.GetConstBufferPtr(), "defgh", 5 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
