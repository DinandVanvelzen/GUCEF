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

#include <algorithm>

#include "gucefGUI_CTableEditHistory.h"

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

CTableEditHistory::CTableEditHistory( void )
    : m_undo()
    , m_redo()
    , m_group()
    , m_groupDepth( 0 )
    , m_maxSteps( 1000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTableEditHistory::SetMaxSteps( UInt32 steps )
{GUCEF_TRACE;

    m_maxSteps = steps > 0 ? steps : 1;
    Trim();
}

/*-------------------------------------------------------------------------*/

UInt32
CTableEditHistory::GetMaxSteps( void ) const
{GUCEF_TRACE;

    return m_maxSteps;
}

/*-------------------------------------------------------------------------*/

void
CTableEditHistory::Trim( void )
{GUCEF_TRACE;

    if ( m_undo.size() > m_maxSteps )
        m_undo.erase( m_undo.begin(), m_undo.begin() + ( m_undo.size() - m_maxSteps ) );
    if ( m_redo.size() > m_maxSteps )
        m_redo.erase( m_redo.begin(), m_redo.begin() + ( m_redo.size() - m_maxSteps ) );
}

/*-------------------------------------------------------------------------*/

void
CTableEditHistory::BeginGroup( void )
{GUCEF_TRACE;

    ++m_groupDepth;
}

/*-------------------------------------------------------------------------*/

void
CTableEditHistory::EndGroup( void )
{GUCEF_TRACE;

    if ( 0 == m_groupDepth )
        return;
    if ( 0 == --m_groupDepth && !m_group.empty() )
    {
        m_undo.push_back( TEditVector() );
        m_undo.back().swap( m_group );
        Trim();
    }
}

/*-------------------------------------------------------------------------*/

void
CTableEditHistory::Record( const CORE::CVariant& rowKey, UInt32 column, const CORE::CVariant& oldValue, const CORE::CVariant& newValue )
{GUCEF_TRACE;

    SEdit edit;
    edit.rowKey = rowKey;
    edit.column = column;
    edit.oldValue = oldValue;
    edit.newValue = newValue;
    m_redo.clear();
    if ( m_groupDepth > 0 )
    {
        m_group.push_back( edit );
        return;
    }
    m_undo.push_back( TEditVector( 1, edit ) );
    Trim();
}

/*-------------------------------------------------------------------------*/

bool
CTableEditHistory::CanUndo( void ) const
{GUCEF_TRACE;

    return 0 == m_groupDepth && !m_undo.empty();
}

/*-------------------------------------------------------------------------*/

bool
CTableEditHistory::CanRedo( void ) const
{GUCEF_TRACE;

    return 0 == m_groupDepth && !m_redo.empty();
}

/*-------------------------------------------------------------------------*/

UInt32
CTableEditHistory::GetUndoStepCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_undo.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CTableEditHistory::GetRedoStepCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_redo.size();
}

/*-------------------------------------------------------------------------*/

bool
CTableEditHistory::TakeUndo( TEditVector& edits )
{GUCEF_TRACE;

    edits.clear();
    if ( !CanUndo() )
        return false;
    TEditVector step;
    step.swap( m_undo.back() );
    m_undo.pop_back();
    edits = step;
    std::reverse( edits.begin(), edits.end() );
    m_redo.push_back( TEditVector() );
    m_redo.back().swap( step );
    Trim();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTableEditHistory::TakeRedo( TEditVector& edits )
{GUCEF_TRACE;

    edits.clear();
    if ( !CanRedo() )
        return false;
    TEditVector step;
    step.swap( m_redo.back() );
    m_redo.pop_back();
    edits = step;
    m_undo.push_back( TEditVector() );
    m_undo.back().swap( step );
    Trim();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTableEditHistory::Clear( void )
{GUCEF_TRACE;

    m_undo.clear();
    m_redo.clear();
    m_group.clear();
    m_groupDepth = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
