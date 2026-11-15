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

#include "gucefGUI_CNavigationHistory.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CNavigationHistory::NavigatedEvent = "GUCEF::GUI::CNavigationHistory::NavigatedEvent";
const CORE::CEvent CNavigationHistory::HistoryChangedEvent = "GUCEF::GUI::CNavigationHistory::HistoryChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNavigationHistory::SEntry::SEntry( void )
    : location()
    , title()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNavigationHistory::SEntry::SEntry( const CString& entryLocation, const CString& entryTitle )
    : location( entryLocation )
    , title( entryTitle )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNavigationHistory::SNavigated::SNavigated( void )
    : location()
    , title()
    , index( -1 )
    , kind( NAVIGATION_VISIT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CNavigationHistory::RegisterEvents( void )
{GUCEF_TRACE;

    NavigatedEvent.Initialize();
    HistoryChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CNavigationHistory::CNavigationHistory( UInt32 maximumEntries )
    : CORE::CNotifier()
    , m_entries()
    , m_index( -1 )
    , m_maximumEntries( maximumEntries )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CNavigationHistory::~CNavigationHistory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CNavigationHistory::CutToMaximum( void )
{GUCEF_TRACE;

    if ( 0 == m_maximumEntries || (UInt32) m_entries.size() <= m_maximumEntries )
        return;
    const UInt32 drop = (UInt32) m_entries.size() - m_maximumEntries;
    m_entries.erase( m_entries.begin(), m_entries.begin() + drop );
    m_index -= (Int32) drop;
    if ( m_index < 0 )
        m_index = m_entries.empty() ? -1 : 0;
}

/*-------------------------------------------------------------------------*/

void
CNavigationHistory::Visit( const CString& location, const CString& title )
{GUCEF_TRACE;

    if ( m_index >= 0 && m_entries[ (UInt32) m_index ].location == location )
    {
        // the place shown: only its title may be new
        if ( !( m_entries[ (UInt32) m_index ].title == title ) && !title.IsNULLOrEmpty() )
        {
            m_entries[ (UInt32) m_index ].title = title;
            NotifyObservers( HistoryChangedEvent );
        }
        return;
    }
    // the places ahead are of another path now
    if ( m_index + 1 < (Int32) m_entries.size() )
        m_entries.erase( m_entries.begin() + ( m_index + 1 ), m_entries.end() );
    m_entries.push_back( TEntry( location, title ) );
    m_index = (Int32) m_entries.size() - 1;
    CutToMaximum();
    NotifyObservers( HistoryChangedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CNavigationHistory::Replace( const CString& location, const CString& title )
{GUCEF_TRACE;

    if ( m_index < 0 )
        return false;
    m_entries[ (UInt32) m_index ] = TEntry( location, title );
    NotifyObservers( HistoryChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNavigationHistory::CanGoBack( void ) const
{GUCEF_TRACE;

    return m_index > 0;
}

/*-------------------------------------------------------------------------*/

bool
CNavigationHistory::CanGoForward( void ) const
{GUCEF_TRACE;

    return m_index >= 0 && m_index + 1 < (Int32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

bool
CNavigationHistory::MoveTo( Int32 index, ENavigationKind kind )
{GUCEF_TRACE;

    if ( index < 0 || index >= (Int32) m_entries.size() || index == m_index )
        return false;
    m_index = index;
    TNavigatedEventData data;
    data.GetData().location = m_entries[ (UInt32) index ].location;
    data.GetData().title = m_entries[ (UInt32) index ].title;
    data.GetData().index = index;
    data.GetData().kind = kind;
    NotifyObservers( NavigatedEvent, &data );
    NotifyObservers( HistoryChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNavigationHistory::Back( void )
{GUCEF_TRACE;

    if ( !CanGoBack() )
        return false;
    return MoveTo( m_index - 1, NAVIGATION_BACK );
}

/*-------------------------------------------------------------------------*/

bool
CNavigationHistory::Forward( void )
{GUCEF_TRACE;

    if ( !CanGoForward() )
        return false;
    return MoveTo( m_index + 1, NAVIGATION_FORWARD );
}

/*-------------------------------------------------------------------------*/

bool
CNavigationHistory::GoToIndex( UInt32 index )
{GUCEF_TRACE;

    return MoveTo( (Int32) index, NAVIGATION_JUMP );
}

/*-------------------------------------------------------------------------*/

bool
CNavigationHistory::HasCurrent( void ) const
{GUCEF_TRACE;

    return m_index >= 0;
}

/*-------------------------------------------------------------------------*/

const CNavigationHistory::TEntry&
CNavigationHistory::GetCurrent( void ) const
{GUCEF_TRACE;

    static const TEntry empty;
    return m_index >= 0 ? m_entries[ (UInt32) m_index ] : empty;
}

/*-------------------------------------------------------------------------*/

Int32
CNavigationHistory::GetCurrentIndex( void ) const
{GUCEF_TRACE;

    return m_index;
}

/*-------------------------------------------------------------------------*/

UInt32
CNavigationHistory::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

const CNavigationHistory::TEntry&
CNavigationHistory::GetEntry( UInt32 index ) const
{GUCEF_TRACE;

    static const TEntry empty;
    return index < (UInt32) m_entries.size() ? m_entries[ index ] : empty;
}

/*-------------------------------------------------------------------------*/

CNavigationHistory::TEntryVector
CNavigationHistory::GetBackEntries( void ) const
{GUCEF_TRACE;

    TEntryVector entries;
    for ( Int32 i=m_index-1; i>=0; --i )
        entries.push_back( m_entries[ (UInt32) i ] );
    return entries;
}

/*-------------------------------------------------------------------------*/

CNavigationHistory::TEntryVector
CNavigationHistory::GetForwardEntries( void ) const
{GUCEF_TRACE;

    TEntryVector entries;
    if ( m_index < 0 )
        return entries;
    for ( UInt32 i=(UInt32)m_index+1; i<(UInt32)m_entries.size(); ++i )
        entries.push_back( m_entries[ i ] );
    return entries;
}

/*-------------------------------------------------------------------------*/

void
CNavigationHistory::Clear( void )
{GUCEF_TRACE;

    if ( m_entries.empty() )
        return;
    m_entries.clear();
    m_index = -1;
    NotifyObservers( HistoryChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CNavigationHistory::SetMaximumEntries( UInt32 maximumEntries )
{GUCEF_TRACE;

    m_maximumEntries = maximumEntries;
    const UInt32 before = (UInt32) m_entries.size();
    CutToMaximum();
    if ( before != (UInt32) m_entries.size() )
        NotifyObservers( HistoryChangedEvent );
}

/*-------------------------------------------------------------------------*/

UInt32
CNavigationHistory::GetMaximumEntries( void ) const
{GUCEF_TRACE;

    return m_maximumEntries;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
