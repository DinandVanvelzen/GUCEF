/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefWEB_CWebViewRect.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebViewRect::CWebViewRect( void )
    : left( 0 )
    , top( 0 )
    , width( 0 )
    , height( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewRect::CWebViewRect( Int32 rectLeft, Int32 rectTop, Int32 rectWidth, Int32 rectHeight )
    : left( rectLeft )
    , top( rectTop )
    , width( rectWidth )
    , height( rectHeight )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebViewRect::IsEmpty( void ) const
{GUCEF_TRACE;

    return width <= 0 || height <= 0;
}

/*-------------------------------------------------------------------------*/

Int32
CWebViewRect::GetRight( void ) const
{GUCEF_TRACE;

    return left + width;
}

/*-------------------------------------------------------------------------*/

Int32
CWebViewRect::GetBottom( void ) const
{GUCEF_TRACE;

    return top + height;
}

/*-------------------------------------------------------------------------*/

void
CWebViewRect::Unite( const CWebViewRect& other )
{GUCEF_TRACE;

    if ( other.IsEmpty() )
        return;
    if ( IsEmpty() )
    {
        *this = other;
        return;
    }
    const Int32 right = GetRight() > other.GetRight() ? GetRight() : other.GetRight();
    const Int32 bottom = GetBottom() > other.GetBottom() ? GetBottom() : other.GetBottom();
    left = left < other.left ? left : other.left;
    top = top < other.top ? top : other.top;
    width = right - left;
    height = bottom - top;
}

/*-------------------------------------------------------------------------*/

CWebViewRect
CWebViewRect::Intersection( const CWebViewRect& other ) const
{GUCEF_TRACE;

    if ( IsEmpty() || other.IsEmpty() )
        return CWebViewRect();
    const Int32 l = left > other.left ? left : other.left;
    const Int32 t = top > other.top ? top : other.top;
    const Int32 r = GetRight() < other.GetRight() ? GetRight() : other.GetRight();
    const Int32 b = GetBottom() < other.GetBottom() ? GetBottom() : other.GetBottom();
    if ( r <= l || b <= t )
        return CWebViewRect();
    return CWebViewRect( l, t, r - l, b - t );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewRect::ClipTo( UInt32 frameWidth, UInt32 frameHeight )
{GUCEF_TRACE;

    *this = Intersection( Whole( frameWidth, frameHeight ) );
    return !IsEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewRect::Contains( Int32 x, Int32 y ) const
{GUCEF_TRACE;

    return !IsEmpty() && x >= left && y >= top && x < GetRight() && y < GetBottom();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewRect::Contains( const CWebViewRect& other ) const
{GUCEF_TRACE;

    if ( other.IsEmpty() )
        return true;
    return !IsEmpty() && other.left >= left && other.top >= top && other.GetRight() <= GetRight() && other.GetBottom() <= GetBottom();
}

/*-------------------------------------------------------------------------*/

UInt64
CWebViewRect::GetArea( void ) const
{GUCEF_TRACE;

    return IsEmpty() ? 0 : (UInt64) width * (UInt64) height;
}

/*-------------------------------------------------------------------------*/

void
CWebViewRect::Clear( void )
{GUCEF_TRACE;

    left = top = width = height = 0;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewRect::operator==( const CWebViewRect& other ) const
{GUCEF_TRACE;

    return left == other.left && top == other.top && width == other.width && height == other.height;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewRect::operator!=( const CWebViewRect& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

CString
CWebViewRect::ToString( void ) const
{GUCEF_TRACE;

    return CORE::Int32ToString( left ) + "," + CORE::Int32ToString( top ) + " " + CORE::Int32ToString( width ) + "x" + CORE::Int32ToString( height );
}

/*-------------------------------------------------------------------------*/

CWebViewRect
CWebViewRect::Whole( UInt32 frameWidth, UInt32 frameHeight )
{GUCEF_TRACE;

    return CWebViewRect( 0, 0, (Int32) frameWidth, (Int32) frameHeight );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
