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

#ifndef TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTWRITER_H
#define TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTREADER_H
#include "tablecodecpluginPARQUET_CThriftCompactReader.h"
#define TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTREADER_H
#endif /* TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTREADER_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Writes the Thrift compact protocol, appending to a buffer. Field headers use the short form for id deltas of 1 to 15
 *  and the long form otherwise; lists use the short header for up to 14 elements.
 *  Structs nest up to CThriftCompactReader::MaxSupportedDepth; deeper nesting is a usage error that IsValid() reports.
 */
class TABLECODECPLUGIN_PARQUET_PUBLIC_CPP CThriftCompactWriter
{
    public:

    explicit CThriftCompactWriter( CORE::CDynamicBuffer& output );

    void BeginStruct( void );
    void EndStruct( void );                     /**< writes the stop field */

    void WriteFieldHeader( UInt8 fieldType, Int16 fieldId );

    void WriteBoolField( Int16 fieldId, bool value );
    void WriteI8Field( Int16 fieldId, Int8 value );
    void WriteI16Field( Int16 fieldId, Int16 value );
    void WriteI32Field( Int16 fieldId, Int32 value );
    void WriteI64Field( Int16 fieldId, Int64 value );
    void WriteDoubleField( Int16 fieldId, Float64 value );
    void WriteBinaryField( Int16 fieldId, const void* bytes, UInt32 length );

    /** A field holding a struct: the header, then BeginStruct(); end it with EndStruct() */
    void BeginStructField( Int16 fieldId );

    /** A field holding a list: the header and the list header; the elements follow */
    void BeginListField( Int16 fieldId, UInt8 elementType, UInt32 size );

    void WriteListHeader( UInt8 elementType, UInt32 size );
    void WriteBoolElement( bool value );
    void WriteI8( Int8 value );
    void WriteI16( Int16 value );
    void WriteI32( Int32 value );
    void WriteI64( Int64 value );
    void WriteDouble( Float64 value );
    void WriteBinary( const void* bytes, UInt32 length );
    void WriteVarint( UInt64 value );

    /** Appends bytes that already are an encoded value (a struct encoded on its own) */
    void WriteRaw( const void* bytes, UInt32 length );

    /**
     *  The next count field headers are written in the long form (the type byte and the zigzag varint id) even when the
     *  short form fits: a valid encoding that every compact protocol reader takes, one byte longer per field for ids up to
     *  63. Writers use it to give a structure a length they need, such as page headers that keep page values aligned.
     */
    inline void SetLongFormFieldCount( UInt32 count ) { m_longFormFields = count; }

    /** False after nesting deeper than supported or ending a struct that was not begun */
    inline bool IsValid( void ) const { return m_valid; }

    inline CORE::CDynamicBuffer& GetOutput( void ) { return m_output; }

    private:

    CThriftCompactWriter( const CThriftCompactWriter& src );                /**< not implemented */
    CThriftCompactWriter& operator=( const CThriftCompactWriter& src );     /**< not implemented */

    void WriteByte( UInt8 value );

    CORE::CDynamicBuffer& m_output;
    UInt32 m_structDepth;
    Int16 m_lastFieldId[ CThriftCompactReader::MaxSupportedDepth + 1 ];
    UInt32 m_longFormFields;
    bool m_valid;
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

#endif /* TABLECODECPLUGIN_PARQUET_CTHRIFTCOMPACTWRITER_H ? */
