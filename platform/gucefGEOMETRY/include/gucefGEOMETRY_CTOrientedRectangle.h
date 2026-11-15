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

#ifndef GUCEF_GEOMETRY_CTORIENTEDRECTANGLE_H
#define GUCEF_GEOMETRY_CTORIENTEDRECTANGLE_H

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

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_GEOMETRY_CTRECTANGLE_H
#include "gucefGEOMETRY_CTRectangle.h"
#define GUCEF_GEOMETRY_CTRECTANGLE_H
#endif /* GUCEF_GEOMETRY_CTRECTANGLE_H ? */

#ifndef GUCEF_GEOMETRY_CTSEGMENT2D_H
#include "gucefGEOMETRY_CTSegment2D.h"
#define GUCEF_GEOMETRY_CTSEGMENT2D_H
#endif /* GUCEF_GEOMETRY_CTSEGMENT2D_H ? */

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
 *  A solid rectangle in 2D (the XY plane of its space) that is rotated about its centre (section 7.2): the centre, the
 *  rotation angle of its X axis (counter-clockwise, zero is along +X) and the half extents along its own axes.
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root and trigonometry.
 *  - Layout: centre, angle and half extents, 5 * sizeof( T ), no virtual functions. The default is the unit square
 *    around the origin (half extents 0.5), not rotated.
 *  - Negative half extents are not valid: such a rectangle contains nothing.
 *  - Realtime: every member except the wireframe, text and DataNode members is realtime-safe.
 *  - Text: "OrientedRectangle(centre (x, y), radians r, halfExtents (x, y))"; DataNode: child nodes centre and halfExtents,
 *    the attribute radians.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTOrientedRectangle
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef MATH::CTVector2D< T > TVector;
    typedef MATH::CTDirectionVector2D< T > TDirection;
    typedef MATH::CTRadian< T > TAngle;

    TPoint centre;
    TAngle rotation;
    TVector halfExtents;

    GUCEF_GEOMETRY_FORCE_INLINE CTOrientedRectangle( void ) : centre(), rotation(), halfExtents( TTraits::Half(), TTraits::Half() ) {}

    GUCEF_GEOMETRY_FORCE_INLINE CTOrientedRectangle( const TPoint& centrePoint, const TAngle& angle, const TVector& halfSize )
        : centre( centrePoint ), rotation( angle ), halfExtents( halfSize )
    {
    }

    /** The direction of the rectangle's own X axis */
    GUCEF_GEOMETRY_FORCE_INLINE TDirection GetAxisX( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TDirection::FromAngle( rotation ); }

    /** The direction of the rectangle's own Y axis: the X axis rotated a quarter turn counter-clockwise */
    GUCEF_GEOMETRY_FORCE_INLINE TDirection GetAxisY( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return GetAxisX().GetPerpendicular(); }

    /** The position of 'point' in the rectangle's own frame: ( ( point - centre ) . axisX, ( point - centre ) . axisY ) */
    TVector ToLocal( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The point at the rectangle frame position 'local' */
    TPoint FromLocal( const TVector& local ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The four corners counter-clockwise, starting at ( -hx, -hy ) of the rectangle frame */
    void GetCorners( TPoint ( &corners )[ 4 ] ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** 4 hx hy; 0 for negative half extents */
    GUCEF_GEOMETRY_FORCE_INLINE T GetArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( TTraits::IsNegative( halfExtents.x ) || TTraits::IsNegative( halfExtents.y ) )
            return TTraits::Zero();
        return ( halfExtents.x * halfExtents.y ) * TTraits::FromInt32( 4 );
    }

    /** True when the point is inside or on the boundary */
    bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The nearest point of the solid: the point itself when inside, otherwise on the boundary */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    GUCEF_GEOMETRY_FORCE_INLINE T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetSquaredLength(); }
    GUCEF_GEOMETRY_FORCE_INLINE T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetLength(); }

    /** The axis aligned rectangle around the four corners */
    CTRectangle< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The corner furthest along 'direction' (the first on ties) */
    TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Appends the four edges counter-clockwise; 'segmentsPerCircle' is not used */
    void GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 segmentsPerCircle ) const;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTOrientedRectangle& a, const CTOrientedRectangle& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return a.centre == b.centre && a.rotation == b.rotation && a.halfExtents == b.halfExtents;
    }

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTOrientedRectangle& a, const CTOrientedRectangle& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTOrientedRectangle& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return centre.Equals( other.centre ) && halfExtents.Equals( other.halfExtents ) &&
               TTraits::EqualsWithin( rotation.GetValue(), other.rotation.GetValue(), TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** "OrientedRectangle(centre (x, y), radians r, halfExtents (x, y))" */
    CORE::CString ToString( void ) const
    {
        return "OrientedRectangle(centre " + centre.ToString() + ", radians " + TTraits::ToString( rotation.GetValue() ) + ", halfExtents " + halfExtents.ToString() + ")";
    }

    /** Parses the ToString form; false leaves this rectangle unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedCentre;
        T parsedRadians;
        TVector parsedHalf;
        if ( !reader.TryLiteral( "OrientedRectangle" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "centre" ) || !reader.TryValue( parsedCentre ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "radians" ) || !reader.TryScalar( parsedRadians ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "halfExtents" ) || !reader.TryValue( parsedHalf ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        centre = parsedCentre;
        rotation = TAngle( parsedRadians );
        halfExtents = parsedHalf;
        return true;
    }

    /** Writes the child nodes centre and halfExtents and the attribute radians into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "centre", centre, settings ) && SerializeShapeScalar( node, "radians", rotation.GetValue() ) &&
               SerializeShapeChild( node, "halfExtents", halfExtents, settings );
    }

    /** Reads the child nodes centre and halfExtents and the attribute radians; a missing or invalid one fails and leaves this rectangle unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TPoint parsedCentre;
        T parsedRadians;
        TVector parsedHalf;
        if ( !DeserializeShapeChild( node, "centre", parsedCentre, settings ) || !DeserializeShapeScalar( node, "radians", parsedRadians ) ||
             !DeserializeShapeChild( node, "halfExtents", parsedHalf, settings ) )
            return false;
        centre = parsedCentre;
        rotation = TAngle( parsedRadians );
        halfExtents = parsedHalf;
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTOrientedRectangle< T >::TVector
CTOrientedRectangle< T >::ToLocal( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TDirection axisX = GetAxisX();
    const TDirection axisY = axisX.GetPerpendicular();
    const TVector d = point - centre;
    return TVector( axisX.Dot( d ), axisY.Dot( d ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTOrientedRectangle< T >::TPoint
CTOrientedRectangle< T >::FromLocal( const TVector& local ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TDirection axisX = GetAxisX();
    const TDirection axisY = axisX.GetPerpendicular();
    return centre + axisX * local.x + axisY * local.y;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTOrientedRectangle< T >::GetCorners( TPoint ( &corners )[ 4 ] ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TDirection axisX = GetAxisX();
    const TDirection axisY = axisX.GetPerpendicular();
    const TVector ex = axisX * halfExtents.x;
    const TVector ey = axisY * halfExtents.y;
    corners[ 0 ] = centre - ex - ey;
    corners[ 1 ] = centre + ex - ey;
    corners[ 2 ] = centre + ex + ey;
    corners[ 3 ] = centre - ex + ey;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTOrientedRectangle< T >::Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( TTraits::IsNegative( halfExtents.x ) || TTraits::IsNegative( halfExtents.y ) )
        return false;
    const TVector local = ToLocal( point );
    return TTraits::Abs( local.x ) <= halfExtents.x && TTraits::Abs( local.y ) <= halfExtents.y;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTOrientedRectangle< T >::TPoint
CTOrientedRectangle< T >::ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( Contains( point ) )
        return point;
    if ( TTraits::IsNegative( halfExtents.x ) || TTraits::IsNegative( halfExtents.y ) )
        return point;
    const TVector local = ToLocal( point );
    const TVector clamped( TTraits::Max( TTraits::Min( local.x, halfExtents.x ), MATH::ComponentNegate( halfExtents.x ) ),
                           TTraits::Max( TTraits::Min( local.y, halfExtents.y ), MATH::ComponentNegate( halfExtents.y ) ) );
    return FromLocal( clamped );
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTRectangle< T >
CTOrientedRectangle< T >::GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TPoint corners[ 4 ];
    GetCorners( corners );
    return CTRectangle< T >::FromPoints( corners, 4 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTOrientedRectangle< T >::TPoint
CTOrientedRectangle< T >::GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TPoint corners[ 4 ];
    GetCorners( corners );
    UInt32 best = 0;
    T bestDot = corners[ 0 ].ToPositionVector().Dot( direction );
    for ( UInt32 i=1; i<4; ++i )
    {
        T d = corners[ i ].ToPositionVector().Dot( direction );
        if ( bestDot < d )
        {
            bestDot = d;
            best = i;
        }
    }
    return corners[ best ];
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTOrientedRectangle< T >::GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 /*segmentsPerCircle*/ ) const
{
    TPoint corners[ 4 ];
    GetCorners( corners );
    for ( UInt32 i=0; i<4; ++i )
        lines.push_back( CTSegment2D< T >( corners[ i ], corners[ ( i + 1 ) % 4 ] ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTOrientedRectangle< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTOrientedRectangle< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTORIENTEDRECTANGLE_H ? */
