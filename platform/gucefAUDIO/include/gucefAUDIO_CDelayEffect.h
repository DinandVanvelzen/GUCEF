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

#ifndef GUCEF_AUDIO_CDELAYEFFECT_H
#define GUCEF_AUDIO_CDELAYEFFECT_H

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
 *  Echo with feedback. Registered as effect type "delay".
 *
 *  Every channel has a delay line (CDelayLine, reads with cubic Hermite interpolation). What the effect gives out is the
 *  delayed signal alone, no dry signal: the mix with the original is made by the effect slot (wet and dry level). The echo
 *  returns to the input of the line through the feedback path: high pass (low cut) and low pass (high cut) filters of
 *  12 dB per octave and then the feedback gain. A low cut at its lowest value (20 Hz) and a high cut at its highest
 *  (20000 Hz) are switched off, not filtered.
 *
 *  Ping-pong sends the feedback of the first channel to the second and the other way round (stereo cross feedback), so
 *  the echoes alternate between left and right. It applies to the first two channels.
 *
 *  Stereo offset adds a delay to the channels with an odd index (the right channel). The delay time and the stereo offset
 *  glide with a one pole of 100 ms on the read position, which is a short pitch change, by design; feedback and the cut
 *  frequencies glide in 20 ms. A channel flagged as low frequency effects in the channel layout passes through unchanged.
 *
 *  Construction option "maxDelay" (milliseconds, default 2000, at most 10000) sets the memory that is reserved. A delay time
 *  longer than that is limited to it.
 *
 *  The tail is the delay time (plus the stereo offset) when there is no feedback, GUCEF_AUDIO_TAIL_INFINITE otherwise.
 *  No latency.
 */
class GUCEF_AUDIO_PUBLIC_CPP CDelayEffect : public CAudioEffect
{
    public:

    enum EParameter
    {
        PARAMETER_TIME = 0      ,   /**< ms, 1 .. 10000 (limited to the maximum delay of the effect) */
        PARAMETER_FEEDBACK      ,   /**< -0.98 .. 0.98 */
        PARAMETER_PING_PONG     ,   /**< boolean */
        PARAMETER_LOW_CUT       ,   /**< Hz, 20 .. 2000, high pass in the feedback path */
        PARAMETER_HIGH_CUT      ,   /**< Hz, 1000 .. 20000, low pass in the feedback path */
        PARAMETER_STEREO_OFFSET ,   /**< ms, 0 .. 50 */

        PARAMETER_COUNT
    };

    static const CString& GetStaticEffectTypeName( void );

    static UInt32 GetStaticParameterCount( void );

    static const CAudioEffectParameterInfo* GetStaticParameterInfos( void );

    /**
     *  @param options  "maxDelay" in milliseconds
     */
    explicit CDelayEffect( const CORE::CValueList& options = CORE::CValueList() );

    virtual ~CDelayEffect() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetEffectTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetTailInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The longest delay time that this effect can have, in milliseconds */
    Float32 GetMaxDelayInMs( void ) const;

    /** Adds the option "maxDelay" (milliseconds), the longest delay this instance can have */
    virtual void GetConstructionOptions( CORE::CValueList& options ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetTimeInMs( Float32 milliseconds );
    void SetFeedback( Float32 feedback );
    void SetPingPong( bool pingPong );
    void SetLowCutInHertz( Float32 hertz );
    void SetHighCutInHertz( Float32 hertz );
    void SetStereoOffsetInMs( Float32 milliseconds );

    protected:

    virtual bool OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnReset( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnProcess( Float32* inOut, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDelayEffect( const CDelayEffect& src );                /**< not implemented */
    CDelayEffect& operator=( const CDelayEffect& src );     /**< not implemented */

    void ApplyParameters( bool jump );
    void StepSubBlock( void );
    void UpdateFilters( void );
    Float32 TimeInFrames( Float32 milliseconds ) const;

    private:

    Float32 m_maxDelayMs;
    UInt32 m_channels;
    UInt32 m_passMask;                          /**< channels that pass unchanged */
    Float64 m_sampleRate;

    CDelayLine m_delay;
    CSvfFilter m_lowCutFilter;                  /**< high pass */
    CSvfFilter m_highCutFilter;                 /**< low pass */
    bool m_lowCutActive;
    bool m_highCutActive;
    bool m_pingPong;

    CAudioParameterSmoother m_time;             /**< frames */
    CAudioParameterSmoother m_offset;           /**< frames */
    CAudioParameterSmoother m_feedback;
    CAudioParameterSmoother m_lowCut;
    CAudioParameterSmoother m_highCut;

    GUCEF::vector< Float32 > m_taps;            /**< scratch: the echo of every channel in the current frame */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CDELAYEFFECT_H ? */
