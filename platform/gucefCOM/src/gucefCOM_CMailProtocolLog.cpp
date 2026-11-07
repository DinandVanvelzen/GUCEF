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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCOM_CMailProtocolLog.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CRedactor
MakeTraceRedactor( void )
{
    CORE::CRedactor redactor( CORE::CRedactor::ForLogLine() );
    redactor.AddFlags( CORE::REDACT_TEXT_EMAILS | CORE::REDACT_SANITIZE_CONTROL );
    redactor.SetMaxLength( 300 );
    return redactor;
}

/*-------------------------------------------------------------------------*/

const CORE::CRedactor&
GetTraceRedactor( void )
{
    static const CORE::CRedactor redactor = MakeTraceRedactor();
    return redactor;
}

/*-------------------------------------------------------------------------*/

bool
IsCredentialCommand( const CORE::CString& word )
{
    return word.Equals( "AUTH", false )         ||
           word.Equals( "AUTHENTICATE", false ) ||
           word.Equals( "LOGIN", false )        ||
           word.Equals( "PASS", false )         ||
           word.Equals( "USER", false )         ||
           word.Equals( "APOP", false );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMailProtocolLog::CMailProtocolLog( void )
    : m_context( "MAIL" )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMailProtocolLog::SetContext( const CORE::CString& protocol, const CORE::CString& host )
{GUCEF_TRACE;

    m_context = protocol + " " + host;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMailProtocolLog::RedactClientLine( const CORE::CString& line, bool secretExchange )
{GUCEF_TRACE;

    if ( secretExchange )
        return GUCEF_COM_REDACTION_MARKER;

    // the command is the first word, or the second behind an IMAP tag
    CORE::CString::StringVector words = line.ParseElements( ' ', false );
    for ( size_t i=0; i<words.size() && i<2; ++i )
    {
        if ( !IsCredentialCommand( words[ i ] ) )
            continue;

        CORE::CString result;
        // what is kept: the tag, the command and for AUTH and AUTHENTICATE the name of the mechanism
        size_t keep = i + 1;
        if ( ( words[ i ].Equals( "AUTH", false ) || words[ i ].Equals( "AUTHENTICATE", false ) ) && words.size() > keep )
            ++keep;
        for ( size_t k=0; k<keep && k<words.size(); ++k )
            result += words[ k ] + " ";
        result += GUCEF_COM_REDACTION_MARKER;
        return GetTraceRedactor().RedactText( result );
    }
    return GetTraceRedactor().RedactText( line );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMailProtocolLog::RedactServerLine( const CORE::CString& line, bool secretExchange )
{GUCEF_TRACE;

    if ( secretExchange )
        return GUCEF_COM_REDACTION_MARKER;
    return GetTraceRedactor().RedactText( line );
}

/*-------------------------------------------------------------------------*/

void
CMailProtocolLog::LogClientLine( const CORE::CString& line, bool secretExchange ) const
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_EVERYTHING, m_context << " C: " << RedactClientLine( line, secretExchange ) );
}

/*-------------------------------------------------------------------------*/

void
CMailProtocolLog::LogServerLine( const CORE::CString& line, bool secretExchange ) const
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_EVERYTHING, m_context << " S: " << RedactServerLine( line, secretExchange ) );
}

/*-------------------------------------------------------------------------*/

void
CMailProtocolLog::LogData( bool outgoing, const char* what, UInt32 sizeInBytes ) const
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_EVERYTHING, m_context << ( outgoing ? " C: " : " S: " ) << what << " (" << sizeInBytes << " bytes, not shown)" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
