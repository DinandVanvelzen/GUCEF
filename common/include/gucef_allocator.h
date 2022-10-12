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

template < class T > class basic_allocator;

template <>
class basic_allocator<void>
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
        typedef basic_allocator<U> other;
    };
};

template < class T >
class basic_allocator
{
public:
    typedef T                 value_type;
    typedef value_type&       reference;
    typedef value_type const& const_reference;
    typedef value_type*       pointer;
    typedef value_type const* const_pointer;
    typedef std::size_t       size_type;
    typedef std::ptrdiff_t    difference_type;

    template <class U>
    struct rebind
    {
        typedef basic_allocator<U> other;
    };

    basic_allocator() GUCEF_NOEXCEPT {}  // not required, unless used
    template <class U> basic_allocator( basic_allocator<U> const& u ) GUCEF_NOEXCEPT 
    {GUCEF_TRACE; }

    pointer
    allocate( size_type n, basic_allocator<void>::const_pointer = 0 )
    {GUCEF_TRACE;

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        return static_cast< pointer >( ::operator new ( (n*sizeof(value_type)), __FILE__, __LINE__ ) );

        #else

        return static_cast< pointer >( ::operator new ( n*sizeof(value_type)) );

        #endif
    }

    void
    deallocate( pointer p, size_type )
    {GUCEF_TRACE;

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        MEMMAN_SetOwner( __FILE__, __LINE__ );

        #endif

        ::operator delete(p);
    }

    void
    construct( pointer p, value_type const& val )
    {GUCEF_TRACE;

        #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED )

        MEMMAN_placement_new( __FILE__, __LINE__, sizeof( val ), p ); 
        
        #endif

        ::new(p) value_type(val);
    }

    void
    destroy( pointer p )
    {GUCEF_TRACE;      

        p->~value_type();
    }

    size_type
    max_size() const GUCEF_NOEXCEPT
    {GUCEF_TRACE;

        return std::numeric_limits<size_type>::max() / sizeof(value_type);
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

template <class T, class U>
bool
operator==( basic_allocator<T> const&, basic_allocator<U> const& )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

template <class T, class U>
bool
operator!=( basic_allocator<T> const& x, basic_allocator<U> const& y )
{GUCEF_TRACE;

    return !(x == y);
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_ALLOCATOR_H ? */
