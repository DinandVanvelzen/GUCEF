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
 
#ifndef GUCEF_CORE_CTDYNAMICDESTRUCTORBASE_H
#define GUCEF_CORE_CTDYNAMICDESTRUCTORBASE_H
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"       /* module macro's */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

class GUCEF_CORE_PUBLIC_CPP CIDynamicVoidDestructor
{
    public:

    virtual ~CIDynamicVoidDestructor() {}
    
    virtual void DestroyKnownVoidedObject( void* objectToBeDestroyed ) const = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  Base class for destructor delegator specializations
 */
template< typename T >
class CTDynamicDestructorBase : public CIDynamicVoidDestructor
{
    public:

    typedef T TDestructorType;
        
    CTDynamicDestructorBase( void );
  
    virtual ~CTDynamicDestructorBase();    
    
    virtual void DestroyObject( T* objectToBeDestroyed ) const = 0;

    virtual void DestroyKnownVoidedObject( void* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE
        {GUCEF_TRACE; DestroyObject( reinterpret_cast< T* >( objectToBeDestroyed ) ); }

    private:

    CTDynamicDestructorBase( const CTDynamicDestructorBase& src );            /**< not implemented */
    CTDynamicDestructorBase& operator=( const CTDynamicDestructorBase& src ); /**< not implemented */

};

/*-------------------------------------------------------------------------*/

/**
 *  Base class for destructor delegator specializations that take a string form hint for the 
 *  derived concrete instantiation's type
 */
template< typename T >
class CTTypeNamedDynamicDestructorBase
{
    public:

    typedef T TDestructorType;
        
    CTTypeNamedDynamicDestructorBase( void );
  
    virtual ~CTTypeNamedDynamicDestructorBase();    
    
    virtual void DestroyObject( T* objectToBeDestroyed, const CString& classTypeName ) const = 0;  

    private:

    CTTypeNamedDynamicDestructorBase( const CTTypeNamedDynamicDestructorBase& src );            /**< not implemented */
    CTTypeNamedDynamicDestructorBase& operator=( const CTTypeNamedDynamicDestructorBase& src ); /**< not implemented */

};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T >
CTDynamicDestructorBase< T >::CTDynamicDestructorBase( void )
{

}

/*-------------------------------------------------------------------------*/
   
template< typename T >
CTDynamicDestructorBase< T >::~CTDynamicDestructorBase()
{

}

/*-------------------------------------------------------------------------*/

template< typename T >
CTTypeNamedDynamicDestructorBase< T >::CTTypeNamedDynamicDestructorBase( void )
{

}

/*-------------------------------------------------------------------------*/
   
template< typename T >
CTTypeNamedDynamicDestructorBase< T >::~CTTypeNamedDynamicDestructorBase()
{

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF *

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTDYNAMICDESTRUCTORBASE_H ? */
