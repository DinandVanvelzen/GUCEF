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

#include <string.h>
#include <limits>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CDATATABLEPRUNER_H
#include "gucefCORE_CDataTablePruner.h"
#define GUCEF_CORE_CDATATABLEPRUNER_H
#endif /* GUCEF_CORE_CDATATABLEPRUNER_H ? */

#include "gucefCORE_CDataTableQuery.h"

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

namespace {

/** How a literal is brought to the column: exactly, or rounded up (lower bounds) or down (upper bounds) */
enum ERounding
{
    ROUND_EXACT = 0 ,
    ROUND_UP        ,
    ROUND_DOWN
};

/** Where the literal lies relative to the values the column can hold */
enum ELiteralFit
{
    FIT_EXACT = 0   ,   /**< the column holds the literal itself */
    FIT_ROUNDED     ,   /**< the nearest value in the rounding direction was taken */
    FIT_BELOW       ,   /**< below every value of the column */
    FIT_ABOVE       ,   /**< above every value of the column */
    FIT_NAN             /**< a NaN literal: no comparison holds */
};

inline bool
IsNaN( Float64 value )
{
    return value != value;
}

bool
IsDateTypeId( UInt8 typeId )
{
    return GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId || GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == typeId;
}

bool
IsSecondsTypeId( UInt8 typeId )
{
    return GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH == typeId || GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH == typeId;
}

bool
IsTimestampTypeId( UInt8 typeId )
{
    TDataTableTimeUnit unit = DATATABLE_TIMEUNIT_MICROSECONDS;
    return IsSecondsTypeId( typeId ) || CDataTableValues::TimeUnitFromTypeId( typeId, unit );
}

/** The next float32 towards +infinity (up) or -infinity, for finite values */
Float32
NextFloat32( Float32 value, bool up )
{
    if ( value != value )
        return value;
    if ( 0.0f == value )
    {
        UInt32 smallest = 1;
        Float32 result = 0.0f;
        memcpy( &result, &smallest, sizeof( result ) );
        return up ? result : -result;
    }
    UInt32 bits = 0;
    memcpy( &bits, &value, sizeof( bits ) );
    const bool positive = value > 0.0f;
    if ( positive == up )
        ++bits;
    else
        --bits;
    Float32 result = 0.0f;
    memcpy( &result, &bits, sizeof( result ) );
    return result;
}

/** The next float64 towards +infinity (up) or -infinity, for finite values */
Float64
NextFloat64( Float64 value, bool up )
{
    if ( value != value )
        return value;
    if ( 0.0 == value )
    {
        UInt64 smallest = 1;
        Float64 result = 0.0;
        memcpy( &result, &smallest, sizeof( result ) );
        return up ? result : -result;
    }
    UInt64 bits = 0;
    memcpy( &bits, &value, sizeof( bits ) );
    const bool positive = value > 0.0;
    if ( positive == up )
        ++bits;
    else
        --bits;
    Float64 result = 0.0;
    memcpy( &result, &bits, sizeof( result ) );
    return result;
}

/** A float64 brought to a float32 column: the value itself, or the nearest float32 in the rounding direction */
ELiteralFit
FitFloat32( Float64 value, ERounding rounding, ELiteralFit fitSoFar, Float64& result )
{
    const Float64 maxFloat32 = 3.4028234663852886e+38;
    if ( value > maxFloat32 || value < -maxFloat32 )
    {
        // beyond the finite float32 range; the infinities themselves are held exactly
        if ( value == value * 2.0 )
        {
            result = value;
            return fitSoFar;
        }
        if ( ROUND_EXACT == rounding )
            return FIT_ROUNDED;
        const Float64 infinity = std::numeric_limits< Float64 >::infinity();
        if ( value > 0.0 )
        {
            result = ( ROUND_DOWN == rounding ) ? maxFloat32 : infinity;
            return FIT_ROUNDED;
        }
        result = ( ROUND_UP == rounding ) ? -maxFloat32 : -infinity;
        return FIT_ROUNDED;
    }

    Float32 narrow = (Float32) value;
    if ( (Float64) narrow == value )
    {
        result = narrow;
        return fitSoFar;
    }
    if ( ROUND_EXACT == rounding )
        return FIT_ROUNDED;
    if ( ROUND_UP == rounding && (Float64) narrow < value )
        narrow = NextFloat32( narrow, true );
    else
    if ( ROUND_DOWN == rounding && (Float64) narrow > value )
        narrow = NextFloat32( narrow, false );
    result = narrow;
    return FIT_ROUNDED;
}

/** The smallest and largest value of an integer column, as signed and unsigned bounds */
void
IntegerColumnRange( TDataTableLogicalType type, Int64& signedMin, UInt64& unsignedMax )
{
    switch ( type )
    {
        case DATATABLE_TYPE_INT8:   { signedMin = -128; unsignedMax = 127; return; }
        case DATATABLE_TYPE_INT16:  { signedMin = -32768; unsignedMax = 32767; return; }
        case DATATABLE_TYPE_INT32:  { signedMin = GUCEF_INT32MIN; unsignedMax = GUCEF_INT32MAX; return; }
        case DATATABLE_TYPE_INT64:  { signedMin = GUCEF_INT64MIN; unsignedMax = (UInt64) GUCEF_INT64MAX; return; }
        case DATATABLE_TYPE_UINT8:  { signedMin = 0; unsignedMax = 255; return; }
        case DATATABLE_TYPE_UINT16: { signedMin = 0; unsignedMax = 65535; return; }
        case DATATABLE_TYPE_UINT32: { signedMin = 0; unsignedMax = 4294967295UL; return; }
        default:                    { signedMin = 0; unsignedMax = GUCEF_UINT64MAX; return; }
    }
}

/** An integer given as sign and magnitude checked against an integer column */
ELiteralFit
FitInteger( const CDataTableColumnDef& column, bool negative, UInt64 magnitude, ELiteralFit fitSoFar, CVariant& out )
{
    Int64 signedMin = 0;
    UInt64 unsignedMax = 0;
    IntegerColumnRange( column.type, signedMin, unsignedMax );
    if ( negative && 0 != magnitude )
    {
        // the magnitude of the smallest value: -( signedMin + 1 ) + 1 avoids negating INT64_MIN
        const UInt64 minMagnitude = (UInt64) ( -( signedMin + 1 ) ) + ( signedMin < 0 ? 1 : 0 );
        if ( signedMin >= 0 || magnitude > minMagnitude )
            return FIT_BELOW;
        const Int64 value = magnitude == (UInt64) GUCEF_INT64MAX + 1 ? GUCEF_INT64MIN : -(Int64) magnitude;
        out = value;
        return fitSoFar;
    }
    if ( magnitude > unsignedMax )
        return FIT_ABOVE;
    if ( DataTableLogicalTypeIsUnsigned( column.type ) )
        out = (UInt64) magnitude;
    else
        out = (Int64) magnitude;
    return fitSoFar;
}

/** A float64 as sign and magnitude of an integer in the rounding direction; FIT_BELOW / FIT_ABOVE beyond 64 bits */
ELiteralFit
FloatToInteger( Float64 value, ERounding rounding, bool& negative, UInt64& magnitude )
{
    if ( IsNaN( value ) )
        return FIT_NAN;
    if ( value >= 18446744073709551616.0 )
        return FIT_ABOVE;
    if ( value < -9223372036854775808.0 )
        return FIT_BELOW;

    ELiteralFit fit = FIT_EXACT;
    Float64 integral = value;
    // truncate towards zero: exact for every float64 within the range checked above
    if ( value >= 0.0 )
    {
        UInt64 truncated = (UInt64) value;
        integral = (Float64) truncated;
        if ( integral != value )
        {
            if ( ROUND_EXACT == rounding )
                return FIT_ROUNDED;
            fit = FIT_ROUNDED;
            if ( ROUND_UP == rounding )
            {
                if ( GUCEF_UINT64MAX == truncated )
                    return FIT_ABOVE;
                ++truncated;
            }
        }
        negative = false;
        magnitude = truncated;
        return fit;
    }

    Int64 truncated = (Int64) value;
    integral = (Float64) truncated;
    if ( integral != value )
    {
        if ( ROUND_EXACT == rounding )
            return FIT_ROUNDED;
        fit = FIT_ROUNDED;
        if ( ROUND_DOWN == rounding )
        {
            if ( GUCEF_INT64MIN == truncated )
                return FIT_BELOW;
            --truncated;
        }
    }
    negative = truncated < 0;
    magnitude = negative ? (UInt64) ( -( truncated + 1 ) ) + 1 : (UInt64) truncated;
    return fit;
}

/**
 *  Brings a literal to the native form of the column (section 5.2). TYPE_MISMATCH for a literal of an unrelated type,
 *  OK otherwise with the fit telling whether the native value in out is the literal itself or rounded, or that the
 *  literal lies outside the column's values.
 */
TDataTableStatus
ResolveLiteral( const CDataTableColumnDef& column, const CVariant& literal, ERounding rounding, CVariant& out, ELiteralFit& fit )
{
    const UInt8 typeId = literal.GetTypeId();
    const bool isDate = IsDateTypeId( typeId );
    const bool isTimestamp = IsTimestampTypeId( typeId );
    const bool isPlainInteger = literal.IsInteger() && !isDate && !isTimestamp && !literal.IsBoolean();
    fit = FIT_EXACT;

    switch ( column.type )
    {
        case DATATABLE_TYPE_BOOL:
        {
            if ( literal.IsBoolean() )
            {
                out = literal.AsBool();
                return DATATABLE_STATUS_OK;
            }
            if ( isPlainInteger )
            {
                CVariant native( literal );
                native.ToNativeEndian();
                if ( native.IsUnsignedInteger() ? native.AsUInt64() <= 1 : ( 0 == native.AsInt64() || 1 == native.AsInt64() ) )
                {
                    out = 0 != native.AsUInt64();
                    return DATATABLE_STATUS_OK;
                }
            }
            return DATATABLE_STATUS_TYPE_MISMATCH;
        }
        case DATATABLE_TYPE_INT8:
        case DATATABLE_TYPE_INT16:
        case DATATABLE_TYPE_INT32:
        case DATATABLE_TYPE_INT64:
        case DATATABLE_TYPE_UINT8:
        case DATATABLE_TYPE_UINT16:
        case DATATABLE_TYPE_UINT32:
        case DATATABLE_TYPE_UINT64:
        {
            if ( isPlainInteger )
            {
                CVariant native( literal );
                native.ToNativeEndian();
                if ( native.IsUnsignedInteger() )
                {
                    fit = FitInteger( column, false, native.AsUInt64(), FIT_EXACT, out );
                    return DATATABLE_STATUS_OK;
                }
                const Int64 value = native.AsInt64();
                const bool negative = value < 0;
                const UInt64 magnitude = negative ? (UInt64) ( -( value + 1 ) ) + 1 : (UInt64) value;
                fit = FitInteger( column, negative, magnitude, FIT_EXACT, out );
                return DATATABLE_STATUS_OK;
            }
            if ( literal.IsDecimal() )
            {
                CDecimal decimal;
                if ( DecimalStatusIsError( literal.GetDecimal( decimal ) ) )
                    return DATATABLE_STATUS_TYPE_MISMATCH;
                ELiteralFit decimalFit = FIT_EXACT;
                Int64 integral = 0;
                if ( DecimalStatusIsError( decimal.ToInt64( ROUNDINGMODE_UNNECESSARY, integral ) ) )
                {
                    if ( ROUND_EXACT == rounding )
                    {
                        fit = FIT_ROUNDED;
                        return DATATABLE_STATUS_OK;
                    }
                    decimalFit = FIT_ROUNDED;
                    if ( DecimalStatusIsError( decimal.ToInt64( ROUND_UP == rounding ? ROUNDINGMODE_CEILING : ROUNDINGMODE_FLOOR, integral ) ) )
                        return DATATABLE_STATUS_TYPE_MISMATCH;
                }
                const bool negative = integral < 0;
                const UInt64 magnitude = negative ? (UInt64) ( -( integral + 1 ) ) + 1 : (UInt64) integral;
                fit = FitInteger( column, negative, magnitude, decimalFit, out );
                return DATATABLE_STATUS_OK;
            }
            if ( literal.IsFloat() )
            {
                bool negative = false;
                UInt64 magnitude = 0;
                fit = FloatToInteger( literal.AsFloat64(), rounding, negative, magnitude );
                if ( FIT_EXACT == fit || ( FIT_ROUNDED == fit && ROUND_EXACT != rounding ) )
                    fit = FitInteger( column, negative, magnitude, fit, out );
                return DATATABLE_STATUS_OK;
            }
            return DATATABLE_STATUS_TYPE_MISMATCH;
        }
        case DATATABLE_TYPE_FLOAT32:
        case DATATABLE_TYPE_FLOAT64:
        {
            const bool narrow = DATATABLE_TYPE_FLOAT32 == column.type;
            Float64 value = 0.0;
            ELiteralFit valueFit = FIT_EXACT;
            if ( literal.IsFloat() )
            {
                value = literal.AsFloat64();
                if ( IsNaN( value ) )
                {
                    fit = FIT_NAN;
                    return DATATABLE_STATUS_OK;
                }
            }
            else
            if ( isPlainInteger )
            {
                // an integer beyond 2^53 has no exact float64 and is refused
                CVariant native( literal );
                native.ToNativeEndian();
                const UInt64 limit = (UInt64) 1 << 53;
                if ( native.IsUnsignedInteger() )
                {
                    if ( native.AsUInt64() > limit )
                        return DATATABLE_STATUS_TYPE_MISMATCH;
                    value = (Float64) native.AsUInt64();
                }
                else
                {
                    const Int64 integer = native.AsInt64();
                    if ( integer > (Int64) limit || integer < -(Int64) limit )
                        return DATATABLE_STATUS_TYPE_MISMATCH;
                    value = (Float64) integer;
                }
            }
            else
            if ( literal.IsDecimal() )
            {
                CDecimal decimal;
                if ( DecimalStatusIsError( literal.GetDecimal( decimal ) ) )
                    return DATATABLE_STATUS_TYPE_MISMATCH;
                value = decimal.ToFloat64();
                CDecimal check;
                if ( DecimalStatusIsError( CDecimal::FromFloat64( value, decimal.GetScale(), ROUNDINGMODE_UNNECESSARY, check ) ) || 0 != check.Compare( decimal ) )
                {
                    if ( ROUND_EXACT == rounding )
                    {
                        fit = FIT_ROUNDED;
                        return DATATABLE_STATUS_OK;
                    }
                    // the float64 nearest the decimal lies on one side of it: floor it at the decimal's scale to tell which
                    CDecimal floored;
                    if ( DecimalStatusIsError( CDecimal::FromFloat64( value, decimal.GetScale(), ROUNDINGMODE_FLOOR, floored ) ) )
                        return DATATABLE_STATUS_TYPE_MISMATCH;
                    const bool above = floored.Compare( decimal ) >= 0;
                    if ( ROUND_UP == rounding && !above )
                        value = NextFloat64( value, true );
                    else
                    if ( ROUND_DOWN == rounding && above )
                        value = NextFloat64( value, false );
                    valueFit = FIT_ROUNDED;
                }
            }
            else
            {
                return DATATABLE_STATUS_TYPE_MISMATCH;
            }

            if ( narrow )
            {
                Float64 narrowed = 0.0;
                fit = FitFloat32( value, rounding, valueFit, narrowed );
                out = narrowed;
                return DATATABLE_STATUS_OK;
            }
            fit = valueFit;
            out = value;
            return DATATABLE_STATUS_OK;
        }
        case DATATABLE_TYPE_DECIMAL:
        {
            CDecimal decimal;
            if ( literal.IsDecimal() )
            {
                if ( DecimalStatusIsError( literal.GetDecimal( decimal ) ) )
                    return DATATABLE_STATUS_TYPE_MISMATCH;
            }
            else
            if ( isPlainInteger )
            {
                CVariant native( literal );
                native.ToNativeEndian();
                if ( native.IsUnsignedInteger() && native.AsUInt64() > (UInt64) GUCEF_INT64MAX )
                {
                    fit = FIT_ABOVE;
                    return DATATABLE_STATUS_OK;
                }
                const Int64 integer = native.AsInt64();
                if ( GUCEF_INT64MIN == integer )
                {
                    fit = FIT_BELOW;
                    return DATATABLE_STATUS_OK;
                }
                if ( DecimalStatusIsError( CDecimal::FromInt64( integer, 0, decimal ) ) )
                {
                    fit = integer < 0 ? FIT_BELOW : FIT_ABOVE;
                    return DATATABLE_STATUS_OK;
                }
            }
            else
            if ( literal.IsFloat() )
            {
                const Float64 value = literal.AsFloat64();
                if ( IsNaN( value ) )
                {
                    fit = FIT_NAN;
                    return DATATABLE_STATUS_OK;
                }
                TDecimalStatus status = CDecimal::FromFloat64( value, column.decimalScale, ROUNDINGMODE_UNNECESSARY, decimal );
                if ( DecimalStatusIsError( status ) )
                {
                    if ( 0 != ( status & DECIMALSTATUS_ROUNDING_REQUIRED ) )
                    {
                        if ( ROUND_EXACT == rounding )
                        {
                            fit = FIT_ROUNDED;
                            return DATATABLE_STATUS_OK;
                        }
                        status = CDecimal::FromFloat64( value, column.decimalScale, ROUND_UP == rounding ? ROUNDINGMODE_CEILING : ROUNDINGMODE_FLOOR, decimal );
                        fit = FIT_ROUNDED;
                    }
                    if ( DecimalStatusIsError( status ) )
                    {
                        fit = value < 0.0 ? FIT_BELOW : FIT_ABOVE;
                        return DATATABLE_STATUS_OK;
                    }
                }
            }
            else
            {
                return DATATABLE_STATUS_TYPE_MISMATCH;
            }

            CDecimal scaled;
            TDecimalStatus status = decimal.Rescale( column.decimalScale, ROUNDINGMODE_UNNECESSARY, scaled );
            if ( DecimalStatusIsError( status ) && 0 != ( status & DECIMALSTATUS_ROUNDING_REQUIRED ) )
            {
                if ( ROUND_EXACT == rounding )
                {
                    fit = FIT_ROUNDED;
                    return DATATABLE_STATUS_OK;
                }
                status = decimal.Rescale( column.decimalScale, ROUND_UP == rounding ? ROUNDINGMODE_CEILING : ROUNDINGMODE_FLOOR, scaled );
                if ( FIT_EXACT == fit )
                    fit = FIT_ROUNDED;
            }
            if ( DecimalStatusIsError( status ) || !scaled.FitsPrecision( column.decimalPrecision ) )
            {
                fit = decimal.IsNegative() ? FIT_BELOW : FIT_ABOVE;
                return DATATABLE_STATUS_OK;
            }
            out = scaled.GetUnscaledValue();
            return DATATABLE_STATUS_OK;
        }
        case DATATABLE_TYPE_DATE:
        {
            if ( isDate )
            {
                CVariant native( literal );
                native.ToNativeEndian();
                out = (Int64) native.AsInt32();
                return DATATABLE_STATUS_OK;
            }
            if ( isTimestamp )
            {
                CVariant native( literal );
                native.ToNativeEndian();
                Int64 ticksPerDay = 86400;
                TDataTableTimeUnit unit = DATATABLE_TIMEUNIT_MICROSECONDS;
                if ( CDataTableValues::TimeUnitFromTypeId( typeId, unit ) )
                    ticksPerDay = CDataTableValues::TicksPerDay( unit );
                const Int64 ticks = native.AsInt64();
                Int64 days = ticks / ticksPerDay;
                const Int64 remainder = ticks % ticksPerDay;
                if ( remainder < 0 )
                    --days;
                if ( 0 != remainder )
                {
                    if ( ROUND_EXACT == rounding )
                    {
                        fit = FIT_ROUNDED;
                        return DATATABLE_STATUS_OK;
                    }
                    fit = FIT_ROUNDED;
                    if ( ROUND_UP == rounding )
                        ++days;
                }
                if ( days < GUCEF_INT32MIN )
                {
                    fit = FIT_BELOW;
                    return DATATABLE_STATUS_OK;
                }
                if ( days > GUCEF_INT32MAX )
                {
                    fit = FIT_ABOVE;
                    return DATATABLE_STATUS_OK;
                }
                out = days;
                return DATATABLE_STATUS_OK;
            }
            return DATATABLE_STATUS_TYPE_MISMATCH;
        }
        case DATATABLE_TYPE_TIMESTAMP:
        {
            if ( isDate )
            {
                CVariant native( literal );
                native.ToNativeEndian();
                const Int64 days = native.AsInt32();
                const Int64 ticksPerDay = CDataTableValues::TicksPerDay( column.timeUnit );
                if ( days > GUCEF_INT64MAX / ticksPerDay )
                {
                    fit = FIT_ABOVE;
                    return DATATABLE_STATUS_OK;
                }
                if ( days < GUCEF_INT64MIN / ticksPerDay )
                {
                    fit = FIT_BELOW;
                    return DATATABLE_STATUS_OK;
                }
                out = days * ticksPerDay;
                return DATATABLE_STATUS_OK;
            }
            if ( isTimestamp )
            {
                CVariant native( literal );
                native.ToNativeEndian();
                const Int64 ticks = native.AsInt64();
                TDataTableTimeUnit unit = DATATABLE_TIMEUNIT_MICROSECONDS;
                if ( !CDataTableValues::TimeUnitFromTypeId( typeId, unit ) )
                {
                    // seconds: widen to the column unit
                    const Int64 perSecond = CDataTableValues::TicksPerSecond( column.timeUnit );
                    if ( ticks > GUCEF_INT64MAX / perSecond )
                        fit = FIT_ABOVE;
                    else
                    if ( ticks < GUCEF_INT64MIN / perSecond )
                        fit = FIT_BELOW;
                    else
                        out = ticks * perSecond;
                    return DATATABLE_STATUS_OK;
                }
                Int64 converted = 0;
                if ( CDataTableValues::ConvertTicks( ticks, unit, column.timeUnit, converted ) )
                {
                    out = converted;
                    return DATATABLE_STATUS_OK;
                }
                // a finer unit that needs rounding, or a coarser one that overflows the column unit
                if ( CDataTableValues::TicksPerSecond( unit ) < CDataTableValues::TicksPerSecond( column.timeUnit ) )
                {
                    fit = ticks < 0 ? FIT_BELOW : FIT_ABOVE;
                    return DATATABLE_STATUS_OK;
                }
                if ( ROUND_EXACT == rounding )
                {
                    fit = FIT_ROUNDED;
                    return DATATABLE_STATUS_OK;
                }
                if ( !CDataTableValues::ConvertTicksRounded( ticks, unit, column.timeUnit, ROUND_UP == rounding, converted ) )
                    return DATATABLE_STATUS_TYPE_MISMATCH;
                fit = FIT_ROUNDED;
                out = converted;
                return DATATABLE_STATUS_OK;
            }
            return DATATABLE_STATUS_TYPE_MISMATCH;
        }
        case DATATABLE_TYPE_UTF8:
        {
            if ( literal.IsString() )
            {
                out = literal.AsUtf8String();
                return DATATABLE_STATUS_OK;
            }
            return DATATABLE_STATUS_TYPE_MISMATCH;
        }
        case DATATABLE_TYPE_BINARY:
        {
            if ( GUCEF_DATATYPE_BINARY_BLOB == typeId || GUCEF_DATATYPE_BINARY_BSOB == typeId )
            {
                out.Set( literal.AsVoidPtr(), literal.ByteSize(), GUCEF_DATATYPE_BINARY_BLOB, false );
                return DATATABLE_STATUS_OK;
            }
            if ( literal.IsString() )
            {
                out.Set( literal.AsCharPtr(), literal.ByteSize( false ), GUCEF_DATATYPE_BINARY_BLOB, false );
                return DATATABLE_STATUS_OK;
            }
            return DATATABLE_STATUS_TYPE_MISMATCH;
        }
        default:
        {
            return DATATABLE_STATUS_UNSUPPORTED;
        }
    }
}

/** Sorts native values ascending (insertion of a small set; IN lists are short) and drops duplicates */
void
SortUnique( TDataTableLogicalType type, CVariant::VariantVector& values )
{
    for ( UInt32 i=1; i<(UInt32) values.size(); ++i )
    {
        UInt32 j = i;
        while ( j > 0 && CDataTablePruner::CompareNative( type, values[ j - 1 ], values[ j ] ) > 0 )
        {
            CVariant swap( values[ j - 1 ] );
            values[ j - 1 ] = values[ j ];
            values[ j ] = swap;
            --j;
        }
    }
    UInt32 kept = 0;
    for ( UInt32 i=0; i<(UInt32) values.size(); ++i )
    {
        if ( kept > 0 && 0 == CDataTablePruner::CompareNative( type, values[ kept - 1 ], values[ i ] ) )
            continue;
        if ( kept != i )
            values[ kept ] = values[ i ];
        ++kept;
    }
    values.resize( kept );
}

/** Resolves one bound of a range: a lower bound rounds up, an upper bound rounds down */
TDataTableStatus
ResolveBound( const CDataTableColumnDef& column, const CVariant& literal, bool isLow, bool& inclusive, bool& hasBound,
              CVariant& bound, bool& matchesNothing )
{
    ELiteralFit fit = FIT_EXACT;
    TDataTableStatus status = ResolveLiteral( column, literal, isLow ? ROUND_UP : ROUND_DOWN, bound, fit );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    switch ( fit )
    {
        case FIT_EXACT:     { hasBound = true; return DATATABLE_STATUS_OK; }
        case FIT_ROUNDED:   { hasBound = true; inclusive = true; return DATATABLE_STATUS_OK; }
        case FIT_NAN:       { matchesNothing = true; hasBound = false; return DATATABLE_STATUS_OK; }
        case FIT_BELOW:
        {
            // below every value: a lower bound always holds, an upper bound never
            hasBound = false;
            if ( !isLow )
                matchesNothing = true;
            return DATATABLE_STATUS_OK;
        }
        case FIT_ABOVE:
        {
            hasBound = false;
            if ( isLow )
                matchesNothing = true;
            return DATATABLE_STATUS_OK;
        }
        default:            { return DATATABLE_STATUS_FAILED; }
    }
}

inline bool
IsUnbounded( const CVariant& value )
{
    return !value.IsInitialized() || value.IsNULL();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableQuery::SSelectedColumn::SSelectedColumn( void )
    : columnName()
    , columnIndex( GUCEF_UINT32MAX )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableQuery::CDataTableQuery( void )
    : m_selection()
    , m_predicates()
    , m_limit( NoLimit )
    , m_batchRowCount( DataTableDefaultBatchRowCount )
    , m_filterMode( DATATABLE_FILTER_EXACT )
    , m_orderedDelivery( true )
    , m_materializeDictionaries( false )
    , m_maxParallelism( 0 )
    , m_maxBatchesInFlight( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::Select( const CString& columnName )
{GUCEF_TRACE;

    SSelectedColumn column;
    column.columnName = columnName;
    m_selection.push_back( column );
    return *this;
}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::Select( UInt32 schemaColumnIndex )
{GUCEF_TRACE;

    SSelectedColumn column;
    column.columnIndex = schemaColumnIndex;
    m_selection.push_back( column );
    return *this;
}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::Where( const CDataTablePredicate& predicate )
{GUCEF_TRACE;

    m_predicates.push_back( predicate );
    return *this;
}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::WhereEquals( const CString& columnName, const CVariant& value )
{GUCEF_TRACE;

    return Where( CDataTablePredicate( columnName, DATATABLE_PREDICATE_EQUALS, value ) );
}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::WhereIn( const CString& columnName, const CVariant::VariantVector& values )
{GUCEF_TRACE;

    return Where( CDataTablePredicate( columnName, values ) );
}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::WhereRange( const CString& columnName, const CVariant& low, bool lowInclusive, const CVariant& high, bool highInclusive )
{GUCEF_TRACE;

    return Where( CDataTablePredicate( columnName, low, lowInclusive, high, highInclusive ) );
}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::WhereNull( const CString& columnName, bool isNull )
{GUCEF_TRACE;

    return Where( CDataTablePredicate( columnName, isNull ? DATATABLE_PREDICATE_IS_NULL : DATATABLE_PREDICATE_IS_NOT_NULL ) );
}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::SetLimit( UInt64 maxRows )
{GUCEF_TRACE;

    m_limit = maxRows;
    return *this;
}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::SetBatchRowCount( UInt32 rows )
{GUCEF_TRACE;

    if ( rows < DataTableMinBatchRowCount )
        rows = DataTableMinBatchRowCount;
    if ( rows > DataTableMaxBatchRowCount )
        rows = DataTableMaxBatchRowCount;
    m_batchRowCount = rows;
    return *this;
}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::SetFilterMode( TDataTableFilterMode mode )
{GUCEF_TRACE;

    m_filterMode = mode;
    return *this;
}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::SetOrderedDelivery( bool ordered )
{GUCEF_TRACE;

    m_orderedDelivery = ordered;
    return *this;
}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::SetMaterializeDictionaries( bool materialize )
{GUCEF_TRACE;

    m_materializeDictionaries = materialize;
    return *this;
}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::SetMaxParallelism( UInt32 tasks )
{GUCEF_TRACE;

    m_maxParallelism = tasks;
    return *this;
}

/*-------------------------------------------------------------------------*/

CDataTableQuery&
CDataTableQuery::SetMaxBatchesInFlight( UInt32 batches )
{GUCEF_TRACE;

    m_maxBatchesInFlight = batches;
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableQuery::Resolve( const CDataTableSchema& schema, CDataTableResolvedQuery& resolved, CString& error ) const
{GUCEF_TRACE;

    resolved.projection.clear();
    resolved.predicates.clear();
    resolved.matchesNothing = false;
    resolved.status = DATATABLE_STATUS_OK;
    resolved.limit = m_limit;
    resolved.batchRowCount = m_batchRowCount;
    resolved.filterMode = m_filterMode;
    resolved.orderedDelivery = m_orderedDelivery;
    resolved.materializeDictionaries = m_materializeDictionaries;
    resolved.maxParallelism = m_maxParallelism;
    resolved.maxBatchesInFlight = m_maxBatchesInFlight;
    if ( resolved.schema.IsNULL() )
        resolved.schema = CreateDataTableSchemaPtr( schema );

    const UInt32 columnCount = schema.GetColumnCount();

    // the projection, in the order given; none selects every column that can be read
    if ( m_selection.empty() )
    {
        for ( UInt32 i=0; i<columnCount; ++i )
        {
            if ( DATATABLE_TYPE_UNSUPPORTED != schema.GetColumn( i ).type )
                resolved.projection.push_back( i );
        }
    }
    for ( UInt32 i=0; i<(UInt32) m_selection.size(); ++i )
    {
        const SSelectedColumn& selected = m_selection[ i ];
        UInt32 index = selected.columnIndex;
        if ( !selected.columnName.IsNULLOrEmpty() )
        {
            Int32 found = schema.FindColumn( selected.columnName );
            if ( found < 0 )
            {
                error = "The table has no column \"" + selected.columnName + "\"";
                resolved.status = DATATABLE_STATUS_NOT_FOUND;
                return false;
            }
            index = (UInt32) found;
        }
        if ( index >= columnCount )
        {
            error = "Column index " + UInt32ToString( index ) + " is outside the " + UInt32ToString( columnCount ) + " columns of the table";
            resolved.status = DATATABLE_STATUS_NOT_FOUND;
            return false;
        }
        if ( DATATABLE_TYPE_UNSUPPORTED == schema.GetColumn( index ).type )
        {
            error = "Column \"" + schema.GetColumn( index ).name + "\" has a type that cannot be read and cannot be selected";
            resolved.status = DATATABLE_STATUS_UNSUPPORTED;
            return false;
        }
        resolved.projection.push_back( index );
    }

    for ( UInt32 p=0; p<(UInt32) m_predicates.size(); ++p )
    {
        const CDataTablePredicate& predicate = m_predicates[ p ];
        if ( !predicate.IsWellFormed( error ) )
        {
            resolved.status = DATATABLE_STATUS_INVALID_ARGUMENT;
            return false;
        }
        Int32 found = schema.FindColumn( predicate.columnName );
        if ( found < 0 )
        {
            error = "A predicate names column \"" + predicate.columnName + "\", which the table does not have";
            resolved.status = DATATABLE_STATUS_NOT_FOUND;
            return false;
        }
        const CDataTableColumnDef& column = schema.GetColumn( (UInt32) found );
        if ( DATATABLE_TYPE_UNSUPPORTED == column.type )
        {
            error = "Column \"" + column.name + "\" has a type that cannot be read and cannot be filtered on";
            resolved.status = DATATABLE_STATUS_UNSUPPORTED;
            return false;
        }

        CDataTableResolvedPredicate result;
        result.columnIndex = (UInt32) found;
        result.op = predicate.op;
        TDataTableStatus status = DATATABLE_STATUS_OK;

        switch ( predicate.op )
        {
            case DATATABLE_PREDICATE_EQUALS:
            case DATATABLE_PREDICATE_NOT_EQUALS:
            {
                ELiteralFit fit = FIT_EXACT;
                status = ResolveLiteral( column, predicate.low, ROUND_EXACT, result.low, fit );
                if ( DATATABLE_STATUS_OK == status && FIT_EXACT != fit )
                {
                    // a literal the column cannot hold: no row equals it, every non-null row differs from it
                    result.low.Clear();
                    if ( DATATABLE_PREDICATE_EQUALS == predicate.op )
                        result.matchesNothing = true;
                    else
                        result.matchesAllNonNull = true;
                }
                break;
            }
            case DATATABLE_PREDICATE_LESS:
            case DATATABLE_PREDICATE_LESS_OR_EQUAL:
            {
                result.highInclusive = DATATABLE_PREDICATE_LESS_OR_EQUAL == predicate.op;
                status = ResolveBound( column, predicate.low, false, result.highInclusive, result.hasHigh, result.high, result.matchesNothing );
                if ( DATATABLE_STATUS_OK == status && !result.hasHigh && !result.matchesNothing )
                    result.matchesAllNonNull = true;
                break;
            }
            case DATATABLE_PREDICATE_GREATER:
            case DATATABLE_PREDICATE_GREATER_OR_EQUAL:
            {
                result.lowInclusive = DATATABLE_PREDICATE_GREATER_OR_EQUAL == predicate.op;
                status = ResolveBound( column, predicate.low, true, result.lowInclusive, result.hasLow, result.low, result.matchesNothing );
                if ( DATATABLE_STATUS_OK == status && !result.hasLow && !result.matchesNothing )
                    result.matchesAllNonNull = true;
                break;
            }
            case DATATABLE_PREDICATE_RANGE:
            {
                result.lowInclusive = predicate.lowInclusive;
                result.highInclusive = predicate.highInclusive;
                if ( !IsUnbounded( predicate.low ) )
                    status = ResolveBound( column, predicate.low, true, result.lowInclusive, result.hasLow, result.low, result.matchesNothing );
                if ( DATATABLE_STATUS_OK == status && !IsUnbounded( predicate.high ) )
                    status = ResolveBound( column, predicate.high, false, result.highInclusive, result.hasHigh, result.high, result.matchesNothing );
                if ( DATATABLE_STATUS_OK == status && !result.matchesNothing )
                {
                    if ( result.hasLow && result.hasHigh )
                    {
                        const Int32 order = CDataTablePruner::CompareNative( column.type, result.low, result.high );
                        if ( order > 0 || ( 0 == order && ( !result.lowInclusive || !result.highInclusive ) ) )
                            result.matchesNothing = true;
                    }
                    else
                    if ( !result.hasLow && !result.hasHigh )
                    {
                        result.matchesAllNonNull = true;
                    }
                }
                break;
            }
            case DATATABLE_PREDICATE_IN:
            {
                for ( UInt32 v=0; v<(UInt32) predicate.values.size(); ++v )
                {
                    CVariant native;
                    ELiteralFit fit = FIT_EXACT;
                    status = ResolveLiteral( column, predicate.values[ v ], ROUND_EXACT, native, fit );
                    if ( DATATABLE_STATUS_OK != status )
                        break;
                    if ( FIT_EXACT == fit )
                        result.values.push_back( native );
                }
                if ( DATATABLE_STATUS_OK == status )
                {
                    SortUnique( column.type, result.values );
                    if ( result.values.empty() )
                        result.matchesNothing = true;
                }
                break;
            }
            case DATATABLE_PREDICATE_IS_NULL:
            {
                if ( !column.nullable )
                    result.matchesNothing = true;
                break;
            }
            case DATATABLE_PREDICATE_IS_NOT_NULL:
            {
                result.matchesAllNonNull = true;
                break;
            }
            default:
            {
                status = DATATABLE_STATUS_INVALID_ARGUMENT;
                break;
            }
        }

        if ( DATATABLE_STATUS_OK != status )
        {
            error = "Predicate " + CString( DataTablePredicateOpToString( predicate.op ) ) + " on column \"" + column.name + "\" (" +
                    column.GetTypeText() + ") has a literal that is not a value of the column type";
            resolved.status = status;
            return false;
        }
        if ( result.matchesNothing )
        {
            result.matchesAllNonNull = false;
            resolved.matchesNothing = true;
        }
        resolved.predicates.push_back( result );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CDataTableQuery::TSelectedColumnVector&
CDataTableQuery::GetSelection( void ) const
{GUCEF_TRACE;

    return m_selection;
}

/*-------------------------------------------------------------------------*/

const CDataTableQuery::TPredicateVector&
CDataTableQuery::GetPredicates( void ) const
{GUCEF_TRACE;

    return m_predicates;
}

/*-------------------------------------------------------------------------*/

UInt64
CDataTableQuery::GetLimit( void ) const
{GUCEF_TRACE;

    return m_limit;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableQuery::GetBatchRowCount( void ) const
{GUCEF_TRACE;

    return m_batchRowCount;
}

/*-------------------------------------------------------------------------*/

TDataTableFilterMode
CDataTableQuery::GetFilterMode( void ) const
{GUCEF_TRACE;

    return m_filterMode;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableQuery::GetOrderedDelivery( void ) const
{GUCEF_TRACE;

    return m_orderedDelivery;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableQuery::GetMaterializeDictionaries( void ) const
{GUCEF_TRACE;

    return m_materializeDictionaries;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableQuery::GetMaxParallelism( void ) const
{GUCEF_TRACE;

    return m_maxParallelism;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataTableQuery::GetMaxBatchesInFlight( void ) const
{GUCEF_TRACE;

    return m_maxBatchesInFlight;
}

/*-------------------------------------------------------------------------*/

CDataTableResolvedQuery::CDataTableResolvedQuery( void )
    : schema()
    , projection()
    , predicates()
    , matchesNothing( false )
    , status( DATATABLE_STATUS_OK )
    , limit( CDataTableQuery::NoLimit )
    , batchRowCount( DataTableDefaultBatchRowCount )
    , filterMode( DATATABLE_FILTER_EXACT )
    , orderedDelivery( true )
    , materializeDictionaries( false )
    , maxParallelism( 0 )
    , maxBatchesInFlight( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDataTableResolvedQuery::GetPredicatesOnColumn( UInt32 schemaColumnIndex, TResolvedPredicateVector& columnPredicates ) const
{GUCEF_TRACE;

    columnPredicates.clear();
    for ( UInt32 i=0; i<(UInt32) predicates.size(); ++i )
    {
        if ( predicates[ i ].columnIndex == schemaColumnIndex )
            columnPredicates.push_back( predicates[ i ] );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
