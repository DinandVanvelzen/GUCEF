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

#include "gucefAUDIO_CAudioSpatialParams.h"

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#include "gucefMATH_SpatialAudioFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H ? */

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

SSpatialWorldState::SSpatialWorldState( void )
    : speedOfSoundInUnits( 343.3f )
    , dopplerFactor( 1.0f )
    , dopplerMinRatio( 0.5f )
    , dopplerMaxRatio( 2.0f )
    , dopplerSmoothingInMs( 50.0f )
    , attenuation()
    , useCenterSpeaker( true )
    , lfeSendGain( 0.0f )
    , rearAttenuationGain( 1.0f )
{

}

/*-------------------------------------------------------------------------*/

bool
SSpatialWorldState::operator==( const SSpatialWorldState& other ) const
{
    return speedOfSoundInUnits == other.speedOfSoundInUnits && dopplerFactor == other.dopplerFactor &&
           dopplerMinRatio == other.dopplerMinRatio && dopplerMaxRatio == other.dopplerMaxRatio &&
           dopplerSmoothingInMs == other.dopplerSmoothingInMs && attenuation == other.attenuation &&
           useCenterSpeaker == other.useCenterSpeaker && lfeSendGain == other.lfeSendGain &&
           rearAttenuationGain == other.rearAttenuationGain;
}

/*-------------------------------------------------------------------------*/

bool
SSpatialWorldState::operator!=( const SSpatialWorldState& other ) const
{
    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

const CString CAudioSpatialParams::ConfigSectionName = "Spatial";
const Float32 CAudioSpatialParams::DefaultSpeedOfSound = 343.3f;
const Float32 CAudioSpatialParams::DefaultUnitsPerMeter = 1.0f;
const Float32 CAudioSpatialParams::DefaultDopplerFactor = 1.0f;
const Float32 CAudioSpatialParams::DefaultDopplerMinRatio = 0.5f;
const Float32 CAudioSpatialParams::DefaultDopplerMaxRatio = 2.0f;
const Float32 CAudioSpatialParams::DefaultDopplerSmoothingInMs = 50.0f;
const Float32 CAudioSpatialParams::LfeSendOffInDecibels = -120.0f;

/*-------------------------------------------------------------------------*/

CAudioSpatialParams::CAudioSpatialParams( void )
    : CORE::CIConfigurable()
    , m_speedOfSound( DefaultSpeedOfSound )
    , m_unitsPerMeter( DefaultUnitsPerMeter )
    , m_dopplerFactor( DefaultDopplerFactor )
    , m_dopplerMinRatio( DefaultDopplerMinRatio )
    , m_dopplerMaxRatio( DefaultDopplerMaxRatio )
    , m_dopplerSmoothingInMs( DefaultDopplerSmoothingInMs )
    , m_defaultAttenuation()
    , m_useCenterSpeaker( true )
    , m_lfeSendInDecibels( LfeSendOffInDecibels )
    , m_rearAttenuationInDecibels( 0.0f )
    , m_spatializerName( "panning" )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioSpatialParams::CAudioSpatialParams( const CAudioSpatialParams& src )
    : CORE::CIConfigurable( src )
    , m_speedOfSound( src.m_speedOfSound )
    , m_unitsPerMeter( src.m_unitsPerMeter )
    , m_dopplerFactor( src.m_dopplerFactor )
    , m_dopplerMinRatio( src.m_dopplerMinRatio )
    , m_dopplerMaxRatio( src.m_dopplerMaxRatio )
    , m_dopplerSmoothingInMs( src.m_dopplerSmoothingInMs )
    , m_defaultAttenuation( src.m_defaultAttenuation )
    , m_useCenterSpeaker( src.m_useCenterSpeaker )
    , m_lfeSendInDecibels( src.m_lfeSendInDecibels )
    , m_rearAttenuationInDecibels( src.m_rearAttenuationInDecibels )
    , m_spatializerName( src.m_spatializerName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioSpatialParams&
CAudioSpatialParams::operator=( const CAudioSpatialParams& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_speedOfSound = src.m_speedOfSound;
        m_unitsPerMeter = src.m_unitsPerMeter;
        m_dopplerFactor = src.m_dopplerFactor;
        m_dopplerMinRatio = src.m_dopplerMinRatio;
        m_dopplerMaxRatio = src.m_dopplerMaxRatio;
        m_dopplerSmoothingInMs = src.m_dopplerSmoothingInMs;
        m_defaultAttenuation = src.m_defaultAttenuation;
        m_useCenterSpeaker = src.m_useCenterSpeaker;
        m_lfeSendInDecibels = src.m_lfeSendInDecibels;
        m_rearAttenuationInDecibels = src.m_rearAttenuationInDecibels;
        m_spatializerName = src.m_spatializerName;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CAudioSpatialParams::~CAudioSpatialParams()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAudioSpatialParams::operator==( const CAudioSpatialParams& other ) const
{GUCEF_TRACE;

    return m_speedOfSound == other.m_speedOfSound && m_unitsPerMeter == other.m_unitsPerMeter &&
           m_dopplerFactor == other.m_dopplerFactor && m_dopplerMinRatio == other.m_dopplerMinRatio &&
           m_dopplerMaxRatio == other.m_dopplerMaxRatio && m_dopplerSmoothingInMs == other.m_dopplerSmoothingInMs &&
           m_defaultAttenuation == other.m_defaultAttenuation && m_useCenterSpeaker == other.m_useCenterSpeaker &&
           m_lfeSendInDecibels == other.m_lfeSendInDecibels && m_rearAttenuationInDecibels == other.m_rearAttenuationInDecibels &&
           m_spatializerName == other.m_spatializerName;
}

/*-------------------------------------------------------------------------*/

bool
CAudioSpatialParams::operator!=( const CAudioSpatialParams& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialParams::GetSpeedOfSound( void ) const
{GUCEF_TRACE;

    return m_speedOfSound;
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatialParams::SetSpeedOfSound( Float32 metersPerSecond )
{GUCEF_TRACE;

    m_speedOfSound = Limited( metersPerSecond, 1.0f, 100000.0f, m_speedOfSound );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialParams::GetUnitsPerMeter( void ) const
{GUCEF_TRACE;

    return m_unitsPerMeter;
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatialParams::SetUnitsPerMeter( Float32 unitsPerMeter )
{GUCEF_TRACE;

    m_unitsPerMeter = Limited( unitsPerMeter, 0.0001f, 100000.0f, m_unitsPerMeter );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialParams::GetSpeedOfSoundInUnits( void ) const
{GUCEF_TRACE;

    return m_speedOfSound * m_unitsPerMeter;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialParams::GetDopplerFactor( void ) const
{GUCEF_TRACE;

    return m_dopplerFactor;
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatialParams::SetDopplerFactor( Float32 factor )
{GUCEF_TRACE;

    m_dopplerFactor = Limited( factor, 0.0f, 10.0f, m_dopplerFactor );
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialParams::GetDopplerMinRatio( void ) const
{GUCEF_TRACE;

    return m_dopplerMinRatio;
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatialParams::SetDopplerMinRatio( Float32 ratio )
{GUCEF_TRACE;

    m_dopplerMinRatio = Limited( ratio, 0.01f, 100.0f, m_dopplerMinRatio );
    if ( m_dopplerMaxRatio < m_dopplerMinRatio )
        m_dopplerMaxRatio = m_dopplerMinRatio;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialParams::GetDopplerMaxRatio( void ) const
{GUCEF_TRACE;

    return m_dopplerMaxRatio;
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatialParams::SetDopplerMaxRatio( Float32 ratio )
{GUCEF_TRACE;

    m_dopplerMaxRatio = Limited( ratio, 0.01f, 100.0f, m_dopplerMaxRatio );
    if ( m_dopplerMinRatio > m_dopplerMaxRatio )
        m_dopplerMinRatio = m_dopplerMaxRatio;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialParams::GetDopplerSmoothingInMs( void ) const
{GUCEF_TRACE;

    return m_dopplerSmoothingInMs;
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatialParams::SetDopplerSmoothingInMs( Float32 milliseconds )
{GUCEF_TRACE;

    m_dopplerSmoothingInMs = Limited( milliseconds, 0.0f, 1000.0f, m_dopplerSmoothingInMs );
}

/*-------------------------------------------------------------------------*/

const CAudioAttenuation&
CAudioSpatialParams::GetDefaultAttenuation( void ) const
{GUCEF_TRACE;

    return m_defaultAttenuation;
}

/*-------------------------------------------------------------------------*/

CAudioAttenuation&
CAudioSpatialParams::GetDefaultAttenuation( void )
{GUCEF_TRACE;

    return m_defaultAttenuation;
}

/*-------------------------------------------------------------------------*/

bool
CAudioSpatialParams::GetUseCenterSpeaker( void ) const
{GUCEF_TRACE;

    return m_useCenterSpeaker;
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatialParams::SetUseCenterSpeaker( bool useCenter )
{GUCEF_TRACE;

    m_useCenterSpeaker = useCenter;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialParams::GetLfeSendInDecibels( void ) const
{GUCEF_TRACE;

    return m_lfeSendInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatialParams::SetLfeSendInDecibels( Float32 decibels )
{GUCEF_TRACE;

    m_lfeSendInDecibels = Limited( decibels, LfeSendOffInDecibels, 12.0f, m_lfeSendInDecibels );
}

/*-------------------------------------------------------------------------*/

bool
CAudioSpatialParams::IsLfeSendEnabled( void ) const
{GUCEF_TRACE;

    return m_lfeSendInDecibels > LfeSendOffInDecibels;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioSpatialParams::GetRearAttenuationInDecibels( void ) const
{GUCEF_TRACE;

    return m_rearAttenuationInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatialParams::SetRearAttenuationInDecibels( Float32 decibels )
{GUCEF_TRACE;

    m_rearAttenuationInDecibels = Limited( decibels, -60.0f, 0.0f, m_rearAttenuationInDecibels );
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioSpatialParams::GetSpatializerName( void ) const
{GUCEF_TRACE;

    return m_spatializerName;
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatialParams::SetSpatializerName( const CString& name )
{GUCEF_TRACE;

    m_spatializerName = name;
}

/*-------------------------------------------------------------------------*/

void
CAudioSpatialParams::GetWorldState( SSpatialWorldState& state ) const
{GUCEF_TRACE;

    state.speedOfSoundInUnits = GetSpeedOfSoundInUnits();
    state.dopplerFactor = m_dopplerFactor;
    state.dopplerMinRatio = m_dopplerMinRatio;
    state.dopplerMaxRatio = m_dopplerMaxRatio;
    state.dopplerSmoothingInMs = m_dopplerSmoothingInMs;
    state.attenuation = m_defaultAttenuation.GetEvaluator();
    state.useCenterSpeaker = m_useCenterSpeaker;
    state.lfeSendGain = IsLfeSendEnabled() ? MATH::DecibelsToGain( m_lfeSendInDecibels ) : 0.0f;
    state.rearAttenuationGain = MATH::DecibelsToGain( m_rearAttenuationInDecibels );
}

/*-------------------------------------------------------------------------*/

bool
CAudioSpatialParams::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "speedOfSound", m_speedOfSound );
    section->SetAttribute( "unitsPerMeter", m_unitsPerMeter );
    section->SetAttribute( "dopplerFactor", m_dopplerFactor );
    section->SetAttribute( "dopplerMinRatio", m_dopplerMinRatio );
    section->SetAttribute( "dopplerMaxRatio", m_dopplerMaxRatio );
    section->SetAttribute( "dopplerSmoothingMs", m_dopplerSmoothingInMs );
    section->SetAttribute( "spatializer", m_spatializerName );
    section->SetAttribute( "useCenterSpeaker", m_useCenterSpeaker );
    section->SetAttribute( "lfeSendDb", m_lfeSendInDecibels );
    section->SetAttribute( "rearAttenuationDb", m_rearAttenuationInDecibels );
    return m_defaultAttenuation.SaveConfig( *section );
}

/*-------------------------------------------------------------------------*/

bool
CAudioSpatialParams::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    SetSpeedOfSound( section->GetAttributeValueOrChildValueByName( "speedOfSound" ).AsFloat32( m_speedOfSound, true ) );
    SetUnitsPerMeter( section->GetAttributeValueOrChildValueByName( "unitsPerMeter" ).AsFloat32( m_unitsPerMeter, true ) );
    SetDopplerFactor( section->GetAttributeValueOrChildValueByName( "dopplerFactor" ).AsFloat32( m_dopplerFactor, true ) );

    // both limits as a pair: the one that was read last must not move the other one away from what the section says
    Float32 minRatio = Limited( section->GetAttributeValueOrChildValueByName( "dopplerMinRatio" ).AsFloat32( m_dopplerMinRatio, true ), 0.01f, 100.0f, m_dopplerMinRatio );
    Float32 maxRatio = Limited( section->GetAttributeValueOrChildValueByName( "dopplerMaxRatio" ).AsFloat32( m_dopplerMaxRatio, true ), 0.01f, 100.0f, m_dopplerMaxRatio );
    if ( minRatio > maxRatio )
        minRatio = maxRatio;
    m_dopplerMinRatio = minRatio;
    m_dopplerMaxRatio = maxRatio;

    SetDopplerSmoothingInMs( section->GetAttributeValueOrChildValueByName( "dopplerSmoothingMs" ).AsFloat32( m_dopplerSmoothingInMs, true ) );

    const CORE::CVariant spatializer = section->GetAttributeValueOrChildValueByName( "spatializer" );
    if ( spatializer.IsInitialized() && !spatializer.IsNULL() )
        m_spatializerName = spatializer.AsString( CString::Empty, true );
    m_useCenterSpeaker = section->GetAttributeValueOrChildValueByName( "useCenterSpeaker" ).AsBool( m_useCenterSpeaker, true );
    SetLfeSendInDecibels( section->GetAttributeValueOrChildValueByName( "lfeSendDb" ).AsFloat32( m_lfeSendInDecibels, true ) );
    SetRearAttenuationInDecibels( section->GetAttributeValueOrChildValueByName( "rearAttenuationDb" ).AsFloat32( m_rearAttenuationInDecibels, true ) );
    m_defaultAttenuation.LoadConfig( *section );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioSpatialParams::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::AUDIO::CAudioSpatialParams";
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
