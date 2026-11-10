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

#include "gucefCORE_CDataTableRowGroupInfo.h"

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

CDataTableRowGroupInfo::CDataTableRowGroupInfo( void )
    : rowCount( 0 )
    , columnStats()
    , sortedOn()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDataTableRowGroupInfo::Clear( void )
{GUCEF_TRACE;

    rowCount = 0;
    columnStats.clear();
    sortedOn.clear();
}

/*-------------------------------------------------------------------------*/

const CDataTableColumnStats*
CDataTableRowGroupInfo::GetColumnStats( UInt32 fileColumnIndex ) const
{GUCEF_TRACE;

    if ( fileColumnIndex >= (UInt32) columnStats.size() )
        return GUCEF_NULL;
    const CDataTableColumnStats& stats = columnStats[ fileColumnIndex ];
    if ( !stats.hasMinMax && !stats.hasNullCount )
        return GUCEF_NULL;
    return &stats;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableRowGroupInfo::operator==( const CDataTableRowGroupInfo& other ) const
{GUCEF_TRACE;

    return rowCount == other.rowCount && columnStats == other.columnStats && sortedOn == other.sortedOn;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
