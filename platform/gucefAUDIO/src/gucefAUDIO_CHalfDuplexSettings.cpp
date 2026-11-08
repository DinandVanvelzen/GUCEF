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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefAUDIO_CHalfDuplexSettings.h"

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** 'value' within the range; anything that is not a number gives 'fallback' */
Float32
Limited( Float32 value, Float32 minimum, Float32 maximum, Float32 fallback )
{
    if ( !( value == value ) )
        return fallback;
    if ( value < minimum )
        return minimum;
    if ( value > maximum )
        return maximum;
    return value;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CHalfDuplexSettings::ConfigSectionName = "HalfDuplex";

/*-------------------------------------------------------------------------*/

CHalfDuplexSettings::CHalfDuplexSettings( void )
    : CORE::CIConfigurable()
    , m_enabled( true )
    , m_farThresholdInDecibels( -50.0f )
    , m_micAttenuationInDecibels( -40.0f )
    , m_breakInMarginInDecibels( 6.0f )
    , m_breakInInMs( 60.0f )
    , m_hangoverInMs( 300.0f )
    , m_couplingTimeInMs( 2000.0f )
    , m_initialCouplingInDecibels( 0.0f )
    , m_farReleaseInMs( 100.0f )
    , m_duckFarEnd( true )
    , m_farEndBusName()
    , m_echoCancellerHandOver( true )
    , m_handOverErleInDecibels( 18.0f )
    , m_voiceActivity()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHalfDuplexSettings::CHalfDuplexSettings( const CHalfDuplexSettings& src )
    : CORE::CIConfigurable( src )
    , m_enabled( src.m_enabled )
    , m_farThresholdInDecibels( src.m_farThresholdInDecibels )
    , m_micAttenuationInDecibels( src.m_micAttenuationInDecibels )
    , m_breakInMarginInDecibels( src.m_breakInMarginInDecibels )
    , m_breakInInMs( src.m_breakInInMs )
    , m_hangoverInMs( src.m_hangoverInMs )
    , m_couplingTimeInMs( src.m_couplingTimeInMs )
    , m_initialCouplingInDecibels( src.m_initialCouplingInDecibels )
    , m_farReleaseInMs( src.m_farReleaseInMs )
    , m_duckFarEnd( src.m_duckFarEnd )
    , m_farEndBusName( src.m_farEndBusName )
    , m_echoCancellerHandOver( src.m_echoCancellerHandOver )
    , m_handOverErleInDecibels( src.m_handOverErleInDecibels )
    , m_voiceActivity( src.m_voiceActivity )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHalfDuplexSettings&
CHalfDuplexSettings::operator=( const CHalfDuplexSettings& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_enabled = src.m_enabled;
        m_farThresholdInDecibels = src.m_farThresholdInDecibels;
        m_micAttenuationInDecibels = src.m_micAttenuationInDecibels;
        m_breakInMarginInDecibels = src.m_breakInMarginInDecibels;
        m_breakInInMs = src.m_breakInInMs;
        m_hangoverInMs = src.m_hangoverInMs;
        m_couplingTimeInMs = src.m_couplingTimeInMs;
        m_initialCouplingInDecibels = src.m_initialCouplingInDecibels;
        m_farReleaseInMs = src.m_farReleaseInMs;
        m_duckFarEnd = src.m_duckFarEnd;
        m_farEndBusName = src.m_farEndBusName;
        m_echoCancellerHandOver = src.m_echoCancellerHandOver;
        m_handOverErleInDecibels = src.m_handOverErleInDecibels;
        m_voiceActivity = src.m_voiceActivity;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CHalfDuplexSettings::~CHalfDuplexSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexSettings::operator==( const CHalfDuplexSettings& other ) const
{GUCEF_TRACE;

    return m_enabled == other.m_enabled &&
           m_farThresholdInDecibels == other.m_farThresholdInDecibels &&
           m_micAttenuationInDecibels == other.m_micAttenuationInDecibels &&
           m_breakInMarginInDecibels == other.m_breakInMarginInDecibels &&
           m_breakInInMs == other.m_breakInInMs &&
           m_hangoverInMs == other.m_hangoverInMs &&
           m_couplingTimeInMs == other.m_couplingTimeInMs &&
           m_initialCouplingInDecibels == other.m_initialCouplingInDecibels &&
           m_farReleaseInMs == other.m_farReleaseInMs &&
           m_duckFarEnd == other.m_duckFarEnd &&
           m_farEndBusName == other.m_farEndBusName &&
           m_echoCancellerHandOver == other.m_echoCancellerHandOver &&
           m_handOverErleInDecibels == other.m_handOverErleInDecibels &&
           m_voiceActivity == other.m_voiceActivity;
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexSettings::operator!=( const CHalfDuplexSettings& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexSettings::GetEnabled( void ) const
{GUCEF_TRACE;

    return m_enabled;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetEnabled( bool value )
{GUCEF_TRACE;

    m_enabled = value;
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexSettings::GetFarThresholdInDecibels( void ) const
{GUCEF_TRACE;

    return m_farThresholdInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetFarThresholdInDecibels( Float32 value )
{GUCEF_TRACE;

    m_farThresholdInDecibels = Limited( value, -90.0f, 0.0f, m_farThresholdInDecibels );
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexSettings::GetMicAttenuationInDecibels( void ) const
{GUCEF_TRACE;

    return m_micAttenuationInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetMicAttenuationInDecibels( Float32 value )
{GUCEF_TRACE;

    m_micAttenuationInDecibels = Limited( value, -80.0f, 0.0f, m_micAttenuationInDecibels );
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexSettings::GetBreakInMarginInDecibels( void ) const
{GUCEF_TRACE;

    return m_breakInMarginInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetBreakInMarginInDecibels( Float32 value )
{GUCEF_TRACE;

    m_breakInMarginInDecibels = Limited( value, 0.0f, 30.0f, m_breakInMarginInDecibels );
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexSettings::GetBreakInInMs( void ) const
{GUCEF_TRACE;

    return m_breakInInMs;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetBreakInInMs( Float32 value )
{GUCEF_TRACE;

    m_breakInInMs = Limited( value, 0.0f, 1000.0f, m_breakInInMs );
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexSettings::GetHangoverInMs( void ) const
{GUCEF_TRACE;

    return m_hangoverInMs;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetHangoverInMs( Float32 value )
{GUCEF_TRACE;

    m_hangoverInMs = Limited( value, 0.0f, 5000.0f, m_hangoverInMs );
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexSettings::GetCouplingTimeInMs( void ) const
{GUCEF_TRACE;

    return m_couplingTimeInMs;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetCouplingTimeInMs( Float32 value )
{GUCEF_TRACE;

    m_couplingTimeInMs = Limited( value, 100.0f, 30000.0f, m_couplingTimeInMs );
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexSettings::GetInitialCouplingInDecibels( void ) const
{GUCEF_TRACE;

    return m_initialCouplingInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetInitialCouplingInDecibels( Float32 value )
{GUCEF_TRACE;

    m_initialCouplingInDecibels = Limited( value, -60.0f, 20.0f, m_initialCouplingInDecibels );
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexSettings::GetFarReleaseInMs( void ) const
{GUCEF_TRACE;

    return m_farReleaseInMs;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetFarReleaseInMs( Float32 value )
{GUCEF_TRACE;

    m_farReleaseInMs = Limited( value, 5.0f, 1000.0f, m_farReleaseInMs );
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexSettings::GetDuckFarEnd( void ) const
{GUCEF_TRACE;

    return m_duckFarEnd;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetDuckFarEnd( bool value )
{GUCEF_TRACE;

    m_duckFarEnd = value;
}

/*-------------------------------------------------------------------------*/

const CString&
CHalfDuplexSettings::GetFarEndBusName( void ) const
{GUCEF_TRACE;

    return m_farEndBusName;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetFarEndBusName( const CString& value )
{GUCEF_TRACE;

    m_farEndBusName = value;
}

/*-------------------------------------------------------------------------*/

const CVoiceActivitySettings&
CHalfDuplexSettings::GetVoiceActivitySettings( void ) const
{GUCEF_TRACE;

    return m_voiceActivity;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetVoiceActivitySettings( const CVoiceActivitySettings& value )
{GUCEF_TRACE;

    m_voiceActivity = value;
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexSettings::GetEchoCancellerHandOver( void ) const
{GUCEF_TRACE;

    return m_echoCancellerHandOver;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetEchoCancellerHandOver( bool handOver )
{GUCEF_TRACE;

    m_echoCancellerHandOver = handOver;
}

/*-------------------------------------------------------------------------*/

Float32
CHalfDuplexSettings::GetHandOverErleInDecibels( void ) const
{GUCEF_TRACE;

    return m_handOverErleInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CHalfDuplexSettings::SetHandOverErleInDecibels( Float32 decibels )
{GUCEF_TRACE;

    m_handOverErleInDecibels = Limited( decibels, 6.0f, 40.0f, m_handOverErleInDecibels );
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexSettings::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "enabled", m_enabled );
    section->SetAttribute( "farThresholdDb", m_farThresholdInDecibels );
    section->SetAttribute( "micAttenuationDb", m_micAttenuationInDecibels );
    section->SetAttribute( "breakInMarginDb", m_breakInMarginInDecibels );
    section->SetAttribute( "breakInMs", m_breakInInMs );
    section->SetAttribute( "hangoverMs", m_hangoverInMs );
    section->SetAttribute( "couplingTimeMs", m_couplingTimeInMs );
    section->SetAttribute( "initialCouplingDb", m_initialCouplingInDecibels );
    section->SetAttribute( "farReleaseMs", m_farReleaseInMs );
    section->SetAttribute( "duckFarEnd", m_duckFarEnd );
    section->SetAttribute( "farEndBus", m_farEndBusName );
    section->SetAttribute( "echoCancellerHandOver", m_echoCancellerHandOver );
    section->SetAttribute( "handOverErleDb", m_handOverErleInDecibels );
    return m_voiceActivity.SaveConfig( *section );
}

/*-------------------------------------------------------------------------*/

bool
CHalfDuplexSettings::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    m_enabled = section->GetAttributeValueOrChildValueByName( "enabled" ).AsBool( m_enabled, true );
    SetFarThresholdInDecibels( section->GetAttributeValueOrChildValueByName( "farThresholdDb" ).AsFloat32( m_farThresholdInDecibels, true ) );
    SetMicAttenuationInDecibels( section->GetAttributeValueOrChildValueByName( "micAttenuationDb" ).AsFloat32( m_micAttenuationInDecibels, true ) );
    SetBreakInMarginInDecibels( section->GetAttributeValueOrChildValueByName( "breakInMarginDb" ).AsFloat32( m_breakInMarginInDecibels, true ) );
    SetBreakInInMs( section->GetAttributeValueOrChildValueByName( "breakInMs" ).AsFloat32( m_breakInInMs, true ) );
    SetHangoverInMs( section->GetAttributeValueOrChildValueByName( "hangoverMs" ).AsFloat32( m_hangoverInMs, true ) );
    SetCouplingTimeInMs( section->GetAttributeValueOrChildValueByName( "couplingTimeMs" ).AsFloat32( m_couplingTimeInMs, true ) );
    SetInitialCouplingInDecibels( section->GetAttributeValueOrChildValueByName( "initialCouplingDb" ).AsFloat32( m_initialCouplingInDecibels, true ) );
    SetFarReleaseInMs( section->GetAttributeValueOrChildValueByName( "farReleaseMs" ).AsFloat32( m_farReleaseInMs, true ) );
    m_duckFarEnd = section->GetAttributeValueOrChildValueByName( "duckFarEnd" ).AsBool( m_duckFarEnd, true );
    m_farEndBusName = section->GetAttributeValueOrChildValueByName( "farEndBus" ).AsString( m_farEndBusName, true );
    m_echoCancellerHandOver = section->GetAttributeValueOrChildValueByName( "echoCancellerHandOver" ).AsBool( m_echoCancellerHandOver, true );
    SetHandOverErleInDecibels( section->GetAttributeValueOrChildValueByName( "handOverErleDb" ).AsFloat32( m_handOverErleInDecibels, true ) );

    // a section without detector settings keeps the ones there are
    m_voiceActivity.LoadConfig( *section );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CHalfDuplexSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::AUDIO::CHalfDuplexSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
