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

#ifndef PUBSUBPLUGIN_FIX_CFIXMESSAGE_H
#define PUBSUBPLUGIN_FIX_CFIXMESSAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef PUBSUBPLUGIN_FIX_MACROS_H
#include "pubsubpluginFIX_macros.h"
#define PUBSUBPLUGIN_FIX_MACROS_H
#endif /* PUBSUBPLUGIN_FIX_MACROS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  FIX (Financial Information eXchange) protocol session message builder.
 *  Provides static builders for all FIX session-level messages.
 *
 *  Wire format uses SOH (ASCII 0x01) as field delimiter.
 *  Fields are TAG=VALUE\x01 pairs.
 *
 *  Note: Parsing is now done zero-copy via CFIXSessionFields + ScanSessionFields
 *  in CFIXPubSubClient. This class is builder-only.
 */
class PUBSUBPLUGIN_FIX_PLUGIN_PRIVATE_CPP CFIXMessage
{
    public:

    // SOH delimiter character used in FIX protocol
    static const char SOH;

    // FIX standard tag numbers (used by session scanner and topic metadata keys)
    static const CORE::UInt32 TAG_BEGIN_STRING    = 8;
    static const CORE::UInt32 TAG_BODY_LENGTH     = 9;
    static const CORE::UInt32 TAG_MSG_TYPE        = 35;
    static const CORE::UInt32 TAG_MSG_SEQ_NUM     = 34;
    static const CORE::UInt32 TAG_SENDER_COMP_ID  = 49;
    static const CORE::UInt32 TAG_TARGET_COMP_ID  = 56;
    static const CORE::UInt32 TAG_SENDING_TIME    = 52;
    static const CORE::UInt32 TAG_CHECK_SUM       = 10;
    static const CORE::UInt32 TAG_POSS_DUP_FLAG   = 43;
    static const CORE::UInt32 TAG_TEST_REQ_ID     = 112;
    static const CORE::UInt32 TAG_BEGIN_SEQ_NO    = 7;
    static const CORE::UInt32 TAG_END_SEQ_NO      = 16;
    static const CORE::UInt32 TAG_NEW_SEQ_NO      = 36;
    static const CORE::UInt32 TAG_GAP_FILL_FLAG   = 123;
    static const CORE::UInt32 TAG_HEARTBT_INT     = 108;
    static const CORE::UInt32 TAG_RESET_SEQ_NUM   = 141;
    static const CORE::UInt32 TAG_APPL_VER_ID     = 1128;

    // --- Session message builders ---

    /**
     *  Build a FIX Logon message (MsgType=A)
     */
    static CORE::CString BuildLogon( const CORE::CString& senderCompId,
                                     const CORE::CString& targetCompId,
                                     const CORE::CString& fixVersion,
                                     CORE::UInt64 outgoingSeqNum,
                                     CORE::UInt32 heartbeatIntervalSecs,
                                     bool resetSeqNumFlag );

    /**
     *  Build a FIX Logout message (MsgType=5)
     */
    static CORE::CString BuildLogout( const CORE::CString& senderCompId,
                                      const CORE::CString& targetCompId,
                                      const CORE::CString& fixVersion,
                                      CORE::UInt64 outgoingSeqNum,
                                      const CORE::CString& text = CORE::CString::Empty );

    /**
     *  Build a FIX Heartbeat message (MsgType=0)
     */
    static CORE::CString BuildHeartbeat( const CORE::CString& senderCompId,
                                         const CORE::CString& targetCompId,
                                         const CORE::CString& fixVersion,
                                         CORE::UInt64 outgoingSeqNum,
                                         const CORE::CString& testReqId = CORE::CString::Empty );

    /**
     *  Build a FIX TestRequest message (MsgType=1)
     */
    static CORE::CString BuildTestRequest( const CORE::CString& senderCompId,
                                           const CORE::CString& targetCompId,
                                           const CORE::CString& fixVersion,
                                           CORE::UInt64 outgoingSeqNum,
                                           const CORE::CString& testReqId );

    /**
     *  Build a FIX ResendRequest message (MsgType=2)
     */
    static CORE::CString BuildResendRequest( const CORE::CString& senderCompId,
                                             const CORE::CString& targetCompId,
                                             const CORE::CString& fixVersion,
                                             CORE::UInt64 outgoingSeqNum,
                                             CORE::UInt64 beginSeqNo,
                                             CORE::UInt64 endSeqNo );

    /**
     *  Build a FIX SequenceReset message (MsgType=4)
     */
    static CORE::CString BuildSequenceReset( const CORE::CString& senderCompId,
                                             const CORE::CString& targetCompId,
                                             const CORE::CString& fixVersion,
                                             CORE::UInt64 outgoingSeqNum,
                                             CORE::UInt64 newSeqNo,
                                             bool gapFillFlag );

    private:

    /**
     *  Calculate FIX CheckSum: sum of all byte values mod 256, formatted as 3 digits.
     *  @param msgWithoutChecksum  The message bytes NOT including the "10=xxx\x01" part
     *  @return 3-digit decimal checksum string
     */
    static CORE::CString CalcCheckSum( const CORE::CString& msgWithoutChecksum );

    /**
     *  Build the standard FIX header body (tags 35, 49, 56, 34, 52).
     *  Does NOT include tag 8 (BeginString) or tag 9 (BodyLength).
     */
    static CORE::CString BuildStandardBody( const CORE::CString& msgType,
                                            const CORE::CString& senderCompId,
                                            const CORE::CString& targetCompId,
                                            CORE::UInt64 seqNum );

    /**
     *  Finalize a FIX message by prepending the BeginString+BodyLength header
     *  and appending the CheckSum trailer.
     *
     *  @param fixVersion  The FIX version string (e.g. "FIX.4.2")
     *  @param body        Everything from tag 35 onwards (without tag 8, 9, 10)
     *  @return            Complete FIX message ready to send
     */
    static CORE::CString FinalizeMsg( const CORE::CString& fixVersion,
                                      const CORE::CString& body );

    /**
     *  Get current UTC time formatted as FIX SendingTime (YYYYMMDD-HH:MM:SS)
     */
    static CORE::CString GetCurrentSendingTime( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FIX */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_FIX_CFIXMESSAGE_H ? */
