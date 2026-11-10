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

#ifndef GUCEF_AUDIO_CFILTEREFFECT_H
#define GUCEF_AUDIO_CFILTEREFFECT_H

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
 *  One filter with a choice of response and slope: low pass, high pass, band pass, notch, all pass, peak and the shelves.
 *  Registered as effect type "filter". Covers the low pass, high pass, band pass, notch, low shelf, high shelf and band
 *  limit filters of other engines. It is one CFilterBand for all channels.
 *
 *  Parameters: "type" (peak, lowShelf, highShelf, lowPass, highPass, bandPass, notch, allPass; default lowPass),
 *  "frequency" (20 to 20000 Hz), "q" (0.1 to 18), "gain" (dB, for peak and the shelves) and "slope" (12 or 24 dB per
 *  octave). The 24 dB slope is two sections in series: a fourth order Butterworth response for low and high pass (the Q
 *  parameter is not used there, the response is -3 dB at the frequency), for the other types two sections with the given Q
 *  and half of the gain each, which makes the transition steeper.
 *
 *  Frequency, Q and gain glide (20 ms); a change of the type or of the slope crossfades in 10 ms. No latency, no tail.
 */
class GUCEF_AUDIO_PUBLIC_CPP CFilterEffect : public CAudioEffect
{
    public:

    enum EParameter
    {
        PARAMETER_TYPE = 0      ,   /**< enumeration (TFilterBandType) */
        PARAMETER_FREQUENCY     ,   /**< Hz, 20 .. 20000 */
        PARAMETER_Q             ,   /**< 0.1 .. 18 */
        PARAMETER_GAIN          ,   /**< dB, -24 .. +24 */
        PARAMETER_SLOPE         ,   /**< dB per octave: 12 or 24 (an integer, 18 and up is 24) */

        PARAMETER_COUNT
    };

    static const CString& GetStaticEffectTypeName( void );

    static UInt32 GetStaticParameterCount( void );

    static const CAudioEffectParameterInfo* GetStaticParameterInfos( void );

    explicit CFilterEffect( const CORE::CValueList& options = CORE::CValueList() );

    virtual ~CFilterEffect() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetEffectTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetFilterType( TFilterBandType type );
    void SetFrequency( Float32 hertz );
    void SetQ( Float32 q );
    void SetGainInDecibels( Float32 decibels );
    void SetSlopeInDecibelsPerOctave( UInt32 slope );

    protected:

    virtual bool OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnReset( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnProcess( Float32* inOut, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CFilterEffect( const CFilterEffect& src );                  /**< not implemented */
    CFilterEffect& operator=( const CFilterEffect& src );       /**< not implemented */

    void ApplyParameters( bool jump );

    private:

    UInt32 m_channels;
    CFilterBand m_band;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CFILTEREFFECT_H ? */
