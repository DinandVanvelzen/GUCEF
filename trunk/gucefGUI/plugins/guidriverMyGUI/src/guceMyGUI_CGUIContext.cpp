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

#include <assert.h>

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUIDRIVERMYGUI_CFORMBACKENDIMP_H
#include "guceMyGUI_CFormBackendImp.h"
#define GUCEF_GUIDRIVERMYGUI_CFORMBACKENDIMP_H
#endif /* GUCEF_GUIDRIVERMYGUI_CFORMBACKENDIMP_H ? */

#include "guceMyGUI_CGUIContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MYGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CGUIContext::CGUIContext( GUI::CGUIDriver& myGuiDriver       ,
                          INPUT::CInputContext* inputContext )
    : GUCEF::GUI::CIGUIContext()     ,
      m_driver( &myGuiDriver )       ,
      m_widgetSet()                  ,
      m_formSet()                    ,
      m_inputContext( inputContext )
{GUCEF_TRACE;

    assert( NULL != m_driver );
}

/*-------------------------------------------------------------------------*/

CGUIContext::~CGUIContext()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUCEF::GUI::CWidget*
CGUIContext::CreateWidget( const CString& widgetName )
{GUCEF_TRACE;

    //GUCEF::GUI::CWidget* widget = m_driver->CreateWidget( widgetName );
    //if ( NULL != widget )
    //{
    //    m_widgetSet.insert( widget );
    //    return widget;
    //}
    return NULL;
}

/*-------------------------------------------------------------------------*/
    
void
CGUIContext::DestroyWidget( GUCEF::GUI::CWidget* widget )
{GUCEF_TRACE;

    m_widgetSet.erase( widget );
    //m_driver->DestroyWidget( widget );
}

/*-------------------------------------------------------------------------*/
    
GUCEF::GUI::CForm*
CGUIContext::CreateForm( const CString& formName )
{GUCEF_TRACE;

    //GUCEF::GUI::CForm* form = m_driver->CreateForm( formName );
    //if ( NULL != form )
    //{
    //    m_formSet.insert( form );
    //    return form;
    //}
    return NULL;
}

/*-------------------------------------------------------------------------*/

void
CGUIContext::DestroyForm( GUCEF::GUI::CForm* form )
{GUCEF_TRACE;

    m_formSet.erase( form );
    //m_driver->DestroyForm( form ); 
}

/*-------------------------------------------------------------------------*/

CGUIContext::TStringSet
CGUIContext::GetAvailableFormTypes( void )
{GUCEF_TRACE;

    return CGUIContext::TStringSet(); //m_driver->GetAvailableFormTypes();
}

/*-------------------------------------------------------------------------*/
    
CGUIContext::TStringSet
CGUIContext::GetAvailableWidgetTypes( void )
{GUCEF_TRACE;

    return CGUIContext::TStringSet(); //m_driver->GetAvailableWidgetTypes();
}

/*-------------------------------------------------------------------------*/
    
GUCEF::GUI::CFormBackend*
CGUIContext::CreateFormBackend( void )
{GUCEF_TRACE;

    return NULL; //m_driver->CreateFormBackend();
}

/*-------------------------------------------------------------------------*/
    
void
CGUIContext::DestroyFormBackend( GUCEF::GUI::CFormBackend* formBackend )
{GUCEF_TRACE;

    //m_driver->DestroyFormBackend( formBackend );
}

/*-------------------------------------------------------------------------*/
    
GUCEF::GUI::CGUIDriver*
CGUIContext::GetDriver( void )
{GUCEF_TRACE;

    return m_driver;
}

/*-------------------------------------------------------------------------*/

CGUIContext::TWidgetSet
CGUIContext::GetOwnedWidgets( void )
{GUCEF_TRACE;
    
    return m_widgetSet;
}

/*-------------------------------------------------------------------------*/
    
CGUIContext::TFormSet
CGUIContext::GetOwnedForms( void )
{GUCEF_TRACE;

    return m_formSet;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MYGUI */
}; /* namespace GUCE */

/*-------------------------------------------------------------------------*/
