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

#ifndef GUCEF_AUDIO_CENVELOPEFOLLOWER_H
#include "gucefAUDIO_CEnvelopeFollower.h"
#define GUCEF_AUDIO_CENVELOPEFOLLOWER_H
#endif /* GUCEF_AUDIO_CENVELOPEFOLLOWER_H ? */

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

static const Float64 DenormalFlushThreshold = 1.0e-15;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEnvelopeFollower::CEnvelopeFollower( void )
    : m_mode( ENVELOPEMODE_PEAK )
    , m_sampleRate( 48000.0 )
    , m_attackMs( 10.0 )
    , m_releaseMs( 100.0 )
    , m_rmsWindowMs( 10.0 )
    , m_attackCoefficient( 0.0 )
    , m_releaseCoefficient( 0.0 )
    , m_rmsCoefficient( 0.0 )
    , m_state( 0.0 )
{GUCEF_TRACE;

    UpdateCoefficients();
}

/*-------------------------------------------------------------------------*/

CEnvelopeFollower::~CEnvelopeFollower()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Float64
CEnvelopeFollower::CoefficientForMs( Float64 milliseconds, Float64 sampleRate )
{
    if ( !( milliseconds > 0.0 ) || !( sampleRate > 0.0 ) )
        return 0.0;
    return exp( -1000.0 / ( milliseconds * sampleRate ) );
}

/*-------------------------------------------------------------------------*/

void
CEnvelopeFollower::UpdateCoefficients( void )
{
    m_attackCoefficient = CoefficientForMs( m_attackMs, m_sampleRate );
    m_releaseCoefficient = CoefficientForMs( m_releaseMs, m_sampleRate );
    m_rmsCoefficient = CoefficientForMs( m_rmsWindowMs, m_sampleRate );
}

/*-------------------------------------------------------------------------*/

bool
CEnvelopeFollower::Prepare( Float64 sampleRate )
{GUCEF_TRACE;

    if ( !( sampleRate > 0.0 ) )
        return false;
    m_sampleRate = sampleRate;
    UpdateCoefficients();
    m_state = 0.0;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEnvelopeFollower::SetMode( TEnvelopeMode mode )
{
    if ( mode != m_mode )
    {
        // the state is an amplitude in one mode and a mean square in the other
        m_mode = mode;
        m_state = 0.0;
    }
}

/*-------------------------------------------------------------------------*/

TEnvelopeMode
CEnvelopeFollower::GetMode( void ) const
{
    return m_mode;
}

/*-------------------------------------------------------------------------*/

void
CEnvelopeFollower::SetAttackTimeInMs( Float64 milliseconds )
{
    m_attackMs = milliseconds;
    m_attackCoefficient = CoefficientForMs( milliseconds, m_sampleRate );
}

/*-------------------------------------------------------------------------*/

void
CEnvelopeFollower::SetReleaseTimeInMs( Float64 milliseconds )
{
    m_releaseMs = milliseconds;
    m_releaseCoefficient = CoefficientForMs( milliseconds, m_sampleRate );
}

/*-------------------------------------------------------------------------*/

void
CEnvelopeFollower::SetRmsWindowInMs( Float64 milliseconds )
{
    m_rmsWindowMs = milliseconds;
    m_rmsCoefficient = CoefficientForMs( milliseconds, m_sampleRate );
}

/*-------------------------------------------------------------------------*/

void
CEnvelopeFollower::Reset( void )
{
    m_state = 0.0;
}

/*-------------------------------------------------------------------------*/

void
CEnvelopeFollower::ProcessBlock( const Float32* input, Float32* envelope, UInt32 frames, UInt32 stride )
{
    if ( GUCEF_NULL == envelope )
    {
        for ( UInt32 f=0; f<frames; ++f )
            Process( input[ (size_t) f * stride ] );
    }
    else
    {
        for ( UInt32 f=0; f<frames; ++f )
            envelope[ f ] = Process( input[ (size_t) f * stride ] );
    }
}

/*-------------------------------------------------------------------------*/

void
CEnvelopeFollower::FlushDenormals( void )
{
    // in RMS mode the state is a mean square: 1e-15 there is 1e-7.5 as amplitude, so the threshold is squared
    const Float64 threshold = ENVELOPEMODE_RMS == m_mode ? DenormalFlushThreshold * DenormalFlushThreshold : DenormalFlushThreshold;
    if ( m_state < threshold )
        m_state = 0.0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
