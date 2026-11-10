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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefGUI_CDrawSurface.h"

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

CDrawSurface::SCommand::SCommand( void )
    : type( COMMAND_FILLRECT )
    , rect()
    , source()
    , x0( 0 )
    , y0( 0 )
    , x1( 0 )
    , y1( 0 )
    , thickness( 1 )
    , colour()
    , image()
    , font()
    , text()
    , points()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CDrawSurface::CDrawSurface( void )
    : CWidget( "DrawSurface" )
    , m_building()
    , m_committed()
    , m_revision( 0 )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CDrawSurface::CDrawSurface( const CString& widgetTypeName )
    : CWidget( widgetTypeName )
    , m_building()
    , m_committed()
    , m_revision( 0 )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CDrawSurface::~CDrawSurface()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CDrawSurface::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CDrawSurface();
}

/*-------------------------------------------------------------------------*/

void
CDrawSurface::Clear( void )
{GUCEF_TRACE;

    m_building.clear();
}

/*-------------------------------------------------------------------------*/

void
CDrawSurface::FillRect( const CRect& rect, const CColour& colour )
{GUCEF_TRACE;

    TCommand command;
    command.type = COMMAND_FILLRECT;
    command.rect = rect;
    command.colour = colour;
    m_building.push_back( command );
}

/*-------------------------------------------------------------------------*/

void
CDrawSurface::StrokeRect( const CRect& rect, const CColour& colour, Int32 thickness )
{GUCEF_TRACE;

    TCommand command;
    command.type = COMMAND_STROKERECT;
    command.rect = rect;
    command.colour = colour;
    command.thickness = thickness;
    m_building.push_back( command );
}

/*-------------------------------------------------------------------------*/

void
CDrawSurface::DrawLine( Int32 x0, Int32 y0, Int32 x1, Int32 y1, const CColour& colour, Int32 thickness )
{GUCEF_TRACE;

    TCommand command;
    command.type = COMMAND_LINE;
    command.x0 = x0;
    command.y0 = y0;
    command.x1 = x1;
    command.y1 = y1;
    command.colour = colour;
    command.thickness = thickness;
    m_building.push_back( command );
}

/*-------------------------------------------------------------------------*/

void
CDrawSurface::DrawImage( const CString& image, const CRect& target, const CRect& source, const CColour& tint )
{GUCEF_TRACE;

    TCommand command;
    command.type = COMMAND_IMAGE;
    command.image = image;
    command.rect = target;
    command.source = source;
    command.colour = tint;
    m_building.push_back( command );
}

/*-------------------------------------------------------------------------*/

void
CDrawSurface::DrawString( const CString& font, const CString& text, Int32 x, Int32 y, const CColour& colour )
{GUCEF_TRACE;

    TCommand command;
    command.type = COMMAND_TEXT;
    command.font = font;
    command.text = text;
    command.x0 = x;
    command.y0 = y;
    command.colour = colour;
    m_building.push_back( command );
}

/*-------------------------------------------------------------------------*/

void
CDrawSurface::DrawPoints( const TPointVector& points, const CColour& colour )
{GUCEF_TRACE;

    if ( points.empty() )
        return;
    TCommand command;
    command.type = COMMAND_POINTS;
    command.points = points;
    command.colour = colour;
    m_building.push_back( command );
}

/*-------------------------------------------------------------------------*/

void
CDrawSurface::PushClip( const CRect& rect )
{GUCEF_TRACE;

    TCommand command;
    command.type = COMMAND_PUSHCLIP;
    command.rect = rect;
    m_building.push_back( command );
}

/*-------------------------------------------------------------------------*/

void
CDrawSurface::PopClip( void )
{GUCEF_TRACE;

    TCommand command;
    command.type = COMMAND_POPCLIP;
    m_building.push_back( command );
}

/*-------------------------------------------------------------------------*/

void
CDrawSurface::Commit( void )
{GUCEF_TRACE;

    m_committed.swap( m_building );
    m_building.clear();
    ++m_revision;
    // a runtime property, so the handler sees the new drawing with the next batch of changes
    SetProperty( EWidgetProperty_Revision, CORE::UInt32ToString( m_revision ) );
}

/*-------------------------------------------------------------------------*/

const CDrawSurface::TCommandVector&
CDrawSurface::GetCommands( void ) const
{GUCEF_TRACE;

    return m_committed;
}

/*-------------------------------------------------------------------------*/

UInt32
CDrawSurface::GetRevision( void ) const
{GUCEF_TRACE;

    return m_revision;
}

/*-------------------------------------------------------------------------*/

const CString&
CDrawSurface::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CDrawSurface";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
