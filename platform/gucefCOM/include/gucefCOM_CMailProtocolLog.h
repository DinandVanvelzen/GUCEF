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

#ifndef GUCEF_COM_CMAILPROTOCOLLOG_H
#define GUCEF_COM_CMAILPROTOCOLLOG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COM_MAILTYPES_H
#include "gucefCOM_MailTypes.h"
#define GUCEF_COM_MAILTYPES_H
#endif /* GUCEF_COM_MAILTYPES_H ? */

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
 *  The one place where the mail clients write what goes over the wire to the log. Credentials, tokens and the
 *  content of messages never reach a log:
 *  - the arguments of the commands that carry a credential (AUTH, AUTHENTICATE, LOGIN, PASS, USER, APOP, with
 *    an IMAP tag in front or not) are replaced by the marker, only the name of the mechanism stays;
 *  - while an authentication exchange is going on, every line in both directions is the marker;
 *  - the text of what is left goes through CORE::CRedactor (tokens, key=value secrets, authentication schemes,
 *    e-mail addresses, IP addresses) and is cut at 300 bytes;
 *  - message data (DATA, RETR, TOP, FETCH literals) is logged as a number of bytes, never as text.
 *
 *  The trace is written at LOGLEVEL_EVERYTHING.
 */
class GUCEF_COM_PUBLIC_CPP CMailProtocolLog
{
    public:

    CMailProtocolLog( void );

    /** "SMTP", "POP3" or "IMAP" and the host: the start of every line that is written */
    void SetContext( const CORE::CString& protocol, const CORE::CString& host );

    /** A line sent to the server. 'secretExchange': the line is part of an authentication exchange */
    void LogClientLine( const CORE::CString& line, bool secretExchange ) const;

    /** A line received from the server */
    void LogServerLine( const CORE::CString& line, bool secretExchange ) const;

    /** Data that is not shown: what it is and how large */
    void LogData( bool outgoing, const char* what, UInt32 sizeInBytes ) const;

    /** What a line to the server looks like in a log */
    static CORE::CString RedactClientLine( const CORE::CString& line, bool secretExchange );

    /** What a line from the server looks like in a log */
    static CORE::CString RedactServerLine( const CORE::CString& line, bool secretExchange );

    private:

    CORE::CString m_context;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMAILPROTOCOLLOG_H ? */
