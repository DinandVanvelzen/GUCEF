/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOSIMULATEDVOICELINK_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOSIMULATEDVOICELINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>
#include <algorithm>

#ifndef GUCEF_AUDIO_CVOICEFRAME_H
#include "gucefAUDIO_CVoiceFrame.h"
#define GUCEF_AUDIO_CVOICEFRAME_H
#endif /* GUCEF_AUDIO_CVOICEFRAME_H ? */

#ifndef GUCEF_AUDIO_CRTPVOICEPACKETCODEC_H
#include "gucefAUDIO_CRtpVoicePacketCodec.h"
#define GUCEF_AUDIO_CRTPVOICEPACKETCODEC_H
#endif /* GUCEF_AUDIO_CRTPVOICEPACKETCODEC_H ? */

#ifndef GUCEF_AUDIO_CVOICERECEIVER_H
#include "gucefAUDIO_CVoiceReceiver.h"
#define GUCEF_AUDIO_CVOICERECEIVER_H
#endif /* GUCEF_AUDIO_CVOICERECEIVER_H ? */

#include "TestAudioEffectSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace VoiceLinkTest {

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

/**
 *  How a simulated network treats the packets of one direction. Every random choice comes from one xorshift generator
 *  of the given seed, so a run is the same every time. Times in microseconds of the simulated clock.
 *
 *  Per packet, in this order: the bandwidth and the queue in front of it (a packet waits until the packets before it
 *  went out at the given rate; one that finds more than 'queueLimitPackets' waiting is dropped), the loss (independent
 *  'randomLoss', and the Gilbert-Elliott chain: in the bad state a packet is lost with 'geLossInBad'), the delay: base
 *  plus jitter (none, uniform in [0, jitter), or Pareto with the given median and the given share above the given
 *  tail value, which a Wi-Fi channel with retries and power save looks like), a stall (with 'stallProbability' per
 *  packet a stall of 'stallDurationUs' begins: what is sent during it arrives together at its end, as after a Wi-Fi
 *  rescan), reordering (with 'reorderProbability' a packet is held back by 1 to 'reorderMaxSteps' steps of
 *  'reorderStepUs', so the next ones overtake it) and duplication (a second copy 0.1 to 3 ms after the first).
 */
struct SLinkModel
{
    enum EJitter { JITTER_NONE = 0, JITTER_UNIFORM, JITTER_PARETO };

    Float64 baseDelayUs;
    EJitter jitter;
    Float64 uniformJitterUs;            /**< JITTER_UNIFORM: U( 0, this ) */
    Float64 paretoMedianUs;             /**< JITTER_PARETO: median of the added delay ... */
    Float64 paretoTailUs;               /**< ... and the share 'paretoTailShare' of the packets above this */
    Float64 paretoTailShare;
    Float64 stallProbability;
    Float64 stallDurationUs;
    Float64 randomLoss;
    Float64 geGoodToBad;                /**< Gilbert-Elliott: p, 0 for no chain */
    Float64 geBadToGood;                /**< r: the mean burst is 1 / r packets */
    Float64 geLossInBad;
    Float64 reorderProbability;
    UInt32 reorderMaxSteps;
    Float64 reorderStepUs;
    Float64 duplicateProbability;
    Float64 bandwidthBitsPerSecond;     /**< 0: no limit */
    UInt32 queueLimitPackets;           /**< 0: no limit */
    UInt32 seed;

    SLinkModel( void )
        : baseDelayUs( 300.0 ), jitter( JITTER_NONE ), uniformJitterUs( 0.0 ), paretoMedianUs( 2000.0 ), paretoTailUs( 30000.0 ), paretoTailShare( 0.01 )
        , stallProbability( 0.0 ), stallDurationUs( 0.0 ), randomLoss( 0.0 ), geGoodToBad( 0.0 ), geBadToGood( 1.0 ), geLossInBad( 1.0 )
        , reorderProbability( 0.0 ), reorderMaxSteps( 2 ), reorderStepUs( 10000.0 ), duplicateProbability( 0.0 ), bandwidthBitsPerSecond( 0.0 )
        , queueLimitPackets( 0 ), seed( 0x2468ACE1 )
    {}

    /** Wired Ethernet of section 6.2: 0.3 ms plus up to 0.2 ms */
    static SLinkModel Wired( UInt32 seed = 0x2468ACE1 )
    {
        SLinkModel model;
        model.baseDelayUs = 300.0;
        model.jitter = JITTER_UNIFORM;
        model.uniformJitterUs = 200.0;
        model.seed = seed;
        return model;
    }

    /** Wi-Fi of section 6.2: 3 ms plus a Pareto delay of median 2 ms with 1 % above 30 ms, 1 % random loss */
    static SLinkModel WiFi( UInt32 seed = 0x13579BDF )
    {
        SLinkModel model;
        model.baseDelayUs = 3000.0;
        model.jitter = JITTER_PARETO;
        model.paretoMedianUs = 2000.0;
        model.paretoTailUs = 30000.0;
        model.paretoTailShare = 0.01;
        model.randomLoss = 0.01;
        model.seed = seed;
        return model;
    }

    /** The shape of the Pareto delay: P( X > x ) = ( scale / x )^shape, with P( X > median ) = 0.5 and P( X > tail ) = share */
    Float64 ParetoShape( void ) const
    {
        return log( 0.5 / paretoTailShare ) / log( paretoTailUs / paretoMedianUs );
    }

    Float64 ParetoScaleUs( void ) const
    {
        return paretoMedianUs / pow( 2.0, 1.0 / ParetoShape() );
    }
};

/*-------------------------------------------------------------------------*/

/** What the link did so far */
struct SLinkStats
{
    UInt64 sent;                    /**< packets handed to Send() */
    UInt64 lost;                    /**< dropped by the loss models */
    UInt64 queueDropped;            /**< dropped because the queue was full */
    UInt64 duplicated;              /**< copies made */
    UInt64 stalled;                 /**< packets held by a stall */
    UInt64 reordered;               /**< packets held back so later ones overtake them */
    UInt64 delivered;               /**< packets read and handed on, copies included */
    UInt64 deliveredOutOfOrder;     /**< delivered after a packet that was sent later */
    UInt64 refused;                 /**< not written (too large) or not read back */
    UInt64 bytes;                   /**< bytes of the packets sent */
    Float64 sumDelayUs;             /**< over the delivered packets: arrival minus send time */
    Float64 maxDelayUs;

    SLinkStats( void )
        : sent( 0 ), lost( 0 ), queueDropped( 0 ), duplicated( 0 ), stalled( 0 ), reordered( 0 ), delivered( 0 ), deliveredOutOfOrder( 0 )
        , refused( 0 ), bytes( 0 ), sumDelayUs( 0.0 ), maxDelayUs( 0.0 )
    {}
};

/*-------------------------------------------------------------------------*/

/**
 *  A simulated network for one direction of voice packets: frames from a sender's PopFrame() are written as RTP
 *  packets (CRtpVoicePacketCodec), held for the delay the model gives them and read back into frames that go to a
 *  receiver's PushFrame() with their arrival time. Time is the simulated clock of the test (microseconds); the arrival
 *  time is converted into the clock of the receiving unit with the scale and offset the caller gives, which is how a
 *  clock drift between two units enters. Test code: allocates freely, one thread at a time.
 */
class CTestSimulatedVoiceLink
{
    public:

    explicit CTestSimulatedVoiceLink( const SLinkModel& model = SLinkModel() )
        : m_model( model ), m_codec(), m_random( ScrambleSeed( model.seed ) ), m_events(), m_order( 0 ), m_geBad( false ), m_stallEndUs( -1.0 ), m_queueFreeAtUs( 0.0 )
        , m_departures(), m_lastDeliveredOrder( 0 ), m_hasDelivered( false ), m_stats()
    {}

    /** A small seed starts xorshift with small numbers (the first packets would all be lost): spread it out */
    static UInt32 ScrambleSeed( UInt32 seed )
    {
        UInt32 x = seed * 2654435761u ^ 0x5BD1E995u;
        return 0 == x ? 0x2468ACE1u : x;
    }

    const SLinkModel& GetModel( void ) const { return m_model; }
    const SLinkStats& GetStats( void ) const { return m_stats; }

    /** The payload type of packets with RFC 2198 redundancy, as the receiver's configuration has it */
    void SetRedPayloadType( UInt8 payloadType ) { m_codec.SetRedPayloadType( payloadType ); }

    /** Packets on their way */
    size_t GetInFlightCount( void ) const { return m_events.size(); }

    /** Sends a frame that the sender had complete at 'sendTimeInUs' */
    void Send( const AUDIO::CVoiceFrame& frame, Float64 sendTimeInUs )
    {
        SEvent event;
        const Int32 written = m_codec.Write( frame, event.packet, (UInt32) sizeof( event.packet ) );
        ++m_stats.sent;
        if ( written <= 0 )
        {
            ++m_stats.refused;
            return;
        }
        event.size = (UInt32) written;
        m_stats.bytes += (UInt64) written;

        // the queue in front of the link: departures at the link rate
        Float64 departureUs = sendTimeInUs;
        if ( m_model.bandwidthBitsPerSecond > 0.0 )
        {
            while ( !m_departures.empty() && m_departures.front() <= sendTimeInUs )
                m_departures.erase( m_departures.begin() );
            if ( 0 != m_model.queueLimitPackets && m_departures.size() >= m_model.queueLimitPackets )
            {
                ++m_stats.queueDropped;
                return;
            }
            const Float64 start = m_queueFreeAtUs > sendTimeInUs ? m_queueFreeAtUs : sendTimeInUs;
            departureUs = start + (Float64) written * 8.0 * 1000000.0 / m_model.bandwidthBitsPerSecond;
            m_queueFreeAtUs = departureUs;
            m_departures.push_back( departureUs );
        }

        // loss
        bool lost = m_model.randomLoss > 0.0 && m_random.NextUnit() < m_model.randomLoss;
        if ( m_model.geGoodToBad > 0.0 )
        {
            if ( m_geBad )
                m_geBad = !( m_random.NextUnit() < m_model.geBadToGood );
            else
                m_geBad = m_random.NextUnit() < m_model.geGoodToBad;
            if ( m_geBad && m_random.NextUnit() < m_model.geLossInBad )
                lost = true;
        }
        if ( lost )
        {
            ++m_stats.lost;
            return;
        }

        // delay
        Float64 delayUs = m_model.baseDelayUs;
        if ( SLinkModel::JITTER_UNIFORM == m_model.jitter )
            delayUs += m_random.NextUnit() * m_model.uniformJitterUs;
        else
        if ( SLinkModel::JITTER_PARETO == m_model.jitter )
        {
            Float64 u = m_random.NextUnit();
            if ( u < 1.0e-12 )
                u = 1.0e-12;
            Float64 extra = m_model.ParetoScaleUs() / pow( u, 1.0 / m_model.ParetoShape() );
            if ( extra > 2000000.0 )
                extra = 2000000.0;
            delayUs += extra;
        }
        Float64 arrivalUs = departureUs + delayUs;
        if ( m_model.stallProbability > 0.0 && sendTimeInUs >= m_stallEndUs && m_random.NextUnit() < m_model.stallProbability )
            m_stallEndUs = sendTimeInUs + m_model.stallDurationUs;
        if ( sendTimeInUs < m_stallEndUs && arrivalUs < m_stallEndUs )
        {
            arrivalUs = m_stallEndUs + 20.0 * (Float64) ( m_order % 8 );
            ++m_stats.stalled;
        }
        if ( m_model.reorderProbability > 0.0 && m_random.NextUnit() < m_model.reorderProbability )
        {
            const UInt32 steps = 1 + ( m_model.reorderMaxSteps > 1 ? m_random.Next() % m_model.reorderMaxSteps : 0 );
            arrivalUs += (Float64) steps * m_model.reorderStepUs + 100.0;
            ++m_stats.reordered;
        }

        event.sendUs = sendTimeInUs;
        Push( event, arrivalUs );
        if ( m_model.duplicateProbability > 0.0 && m_random.NextUnit() < m_model.duplicateProbability )
        {
            ++m_stats.duplicated;
            Push( event, arrivalUs + 100.0 + m_random.NextUnit() * 2900.0 );
        }
    }

    /**
     *  Takes out the next packet that arrived at or before 'nowInUs' and reads it. False when there is none.
     *  'arrivalInUs' and 'sendInUs' are in the simulated clock
     */
    bool PopArrived( Float64 nowInUs, AUDIO::CVoiceFrame& frame, Float64& arrivalInUs, Float64& sendInUs )
    {
        while ( !m_events.empty() && m_events.front().arrivalUs <= nowInUs )
        {
            std::pop_heap( m_events.begin(), m_events.end() );
            const SEvent event = m_events.back();
            m_events.pop_back();
            if ( !m_codec.Read( event.packet, event.size, frame ) )
            {
                ++m_stats.refused;
                continue;
            }
            ++m_stats.delivered;
            if ( m_hasDelivered && event.sendOrder < m_lastDeliveredOrder )
                ++m_stats.deliveredOutOfOrder;
            else
                m_lastDeliveredOrder = event.sendOrder;
            m_hasDelivered = true;
            const Float64 delay = event.arrivalUs - event.sendUs;
            m_stats.sumDelayUs += delay;
            if ( delay > m_stats.maxDelayUs )
                m_stats.maxDelayUs = delay;
            arrivalInUs = event.arrivalUs;
            sendInUs = event.sendUs;
            return true;
        }
        return false;
    }

    /**
     *  Hands every packet that arrived at or before 'nowInUs' to the receiver. The arrival time in the receiver's clock
     *  is offsetInUs + scale * arrival (the clock of a unit whose crystal runs 'scale' times the simulated time).
     *  Returns the frames the receiver took
     */
    UInt32 Deliver( Float64 nowInUs, AUDIO::CVoiceReceiver& receiver, Float64 scale = 1.0, Float64 offsetInUs = 0.0 )
    {
        UInt32 taken = 0;
        AUDIO::CVoiceFrame frame;
        Float64 arrival = 0.0, send = 0.0;
        while ( PopArrived( nowInUs, frame, arrival, send ) )
        {
            const Float64 local = offsetInUs + scale * arrival;
            if ( receiver.PushFrame( frame, (UInt64) ( local + 0.5 ) ) )
                ++taken;
        }
        return taken;
    }

    private:

    struct SEvent
    {
        Float64 arrivalUs;
        Float64 sendUs;
        UInt64 order;           /**< tie breaker: equal arrival times keep the order they were pushed in */
        UInt64 sendOrder;       /**< the order of Send() */
        UInt32 size;
        UInt8 packet[ AUDIO::CRtpVoicePacketCodec::MaxPacketSize ];

        bool operator<( const SEvent& other ) const
        {
            // a max-heap of 'later': the earliest arrival is at the front
            if ( arrivalUs != other.arrivalUs )
                return arrivalUs > other.arrivalUs;
            return order > other.order;
        }
    };

    void Push( SEvent event, Float64 arrivalUs )
    {
        event.arrivalUs = arrivalUs;
        event.order = m_order++;
        event.sendOrder = m_stats.sent;
        m_events.push_back( event );
        std::push_heap( m_events.begin(), m_events.end() );
    }

    SLinkModel m_model;
    AUDIO::CRtpVoicePacketCodec m_codec;
    AudioEffectTest::CXorShift m_random;
    GUCEF::vector< SEvent > m_events;
    UInt64 m_order;
    bool m_geBad;
    Float64 m_stallEndUs;
    Float64 m_queueFreeAtUs;
    GUCEF::vector< Float64 > m_departures;
    UInt64 m_lastDeliveredOrder;
    bool m_hasDelivered;
    SLinkStats m_stats;
};

/*-------------------------------------------------------------------------*/

}; /* namespace VoiceLinkTest */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOSIMULATEDVOICELINK_H ? */
