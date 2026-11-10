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

#ifndef GUCEF_AUDIO_CFILTERBAND_H
#define GUCEF_AUDIO_CFILTERBAND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H
#include "gucefAUDIO_CAudioParameterSmoother.h"
#define GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H
#endif /* GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The responses of an equalizer band or a filter effect. The numeric values are those of the "type" parameters, stable */
enum EFilterBandType
{
    FILTERBANDTYPE_PEAK = 0     ,
    FILTERBANDTYPE_LOWSHELF     ,
    FILTERBANDTYPE_HIGHSHELF    ,
    FILTERBANDTYPE_LOWPASS      ,
    FILTERBANDTYPE_HIGHPASS     ,
    FILTERBANDTYPE_BANDPASS     ,   /**< 0 dB at the center frequency */
    FILTERBANDTYPE_NOTCH        ,
    FILTERBANDTYPE_ALLPASS      ,

    FILTERBANDTYPE_COUNT
};
typedef enum EFilterBandType TFilterBandType;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One filter section for the audio thread, the building block of CEqualizerEffect (one per band) and CFilterEffect: a
 *  response (TFilterBandType) of 12 or 24 dB per octave with a frequency, Q and gain that move smoothly when they are
 *  changed, and that can be switched off.
 *
 *  Usage: Prepare() once on the owner thread (allocates). On the audio thread: SetTarget() with the settings whenever
 *  they may have changed, StepSubBlock() at every sub-block boundary (frames that are a multiple of
 *  GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES on the absolute frame count of the effect, so the result does not depend on how the
 *  audio is split into blocks) and ProcessRun() for the frames up to the next boundary.
 *
 *  Frequency (on log2), Q and gain (in dB) approach their targets with a one pole of 20 ms; the coefficients are
 *  recomputed on the sub-block boundaries while that goes on. A change of the response, of the slope or of the
 *  enabled state cannot be smoothed: the section crossfades over 10 ms from what it produced to what the new settings
 *  produce (a switched off section is the unfiltered signal). A new setting that comes while a crossfade runs is
 *  taken up when it has finished. A section that is off and has settled costs one branch.
 *
 *  The 24 dB slope is two sections in series. Low and high pass use the Butterworth Q values 0.5412 and 1.3066 (Q is not
 *  used); the other responses use two sections with the given Q, the gain split evenly over both.
 */
class GUCEF_AUDIO_PUBLIC_CPP CFilterBand
{
    public:

    CFilterBand( void );

    ~CFilterBand();

    /** The labels of the types, FILTERBANDTYPE_COUNT of them, in the order of the enumeration */
    static const char* const* GetTypeLabels( void );

    /** The response of the state variable filter that a type uses */
    static TFilterResponse GetResponse( TFilterBandType type );

    /** Owner thread: allocates the state for the channels. Starts switched off */
    bool Prepare( UInt32 channelCount, Float64 sampleRate );

    /** Audio thread: clears the state and takes up the settings at once, without crossfade or glide */
    void Reset( bool enabled, TFilterBandType type, bool steep, Float32 frequency, Float32 q, Float32 gainInDecibels );

    /** Audio thread: where the section is going. 'steep' is the 24 dB slope */
    void SetTarget( bool enabled, TFilterBandType type, bool steep, Float32 frequency, Float32 q, Float32 gainInDecibels );

    /** Audio thread: advances the smoothing and the crossfade decisions by one sub-block */
    void StepSubBlock( void );

    /**
     *  Audio thread: filters 'frames' frames (up to the next sub-block boundary) of interleaved audio in place.
     *  Channels whose bit is set in 'passMask' are left alone (an LFE channel)
     */
    void ProcessRun( Float32* interleaved, UInt32 frames, UInt32 passMask );

    /** True when the section changes the audio, or still is changing it (fading out) */
    bool IsActive( void ) const;

    private:

    CFilterBand( const CFilterBand& src );              /**< not implemented */
    CFilterBand& operator=( const CFilterBand& src );   /**< not implemented */

    static Int32 ConfigurationOf( bool enabled, TFilterBandType type, bool steep );
    void ApplyCoefficients( UInt32 slot );
    inline Float32 RunSlot( UInt32 slot, UInt32 channel, Float32 input );

    private:

    // two slots: one holds the settings in effect, the other what was in effect before and fades out
    CSvfFilter m_filters[ 2 ][ 2 ];             /**< [ slot ][ stage in series ] */
    Int32 m_configuration[ 2 ];                 /**< per slot: -1 for off, else type + 8 * steep */
    UInt32 m_live;                              /**< the slot with the settings in effect */
    bool m_fading;
    UInt32 m_fadeLeft;
    UInt32 m_fadeLength;

    Int32 m_wanted;                             /**< configuration asked for */
    CAudioParameterSmoother m_frequency;
    CAudioParameterSmoother m_q;
    CAudioParameterSmoother m_gain;

    UInt32 m_channels;
    Float64 m_sampleRate;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CFILTERBAND_H ? */
