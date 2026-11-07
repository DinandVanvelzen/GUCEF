/*
 *  notifysinkpluginDESKTOP: gucefMULTIPLAY match alert sink plugin for desktop alerts
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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "notifysinkpluginDESKTOP_CDesktopMatchAlertText.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace DESKTOP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Of the text that is given, no more than this is looked at: the rest cannot come through the limits anyway */
static const UInt32 MaxInputBytes = 4096;

/** The most that is taken of a title or a name when it is put in a wording */
static const UInt32 MaxValueBytes = 512;

/** The largest limit of a text that is accepted */
static const UInt32 MaxLimitBytes = 16384;

/** The ellipsis: U+2026 in UTF-8 */
static const char Ellipsis[ 3 ] = { (char) 0xE2, (char) 0x80, (char) 0xA6 };
static const UInt32 EllipsisBytes = 3;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The number of bytes of the text, the terminator the string keeps not counted */
static UInt32
ByteCount( const CORE::CString& text )
{GUCEF_TRACE;

    UInt32 size = text.ByteSize();
    return ( size > 0 ) ? size - 1 : 0;
}

/*-------------------------------------------------------------------------*/

/**
 *  Reads the character at p, of which at most 'available' bytes are there. Well formed UTF-8 only: the shortest form, no
 *  surrogates and nothing above U+10FFFF.
 *  @return the bytes of the character, or 0 when the bytes at p are not the start of a well formed character
 */
static UInt32
DecodeCharacter( const unsigned char* p, UInt32 available, UInt32& codePoint )
{GUCEF_TRACE;

    unsigned char lead = p[ 0 ];
    if ( lead < 0x80 )
    {
        codePoint = lead;
        return 1;
    }

    UInt32 extra = 0;
    unsigned char lowest = 0x80;
    unsigned char highest = 0xBF;
    if ( lead >= 0xC2 && lead <= 0xDF )
    {
        extra = 1;
        codePoint = lead & 0x1F;
    }
    else
    if ( lead >= 0xE0 && lead <= 0xEF )
    {
        extra = 2;
        codePoint = lead & 0x0F;
        if ( 0xE0 == lead ) lowest = 0xA0;          // no shorter form of a character below U+0800
        else
        if ( 0xED == lead ) highest = 0x9F;         // no surrogates
    }
    else
    if ( lead >= 0xF0 && lead <= 0xF4 )
    {
        extra = 3;
        codePoint = lead & 0x07;
        if ( 0xF0 == lead ) lowest = 0x90;          // no shorter form of a character below U+10000
        else
        if ( 0xF4 == lead ) highest = 0x8F;         // nothing above U+10FFFF
    }
    else
        return 0;

    if ( extra >= available )
        return 0;
    if ( p[ 1 ] < lowest || p[ 1 ] > highest )
        return 0;
    codePoint = ( codePoint << 6 ) | ( p[ 1 ] & 0x3F );
    for ( UInt32 i=2; i<=extra; ++i )
    {
        if ( p[ i ] < 0x80 || p[ i ] > 0xBF )
            return 0;
        codePoint = ( codePoint << 6 ) | ( p[ i ] & 0x3F );
    }
    return extra + 1;
}

/*-------------------------------------------------------------------------*/

/** Characters that are not shown as such but as a space: controls, line breaks, characters that hide or reorder text, white space */
static bool
IsSpaceLike( UInt32 codePoint )
{GUCEF_TRACE;

    return codePoint <= 0x20                                        ||      // C0 controls and the space
           ( codePoint >= 0x7F && codePoint <= 0xA0 )               ||      // DEL, C1 controls, no-break space
           ( codePoint >= 0x200B && codePoint <= 0x200F )           ||      // zero width characters and direction marks
           ( codePoint >= 0x2028 && codePoint <= 0x202E )           ||      // line and paragraph separators, embeddings and overrides
           ( codePoint >= 0x2060 && codePoint <= 0x2069 )           ||      // invisible operators, isolates
           0xFEFF == codePoint                                      ||      // byte order mark, zero width no-break space
           ( codePoint >= 0xE0000 && codePoint <= 0xE007F );                // tag characters
}

/*-------------------------------------------------------------------------*/

static UInt32
EncodedLength( UInt32 codePoint, bool escapeMarkup )
{GUCEF_TRACE;

    if ( escapeMarkup )
    {
        if ( '&' == codePoint ) return 5;       // &amp;
        if ( '<' == codePoint || '>' == codePoint ) return 4;       // &lt; &gt;
    }
    if ( codePoint < 0x80 ) return 1;
    if ( codePoint < 0x800 ) return 2;
    if ( codePoint < 0x10000 ) return 3;
    return 4;
}

/*-------------------------------------------------------------------------*/

static void
AppendEncoded( GUCEF::vector< char >& out, UInt32 codePoint, bool escapeMarkup )
{GUCEF_TRACE;

    if ( escapeMarkup )
    {
        const char* entity = GUCEF_NULL;
        if ( '&' == codePoint ) entity = "&amp;";
        else
        if ( '<' == codePoint ) entity = "&lt;";
        else
        if ( '>' == codePoint ) entity = "&gt;";
        if ( GUCEF_NULL != entity )
        {
            out.insert( out.end(), entity, entity + strlen( entity ) );
            return;
        }
    }

    if ( codePoint < 0x80 )
    {
        out.push_back( (char) codePoint );
    }
    else
    if ( codePoint < 0x800 )
    {
        out.push_back( (char)( 0xC0 | ( codePoint >> 6 ) ) );
        out.push_back( (char)( 0x80 | ( codePoint & 0x3F ) ) );
    }
    else
    if ( codePoint < 0x10000 )
    {
        out.push_back( (char)( 0xE0 | ( codePoint >> 12 ) ) );
        out.push_back( (char)( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) ) );
        out.push_back( (char)( 0x80 | ( codePoint & 0x3F ) ) );
    }
    else
    {
        out.push_back( (char)( 0xF0 | ( codePoint >> 18 ) ) );
        out.push_back( (char)( 0x80 | ( ( codePoint >> 12 ) & 0x3F ) ) );
        out.push_back( (char)( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) ) );
        out.push_back( (char)( 0x80 | ( codePoint & 0x3F ) ) );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CDesktopMatchAlertText::Sanitize( const CORE::CString& text, UInt32 maxBytes, bool escapeMarkup )
{GUCEF_TRACE;

    if ( 0 == maxBytes )
        return CORE::CString();
    if ( maxBytes > MaxLimitBytes )
        maxBytes = MaxLimitBytes;

    const unsigned char* bytes = (const unsigned char*) text.C_String();
    UInt32 size = ByteCount( text );
    if ( GUCEF_NULL == bytes || 0 == size )
        return CORE::CString();
    if ( size > MaxInputBytes )
        size = MaxInputBytes;

    // The characters of the text: anything invalid is gone, any run of what is shown as a space is one space, none at either end
    GUCEF::vector< UInt32 > characters;
    characters.reserve( size );
    bool spaceWaits = false;
    UInt32 totalBytes = 0;
    UInt32 at = 0;
    while ( at < size )
    {
        UInt32 codePoint = 0;
        UInt32 used = DecodeCharacter( bytes + at, size - at, codePoint );
        if ( 0 == used )
        {
            ++at;
            continue;
        }
        at += used;

        if ( IsSpaceLike( codePoint ) )
        {
            spaceWaits = true;
            continue;
        }
        if ( spaceWaits && !characters.empty() )
        {
            characters.push_back( ' ' );
            totalBytes += 1;
        }
        spaceWaits = false;
        characters.push_back( codePoint );
        totalBytes += EncodedLength( codePoint, escapeMarkup );
    }

    // What fits is written whole, otherwise as much as fits in front of an ellipsis (where there is room for one)
    bool cut = totalBytes > maxBytes;
    UInt32 budget = maxBytes;
    if ( cut && maxBytes >= EllipsisBytes + 1 )
        budget = maxBytes - EllipsisBytes;

    GUCEF::vector< char > out;
    out.reserve( ( cut ? budget : totalBytes ) + EllipsisBytes );
    UInt32 written = 0;
    for ( size_t i=0; i<characters.size(); ++i )
    {
        UInt32 length = EncodedLength( characters[ i ], escapeMarkup );
        if ( cut && written + length > budget )
            break;
        AppendEncoded( out, characters[ i ], escapeMarkup );
        written += length;
    }

    if ( cut && maxBytes >= EllipsisBytes + 1 )
    {
        while ( !out.empty() && ' ' == out.back() )
            out.pop_back();
        out.insert( out.end(), Ellipsis, Ellipsis + EllipsisBytes );
    }

    if ( out.empty() )
        return CORE::CString();
    return CORE::CString( &out[ 0 ], (UInt32) out.size() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDesktopMatchAlertText::SanitizeSummary( const CORE::CString& text )
{GUCEF_TRACE;

    return Sanitize( text, MaxSummaryBytes, false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDesktopMatchAlertText::SanitizeBody( const CORE::CString& text )
{GUCEF_TRACE;

    return Sanitize( text, MaxBodyBytes, true );
}

/*-------------------------------------------------------------------------*/

/** Appends the bytes of the value, no more than MaxValueBytes and not through the middle of a character */
static void
AppendValue( GUCEF::vector< char >& out, const CORE::CString& value )
{GUCEF_TRACE;

    const char* bytes = value.C_String();
    UInt32 size = ByteCount( value );
    if ( GUCEF_NULL == bytes )
        return;
    if ( size > MaxValueBytes )
    {
        size = MaxValueBytes;
        while ( size > 0 && 0x80 == ( ((unsigned char) bytes[ size ]) & 0xC0 ) )
            --size;
    }
    out.insert( out.end(), bytes, bytes + size );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDesktopMatchAlertText::Expand( const CORE::CString& format ,
                                  const CORE::CString& title  ,
                                  const CORE::CString& mover  ,
                                  UInt32 count                )
{GUCEF_TRACE;

    const char* bytes = format.C_String();
    UInt32 size = ByteCount( format );
    if ( GUCEF_NULL == bytes || 0 == size )
        return CORE::CString();
    if ( size > MaxInputBytes )
        size = MaxInputBytes;

    GUCEF::vector< char > out;
    out.reserve( size + 64 );
    UInt32 at = 0;
    while ( at < size )
    {
        if ( '{' == bytes[ at ] && at + 7 <= size )
        {
            if ( 0 == strncmp( bytes + at, "{title}", 7 ) )
            {
                AppendValue( out, title );
                at += 7;
                continue;
            }
            if ( 0 == strncmp( bytes + at, "{mover}", 7 ) )
            {
                AppendValue( out, mover );
                at += 7;
                continue;
            }
            if ( 0 == strncmp( bytes + at, "{count}", 7 ) )
            {
                AppendValue( out, CORE::UInt32ToString( count ) );
                at += 7;
                continue;
            }
        }
        out.push_back( bytes[ at ] );
        ++at;
    }

    if ( out.empty() )
        return CORE::CString();
    return CORE::CString( &out[ 0 ], (UInt32) out.size() );
}

/*-------------------------------------------------------------------------*/

bool
CDesktopMatchAlertText::IsValidCategory( const CORE::CString& category )
{GUCEF_TRACE;

    UInt32 size = ByteCount( category );
    if ( 0 == size || size > 64 )
        return false;
    const char* bytes = category.C_String();
    for ( UInt32 i=0; i<size; ++i )
    {
        char c = bytes[ i ];
        bool allowed = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '.' == c || '_' == c || '-' == c;
        if ( !allowed )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DESKTOP */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
