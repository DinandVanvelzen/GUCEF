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

#ifndef GUCEF_CORE_CTFACTORY_H
#define GUCEF_CORE_CTFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORYBASE_H
#include "CTFactoryBase.h"
#define GUCEF_CORE_CTFACTORYBASE_H
#endif /* GUCEF_CORE_CTFACTORYBASE_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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
 *  Template for creating a concrete factory as described in the
 *  factory design pattern.
 */
template< class BaseClassType, class ConcreteClassType, class LockType >
class CTFactory : public CTFactoryBase< BaseClassType, LockType > ,
                  public CTTypeNamedDynamicDestructorBase< ConcreteClassType > ,
                  public CTDynamicDestructorBase< ConcreteClassType >
{
    public:

    typedef typename CTFactoryBase< BaseClassType, LockType >::TProductPtr      TProductPtr;
    typedef ConcreteClassType                                                   TConcreteClassType;
    typedef CTSharedPtr< ConcreteClassType, LockType >                          TConcreteProductPtr;

    CTFactory( void );

    CTFactory( const CTFactory& src );

    virtual ~CTFactory();

    CTFactory& operator=( const CTFactory& src );

    /**
     *  Constructs the concrete factory product
     *
     *  @return pointer to the base class of the constructed factory product
     */
    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Allows creation of factories without knowing the decending concrete
     *  classes.
     */
    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Returns a string representing the name of concrete class that can be created
     */
    virtual CString GetConcreteClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyObject( ConcreteClassType* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyObject( ConcreteClassType* objectToBeDestroyed, const CString& classTypeName ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, class LockType >
CTFactory< BaseClassType, ConcreteClassType, LockType >::CTFactory( void )
    : CTFactoryBase< BaseClassType, LockType >()
    , CTTypeNamedDynamicDestructorBase< ConcreteClassType >()
    , CTDynamicDestructorBase< ConcreteClassType >()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, class LockType >
CTFactory< BaseClassType, ConcreteClassType, LockType >::CTFactory( const CTFactory< BaseClassType, ConcreteClassType, LockType >& src )
    : CTFactoryBase< BaseClassType, LockType >( src )
    , CTTypeNamedDynamicDestructorBase< ConcreteClassType >( src )
    , CTDynamicDestructorBase< ConcreteClassType >( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, class LockType >
CTFactory< BaseClassType, ConcreteClassType, LockType >::~CTFactory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, class LockType >
CTFactory< BaseClassType, ConcreteClassType, LockType >&
CTFactory< BaseClassType, ConcreteClassType, LockType >::operator=( const CTFactory< BaseClassType, ConcreteClassType, LockType >& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CTFactoryBase< BaseClassType, LockType >::operator=( src );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, class LockType >
typename CTFactory< BaseClassType, ConcreteClassType, LockType >::TProductPtr
CTFactory< BaseClassType, ConcreteClassType, LockType >::Create( void )
{GUCEF_TRACE;

    ConcreteClassType* concreteObj = GUCEF_NULL;
    try
    {
        concreteObj = GUCEF_NEW ConcreteClassType();
        BaseClassType* baseClassPtr = static_cast< BaseClassType* >( concreteObj );
        return TProductPtr( baseClassPtr, this, concreteObj );
    }
    catch ( const std::exception& )
    {
        GUCEF_DELETE concreteObj;
        concreteObj = GUCEF_NULL;
    }

    return TConcreteProductPtr();
}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, class LockType >
void
CTFactory< BaseClassType, ConcreteClassType, LockType >::DestroyObject( ConcreteClassType* objectToBeDestroyed ) const
{GUCEF_TRACE;

    /*
     *  We delete at the level of the decendant class or concrete factory product if you will,
     *  to ensure it's destructor is called even if the base class does not have a
     *  virtual destructor
     */
    GUCEF_DELETE objectToBeDestroyed;
}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, class LockType >
void
CTFactory< BaseClassType, ConcreteClassType, LockType >::DestroyObject( ConcreteClassType* objectToBeDestroyed, const CString& classTypeName ) const
{GUCEF_TRACE;

    /*
     *  We delete at the level of the decendant class or concrete factory product if you will,
     *  to ensure it's destructor is called even if the base class does not have a
     *  virtual destructor
     */
    GUCEF_DELETE static_cast< ConcreteClassType* >( objectToBeDestroyed );
}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, class LockType >
CICloneable*
CTFactory< BaseClassType, ConcreteClassType, LockType >::Clone( void ) const
{GUCEF_TRACE;

    /*
     *  In contrast to normal cloning we do not need to use the copy constructor
     *  here because a factory is meta data and has no attributes.
     *
     *  Descending classes can of course override this if additional criteria are
     *  to be considered.
     */
    return GUCEF_NEW CTFactory< BaseClassType, ConcreteClassType, LockType >();
}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, class LockType >
CString
CTFactory< BaseClassType, ConcreteClassType, LockType >::GetConcreteClassTypeName( void ) const
{GUCEF_TRACE;

    return ToString< ConcreteClassType >();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTFACTORY_H ? */
