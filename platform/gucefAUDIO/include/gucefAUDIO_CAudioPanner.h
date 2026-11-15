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

#ifndef GUCEF_AUDIO_CAUDIOPANNER_H
#define GUCEF_AUDIO_CAUDIOPANNER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTVBAP2D_H
#include "gucefMATH_CTVbap2D.h"
#define GUCEF_MATH_CTVBAP2D_H
#endif /* GUCEF_MATH_CTVBAP2D_H ? */

#ifndef GUCEF_AUDIO_CAUDIOFORMAT_H
#include "gucefAUDIO_CAudioFormat.h"
#define GUCEF_AUDIO_CAUDIOFORMAT_H
#endif /* GUCEF_AUDIO_CAUDIOFORMAT_H ? */

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
 *  The gains of the speakers of a layout for a source in a direction (design 5.6.3 and 5.6.4): the stereo law for a
 *  stereo layout, pairwise panning on the horizontal ring (vector base amplitude panning in two dimensions,
 *  V. Pulkki, JAES 45(6), 1997, computed by MATH::CTVbap2D) for the others, and the widening of the source toward all
 *  speakers near the listener. A ring of three or more directions with a gap of half a turn or more (left, centre and
 *  right only), which gucefMATH does not accept, is panned here: pairwise inside the covered arcs, at constant power across the gap.
 *  A pure calculator: Prepare() builds the table of speaker angles from a channel layout, after that the gain functions
 *  only read it and are realtime safe.
 *
 *  Normalisation: the squares of the gains of a panned source add up to 2, the power of a mono source panned to the
 *  centre of a stereo layout (gain 1 on both speakers, which is where a non spatial mono voice at pan 0 is), also
 *  after the widening. A layout with a single speaker on the ring has the gain 1.
 *
 *  The speaker angles are those of ITU-R BS.775 (positive to the right): front left and right -30 and +30 degrees, front
 *  centre 0, front left and right of centre -15 and +15, side left and right -90 and +90, back left and right -110 and +110
 *  (-150 and +150 when the layout has side speakers), back centre 180. Top speakers are projected onto the ring at the angle of their
 *  position (top front left and right -45 and +45, top front centre 0, top back left and right -135 and +135, top back
 *  centre 180); the top centre speaker has no direction and gets no gain. The low frequency channel is not panned
 *  (its gain is 0, the voice sends it its own level through GetLfeChannel()). Speakers on the same angle share the gain
 *  of their position at equal power. The centre speakers can be left out (use centre off): front centre and top front centre
 *  are then not on the ring, unless nothing else is left.
 *
 *  Not thread safe for Prepare(): it is done by the owner while no voice uses the object. Everything else reads only.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioPanner
{
    public:

    static const UInt32 MaxChannels = 32;       /**< the most channels of a layout: the 18 speaker positions fit, as do formats with more channels (those get no gain) */

    CAudioPanner( void );

    /** Builds the table of a layout. 'channelLayout' is the mask of ESpeakerPosition, 0 for the usual layout of the channel count. False, with the panner unprepared, for more than MaxChannels channels or none */
    bool Prepare( UInt32 channelCount, UInt32 channelLayout, bool useCenterSpeaker );

    /** Prepare() for the speakers of a format */
    bool Prepare( const CAudioFormat& format, bool useCenterSpeaker );

    bool IsPrepared( void ) const;

    UInt32 GetChannelCount( void ) const;

    /** True when the layout is panned with the stereo law: left and right on the ring and nothing else */
    bool IsStereo( void ) const;

    /** The number of speakers that take part in the panning: those on the ring */
    UInt32 GetRingChannelCount( void ) const;

    /** The number of distinct directions on the ring */
    UInt32 GetRingPositionCount( void ) const;

    /** The channel that is the low frequency channel, -1 when the layout has none */
    Int32 GetLfeChannel( void ) const;

    /** True when the channel is panned: on the ring */
    bool IsRingChannel( UInt32 channel ) const;

    /** The angle of a channel on the ring in degrees, 0 ahead and positive to the right; 0 for a channel that is not on the ring */
    Float32 GetChannelAzimuthInDegrees( UInt32 channel ) const;

    /**
     *  The angle of the speaker position (one of the ESpeakerPosition bits) in degrees: false for the low frequency channel, the
     *  top centre and for anything that is not a single position. 'layoutHasSideSpeakers' decides where the back speakers are
     */
    static bool GetSpeakerAzimuthInDegrees( UInt32 speakerPosition, bool layoutHasSideSpeakers, Float32& degrees );

    /**
     *  The widening factor of a source at a distance: clamp( distance / spreadRadius, 0, 1 ), 1 for a spread radius that is not positive.
     *  Below the spread radius the source spreads toward all speakers and is on all of them at distance 0
     */
    static Float32 GetSpreadFactor( Float32 distance, Float32 spreadRadius ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /**
     *  The stereo law (5.6.3): pan' = pan f, gainLeft = sqrt( 2 ) cos( ( pan' + 1 ) pi / 4 ), gainRight = sqrt( 2 ) sin( ( pan' + 1 ) pi / 4 ),
     *  pan from -1 (left) to 1 (right). Pure, no table needed
     */
    static void GetStereoGains( Float32 pan, Float32 spreadFactor, Float32& gainLeft, Float32& gainRight ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /**
     *  The stereo law for a direction: the pan is sin( azimuth ) (front and back fold onto the same position), with the optional front and back cue
     *  gain *= 1 - ( 1 - rearGain ) max( 0, -cos( azimuth ) ) for a source behind the listener ('rearGain' 1 for none)
     */
    static void GetStereoGainsForAzimuth( Float32 azimuth, Float32 spreadFactor, Float32 rearGain, Float32& gainLeft, Float32& gainRight ) GUCEF_AUDIO_REALTIME_CONTEXT;

    /**
     *  The gain of every channel for a source: writes GetChannelCount() values. 'azimuth' is in radians (0 ahead, positive to the right) and is what the
     *  ring uses, 'pan' is what the stereo law uses (sin( azimuth ) for a 3D source, the pan of the 2D mode), 'spreadFactor' is GetSpreadFactor(), 'rearGain' the
     *  cue of stereo output (1 for none). Channels that are not panned (the low frequency channel) get 0. False, and silence, when the panner is not prepared
     */
    bool GetGains( Float32 azimuth, Float32 pan, Float32 spreadFactor, Float32 rearGain, Float32* gains ) const GUCEF_AUDIO_REALTIME_CONTEXT;

    private:

    /** Where a channel is: not panned or the position on the ring it shares */
    enum { NOT_PANNED = -1 };

    void PanOnRing( Float32 azimuth, Float32* positionGains ) const GUCEF_AUDIO_REALTIME_CONTEXT;

    private:

    UInt32 m_channelCount;
    bool m_prepared;
    bool m_stereo;
    Int32 m_lfeChannel;
    UInt32 m_ringChannelCount;
    UInt32 m_positionCount;                                 /**< distinct directions on the ring, sorted by angle from -180 */
    Int32 m_positionOfChannel[ MaxChannels ];               /**< NOT_PANNED or the index in the position table */
    Float32 m_channelAzimuth[ MaxChannels ];                /**< degrees, valid for ring channels */
    UInt32 m_positionChannelCount[ MaxChannels ];           /**< channels at a position */
    Float32 m_positionAzimuth[ MaxChannels ];               /**< radians */
    Float32 m_positionInverseSpan[ MaxChannels ];           /**< 1 / sin( span ) of the pair of a position and the next one, 0 when the span is 180 degrees or more */
    Float32 m_positionSpan[ MaxChannels ];                  /**< radians between a position and the next one (the last wraps round) */
    Float32 m_targetPower;                                  /**< the sum of the squares of the gains: 2, or 1 for a ring with a single speaker */
    MATH::CTVbap2D< Float32 > m_vbap;                       /**< the pairwise panning of the positions, when gucefMATH accepts the ring (no gap of half a turn or more between three or more positions) */
    Int32 m_stereoLeft;
    Int32 m_stereoRight;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOPANNER_H ? */
