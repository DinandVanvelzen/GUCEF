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

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXCOLUMNCONVERTER_H
#include "tablecodecpluginXLSX_CXlsxColumnConverter.h"
#define TABLECODECPLUGIN_XLSX_CXLSXCOLUMNCONVERTER_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXCOLUMNCONVERTER_H ? */

#include "tablecodecpluginXLSX_CXlsxColumnInference.h"

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

CXlsxColumnInference::CXlsxColumnInference( void )
    : m_sawBlank( false )
    , m_sawBool( false )
    , m_sawNumber( false )
    , m_sawString( false )
    , m_sawDate( false )
    , m_sawTimeOfDay( false )
    , m_allIntegral( true )
    , m_allExact( true )
    , m_maxIntDigits( 1 )
    , m_maxFracDigits( 0 )
    , m_lossyValues( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CXlsxColumnInference::FeedBlank( void )
{GUCEF_TRACE;

    m_sawBlank = true;
}

/*-------------------------------------------------------------------------*/

void
CXlsxColumnInference::Feed( const SXlsxCell& cell, const SXlsxReadOptions& options )
{GUCEF_TRACE;

    switch ( cell.kind )
    {
        case XLSX_CELL_ERROR:
        case XLSX_CELL_UNCACHED_FORMULA:
        {
            // the read refuses these (or makes them null): they tell nothing of the type
            m_sawBlank = true;
            return;
        }
        case XLSX_CELL_STRING:
        {
            if ( options.IsNullToken( cell.text, cell.length ) )
                m_sawBlank = true;
            else
                m_sawString = true;
            return;
        }
        case XLSX_CELL_BOOL:
        {
            m_sawBool = true;
            return;
        }
        case XLSX_CELL_ISODATE:
        {
            Int32 days = 0;
            Int64 micros = 0;
            bool hasTime = false;
            if ( CXlsxDates::ParseIso( cell.text, cell.length, days, micros, hasTime ) )
            {
                m_sawDate = true;
                if ( 0 != micros )
                    m_sawTimeOfDay = true;
            }
            else
                m_sawString = true;
            return;
        }
        case XLSX_CELL_NUMBER:
            break;
    }

    if ( XLSX_NUMBERKIND_DATE == cell.numberKind || XLSX_NUMBERKIND_DATETIME == cell.numberKind )
    {
        SXlsxNumberParts parts;
        if ( CXlsxColumnConverter::SplitNumber( cell.text, cell.length, parts ) && !parts.negative )
        {
            m_sawDate = true;
            if ( XLSX_NUMBERKIND_DATETIME == cell.numberKind || ( !parts.fractionIsZero && CXlsxDates::FractionToMicros( parts.fraction, parts.fractionCount ) > 0 ) )
                m_sawTimeOfDay = true;
            return;
        }
    }

    // a number: the digits of the text tell what holds it exactly
    m_sawNumber = true;
    UInt32 digitCount = 0;
    for ( UInt32 i = 0; i < cell.length; ++i )
        if ( cell.text[ i ] >= '0' && cell.text[ i ] <= '9' )
            ++digitCount;
    if ( digitCount > 15 )
        ++m_lossyValues;

    CORE::CDecimal value;
    if ( CORE::DecimalStatusIsError( CORE::CDecimal::FromChars( cell.text, cell.length, value, CORE::DECIMALPARSE_ALLOW_EXPONENT ) ) )
    {
        m_allExact = false;
        m_allIntegral = false;
        return;
    }
    const CORE::CDecimal normalized = value.Normalized();
    const UInt32 scale = normalized.GetScale();
    const UInt32 digits = normalized.GetDigitCount();
    const UInt32 intDigits = digits > scale ? digits - scale : 1;
    if ( scale > 0 )
        m_allIntegral = false;
    if ( intDigits > m_maxIntDigits )
        m_maxIntDigits = intDigits;
    if ( scale > m_maxFracDigits )
        m_maxFracDigits = scale;
}

/*-------------------------------------------------------------------------*/

UInt64
CXlsxColumnInference::GetLossyFloatValueCount( void ) const
{GUCEF_TRACE;

    return m_lossyValues;
}

/*-------------------------------------------------------------------------*/

CORE::CDataTableColumnDef
CXlsxColumnInference::GetColumn( const CORE::CString& name, bool allNullable ) const
{GUCEF_TRACE;

    const bool nullable = m_sawBlank || allNullable;
    const UInt32 categories = ( m_sawBool ? 1 : 0 ) + ( m_sawNumber ? 1 : 0 ) + ( m_sawString ? 1 : 0 ) + ( m_sawDate ? 1 : 0 );

    if ( 1 != categories || m_sawString )
        return CORE::CDataTableColumnDef( name, CORE::DATATABLE_TYPE_UTF8, nullable );
    if ( m_sawBool )
        return CORE::CDataTableColumnDef( name, CORE::DATATABLE_TYPE_BOOL, nullable );
    if ( m_sawDate )
    {
        if ( m_sawTimeOfDay )
            return CORE::CDataTableColumnDef::Timestamp( name, CORE::DATATABLE_TIMEUNIT_MICROSECONDS, false, nullable );
        return CORE::CDataTableColumnDef( name, CORE::DATATABLE_TYPE_DATE, nullable );
    }

    if ( m_allExact )
    {
        if ( m_allIntegral )
            return CORE::CDataTableColumnDef( name, CORE::DATATABLE_TYPE_INT64, nullable );
        const UInt32 precision = m_maxIntDigits + m_maxFracDigits;
        if ( precision <= CORE::DataTableMaxDecimalPrecision )
            return CORE::CDataTableColumnDef::Decimal( name, (UInt8) precision, (UInt8) m_maxFracDigits, nullable );
    }
    return CORE::CDataTableColumnDef( name, CORE::DATATABLE_TYPE_FLOAT64, nullable );
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
