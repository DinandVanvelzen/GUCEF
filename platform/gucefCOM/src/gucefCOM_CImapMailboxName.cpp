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


#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#include "gucefCOM_CImapMailboxName.h"

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

const char Base64Modified[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,";

int
ModifiedBase64Value( char c )
{
    if ( c >= 'A' && c <= 'Z' ) return c - 'A';
    if ( c >= 'a' && c <= 'z' ) return c - 'a' + 26;
    if ( c >= '0' && c <= '9' ) return c - '0' + 52;
    if ( '+' == c ) return 62;
    if ( ',' == c ) return 63;
    return -1;
}
/*-------------------------------------------------------------------------*/

void
FlushUtf16( GUCEF::vector< char >& out, GUCEF::vector< UInt16 >& units )
{
    if ( units.empty() )
        return;
    GUCEF::vector< UInt8 > bytes;
    for ( size_t i=0; i<units.size(); ++i )
    {
        bytes.push_back( (UInt8)( units[ i ] >> 8 ) );
        bytes.push_back( (UInt8)( units[ i ] & 0xFF ) );
    }
    out.push_back( '&' );
    UInt32 accumulator = 0;
    UInt32 bits = 0;
    for ( size_t i=0; i<bytes.size(); ++i )
    {
        accumulator = ( accumulator << 8 ) | bytes[ i ];
        bits += 8;
        while ( bits >= 6 )
        {
            bits -= 6;
            out.push_back( Base64Modified[ ( accumulator >> bits ) & 0x3F ] );
        }
    }
    if ( bits > 0 )
        out.push_back( Base64Modified[ ( accumulator << ( 6 - bits ) ) & 0x3F ] );
    out.push_back( '-' );
    units.clear();
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CImapMailboxName::Encode( const CORE::CString& utf8Name, CORE::CString& wireName )
{GUCEF_TRACE;

    wireName.Clear();
    const CORE::CString clean = CMimeCodec::SanitizeUtf8( utf8Name.C_String(), ByteLength( utf8Name ) );
    const UInt8* p = reinterpret_cast< const UInt8* >( clean.C_String() );
    const UInt32 size = ByteLength( clean );

    GUCEF::vector< char > out;
    GUCEF::vector< UInt16 > units;
    UInt32 i = 0;
    while ( i < size )
    {
        UInt32 cp = p[ i ];
        UInt32 length = 1;
        if ( cp >= 0xF0 ) { cp &= 0x07; length = 4; }
        else if ( cp >= 0xE0 ) { cp &= 0x0F; length = 3; }
        else if ( cp >= 0xC0 ) { cp &= 0x1F; length = 2; }
        for ( UInt32 k=1; k<length && i + k < size; ++k )
            cp = ( cp << 6 ) | ( p[ i + k ] & 0x3F );
        i += length;

        if ( cp < 0x20 || 0x7F == cp )
            return false;
        if ( cp >= 0x20 && cp <= 0x7E )
        {
            FlushUtf16( out, units );
            out.push_back( (char) cp );
            if ( '&' == cp )
                out.push_back( '-' );
        }
        else
        if ( cp < 0x10000 )
            units.push_back( (UInt16) cp );
        else
        {
            cp -= 0x10000;
            units.push_back( (UInt16)( 0xD800 + ( cp >> 10 ) ) );
            units.push_back( (UInt16)( 0xDC00 + ( cp & 0x3FF ) ) );
        }
    }
    FlushUtf16( out, units );
    if ( !out.empty() )
        wireName = CORE::CString( &out[ 0 ], (UInt32) out.size() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImapMailboxName::Decode( const CORE::CString& wireName, CORE::CString& utf8Name )
{GUCEF_TRACE;

    utf8Name.Clear();
    const char* p = wireName.C_String();
    const UInt32 size = ByteLength( wireName );
    GUCEF::vector< char > out;

    UInt32 i = 0;
    while ( i < size )
    {
        if ( '&' != p[ i ] )
        {
            if ( ( (UInt8) p[ i ] ) >= 0x80 )
            {
                // a server that sends UTF-8 (UTF8=ACCEPT) is read as it is
                out.push_back( p[ i++ ] );
                continue;
            }
            out.push_back( p[ i++ ] );
            continue;
        }
        ++i;
        if ( i < size && '-' == p[ i ] )
        {
            out.push_back( '&' );
            ++i;
            continue;
        }

        GUCEF::vector< UInt8 > bytes;
        UInt32 accumulator = 0;
        UInt32 bits = 0;
        while ( i < size && '-' != p[ i ] )
        {
            const int v = ModifiedBase64Value( p[ i ] );
            if ( v < 0 )
                return false;
            accumulator = ( accumulator << 6 ) | (UInt32) v;
            bits += 6;
            if ( bits >= 8 )
            {
                bits -= 8;
                bytes.push_back( (UInt8)( ( accumulator >> bits ) & 0xFF ) );
            }
            ++i;
        }
        if ( i >= size || bytes.size() % 2 != 0 || bytes.empty() )
            return false;
        ++i;

        for ( size_t u=0; u+1<bytes.size(); u+=2 )
        {
            UInt32 unit = ( (UInt32) bytes[ u ] << 8 ) | bytes[ u + 1 ];
            if ( unit >= 0xD800 && unit <= 0xDBFF )
            {
                if ( u + 3 >= bytes.size() )
                    return false;
                UInt32 low = ( (UInt32) bytes[ u + 2 ] << 8 ) | bytes[ u + 3 ];
                if ( low < 0xDC00 || low > 0xDFFF )
                    return false;
                unit = 0x10000 + ( ( unit - 0xD800 ) << 10 ) + ( low - 0xDC00 );
                u += 2;
            }
            else
            if ( unit >= 0xDC00 && unit <= 0xDFFF )
                return false;

            if ( unit < 0x80 ) out.push_back( (char) unit );
            else if ( unit < 0x800 ) { out.push_back( (char)( 0xC0 | ( unit >> 6 ) ) ); out.push_back( (char)( 0x80 | ( unit & 0x3F ) ) ); }
            else if ( unit < 0x10000 ) { out.push_back( (char)( 0xE0 | ( unit >> 12 ) ) ); out.push_back( (char)( 0x80 | ( ( unit >> 6 ) & 0x3F ) ) ); out.push_back( (char)( 0x80 | ( unit & 0x3F ) ) ); }
            else { out.push_back( (char)( 0xF0 | ( unit >> 18 ) ) ); out.push_back( (char)( 0x80 | ( ( unit >> 12 ) & 0x3F ) ) ); out.push_back( (char)( 0x80 | ( ( unit >> 6 ) & 0x3F ) ) ); out.push_back( (char)( 0x80 | ( unit & 0x3F ) ) ); }
        }
    }
    if ( !out.empty() )
        utf8Name = CMimeCodec::SanitizeUtf8( &out[ 0 ], (UInt32) out.size() );
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
