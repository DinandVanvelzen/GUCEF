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
 
#ifndef GUCEF_CORE_CTDYNAMICDESTRUCTOR_H
#define GUCEF_CORE_CTDYNAMICDESTRUCTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTDYNAMICDESTRUCTORBASE_H
#include "CTDynamicDestructorBase.h"
#define GUCEF_CORE_CTDYNAMICDESTRUCTORBASE_H
#endif /* #define GUCEF_CORE_CTDYNAMICDESTRUCTORBASE_H ? */ 

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
 *  Class meant for delegation of destruction duties.
 */
template< typename T >
class CTDynamicDestructor : public CTDynamicDestructorBase< T >
{
    public:

    typedef T TDestructorType;
        
    /**
     *  
     */
    CTDynamicDestructor( const bool destroySelfOnDestroyObject = false );
  
    virtual void DestroyObject( T* objectToBeDestroyed ) GUCEF_VIRTUAL_OVERRIDE;

    virtual ~CTDynamicDestructor() GUCEF_VIRTUAL_OVERRIDE;    

    private:

    CTDynamicDestructor( const CTDynamicDestructor& src );            /**< not implemented */
    CTDynamicDestructor& operator=( const CTDynamicDestructor& src ); /**< not implemented */

    private:

    bool m_destroySelfOnDestroyObject;
};

/*-------------------------------------------------------------------------*/

/**
 *  Class meant for delegation of destruction duties while leveraging string form type information
 */
template< typename T >
class CTTypeNamedDynamicDestructor : public CTTypeNamedDynamicDestructorBase< T >
                                   , public CTDynamicDestructorBase< T >
{
    public:

    typedef T TDestructorType;
        
    /**
     *  Constructs the dynamic destructor with string form class type information to be retained and 
     *  utilized at the time DestroyObject 
     */
    CTTypeNamedDynamicDestructor( const CString& classTypeName                              , 
                                  CTTypeNamedDynamicDestructorBase< T >* externalDestructor ,
                                  const bool destroySelfOnDestroyObject = false             );
  
    virtual void DestroyObject( T* objectToBeDestroyed, const CString& classTypeName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyObject( T* objectToBeDestroyed ) GUCEF_VIRTUAL_OVERRIDE;

    virtual ~CTTypeNamedDynamicDestructor() GUCEF_VIRTUAL_OVERRIDE;    

    private:

    CTTypeNamedDynamicDestructor( const CTTypeNamedDynamicDestructor& src );            /**< not implemented */
    CTTypeNamedDynamicDestructor& operator=( const CTTypeNamedDynamicDestructor& src ); /**< not implemented */

    private:

    bool m_destroySelfOnDestroyObject;
    CString m_classTypeName;
    CTTypeNamedDynamicDestructorBase< T >* m_externalDestructor;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T >
CTDynamicDestructor< T >::CTDynamicDestructor( const bool destroySelfOnDestroyObject /* = false */ )
    : CTDynamicDestructorBase< T >()                             
    , m_destroySelfOnDestroyObject( destroySelfOnDestroyObject ) 
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
   
template< typename T >
CTDynamicDestructor< T >::~CTDynamicDestructor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T > 
void
CTDynamicDestructor< T >::DestroyObject( T* objectToBeDestroyed )
{GUCEF_TRACE;

    GUCEF_DELETE objectToBeDestroyed;

    if ( m_destroySelfOnDestroyObject )
    {
        GUCEF_DELETE this;
    }
}

/*-------------------------------------------------------------------------*/

template< typename T >
CTTypeNamedDynamicDestructor< T >::CTTypeNamedDynamicDestructor( const CString& classTypeName                              , 
                                                                 CTTypeNamedDynamicDestructorBase< T >* externalDestructor ,
                                                                 const bool destroySelfOnDestroyObject /* = false */       )
    : CTDynamicDestructorBase< T >()                             
    , m_destroySelfOnDestroyObject( destroySelfOnDestroyObject )
    , m_classTypeName( classTypeName )  
    , m_externalDestructor( externalDestructor )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
   
template< typename T >
CTTypeNamedDynamicDestructor< T >::~CTTypeNamedDynamicDestructor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T > 
void
CTTypeNamedDynamicDestructor< T >::DestroyObject( T* objectToBeDestroyed )
{GUCEF_TRACE;

    DestroyObject( objectToBeDestroyed, m_classTypeName );
}

/*-------------------------------------------------------------------------*/

template< typename T > 
void
CTTypeNamedDynamicDestructor< T >::DestroyObject( T* objectToBeDestroyed, const CString& classTypeName )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_externalDestructor )
        m_externalDestructor->DestroyObject( objectToBeDestroyed, classTypeName );
    else
        GUCEF_DELETE objectToBeDestroyed;

    if ( m_destroySelfOnDestroyObject )
    {
        GUCEF_DELETE this;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTDYNAMICDESTRUCTOR_H ? */
