/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
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

#ifndef GUCEF_GEOMETRY_TESTAPP_TESTSPATIALREFERENCE_H
#define GUCEF_GEOMETRY_TESTAPP_TESTSPATIALREFERENCE_H

/*
 *  Helpers of the spatial structure tests of the gucefGEOMETRY test application: visitors that collect handles, the
 *  comparison of two handle sets, and the exact value of a Float64 or integer as a fraction for the references that must
 *  not share code with the predicates.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_MATH_FRACTIONTYPES_H
#include "gucefMATH_fractiontypes.h"
#define GUCEF_MATH_FRACTIONTYPES_H
#endif /* GUCEF_MATH_FRACTIONTYPES_H ? */

#include "TestQueryReference.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace SpatialReference {

/** Collects the handles that a query visits (and the ray parameters of the ray queries); stops after 'limit' when set */
class CCollector
{
    public:

    GUCEF::vector< GUCEF::UInt32 > handles;
    GUCEF::vector< GUCEF::Float64 > parameters;
    GUCEF::UInt32 limit;

    CCollector( void ) : handles(), parameters(), limit( 0xFFFFFFFFU ) {}

    bool Visit( GUCEF::UInt32 handle, const GUCEF::UInt32& /* item */ )
    {
        handles.push_back( handle );
        return handles.size() < limit;
    }

    template < typename T >
    bool Visit( GUCEF::UInt32 handle, const GUCEF::UInt32& /* item */, const T& parameter )
    {
        handles.push_back( handle );
        parameters.push_back( (GUCEF::Float64) parameter );
        return handles.size() < limit;
    }
};

/** True when the two lists hold the same handles (each handle once) */
inline bool
SameSet( GUCEF::vector< GUCEF::UInt32 > a, GUCEF::vector< GUCEF::UInt32 > b )
{
    std::sort( a.begin(), a.end() );
    std::sort( b.begin(), b.end() );
    return a == b;
}

/** True when no handle is in the list twice */
inline bool
NoDuplicates( GUCEF::vector< GUCEF::UInt32 > a )
{
    std::sort( a.begin(), a.end() );
    return std::adjacent_find( a.begin(), a.end() ) == a.end();
}

/** The exact value of a Float64 as a fraction */
inline GUCEF::MATH::TMathBigFraction
ExactFraction( GUCEF::Float64 value )
{
    GUCEF::MATH::TMathBigFraction result( 0, 1 );
    GUCEF::MATH::CTNumericTraits< GUCEF::MATH::TMathBigFraction >::FromFloat64( value, GUCEF::CORE::ROUNDINGMODE_UNNECESSARY, result );
    return result;
}

inline GUCEF::MATH::TMathBigFraction
ExactFraction( GUCEF::Int64 value )
{
    GUCEF::CORE::CBigInteger n;
    GUCEF::CORE::CBigInteger::FromInt64( value, n );
    return GUCEF::MATH::TMathBigFraction( n, GUCEF::CORE::CBigInteger::One() );
}

inline GUCEF::MATH::TMathBigFraction ExactFraction( GUCEF::Int32 value ) { return ExactFraction( (GUCEF::Int64) value ); }
inline GUCEF::MATH::TMathBigFraction ExactFraction( GUCEF::Float32 value ) { return ExactFraction( (GUCEF::Float64) value ); }

template < typename TStorage, GUCEF::UInt32 FractionBits >
inline GUCEF::MATH::TMathBigFraction
ExactFraction( const GUCEF::MATH::CTFixedPoint< TStorage, FractionBits >& value )
{
    GUCEF::CORE::CBigInteger n;
    GUCEF::CORE::CBigInteger::FromInt64( (GUCEF::Int64) value.GetRaw(), n );
    GUCEF::CORE::CBigInteger d = GUCEF::CORE::CBigInteger::One();
    d <<= FractionBits;
    return GUCEF::MATH::TMathBigFraction( n, d );
}

inline GUCEF::Int32
SignOfFraction( const GUCEF::MATH::TMathBigFraction& value )
{
    return GUCEF::MATH::CTNumericTraits< GUCEF::MATH::TMathBigFraction >::IsNegative( value ) ? -1 : ( value == GUCEF::MATH::TMathBigFraction( 0, 1 ) ? 0 : 1 );
}

} /* namespace SpatialReference */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_TESTAPP_TESTSPATIALREFERENCE_H ? */
