/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#include <math.h>

#include "gucefAUDIO_CVoiceJitterBuffer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 SequenceModulo = 65536;
const UInt32 NoBadSequence = SequenceModulo + 1;

/** Extended timestamps start one cycle up, so a frame before the first one never goes below 0 */
const UInt64 TimestampBase = (UInt64) 1 << 32;

/** A timestamp step back of the newest frame after an arrival gap of more than this many timestamp units is taken as a wrap forward */
const Float64 LongIdleInTimestampUnits = 1073741824.0;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVoiceJitterBuffer::SFrameInfo::SFrameInfo( void )
    : frame( GUCEF_NULL )
    , extendedSequenceNumber( 0 )
    , extendedTimestamp( 0 )
    , arrivalTimeInUs( 0 )
    , isRedundant( false )
{

}

/*-------------------------------------------------------------------------*/

CVoiceJitterBuffer::SPullInfo::SPullInfo( void )
    : frame()
    , latenessInTimestampUnits( 0 )
    , skippedFrames( 0 )
    , lateDiscarded( 0 )
    , maxLatenessDiscarded( 0 )
{

}

/*-------------------------------------------------------------------------*/

CVoiceJitterBuffer::SInsertInfo::SInsertInfo( void )
    : extendedSequenceNumber( 0 )
    , extendedTimestamp( 0 )
    , redundantStored( false )
{

}

/*-------------------------------------------------------------------------*/

CVoiceJitterBuffer::SStatistics::SStatistics( void )
    : framesReceived( 0 )
    , framesExpected( 0 )
    , cumulativeLost( 0 )
    , late( 0 )
    , duplicates( 0 )
    , reordered( 0 )
    , redundantStored( 0 )
    , redundantUnused( 0 )
    , overflowDropped( 0 )
    , restarts( 0 )
    , probationDropped( 0 )
    , talkspurtEnds( 0 )
    , extendedHighestSequenceNumber( 0 )
    , interarrivalJitter( 0.0 )
    , bufferedFrames( 0 )
{

}

/*-------------------------------------------------------------------------*/

CVoiceJitterBuffer::CVoiceJitterBuffer( void )
    : m_slots()
    , m_seen()
    , m_slotMask( 0 )
    , m_timestampUnitsPerFrame( 0 )
    , m_timestampRate( 0 )
    , m_streamStarted( false )
    , m_maxSeq( 0 )
    , m_cycles( 0 )
    , m_badSeq( NoBadSequence )
    , m_baseExtSeq( 0 )
    , m_maxExtTs( 0 )
    , m_lastArrivalTimeInUs( 0 )
    , m_epochReceived( 0 )
    , m_hasProbationFrame( false )
    , m_probationFrame()
    , m_probationArrivalTimeInUs( 0 )
    , m_hasTransit( false )
    , m_lastTransit( 0.0 )
    , m_jitter( 0.0 )
    , m_storedCount( 0 )
    , m_lowestStoredExtSeq( 0 )
    , m_highestStoredExtSeq( 0 )
    , m_highestStoredEndTs( 0 )
    , m_hasCursor( false )
    , m_playing( false )
    , m_nextExtSeq( 0 )
    , m_nextExtTs( 0 )
    , m_priorExpected( 0 )
    , m_priorReceived( 0 )
    , m_stats()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoiceJitterBuffer::~CVoiceJitterBuffer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CVoiceJitterBuffer::Prepare( UInt32 slotCount, UInt32 timestampUnitsPerFrame, UInt32 timestampRate )
{GUCEF_TRACE;

    if ( slotCount < 2 || slotCount > MaxSlotCount || 0 != ( slotCount & ( slotCount - 1 ) ) )
        return false;
    if ( 0 == timestampUnitsPerFrame || 0 == timestampRate )
        return false;

    SSlot empty;
    empty.extendedSequenceNumber = 0;
    empty.extendedTimestamp = 0;
    empty.arrivalTimeInUs = 0;
    empty.stored = false;
    empty.isRedundant = false;
    m_slots.assign( slotCount, empty );
    m_seen.assign( slotCount, 0 );
    m_slotMask = slotCount - 1;
    m_timestampUnitsPerFrame = timestampUnitsPerFrame;
    m_timestampRate = timestampRate;
    Reset();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceJitterBuffer::IsPrepared( void ) const
{
    return !m_slots.empty();
}

/*-------------------------------------------------------------------------*/

void
CVoiceJitterBuffer::Reset( void )
{
    m_stats = SStatistics();
    m_priorExpected = 0;
    m_priorReceived = 0;
    ClearStream();
}

/*-------------------------------------------------------------------------*/

void
CVoiceJitterBuffer::ClearStream( void )
{
    const UInt32 slotCount = (UInt32) m_slots.size();
    for ( UInt32 i=0; i<slotCount; ++i )
    {
        m_slots[ i ].stored = false;
        m_slots[ i ].isRedundant = false;
        m_seen[ i ] = 0;
    }
    m_streamStarted = false;
    m_maxSeq = 0;
    m_cycles = 0;
    m_badSeq = NoBadSequence;
    m_baseExtSeq = 0;
    m_maxExtTs = 0;
    m_lastArrivalTimeInUs = 0;
    m_epochReceived = 0;
    m_hasProbationFrame = false;
    m_probationArrivalTimeInUs = 0;
    m_hasTransit = false;
    m_lastTransit = 0.0;
    m_jitter = 0.0;
    m_storedCount = 0;
    m_lowestStoredExtSeq = 0;
    m_highestStoredExtSeq = 0;
    m_highestStoredEndTs = 0;
    m_hasCursor = false;
    m_playing = false;
    m_nextExtSeq = 0;
    m_nextExtTs = 0;
}

/*-------------------------------------------------------------------------*/

void
CVoiceJitterBuffer::InitSequence( UInt16 sequenceNumber, UInt32 timestamp )
{
    m_streamStarted = true;
    m_maxSeq = sequenceNumber;
    m_cycles = SequenceModulo;
    m_badSeq = NoBadSequence;
    m_baseExtSeq = m_cycles + sequenceNumber;
    m_maxExtTs = TimestampBase + timestamp;
    m_epochReceived = 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CVoiceJitterBuffer::ExtendTimestamp( UInt32 timestamp, bool isNewHighest, UInt64 arrivalTimeInUs ) const
{
    const UInt32 reference = (UInt32) m_maxExtTs;
    const Int32 delta = (Int32) ( timestamp - reference );
    if ( isNewHighest && delta < 0 && arrivalTimeInUs > m_lastArrivalTimeInUs )
    {
        // after a long silence the newest frame may be more than half the timestamp range ahead: wrap forward
        const Float64 idle = (Float64) ( arrivalTimeInUs - m_lastArrivalTimeInUs ) * (Float64) m_timestampRate / 1000000.0;
        if ( idle > LongIdleInTimestampUnits )
            return m_maxExtTs + (UInt64) (UInt32) ( timestamp - reference );
    }
    const Int64 extended = (Int64) m_maxExtTs + (Int64) delta;
    return extended > 0 ? (UInt64) extended : 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CVoiceJitterBuffer::GetWindowStart( void ) const
{
    if ( m_hasCursor )
        return m_nextExtSeq;
    return m_lowestStoredExtSeq;
}

/*-------------------------------------------------------------------------*/

void
CVoiceJitterBuffer::UpdateJitter( UInt64 extendedTimestamp, UInt64 arrivalTimeInUs )
{
    // RFC 3550 6.4.1: D = ( Rj - Ri ) - ( Sj - Si ), J += ( |D| - J ) / 16, everything in timestamp units
    const Float64 arrival = (Float64) arrivalTimeInUs * (Float64) m_timestampRate / 1000000.0;
    const Float64 transit = arrival - (Float64) extendedTimestamp;
    if ( m_hasTransit )
    {
        const Float64 d = fabs( transit - m_lastTransit );
        m_jitter += ( d - m_jitter ) / 16.0;
    }
    m_lastTransit = transit;
    m_hasTransit = true;
}

/*-------------------------------------------------------------------------*/

void
CVoiceJitterBuffer::Store( const CVoiceFrame& frame, UInt64 extSeq, UInt64 extTs, UInt64 arrivalTimeInUs, bool isRedundant )
{
    SSlot& slot = m_slots[ (UInt32) ( extSeq & m_slotMask ) ];
    if ( &slot.frame != &frame )
        slot.frame = frame;
    slot.extendedSequenceNumber = extSeq;
    slot.extendedTimestamp = extTs;
    slot.arrivalTimeInUs = arrivalTimeInUs;
    slot.stored = true;
    slot.isRedundant = isRedundant;

    if ( 0 == m_storedCount )
    {
        m_lowestStoredExtSeq = extSeq;
        m_highestStoredExtSeq = extSeq;
        m_highestStoredEndTs = extTs + m_timestampUnitsPerFrame;
    }
    else
    {
        if ( extSeq < m_lowestStoredExtSeq )
            m_lowestStoredExtSeq = extSeq;
        if ( extSeq > m_highestStoredExtSeq )
        {
            m_highestStoredExtSeq = extSeq;
            m_highestStoredEndTs = extTs + m_timestampUnitsPerFrame;
        }
    }
    ++m_storedCount;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceJitterBuffer::FindStoredFrom( UInt64 fromExtSeq, UInt64& extSeq ) const
{
    if ( 0 == m_storedCount || fromExtSeq > m_highestStoredExtSeq )
        return false;
    UInt64 e = fromExtSeq < m_lowestStoredExtSeq ? m_lowestStoredExtSeq : fromExtSeq;
    for ( ; e <= m_highestStoredExtSeq; ++e )
    {
        const SSlot& slot = m_slots[ (UInt32) ( e & m_slotMask ) ];
        if ( slot.stored && slot.extendedSequenceNumber == e )
        {
            extSeq = e;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CVoiceJitterBuffer::RemoveFromSlot( SSlot& slot )
{
    // the frame data stays in the slot until it is overwritten, so a pointer handed out stays valid until the next call
    slot.stored = false;
    --m_storedCount;
    if ( 0 == m_storedCount )
        return;

    const UInt64 removed = slot.extendedSequenceNumber;
    if ( removed == m_lowestStoredExtSeq )
    {
        UInt64 next = 0;
        if ( FindStoredFrom( removed + 1, next ) )
            m_lowestStoredExtSeq = next;
    }
    if ( removed == m_highestStoredExtSeq )
    {
        // rare: only an eviction or a discard out of order takes the newest frame while others stay
        UInt64 e = removed;
        while ( e > m_lowestStoredExtSeq )
        {
            --e;
            const SSlot& candidate = m_slots[ (UInt32) ( e & m_slotMask ) ];
            if ( candidate.stored && candidate.extendedSequenceNumber == e )
            {
                m_highestStoredExtSeq = e;
                m_highestStoredEndTs = candidate.extendedTimestamp + m_timestampUnitsPerFrame;
                break;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVoiceJitterBuffer::MoveWindow( UInt64 newWindowStart )
{
    bool cursorMoved = false;
    const UInt32 slotCount = (UInt32) m_slots.size();
    for ( UInt32 i=0; i<slotCount && m_storedCount > 0; ++i )
    {
        SSlot& slot = m_slots[ i ];
        if ( slot.stored && slot.extendedSequenceNumber < newWindowStart )
        {
            RemoveFromSlot( slot );
            ++m_stats.overflowDropped;
        }
    }
    if ( m_hasCursor && m_nextExtSeq < newWindowStart )
    {
        m_nextExtTs += ( newWindowStart - m_nextExtSeq ) * (UInt64) m_timestampUnitsPerFrame;
        m_nextExtSeq = newWindowStart;
        m_playing = false;
        cursorMoved = true;
    }
    return cursorMoved;
}

/*-------------------------------------------------------------------------*/

void
CVoiceJitterBuffer::FillInfo( const SSlot& slot, SFrameInfo& info ) const
{
    info.frame = &slot.frame;
    info.extendedSequenceNumber = slot.extendedSequenceNumber;
    info.extendedTimestamp = slot.extendedTimestamp;
    info.arrivalTimeInUs = slot.arrivalTimeInUs;
    info.isRedundant = slot.isRedundant;
}

/*-------------------------------------------------------------------------*/

CVoiceJitterBuffer::EInsertResult
CVoiceJitterBuffer::Insert( const CVoiceFrame& frame, UInt64 arrivalTimeInUs )
{
    SInsertInfo info;
    return Insert( frame, arrivalTimeInUs, info );
}

/*-------------------------------------------------------------------------*/

CVoiceJitterBuffer::EInsertResult
CVoiceJitterBuffer::Insert( const CVoiceFrame& frame, UInt64 arrivalTimeInUs, SInsertInfo& info )
{
    info = SInsertInfo();
    if ( m_slots.empty() )
        return INSERT_NOT_PREPARED;
    return InsertPrimary( frame, arrivalTimeInUs, info, true );
}

/*-------------------------------------------------------------------------*/

CVoiceJitterBuffer::EInsertResult
CVoiceJitterBuffer::InsertPrimary( const CVoiceFrame& frame, UInt64 arrivalTimeInUs, SInsertInfo& info, bool countReceived )
{
    const UInt16 seq = frame.GetSequenceNumber();
    UInt64 extSeq = 0;
    bool isNewHighest = false;

    // RFC 3550 appendix A.1
    if ( !m_streamStarted )
    {
        InitSequence( seq, frame.GetTimestamp() );
        extSeq = m_cycles + seq;
        isNewHighest = true;
    }
    else
    {
        const UInt16 udelta = (UInt16) ( seq - m_maxSeq );
        if ( 0 == udelta )
        {
            extSeq = m_cycles + seq;
        }
        else
        if ( udelta < MaxDropout )
        {
            // in order, with a permissible gap
            if ( seq < m_maxSeq )
                m_cycles += SequenceModulo;
            m_maxSeq = seq;
            extSeq = m_cycles + seq;
            isNewHighest = true;
        }
        else
        if ( udelta <= SequenceModulo - MaxMisorder )
        {
            // a large jump: a restart when this frame follows the one held on probation
            if ( m_hasProbationFrame && (UInt32) seq == m_badSeq )
            {
                m_priorExpected += ( m_cycles + m_maxSeq ) - m_baseExtSeq + 1;
                m_priorReceived += m_epochReceived;
                ClearStream();
                ++m_stats.restarts;

                SInsertInfo heldInfo;
                InsertPrimary( m_probationFrame, m_probationArrivalTimeInUs, heldInfo, true );
                InsertPrimary( frame, arrivalTimeInUs, info, countReceived );
                return INSERT_RESTARTED;
            }
            if ( m_hasProbationFrame )
                ++m_stats.probationDropped;
            m_badSeq = (UInt32) ( ( (UInt32) seq + 1 ) & ( SequenceModulo - 1 ) );
            m_probationFrame = frame;
            m_probationArrivalTimeInUs = arrivalTimeInUs;
            m_hasProbationFrame = true;
            return INSERT_PROBATION;
        }
        else
        {
            // older than the newest, within MAX_MISORDER: a late, reordered or duplicate frame
            extSeq = seq <= m_maxSeq ? m_cycles + seq : m_cycles - SequenceModulo + seq;
        }
    }

    const UInt64 extTs = ExtendTimestamp( frame.GetTimestamp(), isNewHighest, arrivalTimeInUs );
    if ( isNewHighest )
        m_maxExtTs = extTs;
    if ( arrivalTimeInUs > m_lastArrivalTimeInUs )
        m_lastArrivalTimeInUs = arrivalTimeInUs;
    info.extendedSequenceNumber = extSeq;
    info.extendedTimestamp = extTs;

    const UInt32 slotCount = (UInt32) m_slots.size();
    const UInt32 index = (UInt32) ( extSeq & m_slotMask );
    if ( m_seen[ index ] == extSeq )
    {
        ++m_stats.duplicates;
        if ( frame.HasRedundantBlock() )
            ++m_stats.redundantUnused;
        return INSERT_DUPLICATE;
    }

    if ( countReceived )
    {
        ++m_stats.framesReceived;
        ++m_epochReceived;
        UpdateJitter( extTs, arrivalTimeInUs );
    }
    if ( extSeq > m_seen[ index ] )
        m_seen[ index ] = extSeq;

    const bool reordered = extSeq < m_cycles + m_maxSeq;
    if ( reordered )
        ++m_stats.reordered;

    // late: behind the playout cursor in sequence or in time
    if ( m_hasCursor && ( extSeq < m_nextExtSeq || extTs < m_nextExtTs ) )
    {
        ++m_stats.late;
        if ( frame.HasRedundantBlock() )
            ++m_stats.redundantUnused;
        return INSERT_LATE;
    }

    UInt64 windowStart = extSeq;
    if ( m_hasCursor )
        windowStart = m_nextExtSeq;
    else
    if ( m_storedCount > 0 )
    {
        if ( extSeq < m_lowestStoredExtSeq )
        {
            // before playout the window may reach back, as far as the newest frame allows
            if ( m_highestStoredExtSeq - extSeq >= slotCount )
            {
                ++m_stats.late;
                return INSERT_LATE;
            }
        }
        else
            windowStart = m_lowestStoredExtSeq;
    }

    EInsertResult result = reordered ? INSERT_REORDERED : INSERT_STORED;
    if ( extSeq >= windowStart + slotCount )
    {
        MoveWindow( extSeq - slotCount + 1 );
        result = INSERT_OVERFLOW;
    }

    SSlot& slot = m_slots[ index ];
    if ( slot.stored )
    {
        // only a redundant copy of this very frame can be there: the primary takes its place
        RemoveFromSlot( slot );
    }
    Store( frame, extSeq, extTs, arrivalTimeInUs, false );

    // a frame from before the first one, taken before the playout started: the stream starts here for RFC 3550 A.3
    if ( extSeq < m_baseExtSeq )
        m_baseExtSeq = extSeq;

    if ( frame.HasRedundantBlock() )
        InsertRedundant( frame, extSeq, extTs, arrivalTimeInUs, info );

    return result;
}

/*-------------------------------------------------------------------------*/

void
CVoiceJitterBuffer::InsertRedundant( const CVoiceFrame& primary, UInt64 primaryExtSeq, UInt64 primaryExtTs, UInt64 arrivalTimeInUs, SInsertInfo& info )
{
    const UInt64 offset = primary.GetRedundantTimestampOffset();
    if ( primaryExtSeq < 1 || primaryExtTs < offset )
    {
        ++m_stats.redundantUnused;
        return;
    }
    const UInt64 extSeq = primaryExtSeq - 1;
    const UInt64 extTs = primaryExtTs - offset;
    const UInt32 index = (UInt32) ( extSeq & m_slotMask );
    SSlot& slot = m_slots[ index ];

    bool usable = m_seen[ index ] != extSeq && !slot.stored;
    if ( usable && m_hasCursor && ( extSeq < m_nextExtSeq || extTs < m_nextExtTs ) )
        usable = false;
    if ( usable && !m_hasCursor && m_highestStoredExtSeq - extSeq >= (UInt64) m_slots.size() )
        usable = false;
    if ( !usable )
    {
        ++m_stats.redundantUnused;
        return;
    }

    // made in place in the slot: no frame copy on the stack
    slot.frame.Clear();
    slot.frame.SetStreamId( primary.GetStreamId() );
    slot.frame.SetSequenceNumber( (UInt16) ( primary.GetSequenceNumber() - 1 ) );
    slot.frame.SetTimestamp( (UInt32) extTs );
    slot.frame.SetMarker( false );
    slot.frame.SetPayloadType( primary.GetRedundantPayloadType() );
    slot.frame.SetPayload( primary.GetRedundantPayload(), primary.GetRedundantSize() );
    Store( slot.frame, extSeq, extTs, arrivalTimeInUs, true );
    ++m_stats.redundantStored;
    info.redundantStored = true;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceJitterBuffer::StartPlayout( void )
{
    if ( 0 == m_storedCount )
        return false;
    const SSlot& oldest = m_slots[ (UInt32) ( m_lowestStoredExtSeq & m_slotMask ) ];
    m_nextExtSeq = oldest.extendedSequenceNumber;
    m_nextExtTs = oldest.extendedTimestamp;
    m_hasCursor = true;
    m_playing = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVoiceJitterBuffer::StopPlayout( void )
{
    m_playing = false;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceJitterBuffer::IsPlaying( void ) const
{
    return m_playing;
}

/*-------------------------------------------------------------------------*/

CVoiceJitterBuffer::EPullResult
CVoiceJitterBuffer::Pull( UInt64 playoutTimestamp, SPullInfo& info )
{
    return Pull( playoutTimestamp, m_nextExtTs, info );
}

/*-------------------------------------------------------------------------*/

CVoiceJitterBuffer::EPullResult
CVoiceJitterBuffer::Pull( UInt64 playoutTimestamp, UInt64 oldestWantedTimestamp, SPullInfo& info )
{
    info = SPullInfo();
    if ( !m_playing )
        return PULL_NOT_PLAYING;

    // bounded: every pass either returns or removes a stored frame
    for ( ;; )
    {
        UInt64 extSeq = 0;
        if ( !FindStoredFrom( m_nextExtSeq, extSeq ) )
            return PULL_EMPTY;

        SSlot& slot = m_slots[ (UInt32) ( extSeq & m_slotMask ) ];
        if ( slot.extendedTimestamp < m_nextExtTs || slot.extendedTimestamp < oldestWantedTimestamp )
        {
            // its time has passed and the caller does not want it any more (or the sender stepped back in time)
            ++m_stats.late;
            ++info.lateDiscarded;
            if ( playoutTimestamp > slot.extendedTimestamp && playoutTimestamp - slot.extendedTimestamp > info.maxLatenessDiscarded )
                info.maxLatenessDiscarded = playoutTimestamp - slot.extendedTimestamp;
            RemoveFromSlot( slot );
            continue;
        }

        const UInt64 missingSequenceNumbers = extSeq - m_nextExtSeq;
        const UInt64 timestampGap = slot.extendedTimestamp - m_nextExtTs;
        if ( timestampGap > missingSequenceNumbers * (UInt64) m_timestampUnitsPerFrame )
        {
            // more time than the missing sequence numbers can fill: the sender paused, a new talkspurt starts here
            ++m_stats.talkspurtEnds;
            m_playing = false;
            return PULL_TALKSPURT_END;
        }

        if ( slot.extendedTimestamp <= playoutTimestamp )
        {
            FillInfo( slot, info.frame );
            info.latenessInTimestampUnits = playoutTimestamp - slot.extendedTimestamp;
            info.skippedFrames = missingSequenceNumbers;
            m_nextExtSeq = extSeq + 1;
            m_nextExtTs = slot.extendedTimestamp + m_timestampUnitsPerFrame;
            RemoveFromSlot( slot );
            return PULL_FRAME;
        }

        if ( slot.extendedTimestamp == playoutTimestamp + m_timestampUnitsPerFrame )
            FillInfo( slot, info.frame );
        return PULL_MISSING;
    }
}

/*-------------------------------------------------------------------------*/

bool
CVoiceJitterBuffer::PeekOldest( SFrameInfo& info ) const
{
    if ( 0 == m_storedCount )
        return false;
    FillInfo( m_slots[ (UInt32) ( m_lowestStoredExtSeq & m_slotMask ) ], info );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceJitterBuffer::PeekNext( UInt64 extendedSequenceNumber, SFrameInfo& info ) const
{
    UInt64 extSeq = 0;
    if ( !FindStoredFrom( extendedSequenceNumber + 1, extSeq ) )
        return false;
    FillInfo( m_slots[ (UInt32) ( extSeq & m_slotMask ) ], info );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceJitterBuffer::GetStoredFrameCount( void ) const
{
    return m_storedCount;
}

/*-------------------------------------------------------------------------*/

UInt64
CVoiceJitterBuffer::GetBufferedEndTimestamp( void ) const
{
    return m_storedCount > 0 ? m_highestStoredEndTs : 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CVoiceJitterBuffer::GetNextTimestamp( void ) const
{
    return m_nextExtTs;
}

/*-------------------------------------------------------------------------*/

UInt64
CVoiceJitterBuffer::GetNextSequenceNumber( void ) const
{
    return m_nextExtSeq;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceJitterBuffer::HasCursor( void ) const
{
    return m_hasCursor;
}

/*-------------------------------------------------------------------------*/

Float64
CVoiceJitterBuffer::GetInterarrivalJitter( void ) const
{
    return m_jitter;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceJitterBuffer::GetSlotCount( void ) const
{
    return (UInt32) m_slots.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceJitterBuffer::GetTimestampUnitsPerFrame( void ) const
{
    return m_timestampUnitsPerFrame;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceJitterBuffer::GetTimestampRate( void ) const
{
    return m_timestampRate;
}

/*-------------------------------------------------------------------------*/

void
CVoiceJitterBuffer::GetStatistics( SStatistics& stats ) const
{
    stats = m_stats;
    const UInt64 epochExpected = m_streamStarted ? ( m_cycles + m_maxSeq ) - m_baseExtSeq + 1 : 0;
    stats.framesExpected = m_priorExpected + epochExpected;
    stats.cumulativeLost = (Int64) stats.framesExpected - (Int64) ( m_priorReceived + m_epochReceived );
    stats.extendedHighestSequenceNumber = m_streamStarted ? m_cycles + m_maxSeq : 0;
    stats.interarrivalJitter = m_jitter;
    stats.bufferedFrames = m_storedCount;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
