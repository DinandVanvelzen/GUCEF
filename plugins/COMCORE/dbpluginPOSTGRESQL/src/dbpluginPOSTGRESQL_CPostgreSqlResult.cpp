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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#include "libpq-fe.h"

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H
#include "dbpluginPOSTGRESQL_CPostgreSqlValues.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H ? */

#include "dbpluginPOSTGRESQL_CPostgreSqlResult.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPostgreSqlResult::CPostgreSqlResult( void )
    : m_result( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPostgreSqlResult::~CPostgreSqlResult()
{GUCEF_TRACE;

    Reset();
}

/*-------------------------------------------------------------------------*/

void
CPostgreSqlResult::Reset( struct pg_result* result )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_result && result != m_result )
        PQclear( m_result );
    m_result = result;
}

/*-------------------------------------------------------------------------*/

struct pg_result*
CPostgreSqlResult::Get( void ) const
{GUCEF_TRACE;

    return m_result;
}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlResult::GetRowCount( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL == m_result ? 0 : (UInt32) PQntuples( m_result );
}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlResult::GetColumnCount( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL == m_result ? 0 : (UInt32) PQnfields( m_result );
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlResult::IsNull( UInt32 row, UInt32 column ) const
{GUCEF_TRACE;

    return GUCEF_NULL == m_result || 0 != PQgetisnull( m_result, (int) row, (int) column );
}

/*-------------------------------------------------------------------------*/

const char*
CPostgreSqlResult::GetValue( UInt32 row, UInt32 column ) const
{GUCEF_TRACE;

    return GUCEF_NULL == m_result ? "" : PQgetvalue( m_result, (int) row, (int) column );
}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlResult::GetLength( UInt32 row, UInt32 column ) const
{GUCEF_TRACE;

    return GUCEF_NULL == m_result ? 0 : (UInt32) PQgetlength( m_result, (int) row, (int) column );
}

/*-------------------------------------------------------------------------*/

UInt64
CPostgreSqlResult::GetAffectedRowCount( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_result )
        return 0;
    const char* text = PQcmdTuples( m_result );
    UInt64 count = 0;
    if ( GUCEF_NULL == text || !CPostgreSqlValues::ParseUInt64( text, (UInt32) strlen( text ), count ) )
        return 0;
    return count;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
