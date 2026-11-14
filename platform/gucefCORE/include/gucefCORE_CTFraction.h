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

#ifndef GUCEF_CORE_EXCEPTIONTHROWMACROS_H
#include "ExceptionThrowMacros.h"
#define GUCEF_CORE_EXCEPTIONTHROWMACROS_H
#endif /* GUCEF_CORE_EXCEPTIONTHROWMACROS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_CORE_CTFRACTIONINTEGEROPS_H
#include "gucefCORE_CTFractionIntegerOps.h"
#define GUCEF_CORE_CTFRACTIONINTEGEROPS_H
#endif /* GUCEF_CORE_CTFRACTIONINTEGEROPS_H ? */

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
    typedef CTFractionIntegerOps< NumeratorType, DenominatorType > TIntegerOps;


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

    /**
     *  Non throwing arithmetic: returns false when the exact result does not fit the numerator and denominator
     *  types (only possible for the Int64 specialization of the integer operations), the fraction is then unchanged.
     *  A zero divisor in TryDivide returns false as well.
     */
    bool TryAdd( const CTFraction< NumeratorType, DenominatorType >& other, CTFraction< NumeratorType, DenominatorType >& result ) const;
    bool TrySubtract( const CTFraction< NumeratorType, DenominatorType >& other, CTFraction< NumeratorType, DenominatorType >& result ) const;
    bool TryMultiply( const CTFraction< NumeratorType, DenominatorType >& other, CTFraction< NumeratorType, DenominatorType >& result ) const;
    bool TryDivide( const CTFraction< NumeratorType, DenominatorType >& other, CTFraction< NumeratorType, DenominatorType >& result ) const;

    /** One rounding: numerator / denominator rounded as the context states. Returns the status of the division */
    TDecimalStatus ToDecimal( const CDecimalContext& context, CDecimal& result ) const;

    /** Exact: unscaled / 10^scale in lowest terms. false when the value does not fit the numerator and denominator types */
    bool FromDecimal( const CDecimal& value );

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

        return TIntegerOps::Compare( lhs.m_numerator, lhs.m_denominator, rhs.m_numerator, rhs.m_denominator ) < 0;
    }

    friend bool operator<=( const CTFraction< NumeratorType, DenominatorType >& lhs, const CTFraction< NumeratorType, DenominatorType >& rhs )
    {GUCEF_TRACE;

        return TIntegerOps::Compare( lhs.m_numerator, lhs.m_denominator, rhs.m_numerator, rhs.m_denominator ) <= 0;
    }

    friend bool operator>( const CTFraction< NumeratorType, DenominatorType >& lhs, const CTFraction< NumeratorType, DenominatorType >& rhs )
    {GUCEF_TRACE;

        return TIntegerOps::Compare( lhs.m_numerator, lhs.m_denominator, rhs.m_numerator, rhs.m_denominator ) > 0;
    }

    friend bool operator>=( const CTFraction< NumeratorType, DenominatorType >& lhs, const CTFraction< NumeratorType, DenominatorType >& rhs )
    {GUCEF_TRACE;

        return TIntegerOps::Compare( lhs.m_numerator, lhs.m_denominator, rhs.m_numerator, rhs.m_denominator ) >= 0;
    }

    friend std::ostream& operator<<( std::ostream& os, const CTFraction< NumeratorType, DenominatorType >& fraction )
    {GUCEF_TRACE;

        os << fraction.m_numerator << '/' << fraction.m_denominator;
        return os;
    }

    /** Euclid, kept for source compatibility. Normalization itself goes through the integer operations policy */
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

    /** Throws std::overflow_error when the exact result does not fit */
    static void ThrowOnOverflow( bool fits );

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

    ThrowOnOverflow( TIntegerOps::Add( m_numerator, m_denominator, other.m_numerator, other.m_denominator ) );
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >& CTFraction< NumeratorType, DenominatorType >::operator-=( const CTFraction< NumeratorType, DenominatorType >& other )
{GUCEF_TRACE;

    ThrowOnOverflow( TIntegerOps::Subtract( m_numerator, m_denominator, other.m_numerator, other.m_denominator ) );
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >& CTFraction< NumeratorType, DenominatorType >::operator*=( const CTFraction< NumeratorType, DenominatorType >& other )
{GUCEF_TRACE;

    ThrowOnOverflow( TIntegerOps::Multiply( m_numerator, m_denominator, other.m_numerator, other.m_denominator ) );
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
    ThrowOnOverflow( TIntegerOps::Divide( m_numerator, m_denominator, other.m_numerator, other.m_denominator ) );
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >& CTFraction< NumeratorType, DenominatorType >::operator+=( NumeratorType value )
{GUCEF_TRACE;

    ThrowOnOverflow( TIntegerOps::AddScalar( m_numerator, m_denominator, value ) );
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >& CTFraction< NumeratorType, DenominatorType >::operator-=( NumeratorType value )
{GUCEF_TRACE;

    ThrowOnOverflow( TIntegerOps::SubtractScalar( m_numerator, m_denominator, value ) );
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
CTFraction< NumeratorType, DenominatorType >& CTFraction< NumeratorType, DenominatorType >::operator*=( NumeratorType value )
{GUCEF_TRACE;

    ThrowOnOverflow( TIntegerOps::MultiplyScalar( m_numerator, m_denominator, value ) );
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
    ThrowOnOverflow( TIntegerOps::DivideScalar( m_numerator, m_denominator, value ) );
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

    NumeratorType numerator = m_numerator;
    DenominatorType denominator = m_denominator;
    ThrowOnOverflow( TIntegerOps::FromMixed( numerator, denominator, wholePart, fractionalPart.m_numerator, fractionalPart.m_denominator ) );
    m_numerator = numerator;
    m_denominator = denominator;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
void
CTFraction< NumeratorType, DenominatorType >::ThrowOnOverflow( bool fits )
{GUCEF_TRACE;

    if ( !fits )
    {
        throw std::overflow_error( "CTFraction: the exact result does not fit the numerator and denominator types" );
    }
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
void
CTFraction< NumeratorType, DenominatorType >::Normalize( void )
{GUCEF_TRACE;

    ThrowOnOverflow( TIntegerOps::Normalize( m_numerator, m_denominator ) );
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
bool
CTFraction< NumeratorType, DenominatorType >::TryAdd( const CTFraction< NumeratorType, DenominatorType >& other, CTFraction< NumeratorType, DenominatorType >& result ) const
{GUCEF_TRACE;

    NumeratorType n = m_numerator;
    DenominatorType d = m_denominator;
    if ( !TIntegerOps::Add( n, d, other.m_numerator, other.m_denominator ) )
        return false;
    result.m_numerator = n;
    result.m_denominator = d;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
bool
CTFraction< NumeratorType, DenominatorType >::TrySubtract( const CTFraction< NumeratorType, DenominatorType >& other, CTFraction< NumeratorType, DenominatorType >& result ) const
{GUCEF_TRACE;

    NumeratorType n = m_numerator;
    DenominatorType d = m_denominator;
    if ( !TIntegerOps::Subtract( n, d, other.m_numerator, other.m_denominator ) )
        return false;
    result.m_numerator = n;
    result.m_denominator = d;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
bool
CTFraction< NumeratorType, DenominatorType >::TryMultiply( const CTFraction< NumeratorType, DenominatorType >& other, CTFraction< NumeratorType, DenominatorType >& result ) const
{GUCEF_TRACE;

    NumeratorType n = m_numerator;
    DenominatorType d = m_denominator;
    if ( !TIntegerOps::Multiply( n, d, other.m_numerator, other.m_denominator ) )
        return false;
    result.m_numerator = n;
    result.m_denominator = d;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
bool
CTFraction< NumeratorType, DenominatorType >::TryDivide( const CTFraction< NumeratorType, DenominatorType >& other, CTFraction< NumeratorType, DenominatorType >& result ) const
{GUCEF_TRACE;

    if ( other.m_numerator == 0 )
        return false;
    NumeratorType n = m_numerator;
    DenominatorType d = m_denominator;
    if ( !TIntegerOps::Divide( n, d, other.m_numerator, other.m_denominator ) )
        return false;
    result.m_numerator = n;
    result.m_denominator = d;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
TDecimalStatus
CTFraction< NumeratorType, DenominatorType >::ToDecimal( const CDecimalContext& context, CDecimal& result ) const
{GUCEF_TRACE;

    return TIntegerOps::ToDecimal( m_numerator, m_denominator, context, result );
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
bool
CTFraction< NumeratorType, DenominatorType >::FromDecimal( const CDecimal& value )
{GUCEF_TRACE;

    NumeratorType n = m_numerator;
    DenominatorType d = m_denominator;
    if ( !TIntegerOps::FromDecimal( value, n, d ) )
        return false;
    if ( !TIntegerOps::Normalize( n, d ) )
        return false;
    m_numerator = n;
    m_denominator = d;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename NumeratorType, typename DenominatorType >
Float32
CTFraction< NumeratorType, DenominatorType >::ToFloat32( void ) const
{GUCEF_TRACE;

    return TIntegerOps::ToFloat32( m_numerator, m_denominator );
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

    return TIntegerOps::ToFloat64( m_numerator, m_denominator );
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

    return TIntegerOps::ToString( m_numerator, m_denominator );
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

    NumeratorType numerator = m_numerator;
    DenominatorType denominator = m_denominator;
    if ( !TIntegerOps::FromString( numStr, denStr, numerator, denominator ) )
        return false;

    m_numerator = numerator;
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
