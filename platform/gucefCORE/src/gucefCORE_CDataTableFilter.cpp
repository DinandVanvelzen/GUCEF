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

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CDATATABLEPRUNER_H
#include "gucefCORE_CDataTablePruner.h"
#define GUCEF_CORE_CDATATABLEPRUNER_H
#endif /* GUCEF_CORE_CDATATABLEPRUNER_H ? */

#include "gucefCORE_CDataTableFilter.h"

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

enum EFilterKind
{
    FILTERKIND_NOTHING = 0  ,   /**< no row matches */
    FILTERKIND_NOT_NULL     ,   /**< every non-null row matches */
    FILTERKIND_IS_NULL      ,
    FILTERKIND_RANGE        ,   /**< inclusive bounds, EQUALS as low == high */
    FILTERKIND_NOT_EQUALS   ,
    FILTERKIND_IN
};

enum EFilterFamily
{
    FILTERFAMILY_NONE = 0   ,
    FILTERFAMILY_SIGNED     ,
    FILTERFAMILY_UNSIGNED   ,
    FILTERFAMILY_FLOAT      ,
    FILTERFAMILY_BYTES
};

EFilterFamily
FilterFamilyOf( TDataTableLogicalType type )
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
        case DATATABLE_TYPE_TIMESTAMP:  return FILTERFAMILY_SIGNED;
        case DATATABLE_TYPE_UINT8:
        case DATATABLE_TYPE_UINT16:
        case DATATABLE_TYPE_UINT32:
        case DATATABLE_TYPE_UINT64:     return FILTERFAMILY_UNSIGNED;
        case DATATABLE_TYPE_FLOAT32:
        case DATATABLE_TYPE_FLOAT64:    return FILTERFAMILY_FLOAT;
        case DATATABLE_TYPE_UTF8:
        case DATATABLE_TYPE_BINARY:     return FILTERFAMILY_BYTES;
        default:                        return FILTERFAMILY_NONE;
    }
}

/** The next float64 up or down; +inf up and -inf down give NaN, which no comparison holds */
Float64
NextFloat64Toward( Float64 value, bool up )
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
    if ( ( value > 0.0 ) == up )
        ++bits;
    else
        --bits;
    Float64 result = 0.0;
    memcpy( &result, &bits, sizeof( result ) );
    return result;
}

inline bool
BitIsSet( const UInt8* bits, UInt32 index )
{
    return 0 != ( bits[ index >> 3 ] & ( 1u << ( index & 7u ) ) );
}

inline Int32
CompareByteRanges( const char* a, UInt32 aLength, const char* b, UInt32 bLength )
{
    const UInt32 common = aLength < bLength ? aLength : bLength;
    if ( common > 0 )
    {
        const int result = memcmp( a, b, common );
        if ( 0 != result )
            return result;
    }
    return aLength < bLength ? -1 : ( aLength > bLength ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

/**
 *  Writes the rows that pass the test to out: all rows below count, or the candidate rows. out may be the candidates:
 *  a row is read before anything is written at or after its position. Branch free in the loop body.
 */
template< class TTest >
UInt32
SelectRows( const TTest& test, const UInt8* validity, const UInt32* candidates, UInt32 count, UInt32* out )
{
    UInt32 selected = 0;
    if ( GUCEF_NULL == candidates )
    {
        if ( GUCEF_NULL == validity )
        {
            for ( UInt32 row=0; row<count; ++row )
            {
                out[ selected ] = row;
                selected += test( row ) ? 1 : 0;
            }
        }
        else
        {
            for ( UInt32 row=0; row<count; ++row )
            {
                out[ selected ] = row;
                selected += ( BitIsSet( validity, row ) && test( row ) ) ? 1 : 0;
            }
        }
    }
    else
    {
        if ( GUCEF_NULL == validity )
        {
            for ( UInt32 i=0; i<count; ++i )
            {
                const UInt32 row = candidates[ i ];
                out[ selected ] = row;
                selected += test( row ) ? 1 : 0;
            }
        }
        else
        {
            for ( UInt32 i=0; i<count; ++i )
            {
                const UInt32 row = candidates[ i ];
                out[ selected ] = row;
                selected += ( BitIsSet( validity, row ) && test( row ) ) ? 1 : 0;
            }
        }
    }
    return selected;
}

/*-------------------------------------------------------------------------*/

template< typename T, typename C >
struct TRangeTest
{
    const T* values;
    C low;
    C high;
    inline bool operator()( UInt32 row ) const { const C value = (C) values[ row ]; return value >= low && value <= high; }
};

template< typename T, typename C >
struct TNotEqualTest
{
    const T* values;
    C operand;
    inline bool operator()( UInt32 row ) const { return (C) values[ row ] != operand; }
};

template< typename T, typename C >
struct TInTest
{
    const T* values;
    const C* set;
    UInt32 setSize;
    inline bool operator()( UInt32 row ) const
    {
        const C value = (C) values[ row ];
        UInt32 first = 0;
        UInt32 last = setSize;
        while ( first < last )
        {
            const UInt32 middle = first + ( ( last - first ) >> 1 );
            if ( set[ middle ] < value )
                first = middle + 1;
            else
                last = middle;
        }
        return first < setSize && set[ first ] == value;
    }
};

struct SNullTest
{
    const UInt8* validity;
    inline bool operator()( UInt32 row ) const { return !BitIsSet( validity, row ); }
};

struct SBoolTest
{
    const UInt8* bits;
    bool matchesFalse;
    bool matchesTrue;
    inline bool operator()( UInt32 row ) const { return BitIsSet( bits, row ) ? matchesTrue : matchesFalse; }
};

struct SAlwaysTest
{
    inline bool operator()( UInt32 ) const { return true; }
};

/** The bytewise test of one row of a variable column; the terminator is not part of the value */
template< class TPredicate >
struct TVariableTest
{
    const UInt32* offsets;
    const char* bytes;
    const TPredicate* predicate;
    inline bool operator()( UInt32 row ) const
    {
        const UInt32 begin = offsets[ row ];
        return predicate->MatchesBytes( bytes + begin, offsets[ row + 1 ] - begin - 1 );
    }
};

struct SDictionaryTest
{
    const UInt32* indices;
    const UInt8* entryBits;
    inline bool operator()( UInt32 row ) const { return BitIsSet( entryBits, indices[ row ] ); }
};

}

/*-------------------------------------------------------------------------*/

/** One predicate compiled for one batch column */
struct CDataTableFilter::SCompiledPredicate
{
    UInt32 predicateIndex;              /**< position in the resolved query, for the guaranteed predicates of a batch */
    UInt32 batchColumn;
    TDataTableLogicalType type;
    EFilterFamily family;
    EFilterKind kind;

    Int64 signedLow;                    /**< inclusive bounds; the operand of NOT_EQUALS in the low member */
    Int64 signedHigh;
    UInt64 unsignedLow;
    UInt64 unsignedHigh;
    Float64 floatLow;
    Float64 floatHigh;

    bool hasLow;                        /**< bytes: the bounds and their inclusiveness */
    bool hasHigh;
    bool lowInclusive;
    bool highInclusive;
    GUCEF::vector< char > lowBytes;
    GUCEF::vector< char > highBytes;

    GUCEF::vector< Int64 > signedSet;   /**< IN, sorted */
    GUCEF::vector< UInt64 > unsignedSet;
    GUCEF::vector< Float64 > floatSet;
    GUCEF::vector< char > setBytes;     /**< IN on bytes: the members back to back, sorted */
    GUCEF::vector< UInt32 > setOffsets; /**< member i is setBytes[ setOffsets[ i ], setOffsets[ i + 1 ] ) */

    CDataTableBatchPtr dictionaryBatch; /**< the dictionary the bitmap was made for, held so its address is not reused */
    const CDataTableColumnBatch* dictionary;
    GUCEF::vector< UInt8 > dictionaryBits;
    UInt32 dictionaryMatches;

    SCompiledPredicate( void )
        : predicateIndex( 0 )
        , batchColumn( 0 )
        , type( DATATABLE_TYPE_UNSUPPORTED )
        , family( FILTERFAMILY_NONE )
        , kind( FILTERKIND_NOTHING )
        , signedLow( GUCEF_INT64MIN )
        , signedHigh( GUCEF_INT64MAX )
        , unsignedLow( 0 )
        , unsignedHigh( GUCEF_UINT64MAX )
        , floatLow( -std::numeric_limits< Float64 >::infinity() )
        , floatHigh( std::numeric_limits< Float64 >::infinity() )
        , hasLow( false )
        , hasHigh( false )
        , lowInclusive( true )
        , highInclusive( true )
        , dictionary( GUCEF_NULL )
        , dictionaryMatches( 0 )
    {
    }

    /** Whether a text or binary value passes; the test of variable values and of dictionary entries */
    bool MatchesBytes( const char* bytes, UInt32 length ) const
    {
        switch ( kind )
        {
            case FILTERKIND_NOT_NULL:   { return true; }
            case FILTERKIND_RANGE:
            {
                if ( hasLow )
                {
                    const Int32 order = CompareByteRanges( bytes, length, lowBytes.empty() ? "" : &lowBytes[ 0 ], (UInt32) lowBytes.size() );
                    if ( order < 0 || ( 0 == order && !lowInclusive ) )
                        return false;
                }
                if ( hasHigh )
                {
                    const Int32 order = CompareByteRanges( bytes, length, highBytes.empty() ? "" : &highBytes[ 0 ], (UInt32) highBytes.size() );
                    if ( order > 0 || ( 0 == order && !highInclusive ) )
                        return false;
                }
                return true;
            }
            case FILTERKIND_NOT_EQUALS:
            {
                return 0 != CompareByteRanges( bytes, length, lowBytes.empty() ? "" : &lowBytes[ 0 ], (UInt32) lowBytes.size() );
            }
            case FILTERKIND_IN:
            {
                const UInt32 count = setOffsets.empty() ? 0 : (UInt32) setOffsets.size() - 1;
                const char* base = setBytes.empty() ? "" : &setBytes[ 0 ];
                UInt32 first = 0;
                UInt32 last = count;
                while ( first < last )
                {
                    const UInt32 middle = first + ( ( last - first ) >> 1 );
                    if ( CompareByteRanges( base + setOffsets[ middle ], setOffsets[ middle + 1 ] - setOffsets[ middle ], bytes, length ) < 0 )
                        first = middle + 1;
                    else
                        last = middle;
                }
                return first < count && 0 == CompareByteRanges( base + setOffsets[ first ], setOffsets[ first + 1 ] - setOffsets[ first ], bytes, length );
            }
            default:                    { return false; }
        }
    }

    /** Whether a value of the signed family passes; used for the two values of a boolean */
    bool MatchesSigned( Int64 value ) const
    {
        switch ( kind )
        {
            case FILTERKIND_NOT_NULL:   { return true; }
            case FILTERKIND_RANGE:      { return value >= signedLow && value <= signedHigh; }
            case FILTERKIND_NOT_EQUALS: { return value != signedLow; }
            case FILTERKIND_IN:
            {
                for ( UInt32 i=0; i<(UInt32) signedSet.size(); ++i )
                    if ( signedSet[ i ] == value )
                        return true;
                return false;
            }
            default:                    { return false; }
        }
    }
};

/*-------------------------------------------------------------------------*/

namespace {

void
CopyBytes( const CVariant& value, GUCEF::vector< char >& bytes )
{
    UInt32 length = 0;
    const char* data = CDataTablePruner::GetNativeBytes( value, length );
    bytes.clear();
    if ( length > 0 && GUCEF_NULL != data )
        bytes.assign( data, data + length );
}

/** Runs the compiled range / not equals / in test of a fixed width column of value type T in the comparison type C */
template< typename T, typename C >
UInt32
SelectFixed( const T* values, const UInt8* validity, EFilterKind kind, C low, C high, const GUCEF::vector< C >& set,
             const UInt32* candidates, UInt32 count, UInt32* out )
{
    switch ( kind )
    {
        case FILTERKIND_RANGE:
        {
            TRangeTest< T, C > test;
            test.values = values;
            test.low = low;
            test.high = high;
            return SelectRows( test, validity, candidates, count, out );
        }
        case FILTERKIND_NOT_EQUALS:
        {
            TNotEqualTest< T, C > test;
            test.values = values;
            test.operand = low;
            return SelectRows( test, validity, candidates, count, out );
        }
        case FILTERKIND_IN:
        {
            TInTest< T, C > test;
            test.values = values;
            test.set = set.empty() ? GUCEF_NULL : &set[ 0 ];
            test.setSize = (UInt32) set.size();
            return SelectRows( test, validity, candidates, count, out );
        }
        default:
        {
            return 0;
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableFilter::CDataTableFilter( void )
    : m_predicates()
    , m_matchesNothing( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableFilter::~CDataTableFilter()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CDataTableFilter::Clear( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_predicates.size(); ++i )
        GUCEF_DELETE m_predicates[ i ];
    m_predicates.clear();
    m_matchesNothing = false;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableFilter::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_predicates.empty() && !m_matchesNothing;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableFilter::MatchesNothing( void ) const
{GUCEF_TRACE;

    return m_matchesNothing;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableFilter::Prepare( const CDataTableResolvedQuery& query                    ,
                           const CDataTableSchema::TColumnIndexVector& batchColumns ,
                           CString& error                                           )
{GUCEF_TRACE;

    Clear();
    m_matchesNothing = query.matchesNothing;
    if ( query.schema.IsNULL() )
    {
        if ( query.predicates.empty() )
            return DATATABLE_STATUS_OK;
        error = "The resolved query has no schema";
        return DATATABLE_STATUS_INVALID_ARGUMENT;
    }

    for ( UInt32 p=0; p<(UInt32) query.predicates.size(); ++p )
    {
        const CDataTableResolvedPredicate& predicate = query.predicates[ p ];
        if ( predicate.columnIndex >= query.schema->GetColumnCount() )
        {
            error = "A predicate refers to column " + ToString( predicate.columnIndex ) + ", which the schema does not have";
            return DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        UInt32 batchColumn = GUCEF_UINT32MAX;
        for ( UInt32 c=0; c<(UInt32) batchColumns.size(); ++c )
        {
            if ( batchColumns[ c ] == predicate.columnIndex )
            {
                batchColumn = c;
                break;
            }
        }
        const CDataTableColumnDef& column = query.schema->GetColumn( predicate.columnIndex );
        if ( GUCEF_UINT32MAX == batchColumn )
        {
            error = "Column \"" + column.name + "\" of a predicate is not among the columns of the batches";
            return DATATABLE_STATUS_INVALID_ARGUMENT;
        }

        SCompiledPredicate* compiled = GUCEF_NEW SCompiledPredicate();
        m_predicates.push_back( compiled );
        compiled->predicateIndex = p;
        compiled->batchColumn = batchColumn;
        compiled->type = column.type;
        compiled->family = FilterFamilyOf( column.type );

        if ( predicate.matchesNothing )
        {
            compiled->kind = FILTERKIND_NOTHING;
            m_matchesNothing = true;
            continue;
        }
        if ( predicate.matchesAllNonNull || DATATABLE_PREDICATE_IS_NOT_NULL == predicate.op )
        {
            compiled->kind = FILTERKIND_NOT_NULL;
            continue;
        }
        if ( DATATABLE_PREDICATE_IS_NULL == predicate.op )
        {
            compiled->kind = FILTERKIND_IS_NULL;
            continue;
        }

        const EFilterFamily family = compiled->family;
        switch ( predicate.op )
        {
            case DATATABLE_PREDICATE_EQUALS:
            {
                compiled->kind = FILTERKIND_RANGE;
                compiled->signedLow = compiled->signedHigh = predicate.low.AsInt64();
                compiled->unsignedLow = compiled->unsignedHigh = predicate.low.AsUInt64();
                compiled->floatLow = compiled->floatHigh = predicate.low.AsFloat64();
                compiled->hasLow = compiled->hasHigh = true;
                compiled->lowInclusive = compiled->highInclusive = true;
                if ( FILTERFAMILY_BYTES == family )
                {
                    CopyBytes( predicate.low, compiled->lowBytes );
                    compiled->highBytes = compiled->lowBytes;
                }
                break;
            }
            case DATATABLE_PREDICATE_NOT_EQUALS:
            {
                compiled->kind = FILTERKIND_NOT_EQUALS;
                compiled->signedLow = predicate.low.AsInt64();
                compiled->unsignedLow = predicate.low.AsUInt64();
                compiled->floatLow = predicate.low.AsFloat64();
                if ( FILTERFAMILY_BYTES == family )
                    CopyBytes( predicate.low, compiled->lowBytes );
                break;
            }
            case DATATABLE_PREDICATE_LESS:
            case DATATABLE_PREDICATE_LESS_OR_EQUAL:
            case DATATABLE_PREDICATE_GREATER:
            case DATATABLE_PREDICATE_GREATER_OR_EQUAL:
            case DATATABLE_PREDICATE_RANGE:
            {
                compiled->kind = FILTERKIND_RANGE;
                compiled->hasLow = predicate.hasLow;
                compiled->hasHigh = predicate.hasHigh;
                compiled->lowInclusive = predicate.lowInclusive;
                compiled->highInclusive = predicate.highInclusive;
                if ( predicate.hasLow )
                {
                    // exclusive bounds of the discrete families become inclusive ones; floats use the next float64
                    compiled->signedLow = predicate.low.AsInt64();
                    compiled->unsignedLow = predicate.low.AsUInt64();
                    compiled->floatLow = predicate.low.AsFloat64();
                    if ( !predicate.lowInclusive )
                    {
                        if ( FILTERFAMILY_SIGNED == family )
                        {
                            if ( GUCEF_INT64MAX == compiled->signedLow )
                                compiled->kind = FILTERKIND_NOTHING;
                            else
                                ++compiled->signedLow;
                        }
                        else
                        if ( FILTERFAMILY_UNSIGNED == family )
                        {
                            if ( GUCEF_UINT64MAX == compiled->unsignedLow )
                                compiled->kind = FILTERKIND_NOTHING;
                            else
                                ++compiled->unsignedLow;
                        }
                        else
                        if ( FILTERFAMILY_FLOAT == family )
                        {
                            compiled->floatLow = NextFloat64Toward( compiled->floatLow, true );
                        }
                    }
                    if ( FILTERFAMILY_BYTES == family )
                        CopyBytes( predicate.low, compiled->lowBytes );
                }
                if ( predicate.hasHigh )
                {
                    compiled->signedHigh = predicate.high.AsInt64();
                    compiled->unsignedHigh = predicate.high.AsUInt64();
                    compiled->floatHigh = predicate.high.AsFloat64();
                    if ( !predicate.highInclusive )
                    {
                        if ( FILTERFAMILY_SIGNED == family )
                        {
                            if ( GUCEF_INT64MIN == compiled->signedHigh )
                                compiled->kind = FILTERKIND_NOTHING;
                            else
                                --compiled->signedHigh;
                        }
                        else
                        if ( FILTERFAMILY_UNSIGNED == family )
                        {
                            if ( 0 == compiled->unsignedHigh )
                                compiled->kind = FILTERKIND_NOTHING;
                            else
                                --compiled->unsignedHigh;
                        }
                        else
                        if ( FILTERFAMILY_FLOAT == family )
                        {
                            compiled->floatHigh = NextFloat64Toward( compiled->floatHigh, false );
                        }
                    }
                    if ( FILTERFAMILY_BYTES == family )
                        CopyBytes( predicate.high, compiled->highBytes );
                }
                if ( FILTERKIND_NOTHING == compiled->kind )
                    m_matchesNothing = true;
                break;
            }
            case DATATABLE_PREDICATE_IN:
            {
                compiled->kind = FILTERKIND_IN;
                for ( UInt32 v=0; v<(UInt32) predicate.values.size(); ++v )
                {
                    const CVariant& value = predicate.values[ v ];
                    switch ( family )
                    {
                        case FILTERFAMILY_SIGNED:   { compiled->signedSet.push_back( value.AsInt64() ); break; }
                        case FILTERFAMILY_UNSIGNED: { compiled->unsignedSet.push_back( value.AsUInt64() ); break; }
                        case FILTERFAMILY_FLOAT:
                        {
                            const Float64 member = value.AsFloat64();
                            if ( member == member )
                                compiled->floatSet.push_back( member );
                            break;
                        }
                        case FILTERFAMILY_BYTES:
                        {
                            if ( compiled->setOffsets.empty() )
                                compiled->setOffsets.push_back( 0 );
                            UInt32 length = 0;
                            const char* bytes = CDataTablePruner::GetNativeBytes( value, length );
                            if ( length > 0 && GUCEF_NULL != bytes )
                                compiled->setBytes.insert( compiled->setBytes.end(), bytes, bytes + length );
                            compiled->setOffsets.push_back( (UInt32) compiled->setBytes.size() );
                            break;
                        }
                        default:                    { break; }
                    }
                }
                break;
            }
            default:
            {
                error = "Predicate on column \"" + column.name + "\" has an operator the filter does not know";
                return DATATABLE_STATUS_INVALID_ARGUMENT;
            }
        }
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CDataTableFilter::Apply( CDataTableBatch& batch, CDataTableBufferPool& pool, UInt32& selectedRowCount )
{GUCEF_TRACE;

    const UInt32 rowCount = batch.GetRowCount();
    selectedRowCount = batch.GetSelectedRowCount();
    if ( m_matchesNothing )
    {
        selectedRowCount = 0;
        return DATATABLE_STATUS_OK;
    }
    if ( m_predicates.empty() || 0 == selectedRowCount )
        return DATATABLE_STATUS_OK;

    const UInt64 guaranteed = batch.GetGuaranteedPredicates();
    const UInt32* candidates = batch.GetSelection();
    UInt32 count = GUCEF_NULL == candidates ? rowCount : selectedRowCount;
    CDataTableBufferPtr selectionBuffer;
    UInt32* selection = GUCEF_NULL;

    for ( UInt32 p=0; p<(UInt32) m_predicates.size() && count > 0; ++p )
    {
        SCompiledPredicate& compiled = *m_predicates[ p ];
        if ( compiled.predicateIndex < 64 && 0 != ( guaranteed & ( (UInt64) 1 << compiled.predicateIndex ) ) )
            continue;
        if ( FILTERKIND_NOTHING == compiled.kind )
        {
            count = 0;
            break;
        }

        const CDataTableColumnBatch& column = batch.GetColumn( compiled.batchColumn );
        const UInt8* validity = column.GetValidity();
        if ( FILTERKIND_NOT_NULL == compiled.kind && GUCEF_NULL == validity )
            continue;
        if ( FILTERKIND_IS_NULL == compiled.kind && GUCEF_NULL == validity )
        {
            count = 0;
            break;
        }
        if ( column.GetRowCount() < rowCount )
            return DATATABLE_STATUS_SCHEMA_MISMATCH;

        if ( GUCEF_NULL == selection )
        {
            TDataTableStatus status = pool.Acquire( rowCount * (UInt32) sizeof( UInt32 ), selectionBuffer );
            if ( DATATABLE_STATUS_OK != status )
                return status;
            selection = static_cast< UInt32* >( selectionBuffer->GetData() );
        }

        UInt32 selected = 0;
        if ( FILTERKIND_IS_NULL == compiled.kind )
        {
            SNullTest test;
            test.validity = validity;
            selected = SelectRows( test, GUCEF_NULL, candidates, count, selection );
        }
        else
        if ( FILTERKIND_NOT_NULL == compiled.kind )
        {
            // the valid rows: the validity check of SelectRows with a test that always holds
            SAlwaysTest always;
            selected = SelectRows( always, validity, candidates, count, selection );
        }
        else
        {
            switch ( column.GetStorage() )
            {
                case DATATABLE_STORAGE_BITS:
                {
                    SBoolTest test;
                    test.bits = column.GetBoolBits();
                    test.matchesFalse = compiled.MatchesSigned( 0 );
                    test.matchesTrue = compiled.MatchesSigned( 1 );
                    selected = SelectRows( test, validity, candidates, count, selection );
                    break;
                }
                case DATATABLE_STORAGE_FIXED:
                {
                    const EFilterKind kind = compiled.kind;
                    switch ( compiled.type )
                    {
                        case DATATABLE_TYPE_INT8:       { selected = SelectFixed< Int8, Int64 >( column.GetInt8Data(), validity, kind, compiled.signedLow, compiled.signedHigh, compiled.signedSet, candidates, count, selection ); break; }
                        case DATATABLE_TYPE_INT16:      { selected = SelectFixed< Int16, Int64 >( column.GetInt16Data(), validity, kind, compiled.signedLow, compiled.signedHigh, compiled.signedSet, candidates, count, selection ); break; }
                        case DATATABLE_TYPE_INT32:
                        case DATATABLE_TYPE_DATE:       { selected = SelectFixed< Int32, Int64 >( column.GetInt32Data(), validity, kind, compiled.signedLow, compiled.signedHigh, compiled.signedSet, candidates, count, selection ); break; }
                        case DATATABLE_TYPE_INT64:
                        case DATATABLE_TYPE_DECIMAL:
                        case DATATABLE_TYPE_TIMESTAMP:  { selected = SelectFixed< Int64, Int64 >( column.GetInt64Data(), validity, kind, compiled.signedLow, compiled.signedHigh, compiled.signedSet, candidates, count, selection ); break; }
                        case DATATABLE_TYPE_UINT8:      { selected = SelectFixed< UInt8, UInt64 >( column.GetUInt8Data(), validity, kind, compiled.unsignedLow, compiled.unsignedHigh, compiled.unsignedSet, candidates, count, selection ); break; }
                        case DATATABLE_TYPE_UINT16:     { selected = SelectFixed< UInt16, UInt64 >( column.GetUInt16Data(), validity, kind, compiled.unsignedLow, compiled.unsignedHigh, compiled.unsignedSet, candidates, count, selection ); break; }
                        case DATATABLE_TYPE_UINT32:     { selected = SelectFixed< UInt32, UInt64 >( column.GetUInt32Data(), validity, kind, compiled.unsignedLow, compiled.unsignedHigh, compiled.unsignedSet, candidates, count, selection ); break; }
                        case DATATABLE_TYPE_UINT64:     { selected = SelectFixed< UInt64, UInt64 >( column.GetUInt64Data(), validity, kind, compiled.unsignedLow, compiled.unsignedHigh, compiled.unsignedSet, candidates, count, selection ); break; }
                        case DATATABLE_TYPE_FLOAT32:    { selected = SelectFixed< Float32, Float64 >( column.GetFloat32Data(), validity, kind, compiled.floatLow, compiled.floatHigh, compiled.floatSet, candidates, count, selection ); break; }
                        case DATATABLE_TYPE_FLOAT64:    { selected = SelectFixed< Float64, Float64 >( column.GetFloat64Data(), validity, kind, compiled.floatLow, compiled.floatHigh, compiled.floatSet, candidates, count, selection ); break; }
                        default:                        { return DATATABLE_STATUS_SCHEMA_MISMATCH; }
                    }
                    if ( GUCEF_NULL == column.GetFixedData() )
                        return DATATABLE_STATUS_SCHEMA_MISMATCH;
                    break;
                }
                case DATATABLE_STORAGE_VARIABLE:
                {
                    if ( FILTERFAMILY_BYTES != compiled.family )
                        return DATATABLE_STATUS_SCHEMA_MISMATCH;
                    TVariableTest< SCompiledPredicate > test;
                    test.offsets = column.GetOffsets();
                    test.bytes = column.GetBytes();
                    test.predicate = &compiled;
                    selected = SelectRows( test, validity, candidates, count, selection );
                    break;
                }
                case DATATABLE_STORAGE_DICTIONARY:
                {
                    if ( FILTERFAMILY_BYTES != compiled.family )
                        return DATATABLE_STATUS_SCHEMA_MISMATCH;
                    const CDataTableColumnBatch* dictionary = column.GetDictionary();
                    if ( GUCEF_NULL == dictionary )
                        return DATATABLE_STATUS_SCHEMA_MISMATCH;
                    if ( dictionary != compiled.dictionary || column.GetDictionaryBatch() != compiled.dictionaryBatch )
                    {
                        // one test per dictionary entry, kept for the batches sharing the dictionary
                        const UInt32 entries = dictionary->GetRowCount();
                        compiled.dictionaryBits.assign( ( entries + 7 ) / 8 + 1, 0 );
                        compiled.dictionaryMatches = 0;
                        for ( UInt32 e=0; e<entries; ++e )
                        {
                            UInt32 length = 0;
                            const char* bytes = dictionary->GetVariableValue( e, length );
                            if ( compiled.MatchesBytes( bytes, length ) )
                            {
                                compiled.dictionaryBits[ e >> 3 ] = (UInt8) ( compiled.dictionaryBits[ e >> 3 ] | ( 1u << ( e & 7u ) ) );
                                ++compiled.dictionaryMatches;
                            }
                        }
                        compiled.dictionary = dictionary;
                        compiled.dictionaryBatch = column.GetDictionaryBatch();
                    }
                    if ( 0 == compiled.dictionaryMatches )
                    {
                        selected = 0;
                        break;
                    }
                    SDictionaryTest test;
                    test.indices = column.GetDictionaryIndices();
                    test.entryBits = &compiled.dictionaryBits[ 0 ];
                    selected = SelectRows( test, validity, candidates, count, selection );
                    break;
                }
                default:
                {
                    if ( rowCount > 0 )
                        return DATATABLE_STATUS_SCHEMA_MISMATCH;
                    break;
                }
            }
        }
        count = selected;
        candidates = selection;
    }

    if ( GUCEF_NULL == selection )
    {
        // nothing narrowed the rows, or a predicate dropped them all before a selection was made
        if ( 0 == count )
            selectedRowCount = 0;
        return DATATABLE_STATUS_OK;
    }

    selectedRowCount = count;
    if ( count == rowCount )
    {
        // every row matches: no selection at all, the fast path for consumers
        batch.SetSelection( GUCEF_NULL, 0, CDataTableBufferPtr() );
        return DATATABLE_STATUS_OK;
    }
    selectionBuffer->SetDataSize( count * (UInt32) sizeof( UInt32 ) );
    batch.SetSelection( selection, count, selectionBuffer );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
