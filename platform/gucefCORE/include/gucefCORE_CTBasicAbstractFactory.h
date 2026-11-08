/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CTBASICABSTRACTFACTORY_H
#define GUCEF_CORE_CTBASICABSTRACTFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CBASICABSTRACTFACTORYBASE_H
#include "gucefCORE_CBasicAbstractFactoryBase.h"
#define GUCEF_CORE_CBASICABSTRACTFACTORYBASE_H
#endif /* GUCEF_CORE_CBASICABSTRACTFACTORYBASE_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORYREGISTRY_H
#include "gucefCORE_CTAbstractFactoryRegistry.h"
#define GUCEF_CORE_CTABSTRACTFACTORYREGISTRY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORYREGISTRY_H ? */

#ifndef GUCEF_CORE_CTFACTORYBASE_H
#include "CTFactoryBase.h"
#define GUCEF_CORE_CTFACTORYBASE_H
#endif /* GUCEF_CORE_CTFACTORYBASE_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

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
 *  Lightweight abstract factory template for the selection and utilization of concrete factories based on a criteria:
 *  no notifier and no events, guarded by a lock of LockType (MT::CNoLock for use by one thread).
 *  The products are made by concrete factories of type CTFactoryBase, without parameters.
 *
 *  Note that when using this template it is mandatory for BaseClassType classes to define the following member
 *  function:
 *      SelectionCriteriaType GetType( void ) const;
 *
 *  Everything but the create is shared by every abstract factory: the registry, the rules for creates on more
 *  threads, removals, replacements, ownership, waits and cycles are documented at CTAbstractFactoryRegistry. A class
 *  derived from it learns of registrations and removals by overriding OnConcreteFactoryRegistered() and
 *  OnConcreteFactoryUnregistered(). The form with events is CTAbstractFactory.
 */
template< typename SelectionCriteriaType, class BaseClassType, class LockType >
class CTBasicAbstractFactory : public CTAbstractFactoryRegistry< SelectionCriteriaType, CTFactoryBase< BaseClassType, LockType >, LockType, CBasicAbstractFactoryBase >
{
    public:

    typedef CTAbstractFactoryRegistry< SelectionCriteriaType, CTFactoryBase< BaseClassType, LockType >, LockType, CBasicAbstractFactoryBase > TBaseRegistry;
    typedef SelectionCriteriaType                                                                                        TSelectionCriteriaType;
    typedef CTCloneableObj< SelectionCriteriaType >                                                                      TKeyContainer;
    typedef CTFactoryBase< BaseClassType, LockType >                                                                     TFactory;
    typedef GUCEF::set< SelectionCriteriaType, std::less< SelectionCriteriaType >, gucef_allocator< SelectionCriteriaType > > TKeySet;
    typedef typename TFactory::TProductPtr                                                                               TProductPtr;

    explicit CTBasicAbstractFactory( const bool assumeFactoryOwnership = false );

    CTBasicAbstractFactory( const CTBasicAbstractFactory& src );

    virtual ~CTBasicAbstractFactory();

    CTBasicAbstractFactory& operator=( const CTBasicAbstractFactory& src );

    /**
     *  Constructs the concrete factory product
     *
     *  The product is constructed without the registry lock, see CTAbstractFactoryRegistry for the threads. A product
     *  constructor that throws leaves the create with the exception, which is no longer counted as under way.
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
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
CTBasicAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::CTBasicAbstractFactory( const bool assumeFactoryOwnership /* = false */ )
    : TBaseRegistry( assumeFactoryOwnership )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
CTBasicAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::CTBasicAbstractFactory( const CTBasicAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >& src )
    : TBaseRegistry( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
CTBasicAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::~CTBasicAbstractFactory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
CTBasicAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >&
CTBasicAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::operator=( const CTBasicAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >& /* src */ )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
typename CTBasicAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::TProductPtr
CTBasicAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::Create( const SelectionCriteriaType& selectedType ) const
{GUCEF_TRACE;

    // The concrete factory is taken under the lock and the create recorded on its entry, the product is made without
    // the lock; the create ends when the scope does, on every way out
    typename TBaseRegistry::CCreateScope create( *this, selectedType );
    TFactory* concreteFactory = create.GetFactory();
    if ( GUCEF_NULL == concreteFactory )
        return TProductPtr();

    TProductPtr product( concreteFactory->Create() );
    #ifdef GUCEF_DEBUG_MODE
    CAbstractFactoryEntryTracker::LogCreate( ToString( selectedType ), !product.IsNULL() );
    #endif
    return product;
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class BaseClassType, class LockType >
typename CTBasicAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::TProductPtr
CTBasicAbstractFactory< SelectionCriteriaType, BaseClassType, LockType >::CreateUsingClassTypeName( const CString& classTypeName ) const
{GUCEF_TRACE;

    // Resolve the class type name to a registered type, then create without the lock, see Create()
    SelectionCriteriaType selectedType;
    if ( !this->TryGetSelectionCriteriaForClassTypeName( classTypeName, selectedType ) )
        return TProductPtr();
    return Create( selectedType );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTBASICABSTRACTFACTORY_H ? */
