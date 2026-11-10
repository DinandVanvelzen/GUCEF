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

#include "gucefGUI_CRoutedEventData.h"

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

CRoutedEventData::CRoutedEventData( void )
    : MT::CICloneable()
    , source( GUCEF_NULL )
    , phase( EVENTPHASE_TARGET )
    , handled( false )
{
}

CRoutedEventData::~CRoutedEventData()
{
}

MT::CICloneable*
CRoutedEventData::Clone( void ) const
{
    return GUCEF_NEW CRoutedEventData( *this );
}

const char*
CRoutedEventData::GetCloneableTypeName( void ) const
{
    return CloneableTypeName();
}

const char*
CRoutedEventData::CloneableTypeName( void )
{
    return "GUCEF::GUI::CRoutedEventData";
}

CRoutedEventData*
CRoutedEventData::FromCloneable( MT::CICloneable* data )
{
    return const_cast< CRoutedEventData* >( FromCloneable( const_cast< const MT::CICloneable* >( data ) ) );
}

const CRoutedEventData*
CRoutedEventData::FromCloneable( const MT::CICloneable* data )
{
    if ( GUCEF_NULL == data )
        return GUCEF_NULL;
    if ( data->IsCloneableType( CRoutedEventData::CloneableTypeName() ) )
        return static_cast< const CRoutedEventData* >( data );
    if ( data->IsCloneableType( CPointerEventData::CloneableTypeName() ) )
        return static_cast< const CPointerEventData* >( data );
    if ( data->IsCloneableType( CWheelEventData::CloneableTypeName() ) )
        return static_cast< const CWheelEventData* >( data );
    if ( data->IsCloneableType( CKeyEventData::CloneableTypeName() ) )
        return static_cast< const CKeyEventData* >( data );
    if ( data->IsCloneableType( CTextEventData::CloneableTypeName() ) )
        return static_cast< const CTextEventData* >( data );
    if ( data->IsCloneableType( CNavigationEventData::CloneableTypeName() ) )
        return static_cast< const CNavigationEventData* >( data );
    if ( data->IsCloneableType( CFocusEventData::CloneableTypeName() ) )
        return static_cast< const CFocusEventData* >( data );
    if ( data->IsCloneableType( CValueChangedEventData::CloneableTypeName() ) )
        return static_cast< const CValueChangedEventData* >( data );
    if ( data->IsCloneableType( CItemEventData::CloneableTypeName() ) )
        return static_cast< const CItemEventData* >( data );
    return GUCEF_NULL;
}

void
CRoutedEventData::SetLocalOrigin( Int32 widgetLeft, Int32 widgetTop )
{
}

/*-------------------------------------------------------------------------*/

CPointerEventData::CPointerEventData( void )
    : CRoutedEventData()
    , x( 0 )
    , y( 0 )
    , localX( 0 )
    , localY( 0 )
    , button( MOUSEBUTTON_NONE )
    , buttonStates( 0 )
    , modifiers( 0 )
    , clickCount( 0 )
{
}

CPointerEventData::~CPointerEventData()
{
}

MT::CICloneable*
CPointerEventData::Clone( void ) const
{
    return GUCEF_NEW CPointerEventData( *this );
}

const char*
CPointerEventData::GetCloneableTypeName( void ) const
{
    return CloneableTypeName();
}

const char*
CPointerEventData::CloneableTypeName( void )
{
    return "GUCEF::GUI::CPointerEventData";
}

void
CPointerEventData::SetLocalOrigin( Int32 widgetLeft, Int32 widgetTop )
{
    localX = x - widgetLeft;
    localY = y - widgetTop;
}

/*-------------------------------------------------------------------------*/

CWheelEventData::CWheelEventData( void )
    : CRoutedEventData()
    , x( 0 )
    , y( 0 )
    , localX( 0 )
    , localY( 0 )
    , delta( 0 )
    , modifiers( 0 )
{
}

CWheelEventData::~CWheelEventData()
{
}

MT::CICloneable*
CWheelEventData::Clone( void ) const
{
    return GUCEF_NEW CWheelEventData( *this );
}

const char*
CWheelEventData::GetCloneableTypeName( void ) const
{
    return CloneableTypeName();
}

const char*
CWheelEventData::CloneableTypeName( void )
{
    return "GUCEF::GUI::CWheelEventData";
}

void
CWheelEventData::SetLocalOrigin( Int32 widgetLeft, Int32 widgetTop )
{
    localX = x - widgetLeft;
    localY = y - widgetTop;
}

/*-------------------------------------------------------------------------*/

CKeyEventData::CKeyEventData( void )
    : CRoutedEventData()
    , keyCode( INPUT::KEYCODE_UNASSIGNED )
    , modifiers( 0 )
{
}

CKeyEventData::~CKeyEventData()
{
}

MT::CICloneable*
CKeyEventData::Clone( void ) const
{
    return GUCEF_NEW CKeyEventData( *this );
}

const char*
CKeyEventData::GetCloneableTypeName( void ) const
{
    return CloneableTypeName();
}

const char*
CKeyEventData::CloneableTypeName( void )
{
    return "GUCEF::GUI::CKeyEventData";
}

/*-------------------------------------------------------------------------*/

CTextEventData::CTextEventData( void )
    : CRoutedEventData()
    , text()
{
}

CTextEventData::~CTextEventData()
{
}

MT::CICloneable*
CTextEventData::Clone( void ) const
{
    return GUCEF_NEW CTextEventData( *this );
}

const char*
CTextEventData::GetCloneableTypeName( void ) const
{
    return CloneableTypeName();
}

const char*
CTextEventData::CloneableTypeName( void )
{
    return "GUCEF::GUI::CTextEventData";
}

/*-------------------------------------------------------------------------*/

CNavigationEventData::CNavigationEventData( void )
    : CRoutedEventData()
    , direction( NAVIGATION_NEXT )
{
}

CNavigationEventData::~CNavigationEventData()
{
}

MT::CICloneable*
CNavigationEventData::Clone( void ) const
{
    return GUCEF_NEW CNavigationEventData( *this );
}

const char*
CNavigationEventData::GetCloneableTypeName( void ) const
{
    return CloneableTypeName();
}

const char*
CNavigationEventData::CloneableTypeName( void )
{
    return "GUCEF::GUI::CNavigationEventData";
}

/*-------------------------------------------------------------------------*/

CFocusEventData::CFocusEventData( void )
    : CRoutedEventData()
    , other( GUCEF_NULL )
{
}

CFocusEventData::~CFocusEventData()
{
}

MT::CICloneable*
CFocusEventData::Clone( void ) const
{
    return GUCEF_NEW CFocusEventData( *this );
}

const char*
CFocusEventData::GetCloneableTypeName( void ) const
{
    return CloneableTypeName();
}

const char*
CFocusEventData::CloneableTypeName( void )
{
    return "GUCEF::GUI::CFocusEventData";
}

/*-------------------------------------------------------------------------*/

CValueChangedEventData::CValueChangedEventData( void )
    : CRoutedEventData()
    , property( EWidgetProperty_Unknown )
    , customPropertyName()
    , oldValue()
    , newValue()
{
}

CValueChangedEventData::~CValueChangedEventData()
{
}

const CString&
CValueChangedEventData::GetPropertyName( void ) const
{
    return EWidgetProperty_Unknown == property ? customPropertyName : WidgetPropertyToString( property );
}

void
CValueChangedEventData::SetPropertyName( const CString& name )
{
    property = StringToWidgetProperty( name );
    customPropertyName = EWidgetProperty_Unknown == property ? name : CString::Empty;
}

MT::CICloneable*
CValueChangedEventData::Clone( void ) const
{
    return GUCEF_NEW CValueChangedEventData( *this );
}

const char*
CValueChangedEventData::GetCloneableTypeName( void ) const
{
    return CloneableTypeName();
}

const char*
CValueChangedEventData::CloneableTypeName( void )
{
    return "GUCEF::GUI::CValueChangedEventData";
}

/*-------------------------------------------------------------------------*/

CPropertyChangedEventData::CPropertyChangedEventData( void )
    : MT::CICloneable()
    , property( EWidgetProperty_Unknown )
    , customPropertyName()
{
}

CPropertyChangedEventData::CPropertyChangedEventData( EWidgetProperty changedProperty )
    : MT::CICloneable()
    , property( changedProperty )
    , customPropertyName()
{
}

CPropertyChangedEventData::CPropertyChangedEventData( const CString& changedPropertyName )
    : MT::CICloneable()
    , property( StringToWidgetProperty( changedPropertyName ) )
    , customPropertyName( EWidgetProperty_Unknown == property ? changedPropertyName : CString::Empty )
{
}

CPropertyChangedEventData::~CPropertyChangedEventData()
{
}

const CString&
CPropertyChangedEventData::GetPropertyName( void ) const
{
    return EWidgetProperty_Unknown == property ? customPropertyName : WidgetPropertyToString( property );
}

MT::CICloneable*
CPropertyChangedEventData::Clone( void ) const
{
    return GUCEF_NEW CPropertyChangedEventData( *this );
}

const char*
CPropertyChangedEventData::GetCloneableTypeName( void ) const
{
    return CloneableTypeName();
}

const char*
CPropertyChangedEventData::CloneableTypeName( void )
{
    return "GUCEF::GUI::CPropertyChangedEventData";
}

/*-------------------------------------------------------------------------*/

CItemEventData::CItemEventData( void )
    : CRoutedEventData()
    , index( -1 )
    , item()
{
}

CItemEventData::~CItemEventData()
{
}

MT::CICloneable*
CItemEventData::Clone( void ) const
{
    return GUCEF_NEW CItemEventData( *this );
}

const char*
CItemEventData::GetCloneableTypeName( void ) const
{
    return CloneableTypeName();
}

const char*
CItemEventData::CloneableTypeName( void )
{
    return "GUCEF::GUI::CItemEventData";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
