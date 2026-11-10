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

#ifndef GUCEF_AUDIO_CREVERBEFFECT_H
#define GUCEF_AUDIO_CREVERBEFFECT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECT_H
#include "gucefAUDIO_CAudioEffect.h"
#define GUCEF_AUDIO_CAUDIOEFFECT_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H
#include "gucefAUDIO_CAudioParameterSmoother.h"
#define GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H
#endif /* GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H ? */

#ifndef GUCEF_AUDIO_CDELAYLINE_H
#include "gucefAUDIO_CDelayLine.h"
#define GUCEF_AUDIO_CDELAYLINE_H
#endif /* GUCEF_AUDIO_CDELAYLINE_H ? */

#ifndef GUCEF_AUDIO_CSVFFILTER_H
#include "gucefAUDIO_CSvfFilter.h"
#define GUCEF_AUDIO_CSVFFILTER_H
#endif /* GUCEF_AUDIO_CSVFFILTER_H ? */

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
 *  Reverberation as a feedback delay network (J.-M. Jot and A. Chaigne, "Digital Delay Networks for Designing Artificial
 *  Reverberators", AES 90th Convention, 1991) with input diffusion after J. Dattorro ("Effect Design Part 1: Reverberator
 *  and Other Filters", JAES 45(9), 1997). Registered as effect type "reverb". What the effect gives out is the reverberation
 *  alone, no dry signal: the mix with the original is made by the effect slot (wet and dry level), usually on a send bus.
 *
 *  Signal path:
 *   1. input: the channels are summed per side (left side channels to the left input, right side channels to the right
 *      input, centre and mono channels to both at -3 dB), then the input low cut (high pass) and the pre-delay
 *   2. diffusion: four series Schroeder allpasses per input, lengths in the ratio 142 : 107 : 379 : 277 of Dattorro at
 *      29.761 kHz scaled to the sample rate, coefficient = diffusion
 *   3. early reflections: 12 taps on the pre-delayed input at fixed times between 5 and 80 ms multiplied by the size, with
 *      decaying gains, alternately for the left and the right side (not in low quality)
 *   4. late reverberation: eight delay lines (four in low quality) with prime lengths (29.7, 37.1, 41.1, 43.7, 53.3, 59.9,
 *      67.7 and 73.1 ms multiplied by the size, rounded to the nearest prime number of frames), a Householder feedback matrix
 *      and in every line an absorption filter, a first order low pass. Its gain is chosen so that the decay time is T60 at
 *      250 Hz and T60 multiplied by the high frequency ratio at 4 kHz: the gain of line i at 250 Hz is
 *      g1 = 10^( -3 Li / ( fs T60 ) ), at 4 kHz g1 multiplied by 10^( -3 Li ( 1 / hfRatio - 1 ) / ( fs T60 ) ), and the pole
 *      and the gain of the filter follow from those two. (Placing the two gains at DC and at the Nyquist frequency instead
 *      leaves the decay time at 4 kHz hardly different from that at low frequencies for the usual settings. Where the two
 *      points cannot be met by one pole without a gain above the low one at DC, the filter is matched at DC and 4 kHz.)
 *      Two of the lines are modulated slowly (0.3 and 0.7 Hz, up to 8 frames at 48 kHz) against metallic
 *      ringing. The decay time cannot be shorter than what the input diffusion rings for (about 0.2 s at the default
 *      diffusion, 0.35 s at the largest), as the diffusion feeds the network
 *   5. output: every output channel takes the eight line outputs with the signs of its own row of an 8x8 Hadamard matrix,
 *      which makes the channels decorrelated; early and late level, width ( out' = mean + width ( out - mean ), for two
 *      channels L' = L ( 1 + w ) / 2 + R ( 1 - w ) / 2 ) and the output high cut (low pass)
 *
 *  A channel flagged as low frequency effects in the channel layout passes through unchanged (the slot mixes the dry signal).
 *
 *  A change of the size moves the delay line read positions: they glide with a one pole of 200 ms (fractional reads, a short
 *  pitch change by design), so the effect is not meant for continuous size modulation. The pre-delay glides in 100 ms, the
 *  other parameters in 20 ms. The early and late level are silence at their minimum of -60 dB.
 *
 *  Construction option "quality": "normal" (default) or "low" (four delay lines and no early reflections; for drivers that
 *  render on a main thread and for low end devices).
 *
 *  The tail is the pre-delay plus ceil( 1.5 T60 fs ) frames, or where the input diffusion rings longer (short decay times,
 *  high diffusion) the time that takes to fall by 90 dB. No latency.
 *
 *  Nothing is allocated after Prepare(), which reserves the memory for the largest size and pre-delay. The feedback state
 *  is flushed to zero below 1e-15, so the tail never reaches the denormal range.
 */
class GUCEF_AUDIO_PUBLIC_CPP CReverbEffect : public CAudioEffect
{
    public:

    enum EParameter
    {
        PARAMETER_PRE_DELAY = 0 ,   /**< ms, 0 .. 500 */
        PARAMETER_SIZE          ,   /**< 0.1 .. 1, scales the delay line lengths and the early reflection times */
        PARAMETER_DECAY_TIME    ,   /**< seconds, 0.1 .. 20, the T60 at low frequencies */
        PARAMETER_HF_RATIO      ,   /**< 0.1 .. 1, decay time at 4 kHz as a fraction of the decay time at low frequencies */
        PARAMETER_DIFFUSION     ,   /**< 0 .. 0.75, coefficient of the input allpasses */
        PARAMETER_EARLY_LEVEL   ,   /**< dB, -60 .. 0 (-60 is silence) */
        PARAMETER_LATE_LEVEL    ,   /**< dB, -60 .. 0 (-60 is silence) */
        PARAMETER_LOW_CUT       ,   /**< Hz, 20 .. 1000, high pass on the input (20 is off) */
        PARAMETER_HIGH_CUT      ,   /**< Hz, 1000 .. 20000, low pass on the output (20000 is off) */
        PARAMETER_WIDTH         ,   /**< 0 .. 1, 0 gives the same signal on all channels */
        PARAMETER_MODULATION    ,   /**< 0 .. 1, depth of the modulation of two delay lines */

        PARAMETER_COUNT
    };

    enum EQuality
    {
        QUALITY_LOW = 0 ,           /**< four delay lines, no early reflections */
        QUALITY_NORMAL              /**< eight delay lines and early reflections */
    };

    static const CString& GetStaticEffectTypeName( void );

    static UInt32 GetStaticParameterCount( void );

    static const CAudioEffectParameterInfo* GetStaticParameterInfos( void );

    /**
     *  @param options  "quality": "low" or "normal"
     */
    explicit CReverbEffect( const CORE::CValueList& options = CORE::CValueList() );

    virtual ~CReverbEffect() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetEffectTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetTailInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    EQuality GetQuality( void ) const;

    /** Number of delay lines of the feedback network: 8, or 4 in low quality */
    UInt32 GetLineCount( void ) const;

    /** Adds the option "quality" ("low" or "normal") */
    virtual void GetConstructionOptions( CORE::CValueList& options ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetPreDelayInMs( Float32 milliseconds );
    void SetSize( Float32 size );
    void SetDecayTimeInSeconds( Float32 seconds );
    void SetHfRatio( Float32 ratio );
    void SetDiffusion( Float32 diffusion );
    void SetEarlyLevelInDecibels( Float32 decibels );
    void SetLateLevelInDecibels( Float32 decibels );
    void SetLowCutInHertz( Float32 hertz );
    void SetHighCutInHertz( Float32 hertz );
    void SetWidth( Float32 width );
    void SetModulationDepth( Float32 depth );

    protected:

    virtual bool OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnReset( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnProcess( Float32* inOut, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CReverbEffect( const CReverbEffect& src );              /**< not implemented */
    CReverbEffect& operator=( const CReverbEffect& src );   /**< not implemented */

    enum
    {
        MAX_LINES = 8           ,
        EARLY_TAPS = 12         ,
        DIFFUSION_STAGES = 4
    };

    /** A gain that moves linearly over one sub-block */
    struct SRamp
    {
        Float32 value;
        Float32 step;
        Float32 end;
    };

    void ApplyParameters( bool jump );
    void StepSubBlock( void );
    void StartRamp( SRamp& ramp, CAudioParameterSmoother& smoother );
    void UpdateAbsorption( void );
    void UpdateFilters( void );
    void ComputeLineTargets( Float32 size, Float32* targets ) const;
    void UpdateRestTaps( void );
    void StepGlides( void );

    template< UInt32 N >
    void ProcessRun( Float32* data, UInt32 run );

    private:

    EQuality m_quality;
    UInt32 m_lineCount;
    UInt32 m_channels;
    Float64 m_sampleRate;

    // channels: the non-LFE ones, with their weights towards the left and right input and the row of their output signs
    GUCEF::vector< UInt32 > m_active;
    GUCEF::vector< Float32 > m_leftWeight;
    GUCEF::vector< Float32 > m_rightWeight;
    GUCEF::vector< Float32 > m_outputSigns;     /**< per active channel MAX_LINES signs, scaled for unit energy */
    UInt32 m_activeCount;

    // dsp state
    CDelayLine m_pre;                           /**< two channels: the input after the low cut, for the pre-delay and the early taps */
    CDelayLine m_diffusion;                     /**< 2 x DIFFUSION_STAGES allpass rings */
    CDelayLine m_lines;                         /**< the delay lines of the feedback network */
    CSvfFilter m_lowCutFilter;                  /**< two channels, high pass */
    CSvfFilter m_highCutFilter;                 /**< every channel, low pass */
    bool m_lowCutActive;
    bool m_highCutActive;
    UInt32 m_diffusionLength[ 2 * DIFFUSION_STAGES ];
    Float32 m_injection[ 2 ][ MAX_LINES ];
    UInt32 m_modulatedLine[ 2 ];

    // feedback network
    Float32 m_linePosition[ MAX_LINES ];        /**< current length in frames, a prime number at rest */
    Float32 m_lineStart[ MAX_LINES ];
    Float32 m_lineTarget[ MAX_LINES ];
    Float32 m_lineA[ MAX_LINES ];               /**< absorption filter pole */
    Float32 m_lineB[ MAX_LINES ];               /**< absorption filter input gain */
    Float32 m_lineState[ MAX_LINES ];
    Float32 m_lineGlide;                        /**< remaining part of the glide of the size, 1 .. 0 */
    Float32 m_lineGlideCoefficient;
    Float32 m_lastSize;
    Float32 m_sizeStart;
    Float32 m_sizeTarget;
    Float32 m_sizeCurrent;
    bool m_absorptionDirty;
    Float64 m_cosLowReference;                  /**< cosine of the angular frequency of the low frequency reference (250 Hz) */
    Float64 m_cosReference;                     /**< cosine of the angular frequency of the high frequency reference (4 kHz) */

    // pre-delay and early reflections
    Float32 m_preStart;
    Float32 m_preTarget;
    Float32 m_preCurrent;
    Float32 m_preGlide;
    Float32 m_preGlideCoefficient;
    Float32 m_tapFrames[ EARLY_TAPS ];          /**< time in frames at size 1 */
    Float32 m_tapGain[ EARLY_TAPS ];
    Float32 m_tapDelay[ EARLY_TAPS ];           /**< current time in frames from the pre-delay input */
    bool m_hasEarly;

    // modulation
    Float64 m_lfoSine[ 2 ];
    Float64 m_lfoCosine[ 2 ];
    Float64 m_lfoRotateSine[ 2 ];               /**< rotation of the oscillators over one sub-block */
    Float64 m_lfoRotateCosine[ 2 ];
    Float32 m_modOffset[ 2 ];                   /**< current offset of the read position of the modulated lines, in frames */
    Float32 m_modStep[ 2 ];                     /**< change of the offset per frame */
    Float32 m_modEnd[ 2 ];                      /**< offset at the end of the sub-block */
    Float32 m_modulationFrames;                 /**< excursion at depth 1 */

    // parameters
    CAudioParameterSmoother m_decay;
    CAudioParameterSmoother m_hfRatio;
    CAudioParameterSmoother m_diffusionAmount;
    CAudioParameterSmoother m_lowCut;
    CAudioParameterSmoother m_highCut;
    CAudioParameterSmoother m_earlyLevel;       /**< linear gain */
    CAudioParameterSmoother m_lateLevel;        /**< linear gain */
    CAudioParameterSmoother m_width;
    CAudioParameterSmoother m_modulation;
    SRamp m_earlyRamp;
    SRamp m_lateRamp;
    SRamp m_widthRamp;
    SRamp m_modulationRamp;
    Float32 m_diffusionCoefficient;
    bool m_earlyActive;
    bool m_widthActive;
    bool m_modulationActive;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CREVERBEFFECT_H ? */
