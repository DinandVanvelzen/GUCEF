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

#ifndef GUCEF_AUDIO_CNOISESUPPRESSOREFFECT_H
#define GUCEF_AUDIO_CNOISESUPPRESSOREFFECT_H

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

#ifndef GUCEF_AUDIO_CATOMICFLOAT_H
#include "gucefAUDIO_CAtomicFloat.h"
#define GUCEF_AUDIO_CATOMICFLOAT_H
#endif /* GUCEF_AUDIO_CATOMICFLOAT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECT_H
#include "gucefAUDIO_CAudioEffect.h"
#define GUCEF_AUDIO_CAUDIOEFFECT_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECT_H ? */

#ifndef GUCEF_AUDIO_CSPECTRALSUPPRESSOR_H
#include "gucefAUDIO_CSpectralSuppressor.h"
#define GUCEF_AUDIO_CSPECTRALSUPPRESSOR_H
#endif /* GUCEF_AUDIO_CSPECTRALSUPPRESSOR_H ? */

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
 *  Noise suppressor: lowers stationary background noise (fans, hum of a room, the hiss of a microphone) under speech by
 *  spectral suppression (CSpectralSuppressor: short-time Fourier transform with a square root Hann window and half
 *  overlap, noise by minimum statistics, decision-directed Wiener gain with a floor, smoothed over frequency and time).
 *  Registered as effect type "noiseSuppressor" (the registry ignores case). Meant for the post-echo stage of the capture
 *  path, at a processing rate of 16 kHz on small units; it works on any channel count (each channel on its own) and at
 *  8000 to 192000 Hz (other rates pass the audio unchanged).
 *
 *  Parameters:
 *   - amount: dB, -40 .. 0, default -15: the deepest attenuation of noise (the gain floor). Deeper floors remove more
 *     noise and risk "musical" artefacts; 0 dB suppresses nothing, then no transform is computed at all (the audio is
 *     only delayed by the latency, so a change of the amount never moves the audio in time)
 *   - strength: 0.5 .. 4, default 1: factor on the noise estimate (over-subtraction); above 1 suppresses harder at the
 *     price of the quiet parts of speech
 *   - bypassBelowHz: Hz, 0 .. 1000, default 100: bins below this frequency are not touched (the high pass of the
 *     pre-echo stage handles them)
 *
 *  Latency: GetLatencyInFrames() = the frame of the transform: 256 frames at 44.1 and 48 kHz (5.3 ms), 128 at 16 to
 *  24 kHz (8 ms at 16 kHz), 64 at 8 kHz (8 ms). The output does not depend on how the audio is cut into blocks; digital
 *  silence passes as silence without a transform. Switched off in its slot (CAudioEffectChain) the effect does not run
 *  and the audio passes bit for bit. Allocates only in Prepare().
 */
class GUCEF_AUDIO_PUBLIC_CPP CNoiseSuppressorEffect : public CAudioEffect
{
    public:

    enum EParameter
    {
        PARAMETER_AMOUNT = 0        ,   /**< dB, -40 .. 0: the gain floor */
        PARAMETER_STRENGTH          ,   /**< 0.5 .. 4: factor on the noise estimate */
        PARAMETER_BYPASS_BELOW_HZ   ,   /**< Hz, 0 .. 1000: bins below are not touched */

        PARAMETER_COUNT
    };

    static const CString& GetStaticEffectTypeName( void );

    static UInt32 GetStaticParameterCount( void );

    static const CAudioEffectParameterInfo* GetStaticParameterInfos( void );

    /** No construction options */
    explicit CNoiseSuppressorEffect( const CORE::CValueList& options = CORE::CValueList() );

    virtual ~CNoiseSuppressorEffect() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetEffectTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetAmountInDecibels( Float32 decibels );
    void SetStrength( Float32 strength );
    void SetBypassBelowHz( Float32 frequency );

    /** The frame of the transform: what the output is delayed by. 0 before Prepare() and at a rate that is not supported */
    virtual UInt32 GetLatencyInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Any thread: the gain of the last transformed frame of the first channel over all bins, power weighted, in dB */
    Float32 GetGainInDecibels( void ) const;

    /** Any thread: frames of the transform (each two FFTs) and frames passed without one, over all channels, since Prepare() */
    UInt64 GetTransformedFrameCount( void ) const;
    UInt64 GetBypassedFrameCount( void ) const;

    /** Owner, while the effect is not processed: the suppressor of a channel, for analysis. Null for another channel */
    const CSpectralSuppressor* GetSuppressor( UInt32 channel ) const;

    protected:

    virtual bool OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnReset( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnProcess( Float32* inOut, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CNoiseSuppressorEffect( const CNoiseSuppressorEffect& src );              /**< not implemented */
    CNoiseSuppressorEffect& operator=( const CNoiseSuppressorEffect& src );   /**< not implemented */

    void ApplyParameters( void );
    void ReleaseSuppressors( void );
    void Publish( void );

    private:

    GUCEF::vector< CSpectralSuppressor* > m_suppressors;   /**< one per channel */
    GUCEF::vector< Float32 > m_channel;                     /**< one channel of a block, for more than one channel */
    UInt32 m_channels;
    UInt32 m_latency;

    // any thread
    CAtomicFloat m_publishedGainDb;
    MT::CTAtomic< UInt64 > m_publishedTransformed;
    MT::CTAtomic< UInt64 > m_publishedBypassed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CNOISESUPPRESSOREFFECT_H ? */
