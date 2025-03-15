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

#ifndef GUCEF_CORE_CTFACTORYWITHPARAM_H
#define GUCEF_CORE_CTFACTORYWITHPARAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "CTFactoryBaseWithParam.h"

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
template< class BaseClassType, class ConcreteClassType, typename ConstructionParamType, typename LockType >
class CTFactoryWithParam : public CTFactoryBaseWithParam< BaseClassType, ConstructionParamType, LockType > ,
                           public CTTypeNamedDynamicDestructorBase< ConcreteClassType > ,
                           public CTDynamicDestructorBase< ConcreteClassType >
{
    public:

    typedef typename CTFactoryBaseWithParam< BaseClassType, ConstructionParamType, LockType >::TProductPtr  TProductPtr;
    typedef ConcreteClassType                                                                               TConcreteClassType;
    typedef CTSharedPtr< ConcreteClassType, LockType >                                                      TConcreteProductPtr;

    /**
     *  Default constructor which acts as a no-op since factories are
     *  meta-data classes
     */
    CTFactoryWithParam( void );

    /**
     *  Copy constructor which acts as a no-op since factories are
     *  meta-data classes
     */
    CTFactoryWithParam( const CTFactoryWithParam& src );

    /**
     *  Default destructor which acts as a no-op since factories are
     *  meta-data classes
     */
    virtual ~CTFactoryWithParam();

    /**
     *  Default assignment operator which acts as a no-op since factories are
     *  meta-data classes
     */
    CTFactoryWithParam& operator=( const CTFactoryWithParam& src );

    /**
     *  Constructs the concrete factory product
     *
     *  @return pointer to the base class of the constructed factory product
     */
    virtual TProductPtr Create( const ConstructionParamType& param );

    /**
     *  Allows creation of factories without knowing the decending concrete
     *  classes.
     */
    virtual CICloneable* Clone( void ) const;

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

template< class BaseClassType, class ConcreteClassType, typename ConstructionParamType, typename LockType >
CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >::CTFactoryWithParam( void )
    : CTFactoryBaseWithParam< BaseClassType, ConstructionParamType, LockType >()
    , CTTypeNamedDynamicDestructorBase< ConcreteClassType >()
    , CTDynamicDestructorBase< ConcreteClassType >()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, typename ConstructionParamType, typename LockType >
CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >::CTFactoryWithParam( const CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >& src )
    : CTFactoryBaseWithParam< BaseClassType, ConstructionParamType, LockType >( src )
    , CTTypeNamedDynamicDestructorBase< ConcreteClassType >( src )
    , CTDynamicDestructorBase< ConcreteClassType >( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, typename ConstructionParamType, typename LockType >
CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >::~CTFactoryWithParam()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, typename ConstructionParamType, typename LockType >
CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >&
CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >::operator=( const CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, typename ConstructionParamType, typename LockType >
typename CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >::TProductPtr
CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >::Create( const ConstructionParamType& param )
{GUCEF_TRACE;

    ConcreteClassType* concreteObj = GUCEF_NULL;
    try
    {
        concreteObj = GUCEF_NEW ConcreteClassType( param );
        BaseClassType* baseClassPtr = static_cast< BaseClassType* >( concreteObj );
        return TProductPtr( baseClassPtr, this, concreteObj );
    }
    catch ( const std::exception& )
    {
        GUCEF_DELETE concreteObj;
        concreteObj = GUCEF_NULL;
    }

    return TProductPtr();
}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, typename ConstructionParamType, typename LockType >
void
CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >::DestroyObject( ConcreteClassType* objectToBeDestroyed ) const
{GUCEF_TRACE;

    /*
     *  We delete at the level of the decendant class or concrete factory product if you will,
     *  to ensure it's destructor is called even if the base class does not have a
     *  virtual destructor
     */
    GUCEF_DELETE objectToBeDestroyed;
}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, typename ConstructionParamType, typename LockType >
void
CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >::DestroyObject( ConcreteClassType* objectToBeDestroyed, const CString& classTypeName ) const
{GUCEF_TRACE;

    /*
     *  We delete at the level of the decendant class or concrete factory product if you will,
     *  to ensure it's destructor is called even if the base class does not have a
     *  virtual destructor
     */
    GUCEF_DELETE static_cast< ConcreteClassType* >( objectToBeDestroyed );
}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, typename ConstructionParamType, typename LockType >
CICloneable*
CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >::Clone( void ) const
{GUCEF_TRACE;

    /*
     *  In contrast to normal cloning we do not need to use the copy constructor
     *  here because a factory is meta data and has no attributes.
     *
     *  Decending classes can offcourse override this if additional criteria are
     *  to be considdered.
     */
    return GUCEF_NEW CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >;
}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, class ConcreteClassType, typename ConstructionParamType, typename LockType >
CString
CTFactoryWithParam< BaseClassType, ConcreteClassType, ConstructionParamType, LockType >::GetConcreteClassTypeName( void ) const
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

#endif /* GUCEF_CORE_CTFACTORYWITHPARAM_H ? */
