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

#ifndef GUCEF_GEOMETRY_SPATIALQUERYREGIONS_H
#define GUCEF_GEOMETRY_SPATIALQUERYREGIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTFRUSTUM_H
#include "gucefGEOMETRY_CTFrustum.h"
#define GUCEF_GEOMETRY_CTFRUSTUM_H
#endif /* GUCEF_GEOMETRY_CTFRUSTUM_H ? */

#ifndef GUCEF_GEOMETRY_FRUSTUMCULLING_H
#include "gucefGEOMETRY_FrustumCulling.h"
#define GUCEF_GEOMETRY_FRUSTUMCULLING_H
#endif /* GUCEF_GEOMETRY_FRUSTUMCULLING_H ? */

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

/*
 *  The query regions of the spatial structures (section 7.8.4 of the design). A region answers two questions through
 *  the same arithmetic, so a structure and a brute force scan agree exactly:
 *      Accepts( lo, hi ): does the item with these closed bounds belong to the result?
 *      OverlapsNode( lo, hi, mask ): can anything inside a node or cell with these bounds be accepted? True never
 *      misses (a node box that contains an item box overlaps whenever the item is accepted); 'mask' is the plane mask
 *      of the frustum region (see FrustumCulling.h), carried from a node to its children, ignored by the others.
 *      GetBounds( lo, hi ): an axis aligned box that contains every accepted item box.
 *  A region is a small value (a few numbers or a plane pointer); it does not own what it points to.
 */

/** The items whose bounds touch or overlap a box (closed) */
template < typename T, UInt32 Dim >
class CTBoxQueryRegion
{
    public:

    CTBoxQueryRegion( const T* lo, const T* hi )
    {
        for ( UInt32 a=0; a<Dim; ++a )
        {
            m_lo[ a ] = lo[ a ];
            m_hi[ a ] = hi[ a ];
        }
    }

    GUCEF_GEOMETRY_FORCE_INLINE bool Accepts( const T* lo, const T* hi ) const
    {
        for ( UInt32 a=0; a<Dim; ++a )
            if ( !( !( hi[ a ] < m_lo[ a ] ) && !( m_hi[ a ] < lo[ a ] ) ) )
                return false;
        return true;
    }

    GUCEF_GEOMETRY_FORCE_INLINE bool OverlapsNode( const T* lo, const T* hi, UInt32& /* mask */ ) const { return Accepts( lo, hi ); }

    void GetBounds( T* lo, T* hi ) const
    {
        for ( UInt32 a=0; a<Dim; ++a )
        {
            lo[ a ] = m_lo[ a ];
            hi[ a ] = m_hi[ a ];
        }
    }

    UInt32 GetInitialMask( void ) const { return 0; }

    private:

    T m_lo[ Dim ];
    T m_hi[ Dim ];
};

/*-------------------------------------------------------------------------*/

/** The items whose bounds are at most 'radius' from a point: the squared distance of the point to the box, summed over the axes in order, is at most radius squared */
template < typename T, UInt32 Dim >
class CTSphereQueryRegion
{
    public:

    CTSphereQueryRegion( const T* centre, const T& radius ) : m_radiusSquared( radius * radius ), m_radius( radius )
    {
        for ( UInt32 a=0; a<Dim; ++a )
            m_centre[ a ] = centre[ a ];
    }

    GUCEF_GEOMETRY_FORCE_INLINE bool Accepts( const T* lo, const T* hi ) const
    {
        T sum = T( 0 );
        for ( UInt32 a=0; a<Dim; ++a )
        {
            T excess = T( 0 );
            if ( m_centre[ a ] < lo[ a ] )
                excess = lo[ a ] - m_centre[ a ];
            else if ( hi[ a ] < m_centre[ a ] )
                excess = m_centre[ a ] - hi[ a ];
            sum = sum + excess * excess;
        }
        return sum <= m_radiusSquared;
    }

    GUCEF_GEOMETRY_FORCE_INLINE bool OverlapsNode( const T* lo, const T* hi, UInt32& /* mask */ ) const { return Accepts( lo, hi ); }

    /** The box around the sphere, widened by 16 units of the last place so that the rounding of Accepts cannot reach beyond it */
    void GetBounds( T* lo, T* hi ) const
    {
        T largest = m_radius < T( 0 ) ? -m_radius : m_radius;
        for ( UInt32 a=0; a<Dim; ++a )
        {
            const T magnitude = m_centre[ a ] < T( 0 ) ? -m_centre[ a ] : m_centre[ a ];
            if ( magnitude > largest )
                largest = magnitude;
        }
        const T slack = largest * MATH::CTNumericTraits< T >::Epsilon() * T( 16 );
        for ( UInt32 a=0; a<Dim; ++a )
        {
            lo[ a ] = m_centre[ a ] - m_radius - slack;
            hi[ a ] = m_centre[ a ] + m_radius + slack;
        }
    }

    UInt32 GetInitialMask( void ) const { return 0; }

    private:

    T m_centre[ Dim ];
    T m_radiusSquared;
    T m_radius;
};

/*-------------------------------------------------------------------------*/

/**
 *  The items whose bounds the planes do not classify OUTSIDE (ClassifyBoxToPlaneSet with a tolerance of zero, the
 *  Classify of a box in a CTFrustum): 3D only. Nodes are tested with the plane mask coherence of FrustumCulling.h on
 *  boxes widened by a few units of the last place, so the rounding of the node test can never reject a node that holds
 *  an accepted item.
 */
template < typename T >
class CTFrustumQueryRegion
{
    public:

    typedef MATH::CTNumericTraits< T > TTraits;

    CTFrustumQueryRegion( const CTPlane< T >* planes, UInt32 planeCount )
        : m_planes( planes ), m_planeCount( planeCount )
    {
    }

    GUCEF_GEOMETRY_FORCE_INLINE bool Accepts( const T* lo, const T* hi ) const
    {
        UInt32 mask = GetInitialMask();
        return FRUSTUMCLASSIFICATION_OUTSIDE != ClassifyBoxLowHighWithMask( m_planes, m_planeCount, lo, hi, mask, TTraits::Zero() );
    }

    bool OverlapsNode( const T* lo, const T* hi, UInt32& mask ) const
    {
        // the slack: 16 units of the last place of the largest coordinate involved
        T largest = TTraits::Zero();
        for ( UInt32 a=0; a<3; ++a )
        {
            largest = TTraits::Max( largest, TTraits::Max( TTraits::Abs( lo[ a ] ), TTraits::Abs( hi[ a ] ) ) );
        }
        const T slack = largest * TTraits::Epsilon() * T( 16 );
        return FRUSTUMCLASSIFICATION_OUTSIDE != ClassifyBoxLowHighWithMask( m_planes, m_planeCount, lo, hi, mask, slack );
    }

    /**
     *  Unbounded: the per plane test is conservative near the edges and corners of the region, so it accepts boxes outside
     *  the bounds of the frustum corners as well, and a structure that narrows the search by bounds would miss them
     */
    void GetBounds( T* lo, T* hi ) const
    {
        for ( UInt32 a=0; a<3; ++a )
        {
            lo[ a ] = TTraits::Lowest();
            hi[ a ] = TTraits::Max();
        }
    }

    /** All planes still to be tested */
    UInt32 GetInitialMask( void ) const { return m_planeCount >= 32 ? 0xFFFFFFFFU : ( ( (UInt32) 1 << m_planeCount ) - 1 ); }

    private:

    const CTPlane< T >* m_planes;
    UInt32 m_planeCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_SPATIALQUERYREGIONS_H ? */
