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

#ifndef GUCEF_AUDIO_CVOICEACTIVITYDETECTOR_H
#define GUCEF_AUDIO_CVOICEACTIVITYDETECTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

#ifndef GUCEF_AUDIO_CATOMICFLOAT_H
#include "gucefAUDIO_CAtomicFloat.h"
#define GUCEF_AUDIO_CATOMICFLOAT_H
#endif /* GUCEF_AUDIO_CATOMICFLOAT_H ? */

#ifndef GUCEF_AUDIO_CSLIDINGMINIMUM_H
#include "gucefAUDIO_CSlidingMinimum.h"
#define GUCEF_AUDIO_CSLIDINGMINIMUM_H
#endif /* GUCEF_AUDIO_CSLIDINGMINIMUM_H ? */

#ifndef GUCEF_AUDIO_CSVFFILTER_H
#include "gucefAUDIO_CSvfFilter.h"
#define GUCEF_AUDIO_CSVFFILTER_H
#endif /* GUCEF_AUDIO_CSVFFILTER_H ? */

#ifndef GUCEF_AUDIO_CVOICEACTIVITYSETTINGS_H
#include "gucefAUDIO_CVoiceActivitySettings.h"
#define GUCEF_AUDIO_CVOICEACTIVITYSETTINGS_H
#endif /* GUCEF_AUDIO_CVOICEACTIVITYSETTINGS_H ? */

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
 *  Tells speech from the background in a mono stream, from the energy of the voice band. It is energy based: it separates
 *  a signal that rises above the noise that was there from a steady background, it does not recognise speech. Steady
 *  noise, hum and short clicks do not trigger it; a loud, steady, speech-like tone does, until the floor has followed it.
 *
 *  The audio is cut into analysis frames of 10 ms, aligned to the absolute frame count of the stream (frame k covers the
 *  frames k * rate / 100 up to ( k + 1 ) * rate / 100, rounded down), so the decisions do not depend on how the audio
 *  is cut into blocks: the same stream gives bit for bit the same decisions and levels for any block sizes. For each frame:
 *   - the energy is the mean square of the audio after a 12 dB per octave high pass at 200 Hz and low pass at
 *     4000 Hz (Butterworth, CSvfFilter; the low pass is kept below 0.45 times the rate), in dB re a full scale RMS of
 *     1.0 (a sine of amplitude 1 reads -3 dB); a frame that is silent reads -120 dB
 *   - the noise floor follows the minimum of the energy smoothed over 50 ms (one pole) in a sliding window of 1.5 s
 *     (CSlidingMinimum): it falls at once when that minimum falls and rises at most as fast as the setting allows
 *     (3 dB per second by default), the simplest form of minimum statistics (R. Martin). It is never below the
 *     minimum level of the settings
 *   - onset: the energy exceeds the floor by the onset margin in two consecutive frames. While active, the energy
 *     above the floor by the release margin keeps the voice active; below it the voice lingers for the hangover time
 *     and then ends. A frame below the minimum level ends the voice at once
 *  The decision follows the last completed frame, so it is up to 10 ms behind the audio, and the onset takes two
 *  frames: a burst of speech is seen 10 to 30 ms after it started.
 *
 *  Prepare() on the owner thread (allocates). Process() on one thread; it never allocates, locks or logs, and flushes
 *  the denormals of its filters at every frame, so it can run on a real time thread whatever the state of the floating
 *  point unit. SetSettings() acts on the next frame but is not guarded against a thread that processes: call it while
 *  nothing processes. IsActive(), GetEnergyInDecibels(), GetNoiseFloorInDecibels() and GetAnalyzedFrameCount() may be
 *  read from any thread (atomics: the values of the last completed frame).
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceActivityDetector
{
    public:

    CVoiceActivityDetector( void );

    ~CVoiceActivityDetector();

    /** The length of an analysis frame in milliseconds (10) */
    static const Float32 AnalysisFrameInMs;

    /** The window over which the minimum of the energy is taken, in milliseconds (1500) */
    static const Float32 FloorWindowInMs;

    /** Owner thread: makes the state for a stream at 'sampleRate' (8000 to 192000 Hz) and clears it. The settings stay. False for another rate */
    bool Prepare( UInt32 sampleRate );

    /** Owner thread: forgets the stream. The settings stay */
    void Release( void );

    bool IsPrepared( void ) const;

    UInt32 GetSampleRate( void ) const;

    /** Any thread while nothing processes: the settings in use from the next frame */
    void SetSettings( const CVoiceActivitySettings& settings );
    const CVoiceActivitySettings& GetSettings( void ) const;

    /** Forgets the audio, the floor and the decision: the next frame is the first. Real time safe */
    void Reset( void );

    /**
     *  Analyses 'frameCount' frames that follow the ones given before. Returns the decision as it is after the last
     *  analysis frame that was completed (what IsActive() gives)
     */
    bool Process( const Float32* mono, UInt32 frameCount );

    /**
     *  As above for a block that starts at the absolute frame position 'firstFrame' (the position the analysis frames
     *  are aligned to). A position other than the one after the previous block is a gap (the audio before was lost, or
     *  the stream restarts): the filters are cleared, the analysis frame that holds the gap is dropped and the onset has
     *  to be seen again, the floor and the hangover stay
     */
    bool Process( const Float32* mono, UInt32 frameCount, UInt64 firstFrame );

    /** Any thread: the decision after the last completed analysis frame */
    bool IsActive( void ) const;

    /** Any thread: the energy of the last completed frame in dB re a full scale RMS of 1.0 (-120 for silence, also before the first frame) */
    Float32 GetEnergyInDecibels( void ) const;

    /** Any thread: the noise floor in the same unit (-120 before the first frame) */
    Float32 GetNoiseFloorInDecibels( void ) const;

    /** Any thread: analysis frames completed since Prepare() or Reset() */
    UInt64 GetAnalyzedFrameCount( void ) const;

    /** The absolute frame position after the last frame given to Process() */
    UInt64 GetPosition( void ) const;

    private:

    CVoiceActivityDetector( const CVoiceActivityDetector& src );              /**< not implemented */
    CVoiceActivityDetector& operator=( const CVoiceActivityDetector& src );   /**< not implemented */

    UInt64 FrameBoundary( UInt64 frameIndex ) const;
    void BeginFrameAt( UInt64 position );
    void FinishFrame( void );
    void Analyze( Float64 meanSquare );
    void ApplySettings( void );
    void ClearState( void );

    private:

    bool m_prepared;
    UInt32 m_sampleRate;
    CVoiceActivitySettings m_settings;
    Float32 m_onsetInDecibels;
    Float32 m_releaseInDecibels;
    Float32 m_minimumLevelInDecibels;
    Float64 m_risePerFrameInDecibels;
    UInt32 m_hangoverFrames;

    CSvfFilter m_highPass;
    CSvfFilter m_lowPass;
    CSlidingMinimum m_minimum;
    Float64 m_smoothing;                /**< one pole coefficient of the energy smoothing, per frame */

    // position in the stream and in the analysis frame
    UInt64 m_position;                  /**< the absolute frame position after the audio given so far */
    UInt64 m_frameIndex;                /**< the analysis frame the position is in */
    UInt64 m_frameStart;                /**< first frame of that analysis frame */
    UInt64 m_frameEnd;                  /**< first frame after it */
    Float64 m_sum;                      /**< sum of squares of the filtered audio of that analysis frame so far */
    bool m_dropFrame;                   /**< the analysis frame is incomplete because of a gap */

    // decision state
    bool m_haveFrame;
    Float64 m_smoothed;
    Float64 m_floorInDecibels;
    UInt32 m_onsetCount;
    UInt32 m_hangoverLeft;
    bool m_active;

    // published
    MT::CTAtomic< bool > m_publishedActive;
    CAtomicFloat m_publishedEnergy;
    CAtomicFloat m_publishedFloor;
    MT::CTAtomic< UInt64 > m_publishedFrames;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICEACTIVITYDETECTOR_H ? */
