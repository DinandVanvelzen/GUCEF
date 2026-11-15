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

#include "gucefAUDIO_CAudioPanner.h"

#ifndef GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#include "gucefMATH_SpatialAudioFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTMATHCONSTANTS_H
#include "gucefMATH_CTMathConstants.h"
#define GUCEF_MATH_CTMATHCONSTANTS_H
#endif /* GUCEF_MATH_CTMATHCONSTANTS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Float32 Pi = MATH::CTMathConstants< Float32 >::Pi();
const Float32 TwoPi = MATH::CTMathConstants< Float32 >::TwoPi();
const Float32 HalfPi = MATH::CTMathConstants< Float32 >::HalfPi();
const Float32 DegreesToRadians = MATH::CTMathConstants< Float32 >::RadiansPerDegree();
const UInt32 SpeakerPositionCount = 18;

/** Two angles in degrees that are the same direction */
const Float32 SamePositionInDegrees = 0.01f;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioPanner::CAudioPanner( void )
    : m_channelCount( 0 )
    , m_prepared( false )
    , m_stereo( false )
    , m_lfeChannel( -1 )
    , m_ringChannelCount( 0 )
    , m_positionCount( 0 )
    , m_targetPower( 2.0f )
    , m_vbap()
    , m_stereoLeft( -1 )
    , m_stereoRight( -1 )
{GUCEF_TRACE;

    for ( UInt32 i = 0; i < MaxChannels; ++i )
    {
        m_positionOfChannel[ i ] = NOT_PANNED;
        m_channelAzimuth[ i ] = 0.0f;
        m_positionChannelCount[ i ] = 0;
        m_positionAzimuth[ i ] = 0.0f;
        m_positionInverseSpan[ i ] = 0.0f;
        m_positionSpan[ i ] = 0.0f;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAudioPanner::GetSpeakerAzimuthInDegrees( UInt32 speakerPosition, bool layoutHasSideSpeakers, Float32& degrees )
{GUCEF_TRACE;

    switch ( speakerPosition )
    {
        case AUDIOSPEAKER_FRONT_LEFT :              degrees = -30.0f; return true;
        case AUDIOSPEAKER_FRONT_RIGHT :             degrees = 30.0f; return true;
        case AUDIOSPEAKER_FRONT_CENTER :            degrees = 0.0f; return true;
        case AUDIOSPEAKER_BACK_LEFT :               degrees = layoutHasSideSpeakers ? -150.0f : -110.0f; return true;
        case AUDIOSPEAKER_BACK_RIGHT :              degrees = layoutHasSideSpeakers ? 150.0f : 110.0f; return true;
        case AUDIOSPEAKER_FRONT_LEFT_OF_CENTER :    degrees = -15.0f; return true;
        case AUDIOSPEAKER_FRONT_RIGHT_OF_CENTER :   degrees = 15.0f; return true;
        case AUDIOSPEAKER_BACK_CENTER :             degrees = 180.0f; return true;
        case AUDIOSPEAKER_SIDE_LEFT :               degrees = -90.0f; return true;
        case AUDIOSPEAKER_SIDE_RIGHT :              degrees = 90.0f; return true;
        case AUDIOSPEAKER_TOP_FRONT_LEFT :          degrees = -45.0f; return true;
        case AUDIOSPEAKER_TOP_FRONT_CENTER :        degrees = 0.0f; return true;
        case AUDIOSPEAKER_TOP_FRONT_RIGHT :         degrees = 45.0f; return true;
        case AUDIOSPEAKER_TOP_BACK_LEFT :           degrees = -135.0f; return true;
        case AUDIOSPEAKER_TOP_BACK_CENTER :         degrees = 180.0f; return true;
        case AUDIOSPEAKER_TOP_BACK_RIGHT :          degrees = 135.0f; return true;
        default :                                   return false;       // low frequency, top centre, or not a single position
    }
}

/*-------------------------------------------------------------------------*/

bool
CAudioPanner::Prepare( const CAudioFormat& format, bool useCenterSpeaker )
{GUCEF_TRACE;

    return Prepare( format.GetChannelCount(), format.GetChannelLayout(), useCenterSpeaker );
}

/*-------------------------------------------------------------------------*/

bool
CAudioPanner::Prepare( UInt32 channelCount, UInt32 channelLayout, bool useCenterSpeaker )
{GUCEF_TRACE;

    *this = CAudioPanner();
    if ( 0 == channelCount || channelCount > MaxChannels )
        return false;

    if ( 0 == channelLayout )
        channelLayout = CAudioFormat::GetDefaultChannelLayout( channelCount );
    const bool hasSideSpeakers = 0 != ( channelLayout & ( AUDIOSPEAKER_SIDE_LEFT | AUDIOSPEAKER_SIDE_RIGHT ) );

    // the speaker of every channel: the channels are in the order of the bits of the layout
    UInt32 speakers[ MaxChannels ];
    UInt32 assigned = 0;
    for ( UInt32 bit = 0; bit < SpeakerPositionCount && assigned < channelCount; ++bit )
    {
        if ( 0 != ( channelLayout & ( 1U << bit ) ) )
            speakers[ assigned++ ] = ( 1U << bit );
    }
    for ( UInt32 i = assigned; i < channelCount; ++i )
        speakers[ i ] = 0;

    m_channelCount = channelCount;
    m_prepared = true;

    // which channels are on the ring: all with a direction, the centre ones only when they are used (or when nothing else is there)
    bool onRing[ MaxChannels ];
    UInt32 ringCount = 0;
    UInt32 centerCount = 0;
    for ( UInt32 i = 0; i < channelCount; ++i )
    {
        onRing[ i ] = false;
        Float32 degrees = 0.0f;
        if ( AUDIOSPEAKER_LOW_FREQUENCY == speakers[ i ] )
        {
            if ( m_lfeChannel < 0 )
                m_lfeChannel = (Int32) i;
            continue;
        }
        if ( !GetSpeakerAzimuthInDegrees( speakers[ i ], hasSideSpeakers, degrees ) )
            continue;

        m_channelAzimuth[ i ] = degrees;
        onRing[ i ] = true;
        ++ringCount;
        if ( AUDIOSPEAKER_FRONT_CENTER == speakers[ i ] || AUDIOSPEAKER_TOP_FRONT_CENTER == speakers[ i ] )
            ++centerCount;
    }
    if ( !useCenterSpeaker && centerCount < ringCount )
    {
        for ( UInt32 i = 0; i < channelCount; ++i )
        {
            if ( onRing[ i ] && ( AUDIOSPEAKER_FRONT_CENTER == speakers[ i ] || AUDIOSPEAKER_TOP_FRONT_CENTER == speakers[ i ] ) )
            {
                onRing[ i ] = false;
                m_channelAzimuth[ i ] = 0.0f;
                --ringCount;
            }
        }
    }
    m_ringChannelCount = ringCount;
    m_targetPower = ringCount >= 2 ? 2.0f : 1.0f;

    // stereo: left and right on the ring and nothing else
    if ( 2 == ringCount )
    {
        Int32 left = -1;
        Int32 right = -1;
        for ( UInt32 i = 0; i < channelCount; ++i )
        {
            if ( onRing[ i ] && AUDIOSPEAKER_FRONT_LEFT == speakers[ i ] )
                left = (Int32) i;
            if ( onRing[ i ] && AUDIOSPEAKER_FRONT_RIGHT == speakers[ i ] )
                right = (Int32) i;
        }
        if ( left >= 0 && right >= 0 )
        {
            m_stereo = true;
            m_stereoLeft = left;
            m_stereoRight = right;
        }
    }

    // the positions of the ring: the channels sorted by angle, channels on the same angle share a position
    UInt32 order[ MaxChannels ];
    UInt32 orderCount = 0;
    for ( UInt32 i = 0; i < channelCount; ++i )
    {
        if ( !onRing[ i ] )
            continue;
        UInt32 at = orderCount;
        while ( at > 0 && m_channelAzimuth[ order[ at - 1 ] ] > m_channelAzimuth[ i ] )
        {
            order[ at ] = order[ at - 1 ];
            --at;
        }
        order[ at ] = i;
        ++orderCount;
    }

    m_positionCount = 0;
    for ( UInt32 k = 0; k < orderCount; ++k )
    {
        const UInt32 channel = order[ k ];
        if ( m_positionCount > 0 && fabsf( m_channelAzimuth[ channel ] - m_positionAzimuth[ m_positionCount - 1 ] / DegreesToRadians ) < SamePositionInDegrees )
        {
            m_positionOfChannel[ channel ] = (Int32) ( m_positionCount - 1 );
            ++m_positionChannelCount[ m_positionCount - 1 ];
        }
        else
        {
            m_positionOfChannel[ channel ] = (Int32) m_positionCount;
            m_positionChannelCount[ m_positionCount ] = 1;
            m_positionAzimuth[ m_positionCount ] = m_channelAzimuth[ channel ] * DegreesToRadians;
            ++m_positionCount;
        }
    }

    // the span from a position to the next, the last one wraps round to the first
    for ( UInt32 i = 0; i < m_positionCount; ++i )
    {
        const UInt32 next = ( i + 1 ) % m_positionCount;
        Float32 span = m_positionAzimuth[ next ] - m_positionAzimuth[ i ];
        if ( span <= 0.0f )
            span += TwoPi;
        m_positionSpan[ i ] = span;

        // vector base amplitude panning needs the pair to span less than half a turn
        m_positionInverseSpan[ i ] = ( m_positionCount > 1 && span < Pi - 1.0e-4f ) ? 1.0f / sinf( span ) : 0.0f;
    }

    // the ring panning of gucefMATH, for every ring it accepts: two positions (a pair with a constant power open side) and rings
    // without a gap of half a turn. A ring of three or more positions with such a gap (left, centre and right only) keeps the
    // panning below, which crosses the gap at constant power
    if ( m_positionCount >= 2 )
    {
        MATH::CTRadian< Float32 > azimuths[ MaxChannels ];
        for ( UInt32 i = 0; i < m_positionCount; ++i )
            azimuths[ i ] = MATH::CTRadian< Float32 >( m_positionAzimuth[ i ] );
        m_vbap.Prepare( azimuths, m_positionCount );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioPanner::IsPrepared( void ) const
{GUCEF_TRACE;

    return m_prepared;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioPanner::GetChannelCount( void ) const
{GUCEF_TRACE;

    return m_channelCount;
}

/*-------------------------------------------------------------------------*/

bool
CAudioPanner::IsStereo( void ) const
{GUCEF_TRACE;

    return m_stereo;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioPanner::GetRingChannelCount( void ) const
{GUCEF_TRACE;

    return m_ringChannelCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioPanner::GetRingPositionCount( void ) const
{GUCEF_TRACE;

    return m_positionCount;
}

/*-------------------------------------------------------------------------*/

Int32
CAudioPanner::GetLfeChannel( void ) const
{GUCEF_TRACE;

    return m_lfeChannel;
}

/*-------------------------------------------------------------------------*/

bool
CAudioPanner::IsRingChannel( UInt32 channel ) const
{GUCEF_TRACE;

    return channel < MaxChannels && m_positionOfChannel[ channel ] != NOT_PANNED;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioPanner::GetChannelAzimuthInDegrees( UInt32 channel ) const
{GUCEF_TRACE;

    return IsRingChannel( channel ) ? m_channelAzimuth[ channel ] : 0.0f;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioPanner::GetSpreadFactor( Float32 distance, Float32 spreadRadius ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    if ( !( spreadRadius > 0.0f ) )
        return 1.0f;
    return MATH::Clamp01( distance / spreadRadius );
}

/*-------------------------------------------------------------------------*/

void
CAudioPanner::GetStereoGains( Float32 pan, Float32 spreadFactor, Float32& gainLeft, Float32& gainRight ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    MATH::ConstantPowerPan( pan * MATH::Clamp01( spreadFactor ), MATH::CTMathConstants< Float32 >::Sqrt2(), gainLeft, gainRight );

    // rounding at the edges must not give a gain under 0
    gainLeft = gainLeft > 0.0f ? gainLeft : 0.0f;
    gainRight = gainRight > 0.0f ? gainRight : 0.0f;
}

/*-------------------------------------------------------------------------*/

void
CAudioPanner::GetStereoGainsForAzimuth( Float32 azimuth, Float32 spreadFactor, Float32 rearGain, Float32& gainLeft, Float32& gainRight ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    GetStereoGains( sinf( azimuth ), spreadFactor, gainLeft, gainRight );

    const Float32 cosine = cosf( azimuth );
    if ( cosine < 0.0f )
    {
        const Float32 cue = 1.0f - ( 1.0f - MATH::Clamp01( rearGain ) ) * -cosine;
        gainLeft *= cue;
        gainRight *= cue;
    }
}

/*-------------------------------------------------------------------------*/

void
CAudioPanner::PanOnRing( Float32 azimuth, Float32* positionGains ) const GUCEF_AUDIO_REALTIME_CONTEXT
{
    for ( UInt32 i = 0; i < m_positionCount; ++i )
        positionGains[ i ] = 0.0f;

    if ( 1 == m_positionCount )
    {
        positionGains[ 0 ] = sqrtf( m_targetPower );
        return;
    }
    if ( m_vbap.IsPrepared() )
    {
        // the positions were given to gucefMATH in their own order, so its gains are those of the positions
        m_vbap.ComputeGains( MATH::CTRadian< Float32 >( azimuth ), positionGains, m_targetPower );
        return;
    }

    // the angle in -pi .. pi
    Float32 angle = azimuth - TwoPi * floorf( ( azimuth + Pi ) / TwoPi );

    // the pair that encloses it: the last position at or before the angle, the last pair wraps round
    UInt32 first = m_positionCount - 1;
    for ( UInt32 i = 0; i < m_positionCount; ++i )
    {
        if ( m_positionAzimuth[ i ] <= angle )
            first = i;
    }
    const UInt32 second = ( first + 1 ) % m_positionCount;

    Float32 offset = angle - m_positionAzimuth[ first ];
    if ( offset < 0.0f )
        offset += TwoPi;
    const Float32 span = m_positionSpan[ first ];
    if ( offset > span )
    {
        // only rounding at the very end of a pair gets here: the nearer end of it
        offset = ( offset - span < TwoPi - offset ) ? span : 0.0f;
    }

    Float32 g1 = 0.0f;
    Float32 g2 = 0.0f;
    if ( m_positionInverseSpan[ first ] > 0.0f )
    {
        // [ g1 g2 ] = p L^-1 for the unit vectors of the pair: sin( a2 - a ) / sin( a2 - a1 ) and sin( a - a1 ) / sin( a2 - a1 )
        g1 = sinf( span - offset ) * m_positionInverseSpan[ first ];
        g2 = sinf( offset ) * m_positionInverseSpan[ first ];
    }
    else
    {
        // a gap of half a turn or more has no pair to solve: constant power over the angle
        const Float32 t = MATH::Clamp01( offset / span );
        g1 = cosf( t * HalfPi );
        g2 = sinf( t * HalfPi );
    }
    if ( g1 < 0.0f ) g1 = 0.0f;
    if ( g2 < 0.0f ) g2 = 0.0f;

    const Float32 power = g1 * g1 + g2 * g2;
    const Float32 normaliser = power > 0.0f ? sqrtf( m_targetPower / power ) : 0.0f;
    positionGains[ first ] = g1 * normaliser;
    positionGains[ second ] = g2 * normaliser;
}

/*-------------------------------------------------------------------------*/

bool
CAudioPanner::GetGains( Float32 azimuth, Float32 pan, Float32 spreadFactor, Float32 rearGain, Float32* gains ) const GUCEF_AUDIO_REALTIME_CONTEXT
{
    for ( UInt32 i = 0; i < m_channelCount; ++i )
        gains[ i ] = 0.0f;
    if ( !m_prepared || 0 == m_ringChannelCount )
        return m_prepared;

    if ( m_stereo )
    {
        Float32 left = 0.0f;
        Float32 right = 0.0f;
        GetStereoGains( pan, spreadFactor, left, right );

        const Float32 cosine = cosf( azimuth );
        if ( cosine < 0.0f )
        {
            const Float32 cue = 1.0f - ( 1.0f - MATH::Clamp01( rearGain ) ) * -cosine;
            left *= cue;
            right *= cue;
        }
        gains[ m_stereoLeft ] = left;
        gains[ m_stereoRight ] = right;
        return true;
    }

    Float32 positionGains[ MaxChannels ];
    PanOnRing( azimuth, positionGains );

    // the channels of a position share its gain at equal power
    Float32 ringGains[ MaxChannels ];
    UInt32 ringIndex = 0;
    for ( UInt32 i = 0; i < m_channelCount; ++i )
    {
        const Int32 position = m_positionOfChannel[ i ];
        if ( NOT_PANNED == position )
            continue;
        ringGains[ ringIndex++ ] = positionGains[ position ] / sqrtf( (Float32) m_positionChannelCount[ position ] );
    }

    // near the listener the source widens toward all speakers, at the same total power
    const Float32 spread = MATH::Clamp01( spreadFactor );
    if ( spread < 1.0f )
        MATH::BlendGainsTowardUniform( ringGains, ringIndex, spread, m_targetPower );

    ringIndex = 0;
    for ( UInt32 i = 0; i < m_channelCount; ++i )
    {
        if ( NOT_PANNED != m_positionOfChannel[ i ] )
            gains[ i ] = ringGains[ ringIndex++ ];
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
