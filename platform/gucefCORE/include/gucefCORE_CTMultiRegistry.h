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

#ifndef GUCEF_CORE_CTMULTIREGISTRY_H
#define GUCEF_CORE_CTMULTIREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTREGISTRY_H
#include "CTRegistry.h"
#define GUCEF_CORE_CTREGISTRY_H
#endif /* GUCEF_CORE_CTREGISTRY_H ? */

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
 *  The Multi Registry provides a mechanism for storing data centrally in a well
 *  managed fashion, adding named groups via sub-registries, helping to prevent object management chaos.
 *  The usage of shared pointers will ensure safe fire-and-forget usage
 *  where object access is concerned
 */
template < class T, class LockType >
class CTMultiRegistry : public MT::CILockable
{
    public:

    typedef LockType                                                TLockType;
    typedef T                                                       TRegistryType;
    typedef CTRegistry< T, LockType >                               TSubRegistry;
    typedef CTSharedPtr< CTRegistry< T, LockType >, LockType >      TSubRegistryPtr;
    typedef typename CTRegistry< T, LockType >::TRegisteredObjPtr   TRegisteredObjPtr;
    typedef typename CTRegistry< T, LockType >::TStringList         TStringList;
    typedef typename TSubRegistry::EAlreadyRegistered               EAlreadyRegistered;
    typedef typename TSubRegistry::EUnregisteredName                EUnregisteredName;

    CTMultiRegistry( void );

    CTMultiRegistry( const CTMultiRegistry& src );

    virtual ~CTMultiRegistry();

    CTMultiRegistry& operator=( const CTMultiRegistry& src );

    /**
     *  Check whether a registry of the given type/name exists
     */
    virtual bool IsRegistered( const CString& registryName ) const;

    /**
     *  Check whether the named entry exists in the given registry
     */
    virtual bool IsRegistered( const CString& registryName  ,
                               const CString& registryEntry ) const;

    virtual TRegisteredObjPtr Lookup( const CString& registryName  ,
                                      const CString& registryEntry ) const;

    virtual bool TryLookup( const CString& registryName   ,
                            const CString& registryEntry  ,
                            TRegisteredObjPtr& locatedObj ,
                            bool caseSensitive = true     ) const;

    virtual void Register( const CString& registryName        ,
                           const CString& registryEntry       ,
                           const TRegisteredObjPtr& sharedPtr );

    virtual bool TryRegister( const CString& registryName        ,
                              const CString& registryEntry       ,
                              const TRegisteredObjPtr& sharedPtr );

    virtual void Unregister( const CString& registryName  ,
                             const CString& registryEntry );

    virtual bool TryUnregister( const CString& registryName  ,
                                const CString& registryEntry );

    virtual void UnregisterAll( void );

    virtual UInt32 GetCount( void ) const;

    virtual UInt32 GetCount( const CString& registryName ) const;

    virtual void GetList( TStringList& destList ) const;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< CString, TSubRegistryPtr > TRegistryList;

    TRegistryList m_list;
    LockType m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< class T, class LockType >
CTMultiRegistry< T, LockType >::CTMultiRegistry( void )
    : m_list()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
CTMultiRegistry< T, LockType >::CTMultiRegistry( const CTMultiRegistry& src )
    : m_list()
    , m_lock()
{GUCEF_TRACE;


    MT::CObjectScopeReadOnlyLock lock( src.m_lock );
    typename TRegistryList::const_iterator i = src.m_list.begin();
    while ( i != src.m_list.end() )
    {
        m_list[ (*i).first ] = TSubRegistryPtr( GUCEF_NEW CTRegistry< T, LockType >( *( (*i).second ) ) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
CTMultiRegistry< T, LockType >::~CTMultiRegistry()
{GUCEF_TRACE;

    UnregisterAll();
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
CTMultiRegistry< T, LockType >&
CTMultiRegistry< T, LockType >::operator=( const CTMultiRegistry& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        MT::CObjectScopeLock lock( m_lock );

        UnregisterAll();

        typename TRegistryList::const_iterator i = src.m_list.begin();
        while ( i != src.m_list.end() )
        {
            m_list[ (*i).first ] = GUCEF_NEW TSubRegistryPtr( *( (*i).second ) );
            ++i;
        }
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
bool
CTMultiRegistry< T, LockType >::IsRegistered( const CString& registryName ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( m_lock );
    return m_list.find( registryName ) != m_list.end();
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
bool
CTMultiRegistry< T, LockType >::IsRegistered( const CString& registryName  ,
                                              const CString& registryEntry ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( m_lock );
    typename TRegistryList::const_iterator i = m_list.find( registryName );
    if ( i != m_list.end() )
    {
        TSubRegistryPtr registry( (*i) );
        lock.EarlyReaderUnlock();

        return registry->IsRegistered( registryEntry );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
typename CTMultiRegistry< T, LockType >::TRegisteredObjPtr
CTMultiRegistry< T, LockType >::Lookup( const CString& registryName  ,
                                        const CString& registryEntry ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( m_lock );
    typename TRegistryList::const_iterator i = m_list.find( registryName );
    if ( i != m_list.end() )
    {
        TSubRegistryPtr registry( (*i) );
        lock.EarlyReaderUnlock();

        return registry->Lookup( registryEntry );
    }

    GUCEF_EMSGTHROW( EUnregisteredName, "gucefCORE::CTMultiRegistry::Lookup(): unregistered name given" );
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
bool
CTMultiRegistry< T, LockType >::TryLookup( const CString& registryName   ,
                                           const CString& registryEntry  ,
                                           TRegisteredObjPtr& locatedObj ,
                                           bool caseSensitive            ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( m_lock );
    typename TRegistryList::const_iterator i = m_list.find( registryName );
    if ( i != m_list.end() )
    {
        TSubRegistryPtr registry( (*i) );
        lock.EarlyReaderUnlock();

        return registry->TryLookup( registryEntry ,
                                    locatedObj    ,
                                    caseSensitive );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTMultiRegistry< T, LockType >::Register( const CString& registryName        ,
                                          const CString& registryEntry       ,
                                          const TRegisteredObjPtr& sharedPtr )
{GUCEF_TRACE;

    TSubRegistryPtr registry;
    {
        MT::CObjectScopeLock lock( m_lock );
        typename TRegistryList::const_iterator i = m_list.find( registryName );
        if ( i != m_list.end() )
        {
            registry = (*i);
        }
        else
        {
            registry = GUCEF_NEW CTRegistry< T, LockType >();
            m_list[ registryName ] = registry;
        }
    }

    return registry->Register( registryEntry ,
                               sharedPtr     );
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
bool
CTMultiRegistry< T, LockType >::TryRegister( const CString& registryName        ,
                                             const CString& registryEntry       ,
                                             const TRegisteredObjPtr& sharedPtr )
{GUCEF_TRACE;

    TSubRegistryPtr registry;
    {
        MT::CObjectScopeLock lock( m_lock );
        typename TRegistryList::const_iterator i = m_list.find( registryName );
        if ( i != m_list.end() )
        {
            registry = (*i);
        }
        else
        {
            registry = GUCEF_NEW CTRegistry< T, LockType >();
            m_list[ registryName ] = registry;
        }
    }

    return registry->TryRegister( registryEntry ,
                                  sharedPtr     );
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTMultiRegistry< T, LockType >::Unregister( const CString& registryName  ,
                                            const CString& registryEntry  )
{GUCEF_TRACE;

    TSubRegistryPtr registry;
    {
        MT::CObjectScopeLock lock( m_lock );
        typename TRegistryList::const_iterator i = m_list.find( registryName );
        if ( i != m_list.end() )
        {
            registry = (*i);
        }
        else
        {
            registry = GUCEF_NEW CTRegistry< T, LockType >();
            m_list[ registryName ] = registry;
        }
    }

    registry->Unregister( registryEntry );
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
bool
CTMultiRegistry< T, LockType >::TryUnregister( const CString& registryName  ,
                                               const CString& registryEntry )
{GUCEF_TRACE;

    TSubRegistryPtr registry;
    {
        MT::CObjectScopeLock lock( m_lock );
        typename TRegistryList::const_iterator i = m_list.find( registryName );
        if ( i != m_list.end() )
        {
            registry = (*i);
        }
        else
        {
            registry = GUCEF_NEW CTRegistry< T, LockType >();
            m_list[ registryName ] = registry;
        }
    }

    return registry->TryUnregister( registryEntry );
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTMultiRegistry< T, LockType >::UnregisterAll( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "TMultiRegistry<>(" + CORE::ToString( this ) + "): Unregistering All" );

    typename TRegistryList::iterator i = m_list.begin();
    while ( i != m_list.end() )
    {
        (*i).second->UnregisterAll();
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
UInt32
CTMultiRegistry< T, LockType >::GetCount( void ) const
{GUCEF_TRACE;

    UInt32 totalCount = 0;
    MT::CObjectScopeReadOnlyLock lock( m_lock );
    typename TRegistryList::const_iterator i = m_list.begin();
    while ( i != m_list.end() )
    {
        totalCount += (*i).second->GetCount();
        ++i;
    }
    return totalCount;
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
UInt32
CTMultiRegistry< T, LockType >::GetCount( const CString& registryName ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( m_lock );
    typename TRegistryList::const_iterator i = m_list.find( registryName );
    if ( i != m_list.end() )
    {
        TSubRegistryPtr registry( (*i) );
        lock.EarlyReaderUnlock();

        return registry->GetCount();
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTMultiRegistry< T, LockType >::GetList( TStringList& destList ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( m_lock );
    typename TRegistryList::const_iterator i = m_list.begin();
    while ( i != m_list.end() )
    {
        (*i).second->GetList( destList );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
MT::TLockStatus
CTMultiRegistry< T, LockType >::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
MT::TLockStatus
CTMultiRegistry< T, LockType >::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
const MT::CILockable*
CTMultiRegistry< T, LockType >::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTREGISTRY_H  ? */
