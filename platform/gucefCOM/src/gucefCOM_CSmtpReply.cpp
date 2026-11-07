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


#include "gucefCOM_CSmtpReply.h"

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

inline UInt32
ByteLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSmtpReply::CSmtpReply( void )
    : code( 0 )
    , enhancedCode()
    , lines()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CSmtpReply::GetText( void ) const
{GUCEF_TRACE;

    CORE::CString text;
    for ( size_t i=0; i<lines.size(); ++i )
    {
        if ( i > 0 )
            text += ' ';
        text += lines[ i ];
    }
    return text;
}

/*-------------------------------------------------------------------------*/

EMailErrorClass
CSmtpReply::GetErrorClass( UInt32 code )
{GUCEF_TRACE;

    if ( code >= 400 && code < 500 )
        return MAILERR_TEMPORARY;
    if ( code >= 500 && code < 600 )
        return MAILERR_PERMANENT;
    if ( code >= 200 && code < 400 )
        return MAILERR_NONE;
    return MAILERR_PROTOCOL;
}

/*-------------------------------------------------------------------------*/

bool
CSmtpReply::ParseLine( const CORE::CString& line, UInt32& code, bool& isLastLine, CORE::CString& text )
{GUCEF_TRACE;

    const char* p = line.C_String();
    const UInt32 size = ByteLength( line );
    if ( size < 3 )
        return false;
    for ( int i=0; i<3; ++i )
        if ( p[ i ] < '0' || p[ i ] > '9' )
            return false;
    code = (UInt32)( ( p[ 0 ] - '0' ) * 100 + ( p[ 1 ] - '0' ) * 10 + ( p[ 2 ] - '0' ) );
    if ( code < 200 || code > 599 )
        return false;
    if ( 3 == size )
    {
        isLastLine = true;
        text.Clear();
        return true;
    }
    if ( ' ' != p[ 3 ] && '-' != p[ 3 ] )
        return false;
    isLastLine = ' ' == p[ 3 ];
    text = size > 4 ? CORE::CString( p + 4, size - 4 ) : CORE::CString();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSmtpReply::SplitEnhancedCode( const CORE::CString& text, UInt32 replyCode, CORE::CString& enhancedCode, CORE::CString& rest )
{GUCEF_TRACE;

    // class.subject.detail: 2.0.0, 5.1.1, 4.7.12 and the class matches the first digit of the reply code
    const char* p = text.C_String();
    const UInt32 size = ByteLength( text );
    if ( size < 5 || p[ 0 ] != (char)( '0' + replyCode / 100 ) || '.' != p[ 1 ] )
        return false;
    UInt32 i = 2;
    UInt32 digits = 0;
    while ( i < size && p[ i ] >= '0' && p[ i ] <= '9' && digits < 3 ) { ++i; ++digits; }
    if ( 0 == digits || i >= size || '.' != p[ i ] )
        return false;
    ++i;
    digits = 0;
    while ( i < size && p[ i ] >= '0' && p[ i ] <= '9' && digits < 3 ) { ++i; ++digits; }
    if ( 0 == digits || ( i < size && ' ' != p[ i ] ) )
        return false;
    enhancedCode = CORE::CString( p, i );
    rest = i < size ? CORE::CString( p + i + 1, size - i - 1 ) : CORE::CString();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
