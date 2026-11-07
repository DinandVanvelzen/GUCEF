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
#ifndef GUCEF_GUI_CTABLEEDITHISTORY_H
#define GUCEF_GUI_CTABLEEDITHISTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

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
 *  The undo and redo steps of a table view model, for a model that wants them (the grid only asks the model, see
 *  CITableViewModel::CanUndo()). The model records every edit it applies with the key of the row, so the history
 *  stays right when rows are inserted, removed or sorted; edits recorded between BeginGroup() and EndGroup() are one
 *  step, so a paste or a fill is undone in one go. Undo hands the edits of the last step back, newest first, each with
 *  the value to put back; redo hands them in the order they were made. A new edit ends the redo steps.
 *
 *  The history is bounded (SetMaxSteps(), 1000 by default, the oldest go first). Not thread safe: a model is used
 *  from one thread.
 */
class GUCEF_GUI_PUBLIC_CPP CTableEditHistory
{
    public:

    /** One edit of one cell */
    struct SEdit
    {
        CORE::CVariant rowKey;
        UInt32 column;
        CORE::CVariant oldValue;
        CORE::CVariant newValue;

        SEdit( void ) : rowKey(), column( 0 ), oldValue(), newValue() {}
    };
    typedef GUCEF::vector< SEdit > TEditVector;

    CTableEditHistory( void );

    /** The most steps kept; at least 1 */
    void SetMaxSteps( UInt32 steps );

    UInt32 GetMaxSteps( void ) const;

    /** The edits until the matching EndGroup() are one step; calls nest */
    void BeginGroup( void );

    void EndGroup( void );

    /** Records an edit that was applied: a step of its own outside a group. Drops the redo steps */
    void Record( const CORE::CVariant& rowKey, UInt32 column, const CORE::CVariant& oldValue, const CORE::CVariant& newValue );

    bool CanUndo( void ) const;

    bool CanRedo( void ) const;

    UInt32 GetUndoStepCount( void ) const;

    UInt32 GetRedoStepCount( void ) const;

    /**
     *  Takes the last step: the edits of it, the last edit first. The model puts every oldValue back, and the step becomes a redo step.
     *  False when there is no step, or a group is open
     */
    bool TakeUndo( TEditVector& edits );

    /** Takes the last undone step: its edits in the order they were made. The model puts every newValue back; the step is an undo step again */
    bool TakeRedo( TEditVector& edits );

    void Clear( void );

    private:

    typedef GUCEF::vector< TEditVector > TStepVector;

    void Trim( void );

    TStepVector m_undo;
    TStepVector m_redo;
    TEditVector m_group;
    UInt32 m_groupDepth;
    UInt32 m_maxSteps;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTABLEEDITHISTORY_H ? */
