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

#ifndef GUCEF_GEOMETRY_CTMOTION3D_H
#define GUCEF_GEOMETRY_CTMOTION3D_H

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

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

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
 *  The rigid motion of a body over one time interval, the input of time of impact (section 7.5.4 of the design): a
 *  start placement (position and rotation) and a constant linear and angular velocity, both per interval: time runs
 *  from 0 to 1 over the interval, so linearVelocity is the displacement over the whole interval and angularVelocity
 *  the rotation vector (axis times angle in radians, world frame) over the whole interval.
 *
 *  - Placement at time t: position + linearVelocity * t, and the rotation by angularVelocity * t (about the body's
 *    position, in the world frame) applied after startRotation. A shape given in the body frame is placed by rotating
 *    it with the rotation at t and moving it to the position at t.
 *  - GetMaxDisplacement( radius ) bounds how far any point within 'radius' of the body position moves during the
 *    interval: |linearVelocity| + |angularVelocity| * radius (the arc never exceeds that length), the bound
 *    conservative advancement relies on.
 *  - Kind: floats and fixed point (Float32, Float64 and TFixed32_32 are exported); needs transcendental functions,
 *    which fixed point evaluates in integer arithmetic (bit-exact).
 *  - Layout: a point, a quaternion and two vectors, 13 * sizeof( T ), no virtual functions. The default motion rests
 *    at the origin without rotation.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text: "Motion3D(position (x, y, z), rotation (x, y, z, w), linear (x, y, z), angular (x, y, z))"; DataNode:
 *    child nodes position, rotation, linear and angular.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTMotion3D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTQuaternion< T > TRotation;

    TPoint startPosition;
    TRotation startRotation;
    TVector linearVelocity;
    TVector angularVelocity;

    GUCEF_GEOMETRY_FORCE_INLINE CTMotion3D( void ) : startPosition(), startRotation(), linearVelocity(), angularVelocity() {}

    GUCEF_GEOMETRY_FORCE_INLINE CTMotion3D( const TPoint& position, const TRotation& rotation, const TVector& linear, const TVector& angular )
        : startPosition( position ), startRotation( rotation ), linearVelocity( linear ), angularVelocity( angular )
    {
    }

    /** A body at rest at the placement */
    static GUCEF_GEOMETRY_FORCE_INLINE CTMotion3D AtRest( const TPoint& position, const TRotation& rotation ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return CTMotion3D( position, rotation, TVector(), TVector() );
    }

    /**
     *  The motion that takes the start placement to the end placement over the interval: the linear velocity is the
     *  displacement, the angular velocity the shortest rotation from the start to the end rotation.
     */
    static CTMotion3D FromPlacements( const TPoint& startPoint, const TRotation& startOrientation, const TPoint& endPoint, const TRotation& endOrientation ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The position at time t (t is a fraction of the interval) */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetPosition( const T& t ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return startPosition + linearVelocity * t; }

    /** The rotation at time t: the unit quaternion of angularVelocity * t applied after startRotation */
    TRotation GetRotation( const T& t ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** True when both velocities are zero */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsAtRest( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return linearVelocity.IsZero() && angularVelocity.IsZero(); }

    /** The longest path over the interval of a point within 'radius' (>= 0) of the body position */
    T GetMaxDisplacement( const T& radius ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTMotion3D& a, const CTMotion3D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return a.startPosition == b.startPosition && a.startRotation == b.startRotation && a.linearVelocity == b.linearVelocity && a.angularVelocity == b.angularVelocity;
    }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTMotion3D& a, const CTMotion3D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    /** "Motion3D(position (x, y, z), rotation (x, y, z, w), linear (x, y, z), angular (x, y, z))" */
    CORE::CString ToString( void ) const
    {
        return "Motion3D(position " + startPosition.ToString() + ", rotation " + startRotation.ToString() + ", linear " + linearVelocity.ToString() +
               ", angular " + angularVelocity.ToString() + ")";
    }

    /** Parses the ToString form; false leaves this motion unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint position;
        TRotation rotation;
        TVector linear;
        TVector angular;
        if ( !reader.TryLiteral( "Motion3D" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "position" ) || !reader.TryValue( position ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "rotation" ) || !reader.TryValue( rotation ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "linear" ) || !reader.TryValue( linear ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "angular" ) || !reader.TryValue( angular ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        *this = CTMotion3D( position, rotation, linear, angular );
        return true;
    }

    /** Writes the child nodes position, rotation, linear and angular into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "position", startPosition, settings ) && SerializeShapeChild( node, "rotation", startRotation, settings ) &&
               SerializeShapeChild( node, "linear", linearVelocity, settings ) && SerializeShapeChild( node, "angular", angularVelocity, settings );
    }

    /** Reads the child nodes position, rotation, linear and angular; a missing or invalid one fails and leaves this motion unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TPoint position;
        TRotation rotation;
        TVector linear;
        TVector angular;
        if ( !DeserializeShapeChild( node, "position", position, settings ) || !DeserializeShapeChild( node, "rotation", rotation, settings ) ||
             !DeserializeShapeChild( node, "linear", linear, settings ) || !DeserializeShapeChild( node, "angular", angular, settings ) )
            return false;
        *this = CTMotion3D( position, rotation, linear, angular );
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTMotion3D< T >
CTMotion3D< T >::FromPlacements( const TPoint& startPoint, const TRotation& startOrientation, const TPoint& endPoint, const TRotation& endOrientation ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TRotation delta = endOrientation * startOrientation.Conjugate();
    if ( TTraits::IsNegative( delta.w ) )
        delta = -delta;
    MATH::CTDirectionVector3D< T > axis;
    MATH::CTRadian< T > angle;
    delta.Normalized().ToAxisAngle( axis, angle );
    return CTMotion3D( startPoint, startOrientation, endPoint - startPoint, axis * angle.GetValue() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTMotion3D< T >::TRotation
CTMotion3D< T >::GetRotation( const T& t ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TVector rotationVector = angularVelocity * t;
    MATH::CTDirectionVector3D< T > axis;
    if ( !MATH::CTDirectionVector3D< T >::TryCreate( rotationVector, axis ) )
        return startRotation;
    const TRotation step = TRotation::FromAxisAngle( axis, MATH::CTRadian< T >( rotationVector.GetLength() ) );
    return ( step * startRotation ).Normalized();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTMotion3D< T >::GetMaxDisplacement( const T& radius ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return linearVelocity.GetLength() + angularVelocity.GetLength() * TTraits::Abs( radius );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTMotion3D< Float32 > TFloat32Motion3D;
typedef CTMotion3D< Float64 > TFloat64Motion3D;
typedef CTMotion3D< MATH::TFixed32_32 > TFixed32_32Motion3D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTMotion3D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTMotion3D< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTMotion3D< MATH::TFixed32_32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTMOTION3D_H ? */
