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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCORE_CDataTableColumnDef.h"

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

/** Strict unsigned decimal number of at most 3 digits */
bool
ParseSmallNumber( const CString& text, UInt32& value )
{
    CString trimmed = text.Trim( true ).Trim( false );
    UInt32 length = trimmed.Length();
    if ( 0 == length || length > 3 )
        return false;
    UInt32 result = 0;
    for ( UInt32 i=0; i<length; ++i )
    {
        char c = (char) trimmed[ i ];
        if ( c < '0' || c > '9' )
            return false;
        result = result * 10 + (UInt32) ( c - '0' );
    }
    value = result;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableColumnDef::CDataTableColumnDef( void )
    : CIDataNodeSerializable()
    , name()
    , type( DATATABLE_TYPE_UNSUPPORTED )
    , nullable( true )
    , decimalPrecision( DataTableMaxDecimalPrecision )
    , decimalScale( 0 )
    , timeUnit( DATATABLE_TIMEUNIT_MICROSECONDS )
    , utcAdjusted( true )
    , encodingHint( DATATABLE_ENCODING_AUTO )
    , description()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableColumnDef::CDataTableColumnDef( const CString& columnName, TDataTableLogicalType columnType, bool isNullable )
    : CIDataNodeSerializable()
    , name( columnName )
    , type( columnType )
    , nullable( isNullable )
    , decimalPrecision( DataTableMaxDecimalPrecision )
    , decimalScale( 0 )
    , timeUnit( DATATABLE_TIMEUNIT_MICROSECONDS )
    , utcAdjusted( true )
    , encodingHint( DATATABLE_ENCODING_AUTO )
    , description()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableColumnDef::CDataTableColumnDef( const CDataTableColumnDef& src )
    : CIDataNodeSerializable( src )
    , name( src.name )
    , type( src.type )
    , nullable( src.nullable )
    , decimalPrecision( src.decimalPrecision )
    , decimalScale( src.decimalScale )
    , timeUnit( src.timeUnit )
    , utcAdjusted( src.utcAdjusted )
    , encodingHint( src.encodingHint )
    , description( src.description )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableColumnDef::~CDataTableColumnDef()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableColumnDef&
CDataTableColumnDef::operator=( const CDataTableColumnDef& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CIDataNodeSerializable::operator=( src );
        name = src.name;
        type = src.type;
        nullable = src.nullable;
        decimalPrecision = src.decimalPrecision;
        decimalScale = src.decimalScale;
        timeUnit = src.timeUnit;
        utcAdjusted = src.utcAdjusted;
        encodingHint = src.encodingHint;
        description = src.description;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnDef::operator==( const CDataTableColumnDef& other ) const
{GUCEF_TRACE;

    return name == other.name                           &&
           type == other.type                           &&
           nullable == other.nullable                   &&
           decimalPrecision == other.decimalPrecision   &&
           decimalScale == other.decimalScale           &&
           timeUnit == other.timeUnit                   &&
           utcAdjusted == other.utcAdjusted             &&
           encodingHint == other.encodingHint           &&
           description == other.description;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnDef::operator!=( const CDataTableColumnDef& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

CDataTableColumnDef
CDataTableColumnDef::Decimal( const CString& columnName, UInt8 precision, UInt8 scale, bool isNullable )
{GUCEF_TRACE;

    CDataTableColumnDef column( columnName, DATATABLE_TYPE_DECIMAL, isNullable );
    column.decimalPrecision = precision;
    column.decimalScale = scale;
    return column;
}

/*-------------------------------------------------------------------------*/

CDataTableColumnDef
CDataTableColumnDef::Timestamp( const CString& columnName, TDataTableTimeUnit unit, bool isUtcAdjusted, bool isNullable )
{GUCEF_TRACE;

    CDataTableColumnDef column( columnName, DATATABLE_TYPE_TIMESTAMP, isNullable );
    column.timeUnit = unit;
    column.utcAdjusted = isUtcAdjusted;
    return column;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnDef::IsSameType( const CDataTableColumnDef& other ) const
{GUCEF_TRACE;

    if ( type != other.type )
        return false;
    if ( DATATABLE_TYPE_DECIMAL == type )
        return decimalPrecision == other.decimalPrecision && decimalScale == other.decimalScale;
    if ( DATATABLE_TYPE_TIMESTAMP == type )
        return timeUnit == other.timeUnit && utcAdjusted == other.utcAdjusted;
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CDataTableColumnDef::GetTypeText( void ) const
{GUCEF_TRACE;

    switch ( type )
    {
        case DATATABLE_TYPE_DECIMAL:
        {
            return CString( "decimal(" ) + UInt32ToString( decimalPrecision ) + ',' + UInt32ToString( decimalScale ) + ')';
        }
        case DATATABLE_TYPE_TIMESTAMP:
        {
            CString text = CString( "timestamp(" ) + DataTableTimeUnitToString( timeUnit );
            if ( utcAdjusted )
                text += ",utc";
            return text + ')';
        }
        default:
        {
            return DataTableLogicalTypeToString( type );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnDef::SetTypeFromText( const CString& typeText )
{GUCEF_TRACE;

    CString text = typeText.Trim( true ).Trim( false );
    Int32 open = text.HasChar( '(' );
    CString baseName = open >= 0 ? text.SubstrFromRange( 0, (UInt32) open ).Trim( false ) : text;
    CString::StringVector params;
    if ( open >= 0 )
    {
        if ( text.Length() < 2 || ')' != text[ text.Length()-1 ] )
            return false;
        CString inner = text.SubstrFromRange( (UInt32) open + 1, text.Length() - 1 );
        params = inner.ParseElements( ',', true );
        if ( params.empty() )
            return false;
    }

    TDataTableLogicalType parsedType = DATATABLE_TYPE_UNSUPPORTED;
    if ( !DataTableLogicalTypeFromString( baseName, parsedType ) || DATATABLE_TYPE_UNSUPPORTED == parsedType )
        return false;

    if ( DATATABLE_TYPE_DECIMAL == parsedType )
    {
        UInt32 precision = DataTableMaxDecimalPrecision;
        UInt32 scale = 0;
        if ( open >= 0 )
        {
            if ( params.size() < 1 || params.size() > 2 || !ParseSmallNumber( params[ 0 ], precision ) )
                return false;
            if ( 2 == params.size() && !ParseSmallNumber( params[ 1 ], scale ) )
                return false;
        }
        if ( precision < 1 || precision > DataTableMaxDecimalPrecision || scale > precision )
            return false;
        type = parsedType;
        decimalPrecision = (UInt8) precision;
        decimalScale = (UInt8) scale;
        return true;
    }

    if ( DATATABLE_TYPE_TIMESTAMP == parsedType )
    {
        TDataTableTimeUnit unit = DATATABLE_TIMEUNIT_MICROSECONDS;
        bool utc = true;
        if ( open >= 0 )
        {
            if ( params.size() > 2 || !DataTableTimeUnitFromString( params[ 0 ].Trim( true ).Trim( false ), unit ) )
                return false;
            utc = false;
            if ( 2 == params.size() )
            {
                if ( params[ 1 ].Trim( true ).Trim( false ) != "utc" )
                    return false;
                utc = true;
            }
        }
        type = parsedType;
        timeUnit = unit;
        utcAdjusted = utc;
        return true;
    }

    if ( open >= 0 )
        return false;
    type = parsedType;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnDef::IsValidName( const CString& columnName )
{GUCEF_TRACE;

    if ( columnName.IsNULLOrEmpty() )
        return false;
    const char* bytes = columnName.C_String();
    UInt32 byteCount = columnName.ByteSize() > 0 ? columnName.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<byteCount; ++i )
    {
        if ( (unsigned char) bytes[ i ] < 0x20 )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnDef::IsValid( CString& error ) const
{GUCEF_TRACE;

    if ( !IsValidName( name ) )
    {
        error = "Column name \"" + name + "\" is empty or holds control characters";
        return false;
    }
    if ( DATATABLE_TYPE_UNSUPPORTED == type )
    {
        error = "Column \"" + name + "\" has no supported type";
        return false;
    }
    if ( DATATABLE_TYPE_DECIMAL == type &&
         ( decimalPrecision < 1 || decimalPrecision > DataTableMaxDecimalPrecision || decimalScale > decimalPrecision ) )
    {
        error = "Column \"" + name + "\" has an invalid decimal precision or scale: " + GetTypeText();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnDef::Serialize( CDataNode& domRootNode                        ,
                                const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "name", name ) && success;
    success = domRootNode.SetAttribute( "type", GetTypeText() ) && success;
    if ( !nullable )
        success = domRootNode.SetAttribute( "nullable", false ) && success;
    if ( DATATABLE_ENCODING_AUTO != encodingHint )
        success = domRootNode.SetAttribute( "encoding", CString( DataTableEncodingHintToString( encodingHint ) ) ) && success;
    if ( !description.IsNULLOrEmpty() )
        success = domRootNode.SetAttribute( "description", description ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnDef::Deserialize( const CDataNode& domRootNode                  ,
                                  const CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CDataTableColumnDef column;
    column.name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( CString::Empty, true );
    if ( !column.SetTypeFromText( domRootNode.GetAttributeValueOrChildValueByName( "type" ).AsString( CString::Empty, true ) ) )
        return false;
    column.nullable = domRootNode.GetAttributeValueOrChildValueByName( "nullable" ).AsBool( true, true );

    CString encoding = domRootNode.GetAttributeValueOrChildValueByName( "encoding" ).AsString( CString::Empty, true );
    if ( !encoding.IsNULLOrEmpty() && !DataTableEncodingHintFromString( encoding, column.encodingHint ) )
        return false;
    column.description = domRootNode.GetAttributeValueOrChildValueByName( "description" ).AsString( CString::Empty, true );

    CString error;
    if ( !column.IsValid( error ) )
        return false;
    *this = column;
    return true;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDataTableColumnDef::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDataTableColumnDef( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CDataTableColumnDef::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CDataTableColumnDef";
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
