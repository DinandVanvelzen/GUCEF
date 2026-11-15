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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#include "gucefAUDIO_CAudioEngine.h"
#define GUCEF_AUDIO_CAUDIOENGINE_H
#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOATTENUATION_H
#include "gucefAUDIO_CAudioAttenuation.h"
#define GUCEF_AUDIO_CAUDIOATTENUATION_H
#endif /* GUCEF_AUDIO_CAUDIOATTENUATION_H ? */

#ifndef GUCEF_AUDIO_CAUDIOSPATIALMATH_H
#include "gucefAUDIO_CAudioSpatialMath.h"
#define GUCEF_AUDIO_CAUDIOSPATIALMATH_H
#endif /* GUCEF_AUDIO_CAUDIOSPATIALMATH_H ? */

#ifndef GUCEF_AUDIO_CAUDIOPANNER_H
#include "gucefAUDIO_CAudioPanner.h"
#define GUCEF_AUDIO_CAUDIOPANNER_H
#endif /* GUCEF_AUDIO_CAUDIOPANNER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOLISTENER_H
#include "gucefAUDIO_CAudioListener.h"
#define GUCEF_AUDIO_CAUDIOLISTENER_H
#endif /* GUCEF_AUDIO_CAUDIOLISTENER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEMITTER_H
#include "gucefAUDIO_CAudioEmitter.h"
#define GUCEF_AUDIO_CAUDIOEMITTER_H
#endif /* GUCEF_AUDIO_CAUDIOEMITTER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOSPATIALPARAMS_H
#include "gucefAUDIO_CAudioSpatialParams.h"
#define GUCEF_AUDIO_CAUDIOSPATIALPARAMS_H
#endif /* GUCEF_AUDIO_CAUDIOSPATIALPARAMS_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioSpatial.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef AUDIO::CAudioSpatialMath TMath;
typedef TMath::TPoint3 TPoint3;
typedef TMath::TVector3 TVector3;

const Float64 Pi = 3.14159265358979323846;
const Float64 DegreesToRadians = Pi / 180.0;

bool
Near( Float64 a, Float64 b, Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

/*
 *  The functions of the audio thread are called through functions marked as real time, so the RealtimeSanitizer build checks
 *  everything they do
 */

void
RtLocate( const TPoint3& listenerPosition, const TVector3& forward, const TVector3& up, const TPoint3& source, bool relative, TMath::SFix& fix ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    TMath::Locate( listenerPosition, forward, up, source, relative, fix );
}

void
RtLocate2D( const TPoint3& listenerPosition, Float32 width, const TPoint3& source, bool relative, TMath::SFix& fix ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    TMath::Locate2D( listenerPosition, width, source, relative, fix );
}

void
RtCone( const TVector3& direction, const TVector3& toListener, Float32 inner, Float32 outer, Float32 outerGain, Float32 outerCutoff, TMath::SConeResult& result ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    TMath::EvaluateCone( direction, toListener, inner, outer, outerGain, outerCutoff, result );
}

Float32
RtDoppler( const TPoint3& lp, const TVector3& lv, const TPoint3& sp, const TVector3& sv, Float32 c, Float32 factor, Float32 minRatio, Float32 maxRatio ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return TMath::GetDopplerRatio( lp, lv, sp, sv, c, factor, minRatio, maxRatio );
}

Float32
RtSmooth( Float32 previous, Float32 target, Float32 seconds, Float32 ms ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return TMath::SmoothDopplerRatio( previous, target, seconds, ms );
}

Float32
RtOcclusionGain( Float32 amount, Float32 db ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return TMath::GetOcclusionGain( amount, db );
}

Float32
RtOcclusionCutoff( Float32 amount, Float32 cutoff ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return TMath::GetOcclusionCutoff( amount, cutoff );
}

Float32
RtAttenuate( const AUDIO::CAudioAttenuation& attenuation, Float32 distance ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return attenuation.Evaluate( distance );
}

Float32
RtAttenuateCutoff( const AUDIO::CAudioAttenuation& attenuation, Float32 distance, Float32 nearCutoff, Float32 farCutoff ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return attenuation.EvaluateCutoff( distance, nearCutoff, farCutoff );
}

bool
RtGains( const AUDIO::CAudioPanner& panner, Float32 azimuth, Float32 pan, Float32 spread, Float32 rear, Float32* gains ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return panner.GetGains( azimuth, pan, spread, rear, gains );
}

Float32
RtSpread( Float32 distance, Float32 radius ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return AUDIO::CAudioPanner::GetSpreadFactor( distance, radius );
}

bool
RtReadListener( AUDIO::CAudioListener& listener, AUDIO::SListenerState& state ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return listener.ReadState( state );
}

bool
RtReadEmitter( AUDIO::CAudioEmitter& emitter, AUDIO::SEmitterState& state ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return emitter.ReadState( state );
}

bool
RtReadWorld( AUDIO::CAudioEngine& engine, AUDIO::SSpatialWorldState& state ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return engine.ReadSpatialWorldState( state );
}

Float32
RtEffectiveSpread( const AUDIO::SEmitterState& emitter, const AUDIO::SSpatialWorldState& world ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return emitter.GetEffectiveSpreadRadius( world );
}

Float32
RtEffectiveGain( const AUDIO::SEmitterState& emitter, const AUDIO::SSpatialWorldState& world, Float32 distance ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return emitter.GetEffectiveAttenuation( world ).Evaluate( distance );
}

TPoint3
P( Float32 x, Float32 y, Float32 z )
{
    return TPoint3( x, y, z );
}

TVector3
V( Float32 x, Float32 y, Float32 z )
{
    return TVector3( x, y, z );
}

/** The layouts of the tests */
UInt32
LayoutOf( UInt32 channels )
{
    return AUDIO::CAudioFormat::GetDefaultChannelLayout( channels );
}

UInt32
Layout714( void )
{
    return LayoutOf( 8 ) | AUDIO::AUDIOSPEAKER_TOP_FRONT_LEFT | AUDIO::AUDIOSPEAKER_TOP_FRONT_RIGHT | AUDIO::AUDIOSPEAKER_TOP_BACK_LEFT | AUDIO::AUDIOSPEAKER_TOP_BACK_RIGHT;
}

UInt32
LayoutWithTwoCenters( void )
{
    return AUDIO::AUDIOSPEAKER_FRONT_LEFT | AUDIO::AUDIOSPEAKER_FRONT_RIGHT | AUDIO::AUDIOSPEAKER_FRONT_CENTER | AUDIO::AUDIOSPEAKER_TOP_FRONT_CENTER | AUDIO::AUDIOSPEAKER_BACK_LEFT | AUDIO::AUDIOSPEAKER_BACK_RIGHT;
}

Float64
SumOfSquares( const Float32* gains, UInt32 count )
{
    Float64 sum = 0.0;
    for ( UInt32 i = 0; i < count; ++i )
        sum += (Float64) gains[ i ] * (Float64) gains[ i ];
    return sum;
}

/** Gains for a direction in the 3D mode: the pan is the sine of the azimuth */
void
GainsForAzimuth( const AUDIO::CAudioPanner& panner, Float64 degrees, Float32 spread, Float32* gains )
{
    const Float32 azimuth = (Float32) ( degrees * DegreesToRadians );
    RtGains( panner, azimuth, sinf( azimuth ), spread, 1.0f, gains );
}

AUDIO::CAudioEngineParams
NullParams( void )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    params.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
    return params;
}

/** Runs a function on its own thread */
class CSpatialTestThread : public MT::CActiveObject
{
    public:

    typedef void ( *TFunction )( void* argument );

    CSpatialTestThread( TFunction function, void* argument ) : MT::CActiveObject(), m_function( function ), m_argument( argument ), m_done( false ) {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_function( m_argument );
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true;
    }
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool WaitUntilDone( UInt32 timeoutInMs )
    {
        UInt32 waited = 0;
        while ( ( !m_done.Load( MT::MEMORDER_ACQUIRE ) || IsActive() ) && waited < timeoutInMs )
        {
            MT::PrecisionDelay( 1 );
            ++waited;
        }
        return m_done.Load( MT::MEMORDER_ACQUIRE ) && !IsActive();
    }

    private:

    TFunction m_function;
    void* m_argument;
    MT::CTAtomic< bool > m_done;
};

/*
 *  The stress run: one thread plays the game and updates 200 emitters and the listener once per millisecond, a second one changes the world
 *  settings, and a third one is the audio thread and reads all of it as fast as it can. Every update carries its own number in all values
 *  that belong together, so a snapshot that mixes two updates shows.
 */
const UInt32 StressEmitterCount = 200;
const UInt32 StressDurationInMs = 1500;

struct SStress
{
    AUDIO::CAudioListener listener;
    AUDIO::CAudioEmitter* emitters[ StressEmitterCount ];
    AUDIO::CAudioEngine engine;
    MT::CTAtomic< bool > stop;
    UInt64 gameTicks;
    UInt64 worldChanges;
    UInt64 readRounds;
    UInt64 newEmitterReads;
    UInt64 torn;
    UInt64 wentBackwards;
    UInt64 lastListener;
    UInt64 lastEmitter[ StressEmitterCount ];

    SStress( void ) : listener(), engine(), stop( false ), gameTicks( 0 ), worldChanges( 0 ), readRounds( 0 ), newEmitterReads( 0 ), torn( 0 ), wentBackwards( 0 ), lastListener( 0 )
    {
        for ( UInt32 i = 0; i < StressEmitterCount; ++i )
        {
            emitters[ i ] = GUCEF_NEW AUDIO::CAudioEmitter();
            lastEmitter[ i ] = 0;
        }
    }

    ~SStress()
    {
        for ( UInt32 i = 0; i < StressEmitterCount; ++i )
            GUCEF_DELETE emitters[ i ];
    }

    static void Game( void* argument )
    {
        SStress& s = *static_cast< SStress* >( argument );
        for ( UInt32 tick = 1; !s.stop.Load( MT::MEMORDER_ACQUIRE ); ++tick )
        {
            const Float32 n = (Float32) tick;
            for ( UInt32 i = 0; i < StressEmitterCount; ++i )
            {
                s.emitters[ i ]->SetPositionAndVelocity( P( n, n, n ), V( n, n, n ) );
            }

            AUDIO::SListenerState state;
            state.position = P( n, 0.0f, 0.0f );
            state.velocity = V( n, 0.0f, 0.0f );
            state.panWidth2D = n + 1.0f;
            s.listener.SetState( state );
            ++s.gameTicks;
            MT::PrecisionDelay( 1 );
        }
    }

    static void World( void* argument )
    {
        SStress& s = *static_cast< SStress* >( argument );
        for ( UInt32 change = 1; !s.stop.Load( MT::MEMORDER_ACQUIRE ); ++change )
        {
            // the speed of sound and the smoothing time always differ by 100 when they are from the same call
            const UInt32 j = 1 + ( change % 800 );
            AUDIO::CAudioSpatialParams params;
            params.SetSpeedOfSound( 100.0f + (Float32) j );
            params.SetDopplerSmoothingInMs( (Float32) j );
            s.engine.SetSpatialParams( params );
            ++s.worldChanges;
            MT::PrecisionDelay( 1 );
        }
    }

    static void ReadAll( SStress& s ) GUCEF_AUDIO_REALTIME_CONTEXT
    {
        AUDIO::SEmitterState emitter;
        for ( UInt32 i = 0; i < StressEmitterCount; ++i )
        {
            const bool isNew = RtReadEmitter( *s.emitters[ i ], emitter );
            const Float32 n = emitter.position.x;
            if ( n != emitter.position.y || n != emitter.position.z || n != emitter.velocity.x || n != emitter.velocity.y || n != emitter.velocity.z )
                ++s.torn;
            if ( isNew )
            {
                ++s.newEmitterReads;
                if ( (UInt64) n < s.lastEmitter[ i ] )
                    ++s.wentBackwards;
                s.lastEmitter[ i ] = (UInt64) n;
            }
        }

        AUDIO::SListenerState listener;
        if ( RtReadListener( s.listener, listener ) )
        {
            if ( listener.position.x != listener.velocity.x || ( listener.position.x > 0.0f && listener.panWidth2D != listener.position.x + 1.0f ) )     // tick 0 is the state of the construction
                ++s.torn;
            if ( (UInt64) listener.position.x < s.lastListener )
                ++s.wentBackwards;
            s.lastListener = (UInt64) listener.position.x;
        }

        AUDIO::SSpatialWorldState world;
        if ( RtReadWorld( s.engine, world ) )
        {
            if ( world.speedOfSoundInUnits - 100.0f != world.dopplerSmoothingInMs )
                ++s.torn;
        }
        ++s.readRounds;
    }

    static void Audio( void* argument )
    {
        SStress& s = *static_cast< SStress* >( argument );
        while ( !s.stop.Load( MT::MEMORDER_ACQUIRE ) )
            ReadAll( s );
        ReadAll( s );
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioSpatialTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Spatial" );

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: Distance models at dMin, 2 dMin, dMax, beyond dMax and below dMin follow the formulas" )
    try
    {
        // the default is the OpenAL one: inverse, clamped, 1, 100, rolloff 1
        AUDIO::CAudioAttenuation defaults;
        ASSERT_TRUE( MATH::DISTANCEMODEL_INVERSE_CLAMPED == defaults.GetModel() && 1.0f == defaults.GetMinDistance() && 100.0f == defaults.GetMaxDistance() && 1.0f == defaults.GetRolloff() );

        const Float64 dMin = 2.0;
        const Float64 dMax = 50.0;
        const Float64 rolloff = 1.5;
        const Float64 distances[ 6 ] = { 0.0, 0.5, dMin, 2.0 * dMin, dMax, 120.0 };
        for ( UInt32 i = 0; i < 6; ++i )
        {
            const Float64 d = distances[ i ];
            const Float64 clamped = d < dMin ? dMin : ( d > dMax ? dMax : d );

            AUDIO::CAudioAttenuation inverse( MATH::DISTANCEMODEL_INVERSE_CLAMPED, (Float32) dMin, (Float32) dMax, (Float32) rolloff );
            ASSERT_TRUE( Near( dMin / ( dMin + rolloff * ( clamped - dMin ) ), RtAttenuate( inverse, (Float32) d ), 1.0e-6 ) );

            AUDIO::CAudioAttenuation linear( MATH::DISTANCEMODEL_LINEAR_CLAMPED, (Float32) dMin, (Float32) dMax, (Float32) rolloff );
            Float64 expectedLinear = 1.0 - rolloff * ( clamped - dMin ) / ( dMax - dMin );
            expectedLinear = expectedLinear < 0.0 ? 0.0 : ( expectedLinear > 1.0 ? 1.0 : expectedLinear );
            ASSERT_TRUE( Near( expectedLinear, RtAttenuate( linear, (Float32) d ), 1.0e-6 ) );

            AUDIO::CAudioAttenuation exponential( MATH::DISTANCEMODEL_EXPONENT_CLAMPED, (Float32) dMin, (Float32) dMax, (Float32) rolloff );
            ASSERT_TRUE( Near( pow( clamped / dMin, -rolloff ), RtAttenuate( exponential, (Float32) d ), 1.0e-6 ) );

            AUDIO::CAudioAttenuation none( MATH::DISTANCEMODEL_NONE, (Float32) dMin, (Float32) dMax, (Float32) rolloff );
            ASSERT_TRUE( 1.0f == RtAttenuate( none, (Float32) d ) );
        }

        // the models that are not clamped keep falling beyond the maximum distance
        AUDIO::CAudioAttenuation unclamped( MATH::DISTANCEMODEL_INVERSE, 2.0f, 50.0f, 1.0f );
        ASSERT_TRUE( Near( 2.0 / ( 2.0 + ( 120.0 - 2.0 ) ), RtAttenuate( unclamped, 120.0f ), 1.0e-6 ) );
        ASSERT_TRUE( Near( 2.0 / ( 2.0 + ( 0.5 - 2.0 ) ), RtAttenuate( unclamped, 0.5f ), 1.0e-6 ) );

        // the settings have limits: nothing negative, nothing that is not a number
        AUDIO::CAudioAttenuation limited;
        limited.SetMinDistance( -4.0f );
        limited.SetMaxDistance( -4.0f );
        limited.SetRolloff( -1.0f );
        ASSERT_TRUE( 0.0f == limited.GetMinDistance() && 0.0f == limited.GetMaxDistance() && 0.0f == limited.GetRolloff() );
        limited.SetMinDistance( 3.0f );
        limited.SetMinDistance( 0.0f / 0.0f );
        ASSERT_TRUE( 3.0f == limited.GetMinDistance() );
        const Float32 gain = RtAttenuate( limited, 7.0f );
        ASSERT_TRUE( gain == gain && gain >= 0.0f && gain < 1.0e30f );

        // a copy is the same and independent
        AUDIO::CAudioAttenuation original( MATH::DISTANCEMODEL_LINEAR_CLAMPED, 1.0f, 10.0f, 1.0f );
        AUDIO::CAudioAttenuation copy( original );
        ASSERT_TRUE( copy == original );
        copy.SetRolloff( 2.0f );
        ASSERT_TRUE( copy != original && 1.0f == original.GetRolloff() );
        copy = original;
        ASSERT_TRUE( copy == original );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: The curve model interpolates in dB over the normalised distance, the distance low pass logarithmically" )
    try
    {
        AUDIO::CAudioAttenuation curve( MATH::DISTANCEMODEL_CURVE, 1.0f, 101.0f, 1.0f );
        ASSERT_TRUE( curve.AddCurvePoint( 0.0f, 0.0f ) && curve.AddCurvePoint( 0.5f, -6.0f ) && curve.AddCurvePoint( 1.0f, -40.0f ) );
        ASSERT_TRUE( 3 == curve.GetCurvePointCount() && 0.5f == curve.GetCurvePointDistance( 1 ) && -6.0f == curve.GetCurvePointGainInDecibels( 1 ) );

        ASSERT_TRUE( Near( 1.0, RtAttenuate( curve, 0.0f ), 1.0e-6 ) );                     // below dMin: the first point
        ASSERT_TRUE( Near( 1.0, RtAttenuate( curve, 1.0f ), 1.0e-6 ) );
        ASSERT_TRUE( Near( pow( 10.0, -6.0 / 20.0 ), RtAttenuate( curve, 51.0f ), 1.0e-6 ) );   // normalised 0.5
        ASSERT_TRUE( Near( pow( 10.0, ( -6.0 + 0.5 * ( -34.0 ) ) / 20.0 ), RtAttenuate( curve, 76.0f ), 1.0e-6 ) );   // normalised 0.75: between -6 and -40
        ASSERT_TRUE( Near( pow( 10.0, -40.0 / 20.0 ), RtAttenuate( curve, 101.0f ), 1.0e-6 ) );
        ASSERT_TRUE( Near( pow( 10.0, -40.0 / 20.0 ), RtAttenuate( curve, 5000.0f ), 1.0e-6 ) );   // beyond dMax: the last point

        // up to 16 points, a point at a distance that is there replaces its gain
        curve.ClearCurve();
        for ( UInt32 i = 0; i < AUDIO::CAudioAttenuation::MaxCurvePoints; ++i )
            ASSERT_TRUE( curve.AddCurvePoint( (Float32) i / 16.0f, -(Float32) i ) );
        ASSERT_FALSE( curve.AddCurvePoint( 0.99f, -1.0f ) );
        ASSERT_TRUE( curve.AddCurvePoint( 0.0f, -3.0f ) && -3.0f == curve.GetCurvePointGainInDecibels( 0 ) && 16 == curve.GetCurvePointCount() );
        ASSERT_FALSE( curve.AddCurvePoint( 0.0f / 0.0f, 0.0f ) );

        // the cutoff goes from the near value at dMin to the far value at dMax, logarithmically
        AUDIO::CAudioAttenuation attenuation( MATH::DISTANCEMODEL_INVERSE_CLAMPED, 1.0f, 101.0f, 1.0f );
        ASSERT_TRUE( Near( 20000.0, RtAttenuateCutoff( attenuation, 0.5f, 20000.0f, 1000.0f ), 0.01 ) );
        ASSERT_TRUE( Near( 20000.0, RtAttenuateCutoff( attenuation, 1.0f, 20000.0f, 1000.0f ), 0.01 ) );
        ASSERT_TRUE( Near( sqrt( 20000.0 * 1000.0 ), RtAttenuateCutoff( attenuation, 51.0f, 20000.0f, 1000.0f ), 0.05 ) );
        ASSERT_TRUE( Near( 1000.0, RtAttenuateCutoff( attenuation, 101.0f, 20000.0f, 1000.0f ), 0.01 ) );
        ASSERT_TRUE( Near( 1000.0, RtAttenuateCutoff( attenuation, 900.0f, 20000.0f, 1000.0f ), 0.01 ) );
        ASSERT_TRUE( Near( 20000.0, RtAttenuateCutoff( attenuation, 51.0f, 20000.0f, 0.0f ), 0.01 ) );      // 0: no low pass
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: The attenuation is saved and loaded as a config section, with the names of the design" )
    try
    {
        AUDIO::CAudioAttenuation original( MATH::DISTANCEMODEL_CURVE, 2.5f, 80.0f, 1.25f );
        original.AddCurvePoint( 0.0f, 0.0f );
        original.AddCurvePoint( 0.4f, -12.0f );
        original.AddCurvePoint( 1.0f, -60.0f );

        CORE::CDataNode config( "Config" );
        ASSERT_TRUE( original.SaveConfig( config ) );
        const CORE::CDataNode* section = config.FindChild( "Attenuation" );
        ASSERT_TRUE( GUCEF_NULL != section && "curve" == section->GetAttributeValue( "model" ).AsString() );
        ASSERT_TRUE( GUCEF_NULL != section->FindChild( "Curve" ) && 3 == section->FindChild( "Curve" )->GetNrOfDirectChildNodes() );

        AUDIO::CAudioAttenuation loaded;
        ASSERT_TRUE( loaded.LoadConfig( config ) && loaded == original );
        AUDIO::CAudioAttenuation fromSection;
        ASSERT_TRUE( GUCEF_NULL != section && fromSection.LoadConfig( *section ) && fromSection == original );

        // saving again replaces the section and the curve
        original.ClearCurve();
        original.AddCurvePoint( 0.5f, -3.0f );
        ASSERT_TRUE( original.SaveConfig( config ) && 1 == config.GetNrOfDirectChildNodes() );
        AUDIO::CAudioAttenuation reloaded;
        ASSERT_TRUE( reloaded.LoadConfig( config ) && 1 == reloaded.GetCurvePointCount() );

        // the model names, all of them in both directions
        const MATH::TDistanceModel models[ 8 ] = { MATH::DISTANCEMODEL_NONE, MATH::DISTANCEMODEL_INVERSE, MATH::DISTANCEMODEL_INVERSE_CLAMPED, MATH::DISTANCEMODEL_LINEAR, MATH::DISTANCEMODEL_LINEAR_CLAMPED,
                                                   MATH::DISTANCEMODEL_EXPONENT, MATH::DISTANCEMODEL_EXPONENT_CLAMPED, MATH::DISTANCEMODEL_CURVE };
        for ( UInt32 i = 0; i < 8; ++i )
        {
            MATH::TDistanceModel parsed = MATH::DISTANCEMODEL_NONE;
            ASSERT_TRUE( AUDIO::CAudioAttenuation::StringToModel( AUDIO::CAudioAttenuation::ModelToString( models[ i ] ), parsed ) && models[ i ] == parsed );
        }
        MATH::TDistanceModel parsed = MATH::DISTANCEMODEL_NONE;
        ASSERT_TRUE( AUDIO::CAudioAttenuation::StringToModel( "Inverse", parsed ) && MATH::DISTANCEMODEL_INVERSE_CLAMPED == parsed );       // the design's names are the clamped models
        ASSERT_TRUE( AUDIO::CAudioAttenuation::StringToModel( "exponential", parsed ) && MATH::DISTANCEMODEL_EXPONENT_CLAMPED == parsed );
        ASSERT_TRUE( AUDIO::CAudioAttenuation::StringToModel( "exponent", parsed ) && MATH::DISTANCEMODEL_EXPONENT == parsed );             // and the names of gucefMATH are read
        ASSERT_TRUE( AUDIO::CAudioAttenuation::StringToModel( "linearclamped", parsed ) && MATH::DISTANCEMODEL_LINEAR_CLAMPED == parsed );
        ASSERT_FALSE( AUDIO::CAudioAttenuation::StringToModel( "cubic", parsed ) );
        ASSERT_TRUE( "inverse" == CORE::CString( AUDIO::CAudioAttenuation::ModelToString( MATH::DISTANCEMODEL_INVERSE_CLAMPED ) ) );
        ASSERT_TRUE( "inverseUnclamped" == CORE::CString( AUDIO::CAudioAttenuation::ModelToString( MATH::DISTANCEMODEL_INVERSE ) ) );

        // what is not there keeps its value, what is not valid is ignored
        CORE::CDataNode partial( "Attenuation" );
        partial.SetAttribute( "model", CORE::CString( "cubic" ) );
        partial.SetAttribute( "maxDistance", 33.0f );
        partial.SetAttribute( "rolloff", -5.0f );
        AUDIO::CAudioAttenuation target( MATH::DISTANCEMODEL_LINEAR_CLAMPED, 4.0f, 90.0f, 2.0f );
        ASSERT_TRUE( target.LoadConfig( partial ) );
        ASSERT_TRUE( MATH::DISTANCEMODEL_LINEAR_CLAMPED == target.GetModel() && 4.0f == target.GetMinDistance() && 33.0f == target.GetMaxDistance() && 0.0f == target.GetRolloff() );

        CORE::CDataNode empty( "Config" );
        ASSERT_FALSE( AUDIO::CAudioAttenuation().LoadConfig( empty ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: The cone gives 1 inside, the outer gain outside and a linear step between, with a cutoff that follows" )
    try
    {
        // 60 degrees inner, 180 outer: full gain up to 30 degrees off axis, the outer gain from 90 degrees
        const TVector3 axis = V( 0.0f, 0.0f, -1.0f );
        const Float32 outerGain = 0.2f;
        TMath::SConeResult cone;
        const Float64 angles[ 7 ] = { 0.0, 30.0, 60.0, 90.0, 120.0, 170.0, 180.0 };
        const Float64 expectedFactor[ 7 ] = { 0.0, 0.0, 0.5, 1.0, 1.0, 1.0, 1.0 };
        for ( UInt32 i = 0; i < 7; ++i )
        {
            const Float64 a = angles[ i ] * DegreesToRadians;
            RtCone( axis, V( (Float32) sin( a ), 0.0f, (Float32) -cos( a ) ), 60.0f, 180.0f, outerGain, 1000.0f, cone );
            ASSERT_TRUE( Near( 1.0 + ( outerGain - 1.0 ) * expectedFactor[ i ], cone.gain, 1.0e-5 ) );
            ASSERT_TRUE( Near( exp( log( 20000.0 ) + ( log( 1000.0 ) - log( 20000.0 ) ) * expectedFactor[ i ] ), cone.cutoff, 0.05 ) );
        }

        // without an outer cutoff there is no low pass
        RtCone( axis, V( 1.0f, 0.0f, 0.0f ), 60.0f, 180.0f, outerGain, 0.0f, cone );
        ASSERT_TRUE( Near( outerGain, cone.gain, 1.0e-6 ) && 20000.0f == cone.cutoff );

        // no cone (360 and 360), no direction, a listener on the emitter: all give 1
        RtCone( axis, V( 0.0f, 0.0f, 1.0f ), 360.0f, 360.0f, outerGain, 1000.0f, cone );
        ASSERT_TRUE( 1.0f == cone.gain && 20000.0f == cone.cutoff );
        RtCone( V( 0.0f, 0.0f, 0.0f ), V( 0.0f, 0.0f, 1.0f ), 60.0f, 180.0f, outerGain, 1000.0f, cone );
        ASSERT_TRUE( 1.0f == cone.gain && 20000.0f == cone.cutoff );
        RtCone( axis, V( 0.0f, 0.0f, 0.0f ), 60.0f, 180.0f, outerGain, 1000.0f, cone );
        ASSERT_TRUE( 1.0f == cone.gain && 20000.0f == cone.cutoff );

        // the length of the vectors does not matter
        RtCone( V( 0.0f, 0.0f, -7.0f ), V( 300.0f, 0.0f, 0.0f ), 60.0f, 180.0f, outerGain, 0.0f, cone );
        ASSERT_TRUE( Near( outerGain, cone.gain, 1.0e-6 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: Occlusion maps to a gain and a cutoff, obstruction touches the direct path only" )
    try
    {
        ASSERT_TRUE( 0.7f == TMath::GetDirectOcclusionAmount( 0.7f, 0.2f ) && 0.9f == TMath::GetDirectOcclusionAmount( 0.7f, 0.9f ) );
        ASSERT_TRUE( 1.0f == TMath::GetDirectOcclusionAmount( 4.0f, 0.0f ) && 0.0f == TMath::GetDirectOcclusionAmount( -1.0f, -2.0f ) );

        const Float64 occlusionGain = pow( 10.0, -12.0 / 20.0 );
        ASSERT_TRUE( Near( 1.0, RtOcclusionGain( 0.0f, -12.0f ), 1.0e-6 ) );
        ASSERT_TRUE( Near( occlusionGain, RtOcclusionGain( 1.0f, -12.0f ), 1.0e-6 ) );
        ASSERT_TRUE( Near( 1.0 + ( occlusionGain - 1.0 ) * 0.25, RtOcclusionGain( 0.25f, -12.0f ), 1.0e-6 ) );     // linear between 1 and the gain of full occlusion
        ASSERT_TRUE( Near( occlusionGain, RtOcclusionGain( 7.0f, -12.0f ), 1.0e-6 ) );

        ASSERT_TRUE( Near( 20000.0, RtOcclusionCutoff( 0.0f, 1500.0f ), 0.01 ) );
        ASSERT_TRUE( Near( 1500.0, RtOcclusionCutoff( 1.0f, 1500.0f ), 0.01 ) );
        ASSERT_TRUE( Near( exp( 0.5 * log( 20000.0 ) + 0.5 * log( 1500.0 ) ), RtOcclusionCutoff( 0.5f, 1500.0f ), 0.05 ) );
        ASSERT_TRUE( 20000.0f == RtOcclusionCutoff( 1.0f, 0.0f ) && 20000.0f == RtOcclusionCutoff( 1.0f, 30000.0f ) );   // no cutoff set: no low pass

        // the sends of a voice use the occlusion alone: an obstructed but not occluded sound keeps its sends
        const Float32 occlusion = 0.0f;
        const Float32 obstruction = 1.0f;
        const Float32 direct = RtOcclusionGain( TMath::GetDirectOcclusionAmount( occlusion, obstruction ), -12.0f );
        const Float32 send = RtOcclusionGain( occlusion, -12.0f );
        ASSERT_TRUE( Near( occlusionGain, direct, 1.0e-6 ) && Near( 1.0, send, 1.0e-6 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: Doppler ratio of an approaching source and listener, the limits, factor 0, and the smoothing" )
    try
    {
        const Float32 c = 343.3f;
        const Float32 v = 34.33f;
        const TPoint3 listener = P( 0.0f, 0.0f, 0.0f );
        const TPoint3 source = P( 100.0f, 0.0f, 0.0f );

        // the source approaches at a tenth of the speed of sound
        ASSERT_TRUE( Near( 343.3 / 308.97, RtDoppler( listener, V( 0.0f, 0.0f, 0.0f ), source, V( -v, 0.0f, 0.0f ), c, 1.0f, 0.5f, 2.0f ), 1.0e-5 ) );
        // the listener approaches: ( c + v ) / c
        ASSERT_TRUE( Near( ( 343.3 + 34.33 ) / 343.3, RtDoppler( listener, V( v, 0.0f, 0.0f ), source, V( 0.0f, 0.0f, 0.0f ), c, 1.0f, 0.5f, 2.0f ), 1.0e-5 ) );
        // moving away gives a lower pitch
        ASSERT_TRUE( Near( 343.3 / ( 343.3 + 34.33 ), RtDoppler( listener, V( 0.0f, 0.0f, 0.0f ), source, V( v, 0.0f, 0.0f ), c, 1.0f, 0.5f, 2.0f ), 1.0e-5 ) );
        // only the speed along the line between them counts
        ASSERT_TRUE( 1.0f == RtDoppler( listener, V( 0.0f, 0.0f, 0.0f ), source, V( 0.0f, 80.0f, 0.0f ), c, 1.0f, 0.5f, 2.0f ) );
        ASSERT_TRUE( Near( 343.3 / ( 343.3 - 0.8 * 34.33 ), RtDoppler( listener, V( 0.0f, 0.0f, 0.0f ), source, V( -0.8f * v, 0.6f * v, 0.0f ), c, 1.0f, 0.5f, 2.0f ), 1.0e-5 ) );

        // the limits are respected, the factor scales the speeds, 0 switches it off, no distance gives 1
        ASSERT_TRUE( 1.1f == RtDoppler( listener, V( 0.0f, 0.0f, 0.0f ), source, V( -200.0f, 0.0f, 0.0f ), c, 1.0f, 0.5f, 1.1f ) );
        ASSERT_TRUE( 0.9f == RtDoppler( listener, V( 0.0f, 0.0f, 0.0f ), source, V( 200.0f, 0.0f, 0.0f ), c, 1.0f, 0.9f, 2.0f ) );
        ASSERT_TRUE( Near( 343.3 / ( 343.3 - 2.0 * 34.33 ), RtDoppler( listener, V( 0.0f, 0.0f, 0.0f ), source, V( -v, 0.0f, 0.0f ), c, 2.0f, 0.5f, 2.0f ), 1.0e-5 ) );
        ASSERT_TRUE( 1.0f == RtDoppler( listener, V( 0.0f, 0.0f, 0.0f ), source, V( -v, 0.0f, 0.0f ), c, 0.0f, 0.5f, 2.0f ) );
        ASSERT_TRUE( 1.0f == RtDoppler( listener, V( 9.0f, 0.0f, 0.0f ), source, V( -v, 0.0f, 0.0f ), 0.0f, 1.0f, 0.5f, 2.0f ) );
        ASSERT_TRUE( 1.0f == RtDoppler( source, V( 9.0f, 0.0f, 0.0f ), source, V( -v, 0.0f, 0.0f ), c, 1.0f, 0.5f, 2.0f ) );
        const Float32 faster = RtDoppler( listener, V( 0.0f, 0.0f, 0.0f ), source, V( -5000.0f, 0.0f, 0.0f ), c, 1.0f, 0.5f, 100.0f );     // faster than sound
        ASSERT_TRUE( faster == faster && faster <= 100.0f && faster >= 0.5f );

        // the smoothing is a one pole: after one time constant 1 - 1 / e of the way, the same whatever the block size
        const Float64 oneStep = RtSmooth( 1.0f, 2.0f, 0.05f, 50.0f );
        ASSERT_TRUE( Near( 1.0 + ( 1.0 - exp( -1.0 ) ), oneStep, 1.0e-5 ) );
        Float32 stepped = 1.0f;
        for ( UInt32 i = 0; i < 10; ++i )
            stepped = RtSmooth( stepped, 2.0f, 0.005f, 50.0f );
        ASSERT_TRUE( Near( oneStep, stepped, 1.0e-5 ) );
        ASSERT_TRUE( 2.0f == RtSmooth( 1.0f, 2.0f, 0.005f, 0.0f ) );        // no smoothing: the target at once
        ASSERT_TRUE( 1.0f == RtSmooth( 1.0f, 2.0f, 0.0f, 50.0f ) );         // no time has passed
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: The stereo law gives the gains of the design at 0, 30, 90 and 180 degrees, and the total power is 2" )
    try
    {
        AUDIO::CAudioPanner panner;
        ASSERT_FALSE( panner.IsPrepared() );
        ASSERT_TRUE( panner.Prepare( 2, 0, true ) );
        ASSERT_TRUE( panner.IsPrepared() && panner.IsStereo() && 2 == panner.GetChannelCount() && 2 == panner.GetRingChannelCount() && panner.GetLfeChannel() < 0 );

        const Float64 degrees[ 9 ] = { 0.0, 30.0, -30.0, 90.0, -90.0, 180.0, -180.0, 150.0, -45.0 };
        for ( UInt32 i = 0; i < 9; ++i )
        {
            const Float64 azimuth = degrees[ i ] * DegreesToRadians;
            const Float64 pan = sin( azimuth );
            const Float64 angle = ( pan + 1.0 ) * Pi / 4.0;
            Float32 gains[ 2 ];
            GainsForAzimuth( panner, degrees[ i ], 1.0f, gains );
            ASSERT_TRUE( Near( sqrt( 2.0 ) * cos( angle ), gains[ 0 ], 1.0e-5 ) && Near( sqrt( 2.0 ) * sin( angle ), gains[ 1 ], 1.0e-5 ) );
        }

        // the named points: ahead and behind are centred with the gain of a non spatial mono voice at pan 0, 90 degrees is all right
        Float32 gains[ 2 ];
        GainsForAzimuth( panner, 0.0, 1.0f, gains );
        ASSERT_TRUE( Near( 1.0, gains[ 0 ], 1.0e-6 ) && Near( 1.0, gains[ 1 ], 1.0e-6 ) );
        GainsForAzimuth( panner, 180.0, 1.0f, gains );
        ASSERT_TRUE( Near( 1.0, gains[ 0 ], 1.0e-5 ) && Near( 1.0, gains[ 1 ], 1.0e-5 ) );
        GainsForAzimuth( panner, 90.0, 1.0f, gains );
        ASSERT_TRUE( Near( 0.0, gains[ 0 ], 1.0e-6 ) && Near( sqrt( 2.0 ), gains[ 1 ], 1.0e-6 ) );
        GainsForAzimuth( panner, -30.0, 1.0f, gains );
        ASSERT_TRUE( Near( sqrt( 2.0 ) * cos( 0.5 * Pi / 4.0 ), gains[ 0 ], 1.0e-5 ) && gains[ 0 ] > gains[ 1 ] );

        // the static form of the law, and the widening: no pan at distance 0
        Float32 left = 0.0f;
        Float32 right = 0.0f;
        AUDIO::CAudioPanner::GetStereoGains( 1.0f, 1.0f, left, right );
        ASSERT_TRUE( Near( 0.0, left, 1.0e-6 ) && Near( sqrt( 2.0 ), right, 1.0e-6 ) );
        AUDIO::CAudioPanner::GetStereoGains( 1.0f, 0.0f, left, right );
        ASSERT_TRUE( Near( 1.0, left, 1.0e-6 ) && Near( 1.0, right, 1.0e-6 ) );
        AUDIO::CAudioPanner::GetStereoGains( 1.0f, 0.5f, left, right );
        ASSERT_TRUE( Near( sqrt( 2.0 ) * cos( 1.5 * Pi / 4.0 ), left, 1.0e-5 ) );

        // the optional rear attenuation: 1 - ( 1 - r ) max( 0, -cos( theta ) ), a cue between front and back
        const Float32 rear = (Float32) pow( 10.0, -6.0 / 20.0 );
        AUDIO::CAudioPanner::GetStereoGainsForAzimuth( (Float32) Pi, 1.0f, rear, left, right );
        ASSERT_TRUE( Near( rear, left, 1.0e-5 ) && Near( rear, right, 1.0e-5 ) );
        AUDIO::CAudioPanner::GetStereoGainsForAzimuth( 0.0f, 1.0f, rear, left, right );
        ASSERT_TRUE( Near( 1.0, left, 1.0e-6 ) );
        AUDIO::CAudioPanner::GetStereoGainsForAzimuth( (Float32) ( 90.0 * DegreesToRadians ), 1.0f, rear, left, right );
        ASSERT_TRUE( Near( sqrt( 2.0 ), right, 1.0e-5 ) );         // at the side nothing is taken
        const Float32 behind = (Float32) ( 120.0 * DegreesToRadians );
        AUDIO::CAudioPanner::GetStereoGainsForAzimuth( behind, 1.0f, rear, left, right );
        ASSERT_TRUE( Near( 1.0 - ( 1.0 - rear ) * 0.5, right / ( sqrt( 2.0 ) * sin( ( sin( 120.0 * DegreesToRadians ) + 1.0 ) * Pi / 4.0 ) ), 1.0e-5 ) );
        // through the panner too
        panner.GetGains( (Float32) Pi, 0.0f, 1.0f, rear, gains );
        ASSERT_TRUE( Near( rear, gains[ 0 ], 1.0e-5 ) );

        // the spread factor: the part of the spread radius, 1 without a radius
        ASSERT_TRUE( 0.0f == RtSpread( 0.0f, 3.0f ) && 0.5f == RtSpread( 1.5f, 3.0f ) && 1.0f == RtSpread( 30.0f, 3.0f ) && 1.0f == RtSpread( 0.0f, 0.0f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: The total power of the gains is 2 for every azimuth in steps of a degree: mono, stereo, quad, 5.1, 7.1, 7.1.4, with and without widening" )
    try
    {
        struct SCase { UInt32 channels; UInt32 layout; bool useCenter; };
        const SCase cases[] =
        {
            { 1, 0, true }, { 2, 0, true }, { 3, 0, true }, { 4, 0, true }, { 5, 0, true }, { 6, 0, true }, { 6, 0, false }, { 7, 0, true }, { 8, 0, true }, { 8, 0, false },
            { 12, Layout714(), true }, { 6, LayoutWithTwoCenters(), true }, { 2, AUDIO::AUDIOSPEAKER_FRONT_LEFT | AUDIO::AUDIOSPEAKER_FRONT_RIGHT, false },
            { 2, AUDIO::AUDIOSPEAKER_BACK_LEFT | AUDIO::AUDIOSPEAKER_BACK_RIGHT, true }, { 3, AUDIO::AUDIOSPEAKER_FRONT_LEFT | AUDIO::AUDIOSPEAKER_FRONT_RIGHT | AUDIO::AUDIOSPEAKER_LOW_FREQUENCY, true }
        };
        const UInt32 caseCount = sizeof( cases ) / sizeof( cases[ 0 ] );
        const Float32 spreads[ 4 ] = { 1.0f, 0.5f, 0.1f, 0.0f };

        for ( UInt32 k = 0; k < caseCount; ++k )
        {
            AUDIO::CAudioPanner panner;
            ASSERT_TRUE( panner.Prepare( cases[ k ].channels, cases[ k ].layout, cases[ k ].useCenter ) );
            const Float64 expectedPower = panner.GetRingChannelCount() >= 2 ? 2.0 : 1.0;

            Float32 gains[ AUDIO::CAudioPanner::MaxChannels ];
            for ( UInt32 s = 0; s < 4; ++s )
            {
                for ( Int32 degrees = -180; degrees <= 180; ++degrees )
                {
                    GainsForAzimuth( panner, (Float64) degrees, spreads[ s ], gains );
                    ASSERT_TRUE( Near( expectedPower, SumOfSquares( gains, panner.GetChannelCount() ), 1.0e-5 ) );
                    for ( UInt32 i = 0; i < panner.GetChannelCount(); ++i )
                        ASSERT_TRUE( gains[ i ] >= 0.0f && gains[ i ] < 1.5f );
                    if ( panner.GetLfeChannel() >= 0 )
                        ASSERT_TRUE( 0.0f == gains[ panner.GetLfeChannel() ] );
                }
            }
        }

        // a panner that was never prepared, or could not be, is silent and says so
        AUDIO::CAudioPanner unprepared;
        Float32 gains[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
        ASSERT_FALSE( RtGains( unprepared, 0.0f, 0.0f, 1.0f, 1.0f, gains ) );
        ASSERT_FALSE( unprepared.Prepare( 0, 0, true ) );
        ASSERT_FALSE( unprepared.Prepare( AUDIO::CAudioPanner::MaxChannels + 1, 0, true ) );
        ASSERT_FALSE( unprepared.IsPrepared() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: The speaker table has the angles of the design, ring panning puts a source at a speaker fully on that speaker" )
    try
    {
        Float32 degrees = 0.0f;
        ASSERT_TRUE( AUDIO::CAudioPanner::GetSpeakerAzimuthInDegrees( AUDIO::AUDIOSPEAKER_FRONT_LEFT, false, degrees ) && -30.0f == degrees );
        ASSERT_TRUE( AUDIO::CAudioPanner::GetSpeakerAzimuthInDegrees( AUDIO::AUDIOSPEAKER_FRONT_RIGHT, false, degrees ) && 30.0f == degrees );
        ASSERT_TRUE( AUDIO::CAudioPanner::GetSpeakerAzimuthInDegrees( AUDIO::AUDIOSPEAKER_FRONT_CENTER, false, degrees ) && 0.0f == degrees );
        ASSERT_TRUE( AUDIO::CAudioPanner::GetSpeakerAzimuthInDegrees( AUDIO::AUDIOSPEAKER_FRONT_LEFT_OF_CENTER, false, degrees ) && -15.0f == degrees );
        ASSERT_TRUE( AUDIO::CAudioPanner::GetSpeakerAzimuthInDegrees( AUDIO::AUDIOSPEAKER_SIDE_RIGHT, false, degrees ) && 90.0f == degrees );
        ASSERT_TRUE( AUDIO::CAudioPanner::GetSpeakerAzimuthInDegrees( AUDIO::AUDIOSPEAKER_BACK_LEFT, false, degrees ) && -110.0f == degrees );
        ASSERT_TRUE( AUDIO::CAudioPanner::GetSpeakerAzimuthInDegrees( AUDIO::AUDIOSPEAKER_BACK_LEFT, true, degrees ) && -150.0f == degrees );
        ASSERT_TRUE( AUDIO::CAudioPanner::GetSpeakerAzimuthInDegrees( AUDIO::AUDIOSPEAKER_BACK_RIGHT, true, degrees ) && 150.0f == degrees );
        ASSERT_TRUE( AUDIO::CAudioPanner::GetSpeakerAzimuthInDegrees( AUDIO::AUDIOSPEAKER_BACK_CENTER, true, degrees ) && 180.0f == degrees );
        ASSERT_TRUE( AUDIO::CAudioPanner::GetSpeakerAzimuthInDegrees( AUDIO::AUDIOSPEAKER_TOP_FRONT_LEFT, true, degrees ) && -45.0f == degrees );
        ASSERT_FALSE( AUDIO::CAudioPanner::GetSpeakerAzimuthInDegrees( AUDIO::AUDIOSPEAKER_LOW_FREQUENCY, true, degrees ) );
        ASSERT_FALSE( AUDIO::CAudioPanner::GetSpeakerAzimuthInDegrees( AUDIO::AUDIOSPEAKER_TOP_CENTER, true, degrees ) );
        ASSERT_FALSE( AUDIO::CAudioPanner::GetSpeakerAzimuthInDegrees( AUDIO::AUDIOSPEAKER_FRONT_LEFT | AUDIO::AUDIOSPEAKER_FRONT_RIGHT, true, degrees ) );

        // 5.1 takes the back speakers at 110 degrees, 7.1 (it has side speakers) at 150, and the channels are in the order of the layout bits
        AUDIO::CAudioPanner five;
        ASSERT_TRUE( five.Prepare( 6, 0, true ) );
        ASSERT_TRUE( !five.IsStereo() && 5 == five.GetRingChannelCount() && 5 == five.GetRingPositionCount() && 3 == five.GetLfeChannel() );
        ASSERT_TRUE( -30.0f == five.GetChannelAzimuthInDegrees( 0 ) && 30.0f == five.GetChannelAzimuthInDegrees( 1 ) && 0.0f == five.GetChannelAzimuthInDegrees( 2 ) );
        ASSERT_TRUE( -110.0f == five.GetChannelAzimuthInDegrees( 4 ) && 110.0f == five.GetChannelAzimuthInDegrees( 5 ) && !five.IsRingChannel( 3 ) );
        AUDIO::CAudioPanner seven;
        ASSERT_TRUE( seven.Prepare( 8, 0, true ) );
        ASSERT_TRUE( 7 == seven.GetRingChannelCount() && -150.0f == seven.GetChannelAzimuthInDegrees( 4 ) && 90.0f == seven.GetChannelAzimuthInDegrees( 7 ) );
        AUDIO::CAudioPanner fromFormat;
        ASSERT_TRUE( fromFormat.Prepare( AUDIO::CAudioFormat( 48000, 6, AUDIO::SAMPLETYPE_FLOAT32 ), true ) && 5 == fromFormat.GetRingChannelCount() );

        // a source at the azimuth of a speaker is on that speaker alone, with the gain of the power 2
        for ( UInt32 k = 0; k < 4; ++k )
        {
            const UInt32 channels[ 4 ] = { 4, 6, 8, 12 };
            const UInt32 layouts[ 4 ] = { 0, 0, 0, Layout714() };
            for ( UInt32 centre = 0; centre < 2; ++centre )
            {
                AUDIO::CAudioPanner panner;
                ASSERT_TRUE( panner.Prepare( channels[ k ], layouts[ k ], 1 == centre ) );
                Float32 gains[ AUDIO::CAudioPanner::MaxChannels ];
                for ( UInt32 c = 0; c < panner.GetChannelCount(); ++c )
                {
                    if ( !panner.IsRingChannel( c ) )
                        continue;
                    GainsForAzimuth( panner, panner.GetChannelAzimuthInDegrees( c ), 1.0f, gains );
                    for ( UInt32 o = 0; o < panner.GetChannelCount(); ++o )
                        ASSERT_TRUE( Near( o == c ? sqrt( 2.0 ) : 0.0, gains[ o ], 1.0e-4 ) );
                }
            }
        }

        // between two speakers only those two sound, and a source in the middle of them (on the sine law) gets the same gain from both when they are symmetric
        Float32 gains[ AUDIO::CAudioPanner::MaxChannels ];
        GainsForAzimuth( five, 15.0, 1.0f, gains );        // between the front centre and the front right
        ASSERT_TRUE( 0.0f == gains[ 0 ] && 0.0f == gains[ 4 ] && 0.0f == gains[ 5 ] && gains[ 1 ] > 0.0f && gains[ 2 ] > 0.0f );
        ASSERT_TRUE( Near( gains[ 1 ], gains[ 2 ], 1.0e-4 ) );
        GainsForAzimuth( five, 180.0, 1.0f, gains );       // directly behind, between the two back speakers
        ASSERT_TRUE( Near( gains[ 4 ], gains[ 5 ], 1.0e-5 ) && gains[ 4 ] > 0.9f && 0.0f == gains[ 0 ] && 0.0f == gains[ 2 ] );

        // the centre speaker can be left out: its gain is 0 and the front pair takes over
        AUDIO::CAudioPanner noCentre;
        ASSERT_TRUE( noCentre.Prepare( 6, 0, false ) );
        ASSERT_TRUE( 4 == noCentre.GetRingChannelCount() && !noCentre.IsRingChannel( 2 ) );
        GainsForAzimuth( noCentre, 0.0, 1.0f, gains );
        ASSERT_TRUE( 0.0f == gains[ 2 ] && Near( gains[ 0 ], gains[ 1 ], 1.0e-5 ) && Near( 2.0, SumOfSquares( gains, 6 ), 1.0e-5 ) );

        // speakers on the same angle share the gain of their position at equal power (front centre and top front centre)
        AUDIO::CAudioPanner shared;
        ASSERT_TRUE( shared.Prepare( 6, LayoutWithTwoCenters(), true ) );
        ASSERT_TRUE( 5 == shared.GetRingPositionCount() && 6 == shared.GetRingChannelCount() );
        GainsForAzimuth( shared, 0.0, 1.0f, gains );
        ASSERT_TRUE( Near( 1.0, gains[ 2 ], 1.0e-4 ) && Near( gains[ 2 ], gains[ 5 ], 1.0e-6 ) && Near( 2.0, SumOfSquares( gains, 6 ), 1.0e-5 ) );     // the channels are FL, FR, FC, BL, BR, TFC

        // a layout with one speaker has the gain 1 for every direction
        AUDIO::CAudioPanner mono;
        ASSERT_TRUE( mono.Prepare( 1, 0, true ) );
        for ( Int32 degrees = -180; degrees <= 180; degrees += 15 )
        {
            GainsForAzimuth( mono, (Float64) degrees, 0.3f, gains );
            ASSERT_TRUE( Near( 1.0, gains[ 0 ], 1.0e-6 ) );
        }

        // the gains change without jumps while the source goes round: at most 0.02 per step of a tenth of a degree (the speakers of 7.1.4 are only 15 degrees apart in places)
        for ( UInt32 k = 0; k < 3; ++k )
        {
            const UInt32 channels[ 3 ] = { 6, 8, 12 };
            AUDIO::CAudioPanner panner;
            ASSERT_TRUE( panner.Prepare( channels[ k ], 2 == k ? Layout714() : 0, true ) );
            Float32 previous[ AUDIO::CAudioPanner::MaxChannels ];
            GainsForAzimuth( panner, -180.0, 0.7f, previous );
            for ( Int32 step = 1; step <= 3600; ++step )
            {
                GainsForAzimuth( panner, -180.0 + 0.1 * step, 0.7f, gains );
                for ( UInt32 c = 0; c < panner.GetChannelCount(); ++c )
                {
                    ASSERT_TRUE( fabs( gains[ c ] - previous[ c ] ) < 0.02 );
                    previous[ c ] = gains[ c ];
                }
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: Close to the listener the sound widens to all speakers, evenly at distance 0" )
    try
    {
        const UInt32 channelCounts[ 4 ] = { 4, 6, 8, 12 };
        for ( UInt32 k = 0; k < 4; ++k )
        {
            AUDIO::CAudioPanner panner;
            ASSERT_TRUE( panner.Prepare( channelCounts[ k ], 3 == k ? Layout714() : 0, true ) );
            const Float32 radius = 4.0f;
            Float32 gains[ AUDIO::CAudioPanner::MaxChannels ];

            // distance 0: every ring speaker the same, whatever the direction, and no gain on the others
            const Float32 even = (Float32) sqrt( 2.0 / (Float64) panner.GetRingChannelCount() );
            for ( Int32 degrees = -180; degrees <= 180; degrees += 20 )
            {
                GainsForAzimuth( panner, (Float64) degrees, RtSpread( 0.0f, radius ), gains );
                for ( UInt32 c = 0; c < panner.GetChannelCount(); ++c )
                    ASSERT_TRUE( Near( panner.IsRingChannel( c ) ? even : 0.0, gains[ c ], 1.0e-5 ) );
            }

            // at the spread radius and beyond the panning is untouched
            Float32 panned[ AUDIO::CAudioPanner::MaxChannels ];
            GainsForAzimuth( panner, 40.0, RtSpread( radius, radius ), gains );
            GainsForAzimuth( panner, 40.0, 1.0f, panned );
            for ( UInt32 c = 0; c < panner.GetChannelCount(); ++c )
                ASSERT_TRUE( gains[ c ] == panned[ c ] );

            // halfway the loudest speaker of the panning lost gain to the others
            GainsForAzimuth( panner, 40.0, RtSpread( 0.5f * radius, radius ), gains );
            UInt32 loudest = 0;
            for ( UInt32 c = 1; c < panner.GetChannelCount(); ++c )
                loudest = panned[ c ] > panned[ loudest ] ? c : loudest;
            ASSERT_TRUE( gains[ loudest ] < panned[ loudest ] );
            ASSERT_TRUE( Near( 2.0, SumOfSquares( gains, panner.GetChannelCount() ), 1.0e-5 ) );
        }

        // stereo: a source at distance 0 is in the middle whatever its direction
        AUDIO::CAudioPanner stereo;
        ASSERT_TRUE( stereo.Prepare( 2, 0, true ) );
        Float32 gains[ 2 ];
        GainsForAzimuth( stereo, 90.0, RtSpread( 0.0f, 2.0f ), gains );
        ASSERT_TRUE( Near( 1.0, gains[ 0 ], 1.0e-6 ) && Near( 1.0, gains[ 1 ], 1.0e-6 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: The listener frame: a source at +X is heard right, left when the listener turned round, above is elevation" )
    try
    {
        TMath::SFix fix;
        const TVector3 forward = V( 0.0f, 0.0f, -1.0f );
        const TVector3 up = V( 0.0f, 1.0f, 0.0f );

        // listener forward -Z, up +Y, source on +X: heard right
        RtLocate( P( 0.0f, 0.0f, 0.0f ), forward, up, P( 10.0f, 0.0f, 0.0f ), false, fix );
        ASSERT_TRUE( Near( 10.0, fix.distance, 1.0e-5 ) && Near( Pi / 2.0, fix.azimuth, 1.0e-6 ) && Near( 1.0, fix.pan, 1.0e-6 ) && Near( 0.0, fix.elevation, 1.0e-6 ) );
        AUDIO::CAudioPanner stereo;
        ASSERT_TRUE( stereo.Prepare( 2, 0, true ) );
        Float32 gains[ 2 ];
        RtGains( stereo, fix.azimuth, fix.pan, 1.0f, 1.0f, gains );
        ASSERT_TRUE( gains[ 1 ] > gains[ 0 ] && Near( 0.0, gains[ 0 ], 1.0e-6 ) );

        // turned round (180 degrees about Y): the same source is on the left
        RtLocate( P( 0.0f, 0.0f, 0.0f ), V( 0.0f, 0.0f, 1.0f ), up, P( 10.0f, 0.0f, 0.0f ), false, fix );
        ASSERT_TRUE( Near( -Pi / 2.0, fix.azimuth, 1.0e-6 ) && Near( -1.0, fix.pan, 1.0e-6 ) );
        RtGains( stereo, fix.azimuth, fix.pan, 1.0f, 1.0f, gains );
        ASSERT_TRUE( gains[ 0 ] > gains[ 1 ] && Near( 0.0, gains[ 1 ], 1.0e-6 ) );

        // ahead, behind, left, and the position of the listener is subtracted
        RtLocate( P( 5.0f, 5.0f, 5.0f ), forward, up, P( 5.0f, 5.0f, -3.0f ), false, fix );
        ASSERT_TRUE( Near( 8.0, fix.distance, 1.0e-5 ) && Near( 0.0, fix.azimuth, 1.0e-6 ) && Near( 0.0, fix.pan, 1.0e-6 ) );
        RtLocate( P( 0.0f, 0.0f, 0.0f ), forward, up, P( 0.0f, 0.0f, 10.0f ), false, fix );
        ASSERT_TRUE( Near( Pi, fabs( fix.azimuth ), 1.0e-5 ) && Near( 0.0, fix.pan, 1.0e-6 ) );
        RtLocate( P( 0.0f, 0.0f, 0.0f ), forward, up, P( -4.0f, 0.0f, -4.0f ), false, fix );
        ASSERT_TRUE( Near( -Pi / 4.0, fix.azimuth, 1.0e-6 ) && Near( -sin( Pi / 4.0 ), fix.pan, 1.0e-6 ) );

        // a listener that looks along +X with up +Y has -Z on its left; above the listener is elevation
        RtLocate( P( 0.0f, 0.0f, 0.0f ), V( 1.0f, 0.0f, 0.0f ), up, P( 0.0f, 0.0f, -3.0f ), false, fix );
        ASSERT_TRUE( Near( -Pi / 2.0, fix.azimuth, 1.0e-6 ) );
        RtLocate( P( 0.0f, 0.0f, 0.0f ), forward, up, P( 0.0f, 4.0f, -4.0f ), false, fix );
        ASSERT_TRUE( Near( Pi / 4.0, fix.elevation, 1.0e-6 ) && Near( 0.0, fix.azimuth, 1.0e-6 ) );
        RtLocate( P( 0.0f, 0.0f, 0.0f ), forward, up, P( 0.0f, -4.0f, 0.0f ), false, fix );
        ASSERT_TRUE( Near( -Pi / 2.0, fix.elevation, 1.0e-6 ) && 0.0f == fix.pan );

        // a tilted listener (up leaning toward +X, its right leaning down): +X is still to the right, and above the horizon of the listener
        RtLocate( P( 0.0f, 0.0f, 0.0f ), forward, V( 1.0f, 1.0f, 0.0f ), P( 10.0f, 0.0f, 0.0f ), false, fix );
        ASSERT_TRUE( Near( Pi / 2.0, fix.azimuth, 1.0e-6 ) );
        ASSERT_TRUE( Near( Pi / 4.0, fix.elevation, 1.0e-6 ) );

        // the vectors need not be unit vectors, and a forward that is zero or along up falls back to the default
        RtLocate( P( 0.0f, 0.0f, 0.0f ), V( 0.0f, 0.0f, -50.0f ), V( 0.0f, 7.0f, 0.0f ), P( 10.0f, 0.0f, 0.0f ), false, fix );
        ASSERT_TRUE( Near( Pi / 2.0, fix.azimuth, 1.0e-6 ) );
        RtLocate( P( 0.0f, 0.0f, 0.0f ), V( 0.0f, 0.0f, 0.0f ), up, P( 10.0f, 0.0f, 0.0f ), false, fix );
        ASSERT_TRUE( Near( Pi / 2.0, fix.azimuth, 1.0e-6 ) );
        RtLocate( P( 0.0f, 0.0f, 0.0f ), V( 0.0f, 3.0f, 0.0f ), up, P( 10.0f, 0.0f, 0.0f ), false, fix );
        ASSERT_TRUE( Near( Pi / 2.0, fix.azimuth, 1.0e-6 ) );

        // at the place of the listener: in front, no direction
        RtLocate( P( 1.0f, 2.0f, 3.0f ), forward, up, P( 1.0f, 2.0f, 3.0f ), false, fix );
        ASSERT_TRUE( 0.0f == fix.distance && 0.0f == fix.azimuth && 0.0f == fix.pan && 0.0f == fix.elevation );

        // a listener relative emitter is in the frame of the listener: it ignores where the listener is and which way it looks
        TMath::SFix reference;
        RtLocate( P( 0.0f, 0.0f, 0.0f ), forward, up, P( 3.0f, 0.0f, -4.0f ), true, reference );
        ASSERT_TRUE( Near( 5.0, reference.distance, 1.0e-5 ) && Near( atan2( 3.0, 4.0 ), reference.azimuth, 1.0e-6 ) );
        RtLocate( P( 100.0f, -50.0f, 7.0f ), V( 0.0f, 0.0f, 1.0f ), up, P( 3.0f, 0.0f, -4.0f ), true, fix );
        ASSERT_TRUE( fix.distance == reference.distance && fix.azimuth == reference.azimuth && fix.pan == reference.pan && fix.elevation == reference.elevation );
        RtLocate( P( -8.0f, 90.0f, 1.0f ), V( 1.0f, 0.0f, 0.0f ), V( 0.0f, 0.0f, 1.0f ), P( 3.0f, 0.0f, -4.0f ), true, fix );
        ASSERT_TRUE( fix.distance == reference.distance && fix.azimuth == reference.azimuth && fix.pan == reference.pan );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: The 2D mode pans from the horizontal offset and measures the distance in the XY plane only" )
    try
    {
        TMath::SFix fix;
        const TPoint3 listener = P( 5.0f, 7.0f, 0.0f );

        RtLocate2D( listener, 10.0f, P( 10.0f, 7.0f, 0.0f ), false, fix );
        ASSERT_TRUE( Near( 0.5, fix.pan, 1.0e-6 ) && Near( 5.0, fix.distance, 1.0e-5 ) && Near( 0.5 * Pi / 2.0, fix.azimuth, 1.0e-6 ) && 0.0f == fix.elevation );
        RtLocate2D( listener, 10.0f, P( -5.0f, 7.0f, 0.0f ), false, fix );
        ASSERT_TRUE( Near( -1.0, fix.pan, 1.0e-6 ) && Near( -Pi / 2.0, fix.azimuth, 1.0e-6 ) );
        RtLocate2D( listener, 10.0f, P( 500.0f, 7.0f, 0.0f ), false, fix );
        ASSERT_TRUE( 1.0f == fix.pan );           // limited to the edge
        RtLocate2D( listener, 4.0f, P( 6.0f, 7.0f, 0.0f ), false, fix );
        ASSERT_TRUE( Near( 0.25, fix.pan, 1.0e-6 ) );     // the pan width is the distance to full left or right

        // the distance is that in the XY plane: the depth is not part of it, nor is the orientation of the listener
        RtLocate2D( listener, 10.0f, P( 8.0f, 11.0f, 400.0f ), false, fix );
        ASSERT_TRUE( Near( 5.0, fix.distance, 1.0e-5 ) && Near( 0.3, fix.pan, 1.0e-6 ) );

        // a pan width that is not positive is 1; relative to the listener the position is the offset
        RtLocate2D( listener, 0.0f, P( 0.25f, 0.0f, 0.0f ), true, fix );
        ASSERT_TRUE( Near( 0.25, fix.pan, 1.0e-6 ) && Near( 0.25, fix.distance, 1.0e-6 ) );

        // stereo takes the pan as it is (also when the azimuth would put it behind), surround takes the azimuth, a quarter turn at the edges
        AUDIO::CAudioPanner stereo;
        ASSERT_TRUE( stereo.Prepare( 2, 0, true ) );
        Float32 gains[ 2 ];
        RtLocate2D( listener, 10.0f, P( 10.0f, 7.0f, 0.0f ), false, fix );
        RtGains( stereo, fix.azimuth, fix.pan, 1.0f, 1.0f, gains );
        ASSERT_TRUE( Near( sqrt( 2.0 ) * cos( 1.5 * Pi / 4.0 ), gains[ 0 ], 1.0e-5 ) );

        AUDIO::CAudioPanner surround;
        ASSERT_TRUE( surround.Prepare( 6, 0, true ) );
        RtLocate2D( listener, 10.0f, P( -5.0f, 7.0f, 0.0f ), false, fix );        // full left: 90 degrees to the left, between the front left and the back left
        Float32 ring[ 6 ];
        RtGains( surround, fix.azimuth, fix.pan, 1.0f, 1.0f, ring );
        ASSERT_TRUE( ring[ 0 ] > 0.0f && ring[ 4 ] > 0.0f && 0.0f == ring[ 1 ] && 0.0f == ring[ 5 ] && Near( 2.0, SumOfSquares( ring, 6 ), 1.0e-5 ) );
        RtLocate2D( listener, 10.0f, listener, false, fix );
        RtGains( surround, fix.azimuth, fix.pan, 1.0f, 1.0f, ring );
        ASSERT_TRUE( Near( sqrt( 2.0 ), ring[ 2 ], 1.0e-4 ) );        // the middle is the front centre
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: The listener keeps its state, refuses what is not valid and publishes snapshots" )
    try
    {
        AUDIO::CAudioListener listener;
        AUDIO::SListenerState state;
        listener.GetState( state );
        ASSERT_TRUE( state == AUDIO::SListenerState() );
        ASSERT_TRUE( state.position == P( 0.0f, 0.0f, 0.0f ) && state.forward == V( 0.0f, 0.0f, -1.0f ) && state.up == V( 0.0f, 1.0f, 0.0f ) && AUDIO::CAudioListener::DefaultPanWidth2D == state.panWidth2D );

        // the audio thread has a state from the start, then only what changed
        AUDIO::SListenerState read;
        ASSERT_TRUE( RtReadListener( listener, read ) && read == state );
        ASSERT_FALSE( RtReadListener( listener, read ) );

        ASSERT_TRUE( listener.SetPose( P( 1.0f, 2.0f, 3.0f ), V( 0.0f, 0.0f, 4.0f ), V( 0.0f, 2.0f, 0.0f ) ) );
        ASSERT_TRUE( RtReadListener( listener, read ) );
        ASSERT_TRUE( read.position == P( 1.0f, 2.0f, 3.0f ) && read.forward == V( 0.0f, 0.0f, 1.0f ) && read.up == V( 0.0f, 1.0f, 0.0f ) );       // normalised when stored
        ASSERT_FALSE( RtReadListener( listener, read ) );

        // every setter publishes, the others keep their value
        ASSERT_TRUE( listener.SetVelocity( V( 5.0f, 0.0f, 0.0f ) ) && RtReadListener( listener, read ) && read.velocity == V( 5.0f, 0.0f, 0.0f ) && read.position == P( 1.0f, 2.0f, 3.0f ) );
        ASSERT_TRUE( listener.SetPosition( P( -1.0f, 0.0f, 0.0f ) ) && RtReadListener( listener, read ) && read.position == P( -1.0f, 0.0f, 0.0f ) && read.velocity == V( 5.0f, 0.0f, 0.0f ) );
        ASSERT_TRUE( listener.SetOrientation( V( 1.0f, 0.0f, 0.0f ), V( 0.0f, 1.0f, 0.0f ) ) && RtReadListener( listener, read ) && read.forward == V( 1.0f, 0.0f, 0.0f ) );
        ASSERT_TRUE( listener.SetPositionAndVelocity( P( 9.0f, 9.0f, 9.0f ), V( 8.0f, 8.0f, 8.0f ) ) && RtReadListener( listener, read ) && read.position == P( 9.0f, 9.0f, 9.0f ) && read.velocity == V( 8.0f, 8.0f, 8.0f ) );
        ASSERT_TRUE( listener.Set2DPanWidth( 20.0f ) && RtReadListener( listener, read ) && 20.0f == read.panWidth2D );

        // several updates between two reads: the audio thread gets the newest one
        listener.SetPosition( P( 1.0f, 0.0f, 0.0f ) );
        listener.SetPosition( P( 2.0f, 0.0f, 0.0f ) );
        listener.SetPosition( P( 3.0f, 0.0f, 0.0f ) );
        ASSERT_TRUE( RtReadListener( listener, read ) && 3.0f == read.position.x && !RtReadListener( listener, read ) && 3.0f == read.position.x );

        // what is not valid is refused and changes nothing: not a number, infinity, no direction, forward along up, no width
        listener.GetState( state );
        const Float32 nan = 0.0f / 0.0f;
        const Float32 infinity = 1.0f / 0.0f;
        ASSERT_FALSE( listener.SetPosition( P( nan, 0.0f, 0.0f ) ) );
        ASSERT_FALSE( listener.SetPosition( P( 0.0f, infinity, 0.0f ) ) );
        ASSERT_FALSE( listener.SetVelocity( V( 0.0f, 0.0f, nan ) ) );
        ASSERT_FALSE( listener.SetOrientation( V( 0.0f, 0.0f, 0.0f ), V( 0.0f, 1.0f, 0.0f ) ) );
        ASSERT_FALSE( listener.SetOrientation( V( 0.0f, 1.0f, 0.0f ), V( 0.0f, 5.0f, 0.0f ) ) );
        ASSERT_FALSE( listener.SetOrientation( V( 1.0f, 0.0f, 0.0f ), V( 0.0f, 0.0f, 0.0f ) ) );
        ASSERT_FALSE( listener.SetPose( P( 1.0f, 1.0f, 1.0f ), V( nan, 0.0f, 0.0f ), V( 0.0f, 1.0f, 0.0f ) ) );
        ASSERT_FALSE( listener.Set2DPanWidth( 0.0f ) || listener.Set2DPanWidth( -3.0f ) || listener.Set2DPanWidth( nan ) );
        AUDIO::SListenerState bad = state;
        bad.up = bad.forward;
        ASSERT_FALSE( listener.SetState( bad ) );
        AUDIO::SListenerState after;
        listener.GetState( after );
        ASSERT_TRUE( after == state && !RtReadListener( listener, read ) );

        // the whole state in one go
        AUDIO::SListenerState whole;
        whole.position = P( 4.0f, 5.0f, 6.0f );
        whole.velocity = V( 1.0f, 1.0f, 1.0f );
        whole.panWidth2D = 3.0f;
        ASSERT_TRUE( listener.SetState( whole ) && RtReadListener( listener, read ) && read == whole );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: An emitter has the defaults of the design, limits its values and publishes whole snapshots" )
    try
    {
        AUDIO::CAudioEmitter emitter;
        AUDIO::SEmitterState state;
        emitter.GetState( state );
        ASSERT_TRUE( AUDIO::AUDIOSPATIAL_3D == state.mode && state.position == P( 0.0f, 0.0f, 0.0f ) && state.velocity == V( 0.0f, 0.0f, 0.0f ) && state.direction == V( 0.0f, 0.0f, 0.0f ) );
        ASSERT_TRUE( !state.listenerRelative && state.useWorldAttenuation && 360.0f == state.coneInnerAngle && 360.0f == state.coneOuterAngle && 0.0f == state.coneOuterGain && 0.0f == state.coneOuterCutoff );
        ASSERT_TRUE( 0.0f == state.spreadRadius && 0.0f == state.distanceCutoff && 0.0f == state.occlusion && 0.0f == state.obstruction && -12.0f == state.occlusionGainInDecibels && 1500.0f == state.occlusionCutoff );
        ASSERT_TRUE( 1.0f == state.dopplerFactor && state.sendsFollowAttenuation && !state.deriveVelocity );
        ASSERT_TRUE( state == AUDIO::SEmitterState() );

        AUDIO::SEmitterState read;
        ASSERT_TRUE( RtReadEmitter( emitter, read ) && read == state && !RtReadEmitter( emitter, read ) );

        // a position and velocity set together arrive together
        ASSERT_TRUE( emitter.SetPositionAndVelocity( P( 1.0f, 2.0f, 3.0f ), V( 4.0f, 5.0f, 6.0f ) ) );
        ASSERT_TRUE( RtReadEmitter( emitter, read ) && read.position == P( 1.0f, 2.0f, 3.0f ) && read.velocity == V( 4.0f, 5.0f, 6.0f ) && !RtReadEmitter( emitter, read ) );

        // the other properties, one at a time
        ASSERT_TRUE( emitter.SetMode( AUDIO::AUDIOSPATIAL_2D ) && emitter.SetListenerRelative( true ) && emitter.SetSendsFollowAttenuation( false ) && emitter.SetDeriveVelocity( true ) );
        ASSERT_TRUE( emitter.SetDirection( V( 0.0f, 0.0f, -9.0f ) ) && emitter.SetCone( 60.0f, 180.0f, 0.25f, 800.0f ) && emitter.SetSpreadRadius( 6.0f ) && emitter.SetDistanceLowPass( 2000.0f ) );
        ASSERT_TRUE( emitter.SetOcclusion( 0.4f ) && emitter.SetObstruction( 0.6f ) && emitter.SetOcclusionResponse( -20.0f, 900.0f ) && emitter.SetDopplerFactor( 2.5f ) );
        ASSERT_TRUE( RtReadEmitter( emitter, read ) );
        ASSERT_TRUE( AUDIO::AUDIOSPATIAL_2D == read.mode && read.listenerRelative && !read.sendsFollowAttenuation && read.deriveVelocity );
        ASSERT_TRUE( read.direction == V( 0.0f, 0.0f, -1.0f ) && 60.0f == read.coneInnerAngle && 180.0f == read.coneOuterAngle && 0.25f == read.coneOuterGain && 800.0f == read.coneOuterCutoff );
        ASSERT_TRUE( 6.0f == read.spreadRadius && 2000.0f == read.distanceCutoff && 0.4f == read.occlusion && 0.6f == read.obstruction && -20.0f == read.occlusionGainInDecibels && 900.0f == read.occlusionCutoff && 2.5f == read.dopplerFactor );
        ASSERT_TRUE( read.position == P( 1.0f, 2.0f, 3.0f ) && read.velocity == V( 4.0f, 5.0f, 6.0f ) );        // untouched by the others

        // values with a sensible limit are limited
        ASSERT_TRUE( emitter.SetCone( 500.0f, 100.0f, 7.0f, 5.0f ) );
        emitter.GetState( state );
        ASSERT_TRUE( 360.0f == state.coneInnerAngle && 360.0f == state.coneOuterAngle && 1.0f == state.coneOuterGain && 20.0f == state.coneOuterCutoff );
        ASSERT_TRUE( emitter.SetCone( 90.0f, 10.0f, -1.0f, 0.0f ) );
        emitter.GetState( state );
        ASSERT_TRUE( 90.0f == state.coneInnerAngle && 90.0f == state.coneOuterAngle && 0.0f == state.coneOuterGain && 0.0f == state.coneOuterCutoff );
        ASSERT_TRUE( emitter.SetOcclusion( 3.0f ) && emitter.SetObstruction( -3.0f ) && emitter.SetOcclusionResponse( 10.0f, 1.0e9f ) && emitter.SetDopplerFactor( 100.0f ) && emitter.SetSpreadRadius( -4.0f ) );
        emitter.GetState( state );
        ASSERT_TRUE( 1.0f == state.occlusion && 0.0f == state.obstruction && 0.0f == state.occlusionGainInDecibels && 20000.0f == state.occlusionCutoff && 10.0f == state.dopplerFactor && 0.0f == state.spreadRadius );
        ASSERT_TRUE( emitter.SetDirection( V( 0.0f, 0.0f, 0.0f ) ) );
        emitter.GetState( state );
        ASSERT_TRUE( state.direction == V( 0.0f, 0.0f, 0.0f ) );     // omnidirectional

        // what is not valid is refused as a whole and changes nothing
        AUDIO::SEmitterState before;
        emitter.GetState( before );
        RtReadEmitter( emitter, read );
        const Float32 nan = 0.0f / 0.0f;
        const Float32 infinity = 1.0f / 0.0f;
        ASSERT_FALSE( emitter.SetPositionAndVelocity( P( 5.0f, 5.0f, 5.0f ), V( nan, 0.0f, 0.0f ) ) );
        ASSERT_FALSE( emitter.SetPosition( P( 0.0f, infinity, 0.0f ) ) );
        ASSERT_FALSE( emitter.SetVelocity( V( 0.0f, 0.0f, nan ) ) );
        ASSERT_FALSE( emitter.SetDirection( V( nan, 0.0f, 0.0f ) ) );
        ASSERT_FALSE( emitter.SetCone( nan, 10.0f, 0.5f, 0.0f ) || emitter.SetCone( 10.0f, 10.0f, nan, 0.0f ) || emitter.SetCone( 10.0f, 10.0f, 0.5f, nan ) );
        ASSERT_FALSE( emitter.SetOcclusion( nan ) || emitter.SetObstruction( nan ) || emitter.SetOcclusionResponse( nan, 100.0f ) || emitter.SetDopplerFactor( nan ) );
        ASSERT_FALSE( emitter.SetSpreadRadius( nan ) || emitter.SetDistanceLowPass( nan ) );
        AUDIO::SEmitterState after;
        emitter.GetState( after );
        ASSERT_TRUE( after == before && !RtReadEmitter( emitter, read ) );

        // an attenuation of its own, or the one of the world
        AUDIO::CAudioSpatialParams world;
        world.GetDefaultAttenuation().SetMinDistance( 3.0f );
        AUDIO::SSpatialWorldState worldState;
        world.GetWorldState( worldState );
        ASSERT_TRUE( emitter.UseWorldAttenuation() && RtReadEmitter( emitter, read ) );
        ASSERT_TRUE( read.useWorldAttenuation && Near( worldState.attenuation.Evaluate( 9.0f ), RtEffectiveGain( read, worldState, 9.0f ), 1.0e-9 ) );
        ASSERT_TRUE( 3.0f == RtEffectiveSpread( read, worldState ) );        // no spread radius set: the minimum distance of the attenuation in effect
        AUDIO::CAudioAttenuation own( MATH::DISTANCEMODEL_LINEAR_CLAMPED, 1.0f, 11.0f, 1.0f );
        ASSERT_TRUE( emitter.SetAttenuation( own ) && RtReadEmitter( emitter, read ) );
        ASSERT_TRUE( !read.useWorldAttenuation && Near( 0.5, RtEffectiveGain( read, worldState, 6.0f ), 1.0e-6 ) && 1.0f == RtEffectiveSpread( read, worldState ) );
        ASSERT_TRUE( emitter.SetSpreadRadius( 4.5f ) && RtReadEmitter( emitter, read ) && 4.5f == RtEffectiveSpread( read, worldState ) );
        ASSERT_TRUE( emitter.UseWorldAttenuation() && RtReadEmitter( emitter, read ) && read.useWorldAttenuation );

        // the whole state in one go, with the limits applied
        AUDIO::SEmitterState whole;
        whole.position = P( 7.0f, 8.0f, 9.0f );
        whole.coneInnerAngle = 40.0f;
        whole.coneOuterAngle = 20.0f;
        whole.direction = V( 5.0f, 0.0f, 0.0f );
        ASSERT_TRUE( emitter.SetState( whole ) && RtReadEmitter( emitter, read ) );
        ASSERT_TRUE( read.position == whole.position && 40.0f == read.coneOuterAngle && read.direction == V( 1.0f, 0.0f, 0.0f ) );
        whole.velocity = V( nan, 0.0f, 0.0f );
        ASSERT_FALSE( emitter.SetState( whole ) );

        // shared like voices do
        AUDIO::TAudioEmitterPtr shared( GUCEF_NEW AUDIO::CAudioEmitter() );
        AUDIO::TAudioEmitterPtr second = shared;
        ASSERT_TRUE( second->SetOcclusion( 0.5f ) );
        shared->GetState( state );
        ASSERT_TRUE( 0.5f == state.occlusion );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: The world settings have the defaults of the design, limit their values and make the state of the audio thread" )
    try
    {
        AUDIO::CAudioSpatialParams params;
        ASSERT_TRUE( 343.3f == params.GetSpeedOfSound() && 1.0f == params.GetUnitsPerMeter() && 1.0f == params.GetDopplerFactor() && 0.5f == params.GetDopplerMinRatio() && 2.0f == params.GetDopplerMaxRatio() );
        ASSERT_TRUE( 50.0f == params.GetDopplerSmoothingInMs() && params.GetUseCenterSpeaker() && !params.IsLfeSendEnabled() && 0.0f == params.GetRearAttenuationInDecibels() && "panning" == params.GetSpatializerName() );
        ASSERT_TRUE( MATH::DISTANCEMODEL_INVERSE_CLAMPED == params.GetDefaultAttenuation().GetModel() && 1.0f == params.GetDefaultAttenuation().GetMinDistance() && 100.0f == params.GetDefaultAttenuation().GetMaxDistance() && 1.0f == params.GetDefaultAttenuation().GetRolloff() );
        ASSERT_TRUE( "Spatial" == AUDIO::CAudioSpatialParams::ConfigSectionName && params == AUDIO::CAudioSpatialParams() );

        // the speed of sound in world units follows the units per metre
        params.SetUnitsPerMeter( 100.0f );
        ASSERT_TRUE( Near( 34330.0, params.GetSpeedOfSoundInUnits(), 0.01 ) );

        // limits, and what is not a number changes nothing
        params.SetSpeedOfSound( 0.0f );
        ASSERT_TRUE( 1.0f == params.GetSpeedOfSound() );
        params.SetSpeedOfSound( 1.0e9f );
        ASSERT_TRUE( 100000.0f == params.GetSpeedOfSound() );
        params.SetSpeedOfSound( 0.0f / 0.0f );
        ASSERT_TRUE( 100000.0f == params.GetSpeedOfSound() );
        params.SetUnitsPerMeter( -1.0f );
        ASSERT_TRUE( params.GetUnitsPerMeter() > 0.0f );
        params.SetDopplerFactor( 50.0f );
        ASSERT_TRUE( 10.0f == params.GetDopplerFactor() );
        params.SetDopplerFactor( -1.0f );
        ASSERT_TRUE( 0.0f == params.GetDopplerFactor() );
        params.SetDopplerSmoothingInMs( 5000.0f );
        ASSERT_TRUE( 1000.0f == params.GetDopplerSmoothingInMs() );
        params.SetRearAttenuationInDecibels( 6.0f );
        ASSERT_TRUE( 0.0f == params.GetRearAttenuationInDecibels() );
        params.SetRearAttenuationInDecibels( -200.0f );
        ASSERT_TRUE( -60.0f == params.GetRearAttenuationInDecibels() );

        // the ratio limits stay in order: moving one past the other moves the other
        AUDIO::CAudioSpatialParams ratios;
        ratios.SetDopplerMinRatio( 3.0f );
        ASSERT_TRUE( 3.0f == ratios.GetDopplerMinRatio() && 3.0f == ratios.GetDopplerMaxRatio() );
        ratios.SetDopplerMaxRatio( 0.25f );
        ASSERT_TRUE( 0.25f == ratios.GetDopplerMaxRatio() && 0.25f == ratios.GetDopplerMinRatio() );

        // the LFE send is off until it has a level
        AUDIO::CAudioSpatialParams lfe;
        lfe.SetLfeSendInDecibels( -6.0f );
        ASSERT_TRUE( lfe.IsLfeSendEnabled() );
        AUDIO::SSpatialWorldState state;
        lfe.GetWorldState( state );
        ASSERT_TRUE( Near( pow( 10.0, -6.0 / 20.0 ), state.lfeSendGain, 1.0e-6 ) );
        lfe.SetLfeSendInDecibels( -500.0f );
        lfe.GetWorldState( state );
        ASSERT_TRUE( !lfe.IsLfeSendEnabled() && 0.0f == state.lfeSendGain );

        // the state for the audio thread: everything in world units and linear gains
        AUDIO::CAudioSpatialParams world;
        world.SetSpeedOfSound( 300.0f );
        world.SetUnitsPerMeter( 2.0f );
        world.SetDopplerFactor( 0.5f );
        world.SetDopplerMinRatio( 0.8f );
        world.SetDopplerMaxRatio( 1.25f );
        world.SetDopplerSmoothingInMs( 20.0f );
        world.SetUseCenterSpeaker( false );
        world.SetRearAttenuationInDecibels( -6.0f );
        world.GetDefaultAttenuation().SetModel( MATH::DISTANCEMODEL_LINEAR_CLAMPED );
        world.GetWorldState( state );
        ASSERT_TRUE( 600.0f == state.speedOfSoundInUnits && 0.5f == state.dopplerFactor && 0.8f == state.dopplerMinRatio && 1.25f == state.dopplerMaxRatio && 20.0f == state.dopplerSmoothingInMs );
        ASSERT_TRUE( !state.useCenterSpeaker && Near( pow( 10.0, -6.0 / 20.0 ), state.rearAttenuationGain, 1.0e-6 ) && MATH::DISTANCEMODEL_LINEAR_CLAMPED == state.attenuation.GetModel() );
        ASSERT_TRUE( state == state && state != AUDIO::SSpatialWorldState() && AUDIO::SSpatialWorldState() == AUDIO::SSpatialWorldState() );
        AUDIO::SSpatialWorldState defaultState;
        AUDIO::CAudioSpatialParams().GetWorldState( defaultState );
        ASSERT_TRUE( defaultState == AUDIO::SSpatialWorldState() );

        // copies are the same and independent
        AUDIO::CAudioSpatialParams copy( world );
        ASSERT_TRUE( copy == world );
        copy.GetDefaultAttenuation().SetRolloff( 4.0f );
        ASSERT_TRUE( copy != world );
        copy = world;
        ASSERT_TRUE( copy == world );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: The world settings are a config section, also as the child Spatial of the engine parameters" )
    try
    {
        AUDIO::CAudioSpatialParams original;
        original.SetSpeedOfSound( 340.0f );
        original.SetUnitsPerMeter( 0.5f );
        original.SetDopplerFactor( 1.5f );
        original.SetDopplerMinRatio( 0.7f );
        original.SetDopplerMaxRatio( 1.6f );
        original.SetDopplerSmoothingInMs( 35.0f );
        original.SetSpatializerName( "hrtf" );
        original.SetUseCenterSpeaker( false );
        original.SetLfeSendInDecibels( -10.0f );
        original.SetRearAttenuationInDecibels( -3.0f );
        original.GetDefaultAttenuation() = AUDIO::CAudioAttenuation( MATH::DISTANCEMODEL_EXPONENT_CLAMPED, 2.0f, 60.0f, 1.5f );

        CORE::CDataNode config( "Config" );
        ASSERT_TRUE( original.SaveConfig( config ) );
        const CORE::CDataNode* section = config.FindChild( "Spatial" );
        ASSERT_TRUE( GUCEF_NULL != section && GUCEF_NULL != section->FindChild( "Attenuation" ) && "hrtf" == section->GetAttributeValue( "spatializer" ).AsString() );
        ASSERT_TRUE( "exponential" == section->FindChild( "Attenuation" )->GetAttributeValue( "model" ).AsString() );

        AUDIO::CAudioSpatialParams loaded;
        ASSERT_TRUE( loaded.LoadConfig( config ) && loaded == original );
        AUDIO::CAudioSpatialParams fromSection;
        ASSERT_TRUE( GUCEF_NULL != section && fromSection.LoadConfig( *section ) && fromSection == original );

        // saving again replaces the section
        ASSERT_TRUE( original.SaveConfig( config ) && 1 == config.GetNrOfDirectChildNodes() );

        // what is not there keeps its value, what is not valid is limited, no section is no settings
        CORE::CDataNode partial( "Spatial" );
        partial.SetAttribute( "dopplerFactor", 0.25f );
        partial.SetAttribute( "speedOfSound", -4.0f );
        AUDIO::CAudioSpatialParams target;
        target.SetSpeedOfSound( 300.0f );
        ASSERT_TRUE( target.LoadConfig( partial ) );
        ASSERT_TRUE( 0.25f == target.GetDopplerFactor() && 1.0f == target.GetSpeedOfSound() && 0.5f == target.GetDopplerMinRatio() && 2.0f == target.GetDopplerMaxRatio() );
        CORE::CDataNode empty( "Config" );
        ASSERT_FALSE( AUDIO::CAudioSpatialParams().LoadConfig( empty ) );

        // the ratio limits are read as a pair
        CORE::CDataNode ratios( "Spatial" );
        ratios.SetAttribute( "dopplerMinRatio", 0.3f );
        ratios.SetAttribute( "dopplerMaxRatio", 0.4f );
        AUDIO::CAudioSpatialParams pair;
        ASSERT_TRUE( pair.LoadConfig( ratios ) && 0.3f == pair.GetDopplerMinRatio() && 0.4f == pair.GetDopplerMaxRatio() );

        // as the child of the engine parameters: only when they are not the defaults
        AUDIO::CAudioEngineParams engineParams;
        CORE::CDataNode engineConfig( "Config" );
        ASSERT_TRUE( engineParams.SaveConfig( engineConfig ) );
        ASSERT_TRUE( GUCEF_NULL != engineConfig.FindChild( "AudioEngine" ) && GUCEF_NULL == engineConfig.FindChild( "AudioEngine" )->FindChild( "Spatial" ) );

        engineParams.GetSpatialParams() = original;
        CORE::CDataNode withSpatial( "Config" );
        ASSERT_TRUE( engineParams.SaveConfig( withSpatial ) );
        ASSERT_TRUE( GUCEF_NULL != withSpatial.FindChild( "AudioEngine" ) && GUCEF_NULL != withSpatial.FindChild( "AudioEngine" )->FindChild( "Spatial" ) );
        AUDIO::CAudioEngineParams loadedEngine;
        ASSERT_TRUE( loadedEngine.LoadConfig( withSpatial ) && loadedEngine.GetSpatialParams() == original );

        // back to the defaults leaves no stale section behind
        engineParams.GetSpatialParams() = AUDIO::CAudioSpatialParams();
        ASSERT_TRUE( engineParams.SaveConfig( withSpatial ) );
        ASSERT_TRUE( GUCEF_NULL == withSpatial.FindChild( "AudioEngine" )->FindChild( "Spatial" ) );

        // copies of the engine parameters have them too
        AUDIO::CAudioEngineParams copy( loadedEngine );
        ASSERT_TRUE( copy.GetSpatialParams() == original );
        AUDIO::CAudioEngineParams assigned;
        assigned = loadedEngine;
        ASSERT_TRUE( assigned.GetSpatialParams() == original );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: The engine publishes the world settings as one snapshot, from the parameters and from SetSpatialParams()" )
    try
    {
        AUDIO::CAudioEngineParams params = NullParams();
        params.GetSpatialParams().SetDopplerFactor( 0.5f );
        params.GetSpatialParams().SetSpeedOfSound( 300.0f );

        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );
        AUDIO::SSpatialWorldState state;
        ASSERT_TRUE( RtReadWorld( engine, state ) );
        ASSERT_TRUE( 0.5f == state.dopplerFactor && 300.0f == state.speedOfSoundInUnits );
        ASSERT_FALSE( RtReadWorld( engine, state ) );
        ASSERT_TRUE( engine.GetSpatialParams() == params.GetSpatialParams() && engine.GetParams().GetSpatialParams() == params.GetSpatialParams() );

        AUDIO::CAudioSpatialParams changed;
        changed.SetSpeedOfSound( 400.0f );
        changed.SetUnitsPerMeter( 2.0f );
        changed.SetDopplerMaxRatio( 3.0f );
        changed.SetUseCenterSpeaker( false );
        engine.SetSpatialParams( changed );
        ASSERT_TRUE( RtReadWorld( engine, state ) );
        ASSERT_TRUE( 800.0f == state.speedOfSoundInUnits && 3.0f == state.dopplerMaxRatio && !state.useCenterSpeaker && 1.0f == state.dopplerFactor );
        ASSERT_TRUE( engine.GetSpatialParams() == changed && engine.GetParams().GetSpatialParams() == changed );
        ASSERT_FALSE( RtReadWorld( engine, state ) );

        // several calls between two reads: the newest one
        for ( UInt32 i = 1; i <= 5; ++i )
        {
            AUDIO::CAudioSpatialParams p;
            p.SetDopplerSmoothingInMs( (Float32) ( 10 * i ) );
            engine.SetSpatialParams( p );
        }
        ASSERT_TRUE( RtReadWorld( engine, state ) && 50.0f == state.dopplerSmoothingInMs && !RtReadWorld( engine, state ) );

        // a device reopen does not touch them
        AUDIO::CAudioEngineParams reopen = engine.GetParams();
        reopen.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
        ASSERT_TRUE( engine.ReopenDevice( reopen ) );
        ASSERT_TRUE( engine.GetSpatialParams().GetDopplerSmoothingInMs() == 50.0f );
        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatial: Game threads updating 200 emitters, the listener and the world at 1 kHz against a reading audio thread see whole, ordered snapshots" )
    try
    {
        SStress stress;
        ASSERT_TRUE( stress.engine.Initialize( NullParams() ) );
        AUDIO::CAudioSpatialParams first;       // the speed of sound and the smoothing time of the first call, as every call of the world thread has them
        first.SetSpeedOfSound( 101.0f );
        first.SetDopplerSmoothingInMs( 1.0f );
        stress.engine.SetSpatialParams( first );

        CSpatialTestThread game( &SStress::Game, &stress );
        CSpatialTestThread world( &SStress::World, &stress );
        CSpatialTestThread audio( &SStress::Audio, &stress );
        ASSERT_TRUE( audio.Activate( GUCEF_NULL, 0, 0 ) && game.Activate( GUCEF_NULL, 0, 0 ) && world.Activate( GUCEF_NULL, 0, 0 ) );

        MT::PrecisionDelay( StressDurationInMs );
        stress.stop.Store( true, MT::MEMORDER_RELEASE );

        ASSERT_TRUE( game.WaitUntilDone( 60000 ) && world.WaitUntilDone( 60000 ) && audio.WaitUntilDone( 60000 ) );
        ASSERT_TRUE( stress.gameTicks > 10 && stress.worldChanges > 10 && stress.readRounds > 10 && stress.newEmitterReads > 10 );
        ASSERT_TRUE( 0 == stress.torn );
        ASSERT_TRUE( 0 == stress.wentBackwards );
        stress.engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
