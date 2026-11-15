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

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLRESULT_H
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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

struct pg_result;

namespace GUCEF {
namespace DBPLUGIN {
namespace POSTGRESQL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Owns a result of libpq (PGresult) and frees it when it goes or is replaced. The values stay valid while it is held;
 *  readable from several threads at once.
 */
class DBPLUGIN_POSTGRESQL_PUBLIC_CPP CPostgreSqlResult
{
    public:

    CPostgreSqlResult( void );

    ~CPostgreSqlResult();

    /** Takes the result over, freeing the one held */
    void Reset( struct pg_result* result = GUCEF_NULL );

    struct pg_result* Get( void ) const;

    UInt32 GetRowCount( void ) const;

    UInt32 GetColumnCount( void ) const;

    bool IsNull( UInt32 row, UInt32 column ) const;

    /** The value as text (zero terminated) */
    const char* GetValue( UInt32 row, UInt32 column ) const;

    UInt32 GetLength( UInt32 row, UInt32 column ) const;

    /** The rows an INSERT, UPDATE or DELETE changed */
    UInt64 GetAffectedRowCount( void ) const;

    private:

    CPostgreSqlResult( const CPostgreSqlResult& src );              /**< not implemented */
    CPostgreSqlResult& operator=( const CPostgreSqlResult& src );   /**< not implemented */

    struct pg_result* m_result;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CPostgreSqlResult, MT::CMutex > CPostgreSqlResultPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLRESULT_H ? */
