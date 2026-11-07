/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMAILMESSAGE_H
#define GUCEF_COM_CMAILMESSAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_COM_CMAILADDRESS_H
#include "gucefCOM_CMailAddress.h"
#define GUCEF_COM_CMAILADDRESS_H
#endif /* GUCEF_COM_CMAILADDRESS_H ? */

#ifndef GUCEF_COM_CMAILHEADERS_H
#include "gucefCOM_CMailHeaders.h"
#define GUCEF_COM_CMAILHEADERS_H
#endif /* GUCEF_COM_CMAILHEADERS_H ? */

#ifndef GUCEF_COM_CMAILATTACHMENT_H
#include "gucefCOM_CMailAttachment.h"
#define GUCEF_COM_CMAILATTACHMENT_H
#endif /* GUCEF_COM_CMAILATTACHMENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An e-mail message as the application sees it: the addresses, the subject, the text and HTML bodies and the
 *  attachments. CMimeComposer writes one to the wire format and CMimeParser reads one from it.
 *
 *  Text is UTF-8. The line ends of the bodies of a parsed message are LF. Bcc recipients are used for the envelope
 *  and never written to the headers.
 */
class GUCEF_COM_PUBLIC_CPP CMailMessage : public CORE::CICloneable
{
    public:

    CMailAddress::TMailAddressVector from;
    CMailAddress sender;                        /**< Sender: when it differs from From */
    CMailAddress::TMailAddressVector replyTo;
    CMailAddress::TMailAddressVector to;
    CMailAddress::TMailAddressVector cc;
    CMailAddress::TMailAddressVector bcc;
    CORE::CString subject;
    CORE::CDateTime date;
    bool hasDate;                               /**< false: the composer uses the time of composing */
    CORE::CString messageId;                    /**< with the angle brackets, empty: the composer makes one */
    CORE::CString::StringVector inReplyTo;      /**< message ids with their angle brackets */
    CORE::CString::StringVector references;
    CORE::CString textBody;
    CORE::CString htmlBody;
    CMailAttachment::TMailAttachmentVector attachments;
    CMailHeaders extraHeaders;                  /**< other fields to write (X-...), and for a parsed message the fields that have no member */
    CMailHeaders headers;                       /**< parsed messages: every field as received */
    UInt32 rawSize;                             /**< parsed messages: the size on the wire */
    CORE::CString::StringVector warnings;       /**< parsed messages: what was not as it should be, in short phrases */

    CMailMessage( void );

    CMailMessage( const CMailMessage& src );

    virtual ~CMailMessage() GUCEF_VIRTUAL_OVERRIDE;

    CMailMessage& operator=( const CMailMessage& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    /** The recipients for the envelope: to, cc and bcc, in that order, without double entries */
    void GetEnvelopeRecipients( CMailAddress::TMailAddressVector& recipients ) const;

    /**
     *  Whether the message can be composed: a valid From, at least one recipient, valid addresses, valid ids and
     *  content types, names and values without CR, LF or NUL, extra headers that are wire safe and not generated by
     *  the composer. The reason is a short phrase that does not repeat the content of the message.
     */
    bool Validate( CORE::CString& reason, bool allowUtf8Addresses = false ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMAILMESSAGE_H ? */
