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

#ifndef GUCEF_BASICHELPERS_H
#include "gucef_basichelpers.h"
#define GUCEF_BASICHELPERS_H
#endif /* GUCEF_BASICHELPERS_H ? */

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

template < typename T >
struct IsCharPtr
{
    enum { value = TypesAreExactlySame< T, char* >::value ||
                   TypesAreExactlySame< T, const char* >::value };
};

template < typename T >
struct IsWCharPtr
{
    enum { value = TypesAreExactlySame< T, wchar_t* >::value ||
                   TypesAreExactlySame< T, const wchar_t* >::value };
};

template < typename T >
struct IsVoidPtr
{
    enum { value = TypesAreExactlySame< T, void* >::value ||
                   TypesAreExactlySame< T, const void* >::value };
};

/*-------------------------------------------------------------------------*/

/**
 *  C++98 compatible SFINAE template helper
 *  Allows checking if T is itself a pointer type
 *  In >= C++11 the analog would be std::is_pointer< T >
 */
template < class T >
struct TypeIsPointerType
{
    enum { value = GUCEF::is_unqualified_pointer< typename GUCEF::remove_cv< T >::type >::value };
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
 *
 *  Note:
 *  - Works with cv-qualified types via remove_cv.
 *  - Covers both built-in unsigned types and the platform's UInt{8,16,32,64}.
 */
template < class T >
struct TypeIsUnsigned
{
    typedef typename remove_cv< T >::type no_cv_t;

    enum { value =
           /* Built-in unsigneds (excluding bool and char) */
           TypesAreExactlySame< no_cv_t, unsigned char >::value  ||
           TypesAreExactlySame< no_cv_t, unsigned short >::value ||
           TypesAreExactlySame< no_cv_t, unsigned int >::value   ||
           TypesAreExactlySame< no_cv_t, unsigned long >::value
        #if defined(_MSC_VER) || defined(__GNUC__)
           || TypesAreExactlySame< no_cv_t, unsigned long long >::value
        #endif
           /* Project fixed-width unsigned aliases */
           || TypesAreExactlySame< no_cv_t, UInt8  >::value
           || TypesAreExactlySame< no_cv_t, UInt16 >::value
           || TypesAreExactlySame< no_cv_t, UInt32 >::value
           || TypesAreExactlySame< no_cv_t, UInt64 >::value
    };
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

