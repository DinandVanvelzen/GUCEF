/*
 *  gucefMT: GUCEF module providing multithreading solutions
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

#ifndef GUCEF_MT_CTATOMIC_H
#define GUCEF_MT_CTATOMIC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stddef.h>

#ifndef GUCEF_MT_ATOMIC_CONFIG_H
#include "gucefMT_atomic_config.h"      /* backend detection */
#define GUCEF_MT_ATOMIC_CONFIG_H
#endif /* GUCEF_MT_ATOMIC_CONFIG_H ? */

#ifndef GUCEF_MT_ATOMIC_BACKENDS_H
#include "gucefMT_atomic_backends.h"    /* the backends */
#define GUCEF_MT_ATOMIC_BACKENDS_H
#endif /* GUCEF_MT_ATOMIC_BACKENDS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The backend used when none is named: the default backend of the platform, except that
 *  8 byte atomics fall back to the lock backend where the default has no 64 bit support.
 */
template< size_t N, typename Tag >
struct TAtomicResolveTag
{
    typedef Tag type;
};

template< size_t N >
struct TAtomicResolveTag< N, void >
{
    typedef GUCEF_MT_ATOMIC_DEFAULT_BACKEND_TAG type;
};

#ifndef GUCEF_MT_ATOMIC_DEFAULT_HAS_64BIT
template<>
struct TAtomicResolveTag< 8, void >
{
    typedef SAtomicBackendLock type;
};
#endif

/** Types whose bit pattern is not their value cannot go through the integer carriers: rejected at compile time */
template< typename T > struct TAtomicIsFloatingPoint            { enum { value = 0 }; };
template<> struct TAtomicIsFloatingPoint< float >               { enum { value = 1 }; };
template<> struct TAtomicIsFloatingPoint< double >              { enum { value = 1 }; };
template<> struct TAtomicIsFloatingPoint< long double >         { enum { value = 1 }; };

/** Size of what a pointer points at, for pointer arithmetic. void and function pointers step in bytes */
template< typename T > struct TAtomicPointeeSize                { enum { value = sizeof( T ) }; };
template<> struct TAtomicPointeeSize< void >                    { enum { value = 1 }; };
template<> struct TAtomicPointeeSize< const void >              { enum { value = 1 }; };
template<> struct TAtomicPointeeSize< volatile void >           { enum { value = 1 }; };
template<> struct TAtomicPointeeSize< const volatile void >     { enum { value = 1 }; };

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An atomic value: every operation is indivisible and visible to all threads according to the
 *  requested memory order, which defaults to the strongest (sequentially consistent) so the simple
 *  use is always right and the relaxed / acquire / release forms are there for code that needs the speed.
 *
 *  T can be any integer type (1, 2, 4 or 8 bytes), bool, an enumeration or a pointer (see below).
 *  The backend is chosen per platform from what the compiler offers (see gucefMT_atomic_config.h);
 *  name a backend tag as second parameter to select one explicitly.
 *
 *  An atomic is neither copyable nor assignable from another atomic: copying two values is not one
 *  indivisible operation. Read the value with Load() and write it with Store().
 *
 *  Example, a counter shared between threads:
 *      CTAtomic< UInt32 > count( 0 );
 *      count.FetchAdd( 1, MEMORDER_RELAXED );
 *
 *  Example, publishing data with release / acquire:
 *      // writer                                      // reader
 *      data = 42;                                     if ( ready.Load( MEMORDER_ACQUIRE ) )
 *      ready.Store( true, MEMORDER_RELEASE );             use( data );
 */
template< typename T, typename Tag = void >
class CTAtomic
{
    public:

    typedef T                                                                   TValue;
    typedef typename TAtomicCarrier< sizeof( T ) >::type                        TCarrier;
    typedef typename TAtomicResolveTag< sizeof( T ), Tag >::type                TBackendTag;
    typedef CTAtomicBackend< TCarrier, TBackendTag >                            TBackend;

    CTAtomic( void ) : m_storage() { TBackend::Init( m_storage, ToCarrier( T() ) ); }
    explicit CTAtomic( T value ) : m_storage() { TBackend::Init( m_storage, ToCarrier( value ) ); }

    /** Reads the value */
    T Load( TMemoryOrder order = MEMORDER_SEQ_CST ) const                           { return FromCarrier( TBackend::Load( m_storage, AtomicLoadOrder( order ) ) ); }

    /** Writes the value */
    void Store( T value, TMemoryOrder order = MEMORDER_SEQ_CST )                    { TBackend::Store( m_storage, ToCarrier( value ), AtomicStoreOrder( order ) ); }

    /** Writes the value and returns the value that was there */
    T Exchange( T value, TMemoryOrder order = MEMORDER_SEQ_CST )                    { return FromCarrier( TBackend::Exchange( m_storage, ToCarrier( value ), order ) ); }

    /**
     *  If the value equals 'expected' it becomes 'desired' and true is returned. Otherwise 'expected' is set
     *  to the value that was found and false is returned. The weak form may fail spuriously (even when the
     *  values were equal) and is meant for retry loops, where it can be cheaper on some processors.
     *  The failure order can never be stronger than the success order, a stronger request is lowered.
     */
    bool CompareExchangeStrong( T& expected, T desired, TMemoryOrder success = MEMORDER_SEQ_CST, TMemoryOrder failure = MEMORDER_SEQ_CST )
    {
        return CompareExchange( expected, desired, success, failure, false );
    }

    bool CompareExchangeWeak( T& expected, T desired, TMemoryOrder success = MEMORDER_SEQ_CST, TMemoryOrder failure = MEMORDER_SEQ_CST )
    {
        return CompareExchange( expected, desired, success, failure, true );
    }

    /** Arithmetic and bit operations. All return the value from before the operation */
    T FetchAdd( T value, TMemoryOrder order = MEMORDER_SEQ_CST )                    { return FromCarrier( TBackend::FetchAdd( m_storage, ToCarrier( value ), order ) ); }
    T FetchSub( T value, TMemoryOrder order = MEMORDER_SEQ_CST )                    { return FromCarrier( TBackend::FetchSub( m_storage, ToCarrier( value ), order ) ); }
    T FetchAnd( T value, TMemoryOrder order = MEMORDER_SEQ_CST )                    { return FromCarrier( TBackend::FetchAnd( m_storage, ToCarrier( value ), order ) ); }
    T FetchOr ( T value, TMemoryOrder order = MEMORDER_SEQ_CST )                    { return FromCarrier( TBackend::FetchOr ( m_storage, ToCarrier( value ), order ) ); }
    T FetchXor( T value, TMemoryOrder order = MEMORDER_SEQ_CST )                    { return FromCarrier( TBackend::FetchXor( m_storage, ToCarrier( value ), order ) ); }

    /** True if the operations never take a lock. Only the lock backend (a last resort) says false */
    bool IsLockFree( void ) const                                                   { return TBackend::IsLockFree( m_storage ); }

    /* Sequentially consistent shorthands, same meaning as for the built in types except that the value is returned, not a reference */
    operator T() const                                                              { return Load(); }
    T operator=( T value )                                                          { Store( value ); return value; }
    T operator++( void )                                                            { return static_cast< T >( FetchAdd( static_cast< T >( 1 ) ) + static_cast< T >( 1 ) ); }
    T operator++( int )                                                             { return FetchAdd( static_cast< T >( 1 ) ); }
    T operator--( void )                                                            { return static_cast< T >( FetchSub( static_cast< T >( 1 ) ) - static_cast< T >( 1 ) ); }
    T operator--( int )                                                             { return FetchSub( static_cast< T >( 1 ) ); }
    T operator+=( T value )                                                         { return static_cast< T >( FetchAdd( value ) + value ); }
    T operator-=( T value )                                                         { return static_cast< T >( FetchSub( value ) - value ); }
    T operator&=( T value )                                                         { return static_cast< T >( FetchAnd( value ) & value ); }
    T operator|=( T value )                                                         { return static_cast< T >( FetchOr ( value ) | value ); }
    T operator^=( T value )                                                         { return static_cast< T >( FetchXor( value ) ^ value ); }

    private:

    CTAtomic( const CTAtomic& src );                /**< not implemented, an atomic cannot be copied */
    CTAtomic& operator=( const CTAtomic& src );     /**< not implemented, an atomic cannot be assigned from another atomic */

    static TCarrier ToCarrier( T value )            { return static_cast< TCarrier >( value ); }
    static T FromCarrier( TCarrier carrier )        { return static_cast< T >( carrier ); }

    bool CompareExchange( T& expected, T desired, TMemoryOrder success, TMemoryOrder failure, bool weak )
    {
        TCarrier e = ToCarrier( expected );
        const bool exchanged = TBackend::CompareExchange( m_storage, e, ToCarrier( desired ), success, AtomicFailureOrder( success, failure ), weak );
        if ( !exchanged )
            expected = FromCarrier( e );
        return exchanged;
    }

    private:

    /* a floating point T would convert by value instead of by bits and silently do the wrong thing */
    typedef char TypeIsNotFloatingPoint[ TAtomicIsFloatingPoint< T >::value ? -1 : 1 ];

    typename TBackend::TStorage m_storage;
};

/*-------------------------------------------------------------------------*/

/**
 *  Atomic pointer. FetchAdd / FetchSub move the pointer by whole elements, as pointer arithmetic does.
 */
template< typename T, typename Tag >
class CTAtomic< T*, Tag >
{
    public:

    typedef T*                                                                  TValue;
    typedef typename TAtomicCarrier< sizeof( T* ) >::type                       TCarrier;
    typedef typename TAtomicResolveTag< sizeof( T* ), Tag >::type               TBackendTag;
    typedef CTAtomicBackend< TCarrier, TBackendTag >                            TBackend;

    CTAtomic( void ) : m_storage() { TBackend::Init( m_storage, ToCarrier( static_cast< T* >( GUCEF_NULL ) ) ); }
    explicit CTAtomic( T* value ) : m_storage() { TBackend::Init( m_storage, ToCarrier( value ) ); }

    T* Load( TMemoryOrder order = MEMORDER_SEQ_CST ) const                          { return FromCarrier( TBackend::Load( m_storage, AtomicLoadOrder( order ) ) ); }
    void Store( T* value, TMemoryOrder order = MEMORDER_SEQ_CST )                   { TBackend::Store( m_storage, ToCarrier( value ), AtomicStoreOrder( order ) ); }
    T* Exchange( T* value, TMemoryOrder order = MEMORDER_SEQ_CST )                  { return FromCarrier( TBackend::Exchange( m_storage, ToCarrier( value ), order ) ); }

    bool CompareExchangeStrong( T*& expected, T* desired, TMemoryOrder success = MEMORDER_SEQ_CST, TMemoryOrder failure = MEMORDER_SEQ_CST )
    {
        return CompareExchange( expected, desired, success, failure, false );
    }

    bool CompareExchangeWeak( T*& expected, T* desired, TMemoryOrder success = MEMORDER_SEQ_CST, TMemoryOrder failure = MEMORDER_SEQ_CST )
    {
        return CompareExchange( expected, desired, success, failure, true );
    }

    T* FetchAdd( ptrdiff_t elements, TMemoryOrder order = MEMORDER_SEQ_CST )        { return FromCarrier( TBackend::FetchAdd( m_storage, Offset( elements ), order ) ); }
    T* FetchSub( ptrdiff_t elements, TMemoryOrder order = MEMORDER_SEQ_CST )        { return FromCarrier( TBackend::FetchSub( m_storage, Offset( elements ), order ) ); }

    bool IsLockFree( void ) const                                                   { return TBackend::IsLockFree( m_storage ); }

    operator T*() const                                                             { return Load(); }
    T* operator=( T* value )                                                        { Store( value ); return value; }
    T* operator++( void )                                                           { return FetchAdd( 1 ) + 1; }
    T* operator++( int )                                                            { return FetchAdd( 1 ); }
    T* operator--( void )                                                           { return FetchSub( 1 ) - 1; }
    T* operator--( int )                                                            { return FetchSub( 1 ); }
    T* operator+=( ptrdiff_t elements )                                             { return FetchAdd( elements ) + elements; }
    T* operator-=( ptrdiff_t elements )                                             { return FetchSub( elements ) - elements; }

    private:

    CTAtomic( const CTAtomic& src );                /**< not implemented, an atomic cannot be copied */
    CTAtomic& operator=( const CTAtomic& src );     /**< not implemented, an atomic cannot be assigned from another atomic */

    static TCarrier ToCarrier( T* value )           { return reinterpret_cast< TCarrier >( value ); }
    static T* FromCarrier( TCarrier carrier )       { return reinterpret_cast< T* >( carrier ); }
    static TCarrier Offset( ptrdiff_t elements )    { return static_cast< TCarrier >( elements * static_cast< ptrdiff_t >( TAtomicPointeeSize< T >::value ) ); }

    bool CompareExchange( T*& expected, T* desired, TMemoryOrder success, TMemoryOrder failure, bool weak )
    {
        TCarrier e = ToCarrier( expected );
        const bool exchanged = TBackend::CompareExchange( m_storage, e, ToCarrier( desired ), success, AtomicFailureOrder( success, failure ), weak );
        if ( !exchanged )
            expected = FromCarrier( e );
        return exchanged;
    }

    private:

    typename TBackend::TStorage m_storage;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Orders memory accesses around this point between threads, without an atomic variable.
 *  The tag selects the backend like it does for CTAtomic, void is the platform default.
 */
template< typename Tag >
inline void AtomicThreadFence( TMemoryOrder order )
{
    CAtomicFence< typename TAtomicResolveTag< sizeof( void* ), Tag >::type >::ThreadFence( order );
}

inline void AtomicThreadFence( TMemoryOrder order = MEMORDER_SEQ_CST )
{
    CAtomicFence< GUCEF_MT_ATOMIC_DEFAULT_BACKEND_TAG >::ThreadFence( order );
}

/** Stops the compiler from reordering around this point, emits no instruction. For a signal handler or interrupt on the same thread */
inline void AtomicSignalFence( TMemoryOrder order = MEMORDER_SEQ_CST )
{
    CAtomicFence< GUCEF_MT_ATOMIC_DEFAULT_BACKEND_TAG >::SignalFence( order );
}

/** Hint for busy wait loops that the processor is spinning: lowers power use and helps the other hyperthread. Never blocks */
inline void AtomicCpuRelax( void )
{
    #if defined( GUCEF_MT_ATOMIC_HAVE_MSVC ) && ( defined( _M_IX86 ) || defined( _M_X64 ) )
    _mm_pause();
    #elif defined( __GNUC__ ) && ( defined( __i386__ ) || defined( __x86_64__ ) )
    __asm__ __volatile__( "pause" : : : "memory" );
    #elif defined( __GNUC__ ) && ( defined( __aarch64__ ) || ( defined( __ARM_ARCH ) && ( __ARM_ARCH >= 7 ) ) )
    __asm__ __volatile__( "yield" : : : "memory" );
    #elif defined( GUCEF_MT_ATOMIC_HAVE_MSVC ) && ( defined( _M_ARM ) || defined( _M_ARM64 ) )
    __yield();
    #else
    /* no hint available, this is still correct */
    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_CTATOMIC_H ? */
