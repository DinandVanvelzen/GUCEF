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

#ifndef GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H
#define GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H

/*
 *  Helpers of the Transformed / TryTransformed members of the shapes (section 7.2 of the design): questions about the
 *  linear part of a CTMatrix4D. Header-only templates, used inside the exported members, so they are compiled in the
 *  module for the predefined kinds.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_MATH_CTMATRIX4D_H
#include "gucefMATH_CTMatrix4D.h"
#define GUCEF_MATH_CTMATRIX4D_H
#endif /* GUCEF_MATH_CTMATRIX4D_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The largest length of the three columns of the linear part: the largest factor by which the matrix stretches a direction along an axis */
template < typename T >
T
MaxAxisScale( const MATH::CTMatrix4D< T >& m ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    const MATH::CTMatrix3D< T > linear = m.GetLinear();
    return TTraits::Max( TTraits::Max( linear.GetColumn( 0 ).GetLength(), linear.GetColumn( 1 ).GetLength() ), linear.GetColumn( 2 ).GetLength() );
}

/*-------------------------------------------------------------------------*/

/**
 *  True when the linear part is a rotation (or reflection) times one scale factor, which is returned in 'scale': the
 *  columns have the same length and are orthogonal within 'tolerance' (relative to the squared length). Spheres and
 *  capsules stay spheres and capsules under such matrices, and only then.
 */
template < typename T >
bool
IsSimilarity( const MATH::CTMatrix4D< T >& m, const T& tolerance, T& scale ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector3D< T > TVector;
    if ( !m.IsAffine() || !m.HasUniformScale( tolerance ) )
        return false;
    const MATH::CTMatrix3D< T > linear = m.GetLinear();
    const TVector c0 = linear.GetColumn( 0 );
    const TVector c1 = linear.GetColumn( 1 );
    const TVector c2 = linear.GetColumn( 2 );
    scale = c0.GetLength();
    if ( !( TTraits::Zero() < scale ) )
        return false;
    const T allowed = tolerance * scale * scale;
    return !( allowed < TTraits::Abs( c0.Dot( c1 ) ) ) && !( allowed < TTraits::Abs( c0.Dot( c2 ) ) ) && !( allowed < TTraits::Abs( c1.Dot( c2 ) ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H ? */
