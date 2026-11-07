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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILOUTGOING_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILOUTGOING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H
#include "matchprovidermail_EMailMatchTypes.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A mail to be written: the data of the match, and the people it is for */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailOutgoing
{
    public:

    TMailMatchKind kind;
    CORE::CString matchId;
    UInt32 revision;
    UInt32 turn;
    CORE::CString title;                    /**< as the players know the match; made safe for the subject */
    COM::CEMailAddress from;
    COM::CEMailAddress::TEMailAddressVector to;
    COM::CEMailAddress::TEMailAddressVector cc;
    CORE::CString inReplyTo;                /**< the message id of the previous mail of the match, with brackets */
    CORE::CString::StringVector references;
    CORE::CString actorName;                /**< who played, accepted, ...: for the readable text */
    CORE::CString nextName;                 /**< whose turn it is now */
    CORE::CString note;                     /**< an extra sentence for the readable text */
    CORE::CString bodyText;                 /**< lines of readable text for the body (the introduction mails), line ends CR LF or LF; every line is made safe and the whole is cut at a few KB */
    CORE::CString packageSha256;
    CORE::CString wantTransfer;
    CORE::CString rematchOf;                /**< the id of the match this one is the rematch of */
    CORE::CString linkText;                 /**< lines with the links that open the match in the application */
    TMailTransfer transfer;
    const CORE::CDynamicBuffer* data;       /**< the bytes that travel (a container, or a signed statement as data tree text), GUCEF_NULL for none */
    CORE::CString dataLabel;                /**< the label in the marker lines when the data travels as text: "MATCH TURN" or "MATCH DATA" */
    CORE::CString dataRevisionName;         /**< the part of the attachment name after the match: "r3" or "record" */
    const CORE::CDynamicBuffer* extraData;  /**< a second block that travels with the first (the first turn with an invitation), GUCEF_NULL for none */
    CORE::CString extraLabel;
    CORE::CString extraRevisionName;

    CEMailOutgoing( void );
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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILOUTGOING_H ? */
