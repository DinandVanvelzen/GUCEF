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

#include "gucefGUI_CTableViewColumn.h"

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

CTableViewColumn::CTableViewColumn( void )
    : name()
    , title()
    , type( CORE::DATATABLE_TYPE_UTF8 )
    , width( 0 )
    , readOnly( true )
    , alignment()
    , decimals( -1 )
    , thousandsSeparator( false )
    , prefix()
    , suffix()
    , dateFormat()
    , booleanDisplay( BOOLEAN_CHECK )
    , nullText()
    , choices()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTableViewColumn::CTableViewColumn( const CString& columnTitle, CORE::TDataTableLogicalType columnType, Int32 columnWidth, bool isReadOnly )
    : name( columnTitle )
    , title( columnTitle )
    , type( columnType )
    , width( columnWidth )
    , readOnly( isReadOnly )
    , alignment()
    , decimals( -1 )
    , thousandsSeparator( false )
    , prefix()
    , suffix()
    , dateFormat()
    , booleanDisplay( BOOLEAN_CHECK )
    , nullText()
    , choices()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CTableViewColumn::GetDisplayTitle( void ) const
{GUCEF_TRACE;

    return title.IsNULLOrEmpty() ? name : title;
}

/*-------------------------------------------------------------------------*/

bool
CTableViewColumn::IsNumeric( void ) const
{GUCEF_TRACE;

    return CORE::DataTableLogicalTypeIsInteger( type ) || CORE::DATATABLE_TYPE_FLOAT32 == type || CORE::DATATABLE_TYPE_FLOAT64 == type || CORE::DATATABLE_TYPE_DECIMAL == type;
}

/*-------------------------------------------------------------------------*/

bool
CTableViewColumn::IsTemporal( void ) const
{GUCEF_TRACE;

    return CORE::DATATABLE_TYPE_DATE == type || CORE::DATATABLE_TYPE_TIMESTAMP == type;
}

/*-------------------------------------------------------------------------*/

CString
CTableViewColumn::GetEffectiveAlignment( void ) const
{GUCEF_TRACE;

    if ( "Left" == alignment || "Center" == alignment || "Right" == alignment )
        return alignment;
    if ( IsNumeric() || IsTemporal() )
        return "Right";
    if ( CORE::DATATABLE_TYPE_BOOL == type && BOOLEAN_CHECK == booleanDisplay )
        return "Center";
    return "Left";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
