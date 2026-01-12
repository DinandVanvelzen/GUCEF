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

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#define GUCEF_CORE_CTSHAREDPTR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <assert.h>

#ifndef GUCEF_CORE_CTBASICSHAREDPTR_H
#include "CTBasicSharedPtr.h"
#define GUCEF_CORE_CTBASICSHAREDPTR_H
#endif /* GUCEF_CORE_CTBASICSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CTDYNAMICDESTRUCTOR_H
#include "CTDynamicDestructor.h"
#define GUCEF_CORE_CTDYNAMICDESTRUCTOR_H
#endif /* GUCEF_CORE_CTDYNAMICDESTRUCTOR_H ? */

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
 *  Templated implementation of a shared pointer.
 *
 *  A shared pointer adds some safety to pointers that get passed
 *  around where objects keep a reference to object instances independent
 *  of each other. In such situations it can become unclear who is responsible
 *  for destroying the referenced object instance.
 *  The shared pointer will reference count the pointer and ensure that even
 *  though the original owner has no knowledge of other classes referencing the
 *  object it can safely manage object destruction as desired.
 *
 *  Note that the default behavior of this template is to delete the referenced
 *  object instance when the reference count hits zero. This raises some
 *  "out-of-scope-memory-management" concerns. While this can be the intended
 *  behavior, it is advised to alter this behavior to provide an in-scope mechanism
 *  ( ie delegating destruction responsibilities to the original creator of the object ).
 *
 *      Method 1:
 *          inherit from this template and redirect instance destruction
 *          This is achieved by overriding the destructor.
 *
 *      Method 2:
 *          Make the owner of the referenced object inherit from CTSharedObjectDestructor
 *          for the given type. Pass the owner in when creating the pointer object.
 *          This will result in in-scope memory management.
 *
 *  Note that this shared pointer implementation's threadsafety is dependent on the LockType
 *  template param. A NoLock could be used as the LockType for implementations where the
 *  shared ptr is garanteed to never cross thread boundaries resulting is a minor optimization.
 */
template< typename T, class LockType >
class CTSharedPtr : public CTBasicSharedPtr< T, LockType >
{
    public:

    typedef CTBasicSharedPtr< T, LockType >                     TBasicSharedPtrBase;
    typedef typename TBasicSharedPtrBase::TTypedDestructor      TTypedDestructor;
    typedef typename TBasicSharedPtrBase::TContainedType        TContainedType;
    typedef typename TBasicSharedPtrBase::TLockType             TLockType;

    /**
     *  Constructor that allows you to create an 'uninitialized' pointer.
     *  You will have to assign a valid shared pointer at a later time
     *  to initialize the shared pointer.
     *
     *  Note that if you attempt to use the shared pointer before it
     *  is initialized an exception will be thrown
     *
     */
    CTSharedPtr( void );

    explicit CTSharedPtr( const int NULLvalue );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
    CTSharedPtr( CTSharedPtr&& src ) GUCEF_NOEXCEPT;
    #endif

    explicit CTSharedPtr( T* ptr                                                      ,
                          CTDynamicDestructorBase< T >* objectDestructor = GUCEF_NULL ,
                          void* originalAddressAsCreated = GUCEF_NULL                 );

    /**
     *  Conversion constructor making it possible to pass
     *  a CTBasicSharedPtr to a function which requires a
     *  CTSharedPtr thanks to an implicit cast using this constructor.
     */
    explicit CTSharedPtr( const CTBasicSharedPtr< T, LockType >& src );

    // inlined copy constructor, has to be inlined in class definition for now due to VC6 limitations
    template< class Derived >
    CTSharedPtr( const CTSharedPtr< Derived, LockType >& src )
        : CTBasicSharedPtr< T, LockType >()
    {GUCEF_TRACE;

        CTBasicSharedPtr< T, LockType >::InitializeUsingRelatedType( src );
    }

    CTSharedPtr( const CTSharedPtr& src );

    virtual ~CTSharedPtr();

    // implemented inline as a workaround for VC6 issues
    template< class Derived >
    CTSharedPtr& operator=( const CTSharedPtr< Derived, LockType >& src )
    {GUCEF_TRACE;

        if ( &reinterpret_cast< const CTSharedPtr& >( src ) != this )
        {
            CTBasicSharedPtr< T, LockType >::operator=( static_cast< const CTBasicSharedPtr< Derived, LockType >& >( src ) );
        }
        return *this;
    }

    CTSharedPtr& operator=( const CTSharedPtr& src );

    inline bool operator==( int other ) const;

    inline bool operator==( const void* other ) const;

    inline bool operator==( const CTSharedPtr& other ) const;

    inline bool operator!=( int other ) const;

    inline bool operator!=( const void* other ) const;

    inline bool operator!=( const CTSharedPtr& other ) const;

    inline bool operator<( const CTSharedPtr& other ) const;

    inline bool operator<( const void* other ) const;

    /**
     *  operator that implements '(*mySharedPtr)'
     *
     *  @throws ENotInitialized if the pointer is not initialized
     */
    inline T& operator*( void );

    /**
     *  operator that implements '(*mySharedPtr)'
     *
     *  @throws ENotInitialized if the pointer is not initialized
     */
    inline const T& operator*( void ) const;

    /**
     *  operator that emulates 'myPointer->'
     *
     *  @throws ENotInitialized if the pointer is not initialized
     */
    inline T* operator->( void );

    /**
     *  operator that emulates 'myPointer->'
     *
     *  @throws ENotInitialized if the pointer is not initialized
     */
    inline const T* operator->( void ) const;

    /**
     *  Conversion operator to bool to facilitate easy ! etc checks against the
     *  pointer being NULL as some people like to do versus an explicit NULL == check.
     */
    inline operator bool() const;

    inline T* GetPointerAlways( void )
        { return CTBasicSharedPtr< T, LockType >::GetPointerAlways(); }

    inline const T* GetPointerAlways( void ) const
        { return CTBasicSharedPtr< T, LockType >::GetPointerAlways(); }

    // implemented inline as a workaround for VC6 issues
    // The dummy param is a VC6 hack for templated member functions
    template< class Derived >
    CTSharedPtr< Derived, LockType >
    StaticCast( bool dummy = true )
    {
        return CTSharedPtr< Derived, LockType >( *this );
    }

    /**
     *  Creates a clone of the shared pointer
     *  Note that this increases the reference count like any copy would
     */
    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  Templated helper class to facilitate creation of shared pointers associated with an existing object.
 *  If T does not itself inherit from CTBasicSharedPtrCreator then this class will help add that functionality
 *  and thus the benefits of integrated shared pointer creation to the enlarged T footprint as a single object.
 *
 *  Note that CTSharedPtrCreator is intended for when T is a known concrete type, not just a forward declaration.
 *  If you need to work with forward declarations use CTBasicSharedPtrCreator instead.
 *
 *  Note that if constructing T is non-trivial you can use this class directly but otherwise its recommended
 *  to use CTSharedObjCreator instead.
 */
template< typename T, class LockType >
class CTSharedPtrCreator : public CTBasicSharedPtrCreator< T, LockType, T >
{
    public:

    typedef CTBasicSharedPtrCreator< T, LockType, T >                            TBasicSharedPtrCreatorBase;
    typedef typename TBasicSharedPtrCreatorBase::TBasicSharedPtrContainedType    TSharedPtrContainedType;
    typedef typename TBasicSharedPtrCreatorBase::TBasicSharedPtrLockType         TSharedPtrLockType;
    typedef CTSharedPtr< T, LockType >                                           TSharedPtrType;
    typedef TSharedPtrContainedType                                              TypeWhenAllocated;
    typedef CTDynamicDestructor< T >                                             TConcreteTypedDestructor;

    virtual CTSharedPtr< T, LockType > CreateSharedPtr( T* dummyForCppNameMangling = GUCEF_NULL ) const;

    CTSharedPtrCreator( TypeWhenAllocated* originalAddressAsCreated );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

    /**
     *  Non-default move constructor that also takes the original address
     *  Allows you to move the pieces that can be moved while still keeping the correct memory references
     */
    CTSharedPtrCreator( CTSharedPtrCreator&& src                    ,
                        TypeWhenAllocated* originalAddressAsCreated ) GUCEF_NOEXCEPT;

    #endif

    virtual ~CTSharedPtrCreator() GUCEF_VIRTUAL_OVERRIDE;

    protected:

    TConcreteTypedDestructor m_objectDestructor;

    private:

    CTSharedPtrCreator( void ) GUCEF_DELETED_MEMBER;                                       /**< dont use */
    CTSharedPtrCreator( const CTSharedPtrCreator& src ) GUCEF_DELETED_MEMBER;              /**< dont use */
    CTSharedPtrCreator& operator=( const CTSharedPtrCreator& src ) GUCEF_DELETED_MEMBER;   /**< dont use */
};

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
class CTSharedObjCreator : public CTSharedPtrCreator< T, LockType >
{
    public:

    typedef T                                                           TSharedObjTypeAtCreation;
    typedef CTSharedPtrCreator< T, LockType >                           TSharedPtrCreatorBase;
    typedef typename CTSharedPtrCreator< T, LockType >::TSharedPtrType  TSharedPtrType;

    static CTSharedPtr< T, LockType > CreateSharedObj( const T* dummyForCppNameMangling = GUCEF_NULL );

    template < typename Param >
    static CTSharedPtr< T, LockType > CreateSharedObjWithParam( const Param& param );

    CTSharedObjCreator( T* derived );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

    /**
     *  Non-default move constructor that also takes the original address
     *  Allows you to move the pieces that can be moved while still keeping the correct memory references
     */
    CTSharedObjCreator( CTSharedObjCreator&& src, T* derived ) GUCEF_NOEXCEPT;

    #endif

    virtual ~CTSharedObjCreator() GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /**
     *  Specialized copy constructor that also takes the original address
     *  Intended for use by derived classes only in legacy allocator scenarios
     */
    CTSharedObjCreator( const CTSharedObjCreator& src, T* derived );

    private:

    CTSharedObjCreator( void ) GUCEF_DELETED_MEMBER;                                       /**< dont use */
    CTSharedObjCreator( const CTSharedObjCreator& src ) GUCEF_DELETED_MEMBER;              /**< dont use */
    CTSharedObjCreator& operator=( const CTSharedObjCreator& src ) GUCEF_DELETED_MEMBER;   /**< dont use */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedPtr< T, LockType >::CTSharedPtr( void )
        : CTBasicSharedPtr< T, LockType >()
{GUCEF_TRACE;
    // Note that if this constructor is used an assignment is required at
    // a later time to initialize the shared pointer
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedPtr< T, LockType >::CTSharedPtr( const int NULLvalue )
    : CTBasicSharedPtr< T, LockType >()
{GUCEF_TRACE;

    // Note that if this constructor is used an assignment is required at
    // a later time to initialize the shared pointer
    assert( NULLvalue == (int) NULL );
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

template< typename T, class LockType >
CTSharedPtr< T, LockType >::CTSharedPtr( CTSharedPtr&& src ) GUCEF_NOEXCEPT
    : CTBasicSharedPtr< T, LockType >( GUCEF_MOVE( src ) )
{GUCEF_TRACE;

}

#endif

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedPtr< T, LockType >::CTSharedPtr( T* ptr                                                            ,
                                         CTDynamicDestructorBase< T >* objectDestructor /* = GUCEF_NULL */ ,
                                         void* originalAddressAsCreated                 /* = GUCEF_NULL */ )
    : CTBasicSharedPtr< T, LockType >()
{GUCEF_TRACE;

    if ( GUCEF_NULL == originalAddressAsCreated )
        originalAddressAsCreated = ptr;

    if ( GUCEF_NULL == ptr || GUCEF_NULL != objectDestructor )
    {
        CTBasicSharedPtr< T, LockType >::Initialize( ptr, objectDestructor, originalAddressAsCreated );
    }
    else
    {
        CTBasicSharedPtr< T, LockType >::Initialize( ptr, GUCEF_NEW CTDynamicDestructor< T >( true ), originalAddressAsCreated );
    }
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedPtr< T, LockType >::CTSharedPtr( const CTSharedPtr< T, LockType >& src )
    : CTBasicSharedPtr< T, LockType >( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedPtr< T, LockType >::CTSharedPtr( const CTBasicSharedPtr< T, LockType >& src )
    : CTBasicSharedPtr< T, LockType >( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedPtr< T, LockType >::~CTSharedPtr()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedPtr< T, LockType >&
CTSharedPtr< T, LockType >::operator=( const CTSharedPtr< T, LockType >& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CTBasicSharedPtr< T, LockType >::operator=( src );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/
/*
template< class Derived >
CTSharedPtr< T >&
CTSharedPtr< T, LockType >::operator=( const CTSharedPtr< Derived, LockType >& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CTSharedPtr< T, LockType > basePtr( static_cast );

        CTBasicSharedPtr< T, LockType >::operator=( src );
    }
    return *this;
}
*/

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTSharedPtr< T, LockType >::operator<( const CTSharedPtr< T, LockType >& other ) const
{GUCEF_TRACE;

    return static_cast< const CTBasicSharedPtr< T, LockType >& >( *this ) < static_cast< const CTBasicSharedPtr< T, LockType >& >( other );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTSharedPtr< T, LockType >::operator<( const void* other ) const
{GUCEF_TRACE;

    return static_cast< const CTBasicSharedPtr< T, LockType >& >( *this ) < other;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTSharedPtr< T, LockType >::operator==( const CTSharedPtr< T, LockType >& other ) const
{GUCEF_TRACE;

    return static_cast< const CTBasicSharedPtr< T, LockType >& >( *this ) == static_cast< const CTBasicSharedPtr< T, LockType >& >( other );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTSharedPtr< T, LockType >::operator==( const void* other ) const
{GUCEF_TRACE;

    return static_cast< const CTBasicSharedPtr< T, LockType >& >( *this ) == other;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTSharedPtr< T, LockType >::operator==( int other ) const
{GUCEF_TRACE;

    return static_cast< const CTBasicSharedPtr< T, LockType >& >( *this ) == other;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
operator==( const void* ptr, const CTSharedPtr< T, LockType >& other )
{GUCEF_TRACE;

    return static_cast< const CTBasicSharedPtr< T, LockType >& >( other ) == ptr;
}

/*-------------------------------------------------------------------------*/

/*
 *  workaround for comparison to NULL, since NULL is a integer by default
 */
template< typename T, class LockType >
inline bool
operator==( const Int32 ptr, const CTSharedPtr< T, LockType >& other )
{GUCEF_TRACE;

    return static_cast< const CTBasicSharedPtr< T, LockType >& >( other ) == ptr;
}

/*-------------------------------------------------------------------------*/

/*
 *  workaround for comparison to NULL, since NULL is a integer by default
 */
template< typename T, class LockType >
inline bool
operator==( const Int64 ptr, const CTSharedPtr< T, LockType >& other )
{GUCEF_TRACE;

    return static_cast< const CTBasicSharedPtr< T, LockType >& >( other ) == ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTSharedPtr< T, LockType >::operator!=( const CTSharedPtr< T, LockType >& other ) const
{GUCEF_TRACE;

    return static_cast< const CTBasicSharedPtr< T, LockType >& >( *this ) != static_cast< const CTBasicSharedPtr< T, LockType >& >( other );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTSharedPtr< T, LockType >::operator!=( int other ) const
{GUCEF_TRACE;

    return static_cast< const CTBasicSharedPtr< T, LockType >& >( *this ) != other;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTSharedPtr< T, LockType >::operator!=( const void* other ) const
{GUCEF_TRACE;

    return static_cast< const CTBasicSharedPtr< T, LockType >& >( *this ) != other;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
operator!=( const void* ptr, const CTSharedPtr< T, LockType >& other )
{GUCEF_TRACE;

    return other != ptr;
}

/*-------------------------------------------------------------------------*/

/*
 *  workaround for comparison to NULL, since NULL is a integer by default
 */
template< typename T, class LockType >
inline bool
operator!=( const Int32 ptr, const CTSharedPtr< T, LockType >& other )
{GUCEF_TRACE;

    return ptr != static_cast< const CTBasicSharedPtr< T, LockType >& >( other );
}

/*-------------------------------------------------------------------------*/

/*
 *  workaround for comparison to NULL, since NULL is a integer by default
 */
template< typename T, class LockType >
inline bool
operator!=( const Int64 ptr, const CTSharedPtr< T, LockType >& other )
{GUCEF_TRACE;

    return ptr != static_cast< const CTBasicSharedPtr< T, LockType >& >( other );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline T&
CTSharedPtr< T, LockType >::operator*( void )
{GUCEF_TRACE;

    return static_cast< CTBasicSharedPtr< T, LockType >* >( this )->operator*();
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline const T&
CTSharedPtr< T, LockType >::operator*( void ) const
{GUCEF_TRACE;

    return static_cast< const CTBasicSharedPtr< T, LockType >* >( this )->operator*();
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline T*
CTSharedPtr< T, LockType >::operator->( void )
{GUCEF_TRACE;

    return static_cast< CTBasicSharedPtr< T, LockType >* >( this )->operator->();
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline const T*
CTSharedPtr< T, LockType >::operator->( void ) const
{GUCEF_TRACE;

    return static_cast< const CTBasicSharedPtr< T, LockType >* >( this )->operator->();
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline
CTSharedPtr< T, LockType >::operator bool() const
{
    return (*this) != (const void*)0;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CICloneable*
CTSharedPtr< T, LockType >::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CTSharedPtr< T, LockType >( *this );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedPtr< T, LockType >
CTSharedPtrCreator< T, LockType >::CreateSharedPtr( T* dummyForCppNameMangling ) const
{GUCEF_TRACE;

    CTSharedPtr< T, LockType > retVal( static_cast< const CTBasicSharedPtrCreator< T, LockType >* >( this )->CreateBasicSharedPtr( dummyForCppNameMangling ) );
    return retVal;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedPtrCreator< T, LockType >::CTSharedPtrCreator( TypeWhenAllocated* originalAddressAsCreated )
    : CTBasicSharedPtrCreator< T, LockType, T >( originalAddressAsCreated )
    , m_objectDestructor( false )
{GUCEF_TRACE;

    CTBasicSharedPtrCreator< T, LockType, T >::InitializeSharedPtrCreatorDataDestructor( &m_objectDestructor );
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

template< typename T, class LockType >
CTSharedPtrCreator< T, LockType >::CTSharedPtrCreator( CTSharedPtrCreator&& src                    ,
                                                       TypeWhenAllocated* originalAddressAsCreated ) GUCEF_NOEXCEPT
    : CTBasicSharedPtrCreator< T, LockType, T >( GUCEF_MOVE( src ), originalAddressAsCreated, GUCEF_NULL )
    , m_objectDestructor( false )
{GUCEF_TRACE;

    try
    {
        CTBasicSharedPtrCreator< T, LockType, T >::InitializeSharedPtrCreatorDataDestructor( &m_objectDestructor );
    }
    catch ( ... )
    {
        // if we failed to init the destructor the shared pointer code itself will refuse to issue new references
        // you will get null'd shared pointers from this object (which you should already be checking for anyway)
        // but at least we avoid memory leaks which are harder to track down
    }
}

#endif

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedPtrCreator< T, LockType >::~CTSharedPtrCreator( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( TBasicSharedPtrCreatorBase::GetBasicSharedPtrData() );

    // Sanity check to ensure no active references remain
    // Such references would be referencing the very object that is being destroyed here
    GUCEF_ASSERT( 0 == TBasicSharedPtrCreatorBase::GetBasicSharedPtrData().m_refCounter );
    if ( 0 != TBasicSharedPtrCreatorBase::GetBasicSharedPtrData().m_refCounter )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CTSharedPtrCreator destructor called while there are still active references to the object" );

        // To avoid invalid memory access we sever the link to the destructor
        // we should still never get here in the first place, but best to minimize damage
        TBasicSharedPtrCreatorBase::OverrideSharedPtrCreatorData( GUCEF_NULL, GUCEF_NULL );
    }
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedPtr< T, LockType >
CTSharedObjCreator< T, LockType >::CreateSharedObj( const T* dummyForCppNameMangling )
{GUCEF_TRACE;

    T* obj = GUCEF_NULL;
    try
    {
        GUCEF_ASSERT( GUCEF_NULL == dummyForCppNameMangling );
        CTSharedPtr< T, LockType > retVal( static_cast< CTBasicSharedPtrCreator< T, LockType, T >* >( ( GUCEF_NEW T() ) )->CreateBasicSharedPtr() );
        return retVal;
    }
    catch ( const std::exception& )
    {
        GUCEF_DELETE obj;
        obj = GUCEF_NULL;

        return CTSharedPtr< T, LockType >();
    }
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
template < typename Param >
CTSharedPtr< T, LockType >
CTSharedObjCreator< T, LockType >::CreateSharedObjWithParam( const Param& param )
{GUCEF_TRACE;

    T* obj = GUCEF_NULL;
    try
    {
        GUCEF_ASSERT( GUCEF_NULL != &param );
        CTSharedPtr< T, LockType > retVal( static_cast< CTBasicSharedPtrCreator< T, LockType, T >* >( ( GUCEF_NEW T( param ) ) )->CreateBasicSharedPtr() );
        return retVal;
    }
    catch ( const std::exception& )
    {
        GUCEF_DELETE obj;
        obj = GUCEF_NULL;

        return CTSharedPtr< T, LockType >();
    }
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedObjCreator< T, LockType >::CTSharedObjCreator( T* derived )
    : CTSharedPtrCreator< T, LockType >( derived )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedObjCreator< T, LockType >::CTSharedObjCreator( const CTSharedObjCreator& src, T* derived )
    : CTSharedPtrCreator< T, LockType >( derived )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

template< typename T, class LockType >
CTSharedObjCreator< T, LockType >::CTSharedObjCreator( CTSharedObjCreator&& src, T* derived ) GUCEF_NOEXCEPT
    : CTSharedPtrCreator< T, LockType >( GUCEF_MOVE( src ), derived )
{GUCEF_TRACE;

}

#endif /* GUCEF_MOVE_SEMANTICS_SUPPORTED ? */

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedObjCreator< T, LockType >::~CTSharedObjCreator( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */
