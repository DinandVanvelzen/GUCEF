/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCORE_CCsvSchemaResolver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CCsvColumnInference                                                //
//                                                                         //
//-------------------------------------------------------------------------*/

CCsvColumnInference::CCsvColumnInference( const SCsvReadOptions& options )
    : m_options( options )
    , m_sawValue( false )
    , m_sawNull( false )
    , m_sawQuotedNullToken( false )
    , m_possibleBool( true )
    , m_possibleInt( true )
    , m_possibleDecimal( true )
    , m_possibleFloat( true )
    , m_possibleTimestamp( true )
    , m_maxIntDigits( 0 )
    , m_maxFracDigits( 0 )
    , m_possibleDateFormats( 0 )
    , m_lossyFloatValues( 0 )
{GUCEF_TRACE;

    for ( size_t i = 0; i < options.dateFormats.size() && i < 32; ++i )
        m_possibleDateFormats |= ( 1u << i );
}

/*-------------------------------------------------------------------------*/

void
CCsvColumnInference::FeedMissing( void )
{GUCEF_TRACE;

    m_sawNull = true;
}

/*-------------------------------------------------------------------------*/

void
CCsvColumnInference::Feed( const char* data, UInt32 length, bool quoted )
{GUCEF_TRACE;

    // Null tokens are no information about the type. Quoted, they are text in a column that turns out to be text
    if ( m_options.HasOnlyEmptyNullToken() ? ( 0 == length ) : m_options.IsNullToken( data, length ) )
    {
        if ( quoted )
            m_sawQuotedNullToken = true;
        else
            m_sawNull = true;
        return;
    }

    m_sawValue = true;

    if ( m_possibleBool )
    {
        bool ignored = false;
        if ( !CCsvTableText::ParseBool( data, length, false, ignored ) )
            m_possibleBool = false;
    }

    if ( m_possibleInt || m_possibleDecimal || m_possibleFloat )
    {
        CCsvTableText::SNumberInfo number;
        if ( !CCsvTableText::ScanNumber( data, length, m_options.numberFormat, true, number ) )
        {
            m_possibleInt = m_possibleDecimal = m_possibleFloat = false;
        }
        else
        if ( number.special )
        {
            // nan and inf are numbers of a float column and of no other
            m_possibleInt = m_possibleDecimal = false;
        }
        else
        if ( number.leadingZeroGuard || number.explicitPlus )
        {
            // 007 and +1555 are identifiers, not numbers, when nobody said so
            m_possibleInt = m_possibleDecimal = m_possibleFloat = false;
        }
        else
        if ( number.hasExponent )
        {
            m_possibleInt = m_possibleDecimal = false;
            Float64 value = 0;
            if ( !CCsvTableText::NumberToFloat64( number, value ) )
                m_possibleFloat = false;
        }
        else
        {
            if ( number.hasPoint )
            {
                m_possibleInt = false;
            }
            else
            if ( m_possibleInt )
            {
                Int64 value = 0;
                if ( !CCsvTableText::NumberToInt64( number, value ) )
                    m_possibleInt = false;
            }

            if ( m_possibleDecimal )
            {
                if ( number.fracDigits > 18 || number.intDigits + number.fracDigits > 18 )
                {
                    m_possibleDecimal = false;
                }
                else
                {
                    if ( number.intDigits > m_maxIntDigits ) m_maxIntDigits = number.intDigits;
                    if ( number.fracDigits > m_maxFracDigits ) m_maxFracDigits = number.fracDigits;
                }
            }

            if ( m_possibleFloat )
            {
                Float64 value = 0;
                if ( !CCsvTableText::NumberToFloat64( number, value ) )
                    m_possibleFloat = false;
                else
                if ( number.intDigits + number.fracDigits > 15 )
                    ++m_lossyFloatValues;
            }
        }
    }

    for ( size_t i = 0; i < m_options.dateFormats.size() && i < 32; ++i )
    {
        if ( 0 != ( m_possibleDateFormats & ( 1u << i ) ) )
        {
            Int32 days = 0;
            if ( !CCsvTableText::ParseDate( m_options.dateFormats[ i ], data, length, days ) )
                m_possibleDateFormats &= ~( 1u << i );
        }
    }

    if ( m_possibleTimestamp )
    {
        Int64 ticks = 0;
        if ( !CCsvTableText::ParseZonedTimestamp( data, length, DATATABLE_TIMEUNIT_MICROSECONDS, ticks ) )
            m_possibleTimestamp = false;
    }
}

/*-------------------------------------------------------------------------*/

UInt64
CCsvColumnInference::GetLossyFloatValueCount( void ) const
{GUCEF_TRACE;

    return m_lossyFloatValues;
}

/*-------------------------------------------------------------------------*/

CDataTableColumnDef
CCsvColumnInference::GetColumn( const CString& name, bool allNullable, CString& dateFormat ) const
{GUCEF_TRACE;

    dateFormat.Clear();
    CDataTableColumnDef column( name, DATATABLE_TYPE_UTF8, true );

    bool isText = true;
    if ( m_sawValue )
    {
        if ( m_possibleBool )
        {
            column.type = DATATABLE_TYPE_BOOL;
            isText = false;
        }
        else
        if ( m_possibleInt )
        {
            column.type = DATATABLE_TYPE_INT64;
            isText = false;
        }
        else
        if ( m_possibleDecimal && m_maxFracDigits > 0 && m_maxIntDigits + m_maxFracDigits <= 18 )
        {
            const UInt32 precision = m_maxIntDigits + m_maxFracDigits;
            column = CDataTableColumnDef::Decimal( name, (UInt8) precision, (UInt8) m_maxFracDigits, true );
            isText = false;
        }
        else
        if ( m_possibleFloat )
        {
            column.type = DATATABLE_TYPE_FLOAT64;
            isText = false;
        }
        else
        if ( 0 != m_possibleDateFormats )
        {
            column.type = DATATABLE_TYPE_DATE;
            isText = false;
            for ( size_t i = 0; i < m_options.dateFormats.size() && i < 32; ++i )
            {
                if ( 0 != ( m_possibleDateFormats & ( 1u << i ) ) )
                {
                    dateFormat = m_options.dateFormats[ i ].text;
                    break;
                }
            }
        }
        else
        if ( m_possibleTimestamp )
        {
            column = CDataTableColumnDef::Timestamp( name, DATATABLE_TIMEUNIT_MICROSECONDS, true, true );
            isText = false;
        }
    }

    // Columns with nulls are nullable. When only part of the rows was looked at that says nothing about the rest
    bool nullable = m_sawNull || ( !isText && m_sawQuotedNullToken );
    if ( allNullable || !m_sawValue )
        nullable = true;
    column.nullable = nullable;
    return column;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CCsvSchemaResolver                                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

static inline UInt32
TextLength( const CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

/** A column name from header text: control characters become blanks, and the ends are trimmed */
static CString
HeaderName( const char* data, UInt32 length )
{GUCEF_TRACE;

    CString name;
    for ( UInt32 i = 0; i < length; ++i )
    {
        const char c = data[ i ];
        name += ( (unsigned char) c < 0x20 || 0x7F == c ) ? ' ' : c;
    }
    return name.Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

/** Empty names become column_<n>, repeated names get _2, _3 and so on; the result is unique */
static void
MakeUniqueNames( CString::StringVector& names )
{GUCEF_TRACE;

    GUCEF::set< CString > taken;
    for ( size_t i = 0; i < names.size(); ++i )
        if ( !names[ i ].IsNULLOrEmpty() )
            taken.insert( names[ i ] );

    GUCEF::set< CString > used;
    for ( size_t i = 0; i < names.size(); ++i )
    {
        CString candidate = names[ i ];
        if ( candidate.IsNULLOrEmpty() )
        {
            candidate = "column_" + UInt32ToString( (UInt32) i + 1 );
            UInt32 suffix = 2;
            while ( used.find( candidate ) != used.end() || taken.find( candidate ) != taken.end() )
                candidate = "column_" + UInt32ToString( (UInt32) i + 1 ) + "_" + UInt32ToString( suffix++ );
        }
        else
        if ( used.find( candidate ) != used.end() )
        {
            UInt32 suffix = 2;
            CString attempt = names[ i ] + "_" + UInt32ToString( suffix );
            while ( used.find( attempt ) != used.end() || taken.find( attempt ) != taken.end() )
                attempt = names[ i ] + "_" + UInt32ToString( ++suffix );
            candidate = attempt;
        }
        used.insert( candidate );
        names[ i ] = candidate;
    }
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvSchemaResolver::Resolve( CIOAccess& source               ,
                             const IOAccessPtr& sourceHolder ,
                             CDataTableBufferPool& pool      ,
                             const SCsvReadOptions& options  ,
                             MT::CMutex* sharedFileLock      ,
                             CITableFileMetaDataPtr& metaData,
                             CString& error                  )
{GUCEF_TRACE;

    metaData.Unlink();
    CCsvRecordReader records( source, sourceHolder, pool, options, sharedFileLock );
    TDataTableStatus status = records.Start( error );
    if ( DATATABLE_STATUS_OK != status )
        return status;

    const bool hasSchema = !options.schemaColumns.empty();
    UInt32 columnCount = 0;
    CString::StringVector names;
    bool gotRecord = false;

    if ( options.hasHeader )
    {
        status = records.Next( (UInt32) options.schemaColumns.size(), gotRecord, error );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        if ( !gotRecord )
        {
            error = "The file has no header row";
            return DATATABLE_STATUS_CORRUPT_DATA;
        }
        columnCount = records.GetFieldCount();
        for ( UInt32 i = 0; i < columnCount; ++i )
        {
            if ( !CCsvTableText::IsAscii( records.GetFieldData( i ), records.GetFieldLength( i ) ) &&
                 !CCsvTableText::IsValidUtf8( records.GetFieldData( i ), records.GetFieldLength( i ) ) )
            {
                error = "The header holds text that is not valid UTF-8 (" + records.GetParser().DescribeLast() + ")";
                return DATATABLE_STATUS_CORRUPT_DATA;
            }
            names.push_back( HeaderName( records.GetFieldData( i ), records.GetFieldLength( i ) ) );
        }
        if ( hasSchema && columnCount != options.schemaColumns.size() )
        {
            error = "The header has " + UInt32ToString( columnCount ) + " columns where the schema of the options has " + UInt32ToString( (UInt32) options.schemaColumns.size() );
            return DATATABLE_STATUS_SCHEMA_MISMATCH;
        }
    }
    else
    if ( hasSchema )
    {
        columnCount = (UInt32) options.schemaColumns.size();
    }
    else
    {
        // No header and no schema: the first record tells the number of columns and is a row itself
        status = records.Next( 0, gotRecord, error );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        if ( !gotRecord )
        {
            error = "The file has no records";
            return DATATABLE_STATUS_CORRUPT_DATA;
        }
        columnCount = records.GetFieldCount();
        status = records.GetParser().Rewind( error );
        if ( DATATABLE_STATUS_OK == status )
            status = records.Start( error );
        if ( DATATABLE_STATUS_OK != status )
            return status;
    }

    if ( columnCount > options.maxColumns )
    {
        error = "The file has more than " + UInt32ToString( options.maxColumns ) + " columns";
        return DATATABLE_STATUS_LIMIT_EXCEEDED;
    }

    CDataTableSchema schema;
    CValueList keyValues;
    UInt32 lossyColumns = 0;

    if ( hasSchema )
    {
        for ( size_t i = 0; i < options.schemaColumns.size(); ++i )
        {
            if ( !schema.AddColumn( options.schemaColumns[ i ].column ) )
            {
                error = "The schema of the options has a column that is not valid or is there twice: \"" + options.schemaColumns[ i ].column.name + "\"";
                return DATATABLE_STATUS_INVALID_ARGUMENT;
            }
        }
    }
    else
    {
        names.resize( columnCount );
        MakeUniqueNames( names );

        // the types the options fix
        GUCEF::vector< const SCsvColumnSpec* > fixed( columnCount, (const SCsvColumnSpec*) GUCEF_NULL );
        for ( size_t o = 0; o < options.columnOverrides.size(); ++o )
        {
            bool found = false;
            for ( UInt32 i = 0; i < columnCount; ++i )
            {
                if ( names[ i ] == options.columnOverrides[ o ].column.name )
                {
                    fixed[ i ] = &options.columnOverrides[ o ];
                    found = true;
                    break;
                }
            }
            if ( !found )
            {
                error = "The option columnTypes names a column that the file does not have: \"" + options.columnOverrides[ o ].column.name + "\"";
                return DATATABLE_STATUS_INVALID_ARGUMENT;
            }
        }

        // the types the values tell
        bool needInference = false;
        for ( UInt32 i = 0; i < columnCount; ++i )
            if ( GUCEF_NULL == fixed[ i ] )
                needInference = true;

        GUCEF::vector< CCsvColumnInference > inference( columnCount, CCsvColumnInference( options ) );
        UInt64 rows = 0;
        while ( needInference && ( 0 == options.inferRowCount || rows < options.inferRowCount ) )
        {
            status = records.Next( columnCount, gotRecord, error );
            if ( DATATABLE_STATUS_OK != status )
                return status;
            if ( !gotRecord )
                break;

            const UInt32 fieldCount = records.GetFieldCount();
            if ( ( fieldCount < columnCount && !options.allowMissingFields ) || ( fieldCount > columnCount && !options.allowExtraFields ) )
            {
                error = "A record has " + UInt32ToString( fieldCount ) + " fields where the file has " + UInt32ToString( columnCount ) +
                        " columns (" + records.GetParser().DescribeLast() + ")";
                return DATATABLE_STATUS_CORRUPT_DATA;
            }
            for ( UInt32 i = 0; i < columnCount; ++i )
            {
                if ( GUCEF_NULL != fixed[ i ] )
                    continue;
                if ( i < fieldCount )
                    inference[ i ].Feed( records.GetFieldData( i ), records.GetFieldLength( i ), records.IsFieldQuoted( i ) );
                else
                    inference[ i ].FeedMissing();
            }
            ++rows;
        }

        const bool allNullable = ( 0 != options.inferRowCount );
        for ( UInt32 i = 0; i < columnCount; ++i )
        {
            CDataTableColumnDef column;
            if ( GUCEF_NULL != fixed[ i ] )
            {
                column = fixed[ i ]->column;
                column.name = names[ i ];
                column.nullable = true;
                if ( fixed[ i ]->hasDateFormat )
                    keyValues.Set( "dateFormat." + names[ i ], CVariant( fixed[ i ]->dateFormat.text ) );
            }
            else
            {
                CString dateFormat;
                column = inference[ i ].GetColumn( names[ i ], allNullable, dateFormat );
                if ( !dateFormat.IsNULLOrEmpty() )
                    keyValues.Set( "dateFormat." + names[ i ], CVariant( dateFormat ) );

                // digits beyond what a float64 keeps: a column of identifiers has to be declared text (section 11.1)
                const UInt64 lossyValues = inference[ i ].GetLossyFloatValueCount();
                if ( DATATABLE_TYPE_FLOAT64 == column.type && lossyValues > 0 )
                {
                    keyValues.Set( "lossyInference." + names[ i ], CVariant( lossyValues ) );
                    ++lossyColumns;
                    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "CsvTableCodec: Column \"" << names[ i ] << "\" was inferred as float64 and " << lossyValues <<
                            " of its values have more than 15 digits, which a float64 rounds. Declare the column as text or decimal if the digits matter" );
                }
            }

            if ( !schema.AddColumn( column ) )
            {
                error = "Column \"" + names[ i ] + "\" cannot be a column of a table";
                return DATATABLE_STATUS_CORRUPT_DATA;
            }
        }
    }

    // the keys are all CStrings: a key written as a character literal is an ASCII variant, which compares equal to a longer
    // UTF-8 key that starts with it ("lossyInference" and "lossyInference.id")
    keyValues.Set( CString( "lossyInference" ), CVariant( (UInt64) lossyColumns ) );

    const CCsvRecordParser& parser = records.GetParser();
    keyValues.Set( "lineEnding", CVariant( parser.GetLineEnding() ) );
    keyValues.Set( "bom", CVariant( CString( parser.HadByteOrderMark() ? "true" : "false" ) ) );
    CString delimiterText;
    delimiterText += options.delimiter;
    keyValues.Set( "delimiter", CVariant( delimiterText ) );

    // made as a pointer to the mutable type, whose destructor does the release, then shared as immutable
    CTSharedPtr< CCsvTableMetaData, MT::CMutex > result( GUCEF_NEW CCsvTableMetaData( CreateDataTableSchemaPtr( schema ), keyValues ) );
    metaData = CITableFileMetaDataPtr( result );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
