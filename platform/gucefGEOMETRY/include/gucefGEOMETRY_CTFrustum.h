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

#ifndef GUCEF_GEOMETRY_CTFRUSTUM_H
#define GUCEF_GEOMETRY_CTFRUSTUM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#include "gucefGEOMETRY_SShapeTextReader.h"
#define GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#endif /* GUCEF_GEOMETRY_SSHAPETEXTREADER_H ? */

#ifndef GUCEF_GEOMETRY_SRAYCASTRESULT_H
#include "gucefGEOMETRY_SRayCastResult.h"
#define GUCEF_GEOMETRY_SRAYCASTRESULT_H
#endif /* GUCEF_GEOMETRY_SRAYCASTRESULT_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H
#include "gucefGEOMETRY_CTGeometryTolerance.h"
#define GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H
#endif /* GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H ? */

#ifndef GUCEF_GEOMETRY_CTPLANE_H
#include "gucefGEOMETRY_CTPlane.h"
#define GUCEF_GEOMETRY_CTPLANE_H
#endif /* GUCEF_GEOMETRY_CTPLANE_H ? */

#ifndef GUCEF_MATH_SPROJECTIONCONVENTION_H
#include "gucefMATH_SProjectionConvention.h"
#define GUCEF_MATH_SPROJECTIONCONVENTION_H
#endif /* GUCEF_MATH_SPROJECTIONCONVENTION_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {
template < typename T > class CTMatrix4D;
}
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The indices of the six planes of a frustum */
enum EFrustumPlane
{
    FRUSTUMPLANE_NEAR = 0 ,
    FRUSTUMPLANE_FAR      ,
    FRUSTUMPLANE_LEFT     ,
    FRUSTUMPLANE_RIGHT    ,
    FRUSTUMPLANE_BOTTOM   ,
    FRUSTUMPLANE_TOP
};
typedef enum EFrustumPlane TFrustumPlane;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T > class CTSphere;
template < typename T > class CTAxisAlignedBox3D;
template < typename T > class CTOrientedBox3D;
template < typename T > class CTRay3D;
template < typename T > class CTSegment3D;

/*-------------------------------------------------------------------------*/

/**
 *  The view volume of a projection: six planes (near, far, left, right, bottom, top) and the eight corners (section 7.2).
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root.
 *  - Layout: six planes and eight points, 36 * sizeof( T ), no virtual functions. The planes are the data; the
 *    corners follow from them and are kept in step by every member that changes the planes. The default frustum is the
 *    clip cube of an identity projection ( [ -1, 1 ] on every axis ).
 *  - The normals of the planes point into the frustum: a point is inside when its signed distance to every plane is not
 *    negative. The corner index has bit 0 for right, bit 1 for top and bit 2 for far.
 *  - FromViewProjection extracts the planes from the combined projection and view matrix ( clip = matrix * point, column
 *    vectors ) by the Gribb-Hartmann rows ( row3 +- row0 / row1 and the depth rows of the depth range ), for every
 *    convention of SProjectionConvention (section 4.5.6): reversed Z swaps the roles of the near and far rows and a
 *    flipped clip space Y swaps bottom and top. An infinite far plane has no finite row: it is replaced by a plane
 *    GetInfiniteFarDistance() behind the near plane, which no coordinate of a scene reaches.
 *  - Realtime: every member except the wireframe, text and DataNode members is realtime-safe.
 *  - Text: "Frustum(near Plane(...), far Plane(...), left Plane(...), right Plane(...), bottom Plane(...), top Plane(...))";
 *    DataNode: child nodes near, far, left, right, bottom, top, each a plane.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTFrustum
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTDirectionVector3D< T > TDirection;
    typedef CTPlane< T > TPlane;

    /** The clip cube [ -1, 1 ]^3 */
    CTFrustum( void );

    /**
     *  The frustum of a combined projection * view matrix. False (out unchanged) when a plane degenerates (a zero or
     *  not finite row, which a valid projection never has except the far row of an infinite projection).
     */
    static bool TryFromViewProjection( const MATH::CTMatrix4D< T >& viewProjection, const MATH::SProjectionConvention& convention, CTFrustum& out ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** As TryFromViewProjection; the clip cube (and an assertion in debug builds) for a degenerate matrix */
    static CTFrustum FromViewProjection( const MATH::CTMatrix4D< T >& viewProjection, const MATH::SProjectionConvention& convention ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The frustum of six planes in the order of TFrustumPlane (normals into the frustum); false (out unchanged) when three planes that meet in a corner do not meet in a point */
    static bool TryFromPlanes( const TPlane ( &planes )[ 6 ], CTFrustum& out ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** How far behind the near plane the replacement of an infinite far plane lies: a quarter of the square root of the largest number of the kind */
    static GUCEF_GEOMETRY_FORCE_INLINE T GetInfiniteFarDistance( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TTraits::Sqrt( TTraits::Max() ) * ( TTraits::Half() * TTraits::Half() );
    }

    /** The plane of a TFrustumPlane index (the near plane for an index above 5) */
    GUCEF_GEOMETRY_FORCE_INLINE const TPlane& GetPlane( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_planes[ index < 6 ? index : 0 ]; }

    /** The six planes in the order of TFrustumPlane, in one array */
    GUCEF_GEOMETRY_FORCE_INLINE const TPlane* GetPlanes( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_planes; }

    /** The corner of an index with bit 0 for right, bit 1 for top and bit 2 for far (higher bits are ignored) */
    GUCEF_GEOMETRY_FORCE_INLINE const TPoint& GetCorner( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_corners[ index & 7 ]; }

    /** True when the far plane is the replacement of an infinite far plane (half the infinite far distance or further behind the near plane) */
    bool IsFarInfinite( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** True when the point is inside or on the surface */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** OUTSIDE when the box lies entirely behind one plane, INSIDE when it is in front of all, else INTERSECTING; conservative near edges and corners (see ClassifySphereToPlaneSet) */
    TFrustumClassification Classify( const CTAxisAlignedBox3D< T >& box ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** As above for an oriented box */
    TFrustumClassification Classify( const CTOrientedBox3D< T >& box ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** As above for a sphere */
    TFrustumClassification Classify( const CTSphere< T >& sphere ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  The tolerant forms (section 7.10): the region is widened by the length tolerance for the magnitude of the operands,
     *  so a point or shape within it of the boundary counts as inside; the forms without tolerance are exact.
     */
    bool Contains( const TPoint& point, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;
    TFrustumClassification Classify( const CTSphere< T >& sphere, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;
    TFrustumClassification Classify( const CTAxisAlignedBox3D< T >& box, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;
    TFrustumClassification Classify( const CTOrientedBox3D< T >& box, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The corner furthest along 'direction' */
    TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The axis aligned box around the eight corners */
    CTAxisAlignedBox3D< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The volume (six tetrahedra around a diagonal); the largest number of the kind for an infinite far plane */
    T GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Cyrus-Beck clipping against the planes (see RayCastPlaneSet): hit where the ray enters, with the outward plane normal, or distance 0 and RAYHIT_STARTED_INSIDE; the feature index is the TFrustumPlane */
    bool RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Appends the twelve edges */
    void GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const;

    /** The image under an affine matrix (a frustum stays a frustum): false (out unchanged) when the linear part is singular */
    bool TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTFrustum& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The image under an affine matrix; this frustum unchanged (and an assertion in debug builds) for a singular matrix */
    CTFrustum Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTFrustum& a, const CTFrustum& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<6; ++i )
        {
            if ( !( a.m_planes[ i ].normal == b.m_planes[ i ].normal ) || !( a.m_planes[ i ].offset == b.m_planes[ i ].offset ) )
                return false;
        }
        return true;
    }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTFrustum& a, const CTFrustum& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    /** The planes equal within the default tolerances of the kind */
    bool Equals( const CTFrustum& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<6; ++i )
        {
            if ( !m_planes[ i ].Equals( other.m_planes[ i ] ) )
                return false;
        }
        return true;
    }

    /** "Frustum(near Plane(...), far Plane(...), left Plane(...), right Plane(...), bottom Plane(...), top Plane(...))" */
    CORE::CString ToString( void ) const
    {
        static const char* const names[ 6 ] = { "near", "far", "left", "right", "bottom", "top" };
        CORE::CString text( "Frustum(" );
        for ( UInt32 i=0; i<6; ++i )
        {
            if ( i > 0 )
                text += ", ";
            text += names[ i ];
            text += ' ';
            text += m_planes[ i ].ToString();
        }
        text += ')';
        return text;
    }

    /** Parses the ToString form; false leaves this frustum unchanged */
    bool FromString( const CORE::CString& text )
    {
        static const char* const names[ 6 ] = { "near", "far", "left", "right", "bottom", "top" };
        SShapeTextReader reader( text );
        TPlane parsed[ 6 ];
        if ( !reader.TryLiteral( "Frustum" ) || !reader.TryChar( '(' ) )
            return false;
        for ( UInt32 i=0; i<6; ++i )
        {
            if ( ( i > 0 && !reader.TryChar( ',' ) ) || !reader.TryLiteral( names[ i ] ) || !reader.TryShape( parsed[ i ] ) )
                return false;
        }
        CTFrustum result;
        if ( !reader.TryChar( ')' ) || !reader.AtEnd() || !TryFromPlanes( parsed, result ) )
            return false;
        *this = result;
        return true;
    }

    /** Writes the child nodes near, far, left, right, bottom and top into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        static const char* const names[ 6 ] = { "near", "far", "left", "right", "bottom", "top" };
        for ( UInt32 i=0; i<6; ++i )
        {
            if ( !SerializeShapeChild( node, names[ i ], m_planes[ i ], settings ) )
                return false;
        }
        return true;
    }

    /** Reads the six child nodes; a missing or invalid one fails and leaves this frustum unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        static const char* const names[ 6 ] = { "near", "far", "left", "right", "bottom", "top" };
        TPlane parsed[ 6 ];
        for ( UInt32 i=0; i<6; ++i )
        {
            if ( !DeserializeShapeChild( node, names[ i ], parsed[ i ], settings ) )
                return false;
        }
        CTFrustum result;
        if ( !TryFromPlanes( parsed, result ) )
            return false;
        *this = result;
        return true;
    }

    private:

    TPlane m_planes[ 6 ];
    TPoint m_corners[ 8 ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTFrustum< T >::CTFrustum( void )
{
    const T one = TTraits::One();
    const T minusOne = MATH::ComponentNegate( one );
    const T zero = TTraits::Zero();
    m_planes[ FRUSTUMPLANE_NEAR ] = TPlane( TDirection( TVector( zero, zero, one ) ), minusOne );
    m_planes[ FRUSTUMPLANE_FAR ] = TPlane( TDirection( TVector( zero, zero, minusOne ) ), minusOne );
    m_planes[ FRUSTUMPLANE_LEFT ] = TPlane( TDirection( TVector( one, zero, zero ) ), minusOne );
    m_planes[ FRUSTUMPLANE_RIGHT ] = TPlane( TDirection( TVector( minusOne, zero, zero ) ), minusOne );
    m_planes[ FRUSTUMPLANE_BOTTOM ] = TPlane( TDirection( TVector( zero, one, zero ) ), minusOne );
    m_planes[ FRUSTUMPLANE_TOP ] = TPlane( TDirection( TVector( zero, minusOne, zero ) ), minusOne );
    for ( UInt32 i=0; i<8; ++i )
        m_corners[ i ] = TPoint( 0 != ( i & 1 ) ? one : minusOne, 0 != ( i & 2 ) ? one : minusOne, 0 != ( i & 4 ) ? one : minusOne );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTFrustum< T >::TryFromPlanes( const TPlane ( &planes )[ 6 ], CTFrustum& out ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    CTFrustum result;
    for ( UInt32 i=0; i<6; ++i )
        result.m_planes[ i ] = planes[ i ];
    for ( UInt32 i=0; i<8; ++i )
    {
        const TPlane& depth = result.m_planes[ 0 != ( i & 4 ) ? FRUSTUMPLANE_FAR : FRUSTUMPLANE_NEAR ];
        const TPlane& side = result.m_planes[ 0 != ( i & 1 ) ? FRUSTUMPLANE_RIGHT : FRUSTUMPLANE_LEFT ];
        const TPlane& vertical = result.m_planes[ 0 != ( i & 2 ) ? FRUSTUMPLANE_TOP : FRUSTUMPLANE_BOTTOM ];
        if ( !TPlane::IntersectPlanes( depth, side, vertical, result.m_corners[ i ] ) )
            return false;
    }
    out = result;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTFrustum< T >::TryFromViewProjection( const MATH::CTMatrix4D< T >& viewProjection, const MATH::SProjectionConvention& convention, CTFrustum& out ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTVector4D< T > TVector4;
    const TVector4 row0 = viewProjection.GetRow( 0 );
    const TVector4 row1 = viewProjection.GetRow( 1 );
    const TVector4 row2 = viewProjection.GetRow( 2 );
    const TVector4 row3 = viewProjection.GetRow( 3 );

    /* the plane rows: clip.x >= -w is row3 + row0, clip.x <= w is row3 - row0, and so on */
    TVector4 rows[ 6 ];
    const TVector4 low = MATH::DEPTHRANGE_ZERO_TO_ONE == convention.depthRange ? row2 : row3 + row2;       /* the depth at the low end of the range */
    const TVector4 high = row3 - row2;                                                                    /* the depth at the high end of the range */
    rows[ FRUSTUMPLANE_NEAR ] = convention.reversedZ ? high : low;
    rows[ FRUSTUMPLANE_FAR ] = convention.reversedZ ? low : high;
    rows[ FRUSTUMPLANE_LEFT ] = row3 + row0;
    rows[ FRUSTUMPLANE_RIGHT ] = row3 - row0;
    rows[ FRUSTUMPLANE_BOTTOM ] = convention.flipY ? row3 - row1 : row3 + row1;
    rows[ FRUSTUMPLANE_TOP ] = convention.flipY ? row3 + row1 : row3 - row1;

    TPlane planes[ 6 ];
    bool farDegenerate = false;
    for ( UInt32 i=0; i<6; ++i )
    {
        const TVector n( rows[ i ].x, rows[ i ].y, rows[ i ].z );
        const T length = n.GetLength();
        TDirection unit;
        if ( !TTraits::IsFinite( length ) || !TTraits::IsFinite( rows[ i ].w ) || !TDirection::TryCreate( n, unit ) )
        {
            if ( FRUSTUMPLANE_FAR == i && TTraits::IsFinite( rows[ i ].w ) && TTraits::IsFinite( length ) )
            {
                farDegenerate = true;
                continue;
            }
            return false;
        }
        planes[ i ] = TPlane( unit, MATH::ComponentNegate( rows[ i ].w ) / length );
    }
    if ( farDegenerate || convention.infiniteFar )
    {
        const TPlane& nearPlane = planes[ FRUSTUMPLANE_NEAR ];
        planes[ FRUSTUMPLANE_FAR ] = TPlane( -nearPlane.normal, MATH::ComponentNegate( nearPlane.offset + GetInfiniteFarDistance() ) );
    }
    return TryFromPlanes( planes, out );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTFrustum< T >
CTFrustum< T >::FromViewProjection( const MATH::CTMatrix4D< T >& viewProjection, const MATH::SProjectionConvention& convention ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    CTFrustum result;
    const bool extracted = TryFromViewProjection( viewProjection, convention, result );
    GUCEF_GEOMETRY_ASSERT( extracted );
    (void) extracted;
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTFrustum< T >::IsFarInfinite( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TPlane& nearPlane = m_planes[ FRUSTUMPLANE_NEAR ];
    const TPlane& farPlane = m_planes[ FRUSTUMPLANE_FAR ];
    if ( !( farPlane.normal.Dot( nearPlane.normal.AsVector() ) < MATH::ComponentNegate( TTraits::Half() ) ) )
        return false;                       /* the planes are not facing each other: a perspective frustum, which is finite */
    const T distance = MATH::ComponentNegate( farPlane.offset ) - nearPlane.offset;
    return GetInfiniteFarDistance() * TTraits::Half() <= distance;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
CTFrustum< T >::Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return PlaneSetContains( m_planes, 6, point );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
CTFrustum< T >::Classify( const CTAxisAlignedBox3D< T >& box ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( box.IsEmpty() )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    return ClassifyBoxToPlaneSet( m_planes, 6, box.GetCentre(), box.GetHalfExtents() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
CTFrustum< T >::Classify( const CTOrientedBox3D< T >& box ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !box.IsValid() )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    TVector axes[ 3 ];
    box.GetAxes( axes );
    return ClassifyOrientedBoxToPlaneSet( m_planes, 6, box.centre, axes, box.halfExtents );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
CTFrustum< T >::Classify( const CTSphere< T >& sphere ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( TTraits::IsNegative( sphere.radius ) )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    return ClassifySphereToPlaneSet( m_planes, 6, sphere.centre, sphere.radius );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTFrustum< T >::TPoint
CTFrustum< T >::GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    UInt32 best = 0;
    T bestDot = m_corners[ 0 ].ToPositionVector().Dot( direction );
    for ( UInt32 i=1; i<8; ++i )
    {
        const T d = m_corners[ i ].ToPositionVector().Dot( direction );
        if ( bestDot < d )
        {
            bestDot = d;
            best = i;
        }
    }
    return m_corners[ best ];
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTAxisAlignedBox3D< T >
CTFrustum< T >::GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTAxisAlignedBox3D< T >::FromPoints( m_corners, 8 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTFrustum< T >::GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( IsFarInfinite() )
        return TTraits::Max();
    static const UInt32 tetrahedra[ 6 ][ 3 ] = { { 1, 3, 7 }, { 3, 2, 7 }, { 2, 6, 7 }, { 6, 4, 7 }, { 4, 5, 7 }, { 5, 1, 7 } };
    T sum = TTraits::Zero();
    for ( UInt32 i=0; i<6; ++i )
    {
        const TVector a = m_corners[ tetrahedra[ i ][ 0 ] ] - m_corners[ 0 ];
        const TVector b = m_corners[ tetrahedra[ i ][ 1 ] ] - m_corners[ 0 ];
        const TVector c = m_corners[ tetrahedra[ i ][ 2 ] ] - m_corners[ 0 ];
        sum = sum + TTraits::Abs( a.Cross( b ).Dot( c ) );
    }
    return sum / TTraits::FromInt32( 6 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTFrustum< T >::RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return RayCastPlaneSet( m_planes, 6, ray, maxDistance, result );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTFrustum< T >::GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 /*segmentsPerCircle*/ ) const
{
    for ( UInt32 corner=0; corner<8; ++corner )
    {
        for ( UInt32 bit=1; bit<=4; bit<<=1 )
        {
            if ( 0 == ( corner & bit ) )
                lines.push_back( CTSegment3D< T >( m_corners[ corner ], m_corners[ corner | bit ] ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTFrustum< T >::TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTFrustum& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TPlane planes[ 6 ];
    for ( UInt32 i=0; i<6; ++i )
    {
        if ( !m_planes[ i ].TryTransformed( matrix, planes[ i ] ) )
            return false;
    }
    return TryFromPlanes( planes, out );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTFrustum< T >
CTFrustum< T >::Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    CTFrustum result;
    const bool transformed = TryTransformed( matrix, result );
    GUCEF_GEOMETRY_ASSERT( transformed );
    return transformed ? result : *this;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTFrustum< T >::Contains( const TPoint& point, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return PlaneSetContains( m_planes, 6, point, tolerance );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
CTFrustum< T >::Classify( const CTSphere< T >& sphere, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( TTraits::IsNegative( sphere.radius ) )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    return ClassifySphereToPlaneSet( m_planes, 6, sphere.centre, sphere.radius, tolerance );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
CTFrustum< T >::Classify( const CTAxisAlignedBox3D< T >& box, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( box.IsEmpty() )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    return ClassifyBoxToPlaneSet( m_planes, 6, box.GetCentre(), box.GetHalfExtents(), tolerance );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TFrustumClassification
CTFrustum< T >::Classify( const CTOrientedBox3D< T >& box, const CTGeometryTolerance< T >& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !box.IsValid() )
        return FRUSTUMCLASSIFICATION_OUTSIDE;
    TVector axes[ 3 ];
    box.GetAxes( axes );
    return ClassifyOrientedBoxToPlaneSet( m_planes, 6, box.centre, axes, box.halfExtents, tolerance );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTFrustum< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTFrustum< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*
 *  The related classes used by the members; included after the class so that each header can be included first.
 */

#ifndef GUCEF_GEOMETRY_PLANESETFUNCTIONS_H
#include "gucefGEOMETRY_PlaneSetFunctions.h"
#define GUCEF_GEOMETRY_PLANESETFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_PLANESETFUNCTIONS_H ? */

#ifndef GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#include "gucefGEOMETRY_CTAxisAlignedBox3D.h"
#define GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#include "gucefGEOMETRY_CTOrientedBox3D.h"
#define GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTORIENTEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTSPHERE_H
#include "gucefGEOMETRY_CTSphere.h"
#define GUCEF_GEOMETRY_CTSPHERE_H
#endif /* GUCEF_GEOMETRY_CTSPHERE_H ? */

#ifndef GUCEF_GEOMETRY_CTRAY3D_H
#include "gucefGEOMETRY_CTRay3D.h"
#define GUCEF_GEOMETRY_CTRAY3D_H
#endif /* GUCEF_GEOMETRY_CTRAY3D_H ? */

#ifndef GUCEF_GEOMETRY_CTSEGMENT3D_H
#include "gucefGEOMETRY_CTSegment3D.h"
#define GUCEF_GEOMETRY_CTSEGMENT3D_H
#endif /* GUCEF_GEOMETRY_CTSEGMENT3D_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTFRUSTUM_H ? */
