/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CTWEAKPTR_H
#define GUCEF_CORE_CTWEAKPTR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  Templated implementation of a weak pointer to an object held by CTSharedPtr / CTBasicSharedPtr.
 *
 *  A weak pointer refers to the object without keeping it alive: the object goes when the last shared
 *  pointer goes, and from then on the weak pointer is expired. To use the object, get a shared pointer
 *  with GetSharedPtr(): it is NULL when the object is gone, else it keeps the object alive while it is
 *  used. Use it for lists of objects owned by others (a driver's contexts, observers, caches) where
 *  holding a shared pointer would keep objects alive past the wishes of their owners.
 *
 *  Only objects whose shared data is allocated on its own (shared pointers made from a raw pointer)
 *  can be referred to weakly. Objects that carry the shared data inside themselves
 *  (CTBasicSharedPtrCreator, CTSharedObjCreator) cannot: the data goes with the object, so a weak
 *  pointer made from such a shared pointer stays NULL (and asserts in debug builds).
 *
 *  Thread safety follows the LockType of the shared pointers, as for CTBasicSharedPtr.
 *
 *  @TODO: use std::weak_ptr internally where available, together with std::shared_ptr for CTSharedPtr,
 *         while keeping the capabilities of CTSharedPtr (dynamic destructors, shared data that is part
 *         of the object, the selectable LockType)
 */
template< typename T, class LockType >
class CTWeakPtr
{
    public:

    typedef T                                         TContainedType;
    typedef LockType                                  TLockType;
    typedef TBasicSharedPtrSharedData< LockType >     TSharedData;

    CTWeakPtr( void );

    CTWeakPtr( const CTWeakPtr& src );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
    CTWeakPtr( CTWeakPtr&& src ) GUCEF_NOEXCEPT;
    #endif

    // inlined, has to be inlined in class definition for now due to VC6 limitations
    template< class Derived >
    CTWeakPtr( const CTBasicSharedPtr< Derived, LockType >& src )
        : m_shared( GUCEF_NULL )
        , m_ptr( GUCEF_NULL )
    {GUCEF_TRACE;

        InitializeUsingRelatedType( src );
    }

    ~CTWeakPtr();

    CTWeakPtr& operator=( const CTWeakPtr& src );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
    CTWeakPtr& operator=( CTWeakPtr&& src ) GUCEF_NOEXCEPT;
    #endif

    // implemented inline as a workaround for VC6 issues
    template< class Derived >
    CTWeakPtr& operator=( const CTBasicSharedPtr< Derived, LockType >& src )
    {GUCEF_TRACE;

        InitializeUsingRelatedType( src );
        return *this;
    }

    /** Refers weakly to the object of the shared pointer; false when it cannot (see the class description) */
    template< class RelatedClass >
    bool InitializeUsingRelatedType( const CTBasicSharedPtr< RelatedClass, LockType >& other )
    {GUCEF_TRACE;

        Unlink();

        const TSharedData* otherShared = other.GetSharedData();
        if ( GUCEF_NULL == otherShared )
            return false;

        MT::CObjectScopeLock lock( otherShared );

        // The static cast below is performed as a compile time validation of the type passed.
        T* relatedClassAsT = static_cast< T* >( const_cast< RelatedClass* >( other.GetPointerAlways() ) );
        if ( GUCEF_NULL == relatedClassAsT || otherShared->m_refCounter <= 0 )
            return false;
        if ( !otherShared->m_hasIndependentLifeCycle )
        {
            // The shared data is part of the object and goes with it, nothing could tell a weak pointer the object is gone
            GUCEF_ASSERT_ALWAYS;
            return false;
        }

        m_shared = const_cast< TSharedData* >( otherShared );
        m_ptr = relatedClassAsT;
        ++(m_shared->m_weakCounter);
        return true;
    }

    /**
     *  A shared pointer to the object, which keeps it alive while it is held; NULL when the object is gone
     */
    CTSharedPtr< T, LockType > GetSharedPtr( void ) const;

    /** Whether the object is gone (or there never was one) */
    bool IsExpired( void ) const;

    /** Stops referring to the object */
    void Unlink( void );

    /** The object, without keeping it alive; only for comparisons and as a key, never to use the object */
    const T* GetPointerForComparison( void ) const;

    inline bool operator==( const CTWeakPtr& other ) const;

    inline bool operator!=( const CTWeakPtr& other ) const;

    inline bool operator<( const CTWeakPtr& other ) const;

    private:

    TSharedData* m_shared;
    T* m_ptr;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTWeakPtr< T, LockType >::CTWeakPtr( void )
    : m_shared( GUCEF_NULL )
    , m_ptr( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTWeakPtr< T, LockType >::CTWeakPtr( const CTWeakPtr& src )
    : m_shared( GUCEF_NULL )
    , m_ptr( GUCEF_NULL )
{GUCEF_TRACE;

    *this = src;
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

template< typename T, class LockType >
CTWeakPtr< T, LockType >::CTWeakPtr( CTWeakPtr&& src ) GUCEF_NOEXCEPT
    : m_shared( src.m_shared )
    , m_ptr( src.m_ptr )
{GUCEF_TRACE;

    // the weak reference moves along, the count stays the same
    src.m_shared = GUCEF_NULL;
    src.m_ptr = GUCEF_NULL;
}

#endif /* GUCEF_MOVE_SEMANTICS_SUPPORTED ? */

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTWeakPtr< T, LockType >::~CTWeakPtr()
{GUCEF_TRACE;

    Unlink();
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTWeakPtr< T, LockType >&
CTWeakPtr< T, LockType >::operator=( const CTWeakPtr& src )
{GUCEF_TRACE;

    if ( &src == this || src.m_shared == m_shared )
        return *this;

    Unlink();
    if ( GUCEF_NULL != src.m_shared )
    {
        MT::CObjectScopeLock lock( src.m_shared );
        m_shared = src.m_shared;
        m_ptr = src.m_ptr;
        ++(m_shared->m_weakCounter);
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

template< typename T, class LockType >
CTWeakPtr< T, LockType >&
CTWeakPtr< T, LockType >::operator=( CTWeakPtr&& src ) GUCEF_NOEXCEPT
{GUCEF_TRACE;

    if ( &src != this )
    {
        Unlink();
        m_shared = src.m_shared;
        m_ptr = src.m_ptr;
        src.m_shared = GUCEF_NULL;
        src.m_ptr = GUCEF_NULL;
    }
    return *this;
}

#endif /* GUCEF_MOVE_SEMANTICS_SUPPORTED ? */

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTSharedPtr< T, LockType >
CTWeakPtr< T, LockType >::GetSharedPtr( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_shared )
        return CTSharedPtr< T, LockType >();

    // a new strong reference, taken under the lock so the object cannot go in the meantime
    CTSharedPtr< T, LockType > result;
    MT::CObjectScopeLock lock( m_shared );
    if ( m_shared->m_refCounter <= 0 || GUCEF_NULL == m_shared->m_voidDestructor )
        return result;
    ++(m_shared->m_refCounter);
    CTBasicSharedPtr< T, LockType >& resultBase = result;
    resultBase.m_shared = m_shared;
    resultBase.m_ptr = m_ptr;
    return result;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
bool
CTWeakPtr< T, LockType >::IsExpired( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_shared )
        return true;
    MT::CObjectScopeLock lock( m_shared );
    return m_shared->m_refCounter <= 0 || GUCEF_NULL == m_shared->m_voidDestructor;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
void
CTWeakPtr< T, LockType >::Unlink( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_shared )
        return;

    TSharedData* shared = m_shared;
    m_shared = GUCEF_NULL;
    m_ptr = GUCEF_NULL;

    // We cannot use CObjectScopeLock here because we may have to delete the lock itself
    shared->m_lock.Lock();
    --(shared->m_weakCounter);
    bool lastReference = shared->m_weakCounter <= 0 && shared->m_refCounter <= 0;
    shared->m_lock.Unlock();

    // the object is gone and nothing refers to the shared data any more
    if ( lastReference )
        GUCEF_DELETE shared;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
const T*
CTWeakPtr< T, LockType >::GetPointerForComparison( void ) const
{GUCEF_TRACE;

    return m_ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTWeakPtr< T, LockType >::operator==( const CTWeakPtr& other ) const
{GUCEF_TRACE;

    return m_shared == other.m_shared;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTWeakPtr< T, LockType >::operator!=( const CTWeakPtr& other ) const
{GUCEF_TRACE;

    return m_shared != other.m_shared;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTWeakPtr< T, LockType >::operator<( const CTWeakPtr& other ) const
{GUCEF_TRACE;

    return m_shared < other.m_shared;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTWEAKPTR_H ? */
