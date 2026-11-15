/*
 *  dbpluginPOSTGRESQL: gucefCOMCORE database backend for PostgreSQL
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

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLECOLUMNBATCH_H
#include "gucefCORE_CDataTableColumnBatch.h"
#define GUCEF_CORE_CDATATABLECOLUMNBATCH_H
#endif /* GUCEF_CORE_CDATATABLECOLUMNBATCH_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CDATATABLECOLUMNDEF_H
#include "gucefCORE_CDataTableColumnDef.h"
#define GUCEF_CORE_CDATATABLECOLUMNDEF_H
#endif /* GUCEF_CORE_CDATATABLECOLUMNDEF_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLPARAMETERS_H
#include "dbpluginPOSTGRESQL_CPostgreSqlParameters.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLPARAMETERS_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLPARAMETERS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace POSTGRESQL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The values of the columns between the data table value model and PostgreSQL. A value goes to the server as a
 *  parameter in a text form that CPostgreSqlSql::MakeParameterExpression() casts or converts to the column type, and
 *  comes back as the text of CPostgreSqlSql::MakeReadExpression():
 *
 *      bool                    "true" / "false"                  read "t" / "f"
 *      integers                decimal digits
 *      float32, float64        the round trip text, "NaN", "Infinity", "-Infinity"
 *      decimal(p,s)            the decimal text of the mantissa at the scale ("-12.34")    read: the mantissa
 *      date                    the days since 1970-01-01                                    read: the same
 *      timestamp               the ticks of the column unit since the epoch                 read: the same
 *      utf8                    the UTF-8 bytes (PostgreSQL text holds no NUL byte)
 *      binary                  the bytes, in the binary format                              read: hexadecimal
 *
 *  Reentrant: static members only.
 */
class DBPLUGIN_POSTGRESQL_PUBLIC_CPP CPostgreSqlValues
{
    public:

    /** Adds the cell of a column batch as a parameter; 0 when it cannot (a NUL byte in text, a full statement) */
    static UInt32 AddCellParameter( const CORE::CDataTableColumnDef& column, const CORE::CDataTableColumnBatch& cells, UInt32 row, CPostgreSqlParameters& parameters );

    /**
     *  Adds a literal of a resolved predicate (the native form of the column, see CDataTableResolvedPredicate) as a
     *  parameter; 0 when it cannot be sent (a NUL byte in text, a full statement)
     */
    static UInt32 AddLiteralParameter( const CORE::CDataTableColumnDef& column, const CORE::CVariant& literal, CPostgreSqlParameters& parameters );

    /** Appends a value the server returned for the column to the column builder: OK, TYPE_MISMATCH or CORRUPT_DATA (text that is not of the type) */
    static CORE::TDataTableStatus AppendResultValue( const CORE::CDataTableColumnDef& column, const char* text, UInt32 byteLength, bool isNull, CORE::CDataTableColumnBuilder& target );

    /** The decimal text of a mantissa at a scale: (12345, 2) is "123.45", (-5, 3) is "-0.005" */
    static CORE::CString MantissaToDecimalText( Int64 mantissa, UInt8 scale );

    /** The text of a double that reads back as the same double, with the PostgreSQL words for NaN and the infinities */
    static CORE::CString Float64ToText( Float64 value );

    /** Strict whole numbers: an optional '-' (signed only) and digits, within the range of the type */
    static bool ParseInt64( const char* text, UInt32 byteLength, Int64& value );
    static bool ParseUInt64( const char* text, UInt32 byteLength, UInt64& value );

    /** A float of the text of the server: the round trip text, "NaN", "Infinity", "-Infinity" */
    static bool ParseFloat64( const char* text, UInt32 byteLength, Float64& value );

    /** Bytes of hexadecimal text, upper or lower case, an even number of digits */
    static bool DecodeHex( const char* text, UInt32 byteLength, GUCEF::vector< UInt8 >& bytes );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H ? */
