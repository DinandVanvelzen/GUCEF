/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CUINT128_H
#include "gucefCORE_CUInt128.h"
#define GUCEF_CORE_CUINT128_H
#endif /* GUCEF_CORE_CUINT128_H ? */

#include "gucefCORE_CDecimalAccumulator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt64 g_accumulatorSignBit = 0x8000000000000000ULL;

/*-------------------------------------------------------------------------*/

/** Sign and magnitude of a two's complement 128 bit value, the magnitude of -2^127 is 2^127 */
static void
ToSignMagnitude( UInt64 high, UInt64 low, bool& negative, CUInt128& magnitude )
{
    negative = 0 != ( high & g_accumulatorSignBit );
    magnitude = CUInt128( high, low );
    if ( negative )
        magnitude = CUInt128::Negate( magnitude );
}

/*-------------------------------------------------------------------------*/

/** Two's complement form of a sign and magnitude, false when it is outside -2^127 .. 2^127 - 1 */
static bool
FromSignMagnitude( bool negative, const CUInt128& magnitude, UInt64& high, UInt64& low )
{
    if ( negative )
    {
        if ( magnitude.high > g_accumulatorSignBit || ( magnitude.high == g_accumulatorSignBit && 0 != magnitude.low ) )
            return false;
        CUInt128 value = CUInt128::Negate( magnitude );
        high = value.high;
        low = value.low;
        return true;
    }
    if ( 0 != ( magnitude.high & g_accumulatorSignBit ) )
        return false;
    high = magnitude.high;
    low = magnitude.low;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDecimalAccumulator::CDecimalAccumulator( UInt8 scale )
    : m_high( 0 )
    , m_low( 0 )
    , m_scale( scale )
    , m_overflowed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDecimalAccumulator::Reset( UInt8 scale )
{GUCEF_TRACE;

    m_high = 0;
    m_low = 0;
    m_scale = scale;
    m_overflowed = false;
}

/*-------------------------------------------------------------------------*/

bool
CDecimalAccumulator::AddTwosComplementChecked( UInt64 high, UInt64 low )
{GUCEF_TRACE;

    UInt64 sumLow = m_low + low;
    UInt64 carry = sumLow < m_low ? 1 : 0;
    UInt64 sumHigh = m_high + high + carry;

    // signed overflow: both operands have the same sign and the sum has the other one
    bool signA = 0 != ( m_high & g_accumulatorSignBit );
    bool signB = 0 != ( high & g_accumulatorSignBit );
    bool signSum = 0 != ( sumHigh & g_accumulatorSignBit );
    if ( signA == signB && signSum != signA )
    {
        m_overflowed = true;
        return false;
    }
    m_high = sumHigh;
    m_low = sumLow;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDecimalAccumulator::AddUnscaled( const Int64* unscaledValues, UInt32 count )
{GUCEF_TRACE;

    if ( GUCEF_NULL == unscaledValues || 0 == count )
        return;

    // fewer than 2^32 values of magnitude below 2^63: the span sum stays below 2^95, carries only
    UInt64 high = 0;
    UInt64 low = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        Int64 value = unscaledValues[ i ];
        UInt64 newLow = low + (UInt64) value;
        high += ( newLow < low ? 1 : 0 ) + ( value < 0 ? ~( (UInt64) 0 ) : 0 );
        low = newLow;
    }
    AddTwosComplementChecked( high, low );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimalAccumulator::RaiseScale( UInt8 newScale )
{GUCEF_TRACE;

    if ( newScale <= m_scale )
        return DECIMALSTATUS_OK;

    bool negative = false;
    CUInt128 magnitude;
    ToSignMagnitude( m_high, m_low, negative, magnitude );
    UInt32 diff = (UInt32) newScale - (UInt32) m_scale;
    m_scale = newScale;
    if ( !CUInt128::MultiplyByPow10Checked( magnitude, diff ) || !FromSignMagnitude( negative, magnitude, m_high, m_low ) )
    {
        m_overflowed = true;
        return DECIMALSTATUS_OVERFLOW;
    }
    return DECIMALSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimalAccumulator::AddMagnitude( bool negative, UInt64 magnitudeHigh, UInt64 magnitudeLow, UInt8 valueScale )
{GUCEF_TRACE;

    CUInt128 magnitude( magnitudeHigh, magnitudeLow );
    UInt64 high = 0;
    UInt64 low = 0;
    if ( !CUInt128::MultiplyByPow10Checked( magnitude, (UInt32) m_scale - (UInt32) valueScale ) ||
         !FromSignMagnitude( negative, magnitude, high, low ) )
    {
        m_overflowed = true;
        return DECIMALSTATUS_OVERFLOW;
    }
    return AddTwosComplementChecked( high, low ) ? DECIMALSTATUS_OK : DECIMALSTATUS_OVERFLOW;
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimalAccumulator::Add( const CDecimal& value )
{GUCEF_TRACE;

    if ( m_scale > MaxScale )
        return DECIMALSTATUS_INVALID_SCALE;

    TDecimalStatus status = RaiseScale( value.m_scale );
    if ( DecimalStatusIsError( status ) )
        return status;

    Int64 unscaled = value.m_unscaled;
    UInt64 magnitude = unscaled < 0 ? 0 - (UInt64) unscaled : (UInt64) unscaled;
    return AddMagnitude( unscaled < 0, 0, magnitude, value.m_scale );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimalAccumulator::Subtract( const CDecimal& value )
{GUCEF_TRACE;

    return Add( value.Negate() );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimalAccumulator::AddProduct( const CDecimal& a, const CDecimal& b )
{GUCEF_TRACE;

    if ( m_scale > MaxScale )
        return DECIMALSTATUS_INVALID_SCALE;

    UInt8 productScale = (UInt8) ( a.m_scale + b.m_scale );
    TDecimalStatus status = RaiseScale( productScale );
    if ( DecimalStatusIsError( status ) )
        return status;

    UInt64 magnitudeA = a.m_unscaled < 0 ? 0 - (UInt64) a.m_unscaled : (UInt64) a.m_unscaled;
    UInt64 magnitudeB = b.m_unscaled < 0 ? 0 - (UInt64) b.m_unscaled : (UInt64) b.m_unscaled;
    CUInt128 product = CUInt128::Multiply( magnitudeA, magnitudeB );     // < 2^126
    bool negative = ( a.m_unscaled < 0 ) != ( b.m_unscaled < 0 ) && !product.IsZero();
    return AddMagnitude( negative, product.high, product.low, productScale );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimalAccumulator::GetResult( CDecimal& result ) const
{GUCEF_TRACE;

    if ( m_scale > MaxScale )
        return DECIMALSTATUS_INVALID_SCALE;
    if ( m_overflowed )
        return DECIMALSTATUS_OVERFLOW;

    bool negative = false;
    CUInt128 magnitude;
    ToSignMagnitude( m_high, m_low, negative, magnitude );
    UInt8 resultScale = m_scale > CDecimal::MaxScale ? (UInt8) CDecimal::MaxScale : m_scale;
    return CDecimal::RoundScaled( negative, magnitude.high, magnitude.low, (Int32) resultScale - (Int32) m_scale, ROUNDINGMODE_UNNECESSARY, resultScale, result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimalAccumulator::GetResult( const CDecimalContext& ctx, CDecimal& result ) const
{GUCEF_TRACE;

    if ( m_scale > MaxScale )
        return DECIMALSTATUS_INVALID_SCALE;
    if ( m_overflowed )
        return DECIMALSTATUS_OVERFLOW;

    bool negative = false;
    CUInt128 magnitude;
    ToSignMagnitude( m_high, m_low, negative, magnitude );
    return CDecimal::RoundScaled( negative, magnitude.high, magnitude.low, (Int32) ctx.GetResultScale() - (Int32) m_scale, ctx.GetRoundingMode(), ctx.GetResultScale(), result );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
