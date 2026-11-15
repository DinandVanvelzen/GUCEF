/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_BATCHFUNCTIONS_H
#include "gucefGEOMETRY_BatchFunctions.h"
#define GUCEF_GEOMETRY_BATCHFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_BATCHFUNCTIONS_H ? */

#ifndef GUCEF_GEOMETRY_FRUSTUMCULLING_H
#include "gucefGEOMETRY_FrustumCulling.h"
#define GUCEF_GEOMETRY_FRUSTUMCULLING_H
#endif /* GUCEF_GEOMETRY_FRUSTUMCULLING_H ? */

#ifndef GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H
#include "gucefGEOMETRY_CGeometryGlobal.h"
#define GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H
#endif /* GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H ? */

#ifndef GUCEF_GEOMETRY_CMODULE_H
#include "gucefGEOMETRY_CModule.h"
#define GUCEF_GEOMETRY_CMODULE_H
#endif /* GUCEF_GEOMETRY_CMODULE_H ? */

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#ifndef GUCEF_MATH_CMATHBATCHDISPATCHER_H
#include "gucefMATH_CMathBatchDispatcher.h"
#define GUCEF_MATH_CMATHBATCHDISPATCHER_H
#endif /* GUCEF_MATH_CMATHBATCHDISPATCHER_H ? */


#include "TestSpatialReference.h"
#include "TestConvexReference.h"
#include "TestGeometryBatchKernels.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::MATH;
using namespace GUCEF::GEOMETRY;
using namespace QueryReference;
using namespace SpatialReference;
using ConvexReference::ScaledCount;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static MATH::CMathBatchDispatcher&
Dispatcher( void )
{
    CGeometryGlobal::Instance();
    return MATH::CMathGlobal::Instance()->GetBatchDispatcher();
}

/** A box of random values, now and then empty, not finite or degenerate */
static void
RandomBox( CRandom& random, Float32* box )
{
    const Float32 specials[ 6 ] = { 0.0f, -0.0f, 1.0f, 3.4e38f, 1.0e-38f, -2.5f };
    for ( UInt32 a=0; a<3; ++a )
    {
        const Float32 c = (Float32) random.Uniform( -50.0, 50.0 );
        const Float32 h = (Float32) random.Uniform( 0.0, 10.0 );
        box[ a ] = c - h;
        box[ 3 + a ] = c + h;
    }
    const UInt32 kind = (UInt32) random.Index( 20 );
    if ( 0 == kind )
    {
        // empty on one axis
        const UInt32 a = (UInt32) random.Index( 3 );
        const Float32 t = box[ a ];
        box[ a ] = box[ 3 + a ] + 1.0f;
        box[ 3 + a ] = t;
    }
    else if ( 1 == kind )
        box[ random.Index( 6 ) ] = specials[ random.Index( 6 ) ];
    else if ( 2 == kind )
    {
        const Float32 zero = 0.0f;
        box[ random.Index( 6 ) ] = random.Chance( 0.5 ) ? zero / zero : 1.0f / zero;
    }
}

static CTMatrix4D< Float32 >
RandomAffine( CRandom& random )
{
    Float32 m[ 16 ];
    for ( UInt32 i=0; i<16; ++i )
        m[ i ] = (Float32) random.Uniform( -3.0, 3.0 );
    m[ 3 ] = m[ 7 ] = m[ 11 ] = 0.0f;
    m[ 15 ] = 1.0f;
    if ( random.Chance( 0.15 ) )
        for ( UInt32 i=0; i<12; ++i )
            if ( random.Chance( 0.5 ) )
                m[ i ] = 0.0f;
    return CTMatrix4D< Float32 >::FromColumnMajor( m );
}

/** Calls every registered provider of a kernel with the same arguments through the typed signature */
template < typename TArgs, typename TFunction >
static bool
ForEachProvider( MATH::TMathBatchKernel kernel, const TArgs& args, UInt32 provider, TFunction& function, void*& context, const char*& name, MATH::TMathBatchTier& tier )
{
    MATH::SMathBatchProvider p;
    if ( !Dispatcher().GetProvider( kernel, provider, p ) )
        return false;
    function = reinterpret_cast< TFunction >( p.function );
    context = p.context;
    name = p.name;
    tier = p.tier;
    (void) args;
    return true;
}

/*-------------------------------------------------------------------------*/

/** TransformAabbs: the providers against the scalar one and against CTAxisAlignedBox3D::Transformed, bit for bit, over counts, misalignment, in place and strides */
static bool
TransformSweep( UInt64 seed, UInt32 rounds )
{
    CRandom random( seed );
    MATH::CMathBatchDispatcher& dispatcher = Dispatcher();
    const MATH::TMathBatchKernel kernel = (MATH::TMathBatchKernel) GEOMETRYBATCHKERNEL_TRANSFORM_AABBS_F32;
    const UInt32 providerCount = dispatcher.GetProviderCount( kernel );
    if ( providerCount < 1 )
        return false;
    UInt32 checks = 0;
    const UInt32 counts[ 12 ] = { 0, 1, 2, 3, 4, 5, 7, 8, 15, 16, 33, 200 };
    for ( UInt32 round=0; round<rounds; ++round )
    {
        const UInt32 count = counts[ round % 12 ];
        const UInt32 inStride = 24 + (UInt32) random.Index( 4 ) * ( 0 == round % 3 ? 0 : 5 );
        const UInt32 outStride = 24 + (UInt32) random.Index( 4 ) * ( 0 == round % 4 ? 0 : 3 );
        const UInt32 inOffset = (UInt32) random.Index( 4 );
        const UInt32 outOffset = (UInt32) random.Index( 4 );
        const CTMatrix4D< Float32 > matrix = RandomAffine( random );
        GUCEF::vector< UInt8 > in( (size_t) inStride * ( count + 1 ) + 8, 0xA5 );
        for ( UInt32 i=0; i<count; ++i )
        {
            Float32 box[ 6 ];
            RandomBox( random, box );
            memcpy( &in[ inOffset + (size_t) i * inStride ], box, sizeof( box ) );
        }
        // the reference: Transformed of the box class, for non empty boxes the scalar element, empty ones pass through
        GUCEF::vector< UInt8 > expected( (size_t) outStride * ( count + 1 ) + 8, 0x5A );
        for ( UInt32 i=0; i<count; ++i )
        {
            CTAxisAlignedBox3D< Float32 > box;
            memcpy( &box, &in[ inOffset + (size_t) i * inStride ], 24 );
            const CTAxisAlignedBox3D< Float32 > moved = box.Transformed( matrix );
            memcpy( &expected[ outOffset + (size_t) i * outStride ], &moved, 24 );
        }
        for ( UInt32 p=0; p<providerCount; ++p )
        {
            MATH::SMathBatchProvider provider;
            dispatcher.GetProvider( kernel, p, provider );
            // out of place into a buffer of marker bytes: the bytes between the elements must stay
            GUCEF::vector< UInt8 > out( expected.size(), 0x5A );
            SGeometryTransformAabbsF32Args args;
            args.matrix = &matrix;
            args.in = count > 0 ? &in[ inOffset ] : static_cast< const void* >( &in[ 0 ] );
            args.inStrideBytes = inStride;
            args.out = &out[ outOffset ];
            args.outStrideBytes = outStride;
            args.count = count;
            if ( MATH::MATHSTATUS_OK != reinterpret_cast< TGeometryTransformAabbsF32Function >( provider.function )( provider.context, args ) )
                return false;
            for ( UInt32 i=0; i<count; ++i )
                if ( 0 != memcmp( &out[ outOffset + (size_t) i * outStride ], &expected[ outOffset + (size_t) i * outStride ], 24 ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeometryBatchKernels: provider " << provider.name << " differs from Transformed in element " << i << " of " << count << ", seed " << seed << " round " << round );
                    return false;
                }
            // nothing outside the elements was written
            for ( size_t b=0; b<out.size(); ++b )
            {
                const bool inElement = b >= outOffset && count > 0 && ( ( b - outOffset ) % outStride ) < 24 && ( b - outOffset ) / outStride < count;
                if ( !inElement && 0x5A != out[ b ] )
                    return false;
            }
            // in place: the input array doubles as the output with the same stride
            GUCEF::vector< UInt8 > inPlace = in;
            SGeometryTransformAabbsF32Args same = args;
            same.in = &inPlace[ inOffset ];
            same.inStrideBytes = inStride;
            same.out = &inPlace[ inOffset ];
            same.outStrideBytes = inStride;
            if ( 0 == count )
                same.in = same.out = &inPlace[ 0 ];
            if ( MATH::MATHSTATUS_OK != reinterpret_cast< TGeometryTransformAabbsF32Function >( provider.function )( provider.context, same ) )
                return false;
            for ( UInt32 i=0; i<count; ++i )
                if ( 0 != memcmp( &inPlace[ inOffset + (size_t) i * inStride ], &expected[ outOffset + (size_t) i * outStride ], 24 ) )
                    return false;
            ++checks;
        }
        // the front end through the dispatcher, contiguous
        if ( count > 0 )
        {
            GUCEF::vector< TFloat32AxisAlignedBox3D > boxes( count ), moved( count );
            for ( UInt32 i=0; i<count; ++i )
                memcpy( &boxes[ i ], &in[ inOffset + (size_t) i * inStride ], 24 );
            if ( MATH::MATHSTATUS_OK != TransformAabbs( matrix, &boxes[ 0 ], &moved[ 0 ], count ) )
                return false;
            for ( UInt32 i=0; i<count; ++i )
            {
                const CTAxisAlignedBox3D< Float32 > e = boxes[ i ].Transformed( matrix );
                if ( 0 != memcmp( &moved[ i ], &e, 24 ) )
                    return false;
            }
            // forced scalar and realtime flags give the same
            GUCEF::vector< TFloat32AxisAlignedBox3D > again( count );
            if ( MATH::MATHSTATUS_OK != TransformAabbs( matrix, &boxes[ 0 ], &again[ 0 ], count, MATH::MATHBATCH_FORCE_SCALAR ) || 0 != memcmp( &again[ 0 ], &moved[ 0 ], (size_t) count * 24 ) )
                return false;
            if ( MATH::MATHSTATUS_OK != TransformAabbs( matrix, &boxes[ 0 ], &again[ 0 ], count, MATH::MATHBATCH_REALTIME ) || 0 != memcmp( &again[ 0 ], &moved[ 0 ], (size_t) count * 24 ) )
                return false;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeometryBatchKernels: TransformAabbs: " << providerCount << " providers, " << checks << " provider runs bit identical with Transformed" );
    return true;
}

/*-------------------------------------------------------------------------*/

/** CullAabbsFrustum: the providers against the scalar one, and the scalar one against Classify of the frustum */
static bool
CullSweep( UInt64 seed, UInt32 rounds )
{
    CRandom random( seed );
    MATH::CMathBatchDispatcher& dispatcher = Dispatcher();
    const MATH::TMathBatchKernel kernel = (MATH::TMathBatchKernel) GEOMETRYBATCHKERNEL_CULL_AABBS_FRUSTUM_F32;
    const UInt32 providerCount = dispatcher.GetProviderCount( kernel );
    if ( providerCount < 1 )
        return false;
    UInt32 classes[ 3 ] = { 0, 0, 0 };
    UInt32 checks = 0;
    const UInt32 counts[ 10 ] = { 0, 1, 3, 4, 5, 8, 13, 64, 100, 257 };
    const UInt32 planeCounts[ 7 ] = { 6, 6, 6, 0, 1, 7, 33 };
    for ( UInt32 round=0; round<rounds; ++round )
    {
        const UInt32 count = counts[ round % 10 ];
        // planes: the frustum of a random camera, or random planes
        GUCEF::vector< TFloat32Plane > planes;
        CTFrustum< Float32 > frustum;
        bool isFrustum = false;
        const UInt32 planeCount = planeCounts[ round % 7 ];
        if ( 6 == planeCount && 0 != round % 14 )
        {
            RandomShape( random, frustum );
            for ( UInt32 i=0; i<6; ++i )
                planes.push_back( frustum.GetPlane( i ) );
            isFrustum = true;
        }
        else
            for ( UInt32 i=0; i<planeCount; ++i )
            {
                D3 normal = random.Direction();
                planes.push_back( TFloat32Plane( CTDirectionVector3D< Float32 >( CTVector3D< Float32 >( (Float32) normal.AsVector().x, (Float32) normal.AsVector().y, (Float32) normal.AsVector().z ) ), (Float32) random.Uniform( -20.0, 5.0 ) ) );
            }
        const UInt32 inStride = 24 + (UInt32) random.Index( 3 ) * 4;
        const UInt32 inOffset = (UInt32) random.Index( 4 );
        const UInt32 outStride = 1 + (UInt32) random.Index( 3 );
        GUCEF::vector< UInt8 > in( (size_t) inStride * ( count + 1 ) + 8, 0 );
        for ( UInt32 i=0; i<count; ++i )
        {
            Float32 box[ 6 ];
            RandomBox( random, box );
            memcpy( &in[ inOffset + (size_t) i * inStride ], box, sizeof( box ) );
        }
        // the reference: the class function of the plane set
        GUCEF::vector< UInt8 > expected( (size_t) outStride * ( count + 1 ), 0x77 );
        for ( UInt32 i=0; i<count; ++i )
        {
            CTAxisAlignedBox3D< Float32 > box;
            memcpy( &box, &in[ inOffset + (size_t) i * inStride ], 24 );
            UInt8 value = (UInt8) FRUSTUMCLASSIFICATION_OUTSIDE;
            if ( !box.IsEmpty() )
                value = (UInt8) ClassifyBoxToPlaneSet( planes.empty() ? static_cast< const TFloat32Plane* >( GUCEF_NULL ) : &planes[ 0 ], planeCount, box.GetCentre(), box.GetHalfExtents() );
            expected[ (size_t) i * outStride ] = value;
            if ( isFrustum )
            {
                // and the Classify member of the frustum
                if ( !box.IsEmpty() && (UInt8) frustum.Classify( box ) != value )
                    return false;
            }
            if ( value < 3 )
                ++classes[ value ];
        }
        for ( UInt32 p=0; p<providerCount; ++p )
        {
            MATH::SMathBatchProvider provider;
            dispatcher.GetProvider( kernel, p, provider );
            GUCEF::vector< UInt8 > out( expected.size(), 0x77 );
            SGeometryCullAabbsFrustumF32Args args;
            args.planes = planes.empty() ? static_cast< const TFloat32Plane* >( GUCEF_NULL ) : &planes[ 0 ];
            args.planeCount = planeCount;
            args.in = &in[ inOffset ];
            args.inStrideBytes = inStride;
            args.out = &out[ 0 ];
            args.outStrideBytes = outStride;
            args.count = count;
            const MATH::TMathStatus status = reinterpret_cast< TGeometryCullAabbsFrustumF32Function >( provider.function )( provider.context, args );
            if ( MATH::MATHSTATUS_OK != status )
            {
                // only a provider that may fail does, and only for more planes than it can take
                if ( 0 == ( provider.flags & MATH::MATHPROVIDER_MAY_FAIL ) || planeCount <= 32 )
                    return false;
                continue;
            }
            if ( out != expected )
            {
                for ( UInt32 i=0; i<count; ++i )
                    if ( out[ (size_t) i * outStride ] != expected[ (size_t) i * outStride ] )
                    {
                        Float32 box[ 6 ];
                        memcpy( box, &in[ inOffset + (size_t) i * inStride ], 24 );
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeometryBatchKernels: provider " << provider.name << " gives " << (Int32) out[ (size_t) i * outStride ] << " instead of " << (Int32) expected[ (size_t) i * outStride ] << " for box " << i << " ( " << box[ 0 ] << " " << box[ 1 ] << " " << box[ 2 ] << " " << box[ 3 ] << " " << box[ 4 ] << " " << box[ 5 ] << " ), " << planeCount << " planes, seed " << seed << " round " << round );
                        break;
                    }
                return false;
            }
            ++checks;
        }
        // the front end through the dispatcher: more planes than the SIMD provider takes fall back to the scalar one
        if ( count > 0 )
        {
            GUCEF::vector< TFloat32AxisAlignedBox3D > boxes( count );
            for ( UInt32 i=0; i<count; ++i )
                memcpy( &boxes[ i ], &in[ inOffset + (size_t) i * inStride ], 24 );
            GUCEF::vector< UInt8 > out( count, 0x77 );
            if ( MATH::MATHSTATUS_OK != CullAabbsFrustum( planes.empty() ? static_cast< const TFloat32Plane* >( GUCEF_NULL ) : &planes[ 0 ], planeCount, &boxes[ 0 ], &out[ 0 ], count ) )
                return false;
            for ( UInt32 i=0; i<count; ++i )
                if ( out[ i ] != expected[ (size_t) i * outStride ] )
                    return false;
            if ( isFrustum )
            {
                GUCEF::vector< UInt8 > viaFrustum( count, 0x77 );
                CullBoxes( frustum, &boxes[ 0 ], count, &viaFrustum[ 0 ] );
                if ( viaFrustum != out )
                    return false;
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeometryBatchKernels: CullAabbsFrustum: " << providerCount << " providers, " << checks << " provider runs bit identical with the plane set classification (outside " << classes[ 0 ] << ", intersecting " << classes[ 1 ] << ", inside " << classes[ 2 ] << ")" );
    return classes[ 0 ] > 0 && classes[ 1 ] > 0 && classes[ 2 ] > 0;
}

/*-------------------------------------------------------------------------*/

/** The registration: ids, owner, provider flags, invalid calls and the removal by owner when the module global goes */
static bool
RegistrationChecks( void )
{
    MATH::CMathBatchDispatcher& dispatcher = Dispatcher();
    if ( !CGeometryGlobal::Instance()->AreBatchKernelsRegistered() )
        return false;
    const MATH::TMathBatchKernel k0 = (MATH::TMathBatchKernel) GEOMETRYBATCHKERNEL_TRANSFORM_AABBS_F32;
    const MATH::TMathBatchKernel k1 = (MATH::TMathBatchKernel) GEOMETRYBATCHKERNEL_CULL_AABBS_FRUSTUM_F32;
    if ( k0 < MATH::MATHBATCHKERNEL_GEOMETRY_FIRST || k1 > MATH::MATHBATCHKERNEL_GEOMETRY_LAST )
        return false;
    if ( GUCEF_NULL == dispatcher.GetKernelId( k0 ) || 0 != strcmp( "GEOMETRY.TransformAabbs.F32", dispatcher.GetKernelId( k0 ) ) )
        return false;
    if ( GUCEF_NULL == dispatcher.GetKernelId( k1 ) || 0 != strcmp( "GEOMETRY.CullAabbsFrustum.F32", dispatcher.GetKernelId( k1 ) ) )
        return false;
    if ( dispatcher.IsKernelAlwaysDeterministic( k0 ) || dispatcher.IsKernelAlwaysDeterministic( k1 ) )
        return false;
    // the scalar provider comes last in the selection order, every provider is bit exact
    for ( UInt32 k=0; k<2; ++k )
    {
        const MATH::TMathBatchKernel kernel = 0 == k ? k0 : k1;
        const UInt32 n = dispatcher.GetProviderCount( kernel );
        MATH::SMathBatchProvider last;
        if ( n < 1 || !dispatcher.GetProvider( kernel, n - 1, last ) || MATH::MATHBATCHTIER_SCALAR != last.tier )
            return false;
        for ( UInt32 p=0; p<n; ++p )
        {
            MATH::SMathBatchProvider provider;
            dispatcher.GetProvider( kernel, p, provider );
            if ( 0 == ( provider.flags & MATH::MATHPROVIDER_BITEXACT ) )
                return false;
        }
        if ( n > 1 )
        {
            MATH::SMathBatchProvider first;
            dispatcher.GetProvider( kernel, 0, first );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeometryBatchKernels: kernel " << dispatcher.GetKernelId( kernel ) << " has the providers " << first.name << " and " << last.name );
        }
    }
    // the realtime flag selects scalar and SIMD only
    const MATH::SMathBatchProvider* selected = dispatcher.SelectProvider( k0, 100, MATH::MATHBATCH_REALTIME, GUCEF_NULL, GUCEF_NULL );
    if ( GUCEF_NULL == selected || selected->tier > MATH::MATHBATCHTIER_SIMD )
        return false;
    const MATH::SMathBatchProvider* forced = dispatcher.SelectProvider( k1, 100, MATH::MATHBATCH_FORCE_SCALAR, GUCEF_NULL, GUCEF_NULL );
    if ( GUCEF_NULL == forced || MATH::MATHBATCHTIER_SCALAR != forced->tier )
        return false;
    // an argument struct with a stride below the element size is refused without writing
    const CTMatrix4D< Float32 > identity;
    TFloat32AxisAlignedBox3D boxes[ 2 ];
    SGeometryTransformAabbsF32Args bad;
    bad.matrix = &identity;
    bad.in = boxes;
    bad.inStrideBytes = 12;
    bad.out = boxes;
    bad.outStrideBytes = 24;
    bad.count = 2;
    if ( MATH::MATHSTATUS_OK == dispatcher.Execute( k0, bad, 2, 0, boxes, boxes ) )
        return false;
    bad.matrix = GUCEF_NULL;
    bad.inStrideBytes = 24;
    if ( MATH::MATHSTATUS_OK == dispatcher.Execute( k0, bad, 2, 0, boxes, boxes ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The module unload removes the providers of the owner, and the next use of the module global registers them again; the
 *  gucefMATH providers stay. Last among the tests of this suite: it destroys and recreates the global.
 */
static bool
UnloadRemovesProviders( void )
{
    MATH::CMathBatchDispatcher& dispatcher = Dispatcher();
    const MATH::TMathBatchKernel k0 = (MATH::TMathBatchKernel) GEOMETRYBATCHKERNEL_TRANSFORM_AABBS_F32;
    const UInt32 before = dispatcher.GetProviderCount( k0 );
    const UInt32 mathBefore = dispatcher.GetProviderCount( MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 );
    if ( before < 1 || mathBefore < 1 )
        return false;
    GEOMETRY::CModule::Unload();
    if ( 0 != dispatcher.GetProviderCount( k0 ) || 0 != dispatcher.GetProviderCount( (MATH::TMathBatchKernel) GEOMETRYBATCHKERNEL_CULL_AABBS_FRUSTUM_F32 ) )
        return false;
    if ( mathBefore != dispatcher.GetProviderCount( MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 ) )
        return false;
    // a call without providers is refused, not crashed
    const CTMatrix4D< Float32 > identity;
    TFloat32AxisAlignedBox3D box;
    SGeometryTransformAabbsF32Args args;
    args.matrix = &identity;
    args.in = &box;
    args.inStrideBytes = 24;
    args.out = &box;
    args.outStrideBytes = 24;
    args.count = 1;
    if ( MATH::MATHSTATUS_OK == dispatcher.Execute( k0, args, 1, 0, &box, &box ) )
        return false;
    // the front end creates the module global again, which registers the kernels
    if ( MATH::MATHSTATUS_OK != TransformAabbs( identity, &box, &box, 1 ) )
        return false;
    return before == dispatcher.GetProviderCount( k0 ) && CGeometryGlobal::Instance()->AreBatchKernelsRegistered();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGeometryBatchKernelsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "GeometryBatchKernels" );

    GUCEF_TESTFW_TESTCASE( "GeometryBatchKernels: the kernels are registered lazily with the dispatcher of gucefMATH under ids 256 and 257, scalar last, all bit exact" )
    try
    {
        ASSERT_TRUE( RegistrationChecks() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeometryBatchKernels: TransformAabbs providers are bit identical with Transformed over counts, misalignment, strides and in place" )
    try
    {
        ASSERT_TRUE( TransformSweep( 11101, ScaledCount( 600 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeometryBatchKernels: CullAabbsFrustum providers are bit identical with the plane set classification and the frustum" )
    try
    {
        ASSERT_TRUE( CullSweep( 11102, ScaledCount( 700 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeometryBatchKernels: unloading the module removes its providers by owner and the next use registers them again" )
    try
    {
        ASSERT_TRUE( UnloadRemovesProviders() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
