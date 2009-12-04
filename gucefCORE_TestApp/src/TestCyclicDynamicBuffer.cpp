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

#include <windows.h>            // needed for DebugBreak()
#include <iostream>

#ifndef GUCEF_CORE_CCYCLICDYNAMICBUFFER_H
#include "CCyclicDynamicBuffer.h"
#define GUCEF_CORE_CCYCLICDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CCYCLICDYNAMICBUFFER_H ? */

#include "TestCyclicDynamicBuffer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE { std::cout << "Test failed @ " << __FILE__ << "(" << __LINE__ << ")\n"; DebugBreak(); }
#define ASSERT_TRUE( test ) if ( !(test) ) { ERRORHERE; } 
#define ASSERT_FALSE( test ) if ( (test) ) { ERRORHERE; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

void
PerformCyclicDynamicBufferTests( void )
{
    std::cout << "\n\n**** COMMENCING CCyclicDynamicBuffer TESTS ****\n";
    
    try
    {
        CORE::CString testDataBlock1 = "1234567890";        
        CORE::CString testDataBlock2 = "abcdefghijklmnopqrstuvwxyz";
        
        CORE::CCyclicDynamicBuffer cdBuffer;
                
        // try writing data and then reading it back in blocks
        // we will use a patern of alternating test data block 1 and 2
        // we will write each data block as a single element
        for ( CORE::UInt16 i=0; i<1000; ++i )
        {
            ASSERT_TRUE( testDataBlock1.Length()+1 == cdBuffer.Write( testDataBlock1.C_String(), testDataBlock1.Length()+1, 1 ) );
            ASSERT_TRUE( testDataBlock2.Length()+1 == cdBuffer.Write( testDataBlock2.C_String(), testDataBlock2.Length()+1, 1 ) );
        }
        
        // Now we will check if we get the data back in the correct order if we read per block
        CORE::CString blockString;
        CORE::CDynamicBuffer blockBuffer;
        bool useBlock1 = true;
        for ( CORE::UInt16 i=0; i<2000; ++i )
        {
            ASSERT_TRUE( 0 != cdBuffer.ReadBlockTo( blockBuffer ) );
            
            // We scan for the string so that if something went wrong it wont immediatly kill the test with
            // a vague memory access problem
            blockString.Scan( static_cast< const char* >( blockBuffer.GetConstBufferPtr() ), blockBuffer.GetDataSize() );
            
            if ( useBlock1 )
            {
                ASSERT_TRUE( blockString == testDataBlock1 );
                useBlock1 = false;
            }
            else
            {
                ASSERT_TRUE( blockString == testDataBlock2 );
                useBlock1 = true;
            }
        }
        
        // make sure the buffer is now empty
        ASSERT_TRUE( 0 == cdBuffer.GetBufferedDataSizeInBytes() );
        ASSERT_TRUE( 0.1 > cdBuffer.GetBufferUsagePercentage() );
        ASSERT_FALSE( cdBuffer.HasBufferedData() );
        
        // Test to make sure the number of free blocks is 0 because all the free blocks should have been
        // consolidated into a single free block by now
        ASSERT_TRUE( cdBuffer.GetNrOfFreeBlocks() == 1 );
        cdBuffer = CORE::CCyclicDynamicBuffer();
        
        
        // Now we will do the same thing again but with intermittend reads
        useBlock1 = true;
        for ( CORE::UInt16 i=0; i<100; ++i )
        {
            ASSERT_TRUE( testDataBlock1.Length()+1 == cdBuffer.Write( testDataBlock1.C_String(), testDataBlock1.Length()+1, 1 ) );
            ASSERT_TRUE( testDataBlock2.Length()+1 == cdBuffer.Write( testDataBlock2.C_String(), testDataBlock2.Length()+1, 1 ) );
            
            ASSERT_TRUE( 0 != cdBuffer.ReadBlockTo( blockBuffer ) );
            blockString.Scan( static_cast< const char* >( blockBuffer.GetConstBufferPtr() ), blockBuffer.GetDataSize() );
            
            if ( useBlock1 )
            {
                // Now we read a block,.. since its FIFO this should always be testDataBlock2 never 1 that got read               
                ASSERT_TRUE( blockString == testDataBlock1 );
                useBlock1 = false;
            }
            else
            {
                // Now we read a block,.. since its FIFO this should always be testDataBlock1 from the previous round and never 2 that got read
                ASSERT_TRUE( blockString == testDataBlock2 );
                useBlock1 = true;
            }
        }
        
        // Test if the buffer still has half the reads, 100 iterations with 2 writes = 200 -> / 2 = 100 used
        ASSERT_TRUE( cdBuffer.GetNrOfUsedBlocks() == 100 );
        ASSERT_TRUE( cdBuffer.HasBufferedData() )
    }
    catch( ... )
    {
        ERRORHERE;
    }

    std::cout << "\n\n**** FINISHED CCyclicDynamicBuffer TESTS ****\n";
}

/*-------------------------------------------------------------------------*/
