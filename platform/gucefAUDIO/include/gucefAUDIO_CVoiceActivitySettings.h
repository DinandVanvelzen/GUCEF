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

#ifndef GUCEF_AUDIO_CVOICEACTIVITYSETTINGS_H
#define GUCEF_AUDIO_CVOICEACTIVITYSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of a CVoiceActivityDetector. Every setter limits its value to the range given, so a settings object
 *  never holds a value the detector cannot use; the release margin never exceeds the onset margin.
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceActivitySettings : public CORE::CIConfigurable
{
    public:

    static const CString ConfigSectionName;         /**< "VoiceActivity" */

    static const Float32 DefaultOnsetInDecibels;                /**< 9 */
    static const Float32 DefaultReleaseInDecibels;              /**< 6 */
    static const Float32 DefaultHangoverInMs;                   /**< 200 */
    static const Float32 DefaultMinimumLevelInDecibels;         /**< -65 */
    static const Float32 DefaultFloorRiseInDecibelsPerSecond;   /**< 3 */

    CVoiceActivitySettings( void );

    CVoiceActivitySettings( const CVoiceActivitySettings& src );

    CVoiceActivitySettings& operator=( const CVoiceActivitySettings& src );

    virtual ~CVoiceActivitySettings() GUCEF_VIRTUAL_OVERRIDE;

    bool operator==( const CVoiceActivitySettings& other ) const;
    bool operator!=( const CVoiceActivitySettings& other ) const;

    /** The energy of a frame must exceed the noise floor by this much, in two consecutive frames, for the voice to start. 0 to 60 dB */
    Float32 GetOnsetInDecibels( void ) const;
    void SetOnsetInDecibels( Float32 decibels );

    /** Once active the voice stays active while the energy exceeds the noise floor by this much. 0 to 60 dB, never more than the onset margin */
    Float32 GetReleaseInDecibels( void ) const;
    void SetReleaseInDecibels( Float32 decibels );

    /** How long the voice stays active after the energy fell below the release margin. 0 to 5000 ms */
    Float32 GetHangoverInMs( void ) const;
    void SetHangoverInMs( Float32 milliseconds );

    /** The band energy (dB relative to a full scale RMS of 1.0) below which the voice is never active. -120 to 0 dB */
    Float32 GetMinimumLevelInDecibels( void ) const;
    void SetMinimumLevelInDecibels( Float32 decibels );

    /** The most the noise floor rises per second when the tracked minimum moves up. 0.1 to 60 dB per second. It falls at once */
    Float32 GetFloorRiseInDecibelsPerSecond( void ) const;
    void SetFloorRiseInDecibelsPerSecond( Float32 decibelsPerSecond );

    /** Writes the section "VoiceActivity" (kept when it is there) with the attributes onsetDb, releaseDb, hangoverMs, minimumLevelDb and floorRiseDbPerSecond */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the section from the tree, or the tree itself when it is that section. False when there is no such section. What it does not have keeps its value */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    Float32 m_onsetInDecibels;
    Float32 m_releaseInDecibels;
    Float32 m_hangoverInMs;
    Float32 m_minimumLevelInDecibels;
    Float32 m_floorRiseInDecibelsPerSecond;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICEACTIVITYSETTINGS_H ? */
