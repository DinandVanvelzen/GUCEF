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

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#define GUCEF_CORE_CTABSTRACTFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <set>

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#include "gucefMT_CObjectScopeReadOnlyLock.h"
#define GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#endif /* GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORYBASE_H
#include "CTFactoryBase.h"
#define GUCEF_CORE_CTFACTORYBASE_H
#endif /* GUCEF_CORE_CTFACTORYBASE_H ? */

#include "CMsgException.h"

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CABSTRACTFACTORYBASE_H
#include "gucefCORE_CAbstractFactoryBase.h"
#define GUCEF_CORE_CABSTRACTFACTORYBASE_H
#endif /* GUCEF_CORE_CABSTRACTFACTORYBASE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEMPLATES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Abstract factory template for the selection and utilization of concrete
 *  factories based on a criteria.
 *
 *  Note that when using this template it is mandatory for BaseClassType
 *  classes to define the following member function:
 *      SelectionCriteriaType GetType( void ) const;
 *
 */
template< typename SelectionCriteriaType, class BaseClassType, class LockType >
class CTAbstractFactory : public CAbstractFactoryBase
{
    public:

    typedef SelectionCriteriaType                                                                                           TSelectionCriteriaType;
    typedef CTCloneableObj< SelectionCriteriaType >                                                                         TKeyContainer;
    typedef CTFactoryBase< BaseClassType, LockType >                                                                        TFactory;
    typedef std::set< SelectionCriteriaType, std::less< SelectionCriteriaType >, gucef_allocator< SelectionCriteriaType > > TKeySet;
    typedef typename CTFactoryBase< BaseClassType, LockType >::TProductPtr                                                  TProductPtr;

    explicit CTAbstractFactory( const bool assumeFactoryOwnership = false ,
                                const bool useEventing = true             );

    CTAbstractFactory( const CTAbstractFactory& src );

    virtual ~CTAbstractFactory();

    CTAbstractFactory& operator=( const CTAbstractFactory& src );

    /**
     *  Constructs the concrete factory product
     *
     *  @return pointer to the base class of the constructed factory product
     */
    TProductPtr Create( const SelectionCriteriaType& selectedType ) const;

    /**
     *  Constructs the concrete factory product using a class type name
     *
     *  @return pointer to the base class of the constructed factory product
     */
    TProductPtr CreateUsingClassTypeName( const CString& classTypeName ) const;

    void RegisterConcreteFactory( const SelectionCriteriaType& selectedType ,
                                  TFactory* concreteFactory                 );

    void UnregisterConcreteFactory( const SelectionCriteriaType& selectedType );

    void UnregisterAllConcreteFactories( void );

    bool IsConstructible( const SelectionCriteriaType& selectedType ) const;

    bool IsClassConstructible( const CString& classTypeName ) const;

    /**
     *  Returns the concete class's type name as produced by a factory linked to the given selection criterea
     */
    CString GetConcreteClassTypeNameForSelectionCriterea( const SelectionCriteriaType& selectedType ) const;

    /**
     *  Returns the factory linked selection criterea for the given concete class's type name
     */
    SelectionCriteriaType GetSelectionCritereaForConcreteClassTypeName( const CString& classTypeName ) const;

    void ObtainKeySet( TKeySet& keySet ) const;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    typedef std::pair< const SelectionCriteriaType, TFactory* >   TFactoryEntryPair;
    typedef std::map< SelectionCriteriaType, TFactory*, std::less< SelectionCriteriaType >, gucef_allocator< TFactoryEntryPair > >  TFactoryList;
    typedef std::pair< const CString, SelectionCriteriaType >   TStringToSCPair;
    typedef std::map< CString, SelectionCriteriaType, std::less< CString >, gucef_allocator< TStringToSCPair > >  TClassTypeNameMap;

    TFactoryList m_concreteFactoryList;
    TClassTypeNameMap m_concreteFactoryTypeMap;
    bool m_assumeFactoryOwnership;
    LockType m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::CTAbstractFactory( const bool assumeFactoryOwnership /* = false */ ,
                                                                                        const bool useEventing /* = true */             )
    : CAbstractFactoryBase( useEventing )
    , m_concreteFactoryList()
    , m_concreteFactoryTypeMap()
    , m_assumeFactoryOwnership( assumeFactoryOwnership )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::CTAbstractFactory( const CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >& src )
    : CAbstractFactoryBase( src )
    , m_concreteFactoryList() // Wont copy raw factory pointers, too much risk of bad access. only copy 'config' style info
    , m_concreteFactoryTypeMap( src.m_concreteFactoryTypeMap )
    , m_assumeFactoryOwnership( src.m_assumeFactoryOwnership )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::~CTAbstractFactory()
{GUCEF_TRACE;

    UnregisterAllConcreteFactories();
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >&
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::operator=( const CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
typename CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::TProductPtr
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::Create( const SelectionCriteriaType& selectedType ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this, GUCEF_MT_VERY_LONG_LOCK_TIMEOUT );
    typename TFactoryList::const_iterator i( m_concreteFactoryList.find( selectedType ) );
    if ( i != m_concreteFactoryList.end() )
    {
        TProductPtr product( (*i).second->Create() );
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, !product.IsNULL() ? "TAbstractFactory<>: Created concrete type \"" + ToString( selectedType ) + "\"" : "TAbstractFactory<>: Failed to create concrete type \"" + ToString( selectedType ) + "\"" );
        return product;
    }
    return TProductPtr();
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
typename CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::TProductPtr
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::CreateUsingClassTypeName( const CString& classTypeName ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this, GUCEF_MT_VERY_LONG_LOCK_TIMEOUT );
    
    // Resolve the class type name to a registered type name
    typename TClassTypeNameMap::const_iterator i( m_concreteFactoryTypeMap.find( classTypeName ) );
    if ( i != m_concreteFactoryTypeMap.end() )
    {
        // Use the registered type name to actually carry out the creation
        return Create( (*i).second );
    }
    return TProductPtr();
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
void
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::ObtainKeySet( TKeySet& keySet ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_VERY_LONG_LOCK_TIMEOUT );
    typename TFactoryList::const_iterator i( m_concreteFactoryList.begin() );
    while ( i != m_concreteFactoryList.end() )
    {
        keySet.insert( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
bool
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::IsConstructible( const SelectionCriteriaType& selectedType ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_VERY_LONG_LOCK_TIMEOUT );
    bool canBeConstructed = m_concreteFactoryList.find( selectedType ) != m_concreteFactoryList.end();
    return canBeConstructed;
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
bool
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::IsClassConstructible( const CString& classTypeName ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_VERY_LONG_LOCK_TIMEOUT );
    bool canBeConstructed = m_concreteFactoryTypeMap.find( classTypeName ) != m_concreteFactoryTypeMap.end();
    return canBeConstructed;
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
CString
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::GetConcreteClassTypeNameForSelectionCriterea( const SelectionCriteriaType& selectedType ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_VERY_LONG_LOCK_TIMEOUT );
    typename TFactoryList::const_iterator i( m_concreteFactoryList.find( selectedType ) );
    if ( i != m_concreteFactoryList.end() )
    {
        return (*i).second->GetConcreteClassTypeName();
    }

    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
SelectionCriteriaType
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::GetSelectionCritereaForConcreteClassTypeName( const CString& classTypeName ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this, GUCEF_MT_VERY_LONG_LOCK_TIMEOUT );
    typename TClassTypeNameMap::const_iterator i( m_concreteFactoryTypeMap.find( classTypeName ) );
    if ( i != m_concreteFactoryTypeMap.end() )
    {
        return (*i).second;
    }

    return SelectionCriteriaType();
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
void
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::RegisterConcreteFactory( const SelectionCriteriaType& selectedType ,
                                                                                              TFactory* concreteFactory                 )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this, GUCEF_MT_VERY_LONG_LOCK_TIMEOUT );
    m_concreteFactoryList[ selectedType ] = concreteFactory;
    m_concreteFactoryTypeMap[ concreteFactory->GetConcreteClassTypeName() ] = selectedType;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "TAbstractFactory<>: Registered concrete factory for type \"" + ToString( selectedType ) + "\" with concrete class type \"" + concreteFactory->GetConcreteClassTypeName() + "\"" );

    if ( m_useEventing )
    {
        TKeyContainer keyContainer( selectedType );
        lock.EarlyUnlock();
        NotifyObservers( ConcreteFactoryRegisteredEvent, &keyContainer );
    }
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
void
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::UnregisterConcreteFactory( const SelectionCriteriaType& selectedType )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this, GUCEF_MT_VERY_LONG_LOCK_TIMEOUT );
    typename TFactoryList::iterator i = m_concreteFactoryList.find( selectedType );
    if ( i != m_concreteFactoryList.end() )
    {
        CString concreteClassTypeName = (*i).second->GetConcreteClassTypeName();

        if ( m_assumeFactoryOwnership )
        {
            GUCEF_DELETE (*i).second;
            (*i).second = GUCEF_NULL;
        }
        m_concreteFactoryList.erase( i );
        m_concreteFactoryTypeMap.erase( concreteClassTypeName );

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "TAbstractFactory<>: Unregistered concrete factory for type \"" + ToString( selectedType ) + "\" with concrete class type \"" + concreteClassTypeName + "\"");

        if ( m_useEventing )
        {
            TKeyContainer keyContainer( selectedType );
            lock.EarlyUnlock();
            NotifyObservers( ConcreteFactoryUnregisteredEvent, &keyContainer );
        }
    }
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
void
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::UnregisterAllConcreteFactories( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this, GUCEF_MT_VERY_LONG_LOCK_TIMEOUT );
    while ( !m_concreteFactoryList.empty() )
    {
        typename TFactoryList::iterator i = m_concreteFactoryList.begin();
        CString concreteClassTypeName = (*i).second->GetConcreteClassTypeName();

        if ( m_assumeFactoryOwnership )
        {
            GUCEF_DELETE (*i).second;
            (*i).second = GUCEF_NULL;
        }

        SelectionCriteriaType selectedType = (*i).first;
        m_concreteFactoryList.erase( i );
        m_concreteFactoryTypeMap.erase( concreteClassTypeName );

        if ( m_useEventing )
        {
            TKeyContainer keyContainer( selectedType );
            NotifyObservers( ConcreteFactoryUnregisteredEvent, &keyContainer );
        }
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "TAbstractFactory<>: Unregistered all concrete factories" );
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
MT::TLockStatus
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
MT::TLockStatus
CTAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */
