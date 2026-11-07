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

#ifndef GUCEF_COM_CSMTPREPLY_H
#define GUCEF_COM_CSMTPREPLY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CEMAILCLIENTBASE_H
#include "gucefCOM_CEMailClientBase.h"
#define GUCEF_COM_CEMAILCLIENTBASE_H
#endif /* GUCEF_COM_CEMAILCLIENTBASE_H ? */

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

/** One reply of an SMTP server, which may span several lines */
class GUCEF_COM_PUBLIC_CPP CSmtpReply
{
    public:

    UInt32 code;                                /**< the three digits */
    CORE::CString enhancedCode;                 /**< "2.0.0", empty when the server gave none */
    CORE::CString::StringVector lines;          /**< the text of every line, without code and enhanced code */

    CSmtpReply( void );

    /** The text of all lines joined by a space */
    CORE::CString GetText( void ) const;

    /** What kind of failure a reply with this code is: 4xx temporary, 5xx permanent, 2xx and 3xx none */
    static EMailErrorClass GetErrorClass( UInt32 code );

    /**
     *  Takes a line of a reply apart: "250-text" continues the reply, "250 text" ends it. A code without text ("250") ends it.
     *  @return false when the line does not start with three digits followed by a space, a hyphen or the end
     */
    static bool ParseLine( const CORE::CString& line, UInt32& code, bool& isLastLine, CORE::CString& text );

    /** Reads an enhanced status code (RFC 3463) from the start of the text, "5.1.1 No such user" gives "5.1.1" and the rest of the text */
    static bool SplitEnhancedCode( const CORE::CString& text, UInt32 replyCode, CORE::CString& enhancedCode, CORE::CString& rest );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CSMTPREPLY_H ? */
