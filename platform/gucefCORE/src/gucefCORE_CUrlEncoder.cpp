/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ?  */

#include "gucefCORE_CUrlEncoder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CUrlEncoder::EncodeToAscii( const CORE::CUtf8String& strToBeEncoded ,
                            CORE::CAsciiString& encodedStr          )
{GUCEF_TRACE;

    // We know that every char in the to-be-encoded string could potentially 
    // result in a url encoded set of 3 bytes. Hence we multiply by 3 for every char to cover the worst case scenario
    // this seems rather wasteful but keep in mind you can keep the 'encodedStr' variable around for repeat processing
    // as such you can end up with very few allocations.
    char* encodedChars = encodedStr.Reserve( strToBeEncoded.ByteSize() * 3, 0 );
    char* encodedCharPos = encodedChars;
    if ( GUCEF_NULL == encodedChars )
        return false;
        
    const char* utf8Cp = strToBeEncoded.C_String();
    for ( UInt32 i=0; i<strToBeEncoded.Length(); ++i )
    {              
        Int32 codePoint = 0;
        const char* newUtf8Cp = strToBeEncoded.NextCodepointPtr( utf8Cp, codePoint );
        if ( GUCEF_NULL != newUtf8Cp )
        {
            if ( ( codePoint >= '0' && codePoint <= '9' ) || // digits 0-9 are allowed
                 ( codePoint >= 'A' && codePoint <= 'Z' ) || // letters A-Z are allowed
                 ( codePoint >= 'a' && codePoint <= 'z' ) || // letters a-z are allowed
                 ( codePoint == '-' || codePoint == '.' || codePoint == '_' || codePoint == '~' ) ) // a few special chars are allowed: "-", ".", "_", "~"
            {
                // Since we checked the ranges we can safely cast to a char
                // ASCII is a subset of UTF8 and this is a subset of ASCII
                *encodedCharPos = (char) codePoint;
                ++encodedCharPos;
            }
            else
            {
                while ( utf8Cp != newUtf8Cp )
                {
                    // Encode the current UTF8 byte
                    char hexCharA = 0;
                    char hexCharB = 0;
                    ConvertByteToHexChars( *utf8Cp, hexCharA, hexCharB );
 
                    *encodedCharPos = '%';
                    ++encodedCharPos;
                    *encodedCharPos = hexCharA;
                    ++encodedCharPos;
                    *encodedCharPos = hexCharB;
                    ++encodedCharPos;
                
                    ++utf8Cp;
                }
            }

            utf8Cp = newUtf8Cp;
        }
        else
        {
            // We already know we should have enough code points
            // As such inability to read them suggests the string is malformed
            return false;
        }
    }

    //encodedStr.SetLength( (UInt32) (encodedCharPos-encodedChars) );

    // @TODO: SetLength doesnt work well here due to pending rework of legacy AsciiString internals
    *encodedCharPos = '\0';
    encodedStr.DetermineLength();

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUrlEncoder::EncodeToAscii( const CORE::CAsciiString& strToBeEncoded , 
                            CORE::CAsciiString& encodedStr           )
{GUCEF_TRACE;

    // We know that every char in the to-be-encoded string could potentially 
    // result in a url encoded set of 3 bytes. Hence we multiply by 3 for every char to cover the worst case scenario
    // this seems rather wasteful but keep in mind you can keep the 'encodedStr' variable around for repeat processing
    // as such you can end up with very few allocations.
    char* encodedChars = encodedStr.Reserve( strToBeEncoded.ByteSize() * 3, 0 );
    char* encodedCharPos = encodedChars;    
    if ( GUCEF_NULL == encodedChars )
        return false;

    // Since the input is just ASCII we only have to worry about a few cases and no variable length encoding
    for ( UInt32 i=0; i<strToBeEncoded.Length(); ++i )
    {
        char codePoint = strToBeEncoded[ i ];
        if ( ( codePoint >= '0' && codePoint <= '9' ) || // digits 0-9 are allowed
             ( codePoint >= 'A' && codePoint <= 'Z' ) || // letters A-Z are allowed
             ( codePoint >= 'a' && codePoint <= 'z' ) || // letters a-z are allowed
             ( codePoint == '-' || codePoint == '.' || codePoint == '_' || codePoint == '~' ) ) // a few special chars are allowed: "-", ".", "_", "~"
        {
            *encodedCharPos = codePoint;
            ++encodedCharPos;
        }
        else
        {
            char hexCharA = 0;
            char hexCharB = 0;
            ConvertByteToHexChars( codePoint, hexCharA, hexCharB );
            
            *encodedCharPos = '%';
            ++encodedCharPos;
            *encodedCharPos = hexCharA;
            ++encodedCharPos;
            *encodedCharPos = hexCharB;
            ++encodedCharPos;
        }
    }

    //encodedStr.SetLength( (UInt32) (encodedCharPos-encodedChars) );
    
    // @TODO: SetLength doesnt work well here due to pending rework of legacy AsciiString internals
    *encodedCharPos = '\0';
    encodedStr.DetermineLength();

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUrlEncoder::DecodeFromAscii( const CORE::CAsciiString& strToBeDecoded , 
                              CORE::CUtf8String& decodedStr            )
{GUCEF_TRACE;

    // since the hex encoding takes 3 bytes per utf8 byte we can count on the ascii string being >= the length of
    // the utf8 decoded string. This helps avoid unneeded memory allocations
    decodedStr.Reserve( strToBeDecoded.ByteSize(), 0 );
         
    // Since the input is just ASCII we only have to worry about a few cases and no variable length encoding
    UInt32 utf8BytesCollected = 0;    
    char utf8TempBuffer[ 5 ] = { 0, 0, 0, 0, 0 };
    for ( UInt32 i=0; i<strToBeDecoded.Length(); ++i )
    {
        char codePoint = strToBeDecoded[ i ];

        while ( '%' == codePoint && utf8BytesCollected < 4 )
        {
            // Since we have found an encoding marker we need to check the next 2 chars
            if ( i+2<strToBeDecoded.Length() )
            {
                char hexCharA = strToBeDecoded[ i+1 ];
                char hexCharB = strToBeDecoded[ i+2 ];
                ConvertHexCharsToByte( hexCharA, hexCharB, utf8TempBuffer[ utf8BytesCollected ] ); 
                ++utf8BytesCollected; 
                i+=2;
            }
            else
            {
                // We should not have run out of bytes at this point
                return false;
            }

            // Did we gather enough bytes to generate a complete utf8 code point?
            Int32 utf32CodePoint = 0;
            Int32 bytesInCurrentUtf8CodePoint = CUtf8String::EncodeUtf8CodePointToUtf32( utf8TempBuffer, 5, utf32CodePoint ); 
            if ( -1 != bytesInCurrentUtf8CodePoint )
            {
                decodedStr += utf32CodePoint;   // Add a previously URL encoded code point
                memset( utf8TempBuffer, 0, 5 );
                utf8BytesCollected = 0;
            }
        }

        if ( 0 == utf8BytesCollected )
        {
            // ASCII fits within UTF8 so a cast will do
            decodedStr += (Int32) codePoint;
        }
        else
        {
            // Malformed: We should not have any partial UTF8 code points that we could not complete
            return false;
        }
    }
    
    return true;
}

/*-------------------------------------------------------------------------*/

void 
CUrlEncoder::Encode( const CORE::CUtf8String& strToBeEncoded, CString& encodedStr )
{GUCEF_TRACE;

    #if ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_ASCII_STRING )
    EncodeToAscii( strToBeEncoded, encodedStr );
    #else
    CORE::CAsciiString encodedAsciiStr;
    EncodeToAscii( strToBeEncoded, encodedAsciiStr );
    encodedStr = ToUtf8String( encodedAsciiStr );
    #endif
}

/*-------------------------------------------------------------------------*/

void
CUrlEncoder::Encode( const CORE::CAsciiString& strToBeEncoded, CString& encodedStr )
{GUCEF_TRACE;

    #if ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_ASCII_STRING )
    EncodeToAscii( strToBeEncoded, encodedStr );
    #else
    CORE::CAsciiString encodedAsciiStr;
    EncodeToAscii( strToBeEncoded, encodedAsciiStr );
    encodedStr = ToUtf8String( encodedAsciiStr );
    #endif
}

/*-------------------------------------------------------------------------*/

void
CUrlEncoder::Decode( const CORE::CUtf8String& strToBeDecoded, CUtf8String& decodedStr )
{GUCEF_TRACE;

    DecodeFromAscii( ToAsciiString( strToBeDecoded ), decodedStr );
}

/*-------------------------------------------------------------------------*/

void
CUrlEncoder::Decode( const CORE::CAsciiString& strToBeDecoded, CUtf8String& decodedStr )
{GUCEF_TRACE;

    DecodeFromAscii( strToBeDecoded, decodedStr );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
