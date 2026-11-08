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

#ifndef GUCEF_AUDIO_CVOICEJITTERBUFFER_H
#define GUCEF_AUDIO_CVOICEJITTERBUFFER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

#ifndef GUCEF_AUDIO_CVOICEFRAME_H
#include "gucefAUDIO_CVoiceFrame.h"
#define GUCEF_AUDIO_CVOICEFRAME_H
#endif /* GUCEF_AUDIO_CVOICEFRAME_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The receive buffer of one voice stream: frames as they come from the network, in any order, with their arrival
 *  time, handed out in playout order by the playout control (CVoicePlayoutController).
 *
 *  Storage: a ring of preallocated frame slots (a power of two, default 64 = 640 ms of 10 ms frames) indexed by the
 *  extended sequence number. Sequence numbers are extended to 64 bits after RFC 3550 appendix A.1 (cycle count,
 *  MAX_DROPOUT 3000, MAX_MISORDER 100); a jump beyond those limits is held on probation and restarts the stream when
 *  the next frame follows it (two sequential frames), otherwise it is dropped. Timestamps are extended to 64 bits
 *  against the timestamp of the newest frame, so wrap-around of either number never matters to the caller.
 *
 *  Insert:
 *  - a frame already received (stored or handed out) is a duplicate and dropped
 *  - a frame older than the playout cursor (its sequence number or its timestamp is behind the frame handed out
 *    last) is late and dropped
 *  - a frame beyond the window of slots moves the window: the frames it pushes out are dropped (counted), playout stops
 *  - otherwise the frame is stored; it counts as reordered when a frame with a higher sequence number came before it
 *  - an RFC 2198 redundant block that travels with a frame is stored as the frame before it (sequence number one
 *    lower, timestamp minus the offset) when its slot is empty and it is not late; a primary frame that arrives later
 *    replaces it
 *
 *  Playout: StartPlayout() puts the cursor on the oldest stored frame; Pull( playoutTimestamp ) hands out the frame
 *  due at that timestamp, says that it is missing while a newer frame of the same talkspurt is there, that nothing is
 *  there, or that the next frame starts a new talkspurt (its timestamp is further ahead than its sequence number
 *  accounts for: the sender paused). A frame whose time has passed but which is newer than the frame handed out last
 *  is still handed out (the caller concealed in its place and plays it late, stretching the playout).
 *
 *  Statistics after RFC 3550: frames received, expected and lost (A.3), interarrival jitter (6.4.1, A.8), plus late,
 *  duplicate, reordered, redundancy and overflow counts.
 *
 *  Threading: Prepare() allocates and belongs to the owner thread. Everything else allocates nothing, takes no lock
 *  and takes a bounded time (at most one pass over the slots), so the render thread can do it. Not thread safe: one
 *  thread at a time (the design has the render thread drain an inbound queue into this buffer).
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceJitterBuffer
{
    public:

    static const UInt32 DefaultSlotCount = 64;
    static const UInt32 MaxSlotCount = 4096;
    static const UInt32 MaxDropout = 3000;          /**< RFC 3550 A.1: largest forward jump of the sequence number that is not a restart */
    static const UInt32 MaxMisorder = 100;          /**< RFC 3550 A.1: largest backward step that is still a late or duplicate frame */

    enum EInsertResult
    {
        INSERT_STORED = 0 ,     /**< stored, in order */
        INSERT_REORDERED ,      /**< stored, it came after a frame with a higher sequence number */
        INSERT_DUPLICATE ,      /**< received before: dropped */
        INSERT_LATE ,           /**< its playout time has passed: dropped */
        INSERT_PROBATION ,      /**< the sequence number jumped: held until the next frame confirms a new stream */
        INSERT_RESTARTED ,      /**< the frame confirmed a new stream: the old frames were dropped, the held frame and this one stored, playout stopped */
        INSERT_OVERFLOW ,       /**< stored, the window of slots moved: older frames were pushed out and playout stopped */
        INSERT_NOT_PREPARED     /**< Prepare() was not called */
    };

    enum EPullResult
    {
        PULL_FRAME = 0 ,        /**< the frame for the playout timestamp, or an older one that is still newer than the last one handed out */
        PULL_MISSING ,          /**< the frame for the playout timestamp is not there, a newer frame of the same talkspurt is */
        PULL_EMPTY ,            /**< nothing is buffered */
        PULL_TALKSPURT_END ,    /**< the next frame starts a new talkspurt; playout stopped, StartPlayout() starts the new one */
        PULL_NOT_PLAYING        /**< StartPlayout() was not called */
    };

    /** A stored frame and what the buffer knows about it */
    struct GUCEF_AUDIO_PUBLIC_CPP SFrameInfo
    {
        const CVoiceFrame* frame;           /**< valid until the next call of a non-const member */
        UInt64 extendedSequenceNumber;
        UInt64 extendedTimestamp;
        UInt64 arrivalTimeInUs;
        bool isRedundant;                   /**< taken from the RFC 2198 block of the next frame */

        SFrameInfo( void );
    };

    /** What Pull() handed out */
    struct GUCEF_AUDIO_PUBLIC_CPP SPullInfo
    {
        SFrameInfo frame;                   /**< PULL_FRAME: the frame; PULL_MISSING: the frame right after the missing one when it is there (for forward error correction), else no frame */
        UInt64 latenessInTimestampUnits;    /**< PULL_FRAME: how far the playout timestamp is past the frame's own timestamp (0 when on time) */
        UInt64 skippedFrames;               /**< PULL_FRAME: sequence numbers between the last frame handed out and this one (lost or late) */
        UInt32 lateDiscarded;               /**< frames dropped by this call because they were older than wanted */
        UInt64 maxLatenessDiscarded;        /**< the largest distance of those from the playout timestamp, in timestamp units */

        SPullInfo( void );
    };

    /** What Insert() did with a frame */
    struct GUCEF_AUDIO_PUBLIC_CPP SInsertInfo
    {
        UInt64 extendedSequenceNumber;      /**< of the frame, when it got one (not for INSERT_PROBATION or INSERT_NOT_PREPARED) */
        UInt64 extendedTimestamp;
        bool redundantStored;               /**< the frame's redundant block filled an empty slot */

        SInsertInfo( void );
    };

    /** Counters since Prepare() or Reset(); they continue over a stream restart */
    struct GUCEF_AUDIO_PUBLIC_CPP SStatistics
    {
        UInt64 framesReceived;              /**< frames that came in, not counting duplicates and frames held on probation (RFC 3550 "received") */
        UInt64 framesExpected;              /**< RFC 3550 A.3: highest extended sequence number minus the first plus one, summed over stream restarts */
        Int64 cumulativeLost;               /**< RFC 3550 A.3: expected minus received (may be negative with duplicates the receiver could not tell) */
        UInt64 late;                        /**< dropped because their playout time had passed */
        UInt64 duplicates;
        UInt64 reordered;                   /**< came after a frame with a higher sequence number */
        UInt64 redundantStored;             /**< frames filled in from an RFC 2198 redundant block */
        UInt64 redundantUnused;             /**< redundant blocks that were not needed (the frame was there, played or late) */
        UInt64 overflowDropped;             /**< stored frames pushed out of the window by a frame too far ahead */
        UInt64 restarts;                    /**< stream restarts after a jump of the sequence number (RFC 3550 A.1 probation) */
        UInt64 probationDropped;            /**< frames held on probation that no next frame confirmed */
        UInt64 talkspurtEnds;               /**< times Pull() found the start of a new talkspurt */
        UInt64 extendedHighestSequenceNumber;
        Float64 interarrivalJitter;         /**< RFC 3550 6.4.1, in timestamp units */
        UInt32 bufferedFrames;              /**< stored frames now */

        SStatistics( void );
    };

    CVoiceJitterBuffer( void );

    ~CVoiceJitterBuffer();

    /**
     *  Allocates the slots and forgets everything. 'slotCount' is a power of two from 2 to MaxSlotCount;
     *  'timestampUnitsPerFrame' is the timestamp distance of two frames (480 for 10 ms at a 48 kHz timestamp rate);
     *  'timestampRate' is the RTP clock rate (48000 for Opus, the sample rate for PCM). Owner thread only. False, and
     *  nothing changed, for values out of range.
     */
    bool Prepare( UInt32 slotCount, UInt32 timestampUnitsPerFrame, UInt32 timestampRate );

    bool IsPrepared( void ) const;

    /** Forgets all frames, the stream and the statistics. Real time safe */
    void Reset( void );

    /**
     *  Takes one frame that arrived at 'arrivalTimeInUs' (any monotonic microsecond clock, the one the playout uses).
     *  Real time safe.
     */
    EInsertResult Insert( const CVoiceFrame& frame, UInt64 arrivalTimeInUs );
    EInsertResult Insert( const CVoiceFrame& frame, UInt64 arrivalTimeInUs, SInsertInfo& info );

    /** Puts the playout cursor on the oldest stored frame. False when nothing is stored. Real time safe */
    bool StartPlayout( void );

    /** Stops the playout; the cursor stays, so older frames are still late. Real time safe */
    void StopPlayout( void );

    bool IsPlaying( void ) const;

    /**
     *  Hands out the frame for the playout timestamp (an extended timestamp, at or after GetNextTimestamp()). A frame
     *  whose timestamp is between GetNextTimestamp() and the playout timestamp (concealment stood in for it) is still
     *  handed out when it is at or after 'oldestWantedTimestamp', else it is dropped as late. On PULL_FRAME the cursor
     *  moves past the frame. Real time safe.
     */
    EPullResult Pull( UInt64 playoutTimestamp, UInt64 oldestWantedTimestamp, SPullInfo& info );

    /** Pull() that wants every frame newer than the last one handed out */
    EPullResult Pull( UInt64 playoutTimestamp, SPullInfo& info );

    /** The oldest stored frame. False when nothing is stored */
    bool PeekOldest( SFrameInfo& info ) const;

    /** The stored frame with the lowest extended sequence number above 'extendedSequenceNumber'. False when there is none */
    bool PeekNext( UInt64 extendedSequenceNumber, SFrameInfo& info ) const;

    /** Stored frames now */
    UInt32 GetStoredFrameCount( void ) const;

    /** The extended timestamp just after the newest stored frame (its timestamp plus a frame), 0 when nothing is stored */
    UInt64 GetBufferedEndTimestamp( void ) const;

    /** The extended timestamp of the next frame to hand out: right after the last one handed out, or of the oldest frame at StartPlayout() */
    UInt64 GetNextTimestamp( void ) const;

    /** The extended sequence number of the next frame to hand out */
    UInt64 GetNextSequenceNumber( void ) const;

    /** True once StartPlayout() put the cursor somewhere since Prepare(), Reset() or a stream restart */
    bool HasCursor( void ) const;

    /** RFC 3550 interarrival jitter in timestamp units */
    Float64 GetInterarrivalJitter( void ) const;

    UInt32 GetSlotCount( void ) const;
    UInt32 GetTimestampUnitsPerFrame( void ) const;
    UInt32 GetTimestampRate( void ) const;

    void GetStatistics( SStatistics& stats ) const;

    private:

    struct SSlot
    {
        CVoiceFrame frame;
        UInt64 extendedSequenceNumber;
        UInt64 extendedTimestamp;
        UInt64 arrivalTimeInUs;
        bool stored;
        bool isRedundant;
    };

    CVoiceJitterBuffer( const CVoiceJitterBuffer& src );                /**< not implemented */
    CVoiceJitterBuffer& operator=( const CVoiceJitterBuffer& src );     /**< not implemented */

    void ClearStream( void );
    void InitSequence( UInt16 sequenceNumber, UInt32 timestamp );
    UInt64 ExtendTimestamp( UInt32 timestamp, bool isNewHighest, UInt64 arrivalTimeInUs ) const;
    UInt64 GetWindowStart( void ) const;
    void UpdateJitter( UInt64 extendedTimestamp, UInt64 arrivalTimeInUs );
    void Store( const CVoiceFrame& frame, UInt64 extSeq, UInt64 extTs, UInt64 arrivalTimeInUs, bool isRedundant );
    void RemoveFromSlot( SSlot& slot );
    bool MoveWindow( UInt64 newWindowStart );
    bool FindStoredFrom( UInt64 fromExtSeq, UInt64& extSeq ) const;
    void FillInfo( const SSlot& slot, SFrameInfo& info ) const;
    void InsertRedundant( const CVoiceFrame& primary, UInt64 primaryExtSeq, UInt64 primaryExtTs, UInt64 arrivalTimeInUs, SInsertInfo& info );
    EInsertResult InsertPrimary( const CVoiceFrame& frame, UInt64 arrivalTimeInUs, SInsertInfo& info, bool countReceived );

    private:

    GUCEF::vector< SSlot > m_slots;
    GUCEF::vector< UInt64 > m_seen;             /**< per slot index: the extended sequence number of the last primary frame received there */
    UInt32 m_slotMask;
    UInt32 m_timestampUnitsPerFrame;
    UInt32 m_timestampRate;

    // stream: RFC 3550 A.1 state
    bool m_streamStarted;
    UInt16 m_maxSeq;
    UInt64 m_cycles;                            /**< multiple of 65536, starts at 65536 so a frame before the first never goes below 0 */
    UInt32 m_badSeq;                            /**< sequence number that would confirm a restart, 65537 = none */
    UInt64 m_baseExtSeq;
    UInt64 m_maxExtTs;                          /**< extended timestamp of the frame with the highest sequence number */
    UInt64 m_lastArrivalTimeInUs;
    UInt64 m_epochReceived;                     /**< received since the last (re)start, for A.3 */
    bool m_hasProbationFrame;
    CVoiceFrame m_probationFrame;
    UInt64 m_probationArrivalTimeInUs;

    // jitter: RFC 3550 A.8
    bool m_hasTransit;
    Float64 m_lastTransit;
    Float64 m_jitter;

    // storage and cursor
    UInt32 m_storedCount;
    UInt64 m_lowestStoredExtSeq;                /**< valid while frames are stored */
    UInt64 m_highestStoredExtSeq;
    UInt64 m_highestStoredEndTs;
    bool m_hasCursor;
    bool m_playing;
    UInt64 m_nextExtSeq;
    UInt64 m_nextExtTs;

    // statistics carried over stream restarts
    UInt64 m_priorExpected;
    UInt64 m_priorReceived;
    SStatistics m_stats;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICEJITTERBUFFER_H ? */
