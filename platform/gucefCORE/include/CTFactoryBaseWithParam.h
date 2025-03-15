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
 
#ifndef GUCEF_CORE_CTFACTORYBASEWITHPARAM_H
#define GUCEF_CORE_CTFACTORYBASEWITHPARAM_H
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

/*-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORYBASEWITHPARAM_CPP
    #pragma warning( push )
#endif

#pragma warning( disable: 4786 ) // identifier was truncated to 'number' characters
#pragma warning( disable: 4251 ) // 'classname' needs to have dll-interface to be used by clients of class 'classname'
#pragma warning( disable: 4284 )  
#pragma warning( disable: 4146 ) // unary minus operator applied to unsigned type, result still unsigned

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
 *  Template for creating an abstract interface for creation and destruction of 
 *  objects without specifieing the concrete type
 */
template< class BaseClassType, typename ConstructionParamType, typename LockType >
class CTFactoryBaseWithParam : public CICloneable
{
    public:

    typedef LockType                                            TLockType;
    typedef BaseClassType                                       TProductType;
    typedef CTBasicSharedPtr< BaseClassType, LockType >         TProductPtr;
    
    CTFactoryBaseWithParam( void );

    CTFactoryBaseWithParam( const CTFactoryBaseWithParam& src );
    
    /**
     *  Destuctor with no special behaviour, implemented to
     *  ensure that a virtual destructor is provided.
     */
    virtual ~CTFactoryBaseWithParam();

    CTFactoryBaseWithParam& operator=( const CTFactoryBaseWithParam& src );

    /**
     *  Constructs the concrete factory product
     *
     *  @return pointer to the base class of the constructed factory product
     */
    virtual TProductPtr Create( const ConstructionParamType& param  ) = 0;

    /**
     *  Returns a string representing the name of concrete class that can be created
     */
    virtual CString GetConcreteClassTypeName( void ) const = 0;
    
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< class BaseClassType, typename ConstructionParamType, typename LockType >
CTFactoryBaseWithParam< BaseClassType, ConstructionParamType, LockType >::CTFactoryBaseWithParam( void )
    : CICloneable()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, typename ConstructionParamType, typename LockType >
CTFactoryBaseWithParam< BaseClassType, ConstructionParamType, LockType >::CTFactoryBaseWithParam( const CTFactoryBaseWithParam& src )
    : CICloneable( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, typename ConstructionParamType, typename LockType >
CTFactoryBaseWithParam< BaseClassType, ConstructionParamType, LockType >::~CTFactoryBaseWithParam()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class BaseClassType, typename ConstructionParamType, typename LockType >
CTFactoryBaseWithParam< BaseClassType, ConstructionParamType, LockType >&
CTFactoryBaseWithParam< BaseClassType, ConstructionParamType, LockType >::operator=( const CTFactoryBaseWithParam& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        // nothing to do,.. meta-data class
    }
    return *this;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORYBASEWITHPARAM_CPP
    #pragma warning( pop )
#endif

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTFACTORYBASEWITHPARAM_H ? */
