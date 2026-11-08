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

#include "gucefAUDIO_CVoiceActivitySettings.h"

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

const CString CVoiceActivitySettings::ConfigSectionName = "VoiceActivity";
const Float32 CVoiceActivitySettings::DefaultOnsetInDecibels = 9.0f;
const Float32 CVoiceActivitySettings::DefaultReleaseInDecibels = 6.0f;
const Float32 CVoiceActivitySettings::DefaultHangoverInMs = 200.0f;
const Float32 CVoiceActivitySettings::DefaultMinimumLevelInDecibels = -65.0f;
const Float32 CVoiceActivitySettings::DefaultFloorRiseInDecibelsPerSecond = 3.0f;

/*-------------------------------------------------------------------------*/

CVoiceActivitySettings::CVoiceActivitySettings( void )
    : CORE::CIConfigurable()
    , m_onsetInDecibels( DefaultOnsetInDecibels )
    , m_releaseInDecibels( DefaultReleaseInDecibels )
    , m_hangoverInMs( DefaultHangoverInMs )
    , m_minimumLevelInDecibels( DefaultMinimumLevelInDecibels )
    , m_floorRiseInDecibelsPerSecond( DefaultFloorRiseInDecibelsPerSecond )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoiceActivitySettings::CVoiceActivitySettings( const CVoiceActivitySettings& src )
    : CORE::CIConfigurable( src )
    , m_onsetInDecibels( src.m_onsetInDecibels )
    , m_releaseInDecibels( src.m_releaseInDecibels )
    , m_hangoverInMs( src.m_hangoverInMs )
    , m_minimumLevelInDecibels( src.m_minimumLevelInDecibels )
    , m_floorRiseInDecibelsPerSecond( src.m_floorRiseInDecibelsPerSecond )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoiceActivitySettings&
CVoiceActivitySettings::operator=( const CVoiceActivitySettings& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_onsetInDecibels = src.m_onsetInDecibels;
        m_releaseInDecibels = src.m_releaseInDecibels;
        m_hangoverInMs = src.m_hangoverInMs;
        m_minimumLevelInDecibels = src.m_minimumLevelInDecibels;
        m_floorRiseInDecibelsPerSecond = src.m_floorRiseInDecibelsPerSecond;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVoiceActivitySettings::~CVoiceActivitySettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CVoiceActivitySettings::operator==( const CVoiceActivitySettings& other ) const
{GUCEF_TRACE;

    return m_onsetInDecibels == other.m_onsetInDecibels && m_releaseInDecibels == other.m_releaseInDecibels &&
           m_hangoverInMs == other.m_hangoverInMs && m_minimumLevelInDecibels == other.m_minimumLevelInDecibels &&
           m_floorRiseInDecibelsPerSecond == other.m_floorRiseInDecibelsPerSecond;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceActivitySettings::operator!=( const CVoiceActivitySettings& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

Float32
CVoiceActivitySettings::GetOnsetInDecibels( void ) const
{GUCEF_TRACE;

    return m_onsetInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CVoiceActivitySettings::SetOnsetInDecibels( Float32 decibels )
{GUCEF_TRACE;

    m_onsetInDecibels = Limited( decibels, 0.0f, 60.0f, m_onsetInDecibels );
    if ( m_releaseInDecibels > m_onsetInDecibels )
        m_releaseInDecibels = m_onsetInDecibels;
}

/*-------------------------------------------------------------------------*/

Float32
CVoiceActivitySettings::GetReleaseInDecibels( void ) const
{GUCEF_TRACE;

    return m_releaseInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CVoiceActivitySettings::SetReleaseInDecibels( Float32 decibels )
{GUCEF_TRACE;

    m_releaseInDecibels = Limited( decibels, 0.0f, m_onsetInDecibels, m_releaseInDecibels );
}

/*-------------------------------------------------------------------------*/

Float32
CVoiceActivitySettings::GetHangoverInMs( void ) const
{GUCEF_TRACE;

    return m_hangoverInMs;
}

/*-------------------------------------------------------------------------*/

void
CVoiceActivitySettings::SetHangoverInMs( Float32 milliseconds )
{GUCEF_TRACE;

    m_hangoverInMs = Limited( milliseconds, 0.0f, 5000.0f, m_hangoverInMs );
}

/*-------------------------------------------------------------------------*/

Float32
CVoiceActivitySettings::GetMinimumLevelInDecibels( void ) const
{GUCEF_TRACE;

    return m_minimumLevelInDecibels;
}

/*-------------------------------------------------------------------------*/

void
CVoiceActivitySettings::SetMinimumLevelInDecibels( Float32 decibels )
{GUCEF_TRACE;

    m_minimumLevelInDecibels = Limited( decibels, -120.0f, 0.0f, m_minimumLevelInDecibels );
}

/*-------------------------------------------------------------------------*/

Float32
CVoiceActivitySettings::GetFloorRiseInDecibelsPerSecond( void ) const
{GUCEF_TRACE;

    return m_floorRiseInDecibelsPerSecond;
}

/*-------------------------------------------------------------------------*/

void
CVoiceActivitySettings::SetFloorRiseInDecibelsPerSecond( Float32 decibelsPerSecond )
{GUCEF_TRACE;

    m_floorRiseInDecibelsPerSecond = Limited( decibelsPerSecond, 0.1f, 60.0f, m_floorRiseInDecibelsPerSecond );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceActivitySettings::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "onsetDb", m_onsetInDecibels );
    section->SetAttribute( "releaseDb", m_releaseInDecibels );
    section->SetAttribute( "hangoverMs", m_hangoverInMs );
    section->SetAttribute( "minimumLevelDb", m_minimumLevelInDecibels );
    section->SetAttribute( "floorRiseDbPerSecond", m_floorRiseInDecibelsPerSecond );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceActivitySettings::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    // the onset first: the release is limited by it
    SetOnsetInDecibels( section->GetAttributeValueOrChildValueByName( "onsetDb" ).AsFloat32( m_onsetInDecibels, true ) );
    SetReleaseInDecibels( section->GetAttributeValueOrChildValueByName( "releaseDb" ).AsFloat32( m_releaseInDecibels, true ) );
    SetHangoverInMs( section->GetAttributeValueOrChildValueByName( "hangoverMs" ).AsFloat32( m_hangoverInMs, true ) );
    SetMinimumLevelInDecibels( section->GetAttributeValueOrChildValueByName( "minimumLevelDb" ).AsFloat32( m_minimumLevelInDecibels, true ) );
    SetFloorRiseInDecibelsPerSecond( section->GetAttributeValueOrChildValueByName( "floorRiseDbPerSecond" ).AsFloat32( m_floorRiseInDecibelsPerSecond, true ) );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CVoiceActivitySettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::AUDIO::CVoiceActivitySettings";
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
