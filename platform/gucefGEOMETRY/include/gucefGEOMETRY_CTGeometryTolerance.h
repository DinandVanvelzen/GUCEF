/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
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

#ifndef GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H
#define GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

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
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The tolerances of the geometric queries (section 7.10 of the design). Every query that compares lengths takes one
 *  (defaulted), so that results do not depend on the scale of the scene:
 *
 *  - absolute: the smallest length that counts as non-zero, in the units of the scene;
 *  - relative: a fraction of the magnitude of the operands (their extent, or the distance from the origin) that
 *    counts as zero, so that the tolerance grows with the coordinates and a shape 10^4 units from the origin is not
 *    held to the precision of one at the origin;
 *  - angular: the smallest sine of an angle between two directions that is not treated as parallel.
 *
 *  GetLengthTolerance( scale ) gives the length below which two points are the same for operands of that magnitude:
 *  the larger of the absolute tolerance and the relative tolerance times the scale.
 *
 *  - Kind: any numeric kind; the defaults come from CTNumericTraits (for Float32 1e-6 absolute, 1e-5 relative and
 *    1e-6 angular; for Float64 1e-12, 1e-10 and 1e-12; zero for the exact kinds, which then compare exactly).
 *  - Layout: three values of the kind, 3 * sizeof( T ), no virtual functions.
 *  - Realtime: every member is realtime-safe.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGeometryTolerance
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;

    T absolute;
    T relative;
    T angular;

    /** The defaults of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE CTGeometryTolerance( void )
        : absolute( TTraits::DefaultAbsoluteTolerance() )
        , relative( TTraits::DefaultRelativeTolerance() )
        , angular( TTraits::DefaultAbsoluteTolerance() )
    {
    }

    GUCEF_GEOMETRY_FORCE_INLINE CTGeometryTolerance( const T& absoluteTolerance, const T& relativeTolerance, const T& angularTolerance )
        : absolute( absoluteTolerance ), relative( relativeTolerance ), angular( angularTolerance )
    {
    }

    /** The defaults of the kind: the same as the default constructed tolerance */
    static GUCEF_GEOMETRY_FORCE_INLINE CTGeometryTolerance Default( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return CTGeometryTolerance(); }

    /** The largest of the absolute tolerance and the relative tolerance times the (non-negative) magnitude of the operands */
    GUCEF_GEOMETRY_FORCE_INLINE T GetLengthTolerance( const T& scale ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        const T scaled = relative * TTraits::Abs( scale );
        return scaled < absolute ? absolute : scaled;
    }

    /** True when |length| is within the tolerance for operands of the given magnitude */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsZeroLength( const T& length, const T& scale ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return !( GetLengthTolerance( scale ) < TTraits::Abs( length ) );
    }

    /** True when the two lengths differ by no more than the tolerance for operands of the given magnitude */
    GUCEF_GEOMETRY_FORCE_INLINE bool LengthsEqual( const T& a, const T& b, const T& scale ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return IsZeroLength( a - b, scale );
    }

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTGeometryTolerance& a, const CTGeometryTolerance& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return a.absolute == b.absolute && a.relative == b.relative && a.angular == b.angular;
    }

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTGeometryTolerance& a, const CTGeometryTolerance& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return !( a == b );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTGeometryTolerance< Float32 > TFloat32GeometryTolerance;
typedef CTGeometryTolerance< Float64 > TFloat64GeometryTolerance;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H ? */
