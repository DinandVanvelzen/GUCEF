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

#ifndef GUCEF_COM_CMIMECOMPOSER_H
#define GUCEF_COM_CMIMECOMPOSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMAILMESSAGE_H
#include "gucefCOM_CMailMessage.h"
#define GUCEF_COM_CMAILMESSAGE_H
#endif /* GUCEF_COM_CMAILMESSAGE_H ? */

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

/** How a message is written */
class GUCEF_COM_PUBLIC_CPP CMimeComposeOptions
{
    public:

    bool allowUtf8Addresses;        /**< addresses with non ASCII characters are written as they are (SMTPUTF8, RFC 6531), else they make the composition fail */
    UInt32 maxSize;                 /**< the composition fails when the result would be larger, 64 MiB */

    CMimeComposeOptions( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  Writes a CMailMessage as an Internet message (RFC 5322, MIME RFC 2045 to 2049).
 *
 *  The result is 7 bit clean with CR LF line ends, lines of at most 78 characters where folding is possible and
 *  never more than 998. Non ASCII text of the subject and of display names is written as RFC 2047 encoded words,
 *  file names with RFC 2231. Text bodies are 7bit, quoted-printable or base64 depending on their content,
 *  attachments base64. The structure follows from the content: a single text part, multipart/alternative for text
 *  and HTML, multipart/related when inline content is referenced, multipart/mixed with attachments.
 *
 *  A message that fails CMailMessage::Validate() is not written: nothing that has a CR, LF or NUL where it
 *  does not belong reaches the output.
 */
class GUCEF_COM_PUBLIC_CPP CMimeComposer
{
    public:

    CMimeComposer( void );

    explicit CMimeComposer( const CMimeComposeOptions& options );

    void SetOptions( const CMimeComposeOptions& options );

    const CMimeComposeOptions& GetOptions( void ) const;

    /** Appends the message to 'out'. @return false with the 'error' (a short phrase) when it cannot be written; 'out' is left as it was */
    bool Compose( const CMailMessage& message, CORE::CDynamicBuffer& out, CORE::CString& error ) const;

    private:

    CMimeComposeOptions m_options;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMIMECOMPOSER_H ? */
