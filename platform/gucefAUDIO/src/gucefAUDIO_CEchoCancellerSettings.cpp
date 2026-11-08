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

#include "gucefAUDIO_CEchoCancellerSettings.h"

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

const CString CEchoCancellerSettings::ConfigSectionName = "EchoCanceller";

/*-------------------------------------------------------------------------*/

CEchoCancellerSettings::CEchoCancellerSettings( void )
    : CORE::CIConfigurable()
    , m_enabled( true )
    , m_filterLengthInMs( 128.0f )
    , m_blockFrames( 0 )
    , m_adaptEveryBlocks( 1 )
    , m_residualSuppression( true )
    , m_echoSuppressionInDecibels( -40.0f )
    , m_residualStrength( 2.0f )
    , m_suppressNoise( false )
    , m_noiseSuppressionInDecibels( -15.0f )
    , m_delayEstimation( true )
    , m_maxDelayInMs( 250.0f )
    , m_farThresholdInDecibels( -70.0f )
    , m_driftCompensation( true )
    , m_convergedErleInDecibels( 12.0f )
    , m_referenceMarginInMs( 0.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEchoCancellerSettings::CEchoCancellerSettings( const CEchoCancellerSettings& src )
    : CORE::CIConfigurable( src )
    , m_enabled( src.m_enabled )
    , m_filterLengthInMs( src.m_filterLengthInMs )
    , m_blockFrames( src.m_blockFrames )
    , m_adaptEveryBlocks( src.m_adaptEveryBlocks )
    , m_residualSuppression( src.m_residualSuppression )
    , m_echoSuppressionInDecibels( src.m_echoSuppressionInDecibels )
    , m_residualStrength( src.m_residualStrength )
    , m_suppressNoise( src.m_suppressNoise )
    , m_noiseSuppressionInDecibels( src.m_noiseSuppressionInDecibels )
    , m_delayEstimation( src.m_delayEstimation )
    , m_maxDelayInMs( src.m_maxDelayInMs )
    , m_farThresholdInDecibels( src.m_farThresholdInDecibels )
    , m_driftCompensation( src.m_driftCompensation )
    , m_convergedErleInDecibels( src.m_convergedErleInDecibels )
    , m_referenceMarginInMs( src.m_referenceMarginInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEchoCancellerSettings&
CEchoCancellerSettings::operator=( const CEchoCancellerSettings& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_enabled = src.m_enabled;
        m_filterLengthInMs = src.m_filterLengthInMs;
        m_blockFrames = src.m_blockFrames;
        m_adaptEveryBlocks = src.m_adaptEveryBlocks;
        m_residualSuppression = src.m_residualSuppression;
        m_echoSuppressionInDecibels = src.m_echoSuppressionInDecibels;
        m_residualStrength = src.m_residualStrength;
        m_suppressNoise = src.m_suppressNoise;
        m_noiseSuppressionInDecibels = src.m_noiseSuppressionInDecibels;
        m_delayEstimation = src.m_delayEstimation;
        m_maxDelayInMs = src.m_maxDelayInMs;
        m_farThresholdInDecibels = src.m_farThresholdInDecibels;
        m_driftCompensation = src.m_driftCompensation;
        m_convergedErleInDecibels = src.m_convergedErleInDecibels;
        m_referenceMarginInMs = src.m_referenceMarginInMs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CEchoCancellerSettings::~CEchoCancellerSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CEchoCancellerSettings::operator==( const CEchoCancellerSettings& other ) const
{GUCEF_TRACE;

    return m_enabled == other.m_enabled &&
           m_filterLengthInMs == other.m_filterLengthInMs &&
           m_blockFrames == other.m_blockFrames &&
           m_adaptEveryBlocks == other.m_adaptEveryBlocks &&
           m_residualSuppression == other.m_residualSuppression &&
           m_echoSuppressionInDecibels == other.m_echoSuppressionInDecibels &&
           m_residualStrength == other.m_residualStrength &&
           m_suppressNoise == other.m_suppressNoise &&
           m_noiseSuppressionInDecibels == other.m_noiseSuppressionInDecibels &&
           m_delayEstimation == other.m_delayEstimation &&
           m_maxDelayInMs == other.m_maxDelayInMs &&
           m_farThresholdInDecibels == other.m_farThresholdInDecibels &&
           m_driftCompensation == other.m_driftCompensation &&
           m_convergedErleInDecibels == other.m_convergedErleInDecibels &&
           m_referenceMarginInMs == other.m_referenceMarginInMs;
}

/*-------------------------------------------------------------------------*/

bool
CEchoCancellerSettings::operator!=( const CEchoCancellerSettings& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CEchoCancellerSettings::GetEnabled( void ) const
{GUCEF_TRACE;

    return m_enabled;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetEnabled( bool enabled )
{GUCEF_TRACE;

    m_enabled = enabled;
}

/*-------------------------------------------------------------------------*/

Float32
CEchoCancellerSettings::GetFilterLengthInMs( void ) const
{GUCEF_TRACE;

    return m_filterLengthInMs;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetFilterLengthInMs( Float32 value )
{GUCEF_TRACE;

    m_filterLengthInMs = Limited( value, 16.0f, 512.0f, m_filterLengthInMs );
}

/*-------------------------------------------------------------------------*/

UInt32
CEchoCancellerSettings::GetBlockFrames( void ) const
{GUCEF_TRACE;

    return m_blockFrames;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetBlockFrames( UInt32 value )
{GUCEF_TRACE;

    // 0 (automatic) or a power of two from 32 to 1024; anything else goes to the nearest one below
    if ( 0 == value )
    {
        m_blockFrames = 0;
        return;
    }
    UInt32 frames = 32;
    while ( frames * 2 <= value && frames < 1024 )
        frames *= 2;
    m_blockFrames = frames;
}

/*-------------------------------------------------------------------------*/

UInt32
CEchoCancellerSettings::GetAdaptEveryBlocks( void ) const
{GUCEF_TRACE;

    return m_adaptEveryBlocks;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetAdaptEveryBlocks( UInt32 value )
{GUCEF_TRACE;

    m_adaptEveryBlocks = value < 1 ? 1 : ( value > 4 ? 4 : value );
}

/*-------------------------------------------------------------------------*/

bool
CEchoCancellerSettings::GetResidualSuppression( void ) const
{GUCEF_TRACE;

    return m_residualSuppression;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetResidualSuppression( bool enabled )
{GUCEF_TRACE;

    m_residualSuppression = enabled;
}

/*-------------------------------------------------------------------------*/

Float32
CEchoCancellerSettings::GetEchoSuppressionInDecibels( void ) const
{GUCEF_TRACE;

    return m_echoSuppressionInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetEchoSuppressionInDecibels( Float32 value )
{GUCEF_TRACE;

    m_echoSuppressionInDecibels = Limited( value, -80.0f, 0.0f, m_echoSuppressionInDecibels );
}

/*-------------------------------------------------------------------------*/

Float32
CEchoCancellerSettings::GetResidualStrength( void ) const
{GUCEF_TRACE;

    return m_residualStrength;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetResidualStrength( Float32 value )
{GUCEF_TRACE;

    m_residualStrength = Limited( value, 0.25f, 8.0f, m_residualStrength );
}

/*-------------------------------------------------------------------------*/

bool
CEchoCancellerSettings::GetSuppressNoise( void ) const
{GUCEF_TRACE;

    return m_suppressNoise;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetSuppressNoise( bool enabled )
{GUCEF_TRACE;

    m_suppressNoise = enabled;
}

/*-------------------------------------------------------------------------*/

Float32
CEchoCancellerSettings::GetNoiseSuppressionInDecibels( void ) const
{GUCEF_TRACE;

    return m_noiseSuppressionInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetNoiseSuppressionInDecibels( Float32 value )
{GUCEF_TRACE;

    m_noiseSuppressionInDecibels = Limited( value, -40.0f, 0.0f, m_noiseSuppressionInDecibels );
}

/*-------------------------------------------------------------------------*/

bool
CEchoCancellerSettings::GetDelayEstimation( void ) const
{GUCEF_TRACE;

    return m_delayEstimation;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetDelayEstimation( bool enabled )
{GUCEF_TRACE;

    m_delayEstimation = enabled;
}

/*-------------------------------------------------------------------------*/

Float32
CEchoCancellerSettings::GetMaxDelayInMs( void ) const
{GUCEF_TRACE;

    return m_maxDelayInMs;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetMaxDelayInMs( Float32 value )
{GUCEF_TRACE;

    m_maxDelayInMs = Limited( value, 10.0f, 1000.0f, m_maxDelayInMs );
}

/*-------------------------------------------------------------------------*/

Float32
CEchoCancellerSettings::GetFarThresholdInDecibels( void ) const
{GUCEF_TRACE;

    return m_farThresholdInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetFarThresholdInDecibels( Float32 value )
{GUCEF_TRACE;

    m_farThresholdInDecibels = Limited( value, -100.0f, -20.0f, m_farThresholdInDecibels );
}

/*-------------------------------------------------------------------------*/

bool
CEchoCancellerSettings::GetDriftCompensation( void ) const
{GUCEF_TRACE;

    return m_driftCompensation;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetDriftCompensation( bool enabled )
{GUCEF_TRACE;

    m_driftCompensation = enabled;
}

/*-------------------------------------------------------------------------*/

Float32
CEchoCancellerSettings::GetConvergedErleInDecibels( void ) const
{GUCEF_TRACE;

    return m_convergedErleInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetConvergedErleInDecibels( Float32 value )
{GUCEF_TRACE;

    m_convergedErleInDecibels = Limited( value, 3.0f, 40.0f, m_convergedErleInDecibels );
}

/*-------------------------------------------------------------------------*/

Float32
CEchoCancellerSettings::GetReferenceMarginInMs( void ) const
{GUCEF_TRACE;

    return m_referenceMarginInMs;
}

/*-------------------------------------------------------------------------*/

void
CEchoCancellerSettings::SetReferenceMarginInMs( Float32 value )
{GUCEF_TRACE;

    m_referenceMarginInMs = Limited( value, 0.0f, 100.0f, m_referenceMarginInMs );
}

/*-------------------------------------------------------------------------*/

bool
CEchoCancellerSettings::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "enabled", m_enabled );
    section->SetAttribute( "filterLengthMs", m_filterLengthInMs );
    section->SetAttribute( "blockFrames", (UInt32) m_blockFrames );
    section->SetAttribute( "adaptEveryBlocks", (UInt32) m_adaptEveryBlocks );
    section->SetAttribute( "residualSuppression", m_residualSuppression );
    section->SetAttribute( "echoSuppressionDb", m_echoSuppressionInDecibels );
    section->SetAttribute( "residualStrength", m_residualStrength );
    section->SetAttribute( "suppressNoise", m_suppressNoise );
    section->SetAttribute( "noiseSuppressionDb", m_noiseSuppressionInDecibels );
    section->SetAttribute( "delayEstimation", m_delayEstimation );
    section->SetAttribute( "maxDelayMs", m_maxDelayInMs );
    section->SetAttribute( "farThresholdDb", m_farThresholdInDecibels );
    section->SetAttribute( "driftCompensation", m_driftCompensation );
    section->SetAttribute( "convergedErleDb", m_convergedErleInDecibels );
    section->SetAttribute( "referenceMarginMs", m_referenceMarginInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEchoCancellerSettings::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    SetEnabled( section->GetAttributeValueOrChildValueByName( "enabled" ).AsBool( m_enabled, true ) );
    SetFilterLengthInMs( section->GetAttributeValueOrChildValueByName( "filterLengthMs" ).AsFloat32( m_filterLengthInMs, true ) );
    SetBlockFrames( section->GetAttributeValueOrChildValueByName( "blockFrames" ).AsUInt32( m_blockFrames, true ) );
    SetAdaptEveryBlocks( section->GetAttributeValueOrChildValueByName( "adaptEveryBlocks" ).AsUInt32( m_adaptEveryBlocks, true ) );
    SetResidualSuppression( section->GetAttributeValueOrChildValueByName( "residualSuppression" ).AsBool( m_residualSuppression, true ) );
    SetEchoSuppressionInDecibels( section->GetAttributeValueOrChildValueByName( "echoSuppressionDb" ).AsFloat32( m_echoSuppressionInDecibels, true ) );
    SetResidualStrength( section->GetAttributeValueOrChildValueByName( "residualStrength" ).AsFloat32( m_residualStrength, true ) );
    SetSuppressNoise( section->GetAttributeValueOrChildValueByName( "suppressNoise" ).AsBool( m_suppressNoise, true ) );
    SetNoiseSuppressionInDecibels( section->GetAttributeValueOrChildValueByName( "noiseSuppressionDb" ).AsFloat32( m_noiseSuppressionInDecibels, true ) );
    SetDelayEstimation( section->GetAttributeValueOrChildValueByName( "delayEstimation" ).AsBool( m_delayEstimation, true ) );
    SetMaxDelayInMs( section->GetAttributeValueOrChildValueByName( "maxDelayMs" ).AsFloat32( m_maxDelayInMs, true ) );
    SetFarThresholdInDecibels( section->GetAttributeValueOrChildValueByName( "farThresholdDb" ).AsFloat32( m_farThresholdInDecibels, true ) );
    SetDriftCompensation( section->GetAttributeValueOrChildValueByName( "driftCompensation" ).AsBool( m_driftCompensation, true ) );
    SetConvergedErleInDecibels( section->GetAttributeValueOrChildValueByName( "convergedErleDb" ).AsFloat32( m_convergedErleInDecibels, true ) );
    SetReferenceMarginInMs( section->GetAttributeValueOrChildValueByName( "referenceMarginMs" ).AsFloat32( m_referenceMarginInMs, true ) );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CEchoCancellerSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::AUDIO::CEchoCancellerSettings";
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
