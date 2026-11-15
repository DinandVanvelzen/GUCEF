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

#ifndef GUCEF_MATH_PROJECTIONFUNCTIONS_H
#define GUCEF_MATH_PROJECTIONFUNCTIONS_H

/*
 *  Screen projection helpers (section 4.5.7 of the design): world point to screen point and back, and the picking ray of a
 *  screen point. They work with any projection convention of SProjectionConvention because the matrices carry it; the
 *  viewport (CTViewport) maps normalised device coordinates to screen space, with the origin at the top left and Y down.
 *  T is a floating point kind. All functions are realtime-safe: no allocation, no locks, no tracing. They go through
 *  CTMatrix4D::TryTransformPointProjective, so points behind the camera and points at infinity are reported and never
 *  silently divided.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTMATRIX4D_H
#include "gucefMATH_CTMatrix4D.h"
#define GUCEF_MATH_CTMATRIX4D_H
#endif /* GUCEF_MATH_CTMATRIX4D_H ? */

#ifndef GUCEF_MATH_CTPOINT2D_H
#include "gucefMATH_CTPoint2D.h"
#define GUCEF_MATH_CTPOINT2D_H
#endif /* GUCEF_MATH_CTPOINT2D_H ? */

#ifndef GUCEF_MATH_CTPOINT3D_H
#include "gucefMATH_CTPoint3D.h"
#define GUCEF_MATH_CTPOINT3D_H
#endif /* GUCEF_MATH_CTPOINT3D_H ? */

#ifndef GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#include "gucefMATH_CTDirectionVector3D.h"
#define GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#endif /* GUCEF_MATH_CTDIRECTIONVECTOR3D_H ? */

#ifndef GUCEF_MATH_SPROJECTIONCONVENTION_H
#include "gucefMATH_SProjectionConvention.h"
#define GUCEF_MATH_SPROJECTIONCONVENTION_H
#endif /* GUCEF_MATH_SPROJECTIONCONVENTION_H ? */

#ifndef GUCEF_MATH_CTVIEWPORT_H
#include "gucefMATH_CTViewport.h"
#define GUCEF_MATH_CTVIEWPORT_H
#endif /* GUCEF_MATH_CTVIEWPORT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The screen position and depth of a world point. 'viewProjection' is projection * view of the camera; 'screenPoint'
 *  receives the position in the screen space of the viewport (top left origin, Y down) and 'depth' the normalised device
 *  depth in the depth range of the projection convention (reversed when the convention reverses it).
 *  False, outputs unchanged, when the point is not in front of the camera (the homogeneous w is not positive), when it
 *  maps to infinity (the point lies on the camera plane) or when the viewport is not valid. Points in front of the
 *  camera but outside the view volume still project: test the screen position and depth for visibility.
 */
template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
Project( const CTPoint3D< T >& worldPoint, const CTMatrix4D< T >& viewProjection, const CTViewport< T >& viewport,
         CTPoint2D< T >& screenPoint, T& depth ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTNumericTraits< T > TTraits;
    if ( !viewport.IsValid() )
        return false;
    CTPoint3D< T > ndc;
    T w = TTraits::Zero();
    if ( !viewProjection.TryTransformPointProjective( worldPoint, ndc, w ) || !( TTraits::Zero() < w ) )
        return false;
    const T half = TTraits::Half();
    const T ndcDown = viewport.ndcYDown ? ndc.y : ComponentNegate( ndc.y );
    screenPoint = CTPoint2D< T >( viewport.x + ( ndc.x * half + half ) * viewport.width,
                                  viewport.y + ( ndcDown * half + half ) * viewport.height );
    depth = ndc.z;
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The world point at a screen position and normalised device depth (the inverse of Project). 'inverseViewProjection' is
 *  the inverse of projection * view. False, 'worldPoint' unchanged, when the viewport is not valid, when the point is at
 *  infinity (for example the far depth of a projection with an infinite far plane) or when the depth lies beyond the
 *  depth range of the projection so that the homogeneous w is not positive (the point would be mirrored behind the camera).
 */
template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
Unproject( const CTPoint2D< T >& screenPoint, const T& depth, const CTMatrix4D< T >& inverseViewProjection,
           const CTViewport< T >& viewport, CTPoint3D< T >& worldPoint ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTNumericTraits< T > TTraits;
    if ( !viewport.IsValid() )
        return false;
    const T two = TTraits::Two();
    const T one = TTraits::One();
    const T ndcX = ( screenPoint.x - viewport.x ) / viewport.width * two - one;
    const T screenY = ( screenPoint.y - viewport.y ) / viewport.height * two - one;
    const T ndcY = viewport.ndcYDown ? screenY : ComponentNegate( screenY );
    T w = TTraits::Zero();
    CTPoint3D< T > result;
    // a negative w is a depth outside the depth range of the projection (for example slightly below 0 with an infinite
    // reversed projection), which would give a point mirrored behind the camera: that is not a point of the view
    if ( !inverseViewProjection.TryTransformPointProjective( CTPoint3D< T >( ndcX, ndcY, depth ), result, w ) || !( TTraits::Zero() < w ) )
        return false;
    worldPoint = result;
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The picking ray of a screen position: 'origin' is the point of the near plane under the position and 'direction' points
 *  away from the camera along the ray (for a perspective projection the ray passes through the camera position; for an
 *  orthographic projection it is the view direction). The convention gives the depth values of the near and far plane; with
 *  an infinite far plane the depth halfway between them is used for the second point. False when the viewport is not
 *  valid or the ray is degenerate (a point at infinity, or both points equal).
 */
template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
GetPickRay( const CTPoint2D< T >& screenPoint, const CTMatrix4D< T >& inverseViewProjection, const CTViewport< T >& viewport,
            const SProjectionConvention& convention, CTPoint3D< T >& origin, CTDirectionVector3D< T >& direction ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTNumericTraits< T > TTraits;
    const T nearDepth = TTraits::FromInt32( convention.GetNearDepth() );
    const T farDepth = TTraits::FromInt32( convention.GetFarDepth() );
    const T secondDepth = convention.infiniteFar ? ( nearDepth + farDepth ) * TTraits::Half() : farDepth;
    CTPoint3D< T > nearPoint;
    CTPoint3D< T > farPoint;
    if ( !Unproject( screenPoint, nearDepth, inverseViewProjection, viewport, nearPoint ) ||
         !Unproject( screenPoint, secondDepth, inverseViewProjection, viewport, farPoint ) )
        return false;
    CTDirectionVector3D< T > rayDirection;
    if ( !CTDirectionVector3D< T >::TryCreate( farPoint - nearPoint, rayDirection ) )
        return false;
    origin = nearPoint;
    direction = rayDirection;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefMATH_ProjectionInstantiations.cpp) are used, section 5.10 */

#define GUCEF_MATH_PROJECTIONFUNCTIONS_KIND( prefix, kind ) \
    prefix bool Project< kind >( const CTPoint3D< kind >&, const CTMatrix4D< kind >&, const CTViewport< kind >&, CTPoint2D< kind >&, kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool Unproject< kind >( const CTPoint2D< kind >&, const kind&, const CTMatrix4D< kind >&, const CTViewport< kind >&, CTPoint3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool GetPickRay< kind >( const CTPoint2D< kind >&, const CTMatrix4D< kind >&, const CTViewport< kind >&, const SProjectionConvention&, CTPoint3D< kind >&, CTDirectionVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_PROJECTIONFUNCTIONS_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_PROJECTIONFUNCTIONS_KIND( prefix, Float32 ) \
    GUCEF_MATH_PROJECTIONFUNCTIONS_KIND( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
GUCEF_MATH_PROJECTIONFUNCTIONS_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_PROJECTIONFUNCTIONS_H ? */
