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

#ifndef GUCEF_AUDIO_CEQUALIZEREFFECT_H
#define GUCEF_AUDIO_CEQUALIZEREFFECT_H

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

#ifndef GUCEF_AUDIO_CFILTERBAND_H
#include "gucefAUDIO_CFilterBand.h"
#define GUCEF_AUDIO_CFILTERBAND_H
#endif /* GUCEF_AUDIO_CFILTERBAND_H ? */

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
 *  Parametric equalizer: a number of bands in series, every band one CFilterBand (a 12 dB per octave state variable filter
 *  with a response, frequency, Q and gain that moves smoothly, and an enabled switch). Registered as effect type "equalizer".
 *
 *  The number of bands is fixed when the effect is made (construction option "bands", 1 to 16, default 8). Parameter ids
 *  are band * 5 + field (see GetParameterId()), the names are "band0Enabled", "band0Type", "band0Frequency", "band0Gain"
 *  and "band0Q" for band 0, and so on. The default bands are a low shelf at 80 Hz, peaks at 200, 500, 1000, 2000 and 5000 Hz,
 *  a high shelf at 10 kHz and a high pass at 20 Hz that is switched off, all at 0 dB. Bands beyond the eighth start
 *  switched off.
 *
 *  Construction option "preset" makes a graphic equalizer instead: "graphic6", "graphic10" or "graphic21" (6, 10 or
 *  21 peaking bands on the usual center frequencies, all at 0 dB, with a Q that makes neighbours overlap).
 *
 *  A channel flagged as low frequency effects in the channel layout passes through unchanged. A band that is switched off
 *  costs one branch. With all gains at 0 dB the audio comes out bit for bit as it went in. No latency, no tail.
 */
class GUCEF_AUDIO_PUBLIC_CPP CEqualizerEffect : public CAudioEffect
{
    public:

    enum EBandField
    {
        BANDFIELD_ENABLED = 0   ,   /**< boolean */
        BANDFIELD_TYPE          ,   /**< enumeration: peak, lowShelf, highShelf, lowPass, highPass, bandPass, notch, allPass (TFilterBandType) */
        BANDFIELD_FREQUENCY     ,   /**< Hz, 20 .. 20000 */
        BANDFIELD_GAIN          ,   /**< dB, -24 .. +24: for peak and shelves */
        BANDFIELD_Q             ,   /**< 0.1 .. 18 */

        BANDFIELD_COUNT
    };

    /** Most bands an equalizer made with the "bands" option has */
    static const UInt32 MaxBands = 16;

    /** Most bands any equalizer has (the graphic preset with 21 bands) */
    static const UInt32 MaxBandsOfPreset = 21;

    static const UInt32 DefaultBands = 8;

    static const CString& GetStaticEffectTypeName( void );

    /** Parameters of an equalizer with the default number of bands */
    static UInt32 GetStaticParameterCount( void );

    /** Descriptors for the largest equalizer: an equalizer with fewer bands uses the first ones */
    static const CAudioEffectParameterInfo* GetStaticParameterInfos( void );

    static UInt32 GetParameterId( UInt32 band, EBandField field );

    /**
     *  @param options  "bands" (1 to 16) and "preset" (graphic6, graphic10, graphic21)
     */
    explicit CEqualizerEffect( const CORE::CValueList& options = CORE::CValueList() );

    virtual ~CEqualizerEffect() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetEffectTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    UInt32 GetBandCount( void ) const;

    /** Adds the option "bands", the number of bands this instance has */
    virtual void GetConstructionOptions( CORE::CValueList& options ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Any thread: sets all the fields of a band, published together */
    void SetBand( UInt32 band, bool enabled, TFilterBandType type, Float32 frequency, Float32 gainInDecibels, Float32 q );

    protected:

    virtual bool OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnReset( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnProcess( Float32* inOut, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CEqualizerEffect( const CEqualizerEffect& src );                /**< not implemented */
    CEqualizerEffect& operator=( const CEqualizerEffect& src );     /**< not implemented */

    void ReleaseBands( void );
    void ApplyParameters( bool jump );

    private:

    UInt32 m_bandCount;
    UInt32 m_channels;
    UInt32 m_passMask;                      /**< channels that are not equalized */
    GUCEF::vector< CFilterBand* > m_bands;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CEQUALIZEREFFECT_H ? */
