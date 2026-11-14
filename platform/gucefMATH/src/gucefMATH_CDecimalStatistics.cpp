/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDECIMALACCUMULATOR_H
#include "gucefCORE_CDecimalAccumulator.h"
#define GUCEF_CORE_CDECIMALACCUMULATOR_H
#endif /* GUCEF_CORE_CDECIMALACCUMULATOR_H ? */

#ifndef GUCEF_CORE_CBIGINTEGER_H
#include "gucefCORE_CBigInteger.h"
#define GUCEF_CORE_CBIGINTEGER_H
#endif /* GUCEF_CORE_CBIGINTEGER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMATH_CDecimalStatistics.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Largest count for which n * n fits a signed 64 bit integer */
static const UInt32 MaxCountForSquare = 3037000499U;

/*-------------------------------------------------------------------------*/

static UInt8
MaxScaleOf( const CORE::CDecimal* values, UInt32 count )
{
    UInt8 scale = 0;
    for ( UInt32 i = 0; i < count; ++i )
        if ( values[ i ].GetScale() > scale )
            scale = values[ i ].GetScale();
    return scale;
}

/*-------------------------------------------------------------------------*/

static CORE::CDecimal
ZeroAt( const CORE::CDecimalContext& ctx )
{
    CORE::CDecimal zero;
    CORE::CDecimal::FromUnscaled( 0, ctx.GetResultScale() <= CORE::CDecimal::MaxScale ? ctx.GetResultScale() : 0, zero );
    return zero;
}

/*-------------------------------------------------------------------------*/

/**
 *  num = n * sum( u * v ) - sum( u ) * sum( v ) with u = x - x0 and v = y - y0, all exact. The covariance is
 *  num / n^2 (population) or num / ( n ( n - 1 ) ) (sample).
 */
static CORE::TDecimalStatus
CenteredCrossNumerator( const CORE::CDecimal* xs, const CORE::CDecimal* ys, UInt32 count, CORE::CDecimal& num )
{
    UInt8 scaleX = MaxScaleOf( xs, count );
    UInt8 scaleY = MaxScaleOf( ys, count );
    if ( (UInt32) scaleX + (UInt32) scaleY > (UInt32) CORE::CDecimal::MaxScale )
        return CORE::DECIMALSTATUS_INVALID_SCALE;

    CORE::CDecimalAccumulator products( (UInt8) ( scaleX + scaleY ) );
    CORE::CDecimalAccumulator sumsU( scaleX );
    CORE::CDecimalAccumulator sumsV( scaleY );
    CORE::TDecimalStatus status = CORE::DECIMALSTATUS_OK;
    for ( UInt32 i = 0; i < count; ++i )
    {
        CORE::CDecimal u;
        CORE::CDecimal v;
        status = xs[ i ].Subtract( xs[ 0 ], u );
        if ( !CORE::DecimalStatusIsError( status ) ) status = ys[ i ].Subtract( ys[ 0 ], v );
        if ( !CORE::DecimalStatusIsError( status ) ) status = products.AddProduct( u, v );
        if ( !CORE::DecimalStatusIsError( status ) ) status = sumsU.Add( u );
        if ( !CORE::DecimalStatusIsError( status ) ) status = sumsV.Add( v );
        if ( CORE::DecimalStatusIsError( status ) )
            return status;
    }

    CORE::CDecimal q;
    CORE::CDecimal su;
    CORE::CDecimal sv;
    status = products.GetResult( q );
    if ( !CORE::DecimalStatusIsError( status ) ) status = sumsU.GetResult( su );
    if ( !CORE::DecimalStatusIsError( status ) ) status = sumsV.GetResult( sv );
    if ( CORE::DecimalStatusIsError( status ) )
        return status;

    CORE::CDecimal scaledQ;
    CORE::CDecimal crossSums;
    status = q.Multiply( (Int64) count, scaledQ );
    if ( !CORE::DecimalStatusIsError( status ) ) status = su.Multiply( sv, crossSums );
    if ( !CORE::DecimalStatusIsError( status ) ) status = scaledQ.Subtract( crossSums, num );
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDecimalStatus
CDecimalStatistics::Mean( const CORE::CDecimal* values, UInt32 count, const CORE::CDecimalContext& ctx, CORE::CDecimal& result )
{GUCEF_TRACE;

    if ( 0 == count )
    {
        result = ZeroAt( ctx );
        return CORE::DECIMALSTATUS_OK;
    }

    CORE::CDecimalAccumulator sum( MaxScaleOf( values, count ) );
    for ( UInt32 i = 0; i < count; ++i )
    {
        CORE::TDecimalStatus status = sum.Add( values[ i ] );
        if ( CORE::DecimalStatusIsError( status ) )
            return status;
    }
    CORE::CDecimal total;
    CORE::TDecimalStatus status = sum.GetResult( total );
    if ( CORE::DecimalStatusIsError( status ) )
        return status;
    return total.Divide( (Int64) count, ctx, result );
}

/*-------------------------------------------------------------------------*/

CORE::TDecimalStatus
CDecimalStatistics::StandardDeviation( const CORE::CDecimal* values, UInt32 count, bool sample, const CORE::CDecimalContext& ctx, CORE::CDecimal& result )
{GUCEF_TRACE;

    if ( count > MaxCountForSquare )
        return CORE::DECIMALSTATUS_OVERFLOW;
    if ( 0 == count || ( sample && count < 2 ) )
    {
        result = ZeroAt( ctx );
        return CORE::DECIMALSTATUS_OK;
    }
    if ( ctx.GetResultScale() > CORE::CDecimal::MaxScale )
        return CORE::DECIMALSTATUS_INVALID_SCALE;
    const CORE::TRoundingMode mode = ctx.GetRoundingMode();
    if ( CORE::ROUNDINGMODE_INVALID == mode )
        return CORE::DECIMALSTATUS_INVALID_OPERAND;

    CORE::CDecimal num;
    CORE::TDecimalStatus status = CenteredCrossNumerator( values, values, count, num );
    if ( CORE::DecimalStatusIsError( status ) )
        return status;
    const Int64 divisor = sample ? (Int64) count * (Int64) ( count - 1 ) : (Int64) count * (Int64) count;

    // root( u / ( D 10^s ) ) at t digits: q = round( root( N / M ) ) with N = u 10^( 2t ) and M = D 10^s
    CORE::CBigInteger ten;
    CORE::CBigInteger powerS;
    CORE::CBigInteger powerT2;
    CORE::CBigInteger bigU;
    CORE::CBigInteger bigD;
    CORE::CBigInteger n;
    CORE::CBigInteger m;
    CORE::TBigIntegerStatus big = CORE::CBigInteger::FromInt64( 10, ten );
    big |= CORE::CBigInteger::FromUInt64( (UInt64) num.GetUnscaledValue(), bigU );      // the numerator is not negative
    big |= CORE::CBigInteger::FromInt64( divisor, bigD );
    big |= ten.Pow( (UInt32) num.GetScale(), powerS );
    big |= ten.Pow( 2U * (UInt32) ctx.GetResultScale(), powerT2 );
    big |= bigU.Multiply( powerT2, n );
    big |= bigD.Multiply( powerS, m );
    if ( 0 != big || num.IsNegative() )
        return CORE::DECIMALSTATUS_INVALID_OPERAND;

    CORE::CBigInteger quotient;
    CORE::CBigInteger remainder;
    CORE::CBigInteger root;
    big = n.DivMod( m, quotient, remainder );
    if ( 0 == big ) big = quotient.ISqrt( root );
    if ( 0 != big )
        return CORE::DECIMALSTATUS_INVALID_OPERAND;

    // exact when root^2 * M == N
    CORE::CBigInteger rootSquared;
    CORE::CBigInteger scaledSquared;
    big = root.Multiply( root, rootSquared );
    big |= rootSquared.Multiply( m, scaledSquared );
    bool exact = ( 0 == big ) && 0 == scaledSquared.Compare( n );
    Int32 againstHalf = 0;      // -1 below, 0 tie, 1 above the half way point between root and root + 1
    if ( !exact )
    {
        CORE::CBigInteger twice;
        CORE::CBigInteger twicePlusOne;
        CORE::CBigInteger squaredOdd;
        CORE::CBigInteger left;
        CORE::CBigInteger right;
        CORE::CBigInteger four;
        big = root.Multiply( (Int64) 2, twice );
        big |= twice.Add( (Int64) 1, twicePlusOne );
        big |= twicePlusOne.Multiply( twicePlusOne, squaredOdd );
        big |= squaredOdd.Multiply( m, right );
        big |= n.Multiply( (Int64) 4, left );
        if ( 0 != big )
            return CORE::DECIMALSTATUS_INVALID_OPERAND;
        againstHalf = left.Compare( right );
    }

    bool increment = false;
    if ( !exact )
    {
        switch ( mode )
        {
            case CORE::ROUNDINGMODE_UNNECESSARY:      return CORE::DECIMALSTATUS_ROUNDING_REQUIRED;
            case CORE::ROUNDINGMODE_TOWARD_ZERO:
            case CORE::ROUNDINGMODE_FLOOR:            increment = false; break;
            case CORE::ROUNDINGMODE_AWAY_FROM_ZERO:
            case CORE::ROUNDINGMODE_CEILING:          increment = true; break;
            case CORE::ROUNDINGMODE_HALF_UP:          increment = againstHalf >= 0; break;
            case CORE::ROUNDINGMODE_HALF_DOWN:        increment = againstHalf > 0; break;
            default:                            // half even
            {
                CORE::CBigInteger lowest;
                CORE::CBigInteger twoDivisor;
                Int64 odd = 0;
                big = root.DivMod( (Int64) 2, lowest, odd );
                increment = againstHalf > 0 || ( 0 == againstHalf && 0 != odd );
                break;
            }
        }
        if ( increment )
        {
            CORE::CBigInteger next;
            if ( 0 != root.Add( (Int64) 1, next ) )
                return CORE::DECIMALSTATUS_INVALID_OPERAND;
            root.Swap( next );
        }
    }

    Int64 unscaledRoot = 0;
    big = root.ToInt64( unscaledRoot );
    if ( 0 != big )
        return CORE::DECIMALSTATUS_OVERFLOW;
    CORE::CDecimal value;
    CORE::TDecimalStatus status2 = CORE::CDecimal::FromUnscaled( unscaledRoot, ctx.GetResultScale(), value );
    if ( CORE::DecimalStatusIsError( status2 ) )
        return status2;
    result = value;
    return exact ? CORE::DECIMALSTATUS_OK : CORE::DECIMALSTATUS_INEXACT;
}

/*-------------------------------------------------------------------------*/

CORE::TDecimalStatus
CDecimalStatistics::Covariance( const CORE::CDecimal* xs, const CORE::CDecimal* ys, UInt32 count, bool sample, const CORE::CDecimalContext& ctx, CORE::CDecimal& result )
{GUCEF_TRACE;

    if ( count > MaxCountForSquare )
        return CORE::DECIMALSTATUS_OVERFLOW;
    if ( 0 == count || ( sample && count < 2 ) )
    {
        result = ZeroAt( ctx );
        return CORE::DECIMALSTATUS_OK;
    }

    CORE::CDecimal num;
    CORE::TDecimalStatus status = CenteredCrossNumerator( xs, ys, count, num );
    if ( CORE::DecimalStatusIsError( status ) )
        return status;

    Int64 divisor = sample ? (Int64) count * (Int64) ( count - 1 ) : (Int64) count * (Int64) count;
    return num.Divide( divisor, ctx, result );
}

/*-------------------------------------------------------------------------*/

CORE::TDecimalStatus
CDecimalStatistics::Variance( const CORE::CDecimal* values, UInt32 count, bool sample, const CORE::CDecimalContext& ctx, CORE::CDecimal& result )
{GUCEF_TRACE;

    return Covariance( values, values, count, sample, ctx, result );
}

/*-------------------------------------------------------------------------*/

CORE::TDecimalStatus
CDecimalStatistics::LinearRegression( const CORE::CDecimal* xs, const CORE::CDecimal* ys, UInt32 count, const CORE::CDecimalContext& ctx,
                                      CORE::CDecimal& slope, CORE::CDecimal& intercept, CORE::CDecimal& rSquared )
{GUCEF_TRACE;

    if ( count < 2 )
        return CORE::DECIMALSTATUS_DIVISION_BY_ZERO;

    CORE::CDecimal nxy;
    CORE::CDecimal nxx;
    CORE::CDecimal nyy;
    CORE::TDecimalStatus status = CenteredCrossNumerator( xs, ys, count, nxy );
    if ( !CORE::DecimalStatusIsError( status ) ) status = CenteredCrossNumerator( xs, xs, count, nxx );
    if ( !CORE::DecimalStatusIsError( status ) ) status = CenteredCrossNumerator( ys, ys, count, nyy );
    if ( CORE::DecimalStatusIsError( status ) )
        return status;
    if ( nxx.IsZero() )
        return CORE::DECIMALSTATUS_DIVISION_BY_ZERO;

    CORE::CDecimal newSlope;
    status = nxy.Divide( nxx, ctx, newSlope );
    if ( CORE::DecimalStatusIsError( status ) )
        return status;

    // intercept = mean( y ) - sum( x ) * nxy / ( n * nxx )
    CORE::CDecimal sumX;
    CORE::CDecimal meanY;
    CORE::CDecimal nTimesNxx;
    CORE::CDecimal slopeTimesSumX;
    CORE::CDecimal newIntercept;
    status = CORE::CDecimal::Sum( xs, count, sumX );
    if ( !CORE::DecimalStatusIsError( status ) ) status = Mean( ys, count, ctx, meanY );
    if ( !CORE::DecimalStatusIsError( status ) ) status = nxx.Multiply( (Int64) count, nTimesNxx );
    if ( !CORE::DecimalStatusIsError( status ) ) status = sumX.MulDiv( nxy, nTimesNxx, ctx, slopeTimesSumX );
    if ( !CORE::DecimalStatusIsError( status ) ) status = meanY.Subtract( slopeTimesSumX, ctx, newIntercept );
    if ( CORE::DecimalStatusIsError( status ) )
        return status;

    // r squared = nxy^2 / ( nxx * nyy ), in two steps so that no product of scales exceeds the maximum scale
    CORE::CDecimal newRSquared;
    if ( nyy.IsZero() )
    {
        status = CORE::CDecimal::FromInt64( 1, ctx.GetResultScale(), newRSquared );
    }
    else
    {
        UInt8 higherScale = ctx.GetResultScale() + 6 <= CORE::CDecimal::MaxScale ? (UInt8) ( ctx.GetResultScale() + 6 ) : CORE::CDecimal::MaxScale;
        CORE::CDecimalContext higherContext( higherScale, ctx.GetRoundingMode() );
        CORE::CDecimal squaredOverNxx;
        status = nxy.MulDiv( nxy, nxx, higherContext, squaredOverNxx );
        if ( !CORE::DecimalStatusIsError( status ) ) status = squaredOverNxx.Divide( nyy, ctx, newRSquared );
    }
    if ( CORE::DecimalStatusIsError( status ) )
        return status;

    slope = newSlope;
    intercept = newIntercept;
    rSquared = newRSquared;
    return CORE::DECIMALSTATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
