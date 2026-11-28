/*
 *  tablecodecpluginXLSX: GUCEF table codec plugin for Microsoft Excel (.xlsx) workbooks
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

#include "tablecodecpluginXLSX_CXlsxFileMetaData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace XLSX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SXlsxLayout::SXlsxLayout( void )
    : sheetIndex( 0 )
    , sheetName()
    , headerRow( 0 )
    , dataStartRow( 1 )
    , firstColumn( 0 )
    , width( 0 )
    , date1904( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CXlsxFileMetaData::CXlsxFileMetaData( const CORE::CDataTableSchemaPtr& schema, const CORE::CValueList& keyValues, const SXlsxLayout& layout, UInt64 rowCount )
    : CORE::CITableFileMetaData()
    , m_schema( schema )
    , m_keyValues( keyValues )
    , m_layout( layout )
    , m_rowCount( rowCount )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CXlsxFileMetaData::~CXlsxFileMetaData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CDataTableSchemaPtr&
CXlsxFileMetaData::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

UInt64
CXlsxFileMetaData::GetRowCount( void ) const
{GUCEF_TRACE;

    return m_rowCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CXlsxFileMetaData::GetRowGroupCount( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CXlsxFileMetaData::GetRowGroupInfo( UInt32 rowGroup, CORE::CDataTableRowGroupInfo& info ) const
{GUCEF_TRACE;

    if ( 0 != rowGroup )
        return false;

    info.Clear();
    info.rowCount = m_rowCount;
    if ( !m_schema.IsNULL() )
        info.columnStats.resize( m_schema->GetColumnCount() );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CXlsxFileMetaData::GetMemoryFootprint( void ) const
{GUCEF_TRACE;

    UInt64 bytes = sizeof( *this );
    if ( !m_schema.IsNULL() )
        bytes += (UInt64) m_schema->GetColumnCount() * 128;
    bytes += (UInt64) m_keyValues.GetKeyCount() * 64;
    return bytes;
}

/*-------------------------------------------------------------------------*/

void
CXlsxFileMetaData::GetKeyValueMetaData( CORE::CValueList& keyValues ) const
{GUCEF_TRACE;

    keyValues = m_keyValues;
}

/*-------------------------------------------------------------------------*/

const SXlsxLayout&
CXlsxFileMetaData::GetLayout( void ) const
{GUCEF_TRACE;

    return m_layout;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XLSX */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
