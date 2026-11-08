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

#ifndef GUCEF_AUDIO_CHALFDUPLEXSETTINGS_H
#define GUCEF_AUDIO_CHALFDUPLEXSETTINGS_H

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
 *  The settings of a CHalfDuplexController (section 4.13 of the intercom design). Every setter limits its value to
 *  the range given. Saved as the section "HalfDuplex" with the attributes enabled, farThresholdDb, micAttenuationDb,
 *  breakInMarginDb, breakInMs, hangoverMs, couplingTimeMs, initialCouplingDb, farReleaseMs, duckFarEnd, farEndBus,
 *  echoCancellerHandOver and handOverErleDb, and the child section "VoiceActivity" (the near-end voice activity detector
 *  of the controller).
 */
class GUCEF_AUDIO_PUBLIC_CPP CHalfDuplexSettings : public CORE::CIConfigurable
{
    public:

    static const CString ConfigSectionName;         /**< "HalfDuplex" */

    CHalfDuplexSettings( void );

    CHalfDuplexSettings( const CHalfDuplexSettings& src );

    CHalfDuplexSettings& operator=( const CHalfDuplexSettings& src );

    virtual ~CHalfDuplexSettings() GUCEF_VIRTUAL_OVERRIDE;

    bool operator==( const CHalfDuplexSettings& other ) const;
    bool operator!=( const CHalfDuplexSettings& other ) const;

    /** Whether the controller acts at all (default true); off, the microphone and the speaker are left alone */
    bool GetEnabled( void ) const;
    void SetEnabled( bool enabled );

    /** The far end is active while the peak of the far-end bus is above this, -90 to 0 dBFS (default -50) */
    Float32 GetFarThresholdInDecibels( void ) const;
    void SetFarThresholdInDecibels( Float32 decibels );

    /** The microphone gain while the far end talks (state FAR), -80 to 0 dB (default -40) */
    Float32 GetMicAttenuationInDecibels( void ) const;
    void SetMicAttenuationInDecibels( Float32 decibels );

    /** The near end breaks in when it is this much louder than the echo the far end makes, 0 to 30 dB (default 6) */
    Float32 GetBreakInMarginInDecibels( void ) const;
    void SetBreakInMarginInDecibels( Float32 decibels );

    /** ... for this long, 0 to 1000 ms (default 60) */
    Float32 GetBreakInInMs( void ) const;
    void SetBreakInInMs( Float32 milliseconds );

    /** FAR ends after the far end was quiet this long, NEAR after the near end was; 0 to 5000 ms (default 300) */
    Float32 GetHangoverInMs( void ) const;
    void SetHangoverInMs( Float32 milliseconds );

    /** The time over which the acoustic coupling is averaged, 100 to 30000 ms (default 2000) */
    Float32 GetCouplingTimeInMs( void ) const;
    void SetCouplingTimeInMs( Float32 milliseconds );

    /** The coupling assumed until one was measured, -60 to 20 dB (default 0: the echo as loud as the far end, cautious) */
    Float32 GetInitialCouplingInDecibels( void ) const;
    void SetInitialCouplingInDecibels( Float32 decibels );

    /**
     *  The release of the far-end envelope the echo is estimated from, 5 to 1000 ms (default 100): it must cover the
     *  time from the bus meter to the microphone (the playback buffer, the acoustic path and its reverberation)
     */
    Float32 GetFarReleaseInMs( void ) const;
    void SetFarReleaseInMs( Float32 milliseconds );

    /** Whether the far end is ducked (the manual key of the controller's ducker) while the near end talks (default true) */
    bool GetDuckFarEnd( void ) const;
    void SetDuckFarEnd( bool duck );

    /** The bus whose meter is the far end (the bus the receivers play on, typically "call"); empty: the master bus */
    const CString& GetFarEndBusName( void ) const;
    void SetFarEndBusName( const CString& busName );

    /**
     *  Whether the controller stands aside while the echo canceller of the capture path holds the echo down (default
     *  true): converged, with an echo return loss enhancement of at least handOverErleDb. It comes back as soon as the
     *  canceller is no longer converged or its enhancement fell 6 dB below that
     */
    bool GetEchoCancellerHandOver( void ) const;
    void SetEchoCancellerHandOver( bool handOver );

    /** The echo return loss enhancement of the canceller from which the controller stands aside, 6 to 40 dB (default 18) */
    Float32 GetHandOverErleInDecibels( void ) const;
    void SetHandOverErleInDecibels( Float32 decibels );

    /** The settings of the controller's own detector of the near-end voice (it looks at the microphone before its gain) */
    const CVoiceActivitySettings& GetVoiceActivitySettings( void ) const;
    void SetVoiceActivitySettings( const CVoiceActivitySettings& settings );

    /** Writes the section "HalfDuplex" (kept when it is there) */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the section from the tree, or the tree itself when it is that section. False when there is none; what it does not have keeps its value */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool m_enabled;
    Float32 m_farThresholdInDecibels;
    Float32 m_micAttenuationInDecibels;
    Float32 m_breakInMarginInDecibels;
    Float32 m_breakInInMs;
    Float32 m_hangoverInMs;
    Float32 m_couplingTimeInMs;
    Float32 m_initialCouplingInDecibels;
    Float32 m_farReleaseInMs;
    bool m_duckFarEnd;
    CString m_farEndBusName;
    bool m_echoCancellerHandOver;
    Float32 m_handOverErleInDecibels;
    CVoiceActivitySettings m_voiceActivity;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CHALFDUPLEXSETTINGS_H ? */
