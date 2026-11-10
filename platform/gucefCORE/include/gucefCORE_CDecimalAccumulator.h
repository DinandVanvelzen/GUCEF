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

#ifndef GUCEF_CORE_CDECIMALACCUMULATOR_H
#define GUCEF_CORE_CDECIMALACCUMULATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

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
 *  Exact sum of many decimals: a two's complement 128 bit total at one scale, narrowed once by GetResult().
 *  Sums whose intermediate exceeds 64 bit but whose final value fits are exact.
 *  The internal scale goes up to 2 * MaxScale so a single product (quantity * price) is always held exactly.
 *
 *  Capacity: the total is one signed 128 bit integer at the internal scale, which is the largest scale of any value
 *  or product added so far. The magnitude of the total is therefore limited to 2^127 / 10^scale: about 1.7e20 at
 *  scale 18, 1.7e8 at scale 30 and 170 at scale 36. One term with a high scale limits the whole total, also the
 *  terms added before it (they are multiplied up to the new scale, which is checked). A sum of products with a
 *  combined scale of 12 or less, the quantity * price case, has room for 1.7e26.
 *
 *  Status precedence of GetResult(): DECIMALSTATUS_INVALID_SCALE (accumulator scale above MaxScale), then the sticky
 *  DECIMALSTATUS_OVERFLOW of the 128 bit total, then the narrowing to the result: DECIMALSTATUS_OVERFLOW when the
 *  result does not fit 64 bits at the result scale, DECIMALSTATUS_ROUNDING_REQUIRED or DECIMALSTATUS_INVALID_OPERAND.
 *  A total that overflowed never reports ROUNDING_REQUIRED. The operations report their own overflow and
 *  INVALID_SCALE directly, the sticky overflow is what GetResult() reports afterwards.
 *  Value type: no heap, no virtuals, not thread-safe (use one per thread and add the results).
 */
class GUCEF_CORE_PUBLIC_CPP CDecimalAccumulator
{
    public:

    static const UInt8 MaxScale = 2 * GUCEF_DECIMAL_MAX_SCALE;

    /** A scale above MaxScale makes every operation report DECIMALSTATUS_INVALID_SCALE */
    explicit CDecimalAccumulator( UInt8 scale );

    void Reset( UInt8 scale );

    /** Adds a value already at the accumulator scale. Inline two's complement 128 bit add, no check */
    void AddUnscaled( Int64 unscaledAtAccumulatorScale );

    /**
     *  Adds a column span of values already at the accumulator scale.
     *  The span is summed locally with carry propagation only and then added with one checked 128 bit add.
     */
    void AddUnscaled( const Int64* unscaledValues, UInt32 count );

    /** Adds an exact value, raises the accumulator scale when the value has a larger scale (checked) */
    TDecimalStatus Add( const CDecimal& value );

    TDecimalStatus Subtract( const CDecimal& value );

    /** Adds the exact product a * b at scale sa + sb: sum of quantity * price */
    TDecimalStatus AddProduct( const CDecimal& a, const CDecimal& b );

    UInt8 GetScale( void ) const { return m_scale; }

    /** Exact narrowing: the total at the accumulator scale, at most CDecimal::MaxScale (ROUNDING_REQUIRED if that loses digits) */
    TDecimalStatus GetResult( CDecimal& result ) const;

    /** The total rounded once to the context */
    TDecimalStatus GetResult( const CDecimalContext& ctx, CDecimal& result ) const;

    private:

    TDecimalStatus RaiseScale( UInt8 newScale );
    TDecimalStatus AddMagnitude( bool negative, UInt64 magnitudeHigh, UInt64 magnitudeLow, UInt8 valueScale );
    bool AddTwosComplementChecked( UInt64 high, UInt64 low );

    UInt64 m_high;      /**< two's complement 128 bit sum */
    UInt64 m_low;
    UInt8 m_scale;
    bool m_overflowed;  /**< sticky, reported by GetResult */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      INLINE IMPLEMENTATION                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

inline void
CDecimalAccumulator::AddUnscaled( Int64 unscaledAtAccumulatorScale )
{
    UInt64 value = (UInt64) unscaledAtAccumulatorScale;
    UInt64 low = m_low + value;
    m_high += ( low < m_low ? 1 : 0 ) + ( unscaledAtAccumulatorScale < 0 ? ~( (UInt64) 0 ) : 0 );
    m_low = low;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDECIMALACCUMULATOR_H ? */
