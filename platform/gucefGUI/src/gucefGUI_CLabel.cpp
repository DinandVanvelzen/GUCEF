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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CLabel.h"

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

const CORE::CEvent CLabel::TextChangedEvent = "GUCEF::GUI::CLabel::TextChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CLabel::RegisterEvents( void )
{GUCEF_TRACE;

    TextChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CLabel::CLabel( void )
    : CWidget( "Label" )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CLabel::~CLabel()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CLabel::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CLabel();
}

/*-------------------------------------------------------------------------*/

bool
CLabel::SetText( const CString& text )
{GUCEF_TRACE;

    return SetProperty( EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

bool
CLabel::GetText( CString& text ) const
{GUCEF_TRACE;

    text = GetProperty( EWidgetProperty_Text );
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CLabel::GetText( void ) const
{GUCEF_TRACE;

    return GetProperty( EWidgetProperty_Text );
}

/*-------------------------------------------------------------------------*/

bool
CLabel::GetNaturalSize( Int32& width, Int32& height ) const
{GUCEF_TRACE;

    width = height = 0;
    if ( GUCEF_NULL == GetContext() )
        return false;
    return GetContext()->MeasureText( GetProperty( EWidgetProperty_Font ), GetProperty( EWidgetProperty_Text ), 0, width, height );
}

/*-------------------------------------------------------------------------*/

void
CLabel::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    if ( property == EWidgetProperty_Text )
    {
        NotifyObservers( TextChangedEvent );
        if ( !IsPropertySet( EWidgetProperty_Size ) && GUCEF_NULL != GetContext() )
            GetContext()->InvalidateLayout();
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CLabel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CLabel";
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
