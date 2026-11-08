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

#include "gucefAUDIO_ETypes.h"

#include "gucefAUDIO_CReverbEffect.h"

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

const Float64 Pi = 3.14159265358979323846;

const Float64 LongestPreDelayMs = 500.0;
const Float64 LongestEarlyTapMs = 80.0;

const Float64 SizeSmoothingSeconds = 0.200;
const Float64 PreDelaySmoothingSeconds = 0.100;
const Float64 ParameterSmoothingSeconds = 0.020;

/** The frequencies at which the decay time (low) and the decay time multiplied by the high frequency ratio (high) apply */
const Float64 LowFrequencyReference = 250.0;
const Float64 HighFrequencyReference = 4000.0;

/** The pole of an absorption filter stays below this: close to 1 is a filter that passes next to nothing */
const Float64 LargestAbsorptionPole = 0.995;

/** The gain of an absorption filter at DC may be this much more than at the low reference frequency */
const Float64 MostDcGainOverLow = 1.05;

/** Excursion of the modulated delay lines at depth 1, in frames at 48 kHz */
const Float64 ModulationFramesAt48k = 8.0;
const Float64 ModulationFrequencies[ 2 ] = { 0.3, 0.7 };

/** The filters on the input and the output are Butterworth sections */
const Float64 FilterQ = 0.7071067811865476;

/** Feedback state below this is silence; flushing it keeps the network out of the denormal range */
const Float32 DenormalFlushThreshold = 1.0e-15f;

/** Cut frequencies at these values switch the filter off */
const Float32 LowCutOff = 20.001f;
const Float32 HighCutOff = 19999.0f;

/** Level at which the early and late reverberation are silent */
const Float32 LevelFloorDecibels = -60.0f;

/** Time in milliseconds of the delay lines at size 1: mutually prime after rounding to a number of frames */
const Float32 LineMilliseconds[ 8 ] = { 29.7f, 37.1f, 41.1f, 43.7f, 53.3f, 59.9f, 67.7f, 73.1f };

/** Low quality uses these four of them */
const UInt32 LowQualityLines[ 4 ] = { 0, 2, 4, 6 };

/** Early reflection times in milliseconds at size 1 and the sign of each reflection */
const Float32 TapMilliseconds[ 12 ] = { 5.0f, 7.9f, 11.6f, 15.3f, 20.2f, 25.7f, 32.1f, 38.9f, 47.3f, 57.1f, 67.9f, 80.0f };
const Float32 TapSigns[ 12 ] = { 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f };
const Float64 TapDecay = 0.85;

/** Lengths of the diffusion allpasses of Dattorro at 29.761 kHz */
const UInt32 DiffusionLengths[ 4 ] = { 142, 107, 379, 277 };
const Float64 DiffusionRate = 29761.0;
const Float64 RightDiffusionScale = 1.13;

/** Rows of the Hadamard matrix that the output channels take, in order of the channels */
const UInt32 OutputRowsNormal[ 8 ] = { 1, 2, 3, 4, 5, 6, 7, 0 };
const UInt32 OutputRowsLow[ 4 ] = { 1, 2, 3, 0 };

inline bool
IsPrime( UInt32 n )
{
    if ( n < 2 )
        return false;
    if ( n < 4 )
        return true;
    if ( 0 == ( n & 1 ) )
        return false;
    for ( UInt32 d=3; d*d<=n; d+=2 )
        if ( 0 == n % d )
            return false;
    return true;
}

/** The prime number nearest to n, the lower one when two are as near */
UInt32
NearestPrime( UInt32 n )
{
    if ( n <= 2 )
        return 2;
    for ( UInt32 d=0; ; ++d )
    {
        if ( n > d && IsPrime( n - d ) )
            return n - d;
        if ( IsPrime( n + d ) )
            return n + d;
    }
}

UInt32
NextPrime( UInt32 n )
{
    do
    {
        ++n;
    }
    while ( !IsPrime( n ) );
    return n;
}

/** Sign of entry i of row 'row' of the Sylvester Hadamard matrix: -1 to the power of the number of ones in i AND row */
inline Float32
HadamardSign( UInt32 i, UInt32 row )
{
    UInt32 v = i & row;
    v ^= v >> 1;
    v ^= v >> 2;
    return ( v & 1 ) ? -1.0f : 1.0f;
}

/**
 *  Sign of line 'line' in the output row of the channel with index 'channelIndex' (not counting the low frequency effects
 *  channel). The first channels take rows of the Hadamard matrix, which are orthogonal. Four lines have only four such rows,
 *  the next four channels take the rows of the matrix with one negative entry (orthogonal among each other, a correlation of
 *  one half with the Hadamard rows), and any channels beyond those a fixed pseudo-random pattern.
 */
Float32
OutputSign( UInt32 lineCount, UInt32 channelIndex, UInt32 line )
{
    if ( channelIndex < lineCount )
        return HadamardSign( line, lineCount == 8 ? OutputRowsNormal[ channelIndex ] : OutputRowsLow[ channelIndex ] );
    if ( 4 == lineCount && channelIndex < 8 )
        return line == 7 - channelIndex ? -1.0f : 1.0f;
    UInt32 h = channelIndex * 2654435761U + line * 40503U + 977U;
    h ^= h >> 13;
    h *= 1274126177U;
    h ^= h >> 16;
    return ( h & 0x100 ) ? -1.0f : 1.0f;
}

inline Float32
LevelToGain( Float32 decibels )
{
    if ( decibels <= LevelFloorDecibels )
        return 0.0f;
    return (Float32) pow( 10.0, (Float64) decibels / 20.0 );
}

/** Which side of the room a speaker is on: -1 left, 1 right, 0 centre */
int
SideOfSpeaker( UInt32 speaker )
{
    const UInt32 left = AUDIOSPEAKER_FRONT_LEFT | AUDIOSPEAKER_BACK_LEFT | AUDIOSPEAKER_FRONT_LEFT_OF_CENTER |
                        AUDIOSPEAKER_SIDE_LEFT | AUDIOSPEAKER_TOP_FRONT_LEFT | AUDIOSPEAKER_TOP_BACK_LEFT;
    const UInt32 right = AUDIOSPEAKER_FRONT_RIGHT | AUDIOSPEAKER_BACK_RIGHT | AUDIOSPEAKER_FRONT_RIGHT_OF_CENTER |
                         AUDIOSPEAKER_SIDE_RIGHT | AUDIOSPEAKER_TOP_FRONT_RIGHT | AUDIOSPEAKER_TOP_BACK_RIGHT;
    if ( speaker & left )
        return -1;
    if ( speaker & right )
        return 1;
    return 0;
}

/** One stage of a Schroeder allpass: v = x - g d, y = d + g v, with d the value 'length' frames old */
inline Float32
AllpassStep( CDelayLine& ring, UInt32 channel, UInt32 length, Float32 g, Float32 x )
{
    const Float32 d = ring.Read( channel, length - 1 );
    Float32 v = x - g * d;
    if ( fabsf( v ) < DenormalFlushThreshold )
        v = 0.0f;
    ring.Push( channel, v );
    return d + g * v;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString&
CReverbEffect::GetStaticEffectTypeName( void )
{GUCEF_TRACE;

    static const CString typeName( "reverb" );
    return typeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CReverbEffect::GetStaticParameterCount( void )
{GUCEF_TRACE;

    return PARAMETER_COUNT;
}

/*-------------------------------------------------------------------------*/

const CAudioEffectParameterInfo*
CReverbEffect::GetStaticParameterInfos( void )
{GUCEF_TRACE;

    static const CAudioEffectParameterInfo infos[ PARAMETER_COUNT ] =
    {
        CAudioEffectParameterInfo( PARAMETER_PRE_DELAY, "preDelay", AUDIOPARAMUNIT_MILLISECONDS, AUDIOPARAMSCALE_LINEAR, 0.0f, 500.0f, 20.0f ),
        CAudioEffectParameterInfo( PARAMETER_SIZE, "size", AUDIOPARAMUNIT_NONE, AUDIOPARAMSCALE_LINEAR, 0.1f, 1.0f, 0.6f ),
        CAudioEffectParameterInfo( PARAMETER_DECAY_TIME, "decayTime", AUDIOPARAMUNIT_SECONDS, AUDIOPARAMSCALE_LOGARITHMIC, 0.1f, 20.0f, 1.8f ),
        CAudioEffectParameterInfo( PARAMETER_HF_RATIO, "hfRatio", AUDIOPARAMUNIT_RATIO, AUDIOPARAMSCALE_LINEAR, 0.1f, 1.0f, 0.5f ),
        CAudioEffectParameterInfo( PARAMETER_DIFFUSION, "diffusion", AUDIOPARAMUNIT_NONE, AUDIOPARAMSCALE_LINEAR, 0.0f, 0.75f, 0.6f ),
        CAudioEffectParameterInfo( PARAMETER_EARLY_LEVEL, "earlyLevel", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -60.0f, 0.0f, -6.0f ),
        CAudioEffectParameterInfo( PARAMETER_LATE_LEVEL, "lateLevel", AUDIOPARAMUNIT_DECIBEL, AUDIOPARAMSCALE_LINEAR, -60.0f, 0.0f, -3.0f ),
        CAudioEffectParameterInfo( PARAMETER_LOW_CUT, "lowCut", AUDIOPARAMUNIT_HERTZ, AUDIOPARAMSCALE_LOGARITHMIC, 20.0f, 1000.0f, 80.0f ),
        CAudioEffectParameterInfo( PARAMETER_HIGH_CUT, "highCut", AUDIOPARAMUNIT_HERTZ, AUDIOPARAMSCALE_LOGARITHMIC, 1000.0f, 20000.0f, 9000.0f ),
        CAudioEffectParameterInfo( PARAMETER_WIDTH, "width", AUDIOPARAMUNIT_NONE, AUDIOPARAMSCALE_LINEAR, 0.0f, 1.0f, 1.0f ),
        CAudioEffectParameterInfo( PARAMETER_MODULATION, "modulationDepth", AUDIOPARAMUNIT_NONE, AUDIOPARAMSCALE_LINEAR, 0.0f, 1.0f, 0.3f )
    };
    return infos;
}

/*-------------------------------------------------------------------------*/

CReverbEffect::CReverbEffect( const CORE::CValueList& options )
    : CAudioEffect( GetStaticParameterInfos(), PARAMETER_COUNT )
    , m_quality( QUALITY_NORMAL )
    , m_lineCount( MAX_LINES )
    , m_channels( 0 )
    , m_sampleRate( 48000.0 )
    , m_active()
    , m_leftWeight()
    , m_rightWeight()
    , m_outputSigns()
    , m_activeCount( 0 )
    , m_pre()
    , m_diffusion()
    , m_lines()
    , m_lowCutFilter()
    , m_highCutFilter()
    , m_lowCutActive( false )
    , m_highCutActive( false )
    , m_lineGlide( 0.0f )
    , m_lineGlideCoefficient( 0.0f )
    , m_lastSize( 0.0f )
    , m_sizeStart( 0.0f )
    , m_sizeTarget( 0.0f )
    , m_sizeCurrent( 0.0f )
    , m_absorptionDirty( true )
    , m_cosLowReference( 1.0 )
    , m_cosReference( 0.0 )
    , m_preStart( 0.0f )
    , m_preTarget( 0.0f )
    , m_preCurrent( 0.0f )
    , m_preGlide( 0.0f )
    , m_preGlideCoefficient( 0.0f )
    , m_hasEarly( true )
    , m_modulationFrames( 0.0f )
    , m_decay()
    , m_hfRatio()
    , m_diffusionAmount()
    , m_lowCut()
    , m_highCut()
    , m_earlyLevel()
    , m_lateLevel()
    , m_width()
    , m_modulation()
    , m_diffusionCoefficient( 0.0f )
    , m_earlyActive( false )
    , m_widthActive( false )
    , m_modulationActive( false )
{GUCEF_TRACE;

    const CString quality = options.GetValueAlways( "quality" ).AsString().Lowercase();
    if ( quality == "low" )
    {
        m_quality = QUALITY_LOW;
        m_lineCount = MAX_LINES / 2;
        m_hasEarly = false;
    }

    for ( UInt32 i=0; i<2*DIFFUSION_STAGES; ++i )
        m_diffusionLength[ i ] = 2;
    for ( UInt32 q=0; q<2; ++q )
    {
        m_modulatedLine[ q ] = q;
        m_lfoSine[ q ] = 0.0;
        m_lfoCosine[ q ] = 1.0;
        m_lfoRotateSine[ q ] = 0.0;
        m_lfoRotateCosine[ q ] = 1.0;
        m_modOffset[ q ] = 0.0f;
        m_modStep[ q ] = 0.0f;
        m_modEnd[ q ] = 0.0f;
        for ( UInt32 i=0; i<MAX_LINES; ++i )
            m_injection[ q ][ i ] = 0.0f;
    }
    for ( UInt32 i=0; i<MAX_LINES; ++i )
    {
        m_linePosition[ i ] = 0.0f;
        m_lineStart[ i ] = 0.0f;
        m_lineTarget[ i ] = 0.0f;
        m_lineA[ i ] = 0.0f;
        m_lineB[ i ] = 0.0f;
        m_lineState[ i ] = 0.0f;
    }
    for ( UInt32 k=0; k<EARLY_TAPS; ++k )
    {
        m_tapFrames[ k ] = 0.0f;
        m_tapGain[ k ] = 0.0f;
        m_tapDelay[ k ] = 0.0f;
    }
    SRamp zero = { 0.0f, 0.0f, 0.0f };
    m_earlyRamp = zero;
    m_lateRamp = zero;
    m_widthRamp = zero;
    m_modulationRamp = zero;
}

/*-------------------------------------------------------------------------*/

CReverbEffect::~CReverbEffect()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CReverbEffect::GetEffectTypeName( void ) const
{GUCEF_TRACE;

    return GetStaticEffectTypeName();
}

/*-------------------------------------------------------------------------*/

CReverbEffect::EQuality
CReverbEffect::GetQuality( void ) const
{GUCEF_TRACE;

    return m_quality;
}

/*-------------------------------------------------------------------------*/

UInt32
CReverbEffect::GetLineCount( void ) const
{GUCEF_TRACE;

    return m_lineCount;
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::GetConstructionOptions( CORE::CValueList& options ) const
{GUCEF_TRACE;

    options.Set( "quality", QUALITY_LOW == m_quality ? "low" : "normal" );
}

/*-------------------------------------------------------------------------*/

UInt32
CReverbEffect::GetTailInFrames( void ) const
{GUCEF_TRACE;

    // the feedback network dies away in 1.5 times the decay time (-90 dB); the input diffusion rings on its own for as long
    // as its longest allpass needs to fall by 90 dB (one trip around that allpass loses 20 log10( 1 / g ) dB), and the
    // delay lines come on top of that; at short decay times the diffusion is what is left
    Float64 body = (Float64) GetParameter( PARAMETER_DECAY_TIME ) * 1.5 * m_sampleRate;
    const Float64 diffusion = (Float64) GetParameter( PARAMETER_DIFFUSION );
    if ( diffusion > 0.0 )
    {
        const Float64 longestStage = floor( floor( (Float64) DiffusionLengths[ 2 ] * m_sampleRate / DiffusionRate + 0.5 ) * RightDiffusionScale + 0.5 );
        const Float64 trips = 90.0 / ( -20.0 * log10( diffusion ) );
        const Float64 ring = trips * longestStage + (Float64) LineMilliseconds[ 7 ] * (Float64) GetParameter( PARAMETER_SIZE ) * m_sampleRate / 1000.0;
        if ( ring > body )
            body = ring;
    }
    const Float64 preDelay = (Float64) GetParameter( PARAMETER_PRE_DELAY ) * m_sampleRate / 1000.0;
    return (UInt32) ( ceil( body ) + floor( preDelay + 0.5 ) );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::SetPreDelayInMs( Float32 milliseconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_PRE_DELAY, milliseconds );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::SetSize( Float32 size )
{GUCEF_TRACE;

    SetParameter( PARAMETER_SIZE, size );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::SetDecayTimeInSeconds( Float32 seconds )
{GUCEF_TRACE;

    SetParameter( PARAMETER_DECAY_TIME, seconds );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::SetHfRatio( Float32 ratio )
{GUCEF_TRACE;

    SetParameter( PARAMETER_HF_RATIO, ratio );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::SetDiffusion( Float32 diffusion )
{GUCEF_TRACE;

    SetParameter( PARAMETER_DIFFUSION, diffusion );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::SetEarlyLevelInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_EARLY_LEVEL, decibels );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::SetLateLevelInDecibels( Float32 decibels )
{GUCEF_TRACE;

    SetParameter( PARAMETER_LATE_LEVEL, decibels );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::SetLowCutInHertz( Float32 hertz )
{GUCEF_TRACE;

    SetParameter( PARAMETER_LOW_CUT, hertz );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::SetHighCutInHertz( Float32 hertz )
{GUCEF_TRACE;

    SetParameter( PARAMETER_HIGH_CUT, hertz );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::SetWidth( Float32 width )
{GUCEF_TRACE;

    SetParameter( PARAMETER_WIDTH, width );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::SetModulationDepth( Float32 depth )
{GUCEF_TRACE;

    SetParameter( PARAMETER_MODULATION, depth );
}

/*-------------------------------------------------------------------------*/

bool
CReverbEffect::OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames )
{GUCEF_TRACE;

    m_channels = format.GetChannelCount();
    m_sampleRate = (Float64) format.GetSampleRate();
    const UInt32 lineCount = m_lineCount;
    const Float64 invSqrtLines = 1.0 / sqrt( (Float64) lineCount );

    // which channels take part: those that are not low frequency effects, and the side of the room each is on
    GUCEF::vector< UInt32 > speakers( m_channels, 0 );
    {
        // the speakers of the layout in channel order; a layout that does not name every channel leaves them unknown
        const UInt32 layout = format.GetChannelLayout();
        GUCEF::vector< UInt32 > ordered;
        for ( UInt32 bit=0; bit<32; ++bit )
            if ( ( layout >> bit ) & 1 )
                ordered.push_back( 1U << bit );
        if ( ordered.size() == m_channels )
            speakers = ordered;
    }
    m_active.clear();
    m_leftWeight.clear();
    m_rightWeight.clear();
    const Float32 centre = 0.70710678118654752f;
    for ( UInt32 c=0; c<m_channels; ++c )
    {
        int side = 0;
        if ( 0 != speakers[ c ] )
        {
            if ( AUDIOSPEAKER_LOW_FREQUENCY == speakers[ c ] )
                continue;
            side = SideOfSpeaker( speakers[ c ] );
        }
        else
            side = 1 == m_channels ? 0 : ( ( c & 1 ) ? 1 : -1 );
        m_active.push_back( c );
        m_leftWeight.push_back( side < 0 ? 1.0f : ( side > 0 ? 0.0f : centre ) );
        m_rightWeight.push_back( side > 0 ? 1.0f : ( side < 0 ? 0.0f : centre ) );
    }
    m_activeCount = (UInt32) m_active.size();

    // the row of the Hadamard matrix of every output channel, scaled so that a row has unit energy
    m_outputSigns.assign( (size_t) m_activeCount * MAX_LINES, 0.0f );
    for ( UInt32 a=0; a<m_activeCount; ++a )
    {
        for ( UInt32 i=0; i<lineCount; ++i )
            m_outputSigns[ (size_t) a * MAX_LINES + i ] = (Float32) ( OutputSign( lineCount, a, i ) * invSqrtLines );
    }
    for ( UInt32 i=0; i<MAX_LINES; ++i )
    {
        m_injection[ 0 ][ i ] = i < lineCount ? (Float32) invSqrtLines : 0.0f;
        m_injection[ 1 ][ i ] = i < lineCount ? (Float32) ( HadamardSign( i, lineCount - 1 ) * invSqrtLines ) : 0.0f;
    }

    // memory for the largest size and pre-delay
    const UInt32 modulationMax = (UInt32) ceil( ModulationFramesAt48k * m_sampleRate / 48000.0 );
    const UInt32 longestLine = (UInt32) ceil( (Float64) LineMilliseconds[ 7 ] * m_sampleRate / 1000.0 ) + 100 + modulationMax;
    const UInt32 longestPre = (UInt32) ceil( ( LongestPreDelayMs + LongestEarlyTapMs ) * m_sampleRate / 1000.0 ) + 8;
    UInt32 longestDiffusion = 0;
    for ( UInt32 j=0; j<DIFFUSION_STAGES; ++j )
    {
        Float64 left = floor( (Float64) DiffusionLengths[ j ] * m_sampleRate / DiffusionRate + 0.5 );
        if ( left < 8.0 )
            left = 8.0;
        const Float64 right = floor( left * RightDiffusionScale + 0.5 );
        m_diffusionLength[ j ] = (UInt32) left;
        m_diffusionLength[ DIFFUSION_STAGES + j ] = (UInt32) right;
        if ( (UInt32) right > longestDiffusion )
            longestDiffusion = (UInt32) right;
    }
    if ( !m_pre.Prepare( 2, longestPre ) ||
         !m_diffusion.Prepare( 2 * DIFFUSION_STAGES, longestDiffusion + 2 ) ||
         !m_lines.Prepare( lineCount, longestLine ) ||
         !m_lowCutFilter.Prepare( 2 ) || !m_highCutFilter.Prepare( m_channels ) )
        return false;

    m_modulationFrames = (Float32) ( ModulationFramesAt48k * m_sampleRate / 48000.0 );
    if ( MAX_LINES == lineCount )
    {
        m_modulatedLine[ 0 ] = 1;
        m_modulatedLine[ 1 ] = 4;
    }
    else
    {
        m_modulatedLine[ 0 ] = 1;
        m_modulatedLine[ 1 ] = 2;
    }
    for ( UInt32 q=0; q<2; ++q )
    {
        const Float64 angle = 2.0 * Pi * ModulationFrequencies[ q ] * (Float64) GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES / m_sampleRate;
        m_lfoRotateSine[ q ] = sin( angle );
        m_lfoRotateCosine[ q ] = cos( angle );
    }

    // early reflections: times and decaying gains, normalised per side
    Float64 energy[ 2 ] = { 0.0, 0.0 };
    for ( UInt32 k=0; k<EARLY_TAPS; ++k )
    {
        m_tapFrames[ k ] = (Float32) ( (Float64) TapMilliseconds[ k ] * m_sampleRate / 1000.0 );
        const Float64 gain = pow( TapDecay, (Float64) k );
        m_tapGain[ k ] = (Float32) gain;
        energy[ k & 1 ] += gain * gain;
    }
    for ( UInt32 k=0; k<EARLY_TAPS; ++k )
        m_tapGain[ k ] = TapSigns[ k ] * (Float32) ( (Float64) m_tapGain[ k ] / sqrt( energy[ k & 1 ] ) );

    // the absorption filters are matched at two reference frequencies, a little below Nyquist at the lowest sample rates
    Float64 lowReference = LowFrequencyReference;
    if ( lowReference > 0.1 * m_sampleRate )
        lowReference = 0.1 * m_sampleRate;
    Float64 highReference = HighFrequencyReference;
    if ( highReference > 0.45 * m_sampleRate )
        highReference = 0.45 * m_sampleRate;
    m_cosLowReference = cos( 2.0 * Pi * lowReference / m_sampleRate );
    m_cosReference = cos( 2.0 * Pi * highReference / m_sampleRate );

    m_lineGlideCoefficient = (Float32) exp( -1.0 / ( SizeSmoothingSeconds * m_sampleRate ) );
    m_preGlideCoefficient = (Float32) exp( -1.0 / ( PreDelaySmoothingSeconds * m_sampleRate ) );

    m_decay.Prepare( AUDIOSMOOTHING_ONE_POLE, ParameterSmoothingSeconds, m_sampleRate, true, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_hfRatio.Prepare( AUDIOSMOOTHING_ONE_POLE, ParameterSmoothingSeconds, m_sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_diffusionAmount.Prepare( AUDIOSMOOTHING_ONE_POLE, ParameterSmoothingSeconds, m_sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_lowCut.Prepare( AUDIOSMOOTHING_ONE_POLE, ParameterSmoothingSeconds, m_sampleRate, true, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_highCut.Prepare( AUDIOSMOOTHING_ONE_POLE, ParameterSmoothingSeconds, m_sampleRate, true, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_earlyLevel.Prepare( AUDIOSMOOTHING_LINEAR_RAMP, ParameterSmoothingSeconds, m_sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_lateLevel.Prepare( AUDIOSMOOTHING_LINEAR_RAMP, ParameterSmoothingSeconds, m_sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_width.Prepare( AUDIOSMOOTHING_LINEAR_RAMP, ParameterSmoothingSeconds, m_sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    m_modulation.Prepare( AUDIOSMOOTHING_LINEAR_RAMP, ParameterSmoothingSeconds, m_sampleRate, false, GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::ComputeLineTargets( Float32 size, Float32* targets ) const
{
    UInt32 previous = 0;
    for ( UInt32 i=0; i<m_lineCount; ++i )
    {
        const UInt32 base = MAX_LINES == m_lineCount ? i : LowQualityLines[ i ];
        Float64 frames = floor( (Float64) LineMilliseconds[ base ] * (Float64) size * m_sampleRate / 1000.0 + 0.5 );
        if ( frames < 16.0 )
            frames = 16.0;
        UInt32 prime = NearestPrime( (UInt32) frames );
        if ( prime <= previous )
            prime = NextPrime( previous );
        previous = prime;
        targets[ i ] = (Float32) prime;
    }
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::UpdateRestTaps( void )
{
    for ( UInt32 k=0; k<EARLY_TAPS; ++k )
        m_tapDelay[ k ] = floorf( m_preTarget + m_tapFrames[ k ] * m_sizeTarget + 0.5f );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::UpdateAbsorption( void )
{
    // gain of a line at the low reference frequency (250 Hz): g1 = 10^( -3 L / ( fs T60 ) ); at the high reference frequency
    // (4 kHz) r g1 with r = 10^( -3 L ( 1 / hfRatio - 1 ) / ( fs T60 ) ), which makes the decay time there T60 multiplied by
    // hfRatio. The first order low pass b / ( 1 - a z^-1 ) has gain g1 at the first and r g1 at the second frequency when a is
    // the smaller root of ( 1 - r^2 ) a^2 - 2 ( cos w1 - r^2 cos w2 ) a + ( 1 - r^2 ) = 0 and b = g1 sqrt( 1 - 2 a cos w1 + a^2 ).
    // With the low reference at DC this is the filter gDC ( 1 - a ) / ( 1 - a z^-1 )
    const Float64 ln10 = 2.30258509299404568402;
    const Float64 decay = (Float64) m_decay.GetValue();
    const Float64 hfRatio = (Float64) m_hfRatio.GetValue();
    const Float64 perFrame = -3.0 / ( m_sampleRate * decay );
    const Float64 c1 = m_cosLowReference;
    const Float64 c2 = m_cosReference;
    for ( UInt32 i=0; i<m_lineCount; ++i )
    {
        const Float64 e = perFrame * (Float64) m_linePosition[ i ];
        const Float64 g1 = exp( ln10 * e );
        const Float64 r = exp( ln10 * e * ( 1.0 / hfRatio - 1.0 ) );
        const Float64 r2 = r * r;
        Float64 a = 0.0;
        Float64 b = g1;
        if ( 1.0 - r2 > 1.0e-9 )
        {
            // matched at the two reference frequencies; when that needs a pole so close to 1 that the gain at DC rises above
            // the gain at the low reference (an infeasible slope for one pole, short decay times with a low ratio), the filter
            // is matched at DC instead, which never has a gain above g1
            const Float64 t = c1 - r2 * c2;
            const Float64 disc = t * t - ( 1.0 - r2 ) * ( 1.0 - r2 );
            if ( disc >= 0.0 )
            {
                a = ( 1.0 - r2 ) / ( t + sqrt( disc ) );
                b = g1 * sqrt( 1.0 - 2.0 * a * c1 + a * a );
            }
            if ( disc < 0.0 || !( a >= 0.0 && a < LargestAbsorptionPole && b <= g1 * MostDcGainOverLow * ( 1.0 - a ) ) )
            {
                const Float64 tDc = 1.0 - r2 * c2;
                const Float64 discDc = r2 * ( 1.0 - c2 ) * ( 2.0 - r2 * ( 1.0 + c2 ) );
                a = ( 1.0 - r2 ) / ( tDc + sqrt( discDc > 0.0 ? discDc : 0.0 ) );
                if ( !( a >= 0.0 ) )
                    a = 0.0;
                if ( a > LargestAbsorptionPole )
                    a = LargestAbsorptionPole;
                b = g1 * ( 1.0 - a );
            }
        }
        m_lineA[ i ] = (Float32) a;
        m_lineB[ i ] = (Float32) b;
    }
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::UpdateFilters( void )
{
    const Float32 lowCut = m_lowCut.GetValue();
    const Float32 highCut = m_highCut.GetValue();
    m_lowCutActive = lowCut > LowCutOff;
    m_highCutActive = highCut < HighCutOff;
    if ( m_lowCutActive )
        m_lowCutFilter.SetResponse( FILTERRESPONSE_HIGHPASS, m_sampleRate, (Float64) lowCut, FilterQ );
    if ( m_highCutActive )
        m_highCutFilter.SetResponse( FILTERRESPONSE_LOWPASS, m_sampleRate, (Float64) highCut, FilterQ );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::ApplyParameters( bool jump )
{
    const Float32 size = ReadParameter( PARAMETER_SIZE );
    const Float32 preDelay = floorf( (Float32) ( (Float64) ReadParameter( PARAMETER_PRE_DELAY ) * m_sampleRate / 1000.0 + 0.5 ) );

    if ( jump )
    {
        m_lastSize = size;
        ComputeLineTargets( size, m_lineTarget );
        for ( UInt32 i=0; i<m_lineCount; ++i )
        {
            m_linePosition[ i ] = m_lineTarget[ i ];
            m_lineStart[ i ] = m_lineTarget[ i ];
        }
        m_lineGlide = 0.0f;
        m_sizeStart = size;
        m_sizeTarget = size;
        m_sizeCurrent = size;
        m_preStart = preDelay;
        m_preTarget = preDelay;
        m_preCurrent = preDelay;
        m_preGlide = 0.0f;

        m_decay.Reset( ReadParameter( PARAMETER_DECAY_TIME ) );
        m_hfRatio.Reset( ReadParameter( PARAMETER_HF_RATIO ) );
        m_diffusionAmount.Reset( ReadParameter( PARAMETER_DIFFUSION ) );
        m_lowCut.Reset( ReadParameter( PARAMETER_LOW_CUT ) );
        m_highCut.Reset( ReadParameter( PARAMETER_HIGH_CUT ) );
        m_earlyLevel.Reset( LevelToGain( ReadParameter( PARAMETER_EARLY_LEVEL ) ) );
        m_lateLevel.Reset( LevelToGain( ReadParameter( PARAMETER_LATE_LEVEL ) ) );
        m_width.Reset( ReadParameter( PARAMETER_WIDTH ) );
        m_modulation.Reset( ReadParameter( PARAMETER_MODULATION ) );

        m_earlyRamp.value = m_earlyRamp.end = m_earlyLevel.GetValue();
        m_lateRamp.value = m_lateRamp.end = m_lateLevel.GetValue();
        m_widthRamp.value = m_widthRamp.end = m_width.GetValue();
        m_modulationRamp.value = m_modulationRamp.end = m_modulation.GetValue();
        m_earlyRamp.step = m_lateRamp.step = m_widthRamp.step = m_modulationRamp.step = 0.0f;

        m_diffusionCoefficient = m_diffusionAmount.GetValue();
        m_earlyActive = m_hasEarly && m_earlyRamp.end != 0.0f;
        m_widthActive = m_widthRamp.end != 1.0f;
        m_modulationActive = m_modulationRamp.end > 0.0f;
        UpdateAbsorption();
        UpdateFilters();
        m_absorptionDirty = false;
    }
    else
    {
        if ( size != m_lastSize )
        {
            m_lastSize = size;
            for ( UInt32 i=0; i<m_lineCount; ++i )
                m_lineStart[ i ] = m_linePosition[ i ];
            ComputeLineTargets( size, m_lineTarget );
            m_sizeStart = m_sizeCurrent;
            m_sizeTarget = size;
            m_lineGlide = 1.0f;
        }
        if ( preDelay != m_preTarget )
        {
            m_preStart = m_preCurrent;
            m_preTarget = preDelay;
            m_preGlide = 1.0f;
        }

        m_decay.SetTarget( ReadParameter( PARAMETER_DECAY_TIME ) );
        m_hfRatio.SetTarget( ReadParameter( PARAMETER_HF_RATIO ) );
        m_diffusionAmount.SetTarget( ReadParameter( PARAMETER_DIFFUSION ) );
        m_lowCut.SetTarget( ReadParameter( PARAMETER_LOW_CUT ) );
        m_highCut.SetTarget( ReadParameter( PARAMETER_HIGH_CUT ) );
        m_earlyLevel.SetTarget( LevelToGain( ReadParameter( PARAMETER_EARLY_LEVEL ) ) );
        m_lateLevel.SetTarget( LevelToGain( ReadParameter( PARAMETER_LATE_LEVEL ) ) );
        m_width.SetTarget( ReadParameter( PARAMETER_WIDTH ) );
        m_modulation.SetTarget( ReadParameter( PARAMETER_MODULATION ) );
        m_absorptionDirty = true;
    }
    UpdateRestTaps();
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::OnReset( void )
{
    HaveParametersChanged();
    m_pre.Reset();
    m_diffusion.Reset();
    m_lines.Reset();
    m_lowCutFilter.Reset();
    m_highCutFilter.Reset();
    for ( UInt32 i=0; i<MAX_LINES; ++i )
        m_lineState[ i ] = 0.0f;
    for ( UInt32 q=0; q<2; ++q )
    {
        m_lfoSine[ q ] = 0.0;
        m_lfoCosine[ q ] = 1.0;
        m_modOffset[ q ] = 0.0f;
        m_modStep[ q ] = 0.0f;
        m_modEnd[ q ] = 0.0f;
    }
    ApplyParameters( true );
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::StartRamp( SRamp& ramp, CAudioParameterSmoother& smoother )
{
    if ( smoother.IsSmoothing() )
    {
        const Float32 next = smoother.Step();
        ramp.value = ramp.end;
        ramp.step = ( next - ramp.value ) * ( 1.0f / (Float32) GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
        ramp.end = next;
    }
    else
    {
        ramp.value = ramp.end = smoother.GetValue();
        ramp.step = 0.0f;
    }
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::StepSubBlock( void )
{
    bool absorption = m_absorptionDirty || m_lineGlide > 0.0f;
    if ( m_decay.IsSmoothing() )
    {
        m_decay.Step();
        absorption = true;
    }
    if ( m_hfRatio.IsSmoothing() )
    {
        m_hfRatio.Step();
        absorption = true;
    }
    if ( absorption )
    {
        UpdateAbsorption();
        m_absorptionDirty = false;
    }

    if ( m_diffusionAmount.IsSmoothing() )
        m_diffusionAmount.Step();
    m_diffusionCoefficient = m_diffusionAmount.GetValue();

    if ( m_lowCut.IsSmoothing() || m_highCut.IsSmoothing() )
    {
        m_lowCut.Step();
        m_highCut.Step();
        UpdateFilters();
    }

    StartRamp( m_earlyRamp, m_earlyLevel );
    StartRamp( m_lateRamp, m_lateLevel );
    StartRamp( m_widthRamp, m_width );
    StartRamp( m_modulationRamp, m_modulation );
    m_earlyActive = m_hasEarly && ( m_earlyRamp.value != 0.0f || m_earlyRamp.end != 0.0f );
    m_widthActive = m_widthRamp.value != 1.0f || m_widthRamp.end != 1.0f;
    m_modulationActive = m_modulationRamp.value > 0.0f || m_modulationRamp.end > 0.0f;

    // the modulation offsets of the two lines move linearly over the sub-block towards the value of the oscillator at its end
    for ( UInt32 q=0; q<2; ++q )
    {
        Float32 end = 0.0f;
        if ( m_modulationActive )
        {
            const Float64 sine = m_lfoSine[ q ] * m_lfoRotateCosine[ q ] + m_lfoCosine[ q ] * m_lfoRotateSine[ q ];
            const Float64 cosine = m_lfoCosine[ q ] * m_lfoRotateCosine[ q ] - m_lfoSine[ q ] * m_lfoRotateSine[ q ];
            m_lfoSine[ q ] = sine;
            m_lfoCosine[ q ] = cosine;
            end = m_modulationRamp.end * m_modulationFrames * (Float32) sine;
        }
        m_modOffset[ q ] = m_modEnd[ q ];
        m_modStep[ q ] = ( end - m_modOffset[ q ] ) * ( 1.0f / (Float32) GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
        m_modEnd[ q ] = end;
    }

    m_lowCutFilter.FlushDenormals();
    m_highCutFilter.FlushDenormals();
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::StepGlides( void )
{
    if ( m_lineGlide > 0.0f )
    {
        Float32 remaining = m_lineGlide * m_lineGlideCoefficient;
        if ( remaining < 1.0e-6f )
        {
            remaining = 0.0f;
            for ( UInt32 i=0; i<m_lineCount; ++i )
                m_linePosition[ i ] = m_lineTarget[ i ];
            m_sizeCurrent = m_sizeTarget;
        }
        else
        {
            for ( UInt32 i=0; i<m_lineCount; ++i )
                m_linePosition[ i ] = m_lineTarget[ i ] + ( m_lineStart[ i ] - m_lineTarget[ i ] ) * remaining;
            m_sizeCurrent = m_sizeTarget + ( m_sizeStart - m_sizeTarget ) * remaining;
        }
        m_lineGlide = remaining;
    }
    if ( m_preGlide > 0.0f )
    {
        Float32 remaining = m_preGlide * m_preGlideCoefficient;
        if ( remaining < 1.0e-6f )
        {
            remaining = 0.0f;
            m_preCurrent = m_preTarget;
        }
        else
            m_preCurrent = m_preTarget + ( m_preStart - m_preTarget ) * remaining;
        m_preGlide = remaining;
    }

    if ( m_hasEarly )
    {
        if ( m_lineGlide > 0.0f || m_preGlide > 0.0f )
        {
            for ( UInt32 k=0; k<EARLY_TAPS; ++k )
                m_tapDelay[ k ] = m_preCurrent + m_tapFrames[ k ] * m_sizeCurrent;
        }
        else
            UpdateRestTaps();
    }
}

/*-------------------------------------------------------------------------*/

template< UInt32 N >
void
CReverbEffect::ProcessRun( Float32* data, UInt32 run )
{
    const UInt32 channels = m_channels;
    const UInt32 activeCount = m_activeCount;
    const UInt32* active = &m_active[ 0 ];
    const Float32* leftWeight = &m_leftWeight[ 0 ];
    const Float32* rightWeight = &m_rightWeight[ 0 ];
    const Float32* signs = &m_outputSigns[ 0 ];
    const Float32 inverseActive = 1.0f / (Float32) activeCount;

    const bool lowCut = m_lowCutActive;
    const bool highCut = m_highCutActive;
    const bool early = m_earlyActive;
    const bool widthActive = m_widthActive;
    const bool modulationActive = m_modulationActive;
    const Float32 diffusion = m_diffusionCoefficient;
    const UInt32 modulated0 = m_modulatedLine[ 0 ];
    const UInt32 modulated1 = m_modulatedLine[ 1 ];

    // the input channels summed per side for the whole run, and through the low cut as one piece
    Float32 sumLeft[ GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES ];
    Float32 sumRight[ GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES ];
    for ( UInt32 f=0; f<run; ++f )
    {
        const Float32* frame = data + (size_t) f * channels;
        Float32 inLeft = 0.0f;
        Float32 inRight = 0.0f;
        for ( UInt32 a=0; a<activeCount; ++a )
        {
            const Float32 x = frame[ active[ a ] ];
            inLeft += leftWeight[ a ] * x;
            inRight += rightWeight[ a ] * x;
        }
        sumLeft[ f ] = inLeft;
        sumRight[ f ] = inRight;
    }
    if ( lowCut )
    {
        m_lowCutFilter.ProcessChannel( 0, sumLeft, sumLeft, run );
        m_lowCutFilter.ProcessChannel( 1, sumRight, sumRight, run );
    }

    Float32 lineA[ N ];
    Float32 lineB[ N ];
    Float32 lineState[ N ];
    for ( UInt32 i=0; i<N; ++i )
    {
        lineA[ i ] = m_lineA[ i ];
        lineB[ i ] = m_lineB[ i ];
        lineState[ i ] = m_lineState[ i ];
    }

    SRamp earlyRamp = m_earlyRamp;
    SRamp lateRamp = m_lateRamp;
    SRamp widthRamp = m_widthRamp;
    for ( UInt32 f=0; f<run; ++f )
    {
        Float32* frame = data + (size_t) f * channels;

        const bool linesGliding = m_lineGlide > 0.0f;
        const bool gliding = linesGliding || m_preGlide > 0.0f;
        if ( gliding )
            StepGlides();

        // input: into the history that serves the pre-delay and the early taps
        m_pre.Push( 0, sumLeft[ f ] );
        m_pre.Push( 1, sumRight[ f ] );

        // pre-delay
        Float32 left;
        Float32 right;
        if ( gliding )
        {
            const Float32 p = m_preCurrent;
            if ( p >= 1.0f )
            {
                left = m_pre.ReadCubic( 0, p );
                right = m_pre.ReadCubic( 1, p );
            }
            else
            {
                left = m_pre.ReadLinear( 0, p );
                right = m_pre.ReadLinear( 1, p );
            }
        }
        else
        {
            const UInt32 p = (UInt32) m_preCurrent;
            left = m_pre.Read( 0, p );
            right = m_pre.Read( 1, p );
        }

        // early reflections from the pre-delayed input
        Float32 earlyLeft = 0.0f;
        Float32 earlyRight = 0.0f;
        if ( early )
        {
            if ( gliding )
            {
                for ( UInt32 k=0; k<EARLY_TAPS; k+=2 )
                {
                    earlyLeft += m_tapGain[ k ] * m_pre.ReadCubic( 0, m_tapDelay[ k ] );
                    earlyRight += m_tapGain[ k + 1 ] * m_pre.ReadCubic( 1, m_tapDelay[ k + 1 ] );
                }
            }
            else
            {
                for ( UInt32 k=0; k<EARLY_TAPS; k+=2 )
                {
                    earlyLeft += m_tapGain[ k ] * m_pre.Read( 0, (UInt32) m_tapDelay[ k ] );
                    earlyRight += m_tapGain[ k + 1 ] * m_pre.Read( 1, (UInt32) m_tapDelay[ k + 1 ] );
                }
            }
        }

        // diffusion
        for ( UInt32 j=0; j<DIFFUSION_STAGES; ++j )
        {
            left = AllpassStep( m_diffusion, j, m_diffusionLength[ j ], diffusion, left );
            right = AllpassStep( m_diffusion, DIFFUSION_STAGES + j, m_diffusionLength[ DIFFUSION_STAGES + j ], diffusion, right );
        }

        // the feedback network
        const Float32 offset0 = m_modOffset[ 0 ];
        const Float32 offset1 = m_modOffset[ 1 ];
        m_modOffset[ 0 ] += m_modStep[ 0 ];
        m_modOffset[ 1 ] += m_modStep[ 1 ];

        Float32 y[ N ];
        Float32 sum = 0.0f;
        for ( UInt32 i=0; i<N; ++i )
        {
            Float32 read;
            const bool modulated = modulationActive && ( i == modulated0 || i == modulated1 );
            if ( linesGliding || modulated )
            {
                Float32 delay = m_linePosition[ i ] - 1.0f;
                if ( modulated )
                    delay += ( i == modulated0 ) ? offset0 : offset1;
                read = m_lines.ReadCubic( i, delay );
            }
            else
                read = m_lines.Read( i, (UInt32) m_linePosition[ i ] - 1 );

            Float32 absorbed = lineB[ i ] * read + lineA[ i ] * lineState[ i ];
            if ( fabsf( absorbed ) < DenormalFlushThreshold )
                absorbed = 0.0f;
            lineState[ i ] = absorbed;
            y[ i ] = absorbed;
            sum += absorbed;
        }
        const Float32 feedback = sum * ( 2.0f / (Float32) N );
        for ( UInt32 i=0; i<N; ++i )
            m_lines.Push( i, y[ i ] - feedback + m_injection[ 0 ][ i ] * left + m_injection[ 1 ][ i ] * right );

        // output
        Float32 mean = 0.0f;
        for ( UInt32 a=0; a<activeCount; ++a )
        {
            const Float32* row = signs + (size_t) a * MAX_LINES;
            Float32 late = 0.0f;
            for ( UInt32 i=0; i<N; ++i )
                late += row[ i ] * y[ i ];
            Float32 out = lateRamp.value * late;
            if ( early )
                out += earlyRamp.value * ( leftWeight[ a ] * earlyLeft + rightWeight[ a ] * earlyRight );
            frame[ active[ a ] ] = out;
            mean += out;
        }
        if ( widthActive )
        {
            mean *= inverseActive;
            for ( UInt32 a=0; a<activeCount; ++a )
            {
                Float32& out = frame[ active[ a ] ];
                out = mean + widthRamp.value * ( out - mean );
            }
        }
        earlyRamp.value += earlyRamp.step;
        lateRamp.value += lateRamp.step;
        widthRamp.value += widthRamp.step;
    }

    for ( UInt32 i=0; i<N; ++i )
        m_lineState[ i ] = lineState[ i ];

    // output high cut over the whole run
    if ( highCut )
    {
        for ( UInt32 a=0; a<activeCount; ++a )
            m_highCutFilter.ProcessChannel( active[ a ], data + active[ a ], data + active[ a ], run, channels );
    }

    m_earlyRamp = earlyRamp;
    m_lateRamp = lateRamp;
    m_widthRamp = widthRamp;
}

/*-------------------------------------------------------------------------*/

void
CReverbEffect::OnProcess( Float32* inOut, UInt32 frames )
{
    if ( HaveParametersChanged() )
        ApplyParameters( false );
    if ( 0 == m_activeCount )
        return;

    UInt64 position = GetProcessedFrameCount();
    UInt32 done = 0;
    while ( done < frames )
    {
        const UInt32 inSubBlock = (UInt32) ( position % GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES );
        if ( 0 == inSubBlock )
            StepSubBlock();
        UInt32 run = GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES - inSubBlock;
        if ( run > frames - done )
            run = frames - done;

        Float32* data = inOut + (size_t) done * m_channels;
        if ( MAX_LINES == m_lineCount )
            ProcessRun< MAX_LINES >( data, run );
        else
            ProcessRun< MAX_LINES / 2 >( data, run );

        done += run;
        position += run;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
