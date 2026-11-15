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

#include "gucefGUI_ETextOverflow.h"

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

struct SOverflowNames
{
    CString names[ TEXTOVERFLOW_COUNT ];
    CString ellipsis;

    SOverflowNames( void )
    {
        names[ TEXTOVERFLOW_CLIP ] = "Clip";
        names[ TEXTOVERFLOW_ELLIPSISEND ] = "EllipsisEnd";
        names[ TEXTOVERFLOW_ELLIPSISSTART ] = "EllipsisStart";
        names[ TEXTOVERFLOW_ELLIPSISMIDDLE ] = "EllipsisMiddle";
        ellipsis = "Ellipsis";
    }
};

const SOverflowNames&
GetOverflowNames( void )
{
    static const SOverflowNames names;
    return names;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString&
TextOverflowToString( ETextOverflow overflow )
{GUCEF_TRACE;

    if ( (UInt32) overflow >= (UInt32) TEXTOVERFLOW_COUNT )
        return CString::Empty;
    return GetOverflowNames().names[ overflow ];
}

/*-------------------------------------------------------------------------*/

ETextOverflow
StringToTextOverflow( const CString& value, ETextOverflow fallback )
{GUCEF_TRACE;

    const SOverflowNames& names = GetOverflowNames();
    if ( value == names.ellipsis )
        return TEXTOVERFLOW_ELLIPSISEND;
    for ( UInt32 i=0; i<(UInt32) TEXTOVERFLOW_COUNT; ++i )
        if ( value == names.names[ i ] )
            return (ETextOverflow) i;
    return fallback;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
