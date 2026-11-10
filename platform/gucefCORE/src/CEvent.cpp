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

#ifndef GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#include "CNotificationIDRegistry.h"
#define GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#endif /* GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

#include "CEvent.h"

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

CEvent::CEvent( void ) GUCEF_NOEXCEPT
    : m_eventID( 0 ) ,
      m_eventName()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CEvent::CEvent( const char* eventName ) GUCEF_NOEXCEPT
    : m_eventID( 0 )           ,
      m_eventName( eventName )
{GUCEF_TRACE;

    // Wait for delayed initialization
}

/*-------------------------------------------------------------------------*/

CEvent::CEvent( const CORE::CString& eventName ) GUCEF_NOEXCEPT
    : m_eventID( 0 )           ,
      m_eventName( eventName )
{GUCEF_TRACE;

    // Wait for delayed initialization
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED

CEvent::CEvent( CEvent&& src ) GUCEF_NOEXCEPT
    : m_eventID( src.m_eventID.Load( MT::MEMORDER_RELAXED ) ) ,
      m_eventName( GUCEF_MOVE( src.m_eventName ) )
{GUCEF_TRACE;

    // reset source to default constructor values
    src.m_eventID.Store( 0, MT::MEMORDER_RELAXED );
}

#endif /* GUCEF_RVALUE_REFERENCES_SUPPORTED ? */

/*-------------------------------------------------------------------------*/

CEvent::CEvent( const UInt32 eventID     ,
                const CString& eventName ) GUCEF_NOEXCEPT
    : m_eventID( eventID )     ,
      m_eventName( eventName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEvent::CEvent( const CEvent& src ) GUCEF_NOEXCEPT
    : m_eventID( src.m_eventID.Load( MT::MEMORDER_RELAXED ) ) ,
      m_eventName( src.m_eventName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEvent::~CEvent()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEvent&
CEvent::operator=( const CEvent& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_eventName = src.m_eventName;
        m_eventID.Store( src.m_eventID.Load( MT::MEMORDER_RELAXED ), MT::MEMORDER_RELAXED );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CEvent&
CEvent::operator=( const CORE::CString& eventName )
{GUCEF_TRACE;

    if ( eventName != m_eventName )
    {
        m_eventName = eventName;
        m_eventID.Store( 0, MT::MEMORDER_RELAXED );    // <- Wait for delayed initialization
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CEvent&
CEvent::operator=( const char* eventName )
{GUCEF_TRACE;

    if ( eventName != m_eventName )
    {
        m_eventName = eventName;
        m_eventID.Store( 0, MT::MEMORDER_RELAXED );    // <- Wait for delayed initialization
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CEvent::operator==( const CEvent& other ) const
{GUCEF_TRACE;

    // An event that has a name resolves its id on first use, see EnsureInitialized()
    EnsureInitialized();
    other.EnsureInitialized();
    return m_eventID.Load( MT::MEMORDER_RELAXED ) == other.m_eventID.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

bool
CEvent::operator==( const CORE::CString& eventName ) const
{GUCEF_TRACE;

    return m_eventName == eventName;
}

/*-------------------------------------------------------------------------*/

bool
CEvent::operator==( const char* eventName ) const
{GUCEF_TRACE;

    return m_eventName == eventName;
}

/*-------------------------------------------------------------------------*/

bool
CEvent::operator!=( const CEvent& other ) const
{GUCEF_TRACE;

    // An event that has a name resolves its id on first use, see EnsureInitialized()
    EnsureInitialized();
    other.EnsureInitialized();
    return m_eventID.Load( MT::MEMORDER_RELAXED ) != other.m_eventID.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

bool
CEvent::operator<( const CEvent& other ) const
{GUCEF_TRACE;

    // An event that has a name resolves its id on first use, see EnsureInitialized()
    EnsureInitialized();
    other.EnsureInitialized();
    return m_eventID.Load( MT::MEMORDER_RELAXED ) < other.m_eventID.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt32
CEvent::GetID( void ) const
{GUCEF_TRACE;

    EnsureInitialized();
    return m_eventID.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEvent::GetName( void ) const
{GUCEF_TRACE;

    return m_eventName;
}

/*-------------------------------------------------------------------------*/

void
CEvent::Initialize( void ) const
{GUCEF_TRACE;

    // For performance reasons we don't perform the lookup again on a non-zero id
    // Threads that find the id missing at the same time each look it up and store the same value, an atomic one:
    // only the id is stored, the name is the key of the lookup and stays as it is, other threads may be reading it
    if ( 0 == m_eventID.Load( MT::MEMORDER_RELAXED ) )
    {
        /*
         *  In this rare occasion a const_cast is legit since we are preforming
         *  an action that has conceptually already occurred,.. initialization.
         *
         *  The fact that we cannot initialize objects right from the start
         *  while allowing event objects to be defined globally is an implementation level problem.
         */
        CEvent registered( CORE::CCoreGlobal::Instance()->GetNotificationIDRegistry().Lookup( m_eventName, true ) );
        const_cast< CEvent& >( *this ).m_eventID.Store( registered.m_eventID.Load( MT::MEMORDER_RELAXED ), MT::MEMORDER_RELAXED );
    }
}

/*-------------------------------------------------------------------------*/

void
CEvent::EnsureInitialized( void ) const
{GUCEF_TRACE;

    // Events are mostly static objects a module registers when it loads. When that registration runs before the static
    // event objects are constructed (the order of static initialization between source files is not defined) their
    // construction leaves them without an id again. An event that has a name can always find its id, so it does here.
    // If you hit the assert below then the event object was never given a name
    if ( 0 == m_eventID.Load( MT::MEMORDER_RELAXED ) && !m_eventName.IsNULLOrEmpty() )
        Initialize();
    assert( m_eventID.Load( MT::MEMORDER_RELAXED ) > 0 );
}

/*-------------------------------------------------------------------------*/

bool
CEvent::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_eventID.Load( MT::MEMORDER_RELAXED ) > 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
