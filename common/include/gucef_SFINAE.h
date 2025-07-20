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

#ifndef GUCEF_SFINAE_H
#define GUCEF_SFINAE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TYPES_H
#include "gucef_types.h"
#define GUCEF_TYPES_H
#endif /* GUCEF_TYPES_H ? */

#ifndef GUCEF_CPP_H
#include "gucef_cpp.h"
#define GUCEF_CPP_H
#endif /* GUCEF_CPP_H ? */

#if defined(__cplusplus)
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEMPLATES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

struct plain_true {};
struct plain_false {};

template <typename T>
struct true_or_false_type
{
    // For the compile time comparison.
    typedef char    yes[1];
    typedef yes     no[2];
};

template <typename T>
struct false_type : true_or_false_type< T >
{
    typedef plain_false  value_type;
    enum { value = false };
};

template <typename T>
struct true_type : true_or_false_type< T >
{
    typedef plain_false  value_type;
    enum { value = true };
};

template <typename T>
struct remove_const
{
    typedef T type;
};

template <typename T>
struct remove_const<const T>
{
    typedef T type;
};

template <typename T>
struct remove_volatile
{
    typedef T type;
};

template <typename T>
struct remove_volatile< volatile T >
{
    typedef T type;
};

template <typename T>
struct remove_cv : remove_const< typename remove_volatile< T >::type > {};

template <typename T>
struct is_unqualified_pointer
{
    enum { value = false_type< T >::value };
};

template <typename T>
struct is_unqualified_pointer<T*>
{
    enum { value = true_type< T >::value };
};


template< class T > struct remove_pointer                    {typedef T type;};
template< class T > struct remove_pointer<T*>                {typedef T type;};
template< class T > struct remove_pointer<T* const>          {typedef T type;};
template< class T > struct remove_pointer<T* volatile>       {typedef T type;};
template< class T > struct remove_pointer<T* const volatile> {typedef T type;};

/*-------------------------------------------------------------------------*/


/**
 *  C++98 backfill for C++11's enable_if<>
 */
template<bool B, class T = void>
struct EnableIf {};

template<class T>
struct EnableIf< true, T > { typedef T type; };

template<bool B, bool B2, class T = void, class T2 = void>
struct EnableIf2 {};

template<class T, class T2>
struct EnableIf2< true, true, T, T2 > { typedef T type; typedef T2 type2; };

template<bool B, bool B2, class T = void, class T2 = void>
struct EnableIfFirstOf2 {};

template<class T, class T2>
struct EnableIfFirstOf2< true, false, T, T2 > { typedef T type; typedef T2 type2; };

template<bool B, class T = void>
struct EnableIfNot {};

template<class T>
struct EnableIfNot< false, T > { typedef T type; };

template<bool B, bool B2, class T = void, class T2 = void>
struct EnableIfNot2 {};

template<class T, class T2>
struct EnableIfNot2< false, false, T, T2 > { typedef T type; typedef T2 type2; };

/*-------------------------------------------------------------------------*/

//
// Primary template: different types
template <typename A, typename B>
struct TypesAreExactlySame
{
    typedef struct false_type< A >                  value_type;
    typedef typename false_type< A >::value_type    plain_value_type;
    enum { value = value_type::value };
};

//
// Specialization: same type
template <typename T>
struct TypesAreExactlySame<T, T>
{
    typedef struct true_type< T >                   value_type;
    typedef typename true_type< T >::value_type     plain_value_type;
    enum { value = value_type::value };
};

/*-------------------------------------------------------------------------*/

/**
 *  C++98 compatible SFINAE template helper
 *  Allows checking if T is itself a pointer type
 *  In >= C++11 the analog would be std::is_pointer_type< T >
 */
template < class T >
struct TypeIsPointerType
{
    // For the compile time comparison.
    typedef char    yes[1];
    typedef yes     no[2];

    template < typename TestPtrType > static yes& test( typename remove_cv< TestPtrType >::type* /*unused*/ ) { static yes result; return result; }
    template < typename TestPtrType > static yes& test( void* /*unused*/ ) { static yes result; return result; }
    template < typename TestPtrType > static no&  test( ... ) { static no result; return result; }

    // The constant used as a return value for the test.
    enum { value = sizeof( test< T >( T() ) ) == sizeof( yes ) };
};

/*-------------------------------------------------------------------------*/

/**
 *  C++98 compatible SFINAE template helper
 *  Allows checking if T is not just a related inheritance type
 *  but is actually a derived class
 */
template < class DerivedClassType, class BaseClassType >
struct TypeIsDerivedFrom 
{
    // For the compile time comparison.
    typedef char    yes[1];
    typedef yes     no[2];
    
    template < typename TestType > static yes& test( const BaseClassType* ) { static yes result; return result; }
    template < typename TestType > static no&  test( ... ) { static no result; return result; }

    // The constant used as a return value for the test.
    enum { value = sizeof( test( static_cast< DerivedClassType* >( GUCEF_NULL ) ) ) == sizeof( yes ) };
};

/*--------------------------------------------------------------------------*/

/**
 *  C++98 compatible SFINAE template helper
 *  Allows checking if T has a publicly accessible copy constructor
 */
template < class TestType >
struct TypeHasCopyConstructor
{
    // For the compile-time comparison.
    typedef char    yes[1];
    typedef yes     no[2];

    template < typename U > 
    static yes& test( int, U(*) = new U(*(U*)0) ) { static yes result; return result; }

    template < typename U > 
    static no& test( ... ) { static no result; return result; }

    // The constant used as a return value for the test.
    enum { value = sizeof( test<TestType>( 0 ) ) == sizeof( yes ) };
};

/*--------------------------------------------------------------------------*/

/**
 *  C++98 compatible SFINAE template helper
 *  Allows checking if T has a publicly accessible default constructor
 */
template < class TestType >
struct TypeHasDefaultConstructor
{
    // For the compile-time comparison.
    typedef char    yes[1];
    typedef yes     no[2];

    template < typename U > 
    static yes& test(int, U(*) = new U);  // Tries to instantiate the default constructor

    template < typename U > 
    static no& test(...);  // Fallback when the default constructor is missing or inaccessible

    // The constant used as a return value for the test.
    enum { value = sizeof(test<TestType>(0)) == sizeof(yes) };
};

/*--------------------------------------------------------------------------*/

/**
 *  C++98 compatible SFINAE template helper
 *  Allows checking if T is an unsigned type
 */
template < class T >
struct TypeIsUnsigned
{
    // For the compile time comparison.
    typedef char    yes[1];
    typedef yes     no[2];

    template < typename TestPtrType >
    static yes& test(typename EnableIf<sizeof(TestPtrType) == sizeof(UInt8), UInt8>::type* /* unused */) { static yes result; return result; }

    template < typename TestPtrType >
    static yes& test(typename EnableIf<sizeof(TestPtrType) == sizeof(UInt16), UInt16>::type* /* unused */) { static yes result; return result; }

    template < typename TestPtrType >
    static yes& test(typename EnableIf<sizeof(TestPtrType) == sizeof(UInt32), UInt32>::type* /* unused */) { static yes result; return result; }

    template < typename TestPtrType >
    static yes& test(typename EnableIf<sizeof(TestPtrType) == sizeof(UInt64), UInt64>::type* /* unused */) { static yes result; return result; }

    template < typename TestPtrType >
    static no& test(...) { static no result; return result; }

    // The constant used as a return value for the test.
    enum { value = sizeof( test< T >( T() ) ) == sizeof( yes ) };
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
#endif /* __cplusplus */

#endif /* GUCEF_SFINAE_H ? */

