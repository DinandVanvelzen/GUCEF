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

#ifndef GUCEF_MATH_CORESTRUCTINTEROP_H
#define GUCEF_MATH_CORESTRUCTINTEROP_H

/*
 *  Conversions between the plain structures of gucefCORE (EStructs.h) and the gucefMATH value types. Free functions,
 *  so that CORE types never appear inside the class templates:
 *  - TPoint2DF / TPoint3DF <-> TFloat32Point2D / TFloat32Point3D: ToMath, ToCore;
 *  - TPoint2DI / TPoint3DI <-> TInt32Point2D / TInt32Point3D: ToMath, ToCore;
 *  - TSize3D ( width, height, depth ) <-> TFloat32Vector3D ( x, y, z ): ToMathVector, ToCoreSize.
 *  Single values convert member-wise; arrays convert with memcpy, which the static assertions on sizeof make valid
 *  (the CORE structures are packed, the gucefMATH types have no padding), never by reinterpret_cast of typed pointers.
 *  The rectangle and box forms of TArea2D / TArea3D are in gucefGEOMETRY_CoreStructInterop.h. All functions are exact
 *  and realtime-safe.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

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
 *  Copies 'count' elements between two types with the same size and component layout. Callers pin the sizes with a
 *  static assertion; 'in' and 'out' must not overlap.
 */
template < typename TFrom, typename TTo >
GUCEF_MATH_FORCE_INLINE void
CopyCoreStructArray( const TFrom* in, TTo* out, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( sizeof( TFrom ) == sizeof( TTo ), Core_struct_and_math_type_sizes_must_match );
    GUCEF_MATH_ASSERT( 0 == count || ( GUCEF_NULL != in && GUCEF_NULL != out ) );
    if ( count > 0 )
        memcpy( static_cast< void* >( out ), static_cast< const void* >( in ), (size_t) count * sizeof( TTo ) );
}

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE TFloat32Point2D ToMath( const CORE::TPoint2DF& point ) GUCEF_MATH_REALTIME_CONTEXT { return TFloat32Point2D( point.x, point.y ); }
GUCEF_MATH_FORCE_INLINE TFloat32Point3D ToMath( const CORE::TPoint3DF& point ) GUCEF_MATH_REALTIME_CONTEXT { return TFloat32Point3D( point.x, point.y, point.z ); }
GUCEF_MATH_FORCE_INLINE TInt32Point2D ToMath( const CORE::TPoint2DI& point ) GUCEF_MATH_REALTIME_CONTEXT { return TInt32Point2D( point.x, point.y ); }
GUCEF_MATH_FORCE_INLINE TInt32Point3D ToMath( const CORE::TPoint3DI& point ) GUCEF_MATH_REALTIME_CONTEXT { return TInt32Point3D( point.x, point.y, point.z ); }

/*-------------------------------------------------------------------------*/

GUCEF_MATH_FORCE_INLINE CORE::TPoint2DF ToCore( const TFloat32Point2D& point ) GUCEF_MATH_REALTIME_CONTEXT { CORE::TPoint2DF result; result.x = point.x; result.y = point.y; return result; }
GUCEF_MATH_FORCE_INLINE CORE::TPoint3DF ToCore( const TFloat32Point3D& point ) GUCEF_MATH_REALTIME_CONTEXT { CORE::TPoint3DF result; result.x = point.x; result.y = point.y; result.z = point.z; return result; }
GUCEF_MATH_FORCE_INLINE CORE::TPoint2DI ToCore( const TInt32Point2D& point ) GUCEF_MATH_REALTIME_CONTEXT { CORE::TPoint2DI result; result.x = point.x; result.y = point.y; return result; }
GUCEF_MATH_FORCE_INLINE CORE::TPoint3DI ToCore( const TInt32Point3D& point ) GUCEF_MATH_REALTIME_CONTEXT { CORE::TPoint3DI result; result.x = point.x; result.y = point.y; result.z = point.z; return result; }

/*-------------------------------------------------------------------------*/

/** ( width, height, depth ) as the vector ( x, y, z ) */
GUCEF_MATH_FORCE_INLINE TFloat32Vector3D ToMathVector( const CORE::TSize3D& size ) GUCEF_MATH_REALTIME_CONTEXT { return TFloat32Vector3D( size.width, size.height, size.depth ); }

/** The vector ( x, y, z ) as ( width, height, depth ) */
GUCEF_MATH_FORCE_INLINE CORE::TSize3D ToCoreSize( const TFloat32Vector3D& vector ) GUCEF_MATH_REALTIME_CONTEXT { CORE::TSize3D result; result.width = vector.x; result.height = vector.y; result.depth = vector.z; return result; }

/*-------------------------------------------------------------------------*/

/** Array forms ('count' elements, no overlap), by memcpy of identical layouts */
GUCEF_MATH_FORCE_INLINE void ToMath( const CORE::TPoint2DF* in, TFloat32Point2D* out, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT { CopyCoreStructArray( in, out, count ); }
GUCEF_MATH_FORCE_INLINE void ToMath( const CORE::TPoint3DF* in, TFloat32Point3D* out, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT { CopyCoreStructArray( in, out, count ); }
GUCEF_MATH_FORCE_INLINE void ToMath( const CORE::TPoint2DI* in, TInt32Point2D* out, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT { CopyCoreStructArray( in, out, count ); }
GUCEF_MATH_FORCE_INLINE void ToMath( const CORE::TPoint3DI* in, TInt32Point3D* out, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT { CopyCoreStructArray( in, out, count ); }
GUCEF_MATH_FORCE_INLINE void ToMathVector( const CORE::TSize3D* in, TFloat32Vector3D* out, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT { CopyCoreStructArray( in, out, count ); }

GUCEF_MATH_FORCE_INLINE void ToCore( const TFloat32Point2D* in, CORE::TPoint2DF* out, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT { CopyCoreStructArray( in, out, count ); }
GUCEF_MATH_FORCE_INLINE void ToCore( const TFloat32Point3D* in, CORE::TPoint3DF* out, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT { CopyCoreStructArray( in, out, count ); }
GUCEF_MATH_FORCE_INLINE void ToCore( const TInt32Point2D* in, CORE::TPoint2DI* out, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT { CopyCoreStructArray( in, out, count ); }
GUCEF_MATH_FORCE_INLINE void ToCore( const TInt32Point3D* in, CORE::TPoint3DI* out, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT { CopyCoreStructArray( in, out, count ); }
GUCEF_MATH_FORCE_INLINE void ToCoreSize( const TFloat32Vector3D* in, CORE::TSize3D* out, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT { CopyCoreStructArray( in, out, count ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CORESTRUCTINTEROP_H ? */
