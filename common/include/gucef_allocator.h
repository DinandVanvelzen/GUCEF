/*
 *  gucef common header: provides header based platform wide facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_ALLOCATOR_H
#define GUCEF_ALLOCATOR_H

#ifdef __cplusplus

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <limits>
#include <utility>
#include <memory>
#if __cplusplus >= 201103L
#include <type_traits>  // for std::is_same used in C++11 static_assert
#endif

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"        /* GUCEF configuration */
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"      /* GUCEF platform compilation targets */
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#ifndef GUCEF_CPP_H
#include "gucef_cpp.h"       
#define GUCEF_CPP_H
#endif /* GUCEF_CPP_H ? */

#ifndef GUCEF_MEMORY_H
#include "gucef_memory.h"       
#define GUCEF_MEMORY_H
#endif /* GUCEF_MEMORY_H ? */

#include <cstddef>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #define NOMINMAX
  #undef max
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

template < class T > class gucef_platform_diagnostic_allocator;

template <>
class gucef_platform_diagnostic_allocator<void>
{
public:
    typedef void              value_type;
    typedef value_type*       pointer;
    typedef value_type const* const_pointer;
    typedef std::size_t       size_type;
    typedef std::ptrdiff_t    difference_type;

    template <class U>
    struct rebind
    {
        typedef gucef_platform_diagnostic_allocator<U> other;
    };
};

template <class T>
class gucef_platform_diagnostic_allocator
{
public:

    typedef T                   value_type;
    typedef value_type&         reference;
    typedef const value_type&   const_reference;
    typedef value_type*         pointer;
    typedef const value_type*   const_pointer;
    typedef std::size_t         size_type;
    typedef std::ptrdiff_t      difference_type;

    template <class U>
    struct rebind
    {
        typedef gucef_platform_diagnostic_allocator<U> other;
    };

    // Ensure rebind works for std::pair<const Key, Value>
    template <typename Key, typename Value>
    struct rebind< std::pair< const Key, Value > >
    {
        typedef gucef_platform_diagnostic_allocator< std::pair< const Key, Value > > other;
    };

    gucef_platform_diagnostic_allocator() GUCEF_NOEXCEPT {}
    
    template <class U>
    gucef_platform_diagnostic_allocator( const gucef_platform_diagnostic_allocator< U >& ) GUCEF_NOEXCEPT 
    {GUCEF_TRACE; }

    pointer
    allocate( size_type n, gucef_platform_diagnostic_allocator< void >::const_pointer = 0 )
    {GUCEF_TRACE;

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        return static_cast< pointer >( ::operator new ( n * sizeof( value_type ), __FILE__, __LINE__ ) );

        #else

        return static_cast< pointer >( ::operator new ( n * sizeof( value_type ) ) );

        #endif
    }

    void
    deallocate( pointer p, size_type )
    {GUCEF_TRACE;

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        MEMMAN_SetOwner( __FILE__, __LINE__, typeid( value_type ).name() );

        #endif

        ::operator delete( p );
    }

    void
    construct( pointer p, const value_type& val )
    {GUCEF_TRACE;

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        MEMMAN_placement_new( __FILE__, __LINE__, sizeof( val ), p, typeid( value_type ).name() );

        #endif

        ::new( static_cast< void* >( p ) ) value_type( val );
    }

    #if defined( GUCEF_MOVE_SEMANTICS_SUPPORTED )
    void
    construct( pointer p, value_type&& val )
    {GUCEF_TRACE;

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        MEMMAN_placement_new( __FILE__, __LINE__, sizeof( val ), p, typeid( value_type ).name() );

        #endif

        ::new( static_cast< void* >( p ) ) value_type( GUCEF_MOVE( val ) );
    }
    #endif

    #if defined( GUCEF_MOVE_SEMANTICS_SUPPORTED )
    template <class... Args>
    void
    construct( pointer p, Args&&... args )
    {GUCEF_TRACE;

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        MEMMAN_placement_new( __FILE__, __LINE__, sizeof( value_type ), p, typeid( value_type ).name() );

        #endif

        ::new( static_cast< void* >( p ) ) value_type( std::forward<Args>(args)... );
    }
    #endif

    template <typename U>
    void
    destroy( U* p )
    {GUCEF_TRACE;

        if ( GUCEF::IsTriviallyDestructible<U>::value )
        {
            return; // no-op for trivial types
        }

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        MEMMAN_ValidatePendingDestructor( __FILE__, __LINE__, p, sizeof(U), typeid( U ).name() );

        #endif

        p->~U();

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        MEMMAN_ValidateFinishedDestructor( __FILE__, __LINE__, p, sizeof(U), typeid( U ).name() );

        #endif
    }

    size_type
    max_size() const GUCEF_NOEXCEPT
    {GUCEF_TRACE;

        return std::numeric_limits<size_type>::max() / sizeof( value_type );
    }

    pointer
    address( reference x ) const
    {GUCEF_TRACE;

        return &x;
    }

    const_pointer
    address( const_reference x ) const
    {GUCEF_TRACE;

        return &x;
    }
};

/*-------------------------------------------------------------------------*/

template <typename Key, typename Value>
class gucef_platform_diagnostic_allocator< std::pair< const Key, Value > >
{
public:

    typedef std::pair<const Key, Value> value_type;
    typedef value_type&                 reference;
    typedef const value_type&           const_reference;
    typedef value_type*                 pointer;
    typedef const value_type*           const_pointer;
    typedef std::size_t                 size_type;
    typedef std::ptrdiff_t              difference_type;

    template <class U>
    struct rebind
    {
        typedef gucef_platform_diagnostic_allocator<U> other;
    };

    gucef_platform_diagnostic_allocator() GUCEF_NOEXCEPT {}
    
    template <class U>
    gucef_platform_diagnostic_allocator( const gucef_platform_diagnostic_allocator<U>& ) GUCEF_NOEXCEPT 
    {GUCEF_TRACE; }

    pointer
    allocate( size_type n, gucef_platform_diagnostic_allocator<void>::const_pointer = 0 )
    {GUCEF_TRACE;

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        return static_cast< pointer >( ::operator new ( n * sizeof( value_type ), __FILE__, __LINE__ ) );

        #else

        return static_cast< pointer >( ::operator new ( n * sizeof( value_type ) ) );

        #endif
    }

    void
    deallocate( pointer p, size_type )
    {GUCEF_TRACE;

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        MEMMAN_SetOwner( __FILE__, __LINE__ );

        #endif

        ::operator delete( p );
    }

    void
    construct( pointer p, const value_type& val )
    {GUCEF_TRACE;

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        MEMMAN_placement_new( __FILE__, __LINE__, sizeof( val ), p, typeid( value_type ).name() );

        #endif

        ::new( static_cast< void* >( p ) ) value_type( val );
    }

    #if defined( GUCEF_MOVE_SEMANTICS_SUPPORTED )
    void
    construct( pointer p, value_type&& val )
    {GUCEF_TRACE;

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        MEMMAN_placement_new( __FILE__, __LINE__, sizeof( val ), p, typeid( value_type ).name() );

        #endif

        // Note: key is const, so it cannot move; value part can still move when provided in the pair.
        ::new( static_cast< void* >( p ) ) value_type( GUCEF_MOVE( val ) );
    }
    #endif

    #if defined( GUCEF_MOVE_SEMANTICS_SUPPORTED )
    template <class... Args>
    void
    construct( pointer p, Args&&... args )
    {GUCEF_TRACE;

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        MEMMAN_placement_new( __FILE__, __LINE__, sizeof( value_type ), p, typeid( value_type ).name() );

        #endif

        ::new( static_cast< void* >( p ) ) value_type( std::forward<Args>(args)... );
    }
    #endif

    template <typename U>
    void
    destroy( U* p )
    {GUCEF_TRACE;

        if ( GUCEF::IsTriviallyDestructible<U>::value )
        {
            return; // no-op for trivial types
        }

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )
        MEMMAN_ValidatePendingDestructor( __FILE__, __LINE__, p, sizeof(U), typeid(U).name() );
        #endif

        p->~U();

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )
        MEMMAN_ValidateFinishedDestructor( __FILE__, __LINE__, p, sizeof(U), typeid(U).name() );
        #endif
    }

    size_type
    max_size() const GUCEF_NOEXCEPT
    {GUCEF_TRACE;

        return std::numeric_limits<size_type>::max() / sizeof( value_type );
    }

    pointer
    address( reference x ) const
    {GUCEF_TRACE;

        return &x;
    }

    const_pointer
    address( const_reference x ) const
    {GUCEF_TRACE;

        return &x;
    }

    // Debugging static_assert to verify specialization is used
    #if __cplusplus >= 201103L  // Only include for C++11 or later
    static_assert(std::is_same<value_type, std::pair<const Key, Value>>::value,
                  "Allocator value_type does not match std::pair<const Key, Value>");
    #endif
};

/*-------------------------------------------------------------------------*/

template <class T, class U>
bool
operator==( gucef_platform_diagnostic_allocator<T> const&, gucef_platform_diagnostic_allocator<U> const& )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

template <class T, class U>
bool
operator!=( gucef_platform_diagnostic_allocator<T> const& x, gucef_platform_diagnostic_allocator<U> const& y )
{GUCEF_TRACE;

    return !(x == y);
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILD SWITCHES                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

// C++11+ allocator alias
//#ifdef GUCEF_USING_KEYWORD_IS_SUPPORTED
//
//  template < typename T >
//  using gucef_allocator =
//  #if ( GUCEF_ALLOCATOR_TYPE == GUCEF_ALLOCATOR_TYPE_PLATFORM_DIAGNOSTIC )
//      GUCEF::gucef_platform_diagnostic_allocator< T >;
//  #else
//      std::allocator< T >;
//  #endif
//
//// C++98 fallback: wrapper struct that derives from the chosen allocator
//#else

  template < typename T >
  class gucef_allocator
  #if ( GUCEF_ALLOCATOR_TYPE == GUCEF_ALLOCATOR_TYPE_PLATFORM_DIAGNOSTIC )
      : public GUCEF::gucef_platform_diagnostic_allocator< T >
  {
public:
    typedef GUCEF::gucef_platform_diagnostic_allocator< T > base_type;
  #else
      : public std::allocator< T >
  {
public:
    typedef std::allocator< T > base_type;

    // When base_type is std::allocator<...>, allow construction from std::allocator<U> as well.
    template <typename U>
    gucef_allocator( const std::allocator<U>& src ) GUCEF_NOEXCEPT
        : base_type( src ) { GUCEF_TRACE; }

  #endif

    typedef typename base_type::value_type      value_type;
    typedef typename base_type::pointer         pointer;
    typedef typename base_type::const_pointer   const_pointer;
    typedef typename base_type::size_type       size_type;
    typedef typename base_type::difference_type difference_type;

    // rebind (required by older STL)
    template <class U>
    struct rebind
    {
        typedef gucef_allocator<U> other;
    };

    gucef_allocator() GUCEF_NOEXCEPT
        : base_type() { GUCEF_TRACE; }

    gucef_allocator( const gucef_allocator& src ) GUCEF_NOEXCEPT
        : base_type( static_cast<const base_type&>( src ) ) { GUCEF_TRACE; }

    template < typename U >
    gucef_allocator( const gucef_allocator< U >& src ) GUCEF_NOEXCEPT
        : base_type( static_cast<const typename gucef_allocator<U>::base_type&>( src ) ) { GUCEF_TRACE; }

    // Allow construction from the underlying base allocator (std::allocator<T> or gucef_platform_diagnostic_allocator<T>)
    gucef_allocator( const base_type& src ) GUCEF_NOEXCEPT
        : base_type( src ) { GUCEF_TRACE; }
  };

//#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_ALLOCATOR_H ? */
