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

#ifndef GUCEF_AUDIO_CAUDIOEFFECT_H
#define GUCEF_AUDIO_CAUDIOEFFECT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_AUDIO_CAUDIOFORMAT_H
#include "gucefAUDIO_CAudioFormat.h"
#define GUCEF_AUDIO_CAUDIOFORMAT_H
#endif /* GUCEF_AUDIO_CAUDIOFORMAT_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTPARAMETERINFO_H
#include "gucefAUDIO_CAudioEffectParameterInfo.h"
#define GUCEF_AUDIO_CAUDIOEFFECTPARAMETERINFO_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTPARAMETERINFO_H ? */

#ifndef GUCEF_AUDIO_CAUDIOPARAMETERBLOCK_H
#include "gucefAUDIO_CAudioParameterBlock.h"
#define GUCEF_AUDIO_CAUDIOPARAMETERBLOCK_H
#endif /* GUCEF_AUDIO_CAUDIOPARAMETERBLOCK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Tail of an effect that can sound forever after its input stopped (feedback at or near 100 %) */
#define GUCEF_AUDIO_TAIL_INFINITE   0xFFFFFFFFUL

/** Frames per sub-block on which effects update coefficients, aligned to the absolute frame count of the effect */
#define GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES  32

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

class CAudioEffectFactory;

/*-------------------------------------------------------------------------*/

/**
 *  Base class of all audio effects: something that changes interleaved float audio in place, block by block, on the
 *  audio thread, steered by parameters that any thread can set.
 *
 *  Life of an effect:
 *   - made (usually by a factory through CAudioEffectRegistry or CAudioEngine::CreateEffect()) and prepared for a format
 *     and a maximum block size on the owner thread: Prepare() allocates everything the effect will ever need
 *   - placed in an effect chain (a bus or voice slot); from then on Process() and Reset() run on the audio thread only,
 *     they never allocate, lock, log or trace
 *   - parameters are set with SetParameter() / SetParameters() from any thread at any time. They are seen by the audio
 *     thread at the start of a block (HaveParametersChanged()) and smoothed by the effect where a jump would be audible
 *
 *  Rules for implementations (OnProcess):
 *   - the output for a given input and parameter history does not depend on how the frames are split into calls.
 *     Coefficient updates happen on sub-blocks of GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES frames aligned to
 *     GetProcessedFrameCount(), not on call boundaries
 *   - channel count and rate come from Prepare(). An LFE channel is passed through unchanged by reverb, delay and
 *     equalizer, and included in the linked detection of dynamics effects
 *   - parameter ids are the indices of the parameter table given to the constructor: an enumeration of the concrete class.
 *     Names are only for configuration
 *
 *  Process() and Reset() are not virtual themselves: they keep the frame counter and call OnProcess() / OnReset().
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioEffect : public CORE::CIConfigurable
{
    public:

    typedef CORE::CTSharedPtr< CAudioEffect, MT::CMutex > TAudioEffectPtr;

    static const CString ClassTypeName;

    virtual ~CAudioEffect() GUCEF_VIRTUAL_OVERRIDE;

    /** Name of the kind of effect, the name it is registered under ("limiter", "reverb", ...) */
    virtual const CString& GetEffectTypeName( void ) const = 0;

    UInt32 GetParameterCount( void ) const;

    /** Descriptor of a parameter, null for an unknown id */
    const CAudioEffectParameterInfo* GetParameterInfo( UInt32 parameterId ) const;

    /** Id of a parameter by its name, without regard for case */
    bool TryGetParameterId( const CString& name, UInt32& parameterId ) const;

    /** Any thread: sets a parameter, limited to the range of its descriptor */
    void SetParameter( UInt32 parameterId, Float32 value );

    /** Any thread: sets several parameters that the audio thread will always see together */
    void SetParameters( const UInt32* parameterIds, const Float32* values, UInt32 count );

    /** Any thread: the value last set */
    Float32 GetParameter( UInt32 parameterId ) const;

    /** Owner thread, while the effect is not processed: allocates for the format and block size and clears the state */
    bool Prepare( const CAudioFormat& format, UInt32 maxBlockFrames );

    bool IsPrepared( void ) const;

    /** True when prepared for this format (rate, channels, layout) and at least this block size */
    bool IsPreparedFor( const CAudioFormat& format, UInt32 maxBlockFrames ) const;

    /** The format given to Prepare() */
    const CAudioFormat& GetFormat( void ) const;

    UInt32 GetMaxBlockFrames( void ) const;

    /** Audio thread allowed: clears the state (delay lines, envelopes), as if no audio had been processed. No allocation */
    void Reset( void );

    /** Audio thread: processes interleaved float audio in place. frames <= the prepared maximum. Unprepared: passes audio unchanged */
    void Process( Float32* inOut, UInt32 frames );

    /** Delay the effect adds to the signal, in frames */
    virtual UInt32 GetLatencyInFrames( void ) const;

    /** How long the effect keeps sounding after its input went silent, in frames. GUCEF_AUDIO_TAIL_INFINITE when it may sustain */
    virtual UInt32 GetTailInFrames( void ) const;

    /** The factory that made this effect, null for one constructed directly */
    CAudioEffectFactory* GetFactory( void ) const;

    /**
     *  Adds to 'options' the construction options that this effect was made with and that its parameters do not carry:
     *  what CAudioEffectFactory::Create() needs to make an effect of the same shape (the number of bands of an equalizer,
     *  the longest delay of a delay). Effects without such options add nothing, which is the default.
     */
    virtual void GetConstructionOptions( CORE::CValueList& options ) const;

    /**
     *  Writes attribute "type", a child "Options" with the construction options when the effect has any
     *  (GetConstructionOptions()), and a child "Parameters" with one attribute per parameter name. Enumerations are
     *  written by their label
     */
    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Reads the parameters by name from the child "Parameters". Unknown names are logged and ignored, missing ones keep their
     *  value. The child "Options" is not read: construction options cannot change in an effect that was made
     */
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Like LoadConfig(), and with a 'baseline' also gives every parameter that the config does not name the value it has in
     *  the baseline, an effect of the same type and shape that nobody has touched since it was made, so that the effect ends
     *  up in a state that depends on the config alone. Only values that differ from the current ones are set (together, so
     *  the audio thread sees them together), an effect that already is as the config says is not touched. False for a
     *  config of another effect type
     */
    bool ApplyConfig( const CORE::CDataNode& config, const CAudioEffect* baseline );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /**
     *  @param parameterInfos   table of the parameters of the concrete class, entry i describes parameter id i. Must
     *                          outlive the effect (a static table of the class)
     */
    CAudioEffect( const CAudioEffectParameterInfo* parameterInfos, UInt32 parameterCount );

    /** Allocates for the format and block size and clears the state. Owner thread */
    virtual bool OnPrepare( const CAudioFormat& format, UInt32 maxBlockFrames ) = 0;

    /** Clears the state. Audio thread allowed, no allocation */
    virtual void OnReset( void ) = 0;

    /** Processes the audio in place. Audio thread */
    virtual void OnProcess( Float32* inOut, UInt32 frames ) = 0;

    /** Audio thread: true when parameters were set since the last call; ReadParameter() then gives the new values */
    bool HaveParametersChanged( void );

    /** Audio thread: a parameter value as of the last HaveParametersChanged() (or Prepare()) */
    inline Float32 ReadParameter( UInt32 parameterId ) const { return m_parameters.Read( parameterId ); }

    /** Frames processed since Prepare() or Reset(): the absolute position for sub-block alignment */
    inline UInt64 GetProcessedFrameCount( void ) const { return m_processedFrames; }

    private:
    friend class CAudioEffectFactory;

    CAudioEffect( const CAudioEffect& src );            /**< not implemented */
    CAudioEffect& operator=( const CAudioEffect& src ); /**< not implemented */

    private:

    const CAudioEffectParameterInfo* m_parameterInfos;
    UInt32 m_parameterCount;
    CAudioParameterBlock m_parameters;
    CAudioFormat m_format;
    UInt32 m_maxBlockFrames;
    bool m_prepared;
    UInt64 m_processedFrames;
    CAudioEffectFactory* m_factory;
};

typedef CAudioEffect::TAudioEffectPtr TAudioEffectPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOEFFECT_H ? */
