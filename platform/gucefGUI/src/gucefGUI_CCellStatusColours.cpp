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

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#include "gucefGUI_CCellStatusColours.h"

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

const char* const StatusNames[ SCellContent::STATUS_COUNT ] =
{
    "normal",
    "muted",
    "attention",
    "negative",
    "positive",
    "estimate",
    "unknown",
    "link"
};

bool
IsStatus( SCellContent::EStatus status )
{
    return (Int32) status >= 0 && (Int32) status < (Int32) SCellContent::STATUS_COUNT;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCellStatusColours::CCellStatusColours( void )
{GUCEF_TRACE;

    Reset();
}

/*-------------------------------------------------------------------------*/

void
CCellStatusColours::Reset( void )
{GUCEF_TRACE;

    for ( Int32 i=0; i<(Int32)SCellContent::STATUS_COUNT; ++i )
    {
        CColour colour;
        if ( !GetDefaultColour( (SCellContent::EStatus) i, colour ) )
            colour = CColour( 0, 0, 0, 0 );
        m_colours[ i ] = colour;
    }
}

/*-------------------------------------------------------------------------*/

bool
CCellStatusColours::GetDefaultColour( SCellContent::EStatus status, CColour& colour )
{GUCEF_TRACE;

    switch ( status )
    {
        case SCellContent::STATUS_MUTED:        colour = CColour( 150, 150, 156, 255 ); return true;
        case SCellContent::STATUS_ATTENTION:    colour = CColour( 240, 176, 72, 255 ); return true;
        case SCellContent::STATUS_NEGATIVE:     colour = CColour( 232, 110, 100, 255 ); return true;
        case SCellContent::STATUS_POSITIVE:     colour = CColour( 110, 200, 130, 255 ); return true;
        case SCellContent::STATUS_ESTIMATE:     colour = CColour( 150, 190, 240, 255 ); return true;
        case SCellContent::STATUS_UNKNOWN:      colour = CColour( 160, 160, 168, 255 ); return true;
        case SCellContent::STATUS_LINK:         colour = CColour( 120, 170, 255, 255 ); return true;
        case SCellContent::STATUS_NORMAL:
        default:                                return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CCellStatusColours::GetColour( SCellContent::EStatus status, CColour& colour ) const
{GUCEF_TRACE;

    if ( !IsStatus( status ) || SCellContent::STATUS_NORMAL == status )
        return false;
    colour = m_colours[ status ];
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCellStatusColours::SetColour( SCellContent::EStatus status, const CColour& colour )
{GUCEF_TRACE;

    if ( !IsStatus( status ) || SCellContent::STATUS_NORMAL == status )
        return;
    m_colours[ status ] = colour;
}

/*-------------------------------------------------------------------------*/

const char*
CCellStatusColours::StatusToString( SCellContent::EStatus status )
{GUCEF_TRACE;

    return IsStatus( status ) ? StatusNames[ status ] : "";
}

/*-------------------------------------------------------------------------*/

bool
CCellStatusColours::StringToStatus( const CString& name, SCellContent::EStatus& status )
{GUCEF_TRACE;

    const CString lower = name.Lowercase();
    for ( Int32 i=0; i<(Int32)SCellContent::STATUS_COUNT; ++i )
    {
        if ( lower == StatusNames[ i ] )
        {
            status = (SCellContent::EStatus) i;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCellStatusColours::Parse( const CString& text )
{GUCEF_TRACE;

    bool allGood = true;
    const TStringVector entries = SplitStringList( text );
    for ( UInt32 i=0; i<(UInt32)entries.size(); ++i )
    {
        const CString& entry = entries[ i ];
        if ( entry.IsNULLOrEmpty() )
            continue;
        const Int32 equals = entry.HasChar( '=', true );
        if ( equals <= 0 )
        {
            allGood = false;
            continue;
        }
        const CString name = entry.SubstrToIndex( (UInt32) equals, true ).Trim( true ).Trim( false );
        const CString value = entry.CutChars( (UInt32) equals + 1, true ).Trim( true ).Trim( false );
        SCellContent::EStatus status = SCellContent::STATUS_NORMAL;
        CColour colour;
        if ( !StringToStatus( name, status ) || SCellContent::STATUS_NORMAL == status || !CColour::FromString( value, colour ) )
        {
            allGood = false;
            continue;
        }
        m_colours[ status ] = colour;
    }
    return allGood;
}

/*-------------------------------------------------------------------------*/

CString
CCellStatusColours::ToString( void ) const
{GUCEF_TRACE;

    TStringVector entries;
    for ( Int32 i=1; i<(Int32)SCellContent::STATUS_COUNT; ++i )
        entries.push_back( CString( StatusNames[ i ] ) + "=" + m_colours[ i ].ToString() );
    return JoinStringList( entries );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
