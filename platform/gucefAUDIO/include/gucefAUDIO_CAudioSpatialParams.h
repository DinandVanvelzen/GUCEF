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

#ifndef GUCEF_AUDIO_CAUDIOSPATIALPARAMS_H
#define GUCEF_AUDIO_CAUDIOSPATIALPARAMS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOATTENUATION_H
#include "gucefAUDIO_CAudioAttenuation.h"
#define GUCEF_AUDIO_CAUDIOATTENUATION_H
#endif /* GUCEF_AUDIO_CAUDIOATTENUATION_H ? */

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
 *  The world settings of the spatial audio as the audio thread sees them: a plain value, copied through a
 *  MT::CTSlotDataExchange (latest only) by the engine. Made by CAudioSpatialParams::GetWorldState(); every value is
 *  already in the unit the audio thread needs (world units, linear gains).
 */
struct GUCEF_AUDIO_PUBLIC_CPP SSpatialWorldState
{
    Float32 speedOfSoundInUnits;        /**< speed of sound in world units per second: metres per second times the units per metre */
    Float32 dopplerFactor;              /**< 0 switches the Doppler effect off */
    Float32 dopplerMinRatio;
    Float32 dopplerMaxRatio;
    Float32 dopplerSmoothingInMs;
    CAudioAttenuation::TEvaluator attenuation;  /**< the attenuation of emitters that have none of their own */
    bool useCenterSpeaker;
    Float32 lfeSendGain;                /**< linear gain of the LFE channel of a spatial voice, 0 for none */
    Float32 rearAttenuationGain;        /**< linear gain of a source behind a stereo listener, 1 for none */

    SSpatialWorldState( void );

    bool operator==( const SSpatialWorldState& other ) const;
    bool operator!=( const SSpatialWorldState& other ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  The world settings of spatial audio: the speed of sound, the unit of the world, Doppler, the attenuation emitters
 *  fall back on and the panning options. Every setter limits its value to the range given, so the settings never hold a
 *  value the spatial voices cannot use. Owned by the application, copied into the engine (CAudioEngine::SetSpatialParams()).
 *
 *  The settings can be saved to and loaded from a configuration tree (CORE::CIConfigurable), the section "Spatial" (a child
 *  of the section "AudioEngine" when they are part of CAudioEngineParams): attributes speedOfSound, unitsPerMeter, dopplerFactor,
 *  dopplerMinRatio, dopplerMaxRatio, dopplerSmoothingMs, spatializer, useCenterSpeaker, lfeSendDb, rearAttenuationDb and the child "Attenuation"
 *  (CAudioAttenuation).
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioSpatialParams : public CORE::CIConfigurable
{
    public:

    static const CString ConfigSectionName;         /**< "Spatial" */

    static const Float32 DefaultSpeedOfSound;               /**< 343.3 metres per second */
    static const Float32 DefaultUnitsPerMeter;              /**< 1 */
    static const Float32 DefaultDopplerFactor;              /**< 1 */
    static const Float32 DefaultDopplerMinRatio;            /**< 0.5 */
    static const Float32 DefaultDopplerMaxRatio;            /**< 2 */
    static const Float32 DefaultDopplerSmoothingInMs;       /**< 50 */
    static const Float32 LfeSendOffInDecibels;              /**< -120: the LFE send level at or under which there is no send */

    CAudioSpatialParams( void );

    CAudioSpatialParams( const CAudioSpatialParams& src );

    CAudioSpatialParams& operator=( const CAudioSpatialParams& src );

    virtual ~CAudioSpatialParams() GUCEF_VIRTUAL_OVERRIDE;

    bool operator==( const CAudioSpatialParams& other ) const;
    bool operator!=( const CAudioSpatialParams& other ) const;

    /** Metres per second, 1 .. 100000 (default 343.3) */
    Float32 GetSpeedOfSound( void ) const;
    void SetSpeedOfSound( Float32 metersPerSecond );

    /** How many world units a metre is, 0.0001 .. 100000 (default 1). The speed of sound and velocities are in world units */
    Float32 GetUnitsPerMeter( void ) const;
    void SetUnitsPerMeter( Float32 unitsPerMeter );

    /** The speed of sound in world units per second */
    Float32 GetSpeedOfSoundInUnits( void ) const;

    /** Scales the Doppler effect, 0 .. 10; 0 switches it off for all voices (default 1) */
    Float32 GetDopplerFactor( void ) const;
    void SetDopplerFactor( Float32 factor );

    /** The limits of the pitch ratio the Doppler effect may give, 0.01 .. 100 (default 0.5 and 2). A minimum above the maximum moves the maximum, a maximum under the minimum moves the minimum */
    Float32 GetDopplerMinRatio( void ) const;
    void SetDopplerMinRatio( Float32 ratio );
    Float32 GetDopplerMaxRatio( void ) const;
    void SetDopplerMaxRatio( Float32 ratio );

    /** Time constant of the smoothing of the Doppler ratio, 0 .. 1000 ms (default 50) */
    Float32 GetDopplerSmoothingInMs( void ) const;
    void SetDopplerSmoothingInMs( Float32 milliseconds );

    /** The attenuation of emitters that do not have their own (default: inverse, minimum distance 1, maximum 100, rolloff 1) */
    const CAudioAttenuation& GetDefaultAttenuation( void ) const;
    CAudioAttenuation& GetDefaultAttenuation( void );

    /** True (default) to pan to the centre speaker of a surround layout too */
    bool GetUseCenterSpeaker( void ) const;
    void SetUseCenterSpeaker( bool useCenter );

    /** Level in dB at which spatial voices feed the LFE channel, -120 .. 12; -120 (default) is off */
    Float32 GetLfeSendInDecibels( void ) const;
    void SetLfeSendInDecibels( Float32 decibels );
    bool IsLfeSendEnabled( void ) const;

    /** How much quieter a source behind a stereo listener is (front and back cue), -60 .. 0 dB (default 0: no cue) */
    Float32 GetRearAttenuationInDecibels( void ) const;
    void SetRearAttenuationInDecibels( Float32 decibels );

    /** The name of the spatializer of the engine (default "panning"). Only the built-in one exists until others are registered */
    const CString& GetSpatializerName( void ) const;
    void SetSpatializerName( const CString& name );

    /** The settings in the form the audio thread reads them */
    void GetWorldState( SSpatialWorldState& state ) const;

    /** Writes the section "Spatial" (kept when it is there), with the child "Attenuation" */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the section from the tree, or the tree itself when it is that section. False when there is no such section. What it does not have keeps its value */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    Float32 m_speedOfSound;
    Float32 m_unitsPerMeter;
    Float32 m_dopplerFactor;
    Float32 m_dopplerMinRatio;
    Float32 m_dopplerMaxRatio;
    Float32 m_dopplerSmoothingInMs;
    CAudioAttenuation m_defaultAttenuation;
    bool m_useCenterSpeaker;
    Float32 m_lfeSendInDecibels;
    Float32 m_rearAttenuationInDecibels;
    CString m_spatializerName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOSPATIALPARAMS_H ? */
