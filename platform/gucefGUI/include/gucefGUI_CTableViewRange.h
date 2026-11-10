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

#ifndef GUCEF_GUI_CTABLEVIEWRANGE_H
#define GUCEF_GUI_CTABLEVIEWRANGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A block of cells of a table: rows [firstRow, firstRow + rowCount) of columns [firstColumn, firstColumn + columnCount),
 *  in the indices of the model. The data of the change events of a table view model, and the window a view announces
 *  to the model before it reads it.
 */
class GUCEF_GUI_PUBLIC_CPP CTableViewRange
{
    public:

    UInt32 firstRow;
    UInt32 rowCount;
    UInt32 firstColumn;
    UInt32 columnCount;

    CTableViewRange( void );

    CTableViewRange( UInt32 firstRowOfRange, UInt32 rowsInRange, UInt32 firstColumnOfRange, UInt32 columnsInRange );

    bool IsEmpty( void ) const;

    /** One past the last row and the last column of the range */
    UInt32 EndRow( void ) const;

    UInt32 EndColumn( void ) const;

    bool Contains( UInt32 row, UInt32 column ) const;

    bool operator==( const CTableViewRange& other ) const;

    bool operator!=( const CTableViewRange& other ) const;
};

/** The event data that carries a range */
typedef CORE::CTCloneableObj< CTableViewRange > TTableViewRangeEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTABLEVIEWRANGE_H ? */
