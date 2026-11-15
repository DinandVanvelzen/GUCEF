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

#ifndef GUCEF_MATH_CTCURVEPOINTTRAITS_H
#define GUCEF_MATH_CTCURVEPOINTTRAITS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <cmath>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#include "gucefCORE_CDataNodeSerializableSettings.h"
#define GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#endif /* GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the curves, splines, paths, smoothing functions and integrators of gucefMATH need to know about the type of
 *  the things they move: the curve classes are written once over a point type TPoint and use only this trait.
 *
 *  - TPoint is a position: a floating point scalar (a 1D curve), CTVector2D / 3D / 4D (a curve through vector space) or
 *    CTPoint2D / 3D (a curve through affine space). TVector is the matching displacement: the type itself for scalars
 *    and vectors, CTVectorND for points. TScalar is the floating point kind of the components.
 *  - The affine rules are kept: Sub( a, b ) of two points gives a vector, SubVectors( v, w ) subtracts two vectors,
 *    Offset( p, v ) moves a point; velocities,
 *    tangents and derivatives are always TVector, so a point is never added to a point.
 *  - Only the floating point kinds Float32 and Float64 are supported; any other kind does not compile (the named
 *    static assertion is reported).
 *  - All functions are static, stateless and realtime-safe. Vector products and lengths go through the members of the
 *    exported vector classes.
 *
 *  A user type becomes usable by specialising this trait with the same members.
 */
template < typename TPoint >
struct CTCurvePointTraits;

/*-------------------------------------------------------------------------*/

/** Scalars: a curve of one dimension */
template < typename T >
struct CTCurveScalarTraits
{
    typedef T TScalar;
    typedef T TPoint;
    typedef T TVector;

    enum { ComponentCount = 1 };

    static GUCEF_MATH_FORCE_INLINE T GetComponent( const T& p, UInt32 ) GUCEF_MATH_REALTIME_CONTEXT { return p; }
    static GUCEF_MATH_FORCE_INLINE void SetComponent( T& p, UInt32, const T& value ) GUCEF_MATH_REALTIME_CONTEXT { p = value; }
    static GUCEF_MATH_FORCE_INLINE T GetVectorComponent( const T& v, UInt32 ) GUCEF_MATH_REALTIME_CONTEXT { return v; }
    static GUCEF_MATH_FORCE_INLINE T Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return T( 0 ); }
    static GUCEF_MATH_FORCE_INLINE T Sub( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return a - b; }
    static GUCEF_MATH_FORCE_INLINE T Offset( const T& p, const T& v ) GUCEF_MATH_REALTIME_CONTEXT { return p + v; }
    static GUCEF_MATH_FORCE_INLINE T Add( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return a + b; }
    static GUCEF_MATH_FORCE_INLINE T SubVectors( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return a - b; }
    static GUCEF_MATH_FORCE_INLINE T Negate( const T& v ) GUCEF_MATH_REALTIME_CONTEXT { return -v; }
    static GUCEF_MATH_FORCE_INLINE T Scale( const T& v, const T& s ) GUCEF_MATH_REALTIME_CONTEXT { return v * s; }
    static GUCEF_MATH_FORCE_INLINE T Dot( const T& a, const T& b ) GUCEF_MATH_REALTIME_CONTEXT { return a * b; }
    static GUCEF_MATH_FORCE_INLINE T Length( const T& v ) GUCEF_MATH_REALTIME_CONTEXT { return v < T( 0 ) ? -v : v; }
};

template <> struct CTCurvePointTraits< Float32 > : public CTCurveScalarTraits< Float32 > {};
template <> struct CTCurvePointTraits< Float64 > : public CTCurveScalarTraits< Float64 > {};

/*-------------------------------------------------------------------------*/

/** Vector types: positions and displacements are the same type */
template < typename TVec, UInt32 N >
struct CTCurveVectorTraits
{
    typedef typename TVec::TValue TScalar;
    typedef TVec TPoint;
    typedef TVec TVector;

    enum { ComponentCount = N };

    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< TScalar >::IsFloatingPoint, Curves_require_a_floating_point_kind );

    static GUCEF_MATH_FORCE_INLINE TScalar GetComponent( const TVec& p, UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT { return p.GetComponent( index ); }
    static GUCEF_MATH_FORCE_INLINE void SetComponent( TVec& p, UInt32 index, const TScalar& value ) GUCEF_MATH_REALTIME_CONTEXT { p.SetComponent( index, value ); }
    static GUCEF_MATH_FORCE_INLINE TScalar GetVectorComponent( const TVec& v, UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT { return v.GetComponent( index ); }
    static GUCEF_MATH_FORCE_INLINE TVec Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return TVec(); }
    static GUCEF_MATH_FORCE_INLINE TVec Sub( const TVec& a, const TVec& b ) GUCEF_MATH_REALTIME_CONTEXT { return a - b; }
    static GUCEF_MATH_FORCE_INLINE TVec Offset( const TVec& p, const TVec& v ) GUCEF_MATH_REALTIME_CONTEXT { return p + v; }
    static GUCEF_MATH_FORCE_INLINE TVec Add( const TVec& a, const TVec& b ) GUCEF_MATH_REALTIME_CONTEXT { return a + b; }
    static GUCEF_MATH_FORCE_INLINE TVec SubVectors( const TVec& a, const TVec& b ) GUCEF_MATH_REALTIME_CONTEXT { return a - b; }
    static GUCEF_MATH_FORCE_INLINE TVec Negate( const TVec& v ) GUCEF_MATH_REALTIME_CONTEXT { return -v; }
    static GUCEF_MATH_FORCE_INLINE TVec Scale( const TVec& v, const TScalar& s ) GUCEF_MATH_REALTIME_CONTEXT { return v * s; }
    static GUCEF_MATH_FORCE_INLINE TScalar Dot( const TVec& a, const TVec& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.Dot( b ); }
    static GUCEF_MATH_FORCE_INLINE TScalar Length( const TVec& v ) GUCEF_MATH_REALTIME_CONTEXT { return v.GetLength(); }
};

template < typename T > struct CTCurvePointTraits< CTVector2D< T > > : public CTCurveVectorTraits< CTVector2D< T >, 2 > {};
template < typename T > struct CTCurvePointTraits< CTVector3D< T > > : public CTCurveVectorTraits< CTVector3D< T >, 3 > {};
template < typename T > struct CTCurvePointTraits< CTVector4D< T > > : public CTCurveVectorTraits< CTVector4D< T >, 4 > {};

/*-------------------------------------------------------------------------*/

/** Point types: positions are points, displacements are the vector class of the point class */
template < typename TPnt, UInt32 N >
struct CTCurveAffineTraits
{
    typedef typename TPnt::TValue TScalar;
    typedef TPnt TPoint;
    typedef typename TPnt::TVector TVector;

    enum { ComponentCount = N };

    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< TScalar >::IsFloatingPoint, Curves_require_a_floating_point_kind );

    static GUCEF_MATH_FORCE_INLINE TScalar GetComponent( const TPnt& p, UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT { return p.GetComponent( index ); }
    static GUCEF_MATH_FORCE_INLINE void SetComponent( TPnt& p, UInt32 index, const TScalar& value ) GUCEF_MATH_REALTIME_CONTEXT { p.SetComponent( index, value ); }
    static GUCEF_MATH_FORCE_INLINE TScalar GetVectorComponent( const TVector& v, UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT { return v.GetComponent( index ); }
    static GUCEF_MATH_FORCE_INLINE TVector Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return TVector(); }
    static GUCEF_MATH_FORCE_INLINE TVector Sub( const TPnt& a, const TPnt& b ) GUCEF_MATH_REALTIME_CONTEXT { return a - b; }
    static GUCEF_MATH_FORCE_INLINE TPnt Offset( const TPnt& p, const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT { return p + v; }
    static GUCEF_MATH_FORCE_INLINE TVector Add( const TVector& a, const TVector& b ) GUCEF_MATH_REALTIME_CONTEXT { return a + b; }
    static GUCEF_MATH_FORCE_INLINE TVector SubVectors( const TVector& a, const TVector& b ) GUCEF_MATH_REALTIME_CONTEXT { return a - b; }
    static GUCEF_MATH_FORCE_INLINE TVector Negate( const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT { return -v; }
    static GUCEF_MATH_FORCE_INLINE TVector Scale( const TVector& v, const TScalar& s ) GUCEF_MATH_REALTIME_CONTEXT { return v * s; }
    static GUCEF_MATH_FORCE_INLINE TScalar Dot( const TVector& a, const TVector& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.Dot( b ); }
    static GUCEF_MATH_FORCE_INLINE TScalar Length( const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT { return v.GetLength(); }
};

template < typename T > struct CTCurvePointTraits< CTPoint2D< T > > : public CTCurveAffineTraits< CTPoint2D< T >, 2 > {};
template < typename T > struct CTCurvePointTraits< CTPoint3D< T > > : public CTCurveAffineTraits< CTPoint3D< T >, 3 > {};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** a + ( b - a ) t on the point type of the trait; exact at t = 0 */
template < typename TPoint >
GUCEF_MATH_FORCE_INLINE TPoint
CurveLerp( const TPoint& a, const TPoint& b, const typename CTCurvePointTraits< TPoint >::TScalar& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTCurvePointTraits< TPoint > TTraits;
    return TTraits::Offset( a, TTraits::Scale( TTraits::Sub( b, a ), t ) );
}

/*-------------------------------------------------------------------------*/

/*
 *  DataNode forms of the values of curves (section 9.2 of the design). A scalar is written as the attribute 'value',
 *  vectors and points with their own Serialize (attributes x, y, z, w). Lists are written as child nodes of one name
 *  and read back from the children of that name; a read goes into a temporary and replaces the target only on
 *  success. Not realtime-safe.
 */

inline bool
CurveValueSerialize( CORE::CDataNode& node, const Float32& value, const CORE::CDataNodeSerializableSettings& )
{
    const char* const names[ 1 ] = { "value" };
    const Float32 components[ 1 ] = { value };
    return SerializeComponents( node, names, components );
}

inline bool
CurveValueSerialize( CORE::CDataNode& node, const Float64& value, const CORE::CDataNodeSerializableSettings& )
{
    const char* const names[ 1 ] = { "value" };
    const Float64 components[ 1 ] = { value };
    return SerializeComponents( node, names, components );
}

template < typename TValue >
bool
CurveValueSerialize( CORE::CDataNode& node, const TValue& value, const CORE::CDataNodeSerializableSettings& settings )
{
    return value.Serialize( node, settings );
}

inline bool
CurveValueDeserialize( const CORE::CDataNode& node, Float32& value, const CORE::CDataNodeSerializableSettings& )
{
    const char* const names[ 1 ] = { "value" };
    Float32 components[ 1 ];
    if ( !DeserializeComponents( node, names, components ) )
        return false;
    value = components[ 0 ];
    return true;
}

inline bool
CurveValueDeserialize( const CORE::CDataNode& node, Float64& value, const CORE::CDataNodeSerializableSettings& )
{
    const char* const names[ 1 ] = { "value" };
    Float64 components[ 1 ];
    if ( !DeserializeComponents( node, names, components ) )
        return false;
    value = components[ 0 ];
    return true;
}

template < typename TValue >
bool
CurveValueDeserialize( const CORE::CDataNode& node, TValue& value, const CORE::CDataNodeSerializableSettings& settings )
{
    return value.Deserialize( node, settings );
}

/** One child node 'childName' per value */
template < typename TValue >
bool
CurveSerializeList( CORE::CDataNode& parent, const char* childName, const GUCEF::vector< TValue >& values, const CORE::CDataNodeSerializableSettings& settings )
{
    for ( size_t i=0; i<values.size(); ++i )
    {
        CORE::CDataNode* child = parent.AddChild( CORE::CString( childName ) );
        if ( GUCEF_NULL == child || !CurveValueSerialize( *child, values[ i ], settings ) )
            return false;
    }
    return true;
}

/** Reads every child named 'childName'; no such child gives an empty list */
template < typename TValue >
bool
CurveDeserializeList( const CORE::CDataNode& parent, const char* childName, GUCEF::vector< TValue >& values, const CORE::CDataNodeSerializableSettings& settings )
{
    GUCEF::vector< TValue > parsed;
    const CORE::CString name( childName );
    for ( const CORE::CDataNode* child = parent.GetFirstChild(); GUCEF_NULL != child; child = child->GetNext() )
    {
        if ( child->GetName() != name )
            continue;
        TValue value;
        if ( !CurveValueDeserialize( *child, value, settings ) )
            return false;
        parsed.push_back( value );
    }
    values.swap( parsed );
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The point types for which the curve class templates are explicitly instantiated inside the module (section 5.10 of
 *  the design): scalars, 2D and 3D vectors and 2D and 3D points of both float kinds. X( type ) is invoked per type;
 *  the names are typedefs so they contain no comma.
 */
#define GUCEF_MATH_CURVE_PREDEFINED_POINTS( X )                                  \
    X( Float32 )            X( Float64 )                                         \
    X( TFloat32Vector2D )   X( TFloat64Vector2D )                                \
    X( TFloat32Vector3D )   X( TFloat64Vector3D )                                \
    X( TFloat32Point2D )    X( TFloat64Point2D )                                 \
    X( TFloat32Point3D )    X( TFloat64Point3D )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTCURVEPOINTTRAITS_H ? */
