/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_AUTH_TESTAPP_TESTTEXT_H
#define GUCEF_AUTH_TESTAPP_TESTTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CURLENCODER_H
#include "gucefCORE_CUrlEncoder.h"
#define GUCEF_CORE_CURLENCODER_H
#endif /* GUCEF_CORE_CURLENCODER_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A text of the character, count times, made in one go: adding to a string one character at a time costs more with every character */
inline GUCEF::CORE::CString
RepeatedText( char c, GUCEF::UInt32 count )
{
    GUCEF::CORE::CDynamicBuffer buffer;
    buffer.SetDataSize( count );
    if ( count > 0 )
        memset( buffer.GetBufferPtr(), (unsigned char) c, count );
    return GUCEF::AUTH::MakeString( static_cast< const char* >( buffer.GetConstBufferPtr() ), count );
}

/** A text of the other text, count times, made in one go */
inline GUCEF::CORE::CString
RepeatedText( const char* text, GUCEF::UInt32 count )
{
    GUCEF::CORE::CDynamicBuffer buffer;
    size_t size = strlen( text );
    for ( GUCEF::UInt32 i=0; i<count; ++i )
        buffer.Append( text, (GUCEF::UInt32) size );
    return GUCEF::AUTH::MakeString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/** The percent encoding of the framework (what a login puts in an address), as a value */
inline GUCEF::CORE::CString
EncodeUrlText( const GUCEF::CORE::CString& text )
{
    GUCEF::CORE::CString encoded;
    GUCEF::CORE::CUrlEncoder::Encode( text, encoded );
    return encoded;
}

/** The text of a JSON string for the JSON documents that a fake provider writes by hand: quotes and escapes */
inline GUCEF::CORE::CString
QuoteJson( const GUCEF::CORE::CString& text )
{
    static const char* digits = "0123456789abcdef";
    const char* p = text.C_String();
    GUCEF::UInt32 size = GUCEF::AUTH::GetByteLength( text );
    GUCEF::CORE::CDynamicBuffer bytes;
    bytes.SetDataSize( 0 );
    bytes.Append( "\"", 1 );
    for ( GUCEF::UInt32 i=0; i<size; ++i )
    {
        unsigned char c = (unsigned char) p[ i ];
        switch ( c )
        {
            case '"' : bytes.Append( "\\\"", 2 ); break;
            case '\\' : bytes.Append( "\\\\", 2 ); break;
            case '\n' : bytes.Append( "\\n", 2 ); break;
            case '\r' : bytes.Append( "\\r", 2 ); break;
            case '\t' : bytes.Append( "\\t", 2 ); break;
            default :
            {
                if ( c < 0x20 )
                {
                    const char escape[ 6 ] = { '\\', 'u', '0', '0', digits[ c >> 4 ], digits[ c & 0xF ] };
                    bytes.Append( escape, 6 );
                }
                else
                    bytes.Append( &p[ i ], 1 );
            }
        }
    }
    bytes.Append( "\"", 1 );
    return GUCEF::AUTH::MakeString( static_cast< const char* >( bytes.GetConstBufferPtr() ), bytes.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTTEXT_H ? */
