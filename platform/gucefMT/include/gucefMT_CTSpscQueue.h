/*
 *  gucefMT: GUCEF module providing multithreading solutions
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

#ifndef GUCEF_MT_CTSPSCQUEUE_H
#define GUCEF_MT_CTSPSCQUEUE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"           /* atomics */
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A queue between exactly one thread that puts items in and exactly one thread that takes them out.
 *  It never locks, never allocates after construction and never waits, which is what a real time thread
 *  (audio, a driver callback) needs to talk to the rest of a program. For more producers or consumers
 *  serialize them with a lock on their side of the queue.
 *
 *  The size is fixed at construction, rounded up to a power of two. Push() on a full queue and Pop() on an
 *  empty queue say so by returning false, the caller decides what to do about it.
 *
 *  T must be copyable and have a default constructor. Items are copied in and out, so keep them small
 *  (a handful of numbers or a pointer), and note that no destructor is run for items still in the queue
 *  before the queue itself is destroyed other than the destruction of the storage.
 *
 *  The ordering guarantee: everything the producer wrote before Push() is visible to the consumer
 *  that Pop()s the item.
 */
template< typename T >
class CTSpscQueue
{
    public:

    /** @param minimumCapacity the queue holds at least this many items (at least 2 are made) */
    explicit CTSpscQueue( UInt32 minimumCapacity );

    ~CTSpscQueue();

    /** Producer thread only. False when the queue is full */
    bool Push( const T& item );

    /** Consumer thread only. False when the queue is empty */
    bool Pop( T& item );

    /** Consumer thread only: looks at the next item without taking it. False when the queue is empty */
    bool Peek( T& item ) const;

    /** Any thread, though the answer may be old by the time it is read */
    bool IsEmpty( void ) const;

    /** Any thread, an estimate while the other side is busy */
    UInt32 GetSize( void ) const;

    UInt32 GetCapacity( void ) const;

    private:

    CTSpscQueue( const CTSpscQueue& src );              /**< not implemented */
    CTSpscQueue& operator=( const CTSpscQueue& src );   /**< not implemented */

    private:

    // The two indices are written by different threads: each sits on a cache line of its own so they do not slow each other down
    char m_padBefore[ GUCEF_MT_CACHE_LINE_SIZE ];
    CTAtomic< UInt32 > m_head;                              /**< next item to take, written by the consumer */
    char m_padMiddle[ GUCEF_MT_CACHE_LINE_SIZE ];
    CTAtomic< UInt32 > m_tail;                              /**< next place to put an item, written by the producer */
    char m_padAfter[ GUCEF_MT_CACHE_LINE_SIZE ];

    T* m_items;
    UInt32 m_capacity;
    UInt32 m_mask;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T >
CTSpscQueue< T >::CTSpscQueue( UInt32 minimumCapacity )
    : m_head( 0 )
    , m_tail( 0 )
    , m_items( GUCEF_NULL )
    , m_capacity( 2 )
    , m_mask( 1 )
{GUCEF_TRACE;

    m_padBefore[ 0 ] = m_padMiddle[ 0 ] = m_padAfter[ 0 ] = 0;
    while ( m_capacity < minimumCapacity && m_capacity < 0x80000000U )
        m_capacity <<= 1;
    m_mask = m_capacity - 1;
    m_items = new T[ m_capacity ];
}

/*-------------------------------------------------------------------------*/

template< typename T >
CTSpscQueue< T >::~CTSpscQueue()
{GUCEF_TRACE;

    delete [] m_items;
    m_items = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

template< typename T >
bool
CTSpscQueue< T >::Push( const T& item )
{
    // no trace macro in this class: it is meant for threads where even that is too much
    const UInt32 tail = m_tail.Load( MEMORDER_RELAXED );
    const UInt32 head = m_head.Load( MEMORDER_ACQUIRE );
    if ( tail - head >= m_capacity )
        return false;

    m_items[ tail & m_mask ] = item;
    m_tail.Store( tail + 1, MEMORDER_RELEASE );
    return true;
}

/*-------------------------------------------------------------------------*/

template< typename T >
bool
CTSpscQueue< T >::Pop( T& item )
{
    const UInt32 head = m_head.Load( MEMORDER_RELAXED );
    const UInt32 tail = m_tail.Load( MEMORDER_ACQUIRE );
    if ( head == tail )
        return false;

    item = m_items[ head & m_mask ];
    m_head.Store( head + 1, MEMORDER_RELEASE );
    return true;
}

/*-------------------------------------------------------------------------*/

template< typename T >
bool
CTSpscQueue< T >::Peek( T& item ) const
{
    const UInt32 head = m_head.Load( MEMORDER_RELAXED );
    const UInt32 tail = m_tail.Load( MEMORDER_ACQUIRE );
    if ( head == tail )
        return false;

    item = m_items[ head & m_mask ];
    return true;
}

/*-------------------------------------------------------------------------*/

template< typename T >
bool
CTSpscQueue< T >::IsEmpty( void ) const
{
    return m_head.Load( MEMORDER_ACQUIRE ) == m_tail.Load( MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

template< typename T >
UInt32
CTSpscQueue< T >::GetSize( void ) const
{
    const UInt32 head = m_head.Load( MEMORDER_ACQUIRE );
    const UInt32 tail = m_tail.Load( MEMORDER_ACQUIRE );
    return tail - head;
}

/*-------------------------------------------------------------------------*/

template< typename T >
UInt32
CTSpscQueue< T >::GetCapacity( void ) const
{
    return m_capacity;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_CTSPSCQUEUE_H ? */
