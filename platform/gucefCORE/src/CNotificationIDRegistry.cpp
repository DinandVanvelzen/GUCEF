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

#include <limits.h>

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CMSGEXCEPTION_H
#include "CMsgException.h"
#define GUCEF_CORE_CMSGEXCEPTION_H
#endif /* GUCEF_CORE_CMSGEXCEPTION_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

#include "CNotificationIDRegistry.h"

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

GUCEF_IMPLEMENT_MSGEXCEPTION( CNotificationIDRegistry, EKeyAlreadyRegistered );
GUCEF_IMPLEMENT_MSGEXCEPTION( CNotificationIDRegistry, EEmptyKeyString );
GUCEF_IMPLEMENT_MSGEXCEPTION( CNotificationIDRegistry, EUnknownKey );
GUCEF_IMPLEMENT_MSGEXCEPTION( CNotificationIDRegistry, EInvalidKey );
GUCEF_IMPLEMENT_MSGEXCEPTION( CNotificationIDRegistry, EInvalidEventID );

/*-------------------------------------------------------------------------*/

CNotificationIDRegistry::CNotificationIDRegistry( void )
    : m_lastid( 1 )
{GUCEF_TRACE;

    /* nothing to do here */
}

/*-------------------------------------------------------------------------*/

CNotificationIDRegistry::~CNotificationIDRegistry()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_list.clear();
}

/*-------------------------------------------------------------------------*/

CEvent
CNotificationIDRegistry::Register( const CString& keyvalue                       ,
                                   const bool okIfAlreadyRegisterd /* = false */ )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    if ( keyvalue.Length() > 0 )
    {
        TRegistryList::const_iterator i = m_list.find( keyvalue );
        if ( i == m_list.end() )
        {
            if ( (GUCEFCORE_UINT32MAX-1) > m_lastid )
            {
                m_list[ keyvalue ] = m_lastid;
                ++m_lastid;

                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Event registered with ID " + UInt32ToString( m_lastid-1 ) + " and name \"" + keyvalue + "\"" );

                return CEvent( m_lastid-1, keyvalue );
            }

            /*
             *  Because all event id's have been used at least once we
             *  now have to perform a linear search for available event id's
             *  In practice it is rather unlikely this code will ever be used due to the insane
             *  amount of event registration actions required.
             */
            for ( UInt32 n=0; n<(GUCEF_UINT32MAX-1); ++n )
            {
                bool found = false;
                i = m_list.begin();
                while ( i != m_list.end() )
                {
                    if ( (*i).second == n )
                    {
                        found = true;
                        break;
                    }
                    ++i;
                }
                if ( !found )
                {
                    m_list[ keyvalue ] = n;
                    return CEvent( n, keyvalue );
                }
            }
        }

        if ( !okIfAlreadyRegisterd )
        {
            GUCEF_EMSGTHROW( EKeyAlreadyRegistered, "CNotificationIDRegistry: Key is already registerd" );
        }
        return CEvent( (*i).second, keyvalue );
    }
    else
    {
        GUCEF_EMSGTHROW( EEmptyKeyString, "CNotificationIDRegistry: Empty key string" );
    }
}

/*-------------------------------------------------------------------------*/

void
CNotificationIDRegistry::Unregister( const CString& keyvalue                      ,
                                     const bool okIfUnknownKeyGiven /* = false */ )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    if ( keyvalue.Length() > 0 )
    {
        TRegistryList::iterator i = m_list.find( keyvalue );
        if ( i != m_list.end() )
        {
            m_list.erase( i );
        }
        else
        {
            GUCEF_EMSGTHROW( EUnknownKey, "CNotificationIDRegistry::Unregister(): unknown notification key string identifier" );
        }
    }
    else
    {
        GUCEF_EMSGTHROW( EUnknownKey, "CNotificationIDRegistry::Unregister(): invalid notification key string identifier" );
    }
}

/*-------------------------------------------------------------------------*/

CEvent
CNotificationIDRegistry::Lookup( const CString& keyvalue     ,
                                 const bool registerUnknown  )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    TRegistryList::iterator i = m_list.find( keyvalue );
    if ( i != m_list.end() )
    {
        return CEvent( (*i).second, keyvalue );
    }

    if ( registerUnknown )
    {
        Register( keyvalue );
        return Lookup( keyvalue );
    }

    GUCEF_EMSGTHROW( EInvalidKey, "CNotificationIDRegistry::Lookup(): invalid notification key identifier" );
}

/*-------------------------------------------------------------------------*/

CString
CNotificationIDRegistry::Lookup( const CEvent& eventID ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    UInt32 id( eventID.GetID() );

    if ( id > 0 )
    {
        TRegistryList::const_iterator i = m_list.begin();
        while ( i != m_list.end() )
        {
            if ( (*i).second == id )
            {
                return (*i).first;
            }
            ++i;
        }
    }

    GUCEF_EMSGTHROW( EInvalidEventID, "CNotificationIDRegistry::Lookup(): invalid eventID" );
}

/*-------------------------------------------------------------------------*/

bool
CNotificationIDRegistry::IsRegistered( const CString& keyvalue ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    TRegistryList::const_iterator i = m_list.find( keyvalue );
    bool found = i != m_list.end();

    return found;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CNotificationIDRegistry::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CNotificationIDRegistry::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_dataLock.Lock();
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CNotificationIDRegistry::Unlock( void ) const
{GUCEF_TRACE;

    return m_dataLock.Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
