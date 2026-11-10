/*
 *  pluginglueWAYLAND: Library that owns the connection to the Wayland compositor for the plugins that share it
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

#include "pluginglueWAYLAND_CWaylandScale.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace WAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
CWaylandScale::LogicalToBuffer( UInt32 logical, UInt32 scale120 )
{GUCEF_TRACE;

    if ( 0 == scale120 )
        scale120 = Denominator;
    const UInt64 product = (UInt64) logical * scale120;
    return (UInt32) ( ( product + ( Denominator / 2 ) ) / Denominator );
}

/*-------------------------------------------------------------------------*/

UInt32
CWaylandScale::BufferToLogical( UInt32 buffer, UInt32 scale120 )
{GUCEF_TRACE;

    if ( 0 == scale120 )
        scale120 = Denominator;
    const UInt64 product = (UInt64) buffer * Denominator;
    return (UInt32) ( ( product + scale120 - 1 ) / scale120 );
}

/*-------------------------------------------------------------------------*/

Int32
CWaylandScale::PointerToBuffer( wl_fixed_t coordinate, UInt32 scale120 )
{GUCEF_TRACE;

    if ( 0 == scale120 )
        scale120 = Denominator;

    // coordinate / 256 logical units, times scale120 / 120: round to the nearest with floor division so negative values round consistently
    const Int64 numerator = (Int64) coordinate * (Int64) scale120;
    const Int64 denominator = (Int64) 256 * (Int64) Denominator;
    Int64 shifted = numerator + denominator / 2;
    Int64 result = shifted / denominator;
    if ( shifted < 0 && 0 != shifted % denominator )
        --result;
    return (Int32) result;
}

/*-------------------------------------------------------------------------*/

UInt32
CWaylandScale::FromWholeScale( Int32 factor )
{GUCEF_TRACE;

    if ( factor < 1 )
        factor = 1;
    return (UInt32) factor * Denominator;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandScale::IsWholeScale( UInt32 scale120 )
{GUCEF_TRACE;

    return 0 == scale120 % Denominator;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWaylandScale::ToString( UInt32 scale120 )
{GUCEF_TRACE;

    const UInt32 whole = scale120 / Denominator;
    UInt32 fraction = ( ( scale120 % Denominator ) * 1000 ) / Denominator;
    CORE::CString text = CORE::UInt32ToString( whole );
    if ( 0 == fraction )
        return text;
    text += '.';
    char digits[ 3 ] = { (char) ( '0' + ( fraction / 100 ) % 10 ), (char) ( '0' + ( fraction / 10 ) % 10 ), (char) ( '0' + fraction % 10 ) };
    UInt32 count = 3;
    while ( count > 0 && '0' == digits[ count - 1 ] )
        --count;
    for ( UInt32 i=0; i<count; ++i )
        text += digits[ i ];
    return text;
}

/*-------------------------------------------------------------------------*/

CWaylandScaleTracker::CWaylandScaleTracker( void )
    : m_fractional120( 0 )
    , m_preferredWhole( 0 )
    , m_outputs()
    , m_fractionalCapable( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWaylandScaleTracker::SetFractionalCapable( bool capable )
{GUCEF_TRACE;

    m_fractionalCapable = capable;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandScaleTracker::IsFractionalCapable( void ) const
{GUCEF_TRACE;

    return m_fractionalCapable;
}

/*-------------------------------------------------------------------------*/

void
CWaylandScaleTracker::OnFractionalScale( UInt32 scale120 )
{GUCEF_TRACE;

    m_fractional120 = scale120;
}

/*-------------------------------------------------------------------------*/

void
CWaylandScaleTracker::OnPreferredBufferScale( Int32 factor )
{GUCEF_TRACE;

    m_preferredWhole = factor;
}

/*-------------------------------------------------------------------------*/

void
CWaylandScaleTracker::OnOutputEntered( const void* output, Int32 wholeScale )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_outputs.size(); ++i )
    {
        if ( m_outputs[ i ].output == output )
        {
            m_outputs[ i ].scale = wholeScale;
            return;
        }
    }
    SOutputScale entry;
    entry.output = output;
    entry.scale = wholeScale;
    m_outputs.push_back( entry );
}

/*-------------------------------------------------------------------------*/

void
CWaylandScaleTracker::OnOutputLeft( const void* output )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_outputs.size(); ++i )
    {
        if ( m_outputs[ i ].output == output )
        {
            m_outputs.erase( m_outputs.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CWaylandScaleTracker::OnOutputScaleChanged( const void* output, Int32 wholeScale )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_outputs.size(); ++i )
    {
        if ( m_outputs[ i ].output == output )
        {
            m_outputs[ i ].scale = wholeScale;
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

CWaylandScaleTracker::EScaleSource
CWaylandScaleTracker::GetSource( void ) const
{GUCEF_TRACE;

    if ( m_fractionalCapable && 0 != m_fractional120 )
        return SCALESOURCE_FRACTIONAL;
    if ( m_preferredWhole > 0 )
        return SCALESOURCE_PREFERRED;
    if ( !m_outputs.empty() )
        return SCALESOURCE_OUTPUTS;
    return SCALESOURCE_DEFAULT;
}

/*-------------------------------------------------------------------------*/

UInt32
CWaylandScaleTracker::GetScale120( void ) const
{GUCEF_TRACE;

    switch ( GetSource() )
    {
        case SCALESOURCE_FRACTIONAL :
            return m_fractional120;
        case SCALESOURCE_PREFERRED :
            return CWaylandScale::FromWholeScale( m_preferredWhole );
        case SCALESOURCE_OUTPUTS :
        {
            Int32 largest = 1;
            for ( UInt32 i=0; i<m_outputs.size(); ++i )
                if ( m_outputs[ i ].scale > largest )
                    largest = m_outputs[ i ].scale;
            return CWaylandScale::FromWholeScale( largest );
        }
        default :
            return CWaylandScale::Denominator;
    }
}

/*-------------------------------------------------------------------------*/

bool
CWaylandScaleTracker::UsesViewport( void ) const
{GUCEF_TRACE;

    // a viewport takes the fractional scales, so the window does not change between modes each time the scale does
    return m_fractionalCapable;
}

/*-------------------------------------------------------------------------*/

Int32
CWaylandScaleTracker::GetWholeBufferScale( void ) const
{GUCEF_TRACE;

    if ( m_fractionalCapable )
        return 1;
    return (Int32) ( GetScale120() / CWaylandScale::Denominator );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
