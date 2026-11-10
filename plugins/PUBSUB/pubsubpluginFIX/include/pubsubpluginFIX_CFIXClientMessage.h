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

#ifndef PUBSUBPLUGIN_FIX_CFIXCLIENTMESSAGE_H
#define PUBSUBPLUGIN_FIX_CFIXCLIENTMESSAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CASCIISTRING_H
#include "gucefCORE_CAsciiString.h"
#define GUCEF_CORE_CASCIISTRING_H
#endif /* GUCEF_CORE_CASCIISTRING_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXCLIENTSESSIONFIELDS_H
#include "pubsubpluginFIX_CFIXClientSessionFields.h"
#define PUBSUBPLUGIN_FIX_CFIXCLIENTSESSIONFIELDS_H
#endif /* PUBSUBPLUGIN_FIX_CFIXCLIENTSESSIONFIELDS_H ? */

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
 *  Note: Parsing is now done zero-copy via CFIXClientSessionFields + ScanSessionFields
 *  in CFIXClientPubSubClient. This class is builder-only.
 */
class PUBSUBPLUGIN_FIX_PLUGIN_PRIVATE_CPP CFIXClientMessage
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

    // --- Zero-copy session field parsing helpers (shared by client and server) ---

    /**
     *  Single-pass session-field scanner.
     *  Sets raw pointer views into the message buffer for all session-relevant tags.
     *  Strictly bounded to [msgStart, msgStart+msgLen).
     *  Returns false if the message is structurally malformed.
     */
    static bool ScanSessionFields( const char* msgStart               ,
                                   CORE::UInt32 msgLen                ,
                                   CFIXClientSessionFields& outFields );

    /**
     *  Parse a decimal integer inline from raw bytes.
     *  Returns 0 if len==0, len>20, or any non-digit character is found. [S4]
     */
    static CORE::UInt64 ParseUInt64Inline( const char* s, CORE::UInt32 len );

    /**
     *  Compare a raw field value against a null-terminated expected string.
     */
    static bool FieldMatchesValue( const char* fieldStart ,
                                   CORE::UInt32 fieldLen  ,
                                   const char* expected   );

    // --- Session message builders ---

    /**
     *  Build a FIX Logon message (MsgType=A)
     */
    static CORE::CAsciiString BuildLogon( const CORE::CAsciiString& senderCompId ,
                                          const CORE::CAsciiString& targetCompId ,
                                          const CORE::CAsciiString& fixVersion   ,
                                          CORE::UInt64 outgoingSeqNum            ,
                                          CORE::UInt32 heartbeatIntervalSecs     ,
                                          bool resetSeqNumFlag                    );

    /**
     *  Build a FIX Logout message (MsgType=5)
     */
    static CORE::CAsciiString BuildLogout( const CORE::CAsciiString& senderCompId                      ,
                                           const CORE::CAsciiString& targetCompId                      ,
                                           const CORE::CAsciiString& fixVersion                        ,
                                           CORE::UInt64 outgoingSeqNum                                 ,
                                           const CORE::CAsciiString& text = CORE::CAsciiString::Empty  );

    /**
     *  Build a FIX Heartbeat message (MsgType=0)
     */
    static CORE::CAsciiString BuildHeartbeat( const CORE::CAsciiString& senderCompId                          ,
                                              const CORE::CAsciiString& targetCompId                          ,
                                              const CORE::CAsciiString& fixVersion                            ,
                                              CORE::UInt64 outgoingSeqNum                                     ,
                                              const CORE::CAsciiString& testReqId = CORE::CAsciiString::Empty );

    /**
     *  Build a FIX TestRequest message (MsgType=1)
     */
    static CORE::CAsciiString BuildTestRequest( const CORE::CAsciiString& senderCompId ,
                                                const CORE::CAsciiString& targetCompId ,
                                                const CORE::CAsciiString& fixVersion   ,
                                                CORE::UInt64 outgoingSeqNum            ,
                                                const CORE::CAsciiString& testReqId    );

    /**
     *  Build a FIX ResendRequest message (MsgType=2)
     */
    static CORE::CAsciiString BuildResendRequest( const CORE::CAsciiString& senderCompId ,
                                                  const CORE::CAsciiString& targetCompId ,
                                                  const CORE::CAsciiString& fixVersion   ,
                                                  CORE::UInt64 outgoingSeqNum            ,
                                                  CORE::UInt64 beginSeqNo                ,
                                                  CORE::UInt64 endSeqNo                   );

    /**
     *  Build a FIX SequenceReset message (MsgType=4)
     */
    static CORE::CAsciiString BuildSequenceReset( const CORE::CAsciiString& senderCompId ,
                                                  const CORE::CAsciiString& targetCompId ,
                                                  const CORE::CAsciiString& fixVersion   ,
                                                  CORE::UInt64 outgoingSeqNum            ,
                                                  CORE::UInt64 newSeqNo                  ,
                                                  bool gapFillFlag                        );

    private:

    /**
     *  Calculate FIX CheckSum: sum of all byte values mod 256, formatted as 3 digits.
     *  @param msgWithoutChecksum  The message bytes NOT including the "10=xxx\x01" part
     *  @return 3-digit decimal checksum string
     */
    static CORE::CAsciiString CalcCheckSum( const CORE::CAsciiString& msgWithoutChecksum );

    /**
     *  Build the standard FIX header body (tags 35, 49, 56, 34, 52).
     *  Does NOT include tag 8 (BeginString) or tag 9 (BodyLength).
     */
    static CORE::CAsciiString BuildStandardBody( const CORE::CAsciiString& msgType      ,
                                                 const CORE::CAsciiString& senderCompId ,
                                                 const CORE::CAsciiString& targetCompId ,
                                                 CORE::UInt64 seqNum                    );

    /**
     *  Finalize a FIX message by prepending the BeginString+BodyLength header
     *  and appending the CheckSum trailer.
     *
     *  @param fixVersion  The FIX version string (e.g. "FIX.4.2")
     *  @param body        Everything from tag 35 onwards (without tag 8, 9, 10)
     *  @return            Complete FIX message ready to send
     */
    static CORE::CAsciiString FinalizeMsg( const CORE::CAsciiString& fixVersion ,
                                           const CORE::CAsciiString& body        );

    /**
     *  Get current UTC time formatted as FIX SendingTime (YYYYMMDD-HH:MM:SS)
     */
    static CORE::CAsciiString GetCurrentSendingTime( void );
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

#endif /* PUBSUBPLUGIN_FIX_CFIXCLIENTMESSAGE_H ? */
