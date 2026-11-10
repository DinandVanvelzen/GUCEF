/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CWindowContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CWindowContext::WindowContextRedrawEvent = "GUCEF::GUI::CWindowContext::WindowContextRedrawEvent";
const CORE::CEvent CWindowContext::WindowContextSizeEvent = "GUCEF::GUI::CWindowContext::WindowContextSizeEvent";
const CORE::CEvent CWindowContext::WindowContextActivateEvent = "GUCEF::GUI::CWindowContext::WindowContextActivateEvent";
const CORE::CEvent CWindowContext::WindowContextCloseRequestEvent = "GUCEF::GUI::CWindowContext::WindowContextCloseRequestEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CWindowContext::CWindowContext( void )
    : CORE::CObservingNotifier()
    , m_guiLayers()
    , m_systemCursorShown( -1 )
{GUCEF_TRACE;
                       
    RegisterEvents();
}

/*-------------------------------------------------------------------------*/                                

CWindowContext::~CWindowContext()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWindowContext::RegisterEvents( void )
{GUCEF_TRACE;

    WindowContextRedrawEvent.Initialize();
    WindowContextSizeEvent.Initialize();
    WindowContextActivateEvent.Initialize();
    WindowContextCloseRequestEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

bool
CWindowContext::SetSystemCursorVisible( bool visible )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CWindowContext::UpdateSystemCursor( void )
{GUCEF_TRACE;

    if ( m_guiLayers.empty() )
        return;
    CGUIContext* inputLayer = m_guiLayers.front();
    for ( TGuiContextLayers::const_reverse_iterator i=m_guiLayers.rbegin(); i!=m_guiLayers.rend(); ++i )
    {
        if ( (*i)->HasVisibleRoots() )
        {
            inputLayer = (*i);
            break;
        }
    }
    const bool visible = inputLayer->GetCursorImage().IsNULLOrEmpty();
    if ( m_systemCursorShown != ( visible ? 1 : 0 ) && SetSystemCursorVisible( visible ) )
        m_systemCursorShown = visible ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

namespace {

bool
LowerLayer( const CGUIContext* a, const CGUIContext* b )
{
    return a->GetLayer() < b->GetLayer();
}

}

/*-------------------------------------------------------------------------*/

const CWindowContext::TGuiContextLayers&
CWindowContext::GetGuiLayers( void ) const
{GUCEF_TRACE;

    return m_guiLayers;
}

/*-------------------------------------------------------------------------*/

void
CWindowContext::RegisterGuiLayer( CGUIContext* context )
{GUCEF_TRACE;

    if ( GUCEF_NULL == context || std::find( m_guiLayers.begin(), m_guiLayers.end(), context ) != m_guiLayers.end() )
        return;
    m_guiLayers.push_back( context );
    SortGuiLayers();
}

/*-------------------------------------------------------------------------*/

void
CWindowContext::UnregisterGuiLayer( CGUIContext* context )
{GUCEF_TRACE;

    TGuiContextLayers::iterator i = std::find( m_guiLayers.begin(), m_guiLayers.end(), context );
    if ( i != m_guiLayers.end() )
        m_guiLayers.erase( i );
}

/*-------------------------------------------------------------------------*/

void
CWindowContext::SortGuiLayers( void )
{GUCEF_TRACE;

    // equal layer numbers keep the order the contexts came in
    std::stable_sort( m_guiLayers.begin(), m_guiLayers.end(), &LowerLayer );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUI */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
