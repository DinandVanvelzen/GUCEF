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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FIX */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
