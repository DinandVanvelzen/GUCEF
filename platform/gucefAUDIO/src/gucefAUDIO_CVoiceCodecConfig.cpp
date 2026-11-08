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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefAUDIO_CVoiceCodecConfig.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CVoiceCodecConfig::ConfigSectionName = "VoiceCodec";

/*-------------------------------------------------------------------------*/

CVoiceCodecConfig::CVoiceCodecConfig( void )
    : CORE::CIConfigurable()
    , m_codec( "opus" )
    , m_sampleRate( 48000 )
    , m_channels( 1 )
    , m_frameDurationInUs( 10000 )
    , m_bitrate( 48000 )
    , m_profile( VOICECODECPROFILE_LOWDELAY )
    , m_complexity( 5 )
    , m_inbandFec( false )
    , m_expectedLossPercentage( 0 )
    , m_payloadType( 111 )
    , m_payloadTypeIsSet( false )
    , m_options()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoiceCodecConfig::CVoiceCodecConfig( const CVoiceCodecConfig& src )
    : CORE::CIConfigurable( src )
    , m_codec( src.m_codec )
    , m_sampleRate( src.m_sampleRate )
    , m_channels( src.m_channels )
    , m_frameDurationInUs( src.m_frameDurationInUs )
    , m_bitrate( src.m_bitrate )
    , m_profile( src.m_profile )
    , m_complexity( src.m_complexity )
    , m_inbandFec( src.m_inbandFec )
    , m_expectedLossPercentage( src.m_expectedLossPercentage )
    , m_payloadType( src.m_payloadType )
    , m_payloadTypeIsSet( src.m_payloadTypeIsSet )
    , m_options( src.m_options )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoiceCodecConfig&
CVoiceCodecConfig::operator=( const CVoiceCodecConfig& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CORE::CIConfigurable::operator=( src );
        m_codec = src.m_codec;
        m_sampleRate = src.m_sampleRate;
        m_channels = src.m_channels;
        m_frameDurationInUs = src.m_frameDurationInUs;
        m_bitrate = src.m_bitrate;
        m_profile = src.m_profile;
        m_complexity = src.m_complexity;
        m_inbandFec = src.m_inbandFec;
        m_expectedLossPercentage = src.m_expectedLossPercentage;
        m_payloadType = src.m_payloadType;
        m_payloadTypeIsSet = src.m_payloadTypeIsSet;
        m_options = src.m_options;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVoiceCodecConfig::~CVoiceCodecConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CVoiceCodecConfig::GetCodec( void ) const
{GUCEF_TRACE;

    return m_codec;
}

/*-------------------------------------------------------------------------*/

void
CVoiceCodecConfig::SetCodec( const CString& codec )
{GUCEF_TRACE;

    m_codec = codec;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceCodecConfig::GetSampleRate( void ) const
{GUCEF_TRACE;

    return m_sampleRate;
}

/*-------------------------------------------------------------------------*/

void
CVoiceCodecConfig::SetSampleRate( UInt32 sampleRate )
{GUCEF_TRACE;

    m_sampleRate = sampleRate;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceCodecConfig::GetChannels( void ) const
{GUCEF_TRACE;

    return m_channels;
}

/*-------------------------------------------------------------------------*/

void
CVoiceCodecConfig::SetChannels( UInt32 channels )
{GUCEF_TRACE;

    m_channels = channels;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceCodecConfig::GetFrameDurationInUs( void ) const
{GUCEF_TRACE;

    return m_frameDurationInUs;
}

/*-------------------------------------------------------------------------*/

void
CVoiceCodecConfig::SetFrameDurationInUs( UInt32 microseconds )
{GUCEF_TRACE;

    m_frameDurationInUs = microseconds;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceCodecConfig::GetFrameSizeInFrames( void ) const
{GUCEF_TRACE;

    return (UInt32) ( ( (UInt64) m_sampleRate * m_frameDurationInUs + 500000ULL ) / 1000000ULL );
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceCodecConfig::GetBitrate( void ) const
{GUCEF_TRACE;

    return m_bitrate;
}

/*-------------------------------------------------------------------------*/

void
CVoiceCodecConfig::SetBitrate( UInt32 bitsPerSecond )
{GUCEF_TRACE;

    m_bitrate = bitsPerSecond;
}

/*-------------------------------------------------------------------------*/

TVoiceCodecProfile
CVoiceCodecConfig::GetProfile( void ) const
{GUCEF_TRACE;

    return m_profile;
}

/*-------------------------------------------------------------------------*/

void
CVoiceCodecConfig::SetProfile( TVoiceCodecProfile profile )
{GUCEF_TRACE;

    m_profile = profile;
}

/*-------------------------------------------------------------------------*/

void
CVoiceCodecConfig::SetProfileDefaults( TVoiceCodecProfile profile )
{GUCEF_TRACE;

    m_profile = profile;
    if ( VOICECODECPROFILE_ROBUST == profile )
    {
        m_frameDurationInUs = 20000;
        m_bitrate = 32000;
        m_inbandFec = true;
        m_expectedLossPercentage = 10;
    }
    else
    {
        m_frameDurationInUs = 10000;
        m_bitrate = 48000;
        m_inbandFec = false;
        m_expectedLossPercentage = 0;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceCodecConfig::GetComplexity( void ) const
{GUCEF_TRACE;

    return m_complexity;
}

/*-------------------------------------------------------------------------*/

void
CVoiceCodecConfig::SetComplexity( UInt32 complexity )
{GUCEF_TRACE;

    m_complexity = complexity;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceCodecConfig::GetInbandFec( void ) const
{GUCEF_TRACE;

    return m_inbandFec;
}

/*-------------------------------------------------------------------------*/

void
CVoiceCodecConfig::SetInbandFec( bool enabled )
{GUCEF_TRACE;

    m_inbandFec = enabled;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceCodecConfig::GetExpectedLossPercentage( void ) const
{GUCEF_TRACE;

    return m_expectedLossPercentage;
}

/*-------------------------------------------------------------------------*/

void
CVoiceCodecConfig::SetExpectedLossPercentage( UInt32 percentage )
{GUCEF_TRACE;

    m_expectedLossPercentage = percentage;
}

/*-------------------------------------------------------------------------*/

UInt8
CVoiceCodecConfig::GetDefaultPayloadType( const CString& codec )
{GUCEF_TRACE;

    return codec.Lowercase() == "pcm" ? 112 : 111;
}

/*-------------------------------------------------------------------------*/

UInt8
CVoiceCodecConfig::GetPayloadType( void ) const
{GUCEF_TRACE;

    return m_payloadTypeIsSet ? m_payloadType : GetDefaultPayloadType( m_codec );
}

/*-------------------------------------------------------------------------*/

void
CVoiceCodecConfig::SetPayloadType( UInt8 payloadType )
{GUCEF_TRACE;

    m_payloadType = payloadType;
    m_payloadTypeIsSet = true;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
CVoiceCodecConfig::GetOptions( void ) const
{GUCEF_TRACE;

    return m_options;
}

/*-------------------------------------------------------------------------*/

CORE::CValueList&
CVoiceCodecConfig::GetOptions( void )
{GUCEF_TRACE;

    return m_options;
}

/*-------------------------------------------------------------------------*/

const char*
CVoiceCodecConfig::ProfileToString( TVoiceCodecProfile profile )
{GUCEF_TRACE;

    return VOICECODECPROFILE_ROBUST == profile ? "robust" : "lowDelay";
}

/*-------------------------------------------------------------------------*/

bool
CVoiceCodecConfig::StringToProfile( const CString& text, TVoiceCodecProfile& profile )
{GUCEF_TRACE;

    const CString name = text.Lowercase();
    if ( name == "lowdelay" )   { profile = VOICECODECPROFILE_LOWDELAY; return true; }
    if ( name == "robust" )     { profile = VOICECODECPROFILE_ROBUST; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceCodecConfig::operator==( const CVoiceCodecConfig& other ) const
{GUCEF_TRACE;

    return m_codec == other.m_codec                                         &&
           m_sampleRate == other.m_sampleRate                               &&
           m_channels == other.m_channels                                   &&
           m_frameDurationInUs == other.m_frameDurationInUs                 &&
           m_bitrate == other.m_bitrate                                     &&
           m_profile == other.m_profile                                     &&
           m_complexity == other.m_complexity                               &&
           m_inbandFec == other.m_inbandFec                                 &&
           m_expectedLossPercentage == other.m_expectedLossPercentage       &&
           GetPayloadType() == other.GetPayloadType()                       &&
           CORE::ToString( m_options ) == CORE::ToString( other.m_options ) ;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceCodecConfig::operator!=( const CVoiceCodecConfig& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceCodecConfig::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "codec", m_codec );
    section->SetAttribute( "sampleRate", m_sampleRate );
    section->SetAttribute( "channels", m_channels );
    section->SetAttribute( "frameDurationUs", m_frameDurationInUs );
    section->SetAttribute( "bitrate", m_bitrate );
    section->SetAttribute( "profile", CString( ProfileToString( m_profile ) ) );
    section->SetAttribute( "complexity", m_complexity );
    section->SetAttribute( "inbandFec", m_inbandFec );
    section->SetAttribute( "expectedLoss", m_expectedLossPercentage );
    section->SetAttribute( "payloadType", (UInt32) GetPayloadType() );
    return section->SetAttributesOfChildByName( "Options", m_options );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceCodecConfig::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    const CORE::CVariant codec = section->GetAttributeValueOrChildValueByName( "codec" );
    if ( codec.IsInitialized() && !codec.IsNULL() )
        m_codec = codec.AsString( m_codec, true );

    m_sampleRate = section->GetAttributeValueOrChildValueByName( "sampleRate" ).AsUInt32( m_sampleRate, true );
    m_channels = section->GetAttributeValueOrChildValueByName( "channels" ).AsUInt32( m_channels, true );
    m_frameDurationInUs = section->GetAttributeValueOrChildValueByName( "frameDurationUs" ).AsUInt32( m_frameDurationInUs, true );
    m_bitrate = section->GetAttributeValueOrChildValueByName( "bitrate" ).AsUInt32( m_bitrate, true );
    m_complexity = section->GetAttributeValueOrChildValueByName( "complexity" ).AsUInt32( m_complexity, true );
    m_inbandFec = section->GetAttributeValueOrChildValueByName( "inbandFec" ).AsBool( m_inbandFec, true );
    m_expectedLossPercentage = section->GetAttributeValueOrChildValueByName( "expectedLoss" ).AsUInt32( m_expectedLossPercentage, true );

    const CORE::CVariant payloadType = section->GetAttributeValueOrChildValueByName( "payloadType" );
    if ( payloadType.IsInitialized() && !payloadType.IsNULL() )
        SetPayloadType( (UInt8) payloadType.AsUInt32( GetPayloadType(), true ) );

    const CString profile = section->GetAttributeValueOrChildValueByName( "profile" ).AsString( CString::Empty, true );
    if ( !profile.IsNULLOrEmpty() && !StringToProfile( profile, m_profile ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "VoiceCodecConfig: Ignoring the unknown profile \"" << profile << "\"" );
    }

    CORE::CValueList options;
    if ( section->GetAttributesOfChildByName( "Options", options ) )
        m_options = options;
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CVoiceCodecConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::AUDIO::CVoiceCodecConfig";
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
