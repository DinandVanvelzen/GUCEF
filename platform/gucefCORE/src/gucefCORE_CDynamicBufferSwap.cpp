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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

#include "gucefCORE_CDynamicBufferSwap.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDynamicBufferSwap::CBufferEntry::CBufferEntry( void )
    : buffer()
    , lock()
    , hasUnreadData( false )
    , associatedDt()
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/

CDynamicBufferSwap::CBufferEntry::CBufferEntry( const CBufferEntry& src )
    : buffer( src.buffer )
    , lock()
    , hasUnreadData( src.hasUnreadData )
    , associatedDt( src.associatedDt )
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/

CDynamicBufferSwap::CBufferEntry&
CDynamicBufferSwap::CBufferEntry::operator=( const CBufferEntry& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        buffer = src.buffer;
        hasUnreadData = src.hasUnreadData;
        associatedDt = src.associatedDt;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CDynamicBufferSwap::CDynamicBufferSwap( UInt32 nrOfBuffers )
    : MT::CILockable()
    , m_buffers()
    , m_currentReaderBufferIndex( -1 )
    , m_readingIsOngoing( false )
    , m_currentWriterBufferIndex( -1 )
    , m_writingIsOngoing( false )
    , m_buffersQueuedToRead( 0 )
    , m_lock()
{GUCEF_TRACE;

    if ( nrOfBuffers < 3 )
        nrOfBuffers = 3;
    m_buffers.resize( nrOfBuffers );
}

/*-------------------------------------------------------------------------*/

CDynamicBufferSwap::~CDynamicBufferSwap()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_buffers.clear();
}

/*-------------------------------------------------------------------------*/

UInt32 
CDynamicBufferSwap::GetBuffersQueuedToRead( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_buffersQueuedToRead;
}                             

/*-------------------------------------------------------------------------*/

bool 
CDynamicBufferSwap::SetNrOfBuffers( UInt32 nrOfBuffers )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    if ( (UInt32) m_buffers.size() < nrOfBuffers )
    {
        m_buffers.resize( nrOfBuffers );
        return true;
    }
    return false;
}   

/*-------------------------------------------------------------------------*/

UInt32 
CDynamicBufferSwap::GetNrOfBuffers( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_buffers.size();
} 

/*-------------------------------------------------------------------------*/

void 
CDynamicBufferSwap::SetMinimalBufferSize( UInt32 minimalBufferSize )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    TBufferEntryVector::iterator i = m_buffers.begin();
    while ( i != m_buffers.end() )
    {
        (*i).buffer.SetBufferSize( minimalBufferSize, false );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void 
CDynamicBufferSwap::GetBufferSizeRange( UInt32& smallest, UInt32& largest ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    largest = 0;
    smallest = GUCEF_MT_UINT32MAX;
    TBufferEntryVector::const_iterator i = m_buffers.begin();
    while ( i != m_buffers.end() )
    {
        UInt32 bufferSize = (*i).buffer.GetBufferSize();
        if ( smallest > bufferSize )
            smallest = bufferSize;
        if ( largest < bufferSize )
            largest = bufferSize;

        ++i;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CDynamicBufferSwap::GetCurrentReaderBufferIndex( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_currentReaderBufferIndex;
}

/*-------------------------------------------------------------------------*/

Int32
CDynamicBufferSwap::GetCurrentWriterBufferIndex( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_currentWriterBufferIndex;
}

/*-------------------------------------------------------------------------*/

CDateTime
CDynamicBufferSwap::GetCurrenWriterBufferAssociatedDt( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_writingIsOngoing && m_currentWriterBufferIndex >= 0 )
    {
        const CBufferEntry& currentBuffer = m_buffers[ m_currentWriterBufferIndex ];
        return currentBuffer.associatedDt;
    }

    return CDateTime::Empty;
}

/*-------------------------------------------------------------------------*/

CDateTime
CDynamicBufferSwap::GetCurrenReaderBufferAssociatedDt( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_readingIsOngoing && m_currentReaderBufferIndex >= 0 )
    {
        const CBufferEntry& currentBuffer = m_buffers[ m_currentReaderBufferIndex ];
        if ( currentBuffer.hasUnreadData )
        {
            return currentBuffer.associatedDt;
        }
    }

    return CDateTime::Empty;
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer*
CDynamicBufferSwap::GetCurrenReaderBuffer( CDateTime& associatedDt    ,
                                           UInt32 lockWaitTimeoutInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_readingIsOngoing && m_currentReaderBufferIndex >= 0 )
    {
        CBufferEntry& currentBuffer = m_buffers[ m_currentReaderBufferIndex ];
        if ( currentBuffer.hasUnreadData )
        {
            associatedDt = currentBuffer.associatedDt;
            return &currentBuffer.buffer;
        }
    }

    associatedDt = CDateTime::Empty;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer* 
CDynamicBufferSwap::GetNextReaderBuffer( CDateTime& associatedDt               ,
                                         bool alwaysBlockForBufferAvailability ,
                                         UInt32 lockWaitTimeoutInMs            ,
                                         Int32* currentReaderBufferIndex       )
{GUCEF_TRACE;

    if ( !SignalEndOfReading() )
    {
        associatedDt = CDateTime::Empty;

        if ( GUCEF_NULL != currentReaderBufferIndex )
            *currentReaderBufferIndex = -1;

        return GUCEF_NULL;
    }
    
    do
    {
        MT::CScopeMutex lock( m_lock );          

        if ( m_currentReaderBufferIndex < 0 )
        {
            CBufferEntry& firstBuffer = m_buffers[ 0 ];        
            if ( firstBuffer.hasUnreadData && firstBuffer.lock.Lock( lockWaitTimeoutInMs ) )
            {
                associatedDt = firstBuffer.associatedDt;
                m_currentReaderBufferIndex = 0;

                if ( GUCEF_NULL != currentReaderBufferIndex )
                    *currentReaderBufferIndex = m_currentReaderBufferIndex;

                m_readingIsOngoing = true;
                return &firstBuffer.buffer;
            }
            else
            {
                if ( alwaysBlockForBufferAvailability )
                {
                    lock.EarlyUnlock();
                    MT::PrecisionDelay( 100 );
                    continue;
                }
                else
                {
                    associatedDt = CDateTime::Empty;

                    if ( GUCEF_NULL != currentReaderBufferIndex )
                        *currentReaderBufferIndex = -1;

                    return GUCEF_NULL;
                }
            }
        }
    
        Int32 nextReaderBufferIndex = m_currentReaderBufferIndex+1;
    
        // loop around as needed
        if ( nextReaderBufferIndex >= m_buffers.size() )
            nextReaderBufferIndex = 0;

        //// Cheap early out check
        //// The reader and the writer should never be on the same index
        //if ( nextReaderBufferIndex == m_currentWriterBufferIndex )
        //{
        //    if ( alwaysBlockForBufferAvailability )
        //    {
        //        // Wait on the writer to finish a buffer
        //        lock.EarlyUnlock();
        //        MT::PrecisionDelay( 100 );
        //        continue;
        //    }
        //    else
        //        return GUCEF_NULL;
        //}

        CBufferEntry& nextBuffer = m_buffers[ nextReaderBufferIndex ];
        if ( nextBuffer.hasUnreadData && nextBuffer.lock.Lock( lockWaitTimeoutInMs ) )
        {           
            m_currentReaderBufferIndex = nextReaderBufferIndex;  
            associatedDt = nextBuffer.associatedDt;

            if ( GUCEF_NULL != currentReaderBufferIndex )
                *currentReaderBufferIndex = m_currentReaderBufferIndex;

            m_readingIsOngoing = true;
            return &nextBuffer.buffer;
        }
        else
        {
            if ( alwaysBlockForBufferAvailability )
            {
                lock.EarlyUnlock();
                MT::PrecisionDelay( 100 );
                continue;
            }
            else
            {
                associatedDt = CDateTime::Empty;

                if ( GUCEF_NULL != currentReaderBufferIndex )
                    *currentReaderBufferIndex = -1;

                return GUCEF_NULL;
            }
        }
    }
    while ( alwaysBlockForBufferAvailability );

    if ( GUCEF_NULL != currentReaderBufferIndex )
        *currentReaderBufferIndex = -1;

    associatedDt = CDateTime::Empty;
    return GUCEF_NULL;
}            

/*-------------------------------------------------------------------------*/

CDynamicBuffer* 
CDynamicBufferSwap::GetNextReaderBuffer( bool alwaysBlockForBufferAvailability ,
                                         UInt32 lockWaitTimeoutInMs            ,
                                         Int32* currentReaderBufferIndex       )
{GUCEF_TRACE;

    CDateTime associatedDt; 
    return GetNextReaderBuffer( associatedDt                     , 
                                alwaysBlockForBufferAvailability ,
                                lockWaitTimeoutInMs              ,
                                currentReaderBufferIndex         );
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer* 
CDynamicBufferSwap::PeekNextReaderBuffer( CDateTime& associatedDt               ,
                                          Int32 peekReaderBufferIndex           ,
                                          Int32* newPeekReaderBufferIndex       )

{GUCEF_TRACE;

    if ( GUCEF_NULL == newPeekReaderBufferIndex )
        return GUCEF_NULL;
    
    if ( peekReaderBufferIndex < 0 )
    {
        *newPeekReaderBufferIndex = -1;
        return GUCEF_NULL;
    }
    
    MT::CScopeMutex lock( m_lock );          
    
    if ( !m_readingIsOngoing )
    {
        // Peeking is supposed to be used in conjunction with GetNextReaderBuffer()
        *newPeekReaderBufferIndex = -1;
        return GUCEF_NULL;
    }
        
    Int32 nextReaderBufferIndex = peekReaderBufferIndex+1;
    
    // loop around as needed
    if ( nextReaderBufferIndex >= m_buffers.size() )
        nextReaderBufferIndex = 0;

    CBufferEntry& nextBuffer = m_buffers[ nextReaderBufferIndex ];
    if ( nextBuffer.hasUnreadData )
    {           
        *newPeekReaderBufferIndex = nextReaderBufferIndex;  
        associatedDt = nextBuffer.associatedDt;

        return &nextBuffer.buffer;
    }

    associatedDt = CDateTime::Empty;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer* 
CDynamicBufferSwap::PeekNextReaderBuffer( Int32 peekReaderBufferIndex     ,
                                          Int32* newPeekReaderBufferIndex )
{GUCEF_TRACE;

    CDateTime associatedDt; 
    return PeekNextReaderBuffer( associatedDt             ,
                                 peekReaderBufferIndex    ,
                                 newPeekReaderBufferIndex );
}

/*-------------------------------------------------------------------------*/

bool 
CDynamicBufferSwap::SignalEndOfReading( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( m_readingIsOngoing && m_currentReaderBufferIndex >= 0 )
    {
        CBufferEntry& currentBuffer = m_buffers[ m_currentReaderBufferIndex ];
        currentBuffer.hasUnreadData = false;
        currentBuffer.buffer.SetDataSize( 0 );
        
        if ( m_buffersQueuedToRead > 0 )
            --m_buffersQueuedToRead;        
        
        m_readingIsOngoing = false;
        return currentBuffer.lock.Unlock();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer* 
CDynamicBufferSwap::GetNextWriterBuffer( const CDateTime& associatedDt         ,
                                         bool alwaysBlockForBufferAvailability ,
                                         UInt32 lockWaitTimeoutInMs            )
{GUCEF_TRACE;

    if ( !SignalEndOfWriting() )
        return GUCEF_NULL;

    do
    {
        MT::CScopeMutex lock( m_lock );

        if ( m_currentWriterBufferIndex < 0 )
        {
            m_currentWriterBufferIndex = m_currentReaderBufferIndex + 1;
            CBufferEntry& firstBuffer = m_buffers[ m_currentWriterBufferIndex ];
        
            lock.EarlyUnlock();

            if ( firstBuffer.lock.Lock( lockWaitTimeoutInMs ) )
            {
                if ( &associatedDt != &CDateTime::Empty )
                    firstBuffer.associatedDt = associatedDt;
                else
                    firstBuffer.associatedDt = CDateTime::NowUTCDateTime();
                        
                firstBuffer.buffer.SetDataSize( 0 );
                m_writingIsOngoing = true;
                return &firstBuffer.buffer;
            }
            else
            {
                if ( alwaysBlockForBufferAvailability )
                {
                    lock.EarlyUnlock();
                    MT::PrecisionDelay( 100 );
                    continue;
                }
                else
                    return GUCEF_NULL;
            }
        }
    
        Int32 nextWriterBufferIndex = m_currentWriterBufferIndex+1;
    
        // loop around as needed
        if ( nextWriterBufferIndex == m_buffers.size() )
            nextWriterBufferIndex = 0;

        //// Cheap early out check
        //// The reader and the writer should never be on the same index
        //if ( nextWriterBufferIndex == m_currentReaderBufferIndex )
        //{
        //    if ( alwaysBlockForBufferAvailability )
        //    {
        //        // Wait on the writer to finish a buffer
        //        lock.EarlyUnlock();
        //        MT::PrecisionDelay( 100 );
        //        continue;
        //    }
        //    else
        //        return GUCEF_NULL;
        //}

        CBufferEntry& nextBuffer = m_buffers[ nextWriterBufferIndex ];

        if ( nextBuffer.hasUnreadData )
        {
            // We need to wait for the reader to keep up with the writer
            if ( alwaysBlockForBufferAvailability )
            {
                lock.EarlyUnlock();
                MT::PrecisionDelay( 100 );
                continue;
            }
            else
                return GUCEF_NULL;
        }
    
        if ( nextBuffer.lock.Lock( lockWaitTimeoutInMs ) )
        {   
            m_currentWriterBufferIndex = nextWriterBufferIndex;

            if ( &associatedDt != &CDateTime::Empty )
                nextBuffer.associatedDt = associatedDt;
            else
                nextBuffer.associatedDt = CDateTime::NowUTCDateTime();

            nextBuffer.buffer.SetDataSize( 0 );
            m_writingIsOngoing = true;
            return &nextBuffer.buffer;
        }
        else
        {
            if ( alwaysBlockForBufferAvailability )
            {
                lock.EarlyUnlock();
                MT::PrecisionDelay( 100 );
                continue;
            }
            else
                return GUCEF_NULL;
        }
    }
    while ( alwaysBlockForBufferAvailability );
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer* 
CDynamicBufferSwap::GetNextWriterBuffer( bool alwaysBlockForBufferAvailability ,
                                         UInt32 lockWaitTimeoutInMs            )
{GUCEF_TRACE;

    return GetNextWriterBuffer( CDateTime::Empty                 ,
                                alwaysBlockForBufferAvailability ,
                                lockWaitTimeoutInMs              );
}

/*-------------------------------------------------------------------------*/

bool 
CDynamicBufferSwap::SignalEndOfWriting( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( m_writingIsOngoing && m_currentWriterBufferIndex >= 0 )
    {
        CBufferEntry& currentBuffer = m_buffers[ m_currentWriterBufferIndex ];
        currentBuffer.hasUnreadData = true;

        ++m_buffersQueuedToRead;
        m_writingIsOngoing = false;

        return currentBuffer.lock.Unlock();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CDynamicBufferSwap::AsLockable(void) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CDynamicBufferSwap::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CDynamicBufferSwap::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
