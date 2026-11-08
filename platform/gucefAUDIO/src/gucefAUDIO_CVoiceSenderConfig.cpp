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

#include "gucefAUDIO_CVoiceSenderConfig.h"

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

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CVoiceSenderConfig::ConfigSectionName = "VoiceSender";

/*-------------------------------------------------------------------------*/

CVoiceSenderConfig::CVoiceSenderConfig( void )
    : CORE::CIConfigurable()
    , m_codecConfig()
    , m_transmitMode( VOICETRANSMITMODE_CONTINUOUS )
    , m_vadHangoverInMs( DefaultVadHangoverInMs )
    , m_preRollFrames( DefaultPreRollFrames )
    , m_redundancy( 0 )
    , m_redPayloadType( DefaultRedPayloadType )
    , m_outboundQueueFrames( DefaultOutboundQueueFrames )
    , m_streamId( 0 )
    , m_hasFirstSequenceNumber( false )
    , m_firstSequenceNumber( 0 )
    , m_hasFirstTimestamp( false )
    , m_firstTimestamp( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoiceSenderConfig::CVoiceSenderConfig( const CVoiceSenderConfig& src )
    : CORE::CIConfigurable( src )
    , m_codecConfig( src.m_codecConfig )
    , m_transmitMode( src.m_transmitMode )
    , m_vadHangoverInMs( src.m_vadHangoverInMs )
    , m_preRollFrames( src.m_preRollFrames )
    , m_redundancy( src.m_redundancy )
    , m_redPayloadType( src.m_redPayloadType )
    , m_outboundQueueFrames( src.m_outboundQueueFrames )
    , m_streamId( src.m_streamId )
    , m_hasFirstSequenceNumber( src.m_hasFirstSequenceNumber )
    , m_firstSequenceNumber( src.m_firstSequenceNumber )
    , m_hasFirstTimestamp( src.m_hasFirstTimestamp )
    , m_firstTimestamp( src.m_firstTimestamp )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVoiceSenderConfig&
CVoiceSenderConfig::operator=( const CVoiceSenderConfig& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_codecConfig = src.m_codecConfig;
        m_transmitMode = src.m_transmitMode;
        m_vadHangoverInMs = src.m_vadHangoverInMs;
        m_preRollFrames = src.m_preRollFrames;
        m_redundancy = src.m_redundancy;
        m_redPayloadType = src.m_redPayloadType;
        m_outboundQueueFrames = src.m_outboundQueueFrames;
        m_streamId = src.m_streamId;
        m_hasFirstSequenceNumber = src.m_hasFirstSequenceNumber;
        m_firstSequenceNumber = src.m_firstSequenceNumber;
        m_hasFirstTimestamp = src.m_hasFirstTimestamp;
        m_firstTimestamp = src.m_firstTimestamp;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVoiceSenderConfig::~CVoiceSenderConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CVoiceCodecConfig&
CVoiceSenderConfig::GetCodecConfig( void ) const
{GUCEF_TRACE;

    return m_codecConfig;
}

/*-------------------------------------------------------------------------*/

CVoiceCodecConfig&
CVoiceSenderConfig::GetCodecConfig( void )
{GUCEF_TRACE;

    return m_codecConfig;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSenderConfig::SetCodecConfig( const CVoiceCodecConfig& codecConfig )
{GUCEF_TRACE;

    m_codecConfig = codecConfig;
}

/*-------------------------------------------------------------------------*/

TVoiceTransmitMode
CVoiceSenderConfig::GetTransmitMode( void ) const
{GUCEF_TRACE;

    return m_transmitMode;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSenderConfig::SetTransmitMode( TVoiceTransmitMode mode )
{GUCEF_TRACE;

    if ( VOICETRANSMITMODE_CONTINUOUS == mode || VOICETRANSMITMODE_VOICE_ACTIVATED == mode || VOICETRANSMITMODE_PUSH_TO_TALK == mode )
        m_transmitMode = mode;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceSenderConfig::GetVadHangoverInMs( void ) const
{GUCEF_TRACE;

    return m_vadHangoverInMs;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSenderConfig::SetVadHangoverInMs( UInt32 milliseconds )
{GUCEF_TRACE;

    m_vadHangoverInMs = LimitedUInt32( milliseconds, 0, MaxVadHangoverInMs );
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceSenderConfig::GetPreRollFrames( void ) const
{GUCEF_TRACE;

    return m_preRollFrames;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSenderConfig::SetPreRollFrames( UInt32 frames )
{GUCEF_TRACE;

    m_preRollFrames = LimitedUInt32( frames, 0, MaxPreRollFrames );
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceSenderConfig::GetRedundancy( void ) const
{GUCEF_TRACE;

    return m_redundancy;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSenderConfig::SetRedundancy( UInt32 redundancy )
{GUCEF_TRACE;

    m_redundancy = LimitedUInt32( redundancy, 0, 1 );
}

/*-------------------------------------------------------------------------*/

UInt8
CVoiceSenderConfig::GetRedPayloadType( void ) const
{GUCEF_TRACE;

    return m_redPayloadType;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSenderConfig::SetRedPayloadType( UInt8 payloadType )
{GUCEF_TRACE;

    if ( payloadType <= 127 )
        m_redPayloadType = payloadType;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceSenderConfig::GetOutboundQueueFrames( void ) const
{GUCEF_TRACE;

    return m_outboundQueueFrames;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSenderConfig::SetOutboundQueueFrames( UInt32 frames )
{GUCEF_TRACE;

    m_outboundQueueFrames = LimitedUInt32( frames, 2, MaxOutboundQueueFrames );
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceSenderConfig::GetStreamId( void ) const
{GUCEF_TRACE;

    return m_streamId;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSenderConfig::SetStreamId( UInt32 streamId )
{GUCEF_TRACE;

    m_streamId = streamId;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSenderConfig::HasFirstSequenceNumber( void ) const
{GUCEF_TRACE;

    return m_hasFirstSequenceNumber;
}

/*-------------------------------------------------------------------------*/

UInt16
CVoiceSenderConfig::GetFirstSequenceNumber( void ) const
{GUCEF_TRACE;

    return m_firstSequenceNumber;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSenderConfig::SetFirstSequenceNumber( UInt16 sequenceNumber )
{GUCEF_TRACE;

    m_firstSequenceNumber = sequenceNumber;
    m_hasFirstSequenceNumber = true;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSenderConfig::ClearFirstSequenceNumber( void )
{GUCEF_TRACE;

    m_firstSequenceNumber = 0;
    m_hasFirstSequenceNumber = false;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSenderConfig::HasFirstTimestamp( void ) const
{GUCEF_TRACE;

    return m_hasFirstTimestamp;
}

/*-------------------------------------------------------------------------*/

UInt32
CVoiceSenderConfig::GetFirstTimestamp( void ) const
{GUCEF_TRACE;

    return m_firstTimestamp;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSenderConfig::SetFirstTimestamp( UInt32 timestamp )
{GUCEF_TRACE;

    m_firstTimestamp = timestamp;
    m_hasFirstTimestamp = true;
}

/*-------------------------------------------------------------------------*/

void
CVoiceSenderConfig::ClearFirstTimestamp( void )
{GUCEF_TRACE;

    m_firstTimestamp = 0;
    m_hasFirstTimestamp = false;
}

/*-------------------------------------------------------------------------*/

const char*
CVoiceSenderConfig::TransmitModeToString( TVoiceTransmitMode mode )
{GUCEF_TRACE;

    switch ( mode )
    {
        case VOICETRANSMITMODE_VOICE_ACTIVATED: return "voiceActivated";
        case VOICETRANSMITMODE_PUSH_TO_TALK: return "pushToTalk";
        case VOICETRANSMITMODE_CONTINUOUS:
        default: return "continuous";
    }
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSenderConfig::StringToTransmitMode( const CString& text, TVoiceTransmitMode& mode )
{GUCEF_TRACE;

    const CString name = text.Lowercase();
    if ( name == "continuous" )     { mode = VOICETRANSMITMODE_CONTINUOUS; return true; }
    if ( name == "voiceactivated" ) { mode = VOICETRANSMITMODE_VOICE_ACTIVATED; return true; }
    if ( name == "pushtotalk" )     { mode = VOICETRANSMITMODE_PUSH_TO_TALK; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSenderConfig::operator==( const CVoiceSenderConfig& other ) const
{GUCEF_TRACE;

    return m_codecConfig == other.m_codecConfig                                 &&
           m_transmitMode == other.m_transmitMode                               &&
           m_vadHangoverInMs == other.m_vadHangoverInMs                         &&
           m_preRollFrames == other.m_preRollFrames                             &&
           m_redundancy == other.m_redundancy                                   &&
           m_redPayloadType == other.m_redPayloadType                           &&
           m_outboundQueueFrames == other.m_outboundQueueFrames                 &&
           m_streamId == other.m_streamId                                       &&
           m_hasFirstSequenceNumber == other.m_hasFirstSequenceNumber           &&
           m_firstSequenceNumber == other.m_firstSequenceNumber                 &&
           m_hasFirstTimestamp == other.m_hasFirstTimestamp                     &&
           m_firstTimestamp == other.m_firstTimestamp;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSenderConfig::operator!=( const CVoiceSenderConfig& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSenderConfig::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "transmitMode", CString( TransmitModeToString( m_transmitMode ) ) );
    section->SetAttribute( "vadHangoverMs", m_vadHangoverInMs );
    section->SetAttribute( "preRollFrames", m_preRollFrames );
    section->SetAttribute( "redundancy", m_redundancy );
    section->SetAttribute( "redPayloadType", (UInt32) m_redPayloadType );
    section->SetAttribute( "outboundQueueFrames", m_outboundQueueFrames );
    section->SetAttribute( "streamId", m_streamId );
    if ( m_hasFirstSequenceNumber )
        section->SetAttribute( "firstSequenceNumber", (UInt32) m_firstSequenceNumber );
    else
        section->DelAttribute( "firstSequenceNumber" );
    if ( m_hasFirstTimestamp )
        section->SetAttribute( "firstTimestamp", m_firstTimestamp );
    else
        section->DelAttribute( "firstTimestamp" );
    return m_codecConfig.SaveConfig( *section );
}

/*-------------------------------------------------------------------------*/

bool
CVoiceSenderConfig::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    const CString mode = section->GetAttributeValueOrChildValueByName( "transmitMode" ).AsString( CString::Empty, true );
    if ( !mode.IsNULLOrEmpty() && !StringToTransmitMode( mode, m_transmitMode ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "VoiceSenderConfig: Ignoring the unknown transmit mode \"" << mode << "\"" );
    }

    SetVadHangoverInMs( section->GetAttributeValueOrChildValueByName( "vadHangoverMs" ).AsUInt32( m_vadHangoverInMs, true ) );
    SetPreRollFrames( section->GetAttributeValueOrChildValueByName( "preRollFrames" ).AsUInt32( m_preRollFrames, true ) );
    SetRedundancy( section->GetAttributeValueOrChildValueByName( "redundancy" ).AsUInt32( m_redundancy, true ) );
    const UInt32 redPayloadType = section->GetAttributeValueOrChildValueByName( "redPayloadType" ).AsUInt32( m_redPayloadType, true );
    if ( redPayloadType <= 127 )
        m_redPayloadType = (UInt8) redPayloadType;
    SetOutboundQueueFrames( section->GetAttributeValueOrChildValueByName( "outboundQueueFrames" ).AsUInt32( m_outboundQueueFrames, true ) );
    m_streamId = section->GetAttributeValueOrChildValueByName( "streamId" ).AsUInt32( m_streamId, true );

    const CORE::CVariant firstSequenceNumber = section->GetAttributeValueOrChildValueByName( "firstSequenceNumber" );
    if ( firstSequenceNumber.IsInitialized() && !firstSequenceNumber.IsNULL() )
        SetFirstSequenceNumber( (UInt16) firstSequenceNumber.AsUInt32( m_firstSequenceNumber, true ) );
    const CORE::CVariant firstTimestamp = section->GetAttributeValueOrChildValueByName( "firstTimestamp" );
    if ( firstTimestamp.IsInitialized() && !firstTimestamp.IsNULL() )
        SetFirstTimestamp( firstTimestamp.AsUInt32( m_firstTimestamp, true ) );

    // a section without a codec configuration keeps the one there is
    m_codecConfig.LoadConfig( *section );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CVoiceSenderConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::AUDIO::CVoiceSenderConfig";
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
