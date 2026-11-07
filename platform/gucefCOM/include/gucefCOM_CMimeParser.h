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

#ifndef GUCEF_COM_CMIMEPARSER_H
#define GUCEF_COM_CMIMEPARSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMIMEPART_H
#include "gucefCOM_CMimePart.h"
#define GUCEF_COM_CMIMEPART_H
#endif /* GUCEF_COM_CMIMEPART_H ? */

#ifndef GUCEF_COM_CEMAILMESSAGE_H
#include "gucefCOM_CEMailMessage.h"
#define GUCEF_COM_CEMAILMESSAGE_H
#endif /* GUCEF_COM_CEMAILMESSAGE_H ? */

#ifndef GUCEF_COM_CMIMELIMITS_H
#include "gucefCOM_CMimeLimits.h"
#define GUCEF_COM_CMIMELIMITS_H
#endif /* GUCEF_COM_CMIMELIMITS_H ? */

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
 *  Reads Internet messages (RFC 5322, MIME RFC 2045 to 2049, 2047, 2231) from bytes.
 *
 *  It accepts what real mail looks like: CRLF, LF and CR line ends, an mbox "From " line in front, folded headers,
 *  multiparts without the closing boundary, transfer encodings with stray characters, file names in three
 *  different notations, encoded words in places they do not belong. What it cannot make sense of is not an
 *  error but a warning in CEMailMessage::warnings. It fails (false) only for what exceeds a limit.
 *  It never reads outside of the bytes it is given.
 */
class GUCEF_COM_PUBLIC_CPP CMimeParser
{
    public:

    CMimeParser( void );

    explicit CMimeParser( const CMimeLimits& limits );

    void SetLimits( const CMimeLimits& limits );

    const CMimeLimits& GetLimits( void ) const;

    /** Reads a whole message. @return false with an 'error' when a limit was exceeded; 'message' is then cleared */
    bool ParseMessage( const void* data, UInt32 size, CEMailMessage& message, CORE::CString* error = GUCEF_NULL ) const;

    /** Reads the structure of a message into a tree of parts */
    bool ParseParts( const void* data, UInt32 size, CMimePart& root, CORE::CString::StringVector* warnings = GUCEF_NULL, CORE::CString* error = GUCEF_NULL ) const;

    /**
     *  Reads a header section only (what the TOP command of POP3 and BODY[HEADER] of IMAP deliver) up to the
     *  empty line. 'bodyOffset' is where the body starts.
     */
    bool ParseHeaders( const void* data, UInt32 size, CEMailHeaders& headers, UInt32& bodyOffset, CORE::CString::StringVector* warnings = GUCEF_NULL, CORE::CString* error = GUCEF_NULL ) const;

    /** Fills the typed fields of the message (addresses, subject, date, ids) from header fields */
    static void ApplyHeaders( const CEMailHeaders& headers, CEMailMessage& message );

    /** The ids between angle brackets in a header value (Message-ID, In-Reply-To, References), at most 'maxIds' */
    static void ExtractMessageIds( const CORE::CString& headerValue, CORE::CString::StringVector& ids, UInt32 maxIds = 100 );

    private:

    struct SState
    {
        UInt32 partCount;
        CORE::CString::StringVector* warnings;
        CORE::CString error;
    };

    bool ParseEntity( const char* data, UInt32 size, CMimePart& part, UInt32 depth, bool topLevel, SState& state ) const;
    bool ParseMultipart( const char* data, UInt32 size, CMimePart& part, UInt32 depth, SState& state ) const;
    bool DecodeBody( const char* data, UInt32 size, CMimePart& part, SState& state ) const;
    void MapPart( const CMimePart& part, CEMailMessage& message, bool& haveText, bool& haveHtml, UInt32& extraTextCount, bool topLevel ) const;
    static void AddWarning( SState& state, const char* text );

    private:

    CMimeLimits m_limits;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMIMEPARSER_H ? */
