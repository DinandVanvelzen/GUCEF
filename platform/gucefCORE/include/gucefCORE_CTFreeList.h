/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2025.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CTFREELIST_H
#define GUCEF_CORE_CTFREELIST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CTBASICSHAREDPTR_H
#include "CTBasicSharedPtr.h"
#define GUCEF_CORE_CTBASICSHAREDPTR_H
#endif /* GUCEF_CORE_CTBASICSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CTDYNAMICDESTRUCTORBASE_H
#include "CTDynamicDestructorBase.h"
#define GUCEF_CORE_CTDYNAMICDESTRUCTORBASE_H
#endif /* GUCEF_CORE_CTDYNAMICDESTRUCTORBASE_H ? */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#include "gucefMT_CObjectScopeReadOnlyLock.h"
#define GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#endif /* GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H ? */

#ifndef GUCEF_CORE_SFINAE_UTILS_H
#include "gucefCORE_SFINAE_utils.h"
#define GUCEF_CORE_SFINAE_UTILS_H
#endif /* GUCEF_CORE_SFINAE_UTILS_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      FORWARD DECLARATIONS                                               //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T, class LockType >
class CTFreeList;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Destructor hook used with CTBasicSharedPtr. Instead of deleting the object
 *  it will move it to the owning free list for later reuse.
 */
template< typename T, class LockType >
class CTFreeListSharedObjectDestructor : public CTDynamicDestructorBase< CTBasicSharedPtrCreatorAccessor< T, LockType > >
{
    public:
    typedef T                                                                   TContainedType;
    typedef LockType                                                            TLockType;
    typedef CTFreeList< T, LockType >                                           TOwnerFreeListType;
    typedef CTBasicSharedPtr< T, LockType >                                     TSharedPtr;
    typedef CTBasicSharedPtrCreatorAccessor< T, LockType >                      TSharedPtrCreator;

    CTFreeListSharedObjectDestructor( CTFreeList< T, LockType >* owner )
        : CTDynamicDestructorBase< CTBasicSharedPtrCreatorAccessor< T, LockType > >()
        , m_owner( owner )
    {GUCEF_TRACE;}

    virtual ~CTFreeListSharedObjectDestructor()
    {GUCEF_TRACE;
    }

    virtual void DestroyObject( TSharedPtrCreator* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_owner && GUCEF_NULL != objectToBeDestroyed )
            m_owner->OnObjectDestruction( objectToBeDestroyed );
    }

    private:
    CTFreeListSharedObjectDestructor( const CTFreeListSharedObjectDestructor& );
    CTFreeListSharedObjectDestructor& operator=( const CTFreeListSharedObjectDestructor& );

    CTFreeList< T, LockType >* m_owner;
};

/*-------------------------------------------------------------------------*/

/**
 *  Basic free list / object pool implementation integrated with CTBasicSharedPtr.
 *
 *  Lifecycle states tracked:
 *    - Active: objects currently in active use (shared pointers handed out by Acquire())
 *    - Dormant: objects flagged by external logic as no longer active but still referenced
 *               (you can move between Active and Dormant using MarkDormant/MarkActive)
 *    - Freed: objects whose shared pointer reference count reached zero. Instead of deleting
 *             the memory they are put in the free list for reuse. Their destructor has been
 *             called so the raw memory block contains an unconstructed object which can be
 *             reconstructed with placement new when re-acquired.
 *
 *  Thread safety behavior is dependent on the LockType template argument identical to
 *  CTBasicSharedPtr semantics.
 */
template< typename T, class LockType >
class CTFreeList : public MT::CILockable
{
    public:

    typedef T                                                                   TContainedType;
    typedef LockType                                                            TLockType;
    typedef CTBasicSharedPtr< T, LockType >                                     TSharedPtr;
    typedef CTBasicSharedPtrCreatorAccessor< T, LockType >                      TSharedPtrCreator;

    enum TReusePolicy
    {
        REUSE_RECONSTRUCT_OBJECT,
        REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE
    };

    CTFreeList( TReusePolicy reusePolicy = REUSE_RECONSTRUCT_OBJECT );

    virtual ~CTFreeList() GUCEF_VIRTUAL_OVERRIDE;

    void Clear( void );

    TReusePolicy GetReusePolicy( void ) const;

    UInt32 GetActiveCount( void ) const;
    UInt32 GetDormantCount( void ) const;
    UInt32 GetFreedCount( void ) const;
    UInt32 GetTotalManagedCount( void ) const;

    template< typename U >
    struct has_clear
    {
        typedef char yes[1];
        typedef char no[2];
        template< typename V, void (V::*)() > struct S { };
        template< typename V > static yes& test( S<V, &V::Clear>* );
        template< typename V > static no&  test( ... );
        enum { value = sizeof( test<U>(0) ) == sizeof( yes ) };
    };

    TSharedPtr Acquire( void );

    bool IncreaseReserve( void );

    bool Reserve( UInt32 count );

    UInt16 GetBlockSize( void ) const;

    bool MarkDormant( const TSharedPtr& sp );

    bool MarkActive( const TSharedPtr& sp );

    /**     
     *  Checks whether the given shared pointer is currently in the active state
     *  Note that this is a snapshot in time only as the state may change immediately after the call
     */
    bool IsActive( const TSharedPtr& sp ) const;

    /**     
     *  Checks whether the given shared pointer is currently in the dormant state
     *  Note that this is a snapshot in time only as the state may change immediately after the call
     */
    bool IsDormant( const TSharedPtr& sp ) const;

    //void PurgeFreed( void )
    //{GUCEF_TRACE;
    //    MT::CObjectScopeLock lock( &m_lock );
    //    typename GUCEF::vector< T* >::iterator i = m_freed.begin();
    //    while ( i != m_freed.end() )
    //    {
    //        if ( m_blockSlots.find( *i ) == m_blockSlots.end() )
    //        {
    //            GUCEF_DELETE *i;
    //        }
    //        ++i;
    //    }
    //    m_freed.clear();
    //}

    //void PurgeBlocks( void )
    //{GUCEF_TRACE;
    //    MT::CObjectScopeLock lock( &m_lock );
    //    typename GUCEF::vector< void* >::iterator b = m_blocks.begin();
    //    while ( b != m_blocks.end() )
    //    {
    //        ::operator delete( *b );
    //        ++b;
    //    }
    //    m_blocks.clear();

    //    if ( !m_freed.empty() )
    //    {
    //        GUCEF::vector< T* > remaining;
    //        remaining.reserve( m_freed.size() );
    //        typename GUCEF::vector< T* >::iterator i = m_freed.begin();
    //        while ( i != m_freed.end() )
    //        {
    //            if ( m_blockSlots.find( *i ) == m_blockSlots.end() )
    //                remaining.push_back( *i );
    //            ++i;
    //        }
    //        m_freed.swap( remaining );
    //    }
    //}

    void OnObjectDestruction( TSharedPtrCreator* object );

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CTFreeListSharedObjectDestructor< T, LockType >     TInternalDestructor;
    typedef gucef_allocator< TSharedPtrCreator >                TAllocator;

    TAllocator m_allocator;

    template< typename U >
    void InvokeClearIfAvailableImpl( U* obj, typename EnableIf< has_clear<U>::value, int >::type = 0 )
    {GUCEF_TRACE;

        obj->Clear();
    }

    template< typename U >
    void InvokeClearIfAvailableImpl( U* obj, typename EnableIf< !has_clear<U>::value, int >::type = 0 )
    {GUCEF_TRACE;

        // There is no clear so we revert back to using the destructor
        m_allocator.destroy( obj );
    }

    void InvokeClearIfAvailable( TSharedPtrCreator* obj )
    {GUCEF_TRACE;

        InvokeClearIfAvailableImpl< TSharedPtrCreator >( obj );
    }

    template< typename U >
    void ReconstructIfNoClearIsAvailableImpl( U* /* obj */, typename EnableIf< has_clear<U>::value, int >::type = 0 )
    {GUCEF_TRACE;

        // If the object was cleared using Clear() there is no need to reconstruct it
        // This is only needed for types that do not have a Clear() method
    }

    template< typename U >
    void ReconstructIfNoClearIsAvailableImpl( U* obj, typename EnableIf< !has_clear<U>::value, int >::type = 0 )
    {GUCEF_TRACE;

        // There is no clear so we revert back to using the constructor
        // Always use placement new to (re)init the object
        m_allocator.construct( obj );
        obj->InitializeSharedPtrCreatorData( obj, &m_destructor );
    }

    void ReconstructIfNoClearIsAvailable( TSharedPtrCreator* obj )
    {GUCEF_TRACE;

        ReconstructIfNoClearIsAvailableImpl< TSharedPtrCreator >( obj );
    }

    CTFreeList( const CTFreeList& ) GUCEF_DELETED_MEMBER;
    CTFreeList& operator=( const CTFreeList& ) GUCEF_DELETED_MEMBER;

    private:

    GUCEF::set< TSharedPtrCreator* >   m_active;
    GUCEF::set< TSharedPtrCreator* >   m_dormant;
    GUCEF::vector< TSharedPtrCreator* > m_freed;
    GUCEF::vector< void* > m_blocks;
    TInternalDestructor m_destructor;
    TReusePolicy m_reusePolicy;
    UInt8 m_growSizePercentage;
    UInt16 m_blockSize;
    mutable LockType m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTFreeList< T, LockType >::CTFreeList( TReusePolicy reusePolicy )
    : MT::CILockable()
    , m_allocator()
    , m_active()
    , m_dormant()
    , m_freed()
    , m_blocks()
    , m_destructor( this )
    , m_reusePolicy( reusePolicy )
    , m_growSizePercentage( 50 )
    , m_blockSize( 100 )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTFreeList< T, LockType >::~CTFreeList()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
void
CTFreeList< T, LockType >::Clear( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );

    typename GUCEF::set< TSharedPtrCreator* >::iterator i = m_active.begin();
    while ( i != m_active.end() )
    {
        TSharedPtrCreator* obj = (*i);
        m_allocator.destroy( obj );
        ++i;
    }
    m_active.clear();

    i = m_dormant.begin();
    while ( i != m_dormant.end() )
    {
        TSharedPtrCreator* obj = (*i);
        m_allocator.destroy( obj );
        ++i;
    }
    m_dormant.clear();

    typename GUCEF::vector< TSharedPtrCreator* >::iterator n = m_freed.begin();
    while ( n != m_freed.end() )
    {
        TSharedPtrCreator* obj = (*n);
        m_allocator.destroy( obj );
        ++n;
    }
    m_freed.clear();
    
    typename GUCEF::vector< void* >::iterator b = m_blocks.begin();
    while ( b != m_blocks.end() )
    {
        GUCEF_FREE( *b );
        ++b;
    }
    m_blocks.clear();
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
typename CTFreeList< T, LockType >::TSharedPtr
CTFreeList< T, LockType >::Acquire( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );

    TSharedPtrCreator* raw = GUCEF_NULL;
    if ( !m_freed.empty() )
    {
        raw = m_freed.back();
        m_freed.pop_back();
        if ( GUCEF_NULL != raw )
        {
            switch ( m_reusePolicy )
            {                
                case REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE:                
                {
                    // Depending on whether the object has a Clear() method we perform a placement new
                    // If it has a Clear() we do not need to reconstruct the object since its using a higher order
                    // reset method with presumably better performance characteristics via better reuse of internal resources
                    ReconstructIfNoClearIsAvailable( raw );
                    break;
                }
                
                case REUSE_RECONSTRUCT_OBJECT:
                default:
                {
                    // Always use placement new to (re)init the object
                    m_allocator.construct( raw );
                    raw->InitializeSharedPtrCreatorData( raw, &m_destructor );
                    break;
                }
            }
        }
    }
    else
    {
        // We have no freed objects available so we need to allocate additional ones
        if ( IncreaseReserve() )
            return Acquire();
        return TSharedPtr();
    }

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == raw )
        return TSharedPtr();                                                                                                                                                                                                                                                      

    // Important: Always create the shared pointer using the raw pointer's CreateBasicSharedPtr method to ensure
    //            that the shared pointer is properly linked to the internal shared data of the raw object
    //            We dont want multiple independent shared data instances for the same object
    TSharedPtr sp( GUCEF_MOVE( raw->CreateBasicSharedPtr() ) );
    m_active.insert( raw );
    return sp;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
bool
CTFreeList< T, LockType >::IncreaseReserve( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    if ( 0 != m_growSizePercentage ) // disable automatic growth when percentage is zero
    {
        UInt32 currentSize = GetTotalManagedCount();
        if ( 0 != currentSize )
        {
            UInt32 increaseBy = (currentSize * m_growSizePercentage) / 100;
            return Reserve( currentSize + increaseBy );
        }
        else
        {
            return Reserve( m_blockSize );
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
bool
CTFreeList< T, LockType >::Reserve( UInt32 count )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );

    UInt32 currentSize = GetTotalManagedCount();
    if ( count < currentSize )
        return false; // Cannot shrink the pool using this method

    UInt32 shortage = count - currentSize;
    if ( shortage == 0 )
        return true; // Already at desired size

    UInt32 nrOfBlocksToAdd = (shortage + m_blockSize - 1) / m_blockSize;
    for ( UInt32 i=0; i<nrOfBlocksToAdd; ++i )
    {
        void* base = GUCEF_CALLOC( m_blockSize, sizeof( TSharedPtrCreator ) );
        if ( GUCEF_NULL == base )
            return false;

        m_blocks.push_back( base );
        char* cursor = static_cast< char* >( base );
        for ( UInt32 n=0; n<m_blockSize; ++n )
        {
            TSharedPtrCreator* slot = reinterpret_cast< TSharedPtrCreator* >( cursor );

            // Use placement new to do the very first construction always
            // Reuse policy only applies to subsequent reuses
            m_allocator.construct( slot );
            slot->InitializeSharedPtrCreatorData( slot, &m_destructor );

            m_freed.push_back( slot );

            cursor += sizeof( TSharedPtrCreator );
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
bool
CTFreeList< T, LockType >::IsActive( const TSharedPtr& sp ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( &m_lock );
    bool isActive = m_active.find( sp.GetPointerAlways() ) != m_active.end();
    lock.EarlyUnlock();
    return isActive;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
bool
CTFreeList< T, LockType >::IsDormant( const TSharedPtr& sp ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( &m_lock );
    bool isDormant = m_dormant.find( sp.GetPointerAlways() ) != m_dormant.end();
    lock.EarlyUnlock();
    return isDormant;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
UInt16
CTFreeList< T, LockType >::GetBlockSize( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( &m_lock );
    UInt16 blockSize = m_blockSize;
    lock.EarlyReaderUnlock();
    return blockSize;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
UInt32
CTFreeList< T, LockType >::GetActiveCount( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( &m_lock );
    UInt32 active = (UInt32) m_active.size();
    lock.EarlyReaderUnlock();
    return active;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
UInt32
CTFreeList< T, LockType >::GetDormantCount( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( &m_lock );
    UInt32 dormant = (UInt32) m_dormant.size();
    lock.EarlyReaderUnlock();
    return dormant;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
UInt32
CTFreeList< T, LockType >::GetFreedCount( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( &m_lock );
    UInt32 freed = (UInt32) m_freed.size();
    lock.EarlyReaderUnlock();
    return freed;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
UInt32
CTFreeList< T, LockType >::GetTotalManagedCount( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( &m_lock );
    UInt32 total = (UInt32) ( m_active.size() + m_dormant.size() + m_freed.size() );
    lock.EarlyReaderUnlock();
    return total;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
bool
CTFreeList< T, LockType >::MarkDormant( const TSharedPtr& sp )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );

    TSharedPtrCreator* raw = static_cast< TSharedPtrCreator* >( const_cast< T* >( sp.GetPointerAlways() ) );
    if ( GUCEF_NULL == raw )
        return false;

    typename GUCEF::set< TSharedPtrCreator* >::iterator i = m_active.find( raw );
    if ( i != m_active.end() )
    {
        m_active.erase( i );
        m_dormant.insert( raw );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
bool
CTFreeList< T, LockType >::MarkActive( const TSharedPtr& sp )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );

    TSharedPtrCreator* raw = static_cast< TSharedPtrCreator* >( const_cast< T* >( sp.GetPointerAlways() ) );
    if ( GUCEF_NULL == raw )
        return false;

    typename GUCEF::set< TSharedPtrCreator* >::iterator i = m_dormant.find( raw );
    if ( i != m_dormant.end() )
    {
        m_dormant.erase( i );
        m_active.insert( raw );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
void
CTFreeList< T, LockType >::OnObjectDestruction( TSharedPtrCreator* object )
{GUCEF_TRACE;

    if ( GUCEF_NULL == object )
        return;

    MT::CObjectScopeLock lock( &m_lock );

    bool wasFound = false;
    typename GUCEF::set< TSharedPtrCreator* >::iterator ia = m_active.find( object );
    if ( ia != m_active.end() )
    {
        m_active.erase( ia );
        wasFound = true;
    }
    else
    {
        typename GUCEF::set< TSharedPtrCreator* >::iterator id = m_dormant.find( object );
        if ( id != m_dormant.end() )
        {
            m_dormant.erase( id );
            wasFound = true;
        }
    }

    if ( wasFound )
    {
        switch ( m_reusePolicy )
        {                
            case REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE:                
            {
                // Prefer Clear() if available (per SFINAE), if not fallback to destructor
                // If it has a Clear() the intent is that we do not need to reconstruct the object since its using a higher order
                // reset method with presumably better performance characteristics via better reuse of internal resources
                InvokeClearIfAvailable( object );
                break;
            }
                
            case REUSE_RECONSTRUCT_OBJECT:
            default:
            {
                // Always use destructor to reset the object
                m_allocator.destroy( object );
                break;
            }
        }

        m_freed.push_back( object );
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "TFreeList<" + CORE::ToString< T >() + ">::OnObjectDestruction: Object being destroyed " + ToString( (void*) object ) + " was not registered as active or dormant in the free list" );
    }    
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
typename CTFreeList< T, LockType >::TReusePolicy
CTFreeList< T, LockType >::GetReusePolicy( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( &m_lock );
    CTFreeList< T, LockType >::TReusePolicy policy = m_reusePolicy;
    lock.EarlyUnlock();
    return policy;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
MT::TLockStatus
CTFreeList< T, LockType >::Lock( UInt32 lockWaitTimeoutInMs ) const 
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
MT::TLockStatus
CTFreeList< T, LockType >::Unlock( void ) const 
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
const MT::CILockable*
CTFreeList< T, LockType >::AsLockable( void ) const
{GUCEF_TRACE;

    return &m_lock;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTFREELIST_H ? */
