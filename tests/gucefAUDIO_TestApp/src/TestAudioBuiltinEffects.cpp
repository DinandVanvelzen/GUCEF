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

#ifndef GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#include "gucefAUDIO_CAudioEffectRegistry.h"
#define GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CLIMITEREFFECT_H
#include "gucefAUDIO_CLimiterEffect.h"
#define GUCEF_AUDIO_CLIMITEREFFECT_H
#endif /* GUCEF_AUDIO_CLIMITEREFFECT_H ? */

#ifndef GUCEF_AUDIO_CCOMPRESSOREFFECT_H
#include "gucefAUDIO_CCompressorEffect.h"
#define GUCEF_AUDIO_CCOMPRESSOREFFECT_H
#endif /* GUCEF_AUDIO_CCOMPRESSOREFFECT_H ? */

#ifndef GUCEF_AUDIO_CEQUALIZEREFFECT_H
#include "gucefAUDIO_CEqualizerEffect.h"
#define GUCEF_AUDIO_CEQUALIZEREFFECT_H
#endif /* GUCEF_AUDIO_CEQUALIZEREFFECT_H ? */

#ifndef GUCEF_AUDIO_CFILTEREFFECT_H
#include "gucefAUDIO_CFilterEffect.h"
#define GUCEF_AUDIO_CFILTEREFFECT_H
#endif /* GUCEF_AUDIO_CFILTEREFFECT_H ? */

#ifndef GUCEF_AUDIO_CDELAYEFFECT_H
#include "gucefAUDIO_CDelayEffect.h"
#define GUCEF_AUDIO_CDELAYEFFECT_H
#endif /* GUCEF_AUDIO_CDELAYEFFECT_H ? */

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

#include "TestAudioEffectSupport.h"
#include "TestAudioBuiltinEffects.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace AudioEffectTest;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 EffectCount = 5;
const char* const EffectNames[ EffectCount ] = { "limiter", "compressor", "equalizer", "filter", "delay" };

AUDIO::TAudioEffectPtr
CreateByName( const char* name, const CORE::CValueList& options = CORE::CValueList() )
{
    return AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( name, options );
}

/** Settings that make the effect do something to noise, every parameter somewhere else than its default where that matters */
void
Exercise( AUDIO::CAudioEffect& effect, const char* name )
{
    const CORE::CString type( name );
    if ( type == "limiter" )
    {
        AUDIO::CLimiterEffect& e = static_cast< AUDIO::CLimiterEffect& >( effect );
        e.SetCeilingInDecibels( -3.0f );
        e.SetReleaseInMs( 80.0f );
        e.SetInputGainInDecibels( 4.0f );
    }
    else
    if ( type == "compressor" )
    {
        AUDIO::CCompressorEffect& e = static_cast< AUDIO::CCompressorEffect& >( effect );
        e.SetThresholdInDecibels( -24.0f );
        e.SetRatio( 6.0f );
        e.SetKneeInDecibels( 9.0f );
        e.SetAttackInMs( 3.0f );
        e.SetReleaseInMs( 90.0f );
        e.SetMakeupInDecibels( 3.0f );
        e.SetStereoLink( 0.7f );
    }
    else
    if ( type == "equalizer" )
    {
        AUDIO::CEqualizerEffect& e = static_cast< AUDIO::CEqualizerEffect& >( effect );
        e.SetBand( 0, true, AUDIO::FILTERBANDTYPE_LOWSHELF, 90.0f, 5.0f, 0.7f );
        e.SetBand( 1, true, AUDIO::FILTERBANDTYPE_PEAK, 250.0f, -7.0f, 2.0f );
        e.SetBand( 3, true, AUDIO::FILTERBANDTYPE_PEAK, 1300.0f, 9.0f, 4.0f );
        e.SetBand( 6, true, AUDIO::FILTERBANDTYPE_HIGHSHELF, 9000.0f, -6.0f, 0.7f );
        e.SetBand( 7, true, AUDIO::FILTERBANDTYPE_HIGHPASS, 30.0f, 0.0f, 0.7f );
    }
    else
    if ( type == "filter" )
    {
        AUDIO::CFilterEffect& e = static_cast< AUDIO::CFilterEffect& >( effect );
        e.SetFilterType( AUDIO::FILTERBANDTYPE_LOWPASS );
        e.SetFrequency( 3000.0f );
        e.SetSlopeInDecibelsPerOctave( 24 );
    }
    else
    {
        AUDIO::CDelayEffect& e = static_cast< AUDIO::CDelayEffect& >( effect );
        e.SetTimeInMs( 83.0f );
        e.SetFeedback( 0.55f );
        e.SetPingPong( true );
        e.SetStereoOffsetInMs( 7.5f );
    }
}

/** Make an effect, set it up as Exercise() does, prepare it */
AUDIO::TAudioEffectPtr
MakeExercised( const char* name, UInt32 channels, UInt32 rate, UInt32 maxBlock )
{
    AUDIO::TAudioEffectPtr effect = CreateByName( name );
    if ( effect.IsNULL() )
        return effect;
    Exercise( *effect, name );
    if ( !effect->Prepare( AUDIO::CAudioFormat( rate, channels, AUDIO::SAMPLETYPE_FLOAT32 ), maxBlock ) )
        effect.Unlink();
    return effect;
}

/** A value for a parameter that is not its default and survives writing to text and back */
Float32
SaveableValueFor( const AUDIO::CAudioEffectParameterInfo& info, CXorShift& random )
{
    switch ( info.GetKind() )
    {
        case AUDIO::AUDIOPARAMKIND_BOOLEAN : return random.Next() & 1 ? 1.0f : 0.0f;
        case AUDIO::AUDIOPARAMKIND_ENUMERATION : return (Float32) ( random.Next() % info.GetEnumLabelCount() );
        case AUDIO::AUDIOPARAMKIND_INTEGER : return info.Clamp( info.GetMinimum() + (Float32) ( random.Next() % 5 ) );
        default : return info.GetMinimum() + ( info.GetMaximum() - info.GetMinimum() ) * (Float32) ( random.Next() % 257 ) / 256.0f;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformRegistryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioBuiltinEffects: The registry makes every built-in effect type, by name without regard for case, and describes its parameters" )
    try
    {
        AUDIO::CAudioEffectRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry();
        ASSERT_TRUE( registry.CreateEffect( "reverbish" ).IsNULL() );

        for ( UInt32 e=0; e<EffectCount; ++e )
        {
            AUDIO::CAudioEffectRegistry::TRegisteredFactoryPtr factory;
            ASSERT_TRUE( registry.TryFindFactory( EffectNames[ e ], factory ) );
            ASSERT_TRUE( factory->GetEffectTypeName() == EffectNames[ e ] );

            CORE::CString upper( EffectNames[ e ] );
            upper = upper.Uppercase();
            AUDIO::TAudioEffectPtr effect = registry.CreateEffect( upper );
            ASSERT_FALSE( effect.IsNULL() );
            ASSERT_TRUE( effect->GetEffectTypeName() == EffectNames[ e ] );
            ASSERT_TRUE( effect->GetFactory() == factory.GetPointerAlways() );
            ASSERT_TRUE( factory->GetLiveInstanceCount() >= 1 );

            ASSERT_TRUE( factory->GetParameterCount() == effect->GetParameterCount() && effect->GetParameterCount() > 0 );
            for ( UInt32 p=0; p<effect->GetParameterCount(); ++p )
            {
                const AUDIO::CAudioEffectParameterInfo* info = effect->GetParameterInfo( p );
                ASSERT_TRUE( GUCEF_NULL != info && p == info->GetId() && !info->GetName().IsNULLOrEmpty() );
                ASSERT_TRUE( info->GetMinimum() <= info->GetDefaultValue() && info->GetDefaultValue() <= info->GetMaximum() );
                ASSERT_TRUE( effect->GetParameter( p ) == info->GetDefaultValue() );
                ASSERT_TRUE( factory->GetParameterInfo( p )->GetName() == info->GetName() );
                UInt32 id = 9999;
                ASSERT_TRUE( effect->TryGetParameterId( info->GetName(), id ) && id == p );             // the names are unique
            }

            // the effect works straight from the registry
            ASSERT_TRUE( effect->Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 512 ) );
            TSamples audio = MakeNoise( 2000, 2, 3, 0.4f );
            Run( *effect, audio, 2, 512 );
            ASSERT_TRUE( AllFiniteAndBelow( audio, 10.0f ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBuiltinEffects: Options given to the registry reach the effect" )
    try
    {
        CORE::CValueList bands;
        bands.Set( "bands", "4" );
        AUDIO::TAudioEffectPtr equalizer = CreateByName( "equalizer", bands );
        ASSERT_FALSE( equalizer.IsNULL() );
        ASSERT_TRUE( 20 == equalizer->GetParameterCount() );

        CORE::CValueList graphic;
        graphic.Set( "preset", "graphic10" );
        AUDIO::TAudioEffectPtr tenBands = CreateByName( "equalizer", graphic );
        ASSERT_TRUE( !tenBands.IsNULL() && 50 == tenBands->GetParameterCount() );

        CORE::CValueList maxDelay;
        maxDelay.Set( "maxDelay", "300" );
        AUDIO::TAudioEffectPtr delay = CreateByName( "delay", maxDelay );
        ASSERT_FALSE( delay.IsNULL() );
        ASSERT_TRUE( Near( static_cast< AUDIO::CDelayEffect* >( delay.GetPointerAlways() )->GetMaxDelayInMs(), 300.0, 1e-6 ) );

        CORE::CValueList mode;
        mode.Set( "mode", "instant" );
        AUDIO::TAudioEffectPtr limiter = CreateByName( "limiter", mode );
        ASSERT_FALSE( limiter.IsNULL() );
        ASSERT_TRUE( AUDIO::AUDIOLIMITERMODE_INSTANT == static_cast< AUDIO::CLimiterEffect* >( limiter.GetPointerAlways() )->GetMode() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformConfigurationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioBuiltinEffects: Parameters are saved by name and loaded again through CIConfigurable, enumerations by label" )
    try
    {
        for ( UInt32 e=0; e<EffectCount; ++e )
        {
            AUDIO::TAudioEffectPtr source = CreateByName( EffectNames[ e ] );
            AUDIO::TAudioEffectPtr target = CreateByName( EffectNames[ e ] );
            ASSERT_TRUE( !source.IsNULL() && !target.IsNULL() );

            // every parameter set to something, saved, loaded into a new effect
            CXorShift random( 100 + e );
            for ( UInt32 p=0; p<source->GetParameterCount(); ++p )
                source->SetParameter( p, SaveableValueFor( *source->GetParameterInfo( p ), random ) );
            CORE::CDataNode config( "Effect" );
            ASSERT_TRUE( source->SaveConfig( config ) );
            ASSERT_TRUE( config.GetAttributeValue( "type" ).AsString() == EffectNames[ e ] );
            const CORE::CDataNode* parameters = config.FindChild( "Parameters" );
            ASSERT_TRUE( GUCEF_NULL != parameters );
            for ( UInt32 p=0; p<source->GetParameterCount(); ++p )
                ASSERT_TRUE( parameters->IsAttribute( source->GetParameterInfo( p )->GetName() ) );

            ASSERT_TRUE( target->LoadConfig( config ) );
            for ( UInt32 p=0; p<source->GetParameterCount(); ++p )
            {
                const AUDIO::CAudioEffectParameterInfo& info = *source->GetParameterInfo( p );
                const Float64 range = (Float64) info.GetMaximum() - (Float64) info.GetMinimum();
                if ( !Near( source->GetParameter( p ), target->GetParameter( p ), 1.0e-5 * ( range > 1.0 ? range : 1.0 ) ) )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioBuiltinEffects: " << EffectNames[ e ] << " parameter " << info.GetName() << " saved " << source->GetParameter( p ) << " loaded " << target->GetParameter( p ) );
                ASSERT_TRUE( Near( source->GetParameter( p ), target->GetParameter( p ), 1.0e-5 * ( range > 1.0 ? range : 1.0 ) ) );
            }

            // the configuration of another type of effect is refused
            CORE::CDataNode foreign( "Effect" );
            foreign.SetAttribute( "type", EffectNames[ ( e + 1 ) % EffectCount ] );
            ASSERT_FALSE( target->LoadConfig( foreign ) );
        }

        // enumerations are written as their label
        AUDIO::TAudioEffectPtr limiter = CreateByName( "limiter" );
        static_cast< AUDIO::CLimiterEffect* >( limiter.GetPointerAlways() )->SetMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
        CORE::CDataNode config( "Effect" );
        ASSERT_TRUE( limiter->SaveConfig( config ) );
        ASSERT_TRUE( config.FindChild( "Parameters" )->GetAttributeValue( "mode" ).AsString() == "instant" );
        AUDIO::TAudioEffectPtr filter = CreateByName( "filter" );
        CORE::CDataNode filterConfig( "Effect" );
        filterConfig.FindOrAddChild( "Parameters" )->SetAttribute( "type", "notch" );
        filterConfig.FindOrAddChild( "Parameters" )->SetAttribute( "slope", (Int32) 24 );
        ASSERT_TRUE( filter->LoadConfig( filterConfig ) );
        ASSERT_TRUE( (Float32) AUDIO::FILTERBANDTYPE_NOTCH == filter->GetParameter( AUDIO::CFilterEffect::PARAMETER_TYPE ) );
        ASSERT_TRUE( 24.0f == filter->GetParameter( AUDIO::CFilterEffect::PARAMETER_SLOPE ) );

        // an equalizer with a different number of bands takes over the bands it has, and ignores the others
        CORE::CValueList bands;
        bands.Set( "bands", "2" );
        AUDIO::TAudioEffectPtr small = CreateByName( "equalizer", bands );
        AUDIO::TAudioEffectPtr full = CreateByName( "equalizer" );
        full->SetParameter( AUDIO::CEqualizerEffect::GetParameterId( 1, AUDIO::CEqualizerEffect::BANDFIELD_GAIN ), 6.0f );
        full->SetParameter( AUDIO::CEqualizerEffect::GetParameterId( 5, AUDIO::CEqualizerEffect::BANDFIELD_GAIN ), 6.0f );
        CORE::CDataNode fullConfig( "Effect" );
        ASSERT_TRUE( full->SaveConfig( fullConfig ) );
        ASSERT_TRUE( small->LoadConfig( fullConfig ) );
        ASSERT_TRUE( 6.0f == small->GetParameter( AUDIO::CEqualizerEffect::GetParameterId( 1, AUDIO::CEqualizerEffect::BANDFIELD_GAIN ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformProcessingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioBuiltinEffects: Output does not depend on how the frames are split into calls (10 s of noise, random block sizes), bit for bit" )
    try
    {
        const UInt32 frames = 480000;
        for ( UInt32 e=0; e<EffectCount; ++e )
        {
            const TSamples input = MakeNoise( frames, 2, 200 + e, 0.9f );

            AUDIO::TAudioEffectPtr whole = MakeExercised( EffectNames[ e ], 2, 48000, frames );
            AUDIO::TAudioEffectPtr split = MakeExercised( EffectNames[ e ], 2, 48000, 2048 );
            AUDIO::TAudioEffectPtr tiny = MakeExercised( EffectNames[ e ], 2, 48000, 2048 );
            ASSERT_TRUE( !whole.IsNULL() && !split.IsNULL() && !tiny.IsNULL() );

            TSamples a( input ), b( input ), c( input );
            Run( *whole, a, 2, frames );
            RunInRandomBlocks( *split, b, 2, 2000, 31 + e );
            Run( *tiny, c, 2, 33 );
            ASSERT_TRUE( IsSame( a, b ) );
            ASSERT_TRUE( IsSame( a, c ) );
            ASSERT_FALSE( IsSame( a, input ) );             // and the effect did something
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBuiltinEffects: Reset brings an effect back to the state of a new one" )
    try
    {
        for ( UInt32 e=0; e<EffectCount; ++e )
        {
            AUDIO::TAudioEffectPtr effect = MakeExercised( EffectNames[ e ], 2, 48000, 1024 );
            ASSERT_FALSE( effect.IsNULL() );
            const TSamples input = MakeNoise( 30000, 2, 300 + e, 0.8f );
            TSamples first( input );
            Run( *effect, first, 2, 777 );
            TSamples other = MakeNoise( 9000, 2, 301, 1.5f );
            Run( *effect, other, 2, 500 );
            ResetRealtime( *effect );
            TSamples second( input );
            Run( *effect, second, 2, 777 );
            ASSERT_TRUE( IsSame( first, second ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBuiltinEffects: Every channel count and sample rate works and gives numbers" )
    try
    {
        const UInt32 channelCounts[] = { 1, 2, 4, 6, 8 };
        const UInt32 rates[] = { 22050, 44100, 96000 };
        for ( UInt32 e=0; e<EffectCount; ++e )
            for ( UInt32 c=0; c<5; ++c )
                for ( UInt32 r=0; r<3; ++r )
                {
                    AUDIO::TAudioEffectPtr effect = MakeExercised( EffectNames[ e ], channelCounts[ c ], rates[ r ], 512 );
                    ASSERT_FALSE( effect.IsNULL() );
                    TSamples audio = MakeNoise( 6000, channelCounts[ c ], 7, 0.8f );
                    Run( *effect, audio, channelCounts[ c ], 300 );
                    ASSERT_TRUE( AllFiniteAndBelow( audio, 100.0f ) );
                }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBuiltinEffects: Decaying state does not end up in the denormal range: after loud audio and silence there is no denormal number, and the tail ends in exact zeros" )
    try
    {
        for ( UInt32 e=0; e<EffectCount; ++e )
        {
            AUDIO::TAudioEffectPtr effect = CreateByName( EffectNames[ e ] );
            ASSERT_FALSE( effect.IsNULL() );
            if ( e == 2 )
            {
                // a narrow low band that rings for a while (a boosted peak with this Q rings for tens of seconds, a cut one for a fraction of a second)
                static_cast< AUDIO::CEqualizerEffect* >( effect.GetPointerAlways() )->SetBand( 0, true, AUDIO::FILTERBANDTYPE_PEAK, 100.0f, -12.0f, 18.0f );
            }
            else
            if ( e == 3 )
            {
                AUDIO::CFilterEffect* filter = static_cast< AUDIO::CFilterEffect* >( effect.GetPointerAlways() );
                filter->SetFrequency( 100.0f );
                filter->SetQ( 18.0f );
            }
            else
            if ( e == 4 )
            {
                AUDIO::CDelayEffect* delay = static_cast< AUDIO::CDelayEffect* >( effect.GetPointerAlways() );
                delay->SetTimeInMs( 2.0f );
                delay->SetFeedback( -0.9f );
            }
            ASSERT_TRUE( effect->Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 1024 ) );

            TSamples audio = MakeNoise( 24000, 2, 400 + e, 0.9f );
            audio.resize( (size_t) 48000 * 2 * 7, 0.0f );                  // half a second of noise, then six and a half seconds of silence
            Run( *effect, audio, 2, 480 );
            ASSERT_TRUE( 0 == CountSubnormal( audio ) );
            ASSERT_TRUE( AllFiniteAndBelow( audio, 100.0f ) );
            size_t nonZero = 0;
            size_t firstNonZero = audio.size();
            for ( size_t i=audio.size()-48000*2; i<audio.size(); ++i )
                if ( 0.0f != audio[ i ] )
                {
                    ++nonZero;
                    if ( firstNonZero > i ) firstNonZero = i;
                }
            if ( nonZero > 0 )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioBuiltinEffects: " << EffectNames[ e ] << " is not silent after 6 s of silence: " << nonZero << " samples, the first at " << firstNonZero << " is " << audio[ firstNonZero ] );
            ASSERT_TRUE( 0 == nonZero );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBuiltinEffects: Numbers in the denormal range as input give numbers and no denormal output" )
    try
    {
        for ( UInt32 e=0; e<EffectCount; ++e )
        {
            AUDIO::TAudioEffectPtr effect = MakeExercised( EffectNames[ e ], 2, 48000, 1024 );
            ASSERT_FALSE( effect.IsNULL() );
            TSamples audio = MakeNoise( 20000, 2, 500 + e, 1.0e-40f );
            ASSERT_TRUE( CountSubnormal( audio ) > 0 );
            Run( *effect, audio, 2, 480 );
            ASSERT_TRUE( AllFiniteAndBelow( audio, 1.0f ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformRealtimeTests( void )
{GUCEF_TRACE;

    // Process() and Reset() are called through a function that is a real time context in a RealtimeSanitizer build: any allocation, lock
    // or blocking call in the effect, with parameters changing between the blocks, aborts the run there. Without the sanitizer the test
    // shows that the effects work through the same sequence of calls
    GUCEF_TESTFW_TESTCASE( "AudioBuiltinEffects: Processing, resetting and parameter changes between blocks allocate and block nothing (RealtimeSanitizer)" )
    try
    {
        for ( UInt32 e=0; e<EffectCount; ++e )
        {
            CORE::CValueList options;
            if ( 2 == e )
                options.Set( "bands", "16" );
            AUDIO::TAudioEffectPtr effect = CreateByName( EffectNames[ e ], options );
            ASSERT_FALSE( effect.IsNULL() );
            ASSERT_TRUE( effect->Prepare( AUDIO::CAudioFormat( 48000, 6, AUDIO::SAMPLETYPE_FLOAT32 ), 512 ) );

            CXorShift random( 900 + e );
            TSamples block = MakeNoise( 512, 6, 5, 0.8f );
            for ( UInt32 round=0; round<300; ++round )
            {
                // every parameter to a random value, now and then all at once
                if ( 0 == round % 3 )
                {
                    for ( UInt32 p=0; p<effect->GetParameterCount(); ++p )
                    {
                        const AUDIO::CAudioEffectParameterInfo& info = *effect->GetParameterInfo( p );
                        effect->SetParameter( p, info.GetMinimum() + ( info.GetMaximum() - info.GetMinimum() ) * (Float32) random.NextUnit() );
                    }
                }
                else
                if ( 1 == round % 3 )
                {
                    const UInt32 ids[ 2 ] = { random.Next() % effect->GetParameterCount(), random.Next() % effect->GetParameterCount() };
                    const Float32 values[ 2 ] = { (Float32) random.NextUnit() * 4.0f, (Float32) random.NextUnit() * 100.0f };
                    effect->SetParameters( ids, values, 2 );
                }
                if ( 0 == round % 50 )
                    ResetRealtime( *effect );
                TSamples data( block );
                const UInt32 frames = 1 + random.Next() % 512;
                ProcessRealtime( *effect, &data[ 0 ], frames );
                ASSERT_TRUE( AllFiniteAndBelow( data, 1.0e8f ) );
            }
        }

        // the limiter changes its lookahead and mode on the audio thread
        AUDIO::CLimiterEffect limiter;
        ASSERT_TRUE( limiter.Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 512 ) );
        TSamples data = MakeNoise( 512, 2, 6, 3.0f );
        const Float32 lookaheads[] = { 0.5f, 10.0f, 3.0f, 1.5f };
        for ( UInt32 i=0; i<12; ++i )
        {
            limiter.SetLookaheadInMs( lookaheads[ i % 4 ] );
            limiter.SetMode( (AUDIO::TAudioLimiterMode) ( i % 3 ) );
            TSamples copy( data );
            ProcessRealtime( limiter, &copy[ 0 ], 512 );
            ASSERT_TRUE( AllFiniteAndBelow( copy, 10.0f ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioBuiltinEffectTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioBuiltinEffects TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.BuiltinEffects" );

    PerformRegistryTests();
    PerformConfigurationTests();
    PerformProcessingTests();
    PerformRealtimeTests();
}

/*-------------------------------------------------------------------------*/
