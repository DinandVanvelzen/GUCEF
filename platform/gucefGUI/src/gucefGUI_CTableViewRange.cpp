/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#include "gucefGUI_CTableViewRange.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTableViewRange::CTableViewRange( void )
    : firstRow( 0 )
    , rowCount( 0 )
    , firstColumn( 0 )
    , columnCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTableViewRange::CTableViewRange( UInt32 firstRowOfRange, UInt32 rowsInRange, UInt32 firstColumnOfRange, UInt32 columnsInRange )
    : firstRow( firstRowOfRange )
    , rowCount( rowsInRange )
    , firstColumn( firstColumnOfRange )
    , columnCount( columnsInRange )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CTableViewRange::IsEmpty( void ) const
{GUCEF_TRACE;

    return 0 == rowCount || 0 == columnCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CTableViewRange::EndRow( void ) const
{GUCEF_TRACE;

    return firstRow + rowCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CTableViewRange::EndColumn( void ) const
{GUCEF_TRACE;

    return firstColumn + columnCount;
}

/*-------------------------------------------------------------------------*/

bool
CTableViewRange::Contains( UInt32 row, UInt32 column ) const
{GUCEF_TRACE;

    return row >= firstRow && row - firstRow < rowCount && column >= firstColumn && column - firstColumn < columnCount;
}

/*-------------------------------------------------------------------------*/

bool
CTableViewRange::operator==( const CTableViewRange& other ) const
{GUCEF_TRACE;

    return firstRow == other.firstRow && rowCount == other.rowCount && firstColumn == other.firstColumn && columnCount == other.columnCount;
}

/*-------------------------------------------------------------------------*/

bool
CTableViewRange::operator!=( const CTableViewRange& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
