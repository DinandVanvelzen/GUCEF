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

#include "gucefGUI_CWidgetPropertyValue.h"

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

CWidgetPropertyValue::CWidgetPropertyValue( void )
    : m_text()
    , m_read( 0 )
    , m_bool( false )
    , m_int( 0 )
    , m_float( 0.0f )
    , m_colour( 0, 0, 0, 0 )
    , m_colourValid( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWidgetPropertyValue::CWidgetPropertyValue( const CString& text )
    : m_text( text )
    , m_read( 0 )
    , m_bool( false )
    , m_int( 0 )
    , m_float( 0.0f )
    , m_colour( 0, 0, 0, 0 )
    , m_colourValid( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CWidgetPropertyValue::GetText( void ) const
{
    return m_text;
}

/*-------------------------------------------------------------------------*/

void
CWidgetPropertyValue::SetText( const CString& text )
{GUCEF_TRACE;

    m_text = text;
    m_read = 0;
}

/*-------------------------------------------------------------------------*/

bool
CWidgetPropertyValue::AsBool( void ) const
{
    if ( 0 == ( m_read & READ_BOOL ) )
    {
        m_bool = CORE::StringToBool( m_text, false );
        m_read |= READ_BOOL;
    }
    return m_bool;
}

/*-------------------------------------------------------------------------*/

Int32
CWidgetPropertyValue::AsInt( void ) const
{
    if ( 0 == ( m_read & READ_INT ) )
    {
        m_int = CORE::StringToInt32( m_text, 0 );
        m_read |= READ_INT;
    }
    return m_int;
}

/*-------------------------------------------------------------------------*/

Float32
CWidgetPropertyValue::AsFloat( void ) const
{
    if ( 0 == ( m_read & READ_FLOAT ) )
    {
        m_float = CORE::StringToFloat( m_text, 0.0f );
        m_read |= READ_FLOAT;
    }
    return m_float;
}

/*-------------------------------------------------------------------------*/

const CColour&
CWidgetPropertyValue::AsColour( void ) const
{
    if ( 0 == ( m_read & READ_COLOUR ) )
    {
        m_colour = CColour( 0, 0, 0, 0 );
        m_colourValid = CColour::FromString( m_text, m_colour );
        m_read |= READ_COLOUR;
    }
    return m_colour;
}

/*-------------------------------------------------------------------------*/

void
CWidgetPropertyValue::ReadAll( void ) const
{GUCEF_TRACE;

    AsBool();
    AsInt();
    AsFloat();
    AsColour();
}

/*-------------------------------------------------------------------------*/

bool
CWidgetPropertyValue::TryAsColour( CColour& colour ) const
{
    AsColour();
    if ( m_colourValid )
        colour = m_colour;
    return m_colourValid;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
