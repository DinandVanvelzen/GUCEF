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

#ifndef GUCEF_GUI_CTABCONTENTPANE_H
#define GUCEF_GUI_CTABCONTENTPANE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CPANEL_H
#include "gucefGUI_CPanel.h"
#define GUCEF_GUI_CPANEL_H
#endif /* GUCEF_GUI_CPANEL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CTabControl;

/** One page of a tab control; the tab control shows the page of the active tab and hides the others */
class GUCEF_GUI_PUBLIC_CPP CTabContentPane : public CPanel
{
    public:

    CTabContentPane( void );

    virtual ~CTabContentPane();

    virtual CTabControl* GetParentTabControl( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "TabContentPane"; }

    static CWidget* Create( void );

    private:

    CTabContentPane( const CTabContentPane& src );
    CTabContentPane& operator=( const CTabContentPane& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTABCONTENTPANE_H ? */
