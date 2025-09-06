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

#ifndef GUCEF_STL_H
#define GUCEF_STL_H

#ifdef __cplusplus

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <vector>
#include <set>
#include <string>
#include <functional>           // For std::less, std::equal_to, etc.

// Determine if we have C++11 support for template aliases and unordered_map
#undef GUCEF_STL_HAVE_CXX11_FEATURES
#if defined(__cplusplus)
    // Check for C++11 standard
    #if __cplusplus >= 201103L
        #define GUCEF_STL_HAVE_CXX11_FEATURES 1
    // Check for specific compiler support even if __cplusplus is not set correctly
    #elif defined(_MSC_VER) && _MSC_VER >= 1600  // VS2010+
        #define GUCEF_STL_HAVE_CXX11_FEATURES 1
    #elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))  // GCC 4.3+
        // GCC 4.3+ has template aliases support, but may need -std=c++11
        #ifdef __GXX_EXPERIMENTAL_CXX0X__
            #define GUCEF_STL_HAVE_CXX11_FEATURES 1
        #elif __cplusplus > 199711L  // Some indication of C++11 mode
            #define GUCEF_STL_HAVE_CXX11_FEATURES 1
        #endif
    #elif defined(__clang__)
        // Clang generally supports template aliases when C++11 is enabled
        #if __has_feature(cxx_alias_templates)
            #define GUCEF_STL_HAVE_CXX11_FEATURES 1
        #elif __cplusplus > 199711L
            #define GUCEF_STL_HAVE_CXX11_FEATURES 1
        #endif
    #endif
#endif

#ifdef GUCEF_STL_HAVE_CXX11_FEATURES
#include <unordered_map>
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

#ifndef GUCEF_ALLOCATOR_H
#include "gucef_allocator.h"       
#define GUCEF_ALLOCATOR_H
#endif /* GUCEF_ALLOCATOR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      STL-COMPATIBLE CONTAINER ALIASES                                   //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  STL-compatible container aliases using gucef_allocator by default
 *  This provides a cleaner toggle - you can use GUCEF::vector<int> instead of std::vector<int>
 *  and get the gucef_allocator automatically
 *
 *  For C++11 and above, we use template aliases (using declarations)
 *  For C++98, we use class-based template forwarding
 */

#ifdef GUCEF_STL_HAVE_CXX11_FEATURES  // C++11+ with template alias support

// Vector with gucef_allocator - STL-compatible syntax (C++11+)
template < typename T, typename Allocator = gucef_allocator< T > >
using vector = std::vector< T, Allocator >;

// Set with gucef_allocator - STL-compatible syntax (C++11+)
template < typename T, typename Compare = std::less< T >, typename Allocator = gucef_allocator< T > >
using set = std::set< T, Compare, Allocator >;

// Map with gucef_allocator - STL-compatible syntax (C++11+)
template < typename Key, typename Value, typename Compare = std::less< Key >, typename Allocator = gucef_allocator< std::pair< const Key, Value > > >
using map = std::map< Key, Value, Compare, Allocator >;

// Unordered map with gucef_allocator - STL-compatible syntax (C++11+)
template < typename Key, typename Value, typename Hash = std::hash< Key >, typename KeyEqual = std::equal_to< Key >, typename Allocator = gucef_allocator< std::pair< const Key, Value > > >
using unordered_map = std::unordered_map< Key, Value, Hash, KeyEqual, Allocator >;

// String with gucef_allocator - STL-compatible syntax (C++11+)
template < typename CharT, typename Traits = std::char_traits< CharT >, typename Allocator = gucef_allocator< CharT > >
using basic_string = std::basic_string< CharT, Traits, Allocator >;

// Common string types with gucef_allocator (C++11+)
using string = basic_string< char >;
using wstring = basic_string< wchar_t >;

#else  // C++98 compatibility

// Vector with gucef_allocator - STL-compatible syntax (C++98)
template < typename T, typename Allocator = gucef_allocator< T > >
struct vector : public std::vector< T, Allocator >
{
    typedef std::vector< T, Allocator > base_type;
    typedef typename base_type::size_type size_type;
    typedef typename base_type::value_type value_type;
    typedef typename base_type::reference reference;
    typedef typename base_type::const_reference const_reference;
    typedef typename base_type::iterator iterator;
    typedef typename base_type::const_iterator const_iterator;
    
    vector() : base_type() {}
    
    explicit vector( const Allocator& alloc ) : base_type( alloc ) {}
    
    explicit vector( size_type n, const T& value = T(), const Allocator& alloc = Allocator() )
        : base_type( n, value, alloc ) {}
    
    template< typename InputIterator >
    vector( InputIterator first, InputIterator last, const Allocator& alloc = Allocator() )
        : base_type( first, last, alloc ) {}
    
    vector( const vector& other ) : base_type( other ) {}
    
    vector& operator=( const vector& other )
    {
        if ( this != &other )
            base_type::operator=( other );
        return *this;
    }
};

// Set with gucef_allocator - STL-compatible syntax (C++98)
template < typename T, typename Compare = std::less< T >, typename Allocator = gucef_allocator< T > >
struct set : public std::set< T, Compare, Allocator >
{
    typedef std::set< T, Compare, Allocator > base_type;
    typedef typename base_type::size_type size_type;
    typedef typename base_type::value_type value_type;
    typedef typename base_type::reference reference;
    typedef typename base_type::const_reference const_reference;
    typedef typename base_type::iterator iterator;
    typedef typename base_type::const_iterator const_iterator;
    typedef typename base_type::key_type key_type;
    
    set() : base_type() {}
    
    explicit set( const Compare& comp, const Allocator& alloc = Allocator() ) : base_type( comp, alloc ) {}
    
    template< typename InputIterator >
    set( InputIterator first, InputIterator last, const Compare& comp = Compare(), const Allocator& alloc = Allocator() )
        : base_type( first, last, comp, alloc ) {}
    
    set( const set& other ) : base_type( other ) {}
    
    set& operator=( const set& other )
    {
        if ( this != &other )
            base_type::operator=( other );
        return *this;
    }
};

// Map with gucef_allocator - STL-compatible syntax (C++98)
template < typename Key, typename Value, typename Compare = std::less< Key >, typename Allocator = gucef_allocator< std::pair< const Key, Value > > >
struct map : public std::map< Key, Value, Compare, Allocator >
{
    typedef std::map< Key, Value, Compare, Allocator > base_type;
    typedef typename base_type::size_type size_type;
    typedef typename base_type::value_type value_type;
    typedef typename base_type::reference reference;
    typedef typename base_type::const_reference const_reference;
    typedef typename base_type::iterator iterator;
    typedef typename base_type::const_iterator const_iterator;
    typedef typename base_type::key_type key_type;
    typedef typename base_type::mapped_type mapped_type;
    
    map() : base_type() {}
    
    explicit map( const Compare& comp, const Allocator& alloc = Allocator() ) : base_type( comp, alloc ) {}
    
    template< typename InputIterator >
    map( InputIterator first, InputIterator last, const Compare& comp = Compare(), const Allocator& alloc = Allocator() )
        : base_type( first, last, comp, alloc ) {}
    
    map( const map& other ) : base_type( other ) {}
    
    map& operator=( const map& other )
    {
        if ( this != &other )
            base_type::operator=( other );
        return *this;
    }
};

// String with gucef_allocator - STL-compatible syntax (C++98)
template < typename CharT, typename Traits = std::char_traits< CharT >, typename Allocator = gucef_allocator< CharT > >
struct basic_string : public std::basic_string< CharT, Traits, Allocator >
{
    typedef std::basic_string< CharT, Traits, Allocator > base_type;
    typedef typename base_type::size_type size_type;
    typedef typename base_type::value_type value_type;
    typedef typename base_type::reference reference;
    typedef typename base_type::const_reference const_reference;
    typedef typename base_type::iterator iterator;
    typedef typename base_type::const_iterator const_iterator;
    
    basic_string() : base_type() {}
    
    explicit basic_string( const Allocator& alloc ) : base_type( alloc ) {}
    
    basic_string( const CharT* s, const Allocator& alloc = Allocator() ) : base_type( s, alloc ) {}
    
    basic_string( const CharT* s, size_type n, const Allocator& alloc = Allocator() ) : base_type( s, n, alloc ) {}
    
    basic_string( size_type n, CharT c, const Allocator& alloc = Allocator() ) : base_type( n, c, alloc ) {}
    
    basic_string( const basic_string& other ) : base_type( other ) {}
    
    basic_string& operator=( const basic_string& other )
    {
        if ( this != &other )
            base_type::operator=( other );
        return *this;
    }
    
    basic_string& operator=( const CharT* s )
    {
        base_type::operator=( s );
        return *this;
    }
};

// Common string types with gucef_allocator (C++98)
typedef basic_string< char > string;
typedef basic_string< wchar_t > wstring;

#endif  // C++98 vs C++11+ version check

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_STL_H ? */
