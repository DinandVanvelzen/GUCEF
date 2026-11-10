/*
 *  tablecodecpluginPARQUET: GUCEF table codec plugin for Apache Parquet files
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

#ifndef TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTREADER_H
#define TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_MACROS_H
#include "tablecodecpluginPARQUET_macros.h"
#define TABLECODECPLUGIN_PARQUET_MACROS_H
#endif /* TABLECODECPLUGIN_PARQUET_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace PARQUET {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The types of the Thrift compact protocol, as field types (the low nibble of a field header) and element types */
enum EThriftCompactType
{
    THRIFT_COMPACT_STOP         = 0     ,
    THRIFT_COMPACT_BOOL_TRUE    = 1     ,   /**< a field type; as an element type: a bool, also written as 2 */
    THRIFT_COMPACT_BOOL_FALSE   = 2     ,
    THRIFT_COMPACT_I8           = 3     ,
    THRIFT_COMPACT_I16          = 4     ,
    THRIFT_COMPACT_I32          = 5     ,
    THRIFT_COMPACT_I64          = 6     ,
    THRIFT_COMPACT_DOUBLE       = 7     ,
    THRIFT_COMPACT_BINARY       = 8     ,
    THRIFT_COMPACT_LIST         = 9     ,
    THRIFT_COMPACT_SET          = 10    ,
    THRIFT_COMPACT_MAP          = 11    ,
    THRIFT_COMPACT_STRUCT       = 12    ,
    THRIFT_COMPACT_UUID         = 13
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads the Thrift compact protocol from a byte range, without allocating.
 *
 *  Field headers carry a field id delta in the high nibble (0: a zigzag varint i16 id follows) and the type in the low
 *  nibble; i16, i32 and i64 are zigzag ULEB128 varints, i8 is one byte, doubles are 8 bytes little endian, binary is a
 *  varint length and the bytes. Field id deltas restart in every struct.
 *
 *  The input is untrusted: a varint of more than 10 bytes, a value beyond its type, a length or element count larger
 *  than the bytes left, an unknown type and nesting (structs, lists, sets, maps) deeper than the maximum depth make the
 *  reader fail. The first failure sticks: every later call returns false. Binary values are returned as pointers into
 *  the input.
 */
class TABLECODECPLUGIN_PARQUET_PUBLIC_CPP CThriftCompactReader
{
    public:

    static const UInt32 DefaultMaxDepth = 32;
    static const UInt32 MaxSupportedDepth = 64;

    CThriftCompactReader( const UInt8* data, UInt32 size, UInt32 maxDepth = DefaultMaxDepth );

    /** Enters a struct: the field ids of its fields count from 0 */
    bool BeginStruct( void );

    /** Leaves the struct entered last */
    bool EndStruct( void );

    /** The next field header of the current struct; type THRIFT_COMPACT_STOP (fieldId 0) ends the struct */
    bool ReadFieldHeader( UInt8& fieldType, Int16& fieldId );

    /** The value of a bool field: given by its field type */
    bool ReadBoolField( UInt8 fieldType, bool& value );

    /** A bool element of a list, set or map: one byte, 1 is true, 2 (or 0) is false */
    bool ReadBoolElement( bool& value );

    bool ReadI8( Int8& value );
    bool ReadI16( Int16& value );
    bool ReadI32( Int32& value );
    bool ReadI64( Int64& value );
    bool ReadDouble( Float64& value );

    /** A binary or string value: a pointer into the input and its length */
    bool ReadBinary( const UInt8*& bytes, UInt32& length );

    /** A list or set header: the element type and the element count, which is checked against the bytes left */
    bool ReadListHeader( UInt8& elementType, UInt32& size );

    /** A map header; the types are 0 for an empty map */
    bool ReadMapHeader( UInt8& keyType, UInt8& valueType, UInt32& size );

    /** Enters and leaves a list, set or map, for the nesting depth */
    bool BeginContainer( void );
    bool EndContainer( void );

    /** Skips a value of the given field or element type, nested values included */
    bool Skip( UInt8 type );

    /** Skips a field value: a bool field has no value, every other type is skipped */
    bool SkipField( UInt8 fieldType );

    /** A raw unsigned varint (ULEB128) of at most 10 bytes */
    bool ReadVarint( UInt64& value );

    /** Marks the input as not valid: for semantic errors found by the caller (a missing required field) */
    bool Fail( CORE::TDataTableStatus status, const char* message );

    inline UInt32 GetPosition( void ) const { return m_position; }
    inline UInt32 GetRemaining( void ) const { return m_size - m_position; }
    inline UInt32 GetDepth( void ) const { return m_depth; }
    inline bool HasFailed( void ) const { return CORE::DATATABLE_STATUS_OK != m_status; }
    inline CORE::TDataTableStatus GetStatus( void ) const { return m_status; }

    /** The failure message, with the byte position where it happened; empty when nothing failed */
    CORE::CString GetErrorMessage( void ) const;

    private:

    CThriftCompactReader( const CThriftCompactReader& src );                /**< not implemented */
    CThriftCompactReader& operator=( const CThriftCompactReader& src );     /**< not implemented */

    bool Enter( void );
    bool Leave( void );
    bool SkipElements( UInt8 elementType, UInt32 count );
    static UInt32 MinimumElementBytes( UInt8 type );

    const UInt8* m_data;
    UInt32 m_size;
    UInt32 m_position;
    UInt32 m_maxDepth;
    UInt32 m_depth;
    UInt32 m_structDepth;
    Int16 m_lastFieldId[ MaxSupportedDepth + 1 ];
    CORE::TDataTableStatus m_status;
    const char* m_error;
    UInt32 m_errorPosition;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PARQUET */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTREADER_H ? */
