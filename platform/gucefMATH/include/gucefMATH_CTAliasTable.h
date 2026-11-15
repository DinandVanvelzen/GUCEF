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

#ifndef GUCEF_MATH_CTALIASTABLE_H
#define GUCEF_MATH_CTALIASTABLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#include "gucefMATH_CTNumericTraits.h"
#define GUCEF_MATH_CTNUMERICTRAITS_H
#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Walker / Vose alias table (Vose 1991): draws an index with probability proportional to its weight in O( 1 ) time
 *  per draw after an O( n ) construction. For repeated draws from the same weights; CTRandomStream::Pick is the
 *  linear scan for a single draw.
 *
 *  - T is Float32 or Float64 (predefined kinds are exported instantiations compiled without floating-point contraction).
 *  - Initialize() allocates (the two tables are GUCEF::vector members): not realtime-safe. Sample() reads only, is
 *    realtime-safe, and const: a table may be shared between threads once built.
 *  - Negative and NaN weights count as 0. A table over zero indices or zero total weight is not valid and keeps its
 *    previous content.
 *  - A draw needs one column and one acceptance number: Sample( u1, u2 ) takes them from the caller (uniform in
 *    [0, 1)), Sample( stream ) draws them from a random stream.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTAliasTable
{
    public:

    typedef T TValue;

    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, Alias_tables_require_a_floating_point_kind );

    /** An empty table: IsValid() is false */
    CTAliasTable( void ) {}

    /** Builds the table of 'weights'; check IsValid() */
    CTAliasTable( const T* weights, UInt32 count ) { Initialize( weights, count ); }

    /** Builds the table of 'weights'; false (table unchanged) when count is 0 or the weights have no positive finite sum */
    bool Initialize( const T* weights, UInt32 count );

    /** The number of indices the table draws from; 0 for an empty table */
    GUCEF_MATH_FORCE_INLINE UInt32 GetCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return (UInt32) m_acceptance.size(); }

    GUCEF_MATH_FORCE_INLINE bool IsValid( void ) const GUCEF_MATH_REALTIME_CONTEXT { return !m_acceptance.empty(); }

    /** The probability with which column 'index' keeps its own index; 0 outside the table */
    GUCEF_MATH_FORCE_INLINE T GetAcceptance( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT { return index < GetCount() ? m_acceptance[ index ] : T( 0 ); }

    /** The index that column 'index' yields otherwise; 0 outside the table */
    GUCEF_MATH_FORCE_INLINE UInt32 GetAlias( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT { return index < GetCount() ? m_alias[ index ] : 0; }

    /** The index drawn for the uniform numbers u1 (column) and u2 (acceptance), both in [0, 1); 0 for an empty table */
    GUCEF_MATH_FORCE_INLINE UInt32 Sample( const T& u1, const T& u2 ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        const UInt32 count = GetCount();
        if ( count == 0 )
            return 0;
        UInt32 column = u1 > T( 0 ) ? ( u1 < T( 1 ) ? (UInt32) ( u1 * (T) count ) : count - 1 ) : 0;
        if ( column >= count )
            column = count - 1;     // u1 * count can round up to count
        return u2 < m_acceptance[ column ] ? column : m_alias[ column ];
    }

    /** Draws with two numbers of the random stream ( CTRandomStream of any generator ) */
    template < typename TStream >
    GUCEF_MATH_FORCE_INLINE UInt32 Sample( TStream& stream ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        const UInt32 count = GetCount();
        if ( count == 0 )
            return 0;
        const UInt32 column = stream.Below( count );
        return DrawUnit( stream, (const T*) GUCEF_NULL ) < m_acceptance[ column ] ? column : m_alias[ column ];
    }

    private:

    template < typename TStream > static GUCEF_MATH_FORCE_INLINE Float32 DrawUnit( TStream& stream, const Float32* ) { return stream.NextFloat32(); }
    template < typename TStream > static GUCEF_MATH_FORCE_INLINE Float64 DrawUnit( TStream& stream, const Float64* ) { return stream.NextFloat64(); }

    GUCEF::vector< T > m_acceptance;
    GUCEF::vector< UInt32 > m_alias;
};

/*-------------------------------------------------------------------------*/

typedef CTAliasTable< Float32 > TFloat32AliasTable;
typedef CTAliasTable< Float64 > TFloat64AliasTable;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTAliasTable< T >::Initialize( const T* weights, UInt32 count )
{
    if ( count == 0 || GUCEF_NULL == weights )
        return false;

    Float64 total = 0.0;
    for ( UInt32 i=0; i<count; ++i )
        if ( weights[ i ] > T( 0 ) )
            total = total + (Float64) weights[ i ];
    if ( !( total > 0.0 ) || total - total != 0.0 )
        return false;

    // probabilities scaled so that the average is 1: indices below 1 are filled up from indices above 1
    GUCEF::vector< Float64 > scaled;
    GUCEF::vector< UInt32 > small;
    GUCEF::vector< UInt32 > large;
    scaled.resize( count );
    small.reserve( count );
    large.reserve( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        scaled[ i ] = weights[ i ] > T( 0 ) ? (Float64) weights[ i ] * (Float64) count / total : 0.0;
        if ( scaled[ i ] < 1.0 )
            small.push_back( i );
        else
            large.push_back( i );
    }

    GUCEF::vector< T > acceptance;
    GUCEF::vector< UInt32 > alias;
    acceptance.resize( count, T( 1 ) );
    alias.resize( count );
    for ( UInt32 i=0; i<count; ++i )
        alias[ i ] = i;

    while ( !small.empty() && !large.empty() )
    {
        const UInt32 less = small.back();
        small.pop_back();
        const UInt32 more = large.back();
        large.pop_back();
        acceptance[ less ] = (T) scaled[ less ];
        alias[ less ] = more;
        scaled[ more ] = ( scaled[ more ] + scaled[ less ] ) - 1.0;
        if ( scaled[ more ] < 1.0 )
            small.push_back( more );
        else
            large.push_back( more );
    }
    // what is left has probability 1 up to rounding: the column always keeps its index
    for ( size_t i=0; i<large.size(); ++i )
        acceptance[ large[ i ] ] = T( 1 );
    for ( size_t i=0; i<small.size(); ++i )
        acceptance[ small[ i ] ] = T( 1 );

    m_acceptance.swap( acceptance );
    m_alias.swap( alias );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_CTALIASTABLE_MEMBERS( prefix, kind ) \
    prefix bool CTAliasTable< kind >::Initialize( const kind*, UInt32 );

#define GUCEF_MATH_CTALIASTABLE_PREDEFINED( prefix ) \
    GUCEF_MATH_CTALIASTABLE_MEMBERS( prefix, Float32 ) \
    GUCEF_MATH_CTALIASTABLE_MEMBERS( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
GUCEF_MATH_CTALIASTABLE_PREDEFINED( GUCEF_MATH_EXTERN_TEMPLATE )
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTALIASTABLE_H ? */
