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

#include <stdio.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CSpinner.h"

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

CSpinner::CSpinner( void )
    : CWidget( "Spinner" )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CSpinner::~CSpinner()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CSpinner::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CSpinner();
}

/*-------------------------------------------------------------------------*/

Float32
CSpinner::Clamp( Float32 value ) const
{GUCEF_TRACE;

    Float32 minimum = GetFloatProperty( EWidgetProperty_Min );
    Float32 maximum = GetFloatProperty( EWidgetProperty_Max );
    if ( value < minimum ) value = minimum;
    if ( value > maximum ) value = maximum;
    return value;
}

/*-------------------------------------------------------------------------*/

bool
CSpinner::SetValueUInt32( const UInt32 value )
{GUCEF_TRACE;

    return SetValueFloat32( (Float32) value );
}

/*-------------------------------------------------------------------------*/

bool
CSpinner::SetValueInt32( const Int32 value )
{GUCEF_TRACE;

    return SetValueFloat32( (Float32) value );
}

/*-------------------------------------------------------------------------*/

bool
CSpinner::SetValueFloat32( const Float32 value )
{GUCEF_TRACE;

    return SetFloatProperty( EWidgetProperty_Value, Clamp( value ) );
}

/*-------------------------------------------------------------------------*/

UInt32
CSpinner::GetValueUInt32( void ) const
{GUCEF_TRACE;

    Float32 value = GetValueFloat32();
    return value <= 0.0f ? 0 : (UInt32) ( value + 0.5f );
}

/*-------------------------------------------------------------------------*/

Int32
CSpinner::GetValueInt32( void ) const
{GUCEF_TRACE;

    Float32 value = GetValueFloat32();
    return (Int32) ( value < 0.0f ? value - 0.5f : value + 0.5f );
}

/*-------------------------------------------------------------------------*/

Float32
CSpinner::GetValueFloat32( void ) const
{GUCEF_TRACE;

    return GetFloatProperty( EWidgetProperty_Value );
}

/*-------------------------------------------------------------------------*/

bool
CSpinner::StepValue( Int32 steps )
{GUCEF_TRACE;

    Float32 value = Clamp( GetValueFloat32() + (Float32) steps * GetFloatProperty( EWidgetProperty_Step ) );
    return ChangeValueByUser( EWidgetProperty_Value, CORE::FloatToString( value ) );
}

/*-------------------------------------------------------------------------*/

CString
CSpinner::GetValueText( void ) const
{GUCEF_TRACE;

    Int32 decimals = GetIntProperty( EWidgetProperty_Decimals );
    if ( decimals <= 0 )
        return CORE::Int32ToString( GetValueInt32() );
    char buffer[ 64 ];
    snprintf( buffer, sizeof( buffer ), "%.*f", (int) decimals, (double) GetValueFloat32() );
    return buffer;
}

/*-------------------------------------------------------------------------*/

bool
CSpinner::GetNaturalSize( Int32& width, Int32& height ) const
{GUCEF_TRACE;

    width = height = 0;
    if ( GUCEF_NULL == GetContext() )
        return false;
    GetContext()->MeasureText( GetProperty( EWidgetProperty_Font ), CORE::FloatToString( GetFloatProperty( EWidgetProperty_Max ) ), 0, width, height );
    width += 32;
    height += 4;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSpinner::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseClickEvent == eventId )
    {
        CPointerEventData* pointer = static_cast< CPointerEventData* >( &eventData );
        if ( MOUSEBUTTON_LEFT != pointer->button && MOUSEBUTTON_RIGHT != pointer->button )
            return false;
        Int32 part = GetRect().width / 4;
        Int32 steps = MOUSEBUTTON_RIGHT == pointer->button ? 10 : 1;
        if ( pointer->localX < part )
            return StepValue( -steps );
        if ( pointer->localX >= GetRect().width - part )
            return StepValue( steps );
        return false;
    }
    if ( MouseWheelEvent == eventId )
        return StepValue( static_cast< CWheelEventData* >( &eventData )->delta );
    if ( NavigationEvent == eventId )
    {
        TNavigation direction = static_cast< CNavigationEventData* >( &eventData )->direction;
        if ( NAVIGATION_UP == direction || NAVIGATION_RIGHT == direction )
            return StepValue( 1 );
        if ( NAVIGATION_DOWN == direction || NAVIGATION_LEFT == direction )
            return StepValue( -1 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CSpinner::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CSpinner";
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
