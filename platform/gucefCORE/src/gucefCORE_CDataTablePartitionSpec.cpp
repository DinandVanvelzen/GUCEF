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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CDATATABLEPRUNER_H
#include "gucefCORE_CDataTablePruner.h"
#define GUCEF_CORE_CDATATABLEPRUNER_H
#endif /* GUCEF_CORE_CDATATABLEPRUNER_H ? */

#include "gucefCORE_CDataTablePartitionSpec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char g_hexDigits[] = "0123456789ABCDEF";

inline bool
IsPlainSegmentByte( unsigned char c )
{
    return ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '.' == c || '_' == c || '-' == c;
}

/** Whether the byte can occur in an escaped segment value: the plain bytes and the '%' of an escape with its hex digits */
inline bool
IsEscapedValueByte( unsigned char c )
{
    return IsPlainSegmentByte( c ) || '%' == c;
}

inline bool
IsFieldNameChar( unsigned char c )
{
    return ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || '_' == c;
}

inline char
ToUpperAscii( char c )
{
    return ( c >= 'a' && c <= 'z' ) ? (char) ( c - 'a' + 'A' ) : c;
}

/** True when the bytes, ignoring case, start with a Windows device name that is alone or followed by '.' */
bool
StartsWithWindowsDeviceName( const char* bytes, UInt32 length )
{
    static const char* const threeLetterNames[] = { "CON", "PRN", "AUX", "NUL" };
    static const char* const numberedNames[] = { "COM", "LPT" };

    for ( UInt32 i=0; i<4; ++i )
    {
        if ( length >= 3 && ToUpperAscii( bytes[ 0 ] ) == threeLetterNames[ i ][ 0 ] &&
             ToUpperAscii( bytes[ 1 ] ) == threeLetterNames[ i ][ 1 ] && ToUpperAscii( bytes[ 2 ] ) == threeLetterNames[ i ][ 2 ] &&
             ( 3 == length || '.' == bytes[ 3 ] ) )
            return true;
    }
    for ( UInt32 i=0; i<2; ++i )
    {
        if ( length >= 4 && ToUpperAscii( bytes[ 0 ] ) == numberedNames[ i ][ 0 ] &&
             ToUpperAscii( bytes[ 1 ] ) == numberedNames[ i ][ 1 ] && ToUpperAscii( bytes[ 2 ] ) == numberedNames[ i ][ 2 ] &&
             bytes[ 3 ] >= '1' && bytes[ 3 ] <= '9' && ( 4 == length || '.' == bytes[ 4 ] ) )
            return true;
    }
    return false;
}

/** A string of the given bytes, which need not be zero terminated */
CUtf8String
Utf8FromBytes( const char* bytes, UInt32 length )
{
    CUtf8String result;
    if ( length > 0 )
        result.Append( bytes, length );
    return result;
}

inline Int32
HexValue( char c )
{
    if ( c >= '0' && c <= '9' ) return c - '0';
    if ( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
    if ( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
    return -1;
}

/** The days of a date or timestamp variant of any unit, floor division for timestamps */
bool
DaysOfDateOrTimestamp( const CVariant& value, Int32& days )
{
    const UInt8 typeId = value.GetTypeId();
    if ( GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId || GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId )
    {
        days = value.AsInt32();
        return true;
    }

    Int64 ticksPerDay = 0;
    TDataTableTimeUnit unit = DATATABLE_TIMEUNIT_MICROSECONDS;
    if ( CDataTableValues::TimeUnitFromTypeId( typeId, unit ) )
        ticksPerDay = CDataTableValues::TicksPerDay( unit );
    else
    if ( GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH == typeId || GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH == typeId )
        ticksPerDay = 86400;
    else
        return false;

    CVariant native( value );
    native.ToNativeEndian();
    Int64 ticks = native.AsInt64();
    Int64 result = ticks / ticksPerDay;
    if ( ( ticks % ticksPerDay ) < 0 )
        --result;
    if ( result < GUCEF_INT32MIN || result > GUCEF_INT32MAX )
        return false;
    days = (Int32) result;
    return true;
}

/** Parses decimal digits with an optional leading '-' into sign and magnitude; false for anything else or overflow */
bool
ParseIntegerText( const char* text, UInt32 length, bool& negative, UInt64& magnitude )
{
    negative = false;
    magnitude = 0;
    UInt32 pos = 0;
    if ( pos < length && '-' == text[ pos ] )
    {
        negative = true;
        ++pos;
    }
    if ( pos >= length )
        return false;
    for ( ; pos<length; ++pos )
    {
        const char c = text[ pos ];
        if ( c < '0' || c > '9' )
            return false;
        const UInt64 digit = (UInt64) ( c - '0' );
        if ( magnitude > ( GUCEF_UINT64MAX - digit ) / 10 )
            return false;
        magnitude = magnitude * 10 + digit;
    }
    return true;
}

/**
 *  The native value (5.2) an IDENTITY key value has in its source column: text parsed with the column type, or a
 *  typed value. False when it cannot be told, which never prunes.
 */
bool
IdentityKeyToNative( const CDataTableColumnDef& column, const CVariant& keyValue, CVariant& native )
{
    if ( keyValue.IsString() )
    {
        const CUtf8String text = keyValue.AsUtf8String();
        const char* bytes = text.C_String();
        const UInt32 length = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
        switch ( column.type )
        {
            case DATATABLE_TYPE_UTF8:
            {
                native = text;
                return true;
            }
            case DATATABLE_TYPE_BINARY:
            {
                native.Set( bytes, length, GUCEF_DATATYPE_BINARY_BLOB, false );
                return true;
            }
            case DATATABLE_TYPE_DATE:
            {
                Int32 days = 0;
                if ( !CDataTableValues::ParseDate( bytes, length, days ) )
                    return false;
                native = (Int64) days;
                return true;
            }
            default:
            {
                if ( !DataTableLogicalTypeIsInteger( column.type ) )
                    return false;
                bool negative = false;
                UInt64 magnitude = 0;
                if ( !ParseIntegerText( bytes, length, negative, magnitude ) )
                    return false;
                if ( DataTableLogicalTypeIsUnsigned( column.type ) )
                {
                    if ( negative && 0 != magnitude )
                        return false;
                    native = magnitude;
                    return true;
                }
                if ( magnitude > (UInt64) GUCEF_INT64MAX + ( negative ? 1 : 0 ) )
                    return false;
                native = negative ? ( magnitude == (UInt64) GUCEF_INT64MAX + 1 ? GUCEF_INT64MIN : -(Int64) magnitude ) : (Int64) magnitude;
                return true;
            }
        }
    }

    const UInt8 typeId = keyValue.GetTypeId();
    if ( GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId || GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId )
    {
        if ( DATATABLE_TYPE_DATE != column.type )
            return false;
        CVariant value( keyValue );
        value.ToNativeEndian();
        native = (Int64) value.AsInt32();
        return true;
    }
    if ( keyValue.IsInteger() && DataTableLogicalTypeIsInteger( column.type ) )
    {
        TDataTableTimeUnit unit = DATATABLE_TIMEUNIT_MICROSECONDS;
        if ( CDataTableValues::TimeUnitFromTypeId( typeId, unit ) )
            return false;
        CVariant value( keyValue );
        value.ToNativeEndian();
        if ( DataTableLogicalTypeIsUnsigned( column.type ) )
        {
            if ( value.IsSignedInteger() && value.AsInt64() < 0 )
                return false;
            native = value.AsUInt64();
            return true;
        }
        if ( value.IsUnsignedInteger() && value.AsUInt64() > (UInt64) GUCEF_INT64MAX )
            return false;
        native = value.AsInt64();
        return true;
    }
    return false;
}

}


/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTablePartitionSpec::SField::SField( void )
    : name()
    , sourceColumn()
    , transform( DATATABLE_PARTITION_TRANSFORM_IDENTITY )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::SField::operator==( const SField& other ) const
{GUCEF_TRACE;

    return name == other.name && sourceColumn == other.sourceColumn && transform == other.transform;
}

/*-------------------------------------------------------------------------*/

CDataTablePartitionSpec::CDataTablePartitionSpec( void )
    : CIDataNodeSerializable()
    , m_fields()
    , m_pathTemplate()
    , m_segments()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTablePartitionSpec::CDataTablePartitionSpec( const CDataTablePartitionSpec& src )
    : CIDataNodeSerializable( src )
    , m_fields( src.m_fields )
    , m_pathTemplate( src.m_pathTemplate )
    , m_segments( src.m_segments )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTablePartitionSpec::~CDataTablePartitionSpec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTablePartitionSpec&
CDataTablePartitionSpec::operator=( const CDataTablePartitionSpec& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CIDataNodeSerializable::operator=( src );
        m_fields = src.m_fields;
        m_pathTemplate = src.m_pathTemplate;
        m_segments = src.m_segments;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::operator==( const CDataTablePartitionSpec& other ) const
{GUCEF_TRACE;

    return m_fields == other.m_fields && m_pathTemplate == other.m_pathTemplate;
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::operator!=( const CDataTablePartitionSpec& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::AddField( const CString& name, const CString& sourceColumn, TDataTablePartitionTransform transform )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() || sourceColumn.IsNULLOrEmpty() || GUCEF_NULL != FindField( name ) )
        return false;
    CUtf8String utf8Name = ToUtf8String( name );
    const char* bytes = utf8Name.C_String();
    for ( UInt32 i=0; i<utf8Name.ByteSize()-1; ++i )
    {
        if ( !IsFieldNameChar( (unsigned char) bytes[ i ] ) )
            return false;
    }
    if ( transform < DATATABLE_PARTITION_TRANSFORM_IDENTITY || transform > DATATABLE_PARTITION_TRANSFORM_DAY )
        return false;

    SField field;
    field.name = name;
    field.sourceColumn = sourceColumn;
    field.transform = transform;
    m_fields.push_back( field );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::ParseTemplate( const CString& pathTemplate, TSegmentVector& segments )
{GUCEF_TRACE;

    segments.clear();
    CUtf8String utf8 = ToUtf8String( pathTemplate );
    const char* bytes = utf8.C_String();
    UInt32 length = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
    if ( 0 == length )
        return false;

    TSegmentPartVector parts;
    CUtf8String literal;
    CUtf8String segmentText;
    bool lastWasToken = false;
    UInt32 i = 0;
    while ( i <= length )
    {
        char c = i < length ? bytes[ i ] : '/';
        if ( '/' == c )
        {
            // end of a segment
            if ( segmentText.IsNULLOrEmpty() || segmentText == "." || segmentText == ".." )
                return false;
            if ( !literal.IsNULLOrEmpty() || parts.empty() )
            {
                SSegmentPart part;
                part.literal = ToString( literal );
                parts.push_back( part );
            }
            segments.push_back( parts );
            parts.clear();
            literal.Clear();
            segmentText.Clear();
            lastWasToken = false;
            ++i;
            continue;
        }
        if ( '\\' == c || '}' == c )
            return false;
        if ( '{' == c )
        {
            if ( lastWasToken && literal.IsNULLOrEmpty() )
                return false;   // two tokens touch: the segment could not be parsed back
            UInt32 end = i + 1;
            while ( end < length && IsFieldNameChar( (unsigned char) bytes[ end ] ) )
                ++end;
            if ( end >= length || '}' != bytes[ end ] || end == i + 1 )
                return false;
            SSegmentPart part;
            part.literal = ToString( literal );
            part.token = ToString( Utf8FromBytes( bytes + i + 1, end - i - 1 ) );
            parts.push_back( part );
            segmentText.Append( bytes + i, end - i + 1 );
            literal.Clear();
            lastWasToken = true;
            i = end + 1;
            continue;
        }
        literal.Append( bytes + i, 1 );
        segmentText.Append( bytes + i, 1 );
        ++i;
    }
    return !segments.empty();
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::SetPathTemplate( const CString& pathTemplate )
{GUCEF_TRACE;

    TSegmentVector segments;
    if ( !ParseTemplate( pathTemplate, segments ) )
        return false;
    m_pathTemplate = pathTemplate;
    m_segments = segments;
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CDataTablePartitionSpec::GetPathTemplate( void ) const
{GUCEF_TRACE;

    return m_pathTemplate;
}

/*-------------------------------------------------------------------------*/

const CDataTablePartitionSpec::TFieldVector&
CDataTablePartitionSpec::GetFields( void ) const
{GUCEF_TRACE;

    return m_fields;
}

/*-------------------------------------------------------------------------*/

const CDataTablePartitionSpec::SField*
CDataTablePartitionSpec::FindField( const CString& name ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_fields.size(); ++i )
    {
        if ( m_fields[ i ].name == name )
            return &m_fields[ i ];
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTablePartitionSpec::GetSegmentCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_segments.size();
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::IsPartitioned( void ) const
{GUCEF_TRACE;

    return !m_fields.empty();
}

/*-------------------------------------------------------------------------*/

void
CDataTablePartitionSpec::Clear( void )
{GUCEF_TRACE;

    m_fields.clear();
    m_pathTemplate.Clear();
    m_segments.clear();
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::Validate( const CDataTableSchema& schema, CString& error ) const
{GUCEF_TRACE;

    if ( m_fields.empty() )
    {
        if ( !m_pathTemplate.IsNULLOrEmpty() )
        {
            error = "A path template is set but there are no partition fields";
            return false;
        }
        return true;
    }
    if ( m_segments.empty() )
    {
        error = "Partition fields are set but there is no path template";
        return false;
    }

    // every token is a field, every field appears exactly once
    GUCEF::vector< UInt32 > occurrences( m_fields.size(), 0 );
    for ( UInt32 s=0; s<(UInt32) m_segments.size(); ++s )
    {
        for ( UInt32 p=0; p<(UInt32) m_segments[ s ].size(); ++p )
        {
            const CString& token = m_segments[ s ][ p ].token;
            if ( token.IsNULLOrEmpty() )
                continue;
            bool found = false;
            for ( UInt32 f=0; f<(UInt32) m_fields.size(); ++f )
            {
                if ( m_fields[ f ].name == token )
                {
                    ++occurrences[ f ];
                    found = true;
                }
            }
            if ( !found )
            {
                error = "The path template uses {" + token + "} which is not a partition field";
                return false;
            }
        }
    }

    // ParseSegment ends a value at the first occurrence of the separator that follows it: a separator between two fields
    // must hold a byte no escaped value holds, else a value such as "BF-B" or "BRK.B" would be cut there. Such a
    // separator cannot start inside a value either: its occurrences would repeat with a period made of value bytes only
    for ( UInt32 s=0; s<(UInt32) m_segments.size(); ++s )
    {
        const TSegmentPartVector& parts = m_segments[ s ];
        for ( UInt32 p=1; p<(UInt32) parts.size(); ++p )
        {
            if ( parts[ p ].token.IsNULLOrEmpty() || parts[ p-1 ].token.IsNULLOrEmpty() )
                continue;
            CUtf8String separator = ToUtf8String( parts[ p ].literal );
            const UInt32 length = separator.ByteSize() > 0 ? separator.ByteSize() - 1 : 0;
            bool distinct = false;
            for ( UInt32 i=0; i<length && !distinct; ++i )
                distinct = !IsEscapedValueByte( (unsigned char) separator.C_String()[ i ] );
            if ( !distinct )
            {
                error = "The separator \"" + parts[ p ].literal + "\" between {" + parts[ p-1 ].token + "} and {" + parts[ p ].token +
                        "} in the path template can occur inside a value (escaped values hold A-Z, 0-9, '.', '_', '-' and '%'): " +
                        "it needs a byte outside those, for example '~' or a lower case letter";
                return false;
            }
        }
    }

    for ( UInt32 f=0; f<(UInt32) m_fields.size(); ++f )
    {
        const SField& field = m_fields[ f ];
        if ( 1 != occurrences[ f ] )
        {
            error = "Partition field \"" + field.name + "\" must appear exactly once in the path template";
            return false;
        }

        Int32 columnIndex = schema.FindColumn( field.sourceColumn );
        if ( columnIndex < 0 )
        {
            error = "Partition field \"" + field.name + "\" refers to column \"" + field.sourceColumn + "\" which does not exist";
            return false;
        }
        const CDataTableColumnDef& column = schema.GetColumn( (UInt32) columnIndex );
        if ( column.nullable )
        {
            error = "Partition source column \"" + column.name + "\" is nullable";
            return false;
        }

        bool typeFits = false;
        if ( DATATABLE_PARTITION_TRANSFORM_IDENTITY == field.transform )
            typeFits = DATATABLE_TYPE_UTF8 == column.type || DATATABLE_TYPE_DATE == column.type || DataTableLogicalTypeIsInteger( column.type );
        else
            typeFits = DATATABLE_TYPE_DATE == column.type || DATATABLE_TYPE_TIMESTAMP == column.type;
        if ( !typeFits )
        {
            error = "Partition field \"" + field.name + "\" cannot apply " + DataTablePartitionTransformToString( field.transform ) +
                    " to column \"" + column.name + "\" of type " + column.GetTypeText();
            return false;
        }

        const CDataTableSchema::TColumnIndexVector& primaryKey = schema.GetPrimaryKey();
        if ( !primaryKey.empty() )
        {
            bool inKey = false;
            for ( UInt32 k=0; k<(UInt32) primaryKey.size(); ++k )
                inKey = inKey || primaryKey[ k ] == (UInt32) columnIndex;
            if ( !inKey )
            {
                error = "Partition source column \"" + column.name + "\" is not part of the primary key";
                return false;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CDataTablePartitionSpec::EscapeSegmentValue( const CString& valueText )
{GUCEF_TRACE;

    CUtf8String utf8 = ToUtf8String( valueText );
    const char* bytes = utf8.C_String();
    UInt32 length = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
    bool escapeFirst = StartsWithWindowsDeviceName( bytes, length );

    CUtf8String result;
    for ( UInt32 i=0; i<length; ++i )
    {
        unsigned char c = (unsigned char) bytes[ i ];
        bool escape = !IsPlainSegmentByte( c ) ||
                      ( '.' == c && ( 0 == i || length - 1 == i ) ) ||
                      ( 0 == i && escapeFirst );
        if ( escape )
        {
            char hex[ 3 ] = { '%', g_hexDigits[ c >> 4 ], g_hexDigits[ c & 0x0F ] };
            result.Append( hex, 3 );
        }
        else
        {
            result.Append( bytes + i, 1 );
        }
    }
    return ToString( result );
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::UnescapeSegmentValue( const CString& escapedText, CString& valueText )
{GUCEF_TRACE;

    CUtf8String utf8 = ToUtf8String( escapedText );
    const char* bytes = utf8.C_String();
    UInt32 length = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
    if ( 0 == length )
        return false;

    GUCEF::vector< char > decoded;
    decoded.reserve( length + 1 );
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( '%' == bytes[ i ] )
        {
            if ( i + 2 >= length )
                return false;
            Int32 high = HexValue( bytes[ i+1 ] );
            Int32 low = HexValue( bytes[ i+2 ] );
            if ( high < 0 || low < 0 )
                return false;
            decoded.push_back( (char) ( ( high << 4 ) | low ) );
            i += 2;
        }
        else
        {
            decoded.push_back( bytes[ i ] );
        }
    }
    decoded.push_back( '\0' );

    CString value = ToString( CUtf8String( &decoded[ 0 ] ) );

    // only the canonical escaped form is accepted, so a path maps to exactly one value
    if ( EscapeSegmentValue( value ) != escapedText )
        return false;
    valueText = value;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::FormatFieldValue( const SField& field, const CVariant& value, CString& text )
{GUCEF_TRACE;

    const UInt8 typeId = value.GetTypeId();
    const bool isDate = GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId || GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId;

    if ( DATATABLE_PARTITION_TRANSFORM_IDENTITY == field.transform )
    {
        if ( isDate )
        {
            text = CDataTableValues::DateToString( value.AsInt32() );
            return true;
        }
        if ( value.IsString() )
        {
            text = value.AsString();
            return !text.IsNULLOrEmpty();
        }
        TDataTableTimeUnit unit = DATATABLE_TIMEUNIT_MICROSECONDS;
        if ( CDataTableValues::TimeUnitFromTypeId( typeId, unit ) )
            return false;
        if ( value.IsSignedInteger() )
        {
            CVariant native( value );
            native.ToNativeEndian();
            text = Int64ToString( native.AsInt64() );
            return true;
        }
        if ( value.IsUnsignedInteger() )
        {
            CVariant native( value );
            native.ToNativeEndian();
            text = UInt64ToString( native.AsUInt64() );
            return true;
        }
        return false;
    }

    Int32 days = 0;
    if ( !DaysOfDateOrTimestamp( value, days ) )
        return false;
    CString dateText = CDataTableValues::DateToString( days );
    switch ( field.transform )
    {
        case DATATABLE_PARTITION_TRANSFORM_YEAR:    { text = dateText.SubstrFromRange( 0, dateText.Length() - 6 ); return true; }
        case DATATABLE_PARTITION_TRANSFORM_MONTH:   { text = dateText.SubstrFromRange( 0, dateText.Length() - 3 ); return true; }
        case DATATABLE_PARTITION_TRANSFORM_DAY:     { text = dateText; return true; }
        default:                                    { return false; }
    }
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::ParseFieldValue( const SField& field, const CString& text, CVariant& value )
{GUCEF_TRACE;

    if ( DATATABLE_PARTITION_TRANSFORM_IDENTITY == field.transform )
    {
        value = ToUtf8String( text );
        return true;
    }

    CString dateText = text;
    if ( DATATABLE_PARTITION_TRANSFORM_YEAR == field.transform )
        dateText += "-01-01";
    else
    if ( DATATABLE_PARTITION_TRANSFORM_MONTH == field.transform )
        dateText += "-01";

    CUtf8String utf8 = ToUtf8String( dateText );
    Int32 days = 0;
    if ( !CDataTableValues::ParseDate( utf8.C_String(), utf8.ByteSize() - 1, days ) )
        return false;
    value = CDataTableValues::Date( days );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::BuildPath( const CValueList& keyValues, CString& relativePath ) const
{GUCEF_TRACE;

    if ( m_segments.empty() )
        return false;

    CString path;
    for ( UInt32 s=0; s<(UInt32) m_segments.size(); ++s )
    {
        CString segmentName;
        if ( !BuildSegment( s, keyValues, segmentName ) )
            return false;
        if ( s > 0 )
            path += '/';
        path += segmentName;
    }
    relativePath = path;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::BuildSegment( UInt32 segmentIndex, const CValueList& keyValues, CString& segmentName ) const
{GUCEF_TRACE;

    if ( segmentIndex >= (UInt32) m_segments.size() )
        return false;

    CString name;
    const TSegmentPartVector& parts = m_segments[ segmentIndex ];
    for ( UInt32 p=0; p<(UInt32) parts.size(); ++p )
    {
        name += parts[ p ].literal;
        if ( parts[ p ].token.IsNULLOrEmpty() )
            continue;

        const SField* field = FindField( parts[ p ].token );
        CVariant value;
        CString text;
        if ( GUCEF_NULL == field || !keyValues.TryGetValue( field->name, value ) || !FormatFieldValue( *field, value, text ) )
            return false;
        name += EscapeSegmentValue( text );
    }
    segmentName = name;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDataTablePartitionSpec::GetSegmentFields( UInt32 segmentIndex, CString::StringVector& fieldNames ) const
{GUCEF_TRACE;

    fieldNames.clear();
    if ( segmentIndex >= (UInt32) m_segments.size() )
        return;
    const TSegmentPartVector& parts = m_segments[ segmentIndex ];
    for ( UInt32 p=0; p<(UInt32) parts.size(); ++p )
    {
        if ( !parts[ p ].token.IsNULLOrEmpty() )
            fieldNames.push_back( parts[ p ].token );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::ParseSegment( UInt32 segmentIndex, const CString& name, CValueList& keyValues ) const
{GUCEF_TRACE;

    if ( segmentIndex >= (UInt32) m_segments.size() )
        return false;

    CUtf8String utf8 = ToUtf8String( name );
    const char* bytes = utf8.C_String();
    const UInt32 length = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( (unsigned char) bytes[ i ] >= 0x80 )
            return false;   // escaped names are plain ASCII
    }

    const TSegmentPartVector& parts = m_segments[ segmentIndex ];
    CValueList parsed;
    UInt32 pos = 0;
    for ( UInt32 p=0; p<(UInt32) parts.size(); ++p )
    {
        CUtf8String literal = ToUtf8String( parts[ p ].literal );
        const UInt32 literalLength = literal.ByteSize() > 0 ? literal.ByteSize() - 1 : 0;
        if ( pos + literalLength > length || 0 != memcmp( bytes + pos, literal.C_String(), literalLength ) )
            return false;
        pos += literalLength;
        if ( parts[ p ].token.IsNULLOrEmpty() )
            continue;

        // the value runs up to the literal that follows: the suffix of the segment, or its next occurrence
        UInt32 end = length;
        if ( p + 1 < (UInt32) parts.size() )
        {
            CUtf8String next = ToUtf8String( parts[ p+1 ].literal );
            const UInt32 nextLength = next.ByteSize() > 0 ? next.ByteSize() - 1 : 0;
            if ( parts[ p+1 ].token.IsNULLOrEmpty() && p + 2 == (UInt32) parts.size() )
            {
                if ( nextLength > length || length - nextLength < pos )
                    return false;
                end = length - nextLength;
            }
            else
            {
                end = pos + 1;
                while ( end + nextLength <= length && 0 != memcmp( bytes + end, next.C_String(), nextLength ) )
                    ++end;
                if ( end + nextLength > length )
                    return false;
            }
        }
        if ( end <= pos )
            return false;

        const SField* field = FindField( parts[ p ].token );
        CString valueText;
        CVariant value;
        if ( GUCEF_NULL == field ||
             !UnescapeSegmentValue( ToString( Utf8FromBytes( bytes + pos, end - pos ) ), valueText ) ||
             !ParseFieldValue( *field, valueText, value ) )
            return false;
        parsed.Delete( field->name );
        parsed.Set( field->name, value );
        pos = end;
    }
    if ( pos != length )
        return false;

    // the parsed fields replace the values the keys had
    CValueList::TValueMap::const_iterator i = parsed.GetDataBeginIterator();
    while ( i != parsed.GetDataEndIterator() )
    {
        keyValues.Delete( (*i).first );
        if ( !(*i).second.empty() )
            keyValues.Set( (*i).first, (*i).second.front() );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::MayMatch( const CValueList& keyValues, const CDataTableResolvedQuery& query ) const
{GUCEF_TRACE;

    if ( query.matchesNothing )
        return false;
    if ( query.schema.IsNULL() || query.predicates.empty() )
        return true;

    const CDataTableSchema& schema = *query.schema;
    for ( UInt32 f=0; f<(UInt32) m_fields.size(); ++f )
    {
        const SField& field = m_fields[ f ];
        const Int32 columnIndex = schema.FindColumn( field.sourceColumn );
        if ( columnIndex < 0 )
            continue;
        CVariant keyValue;
        if ( !keyValues.TryGetValue( field.name, keyValue ) || !keyValue.IsInitialized() || keyValue.IsNULL() )
            continue;
        const CDataTableColumnDef& column = schema.GetColumn( (UInt32) columnIndex );

        // the partition as statistics: one value (identity) or the range of its period, no nulls (source columns are not nullable)
        CDataTableColumnStats stats;
        stats.hasNullCount = true;
        stats.nullCount = 0;
        stats.valueCount = 1;
        stats.hasMinMax = true;
        if ( DATATABLE_PARTITION_TRANSFORM_IDENTITY == field.transform )
        {
            if ( !IdentityKeyToNative( column, keyValue, stats.minValue ) )
                continue;
            stats.maxValue = stats.minValue;
            stats.minExact = true;
            stats.maxExact = true;
        }
        else
        {
            Int32 days = 0;
            if ( !DaysOfDateOrTimestamp( keyValue, days ) )
                continue;
            Int32 year = 0;
            UInt32 month = 0;
            UInt32 day = 0;
            CDataTableValues::CivilFromDays( days, year, month, day );
            Int32 first = days;
            Int32 last = days;
            bool known = true;
            if ( DATATABLE_PARTITION_TRANSFORM_YEAR == field.transform )
            {
                Int32 next = 0;
                known = CDataTableValues::DaysFromCivil( year, 1, 1, first ) && CDataTableValues::DaysFromCivil( year + 1, 1, 1, next );
                last = next - 1;
            }
            else
            if ( DATATABLE_PARTITION_TRANSFORM_MONTH == field.transform )
            {
                Int32 next = 0;
                known = CDataTableValues::DaysFromCivil( year, month, 1, first ) &&
                        CDataTableValues::DaysFromCivil( 12 == month ? year + 1 : year, 12 == month ? 1 : month + 1, 1, next );
                last = next - 1;
            }
            if ( !known )
                continue;

            if ( DATATABLE_TYPE_DATE == column.type )
            {
                stats.minValue = (Int64) first;
                stats.maxValue = (Int64) last;
            }
            else
            if ( DATATABLE_TYPE_TIMESTAMP == column.type )
            {
                const Int64 ticksPerDay = CDataTableValues::TicksPerDay( column.timeUnit );
                const Int64 lastDayEnd = (Int64) last + 1;
                if ( (Int64) first < GUCEF_INT64MIN / ticksPerDay || lastDayEnd > GUCEF_INT64MAX / ticksPerDay )
                    continue;
                stats.minValue = (Int64) first * ticksPerDay;
                stats.maxValue = lastDayEnd * ticksPerDay - 1;
            }
            else
            {
                continue;
            }
            // the bounds of a period, not values known to be present
            stats.minExact = first == last && DATATABLE_TYPE_DATE == column.type;
            stats.maxExact = stats.minExact;
        }

        for ( UInt32 p=0; p<(UInt32) query.predicates.size(); ++p )
        {
            const CDataTableResolvedPredicate& predicate = query.predicates[ p ];
            if ( predicate.columnIndex == (UInt32) columnIndex && !CDataTablePruner::MayMatch( predicate, column, stats ) )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::Serialize( CDataNode& domRootNode                        ,
                                    const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    CDataNode* fieldsNode = domRootNode.AddChild( "fields", GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == fieldsNode )
        return false;
    for ( UInt32 i=0; i<(UInt32) m_fields.size(); ++i )
    {
        CDataNode* fieldNode = fieldsNode->AddChild( "field", GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL == fieldNode )
            return false;
        success = fieldNode->SetAttribute( "name", m_fields[ i ].name ) && success;
        success = fieldNode->SetAttribute( "source", m_fields[ i ].sourceColumn ) && success;
        success = fieldNode->SetAttribute( "transform", CString( DataTablePartitionTransformToString( m_fields[ i ].transform ) ) ) && success;
    }
    success = domRootNode.SetAttribute( "pathTemplate", m_pathTemplate ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePartitionSpec::Deserialize( const CDataNode& domRootNode                  ,
                                      const CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CDataTablePartitionSpec spec;
    const CDataNode* fieldsNode = domRootNode.FindChild( "fields" );
    if ( GUCEF_NULL != fieldsNode )
    {
        CDataNode::const_iterator i = fieldsNode->ConstBegin();
        while ( i != fieldsNode->ConstEnd() )
        {
            const CDataNode& fieldNode = *(*i);
            TDataTablePartitionTransform transform = DATATABLE_PARTITION_TRANSFORM_IDENTITY;
            CString transformText = fieldNode.GetAttributeValueOrChildValueByName( "transform" ).AsString( CString::Empty, true );
            if ( !transformText.IsNULLOrEmpty() && !DataTablePartitionTransformFromString( transformText, transform ) )
                return false;
            if ( !spec.AddField( fieldNode.GetAttributeValueOrChildValueByName( "name" ).AsString( CString::Empty, true ),
                                 fieldNode.GetAttributeValueOrChildValueByName( "source" ).AsString( CString::Empty, true ),
                                 transform ) )
                return false;
            ++i;
        }
    }

    CString pathTemplate = domRootNode.GetAttributeValueOrChildValueByName( "pathTemplate" ).AsString( CString::Empty, true );
    if ( !pathTemplate.IsNULLOrEmpty() && !spec.SetPathTemplate( pathTemplate ) )
        return false;

    *this = spec;
    return true;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDataTablePartitionSpec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDataTablePartitionSpec( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CDataTablePartitionSpec::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CDataTablePartitionSpec";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
