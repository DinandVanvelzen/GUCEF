/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFERSWAP_H
#include "gucefCORE_CDynamicBufferSwap.h"
#define GUCEF_CORE_CDYNAMICBUFFERSWAP_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERSWAP_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_VIEW_H
#include "gucefCORE_CDynamicBufferView.h"
#define GUCEF_CORE_CDYNAMICBUFFER_VIEW_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_VIEW_H ? */

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

#include "TestDynamicBufferSwap.h"

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

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

void
PerformDynamicBufferSwapTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CDynamicBufferSwap TESTS" );
    
    GUCEF_TESTFW_SUITE_SCOPE( "CDynamicBufferSwap" );

    GUCEF_TESTFW_TESTCASE( "Test 1: Buffer swap read/write operations" )
        try
        {       
            // Check basic setup
            
            CORE::CDynamicBufferSwap buffers( 3 );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        ASSERT_TRUE( 0 == buffers.GetBuffersQueuedToRead() );
        ASSERT_TRUE( CORE::CDateTime::Empty == buffers.GetCurrenReaderBufferAssociatedDt() );
        ASSERT_TRUE( CORE::CDateTime::Empty == buffers.GetCurrenWriterBufferAssociatedDt() );
        ASSERT_TRUE( -1 == buffers.GetCurrentWriterBufferIndex() );
        ASSERT_TRUE( -1 == buffers.GetCurrentReaderBufferIndex() );

        // Now do basic buffer sizing operations
        
        UInt32 smallest = 0;
        UInt32 largest = 0;
        buffers.GetBufferSizeRange( smallest, largest );
        ASSERT_TRUE( 0 == smallest );
        ASSERT_TRUE( 0 == largest );
        
        buffers.SetMinimalBufferSize( 1000 );        
        smallest = 0;
        largest = 0;
        buffers.GetBufferSizeRange( smallest, largest );
        ASSERT_TRUE( 1000 == smallest );
        ASSERT_TRUE( 1000 == largest );

        buffers.SetMinimalBufferSize( 10000 );        
        smallest = 0;
        largest = 0;
        buffers.GetBufferSizeRange( smallest, largest );
        ASSERT_TRUE( 10000 == smallest );
        ASSERT_TRUE( 10000 == largest );

        // Now try and get a write buffer plus sanity checks
        
        ASSERT_TRUE( CORE::CDateTime::Empty == buffers.GetCurrenWriterBufferAssociatedDt() );
        CORE::CDateTime beforeOperationDt = CORE::CDateTime::NowUTCDateTime();          // <- time sensitive, dont breakpoint!
        CORE::CDynamicBuffer* writeBuffer = buffers.GetNextWriterBuffer( false, 25 );   // <- time sensitive, dont breakpoint!     
        CORE::CDateTime writeStartDt = buffers.GetCurrenWriterBufferAssociatedDt();
        CORE::CDateTime firstWriteStartDt = writeStartDt; // <- used later for reader test
        ASSERT_TRUE( GUCEF_NULL != writeBuffer );

        // even with a slow running test (bogged down test machine?) it should not have taken more then 1sec
        // to get the buffer assigned. Note that the lock timeout above is 25 ms and we are only using 1 thread
        Int64 timeDifferenceInMs = beforeOperationDt.GetTimeDifferenceInMillisecondsTowards( writeStartDt ); 
        ASSERT_TRUE( timeDifferenceInMs >= 0 && timeDifferenceInMs < 1000 );

        ASSERT_TRUE( 0 == buffers.GetCurrentWriterBufferIndex() );
        ASSERT_TRUE( -1 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 0 == writeBuffer->GetDataSize() );
        ASSERT_TRUE( 10000 == writeBuffer->GetBufferSize() );

        // try writing data and then reading it back in blocks
        // we will use a patern of alternating test data block 1 and 2
        // we will write each data block as a single element

        CORE::CString testDataBlock1 = "1234567890";        
        CORE::CString testDataBlock2 = "abcdefghijklmnopqrstuvwxyz";
        UInt32 bytesWritten = 0;
        for ( CORE::UInt16 i=0; i<1000; ++i )
        {
            bytesWritten += writeBuffer->Append( testDataBlock1.C_String(), testDataBlock1.ByteSize(), true );
            bytesWritten += writeBuffer->Append( testDataBlock2.C_String(), testDataBlock2.ByteSize(), true );            
        }
        UInt32 bytesIntendedToBeWritten = ( testDataBlock1.ByteSize() + testDataBlock2.ByteSize() ) * 1000;
        ASSERT_TRUE( bytesIntendedToBeWritten == bytesWritten );
        ASSERT_TRUE( bytesIntendedToBeWritten == writeBuffer->GetDataSize() );
        ASSERT_TRUE( -1 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 0 == buffers.GetBuffersQueuedToRead() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        
        // the buffer should have resized as we wrote since 10000 bytes is not enough
        ASSERT_TRUE( bytesIntendedToBeWritten == writeBuffer->GetBufferSize() ); 

        // Since we have 3 buffers, lets fill another one
        // This time with numbers only
        // Note that we do not signal the end of writing explicitly, we just ask for the next buffer which should have the same
        // implicit effect

        writeBuffer = buffers.GetNextWriterBuffer( false, 25 );        
        ASSERT_TRUE( GUCEF_NULL != writeBuffer );
        ASSERT_TRUE( 0 == writeBuffer->GetDataSize() );
        ASSERT_TRUE( -1 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 1 == buffers.GetBuffersQueuedToRead() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        bytesWritten = 0;
        for ( CORE::UInt16 i=0; i<1000; ++i )
        {
            bytesWritten += writeBuffer->Append( testDataBlock1.C_String(), testDataBlock1.ByteSize(), true );
        }
        bytesIntendedToBeWritten = testDataBlock1.ByteSize() * 1000;
        ASSERT_TRUE( bytesIntendedToBeWritten == bytesWritten );
        ASSERT_TRUE( bytesIntendedToBeWritten == writeBuffer->GetDataSize() );

        // For this second buffer we will explicitly signal that we are done writing
        writeBuffer = GUCEF_NULL;
        ASSERT_TRUE( buffers.SignalEndOfWriting() );
        ASSERT_TRUE( 2 == buffers.GetBuffersQueuedToRead() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        ASSERT_TRUE( -1 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 1 == buffers.GetCurrentWriterBufferIndex() );

        // Since we have 3 buffers, lets fill another one
        // This time with letters only

        writeBuffer = buffers.GetNextWriterBuffer( false, 25 );        
        ASSERT_TRUE( GUCEF_NULL != writeBuffer );        
        ASSERT_TRUE( 0 == writeBuffer->GetDataSize() );
        ASSERT_TRUE( -1 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 2 == buffers.GetBuffersQueuedToRead() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        bytesWritten = 0;
        for ( CORE::UInt16 i=0; i<1000; ++i )
        {
            bytesWritten += writeBuffer->Append( testDataBlock2.C_String(), testDataBlock2.ByteSize(), true );
        }
        bytesIntendedToBeWritten = testDataBlock2.ByteSize() * 1000;
        ASSERT_TRUE( bytesIntendedToBeWritten == writeBuffer->GetDataSize() );
        ASSERT_TRUE( bytesIntendedToBeWritten == bytesWritten );
        
        writeStartDt = buffers.GetCurrenWriterBufferAssociatedDt();
        ASSERT_TRUE( CORE::CDateTime::Empty != writeStartDt );

        // For this third buffer we will also explicitly signal that we are done writing
        writeBuffer = GUCEF_NULL;
        ASSERT_TRUE( buffers.SignalEndOfWriting() );
        ASSERT_TRUE( 3 == buffers.GetBuffersQueuedToRead() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        ASSERT_TRUE( -1 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 2 == buffers.GetCurrentWriterBufferIndex() );
        ASSERT_TRUE( CORE::CDateTime::Empty == buffers.GetCurrenWriterBufferAssociatedDt() );

        // Since we have 3 buffers and we have not read anything we should not be able to get another
        // we are full        
        for ( CORE::UInt16 i=0; i<25; ++i )
        {
            writeBuffer = buffers.GetNextWriterBuffer( false, 25 );        
            ASSERT_TRUE( GUCEF_NULL == writeBuffer ); 
        }
        ASSERT_TRUE( 3 == buffers.GetBuffersQueuedToRead() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        ASSERT_TRUE( -1 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 2 == buffers.GetCurrentWriterBufferIndex() );

        // Since we already ended writing, signaling so again should be a no-op
        ASSERT_TRUE( buffers.SignalEndOfWriting() );
        ASSERT_TRUE( 3 == buffers.GetBuffersQueuedToRead() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        ASSERT_TRUE( -1 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 2 == buffers.GetCurrentWriterBufferIndex() );
               
        // Now try and get a read buffer plus sanity checks
        
        ASSERT_TRUE( CORE::CDateTime::Empty == buffers.GetCurrenReaderBufferAssociatedDt() );        
        CORE::CDateTime writeStartDtPerReader;
        CORE::CDynamicBuffer* readBuffer = buffers.GetNextReaderBuffer( writeStartDtPerReader, false, 25 );
        ASSERT_TRUE( GUCEF_NULL != readBuffer );
        ASSERT_TRUE( writeStartDtPerReader == buffers.GetCurrenReaderBufferAssociatedDt() );        
        ASSERT_TRUE( 0 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 2 == buffers.GetCurrentWriterBufferIndex() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        ASSERT_TRUE( 3 == buffers.GetBuffersQueuedToRead() );

        // Ok we got a read buffer but is it the correct one
        ASSERT_TRUE( writeStartDtPerReader == firstWriteStartDt );
        bytesIntendedToBeWritten = ( testDataBlock1.ByteSize() + testDataBlock2.ByteSize() ) * 1000;
        ASSERT_TRUE( readBuffer->GetDataSize() == bytesIntendedToBeWritten );
        
        // Now we will check if we get the data back in the correct order if we read per block
        CORE::CDynamicBufferAccess bufferAccess( readBuffer );
        bool useBlock1 = true;
        for ( CORE::UInt16 i=0; i<2000; ++i )
        {
            CORE::CString testStr = bufferAccess.ReadString();
            if ( useBlock1 )
            {
                ASSERT_TRUE( testStr == testDataBlock1 );
                useBlock1 = false;
            }
            else
            {
                ASSERT_TRUE( testStr == testDataBlock2 );
                useBlock1 = true;
            }
        }

        // Now proceed to the next buffer, implicitly signaling we are done with the current buffer

        readBuffer = buffers.GetNextReaderBuffer( writeStartDtPerReader, false, 25 );
        ASSERT_TRUE( GUCEF_NULL != readBuffer );
        ASSERT_TRUE( writeStartDtPerReader == buffers.GetCurrenReaderBufferAssociatedDt() );        
        ASSERT_TRUE( 1 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 2 == buffers.GetCurrentWriterBufferIndex() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        ASSERT_TRUE( 2 == buffers.GetBuffersQueuedToRead() );

        // Now we will check if we got the correct data back
        bytesIntendedToBeWritten = testDataBlock1.ByteSize() * 1000;
        ASSERT_TRUE( readBuffer->GetDataSize() == bytesIntendedToBeWritten );

        bufferAccess.LinkTo( readBuffer, false );
        for ( CORE::UInt16 i=0; i<1000; ++i )
        {
            CORE::CString testStr = bufferAccess.ReadString();
            ASSERT_TRUE( testStr == testDataBlock1 );
        }

        // Now we signal an explicit end to reading
        ASSERT_TRUE( buffers.SignalEndOfReading() );
        readBuffer = GUCEF_NULL;
        ASSERT_TRUE( CORE::CDateTime::Empty == buffers.GetCurrenReaderBufferAssociatedDt() );
        ASSERT_TRUE( 1 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 2 == buffers.GetCurrentWriterBufferIndex() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        ASSERT_TRUE( 1 == buffers.GetBuffersQueuedToRead() );

        // Now that we have read 2 buffers we thus have 2 buffers available to write again
        // Lets try doing another write

        CORE::CString testDataBlock3 = "FOO()()()()()()()()()()()()()()()()()BAR";
        writeBuffer = buffers.GetNextWriterBuffer( false, 25 );        
        ASSERT_TRUE( GUCEF_NULL != writeBuffer );        
        ASSERT_TRUE( 0 == writeBuffer->GetDataSize() );
        ASSERT_TRUE( 1 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 0 == buffers.GetCurrentWriterBufferIndex() );
        ASSERT_TRUE( 1 == buffers.GetBuffersQueuedToRead() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        bytesWritten = 0;
        for ( CORE::UInt16 i=0; i<1000; ++i )
        {
            bytesWritten += writeBuffer->Append( testDataBlock3.C_String(), testDataBlock3.ByteSize(), true );
        }
        bytesIntendedToBeWritten = testDataBlock3.ByteSize() * 1000;
        ASSERT_TRUE( buffers.SignalEndOfWriting() );
        ASSERT_TRUE( 2 == buffers.GetBuffersQueuedToRead() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        ASSERT_TRUE( 1 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 0 == buffers.GetCurrentWriterBufferIndex() );
        ASSERT_TRUE( bytesIntendedToBeWritten == writeBuffer->GetDataSize() );
        ASSERT_TRUE( bytesIntendedToBeWritten == bytesWritten );

        // Now read again
        // Note that this should read the 3rd buffer we wrote, not the one we just wrote
        
        readBuffer = buffers.GetNextReaderBuffer( writeStartDtPerReader, false, 25 );
        ASSERT_TRUE( GUCEF_NULL != readBuffer );
        ASSERT_TRUE( 2 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 0 == buffers.GetCurrentWriterBufferIndex() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        ASSERT_TRUE( 2 == buffers.GetBuffersQueuedToRead() );
        ASSERT_TRUE( CORE::CDateTime::Empty != writeStartDtPerReader );

        // Now we will check if we got the correct data back
        bytesIntendedToBeWritten = testDataBlock2.ByteSize() * 1000;
        ASSERT_TRUE( readBuffer->GetDataSize() == bytesIntendedToBeWritten );

        bufferAccess.LinkTo( readBuffer, false );
        for ( CORE::UInt16 i=0; i<1000; ++i )
        {
            CORE::CString testStr = bufferAccess.ReadString();
            ASSERT_TRUE( testStr == testDataBlock2 );
        }

        // Now read again
        // Note that this should read the 4th buffer we wrote
        
        readBuffer = buffers.GetNextReaderBuffer( writeStartDtPerReader, false, 25 );
        ASSERT_TRUE( GUCEF_NULL != readBuffer );
        ASSERT_TRUE( 0 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 0 == buffers.GetCurrentWriterBufferIndex() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        ASSERT_TRUE( 1 == buffers.GetBuffersQueuedToRead() );
        ASSERT_TRUE( CORE::CDateTime::Empty != writeStartDtPerReader );
        ASSERT_TRUE( writeStartDtPerReader == buffers.GetCurrenReaderBufferAssociatedDt() );        

        // Now we will check if we got the correct data back
        bytesIntendedToBeWritten = testDataBlock3.ByteSize() * 1000;
        ASSERT_TRUE( readBuffer->GetDataSize() == bytesIntendedToBeWritten );

        bufferAccess.LinkTo( readBuffer, false );
        for ( CORE::UInt16 i=0; i<1000; ++i )
        {
            CORE::CString testStr = bufferAccess.ReadString();
            ASSERT_TRUE( testStr == testDataBlock3 );
        }

        // Now attempt to read again
        // Note that we only wrote 4 buffer's worth so this should not give us anything
        
        readBuffer = buffers.GetNextReaderBuffer( writeStartDtPerReader, false, 25 );
        ASSERT_TRUE( GUCEF_NULL == readBuffer );
        ASSERT_TRUE( 0 == buffers.GetCurrentReaderBufferIndex() );
        ASSERT_TRUE( 0 == buffers.GetCurrentWriterBufferIndex() );
        ASSERT_TRUE( 3 == buffers.GetNrOfBuffers() );
        ASSERT_TRUE( 0 == buffers.GetBuffersQueuedToRead() );
        ASSERT_TRUE( CORE::CDateTime::Empty == writeStartDtPerReader );
        ASSERT_TRUE( writeStartDtPerReader == buffers.GetCurrenReaderBufferAssociatedDt() );        
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CDynamicBufferSwap TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/

void
PerformDynamicBufferViewTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CDynamicBufferView TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CDynamicBufferView" );

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CDynamicBufferView] Default construct: empty" )
    try
    {
        CORE::CDynamicBufferView view;
        ASSERT_TRUE(  0 == view.GetDataSize() );
        ASSERT_TRUE(  0 == view.GetBufferSize() );
        ASSERT_TRUE(  GUCEF_NULL == view.GetConstBufferPtr() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CDynamicBufferView] Construct from (void*, size): no copy" )
    try
    {
        static const char rawData[] = "hello view";
        CORE::CDynamicBufferView view( rawData, sizeof( rawData ) );
        ASSERT_TRUE(  view.IsLinked() );
        ASSERT_TRUE(  sizeof( rawData ) == view.GetBufferSize() );
        ASSERT_TRUE(  view.GetConstBufferPtr() == static_cast< const void* >( rawData ) );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CDynamicBufferView] Construct from CDynamicBuffer: no copy" )
    try
    {
        CORE::CDynamicBuffer owned( 16, false );
        owned.CopyFrom( 5, "hello" );
        owned.SetDataSize( 5 );

        CORE::CDynamicBufferView view( owned );
        ASSERT_TRUE(  view.IsLinked() );
        ASSERT_TRUE(  view.GetConstBufferPtr() == owned.GetConstBufferPtr() );
        ASSERT_TRUE(  view.GetDataSize() == owned.GetDataSize() );
        ASSERT_TRUE(  view == owned );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CDynamicBufferView] Copy rebinds; both point to same raw buffer" )
    try
    {
        static const char rawData[] = "copy rebind";
        CORE::CDynamicBufferView view1( rawData, sizeof( rawData ) );
        CORE::CDynamicBufferView view2( view1 );
        ASSERT_TRUE(  view2.IsLinked() );
        ASSERT_TRUE(  view2.GetConstBufferPtr() == static_cast< const void* >( rawData ) );
        ASSERT_TRUE(  view2.GetConstBufferPtr() == view1.GetConstBufferPtr() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CDynamicBufferView] Implicit conversion to const CDynamicBuffer& works" )
    try
    {
        CORE::CDynamicBuffer owned( 8, false );
        owned.CopyFrom( 4, "test" );
        owned.SetDataSize( 4 );

        CORE::CDynamicBufferView view( owned );

        /* operator== takes const CDynamicBuffer& — implicit conversion used */
        ASSERT_TRUE(  view == owned );

        const CORE::CDynamicBuffer& ref = view;
        ASSERT_TRUE(  ref.GetConstBufferPtr() == owned.GetConstBufferPtr() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CDynamicBufferView] CopyTo copies data correctly" )
    try
    {
        static const char rawData[] = { 1, 2, 3, 4, 5 };
        CORE::CDynamicBufferView view( rawData, sizeof( rawData ) );
        view.LinkTo( rawData, sizeof( rawData ) );
        /* Manually set data size on inner buffer via AsBuffer() — data size equals buffer size here */
        char dest[ 5 ] = { 0 };
        CORE::UInt32 copied = view.CopyTo( 0, sizeof( rawData ), dest );
        ASSERT_TRUE(  sizeof( rawData ) == copied );
        for ( int i = 0; i < 5; ++i )
            ASSERT_TRUE(  dest[ i ] == rawData[ i ] );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CDynamicBufferView] Find locates byte sequence" )
    try
    {
        static const char rawData[] = "abcdefgh";
        CORE::CDynamicBufferView view( rawData, sizeof( rawData ) );
        CORE::Int32 pos = view.Find( "cde", 3, 0 );
        ASSERT_TRUE(  2 == pos );

        pos = view.Find( "xyz", 3, 0 );
        ASSERT_TRUE(  -1 == pos );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CDynamicBufferView] LinkTo rebinds to new buffer" )
    try
    {
        static const char buf1[] = "first";
        static const char buf2[] = "second";

        CORE::CDynamicBufferView view( buf1, sizeof( buf1 ) );
        ASSERT_TRUE(  view.GetConstBufferPtr() == static_cast< const void* >( buf1 ) );

        view.LinkTo( buf2, sizeof( buf2 ) );
        ASSERT_TRUE(  view.GetConstBufferPtr() == static_cast< const void* >( buf2 ) );
        ASSERT_TRUE(  view.IsLinked() );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    /* ------------------------------------------------------------------ */
    GUCEF_TESTFW_TESTCASE( "[CDynamicBufferView] AsConstTypePtr<T> accesses typed data" )
    try
    {
        static const CORE::UInt32 vals[] = { 0xDEADBEEFu, 0xCAFEBABEu };
        CORE::CDynamicBufferView view( vals, sizeof( vals ) );

        const CORE::UInt32* ptr = view.AsConstTypePtr< CORE::UInt32 >( 0 );
        ASSERT_TRUE(  GUCEF_NULL != ptr );
        ASSERT_TRUE(  0xDEADBEEFu == ptr[ 0 ] );
        ASSERT_TRUE(  0xCAFEBABEu == ptr[ 1 ] );
    }
    catch( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CDynamicBufferView TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
