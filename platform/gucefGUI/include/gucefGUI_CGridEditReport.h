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
#ifndef GUCEF_GUI_CGRIDEDITREPORT_H
#define GUCEF_GUI_CGRIDEDITREPORT_H

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
 *  What an operation of a grid on its cells did, the data of the edit events of CGridView: the one cell of an edit, or
 *  the summary of a copy, cut, paste, fill or clear. 'row' and 'column' are the first cell concerned (the cell of
 *  an edit, the top left cell of a block), columns are columns of the data; -1 for none.
 */
class GUCEF_GUI_PUBLIC_CPP CGridEditReport
{
    public:

    enum EOperation
    {
        OPERATION_EDIT = 0  ,   /**< the editor of one cell */
        OPERATION_TOGGLE    ,   /**< a check box toggled */
        OPERATION_COPY      ,
        OPERATION_CUT       ,
        OPERATION_PASTE     ,
        OPERATION_FILL_DOWN ,
        OPERATION_FILL_RIGHT,
        OPERATION_CLEAR     ,
        OPERATION_UNDO      ,
        OPERATION_REDO
    };

    EOperation operation;
    Int32 row;
    Int32 column;
    UInt32 cellsApplied;        /**< cells the model took (for a copy: cells copied) */
    UInt32 cellsRefused;        /**< cells whose value did not parse or the model refused */
    UInt32 cellsReadOnly;       /**< cells the model says are read only */
    UInt32 cellsSkipped;        /**< cells outside the grid, or left out because they have nothing to copy from */
    bool refused;               /**< the operation as a whole did not happen; 'message' says why */
    CString message;            /**< the reason of a refusal; else the reason of the first cell that was refused */
    Int32 problemRow;           /**< the first cell that was refused or read only, -1 for none */
    Int32 problemColumn;

    CGridEditReport( void );

    explicit CGridEditReport( EOperation editOperation );

    /** The number of cells that were looked at */
    UInt32 GetCellCount( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDEDITREPORT_H ? */
