/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHCODEC_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COM_CEMAILMESSAGE_H
#include "gucefCOM_CEMailMessage.h"
#define GUCEF_COM_CEMAILMESSAGE_H
#endif /* GUCEF_COM_CEMAILMESSAGE_H ? */

#ifndef GUCEF_MULTIPLAY_H
#include "gucefMULTIPLAY.h"
#define GUCEF_MULTIPLAY_H
#endif /* GUCEF_MULTIPLAY_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#include "matchprovidermail_macros.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H
#include "matchprovidermail_EMailMatchTypes.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINCOMING_H
#include "matchprovidermail_CEMailIncoming.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINCOMING_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINCOMING_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILOUTGOING_H
#include "matchprovidermail_CEMailOutgoing.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILOUTGOING_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILOUTGOING_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILSIGNEDSTATEMENT_H
#include "matchprovidermail_CEMailSignedStatement.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILSIGNEDSTATEMENT_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILSIGNEDSTATEMENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The setting keys of a match record that the mail provider reads */
#define GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_TRANSFER         "mailTransfer"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_SETTING_DISTRIBUTION     "mailDistribution"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Writes and reads the mails of the mail match provider; no network and no state. A mail of a match has the headers
 *  X-Redux-Match, X-Redux-Revision, X-Redux-Kind and X-Redux-Protocol (1), the subject
 *  "[Redux <first 8 of the match id> r<revision> t<turn>] <title>", a plain text body that a person can read without
 *  the game, and the data either as the attachment redux-<id8>-<name>.rdxturn or as armored text in the body between
 *  marker lines. Everything read from a mail is hostile: sizes are checked before anything is copied, attachment names are
 *  never used.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailMatchCodec
{
    public:

    static const char* HeaderMatch;             /**< "X-Redux-Match" */
    static const char* HeaderRevision;
    static const char* HeaderKind;
    static const char* HeaderProtocol;
    static const char* HeaderPackage;
    static const char* HeaderTransfer;
    static const char* HeaderRematchOf;
    static const char* ArmorLabelTurn;          /**< "MATCH TURN" */
    static const char* ArmorLabelData;          /**< "MATCH DATA" */
    static const UInt32 ProtocolVersion = 1;
    static const UInt32 MaxSubjectTitleBytes = 80;

    /** The title without anything that could end a header or confuse a list: control characters become spaces, at most MaxSubjectTitleBytes bytes on a character boundary */
    static CORE::CString SanitizeTitle( const CORE::CString& title );

    /** "[Redux 3f2504e0 r3 t3] title" */
    static CORE::CString MakeSubject( const CORE::CString& matchId, UInt32 revision, UInt32 turn, const CORE::CString& title );

    /** "redux-3f2504e0-r3.rdxturn": a name made here, from the match id only */
    static CORE::CString MakeAttachmentName( const CORE::CString& matchId, const CORE::CString& part );

    /** A new Message-ID with angle brackets, for a mail from the address: unique and unguessable */
    static CORE::CString MakeMessageId( const CORE::CString& fromAddress );

    /** Composes the message; false with a reason (no secrets, no addresses) when it is not valid */
    static bool Compose( const CEMailOutgoing& outgoing, COM::CEMailMessage& message, CORE::CString& reason );

    /** Reads what the headers say. Never fails: 'valid' says whether this is a mail of a match */
    static void ReadHeaders( const COM::CEMailHeaders& headers, CEMailIncoming& incoming );

    /** Whether the headers are those of a bounce, an automatic reply or a bulk message */
    static bool IsAutomatic( const COM::CEMailHeaders& headers );

    /**
     *  Takes the data out of a mail: the attachment (any name) when there is one, else the block of text of the match in the body.
     *
     *  @param label the label of the marker lines
     *  @param wantedRevision the revision of the block of text, 0 for any
     *  @param maxBytes the cap of the data
     */
    static TMailPayloadResult ExtractData( const COM::CEMailMessage& message  ,
                                           const CORE::CString& matchId      ,
                                           UInt32 wantedRevision             ,
                                           const CORE::CString& label        ,
                                           UInt32 maxBytes                   ,
                                           UInt32 maxTextBytes               ,
                                           CORE::CDynamicBuffer& data        ,
                                           TMailTransfer& usedTransfer       );

    /** The text of a signed record as bytes (a data tree stored as JSON), within maxBytes */
    static bool SerializeToBytes( const CORE::CIDataNodeSerializable& source, CORE::CDynamicBuffer& bytes, UInt32 maxBytes );

    /** The reverse, for bytes from outside: bounded, and the target decides what it accepts */
    static bool DeserializeFromBytes( const void* data, UInt32 size, CORE::CIDataNodeSerializable& target, UInt32 maxBytes );

    /** The transfer and distribution of a match from its settings, with the defaults (attachment, all) for what is missing or unknown */
    static TMailTransfer GetTransfer( const MULTIPLAY::CMatchInfo& info );
    static TMailDistribution GetDistribution( const MULTIPLAY::CMatchInfo& info );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHCODEC_H ? */
