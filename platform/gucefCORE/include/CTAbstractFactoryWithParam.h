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

#ifndef GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H
#define GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H

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

#include "CTFactoryWithParam.h"
#include "CTFactoryBaseWithParam.h"
#include "CMsgException.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H */

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
template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
class CTAbstractFactoryWithParam : public CAbstractFactoryBase 
{
    public:

    typedef SelectionCriteriaType                                                                                               TSelectionCriteriaType;
    typedef CTCloneableObj< SelectionCriteriaType >                                                                             TKeyContainer;
    typedef CTFactoryBaseWithParam< BaseClassType, ConstructionParamType, LockType >                                            TFactory;
    typedef std::set< SelectionCriteriaType, std::less< SelectionCriteriaType >, gucef_allocator< SelectionCriteriaType > >     TKeySet;
    typedef typename TFactory::TProductPtr                                                                                      TProductPtr;

    explicit CTAbstractFactoryWithParam( const bool assumeFactoryOwnership = false ,
                                         const bool useEventing = true             );

    CTAbstractFactoryWithParam( const CTAbstractFactoryWithParam& src );

    virtual ~CTAbstractFactoryWithParam();

    CTAbstractFactoryWithParam& operator=( const CTAbstractFactoryWithParam& src );

    /**
     *  Constructs the concrete factory product
     *
     *  @return pointer to the base class of the constructed factory product
     */
    TProductPtr Create( const SelectionCriteriaType& selectedType ,
                        const ConstructionParamType& param        ) const;

    /**
     *  Constructs the concrete factory product using a class type name
     *
     *  @return pointer to the base class of the constructed factory product
     */
    TProductPtr CreateUsingClassTypeName( const CString& classTypeName       ,
                                          const ConstructionParamType& param ) const;

    void RegisterConcreteFactory( const SelectionCriteriaType& selectedType ,
                                  TFactory* concreteFactory                 );

    void UnregisterConcreteFactory( const SelectionCriteriaType& selectedType );

    void UnregisterAllConcreteFactories( void );

    bool IsConstructible( const SelectionCriteriaType& selectedType ) const;

    /**
     *  Returns the concete class's type name as produced by a factory linked to the given selection criterea
     */
    CString GetConcreteClassTypeNameForSelectionCriterea( const SelectionCriteriaType& selectedType ) const;

    /**
     *  Returns the factory linked selection criterea for the given concete class's type name
     */
    SelectionCriteriaType GetSelectionCritereaForConcreteClassTypeName( const CString& classTypeName ) const;

    void ObtainKeySet( TKeySet& keySet ) const;

    private:
    typedef std::pair< SelectionCriteriaType, TFactory* >   TFactoryEntryPair;
    typedef std::map< SelectionCriteriaType, TFactory*, std::less< SelectionCriteriaType >, gucef_allocator< TFactoryEntryPair > >  TFactoryList;
    typedef std::pair< CString, SelectionCriteriaType >   TStringToSCPair;
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

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::CTAbstractFactoryWithParam( const bool assumeFactoryOwnership /* = false */ ,
                                                                                                                                 const bool useEventing /* = true */             )
    : CAbstractFactoryBase( useEventing )
    , m_concreteFactoryList()
    , m_concreteFactoryTypeMap()
    , m_assumeFactoryOwnership( assumeFactoryOwnership )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::CTAbstractFactoryWithParam( const CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >& src )
    : CAbstractFactoryBase( src )
    , m_concreteFactoryList() // Wont copy raw factory pointers, too much risk of bad access. only copy 'config' style info
    , m_concreteFactoryTypeMap( src.m_concreteFactoryTypeMap )
    , m_assumeFactoryOwnership( src.m_assumeFactoryOwnership )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::~CTAbstractFactoryWithParam()
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    if ( m_assumeFactoryOwnership )
    {
        typename TFactoryList::iterator i = m_concreteFactoryList.begin();
        while ( i != m_concreteFactoryList.end() )
        {
            GUCEF_DELETE (*i).second;
            (*i).second = GUCEF_NULL;
            ++i;
        }
    }
    m_concreteFactoryList.clear();
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >&
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::operator=( const CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >& /* src */ )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
typename CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::TProductPtr
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::Create( const SelectionCriteriaType& selectedType ,
																                                             const ConstructionParamType& param        ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    typename TFactoryList::const_iterator i( m_concreteFactoryList.find( selectedType ) );
    if ( i != m_concreteFactoryList.end() )
    {
        TProductPtr product( (*i).second->Create( param ) );

        #ifdef GUCEF_DEBUG_MODE
        if ( !product.IsNULL() )
            { GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, !product.IsNULL() ? "TAbstractFactoryWithParam<>: Created concrete type \"" + ToString( selectedType ) + "\"" : "TAbstractFactoryWithParam<>: Failed to create concrete type \"" + ToString( selectedType ) + "\"" ); }
        else
            { GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "TAbstractFactoryWithParam<>: Failed to create concrete type \"" + ToString( selectedType ) + "\"" ); }
        #endif /* GUCEF_DEBUG_MODE */

        return product;
    }
    return TProductPtr();
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
typename CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::TProductPtr
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::CreateUsingClassTypeName( const CString& classTypeName       ,
                                                                                                                               const ConstructionParamType& param ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this, GUCEF_MT_VERY_LONG_LOCK_TIMEOUT );
    
    // Resolve the class type name to a registered type name
    typename TClassTypeNameMap::const_iterator i( m_concreteFactoryTypeMap.find( classTypeName ) );
    if ( i != m_concreteFactoryTypeMap.end() )
    {
        // Use the registered type name to actually carry out the creation
        const SelectionCriteriaType& selectedType = (*i).second;
        return Create( selectedType, param );
    }
    return TProductPtr();
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
void
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::ObtainKeySet( TKeySet& keySet ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    typename TFactoryList::const_iterator i( m_concreteFactoryList.begin() );
    while ( i != m_concreteFactoryList.end() )
    {
        keySet.insert( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
bool
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::IsConstructible( const SelectionCriteriaType& selectedType ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_concreteFactoryList.find( selectedType ) != m_concreteFactoryList.end();
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
CString
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::GetConcreteClassTypeNameForSelectionCriterea( const SelectionCriteriaType& selectedType ) const
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

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
SelectionCriteriaType
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::GetSelectionCritereaForConcreteClassTypeName( const CString& classTypeName ) const
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

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
void
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::RegisterConcreteFactory( const SelectionCriteriaType& selectedType ,
                                                                                                                              TFactory* concreteFactory                 )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_concreteFactoryList[ selectedType ] = concreteFactory;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "TAbstractFactoryWithParam<>: Registered concrete factory for type \"" + ToString( selectedType ) + "\"" );

    if ( m_useEventing )
    {
        TKeyContainer keyContainer( selectedType );
        NotifyObservers( ConcreteFactoryRegisteredEvent, &keyContainer );
    }
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
void
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::UnregisterConcreteFactory( const SelectionCriteriaType& selectedType )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    typename TFactoryList::iterator i = m_concreteFactoryList.find( selectedType );
    if ( i != m_concreteFactoryList.end() )
    {
        if ( m_assumeFactoryOwnership )
        {
            GUCEF_DELETE (*i).second;
        }
        m_concreteFactoryList.erase( i );

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "TAbstractFactoryWithParam<>: Unregistered concrete factory for type \"" + ToString( selectedType ) + "\"" );

        if ( m_useEventing )
        {
            TKeyContainer keyContainer( selectedType );
            NotifyObservers( ConcreteFactoryUnregisteredEvent, &keyContainer );
        }
    }
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
void
CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType >::UnregisterAllConcreteFactories( void )
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

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "TAbstractFactoryWithParam<>: Unregistered concrete factory for type \"" + ToString( selectedType ) + "\"" );

        if ( m_useEventing )
        {
            TKeyContainer keyContainer( selectedType );
            NotifyObservers( ConcreteFactoryUnregisteredEvent, &keyContainer );
        }
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "TAbstractFactoryWithParam<>: Unregistered all concrete factories" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H ? */
