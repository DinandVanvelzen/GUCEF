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

#include "gucefGUI_CChartMarker.h"

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

CChartMarker::CChartMarker( void )
    : m_id()
    , m_x( 0 )
    , m_seriesIndex( -1 )
    , m_shape( SHAPE_DEFAULT )
    , m_hasColour( false )
    , m_colour()
    , m_styleClass()
    , m_layer()
    , m_label()
    , m_tooltip()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChartMarker::CChartMarker( const CString& id, Int64 x )
    : m_id( id )
    , m_x( x )
    , m_seriesIndex( -1 )
    , m_shape( SHAPE_DEFAULT )
    , m_hasColour( false )
    , m_colour()
    , m_styleClass()
    , m_layer()
    , m_label()
    , m_tooltip()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CChartMarker::SetId( const CString& id )
{GUCEF_TRACE;

    m_id = id;
}

/*-------------------------------------------------------------------------*/

const CString&
CChartMarker::GetId( void ) const
{GUCEF_TRACE;

    return m_id;
}

/*-------------------------------------------------------------------------*/

void
CChartMarker::SetX( Int64 x )
{GUCEF_TRACE;

    m_x = x;
}

/*-------------------------------------------------------------------------*/

Int64
CChartMarker::GetX( void ) const
{GUCEF_TRACE;

    return m_x;
}

/*-------------------------------------------------------------------------*/

void
CChartMarker::SetSeriesIndex( Int32 seriesIndex )
{GUCEF_TRACE;

    m_seriesIndex = seriesIndex < 0 ? -1 : seriesIndex;
}

/*-------------------------------------------------------------------------*/

Int32
CChartMarker::GetSeriesIndex( void ) const
{GUCEF_TRACE;

    return m_seriesIndex;
}

/*-------------------------------------------------------------------------*/

void
CChartMarker::SetShape( TShape shape )
{GUCEF_TRACE;

    m_shape = shape;
}

/*-------------------------------------------------------------------------*/

CChartMarker::TShape
CChartMarker::GetShape( void ) const
{GUCEF_TRACE;

    return m_shape;
}

/*-------------------------------------------------------------------------*/

void
CChartMarker::SetColour( const CColour& colour )
{GUCEF_TRACE;

    m_colour = colour;
    m_hasColour = true;
}

/*-------------------------------------------------------------------------*/

void
CChartMarker::ResetColour( void )
{GUCEF_TRACE;

    m_colour = CColour();
    m_hasColour = false;
}

/*-------------------------------------------------------------------------*/

bool
CChartMarker::HasColour( void ) const
{GUCEF_TRACE;

    return m_hasColour;
}

/*-------------------------------------------------------------------------*/

const CColour&
CChartMarker::GetColour( void ) const
{GUCEF_TRACE;

    return m_colour;
}

/*-------------------------------------------------------------------------*/

void
CChartMarker::SetStyleClass( const CString& styleClass )
{GUCEF_TRACE;

    m_styleClass = styleClass;
}

/*-------------------------------------------------------------------------*/

const CString&
CChartMarker::GetStyleClass( void ) const
{GUCEF_TRACE;

    return m_styleClass;
}

/*-------------------------------------------------------------------------*/

void
CChartMarker::SetLayer( const CString& layer )
{GUCEF_TRACE;

    m_layer = layer;
}

/*-------------------------------------------------------------------------*/

const CString&
CChartMarker::GetLayer( void ) const
{GUCEF_TRACE;

    return m_layer;
}

/*-------------------------------------------------------------------------*/

void
CChartMarker::SetLabel( const CString& label )
{GUCEF_TRACE;

    m_label = label;
}

/*-------------------------------------------------------------------------*/

const CString&
CChartMarker::GetLabel( void ) const
{GUCEF_TRACE;

    return m_label;
}

/*-------------------------------------------------------------------------*/

void
CChartMarker::SetTooltip( const CString& tooltip )
{GUCEF_TRACE;

    m_tooltip = tooltip;
}

/*-------------------------------------------------------------------------*/

const CString&
CChartMarker::GetTooltip( void ) const
{GUCEF_TRACE;

    return m_tooltip;
}

/*-------------------------------------------------------------------------*/

const char*
CChartMarker::ShapeToString( TShape shape )
{GUCEF_TRACE;

    switch ( shape )
    {
        case SHAPE_DIAMOND: return "diamond";
        case SHAPE_TRIANGLE_UP: return "triangle-up";
        case SHAPE_TRIANGLE_DOWN: return "triangle-down";
        case SHAPE_SQUARE: return "square";
        case SHAPE_CIRCLE: return "circle";
        case SHAPE_FLAG: return "flag";
        default: return "default";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
