/*
 *  pubsubpluginFIX: Generic GUCEF COMCORE plugin for providing pubsub via FIX protocol
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#include <string.h>
#include <stdio.h>
#include <time.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "pubsubpluginFIX_CFIXMessage.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace FIX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char CFIXMessage::SOH = '\x01';

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFIXMessage::CFIXMessage( void )
    : m_fields()
    , m_rawMessage()
    , m_isValid( false )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXMessage::CFIXMessage( const CFIXMessage& src )
    : m_fields( src.m_fields )
    , m_rawMessage( src.m_rawMessage )
    , m_isValid( src.m_isValid )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXMessage::~CFIXMessage()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXMessage&
CFIXMessage::operator=( const CFIXMessage& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_fields = src.m_fields;
        m_rawMessage = src.m_rawMessage;
        m_isValid = src.m_isValid;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CFIXMessage::Parse( const CORE::CString& raw, CFIXMessage& out )
{GUCEF_TRACE;

    out.m_fields.clear();
    out.m_rawMessage = raw;
    out.m_isValid = false;

    if ( raw.IsNULLOrEmpty() )
        return false;

    const char* rawPtr = raw.C_String();
    CORE::UInt32 rawLen = (CORE::UInt32) raw.Length();

    CORE::UInt32 fieldStart = 0;
    while ( fieldStart < rawLen )
    {
        // Find SOH terminator
        CORE::UInt32 fieldEnd = fieldStart;
        while ( fieldEnd < rawLen && rawPtr[ fieldEnd ] != SOH )
            ++fieldEnd;

        if ( fieldEnd > fieldStart )
        {
            // Find '=' separator
            CORE::UInt32 eqPos = fieldStart;
            while ( eqPos < fieldEnd && rawPtr[ eqPos ] != '=' )
                ++eqPos;

            if ( eqPos < fieldEnd )
            {
                // tag is rawPtr[fieldStart..eqPos-1]
                CORE::CString tagStr( rawPtr + fieldStart, eqPos - fieldStart );
                // value is rawPtr[eqPos+1..fieldEnd-1]
                CORE::CString valStr( rawPtr + eqPos + 1, fieldEnd - eqPos - 1 );

                CORE::UInt32 tag = CORE::StringToUInt32( tagStr );
                out.m_fields[ tag ] = valStr;
            }
        }

        fieldStart = fieldEnd + 1; // skip SOH
    }

    // Validate BodyLength (tag 9)
    // BodyLength = bytes from first char AFTER tag 9's SOH to SOH of tag 10 (inclusive)
    TFixFieldMap::const_iterator bodyLenIt = out.m_fields.find( TAG_BODY_LENGTH );
    if ( bodyLenIt == out.m_fields.end() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CFIXMessage::Parse: Missing BodyLength (tag 9)" );
        return false;
    }
    CORE::UInt32 claimedBodyLength = CORE::StringToUInt32( bodyLenIt->second );

    // Find the position of "9=xxx\x01" in raw
    // BodyLength starts counting at first byte AFTER that SOH
    // Find "9=" in the raw message
    const char* tag9Pos = GUCEF_NULL;
    const char* searchPtr = rawPtr;
    const char* rawEnd = rawPtr + rawLen;
    while ( searchPtr < rawEnd - 2 )
    {
        if ( searchPtr[0] == '9' && searchPtr[1] == '=' )
        {
            // Make sure this is a proper field start (preceded by SOH or at start)
            if ( searchPtr == rawPtr || *(searchPtr - 1) == SOH )
            {
                tag9Pos = searchPtr;
                break;
            }
        }
        ++searchPtr;
    }

    if ( GUCEF_NULL == tag9Pos )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CFIXMessage::Parse: Cannot locate tag 9 field" );
        return false;
    }

    // Find SOH after tag 9 value
    const char* tag9SohPos = tag9Pos;
    while ( tag9SohPos < rawEnd && *tag9SohPos != SOH )
        ++tag9SohPos;

    if ( tag9SohPos >= rawEnd )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CFIXMessage::Parse: Unterminated tag 9 field" );
        return false;
    }

    // BodyLength starts at the byte after tag 9's SOH
    const char* bodyStart = tag9SohPos + 1;

    // Find "10=" tag which marks end of body
    // BodyLength counts up to and including the SOH of the last field before tag 10
    // i.e., bodyEnd = start of "10=" text
    const char* tag10Pos = GUCEF_NULL;
    const char* scanPtr = bodyStart;
    while ( scanPtr < rawEnd - 3 )
    {
        if ( scanPtr[0] == '1' && scanPtr[1] == '0' && scanPtr[2] == '=' )
        {
            if ( scanPtr == rawPtr || *(scanPtr - 1) == SOH )
            {
                tag10Pos = scanPtr;
                break;
            }
        }
        ++scanPtr;
    }

    if ( GUCEF_NULL == tag10Pos )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CFIXMessage::Parse: Cannot locate tag 10 (CheckSum) field" );
        return false;
    }

    CORE::UInt32 actualBodyLength = (CORE::UInt32)( tag10Pos - bodyStart );
    if ( actualBodyLength != claimedBodyLength )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CFIXMessage::Parse: BodyLength mismatch: claimed=" +
            CORE::ToString( claimedBodyLength ) + " actual=" + CORE::ToString( actualBodyLength ) );
        return false;
    }

    // Validate CheckSum (tag 10)
    // CheckSum = sum of all bytes from beginning of message up to (not including) "10=xxx\x01"
    CORE::UInt32 byteSum = 0;
    for ( const char* p = rawPtr; p < tag10Pos; ++p )
        byteSum += (CORE::UInt8) *p;
    CORE::UInt32 calcChecksum = byteSum % 256;

    TFixFieldMap::const_iterator checksumIt = out.m_fields.find( TAG_CHECK_SUM );
    if ( checksumIt == out.m_fields.end() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CFIXMessage::Parse: Missing CheckSum (tag 10)" );
        return false;
    }
    CORE::UInt32 claimedChecksum = CORE::StringToUInt32( checksumIt->second );

    if ( calcChecksum != claimedChecksum )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CFIXMessage::Parse: CheckSum mismatch: claimed=" +
            CORE::ToString( claimedChecksum ) + " calc=" + CORE::ToString( calcChecksum ) );
        return false;
    }

    out.m_isValid = true;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::GetField( CORE::UInt32 tag, const CORE::CString& defaultVal ) const
{GUCEF_TRACE;

    TFixFieldMap::const_iterator i = m_fields.find( tag );
    if ( i != m_fields.end() )
        return i->second;
    return defaultVal;
}

/*-------------------------------------------------------------------------*/

bool
CFIXMessage::HasField( CORE::UInt32 tag ) const
{GUCEF_TRACE;

    return m_fields.find( tag ) != m_fields.end();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::GetMsgType( void ) const
{GUCEF_TRACE;

    return GetField( TAG_MSG_TYPE );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::GetMsgSeqNum( void ) const
{GUCEF_TRACE;

    return GetField( TAG_MSG_SEQ_NUM );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::GetBeginString( void ) const
{GUCEF_TRACE;

    return GetField( TAG_BEGIN_STRING );
}

/*-------------------------------------------------------------------------*/

CORE::UInt64
CFIXMessage::GetMsgSeqNumAsUInt64( void ) const
{GUCEF_TRACE;

    return CORE::StringToUInt64( GetMsgSeqNum() );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFIXMessage::GetRawMessage( void ) const
{GUCEF_TRACE;

    return m_rawMessage;
}

/*-------------------------------------------------------------------------*/

bool
CFIXMessage::IsValid( void ) const
{GUCEF_TRACE;

    return m_isValid;
}

/*-------------------------------------------------------------------------*/

const CFIXMessage::TFixFieldMap&
CFIXMessage::GetFields( void ) const
{GUCEF_TRACE;

    return m_fields;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::GetCurrentSendingTime( void )
{GUCEF_TRACE;

    CORE::CDateTime now = CORE::CDateTime::NowUTCDateTime();

    // Format: YYYYMMDD-HH:MM:SS
    char buf[ 32 ];
    CORE::Int32 yr   = now.GetDate().GetYear();
    CORE::Int32 mon  = now.GetDate().GetMonth();
    CORE::Int32 day  = now.GetDate().GetDay();
    CORE::Int32 hr   = now.GetTime().GetHours();
    CORE::Int32 min  = now.GetTime().GetMinutes();
    CORE::Int32 sec  = now.GetTime().GetSeconds();

    ::sprintf( buf, "%04d%02d%02d-%02d:%02d:%02d", yr, mon, day, hr, min, sec );
    return CORE::CString( buf );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::CalcCheckSum( const CORE::CString& msgWithoutChecksum )
{GUCEF_TRACE;

    CORE::UInt32 byteSum = 0;
    const char* ptr = msgWithoutChecksum.C_String();
    CORE::UInt32 len = (CORE::UInt32) msgWithoutChecksum.Length();
    for ( CORE::UInt32 i = 0; i < len; ++i )
        byteSum += (CORE::UInt8) ptr[ i ];
    CORE::UInt32 checksum = byteSum % 256;

    char buf[ 8 ];
    ::sprintf( buf, "%03u", checksum );
    return CORE::CString( buf );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::BuildStandardBody( const CORE::CString& msgType,
                                const CORE::CString& senderCompId,
                                const CORE::CString& targetCompId,
                                CORE::UInt64 seqNum )
{GUCEF_TRACE;

    // Build body: 35, 49, 56, 34, 52
    CORE::CString body;
    body += "35=" + msgType + SOH;
    body += "49=" + senderCompId + SOH;
    body += "56=" + targetCompId + SOH;
    body += "34=" + CORE::ToString( seqNum ) + SOH;
    body += "52=" + GetCurrentSendingTime() + SOH;
    return body;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::FinalizeMsg( const CORE::CString& fixVersion,
                          const CORE::CString& body )
{GUCEF_TRACE;

    // body is everything from tag 35 onwards (not including 8=, 9=, 10=)
    CORE::UInt32 bodyLen = (CORE::UInt32) body.Length();

    CORE::CString header;
    header += "8=" + fixVersion + SOH;
    header += "9=" + CORE::ToString( bodyLen ) + SOH;

    CORE::CString msgWithoutChecksum = header + body;
    CORE::CString checksum = CalcCheckSum( msgWithoutChecksum );

    return msgWithoutChecksum + "10=" + checksum + SOH;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::BuildLogon( const CORE::CString& senderCompId,
                         const CORE::CString& targetCompId,
                         const CORE::CString& fixVersion,
                         CORE::UInt64 outgoingSeqNum,
                         CORE::UInt32 heartbeatIntervalSecs,
                         bool resetSeqNumFlag )
{GUCEF_TRACE;

    CORE::CString body = BuildStandardBody( "A", senderCompId, targetCompId, outgoingSeqNum );
    body += "98=0" + CORE::CString( SOH );               // EncryptMethod=0 (None)
    body += "108=" + CORE::ToString( heartbeatIntervalSecs ) + SOH;
    if ( resetSeqNumFlag )
        body += "141=Y" + CORE::CString( SOH );

    return FinalizeMsg( fixVersion, body );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::BuildLogout( const CORE::CString& senderCompId,
                          const CORE::CString& targetCompId,
                          const CORE::CString& fixVersion,
                          CORE::UInt64 outgoingSeqNum,
                          const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString body = BuildStandardBody( "5", senderCompId, targetCompId, outgoingSeqNum );
    if ( !text.IsNULLOrEmpty() )
        body += "58=" + text + SOH;

    return FinalizeMsg( fixVersion, body );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::BuildHeartbeat( const CORE::CString& senderCompId,
                             const CORE::CString& targetCompId,
                             const CORE::CString& fixVersion,
                             CORE::UInt64 outgoingSeqNum,
                             const CORE::CString& testReqId )
{GUCEF_TRACE;

    CORE::CString body = BuildStandardBody( "0", senderCompId, targetCompId, outgoingSeqNum );
    if ( !testReqId.IsNULLOrEmpty() )
        body += "112=" + testReqId + SOH;

    return FinalizeMsg( fixVersion, body );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::BuildTestRequest( const CORE::CString& senderCompId,
                               const CORE::CString& targetCompId,
                               const CORE::CString& fixVersion,
                               CORE::UInt64 outgoingSeqNum,
                               const CORE::CString& testReqId )
{GUCEF_TRACE;

    CORE::CString body = BuildStandardBody( "1", senderCompId, targetCompId, outgoingSeqNum );
    body += "112=" + testReqId + SOH;

    return FinalizeMsg( fixVersion, body );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::BuildResendRequest( const CORE::CString& senderCompId,
                                 const CORE::CString& targetCompId,
                                 const CORE::CString& fixVersion,
                                 CORE::UInt64 outgoingSeqNum,
                                 CORE::UInt64 beginSeqNo,
                                 CORE::UInt64 endSeqNo )
{GUCEF_TRACE;

    CORE::CString body = BuildStandardBody( "2", senderCompId, targetCompId, outgoingSeqNum );
    body += "7=" + CORE::ToString( beginSeqNo ) + SOH;
    body += "16=" + CORE::ToString( endSeqNo ) + SOH;

    return FinalizeMsg( fixVersion, body );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFIXMessage::BuildSequenceReset( const CORE::CString& senderCompId,
                                 const CORE::CString& targetCompId,
                                 const CORE::CString& fixVersion,
                                 CORE::UInt64 outgoingSeqNum,
                                 CORE::UInt64 newSeqNo,
                                 bool gapFillFlag )
{GUCEF_TRACE;

    CORE::CString body = BuildStandardBody( "4", senderCompId, targetCompId, outgoingSeqNum );
    if ( gapFillFlag )
        body += "123=Y" + CORE::CString( SOH );
    body += "36=" + CORE::ToString( newSeqNo ) + SOH;

    return FinalizeMsg( fixVersion, body );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FIX */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
