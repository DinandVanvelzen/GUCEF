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

#ifndef GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H
#define GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

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

/** How a parameter moves from its current value to a new target */
enum EAudioSmoothing
{
    AUDIOSMOOTHING_NONE = 0     ,   /**< jumps to the target */
    AUDIOSMOOTHING_LINEAR_RAMP  ,   /**< straight line to the target in a fixed time (gains) */
    AUDIOSMOOTHING_ONE_POLE         /**< exponential approach with a time constant (frequencies, Q, ratios, times) */
};
typedef enum EAudioSmoothing TAudioSmoothing;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Smooths the changes of one effect parameter so they do not click or zipper. The value moves in steps of a fixed
 *  number of frames (1 for per sample smoothing, 32 for coefficient updates on sub-blocks); every Step() call advances
 *  one step. Smoothing works on the value itself or, for logarithmic parameters such as frequencies, on its base 2
 *  logarithm, so a sweep moves evenly per octave.
 *
 *  Used on the audio thread: no allocation, no locks. Prepare() is meant for the owner thread (it calls the math library).
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioParameterSmoother
{
    public:

    CAudioParameterSmoother( void );

    /**
     *  @param mode             linear ramp, one pole or none
     *  @param timeInSeconds    ramp duration (linear ramp) or time constant (one pole). 0 behaves as no smoothing
     *  @param sampleRate       frames per second
     *  @param logarithmic      smooth on log2 of the value: for positive values only (frequencies)
     *  @param stepFrames       frames per Step()
     */
    void Prepare( TAudioSmoothing mode, Float64 timeInSeconds, Float64 sampleRate, bool logarithmic = false, UInt32 stepFrames = 1 );

    /** Jumps to a value: no smoothing in progress afterwards */
    void Reset( Float32 value );

    /** Sets where the value is going. Starts a new ramp from the current value (linear) or changes the target (one pole) */
    void SetTarget( Float32 target );

    /** Linear ramp to the target in the given number of steps, whatever the prepared time: for changes spread over a block */
    void SetTargetInSteps( Float32 target, UInt32 steps );

    /** Advances one step and returns the new value */
    Float32 Step( void );

    /** The current value, without advancing */
    Float32 GetValue( void ) const;

    Float32 GetTarget( void ) const;

    /** True while the value has not reached its target yet */
    bool IsSmoothing( void ) const;

    UInt32 GetStepFrames( void ) const;

    private:

    Float64 ToDomain( Float32 value ) const;
    Float32 FromDomain( Float64 value ) const;

    private:

    TAudioSmoothing m_mode;
    bool m_logarithmic;
    UInt32 m_stepFrames;
    UInt32 m_rampSteps;             /**< linear: steps of a full ramp */
    Float64 m_coefficient;          /**< one pole: how much of the remaining distance is kept per step */
    Float64 m_current;              /**< in the smoothing domain */
    Float64 m_target;               /**< in the smoothing domain */
    Float64 m_increment;            /**< linear: per step */
    UInt32 m_stepsLeft;             /**< linear: steps until the target */
    Float32 m_currentValue;         /**< m_current outside the domain */
    Float32 m_targetValue;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOPARAMETERSMOOTHER_H ? */
