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

#include "gucefGUI_CHelpHistory.h"

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

CHelpHistory::CHelpHistory( UInt32 maximumEntries )
    : m_entries()
    , m_index( -1 )
    , m_maximumEntries( maximumEntries )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CHelpHistory::Visit( const CString& topicId )
{GUCEF_TRACE;

    if ( topicId.IsNULLOrEmpty() )
        return;
    if ( m_index >= 0 && m_entries[ (size_t) m_index ] == topicId )
        return;
    // the topics ahead of the current one go
    m_entries.resize( (size_t) ( m_index + 1 ) );
    m_entries.push_back( topicId );
    m_index = (Int32) m_entries.size() - 1;
    Trim();
}

/*-------------------------------------------------------------------------*/

bool
CHelpHistory::CanGoBack( void ) const
{GUCEF_TRACE;

    return m_index > 0;
}

/*-------------------------------------------------------------------------*/

bool
CHelpHistory::CanGoForward( void ) const
{GUCEF_TRACE;

    return m_index >= 0 && (size_t) ( m_index + 1 ) < m_entries.size();
}

/*-------------------------------------------------------------------------*/

bool
CHelpHistory::Back( void )
{GUCEF_TRACE;

    if ( !CanGoBack() )
        return false;
    --m_index;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHelpHistory::Forward( void )
{GUCEF_TRACE;

    if ( !CanGoForward() )
        return false;
    ++m_index;
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CHelpHistory::GetCurrent( void ) const
{GUCEF_TRACE;

    if ( m_index < 0 || (size_t) m_index >= m_entries.size() )
        return CString::Empty;
    return m_entries[ (size_t) m_index ];
}

/*-------------------------------------------------------------------------*/

void
CHelpHistory::Clear( void )
{GUCEF_TRACE;

    m_entries.clear();
    m_index = -1;
}

/*-------------------------------------------------------------------------*/

UInt32
CHelpHistory::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

Int32
CHelpHistory::GetIndex( void ) const
{GUCEF_TRACE;

    return m_index;
}

/*-------------------------------------------------------------------------*/

const CHelpHistory::TStringVector&
CHelpHistory::GetEntries( void ) const
{GUCEF_TRACE;

    return m_entries;
}

/*-------------------------------------------------------------------------*/

void
CHelpHistory::SetMaximumEntries( UInt32 maximumEntries )
{GUCEF_TRACE;

    m_maximumEntries = maximumEntries;
    Trim();
}

/*-------------------------------------------------------------------------*/

UInt32
CHelpHistory::GetMaximumEntries( void ) const
{GUCEF_TRACE;

    return m_maximumEntries;
}

/*-------------------------------------------------------------------------*/

void
CHelpHistory::Trim( void )
{GUCEF_TRACE;

    if ( 0 == m_maximumEntries || m_entries.size() <= (size_t) m_maximumEntries )
        return;
    const size_t drop = m_entries.size() - (size_t) m_maximumEntries;
    m_entries.erase( m_entries.begin(), m_entries.begin() + (Int32) drop );
    m_index -= (Int32) drop;
    if ( m_index < 0 )
        m_index = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
