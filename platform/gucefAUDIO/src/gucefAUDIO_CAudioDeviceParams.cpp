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

#include "gucefAUDIO_CAudioDeviceParams.h"

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOSETTINGS_H
#include "gucefAUDIO_CAudioSettings.h"
#define GUCEF_AUDIO_CAUDIOSETTINGS_H
#endif /* GUCEF_AUDIO_CAUDIOSETTINGS_H ? */

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

/** Reads a text value when the section has it, an empty text included: saved as empty it has to load as empty */
void
LoadText( const CORE::CDataNode& section, const char* name, CString& target )
{
    const CORE::CVariant value = section.GetAttributeValueOrChildValueByName( name );
    if ( value.IsInitialized() && !value.IsNULL() )
        target = value.AsString( CString::Empty, true );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CAudioDeviceParams::ConfigSectionName = "AudioDevice";

/*-------------------------------------------------------------------------*/

CAudioDeviceParams::CAudioDeviceParams( void )
    : CORE::CIConfigurable()
    , m_deviceId()
    , m_direction( AUDIODIRECTION_PLAYBACK )
    , m_preferredFormat()
    , m_bufferSizeInFrames( 0 )
    , m_shareMode( AUDIOSHAREMODE_SHARED )
    , m_options()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioDeviceParams::CAudioDeviceParams( const CAudioDeviceParams& src )
    : CORE::CIConfigurable( src )
    , m_deviceId( src.m_deviceId )
    , m_direction( src.m_direction )
    , m_preferredFormat( src.m_preferredFormat )
    , m_bufferSizeInFrames( src.m_bufferSizeInFrames )
    , m_shareMode( src.m_shareMode )
    , m_options( src.m_options )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioDeviceParams&
CAudioDeviceParams::operator=( const CAudioDeviceParams& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_deviceId = src.m_deviceId;
        m_direction = src.m_direction;
        m_preferredFormat = src.m_preferredFormat;
        m_bufferSizeInFrames = src.m_bufferSizeInFrames;
        m_shareMode = src.m_shareMode;
        m_options = src.m_options;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CAudioDeviceParams::~CAudioDeviceParams()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CAudioDeviceParams::GetDeviceId( void ) const
{GUCEF_TRACE;

    return m_deviceId;
}

/*-------------------------------------------------------------------------*/

void
CAudioDeviceParams::SetDeviceId( const CString& deviceId )
{GUCEF_TRACE;

    m_deviceId = deviceId;
}

/*-------------------------------------------------------------------------*/

TAudioDirection
CAudioDeviceParams::GetDirection( void ) const
{GUCEF_TRACE;

    return m_direction;
}

/*-------------------------------------------------------------------------*/

void
CAudioDeviceParams::SetDirection( TAudioDirection direction )
{GUCEF_TRACE;

    m_direction = direction;
}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CAudioDeviceParams::GetPreferredFormat( void ) const
{GUCEF_TRACE;

    return m_preferredFormat;
}

/*-------------------------------------------------------------------------*/

void
CAudioDeviceParams::SetPreferredFormat( const CAudioFormat& format )
{GUCEF_TRACE;

    m_preferredFormat = format;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioDeviceParams::GetBufferSizeInFrames( void ) const
{GUCEF_TRACE;

    return m_bufferSizeInFrames;
}

/*-------------------------------------------------------------------------*/

void
CAudioDeviceParams::SetBufferSizeInFrames( UInt32 frames )
{GUCEF_TRACE;

    m_bufferSizeInFrames = frames;
}

/*-------------------------------------------------------------------------*/

TAudioShareMode
CAudioDeviceParams::GetShareMode( void ) const
{GUCEF_TRACE;

    return m_shareMode;
}

/*-------------------------------------------------------------------------*/

void
CAudioDeviceParams::SetShareMode( TAudioShareMode shareMode )
{GUCEF_TRACE;

    m_shareMode = shareMode;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
CAudioDeviceParams::GetOptions( void ) const
{GUCEF_TRACE;

    return m_options;
}

/*-------------------------------------------------------------------------*/

CORE::CValueList&
CAudioDeviceParams::GetOptions( void )
{GUCEF_TRACE;

    return m_options;
}


/*-------------------------------------------------------------------------*/

const char*
CAudioDeviceParams::ShareModeToString( TAudioShareMode shareMode )
{GUCEF_TRACE;

    return AUDIOSHAREMODE_EXCLUSIVE == shareMode ? "exclusive" : "shared";
}

/*-------------------------------------------------------------------------*/

bool
CAudioDeviceParams::StringToShareMode( const CString& text, TAudioShareMode& shareMode )
{GUCEF_TRACE;

    const CString name = text.Lowercase();
    if ( name == "shared" )         { shareMode = AUDIOSHAREMODE_SHARED; return true; }
    if ( name == "exclusive" )      { shareMode = AUDIOSHAREMODE_EXCLUSIVE; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

const char*
CAudioDeviceParams::DirectionToString( TAudioDirection direction )
{GUCEF_TRACE;

    return AUDIODIRECTION_CAPTURE == direction ? "capture" : "playback";
}

/*-------------------------------------------------------------------------*/

bool
CAudioDeviceParams::StringToDirection( const CString& text, TAudioDirection& direction )
{GUCEF_TRACE;

    const CString name = text.Lowercase();
    if ( name == "playback" )       { direction = AUDIODIRECTION_PLAYBACK; return true; }
    if ( name == "capture" )        { direction = AUDIODIRECTION_CAPTURE; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAudioDeviceParams::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "id", m_deviceId );
    section->SetAttribute( "direction", CString( DirectionToString( m_direction ) ) );
    section->SetAttribute( "shareMode", CString( ShareModeToString( m_shareMode ) ) );
    section->SetAttribute( "bufferFrames", m_bufferSizeInFrames );
    section->SetAttribute( "sampleRate", m_preferredFormat.GetSampleRate() );
    section->SetAttribute( "channels", m_preferredFormat.GetChannelCount() );
    section->SetAttribute( "sampleType", CString( CAudioFormat::SampleTypeToString( m_preferredFormat.GetSampleType() ) ) );
    section->SetAttribute( "speakerLayout", CAudioSettings::ChannelLayoutToString( m_preferredFormat.HasExplicitChannelLayout() ? m_preferredFormat.GetChannelLayout() : 0 ) );

    section->DelChild( "Options" );
    if ( m_options.GetKeyCount() > 0 )
    {
        CORE::CDataNode* options = section->AddChild( "Options" );
        if ( GUCEF_NULL == options )
            return false;
        for ( UInt32 i=0; i<m_options.GetKeyCount(); ++i )
        {
            const CString key = m_options.GetKey( i ).AsString();
            options->SetAttribute( key, m_options.GetValueAlways( key, "" ) );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioDeviceParams::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    LoadText( *section, "id", m_deviceId );

    const CString direction = section->GetAttributeValueOrChildValueByName( "direction" ).AsString( CString::Empty, true );
    if ( !direction.IsNULLOrEmpty() && !StringToDirection( direction, m_direction ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioDeviceParams: Ignoring the unknown direction \"" << direction << "\"" );
    }

    const CString shareMode = section->GetAttributeValueOrChildValueByName( "shareMode" ).AsString( CString::Empty, true );
    if ( !shareMode.IsNULLOrEmpty() && !StringToShareMode( shareMode, m_shareMode ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioDeviceParams: Ignoring the unknown share mode \"" << shareMode << "\"" );
    }

    m_bufferSizeInFrames = section->GetAttributeValueOrChildValueByName( "bufferFrames" ).AsUInt32( m_bufferSizeInFrames, true );

    // the preferred format: what is not named stays
    UInt32 sampleRate = m_preferredFormat.GetSampleRate();
    UInt32 channels = m_preferredFormat.GetChannelCount();
    TSampleType sampleType = m_preferredFormat.GetSampleType();
    UInt32 layout = m_preferredFormat.HasExplicitChannelLayout() ? m_preferredFormat.GetChannelLayout() : 0;
    sampleRate = section->GetAttributeValueOrChildValueByName( "sampleRate" ).AsUInt32( sampleRate, true );
    channels = section->GetAttributeValueOrChildValueByName( "channels" ).AsUInt32( channels, true );

    const CString sampleTypeName = section->GetAttributeValueOrChildValueByName( "sampleType" ).AsString( CString::Empty, true );
    if ( !sampleTypeName.IsNULLOrEmpty() )
    {
        const TSampleType parsed = CAudioFormat::SampleTypeFromString( sampleTypeName );
        if ( SAMPLETYPE_UNKNOWN != parsed || sampleTypeName.Lowercase() == "unknown" )
            sampleType = parsed;
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioDeviceParams: Ignoring the unknown sample type \"" << sampleTypeName << "\"" );
        }
    }

    const CString layoutName = section->GetAttributeValueOrChildValueByName( "speakerLayout" ).AsString( CString::Empty, true );
    if ( !layoutName.IsNULLOrEmpty() )
    {
        const UInt32 unknown = 0xFFFFFFFFU;
        const UInt32 parsed = CAudioSettings::StringToChannelLayout( layoutName, unknown );
        if ( unknown == parsed )
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AudioDeviceParams: Ignoring the unknown speaker layout \"" << layoutName << "\"" );
        }
        else
            layout = parsed;
    }
    m_preferredFormat = CAudioFormat( sampleRate, channels, sampleType, layout );

    const CORE::CDataNode* options = section->FindChild( "Options" );
    if ( GUCEF_NULL != options )
    {
        m_options.Clear();
        for ( CORE::CDataNode::TAttributeMap::const_iterator a=options->AttributeBegin(); a!=options->AttributeEnd(); ++a )
            m_options.Set( (*a).first, (*a).second );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioDeviceParams::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::AUDIO::CAudioDeviceParams";
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
