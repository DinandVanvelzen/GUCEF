/*
 *  guidriverMyGUI: glue module for the MyGUI GUI backend
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

#ifndef GUCEF_MYGUI_CFORMBACKENDIMP_H
#include "guceMyGUI_CFormBackendImp.h"
#define GUCEF_MYGUI_CFORMBACKENDIMP_H
#endif /* GUCEF_MYGUI_CFORMBACKENDIMP_H ? */

#ifndef GUIDRIVER_MYGUI_WIDGETS_H
#include "guidriverMyGUI_widgets.h"
#define GUIDRIVER_MYGUI_WIDGETS_H
#endif /* GUIDRIVER_MYGUI_WIDGETS_H ? */

#include "guidriverMyGUI_CMyGuiDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MYGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CMyGUIDriver::CMyGUIDriver( void )
    : GUI::CGUIDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMyGUIDriver::~CMyGUIDriver()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CMyGUIDriver::TStringSet
CMyGUIDriver::GetAvailableFormTypes( void )
{GUCEF_TRACE;

    TStringSet formsAvailable;

    formsAvailable.insert( "Form" );
    formsAvailable.insert( "FormEx" );

    return formsAvailable;
}

/*-------------------------------------------------------------------------*/

CMyGUIDriver::TStringSet
CMyGUIDriver::GetAvailableWidgetTypes( void )
{GUCEF_TRACE;

    TStringSet widgetsAvailable;

    widgetsAvailable.insert( "Widget" );
    widgetsAvailable.insert( "Window" );
    widgetsAvailable.insert( "Button" );

    return widgetsAvailable;
}

/*-------------------------------------------------------------------------*/

const GUI::CString&
CMyGUIDriver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static GUI::CString classTypeName = "GUCEF::MYGUI::CMyGUIDriver";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

GUCEF::GUI::CWidget*
CMyGUIDriver::CreateWidget( const GUI::CString& widgetName )
{GUCEF_TRACE;

    if ( "Widget" == widgetName )
    {
        MyGUI::Widget* widget = new MyGUI::Widget();
        TBasicWidgetImp* widgetWrapper = new TBasicWidgetImp();
        widgetWrapper->Hook( widget );
        return widgetWrapper;
    }
    else
    if ( "Window" == widgetName )
    {
        MyGUI::Window* widget = new MyGUI::Window();
        CWindowImp* widgetWrapper = new CWindowImp();
        widgetWrapper->Hook( widget );
        return widgetWrapper;
    }
    else
    if ( "Button" == widgetName )
    {
        MyGUI::Button* widget = new MyGUI::Button();
        CButtonImp* widgetWrapper = new CButtonImp();
        widgetWrapper->Hook( widget );
        return widgetWrapper;
    }

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CMyGUIDriver::DestroyWidget( GUI::CWidget* widget )
{GUCEF_TRACE;

    delete widget;
}

/*-------------------------------------------------------------------------*/

GUCEF::GUI::CFormBackend*
CMyGUIDriver::CreateFormBackend( void )
{GUCEF_TRACE;

    return new CFormBackendImp();
}

/*-------------------------------------------------------------------------*/

void
CMyGUIDriver::DestroyFormBackend( GUI::CFormBackend* formBackend )
{GUCEF_TRACE;

    delete static_cast< CFormBackendImp* >( formBackend );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MYGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
