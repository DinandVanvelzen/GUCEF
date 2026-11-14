/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <assert.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCORE_CObserverSwitch.h"

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF { 
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CObserverSwitch::CObserverSwitch( void )
    : CObservingNotifier()  ,
      m_observerGroups()    ,
      m_activeGroupName()   ,
      m_activeGroup( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CObserverSwitch::CObserverSwitch( const CObserverSwitch& src )
    : CObservingNotifier( src ) ,
      m_observerGroups()        ,
      m_activeGroupName()       ,
      m_activeGroup( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CObserverSwitch::~CObserverSwitch()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CObserverSwitch& 
CObserverSwitch::operator=( const CObserverSwitch& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

void
CObserverSwitch::AddObserverToGroup( const CString& groupName ,
                                     CObserver& observer      )
{GUCEF_TRACE;

    assert( GUCEF_NULL != &observer );
    
    Lock();
    
    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CObserverSwitch(" + PointerToString( this ) + "): Adding observer " + observer.GetClassTypeName() + "(" + PointerToString( &observer ) + ") to group " + groupName );
    
    (m_observerGroups[ groupName ]).insert( &observer );
    
    // Make sure our shortcut pointer gets set if this is the first item in the
    // already active group
    if ( GUCEF_NULL == m_activeGroup )
    {
        if ( groupName == m_activeGroupName )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CObserverSwitch(" + PointerToString( this ) + "): Setting active group to " + groupName );
            m_activeGroup = &( (*m_observerGroups.find( groupName ) ).second );
        }
    }
    
    Unlock();
}

/*-------------------------------------------------------------------------*/

void
CObserverSwitch::RemoveObserverFromGroup( const CString& groupName ,
                                          CObserver& observer      )
{GUCEF_TRACE;

    assert( GUCEF_NULL != &observer );
    
    Lock();
    
    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CObserverSwitch(" + PointerToString( this ) + "): Removing observer " + observer.GetClassTypeName() + "(" + PointerToString( &observer ) + ") from group " + groupName );
    
    TObserverGroupMap::iterator i = m_observerGroups.find( groupName );
    if ( i != m_observerGroups.end() )
    {
        TObserverSet& set = (*i).second;
        set.erase( &observer );
        if ( set.size() == 0 )
        {
            m_observerGroups.erase( i );
        }
    }
    Unlock();
}
    
/*-------------------------------------------------------------------------*/

void 
CObserverSwitch::SetActiveGroup( const CString& groupName )
{GUCEF_TRACE;

    Lock();
    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CObserverSwitch(" + PointerToString( this ) + "): Setting active group to " + groupName );
    m_activeGroupName = groupName;
    TObserverGroupMap::iterator i = m_observerGroups.find( groupName );
    if ( i != m_observerGroups.end() )
    {
        m_activeGroup = &(*i).second;
    }
    else
    {
        m_activeGroup = GUCEF_NULL;
    }
    Unlock();
}

/*-------------------------------------------------------------------------*/

const CString& 
CObserverSwitch::GetActiveGroup( void ) const
{GUCEF_TRACE;

    return m_activeGroupName;
}

/*-------------------------------------------------------------------------*/

void
CObserverSwitch::OnNotify( CNotifier* notifier                 ,
                           const CEvent& eventid               ,
                           CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    Lock();

    GUCEF_DEBUG_LOG( LOGLEVEL_EVERYTHING, "CObserverSwitch(" + PointerToString( this ) + "): Dispatching event \"" + eventid.GetName() + "\" to group: " + m_activeGroupName );

    if ( GUCEF_NULL != m_activeGroup )
    {
        TObserverSet::iterator i = m_activeGroup->begin();
        while ( i != m_activeGroup->end() )
        {
            if ( !NotifySpecificObserver( *notifier ,
                                          *(*i)     ,
                                          eventid   ,
                                          eventdata ) )
            {
                Unlock();
                return;
            }
            ++i;
        }
    }
    Unlock();
}

/*-------------------------------------------------------------------------*/

const CString&
CObserverSwitch::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::CORE::CObserverSwitch";
    return typeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
