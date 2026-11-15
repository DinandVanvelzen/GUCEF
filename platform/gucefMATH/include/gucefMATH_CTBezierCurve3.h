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

#ifndef GUCEF_MATH_CTBEZIERCURVE3_H
#define GUCEF_MATH_CTBEZIERCURVE3_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTCURVEPOINTTRAITS_H
#include "gucefMATH_CTCurvePointTraits.h"
#define GUCEF_MATH_CTCURVEPOINTTRAITS_H
#endif /* GUCEF_MATH_CTCURVEPOINTTRAITS_H ? */

#ifndef GUCEF_MATH_CTPOLYNOMIALSOLVER_H
#include "gucefMATH_CTPolynomialSolver.h"
#define GUCEF_MATH_CTPOLYNOMIALSOLVER_H
#endif /* GUCEF_MATH_CTPOLYNOMIALSOLVER_H ? */

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
 *  A cubic Bezier curve with 4 control points over the parameter [ 0, 1 ]: it starts at the first and ends at the
 *  last control point and is tangent to the first and last control polygon edge there.
 *
 *  - TPoint: Float32 / Float64 (1D), CTVector2D / 3D / 4D or CTPoint2D / 3D of those kinds (CTCurvePointTraits); any
 *    other kind does not compile (named static assertion).
 *  - Layout: exactly the 4 control points p0 .. p3, public, no virtual functions, no heap use.
 *  - Queries are realtime-safe. Evaluate() is exact at t = 0 and t = 1 and extrapolates the polynomial outside [ 0, 1 ].
 *  - The affine rules hold: points are only combined through differences, so a CTPoint curve never adds points.
 *  - DataNode: a child node 'point' per control point (attributes of the point type); no text form.
 *  - Curve interface used by CTArcLengthTable and the curve queries: TTraits, TVector, TScalar, Evaluate,
 *    GetDerivative, GetMinParameter, GetMaxParameter.
 */
template < typename TPoint >
class GUCEF_MATH_TEMPLATE_PUBLIC CTBezierCurve3
{
    public:

    typedef CTCurvePointTraits< TPoint > TTraits;
    typedef typename TTraits::TScalar TScalar;
    typedef typename TTraits::TVector TVector;

    TPoint p0;
    TPoint p1;
    TPoint p2;
    TPoint p3;

    /** All control points at the default point */
    CTBezierCurve3( void ) : p0(), p1(), p2(), p3() {}
    CTBezierCurve3( const TPoint& a, const TPoint& b, const TPoint& c, const TPoint& d ) : p0( a ), p1( b ), p2( c ), p3( d ) {}

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
    void Split( const TScalar& t, CTBezierCurve3& left, CTBezierCurve3& right ) const GUCEF_MATH_REALTIME_CONTEXT;

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
CTBezierCurve3< TPoint >::Evaluate( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( t == TScalar( 0 ) ) return p0;
    if ( t == TScalar( 1 ) ) return p3;
    const TScalar s = TScalar( 1 ) - t;
    const TScalar b1 = TScalar( 3 ) * s * s * t;
    const TScalar b2 = TScalar( 3 ) * s * t * t;
    const TScalar b3 = t * t * t;
    // relative to p0, so the affine rules hold; the Bernstein weights sum to one
    TVector sum = TTraits::Scale( TTraits::Sub( p1, p0 ), b1 );
    sum = TTraits::Add( sum, TTraits::Scale( TTraits::Sub( p2, p0 ), b2 ) );
    sum = TTraits::Add( sum, TTraits::Scale( TTraits::Sub( p3, p0 ), b3 ) );
    return TTraits::Offset( p0, sum );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTBezierCurve3< TPoint >::TVector
CTBezierCurve3< TPoint >::GetDerivative( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const TScalar s = TScalar( 1 ) - t;
    TVector sum = TTraits::Scale( TTraits::Sub( p1, p0 ), s * s );
    sum = TTraits::Add( sum, TTraits::Scale( TTraits::Sub( p2, p1 ), TScalar( 2 ) * s * t ) );
    sum = TTraits::Add( sum, TTraits::Scale( TTraits::Sub( p3, p2 ), t * t ) );
    return TTraits::Scale( sum, TScalar( 3 ) );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTBezierCurve3< TPoint >::TVector
CTBezierCurve3< TPoint >::GetSecondDerivative( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const TVector d0 = TTraits::Sub( p1, p0 );
    const TVector d1 = TTraits::Sub( p2, p1 );
    const TVector d2 = TTraits::Sub( p3, p2 );
    const TVector e0 = TTraits::SubVectors( d1, d0 );
    const TVector e1 = TTraits::SubVectors( d2, d1 );
    return TTraits::Scale( TTraits::Add( TTraits::Scale( e0, TScalar( 1 ) - t ), TTraits::Scale( e1, t ) ), TScalar( 6 ) );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE void
CTBezierCurve3< TPoint >::Split( const TScalar& t, CTBezierCurve3& left, CTBezierCurve3& right ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const TPoint a = CurveLerp( p0, p1, t );
    const TPoint b = CurveLerp( p1, p2, t );
    const TPoint c = CurveLerp( p2, p3, t );
    const TPoint d = CurveLerp( a, b, t );
    const TPoint e = CurveLerp( b, c, t );
    const TPoint f = CurveLerp( d, e, t );
    const TPoint start = p0;
    const TPoint end = p3;
    left = CTBezierCurve3( start, a, d, f );
    right = CTBezierCurve3( f, e, c, end );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE void
CTBezierCurve3< TPoint >::GetBounds( TPoint& minimum, TPoint& maximum ) const GUCEF_MATH_REALTIME_CONTEXT
{
    minimum = p0;
    maximum = p0;
    const TPoint end = p3;
    for ( UInt32 i=0; i<TTraits::ComponentCount; ++i )
    {
        const TScalar e = TTraits::GetComponent( end, i );
        if ( e < TTraits::GetComponent( minimum, i ) ) TTraits::SetComponent( minimum, i, e );
        if ( e > TTraits::GetComponent( maximum, i ) ) TTraits::SetComponent( maximum, i, e );
    }
    // dP/dt / 3 = a t^2 + b t + c per component with a = d0 - 2 d1 + d2, b = 2 ( d1 - d0 ), c = d0
    const TVector d0 = TTraits::Sub( p1, p0 );
    const TVector d1 = TTraits::Sub( p2, p1 );
    const TVector d2 = TTraits::Sub( p3, p2 );
    for ( UInt32 i=0; i<TTraits::ComponentCount; ++i )
    {
        TScalar roots[ 2 ];
        const TScalar v0 = TTraits::GetVectorComponent( d0, i );
        const TScalar v1 = TTraits::GetVectorComponent( d1, i );
        const TScalar v2 = TTraits::GetVectorComponent( d2, i );
        const UInt32 count = CTPolynomialSolver< TScalar >::SolveQuadratic( v0 - TScalar( 2 ) * v1 + v2, TScalar( 2 ) * ( v1 - v0 ), v0, roots );
        for ( UInt32 r=0; r<count; ++r )
        {
            if ( roots[ r ] > TScalar( 0 ) && roots[ r ] < TScalar( 1 ) )
            {
                const TPoint q = Evaluate( roots[ r ] );
                for ( UInt32 j=0; j<TTraits::ComponentCount; ++j )
                {
                    const TScalar value = TTraits::GetComponent( q, j );
                    if ( value < TTraits::GetComponent( minimum, j ) ) TTraits::SetComponent( minimum, j, value );
                    if ( value > TTraits::GetComponent( maximum, j ) ) TTraits::SetComponent( maximum, j, value );
                }
            }
        }
    }
}

template < typename TPoint >
bool
CTBezierCurve3< TPoint >::Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
{
    GUCEF::vector< TPoint > points;
    points.push_back( p0 ); points.push_back( p1 ); points.push_back( p2 ); points.push_back( p3 );
    return CurveSerializeList( node, "point", points, settings );
}

template < typename TPoint >
bool
CTBezierCurve3< TPoint >::Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
{
    GUCEF::vector< TPoint > points;
    if ( !CurveDeserializeList( node, "point", points, settings ) || 4 != points.size() )
        return false;
    p0 = points[ 0 ]; p1 = points[ 1 ]; p2 = points[ 2 ]; p3 = points[ 3 ];
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_CTBEZIERCURVE3_EXTERN( kind ) GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTBezierCurve3< kind > )
GUCEF_MATH_CURVE_PREDEFINED_POINTS( GUCEF_MATH_CTBEZIERCURVE3_EXTERN )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTBEZIERCURVE3_H ? */
