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

#include "gucefGUI_CButton.h"

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

const CORE::CEvent CButton::ButtonDownEvent = "GUCEF::GUI::CButton::ButtonDownEvent";
const CORE::CEvent CButton::ButtonUpEvent = "GUCEF::GUI::CButton::ButtonUpEvent";
const CORE::CEvent CButton::ButtonClickedEvent = "GUCEF::GUI::CButton::ButtonClickedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CButton::RegisterEvents( void )
{GUCEF_TRACE;

    ButtonDownEvent.Initialize();
    ButtonUpEvent.Initialize();
    ButtonClickedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CButton::CButton( void )
    : CWidget( "Button" )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CButton::CButton( const CString& widgetTypeName )
    : CWidget( widgetTypeName )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CButton::~CButton()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWidget*
CButton::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CButton();
}

/*-------------------------------------------------------------------------*/

bool
CButton::SetButtonText( const CString& newText )
{GUCEF_TRACE;

    return SetProperty( EWidgetProperty_Text, newText );
}

/*-------------------------------------------------------------------------*/

bool
CButton::GetButtonText( CString& text ) const
{GUCEF_TRACE;

    text = GetProperty( EWidgetProperty_Text );
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CButton::GetCurrentImage( void ) const
{GUCEF_TRACE;

    CString normal = GetProperty( EWidgetProperty_NormalImage );
    CString image;
    UInt32 state = GetState();
    if ( 0 != ( state & STATE_DISABLED ) )
        image = GetProperty( EWidgetProperty_DisabledImage );
    else
    if ( 0 != ( state & STATE_PRESSED ) && 0 != ( state & STATE_HOVERED ) )
        image = GetProperty( EWidgetProperty_PushedImage );
    else
    if ( 0 != ( state & STATE_HOVERED ) )
        image = GetProperty( EWidgetProperty_HoverImage );
    else
    if ( 0 != ( state & STATE_CHECKED ) )
        image = GetProperty( EWidgetProperty_CheckedImage );
    return image.IsNULLOrEmpty() ? normal : image;
}

/*-------------------------------------------------------------------------*/

bool
CButton::GetNaturalSize( Int32& width, Int32& height ) const
{GUCEF_TRACE;

    width = height = 0;
    CGUIContext* context = GetContext();
    if ( GUCEF_NULL == context )
        return false;

    // the picture decides the size, else the text with a small margin
    CString normal = GetProperty( EWidgetProperty_NormalImage );
    if ( !normal.IsNULLOrEmpty() && context->GetImageLogicalSize( normal, width, height ) )
        return true;
    CString text = GetProperty( EWidgetProperty_Text );
    if ( !text.IsNULLOrEmpty() && context->MeasureText( GetProperty( EWidgetProperty_Font ), text, 0, width, height ) )
    {
        width += 8;
        height += 4;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CButton::IsActivatable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CButton::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseDownEvent == eventId )
    {
        NotifyObservers( ButtonDownEvent );
        return true;
    }
    if ( MouseUpEvent == eventId )
    {
        NotifyObservers( ButtonUpEvent );
        return true;
    }
    if ( ActivatedEvent == eventId )
    {
        NotifyObservers( ButtonClickedEvent );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CButton::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CButton";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
