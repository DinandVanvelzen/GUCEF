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

#include "gucefGUI_CGridColumnFit.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGridColumnFit::CGridColumnFit( void )
    : mode( MODE_AUTO )
    , minWidth( 0 )
    , maxWidth( 0 )
    , priority( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGridColumnFit::CGridColumnFit( EMode fitMode, Int32 minimumWidth, Int32 maximumWidth, Int32 fitPriority )
    : mode( fitMode )
    , minWidth( minimumWidth )
    , maxWidth( maximumWidth )
    , priority( fitPriority )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CGridColumnFit::operator==( const CGridColumnFit& other ) const
{GUCEF_TRACE;

    return mode == other.mode && minWidth == other.minWidth && maxWidth == other.maxWidth && priority == other.priority;
}

/*-------------------------------------------------------------------------*/

bool
CGridColumnFit::operator!=( const CGridColumnFit& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

CString
CGridColumnFit::ModeToString( EMode mode )
{GUCEF_TRACE;

    switch ( mode )
    {
        case MODE_CONTENT:  return "Content";
        case MODE_FIXED:    return "Fixed";
        case MODE_FILL:     return "Fill";
        case MODE_AUTO:
        default:            return "Auto";
    }
}

/*-------------------------------------------------------------------------*/

bool
CGridColumnFit::StringToMode( const CString& text, EMode& mode )
{GUCEF_TRACE;

    const CString lower = text.Lowercase();
    if ( "auto" == lower )
        mode = MODE_AUTO;
    else
    if ( "content" == lower )
        mode = MODE_CONTENT;
    else
    if ( "fixed" == lower )
        mode = MODE_FIXED;
    else
    if ( "fill" == lower )
        mode = MODE_FILL;
    else
        return false;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
