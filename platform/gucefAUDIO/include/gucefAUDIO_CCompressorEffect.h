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

#ifndef GUCEF_AUDIO_CCOMPRESSOREFFECT_H
#define GUCEF_AUDIO_CCOMPRESSOREFFECT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_AUDIO_CATOMICFLOAT_H
#include "gucefAUDIO_CAtomicFloat.h"
#define GUCEF_AUDIO_CATOMICFLOAT_H
#endif /* GUCEF_AUDIO_CATOMICFLOAT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECT_H
#include "gucefAUDIO_CAudioEffect.h"
#define GUCEF_AUDIO_CAUDIOEFFECT_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H
#include "gucefAUDIO_CAudioParameterSmoother.h"
#define GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H
#endif /* GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Feed forward dynamic range compressor in the log domain, after D. Giannoulis, M. Massberg and J. D. Reiss, "Digital
 *  Dynamic Range Compressor Design - A Tutorial and Analysis", JAES 60(6), 2012, with the smooth branching detector
 *  placed after the gain computer. Registered as effect type "compressor".
 *
 *  Per frame and channel the level is the absolute sample (detector "peak") or the root mean square over a window
 *  (detector "rms"). The gain computer turns the level x in dB into an amount of gain reduction with threshold T, ratio R
 *  and knee width W:
 *   - 2( x - T ) < -W: none
 *   - 2| x - T | <= W: ( 1 - 1/R ) ( x - T + W/2 )^2 / ( 2W )
 *   - 2( x - T ) > W: ( 1 - 1/R ) ( x - T )
 *  (a knee of 0 is a hard knee). Logarithm and exponent are only computed for a level above T - W/2; below it the gain
 *  reduction is exactly 0 dB, so a signal that stays below it passes bit for bit (with a makeup gain of 0 dB).
 *  The reduction is smoothed with the attack time (more reduction) and the release time (less reduction) as one pole
 *  time constants, and the makeup gain is added.
 *
 *  Stereo link: the reduction of every channel moves from its own value (link 0) to the strongest reduction of all
 *  channels (link 1), so a linked compressor does not shift the image. All channels take part, an LFE channel included.
 *
 *  Threshold, ratio and knee are smoothed (20 ms one pole, per sub-block), the makeup gain is a linear ramp over the block
 *  in which it changes. No latency, no tail.
 */
class GUCEF_AUDIO_PUBLIC_CPP CCompressorEffect : public CAudioEffect
{
    public:

    enum EParameter
    {
        PARAMETER_THRESHOLD = 0 ,   /**< dB, -60 .. 0 */
        PARAMETER_RATIO         ,   /**< 1 .. 20 (20 is limiting) */
        PARAMETER_KNEE          ,   /**< dB, 0 .. 24 */
        PARAMETER_ATTACK        ,   /**< ms, 0.1 .. 200 */
        PARAMETER_RELEASE       ,   /**< ms, 10 .. 3000 */
        PARAMETER_MAKEUP        ,   /**< dB, 0 .. 24 */
        PARAMETER_DETECTOR      ,   /**< enumeration: peak, rms (the values of TEnvelopeMode) */
        PARAMETER_RMS_WINDOW    ,   /**< ms, 1 .. 300 */
        PARAMETER_STEREO_LINK   ,   /**< 0 .. 1 */

        PARAMETER_COUNT
    };

    static const CString& GetStaticEffectTypeName( void );

    static UInt32 GetStaticParameterCount( void );

    static const CAudioEffectParameterInfo* GetStaticParameterInfos( void );

    explicit CCompressorEffect( const CORE::CValueList& options = CORE::CValueList() );

    virtual ~CCompressorEffect() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetEffectTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetThresholdInDecibels( Float32 decibels );
    void SetRatio( Float32 ratio );
    void SetKneeInDecibels( Float32 decibels );
    void SetAttackInMs( Float32 milliseconds );
    void SetReleaseInMs( Float32 milliseconds );
    void SetMakeupInDecibels( Float32 decibels );
    void SetDetector( TEnvelopeMode mode );
    void SetRmsWindowInMs( Float32 milliseconds );
    void SetStereoLink( Float32 link );

    /** Any thread: the strongest gain reduction in dB (a positive number) of the last block, makeup gain not included. For meters */
    Float32 GetGainReductionInDecibels( void ) const;

    protected:

    virtual bool OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnReset( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnProcess( Float32* inOut, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CCompressorEffect( const CCompressorEffect& src );              /**< not implemented */
    CCompressorEffect& operator=( const CCompressorEffect& src );   /**< not implemented */

    void ReleaseFollowers( void );
    void ApplyParameters( bool jump );
    void StepSubBlock( void );
    void UpdateCurve( void );
    Float32 ReductionFor( Float32 level ) const;

    private:

    UInt32 m_channels;
    Float64 m_sampleRate;

    // parameters in effect
    CAudioParameterSmoother m_threshold;
    CAudioParameterSmoother m_ratio;
    CAudioParameterSmoother m_knee;
    Float64 m_attackCoefficient;
    Float64 m_releaseCoefficient;
    Float32 m_makeupGain;                       /**< linear, where the ramp is now */
    Float32 m_makeupTarget;
    TEnvelopeMode m_detector;
    Float32 m_rmsWindow;
    Float32 m_link;

    // the curve at the current threshold, ratio and knee
    Float32 m_curveThreshold;
    Float32 m_curveKnee;
    Float32 m_curveSlope;                       /**< 1 - 1 / ratio */
    Float32 m_curveFloor;                       /**< linear level below which there is no reduction: T - W/2 */

    // state per channel
    GUCEF::vector< CEnvelopeFollower* > m_followers;
    GUCEF::vector< Float64 > m_smoothed;        /**< smoothed gain reduction in dB */
    GUCEF::vector< Float32 > m_reduction;       /**< scratch: gain reduction of the current frame */

    CAtomicFloat m_gainReduction;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CCOMPRESSOREFFECT_H ? */
