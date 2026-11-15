/*
 *  guidriverCEGUI: glue module for the CEGUI GUI backend
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef _CEGUITitlebar_h_
#include "CEGUI/widgets/Titlebar.h"
#define _CEGUITitlebar_h_
#endif /* _CEGUITitlebar_h_ ? */

#include "guidriverCEGUI_CWindowImp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERCEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CWindowImp::CWindowImp( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CWindowImp::~CWindowImp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
bool
CWindowImp::SetTitleBarText( const CString& title )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_window )
    {
        m_window->getTitlebar()->setText( title.C_String() );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWindowImp::GetTitleBarText( CString& title ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_window )
    {
        title = m_window->getTitlebar()->getText().c_str();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/
    
bool
CWindowImp::SetTitleBarVisible( const bool isVisible )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_window )
    {
        m_window->getTitlebar()->setVisible( isVisible );
        return true;
    }
    return false;
}
    
/*-------------------------------------------------------------------------*/

bool
CWindowImp::GetTitleBarVisible( bool& isVisible )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_window )
    {
        isVisible = m_window->getTitlebar()->isVisible();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWindowImp::ShowModal( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_window )
    {
        m_window->setModalState( true );
        m_window->moveToFront();
        CWidgetImp< GUCEF::GUI::CWindow >::SetVisibility( true );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWindowImp::Hide( void )
{GUCEF_TRACE;

    m_window->setModalState( false );
    CWidgetImp< GUCEF::GUI::CWindow >::SetVisibility( false );
    return true;
}

/*-------------------------------------------------------------------------*/
    
void
CWindowImp::Hook( CEGUI::FrameWindow* window )
{GUCEF_TRACE;

    m_window = window;
    CWidgetImp< GUCEF::GUI::CWindow >::Hook( window );
}
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERCEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
