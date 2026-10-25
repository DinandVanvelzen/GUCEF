/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_GUI_CTABCONTROL_H
#include "gucefGUI_CTabControl.h"
#define GUCEF_GUI_CTABCONTROL_H
#endif /* GUCEF_GUI_CTABCONTROL_H ? */

#include "gucefGUI_CTabContentPane.h"

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

CTabContentPane::CTabContentPane( void )
    : CPanel( "TabContentPane" )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CTabContentPane::~CTabContentPane()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CTabContentPane::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CTabContentPane();
}

/*-------------------------------------------------------------------------*/

CTabControl*
CTabContentPane::GetParentTabControl( void )
{GUCEF_TRACE;

    return CWidget::As< CTabControl >( GetParentWidget() );
}

/*-------------------------------------------------------------------------*/

const CString&
CTabContentPane::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CTabContentPane";
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
