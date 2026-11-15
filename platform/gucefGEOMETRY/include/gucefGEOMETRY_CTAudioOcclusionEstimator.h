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

#ifndef GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H
#define GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#include "gucefMATH_LowDiscrepancySequences.h"
#define GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#endif /* GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H ? */

#ifndef GUCEF_MATH_SAMPLEWARPING_H
#include "gucefMATH_SampleWarping.h"
#define GUCEF_MATH_SAMPLEWARPING_H
#endif /* GUCEF_MATH_SAMPLEWARPING_H ? */

#ifndef GUCEF_GEOMETRY_CTRAYCASTER_H
#include "gucefGEOMETRY_CTRayCaster.h"
#define GUCEF_GEOMETRY_CTRAYCASTER_H
#endif /* GUCEF_GEOMETRY_CTRAYCASTER_H ? */

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
 *  The result of one estimate of CTAudioOcclusionEstimator: plain data (copyable by assignment, no heap, no virtual
 *  functions), so the owner thread can publish it through a latest-only MT::CTSlotDataExchange or hand the two values
 *  to an audio emitter (gucefAUDIO CAudioEmitter::SetOcclusion and SetObstruction, audio design 5.4 and 5.6.5).
 */
template < typename T >
struct GUCEF_GEOMETRY_TEMPLATE_PUBLIC SAudioOcclusionEstimate
{
    T occlusion;                    /**< 0 .. 1: the share of the sampled source to listener rays that are blocked */
    T obstruction;                  /**< 0 .. 1: 0 while the direct path is open, else 1 minus the open share of the offset paths (see the estimator) */
    bool directBlocked;             /**< the centre to centre path is blocked */
    UInt32 raysCast;                /**< the number of occlusion rays that were cast */
    UInt32 raysBlocked;             /**< of those, the number that were blocked */
    UInt32 offsetPathsCast;         /**< the number of obstruction offset paths that were cast (0 while the direct path is open) */
    UInt32 offsetPathsOpen;         /**< of those, the number that were open */

    SAudioOcclusionEstimate( void ) : occlusion( T( 0 ) ), obstruction( T( 0 ) ), directBlocked( false ), raysCast( 0 ), raysBlocked( 0 ), offsetPathsCast( 0 ), offsetPathsOpen( 0 ) {}
};

/*-------------------------------------------------------------------------*/

/**
 *  Occlusion and obstruction of a sound source, from the geometry of a CTRayCaster (section 7.11 of the design; audio
 *  design 5.4 and 5.6.5). The two values are what gucefAUDIO expects the game to supply per emitter; smoothing them
 *  over time is the consumer's choice (a blocked path appears in one frame, the ear should not).
 *
 *  Occlusion: N rays, ray i from a point on a sphere of radius 'sourceRadius' around the source to a point on a sphere
 *  of radius 'listenerRadius' around the listener; occlusion is the blocked share, so a wall that covers part of the
 *  view gives a partial value (a doorway, the edge of a pillar). A ray is blocked when any object of the ray caster
 *  that passes the layer mask and the filter lies on the segment (CTRayCaster::HasLineOfSight). With both radii 0 every
 *  ray would be the same one, so a single ray is cast and the result is 0 or 1.
 *
 *  Obstruction: the direct path, source centre to listener centre, is tested first; while it is open the obstruction
 *  is 0. When it is blocked, M offset paths run from the source centre to points on a sphere of radius
 *  'offsetRadius' around the listener (the listener's immediate surroundings); with 'open' the share of them that is
 *  open, obstruction = 1 - open / openShareForNoObstruction, limited to 0 .. 1. The default share is 1 (obstruction =
 *  the blocked share of the offset paths); a smaller value such as 0.5 lets the sound count as unobstructed once half
 *  of the surroundings can hear it (diffraction round a corner), which is also the value to use when the floor is part
 *  of the scene and always blocks the lower half of the sphere. A fully blocking wall gives 1 for both values.
 *
 *  Determinism: the sample points come from the low discrepancy sequence of gucefMATH (a four dimensional Sobol
 *  sequence, dimensions 0 and 1 for the source point and 2 and 3 for the listener point, mapped to the sphere by
 *  MATH::UniformSphere; the offset points are MATH::FibonacciSphere), so the same scene and arguments always give the
 *  same estimate and there is no random generator. Ray i uses sequence index 'startIndex' + i + 1 (index 0 is the
 *  customary skipped first point); an owner that estimates every frame can pass a growing 'startIndex' to rotate the
 *  pattern over time.
 *
 *  Accuracy: for a scene whose blocked share of the ray space is p the estimate is p plus the discrepancy of the
 *  first N points of the sequence, which for the half space splits of the tests is far below the error 1 / sqrt( N )
 *  (two standard errors of N independent random rays at p = 0.5) that this class promises: |estimate - p| <= 1 /
 *  sqrt( N ). A plane through the centre of a sampling sphere splits its points into two halves of N / 2 plus or minus
 *  a few.
 *
 *  Threading: owner thread, with the ray caster brought up to date by Update after the last change of the scene (the
 *  queries are const and may also run on several threads, each with its own estimator, between changes). Not
 *  realtime-safe (ray queries). No allocation happens in an estimate. The estimator does not own the ray caster or the
 *  filter and the caster must outlive it. Kinds: Float32 and Float64 are exported.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTAudioOcclusionEstimator
{
    public:

    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef CTRayCaster< T > TRayCaster;
    typedef CTGeoRayFilter< T > TFilter;
    typedef SAudioOcclusionEstimate< T > TEstimate;

    /** The most rays and offset paths a single estimate casts */
    enum { MaxSampleCount = 4096 };

    /** An estimator over the ray caster ( may be null: then nothing blocks and every estimate is 0 ) with 32 rays, point spheres, 8 offset paths of radius 1 */
    explicit CTAudioOcclusionEstimator( const TRayCaster* rayCaster = GUCEF_NULL );

    /** The ray caster the queries go to; not owned, null means an empty scene */
    void SetRayCaster( const TRayCaster* rayCaster ) { m_rayCaster = rayCaster; }
    const TRayCaster* GetRayCaster( void ) const { return m_rayCaster; }

    /** The layer mask objects must share to block (default all layers) */
    void SetLayerMask( UInt32 layerMask ) { m_layerMask = layerMask; }
    UInt32 GetLayerMask( void ) const { return m_layerMask; }

    /** An optional filter that objects must pass to block (not owned; null accepts everything); use it to ignore the object that carries the emitter */
    void SetFilter( const TFilter* filter ) { m_filter = filter; }
    const TFilter* GetFilter( void ) const { return m_filter; }

    /** The number N of occlusion rays of an estimate, limited to 1 .. MaxSampleCount */
    void SetRayCount( UInt32 count );
    UInt32 GetRayCount( void ) const { return m_rayCount; }

    /** The radius of the sphere around the source that the rays start on (world units, not negative; 0 = the centre) */
    void SetSourceRadius( const T& radius ) { m_sourceRadius = Radius( radius ); }
    const T& GetSourceRadius( void ) const { return m_sourceRadius; }

    /** The radius of the sphere around the listener that the rays end on (world units, not negative; 0 = the centre) */
    void SetListenerRadius( const T& radius ) { m_listenerRadius = Radius( radius ); }
    const T& GetListenerRadius( void ) const { return m_listenerRadius; }

    /** The number M of offset paths of the obstruction test, limited to 1 .. MaxSampleCount */
    void SetOffsetPathCount( UInt32 count );
    UInt32 GetOffsetPathCount( void ) const { return m_offsetPathCount; }

    /** The radius of the sphere around the listener that the offset paths end on (world units, not negative) */
    void SetOffsetRadius( const T& radius ) { m_offsetRadius = Radius( radius ); }
    const T& GetOffsetRadius( void ) const { return m_offsetRadius; }

    /** The open share of the offset paths at and above which the obstruction is 0 (default 1; limited to a value in ( 0, 1 ]) */
    void SetOpenShareForNoObstruction( const T& share );
    const T& GetOpenShareForNoObstruction( void ) const { return m_openShareForNoObstruction; }

    /** Estimates occlusion and obstruction between a source and a listener position; 'startIndex' rotates the sample pattern */
    void Estimate( const TPoint& source, const TPoint& listener, TEstimate& estimate, UInt32 startIndex = 0 ) const;

    /** The occlusion alone (the ray share only; the obstruction fields of 'estimate' are untouched) */
    void EstimateOcclusion( const TPoint& source, const TPoint& listener, TEstimate& estimate, UInt32 startIndex ) const;

    private:

    static T Radius( const T& radius ) { return radius > TTraits::Zero() ? radius : TTraits::Zero(); }

    bool IsOpen( const TPoint& from, const TPoint& to ) const
    {
        return GUCEF_NULL == m_rayCaster || m_rayCaster->HasLineOfSight( from, to, m_layerMask, m_filter );
    }

    private:

    const TRayCaster* m_rayCaster;
    const TFilter* m_filter;
    UInt32 m_layerMask;
    UInt32 m_rayCount;
    T m_sourceRadius;
    T m_listenerRadius;
    UInt32 m_offsetPathCount;
    T m_offsetRadius;
    T m_openShareForNoObstruction;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTAudioOcclusionEstimator< T >::CTAudioOcclusionEstimator( const TRayCaster* rayCaster )
    : m_rayCaster( rayCaster ), m_filter( GUCEF_NULL ), m_layerMask( 0xFFFFFFFFU ), m_rayCount( 32 ), m_sourceRadius( TTraits::Zero() ),
      m_listenerRadius( TTraits::Zero() ), m_offsetPathCount( 8 ), m_offsetRadius( TTraits::One() ), m_openShareForNoObstruction( TTraits::One() )
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTAudioOcclusionEstimator< T >::SetRayCount( UInt32 count )
{
    m_rayCount = count < 1 ? 1 : ( count > (UInt32) MaxSampleCount ? (UInt32) MaxSampleCount : count );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTAudioOcclusionEstimator< T >::SetOffsetPathCount( UInt32 count )
{
    m_offsetPathCount = count < 1 ? 1 : ( count > (UInt32) MaxSampleCount ? (UInt32) MaxSampleCount : count );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTAudioOcclusionEstimator< T >::SetOpenShareForNoObstruction( const T& share )
{
    if ( !( share > TTraits::Zero() ) )
        return;
    m_openShareForNoObstruction = share < TTraits::One() ? share : TTraits::One();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTAudioOcclusionEstimator< T >::EstimateOcclusion( const TPoint& source, const TPoint& listener, TEstimate& estimate, UInt32 startIndex ) const
{
    estimate.raysBlocked = 0;
    estimate.raysCast = 0;
    if ( GUCEF_NULL == m_rayCaster )
    {
        estimate.occlusion = TTraits::Zero();
        return;
    }

    const bool points = !( TTraits::Zero() < m_sourceRadius ) && !( TTraits::Zero() < m_listenerRadius );
    const UInt32 count = points ? 1 : m_rayCount;
    UInt32 blocked = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        // each draw in its own statement: the order of evaluation of function arguments is not defined
        const UInt32 index = startIndex + i + 1;
        const T u0 = MATH::Sobol< T >( index, 0 );
        const T u1 = MATH::Sobol< T >( index, 1 );
        const T u2 = MATH::Sobol< T >( index, 2 );
        const T u3 = MATH::Sobol< T >( index, 3 );
        const TPoint from = TTraits::Zero() < m_sourceRadius ? source + MATH::UniformSphere< T >( u0, u1 ) * m_sourceRadius : source;
        const TPoint to = TTraits::Zero() < m_listenerRadius ? listener + MATH::UniformSphere< T >( u2, u3 ) * m_listenerRadius : listener;
        if ( !IsOpen( from, to ) )
            ++blocked;
    }
    estimate.raysCast = count;
    estimate.raysBlocked = blocked;
    estimate.occlusion = TTraits::FromInt32( (Int32) blocked ) / TTraits::FromInt32( (Int32) count );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTAudioOcclusionEstimator< T >::Estimate( const TPoint& source, const TPoint& listener, TEstimate& estimate, UInt32 startIndex ) const
{
    estimate = TEstimate();
    if ( GUCEF_NULL == m_rayCaster )
        return;

    EstimateOcclusion( source, listener, estimate, startIndex );

    estimate.directBlocked = !IsOpen( source, listener );
    if ( !estimate.directBlocked )
        return;

    UInt32 open = 0;
    for ( UInt32 j=0; j<m_offsetPathCount; ++j )
    {
        const TPoint to = listener + MATH::FibonacciSphere< T >( j + ( startIndex % m_offsetPathCount ), m_offsetPathCount ) * m_offsetRadius;
        if ( IsOpen( source, to ) )
            ++open;
    }
    estimate.offsetPathsCast = m_offsetPathCount;
    estimate.offsetPathsOpen = open;
    const T share = TTraits::FromInt32( (Int32) open ) / TTraits::FromInt32( (Int32) m_offsetPathCount );
    T obstruction = TTraits::One() - share / m_openShareForNoObstruction;
    if ( obstruction < TTraits::Zero() )
        obstruction = TTraits::Zero();
    else
    if ( obstruction > TTraits::One() )
        obstruction = TTraits::One();
    estimate.obstruction = obstruction;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTAudioOcclusionEstimator< Float32 > CAudioOcclusionEstimator;
typedef CTAudioOcclusionEstimator< Float64 > CAudioOcclusionEstimator64;
typedef SAudioOcclusionEstimate< Float32 > TFloat32AudioOcclusionEstimate;
typedef SAudioOcclusionEstimate< Float64 > TFloat64AudioOcclusionEstimate;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( SAudioOcclusionEstimate< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( SAudioOcclusionEstimate< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTAudioOcclusionEstimator< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTAudioOcclusionEstimator< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H ? */
