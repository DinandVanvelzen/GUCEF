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

#ifndef GUCEF_MATH_CTVBAP2D_H
#define GUCEF_MATH_CTVBAP2D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

#ifndef GUCEF_MATH_CTMATRIX2D_H
#include "gucefMATH_CTMatrix2D.h"
#define GUCEF_MATH_CTMATRIX2D_H
#endif /* GUCEF_MATH_CTMATRIX2D_H ? */

#ifndef GUCEF_MATH_CTVECTOR2D_H
#include "gucefMATH_CTVector2D.h"
#define GUCEF_MATH_CTVECTOR2D_H
#endif /* GUCEF_MATH_CTVECTOR2D_H ? */

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
 *  Two dimensional vector base amplitude panning over a horizontal ring of speakers (section 4.7.5 of the design; V. Pulkki,
 *  "Virtual Sound Source Positioning Using Vector Base Amplitude Panning", JAES 45(6), 1997).
 *
 *  - Azimuths follow the listener convention of gucefAUDIO: 0 straight ahead, positive to the right, in radians; any
 *    angle is accepted and wrapped (a source azimuth of 2 pi is straight ahead).
 *  - Prepare sorts the speakers into a ring and precomputes for every adjacent pair the inverse of the 2x2 matrix whose
 *    columns are the unit vectors ( sin a, cos a ) of its two speakers. ComputeGains finds the pair that encloses the source
 *    azimuth and solves [ g1 g2 ] = [ sin a  cos a ] L^-1 (L the matrix with the speaker vectors as rows), which is the
 *    pair of gains that places the source vector between the speakers; negative round-off is clamped to 0 and the gains are
 *    scaled so that the sum of their squares equals the target power (gucefAUDIO uses 2). For a source at the azimuth of
 *    a speaker that speaker gets all of the power; between two speakers the gains are continuous. All other outputs are 0.
 *  - Layouts: 2 to MaxSpeakers speakers. Prepare rejects (returns false and leaves the object unprepared): fewer than 2 or
 *    more than MaxSpeakers speakers, a null pointer, non-finite azimuths, coincident speakers (closer than the square root
 *    of the epsilon of the kind, about 0.02 degrees for Float32), and rings with a gap between adjacent speakers of 180
 *    degrees or more (minus that tolerance). One exception is the stereo pair: two speakers form a pair, not a ring; the
 *    source is panned by VBAP between them inside the arc smaller than 180 degrees and outside it (the long way round) the
 *    power is cross-faded between the two speakers with a constant power law, so every azimuth gives defined and continuous
 *    gains. A pair of exactly opposite speakers (+-90 degrees, or one ahead and one behind) has two arcs of 180 degrees, on which VBAP
 *    has no solution: both arcs are cross-faded the same way (the source straight ahead and straight behind get equal gains).
 *    Layouts with a larger open side (for example left, centre and right only) are rejected: add a rear speaker.
 *  - The gains array of ComputeGains has GetSpeakerCount() entries, in the order of the azimuths given to Prepare. An
 *    object that is not prepared, a non-finite azimuth or a target power that is not positive writes zero gains (nothing
 *    for an unprepared object).
 *  - Capacity: fixed, MaxSpeakers = 32; no heap use at any point.
 *  - Threading: neither Prepare nor ComputeGains allocates, locks or traces; the work of Prepare is bounded (a sort of at
 *    most 32 speakers and at most 32 2x2 inverses), so it may also run on the audio thread when a layout changes. ComputeGains
 *    is const, so one prepared object can be used by several threads.
 *  - Kind: T is a floating point kind.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTVbap2D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;

    GUCEF_MATH_STATIC_ASSERT( TTraits::IsFloatingPoint, Vbap_requires_a_floating_point_kind );

    enum { MaxSpeakers = 32 };

    /** An unprepared panner without speakers */
    CTVbap2D( void )
        : m_count( 0 )
    {
        for ( UInt32 i = 0; i < (UInt32) MaxSpeakers; ++i )
        {
            m_order[ i ] = 0;
            m_angle[ i ] = TTraits::Zero();
            m_gap[ i ] = TTraits::Zero();
            m_covered[ i ] = false;
        }
        m_angle[ MaxSpeakers ] = TTraits::Zero();
    }

    /**
     *  Prepares the ring of speakers at the given azimuths (see the class comment for the layouts that are rejected). On
     *  false the object is unprepared (GetSpeakerCount() == 0).
     */
    bool Prepare( const CTRadian< T >* speakerAzimuths, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Forgets the layout */
    GUCEF_MATH_FORCE_INLINE void Clear( void ) GUCEF_MATH_REALTIME_CONTEXT { m_count = 0; }

    /** The number of speakers of the prepared layout, 0 when unprepared */
    GUCEF_MATH_FORCE_INLINE UInt32 GetSpeakerCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_count; }

    GUCEF_MATH_FORCE_INLINE bool IsPrepared( void ) const GUCEF_MATH_REALTIME_CONTEXT { return 0 != m_count; }

    /**
     *  The gains of the speakers for a source at 'azimuth': writes GetSpeakerCount() values into 'gains' with the sum of
     *  their squares equal to targetPower (see the class comment).
     */
    void ComputeGains( const CTRadian< T >& azimuth, T* gains, const T& targetPower ) const GUCEF_MATH_REALTIME_CONTEXT;

    private:

    /** The module's batch kernel (VbapGains2D) reads the prepared ring to compute the gains of several sources at once */
    friend struct SMathVbapBatchAccess;

    UInt32 m_count;
    UInt32 m_order[ MaxSpeakers ];              // ring position -> index of the speaker as given to Prepare
    T m_angle[ MaxSpeakers + 1 ];               // ring azimuths ascending in ( -pi, pi ]; [ count ] is [ 0 ] + 2 pi
    T m_gap[ MaxSpeakers ];                     // arc from ring position k to k + 1
    bool m_covered[ MaxSpeakers ];              // the arc is smaller than 180 degrees and has a VBAP inverse
    CTMatrix2D< T > m_inverse[ MaxSpeakers ];   // inverse of the matrix with the unit vectors of the pair as columns
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/* The related classes used by the members, included after the class (any header of the family can be included first) */

#ifndef GUCEF_MATH_ANGLEFUNCTIONS_H
#include "gucefMATH_AngleFunctions.h"
#define GUCEF_MATH_ANGLEFUNCTIONS_H
#endif /* GUCEF_MATH_ANGLEFUNCTIONS_H ? */

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive members, out of the class: with GUCEF_MATH_PRECISE_INLINE (empty by default) they are not
 *  inline, so for the predefined kinds the copy compiled in the module (without floating-point contraction) runs.
 */

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTVbap2D< T >::Prepare( const CTRadian< T >* speakerAzimuths, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    m_count = 0;
    if ( GUCEF_NULL == speakerAzimuths || count < 2 || count > (UInt32) MaxSpeakers )
        return false;

    // wrap every azimuth into ( -pi, pi ] and sort the ring positions by insertion (at most 32 entries)
    T wrapped[ MaxSpeakers ];
    UInt32 order[ MaxSpeakers ];
    for ( UInt32 i = 0; i < count; ++i )
    {
        if ( !TTraits::IsFinite( speakerAzimuths[ i ].GetValue() ) )
            return false;
        wrapped[ i ] = WrapAngle( speakerAzimuths[ i ] ).GetValue();
        UInt32 position = i;
        while ( position > 0 && wrapped[ i ] < wrapped[ order[ position - 1 ] ] )
        {
            order[ position ] = order[ position - 1 ];
            --position;
        }
        order[ position ] = i;
    }

    const T pi = CTMathConstants< T >::Pi();
    const T twoPi = CTMathConstants< T >::TwoPi();
    const T tolerance = TTraits::Sqrt( TTraits::Epsilon() );
    T angle[ MaxSpeakers + 1 ];
    for ( UInt32 k = 0; k < count; ++k )
        angle[ k ] = wrapped[ order[ k ] ];
    angle[ count ] = angle[ 0 ] + twoPi;

    T gap[ MaxSpeakers ];
    bool covered[ MaxSpeakers ];
    UInt32 openArcs = 0;
    for ( UInt32 k = 0; k < count; ++k )
    {
        gap[ k ] = angle[ k + 1 ] - angle[ k ];
        if ( !( tolerance < gap[ k ] ) )
            return false;       // coincident speakers
        covered[ k ] = gap[ k ] < pi - tolerance;
        if ( !covered[ k ] )
            ++openArcs;
    }

    // a ring needs every arc covered; two speakers are a pair: one covered arc and one open arc, or exactly opposite speakers
    // (a pair at +-90 degrees) with two open arcs, both panned by the cross-fade
    if ( count > 2 && 0 != openArcs )
        return false;

    CTMatrix2D< T > inverse[ MaxSpeakers ];
    for ( UInt32 k = 0; k < count; ++k )
    {
        if ( !covered[ k ] )
            continue;
        const T a1 = angle[ k ];
        const T a2 = angle[ k + 1 ];
        const CTMatrix2D< T > basis = CTMatrix2D< T >::FromColumns( CTVector2D< T >( TTraits::Sin( a1 ), TTraits::Cos( a1 ) ),
                                                                    CTVector2D< T >( TTraits::Sin( a2 ), TTraits::Cos( a2 ) ) );
        if ( !basis.TryInverse( inverse[ k ], TTraits::Zero() ) )
            return false;
    }

    for ( UInt32 k = 0; k < count; ++k )
    {
        m_order[ k ] = order[ k ];
        m_gap[ k ] = gap[ k ];
        m_covered[ k ] = covered[ k ];
        m_inverse[ k ] = inverse[ k ];
    }
    for ( UInt32 k = 0; k <= count; ++k )
        m_angle[ k ] = angle[ k ];
    m_count = count;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE void
CTVbap2D< T >::ComputeGains( const CTRadian< T >& azimuth, T* gains, const T& targetPower ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == m_count || GUCEF_NULL == gains )
        return;
    for ( UInt32 i = 0; i < m_count; ++i )
        gains[ i ] = TTraits::Zero();
    if ( !TTraits::IsFinite( azimuth.GetValue() ) || !( TTraits::Zero() < targetPower ) )
        return;

    // the azimuth moved into the ring range [ angle[ 0 ], angle[ 0 ] + 2 pi ) and the arc that contains it
    const T start = m_angle[ 0 ];
    T position = start + TTraits::ModFloor( azimuth.GetValue() - start, CTMathConstants< T >::TwoPi() );
    if ( !( position < m_angle[ m_count ] ) )
        position = start;
    UInt32 k = 0;
    while ( k + 1 < m_count && m_angle[ k + 1 ] <= position )
        ++k;

    T g1 = TTraits::One();
    T g2 = TTraits::Zero();
    if ( m_covered[ k ] )
    {
        const CTVector2D< T > gainPair = m_inverse[ k ].Transform( CTVector2D< T >( TTraits::Sin( position ), TTraits::Cos( position ) ) );
        g1 = gainPair.x < TTraits::Zero() ? TTraits::Zero() : gainPair.x;
        g2 = gainPair.y < TTraits::Zero() ? TTraits::Zero() : gainPair.y;
    }
    else
    {
        // the open side of a stereo pair: a constant power cross-fade between its two ends
        const T fraction = ( position - m_angle[ k ] ) / m_gap[ k ];
        const T quarterTurn = fraction * CTMathConstants< T >::HalfPi();
        g1 = TTraits::Cos( quarterTurn );
        g2 = TTraits::Sin( quarterTurn );
    }

    const T power = g1 * g1 + g2 * g2;
    if ( !( TTraits::Zero() < power ) )
    {
        g1 = TTraits::One();
        g2 = TTraits::Zero();
    }
    const T scale = TTraits::Sqrt( targetPower / ( TTraits::Zero() < power ? power : TTraits::One() ) );
    gains[ m_order[ k ] ] = g1 * scale;
    gains[ m_order[ ( k + 1 ) % m_count ] ] = g2 * scale;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTVbap2D< Float32 > TFloat32Vbap2D;
typedef CTVbap2D< Float64 > TFloat64Vbap2D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive members of the predefined kinds are instantiated in the module and declared extern here
 *  (section 5.10 of the design), member by member. A list macro takes the prefix (GUCEF_MATH_EXTERN_TEMPLATE in
 *  headers, GUCEF_MATH_INSTANTIATE_TEMPLATE in the module source) and the kind (a name without commas).
 */

#define GUCEF_MATH_CTVBAP2D_REAL_KIND( prefix, kind ) \
    prefix bool CTVbap2D< kind >::Prepare( const CTRadian< kind >*, UInt32 ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix void CTVbap2D< kind >::ComputeGains( const CTRadian< kind >&, kind*, const kind& ) const GUCEF_MATH_REALTIME_CONTEXT;

/** Every predefined floating point kind */
#define GUCEF_MATH_CTVBAP2D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTVBAP2D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTVBAP2D_REAL_KIND( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTVBAP2D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTVBAP2D_H ? */
