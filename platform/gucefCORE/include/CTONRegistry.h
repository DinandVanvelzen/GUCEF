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

#ifndef GUCEF_CORE_CTONREGISTRY_H
#define GUCEF_CORE_CTONREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTREGISTRY_H
#include "CTRegistry.h"
#define GUCEF_CORE_CTREGISTRY_H
#endif /* GUCEF_CORE_CTREGISTRY_H ? */

#ifndef GUCEF_CORE_CTOBSERVINGNOTIFIEREXPANSION_H
#include "CTObservingNotifierExpansion.h"
#define GUCEF_CORE_CTOBSERVINGNOTIFIEREXPANSION_H
#endif /* GUCEF_CORE_CTOBSERVINGNOTIFIEREXPANSION_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Interface class for the events emitted by CTONRegistry template
 *  instantiations.
 */
class GUCEF_CORE_PUBLIC_CPP CTONRegistryEvents
{
    public:

    static const CEvent ItemRegisteredEvent;
    static const CEvent ItemUnregisteredEvent;

    typedef TCloneableString ItemRegisteredEventData;
    typedef TCloneableString ItemUnregisteredEventData;

    static void RegisterEvents( void );

    public:

    CTONRegistryEvents( void );
    CTONRegistryEvents( const CTONRegistryEvents& src );
    virtual ~CTONRegistryEvents();
    CTONRegistryEvents& operator=( const CTONRegistryEvents& src );
};

/*-------------------------------------------------------------------------*/

/**
 *  Template that expands the normal CTRegistry functionality with
 *  notification.
 */
template < class T, class LockType >
class CTONRegistry : public CTObservingNotifierExpansion< CTRegistry< T, LockType > > ,
                     public CTONRegistryEvents /* <- interface */
{
    public:

    typedef LockType                                                                                    TLockType;
    typedef T                                                                                           TRegisteredType;
    typedef typename CTObservingNotifierExpansion< CTRegistry< T, LockType > >::TRegisteredObjPtr       TRegisteredObjPtr;
    typedef typename CTObservingNotifierExpansion< CTRegistry< T, LockType > >::TRegisteredObjPtrVector TRegisteredObjPtrVector;
    typedef CTObservingNotifierExpansion< CTRegistry< T, LockType > >                                   TExpansionBase;

    CTONRegistry( void );

    CTONRegistry( const CTONRegistry& src );

    virtual ~CTONRegistry();

    CTONRegistry& operator=( const CTONRegistry& src );

    virtual void Register( const CString& name                ,
                           const TRegisteredObjPtr& sharedPtr ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool TryRegister( const CString& name                ,
                              const TRegisteredObjPtr& sharedPtr ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Unregister( const CString& name ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void UnregisterAll( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< class T, class LockType >
CTONRegistry< T, LockType >::CTONRegistry( void )
    : CTObservingNotifierExpansion< CTRegistry< T, LockType > >()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
CTONRegistry< T, LockType >::CTONRegistry( const CTONRegistry& src )
    : CTObservingNotifierExpansion< CTRegistry< T, LockType > >( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
CTONRegistry< T, LockType >::~CTONRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
CTONRegistry< T, LockType >&
CTONRegistry< T, LockType >::operator=( const CTONRegistry& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CTObservingNotifierExpansion< CTRegistry< T, LockType > >::operator=( src );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
const CString&
CTONRegistry< T, LockType >::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::CORE::CTONRegistry< T, LockType >";
    return typeName;
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTONRegistry< T, LockType >::Register( const CString& name                ,
                                       const TRegisteredObjPtr& sharedPtr )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( AsLockable() );    

    try
    {
        CTObservingNotifierExpansion< CTRegistry< T, LockType > >::Register( name, sharedPtr );
        ItemRegisteredEventData eData( name );
        TExpansionBase::NotifyObservers( ItemRegisteredEvent, &eData );
    }
    catch ( typename TExpansionBase::EAlreadyRegistered& e )
    {
        throw e;
    }
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
bool
CTONRegistry< T, LockType >::TryRegister( const CString& name                ,
                                          const TRegisteredObjPtr& sharedPtr )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( AsLockable() );

    bool wasRegistered = CTObservingNotifierExpansion< CTRegistry< T, LockType > >::TryRegister( name, sharedPtr );
    if ( wasRegistered )
    {
        ItemRegisteredEventData eData( name );
        TExpansionBase::NotifyObservers( ItemRegisteredEvent, &eData );
    }

    return wasRegistered;
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTONRegistry< T, LockType >::Unregister( const CString& name )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( AsLockable() );

    CTObservingNotifierExpansion< CTRegistry< T, LockType > >::Unregister( name );
    ItemRegisteredEventData eData( name );
    TExpansionBase::NotifyObservers( ItemRegisteredEvent, &eData );
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTONRegistry< T, LockType >::UnregisterAll( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( AsLockable() );

    CString::StringVector list;
    this->GetRegisteredObjNames( list );

    for ( UInt32 i=0; i<list.size(); ++i )
    {
        Unregister( list[ i ] );
    }
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
const MT::CILockable* 
CTONRegistry< T, LockType >::AsLockable( void ) const
{GUCEF_TRACE;

    return TExpansionBase::AsLockable();
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
MT::TLockStatus
CTONRegistry< T, LockType >::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return TExpansionBase::Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
MT::TLockStatus 
CTONRegistry< T, LockType >::Unlock( void ) const
{GUCEF_TRACE;

    return TExpansionBase::Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTONREGISTRY_H  ? */
