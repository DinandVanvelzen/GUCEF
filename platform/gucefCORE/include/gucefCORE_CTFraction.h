/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CTFRACTION_H
#define GUCEF_CORE_CTFRACTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <iostream>
#include <stdexcept>

#if __cplusplus >= 201703L // numeric header was added in C++17
  #include <numeric> // For std::gcd
#endif

#include <algorithm>

#ifndef GUCEF_EXCEPTIONS_H
#include "gucef_exceptions.h"
#define GUCEF_EXCEPTIONS_H
#endif /* GUCEF_EXCEPTIONS_H ? */

#ifndef GUCEF_SFINAE_H
#include "gucef_SFINAE.h"
#define GUCEF_SFINAE_H
#endif /* GUCEF_SFINAE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_C_VARIANTDATA_H
#include "gucefCORE_c_variantdata.h"
#define GUCEF_CORE_C_VARIANTDATA_H
#endif /* GUCEF_CORE_C_VARIANTDATA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Templated fraction class that supports basic arithmetic operations.
 */
template < typename NumeratorType, typename DenominatorType >
class CTFraction
{
    public:

    typedef NumeratorType   TNumeratorType;
    typedef DenominatorType TDenominatorType;


    CTFraction( NumeratorType numerator = 0, DenominatorType denominator = 1 );
    CTFraction( TInt32T2Fraction int32t2_data );
    CTFraction( TUInt32T2Fraction uint32t2_data );
    CTFraction( const TInt64T2Fraction& int64_int32_data );
    CTFraction( const TUInt64T2Fraction& uint64_uint32_data );
    CTFraction( const TInt64WInt32Fraction& int64_int32_data );
    CTFraction( const TUInt64WUInt32Fraction& uint64_uint32_data );

    NumeratorType GetNumerator( void) const;
    DenominatorType GetDenominator( void ) const;

    bool ToMixedFraction( NumeratorType& wholePart, CTFraction< NumeratorType, DenominatorType >& fractionalPart ) const;
    void FromMixedFraction( NumeratorType wholePart, const CTFraction< NumeratorType, DenominatorType>& fractionalPart );

    Float32 ToFloat32( void ) const;
    CTFraction< NumeratorType, DenominatorType >& FromFloat32( Float32 value, Float32 precision = 1000000 );
    static CTFraction< NumeratorType, DenominatorType > ConvertFloat32ToFraction( Float32 value, Float32 precision = 1000000 );

    Float64 ToFloat64( void ) const;
    CTFraction< NumeratorType, DenominatorType >& FromFloat64( Float64 value, Float64 precision = 1000000 );
    static CTFraction< NumeratorType, DenominatorType > ConvertFloat64ToFraction( Float64 value, Float64 precision = 1000000 );

    CTFraction< NumeratorType, DenominatorType >& operator+=( const CTFraction< NumeratorType, DenominatorType >& other );
    CTFraction< NumeratorType, DenominatorType >& operator-=( const CTFraction< NumeratorType, DenominatorType >& other );
    CTFraction< NumeratorType, DenominatorType >& operator*=( const CTFraction< NumeratorType, DenominatorType >& other );
    CTFraction< NumeratorType, DenominatorType >& operator/=( const CTFraction< NumeratorType, DenominatorType >& other );

    CTFraction< NumeratorType, DenominatorType >& operator+=( NumeratorType value );
    CTFraction< NumeratorType, DenominatorType >& operator-=( NumeratorType value );
    CTFraction< NumeratorType, DenominatorType >& operator*=( NumeratorType value );
    CTFraction< NumeratorType, DenominatorType >& operator/=( NumeratorType value );

    CString ToString( void ) const;
    bool FromString( const CString& str );

    friend CTFraction< NumeratorType, DenominatorType > operator+( CTFraction< NumeratorType, DenominatorType > lhs, const CTFraction< NumeratorType, DenominatorType >& rhs )
    {GUCEF_TRACE;

        lhs += rhs;
        return lhs;
    }

    friend CTFraction< NumeratorType, DenominatorType > operator-( CTFraction< NumeratorType, DenominatorType > lhs, const CTFraction< NumeratorType, DenominatorType >& rhs )
    {GUCEF_TRACE;
        lhs -= rhs;
        return lhs;
    }

    friend CTFraction< NumeratorType, DenominatorType > operator*( CTFraction< NumeratorType, DenominatorType > lhs, const CTFraction< NumeratorType, DenominatorType >& rhs )
    {GUCEF_TRACE;

        lhs *= rhs;
        return lhs;
    }

    friend CTFraction< NumeratorType, DenominatorType > operator/( CTFraction< NumeratorType, DenominatorType > lhs, const CTFraction< NumeratorType, DenominatorType >& rhs )
    {GUCEF_TRACE;

        lhs /= rhs;
        return lhs;
    }

    friend CTFraction< NumeratorType, DenominatorType > operator+( CTFraction< NumeratorType, DenominatorType > lhs, NumeratorType rhs )
    {GUCEF_TRACE;

        lhs += rhs;
        return lhs;
    }

    friend CTFraction< NumeratorType, DenominatorType > operator-( CTFraction< NumeratorType, DenominatorType > lhs, NumeratorType rhs )
    {GUCEF_TRACE;

        lhs -= rhs;
        return lhs;
    }

    friend CTFraction< NumeratorType, DenominatorType > operator*( CTFraction< NumeratorType, DenominatorType > lhs, NumeratorType rhs )
    {GUCEF_TRACE;

        lhs *= rhs;
        return lhs;
    }

    friend CTFraction< NumeratorType, DenominatorType > operator/( CTFraction< NumeratorType, DenominatorType > lhs, NumeratorType rhs )
    {GUCEF_TRACE;

        lhs /= rhs;
        return lhs;
    }

    friend bool operator==( const CTFraction< NumeratorType, DenominatorType >& lhs, const CTFraction< NumeratorType, DenominatorType >& rhs )
    {GUCEF_TRACE;

        return lhs.m_numerator == rhs.m_numerator && lhs.m_denominator == rhs.m_denominator;
    }

    friend bool operator!=( const CTFraction< NumeratorType, DenominatorType >& lhs, const CTFraction< NumeratorType, DenominatorType >& rhs )
    {GUCEF_TRACE;

        return !(lhs == rhs);
    }

    friend bool operator<( const CTFraction< NumeratorType, DenominatorType >& lhs, const CTFraction< NumeratorType, DenominatorType >& rhs )
    {GUCEF_TRACE;

        return lhs.m_numerator * rhs.m_denominator < rhs.m_numerator * lhs.m_denominator;
    }

    friend bool operator<=( const CTFraction< NumeratorType, DenominatorType >& lhs, const CTFraction< NumeratorType, DenominatorType >& rhs )
    {GUCEF_TRACE;

        return lhs.m_numerator * rhs.m_denominator <= rhs.m_numerator * lhs.m_denominator;
    }

    friend bool operator>( const CTFraction< NumeratorType, DenominatorType >& lhs, const CTFraction< NumeratorType, DenominatorType >& rhs )
    {GUCEF_TRACE;

        return lhs.m_numerator * rhs.m_denominator > rhs.m_numerator * lhs.m_denominator;
    }

    friend bool operator>=( const CTFraction< NumeratorType, DenominatorType >& lhs, const CTFraction< NumeratorType, DenominatorType >& rhs )
    {GUCEF_TRACE;

        return lhs.m_numerator * rhs.m_denominator >= rhs.m_numerator * lhs.m_denominator;
    }

    friend std::ostream& operator<<( std::ostream& os, const CTFraction< NumeratorType, DenominatorType >& fraction )
    {GUCEF_TRACE;

        os << fraction.m_numerator << '/' << fraction.m_denominator;
        return os;
    }

    template< typename T >
    inline T gcd( T a, T b )
    {
        T c;
        while (b)
        {
            c = b;
            b = a % b;
            a = c;
        }
        return a;
    }

    private:

    void Normalize( void );

    template < typename T = DenominatorType >
    typename EnableIf< TypeIsUnsigned< T >::value, void >::type
    MakeDenominatorPositive( T denomTypeVar );

    template < typename T = DenominatorType >
    typename EnableIfNot< TypeIsUnsigned< T >::value, void >::type
    MakeDenominatorPositive( T denomTypeVar );

    private:

    NumeratorType m_numerator;
    DenominatorType m_denominator;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >::CTFraction( NumeratorType numerator, DenominatorType denominator )
    : m_numerator( numerator )
    , m_denominator( denominator )
{GUCEF_TRACE;

    if ( m_denominator == 0 )
    {
        GUCEF_EMSGTHROW( GUCEF::invalid_argument, "Denominator cannot be zero" );
    }
    Normalize();
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >::CTFraction( TInt32T2Fraction int32t2_data )
    : m_numerator( static_cast< NumeratorType >( int32t2_data.numerator ) )
    , m_denominator( static_cast< DenominatorType >( int32t2_data.denominator ) )
{GUCEF_TRACE;

    if ( m_denominator == 0 )
    {
        GUCEF_EMSGTHROW( GUCEF::invalid_argument, "Denominator cannot be zero" );
    }
    Normalize();
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >::CTFraction( TUInt32T2Fraction uint32t2_data )
    : m_numerator( static_cast< NumeratorType >( uint32t2_data.numerator ) )
    , m_denominator( static_cast< DenominatorType >( uint32t2_data.denominator ) )
{GUCEF_TRACE;

    if ( m_denominator == 0 )
    {
        GUCEF_EMSGTHROW( GUCEF::invalid_argument, "Denominator cannot be zero" );
    }
    Normalize();
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >::CTFraction( const TInt64T2Fraction& int64_int32_data )
    : m_numerator( static_cast< NumeratorType >( int64_int32_data.numerator ) )
    , m_denominator( static_cast< DenominatorType >( int64_int32_data.denominator ) )
{GUCEF_TRACE;

    if ( m_denominator == 0 )
    {
        GUCEF_EMSGTHROW( GUCEF::invalid_argument, "Denominator cannot be zero" );
    }
    Normalize();
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >::CTFraction( const TUInt64T2Fraction& uint64_uint32_data )
    : m_numerator( static_cast< NumeratorType >( uint64_uint32_data.numerator ) )
    , m_denominator( static_cast< DenominatorType >( uint64_uint32_data.denominator ) )
{GUCEF_TRACE;

    if ( m_denominator == 0 )
    {
        GUCEF_EMSGTHROW( GUCEF::invalid_argument, "Denominator cannot be zero" );
    }
    Normalize();
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >::CTFraction( const TInt64WInt32Fraction& int64_int32_data )
    : m_numerator( static_cast< NumeratorType >( int64_int32_data.numerator ) )
    , m_denominator( static_cast< DenominatorType >( int64_int32_data.denominator ) )
{GUCEF_TRACE;

    if ( m_denominator == 0 )
    {
        GUCEF_EMSGTHROW( GUCEF::invalid_argument, "Denominator cannot be zero" );
    }
    Normalize();
}


/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >::CTFraction( const TUInt64WUInt32Fraction& uint64_uint32_data )
    : m_numerator( static_cast< NumeratorType >( uint64_uint32_data.numerator ) )
    , m_denominator( static_cast< DenominatorType >( uint64_uint32_data.denominator ) )
{GUCEF_TRACE;

    if ( m_denominator == 0 )
    {
        GUCEF_EMSGTHROW( GUCEF::invalid_argument, "Denominator cannot be zero" );
    }
    Normalize();
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
NumeratorType
CTFraction< NumeratorType, DenominatorType >::GetNumerator( void ) const
{GUCEF_TRACE;

    return m_numerator;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
DenominatorType
CTFraction< NumeratorType, DenominatorType >::GetDenominator( void ) const
{GUCEF_TRACE;

    return m_denominator;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >& CTFraction< NumeratorType, DenominatorType >::operator+=( const CTFraction< NumeratorType, DenominatorType >& other )
{GUCEF_TRACE;

    m_numerator = m_numerator * other.m_denominator + other.m_numerator * m_denominator;
    m_denominator *= other.m_denominator;
    Normalize();
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >& CTFraction< NumeratorType, DenominatorType >::operator-=( const CTFraction< NumeratorType, DenominatorType >& other )
{GUCEF_TRACE;

    m_numerator = m_numerator * other.m_denominator - other.m_numerator * m_denominator;
    m_denominator *= other.m_denominator;
    Normalize();
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >& CTFraction< NumeratorType, DenominatorType >::operator*=( const CTFraction< NumeratorType, DenominatorType >& other )
{GUCEF_TRACE;

    m_numerator *= other.m_numerator;
    m_denominator *= other.m_denominator;
    Normalize();
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >& CTFraction< NumeratorType, DenominatorType >::operator/=( const CTFraction< NumeratorType, DenominatorType >& other )
{GUCEF_TRACE;

    if ( other.m_numerator == 0 )
    {
        GUCEF_EMSGTHROW( GUCEF::invalid_argument, "Division by zero" );
    }
    m_numerator *= other.m_denominator;
    m_denominator *= other.m_numerator;
    Normalize();
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >& CTFraction< NumeratorType, DenominatorType >::operator+=( NumeratorType value )
{GUCEF_TRACE;

    m_numerator += value * m_denominator;
    Normalize();
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >& CTFraction< NumeratorType, DenominatorType >::operator-=( NumeratorType value )
{GUCEF_TRACE;

    m_numerator -= value * m_denominator;
    Normalize();
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >& CTFraction< NumeratorType, DenominatorType >::operator*=( NumeratorType value )
{GUCEF_TRACE;

    m_numerator *= value;
    Normalize();
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >& CTFraction< NumeratorType, DenominatorType >::operator/=( NumeratorType value )
{GUCEF_TRACE;

    if ( value == 0 )
    {
        GUCEF_EMSGTHROW( GUCEF::invalid_argument, "Division by zero" );
    }
    m_denominator *= value;
    Normalize();
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
bool
CTFraction< NumeratorType, DenominatorType >::ToMixedFraction( NumeratorType& wholePart, CTFraction< NumeratorType, DenominatorType >& fractionalPart ) const
{GUCEF_TRACE;

    wholePart = m_numerator / m_denominator;
    NumeratorType newNumerator = m_numerator % m_denominator;
    fractionalPart = CTFraction< NumeratorType, DenominatorType >( newNumerator, m_denominator );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
void
CTFraction< NumeratorType, DenominatorType >::FromMixedFraction( NumeratorType wholePart, const CTFraction< NumeratorType, DenominatorType >& fractionalPart )
{GUCEF_TRACE;

    m_numerator = wholePart * fractionalPart.m_denominator + fractionalPart.m_numerator;
    m_denominator = fractionalPart.m_denominator;
    Normalize();
}

///*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
template < typename T >
typename EnableIf< TypeIsUnsigned<T>::value, void >::type
CTFraction< NumeratorType, DenominatorType >::MakeDenominatorPositive( T /*denomTypeVar*/ )
{GUCEF_TRACE;

    // Nothing to do here for unsigned denominators
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
template < typename T >
typename EnableIfNot< TypeIsUnsigned<T>::value, void >::type
CTFraction< NumeratorType, DenominatorType >::MakeDenominatorPositive( T /*denomTypeVar*/ )
{GUCEF_TRACE;

    if ( m_denominator < 0 )
    {
        m_numerator = -m_numerator;
        m_denominator = -m_denominator;
    }
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
void
CTFraction< NumeratorType, DenominatorType >::Normalize( void )
{GUCEF_TRACE;

    MakeDenominatorPositive( m_denominator );
    
    #if __cplusplus >= 201703L // numeric header was added in C++17
    NumeratorType gcdValue = static_cast< NumeratorType >( std::gcd( m_numerator, m_denominator ) );
    #else
      //#if ( GUCEF_COMPILER == GUCEF_COMPILER_GNUC )
      //  #if ( __GNUC__ >= 5 ) // use GCC compiler build-in
      //  NumeratorType gcdValue = static_cast< NumeratorType >( std::__gcd( m_numerator, m_denominator ) );
      //  #else // use our own template's version
      //  NumeratorType gcdValue = gcd< NumeratorType >( m_numerator, static_cast< NumeratorType >( m_denominator ) );
      //  #endif
      //#else // use our own template's version
      // gcd requires non-negative inputs to return a correct positive result.
      // After MakeDenominatorPositive(), m_denominator is always >= 0.
      // The numerator may be negative for signed types, so we pass its absolute value.
      NumeratorType absNumerator = m_numerator;
      if ( absNumerator < NumeratorType( 0 ) )
          absNumerator = NumeratorType( 0 ) - absNumerator;
      NumeratorType gcdValue = gcd< NumeratorType >( absNumerator, static_cast< NumeratorType >( m_denominator ) );
      //#endif
    #endif

    m_numerator /= gcdValue;
    m_denominator /= static_cast< DenominatorType >( gcdValue );
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
Float32
CTFraction< NumeratorType, DenominatorType >::ToFloat32( void ) const
{GUCEF_TRACE;

    return static_cast< Float32 >( m_numerator ) / m_denominator;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >&
CTFraction< NumeratorType, DenominatorType >::FromFloat32( Float32 value, Float32 precision )
{GUCEF_TRACE;

    *this = ConvertFloat32ToFraction( value, precision );
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >
CTFraction< NumeratorType, DenominatorType >::ConvertFloat32ToFraction( Float32 value, Float32 precision )
{GUCEF_TRACE;

    NumeratorType numerator = static_cast< NumeratorType >( value * precision );
    DenominatorType denominator = static_cast< DenominatorType >( precision );
    return CTFraction< NumeratorType, DenominatorType >( numerator, denominator );
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
Float64
CTFraction< NumeratorType, DenominatorType >::ToFloat64( void ) const
{GUCEF_TRACE;

    return static_cast< Float64 >( m_numerator ) / m_denominator;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >&
CTFraction< NumeratorType, DenominatorType >::FromFloat64( Float64 value, Float64 precision )
{GUCEF_TRACE;

    *this = ConvertFloat64ToFraction( value, precision );
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >
CTFraction< NumeratorType, DenominatorType >::ConvertFloat64ToFraction( Float64 value, Float64 precision )
{GUCEF_TRACE;

    NumeratorType numerator = static_cast< NumeratorType >( value * precision );
    DenominatorType denominator = static_cast< DenominatorType >( precision );
    return CTFraction< NumeratorType, DenominatorType >( numerator, denominator );
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CString
CTFraction< NumeratorType, DenominatorType >::ToString( void ) const
{GUCEF_TRACE;

    return CORE::ToString( m_numerator ) + "/" + CORE::ToString( m_denominator );
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
bool
CTFraction< NumeratorType, DenominatorType >::FromString( const CString& str )
{GUCEF_TRACE;

    Int32 slashPos = str.HasChar( '/' );
    if ( slashPos < 0 )
        return false;

    CString numStr = str.SubstrToIndex( slashPos, true );
    CString denStr = str.SubstrToIndex( slashPos+1, false );

    DenominatorType denominator = StringToNumber< DenominatorType >( denStr );
    if ( 0 == denominator )
        return false;

    m_numerator = StringToNumber< NumeratorType >( numStr );
    m_denominator = denominator;
    Normalize();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTFraction< Int8, Int8 >        TInt8Fraction;
typedef CTFraction< UInt8, UInt8 >      TUInt8Fraction;
typedef CTFraction< Int16, Int16 >      TInt16Fraction;
typedef CTFraction< UInt16, UInt16 >    TUInt16Fraction;
typedef CTFraction< Int32, Int32 >      TInt32Fraction;
typedef CTFraction< UInt32, UInt32 >    TUInt32Fraction;
typedef CTFraction< Int64, Int32 >      TInt64Int32Fraction;
typedef CTFraction< UInt64, UInt32 >    TUInt64UInt32Fraction;
typedef CTFraction< Int64, Int64 >      TInt64Fraction;
typedef CTFraction< UInt64, UInt64 >    TUInt64Fraction;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

inline CString ToString( const TInt8Fraction& fraction ) {GUCEF_TRACE; return fraction.ToString(); }
inline CString ToString( const TUInt8Fraction& fraction ) {GUCEF_TRACE; return fraction.ToString(); }
inline CString ToString( const TInt16Fraction& fraction ) {GUCEF_TRACE; return fraction.ToString(); }
inline CString ToString( const TUInt16Fraction& fraction ) {GUCEF_TRACE; return fraction.ToString(); }
inline CString ToString( const TInt32Fraction& fraction ) {GUCEF_TRACE; return fraction.ToString(); }
inline CString ToString( const TUInt32Fraction& fraction ) {GUCEF_TRACE; return fraction.ToString(); }
inline CString ToString( const TInt64Int32Fraction& fraction ) {GUCEF_TRACE; return fraction.ToString(); }
inline CString ToString( const TUInt64UInt32Fraction& fraction ) {GUCEF_TRACE; return fraction.ToString(); }
inline CString ToString( const TInt64Fraction& fraction ) {GUCEF_TRACE; return fraction.ToString(); }
inline CString ToString( const TUInt64Fraction& fraction ) {GUCEF_TRACE; return fraction.ToString(); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTFRACTION_H  ? */
