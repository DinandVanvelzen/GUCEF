/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "CCyclicDynamicBuffer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CCyclicDynamicBuffer::CCyclicDynamicBuffer( const UInt32 initialBufferSize /* = 0 */ )
    : m_buffer( initialBufferSize, true ) ,
      m_freeBlocks()                      ,
      m_usedBlocks()
{GUCEF_TRACE;
    
    if ( initialBufferSize > 0 )
    {
        TDataChunk freeBlock;
        freeBlock.blockSize = initialBufferSize;
        freeBlock.startOffset = 0;
        m_freeBlocks.push_back( freeBlock );
    }
}

/*-------------------------------------------------------------------------*/
    
CCyclicDynamicBuffer::~CCyclicDynamicBuffer()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/
    
CCyclicDynamicBuffer::CCyclicDynamicBuffer( const CCyclicDynamicBuffer& src )
    : m_buffer( src.m_buffer )         ,
      m_freeBlocks( src.m_freeBlocks ) ,
      m_usedBlocks( src.m_usedBlocks )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CCyclicDynamicBuffer&
CCyclicDynamicBuffer::operator=( const CCyclicDynamicBuffer& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        Clear();
        
        src.Lock();
        Lock();
        
        m_buffer = src.m_buffer;
        m_usedBlocks = src.m_usedBlocks;
        m_freeBlocks = src.m_freeBlocks;
        
        Unlock();
        src.Unlock();
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::ReadBlockTo( CDynamicBuffer& buffer )
{GUCEF_TRACE;
    
    UInt32 bytesRead = 0;
    
    Lock();
    
    // We will simply pop a FIFO element
    TBlockList::iterator i = m_usedBlocks.begin();
    if ( i != m_usedBlocks.end() )
    {        
        TDataChunk& dataChunck = (*i);
        
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CCyclicDynamicBuffer: Reading an entire block of size " + UInt32ToString( dataChunck.blockSize ) + "bytes out of the buffer" );
        
        // Ensure that the receiving buffer is large enough to handle the block
        buffer.SetBufferSize( dataChunck.blockSize, false );
        
        // Copy the entire element into the given buffer and set the data size
        bytesRead = m_buffer.CopyTo( dataChunck.startOffset ,
                                     dataChunck.blockSize   ,
                                     buffer.GetBufferPtr()  );
        buffer.SetDataSize( bytesRead );
        
        // Mark the element as free'd
        TDataChunk freeDataChunck = dataChunck;
        m_freeBlocks.push_back( freeDataChunck );
        m_usedBlocks.erase( i );
    }
    
    TidyFreeBlocks();
    
    Unlock();
    
    return bytesRead;
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::SkipRead( const UInt32 bytesToSkip )
{GUCEF_TRACE;
    
    UInt32 totalBytesRead = 0, totalBytes = bytesToSkip;
    bool blocksFreed = false;
    
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CCyclicDynamicBuffer: Skipping data in the buffer" );    

    // We will simply pop FIFO elements until we hit the byte count
    for ( TBlockList::iterator i = m_usedBlocks.begin(); i != m_usedBlocks.end(); ++i )
    {
        TDataChunk& dataChunck = (*i);
        if ( totalBytes >= dataChunck.blockSize )
        {
            totalBytesRead += dataChunck.blockSize;
            totalBytes -= dataChunck.blockSize;
            
            m_freeBlocks.push_back( (*i) );
            m_usedBlocks.erase( i );
            blocksFreed = true;
            
            i = m_usedBlocks.begin();
            if ( 0 == totalBytes || i == m_usedBlocks.end() )
            {
                // There is no more data to skip
                break;
            }
        }
        else
        {
            totalBytesRead += totalBytes;         

            // Mark the segment as free'd
            TDataChunk freeDataChunck; 
            freeDataChunck.blockSize = totalBytes;
            freeDataChunck.startOffset = dataChunck.startOffset;

            // Shrink the block to remove the data thats skipped
            dataChunck.blockSize -= totalBytes;            
            dataChunck.startOffset += totalBytes;

            m_freeBlocks.push_back( freeDataChunck );
            blocksFreed = true;
            break;
        }
    }

    // Optimize the free blocks
    if ( blocksFreed )
    {
        TidyFreeBlocks();
    }
    return totalBytesRead;
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::PeekBlock( CDynamicBuffer& buffer )
{GUCEF_TRACE;
    
    UInt32 bytesRead = 0;
    
    Lock();
    
    // We will simply pop a FIFO element
    TBlockList::iterator i = m_usedBlocks.begin();
    if ( i != m_usedBlocks.end() )
    {        
        TDataChunk& dataChunck = (*i);
        
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CCyclicDynamicBuffer: Peeking an entire block of size " + UInt32ToString( dataChunck.blockSize ) + "bytes" );
        
        // Ensure that the receiving buffer is large enough to handle the block
        buffer.SetBufferSize( dataChunck.blockSize, false );
        
        // Copy the entire element into the given buffer and set the data size
        bytesRead = m_buffer.CopyTo( dataChunck.startOffset ,
                                     dataChunck.blockSize   ,
                                     buffer.GetBufferPtr()  );
        buffer.SetDataSize( bytesRead );
    }

    Unlock();
    
    return bytesRead;
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::Peek( void* destBuffer             ,
                            const UInt32 bytesPerElement ,
                            const UInt32 elementsToRead  )
{GUCEF_TRACE;

    Lock();

    UInt8* destPtr = (UInt8*) destBuffer;    
    TBlockList peekBlocks;
    UInt32 totalBytesRead = 0; UInt32 bytesRead = 0;
    for ( UInt32 i=0; i<elementsToRead; ++i )
    {        
        TBlockList elementBlocks;
        bytesRead = ReadElement( destPtr, bytesPerElement, false, elementBlocks );

        TBlockList::iterator n = elementBlocks.begin();
        while ( n != elementBlocks.end() )
        {
            peekBlocks.push_back( (*n) );
            ++n;
        }

        if ( 0 == bytesRead ) break;

        totalBytesRead += bytesRead;
        destPtr += bytesRead;
    }

    // Since we are only peeking we should leave all the data available for subsequent peek/read operations
    // We will now roll-back the read making it a peek
    TBlockList::reverse_iterator m = peekBlocks.rbegin();
    while ( m != peekBlocks.rend() )
    {
        m_usedBlocks.push_front( (*m) );
        ++m;
    } 

    Unlock();
    return totalBytesRead;
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::CopyBlocksToBuffer( const TBlockList& blockList ,
                                          void* destBuffer            )
{GUCEF_TRACE;

    UInt8* destPtr = (UInt8*) destBuffer;
    UInt32 totalBytesRead = 0; UInt32 bytesRead = 0;
    TBlockList::const_iterator i = blockList.begin();
    while ( i != blockList.end() )
    {
        const TDataChunk& dataChunck = (*i);

        bytesRead = m_buffer.CopyTo( dataChunck.startOffset   ,
                                     dataChunck.blockSize     ,
                                     destPtr + totalBytesRead );
        totalBytesRead += bytesRead;
        ++i;
    }

    return totalBytesRead;
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::PeekElement( void* destBuffer         ,
                                   const UInt32 elementSize )
{GUCEF_TRACE;

    Lock();
    
    UInt32 totalBytesRead = 0, totalBytes = elementSize;    
    TBlockList elementBlocks;

    // We will simply pop FIFO elements until we hit the byte count
    TBlockList::iterator i = m_usedBlocks.begin();
    while ( i != m_usedBlocks.end() )
    {
        TDataChunk& dataChunck = (*i);
        if ( totalBytes >= dataChunck.blockSize )
        {
            elementBlocks.push_back( dataChunck );

            totalBytesRead += dataChunck.blockSize;
            totalBytes -= dataChunck.blockSize;             

            if ( 0 == totalBytes || i == m_usedBlocks.end() )
            {
                // There is no more data to read
                break;
            }
        }
        else
        {
            TDataChunk elementChunck;
            elementChunck.startOffset = dataChunck.startOffset;
            elementChunck.blockSize = totalBytes;
            elementBlocks.push_back( elementChunck );

            totalBytesRead += totalBytes; 
            
            break;
        }
        ++i;
    }

    if ( totalBytesRead == elementSize )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CCyclicDynamicBuffer: Peeking element from the buffer" );
        
        // We successfully located the blocks for a single element
        // We can now copy these blocks to the output buffer
        CopyBlocksToBuffer( elementBlocks, destBuffer );

        Unlock();
        return elementSize;
    }

    Unlock();
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::ReadElement( void* destBuffer          ,
                                   const UInt32 elementSize  ,
                                   bool freeBlocksOnSuccess  ,
                                   TBlockList& elementBlocks )
{GUCEF_TRACE;

    UInt32 totalBytesRead = 0, totalBytes = elementSize;    

    // We will simply pop FIFO elements until we hit the byte count
    TBlockList::iterator i = m_usedBlocks.begin();
    while ( i != m_usedBlocks.end() )
    {
        TDataChunk& dataChunck = (*i);
        if ( totalBytes >= dataChunck.blockSize )
        {
            elementBlocks.push_back( dataChunck );

            totalBytesRead += dataChunck.blockSize;
            totalBytes -= dataChunck.blockSize;             
          
            m_usedBlocks.erase( i );            
            i = m_usedBlocks.begin();

            if ( 0 == totalBytes || i == m_usedBlocks.end() )
            {
                // There is no more data to read
                break;
            }
        }
        else
        {
            TDataChunk elementChunck;
            elementChunck.startOffset = dataChunck.startOffset;
            elementChunck.blockSize = totalBytes;
            elementBlocks.push_back( elementChunck );

            totalBytesRead += totalBytes;         
            
            // Shrink the block to remove the data thats read
            dataChunck.blockSize -= totalBytes;            
            dataChunck.startOffset += totalBytes;
            
            break;
        }
    }

    if ( totalBytesRead == elementSize )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CCyclicDynamicBuffer: Reading data out of the buffer" );
        
        // We successfully located the blocks for a single element
        // We can now copy these blocks to the output buffer
        CopyBlocksToBuffer( elementBlocks, destBuffer );

        if ( freeBlocksOnSuccess )
        {
            // Now that we copied the element blocks we can register them as free blocks
            TBlockList::iterator n = elementBlocks.begin();
            while ( n != elementBlocks.end() )
            {
                m_freeBlocks.push_back( (*n) );
                ++n;
            }
        }

        return elementSize;
    }
    else
    {
        // Because we failed to read the entire element we will restore the element blocks we used up
        // as "used blocks" making them available for a new request. Basically rolling back the block edits.
        TBlockList::reverse_iterator i = elementBlocks.rbegin();
        while ( i != elementBlocks.rend() )
        {
            m_usedBlocks.push_front( (*i) );
            ++i;
        }
        
        return 0;        
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::Read( void* destBuffer             ,
                            const UInt32 bytesPerElement ,
                            const UInt32 elementsToRead  )
{GUCEF_TRACE;

    Lock();

    UInt8* destPtr = (UInt8*) destBuffer;
    UInt32 totalBytesRead = 0; UInt32 bytesRead = 0;
    for ( UInt32 i=0; i<elementsToRead; ++i )
    {
        TBlockList elementBlocks;
        bytesRead = ReadElement( destPtr, bytesPerElement, true, elementBlocks );
        if ( 0 == bytesRead )
        {
            // Unable to read a complete element
            // Optimize the free blocks then abort
            TidyFreeBlocks();
            return totalBytesRead; 
        }

        totalBytesRead += bytesRead;
        destPtr += bytesRead;
    }

    // Optimize the free blocks
    TidyFreeBlocks();

    Unlock();
    return totalBytesRead;
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::WriteAtFrontOfQueue( const void* srcBuffer        ,
                                           const UInt32 bytesPerElement ,
                                           const UInt32 elementsToWrite )
{GUCEF_TRACE;

    return WriteImp( srcBuffer       ,
                     bytesPerElement ,
                     elementsToWrite ,
                     false           );
}

/*-------------------------------------------------------------------------*/
    
UInt32
CCyclicDynamicBuffer::Write( const void* srcBuffer        ,
                             const UInt32 bytesPerElement ,
                             const UInt32 elementsToWrite )
{GUCEF_TRACE;

    return WriteImp( srcBuffer       ,
                     bytesPerElement ,
                     elementsToWrite ,
                     true            );
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::Write( const CDynamicBuffer& buffer          ,
                             const UInt32 offsetInBuffer /* = 0 */ )
{GUCEF_TRACE;

    Int32 bytesPerElement = (Int32) buffer.GetDataSize() - offsetInBuffer;    
    if ( bytesPerElement > 0 )
    {
        return WriteImp( buffer.GetConstBufferPtr( offsetInBuffer ) ,
                         (UInt32)bytesPerElement                    ,
                         1                                          ,
                         true                                       );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/
    
UInt32
CCyclicDynamicBuffer::WriteImp( const void* srcBuffer        ,
                                const UInt32 bytesPerElement ,
                                const UInt32 elementsToWrite ,
                                const bool writeToBack       )
{GUCEF_TRACE;

    // Initial sanity check
    if ( bytesPerElement == 0 || elementsToWrite == 0 ) return 0;
    
    Lock();
    
    UInt32 totalBytesWritten = 0;
    UInt32 totalBytes = bytesPerElement * elementsToWrite;
    UInt32 remainingElements = elementsToWrite;
    
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CCyclicDynamicBuffer: Writing " + UInt32ToString( totalBytes ) + " bytes of data into the buffer" );
    
    for ( TBlockList::iterator i = m_freeBlocks.begin(); i != m_freeBlocks.end(); ++i )
    {            
        if ( (*i).blockSize >= bytesPerElement )
        {
            // Calculate how many elements and thus bytes we can fit in the block
            UInt32 bytesToWrite, elementsToWrite;
            UInt32 nrOfElementsThatFitInBlock = (*i).blockSize / bytesPerElement;
            if ( nrOfElementsThatFitInBlock >= remainingElements )
            {
                elementsToWrite = remainingElements;
                bytesToWrite = bytesPerElement * remainingElements;
            }
            else
            {
                elementsToWrite = nrOfElementsThatFitInBlock;
                bytesToWrite = bytesPerElement * elementsToWrite;
            }
            
            // Copy the max number of elements
            UInt32 bytesWritten = m_buffer.CopyFrom( (*i).startOffset                       ,
                                                     bytesToWrite                           ,
                                                     ((Int8*)srcBuffer) + totalBytesWritten );
            totalBytesWritten += bytesToWrite;
            remainingElements -= elementsToWrite;
            
            // Sanity check
            if ( bytesWritten < bytesToWrite )
            {
                // something went wrong,.. intentionally do not update administration
                // as if the write to buffer never happened
                
                Unlock();                
                return totalBytesWritten;
            }
            
            TDataChunk usedDataChunck;
            usedDataChunck.blockSize = bytesWritten;
            usedDataChunck.startOffset = (*i).startOffset;                    
            if ( writeToBack )
            {
                // push the used-block-info ontop of our FIFO stack
                m_usedBlocks.push_back( usedDataChunck );
            }
            else
            {
                // push the used-block-info to the bottom of our FIFO stack
                m_usedBlocks.insert( m_usedBlocks.begin(), usedDataChunck );
            }
            
            // Check if we have a block remnant in the now (partially?) used free block
            if ( (*i).blockSize > bytesWritten )
            {
                // store the free-block-info
                (*i).blockSize -= bytesWritten;
                (*i).startOffset += bytesWritten;
            }
            else
            {
                // remove the block from our list of available blocks
                m_freeBlocks.erase( i );
                i = m_freeBlocks.begin();
            }
            
            // Check if we should stop this free block matching excercise
            if ( 0 == remainingElements || i == m_freeBlocks.end() )
            {
                break;
            }
        }
    }
    
    // Check if we where able to store the given data using free blocks 
    // in the existing buffer space
    if ( remainingElements > 0 )
    {
        // We where unable to store everything using the available free blocks
        // We will create a new used block and enlarge the buffer if needed to
        // store the excess data
        UInt32 bytesToWrite = totalBytes - totalBytesWritten;
        UInt32 startOffset = m_buffer.GetDataSize();
        m_buffer.Append( ((Int8*)srcBuffer) + totalBytesWritten ,
                         bytesToWrite                           ,
                         true                                   );
                         
        // push the used-block-info on our FIFO stack
        TDataChunk usedDataChunck;
        usedDataChunck.blockSize = bytesToWrite;
        usedDataChunck.startOffset = startOffset;
        if ( writeToBack )
        {
            // push the used-block-info ontop of our FIFO stack
            m_usedBlocks.push_back( usedDataChunck );
        }
        else
        {
            // push the used-block-info to the bottom of our FIFO stack
            m_usedBlocks.insert( m_usedBlocks.begin(), usedDataChunck );
        }
        
        // We have now written everything into the buffer
        totalBytesWritten = totalBytes;
    }
    
    TidyFreeBlocks();
    
    Unlock(); 
    
    return totalBytesWritten;
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::GetNrOfUsedBlocks( void ) const
{GUCEF_TRACE;

    Lock();    
    UInt32 nrOfUsedBlocks = (UInt32) m_usedBlocks.size();    
    Unlock();
    return nrOfUsedBlocks;
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::GetNrOfFreeBlocks( void ) const
{GUCEF_TRACE;

    Lock();    
    UInt32 nrOfFreeBlocks = (UInt32) m_freeBlocks.size();    
    Unlock();
    return nrOfFreeBlocks;
}

/*-------------------------------------------------------------------------*/

void
CCyclicDynamicBuffer::TidyFreeBlocks( void )
{GUCEF_TRACE;

    if ( !m_freeBlocks.empty() )
    {        
        // first we (bubble-)sort the blocks based on their offset
        bool listMutation = true;
        while ( listMutation )
        {            
            listMutation = false;
            TBlockList::iterator i = m_freeBlocks.begin();
            UInt32 prevOffset = (*i).startOffset;
            for ( ++i;  i != m_freeBlocks.end(); ++i )
            {                
                if ( (*i).startOffset < prevOffset )
                {
                    TBlockList::iterator n = i;
                    --n;
                    TDataChunk chunck = (*n);
                    (*n) = (*i);
                    (*i) = chunck;
                    
                    listMutation = true;
                }
                prevOffset = (*i).startOffset;
            }
        }

        // now we (bubble) merge blocks that are adjecent
        listMutation = true;
        while ( listMutation )
        {
            listMutation = false;
            
            TBlockList::iterator i = m_freeBlocks.begin();
            TBlockList::iterator n = i;
            
            for ( ++i; i != m_freeBlocks.end(); ++i )
            {
                const TDataChunk prevChunck = (*n);
                if ( prevChunck.startOffset + prevChunck.blockSize == (*i).startOffset )
                {
                    // Perform a block merge
                    (*n).blockSize = prevChunck.blockSize + (*i).blockSize;
                    (*n).startOffset = prevChunck.startOffset;                    
                    m_freeBlocks.erase( i );
                    
                    listMutation = true;
                    break;
                }
                ++n;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CCyclicDynamicBuffer::Clear( const bool logicalClearOnly /* = false */ )
{GUCEF_TRACE;

    Lock();
    
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CCyclicDynamicBuffer: Clearing the buffer" );
    
    m_usedBlocks.clear();
    m_freeBlocks.clear();
    m_buffer.Clear( logicalClearOnly );
    
    Unlock();
}

/*-------------------------------------------------------------------------*/

bool
CCyclicDynamicBuffer::HasBufferedData( void ) const
{GUCEF_TRACE;

    return !m_usedBlocks.empty();
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::GetBufferedDataSizeInBytes( void ) const
{GUCEF_TRACE;

    Lock();
    
    UInt32 totalBytes = 0;
    for ( TBlockList::const_iterator i = m_usedBlocks.begin(); i != m_usedBlocks.end(); ++i )
    {
        totalBytes += (*i).blockSize;
    }    
    
    Unlock();
    
    return totalBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CCyclicDynamicBuffer::GetTotalBufferSizeInBytes( void ) const
{GUCEF_TRACE;

    Lock();
    
    UInt32 totalBufferSize = m_buffer.GetBufferSize();
    
    Unlock();
    
    return totalBufferSize;
}

/*-------------------------------------------------------------------------*/

Float32
CCyclicDynamicBuffer::GetBufferUsagePercentage( void ) const
{GUCEF_TRACE;
    
    Lock();
    
    Float32 usagePerc = GetBufferedDataSizeInBytes() / ( 1.0f * m_buffer.GetBufferSize() );
    
    Unlock();
    
    return usagePerc;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CCyclicDynamicBuffer::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/
    
MT::TLockStatus
CCyclicDynamicBuffer::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    // non-operation, implement in descending class for threadsafety if desired
    return MT::LOCKSTATUS_NOT_APPLICABLE;
}

/*-------------------------------------------------------------------------*/
    
MT::TLockStatus
CCyclicDynamicBuffer::Unlock( void ) const
{GUCEF_TRACE;

    // non-operation, implement in descending class for threadsafety if desired
    return MT::LOCKSTATUS_NOT_APPLICABLE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
