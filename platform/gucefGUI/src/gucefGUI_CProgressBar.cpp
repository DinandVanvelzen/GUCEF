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

#include "gucefGUI_CProgressBar.h"

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

CProgressBar::CProgressBar( void )
    : CWidget( "ProgressBar" )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CProgressBar::~CProgressBar()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CProgressBar::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CProgressBar();
}

/*-------------------------------------------------------------------------*/

Float32
CProgressBar::GetValue( void ) const
{GUCEF_TRACE;

    return GetFloatProperty( EWidgetProperty_Value );
}

/*-------------------------------------------------------------------------*/

bool
CProgressBar::SetValue( Float32 value )
{GUCEF_TRACE;

    return SetFloatProperty( EWidgetProperty_Value, value );
}

/*-------------------------------------------------------------------------*/

Float32
CProgressBar::GetFraction( void ) const
{GUCEF_TRACE;

    Float32 minimum = GetFloatProperty( EWidgetProperty_Min );
    Float32 range = GetFloatProperty( EWidgetProperty_Max ) - minimum;
    if ( range <= 0.0f )
        return 0.0f;
    Float32 fraction = ( GetValue() - minimum ) / range;
    if ( fraction < 0.0f ) fraction = 0.0f;
    if ( fraction > 1.0f ) fraction = 1.0f;
    return fraction;
}

/*-------------------------------------------------------------------------*/

const CString&
CProgressBar::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CProgressBar";
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
