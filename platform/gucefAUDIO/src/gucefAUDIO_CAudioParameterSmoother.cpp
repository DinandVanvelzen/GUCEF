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

#ifndef GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H
#include "gucefAUDIO_CAudioParameterSmoother.h"
#define GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H
#endif /* GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H ? */

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

/** Closer than this (relative, in the smoothing domain) and the one pole has arrived */
const Float64 SettleTolerance = 1e-6;

/** Smallest value the logarithmic domain accepts */
const Float64 SmallestLogValue = 1e-12;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioParameterSmoother::CAudioParameterSmoother( void )
    : m_mode( AUDIOSMOOTHING_NONE )
    , m_logarithmic( false )
    , m_stepFrames( 1 )
    , m_rampSteps( 1 )
    , m_coefficient( 0.0 )
    , m_current( 0.0 )
    , m_target( 0.0 )
    , m_increment( 0.0 )
    , m_stepsLeft( 0 )
    , m_currentValue( 0.0f )
    , m_targetValue( 0.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAudioParameterSmoother::Prepare( TAudioSmoothing mode, Float64 timeInSeconds, Float64 sampleRate, bool logarithmic, UInt32 stepFrames )
{GUCEF_TRACE;

    m_mode = mode;
    m_logarithmic = logarithmic;
    m_stepFrames = stepFrames > 0 ? stepFrames : 1;

    const Float64 stepsPerSecond = ( sampleRate > 0.0 ? sampleRate : 48000.0 ) / (Float64) m_stepFrames;
    const Float64 steps = timeInSeconds > 0.0 ? timeInSeconds * stepsPerSecond : 0.0;
    if ( steps < 1.0 )
        m_mode = AUDIOSMOOTHING_NONE;

    m_rampSteps = steps >= 1.0 ? (UInt32) ( steps + 0.5 ) : 1;
    m_coefficient = steps >= 1.0 ? exp( -1.0 / steps ) : 0.0;
    Reset( m_currentValue );
}

/*-------------------------------------------------------------------------*/

Float64
CAudioParameterSmoother::ToDomain( Float32 value ) const
{
    if ( !m_logarithmic )
        return (Float64) value;
    const Float64 v = (Float64) value > SmallestLogValue ? (Float64) value : SmallestLogValue;
    return log( v ) / log( 2.0 );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioParameterSmoother::FromDomain( Float64 value ) const
{
    return m_logarithmic ? (Float32) pow( 2.0, value ) : (Float32) value;
}

/*-------------------------------------------------------------------------*/

void
CAudioParameterSmoother::Reset( Float32 value )
{
    m_currentValue = value;
    m_targetValue = value;
    m_current = ToDomain( value );
    m_target = m_current;
    m_increment = 0.0;
    m_stepsLeft = 0;
}

/*-------------------------------------------------------------------------*/

void
CAudioParameterSmoother::SetTarget( Float32 target )
{
    if ( AUDIOSMOOTHING_LINEAR_RAMP == m_mode )
    {
        SetTargetInSteps( target, m_rampSteps );
        return;
    }
    if ( AUDIOSMOOTHING_NONE == m_mode )
    {
        Reset( target );
        return;
    }
    m_targetValue = target;
    m_target = ToDomain( target );
}

/*-------------------------------------------------------------------------*/

void
CAudioParameterSmoother::SetTargetInSteps( Float32 target, UInt32 steps )
{
    if ( 0 == steps )
    {
        Reset( target );
        return;
    }
    m_targetValue = target;
    m_target = ToDomain( target );
    m_stepsLeft = steps;
    m_increment = ( m_target - m_current ) / (Float64) steps;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioParameterSmoother::Step( void )
{
    if ( m_stepsLeft > 0 )
    {
        // a linear ramp, whatever the mode it was started in
        if ( 0 == --m_stepsLeft )
        {
            m_current = m_target;
            m_currentValue = m_targetValue;
        }
        else
        {
            m_current += m_increment;
            m_currentValue = FromDomain( m_current );
        }
        return m_currentValue;
    }

    if ( AUDIOSMOOTHING_ONE_POLE == m_mode && m_current != m_target )
    {
        m_current = m_target + ( m_current - m_target ) * m_coefficient;
        const Float64 scale = fabs( m_target ) > 1.0 ? fabs( m_target ) : 1.0;
        if ( fabs( m_current - m_target ) <= SettleTolerance * scale )
        {
            m_current = m_target;
            m_currentValue = m_targetValue;
        }
        else
            m_currentValue = FromDomain( m_current );
    }
    return m_currentValue;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioParameterSmoother::GetValue( void ) const
{
    return m_currentValue;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioParameterSmoother::GetTarget( void ) const
{
    return m_targetValue;
}

/*-------------------------------------------------------------------------*/

bool
CAudioParameterSmoother::IsSmoothing( void ) const
{
    return m_stepsLeft > 0 || m_current != m_target;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioParameterSmoother::GetStepFrames( void ) const
{
    return m_stepFrames;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
