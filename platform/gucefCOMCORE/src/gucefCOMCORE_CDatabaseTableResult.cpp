/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefCOMCORE_CDatabaseTableResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDatabaseTableResult::CDatabaseTableResult( void )
    : CORE::CICloneable()
    , table()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseTableResult::CDatabaseTableResult( const CORE::CIDataTablePtr& openedTable )
    : CORE::CICloneable()
    , table( openedTable )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseTableResult::CDatabaseTableResult( const CDatabaseTableResult& src )
    : CORE::CICloneable( src )
    , table( src.table )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseTableResult::~CDatabaseTableResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseTableResult&
CDatabaseTableResult::operator=( const CDatabaseTableResult& src )
{GUCEF_TRACE;

    if ( &src != this )
        table = src.table;
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CDatabaseTableResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDatabaseTableResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CDatabaseTableResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CDatabaseTableResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COMCORE::CDatabaseTableResult";
}

/*-------------------------------------------------------------------------*/

bool
CDatabaseTableResult::TryGetFromFuture( const CORE::CFutureResult& future, CORE::CIDataTablePtr& table )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return false;

    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() || CORE::TTaskStatus::TASKSTATUS_FINISHED != task->GetTaskStatus() )
        return false;

    const CDatabaseTableResult* data = CORE::CICloneable::CloneableAs< CDatabaseTableResult >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return false;

    table = data->table;
    return true;
}

/*-------------------------------------------------------------------------*/

CDatabaseTableNamesResult::CDatabaseTableNamesResult( void )
    : CORE::CICloneable()
    , tableNames()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseTableNamesResult::CDatabaseTableNamesResult( const CORE::CString::StringSet& names )
    : CORE::CICloneable()
    , tableNames( names )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseTableNamesResult::CDatabaseTableNamesResult( const CDatabaseTableNamesResult& src )
    : CORE::CICloneable( src )
    , tableNames( src.tableNames )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseTableNamesResult::~CDatabaseTableNamesResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseTableNamesResult&
CDatabaseTableNamesResult::operator=( const CDatabaseTableNamesResult& src )
{GUCEF_TRACE;

    if ( &src != this )
        tableNames = src.tableNames;
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CDatabaseTableNamesResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDatabaseTableNamesResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CDatabaseTableNamesResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CDatabaseTableNamesResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COMCORE::CDatabaseTableNamesResult";
}

/*-------------------------------------------------------------------------*/

bool
CDatabaseTableNamesResult::TryGetFromFuture( const CORE::CFutureResult& future, CORE::CString::StringSet& tableNames )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return false;

    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() || CORE::TTaskStatus::TASKSTATUS_FINISHED != task->GetTaskStatus() )
        return false;

    const CDatabaseTableNamesResult* data = CORE::CICloneable::CloneableAs< CDatabaseTableNamesResult >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return false;

    tableNames = data->tableNames;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
