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

#ifndef GUCEF_MATH_CTBEZIERCURVE2_H
#define GUCEF_MATH_CTBEZIERCURVE2_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTCURVEPOINTTRAITS_H
#include "gucefMATH_CTCurvePointTraits.h"
#define GUCEF_MATH_CTCURVEPOINTTRAITS_H
#endif /* GUCEF_MATH_CTCURVEPOINTTRAITS_H ? */

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
 *  A quadratic Bezier curve with 3 control points over the parameter [ 0, 1 ]: it starts at the first and ends at the
 *  last control point and is tangent to the first and last control polygon edge there.
 *
 *  - TPoint: Float32 / Float64 (1D), CTVector2D / 3D / 4D or CTPoint2D / 3D of those kinds (CTCurvePointTraits); any
 *    other kind does not compile (named static assertion).
 *  - Layout: exactly the 3 control points p0 .. p2, public, no virtual functions, no heap use.
 *  - Queries are realtime-safe. Evaluate() is exact at t = 0 and t = 1 and extrapolates the polynomial outside [ 0, 1 ].
 *  - The affine rules hold: points are only combined through differences, so a CTPoint curve never adds points.
 *  - DataNode: a child node 'point' per control point (attributes of the point type); no text form.
 *  - Curve interface used by CTArcLengthTable and the curve queries: TTraits, TVector, TScalar, Evaluate,
 *    GetDerivative, GetMinParameter, GetMaxParameter.
 */
template < typename TPoint >
class GUCEF_MATH_TEMPLATE_PUBLIC CTBezierCurve2
{
    public:

    typedef CTCurvePointTraits< TPoint > TTraits;
    typedef typename TTraits::TScalar TScalar;
    typedef typename TTraits::TVector TVector;

    TPoint p0;
    TPoint p1;
    TPoint p2;

    /** All control points at the default point */
    CTBezierCurve2( void ) : p0(), p1(), p2() {}
    CTBezierCurve2( const TPoint& a, const TPoint& b, const TPoint& c ) : p0( a ), p1( b ), p2( c ) {}

    GUCEF_MATH_FORCE_INLINE TScalar GetMinParameter( void ) const GUCEF_MATH_REALTIME_CONTEXT { return TScalar( 0 ); }
    GUCEF_MATH_FORCE_INLINE TScalar GetMaxParameter( void ) const GUCEF_MATH_REALTIME_CONTEXT { return TScalar( 1 ); }

    /** The point at parameter t */
    TPoint Evaluate( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** dP/dt at parameter t */
    TVector GetDerivative( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** d2P/dt2 at parameter t */
    TVector GetSecondDerivative( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Splits at parameter t with de Casteljau's algorithm: 'left' covers [ 0, t ] and 'right' [ t, 1 ] of this curve,
     *  each reparametrised to [ 0, 1 ]. Either output may be this object.
     */
    void Split( const TScalar& t, CTBezierCurve2& left, CTBezierCurve2& right ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The tight axis-aligned bounds of the curve (not of the control polygon): the end points and the extrema of every
     *  component, found from the roots of the derivative.
     */
    void GetBounds( TPoint& minimum, TPoint& maximum ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Writes a child node 'point' per control point (not realtime-safe) */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const;

    /** Reads what Serialize writes; any error (including a wrong point count) leaves the curve unchanged (not realtime-safe) */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings );
};

/*-------------------------------------------------------------------------*/

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE TPoint
CTBezierCurve2< TPoint >::Evaluate( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( t == TScalar( 0 ) ) return p0;
    if ( t == TScalar( 1 ) ) return p2;
    const TScalar s = TScalar( 1 ) - t;
    TVector sum = TTraits::Scale( TTraits::Sub( p1, p0 ), TScalar( 2 ) * s * t );
    sum = TTraits::Add( sum, TTraits::Scale( TTraits::Sub( p2, p0 ), t * t ) );
    return TTraits::Offset( p0, sum );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTBezierCurve2< TPoint >::TVector
CTBezierCurve2< TPoint >::GetDerivative( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const TVector sum = TTraits::Add( TTraits::Scale( TTraits::Sub( p1, p0 ), TScalar( 1 ) - t ), TTraits::Scale( TTraits::Sub( p2, p1 ), t ) );
    return TTraits::Scale( sum, TScalar( 2 ) );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTBezierCurve2< TPoint >::TVector
CTBezierCurve2< TPoint >::GetSecondDerivative( const TScalar& ) const GUCEF_MATH_REALTIME_CONTEXT
{
    // constant: 2 ( p2 - 2 p1 + p0 )
    return TTraits::Scale( TTraits::SubVectors( TTraits::Sub( p2, p1 ), TTraits::Sub( p1, p0 ) ), TScalar( 2 ) );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE void
CTBezierCurve2< TPoint >::Split( const TScalar& t, CTBezierCurve2& left, CTBezierCurve2& right ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const TPoint a = CurveLerp( p0, p1, t );
    const TPoint b = CurveLerp( p1, p2, t );
    const TPoint c = CurveLerp( a, b, t );
    const TPoint start = p0;
    const TPoint end = p2;
    left = CTBezierCurve2( start, a, c );
    right = CTBezierCurve2( c, b, end );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE void
CTBezierCurve2< TPoint >::GetBounds( TPoint& minimum, TPoint& maximum ) const GUCEF_MATH_REALTIME_CONTEXT
{
    minimum = p0;
    maximum = p0;
    const TPoint end = p2;
    for ( UInt32 i=0; i<TTraits::ComponentCount; ++i )
    {
        const TScalar e = TTraits::GetComponent( end, i );
        if ( e < TTraits::GetComponent( minimum, i ) ) TTraits::SetComponent( minimum, i, e );
        if ( e > TTraits::GetComponent( maximum, i ) ) TTraits::SetComponent( maximum, i, e );
    }
    // dP/dt / 2 = d0 ( 1 - t ) + d1 t per component: zero at t = d0 / ( d0 - d1 )
    const TVector d0 = TTraits::Sub( p1, p0 );
    const TVector d1 = TTraits::Sub( p2, p1 );
    for ( UInt32 i=0; i<TTraits::ComponentCount; ++i )
    {
        const TScalar v0 = TTraits::GetVectorComponent( d0, i );
        const TScalar v1 = TTraits::GetVectorComponent( d1, i );
        const TScalar denominator = v0 - v1;
        if ( denominator == TScalar( 0 ) )
            continue;
        const TScalar root = v0 / denominator;
        if ( root > TScalar( 0 ) && root < TScalar( 1 ) )
        {
            const TPoint q = Evaluate( root );
            for ( UInt32 j=0; j<TTraits::ComponentCount; ++j )
            {
                const TScalar value = TTraits::GetComponent( q, j );
                if ( value < TTraits::GetComponent( minimum, j ) ) TTraits::SetComponent( minimum, j, value );
                if ( value > TTraits::GetComponent( maximum, j ) ) TTraits::SetComponent( maximum, j, value );
            }
        }
    }
}

template < typename TPoint >
bool
CTBezierCurve2< TPoint >::Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
{
    GUCEF::vector< TPoint > points;
    points.push_back( p0 ); points.push_back( p1 ); points.push_back( p2 );
    return CurveSerializeList( node, "point", points, settings );
}

template < typename TPoint >
bool
CTBezierCurve2< TPoint >::Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
{
    GUCEF::vector< TPoint > points;
    if ( !CurveDeserializeList( node, "point", points, settings ) || 3 != points.size() )
        return false;
    p0 = points[ 0 ]; p1 = points[ 1 ]; p2 = points[ 2 ];
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_CTBEZIERCURVE2_EXTERN( kind ) GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTBezierCurve2< kind > )
GUCEF_MATH_CURVE_PREDEFINED_POINTS( GUCEF_MATH_CTBEZIERCURVE2_EXTERN )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTBEZIERCURVE2_H ? */
