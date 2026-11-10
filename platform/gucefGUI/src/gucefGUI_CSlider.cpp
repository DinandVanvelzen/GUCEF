/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefGUI_CSlider.h"

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

CSlider::CSlider( void )
    : CWidget( "Slider" )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CSlider::CSlider( const CString& widgetTypeName )
    : CWidget( widgetTypeName )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CSlider::~CSlider()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CSlider::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CSlider();
}

/*-------------------------------------------------------------------------*/

CWidget*
CSlider::CreateScrollbar( void )
{GUCEF_TRACE;

    return GUCEF_NEW CSlider( "Scrollbar" );
}

/*-------------------------------------------------------------------------*/

Float32
CSlider::GetValue( void ) const
{GUCEF_TRACE;

    return GetFloatProperty( EWidgetProperty_Value );
}

/*-------------------------------------------------------------------------*/

bool
CSlider::SetValue( Float32 value )
{GUCEF_TRACE;

    return SetFloatProperty( EWidgetProperty_Value, Snap( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CSlider::IsVertical( void ) const
{GUCEF_TRACE;

    return GetProperty( EWidgetProperty_Orientation ) == "Vertical";
}

/*-------------------------------------------------------------------------*/

Float32
CSlider::Snap( Float32 value ) const
{GUCEF_TRACE;

    Float32 minimum = GetFloatProperty( EWidgetProperty_Min );
    Float32 maximum = GetFloatProperty( EWidgetProperty_Max );
    Float32 step = GetFloatProperty( EWidgetProperty_Step );
    if ( step > 0.0f )
    {
        Float32 steps = ( value - minimum ) / step;
        value = minimum + (Float32) (Int64) ( steps + ( steps < 0.0f ? -0.5f : 0.5f ) ) * step;
    }
    if ( value < minimum ) value = minimum;
    if ( value > maximum ) value = maximum;
    return value;
}

/*-------------------------------------------------------------------------*/

Int32
CSlider::GetThumbLength( void ) const
{GUCEF_TRACE;

    Int32 length = IsVertical() ? GetRect().height : GetRect().width;
    if ( GetType() == "Scrollbar" )
    {
        Float32 range = GetFloatProperty( EWidgetProperty_Max ) - GetFloatProperty( EWidgetProperty_Min );
        Float32 page = GetFloatProperty( EWidgetProperty_PageSize );
        if ( range + page > 0.0f )
        {
            Int32 thumb = (Int32) ( (Float32) length * page / ( range + page ) );
            return thumb < 8 ? 8 : thumb;
        }
    }
    Int32 thumb = GetIntProperty( EWidgetProperty_ThumbSize );
    return thumb > length ? length : thumb;
}

/*-------------------------------------------------------------------------*/

CRect
CSlider::GetThumbRect( void ) const
{GUCEF_TRACE;

    Float32 minimum = GetFloatProperty( EWidgetProperty_Min );
    Float32 range = GetFloatProperty( EWidgetProperty_Max ) - minimum;
    Float32 fraction = range > 0.0f ? ( GetValue() - minimum ) / range : 0.0f;
    Int32 thumb = GetThumbLength();
    if ( IsVertical() )
    {
        Int32 travel = GetRect().height - thumb;
        return CRect( 0, (Int32) ( fraction * (Float32) travel + 0.5f ), GetRect().width, thumb );
    }
    Int32 travel = GetRect().width - thumb;
    return CRect( (Int32) ( fraction * (Float32) travel + 0.5f ), 0, thumb, GetRect().height );
}

/*-------------------------------------------------------------------------*/

Float32
CSlider::ValueAt( Int32 localX, Int32 localY ) const
{GUCEF_TRACE;

    Int32 thumb = GetThumbLength();
    Int32 travel = ( IsVertical() ? GetRect().height : GetRect().width ) - thumb;
    Int32 position = ( IsVertical() ? localY : localX ) - thumb / 2;
    Float32 fraction = travel > 0 ? (Float32) position / (Float32) travel : 0.0f;
    if ( fraction < 0.0f ) fraction = 0.0f;
    if ( fraction > 1.0f ) fraction = 1.0f;
    Float32 minimum = GetFloatProperty( EWidgetProperty_Min );
    return Snap( minimum + fraction * ( GetFloatProperty( EWidgetProperty_Max ) - minimum ) );
}

/*-------------------------------------------------------------------------*/

bool
CSlider::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseDownEvent == eventId || ( MouseMoveEvent == eventId && IsPressed() ) )
    {
        CPointerEventData* pointer = static_cast< CPointerEventData* >( &eventData );
        if ( MouseDownEvent == eventId && MOUSEBUTTON_LEFT != pointer->button )
            return false;
        return ChangeValueByUser( EWidgetProperty_Value, CORE::FloatToString( ValueAt( pointer->localX, pointer->localY ) ) );
    }
    Float32 step = GetFloatProperty( EWidgetProperty_Step );
    if ( step <= 0.0f )
        step = 1.0f;
    if ( MouseWheelEvent == eventId )
    {
        Int32 delta = static_cast< CWheelEventData* >( &eventData )->delta;
        // a vertical scroll bar goes down when the wheel turns towards the user
        if ( IsVertical() )
            delta = -delta;
        return ChangeValueByUser( EWidgetProperty_Value, CORE::FloatToString( Snap( GetValue() + (Float32) delta * step ) ) );
    }
    if ( NavigationEvent == eventId )
    {
        TNavigation direction = static_cast< CNavigationEventData* >( &eventData )->direction;
        Float32 change = 0.0f;
        if ( NAVIGATION_RIGHT == direction || NAVIGATION_DOWN == direction ) change = step;
        if ( NAVIGATION_LEFT == direction || NAVIGATION_UP == direction ) change = -step;
        if ( 0.0f == change )
            return false;
        return ChangeValueByUser( EWidgetProperty_Value, CORE::FloatToString( Snap( GetValue() + change ) ) );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CSlider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CSlider";
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
