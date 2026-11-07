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


#include "gucefCORE_CDataTableWriteResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableWriteResult::SPartitionResult::SPartitionResult( void )
    : path()
    , status( DATATABLE_STATUS_OK )
    , rowsWritten( 0 )
    , rowsReplaced( 0 )
    , rowsDeleted( 0 )
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableWriteResult::CDataTableWriteResult( void )
    : CICloneable()
    , partitions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableWriteResult::CDataTableWriteResult( const CDataTableWriteResult& src )
    : CICloneable( src )
    , partitions( src.partitions )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableWriteResult::~CDataTableWriteResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableWriteResult&
CDataTableWriteResult::operator=( const CDataTableWriteResult& src )
{GUCEF_TRACE;

    if ( &src != this )
        partitions = src.partitions;
    return *this;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableWriteResult::GetOverallStatus( void ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) partitions.size(); ++i )
    {
        if ( DATATABLE_STATUS_OK != partitions[ i ].status )
            return partitions[ i ].status;
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

UInt64
CDataTableWriteResult::GetTotalRowsWritten( void ) const
{GUCEF_TRACE;

    UInt64 total = 0;
    for ( UInt32 i=0; i<(UInt32) partitions.size(); ++i )
        total += partitions[ i ].rowsWritten;
    return total;
}

/*-------------------------------------------------------------------------*/

void
CDataTableWriteResult::Clear( void )
{GUCEF_TRACE;

    partitions.clear();
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDataTableWriteResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDataTableWriteResult( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
