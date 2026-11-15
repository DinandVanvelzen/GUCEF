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

#ifndef GUCEF_MATH_CTHERMITESPLINE_H
#define GUCEF_MATH_CTHERMITESPLINE_H

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  One cubic Hermite segment from p0 to p1 with the tangents m0 and m1 ( dP/du at u = 0 and u = 1 ), evaluated at
 *  the segment parameter u. u is not clamped: outside [ 0, 1 ] the cubic is extrapolated. The result is exactly p0
 *  at u = 0 and exactly p1 at u = 1. Shared by CTHermiteSpline, CTCatmullRomSpline and CTCurve. Realtime-safe.
 */

template < typename TPoint >
GUCEF_MATH_FORCE_INLINE TPoint
HermiteEvaluate( const TPoint& p0, const typename CTCurvePointTraits< TPoint >::TVector& m0,
                 const TPoint& p1, const typename CTCurvePointTraits< TPoint >::TVector& m1,
                 const typename CTCurvePointTraits< TPoint >::TScalar& u ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTCurvePointTraits< TPoint > TTraits;
    typedef typename TTraits::TScalar TScalar;
    if ( u == TScalar( 0 ) ) return p0;
    if ( u == TScalar( 1 ) ) return p1;
    const TScalar u2 = u * u;
    const TScalar u3 = u2 * u;
    const TScalar h10 = u3 - TScalar( 2 ) * u2 + u;
    const TScalar h01 = TScalar( -2 ) * u3 + TScalar( 3 ) * u2;
    const TScalar h11 = u3 - u2;
    // p0 + h10 m0 + h01 ( p1 - p0 ) + h11 m1 ( the basis h00 + h01 is one, so the offset form is exact in p0 )
    return TTraits::Offset( p0, TTraits::Add( TTraits::Add( TTraits::Scale( m0, h10 ), TTraits::Scale( TTraits::Sub( p1, p0 ), h01 ) ), TTraits::Scale( m1, h11 ) ) );
}

/** dP/du of the segment of HermiteEvaluate() */
template < typename TPoint >
GUCEF_MATH_FORCE_INLINE typename CTCurvePointTraits< TPoint >::TVector
HermiteDerivative( const TPoint& p0, const typename CTCurvePointTraits< TPoint >::TVector& m0,
                   const TPoint& p1, const typename CTCurvePointTraits< TPoint >::TVector& m1,
                   const typename CTCurvePointTraits< TPoint >::TScalar& u ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTCurvePointTraits< TPoint > TTraits;
    typedef typename TTraits::TScalar TScalar;
    const TScalar u2 = u * u;
    const TScalar d10 = TScalar( 3 ) * u2 - TScalar( 4 ) * u + TScalar( 1 );
    const TScalar d01 = TScalar( -6 ) * u2 + TScalar( 6 ) * u;
    const TScalar d11 = TScalar( 3 ) * u2 - TScalar( 2 ) * u;
    return TTraits::Add( TTraits::Add( TTraits::Scale( m0, d10 ), TTraits::Scale( TTraits::Sub( p1, p0 ), d01 ) ), TTraits::Scale( m1, d11 ) );
}

/** d2P/du2 of the segment of HermiteEvaluate() */
template < typename TPoint >
GUCEF_MATH_FORCE_INLINE typename CTCurvePointTraits< TPoint >::TVector
HermiteSecondDerivative( const TPoint& p0, const typename CTCurvePointTraits< TPoint >::TVector& m0,
                         const TPoint& p1, const typename CTCurvePointTraits< TPoint >::TVector& m1,
                         const typename CTCurvePointTraits< TPoint >::TScalar& u ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTCurvePointTraits< TPoint > TTraits;
    typedef typename TTraits::TScalar TScalar;
    const TScalar e10 = TScalar( 6 ) * u - TScalar( 4 );
    const TScalar e01 = TScalar( -12 ) * u + TScalar( 6 );
    const TScalar e11 = TScalar( 6 ) * u - TScalar( 2 );
    return TTraits::Add( TTraits::Add( TTraits::Scale( m0, e10 ), TTraits::Scale( TTraits::Sub( p1, p0 ), e01 ) ), TTraits::Scale( m1, e11 ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A chain of cubic Hermite segments with explicit tangents: every key has a position and an incoming and an outgoing
 *  tangent ( dP/dt, the velocity through the key ). Segment i runs from key i to key i + 1 over the parameter
 *  [ i, i + 1 ], so the spline parameter is the key index plus the position inside the segment; the spline is C1 where
 *  the in and out tangent of a key are equal (AddKey with one tangent) and has a corner where they differ.
 *
 *  - TPoint: Float32 / Float64 (1D), CTVector2D / 3D / 4D or CTPoint2D / 3D of those kinds (CTCurvePointTraits).
 *  - Memory: the keys are held in a GUCEF::vector; a query never allocates. A spline of fewer than two keys evaluates
 *    to its single key (or the default point when it has none) with a zero derivative.
 *  - Queries are realtime-safe; AddKey, SetKey, RemoveKey and Clear are not (they allocate). Concurrent const use is safe.
 *  - Parameters outside [ GetMinParameter(), GetMaxParameter() ] are clamped.
 *  - DataNode form: a child node per key (see section 9.2 of the design).
 *  - Curve interface used by CTArcLengthTable and the curve queries: TTraits, TVector, TScalar, Evaluate,
 *    GetDerivative, GetMinParameter, GetMaxParameter.
 */
template < typename TPoint >
class GUCEF_MATH_TEMPLATE_PUBLIC CTHermiteSpline
{
    public:

    typedef CTCurvePointTraits< TPoint > TTraits;
    typedef typename TTraits::TScalar TScalar;
    typedef typename TTraits::TVector TVector;

    struct SKey
    {
        TPoint position;
        TVector inTangent;
        TVector outTangent;

        SKey( void ) : position(), inTangent( TTraits::Zero() ), outTangent( TTraits::Zero() ) {}
        SKey( const TPoint& p, const TVector& in, const TVector& out ) : position( p ), inTangent( in ), outTangent( out ) {}
    };

    CTHermiteSpline( void );

    /** Appends a key with the same tangent on both sides */
    void AddKey( const TPoint& position, const TVector& tangent );

    /** Appends a key with different incoming and outgoing tangents */
    void AddKey( const TPoint& position, const TVector& inTangent, const TVector& outTangent );

    /** Replaces key 'index'; false (nothing changed) when the index is out of range */
    bool SetKey( UInt32 index, const SKey& key );

    /** Removes key 'index'; false when the index is out of range */
    bool RemoveKey( UInt32 index );

    void Clear( void );

    GUCEF_MATH_FORCE_INLINE UInt32 GetKeyCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return (UInt32) m_keys.size(); }

    /** Key 'index'; the default key (zero position and tangents) when the index is out of range */
    SKey GetKey( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Number of segments: key count - 1, or 0 */
    GUCEF_MATH_FORCE_INLINE UInt32 GetSegmentCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_keys.size() > 1 ? (UInt32) m_keys.size() - 1 : 0; }

    GUCEF_MATH_FORCE_INLINE TScalar GetMinParameter( void ) const GUCEF_MATH_REALTIME_CONTEXT { return TScalar( 0 ); }
    GUCEF_MATH_FORCE_INLINE TScalar GetMaxParameter( void ) const GUCEF_MATH_REALTIME_CONTEXT { return TScalar( GetSegmentCount() ); }

    /** The point at parameter t; t is clamped to the parameter range */
    TPoint Evaluate( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** dP/dt at parameter t (zero for fewer than two keys) */
    TVector GetDerivative( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** d2P/dt2 at parameter t (zero for fewer than two keys) */
    TVector GetSecondDerivative( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Appends the parameters where the segments join (the interior key indices) to 'breakpoints' (see CTArcLengthTable::BuildPiecewise) */
    void GetBreakpoints( GUCEF::vector< TScalar >& breakpoints ) const;

    /** Writes a child node 'key' per key with the child nodes 'position', 'in' and 'out' (not realtime-safe) */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const;

    /** Reads what Serialize writes; any error leaves the spline unchanged (not realtime-safe) */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings );

    private:

    /** Segment index and local parameter for t; false when there is no segment */
    bool Locate( const TScalar& t, UInt32& segment, TScalar& u ) const GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF::vector< SKey > m_keys;
};

/*-------------------------------------------------------------------------*/

template < typename TPoint >
CTHermiteSpline< TPoint >::CTHermiteSpline( void )
    : m_keys()
{
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE void
CTHermiteSpline< TPoint >::AddKey( const TPoint& position, const TVector& tangent )
{
    m_keys.push_back( SKey( position, tangent, tangent ) );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE void
CTHermiteSpline< TPoint >::AddKey( const TPoint& position, const TVector& inTangent, const TVector& outTangent )
{
    m_keys.push_back( SKey( position, inTangent, outTangent ) );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE bool
CTHermiteSpline< TPoint >::SetKey( UInt32 index, const SKey& key )
{
    if ( index >= m_keys.size() )
        return false;
    m_keys[ index ] = key;
    return true;
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE bool
CTHermiteSpline< TPoint >::RemoveKey( UInt32 index )
{
    if ( index >= m_keys.size() )
        return false;
    m_keys.erase( m_keys.begin() + index );
    return true;
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE void
CTHermiteSpline< TPoint >::Clear( void )
{
    m_keys.clear();
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTHermiteSpline< TPoint >::SKey
CTHermiteSpline< TPoint >::GetKey( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return index < m_keys.size() ? m_keys[ index ] : SKey();
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE bool
CTHermiteSpline< TPoint >::Locate( const TScalar& t, UInt32& segment, TScalar& u ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt32 segments = GetSegmentCount();
    if ( 0 == segments )
        return false;
    TScalar clamped = t;
    if ( !( clamped > TScalar( 0 ) ) )
        clamped = TScalar( 0 );
    if ( clamped > TScalar( segments ) )
        clamped = TScalar( segments );
    UInt32 index = (UInt32) clamped;
    if ( index >= segments )
        index = segments - 1;
    segment = index;
    u = clamped - TScalar( index );
    return true;
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE TPoint
CTHermiteSpline< TPoint >::Evaluate( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 segment = 0;
    TScalar u = TScalar( 0 );
    if ( !Locate( t, segment, u ) )
        return m_keys.empty() ? TPoint() : m_keys[ 0 ].position;
    const SKey& a = m_keys[ segment ];
    const SKey& b = m_keys[ segment + 1 ];
    return HermiteEvaluate( a.position, a.outTangent, b.position, b.inTangent, u );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTHermiteSpline< TPoint >::TVector
CTHermiteSpline< TPoint >::GetDerivative( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 segment = 0;
    TScalar u = TScalar( 0 );
    if ( !Locate( t, segment, u ) )
        return TTraits::Zero();
    const SKey& a = m_keys[ segment ];
    const SKey& b = m_keys[ segment + 1 ];
    return HermiteDerivative( a.position, a.outTangent, b.position, b.inTangent, u );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTHermiteSpline< TPoint >::TVector
CTHermiteSpline< TPoint >::GetSecondDerivative( const TScalar& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 segment = 0;
    TScalar u = TScalar( 0 );
    if ( !Locate( t, segment, u ) )
        return TTraits::Zero();
    const SKey& a = m_keys[ segment ];
    const SKey& b = m_keys[ segment + 1 ];
    return HermiteSecondDerivative( a.position, a.outTangent, b.position, b.inTangent, u );
}

template < typename TPoint >
void
CTHermiteSpline< TPoint >::GetBreakpoints( GUCEF::vector< TScalar >& breakpoints ) const
{
    for ( size_t i=1; i+1<m_keys.size(); ++i )
        breakpoints.push_back( TScalar( i ) );
}

template < typename TPoint >
bool
CTHermiteSpline< TPoint >::Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
{
    for ( size_t i=0; i<m_keys.size(); ++i )
    {
        CORE::CDataNode* key = node.AddChild( CORE::CString( "key" ) );
        if ( GUCEF_NULL == key )
            return false;
        CORE::CDataNode* position = key->AddChild( CORE::CString( "position" ) );
        CORE::CDataNode* in = key->AddChild( CORE::CString( "in" ) );
        CORE::CDataNode* out = key->AddChild( CORE::CString( "out" ) );
        if ( GUCEF_NULL == position || GUCEF_NULL == in || GUCEF_NULL == out )
            return false;
        if ( !CurveValueSerialize( *position, m_keys[ i ].position, settings ) ||
             !CurveValueSerialize( *in, m_keys[ i ].inTangent, settings )      ||
             !CurveValueSerialize( *out, m_keys[ i ].outTangent, settings )     )
            return false;
    }
    return true;
}

template < typename TPoint >
bool
CTHermiteSpline< TPoint >::Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
{
    GUCEF::vector< SKey > parsed;
    const CORE::CString keyName( "key" );
    for ( const CORE::CDataNode* key = node.GetFirstChild(); GUCEF_NULL != key; key = key->GetNext() )
    {
        if ( key->GetName() != keyName )
            continue;
        const CORE::CDataNode* position = key->FindChild( CORE::CString( "position" ) );
        const CORE::CDataNode* in = key->FindChild( CORE::CString( "in" ) );
        const CORE::CDataNode* out = key->FindChild( CORE::CString( "out" ) );
        SKey value;
        if ( GUCEF_NULL == position || GUCEF_NULL == in || GUCEF_NULL == out )
            return false;
        if ( !CurveValueDeserialize( *position, value.position, settings ) ||
             !CurveValueDeserialize( *in, value.inTangent, settings )      ||
             !CurveValueDeserialize( *out, value.outTangent, settings )     )
            return false;
        parsed.push_back( value );
    }
    m_keys.swap( parsed );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_CTHERMITESPLINE_EXTERN( kind ) GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTHermiteSpline< kind > )
GUCEF_MATH_CURVE_PREDEFINED_POINTS( GUCEF_MATH_CTHERMITESPLINE_EXTERN )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTHERMITESPLINE_H ? */
