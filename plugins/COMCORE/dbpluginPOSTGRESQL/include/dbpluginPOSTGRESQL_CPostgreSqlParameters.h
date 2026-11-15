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

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLPARAMETERS_H
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLPARAMETERS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef DBPLUGIN_POSTGRESQL_MACROS_H
#include "dbpluginPOSTGRESQL_macros.h"
#define DBPLUGIN_POSTGRESQL_MACROS_H
#endif /* DBPLUGIN_POSTGRESQL_MACROS_H ? */

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
 *  The parameters $1, $2, ... of a statement, as PQexecParams() takes them: text values (the text form PostgreSQL
 *  reads, cast in the statement to the type of the column), binary values (bytea) and nulls. Values are never put into
 *  the text of a statement. The bytes of all values share one buffer.
 */
class DBPLUGIN_POSTGRESQL_PUBLIC_CPP CPostgreSqlParameters
{
    public:

    /** PostgreSQL takes at most 65535 parameters in one statement */
    static const UInt32 MaxCount = 65535;

    CPostgreSqlParameters( void );

    /** Adds a text value; the number of the parameter ($n), 0 when the statement is full or the text holds a NUL byte */
    UInt32 AddText( const char* text, UInt32 byteLength );
    UInt32 AddText( const CORE::CString& text );

    /** Adds a binary value (sent in the binary format, for bytea); 0 when the statement is full */
    UInt32 AddBinary( const void* bytes, UInt32 byteLength );

    /** Adds a null; 0 when the statement is full */
    UInt32 AddNull( void );

    UInt32 GetCount( void ) const;

    /** The value of parameter n (1 based) as text, for tests: binary values as lower case hex, nulls as "NULL" */
    CORE::CString GetDisplayText( UInt32 number ) const;

    bool IsNull( UInt32 number ) const;

    bool IsBinary( UInt32 number ) const;

    void Clear( void );

    /** The arrays of PQexecParams(), valid until the next change */
    const char* const* GetValues( void ) const;
    const int* GetLengths( void ) const;
    const int* GetFormats( void ) const;

    private:

    enum EKind
    {
        KIND_TEXT = 0   ,
        KIND_BINARY     ,
        KIND_NULL
    };

    UInt32 Add( EKind kind, const void* bytes, UInt32 byteLength );
    void Prepare( void ) const;

    GUCEF::vector< char > m_bytes;                  /**< all values, text ones followed by a zero byte */
    GUCEF::vector< UInt32 > m_offsets;
    GUCEF::vector< int > m_lengths;
    GUCEF::vector< int > m_formats;
    GUCEF::vector< UInt8 > m_kinds;
    mutable GUCEF::vector< const char* > m_values;  /**< pointers into m_bytes, made when asked for */
    mutable bool m_prepared;
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

#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLPARAMETERS_H ? */
