/*
 *  gucefDRNUTILSGUI: GUCEF module providing the forms that view and control job schedulers and the task manager
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
//      INCLUDES                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#include "gucefDRNUTILSGUI_CTimeDisplayTableModel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILSGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

CTimeDisplayTableModel::CTimeDisplayTableModel( const CORE::CIDataTablePtr& table       ,
                                                const SGridColumnSpec* specs            ,
                                                UInt32 specCount                        ,
                                                const CTimeDisplaySettings& timeDisplay )
    : GUI::CDataTableViewModel( table )
    , m_presented()
    , m_formatter( timeDisplay )
{GUCEF_TRACE;

    if ( !IsValid() )
        return;

    const CORE::CDataTableSchemaPtr schema = table->GetSchema();
    if ( schema.IsNULL() )
        return;

    // the columns of the form that the table has, then the hints of the table: a column the table hides is not shown, except the key (the first)
    CORE::CString::StringVector names;
    if ( GUCEF_NULL != specs && specCount > 0 )
    {
        for ( UInt32 i=0; i<specCount; ++i )
            if ( schema->FindColumn( specs[ i ].name ) >= 0 )
                names.push_back( specs[ i ].name );
    }
    else
    {
        for ( UInt32 i=0; i<schema->GetColumnCount(); ++i )
            names.push_back( schema->GetColumn( i ).name );
    }
    if ( names.empty() )
        return;

    CORE::CString::StringVector shown;
    for ( size_t i=0; i<names.size(); ++i )
    {
        CORE::CValueList hints;
        CORE::CVariant hidden;
        if ( 0 != i && table->GetColumnHints( (UInt32) schema->FindColumn( names[ i ] ), hints ) &&
             hints.TryGetValue( "hidden", hidden ) && hidden.AsBool( false, false ) )
            continue;
        shown.push_back( names[ i ] );
    }
    if ( !SetColumns( shown ) )
        return;

    for ( UInt32 i=0; i<GUI::CDataTableViewModel::GetColumnCount(); ++i )
    {
        GUI::CTableViewColumn column = GUI::CDataTableViewModel::GetColumn( i );
        if ( GUCEF_NULL != specs )
        {
            for ( UInt32 s=0; s<specCount; ++s )
            {
                if ( column.name == specs[ s ].name )
                {
                    if ( GUCEF_NULL != specs[ s ].title )
                        column.title = specs[ s ].title;
                    if ( specs[ s ].width > 0 )
                        column.width = specs[ s ].width;
                    break;
                }
            }
        }
        CORE::CValueList hints;
        if ( table->GetColumnHints( (UInt32) schema->FindColumn( column.name ), hints ) )
        {
            CORE::CVariant value;
            if ( hints.TryGetValue( "title", value ) && !value.AsString( CORE::CString::Empty, false ).IsNULLOrEmpty() )
                column.title = value.AsString( CORE::CString::Empty, false );
            if ( hints.TryGetValue( "width", value ) && value.AsInt32( 0, false ) > 0 )
                column.width = value.AsInt32( 0, false );
            if ( hints.TryGetValue( "alignment", value ) )
                column.alignment = value.AsString( CORE::CString::Empty, false );
            if ( hints.TryGetValue( "decimals", value ) )
                column.decimals = value.AsInt32( column.decimals, false );
            if ( hints.TryGetValue( "thousandsSeparator", value ) )
                column.thousandsSeparator = value.AsBool( column.thousandsSeparator, false );
            if ( hints.TryGetValue( "prefix", value ) )
                column.prefix = value.AsString( CORE::CString::Empty, false );
            if ( hints.TryGetValue( "suffix", value ) )
                column.suffix = value.AsString( CORE::CString::Empty, false );
            if ( hints.TryGetValue( "booleanDisplay", value ) )
                column.booleanDisplay = value.AsString( CORE::CString::Empty, false ).Lowercase() == "text" ? GUI::CTableViewColumn::BOOLEAN_TEXT : GUI::CTableViewColumn::BOOLEAN_CHECK;
            if ( hints.TryGetValue( "nullText", value ) )
                column.nullText = value.AsString( CORE::CString::Empty, false );
        }
        if ( column.title.IsNULLOrEmpty() )
            column.title = column.name;
        m_presented.push_back( column );
    }
}

/*-------------------------------------------------------------------------*/

CTimeDisplayTableModel::~CTimeDisplayTableModel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTimeDisplayTableModel::SetTimeDisplay( const CTimeDisplaySettings& timeDisplay )
{GUCEF_TRACE;

    m_formatter.SetSettings( timeDisplay );
    Invalidate();
}

/*-------------------------------------------------------------------------*/

const CTimeDisplayFormatter&
CTimeDisplayTableModel::GetFormatter( void ) const
{GUCEF_TRACE;

    return m_formatter;
}

/*-------------------------------------------------------------------------*/

UInt32
CTimeDisplayTableModel::FindShownColumn( const CORE::CString& columnName ) const
{GUCEF_TRACE;

    const UInt32 count = GetColumnCount();
    for ( UInt32 i=0; i<count; ++i )
        if ( GetColumn( i ).name == columnName )
            return i;
    return GUCEF_UINT32MAX;
}

/*-------------------------------------------------------------------------*/

const GUI::CTableViewColumn&
CTimeDisplayTableModel::GetColumn( UInt32 column ) const
{GUCEF_TRACE;

    if ( column < (UInt32) m_presented.size() )
        return m_presented[ column ];
    return GUI::CDataTableViewModel::GetColumn( column );
}

/*-------------------------------------------------------------------------*/

bool
CTimeDisplayTableModel::GetTimestampMs( UInt32 row, UInt32 column, Int64& utcMs ) const
{GUCEF_TRACE;

    CORE::CVariant value;
    if ( !GetCellValue( row, column, value ) )
        return false;
    CORE::TDataTableTimeUnit unit = CORE::DATATABLE_TIMEUNIT_MILLISECONDS;
    if ( !CORE::CDataTableValues::TimeUnitFromTypeId( value.GetTypeId(), unit ) )
        return false;
    return CORE::CDataTableValues::TicksFromVariant( value, CORE::DATATABLE_TIMEUNIT_MILLISECONDS, utcMs );
}

/*-------------------------------------------------------------------------*/

bool
CTimeDisplayTableModel::GetCellContent( UInt32 row, UInt32 column, GUI::SCellContent& content ) const
{GUCEF_TRACE;

    Int64 utcMs = 0;
    if ( GetTimestampMs( row, column, utcMs ) )
    {
        content.kind = GUI::SCellContent::KIND_TEXT;
        content.text = m_formatter.FormatUtcMs( utcMs );
        content.isNumber = false;
        return true;
    }
    return GUI::CDataTableViewModel::GetCellContent( row, column, content );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTimeDisplayTableModel::GetCellUtcText( UInt32 row, UInt32 column ) const
{GUCEF_TRACE;

    Int64 utcMs = 0;
    if ( GetTimestampMs( row, column, utcMs ) )
        return m_formatter.FormatUtcTooltip( utcMs );
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTimeDisplayTableModel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString typeName = "GUCEF::DRNUTILSGUI::CTimeDisplayTableModel";
    return typeName;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILSGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
