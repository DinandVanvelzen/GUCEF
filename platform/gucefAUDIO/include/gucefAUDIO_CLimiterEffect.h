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

#ifndef GUCEF_AUDIO_CLIMITEREFFECT_H
#define GUCEF_AUDIO_CLIMITEREFFECT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_AUDIO_ETYPES_H
#include "gucefAUDIO_ETypes.h"
#define GUCEF_AUDIO_ETYPES_H
#endif /* GUCEF_AUDIO_ETYPES_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECT_H
#include "gucefAUDIO_CAudioEffect.h"
#define GUCEF_AUDIO_CAUDIOEFFECT_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECT_H ? */

#ifndef GUCEF_AUDIO_CDELAYLINE_H
#include "gucefAUDIO_CDelayLine.h"
#define GUCEF_AUDIO_CDELAYLINE_H
#endif /* GUCEF_AUDIO_CDELAYLINE_H ? */

#ifndef GUCEF_AUDIO_CSLIDINGMINIMUM_H
#include "gucefAUDIO_CSlidingMinimum.h"
#define GUCEF_AUDIO_CSLIDINGMINIMUM_H
#endif /* GUCEF_AUDIO_CSLIDINGMINIMUM_H ? */

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
 *  Peak limiter: keeps the signal at or below a ceiling. Registered as effect type "limiter".
 *
 *  Modes (parameter "mode", the values of TAudioLimiterMode):
 *   - off: the audio passes unchanged
 *   - instant: no delay. The gain goes down at once as far as the loudest channel of a frame needs and comes back with a
 *     one pole release. With a ceiling of 0.98 (linear) and a release of 50 ms this is, bit for bit, the limiter of the
 *     mixer master stage (see GetMixerCeilingInDecibels()). A frame can come out a rounding step above the ceiling
 *   - lookahead: a brickwall limiter. It looks GetLatencyInFrames() frames ahead, finds the strongest gain reduction in
 *     that window (CSlidingMinimum), spreads it into a smooth attack of that length (moving average) and releases with a
 *     one pole. No output sample is above the ceiling. The audio is delayed by the lookahead
 *
 *  All channels are detected together (the loudest channel of a frame decides, an LFE channel included), and all get the
 *  same gain. Changing the mode or the lookahead restarts the limiter state; the other parameters take effect without a break.
 *  The input gain is ramped linearly over the block in which it changes.
 *
 *  Process() does not allocate, lock or log. Changing the lookahead on the audio thread re-sizes the window of the sliding
 *  minimum inside the memory reserved for the longest lookahead.
 */
class GUCEF_AUDIO_PUBLIC_CPP CLimiterEffect : public CAudioEffect
{
    public:

    enum EParameter
    {
        PARAMETER_MODE = 0      ,   /**< enumeration: off, instant, lookahead */
        PARAMETER_CEILING       ,   /**< dBFS, -12 .. 0 */
        PARAMETER_LOOKAHEAD     ,   /**< ms, 0.5 .. 10 */
        PARAMETER_RELEASE       ,   /**< ms, 10 .. 1000: time constant of the one pole release */
        PARAMETER_INPUT_GAIN    ,   /**< dB, -12 .. +24, applied before detection */

        PARAMETER_COUNT
    };

    static const CString& GetStaticEffectTypeName( void );

    static UInt32 GetStaticParameterCount( void );

    static const CAudioEffectParameterInfo* GetStaticParameterInfos( void );

    /** The ceiling in dBFS whose linear value is the 0.98 of the mixer limiter: instant mode with it matches the mixer exactly */
    static Float32 GetMixerCeilingInDecibels( void );

    /**
     *  @param options  "mode" (off, instant, lookahead) sets the mode the effect starts with. Without it the mode is lookahead
     */
    explicit CLimiterEffect( const CORE::CValueList& options = CORE::CValueList() );

    virtual ~CLimiterEffect() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetEffectTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Frames of lookahead in lookahead mode (as set at the time of the call), 0 in the other modes */
    virtual UInt32 GetLatencyInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetMode( TAudioLimiterMode mode );
    TAudioLimiterMode GetMode( void ) const;
    void SetCeilingInDecibels( Float32 decibels );
    void SetLookaheadInMs( Float32 milliseconds );
    void SetReleaseInMs( Float32 milliseconds );
    void SetInputGainInDecibels( Float32 decibels );

    /** Frames in which the limiter turned the audio down, since it was made (counted per block, readable on any thread) */
    UInt64 GetLimitedFrameCount( void ) const;

    protected:

    virtual bool OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnReset( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnProcess( Float32* inOut, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CLimiterEffect( const CLimiterEffect& src );                /**< not implemented */
    CLimiterEffect& operator=( const CLimiterEffect& src );     /**< not implemented */

    void ApplyParameters( void );
    void ClearState( void );
    UInt32 LookaheadFramesFor( Float32 milliseconds ) const;
    void ProcessInstant( Float32* inOut, UInt32 frames );
    void ProcessLookahead( Float32* inOut, UInt32 frames );

    private:

    UInt32 m_channels;
    Float64 m_sampleRate;
    UInt32 m_maxLookahead;                      /**< frames reserved for the longest lookahead */

    // settings in effect, as of the last block start
    TAudioLimiterMode m_mode;
    UInt32 m_lookahead;                         /**< frames */
    Float64 m_ceilingLinear;
    Float32 m_ceilingInstant;                   /**< m_ceilingLinear as a Float32, the way the mixer limiter has it */
    Float32 m_ceilingBelow;                     /**< the largest Float32 that is not above m_ceilingLinear: nothing leaves above it */
    Float32 m_releaseInstant;                   /**< share of the way back to unity per frame, instant mode */
    Float64 m_releaseLookahead;                 /**< share of the way back to the target per frame, lookahead mode */
    Float32 m_inputGain;                        /**< linear, where the ramp is now */
    Float32 m_inputGainTarget;

    // instant mode
    Float32 m_instantGain;

    // lookahead mode
    CDelayLine m_delay;                         /**< the audio, held back by the lookahead */
    CSlidingMinimum m_minimum;                  /**< strongest reduction in the window */
    GUCEF::vector< Float32 > m_averageRing;     /**< the last lookahead + 1 minima, for the moving average */
    UInt32 m_averagePosition;
    Float64 m_averageSum;
    UInt32 m_belowUnity;                        /**< values in the ring that are below 1 */
    Float64 m_gain;                             /**< the smoothed gain, after the release */

    MT::CTAtomic< UInt64 > m_limitedFrames;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CLIMITEREFFECT_H ? */
