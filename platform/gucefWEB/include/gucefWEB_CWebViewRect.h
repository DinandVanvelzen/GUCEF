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

#ifndef GUCEF_WEB_CWEBVIEWRECT_H
#define GUCEF_WEB_CWEBVIEWRECT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPES_H
#include "gucefWEB_WebViewTypes.h"
#define GUCEF_WEB_WEBVIEWTYPES_H
#endif /* GUCEF_WEB_WEBVIEWTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A rectangle in pixels of a frame of a view: the dirty rectangles of frames and their unions. Empty when the width
 *  or the height is not positive. A plain value, copied freely, also between threads through the frame exchange.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewRect
{
    public:

    Int32 left;
    Int32 top;
    Int32 width;
    Int32 height;

    CWebViewRect( void );

    CWebViewRect( Int32 rectLeft, Int32 rectTop, Int32 rectWidth, Int32 rectHeight );

    bool IsEmpty( void ) const;

    /** One past the last column */
    Int32 GetRight( void ) const;

    /** One past the last row */
    Int32 GetBottom( void ) const;

    /** Makes this the bounding rectangle of this and 'other'; an empty rectangle adds nothing */
    void Unite( const CWebViewRect& other );

    /** What both have in common, empty when nothing */
    CWebViewRect Intersection( const CWebViewRect& other ) const;

    /** Cuts the rectangle to a frame of the given size; false when nothing is left */
    bool ClipTo( UInt32 frameWidth, UInt32 frameHeight );

    bool Contains( Int32 x, Int32 y ) const;

    /** True when 'other' lies inside this one (an empty one lies inside anything) */
    bool Contains( const CWebViewRect& other ) const;

    /** The number of pixels it covers */
    UInt64 GetArea( void ) const;

    void Clear( void );

    bool operator==( const CWebViewRect& other ) const;

    bool operator!=( const CWebViewRect& other ) const;

    /** "left,top widthxheight" for logs and reports */
    CString ToString( void ) const;

    /** All of a frame of the given size */
    static CWebViewRect Whole( UInt32 frameWidth, UInt32 frameHeight );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWRECT_H ? */
