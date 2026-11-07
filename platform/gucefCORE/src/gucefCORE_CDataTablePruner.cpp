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

#include "gucefCORE_CDataTablePruner.h"

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

enum ENativeFamily
{
    NATIVE_NONE = 0     ,
    NATIVE_SIGNED       ,   /**< Int64: signed integers, decimals, dates, timestamps, booleans */
    NATIVE_UNSIGNED     ,   /**< UInt64 */
    NATIVE_FLOAT        ,   /**< Float64 */
    NATIVE_BYTES            /**< text and binary */
};

struct SNativeValue
{
    Int64 i;
    UInt64 u;
    Float64 f;
    const char* bytes;
    UInt32 byteLength;
};

ENativeFamily
NativeFamilyOf( TDataTableLogicalType type )
{
    switch ( type )
    {
        case DATATABLE_TYPE_BOOL:
        case DATATABLE_TYPE_INT8:
        case DATATABLE_TYPE_INT16:
        case DATATABLE_TYPE_INT32:
        case DATATABLE_TYPE_INT64:
        case DATATABLE_TYPE_DECIMAL:
        case DATATABLE_TYPE_DATE:
        case DATATABLE_TYPE_TIMESTAMP:  return NATIVE_SIGNED;
        case DATATABLE_TYPE_UINT8:
        case DATATABLE_TYPE_UINT16:
        case DATATABLE_TYPE_UINT32:
        case DATATABLE_TYPE_UINT64:     return NATIVE_UNSIGNED;
        case DATATABLE_TYPE_FLOAT32:
        case DATATABLE_TYPE_FLOAT64:    return NATIVE_FLOAT;
        case DATATABLE_TYPE_UTF8:
        case DATATABLE_TYPE_BINARY:     return NATIVE_BYTES;
        default:                        return NATIVE_NONE;
    }
}

inline bool
IsNaN( Float64 value )
{
    return value != value;
}

bool
ReadNative( ENativeFamily family, const CVariant& value, SNativeValue& out )
{
    if ( !value.IsInitialized() || value.IsNULL() )
        return false;
    switch ( family )
    {
        case NATIVE_SIGNED:     { out.i = value.AsInt64(); return true; }
        case NATIVE_UNSIGNED:   { out.u = value.AsUInt64(); return true; }
        case NATIVE_FLOAT:      { out.f = value.AsFloat64(); return true; }
        case NATIVE_BYTES:
        {
            out.bytes = CDataTablePruner::GetNativeBytes( value, out.byteLength );
            return GUCEF_NULL != out.bytes || 0 == out.byteLength;
        }
        default:                { return false; }
    }
}

inline Int32
CompareBytes( const char* a, UInt32 aLength, const char* b, UInt32 bLength )
{
    const UInt32 common = aLength < bLength ? aLength : bLength;
    if ( common > 0 )
    {
        int result = memcmp( a, b, common );
        if ( 0 != result )
            return result < 0 ? -1 : 1;
    }
    return aLength < bLength ? -1 : ( aLength > bLength ? 1 : 0 );
}

/** Orders two native values of one family; floats must not be NaN */
inline Int32
CompareValues( ENativeFamily family, const SNativeValue& a, const SNativeValue& b )
{
    switch ( family )
    {
        case NATIVE_SIGNED:     { return a.i < b.i ? -1 : ( a.i > b.i ? 1 : 0 ); }
        case NATIVE_UNSIGNED:   { return a.u < b.u ? -1 : ( a.u > b.u ? 1 : 0 ); }
        case NATIVE_FLOAT:      { return a.f < b.f ? -1 : ( a.f > b.f ? 1 : 0 ); }
        case NATIVE_BYTES:      { return CompareBytes( a.bytes, a.byteLength, b.bytes, b.byteLength ); }
        default:                { return 0; }
    }
}

/** Mantissas of a decimal at a lower scale brought to the column scale; false on overflow */
bool
RescaleMantissa( Int64& mantissa, UInt8 fromScale, UInt8 toScale )
{
    for ( UInt8 s=fromScale; s<toScale; ++s )
    {
        if ( mantissa > GUCEF_INT64MAX / 10 || mantissa < -( GUCEF_INT64MAX / 10 ) )
            return false;
        mantissa *= 10;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
CDataTablePruner::GetNativeBytes( const CVariant& value, UInt32& byteLength )
{GUCEF_TRACE;

    if ( value.IsString() )
    {
        byteLength = value.ByteSize( false );
        return value.AsCharPtr();
    }
    byteLength = value.ByteSize();
    return static_cast< const char* >( value.AsVoidPtr() );
}

/*-------------------------------------------------------------------------*/

Int32
CDataTablePruner::CompareNative( TDataTableLogicalType type, const CVariant& a, const CVariant& b )
{GUCEF_TRACE;

    const ENativeFamily family = NativeFamilyOf( type );
    SNativeValue left;
    SNativeValue right;
    if ( !ReadNative( family, a, left ) || !ReadNative( family, b, right ) )
        return 0;
    if ( NATIVE_FLOAT == family )
    {
        const bool leftNaN = IsNaN( left.f );
        const bool rightNaN = IsNaN( right.f );
        if ( leftNaN || rightNaN )
            return leftNaN == rightNaN ? 0 : ( leftNaN ? 1 : -1 );
    }
    return CompareValues( family, left, right );
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePruner::MayMatch( const CDataTableResolvedPredicate& predicate ,
                            const CDataTableColumnDef& column            ,
                            const CDataTableColumnStats& stats           ,
                            const CDataTableColumnDef* statsColumn       )
{GUCEF_TRACE;

    if ( predicate.matchesNothing )
        return false;

    const bool allNull = stats.hasNullCount && stats.valueCount > 0 && stats.nullCount >= stats.valueCount;
    switch ( predicate.op )
    {
        case DATATABLE_PREDICATE_IS_NULL:       { return !( stats.hasNullCount && 0 == stats.nullCount ); }
        case DATATABLE_PREDICATE_IS_NOT_NULL:   { return !allNull; }
        default:                                { break; }
    }

    // a comparison or set test never matches a null
    if ( allNull )
        return false;
    if ( predicate.matchesAllNonNull || !stats.hasMinMax )
        return true;

    const ENativeFamily family = NativeFamilyOf( column.type );
    SNativeValue minValue;
    SNativeValue maxValue;
    if ( !ReadNative( family, stats.minValue, minValue ) || !ReadNative( family, stats.maxValue, maxValue ) )
        return true;

    if ( NATIVE_FLOAT == family )
    {
        // NaN in the statistics makes them unusable; a zero min is -0.0 and a zero max +0.0 (IEEE compares them equal)
        if ( IsNaN( minValue.f ) || IsNaN( maxValue.f ) )
            return true;
    }
    else
    if ( NATIVE_SIGNED == family && DATATABLE_TYPE_DECIMAL == column.type && GUCEF_NULL != statsColumn &&
         DATATABLE_TYPE_DECIMAL == statsColumn->type && statsColumn->decimalScale < column.decimalScale )
    {
        if ( !RescaleMantissa( minValue.i, statsColumn->decimalScale, column.decimalScale ) ||
             !RescaleMantissa( maxValue.i, statsColumn->decimalScale, column.decimalScale ) )
            return true;
    }

    switch ( predicate.op )
    {
        case DATATABLE_PREDICATE_EQUALS:
        {
            SNativeValue value;
            if ( !ReadNative( family, predicate.low, value ) )
                return true;
            if ( NATIVE_FLOAT == family && IsNaN( value.f ) )
                return false;
            return CompareValues( family, value, minValue ) >= 0 && CompareValues( family, value, maxValue ) <= 0;
        }
        case DATATABLE_PREDICATE_NOT_EQUALS:
        {
            SNativeValue value;
            if ( !ReadNative( family, predicate.low, value ) )
                return true;
            if ( !stats.minExact || !stats.maxExact )
                return true;
            // NaN cells are left out of the bounds and match every NOT_EQUALS: only a unit known to hold none can be pruned
            if ( NATIVE_FLOAT == family && ( IsNaN( value.f ) || !stats.hasNanCount || stats.nanCount > 0 ) )
                return true;
            return !( 0 == CompareValues( family, minValue, maxValue ) && 0 == CompareValues( family, value, minValue ) );
        }
        case DATATABLE_PREDICATE_LESS:
        case DATATABLE_PREDICATE_LESS_OR_EQUAL:
        case DATATABLE_PREDICATE_GREATER:
        case DATATABLE_PREDICATE_GREATER_OR_EQUAL:
        case DATATABLE_PREDICATE_RANGE:
        {
            if ( predicate.hasHigh )
            {
                SNativeValue high;
                if ( ReadNative( family, predicate.high, high ) )
                {
                    if ( NATIVE_FLOAT == family && IsNaN( high.f ) )
                        return false;
                    const Int32 order = CompareValues( family, high, minValue );
                    if ( order < 0 || ( 0 == order && !predicate.highInclusive ) )
                        return false;
                }
            }
            if ( predicate.hasLow )
            {
                SNativeValue low;
                if ( ReadNative( family, predicate.low, low ) )
                {
                    if ( NATIVE_FLOAT == family && IsNaN( low.f ) )
                        return false;
                    const Int32 order = CompareValues( family, low, maxValue );
                    if ( order > 0 || ( 0 == order && !predicate.lowInclusive ) )
                        return false;
                }
            }
            return true;
        }
        case DATATABLE_PREDICATE_IN:
        {
            // the values are sorted: find the first that is not below the minimum
            const UInt32 count = (UInt32) predicate.values.size();
            UInt32 first = 0;
            UInt32 last = count;
            while ( first < last )
            {
                const UInt32 middle = first + ( ( last - first ) >> 1 );
                SNativeValue value;
                if ( !ReadNative( family, predicate.values[ middle ], value ) )
                    return true;
                if ( CompareValues( family, value, minValue ) < 0 )
                    first = middle + 1;
                else
                    last = middle;
            }
            if ( first >= count )
                return false;
            SNativeValue candidate;
            if ( !ReadNative( family, predicate.values[ first ], candidate ) )
                return true;
            return CompareValues( family, candidate, maxValue ) <= 0;
        }
        default:
        {
            return true;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CDataTablePruner::RowGroupMayMatch( const CDataTableResolvedQuery& query                               ,
                                    const CDataTableRowGroupInfo& rowGroup                             ,
                                    const CDataTableSchema::TColumnIndexVector& fileColumnOfTableColumn ,
                                    const CDataTableSchema* fileSchema                                 )
{GUCEF_TRACE;

    if ( query.matchesNothing || 0 == rowGroup.rowCount )
        return false;
    if ( query.schema.IsNULL() )
        return true;

    const CDataTableSchema& schema = *query.schema;
    for ( UInt32 i=0; i<(UInt32) query.predicates.size(); ++i )
    {
        const CDataTableResolvedPredicate& predicate = query.predicates[ i ];
        if ( predicate.columnIndex >= schema.GetColumnCount() )
            continue;

        UInt32 fileColumn = predicate.columnIndex;
        if ( !fileColumnOfTableColumn.empty() )
            fileColumn = predicate.columnIndex < (UInt32) fileColumnOfTableColumn.size() ? fileColumnOfTableColumn[ predicate.columnIndex ] : GUCEF_UINT32MAX;

        if ( GUCEF_UINT32MAX == fileColumn )
        {
            // the file lacks the column: every row is null there
            if ( DATATABLE_PREDICATE_IS_NULL != predicate.op || predicate.matchesNothing )
                return false;
            continue;
        }

        const CDataTableColumnStats* stats = rowGroup.GetColumnStats( fileColumn );
        if ( GUCEF_NULL == stats )
            continue;

        const CDataTableColumnDef* statsColumn = GUCEF_NULL;
        if ( GUCEF_NULL != fileSchema && fileColumn < fileSchema->GetColumnCount() )
            statsColumn = &fileSchema->GetColumn( fileColumn );

        if ( !MayMatch( predicate, schema.GetColumn( predicate.columnIndex ), *stats, statsColumn ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
