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

#ifndef GUCEF_GUI_CTABLECELLFORMATTER_H
#include "gucefGUI_CTableCellFormatter.h"
#define GUCEF_GUI_CTABLECELLFORMATTER_H
#endif /* GUCEF_GUI_CTABLECELLFORMATTER_H ? */

#include "gucefGUI_CITableViewModel.h"

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

const CORE::CEvent CITableViewModel::CellsChangedEvent = "GUCEF::GUI::CITableViewModel::CellsChangedEvent";
const CORE::CEvent CITableViewModel::RowsInsertedEvent = "GUCEF::GUI::CITableViewModel::RowsInsertedEvent";
const CORE::CEvent CITableViewModel::RowsRemovedEvent = "GUCEF::GUI::CITableViewModel::RowsRemovedEvent";
const CORE::CEvent CITableViewModel::ColumnsChangedEvent = "GUCEF::GUI::CITableViewModel::ColumnsChangedEvent";
const CORE::CEvent CITableViewModel::FooterChangedEvent = "GUCEF::GUI::CITableViewModel::FooterChangedEvent";
const CORE::CEvent CITableViewModel::ModelResetEvent = "GUCEF::GUI::CITableViewModel::ModelResetEvent";
const CORE::CEvent CITableViewModel::BatchStartedEvent = "GUCEF::GUI::CITableViewModel::BatchStartedEvent";
const CORE::CEvent CITableViewModel::BatchFinishedEvent = "GUCEF::GUI::CITableViewModel::BatchFinishedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CITableViewModel::RegisterEvents( void )
{GUCEF_TRACE;

    CellsChangedEvent.Initialize();
    RowsInsertedEvent.Initialize();
    RowsRemovedEvent.Initialize();
    ColumnsChangedEvent.Initialize();
    FooterChangedEvent.Initialize();
    ModelResetEvent.Initialize();
    BatchStartedEvent.Initialize();
    BatchFinishedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CITableViewModel::CITableViewModel( void )
    : CORE::CObservingNotifier()
    , m_batchDepth( 0 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CITableViewModel::~CITableViewModel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::GetCellContent( UInt32 row, UInt32 column, SCellContent& content ) const
{GUCEF_TRACE;

    CORE::CVariant value;
    if ( !GetCellValue( row, column, value ) )
    {
        content.kind = SCellContent::KIND_TEXT;
        content.text.Clear();
        content.isNumber = false;
        return false;
    }
    CTableCellFormatter::Format( value, GetColumn( column ), content );
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CITableViewModel::GetCellText( UInt32 row, UInt32 column ) const
{GUCEF_TRACE;

    SCellContent content;
    if ( !GetCellContent( row, column, content ) )
        return CString::Empty;
    return content.text;
}

/*-------------------------------------------------------------------------*/

void
CITableViewModel::PrepareRange( const CTableViewRange& visible )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::GetColumnSpan( UInt32 column, UInt32 firstRow, UInt32 rowCount, CORE::CVariant::VariantVector& values ) const
{GUCEF_TRACE;

    values.clear();
    if ( column >= GetColumnCount() )
        return false;
    const UInt32 total = GetRowCount();
    if ( firstRow >= total )
        return true;
    const UInt32 count = rowCount < total - firstRow ? rowCount : total - firstRow;
    values.reserve( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CVariant value;
        GetCellValue( firstRow + i, column, value );
        values.push_back( value );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::GetRowKey( UInt32 row, CORE::CVariant& key ) const
{GUCEF_TRACE;

    if ( row >= GetRowCount() )
        return false;
    key = row;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::FindRowByKey( const CORE::CVariant& key, UInt32& row ) const
{GUCEF_TRACE;

    if ( !key.IsNumber() )
        return false;
    const UInt32 candidate = key.AsUInt32();
    if ( candidate >= GetRowCount() )
        return false;
    row = candidate;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::GetRowTitle( UInt32 row, CString& title ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::IsRowEmphasized( UInt32 row ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::HasFooter( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::GetFooterValue( UInt32 column, CORE::CVariant& value ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::GetFooterContent( UInt32 column, SCellContent& content ) const
{GUCEF_TRACE;

    CORE::CVariant value;
    if ( !HasFooter() || !GetFooterValue( column, value ) )
    {
        content.kind = SCellContent::KIND_TEXT;
        content.text.Clear();
        content.isNumber = false;
        return false;
    }
    CTableCellFormatter::Format( value, GetColumn( column ), content );
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CITableViewModel::GetFooterText( UInt32 column ) const
{GUCEF_TRACE;

    SCellContent content;
    if ( !GetFooterContent( column, content ) )
        return CString::Empty;
    return content.text;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::IsSortSupported( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::SortByColumn( UInt32 column, bool ascending )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::IsCellReadOnly( UInt32 row, UInt32 column ) const
{GUCEF_TRACE;

    return GetColumn( column ).readOnly;
}

/*-------------------------------------------------------------------------*/

CTableEditResult
CITableViewModel::ValidateCellEdit( UInt32 row, UInt32 column, const CORE::CVariant& value ) const
{GUCEF_TRACE;

    if ( IsCellReadOnly( row, column ) )
        return CTableEditResult::ReadOnly();
    return CTableEditResult::Accepted();
}

/*-------------------------------------------------------------------------*/

CTableEditResult
CITableViewModel::SetCellValue( UInt32 row, UInt32 column, const CORE::CVariant& value )
{GUCEF_TRACE;

    return CTableEditResult::ReadOnly();
}

/*-------------------------------------------------------------------------*/

void
CITableViewModel::BeginEditGroup( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CITableViewModel::EndEditGroup( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::CanUndo( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::CanRedo( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::Undo( void )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::Redo( void )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CITableViewModel::IsInBatch( void ) const
{GUCEF_TRACE;

    return m_batchDepth > 0;
}

/*-------------------------------------------------------------------------*/

void
CITableViewModel::BeginBatch( void )
{GUCEF_TRACE;

    if ( 1 == ++m_batchDepth )
        NotifyObservers( BatchStartedEvent );
}

/*-------------------------------------------------------------------------*/

void
CITableViewModel::EndBatch( void )
{GUCEF_TRACE;

    if ( 0 == m_batchDepth )
        return;
    if ( 0 == --m_batchDepth )
        NotifyObservers( BatchFinishedEvent );
}

/*-------------------------------------------------------------------------*/

const CString&
CITableViewModel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CITableViewModel";
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
