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

#ifndef GUCEF_CPP_H
#define GUCEF_CPP_H

/*
 *  Macros to allow the GUCEF platform to use some newer C++ features while
 *  still supporting as old a compiler as possible for as wide a reach as possible
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"        /* GUCEF configuration */
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"      /* GUCEF platform compilation targets */
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if defined(__cplusplus)
namespace GUCEF {
#endif /* defined(__cplusplus) */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#if defined(__cplusplus)
    #undef GUCEF_RVALUE_REFERENCES_SUPPORTED
    #if defined( _MSC_VER )  /* MS visual studio */
        #if _MSC_VER >= 1600  /* Visual Studio 2010 and beyond support the feature regardless of configured C++ version */
            #define GUCEF_RVALUE_REFERENCES_SUPPORTED 1
        #endif
    #elif defined( __GNUG__ )  /* GNU C++ compiler */
        #if __cpp_rvalue_references >= 200610 /* Earlier Clang versions may support the feature regardless of configured C++ version */
            #define GUCEF_RVALUE_REFERENCES_SUPPORTED 1
        #endif
    #else
        /* Unknown compiler, fall back to cpp version check */
        #if __cplusplus >= 201103L  /* >= C++ 11 : This is not fullproof since not every compiler truly supports the spec */
            #define GUCEF_RVALUE_REFERENCES_SUPPORTED 1
        #endif
    #endif
#endif

/*-------------------------------------------------------------------------*/

#undef GUCEF_MOVE_SEMANTICS_SUPPORTED
#ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED
  #define GUCEF_MOVE_SEMANTICS_SUPPORTED 1
#endif

/*-------------------------------------------------------------------------*/

#if defined(__cplusplus)
  #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
      #define GUCEF_MOVE( m ) std::move( m )
      #define GUCEF_FORWARD( T, m ) std::forward< T >( m )
  #else
      #define GUCEF_MOVE( m ) m
      #define GUCEF_FORWARD( T, m ) m
  #endif
#endif

/*-------------------------------------------------------------------------*/

#if defined(__cplusplus)
    #undef GUCEF_NOEXCEPT_IS_SUPPORTED
    #if __cplusplus >= 201103L /* >= C++ 11 : This is not fullproof since not every compiler truly supports the spec */
        #if defined( __GNUG__ )  /* GNU C++ compiler */
            #if ( GCC_VERSION > 40600 )
                #define GUCEF_NOEXCEPT_IS_SUPPORTED 1
            #endif
        #else
            /* Rely on only cpp version check */
            #define GUCEF_NOEXCEPT_IS_SUPPORTED 1
        #endif
    #else /* some compilers still support it before official inclusion in C++11 */
        #if defined(__clang__)
            #if __has_feature(cxx_noexcept)
                #define GUCEF_NOEXCEPT_IS_SUPPORTED 1
            #endif
        #else
            #if defined(__GXX_EXPERIMENTAL_CXX0X__) && __GNUC__ * 10 + __GNUC_MINOR__ >= 46 || \
                defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023026
                #define GUCEF_NOEXCEPT_IS_SUPPORTED 1
            #endif
        #endif
    #endif
#endif

#if defined(__cplusplus)
  #ifdef GUCEF_NOEXCEPT_IS_SUPPORTED
    #define GUCEF_NOEXCEPT noexcept( true )
  #else
    #define GUCEF_NOEXCEPT
  #endif
#endif

/*-------------------------------------------------------------------------*/

#if defined(__cplusplus)
    #undef GUCEF_COPY_CONSTRUCTABLE_CHECK_IS_SUPPORTED
    #if __cplusplus >= 201103L /* std::is_copy_constructible is C++11 and newer only */
        #define GUCEF_COPY_CONSTRUCTABLE_CHECK_IS_SUPPORTED 1
    #endif
#endif

/*-------------------------------------------------------------------------*/

#if defined(__cplusplus)
  #undef GUCEF_USING_KEYWORD_IS_SUPPORTED

  /* Standard-compliant C++11 and later (all compilers, including GCC/Clang) */
  #if ( __cplusplus >= 201103L )
    #define GUCEF_USING_KEYWORD_IS_SUPPORTED 1

  /* Older MSVC: use _MSC_VER guard because it reports __cplusplus poorly */
  #elif defined( _MSC_VER ) && ( _MSC_VER >= 1600 )

    /* VS2010+ have `using` type alias support in C++11-ish mode */
    #define GUCEF_USING_KEYWORD_IS_SUPPORTED 1

  #endif

#endif

/*-------------------------------------------------------------------------*/

#undef GUCEF_VIRTUAL_OVERRIDE_IS_SUPPORTED
#if defined(__cplusplus)
  #if ( __cplusplus >= 201103L ) /* Are we using C++11 or newer */
    #define GUCEF_VIRTUAL_OVERRIDE_IS_SUPPORTED  1
  #else
    #if defined( _MSC_VER )   /* Visual Studio has support even if the language version specified is older */
      #if ( _MSC_VER > 1600 ) /* Visual Studio 2010 (VC10) */
        #define GUCEF_VIRTUAL_OVERRIDE_IS_SUPPORTED  1
      #endif
    #endif
  #endif
#endif

#if defined(__cplusplus)
  #ifdef GUCEF_VIRTUAL_OVERRIDE_IS_SUPPORTED
    #define GUCEF_VIRTUAL_OVERRIDE override
  #else
    #define GUCEF_VIRTUAL_OVERRIDE
  #endif
#endif

/*-------------------------------------------------------------------------*/

#undef GUCEF_PREDICT_IS_SUPPORTED
#undef GUCEF_PREDICT_TRUE
#undef GUCEF_PREDICT_FALSE
#if defined( GUCEF_PREDICT_TRUE ) || defined( GUCEF_PREDICT_FALSE )
  #error GUCEF_PREDICT_(TRUE|FALSE) was previously defined
#endif
#if ( defined(_MSC_VER) && _MSVC_LANG>= 202002L ) || __cplusplus >= 202002L  /* Are we using C++20 or newer? If so use attributes */
  #define GUCEF_PREDICT_TRUE( x ) ( x ) [[likely]]
  #define GUCEF_PREDICT_FALSE( x ) ( x ) [[unlikely]]
  #define GUCEF_PREDICT_IS_SUPPORTED 1
#elif ( ( GUCEF_COMPILER == GUCEF_COMPILER_GNUC ) && ( GCC_VERSION >= 30000 ) )
  #define GUCEF_PREDICT_TRUE( x ) (__builtin_expect(false || (x), true))
  #define GUCEF_PREDICT_FALSE( x ) (__builtin_expect(false || (x), false))
  #define GUCEF_PREDICT_IS_SUPPORTED 1
#elif ( ( GUCEF_COMPILER == GUCEF_COMPILER_CLANG ) && ( GUCEF_COMP_VER >= 30900 ) )
  #define GUCEF_PREDICT_TRUE( x ) (__builtin_expect(false || (x), true))
  #define GUCEF_PREDICT_FALSE( x ) (__builtin_expect(false || (x), false))
  #define GUCEF_PREDICT_IS_SUPPORTED 1
#else
  #define GUCEF_PREDICT_TRUE( x ) ( x )
  #define GUCEF_PREDICT_FALSE( x ) ( x )
#endif

/*-------------------------------------------------------------------------*/

#if __cplusplus >= 201103L
    #define GUCEF_DELETED_MEMBER = delete
#else
    #define GUCEF_DELETED_MEMBER
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEMPLATES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if defined(__cplusplus)

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

template<bool B>
struct BoolToType;

template<>
struct BoolToType< true >
{
    typedef plain_true type;
};

template<>
struct BoolToType< false >
{
    typedef plain_false type;
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
 *  Arity tags (number of arguments) for function_traits.
 */
struct arity_0 {};
struct arity_1 {};
struct arity_2 {};
struct arity_3 {};
struct arity_4 {};
struct arity_5 {};
struct arity_6 {};
struct arity_7 {};
struct arity_8 {};
struct arity_9 {};
struct arity_10 {};

/*-------------------------------------------------------------------------*/

//— function_traits for free functions (arity 0…10)
template< typename Sig >     struct function_traits;

// arity 0
template<typename R>
struct function_traits<R()> {
    typedef R       result_type;
    typedef arity_0 tag;
};

// arity 1
template<typename R, typename A1>
struct function_traits<R(A1)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef arity_1 tag;
};

// arity 2
template<typename R, typename A1, typename A2>
struct function_traits<R(A1, A2)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef arity_2 tag;
};

// arity 3
template<typename R, typename A1, typename A2, typename A3>
struct function_traits<R(A1, A2, A3)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef arity_3 tag;
};

// arity 4
template<typename R, typename A1, typename A2, typename A3, typename A4>
struct function_traits<R(A1, A2, A3, A4)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef arity_4 tag;
};

// arity 5
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
struct function_traits<R(A1, A2, A3, A4, A5)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef arity_5 tag;
};

// arity 6
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct function_traits<R(A1, A2, A3, A4, A5, A6)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef arity_6 tag;
};

// arity 7
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
struct function_traits<R(A1, A2, A3, A4, A5, A6, A7)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef A7      arg7_type;
    typedef arity_7 tag;
};

// arity 8
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
struct function_traits<R(A1, A2, A3, A4, A5, A6, A7, A8)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef A7      arg7_type;
    typedef A8      arg8_type;
    typedef arity_8 tag;
};

// arity 9
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
struct function_traits<R(A1, A2, A3, A4, A5, A6, A7, A8, A9)> {
    typedef R       result_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef A7      arg7_type;
    typedef A8      arg8_type;
    typedef A9      arg9_type;
    typedef arity_9 tag;
};

// arity 10
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
struct function_traits<R(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)> {
    typedef R        result_type;
    typedef A1       arg1_type;
    typedef A2       arg2_type;
    typedef A3       arg3_type;
    typedef A4       arg4_type;
    typedef A5       arg5_type;
    typedef A6       arg6_type;
    typedef A7       arg7_type;
    typedef A8       arg8_type;
    typedef A9       arg9_type;
    typedef A10      arg10_type;
    typedef arity_10 tag;
};

/*-------------------------------------------------------------------------*/

//— member_function_traits for non-const and const member function pointers (arity 0…10)
template< typename MF > struct member_function_traits;

// non-const, arity 0
template< typename R, typename C >
struct member_function_traits< R (C::*)() > {
    typedef R       result_type;
    typedef C       class_type;
    typedef arity_0 tag;
};

// const, arity 0
template< typename R, typename C >
struct member_function_traits< R (C::*)() const > {
    typedef R       result_type;
    typedef C       class_type;
    typedef arity_0 tag;
};

// non-const, arity 1
template< typename R, typename C, typename A1 >
struct member_function_traits< R (C::*)(A1) > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef arity_1 tag;
};

// const, arity 1
template< typename R, typename C, typename A1 >
struct member_function_traits< R (C::*)(A1) const > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef arity_1 tag;
};

// non-const, arity 2
template< typename R, typename C, typename A1, typename A2 >
struct member_function_traits< R (C::*)(A1, A2) > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef arity_2 tag;
};

// const, arity 2
template< typename R, typename C, typename A1, typename A2 >
struct member_function_traits< R (C::*)(A1, A2) const > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef arity_2 tag;
};

// non-const, arity 3
template< typename R, typename C, typename A1, typename A2, typename A3 >
struct member_function_traits< R (C::*)(A1, A2, A3) > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef arity_3 tag;
};

// const, arity 3
template< typename R, typename C, typename A1, typename A2, typename A3 >
struct member_function_traits< R (C::*)(A1, A2, A3) const > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef arity_3 tag;
};

// non-const, arity 4
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4) > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef arity_4 tag;
};

// const, arity 4
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4) const > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef arity_4 tag;
};

// non-const, arity 5
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4, A5) > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef arity_5 tag;
};

// const, arity 5
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4, A5) const > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef arity_5 tag;
};

// non-const, arity 6
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4, A5, A6) > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef arity_6 tag;
};

// const, arity 6
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4, A5, A6) const > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef arity_6 tag;
};

// non-const, arity 7
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4, A5, A6, A7) > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef A7      arg7_type;
    typedef arity_7 tag;
};

// const, arity 7
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4, A5, A6, A7) const > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef A7      arg7_type;
    typedef arity_7 tag;
};

// non-const, arity 8
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4, A5, A6, A7, A8) > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef A7      arg7_type;
    typedef A8      arg8_type;
    typedef arity_8 tag;
};

// const, arity 8
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4, A5, A6, A7, A8) const > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef A7      arg7_type;
    typedef A8      arg8_type;
    typedef arity_8 tag;
};

// non-const, arity 9
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4, A5, A6, A7, A8, A9) > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef A7      arg7_type;
    typedef A8      arg8_type;
    typedef A9      arg9_type;
    typedef arity_9 tag;
};

// const, arity 9
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const > {
    typedef R       result_type;
    typedef C       class_type;
    typedef A1      arg1_type;
    typedef A2      arg2_type;
    typedef A3      arg3_type;
    typedef A4      arg4_type;
    typedef A5      arg5_type;
    typedef A6      arg6_type;
    typedef A7      arg7_type;
    typedef A8      arg8_type;
    typedef A9      arg9_type;
    typedef arity_9 tag;
};

// non-const, arity 10
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) > {
    typedef R        result_type;
    typedef C        class_type;
    typedef A1       arg1_type;
    typedef A2       arg2_type;
    typedef A3       arg3_type;
    typedef A4       arg4_type;
    typedef A5       arg5_type;
    typedef A6       arg6_type;
    typedef A7       arg7_type;
    typedef A8       arg8_type;
    typedef A9       arg9_type;
    typedef A10      arg10_type;
    typedef arity_10 tag;
};

// const, arity 10
template< typename R, typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10 >
struct member_function_traits< R (C::*)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const > {
    typedef R        result_type;
    typedef C        class_type;
    typedef A1       arg1_type;
    typedef A2       arg2_type;
    typedef A3       arg3_type;
    typedef A4       arg4_type;
    typedef A5       arg5_type;
    typedef A6       arg6_type;
    typedef A7       arg7_type;
    typedef A8       arg8_type;
    typedef A9       arg9_type;
    typedef A10      arg10_type;
    typedef arity_10 tag;
};

#endif /* defined(__cplusplus) */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if defined(__cplusplus)
}; /* namespace GUCEF */
#endif /* defined(__cplusplus) */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CPP_H ? */

