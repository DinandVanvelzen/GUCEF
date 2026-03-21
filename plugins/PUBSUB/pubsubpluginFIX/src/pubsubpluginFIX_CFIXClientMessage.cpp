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

#include "pubsubpluginFIX_CFIXClientMessage.h"

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

const char CFIXClientMessage::SOH = '\x01';

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CAsciiString
CFIXClientMessage::GetCurrentSendingTime( void )
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
    return CORE::CAsciiString( buf );
}

/*-------------------------------------------------------------------------*/

CORE::CAsciiString
CFIXClientMessage::CalcCheckSum( const CORE::CAsciiString& msgWithoutChecksum )
{GUCEF_TRACE;

    CORE::UInt32 byteSum = 0;
    const char* ptr = msgWithoutChecksum.C_String();
    CORE::UInt32 len = (CORE::UInt32) msgWithoutChecksum.Length();
    for ( CORE::UInt32 i = 0; i < len; ++i )
        byteSum += (CORE::UInt8) ptr[ i ];
    CORE::UInt32 checksum = byteSum % 256;

    char buf[ 8 ];
    ::sprintf( buf, "%03u", checksum );
    return CORE::CAsciiString( buf );
}

/*-------------------------------------------------------------------------*/

CORE::CAsciiString
CFIXClientMessage::BuildStandardBody( const CORE::CAsciiString& msgType      ,
                                      const CORE::CAsciiString& senderCompId ,
                                      const CORE::CAsciiString& targetCompId ,
                                      CORE::UInt64 seqNum                    )
{GUCEF_TRACE;

    // Build body: 35, 49, 56, 34, 52
    CORE::CAsciiString body;
    body += "35=" + msgType + SOH;
    body += "49=" + senderCompId + SOH;
    body += "56=" + targetCompId + SOH;
    body += "34=" + CORE::ToString( seqNum ) + SOH;
    body += "52=" + GetCurrentSendingTime() + SOH;
    return body;
}

/*-------------------------------------------------------------------------*/

CORE::CAsciiString
CFIXClientMessage::FinalizeMsg( const CORE::CAsciiString& fixVersion ,
                                const CORE::CAsciiString& body        )
{GUCEF_TRACE;

    // body is everything from tag 35 onwards (not including 8=, 9=, 10=)
    CORE::UInt32 bodyLen = (CORE::UInt32) body.Length();

    CORE::CAsciiString header;
    header += "8=" + fixVersion + SOH;
    header += "9=" + CORE::ToString( bodyLen ) + SOH;

    CORE::CAsciiString msgWithoutChecksum = header + body;
    CORE::CAsciiString checksum = CalcCheckSum( msgWithoutChecksum );

    return msgWithoutChecksum + "10=" + checksum + SOH;
}

/*-------------------------------------------------------------------------*/

CORE::CAsciiString
CFIXClientMessage::BuildLogon( const CORE::CAsciiString& senderCompId ,
                               const CORE::CAsciiString& targetCompId ,
                               const CORE::CAsciiString& fixVersion   ,
                               CORE::UInt64 outgoingSeqNum            ,
                               CORE::UInt32 heartbeatIntervalSecs     ,
                               bool resetSeqNumFlag                    )
{GUCEF_TRACE;

    CORE::CAsciiString body = BuildStandardBody( "A", senderCompId, targetCompId, outgoingSeqNum );
    body += "98=0" + CORE::CAsciiString( SOH );               // EncryptMethod=0 (None)
    body += "108=" + CORE::ToString( heartbeatIntervalSecs ) + SOH;
    if ( resetSeqNumFlag )
        body += "141=Y" + CORE::CAsciiString( SOH );

    return FinalizeMsg( fixVersion, body );
}

/*-------------------------------------------------------------------------*/

CORE::CAsciiString
CFIXClientMessage::BuildLogout( const CORE::CAsciiString& senderCompId ,
                                const CORE::CAsciiString& targetCompId ,
                                const CORE::CAsciiString& fixVersion   ,
                                CORE::UInt64 outgoingSeqNum            ,
                                const CORE::CAsciiString& text          )
{GUCEF_TRACE;

    CORE::CAsciiString body = BuildStandardBody( "5", senderCompId, targetCompId, outgoingSeqNum );
    if ( !text.IsNULLOrEmpty() )
        body += "58=" + text + SOH;

    return FinalizeMsg( fixVersion, body );
}

/*-------------------------------------------------------------------------*/

CORE::CAsciiString
CFIXClientMessage::BuildHeartbeat( const CORE::CAsciiString& senderCompId ,
                                   const CORE::CAsciiString& targetCompId ,
                                   const CORE::CAsciiString& fixVersion   ,
                                   CORE::UInt64 outgoingSeqNum            ,
                                   const CORE::CAsciiString& testReqId    )
{GUCEF_TRACE;

    CORE::CAsciiString body = BuildStandardBody( "0", senderCompId, targetCompId, outgoingSeqNum );
    if ( !testReqId.IsNULLOrEmpty() )
        body += "112=" + testReqId + SOH;

    return FinalizeMsg( fixVersion, body );
}

/*-------------------------------------------------------------------------*/

CORE::CAsciiString
CFIXClientMessage::BuildTestRequest( const CORE::CAsciiString& senderCompId ,
                                     const CORE::CAsciiString& targetCompId ,
                                     const CORE::CAsciiString& fixVersion   ,
                                     CORE::UInt64 outgoingSeqNum            ,
                                     const CORE::CAsciiString& testReqId    )
{GUCEF_TRACE;

    CORE::CAsciiString body = BuildStandardBody( "1", senderCompId, targetCompId, outgoingSeqNum );
    body += "112=" + testReqId + SOH;

    return FinalizeMsg( fixVersion, body );
}

/*-------------------------------------------------------------------------*/

CORE::CAsciiString
CFIXClientMessage::BuildResendRequest( const CORE::CAsciiString& senderCompId ,
                                       const CORE::CAsciiString& targetCompId ,
                                       const CORE::CAsciiString& fixVersion   ,
                                       CORE::UInt64 outgoingSeqNum            ,
                                       CORE::UInt64 beginSeqNo                ,
                                       CORE::UInt64 endSeqNo                   )
{GUCEF_TRACE;

    CORE::CAsciiString body = BuildStandardBody( "2", senderCompId, targetCompId, outgoingSeqNum );
    body += "7=" + CORE::ToString( beginSeqNo ) + SOH;
    body += "16=" + CORE::ToString( endSeqNo ) + SOH;

    return FinalizeMsg( fixVersion, body );
}

/*-------------------------------------------------------------------------*/

CORE::CAsciiString
CFIXClientMessage::BuildSequenceReset( const CORE::CAsciiString& senderCompId ,
                                       const CORE::CAsciiString& targetCompId ,
                                       const CORE::CAsciiString& fixVersion   ,
                                       CORE::UInt64 outgoingSeqNum            ,
                                       CORE::UInt64 newSeqNo                  ,
                                       bool gapFillFlag                        )
{GUCEF_TRACE;

    CORE::CAsciiString body = BuildStandardBody( "4", senderCompId, targetCompId, outgoingSeqNum );
    if ( gapFillFlag )
        body += "123=Y" + CORE::CAsciiString( SOH );
    body += "36=" + CORE::ToString( newSeqNo ) + SOH;

    return FinalizeMsg( fixVersion, body );
}

// [S4] Parse decimal integer inline. Returns 0 on overflow or non-digit.
CORE::UInt64
CFIXClientMessage::ParseUInt64Inline( const char* s, CORE::UInt32 len )
{
    if ( len == 0 || len > 20 )
        return 0;  // [S4] overflow guard: a 21-digit string overflows UInt64
    CORE::UInt64 r = 0;
    for ( CORE::UInt32 i = 0; i < len; ++i )
    {
        if ( s[ i ] < '0' || s[ i ] > '9' )
            return 0;  // [S4] non-digit guard
        r = r * 10 + ( (CORE::UInt8)s[ i ] - '0' );
    }
    return r;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientMessage::FieldMatchesValue( const char* fieldStart ,
                                      CORE::UInt32 fieldLen  ,
                                      const char* expected   )
{
    if ( GUCEF_NULL == fieldStart || GUCEF_NULL == expected )
        return false;
    CORE::UInt32 expectedLen = (CORE::UInt32) ::strlen( expected );
    if ( fieldLen != expectedLen )
        return false;
    return ::memcmp( fieldStart, expected, fieldLen ) == 0;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientMessage::ScanSessionFields( const char* msgStart               ,
                                      CORE::UInt32 msgLen                ,
                                      CFIXClientSessionFields& outFields )
{
    // Maximum value length we accept for any session-level field we store [S3]
    static const CORE::UInt32 MAX_SESSION_FIELD_VALUE_LEN = 256;
    // Maximum tag number digits [S6]
    static const CORE::UInt32 MAX_TAG_DIGITS = 6;

    const char  soh    = CFIXClientMessage::SOH;
    const char* msgEnd = msgStart + msgLen;
    const char* pos    = msgStart;

    while ( pos < msgEnd )
    {
        // [S6] Parse tag number: max MAX_TAG_DIGITS ASCII digits before '='
        CORE::UInt32 tag        = 0;
        CORE::UInt32 digitCount = 0;
        while ( pos < msgEnd && digitCount < MAX_TAG_DIGITS )
        {
            char c = *pos;
            if ( c == '=' )
                break;
            if ( c < '0' || c > '9' )
                return false;  // [S6] non-digit before '=' — malformed
            tag = tag * 10 + (CORE::UInt32)( (CORE::UInt8)c - '0' );
            ++pos;
            ++digitCount;
        }
        if ( pos >= msgEnd )
            break;  // end of message (normal after last field's SOH)
        if ( *pos != '=' )
        {
            // More than MAX_TAG_DIGITS digits or non-'=' after digits — malformed [S6]
            if ( digitCount >= MAX_TAG_DIGITS )
                return false;
            break;  // likely end of data
        }
        ++pos;  // skip '='

        // [S3] Scan value until SOH, strictly bounded within [msgStart, msgEnd)
        const char* valueStart = pos;
        while ( pos < msgEnd && *pos != soh )
            ++pos;
        if ( pos >= msgEnd )
            return false;  // [S3] malformed — no SOH terminating this field value

        CORE::UInt32 valueLen = (CORE::UInt32)( pos - valueStart );
        ++pos;  // skip SOH

        // Store only session-relevant fields; apply [S3] cap only to fields we record
        switch ( tag )
        {
            case 8:
                if ( valueLen > MAX_SESSION_FIELD_VALUE_LEN ) return false;
                outFields.beginStringStart = valueStart;
                outFields.beginStringLen   = valueLen;
                break;
            case 34:
                if ( valueLen > MAX_SESSION_FIELD_VALUE_LEN ) return false;
                outFields.seqNumStart = valueStart;
                outFields.seqNumLen   = valueLen;
                outFields.seqNumVal   = ParseUInt64Inline( valueStart, valueLen );  // [S4]
                break;
            case 35:
                if ( valueLen > MAX_SESSION_FIELD_VALUE_LEN ) return false;
                outFields.msgTypeStart = valueStart;
                outFields.msgTypeLen   = valueLen;
                break;
            case 43:
                if ( valueLen > MAX_SESSION_FIELD_VALUE_LEN ) return false;
                outFields.possDupFlagStart = valueStart;
                outFields.possDupFlagLen   = valueLen;
                break;
            case 49:
                if ( valueLen > MAX_SESSION_FIELD_VALUE_LEN ) return false;
                outFields.senderStart = valueStart;
                outFields.senderLen   = valueLen;
                break;
            case 56:
                if ( valueLen > MAX_SESSION_FIELD_VALUE_LEN ) return false;
                outFields.targetStart = valueStart;
                outFields.targetLen   = valueLen;
                break;
            case 7:
                if ( valueLen > MAX_SESSION_FIELD_VALUE_LEN ) return false;
                outFields.beginSeqNoStart = valueStart;
                outFields.beginSeqNoLen   = valueLen;
                break;
            case 36:
                if ( valueLen > MAX_SESSION_FIELD_VALUE_LEN ) return false;
                outFields.newSeqNoStart = valueStart;
                outFields.newSeqNoLen   = valueLen;
                break;
            case 108:
                if ( valueLen > MAX_SESSION_FIELD_VALUE_LEN ) return false;
                outFields.hbIntStart = valueStart;
                outFields.hbIntLen   = valueLen;
                break;
            case 112:
                if ( valueLen > MAX_SESSION_FIELD_VALUE_LEN ) return false;
                outFields.testReqIdStart = valueStart;
                outFields.testReqIdLen   = valueLen;
                break;
            case 141:
                if ( valueLen > MAX_SESSION_FIELD_VALUE_LEN ) return false;
                outFields.resetFlagStart = valueStart;
                outFields.resetFlagLen   = valueLen;
                break;
            default:
                break;  // skip non-session fields — no cap, no allocation
        }
    }
    return true;
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
