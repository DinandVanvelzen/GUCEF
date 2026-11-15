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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefGUI_CRecentLocations.h"

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

const CORE::CEvent CRecentLocations::ChangedEvent = "GUCEF::GUI::CRecentLocations::ChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CRecentLocations::RegisterEvents( void )
{GUCEF_TRACE;

    ChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CRecentLocations::CRecentLocations( UInt32 maxCount )
    : CORE::CNotifier()
    , m_locations()
    , m_maxCount( maxCount > 0 ? maxCount : 1 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CRecentLocations::~CRecentLocations()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRecentLocations::SetMaxCount( UInt32 maxCount )
{GUCEF_TRACE;

    m_maxCount = maxCount > 0 ? maxCount : 1;
    if ( m_locations.size() > m_maxCount )
    {
        CutToMaxCount();
        NotifyObservers( ChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CRecentLocations::GetMaxCount( void ) const
{GUCEF_TRACE;

    return m_maxCount;
}

/*-------------------------------------------------------------------------*/

CString
CRecentLocations::Normalize( const CString& path )
{GUCEF_TRACE;

    CString result = path.ReplaceChar( '\\', '/' );
    while ( result.Length() > 1 && '/' == result[ result.Length()-1 ] )
        result = result.CutChars( 1, false );
    return result;
}

/*-------------------------------------------------------------------------*/

CString
CRecentLocations::GetShortName( const CString& path )
{GUCEF_TRACE;

    const CString normalized = Normalize( path );
    const Int32 slash = normalized.HasChar( '/', false );
    if ( slash < 0 || (UInt32) slash+1 >= normalized.Length() )
        return normalized;
    return normalized.SubstrFromRange( (UInt32) slash+1, normalized.Length() );
}

/*-------------------------------------------------------------------------*/

void
CRecentLocations::CutToMaxCount( void )
{GUCEF_TRACE;

    while ( m_locations.size() > m_maxCount )
        m_locations.pop_back();
}

/*-------------------------------------------------------------------------*/

bool
CRecentLocations::Add( const CString& path )
{GUCEF_TRACE;

    const CString location = Normalize( path );
    if ( location.IsNULLOrEmpty() )
        return false;
    if ( !m_locations.empty() && m_locations.front() == location )
        return true;

    CString::StringVector::iterator i = m_locations.begin();
    while ( i != m_locations.end() )
    {
        if ( (*i) == location )
        {
            m_locations.erase( i );
            break;
        }
        ++i;
    }
    m_locations.insert( m_locations.begin(), location );
    CutToMaxCount();
    NotifyObservers( ChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRecentLocations::Remove( const CString& path )
{GUCEF_TRACE;

    const CString location = Normalize( path );
    CString::StringVector::iterator i = m_locations.begin();
    while ( i != m_locations.end() )
    {
        if ( (*i) == location )
        {
            m_locations.erase( i );
            NotifyObservers( ChangedEvent );
            return true;
        }
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRecentLocations::Clear( void )
{GUCEF_TRACE;

    if ( !m_locations.empty() )
    {
        m_locations.clear();
        NotifyObservers( ChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CRecentLocations::Prune( void )
{GUCEF_TRACE;

    UInt32 removed = 0;
    CString::StringVector kept;
    for ( size_t i=0; i<m_locations.size(); ++i )
    {
        const CString native = CORE::RelativePath( m_locations[ i ] );
        if ( CORE::DirExists( native ) || CORE::FileExists( native ) )
            kept.push_back( m_locations[ i ] );
        else
            ++removed;
    }
    if ( removed > 0 )
    {
        m_locations = kept;
        NotifyObservers( ChangedEvent );
    }
    return removed;
}

/*-------------------------------------------------------------------------*/

UInt32
CRecentLocations::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_locations.size();
}

/*-------------------------------------------------------------------------*/

const CString&
CRecentLocations::Get( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_locations.size() ? m_locations[ index ] : CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CString::StringVector&
CRecentLocations::GetAll( void ) const
{GUCEF_TRACE;

    return m_locations;
}

/*-------------------------------------------------------------------------*/

bool
CRecentLocations::Contains( const CString& path ) const
{GUCEF_TRACE;

    const CString location = Normalize( path );
    for ( size_t i=0; i<m_locations.size(); ++i )
        if ( m_locations[ i ] == location )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRecentLocations::Save( CORE::CDataNode& section, const CString& name ) const
{GUCEF_TRACE;

    section.DelChild( name );
    return section.SetValuesOfChildByName( name, m_locations );
}

/*-------------------------------------------------------------------------*/

bool
CRecentLocations::Load( const CORE::CDataNode& section, const CString& name )
{GUCEF_TRACE;

    CString::StringVector stored;
    const bool found = section.GetValuesOfChildByName( name, stored );

    CString::StringVector loaded;
    for ( size_t i=0; i<stored.size() && loaded.size() < m_maxCount; ++i )
    {
        const CString location = Normalize( stored[ i ] );
        bool duplicate = false;
        for ( size_t l=0; l<loaded.size(); ++l )
            duplicate = duplicate || loaded[ l ] == location;
        if ( !location.IsNULLOrEmpty() && !duplicate )
            loaded.push_back( location );
    }
    const bool changed = loaded != m_locations;
    m_locations = loaded;
    Prune();
    if ( changed )
        NotifyObservers( ChangedEvent );
    return found;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
