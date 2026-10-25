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

#include "gucefGUI_CCheckbox.h"

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

const CORE::CEvent CCheckbox::BoxCheckedEvent = "GUCEF::GUI::CCheckbox::BoxCheckedEvent";
const CORE::CEvent CCheckbox::BoxUncheckedEvent = "GUCEF::GUI::CCheckbox::BoxUncheckedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CCheckbox::RegisterEvents( void )
{GUCEF_TRACE;

    BoxCheckedEvent.Initialize();
    BoxUncheckedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CCheckbox::CCheckbox( void )
    : CButton( "Checkbox" )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CCheckbox::~CCheckbox()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CCheckbox::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CCheckbox();
}

/*-------------------------------------------------------------------------*/

bool
CCheckbox::SetCheckedState( const bool isChecked )
{GUCEF_TRACE;

    return SetBoolProperty( EWidgetProperty_Checked, isChecked );
}

/*-------------------------------------------------------------------------*/

bool
CCheckbox::GetCheckedState( void ) const
{GUCEF_TRACE;

    return GetBoolProperty( EWidgetProperty_Checked );
}

/*-------------------------------------------------------------------------*/

bool
CCheckbox::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( ActivatedEvent == eventId )
    {
        ChangeValueByUser( EWidgetProperty_Checked, GetCheckedState() ? "false" : "true" );
        NotifyObservers( ButtonClickedEvent );
        return true;
    }
    return CButton::OnDefaultAction( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CCheckbox::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    if ( property == EWidgetProperty_Checked )
        NotifyObservers( GetCheckedState() ? BoxCheckedEvent : BoxUncheckedEvent );
}

/*-------------------------------------------------------------------------*/

const CString&
CCheckbox::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CCheckbox";
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
