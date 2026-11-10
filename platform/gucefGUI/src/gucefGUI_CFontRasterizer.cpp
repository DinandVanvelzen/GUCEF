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

#include "gucefGUI_CFontRasterizer.h"

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

CFontRasterizer::SCodePointRange::SCodePointRange( void )
    : first( 0 )
    , last( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFontRasterizer::SCodePointRange::SCodePointRange( UInt32 firstCodePoint, UInt32 lastCodePoint )
    : first( firstCodePoint )
    , last( lastCodePoint )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFontRasterizer::TCodePointRanges
CFontRasterizer::GetLatin1Ranges( void )
{GUCEF_TRACE;

    TCodePointRanges ranges;
    ranges.push_back( TCodePointRange( 0x20, 0x7E ) );
    ranges.push_back( TCodePointRange( 0xA0, 0xFF ) );
    return ranges;
}

/*-------------------------------------------------------------------------*/

CFontRasterizer::TCodePointRanges
CFontRasterizer::GetUiRanges( void )
{GUCEF_TRACE;

    TCodePointRanges ranges = GetLatin1Ranges();
    ranges.push_back( TCodePointRange( 0x2022, 0x2022 ) );
    return ranges;
}

/*-------------------------------------------------------------------------*/

CFontRasterizer::CFontRasterizer( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFontRasterizer::~CFontRasterizer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
