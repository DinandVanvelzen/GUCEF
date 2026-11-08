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

#include "gucefAUDIO_CVoiceReceiverConfig.h"

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

UInt32
LimitedUInt32( UInt32 value, UInt32 minimum, UInt32 maximum )
{
    if ( value < minimum )
        return minimum;
    if ( value > maximum )
        return maximum;
    return value;
}

/** 'value' within the range; anything that is not a number gives 'fallback' */
Float64
LimitedFloat64( Float64 value, Float64 minimum, Float64 maximum, Float64 fallback )
{
    if ( !( value == value ) )
        return fallback;
    if ( value < minimum )
        return minimum;
    if ( value > maximum )
        return maximum;
    return value;
}

const Float64 MaxMarginLimitInMs = 2000.0;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CVoiceReceiverConfig::ConfigSectionName = "VoiceReceiver";
const Float64 CVoiceReceiverConfig::MaxLatePercentage = 20.0;

/*-------------------------------------------------------------------------*/

CVoiceReceiverConfig::CVoiceReceiverConfig( void )
    : CORE::CIConfigurable()
    , m_codecConfig()
    , m_playoutProfile( VOICEPLAYOUTPROFILE_WIRELESS )
    , m_initialMarginInMs( 20.0 )
    , m_minMarginInMs( 6.0 )
    , m_maxMarginInMs( 200.0 )
    , m_windowInS( 4.0 )
    , m_latePercentage( 1.0 )
    , m_inboundQueueFrames( DefaultInboundQueueFrames )
    , m_slotCount( DefaultSlotCount )
    , m_concealLimit( DefaultConcealLimit )
    , m_streamId( 0 )
    , m_redPayloadType( DefaultRedPayloadType )
{GUCEF_TRACE;

    SetPlayoutProfile( VOICEPLAYOUTPROFILE_WIRELESS );
}

/*-------------------------------------------------------------------------*/

CVoiceReceiverConfig::CVoiceReceiverConfig( const CVoiceReceiverConfig& src )
    : CORE::CIConfigurable( src )
    , m_codecConfig( src.m_codecConfig )
    , m_playoutProfile( src.m_playoutProfile )
    , m_initialMarginInMs( src.m_initialMarginInMs )
    , m_minMarginInMs( src.m_minMarginInMs )
    , m_maxMarginInMs( src.m_maxMarginInMs )
    , m_windowInS( src.m_windowInS )
    , m_latePercentage( src.m_latePercentage )
    , m_inboundQueueFrames( src.m_inboundQueueFrames )
    , m_slotCount( src.m_slotCount )
    , m_concealLimit( src.m_concealLimit )
    , m_streamId( src.m_streamId )
    , m_redPayloadType( src.m_redPayloadType )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoiceReceiverConfig&
CVoiceReceiverConfig::operator=( const CVoiceReceiverConfig& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_codecConfig = src.m_codecConfig;
        m_playoutProfile = src.m_playoutProfile;
        m_initialMarginInMs = src.m_initialMarginInMs;
        m_minMarginInMs = src.m_minMarginInMs;
        m_maxMarginInMs = src.m_maxMarginInMs;
        m_windowInS = src.m_windowInS;
        m_latePercentage = src.m_latePercentage;
        m_inboundQueueFrames = src.m_inboundQueueFrames;
        m_slotCount = src.m_slotCount;
        m_concealLimit = src.m_concealLimit;
        m_streamId = src.m_streamId;
        m_redPayloadType = src.m_redPayloadType;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVoiceReceiverConfig::~CVoiceReceiverConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CVoiceCodecConfig&
CVoiceReceiverConfig::GetCodecConfig( void ) const
{GUCEF_TRACE;

    return m_codecConfig;
}

/*-------------------------------------------------------------------------*/

CVoiceCodecConfig&
CVoiceReceiverConfig::GetCodecConfig( void )
{GUCEF_TRACE;

    return m_codecConfig;
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverConfig::SetCodecConfig( const CVoiceCodecConfig& codecConfig )
{GUCEF_TRACE;

    m_codecConfig = codecConfig;
}

/*-------------------------------------------------------------------------*/

TVoicePlayoutProfile
CVoiceReceiverConfig::GetPlayoutProfile( void ) const
{GUCEF_TRACE;

    return m_playoutProfile;
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverConfig::SetPlayoutProfile( TVoicePlayoutProfile profile )
{GUCEF_TRACE;

    if ( VOICEPLAYOUTPROFILE_WIRED == profile )
    {
        m_playoutProfile = VOICEPLAYOUTPROFILE_WIRED;
        m_initialMarginInMs = 4.0;
        m_minMarginInMs = 2.0;
        m_maxMarginInMs = 60.0;
        m_windowInS = 2.0;
        m_latePercentage = 0.0;
    }
    else
    if ( VOICEPLAYOUTPROFILE_WIRELESS == profile )
    {
        m_playoutProfile = VOICEPLAYOUTPROFILE_WIRELESS;
        m_initialMarginInMs = 20.0;
        m_minMarginInMs = 6.0;
        m_maxMarginInMs = 200.0;
        m_windowInS = 4.0;
        m_latePercentage = 1.0;
    }
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverConfig::LimitMargins( void )
{GUCEF_TRACE;

    if ( m_maxMarginInMs < m_minMarginInMs )
        m_maxMarginInMs = m_minMarginInMs;
    if ( m_initialMarginInMs < m_minMarginInMs )
        m_initialMarginInMs = m_minMarginInMs;
    if ( m_initialMarginInMs > m_maxMarginInMs )
        m_initialMarginInMs = m_maxMarginInMs;
}

/*-------------------------------------------------------------------------*/

Float64
CVoiceReceiverConfig::GetInitialMarginInMs( void ) const
{GUCEF_TRACE;

    return m_initialMarginInMs;
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverConfig::SetInitialMarginInMs( Float64 milliseconds )
{GUCEF_TRACE;

    m_initialMarginInMs = LimitedFloat64( milliseconds, m_minMarginInMs, m_maxMarginInMs, m_initialMarginInMs );
}

/*-------------------------------------------------------------------------*/

Float64
CVoiceReceiverConfig::GetMinMarginInMs( void ) const
{GUCEF_TRACE;

    return m_minMarginInMs;
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverConfig::SetMinMarginInMs( Float64 milliseconds )
{GUCEF_TRACE;

    m_minMarginInMs = LimitedFloat64( milliseconds, 0.0, 1000.0, m_minMarginInMs );
    LimitMargins();
}

/*-------------------------------------------------------------------------*/

Float64
CVoiceReceiverConfig::GetMaxMarginInMs( void ) const
{GUCEF_TRACE;

    return m_maxMarginInMs;
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverConfig::SetMaxMarginInMs( Float64 milliseconds )
{GUCEF_TRACE;

    m_maxMarginInMs = LimitedFloat64( milliseconds, m_minMarginInMs, MaxMarginLimitInMs, m_maxMarginInMs );
    LimitMargins();
}

/*-------------------------------------------------------------------------*/

Float64
CVoiceReceiverConfig::GetWindowInS( void ) const
{GUCEF_TRACE;

    return m_windowInS;
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverConfig::SetWindowInS( Float64 seconds )
{GUCEF_TRACE;

    m_windowInS = LimitedFloat64( seconds, 0.1, 60.0, m_windowInS );
}

/*-------------------------------------------------------------------------*/

Float64
CVoiceReceiverConfig::GetLatePercentage( void ) const
{GUCEF_TRACE;

    return m_latePercentage;
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverConfig::SetLatePercentage( Float64 percentage )
{GUCEF_TRACE;

    m_latePercentage = LimitedFloat64( percentage, 0.0, MaxLatePercentage, m_latePercentage );
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceReceiverConfig::GetInboundQueueFrames( void ) const
{GUCEF_TRACE;

    return m_inboundQueueFrames;
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverConfig::SetInboundQueueFrames( UInt32 frames )
{GUCEF_TRACE;

    m_inboundQueueFrames = LimitedUInt32( frames, 2, MaxInboundQueueFrames );
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceReceiverConfig::GetSlotCount( void ) const
{GUCEF_TRACE;

    return m_slotCount;
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverConfig::SetSlotCount( UInt32 slots )
{GUCEF_TRACE;

    const UInt32 wanted = LimitedUInt32( slots, 2, 4096 );
    UInt32 count = 2;
    while ( count < wanted )
        count <<= 1;
    m_slotCount = count;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceReceiverConfig::GetConcealLimit( void ) const
{GUCEF_TRACE;

    return m_concealLimit;
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverConfig::SetConcealLimit( UInt32 frames )
{GUCEF_TRACE;

    m_concealLimit = LimitedUInt32( frames, 1, MaxConcealLimit );
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceReceiverConfig::GetStreamId( void ) const
{GUCEF_TRACE;

    return m_streamId;
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverConfig::SetStreamId( UInt32 streamId )
{GUCEF_TRACE;

    m_streamId = streamId;
}

/*-------------------------------------------------------------------------*/

UInt8
CVoiceReceiverConfig::GetRedPayloadType( void ) const
{GUCEF_TRACE;

    return m_redPayloadType;
}

/*-------------------------------------------------------------------------*/

void
CVoiceReceiverConfig::SetRedPayloadType( UInt8 payloadType )
{GUCEF_TRACE;

    if ( payloadType <= 127 )
        m_redPayloadType = payloadType;
}

/*-------------------------------------------------------------------------*/

const char*
CVoiceReceiverConfig::PlayoutProfileToString( TVoicePlayoutProfile profile )
{GUCEF_TRACE;

    return VOICEPLAYOUTPROFILE_WIRED == profile ? "wired" : "wireless";
}

/*-------------------------------------------------------------------------*/

bool
CVoiceReceiverConfig::StringToPlayoutProfile( const CString& text, TVoicePlayoutProfile& profile )
{GUCEF_TRACE;

    const CString name = text.Lowercase();
    if ( name == "wireless" )   { profile = VOICEPLAYOUTPROFILE_WIRELESS; return true; }
    if ( name == "wired" )      { profile = VOICEPLAYOUTPROFILE_WIRED; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceReceiverConfig::operator==( const CVoiceReceiverConfig& other ) const
{GUCEF_TRACE;

    return m_codecConfig == other.m_codecConfig                     &&
           m_playoutProfile == other.m_playoutProfile               &&
           m_initialMarginInMs == other.m_initialMarginInMs         &&
           m_minMarginInMs == other.m_minMarginInMs                 &&
           m_maxMarginInMs == other.m_maxMarginInMs                 &&
           m_windowInS == other.m_windowInS                         &&
           m_latePercentage == other.m_latePercentage               &&
           m_inboundQueueFrames == other.m_inboundQueueFrames       &&
           m_slotCount == other.m_slotCount                         &&
           m_concealLimit == other.m_concealLimit                   &&
           m_streamId == other.m_streamId                           &&
           m_redPayloadType == other.m_redPayloadType;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceReceiverConfig::operator!=( const CVoiceReceiverConfig& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceReceiverConfig::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "playoutProfile", CString( PlayoutProfileToString( m_playoutProfile ) ) );
    section->SetAttribute( "initialMarginMs", CORE::CVariant( m_initialMarginInMs ) );
    section->SetAttribute( "minMarginMs", CORE::CVariant( m_minMarginInMs ) );
    section->SetAttribute( "maxMarginMs", CORE::CVariant( m_maxMarginInMs ) );
    section->SetAttribute( "windowS", CORE::CVariant( m_windowInS ) );
    section->SetAttribute( "latePercent", CORE::CVariant( m_latePercentage ) );
    section->SetAttribute( "inboundQueueFrames", m_inboundQueueFrames );
    section->SetAttribute( "slotCount", m_slotCount );
    section->SetAttribute( "concealLimit", m_concealLimit );
    section->SetAttribute( "streamId", m_streamId );
    section->SetAttribute( "redPayloadType", (UInt32) m_redPayloadType );
    return m_codecConfig.SaveConfig( *section );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceReceiverConfig::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    // the profile first: it sets the margins and the window, which the section may then override
    const CString profileName = section->GetAttributeValueOrChildValueByName( "playoutProfile" ).AsString( CString::Empty, true );
    if ( !profileName.IsNULLOrEmpty() )
    {
        TVoicePlayoutProfile profile = m_playoutProfile;
        if ( StringToPlayoutProfile( profileName, profile ) )
            SetPlayoutProfile( profile );
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "VoiceReceiverConfig: Ignoring the unknown playout profile \"" << profileName << "\"" );
        }
    }

    // the limits before the initial margin, which they bound
    const Float64 minMargin = section->GetAttributeValueOrChildValueByName( "minMarginMs" ).AsFloat64( m_minMarginInMs, true );
    const Float64 maxMargin = section->GetAttributeValueOrChildValueByName( "maxMarginMs" ).AsFloat64( m_maxMarginInMs, true );
    const Float64 initialMargin = section->GetAttributeValueOrChildValueByName( "initialMarginMs" ).AsFloat64( m_initialMarginInMs, true );
    m_minMarginInMs = LimitedFloat64( minMargin, 0.0, 1000.0, m_minMarginInMs );
    m_maxMarginInMs = LimitedFloat64( maxMargin, 0.0, MaxMarginLimitInMs, m_maxMarginInMs );
    m_initialMarginInMs = LimitedFloat64( initialMargin, 0.0, MaxMarginLimitInMs, m_initialMarginInMs );
    LimitMargins();
    SetWindowInS( section->GetAttributeValueOrChildValueByName( "windowS" ).AsFloat64( m_windowInS, true ) );
    SetLatePercentage( section->GetAttributeValueOrChildValueByName( "latePercent" ).AsFloat64( m_latePercentage, true ) );

    SetInboundQueueFrames( section->GetAttributeValueOrChildValueByName( "inboundQueueFrames" ).AsUInt32( m_inboundQueueFrames, true ) );
    SetSlotCount( section->GetAttributeValueOrChildValueByName( "slotCount" ).AsUInt32( m_slotCount, true ) );
    SetConcealLimit( section->GetAttributeValueOrChildValueByName( "concealLimit" ).AsUInt32( m_concealLimit, true ) );
    m_streamId = section->GetAttributeValueOrChildValueByName( "streamId" ).AsUInt32( m_streamId, true );
    const UInt32 redPayloadType = section->GetAttributeValueOrChildValueByName( "redPayloadType" ).AsUInt32( m_redPayloadType, true );
    if ( redPayloadType <= 127 )
        m_redPayloadType = (UInt8) redPayloadType;

    // a section without a codec configuration keeps the one there is
    m_codecConfig.LoadConfig( *section );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CVoiceReceiverConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::AUDIO::CVoiceReceiverConfig";
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
