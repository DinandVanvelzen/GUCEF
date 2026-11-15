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

#ifndef GUCEF_GEOMETRY_SUPPORTMAPPINGS_H
#define GUCEF_GEOMETRY_SUPPORTMAPPINGS_H

/*
 *  The support mappings of section 7.6.1 of the design: the input of GJK, EPA, the GJK ray cast and time of impact.
 *
 *  A support mapping is any type (a template parameter, never a virtual call) with
 *
 *  - typedef TValue                                               the numeric kind;
 *  - CTPoint3D< T > GetSupport( const CTVector3D< T >& d ) const   a point of the core furthest along d (any one on
 *                                                                 ties; d may have any length, the zero vector
 *                                                                 gives some point of the core);
 *  - T GetMargin( void ) const                                    the radius around the core: the shape is the core
 *                                                                 grown by this distance (0 for shapes without one);
 *  - CTPoint3D< T > GetAnyPoint( void ) const                     a point of the core (the start of GJK).
 *
 *  Shapes with a radius (sphere, capsule) enter as a core (point, segment) plus a margin, which keeps GJK on the cores
 *  well-conditioned near contact (section 7.5.1). GetSupportWithMargin gives the support of the whole shape. GJK, EPA
 *  and the casts pass search directions scaled by CTSearchDirectionScaler (largest component in [1/2, 1)), so in
 *  fixed point a support mapping may form dot products of the direction with coordinates of up to about 7e8.
 *
 *  The adaptors are small value types; the ones that refer to a shape (CTShapeSupport) hold a pointer to it, so the
 *  shape must outlive the adaptor. All members are realtime-safe and allocation-free.
 *
 *  CTSupportMapping< TShape > maps every convex shape of the library to its support mapping:
 *  CTSupportMapping< TShape >::Make( shape ). Cylinders and cones use their own adaptors (CTCylinderSupport,
 *  CTConeSupport): the axis is normalised once per query and the support stays on the surface for directions nearly
 *  parallel to the axis.
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

#ifndef GUCEF_GEOMETRY_SUPPORTFUNCTIONS_H
#include "gucefGEOMETRY_SupportFunctions.h"
#define GUCEF_GEOMETRY_SUPPORTFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_SUPPORTFUNCTIONS_H ? */

#ifndef GUCEF_GEOMETRY_VOLUMESHAPES_H
#include "gucefGEOMETRY_volumeshapes.h"
#define GUCEF_GEOMETRY_VOLUMESHAPES_H
#endif /* GUCEF_GEOMETRY_VOLUMESHAPES_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

template < typename T > class CTConvexHull3D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Scales a search direction by a power of two so that its largest component lies in [1/2, 1) (the zero vector stays
 *  zero). Support mappings form dot products of the direction with absolute vertex coordinates; in fixed point a
 *  direction as long as a coordinate would make such a product overflow Q32.32, a short one would lose its
 *  resolution. Floats are returned unchanged: a power-of-two scaling could not change their support choice, and
 *  their range has no such limit, so their results stay bit-identical.
 */
template < typename T >
class CTSearchDirectionScaler
{
    public:
    static GUCEF_GEOMETRY_FORCE_INLINE MATH::CTVector3D< T > Scale( const MATH::CTVector3D< T >& direction ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return direction; }
};

template < typename TStorage, UInt32 FractionBits >
class CTSearchDirectionScaler< MATH::CTFixedPoint< TStorage, FractionBits > >
{
    public:
    typedef MATH::CTFixedPoint< TStorage, FractionBits > TFixed;

    static MATH::CTVector3D< TFixed > Scale( const MATH::CTVector3D< TFixed >& direction ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        const Int64 raw[ 3 ] = { (Int64) direction.x.GetRaw(), (Int64) direction.y.GetRaw(), (Int64) direction.z.GetRaw() };
        UInt64 magnitude[ 3 ];
        UInt64 largest = 0;
        for ( UInt32 i=0; i<3; ++i )
        {
            magnitude[ i ] = raw[ i ] < 0 ? (UInt64) 0 - (UInt64) raw[ i ] : (UInt64) raw[ i ];
            largest = magnitude[ i ] > largest ? magnitude[ i ] : largest;
        }
        if ( 0 == largest )
            return direction;
        Int32 bit = 63;
        while ( 0 == ( largest >> bit ) )
            --bit;
        const Int32 shift = (Int32) FractionBits - 1 - bit;
        TStorage scaled[ 3 ];
        for ( UInt32 i=0; i<3; ++i )
        {
            /* the magnitude is shifted, so that the scaling is symmetric under negation */
            const UInt64 m = shift >= 0 ? magnitude[ i ] << shift : magnitude[ i ] >> ( -shift );
            scaled[ i ] = (TStorage) ( raw[ i ] < 0 ? -(Int64) m : (Int64) m );
        }
        return MATH::CTVector3D< TFixed >( TFixed::FromRaw( scaled[ 0 ] ), TFixed::FromRaw( scaled[ 1 ] ), TFixed::FromRaw( scaled[ 2 ] ) );
    }
};

/*-------------------------------------------------------------------------*/

/** A single point as a core (the core of a sphere) */
template < typename T >
class CTPointSupport
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;

    TPoint point;

    GUCEF_GEOMETRY_FORCE_INLINE CTPointSupport( void ) : point() {}
    GUCEF_GEOMETRY_FORCE_INLINE explicit CTPointSupport( const TPoint& p ) : point( p ) {}

    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetSupport( const TVector& /*direction*/ ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return point; }
    GUCEF_GEOMETRY_FORCE_INLINE T GetMargin( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TTraits::Zero(); }
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetAnyPoint( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return point; }
};

/*-------------------------------------------------------------------------*/

/** A segment as a core (the core of a capsule); the start on ties */
template < typename T >
class CTSegmentSupport
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;

    TPoint start;
    TPoint end;

    GUCEF_GEOMETRY_FORCE_INLINE CTSegmentSupport( void ) : start(), end() {}
    GUCEF_GEOMETRY_FORCE_INLINE CTSegmentSupport( const TPoint& a, const TPoint& b ) : start( a ), end( b ) {}

    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TTraits::Zero() < ( end - start ).Dot( direction ) ? end : start;
    }
    GUCEF_GEOMETRY_FORCE_INLINE T GetMargin( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TTraits::Zero(); }
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetAnyPoint( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return start; }
};

/*-------------------------------------------------------------------------*/

/**
 *  Any convex shape with a GetSupport member, without margin (triangle, boxes, cylinder, cone, hull, frustum). Holds a
 *  pointer: the shape must outlive the adaptor.
 */
template < typename TShape >
class CTShapeSupport
{
    public:

    typedef typename TShape::TValue TValue;
    typedef MATH::CTNumericTraits< TValue > TTraits;
    typedef MATH::CTPoint3D< TValue > TPoint;
    typedef MATH::CTVector3D< TValue > TVector;

    GUCEF_GEOMETRY_FORCE_INLINE explicit CTShapeSupport( const TShape& shape ) : m_shape( &shape ) {}

    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_shape->GetSupport( direction ); }
    GUCEF_GEOMETRY_FORCE_INLINE TValue GetMargin( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TTraits::Zero(); }

    /** The support along +X: a point of the shape that depends only on the shape */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetAnyPoint( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_shape->GetSupport( TVector::UnitX() ); }

    GUCEF_GEOMETRY_FORCE_INLINE const TShape& GetShape( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return *m_shape; }

    private:

    const TShape* m_shape;
};

/*-------------------------------------------------------------------------*/

/* GetPerpendicularUnit, GetCylinderSupport and GetConeSupport are in gucefGEOMETRY_SupportFunctions.h, shared with the GetSupport members of the shapes */

/*-------------------------------------------------------------------------*/

/**
 *  The solid cylinder as a support mapping: the cap centre on the side of the direction plus the radius along the
 *  part of the direction perpendicular to the axis. The axis is normalised once per adaptor, and the perpendicular
 *  part is projected twice, so the support stays on the cylinder also for directions nearly parallel to the axis.
 */
template < typename T >
class CTCylinderSupport
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;

    TPoint start;
    TPoint end;
    TVector axis;
    T radius;

    CTCylinderSupport( const TPoint& startPoint, const TPoint& endPoint, const T& cylinderRadius ) GUCEF_GEOMETRY_REALTIME_CONTEXT
        : start( startPoint ), end( endPoint ), axis(), radius( cylinderRadius )
    {
        TVector unit;
        if ( ( end - start ).TryNormalize( unit, TTraits::Zero() ) )
            axis = unit;
    }

    GUCEF_GEOMETRY_PRECISE_INLINE TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return GetCylinderSupport( start, end, axis, radius, direction );
    }
    GUCEF_GEOMETRY_FORCE_INLINE T GetMargin( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TTraits::Zero(); }
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetAnyPoint( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return start; }
};

/*-------------------------------------------------------------------------*/

/** The solid cone as a support mapping: the apex or the rim point along the perpendicular part of the direction (see CTCylinderSupport) */
template < typename T >
class CTConeSupport
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;

    TPoint apex;
    TPoint base;
    TVector axis;
    T baseRadius;

    CTConeSupport( const TPoint& apexPoint, const TVector& unitAxis, const T& height, const T& radius ) GUCEF_GEOMETRY_REALTIME_CONTEXT
        : apex( apexPoint ), base( apexPoint + unitAxis * height ), axis( unitAxis ), baseRadius( radius )
    {
    }

    GUCEF_GEOMETRY_PRECISE_INLINE TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return GetConeSupport( apex, base, axis, baseRadius, direction );
    }
    GUCEF_GEOMETRY_FORCE_INLINE T GetMargin( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TTraits::Zero(); }
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetAnyPoint( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return apex; }
};

/*-------------------------------------------------------------------------*/

/** A core grown by a radius: the margin adds to the margin of the core (a negative radius is treated as 0) */
template < typename TCore >
class CTMarginSupport
{
    public:

    typedef typename TCore::TValue TValue;
    typedef MATH::CTNumericTraits< TValue > TTraits;
    typedef MATH::CTPoint3D< TValue > TPoint;
    typedef MATH::CTVector3D< TValue > TVector;

    TCore core;
    TValue margin;

    GUCEF_GEOMETRY_FORCE_INLINE CTMarginSupport( const TCore& coreSupport, const TValue& radius )
        : core( coreSupport ), margin( TTraits::IsNegative( radius ) ? TTraits::Zero() : radius )
    {
    }

    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return core.GetSupport( direction ); }
    GUCEF_GEOMETRY_FORCE_INLINE TValue GetMargin( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return core.GetMargin() + margin; }
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetAnyPoint( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return core.GetAnyPoint(); }
};

/*-------------------------------------------------------------------------*/

/**
 *  A support mapping moved by an affine map x -> linear * x + translation. The core is mapped exactly for every
 *  linear part; the margin is kept as it is, which is exact for rigid maps (rotations, reflections), the case of
 *  placed shapes and moving bodies.
 */
template < typename TSupport >
class CTTransformedSupport
{
    public:

    typedef typename TSupport::TValue TValue;
    typedef MATH::CTNumericTraits< TValue > TTraits;
    typedef MATH::CTPoint3D< TValue > TPoint;
    typedef MATH::CTVector3D< TValue > TVector;
    typedef MATH::CTMatrix3D< TValue > TMatrix;

    TSupport inner;
    TMatrix linear;
    TMatrix linearTransposed;
    TVector translation;

    GUCEF_GEOMETRY_FORCE_INLINE CTTransformedSupport( const TSupport& support, const TMatrix& linearPart, const TVector& translationPart )
        : inner( support ), linear( linearPart ), linearTransposed( linearPart.Transposed() ), translation( translationPart )
    {
    }

    /** The rigid placement: rotation (a unit quaternion) then the move to 'position' */
    CTTransformedSupport( const TSupport& support, const MATH::CTQuaternion< TValue >& rotation, const TPoint& position ) GUCEF_GEOMETRY_REALTIME_CONTEXT
        : inner( support ), linear( rotation.ToRotationMatrix3D() ), linearTransposed(), translation( position.ToPositionVector() )
    {
        linearTransposed = linear.Transposed();
    }

    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        const TPoint local = inner.GetSupport( linearTransposed * direction );
        return TPoint( linear * local.ToPositionVector() + translation );
    }
    GUCEF_GEOMETRY_FORCE_INLINE TValue GetMargin( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return inner.GetMargin(); }
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetAnyPoint( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TPoint( linear * inner.GetAnyPoint().ToPositionVector() + translation );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  The Minkowski difference A - B = { a - b }, as points: support( d ) = supportA( d ) - supportB( -d ); the margin is
 *  the sum of the margins. The shapes overlap exactly when the difference contains the origin.
 */
template < typename TA, typename TB >
class CTMinkowskiDifference
{
    public:

    typedef typename TA::TValue TValue;
    typedef MATH::CTNumericTraits< TValue > TTraits;
    typedef MATH::CTPoint3D< TValue > TPoint;
    typedef MATH::CTVector3D< TValue > TVector;

    TA a;
    TB b;

    GUCEF_GEOMETRY_FORCE_INLINE CTMinkowskiDifference( const TA& first, const TB& second ) : a( first ), b( second ) {}

    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TPoint( a.GetSupport( direction ) - b.GetSupport( -direction ) );
    }
    GUCEF_GEOMETRY_FORCE_INLINE TValue GetMargin( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return a.GetMargin() + b.GetMargin(); }
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetAnyPoint( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TPoint( a.GetAnyPoint() - b.GetAnyPoint() ); }
};

/*-------------------------------------------------------------------------*/

/** A support mapping swept along a translation: the convex hull of the shape at the start and at start + sweep */
template < typename TSupport >
class CTSweptSupport
{
    public:

    typedef typename TSupport::TValue TValue;
    typedef MATH::CTNumericTraits< TValue > TTraits;
    typedef MATH::CTPoint3D< TValue > TPoint;
    typedef MATH::CTVector3D< TValue > TVector;

    TSupport inner;
    TVector sweep;

    GUCEF_GEOMETRY_FORCE_INLINE CTSweptSupport( const TSupport& support, const TVector& translation ) : inner( support ), sweep( translation ) {}

    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        const TPoint p = inner.GetSupport( direction );
        return TTraits::Zero() < sweep.Dot( direction ) ? p + sweep : p;
    }
    GUCEF_GEOMETRY_FORCE_INLINE TValue GetMargin( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return inner.GetMargin(); }
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetAnyPoint( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return inner.GetAnyPoint(); }
};

/*-------------------------------------------------------------------------*/

/**
 *  The support mapping of a shape of the library: typedef TSupport and static TSupport Make( const TShape& ). The
 *  primary template takes the shape's own GetSupport without margin; points, spheres and capsules are specialised to
 *  a core plus margin.
 */
template < typename TShape >
class CTSupportMapping
{
    public:
    typedef CTShapeSupport< TShape > TSupport;
    static GUCEF_GEOMETRY_FORCE_INLINE TSupport Make( const TShape& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return TSupport( shape ); }
};

template < typename T >
class CTSupportMapping< MATH::CTPoint3D< T > >
{
    public:
    typedef CTPointSupport< T > TSupport;
    static GUCEF_GEOMETRY_FORCE_INLINE TSupport Make( const MATH::CTPoint3D< T >& point ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return TSupport( point ); }
};

template < typename T >
class CTSupportMapping< CTSegment3D< T > >
{
    public:
    typedef CTSegmentSupport< T > TSupport;
    static GUCEF_GEOMETRY_FORCE_INLINE TSupport Make( const CTSegment3D< T >& segment ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return TSupport( segment.start, segment.end ); }
};

template < typename T >
class CTSupportMapping< CTSphere< T > >
{
    public:
    typedef CTMarginSupport< CTPointSupport< T > > TSupport;
    static GUCEF_GEOMETRY_FORCE_INLINE TSupport Make( const CTSphere< T >& sphere ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return TSupport( CTPointSupport< T >( sphere.centre ), sphere.radius ); }
};

template < typename T >
class CTSupportMapping< CTCapsule< T > >
{
    public:
    typedef CTMarginSupport< CTSegmentSupport< T > > TSupport;
    static GUCEF_GEOMETRY_FORCE_INLINE TSupport Make( const CTCapsule< T >& capsule ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return TSupport( CTSegmentSupport< T >( capsule.start, capsule.end ), capsule.radius ); }
};

template < typename T >
class CTSupportMapping< CTCylinder< T > >
{
    public:
    typedef CTCylinderSupport< T > TSupport;
    static GUCEF_GEOMETRY_FORCE_INLINE TSupport Make( const CTCylinder< T >& cylinder ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return TSupport( cylinder.start, cylinder.end, cylinder.radius ); }
};

template < typename T >
class CTSupportMapping< CTCone< T > >
{
    public:
    typedef CTConeSupport< T > TSupport;
    static GUCEF_GEOMETRY_FORCE_INLINE TSupport Make( const CTCone< T >& cone ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return TSupport( cone.apex, cone.axis.AsVector(), cone.height, cone.baseRadius ); }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The support of the whole shape (core grown by the margin): the core support plus the margin along the unit
 *  direction. A zero direction gives the core support moved by the margin along +X.
 */
template < typename TSupport >
GUCEF_GEOMETRY_PRECISE_INLINE MATH::CTPoint3D< typename TSupport::TValue >
GetSupportWithMargin( const TSupport& support, const MATH::CTVector3D< typename TSupport::TValue >& direction ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef typename TSupport::TValue T;
    typedef MATH::CTNumericTraits< T > TTraits;
    const MATH::CTPoint3D< T > core = support.GetSupport( direction );
    const T margin = support.GetMargin();
    if ( TTraits::Zero() == margin )
        return core;
    MATH::CTDirectionVector3D< T > unit;
    if ( !MATH::CTDirectionVector3D< T >::TryCreate( direction, unit ) )
        return core + MATH::CTVector3D< T >( margin, TTraits::Zero(), TTraits::Zero() );
    return core + unit * margin;
}

/*-------------------------------------------------------------------------*/

/**
 *  The axis aligned bounds of the whole shape (core grown by the margin) from its supports along the six axis
 *  directions: exact for convex shapes.
 */
template < typename TSupport >
GUCEF_GEOMETRY_PRECISE_INLINE CTAxisAlignedBox3D< typename TSupport::TValue >
GetSupportBounds( const TSupport& support ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef typename TSupport::TValue T;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector3D< T > TVector;
    const T one = TTraits::One();
    const T zero = TTraits::Zero();
    const T margin = support.GetMargin();
    const T maxX = support.GetSupport( TVector( one, zero, zero ) ).x + margin;
    const T maxY = support.GetSupport( TVector( zero, one, zero ) ).y + margin;
    const T maxZ = support.GetSupport( TVector( zero, zero, one ) ).z + margin;
    const T minX = support.GetSupport( TVector( -one, zero, zero ) ).x - margin;
    const T minY = support.GetSupport( TVector( zero, -one, zero ) ).y - margin;
    const T minZ = support.GetSupport( TVector( zero, zero, -one ) ).z - margin;
    return CTAxisAlignedBox3D< T >( MATH::CTPoint3D< T >( minX, minY, minZ ), MATH::CTPoint3D< T >( maxX, maxY, maxZ ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_SUPPORTMAPPINGS_H ? */
