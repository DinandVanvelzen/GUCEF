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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOTESTEFFECT_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOTESTEFFECT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_AUDIO_CTAUDIOEFFECTFACTORY_H
#include "gucefAUDIO_CTAudioEffectFactory.h"
#define GUCEF_AUDIO_CTAUDIOEFFECTFACTORY_H
#endif /* GUCEF_AUDIO_CTAUDIOEFFECTFACTORY_H ? */

#ifndef GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H
#include "gucefAUDIO_CAudioParameterSmoother.h"
#define GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H
#endif /* GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Effect for the framework tests: a one-pole low pass with a gain, both smoothed and updated on sub-blocks aligned to
 *  the processed frame count as the effect rules ask, plus two parameters that are always set together (pairA, pairB)
 *  so the tests can see whether they are ever observed apart. Records the thread its destructor ran on.
 *
 *  Option "invert" = true makes it invert the signal (an effect whose output is easy to recognise).
 */
class CTestAudioEffect : public GUCEF::AUDIO::CAudioEffect
{
    public:

    enum EParameter
    {
        PARAMETER_GAIN = 0  ,
        PARAMETER_CUTOFF    ,
        PARAMETER_PAIR_A    ,
        PARAMETER_PAIR_B    ,
        PARAMETER_MODE      ,
        PARAMETER_BYPASS    ,

        PARAMETER_COUNT
    };

    static const GUCEF::CORE::CString& GetStaticEffectTypeName( void )
    {
        static const GUCEF::CORE::CString typeName( "testeffect" );
        return typeName;
    }

    static GUCEF::UInt32 GetStaticParameterCount( void )
    {
        return PARAMETER_COUNT;
    }

    static const GUCEF::AUDIO::CAudioEffectParameterInfo* GetStaticParameterInfos( void )
    {
        static const char* const modeLabels[ 2 ] = { "plain", "inverted" };
        static const GUCEF::AUDIO::CAudioEffectParameterInfo infos[ PARAMETER_COUNT ] =
        {
            GUCEF::AUDIO::CAudioEffectParameterInfo( PARAMETER_GAIN, "gain", GUCEF::AUDIO::AUDIOPARAMUNIT_LINEAR_GAIN, GUCEF::AUDIO::AUDIOPARAMSCALE_LINEAR, 0.0f, 4.0f, 1.0f ),
            GUCEF::AUDIO::CAudioEffectParameterInfo( PARAMETER_CUTOFF, "cutoff", GUCEF::AUDIO::AUDIOPARAMUNIT_HERTZ, GUCEF::AUDIO::AUDIOPARAMSCALE_LOGARITHMIC, 20.0f, 20000.0f, 20000.0f ),
            GUCEF::AUDIO::CAudioEffectParameterInfo( PARAMETER_PAIR_A, "pairA", GUCEF::AUDIO::AUDIOPARAMUNIT_NONE, GUCEF::AUDIO::AUDIOPARAMSCALE_LINEAR, 0.0f, 1e9f, 0.0f, GUCEF::AUDIO::AUDIOPARAMKIND_INTEGER ),
            GUCEF::AUDIO::CAudioEffectParameterInfo( PARAMETER_PAIR_B, "pairB", GUCEF::AUDIO::AUDIOPARAMUNIT_NONE, GUCEF::AUDIO::AUDIOPARAMSCALE_LINEAR, 0.0f, 1e9f, 0.0f, GUCEF::AUDIO::AUDIOPARAMKIND_INTEGER ),
            GUCEF::AUDIO::CAudioEffectParameterInfo( PARAMETER_MODE, "mode", modeLabels, 2, 0 ),
            GUCEF::AUDIO::CAudioEffectParameterInfo( PARAMETER_BYPASS, "bypass", GUCEF::AUDIO::AUDIOPARAMUNIT_NONE, GUCEF::AUDIO::AUDIOPARAMSCALE_LINEAR, 0.0f, 1.0f, 0.0f, GUCEF::AUDIO::AUDIOPARAMKIND_BOOLEAN )
        };
        return infos;
    }

    /** Thread id of the destructor of the most recently destroyed test effect */
    static GUCEF::MT::CTAtomic< GUCEF::TThreadId >& LastDestructorThread( void )
    {
        static GUCEF::MT::CTAtomic< GUCEF::TThreadId > threadId( 0 );
        return threadId;
    }

    static GUCEF::MT::CTAtomic< GUCEF::UInt32 >& DestroyedCount( void )
    {
        static GUCEF::MT::CTAtomic< GUCEF::UInt32 > count( 0 );
        return count;
    }

    explicit CTestAudioEffect( const GUCEF::CORE::CValueList& options = GUCEF::CORE::CValueList() )
        : GUCEF::AUDIO::CAudioEffect( GetStaticParameterInfos(), PARAMETER_COUNT )
        , m_invert( options.GetValueAlways( "invert" ).AsBool( false ) )
        , m_channels( 0 )
        , m_rate( 48000.0 )
        , m_state()
        , m_gain()
        , m_cutoff()
        , m_currentGain( 1.0f )
        , m_coefficient( 1.0f )
        , m_mixedPairs( 0 )
        , m_pairChecks( 0 )
        , m_resets( 0 )
    {
    }

    virtual ~CTestAudioEffect()
    {
        LastDestructorThread().Store( GUCEF::MT::GetCurrentThreadID() );
        DestroyedCount().FetchAdd( 1 );
    }

    virtual const GUCEF::CORE::CString& GetEffectTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return GetStaticEffectTypeName();
    }

    /** Times the audio thread saw pairA and pairB differ, and times it looked */
    GUCEF::UInt32 GetMixedPairCount( void ) const { return m_mixedPairs.Load(); }
    GUCEF::UInt32 GetPairCheckCount( void ) const { return m_pairChecks.Load(); }
    GUCEF::UInt32 GetResetCount( void ) const { return m_resets.Load(); }

    protected:

    virtual bool OnPrepare( const GUCEF::AUDIO::CAudioFormat& format, GUCEF::UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_channels = format.GetChannelCount();
        m_rate = (GUCEF::Float64) format.GetSampleRate();
        m_state.assign( m_channels, 0.0f );
        m_gain.Prepare( GUCEF::AUDIO::AUDIOSMOOTHING_ONE_POLE, 0.020, m_rate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
        m_cutoff.Prepare( GUCEF::AUDIO::AUDIOSMOOTHING_ONE_POLE, 0.020, m_rate, true, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
        return true;
    }

    virtual void OnReset( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( size_t c=0; c<m_state.size(); ++c )
            m_state[ c ] = 0.0f;
        m_gain.Reset( ReadParameter( PARAMETER_GAIN ) );
        m_cutoff.Reset( ReadParameter( PARAMETER_CUTOFF ) );
        m_currentGain = m_gain.GetValue();
        m_coefficient = Coefficient( m_cutoff.GetValue() );
        m_resets.FetchAdd( 1, GUCEF::MT::MEMORDER_RELAXED );
    }

    virtual void OnProcess( GUCEF::Float32* inOut, GUCEF::UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( HaveParametersChanged() )
        {
            m_gain.SetTarget( ReadParameter( PARAMETER_GAIN ) );
            m_cutoff.SetTarget( ReadParameter( PARAMETER_CUTOFF ) );
            m_pairChecks.FetchAdd( 1, GUCEF::MT::MEMORDER_RELAXED );
            if ( ReadParameter( PARAMETER_PAIR_A ) != ReadParameter( PARAMETER_PAIR_B ) )
                m_mixedPairs.FetchAdd( 1, GUCEF::MT::MEMORDER_RELAXED );
        }
        if ( 0.0f != ReadParameter( PARAMETER_BYPASS ) )
            return;
        const GUCEF::Float32 sign = ( m_invert || 1.0f == ReadParameter( PARAMETER_MODE ) ) ? -1.0f : 1.0f;

        GUCEF::UInt64 position = GetProcessedFrameCount();
        GUCEF::UInt32 done = 0;
        while ( done < frames )
        {
            const GUCEF::UInt32 inSubBlock = (GUCEF::UInt32) ( position % GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
            if ( 0 == inSubBlock )
            {
                m_currentGain = m_gain.Step();
                if ( m_cutoff.IsSmoothing() )
                    m_coefficient = Coefficient( m_cutoff.Step() );
            }
            GUCEF::UInt32 run = GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES - inSubBlock;
            if ( run > frames - done )
                run = frames - done;
            for ( GUCEF::UInt32 f=0; f<run; ++f )
            {
                GUCEF::Float32* frame = inOut + (size_t) ( done + f ) * m_channels;
                for ( GUCEF::UInt32 c=0; c<m_channels; ++c )
                {
                    if ( 1.0f == m_coefficient )
                        m_state[ c ] = frame[ c ];                                          // fully open: exactly the input
                    else
                        m_state[ c ] += m_coefficient * ( frame[ c ] - m_state[ c ] );
                    frame[ c ] = sign * m_currentGain * m_state[ c ];
                }
            }
            done += run;
            position += run;
        }
    }

    private:

    GUCEF::Float32 Coefficient( GUCEF::Float32 cutoff ) const
    {
        if ( cutoff >= 20000.0f )
            return 1.0f;                    // fully open: the filter passes the signal unchanged
        return (GUCEF::Float32) ( 1.0 - exp( -2.0 * 3.14159265358979 * (GUCEF::Float64) cutoff / m_rate ) );
    }

    bool m_invert;
    GUCEF::UInt32 m_channels;
    GUCEF::Float64 m_rate;
    GUCEF::vector< GUCEF::Float32 > m_state;
    GUCEF::AUDIO::CAudioParameterSmoother m_gain;
    GUCEF::AUDIO::CAudioParameterSmoother m_cutoff;
    GUCEF::Float32 m_currentGain;
    GUCEF::Float32 m_coefficient;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > m_mixedPairs;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > m_pairChecks;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > m_resets;
};

typedef GUCEF::AUDIO::CTAudioEffectFactory< CTestAudioEffect > TTestAudioEffectFactory;

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOTESTEFFECT_H ? */
