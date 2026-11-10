/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#include <stdlib.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefGUI_Types.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Splits on ',' and parses each part as a decimal integer; false when a part is not a number */
bool
ParseIntList( const CString& str, Int32* values, UInt32 maxValues, UInt32& count )
{
    count = 0;
    CString::StringVector parts = str.ParseElements( ',', true );
    if ( parts.empty() || parts.size() > maxValues )
        return false;
    for ( UInt32 i=0; i<(UInt32)parts.size(); ++i )
    {
        CString part = parts[ i ].Trim( true ).Trim( false );
        if ( part.IsNULLOrEmpty() )
            return false;
        const char* text = part.C_String();
        char* end = GUCEF_NULL;
        long value = strtol( text, &end, 10 );
        if ( end == text || '\0' != *end )
            return false;
        values[ count++ ] = (Int32) value;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRect::CRect( void )
    : left( 0 )
    , top( 0 )
    , width( 0 )
    , height( 0 )
{
}

/*-------------------------------------------------------------------------*/

CRect::CRect( Int32 l, Int32 t, Int32 w, Int32 h )
    : left( l )
    , top( t )
    , width( w )
    , height( h )
{
}

/*-------------------------------------------------------------------------*/

bool
CRect::Contains( Int32 x, Int32 y ) const
{
    return x >= left && y >= top && x < left + width && y < top + height;
}

/*-------------------------------------------------------------------------*/

bool
CRect::Intersects( const CRect& other ) const
{
    return !Intersection( other ).IsEmpty();
}

/*-------------------------------------------------------------------------*/

CRect
CRect::Intersection( const CRect& other ) const
{
    Int32 l = left > other.left ? left : other.left;
    Int32 t = top > other.top ? top : other.top;
    Int32 r = Right() < other.Right() ? Right() : other.Right();
    Int32 b = Bottom() < other.Bottom() ? Bottom() : other.Bottom();
    if ( r <= l || b <= t )
        return CRect( l, t, 0, 0 );
    return CRect( l, t, r - l, b - t );
}

/*-------------------------------------------------------------------------*/

bool
CRect::IsEmpty( void ) const
{
    return width <= 0 || height <= 0;
}

/*-------------------------------------------------------------------------*/

Int32
CRect::Right( void ) const
{
    return left + width;
}

/*-------------------------------------------------------------------------*/

Int32
CRect::Bottom( void ) const
{
    return top + height;
}

/*-------------------------------------------------------------------------*/

bool
CRect::operator==( const CRect& other ) const
{
    return left == other.left && top == other.top && width == other.width && height == other.height;
}

/*-------------------------------------------------------------------------*/

bool
CRect::operator!=( const CRect& other ) const
{
    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

CString
CRect::ToString( void ) const
{
    return CORE::Int32ToString( left ) + ',' + CORE::Int32ToString( top ) + ',' + CORE::Int32ToString( width ) + ',' + CORE::Int32ToString( height );
}

/*-------------------------------------------------------------------------*/

bool
CRect::FromString( const CString& str, CRect& rect )
{
    Int32 values[ 4 ];
    UInt32 count = 0;
    if ( !ParseIntList( str, values, 4, count ) || 4 != count )
        return false;
    rect = CRect( values[ 0 ], values[ 1 ], values[ 2 ], values[ 3 ] );
    return true;
}

/*-------------------------------------------------------------------------*/

CColour::CColour( void )
    : r( 0 )
    , g( 0 )
    , b( 0 )
    , a( 255 )
{
}

/*-------------------------------------------------------------------------*/

CColour::CColour( UInt8 red, UInt8 green, UInt8 blue, UInt8 alpha )
    : r( red )
    , g( green )
    , b( blue )
    , a( alpha )
{
}

/*-------------------------------------------------------------------------*/

bool
CColour::operator==( const CColour& other ) const
{
    return r == other.r && g == other.g && b == other.b && a == other.a;
}

/*-------------------------------------------------------------------------*/

bool
CColour::operator!=( const CColour& other ) const
{
    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

CString
CColour::ToString( void ) const
{
    return CORE::UInt32ToString( r ) + ',' + CORE::UInt32ToString( g ) + ',' + CORE::UInt32ToString( b ) + ',' + CORE::UInt32ToString( a );
}

/*-------------------------------------------------------------------------*/

bool
CColour::FromString( const CString& str, CColour& colour )
{
    CString text = str.Trim( true ).Trim( false );
    if ( text.IsNULLOrEmpty() )
        return false;

    if ( '#' == text[ 0 ] )
    {
        // #rrggbb or #rrggbbaa
        UInt32 digits = text.Length() - 1;
        if ( 6 != digits && 8 != digits )
            return false;
        UInt8 channels[ 4 ] = { 0, 0, 0, 255 };
        for ( UInt32 i=0; i<digits/2; ++i )
        {
            char pair[ 3 ] = { (char) text[ 1 + 2*i ], (char) text[ 2 + 2*i ], '\0' };
            char* end = GUCEF_NULL;
            long value = strtol( pair, &end, 16 );
            if ( end != pair + 2 )
                return false;
            channels[ i ] = (UInt8) value;
        }
        colour = CColour( channels[ 0 ], channels[ 1 ], channels[ 2 ], channels[ 3 ] );
        return true;
    }

    Int32 values[ 4 ];
    UInt32 count = 0;
    if ( !ParseIntList( text, values, 4, count ) || count < 3 )
        return false;
    for ( UInt32 i=0; i<count; ++i )
        if ( values[ i ] < 0 || values[ i ] > 255 )
            return false;
    colour = CColour( (UInt8) values[ 0 ], (UInt8) values[ 1 ], (UInt8) values[ 2 ], 4 == count ? (UInt8) values[ 3 ] : 255 );
    return true;
}

/*-------------------------------------------------------------------------*/

CRect
CalculatePresentRect( UInt32 logicalWidth  ,
                      UInt32 logicalHeight ,
                      UInt32 windowWidth   ,
                      UInt32 windowHeight  ,
                      TPresentMode mode    )
{
    if ( 0 == logicalWidth || 0 == logicalHeight || 0 == windowWidth || 0 == windowHeight )
        return CRect();

    if ( PRESENTMODE_STRETCH == mode )
        return CRect( 0, 0, (Int32) windowWidth, (Int32) windowHeight );

    UInt32 w = 0;
    UInt32 h = 0;
    if ( PRESENTMODE_INTEGER == mode )
    {
        UInt32 scaleX = windowWidth / logicalWidth;
        UInt32 scaleY = windowHeight / logicalHeight;
        UInt32 scale = scaleX < scaleY ? scaleX : scaleY;
        if ( 0 == scale )
        {
            // the window is smaller than the picture: fall back to a fitting non integer scale
            return CalculatePresentRect( logicalWidth, logicalHeight, windowWidth, windowHeight, PRESENTMODE_NEAREST );
        }
        w = logicalWidth * scale;
        h = logicalHeight * scale;
    }
    else
    {
        // the largest size with the aspect of the picture that fits
        if ( (UInt64) windowWidth * logicalHeight <= (UInt64) windowHeight * logicalWidth )
        {
            w = windowWidth;
            h = (UInt32) ( (UInt64) windowWidth * logicalHeight / logicalWidth );
        }
        else
        {
            h = windowHeight;
            w = (UInt32) ( (UInt64) windowHeight * logicalWidth / logicalHeight );
        }
    }
    return CRect( (Int32) ( windowWidth - w ) / 2, (Int32) ( windowHeight - h ) / 2, (Int32) w, (Int32) h );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
