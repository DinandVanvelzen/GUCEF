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

#ifndef GUCEF_MT_CTSLOTDATAEXCHANGE_H
#define GUCEF_MT_CTSLOTDATAEXCHANGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"           /* atomics */
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CTSPSCQUEUE_H
#include "gucefMT_CTSpscQueue.h"        /* queue used by the in order delivery */
#define GUCEF_MT_CTSPSCQUEUE_H
#endif /* GUCEF_MT_CTSPSCQUEUE_H ? */

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"         /* blocking of the sleeping side */
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"         /* time keeping and sleeping */
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_SLOTDATAEXCHANGEPOLICIES_H
#include "gucefMT_SlotDataExchangePolicies.h"   /* delivery and full policies */
#define GUCEF_MT_SLOTDATAEXCHANGEPOLICIES_H
#endif /* GUCEF_MT_SLOTDATAEXCHANGEPOLICIES_H ? */

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
 *  Passes values of a fixed size from one writer thread to one or more reader threads through a fixed
 *  set of slots, without locks on the fast path. How the values are delivered is chosen at compile time
 *  with policy classes (see gucefMT_SlotDataExchangePolicies.h), so no run time branch is paid for it.
 *
 *      CTSlotDataExchange< T, SSlotDeliveryAllInOrder, SSlotFullFail >         every value, in order, one reader, Write() fails when full
 *      CTSlotDataExchange< T, SSlotDeliveryAllInOrder, SSlotFullBlock >        same, Write() waits when full (with a timeout)
 *      CTSlotDataExchange< T, SSlotDeliveryLatestOnly, SSlotFullFail, N >      newest value only, N readers, nobody ever waits
 *
 *  The last parameter (MaxReaders) is only used by the latest only delivery. For the in order delivery it
 *  must be 1. The full policy is only used by the in order delivery. See TSlotDataExchangeTypes for short names.
 *
 *  General contract, for all combinations:
 *   - T is a plain value type: copyable by assignment, with a default constructor. Values are copied in
 *     and out, so keep them small. No destructor is run for the values in the slots before the exchange
 *     itself is destroyed other than the destruction of the storage.
 *   - Exactly one thread writes at any time. The writing thread may change over time as long as the
 *     hand over is ordered by something else (a join, a lock). Likewise there are at most MaxReaders
 *     reading threads (exactly one for the in order delivery).
 *   - All storage is allocated by the constructor. Reading and writing never allocate.
 *   - Everything the writer did before Write() is visible to a reader that obtains that value by Read().
 *
 *  Which of the combinations is the right one:
 *   - Commands, events or samples that must not get lost: in order. The slot count is the burst capacity.
 *   - A state that is sampled (a volume, a transform, a set of parameters): latest only.
 *
 *  This is the general variant, the primary template is declared here and only the combinations
 *  above are defined.
 */
template< typename T, typename DeliveryPolicy, typename FullPolicy = SSlotFullFail, UInt32 MaxReaders = 1 >
class CTSlotDataExchange;

/*-------------------------------------------------------------------------*/

/**
 *  In order delivery, fail when full. Built on CTSpscQueue: never locks, never waits, makes no system calls.
 *
 *  Memory ordering: everything is the ordering of the queue. Push() publishes the item with a release store
 *  of the tail index and Pop() takes it with an acquire load of that index, so the item content is
 *  visible to the reader. Pop() releases the head index which the writer acquires before it reuses the slot,
 *  so the reader is done with a slot before the writer overwrites it.
 */
template< typename T, UInt32 MaxReaders >
class CTSlotDataExchange< T, SSlotDeliveryAllInOrder, SSlotFullFail, MaxReaders >
{
    public:

    /** @param minimumSlots at least this many values can be in flight (rounded up to a power of two, at least 2) */
    explicit CTSlotDataExchange( UInt32 minimumSlots ) : m_queue( minimumSlots ) {}

    /** Writer thread only. False when all slots hold unread values, the value is then not stored */
    bool Write( const T& value )                { return m_queue.Push( value ); }

    /** Reader thread only. False when there is no unread value, 'out' is then left alone */
    bool Read( T& out )                         { return m_queue.Pop( out ); }

    /** Any thread, though the answer may be old by the time it is read. True when there are no unread values */
    bool IsEmpty( void ) const                  { return m_queue.IsEmpty(); }

    /** Any thread: true when there is at least one unread value */
    bool HasData( void ) const                  { return !m_queue.IsEmpty(); }

    /** Any thread, an estimate while the other side is busy: the number of unread values */
    UInt32 GetSize( void ) const                { return m_queue.GetSize(); }

    UInt32 GetSlotCount( void ) const           { return m_queue.GetCapacity(); }

    /** The number of unread values that can be held, which is the number of slots */
    UInt32 GetCapacity( void ) const            { return m_queue.GetCapacity(); }

    private:

    CTSlotDataExchange( const CTSlotDataExchange& src );                /**< not implemented */
    CTSlotDataExchange& operator=( const CTSlotDataExchange& src );     /**< not implemented */

    typedef char InOrderDeliveryHasOneReader[ MaxReaders == 1 ? 1 : -1 ];

    private:

    CTSpscQueue< T > m_queue;
};

/*-------------------------------------------------------------------------*/

/**
 *  In order delivery, wait when full. The fast path is that of the queue: no system calls. A thread only
 *  sleeps when it has found no room (writer) or no data (reader), and the other side only does extra work
 *  (hands over a wake up token) when the sleeper announced itself.
 *
 *  The sleeping itself is a short spin on the wake up token followed by a blocking wait on a semaphore: the
 *  token is a semaphore post, so a sleeper is woken by the operating system the moment the other side posts
 *  and nothing is polled. The only code that knows about this is CWaitHandshake below. A thread that does
 *  not need to sleep pays nothing; the other side makes a system call only when it claimed the flag of a
 *  sleeper.
 *
 *  The announce handshake, per side (the waiting side W, the other side O), all with sequential consistency:
 *
 *      W: waiting = 1;  fence;  re-check the queue;  if still no room / data: sleep until the token arrives
 *      O: change the queue;  fence;  if ( waiting ) { waiting = 0;  hand over the token }
 *
 *  This is a store followed by a load on both sides (Dekker style). With anything weaker than sequential
 *  consistency each side may read the value the other wrote before the other write became visible: W would
 *  miss the queue change and O would miss the flag, and W would sleep forever. With sequentially consistent
 *  flag operations and fences either W sees the queue change in its re-check (and does not sleep) or O sees
 *  the flag (and posts). The fence is what orders the queue update, which is a plain release store, before
 *  the flag load. The sleeper clears the flag again by exchange when it did not need to sleep; if that
 *  exchange finds the flag already cleared the other side has claimed it and a post is on its way, which
 *  the sleeper then takes. Tokens and wake ups therefore always pair up and nothing is left behind. The price of the fast path is one fence and one flag load per Write() and Read().
 */
template< typename T, UInt32 MaxReaders >
class CTSlotDataExchange< T, SSlotDeliveryAllInOrder, SSlotFullBlock, MaxReaders >
{
    public:

    /** @param minimumSlots at least this many values can be in flight (rounded up to a power of two, at least 2) */
    explicit CTSlotDataExchange( UInt32 minimumSlots ) : m_queue( minimumSlots ), m_spaceAvailable(), m_dataAvailable()
    {
        m_padBefore[ 0 ] = m_padAfter[ 0 ] = 0;
    }

    /**
     *  Writer thread only. Waits up to timeoutMs for a free slot when all are in use (0 does not wait,
     *  SlotWaitForever never gives up). False when no slot became free in time, the value is then not stored.
     */
    bool Write( const T& value, UInt32 timeoutMs )
    {
        if ( m_queue.Push( value ) )
        {
            m_dataAvailable.Notify();
            return true;
        }
        if ( 0 == timeoutMs )
            return false;

        const UInt64 start = PrecisionTickCount();
        for (;;)
        {
            m_spaceAvailable.Arm();
            if ( m_queue.Push( value ) )
            {
                m_spaceAvailable.Disarm();
                m_dataAvailable.Notify();
                return true;
            }

            if ( !m_spaceAvailable.Sleep( RemainingMs( start, timeoutMs ) ) )
            {
                // timed out, unless the reader just claimed our flag: then there is room now
                if ( m_spaceAvailable.Disarm() )
                    return false;
            }
            if ( m_queue.Push( value ) )
            {
                m_dataAvailable.Notify();
                return true;
            }
            if ( SlotWaitForever != timeoutMs && 0 == RemainingMs( start, timeoutMs ) )
                return false;
        }
    }

    /** Reader thread only. False when there is no unread value, 'out' is then left alone */
    bool Read( T& out )
    {
        if ( m_queue.Pop( out ) )
        {
            m_spaceAvailable.Notify();
            return true;
        }
        return false;
    }

    /**
     *  Reader thread only. Waits up to timeoutMs for an unread value (0 does not wait, SlotWaitForever never
     *  gives up). True when there is one to Read().
     */
    bool WaitForData( UInt32 timeoutMs )
    {
        if ( !m_queue.IsEmpty() )
            return true;
        if ( 0 == timeoutMs )
            return false;

        const UInt64 start = PrecisionTickCount();
        for (;;)
        {
            m_dataAvailable.Arm();
            if ( !m_queue.IsEmpty() )
            {
                m_dataAvailable.Disarm();
                return true;
            }

            if ( !m_dataAvailable.Sleep( RemainingMs( start, timeoutMs ) ) )
            {
                if ( m_dataAvailable.Disarm() )
                    return !m_queue.IsEmpty();
            }
            if ( !m_queue.IsEmpty() )
                return true;
            if ( SlotWaitForever != timeoutMs && 0 == RemainingMs( start, timeoutMs ) )
                return false;
        }
    }

    /** Any thread, though the answer may be old by the time it is read. True when there are no unread values */
    bool IsEmpty( void ) const                  { return m_queue.IsEmpty(); }

    /** Any thread: true when there is at least one unread value */
    bool HasData( void ) const                  { return !m_queue.IsEmpty(); }

    /** Any thread, an estimate while the other side is busy: the number of unread values */
    UInt32 GetSize( void ) const                { return m_queue.GetSize(); }

    UInt32 GetSlotCount( void ) const           { return m_queue.GetCapacity(); }

    /** The number of unread values that can be held, which is the number of slots */
    UInt32 GetCapacity( void ) const            { return m_queue.GetCapacity(); }

    private:

    CTSlotDataExchange( const CTSlotDataExchange& src );                /**< not implemented */
    CTSlotDataExchange& operator=( const CTSlotDataExchange& src );     /**< not implemented */

    typedef char InOrderDeliveryHasOneReader[ MaxReaders == 1 ? 1 : -1 ];

    /** One side of the announce handshake described above: the flag and the wake up token it guards */
    class CWaitHandshake
    {
        public:

        CWaitHandshake( void ) : m_waiting( 0 ), m_token( 0 ) {}

        /** Waiting side: announce that we are about to sleep. Re-check the condition after this */
        void Arm( void )
        {
            m_waiting.Store( 1, MEMORDER_SEQ_CST );
            AtomicThreadFence( MEMORDER_SEQ_CST );
        }

        /**
         *  Waiting side: take the announcement back. True when nobody had claimed it. False when the other
         *  side had already claimed it, in which case its token is taken here so none is left behind
         */
        bool Disarm( void )
        {
            if ( 0 == m_waiting.Exchange( 0, MEMORDER_SEQ_CST ) )
            {
                Sleep( SlotWaitForever );       // the claim is made, the post follows at once
                return false;
            }
            return true;
        }

        /**
         *  Waiting side: sleeps until a token arrives and takes it. False on timeout. SlotWaitForever is the
         *  same value as GUCEF_MT_INFINITE_LOCK_TIMEOUT, which is how the semaphore takes "no limit"
         */
        bool Sleep( UInt32 timeoutMs )
        {
            for ( UInt32 spin=0; spin<SPIN_COUNT; ++spin )
            {
                if ( 0 == m_token.TryWait() )
                    return true;
                AtomicCpuRelax();
            }
            return 0 == m_token.WaitOrTimeout( timeoutMs );
        }

        /** Other side: call after changing the queue. Costs a fence and a load, more only when somebody sleeps */
        void Notify( void )
        {
            AtomicThreadFence( MEMORDER_SEQ_CST );
            if ( 0 != m_waiting.Load( MEMORDER_SEQ_CST ) && 0 != m_waiting.Exchange( 0, MEMORDER_SEQ_CST ) )
                m_token.Post();
        }

        private:

        enum { SPIN_COUNT = 256 };

        CTAtomic< UInt32 > m_waiting;       /**< the waiting side has announced that it is going to sleep */
        CSemaphore m_token;                 /**< the other side posts this to wake the sleeper */
    };

    static UInt32 RemainingMs( UInt64 startTicks, UInt32 timeoutMs )
    {
        if ( SlotWaitForever == timeoutMs )
            return SlotWaitForever;
        const Float64 elapsed = PrecisionTimerTicksToMs( PrecisionTickCount() - startTicks );
        return elapsed >= static_cast< Float64 >( timeoutMs ) ? 0 : static_cast< UInt32 >( static_cast< Float64 >( timeoutMs ) - elapsed );
    }

    private:

    CTSpscQueue< T > m_queue;
    char m_padBefore[ GUCEF_MT_CACHE_LINE_SIZE ];
    CWaitHandshake m_spaceAvailable;        /**< the writer waits here, the reader signals */
    char m_padAfter[ GUCEF_MT_CACHE_LINE_SIZE ];
    CWaitHandshake m_dataAvailable;         /**< the reader waits here, the writer signals */
};

/*-------------------------------------------------------------------------*/

/**
 *  Latest only delivery for one reader: the classic triple buffer. Three slots, the writer owns one (w),
 *  the reader owns one (r) and the third (the middle slot) is the hand over place. One atomic word holds
 *  the index of the middle slot and a FRESH bit that says the writer put it there and the reader did not
 *  take it yet. Neither side ever waits or loops.
 *
 *  Memory ordering. Write() copies the value into slot w, then exchanges w (with FRESH) into the state
 *  and takes the old middle slot as its new w. Read() checks FRESH and, if set, exchanges r into the state
 *  (clearing FRESH) and takes the old middle slot as its new r. Both exchanges are acquire + release:
 *   - the release half publishes: the value copied into slot w is visible to whoever acquires the state
 *     word that holds w. The reader's exchange is that acquire.
 *   - the acquire half makes sure a slot handed back is no longer in use: the slot a side receives is the
 *     one the other side released when it exchanged it in, and the other side finished using it (copying
 *     from it, for the reader) before that release. So the writer never overwrites a slot while the reader
 *     still copies from it, and the reader never copies a slot that is half written.
 *  The FRESH test before the exchange is a plain acquire load: when it is clear nothing is exchanged and
 *  the reader keeps reading slot r, which only the reader owns.
 *
 *  Read() also copies the (unchanged) slot r into 'out' when there is nothing new but a value was
 *  received before, and then returns false: 'out' then equals what it held after the previous Read().
 */
template< typename T, typename FullPolicy >
class CTSlotDataExchange< T, SSlotDeliveryLatestOnly, FullPolicy, 1 >
{
    public:

    CTSlotDataExchange( void )
        : m_state( 2 )
        , m_writeSlot( 0 )
        , m_written( false )
        , m_everWritten( 0 )
        , m_readSlot( 1 )
        , m_hasValue( false )
        , m_slots( GUCEF_NULL )
    {GUCEF_TRACE;

        m_padBefore[ 0 ] = m_padMiddle[ 0 ] = m_padReader[ 0 ] = m_padAfter[ 0 ] = 0;
        m_slots = new SSlot[ 3 ];
    }

    ~CTSlotDataExchange()
    {GUCEF_TRACE;

        delete [] m_slots;
        m_slots = GUCEF_NULL;
    }

    /** Writer thread only. Never waits, never fails: an unread older value is replaced */
    void Write( const T& value )
    {
        m_slots[ m_writeSlot ].value = value;
        if ( !m_written )
        {
            m_written = true;
            m_everWritten.Store( 1, MEMORDER_RELEASE );
        }
        const UInt32 old = m_state.Exchange( m_writeSlot | FRESH, MEMORDER_ACQ_REL );
        m_writeSlot = old & INDEX_MASK;
    }

    /**
     *  Reader thread only. True when the value in 'out' is new: written since the previous Read().
     *  False when it is the same value as after the previous Read(), or, when nothing was ever written,
     *  when 'out' was left alone.
     */
    bool Read( T& out )
    {
        bool isNew = false;
        if ( 0 != ( m_state.Load( MEMORDER_ACQUIRE ) & FRESH ) )
        {
            const UInt32 old = m_state.Exchange( m_readSlot, MEMORDER_ACQ_REL );
            m_readSlot = old & INDEX_MASK;
            m_hasValue = true;
            isNew = true;
        }
        if ( m_hasValue )
            out = m_slots[ m_readSlot ].value;
        return isNew;
    }

    /** Any thread: true when a value has been written at least once */
    bool HasData( void ) const                  { return 0 != m_everWritten.Load( MEMORDER_ACQUIRE ); }

    /** Any thread: true when nothing has been written yet */
    bool IsEmpty( void ) const                  { return !HasData(); }

    /** Any thread: true when a value is waiting that no Read() has taken yet */
    bool HasNewData( void ) const               { return 0 != ( m_state.Load( MEMORDER_ACQUIRE ) & FRESH ); }

    UInt32 GetSlotCount( void ) const           { return 3; }

    /** The number of values that can be held for reading: the newest one */
    UInt32 GetCapacity( void ) const            { return 1; }

    private:

    CTSlotDataExchange( const CTSlotDataExchange& src );                /**< not implemented */
    CTSlotDataExchange& operator=( const CTSlotDataExchange& src );     /**< not implemented */

    enum
    {
        INDEX_MASK = 0x3,
        FRESH = 0x80
    };

    // padded so that neighbouring values do not share a cache line, the writer and reader work on adjacent slots
    struct SSlot
    {
        T value;
        char pad[ GUCEF_MT_CACHE_LINE_SIZE ];
        SSlot( void ) : value() { pad[ 0 ] = 0; }
    };

    private:

    char m_padBefore[ GUCEF_MT_CACHE_LINE_SIZE ];
    CTAtomic< UInt32 > m_state;                 /**< middle slot index, FRESH when the writer put it there */
    char m_padMiddle[ GUCEF_MT_CACHE_LINE_SIZE ];
    UInt32 m_writeSlot;                         /**< writer thread only */
    bool m_written;                             /**< writer thread only */
    CTAtomic< UInt32 > m_everWritten;           /**< set once by the writer, for HasData() */
    char m_padReader[ GUCEF_MT_CACHE_LINE_SIZE ];
    UInt32 m_readSlot;                          /**< reader thread only */
    bool m_hasValue;                            /**< reader thread only */
    char m_padAfter[ GUCEF_MT_CACHE_LINE_SIZE ];
    SSlot* m_slots;
};

/*-------------------------------------------------------------------------*/

/**
 *  Latest only delivery for up to MaxReaders reader threads, which copy the value out. There are
 *  MaxReaders + 2 slots. Per slot an atomic count of the readers that are copying from it and a version
 *  (the number of the write that filled it, starting at 1). One atomic 'latest' holds the index of the newest
 *  complete slot. The writer keeps a slot of its own (w) that is neither latest nor in use by a reader.
 *  Nobody ever waits; a reader only repeats an attempt when a write replaced the latest slot in the instant
 *  it was looking at it.
 *
 *  Write():   1. copy into slot w, set its version        2. latest = w (exchange)
 *             3. pick the next w: a slot that is not latest and has a reader count of zero
 *  Read():    1. i = latest; count[i]++; if latest != i then count[i]--, try again
 *             2. copy slot i and its version             3. count[i]--
 *
 *  Why a reader never copies a slot while the writer writes it: the writer only writes the slot it picked in
 *  step 3, which was not latest and had count zero. A reader only copies after its re-check found the slot
 *  to still be latest, with its own count already visible. Once the writer publishes another slot the old latest
 *  can only be picked again if its count reads zero, which cannot be while a reader copies from it.
 *  A slot always exists for step 3: each of the at most MaxReaders readers holds at most one count, one more
 *  slot is latest, and there are MaxReaders + 2 slots. (With more reader threads than MaxReaders this no
 *  longer holds: Write() then keeps looking until a slot frees up, so it only waits in a use that breaks
 *  the contract.)
 *
 *  Why sequentially consistent: the reader does "count[i]++ then load latest" and the writer does "latest
 *  exchange then load count[i]". Each side stores and then loads a different variable (Dekker style). With
 *  release / acquire alone both loads may return the old values: the reader sees i still being latest and
 *  starts copying while the writer sees count zero and starts writing the slot. In the single total order
 *  of sequentially consistent operations either the reader's increment precedes the writer's scan, so the
 *  writer sees the count and picks another slot, or the writer's exchange precedes the reader's re-check, so
 *  the reader sees that i is no longer latest and retries. The increments themselves are also sequentially
 *  consistent for that reason; the decrements only need release: they publish that the copy is done to the
 *  writer's scan, which loads with acquire at the least (it is sequentially consistent).
 *
 *  The version tells a reader whether the value is new to it: each reader keeps its own 'last seen version'.
 *  A reader that reads often gets the same version many times, and then copies nothing.
 */
template< typename T, typename FullPolicy, UInt32 MaxReaders >
class CTSlotDataExchange< T, SSlotDeliveryLatestOnly, FullPolicy, MaxReaders >
{
    public:

    CTSlotDataExchange( void )
        : m_latest( 1 )
        , m_writeSlot( 0 )
        , m_writeCounter( 0 )
        , m_everWritten( 0 )
        , m_slotCount( MaxReaders + 2 )
        , m_slots( GUCEF_NULL )
    {GUCEF_TRACE;

        m_padBefore[ 0 ] = m_padMiddle[ 0 ] = m_padAfter[ 0 ] = 0;
        m_slots = new SSlot[ m_slotCount ];
    }

    ~CTSlotDataExchange()
    {GUCEF_TRACE;

        delete [] m_slots;
        m_slots = GUCEF_NULL;
    }

    /** Writer thread only. Never waits (for a correct number of readers), never fails */
    void Write( const T& value )
    {
        SSlot& slot = m_slots[ m_writeSlot ];
        slot.value = value;
        slot.version = ++m_writeCounter;
        if ( 1 == m_writeCounter )
            m_everWritten.Store( 1, MEMORDER_RELEASE );
        m_latest.Exchange( m_writeSlot, MEMORDER_SEQ_CST );

        // The slot we just published is latest. Find one that is not, and that no reader counts itself on
        for (;;)
        {
            for ( UInt32 n=1; n<m_slotCount; ++n )
            {
                const UInt32 i = ( m_writeSlot + n ) % m_slotCount;
                if ( 0 == m_slots[ i ].readers.Load( MEMORDER_SEQ_CST ) )
                {
                    m_writeSlot = i;
                    return;
                }
            }
            AtomicCpuRelax();   // only reachable with more reader threads than MaxReaders
        }
    }

    /**
     *  Reader thread (each reader has its own lastSeenVersion, start it at 0). True when the newest value
     *  was not seen by this reader yet: 'out' and lastSeenVersion are updated. False when there is nothing
     *  new, or nothing was ever written; 'out' is then left alone.
     */
    bool Read( T& out, UInt64& lastSeenVersion )
    {
        for (;;)
        {
            const UInt32 i = m_latest.Load( MEMORDER_SEQ_CST );
            SSlot& slot = m_slots[ i ];
            slot.readers.FetchAdd( 1, MEMORDER_SEQ_CST );
            if ( m_latest.Load( MEMORDER_SEQ_CST ) != i )
            {
                slot.readers.FetchSub( 1, MEMORDER_RELEASE );
                continue;
            }

            // slot i is latest and counted: the writer will not touch it until we are done
            const UInt64 version = slot.version;
            const bool isNew = 0 != version && version != lastSeenVersion;
            if ( isNew )
            {
                out = slot.value;
                lastSeenVersion = version;
            }
            slot.readers.FetchSub( 1, MEMORDER_RELEASE );
            return isNew;
        }
    }

    /** Any thread: true when a value has been written at least once */
    bool HasData( void ) const                  { return !IsEmpty(); }

    /** Any thread: true when nothing has been written yet */
    bool IsEmpty( void ) const
    {
        return 0 == m_everWritten.Load( MEMORDER_ACQUIRE );
    }

    UInt32 GetSlotCount( void ) const           { return m_slotCount; }

    /** The number of values that can be held for reading: the newest one */
    UInt32 GetCapacity( void ) const            { return 1; }

    /** The number of reader threads this exchange was made for */
    UInt32 GetMaxReaders( void ) const          { return MaxReaders; }

    private:

    CTSlotDataExchange( const CTSlotDataExchange& src );                /**< not implemented */
    CTSlotDataExchange& operator=( const CTSlotDataExchange& src );     /**< not implemented */

    typedef char LatestOnlyNeedsAReader[ MaxReaders >= 1 ? 1 : -1 ];

    struct SSlot
    {
        T value;
        UInt64 version;                         /**< written by the writer while the slot is not latest and not counted */
        CTAtomic< UInt32 > readers;             /**< readers that are, or are about to be, copying from this slot */
        char pad[ GUCEF_MT_CACHE_LINE_SIZE ];
        SSlot( void ) : value(), version( 0 ), readers( 0 ) { pad[ 0 ] = 0; }
    };

    private:

    char m_padBefore[ GUCEF_MT_CACHE_LINE_SIZE ];
    CTAtomic< UInt32 > m_latest;                /**< index of the newest complete slot */
    char m_padMiddle[ GUCEF_MT_CACHE_LINE_SIZE ];
    UInt32 m_writeSlot;                         /**< writer thread only */
    UInt64 m_writeCounter;                      /**< writer thread only */
    CTAtomic< UInt32 > m_everWritten;           /**< set once by the writer, for HasData() */
    char m_padAfter[ GUCEF_MT_CACHE_LINE_SIZE ];
    UInt32 m_slotCount;
    SSlot* m_slots;
};

/*-------------------------------------------------------------------------*/

/**
 *  Short names for the common combinations, as C++98 has no alias templates:
 *      TSlotDataExchangeTypes< float >::TLatest                      newest value, one reader
 *      TSlotDataExchangeTypes< float, 4 >::TLatest                   newest value, up to 4 readers
 *      TSlotDataExchangeTypes< Command >::TInOrderFail               every value in order, Write() fails when full
 *      TSlotDataExchangeTypes< Command >::TInOrderBlock              every value in order, Write() waits when full
 */
template< typename T, UInt32 MaxReaders = 1 >
struct TSlotDataExchangeTypes
{
    typedef CTSlotDataExchange< T, SSlotDeliveryAllInOrder, SSlotFullFail >                     TInOrderFail;
    typedef CTSlotDataExchange< T, SSlotDeliveryAllInOrder, SSlotFullBlock >                    TInOrderBlock;
    typedef CTSlotDataExchange< T, SSlotDeliveryLatestOnly, SSlotFullFail, MaxReaders >         TLatest;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_CTSLOTDATAEXCHANGE_H ? */
