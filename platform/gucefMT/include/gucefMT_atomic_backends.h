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

#ifndef GUCEF_MT_ATOMIC_BACKENDS_H
#define GUCEF_MT_ATOMIC_BACKENDS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stddef.h>

#ifndef GUCEF_MT_ETYPES_H
#include "gucefMT_ETypes.h"             /* simple types */
#define GUCEF_MT_ETYPES_H
#endif /* GUCEF_MT_ETYPES_H ? */

#ifndef GUCEF_MT_ATOMIC_CONFIG_H
#include "gucefMT_atomic_config.h"      /* backend detection */
#define GUCEF_MT_ATOMIC_CONFIG_H
#endif /* GUCEF_MT_ATOMIC_CONFIG_H ? */

#ifndef GUCEF_MT_MACROS_H
#include "gucefMT_macros.h"             /* module macros */
#define GUCEF_MT_MACROS_H
#endif /* GUCEF_MT_MACROS_H ? */

#ifdef GUCEF_MT_ATOMIC_HAVE_STD
  #include <atomic>
#endif

#ifdef GUCEF_MT_ATOMIC_HAVE_MSVC
  #include <intrin.h>
  #if defined( _M_IX86 ) || defined( _M_X64 )
    #include <emmintrin.h>              /* _mm_pause */
  #endif
#endif

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
 *  Memory ordering of an atomic operation, same meaning as the C++11 memory orders.
 *  MEMORDER_CONSUME is treated as MEMORDER_ACQUIRE, as every current compiler does.
 *  Orders that make no sense for an operation (a release load) are strengthened to MEMORDER_SEQ_CST.
 */
enum EMemoryOrder
{
    MEMORDER_RELAXED = 0 ,
    MEMORDER_CONSUME     ,
    MEMORDER_ACQUIRE     ,
    MEMORDER_RELEASE     ,
    MEMORDER_ACQ_REL     ,
    MEMORDER_SEQ_CST
};
typedef enum EMemoryOrder TMemoryOrder;

/*-------------------------------------------------------------------------*/

/** Backend tags, see gucefMT_atomic_config.h */
struct SAtomicBackendStd        {};
struct SAtomicBackendGccAtomic  {};
struct SAtomicBackendGccSync    {};
struct SAtomicBackendMsvc       {};
struct SAtomicBackendLock       {};

/*-------------------------------------------------------------------------*/

/** The unsigned integer type that carries a value of N bytes through the backends */
template< size_t N > struct TAtomicCarrier;
template<> struct TAtomicCarrier< 1 > { typedef UInt8  type; };
template<> struct TAtomicCarrier< 2 > { typedef UInt16 type; };
template<> struct TAtomicCarrier< 4 > { typedef UInt32 type; };
template<> struct TAtomicCarrier< 8 > { typedef UInt64 type; };

/*-------------------------------------------------------------------------*/

#if defined( _MSC_VER )
  #define GUCEF_MT_ATOMIC_ALIGNED( n ) __declspec( align( n ) )
#elif defined( __GNUC__ )
  #define GUCEF_MT_ATOMIC_ALIGNED( n ) __attribute__(( aligned( n ) ))
#else
  #define GUCEF_MT_ATOMIC_ALIGNED( n )
#endif

/** Naturally aligned storage, needed for the 8 byte atomics on 32 bit targets */
template< typename C > struct TAtomicAlignedStorage;
template<> struct GUCEF_MT_ATOMIC_ALIGNED( 1 ) TAtomicAlignedStorage< UInt8 >  { volatile UInt8  value; };
template<> struct GUCEF_MT_ATOMIC_ALIGNED( 2 ) TAtomicAlignedStorage< UInt16 > { volatile UInt16 value; };
template<> struct GUCEF_MT_ATOMIC_ALIGNED( 4 ) TAtomicAlignedStorage< UInt32 > { volatile UInt32 value; };
template<> struct GUCEF_MT_ATOMIC_ALIGNED( 8 ) TAtomicAlignedStorage< UInt64 > { volatile UInt64 value; };

/*-------------------------------------------------------------------------*/

/**
 *  Every backend is a specialization of this template for a carrier type C (UInt8 ... UInt64) and a tag.
 *  A backend provides:
 *    typedef ... TStorage;
 *    static void Init( TStorage&, C );
 *    static C    Load( const TStorage&, TMemoryOrder );
 *    static void Store( TStorage&, C, TMemoryOrder );
 *    static C    Exchange( TStorage&, C, TMemoryOrder );
 *    static bool CompareExchange( TStorage&, C& expected, C desired, TMemoryOrder success, TMemoryOrder failure, bool weak );
 *    static C    FetchAdd / FetchSub / FetchAnd / FetchOr / FetchXor( TStorage&, C, TMemoryOrder );  return the previous value
 *    static bool IsLockFree( const TStorage& );
 *  Orders arrive normalized (see the helpers below), a backend does not have to validate them.
 */
template< typename C, typename Tag > class CTAtomicBackend;

/** Operations a backend can inherit when it has no native instruction: built on CompareExchange */
template< typename TBackend, typename C >
class CTAtomicCasLoopOps
{
    public:

    template< typename S > static C FetchAdd( S& s, C v, TMemoryOrder o ) { C e = TBackend::Load( s, MEMORDER_RELAXED ); while ( !TBackend::CompareExchange( s, e, static_cast< C >( e + v ), o, MEMORDER_RELAXED, false ) ) {} return e; }
    template< typename S > static C FetchSub( S& s, C v, TMemoryOrder o ) { C e = TBackend::Load( s, MEMORDER_RELAXED ); while ( !TBackend::CompareExchange( s, e, static_cast< C >( e - v ), o, MEMORDER_RELAXED, false ) ) {} return e; }
    template< typename S > static C FetchAnd( S& s, C v, TMemoryOrder o ) { C e = TBackend::Load( s, MEMORDER_RELAXED ); while ( !TBackend::CompareExchange( s, e, static_cast< C >( e & v ), o, MEMORDER_RELAXED, false ) ) {} return e; }
    template< typename S > static C FetchOr ( S& s, C v, TMemoryOrder o ) { C e = TBackend::Load( s, MEMORDER_RELAXED ); while ( !TBackend::CompareExchange( s, e, static_cast< C >( e | v ), o, MEMORDER_RELAXED, false ) ) {} return e; }
    template< typename S > static C FetchXor( S& s, C v, TMemoryOrder o ) { C e = TBackend::Load( s, MEMORDER_RELAXED ); while ( !TBackend::CompareExchange( s, e, static_cast< C >( e ^ v ), o, MEMORDER_RELAXED, false ) ) {} return e; }
};

/*-------------------------------------------------------------------------*/

/** Fences per backend: ThreadFence orders memory between threads, SignalFence only stops compiler reordering */
template< typename Tag > struct CAtomicFence;

/** Order normalization helpers, used by CTAtomic before it calls a backend */
inline TMemoryOrder AtomicLoadOrder( TMemoryOrder o )
{
    return ( MEMORDER_RELEASE == o || MEMORDER_ACQ_REL == o ) ? MEMORDER_SEQ_CST : o;
}

inline TMemoryOrder AtomicStoreOrder( TMemoryOrder o )
{
    return ( MEMORDER_CONSUME == o || MEMORDER_ACQUIRE == o || MEMORDER_ACQ_REL == o ) ? MEMORDER_SEQ_CST : o;
}

/** The failure order of a compare exchange may be neither a release order nor stronger than the success order */
inline TMemoryOrder AtomicFailureOrder( TMemoryOrder success, TMemoryOrder failure )
{
    TMemoryOrder strongestAllowed = MEMORDER_SEQ_CST;
    switch ( success )
    {
        case MEMORDER_RELAXED :
        case MEMORDER_RELEASE : strongestAllowed = MEMORDER_RELAXED; break;
        case MEMORDER_CONSUME :
        case MEMORDER_ACQUIRE :
        case MEMORDER_ACQ_REL : strongestAllowed = MEMORDER_ACQUIRE; break;
        default               : strongestAllowed = MEMORDER_SEQ_CST; break;
    }
    if ( MEMORDER_RELEASE == failure ) failure = MEMORDER_RELAXED;
    if ( MEMORDER_ACQ_REL == failure ) failure = MEMORDER_ACQUIRE;
    if ( MEMORDER_CONSUME == failure ) failure = MEMORDER_ACQUIRE;
    if ( MEMORDER_SEQ_CST == failure && MEMORDER_SEQ_CST != strongestAllowed ) failure = strongestAllowed;
    if ( MEMORDER_ACQUIRE == failure && MEMORDER_RELAXED == strongestAllowed ) failure = MEMORDER_RELAXED;
    return failure;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      BACKEND: C++11 <atomic>                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_MT_ATOMIC_HAVE_STD

template< typename C >
class CTAtomicBackend< C, SAtomicBackendStd > : public CTAtomicCasLoopOps< CTAtomicBackend< C, SAtomicBackendStd >, C >
{
    public:

    typedef std::atomic< C > TStorage;

    static std::memory_order ToStd( TMemoryOrder o )
    {
        switch ( o )
        {
            case MEMORDER_RELAXED : return std::memory_order_relaxed;
            case MEMORDER_CONSUME :
            case MEMORDER_ACQUIRE : return std::memory_order_acquire;
            case MEMORDER_RELEASE : return std::memory_order_release;
            case MEMORDER_ACQ_REL : return std::memory_order_acq_rel;
            default               : return std::memory_order_seq_cst;
        }
    }

    static void Init( TStorage& s, C v )                                    { s.store( v, std::memory_order_relaxed ); }
    static C    Load( const TStorage& s, TMemoryOrder o )                   { return s.load( ToStd( o ) ); }
    static void Store( TStorage& s, C v, TMemoryOrder o )                   { s.store( v, ToStd( o ) ); }
    static C    Exchange( TStorage& s, C v, TMemoryOrder o )                { return s.exchange( v, ToStd( o ) ); }

    static bool CompareExchange( TStorage& s, C& expected, C desired, TMemoryOrder success, TMemoryOrder failure, bool weak )
    {
        return weak ? s.compare_exchange_weak( expected, desired, ToStd( success ), ToStd( failure ) )
                    : s.compare_exchange_strong( expected, desired, ToStd( success ), ToStd( failure ) );
    }

    static C FetchAdd( TStorage& s, C v, TMemoryOrder o )                   { return s.fetch_add( v, ToStd( o ) ); }
    static C FetchSub( TStorage& s, C v, TMemoryOrder o )                   { return s.fetch_sub( v, ToStd( o ) ); }
    static C FetchAnd( TStorage& s, C v, TMemoryOrder o )                   { return s.fetch_and( v, ToStd( o ) ); }
    static C FetchOr ( TStorage& s, C v, TMemoryOrder o )                   { return s.fetch_or ( v, ToStd( o ) ); }
    static C FetchXor( TStorage& s, C v, TMemoryOrder o )                   { return s.fetch_xor( v, ToStd( o ) ); }

    static bool IsLockFree( const TStorage& s )                             { return s.is_lock_free(); }
};

template<> struct CAtomicFence< SAtomicBackendStd >
{
    static void ThreadFence( TMemoryOrder o ) { std::atomic_thread_fence( CTAtomicBackend< UInt8, SAtomicBackendStd >::ToStd( o ) ); }
    static void SignalFence( TMemoryOrder o ) { std::atomic_signal_fence( CTAtomicBackend< UInt8, SAtomicBackendStd >::ToStd( o ) ); }
};

#endif /* GUCEF_MT_ATOMIC_HAVE_STD ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      BACKEND: GCC / Clang __atomic builtins                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_MT_ATOMIC_HAVE_GCC_ATOMIC

/* Every order is a compile time constant at the builtin, which is what the compilers need to emit the cheapest code */
#define GUCEF_MT_ATOMIC_GCC_SWITCH_RMW( OP, o )                                       \
    switch ( o )                                                                      \
    {                                                                                 \
        case MEMORDER_RELAXED : return OP( __ATOMIC_RELAXED );                        \
        case MEMORDER_CONSUME :                                                       \
        case MEMORDER_ACQUIRE : return OP( __ATOMIC_ACQUIRE );                        \
        case MEMORDER_RELEASE : return OP( __ATOMIC_RELEASE );                        \
        case MEMORDER_ACQ_REL : return OP( __ATOMIC_ACQ_REL );                        \
        default               : return OP( __ATOMIC_SEQ_CST );                        \
    }

#define GUCEF_MT_ATOMIC_GCC_SWITCH_LOAD( OP, o )                                      \
    switch ( o )                                                                      \
    {                                                                                 \
        case MEMORDER_RELAXED : return OP( __ATOMIC_RELAXED );                        \
        case MEMORDER_CONSUME :                                                       \
        case MEMORDER_ACQUIRE : return OP( __ATOMIC_ACQUIRE );                        \
        default               : return OP( __ATOMIC_SEQ_CST );                        \
    }

#define GUCEF_MT_ATOMIC_GCC_SWITCH_STORE( OP, o )                                     \
    switch ( o )                                                                      \
    {                                                                                 \
        case MEMORDER_RELAXED : OP( __ATOMIC_RELAXED ); return;                       \
        case MEMORDER_RELEASE : OP( __ATOMIC_RELEASE ); return;                       \
        default               : OP( __ATOMIC_SEQ_CST ); return;                       \
    }

template< typename C >
class CTAtomicBackend< C, SAtomicBackendGccAtomic > : public CTAtomicCasLoopOps< CTAtomicBackend< C, SAtomicBackendGccAtomic >, C >
{
    public:

    typedef TAtomicAlignedStorage< C > TStorage;

    static void Init( TStorage& s, C v ) { __atomic_store_n( &s.value, v, __ATOMIC_RELAXED ); }

    static C Load( const TStorage& s, TMemoryOrder o )
    {
        #define GUCEF_MT_ATOMIC_OP( mo ) __atomic_load_n( &s.value, mo )
        GUCEF_MT_ATOMIC_GCC_SWITCH_LOAD( GUCEF_MT_ATOMIC_OP, o )
        #undef GUCEF_MT_ATOMIC_OP
    }

    static void Store( TStorage& s, C v, TMemoryOrder o )
    {
        #define GUCEF_MT_ATOMIC_OP( mo ) __atomic_store_n( &s.value, v, mo )
        GUCEF_MT_ATOMIC_GCC_SWITCH_STORE( GUCEF_MT_ATOMIC_OP, o )
        #undef GUCEF_MT_ATOMIC_OP
    }

    static C Exchange( TStorage& s, C v, TMemoryOrder o )
    {
        #define GUCEF_MT_ATOMIC_OP( mo ) __atomic_exchange_n( &s.value, v, mo )
        GUCEF_MT_ATOMIC_GCC_SWITCH_RMW( GUCEF_MT_ATOMIC_OP, o )
        #undef GUCEF_MT_ATOMIC_OP
    }

    /* Failure orders that can never be stronger than the success order, so no invalid combination is ever compiled in */
    #define GUCEF_MT_ATOMIC_CAS( weakFlag, so, fo ) __atomic_compare_exchange_n( &s.value, &expected, desired, weakFlag, so, fo )
    #define GUCEF_MT_ATOMIC_CAS_ALL( weakFlag )                                                                                                                             \
        switch ( success )                                                                                                                                                  \
        {                                                                                                                                                                   \
            case MEMORDER_RELAXED : return GUCEF_MT_ATOMIC_CAS( weakFlag, __ATOMIC_RELAXED, __ATOMIC_RELAXED );                                                             \
            case MEMORDER_CONSUME :                                                                                                                                         \
            case MEMORDER_ACQUIRE : return MEMORDER_RELAXED == failure ? GUCEF_MT_ATOMIC_CAS( weakFlag, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED )                                \
                                                                       : GUCEF_MT_ATOMIC_CAS( weakFlag, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE );                               \
            case MEMORDER_RELEASE : return GUCEF_MT_ATOMIC_CAS( weakFlag, __ATOMIC_RELEASE, __ATOMIC_RELAXED );                                                             \
            case MEMORDER_ACQ_REL : return MEMORDER_RELAXED == failure ? GUCEF_MT_ATOMIC_CAS( weakFlag, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED )                                \
                                                                       : GUCEF_MT_ATOMIC_CAS( weakFlag, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE );                               \
            default               : return MEMORDER_RELAXED == failure ? GUCEF_MT_ATOMIC_CAS( weakFlag, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED )                                \
                                         : MEMORDER_SEQ_CST == failure ? GUCEF_MT_ATOMIC_CAS( weakFlag, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST )                                \
                                                                       : GUCEF_MT_ATOMIC_CAS( weakFlag, __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE );                               \
        }

    static bool CompareExchange( TStorage& s, C& expected, C desired, TMemoryOrder success, TMemoryOrder failure, bool weak )
    {
        if ( weak )
        {
            GUCEF_MT_ATOMIC_CAS_ALL( true )
        }
        else
        {
            GUCEF_MT_ATOMIC_CAS_ALL( false )
        }
    }
    #undef GUCEF_MT_ATOMIC_CAS_ALL
    #undef GUCEF_MT_ATOMIC_CAS

    static C FetchAdd( TStorage& s, C v, TMemoryOrder o )
    {
        #define GUCEF_MT_ATOMIC_OP( mo ) __atomic_fetch_add( &s.value, v, mo )
        GUCEF_MT_ATOMIC_GCC_SWITCH_RMW( GUCEF_MT_ATOMIC_OP, o )
        #undef GUCEF_MT_ATOMIC_OP
    }

    static C FetchSub( TStorage& s, C v, TMemoryOrder o )
    {
        #define GUCEF_MT_ATOMIC_OP( mo ) __atomic_fetch_sub( &s.value, v, mo )
        GUCEF_MT_ATOMIC_GCC_SWITCH_RMW( GUCEF_MT_ATOMIC_OP, o )
        #undef GUCEF_MT_ATOMIC_OP
    }

    static C FetchAnd( TStorage& s, C v, TMemoryOrder o )
    {
        #define GUCEF_MT_ATOMIC_OP( mo ) __atomic_fetch_and( &s.value, v, mo )
        GUCEF_MT_ATOMIC_GCC_SWITCH_RMW( GUCEF_MT_ATOMIC_OP, o )
        #undef GUCEF_MT_ATOMIC_OP
    }

    static C FetchOr( TStorage& s, C v, TMemoryOrder o )
    {
        #define GUCEF_MT_ATOMIC_OP( mo ) __atomic_fetch_or( &s.value, v, mo )
        GUCEF_MT_ATOMIC_GCC_SWITCH_RMW( GUCEF_MT_ATOMIC_OP, o )
        #undef GUCEF_MT_ATOMIC_OP
    }

    static C FetchXor( TStorage& s, C v, TMemoryOrder o )
    {
        #define GUCEF_MT_ATOMIC_OP( mo ) __atomic_fetch_xor( &s.value, v, mo )
        GUCEF_MT_ATOMIC_GCC_SWITCH_RMW( GUCEF_MT_ATOMIC_OP, o )
        #undef GUCEF_MT_ATOMIC_OP
    }

    static bool IsLockFree( const TStorage& s ) { return __atomic_is_lock_free( sizeof( C ), const_cast< C* >( reinterpret_cast< const volatile C* >( &s.value ) ) ); }
};

template<> struct CAtomicFence< SAtomicBackendGccAtomic >
{
    static void ThreadFence( TMemoryOrder o )
    {
        switch ( o )
        {
            case MEMORDER_RELAXED : break;
            case MEMORDER_CONSUME :
            case MEMORDER_ACQUIRE : __atomic_thread_fence( __ATOMIC_ACQUIRE ); break;
            case MEMORDER_RELEASE : __atomic_thread_fence( __ATOMIC_RELEASE ); break;
            case MEMORDER_ACQ_REL : __atomic_thread_fence( __ATOMIC_ACQ_REL ); break;
            default               : __atomic_thread_fence( __ATOMIC_SEQ_CST ); break;
        }
    }
    static void SignalFence( TMemoryOrder o )
    {
        switch ( o )
        {
            case MEMORDER_RELAXED : break;
            case MEMORDER_CONSUME :
            case MEMORDER_ACQUIRE : __atomic_signal_fence( __ATOMIC_ACQUIRE ); break;
            case MEMORDER_RELEASE : __atomic_signal_fence( __ATOMIC_RELEASE ); break;
            case MEMORDER_ACQ_REL : __atomic_signal_fence( __ATOMIC_ACQ_REL ); break;
            default               : __atomic_signal_fence( __ATOMIC_SEQ_CST ); break;
        }
    }
};

#undef GUCEF_MT_ATOMIC_GCC_SWITCH_RMW
#undef GUCEF_MT_ATOMIC_GCC_SWITCH_LOAD
#undef GUCEF_MT_ATOMIC_GCC_SWITCH_STORE

#endif /* GUCEF_MT_ATOMIC_HAVE_GCC_ATOMIC ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      BACKEND: legacy GCC __sync builtins                                //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_MT_ATOMIC_HAVE_GCC_SYNC

/**
 *  The __sync builtins are all full barriers, so every requested order is satisfied by doing more than needed.
 *  Plain loads and stores of aligned values up to the machine word are atomic on the targets GCC offers
 *  these builtins for, the barriers around them give them their ordering.
 */
template< typename C >
class CTAtomicBackend< C, SAtomicBackendGccSync > : public CTAtomicCasLoopOps< CTAtomicBackend< C, SAtomicBackendGccSync >, C >
{
    public:

    typedef TAtomicAlignedStorage< C > TStorage;

    static void Init( TStorage& s, C v )                                    { s.value = v; __sync_synchronize(); }

    static C Load( const TStorage& s, TMemoryOrder o )
    {
        if ( MEMORDER_RELAXED == o )
            return s.value;
        __sync_synchronize();
        C v = s.value;
        __sync_synchronize();
        return v;
    }

    static void Store( TStorage& s, C v, TMemoryOrder o )
    {
        if ( MEMORDER_RELAXED == o )
        {
            s.value = v;
            return;
        }
        __sync_synchronize();
        s.value = v;
        if ( MEMORDER_SEQ_CST == o )
            __sync_synchronize();
    }

    static C Exchange( TStorage& s, C v, TMemoryOrder )
    {
        C e = s.value;
        while ( true )
        {
            C seen = __sync_val_compare_and_swap( &s.value, e, v );
            if ( seen == e )
                return e;
            e = seen;
        }
    }

    static bool CompareExchange( TStorage& s, C& expected, C desired, TMemoryOrder, TMemoryOrder, bool )
    {
        C seen = __sync_val_compare_and_swap( &s.value, expected, desired );
        if ( seen == expected )
            return true;
        expected = seen;
        return false;
    }

    static C FetchAdd( TStorage& s, C v, TMemoryOrder )                     { return __sync_fetch_and_add( &s.value, v ); }
    static C FetchSub( TStorage& s, C v, TMemoryOrder )                     { return __sync_fetch_and_sub( &s.value, v ); }
    static C FetchAnd( TStorage& s, C v, TMemoryOrder )                     { return __sync_fetch_and_and( &s.value, v ); }
    static C FetchOr ( TStorage& s, C v, TMemoryOrder )                     { return __sync_fetch_and_or ( &s.value, v ); }
    static C FetchXor( TStorage& s, C v, TMemoryOrder )                     { return __sync_fetch_and_xor( &s.value, v ); }

    static bool IsLockFree( const TStorage& )                               { return true; }
};

template<> struct CAtomicFence< SAtomicBackendGccSync >
{
    static void ThreadFence( TMemoryOrder o ) { if ( MEMORDER_RELAXED != o ) __sync_synchronize(); }
    static void SignalFence( TMemoryOrder o ) { if ( MEMORDER_RELAXED != o ) __asm__ __volatile__( "" : : : "memory" ); }
};

#endif /* GUCEF_MT_ATOMIC_HAVE_GCC_SYNC ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      BACKEND: Visual Studio intrinsics                                  //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_MT_ATOMIC_HAVE_MSVC

#if defined( _M_IX86 ) || defined( _M_X64 )
  #define GUCEF_MT_ATOMIC_MSVC_X86 1
#elif defined( _M_ARM ) || defined( _M_ARM64 ) || defined( _M_ARM64EC )
  #define GUCEF_MT_ATOMIC_MSVC_ARM 1
#endif

#if defined( _M_X64 ) || defined( _M_ARM64 ) || defined( _M_ARM64EC )
  #define GUCEF_MT_ATOMIC_MSVC_NATIVE64 1
#endif

/*
 *  The interlocked intrinsics are full barriers on every Visual Studio target, which satisfies every
 *  memory order. Plain loads and stores get the cheapest correct form per architecture.
 *  Note: this backend is written against the documented intrinsics and could not be run in the
 *  environment it was written in. The Linux builds cover the shared logic through the other backends.
 */
template< size_t N > struct SAtomicMsvcOps;

template<> struct SAtomicMsvcOps< 1 >
{
    typedef char T;
    static T Cas( volatile T* p, T desired, T expected )    { return _InterlockedCompareExchange8( p, desired, expected ); }
    static T Xchg( volatile T* p, T v )                     { return _InterlockedExchange8( p, v ); }
    static T Add( volatile T* p, T v )                      { return _InterlockedExchangeAdd8( p, v ); }
    static T And( volatile T* p, T v )                      { return _InterlockedAnd8( p, v ); }
    static T Or ( volatile T* p, T v )                      { return _InterlockedOr8( p, v ); }
    static T Xor( volatile T* p, T v )                      { return _InterlockedXor8( p, v ); }
    static T RawLoad( const volatile T* p )                 { return __iso_volatile_load8( p ); }
    static void RawStore( volatile T* p, T v )              { __iso_volatile_store8( p, v ); }
    static bool HasAtomicRawAccess( void )                  { return true; }
};

template<> struct SAtomicMsvcOps< 2 >
{
    typedef short T;
    static T Cas( volatile T* p, T desired, T expected )    { return _InterlockedCompareExchange16( p, desired, expected ); }
    static T Xchg( volatile T* p, T v )                     { return _InterlockedExchange16( p, v ); }
    static T Add( volatile T* p, T v )                      { return _InterlockedExchangeAdd16( p, v ); }
    static T And( volatile T* p, T v )                      { return _InterlockedAnd16( p, v ); }
    static T Or ( volatile T* p, T v )                      { return _InterlockedOr16( p, v ); }
    static T Xor( volatile T* p, T v )                      { return _InterlockedXor16( p, v ); }
    static T RawLoad( const volatile T* p )                 { return __iso_volatile_load16( p ); }
    static void RawStore( volatile T* p, T v )              { __iso_volatile_store16( p, v ); }
    static bool HasAtomicRawAccess( void )                  { return true; }
};

template<> struct SAtomicMsvcOps< 4 >
{
    typedef long T;
    static T Cas( volatile T* p, T desired, T expected )    { return _InterlockedCompareExchange( p, desired, expected ); }
    static T Xchg( volatile T* p, T v )                     { return _InterlockedExchange( p, v ); }
    static T Add( volatile T* p, T v )                      { return _InterlockedExchangeAdd( p, v ); }
    static T And( volatile T* p, T v )                      { return _InterlockedAnd( p, v ); }
    static T Or ( volatile T* p, T v )                      { return _InterlockedOr( p, v ); }
    static T Xor( volatile T* p, T v )                      { return _InterlockedXor( p, v ); }
    static T RawLoad( const volatile T* p )                 { return __iso_volatile_load32( reinterpret_cast< const volatile int* >( p ) ); }
    static void RawStore( volatile T* p, T v )              { __iso_volatile_store32( reinterpret_cast< volatile int* >( p ), static_cast< int >( v ) ); }
    static bool HasAtomicRawAccess( void )                  { return true; }
};

template<> struct SAtomicMsvcOps< 8 >
{
    typedef __int64 T;
    static T Cas( volatile T* p, T desired, T expected )    { return _InterlockedCompareExchange64( p, desired, expected ); }
    #ifdef GUCEF_MT_ATOMIC_MSVC_NATIVE64
    static T Xchg( volatile T* p, T v )                     { return _InterlockedExchange64( p, v ); }
    static T Add( volatile T* p, T v )                      { return _InterlockedExchangeAdd64( p, v ); }
    static T And( volatile T* p, T v )                      { return _InterlockedAnd64( p, v ); }
    static T Or ( volatile T* p, T v )                      { return _InterlockedOr64( p, v ); }
    static T Xor( volatile T* p, T v )                      { return _InterlockedXor64( p, v ); }
    static bool HasAtomicRawAccess( void )                  { return true; }
    #else
    /* 32 bit targets only have the 64 bit compare and exchange (cmpxchg8b and friends) */
    static T Xchg( volatile T* p, T v )                     { T e = *p; for ( ;; ) { T s = Cas( p, v, e ); if ( s == e ) return e; e = s; } }
    static T Add( volatile T* p, T v )                      { T e = *p; for ( ;; ) { T s = Cas( p, e + v, e ); if ( s == e ) return e; e = s; } }
    static T And( volatile T* p, T v )                      { T e = *p; for ( ;; ) { T s = Cas( p, e & v, e ); if ( s == e ) return e; e = s; } }
    static T Or ( volatile T* p, T v )                      { T e = *p; for ( ;; ) { T s = Cas( p, e | v, e ); if ( s == e ) return e; e = s; } }
    static T Xor( volatile T* p, T v )                      { T e = *p; for ( ;; ) { T s = Cas( p, e ^ v, e ); if ( s == e ) return e; e = s; } }
    static bool HasAtomicRawAccess( void )                  { return false; }
    #endif
    static T RawLoad( const volatile T* p )                 { return __iso_volatile_load64( p ); }
    static void RawStore( volatile T* p, T v )              { __iso_volatile_store64( p, v ); }
};

template< typename C >
class CTAtomicBackend< C, SAtomicBackendMsvc > : public CTAtomicCasLoopOps< CTAtomicBackend< C, SAtomicBackendMsvc >, C >
{
    public:

    typedef TAtomicAlignedStorage< C > TStorage;

    private:

    typedef SAtomicMsvcOps< sizeof( C ) > TOps;
    typedef typename TOps::T TNative;

    static volatile TNative* Ptr( TStorage& s )                { return reinterpret_cast< volatile TNative* >( &s.value ); }
    static const volatile TNative* Ptr( const TStorage& s )    { return reinterpret_cast< const volatile TNative* >( &s.value ); }

    public:

    static void Init( TStorage& s, C v )                       { s.value = v; _ReadWriteBarrier(); }

    static C Load( const TStorage& s, TMemoryOrder o )
    {
        #if defined( GUCEF_MT_ATOMIC_MSVC_X86 )
        if ( TOps::HasAtomicRawAccess() )
        {
            C v = static_cast< C >( TOps::RawLoad( Ptr( s ) ) );
            _ReadWriteBarrier();
            return v;
        }
        #elif defined( GUCEF_MT_ATOMIC_MSVC_ARM )
        if ( TOps::HasAtomicRawAccess() )
        {
            C v = static_cast< C >( TOps::RawLoad( Ptr( s ) ) );
            if ( MEMORDER_RELAXED != o )
                __dmb( 0xB );
            return v;
        }
        #endif
        (void) o;
        return static_cast< C >( TOps::Cas( const_cast< volatile TNative* >( Ptr( s ) ), 0, 0 ) );
    }

    static void Store( TStorage& s, C v, TMemoryOrder o )
    {
        #if defined( GUCEF_MT_ATOMIC_MSVC_X86 )
        if ( TOps::HasAtomicRawAccess() )
        {
            if ( MEMORDER_SEQ_CST == o )
            {
                TOps::Xchg( Ptr( s ), static_cast< TNative >( v ) );
            }
            else
            {
                _ReadWriteBarrier();
                TOps::RawStore( Ptr( s ), static_cast< TNative >( v ) );
            }
            return;
        }
        #elif defined( GUCEF_MT_ATOMIC_MSVC_ARM )
        if ( TOps::HasAtomicRawAccess() )
        {
            if ( MEMORDER_RELAXED != o )
                __dmb( 0xB );
            TOps::RawStore( Ptr( s ), static_cast< TNative >( v ) );
            if ( MEMORDER_SEQ_CST == o )
                __dmb( 0xB );
            return;
        }
        #endif
        (void) o;
        TOps::Xchg( Ptr( s ), static_cast< TNative >( v ) );
    }

    static C Exchange( TStorage& s, C v, TMemoryOrder )        { return static_cast< C >( TOps::Xchg( Ptr( s ), static_cast< TNative >( v ) ) ); }

    static bool CompareExchange( TStorage& s, C& expected, C desired, TMemoryOrder, TMemoryOrder, bool )
    {
        const TNative seen = TOps::Cas( Ptr( s ), static_cast< TNative >( desired ), static_cast< TNative >( expected ) );
        if ( static_cast< C >( seen ) == expected )
            return true;
        expected = static_cast< C >( seen );
        return false;
    }

    static C FetchAdd( TStorage& s, C v, TMemoryOrder )        { return static_cast< C >( TOps::Add( Ptr( s ), static_cast< TNative >( v ) ) ); }
    static C FetchSub( TStorage& s, C v, TMemoryOrder )        { return static_cast< C >( TOps::Add( Ptr( s ), static_cast< TNative >( static_cast< C >( 0 - v ) ) ) ); }
    static C FetchAnd( TStorage& s, C v, TMemoryOrder )        { return static_cast< C >( TOps::And( Ptr( s ), static_cast< TNative >( v ) ) ); }
    static C FetchOr ( TStorage& s, C v, TMemoryOrder )        { return static_cast< C >( TOps::Or ( Ptr( s ), static_cast< TNative >( v ) ) ); }
    static C FetchXor( TStorage& s, C v, TMemoryOrder )        { return static_cast< C >( TOps::Xor( Ptr( s ), static_cast< TNative >( v ) ) ); }

    static bool IsLockFree( const TStorage& )                  { return true; }
};

template<> struct CAtomicFence< SAtomicBackendMsvc >
{
    static void ThreadFence( TMemoryOrder o )
    {
        if ( MEMORDER_RELAXED == o )
            return;
        _ReadWriteBarrier();
        #if defined( GUCEF_MT_ATOMIC_MSVC_ARM )
        __dmb( 0xB );
        #else
        if ( MEMORDER_SEQ_CST == o )
        {
            volatile long barrier = 0;
            _InterlockedOr( &barrier, 0 );
        }
        #endif
    }
    static void SignalFence( TMemoryOrder o ) { if ( MEMORDER_RELAXED != o ) _ReadWriteBarrier(); }
};

#endif /* GUCEF_MT_ATOMIC_HAVE_MSVC ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      BACKEND: striped locks                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Process wide pool of locks, a lock is picked by the address of the atomic. Lives in gucefMT. */
class GUCEF_MT_PUBLIC_CPP CAtomicLockPool
{
    public:

    static void Lock( const volatile void* address );
    static void Unlock( const volatile void* address );
};

class CAtomicLockScope
{
    public:

    explicit CAtomicLockScope( const volatile void* address ) : m_address( address ) { CAtomicLockPool::Lock( m_address ); }
    ~CAtomicLockScope() { CAtomicLockPool::Unlock( m_address ); }

    private:

    CAtomicLockScope( const CAtomicLockScope& src );
    CAtomicLockScope& operator=( const CAtomicLockScope& src );
    const volatile void* m_address;
};

/**
 *  Works everywhere there is a mutex and is not lock free. It is what 8 byte atomics fall back to on 32 bit
 *  targets that have no 64 bit compare and swap, and what every atomic falls back to when no compiler
 *  support at all was detected.
 */
template< typename C >
class CTAtomicBackend< C, SAtomicBackendLock > : public CTAtomicCasLoopOps< CTAtomicBackend< C, SAtomicBackendLock >, C >
{
    public:

    typedef TAtomicAlignedStorage< C > TStorage;

    static void Init( TStorage& s, C v )                                    { CAtomicLockScope lock( &s ); s.value = v; }
    static C    Load( const TStorage& s, TMemoryOrder )                     { CAtomicLockScope lock( &s ); return s.value; }
    static void Store( TStorage& s, C v, TMemoryOrder )                     { CAtomicLockScope lock( &s ); s.value = v; }
    static C    Exchange( TStorage& s, C v, TMemoryOrder )                  { CAtomicLockScope lock( &s ); C old = s.value; s.value = v; return old; }

    static bool CompareExchange( TStorage& s, C& expected, C desired, TMemoryOrder, TMemoryOrder, bool )
    {
        CAtomicLockScope lock( &s );
        if ( s.value == expected )
        {
            s.value = desired;
            return true;
        }
        expected = s.value;
        return false;
    }

    static C FetchAdd( TStorage& s, C v, TMemoryOrder )                     { CAtomicLockScope lock( &s ); C old = s.value; s.value = static_cast< C >( old + v ); return old; }
    static C FetchSub( TStorage& s, C v, TMemoryOrder )                     { CAtomicLockScope lock( &s ); C old = s.value; s.value = static_cast< C >( old - v ); return old; }
    static C FetchAnd( TStorage& s, C v, TMemoryOrder )                     { CAtomicLockScope lock( &s ); C old = s.value; s.value = static_cast< C >( old & v ); return old; }
    static C FetchOr ( TStorage& s, C v, TMemoryOrder )                     { CAtomicLockScope lock( &s ); C old = s.value; s.value = static_cast< C >( old | v ); return old; }
    static C FetchXor( TStorage& s, C v, TMemoryOrder )                     { CAtomicLockScope lock( &s ); C old = s.value; s.value = static_cast< C >( old ^ v ); return old; }

    static bool IsLockFree( const TStorage& )                               { return false; }
};

template<> struct CAtomicFence< SAtomicBackendLock >
{
    /* Taking a lock is a full barrier, a fence only has to order surrounding non atomic accesses the same way */
    static void ThreadFence( TMemoryOrder o )
    {
        if ( MEMORDER_RELAXED == o )
            return;
        static volatile char fenceAddress = 0;
        CAtomicLockScope lock( &fenceAddress );
    }
    static void SignalFence( TMemoryOrder ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_ATOMIC_BACKENDS_H ? */
