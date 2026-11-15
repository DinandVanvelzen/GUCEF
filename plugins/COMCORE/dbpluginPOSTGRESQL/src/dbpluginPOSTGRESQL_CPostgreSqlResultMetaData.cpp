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

#include "dbpluginPOSTGRESQL_CPostgreSqlResultMetaData.h"

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

CPostgreSqlResultMetaData::CPostgreSqlResultMetaData( const CORE::CDataTableSchemaPtr& schema )
    : CORE::CITableFileMetaData()
    , m_schema( schema )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPostgreSqlResultMetaData::~CPostgreSqlResultMetaData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CDataTableSchemaPtr&
CPostgreSqlResultMetaData::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

UInt64
CPostgreSqlResultMetaData::GetRowCount( void ) const
{GUCEF_TRACE;

    return GUCEF_UINT64MAX;
}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlResultMetaData::GetRowGroupCount( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlResultMetaData::GetRowGroupInfo( UInt32 rowGroup, CORE::CDataTableRowGroupInfo& info ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

UInt64
CPostgreSqlResultMetaData::GetMemoryFootprint( void ) const
{GUCEF_TRACE;

    return sizeof( *this );
}

/*-------------------------------------------------------------------------*/

void
CPostgreSqlResultMetaData::GetKeyValueMetaData( CORE::CValueList& keyValues ) const
{GUCEF_TRACE;

    keyValues.Clear();
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
