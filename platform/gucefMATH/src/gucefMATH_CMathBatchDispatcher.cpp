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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "gucefMATH_CMathBatchDispatcher.h"

#include "gucefMATH_BatchKernels.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The immutable state of one kernel: published once, never changed afterwards */
struct CMathBatchDispatcher::SProviderList
{
    UInt32 providerCount;
    SMathBatchProvider providers[ CMathBatchDispatcher::MaxProvidersPerKernel ];
    const void* owners[ CMathBatchDispatcher::MaxProvidersPerKernel ];
    bool alwaysDeterministic;
    const void* kernelOwner;                                        /**< NULL for MATH kernels */
    char kernelId[ CMathBatchDispatcher::MaxKernelIdLength + 1 ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CMathBatchDispatcher::MaxProvidersPerKernel;
const UInt32 CMathBatchDispatcher::MaxKernelIdLength;

namespace {

/** The string ids of the MATH kernels, in the order of EMathBatchKernel (gucefCOMPUTE section 5.1) */
const char* const MathKernelIds[ MATHBATCHKERNEL_MATH_COUNT ] =
{
    "MATH.TransformPoints3.F32",
    "MATH.TransformVectors3.F32",
    "MATH.TransformNormals3.F32",
    "MATH.TransformVectors4.F32",
    "MATH.MultiplyMatrices4.F32",
    "MATH.QuaternionsToMatrices.F32",
    "MATH.DotProducts3.F32",
    "MATH.NormalizeVectors3.F32",
    "MATH.ConvertF32ToF16.F16",
    "MATH.ConvertF16ToF32.F32",
    "MATH.TransformPoints3.F64",
    "MATH.TransformVectors3.F64",
    "MATH.MultiplyMatrices4.F64",
    "MATH.TransformPoints3.FX32",
    "MATH.MultiplyMatrices4.FX32",
    "MATH.VbapGains2D.F32",
    "MATH.NoiseField2D.F32",
    "MATH.NoiseField3D.F32"
};

/** The kernels over Q16.16 fixed point: every call of them is deterministic */
GUCEF_MATH_FORCE_INLINE bool
IsFixedPointKernel( UInt32 kernel )
{
    return (UInt32) MATHBATCHKERNEL_TRANSFORM_POINTS3_FX32 == kernel || (UInt32) MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32 == kernel;
}

GUCEF_MATH_FORCE_INLINE UInt32
MinTier( UInt32 a, UInt32 b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a < b ? a : b;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMathBatchDispatcher::CMathBatchDispatcher( void )
    : m_maxTier( (UInt32) MATHBATCHTIER_ACCELERATOR )
    , m_cpuFeatures( CORE::GetCpuIsaFeatures() )
    , m_lock()
    , m_allLists()
{GUCEF_TRACE;

    for ( UInt32 k=0; k<MATHBATCHKERNEL_SLOT_COUNT; ++k )
    {
        m_lists[ k ].Store( GUCEF_NULL, MT::MEMORDER_RELAXED );
        m_kernelMaxTier[ k ].Store( (UInt32) MATHBATCHTIER_ACCELERATOR, MT::MEMORDER_RELAXED );
        m_epochs[ k ].Store( 0, MT::MEMORDER_RELAXED );
        m_activeCalls[ k ][ 0 ].Store( 0, MT::MEMORDER_RELAXED );
        m_activeCalls[ k ][ 1 ].Store( 0, MT::MEMORDER_RELAXED );
    }

    MT::CScopeMutex lock( m_lock );
    m_allLists.reserve( 4 * MATHBATCHKERNEL_MATH_COUNT );
    for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
    {
        SProviderList* list = GUCEF_NEW SProviderList;
        memset( list, 0, sizeof( SProviderList ) );
        strncpy( list->kernelId, MathKernelIds[ k ], MaxKernelIdLength );
        list->alwaysDeterministic = IsFixedPointKernel( k );
        Publish( (TMathBatchKernel) k, list );
    }
}

/*-------------------------------------------------------------------------*/

CMathBatchDispatcher::~CMathBatchDispatcher()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 k=0; k<MATHBATCHKERNEL_SLOT_COUNT; ++k )
        m_lists[ k ].Store( GUCEF_NULL, MT::MEMORDER_RELEASE );
    for ( TProviderListVector::iterator i = m_allLists.begin(); i != m_allLists.end(); ++i )
        GUCEF_DELETE (*i);
    m_allLists.clear();
}

/*-------------------------------------------------------------------------*/

bool
CMathBatchDispatcher::IsValidKernel( TMathBatchKernel kernel )
{
    const UInt32 id = (UInt32) kernel;
    return id < (UInt32) MATHBATCHKERNEL_MATH_COUNT || ( id >= (UInt32) MATHBATCHKERNEL_EXTERNAL_FIRST && id <= (UInt32) MATHBATCHKERNEL_EXTERNAL_LAST );
}

/*-------------------------------------------------------------------------*/

const CMathBatchDispatcher::SProviderList*
CMathBatchDispatcher::LoadList( TMathBatchKernel kernel ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !IsValidKernel( kernel ) )
        return GUCEF_NULL;
    return static_cast< const SProviderList* >( m_lists[ kernel ].Load( MT::MEMORDER_ACQUIRE ) );
}

/*-------------------------------------------------------------------------*/

void
CMathBatchDispatcher::Publish( TMathBatchKernel kernel, SProviderList* list )
{
    // called with m_lock held; the list is kept until destruction because calls may still be reading the old one
    if ( GUCEF_NULL != list )
        m_allLists.push_back( list );
    m_lists[ kernel ].Store( list, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

UInt32
CMathBatchDispatcher::RegisterBuiltInProviders( UInt64 allowedCpuFeatures )
{GUCEF_TRACE;

    SMathBatchKernelSet scalarSet;
    memset( &scalarSet, 0, sizeof( scalarSet ) );
    MathBatchGetScalarKernels( scalarSet );

    // the SIMD sets in order of preference: per kernel the first one available on this CPU is registered
    // (only the sets of the instruction set family the build targets have functions; the others are empty)
    const UInt32 simdSetCount = 5;
    SMathBatchKernelSet simdSets[ 5 ];
    memset( simdSets, 0, sizeof( simdSets ) );
    MathBatchGetAvx2Kernels( simdSets[ 0 ] );
    MathBatchGetF16cKernels( simdSets[ 1 ] );
    MathBatchGetSse2Kernels( simdSets[ 2 ] );
    MathBatchGetNeonKernels( simdSets[ 3 ] );
    MathBatchGetWasm128Kernels( simdSets[ 4 ] );

    const UInt64 usableFeatures = m_cpuFeatures & allowedCpuFeatures;
    UInt32 registered = 0;
    for ( UInt32 k=0; k<MATHBATCHKERNEL_MATH_COUNT; ++k )
    {
        SMathBatchProvider provider;
        provider.tier = MATHBATCHTIER_SCALAR;
        provider.name = scalarSet.name;
        provider.function = scalarSet.functions[ k ];
        provider.minCount = 0;
        provider.maxCount = 0xFFFFFFFF;
        provider.flags = MATHPROVIDER_BITEXACT;
        provider.alignment = 0;
        provider.requiredCpuFeatures = 0;
        provider.context = GUCEF_NULL;
        if ( RegisterProvider( (TMathBatchKernel) k, provider, this ) )
            ++registered;

        for ( UInt32 s=0; s<simdSetCount; ++s )
        {
            const SMathBatchKernelSet& set = simdSets[ s ];
            if ( GUCEF_NULL == set.functions[ k ] || set.requiredCpuFeatures != ( usableFeatures & set.requiredCpuFeatures ) )
                continue;
            provider.tier = set.tier;
            provider.name = set.name;
            provider.function = set.functions[ k ];
            provider.requiredCpuFeatures = set.requiredCpuFeatures;
            provider.flags = 0 != ( set.notBitExactKernels & ( 1u << k ) ) ? 0 : MATHPROVIDER_BITEXACT;
            #if ( GUCEF_CPU_REQUIRES_ALIGNED_ACCESS == 1 )
            // section 5.4: on such CPUs the SIMD providers only serve calls whose pointers and strides are 16 byte multiples
            provider.flags |= MATHPROVIDER_REQUIRES_ALIGNED_INPUT;
            provider.alignment = 16;
            #endif
            if ( RegisterProvider( (TMathBatchKernel) k, provider, this ) )
                ++registered;
            break;
        }
    }
    return registered;
}

/*-------------------------------------------------------------------------*/

bool
CMathBatchDispatcher::RegisterProvider( TMathBatchKernel kernel, const SMathBatchProvider& provider, const void* owner )
{GUCEF_TRACE;

    if ( !IsValidKernel( kernel ) || GUCEF_NULL == provider.function || GUCEF_NULL == owner )
        return false;
    if ( (UInt32) provider.tier > (UInt32) MATHBATCHTIER_ACCELERATOR || provider.minCount > provider.maxCount )
        return false;
    if ( 0 != ( provider.flags & MATHPROVIDER_REQUIRES_ALIGNED_INPUT ) &&
         ( 0 == provider.alignment || 0 != ( provider.alignment & ( provider.alignment - 1 ) ) ) )
        return false;
    if ( provider.requiredCpuFeatures != ( m_cpuFeatures & provider.requiredCpuFeatures ) )
        return false;

    MT::CScopeMutex lock( m_lock );
    const SProviderList* current = static_cast< const SProviderList* >( m_lists[ kernel ].Load( MT::MEMORDER_ACQUIRE ) );
    if ( GUCEF_NULL == current || current->providerCount >= MaxProvidersPerKernel )
        return false;

    SProviderList* updated = GUCEF_NEW SProviderList( *current );

    // descending tier, registration order within a tier
    UInt32 position = 0;
    while ( position < updated->providerCount && (UInt32) updated->providers[ position ].tier >= (UInt32) provider.tier )
        ++position;
    for ( UInt32 i=updated->providerCount; i>position; --i )
    {
        updated->providers[ i ] = updated->providers[ i - 1 ];
        updated->owners[ i ] = updated->owners[ i - 1 ];
    }
    updated->providers[ position ] = provider;
    updated->owners[ position ] = owner;
    ++updated->providerCount;

    Publish( kernel, updated );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CMathBatchDispatcher::UnregisterProviders( const void* owner )
{GUCEF_TRACE;

    if ( GUCEF_NULL == owner )
        return 0;

    MT::CScopeMutex lock( m_lock );
    UInt32 removed = 0;
    for ( UInt32 k=0; k<MATHBATCHKERNEL_SLOT_COUNT; ++k )
    {
        const SProviderList* current = static_cast< const SProviderList* >( m_lists[ k ].Load( MT::MEMORDER_ACQUIRE ) );
        if ( GUCEF_NULL == current )
            continue;

        UInt32 owned = 0;
        for ( UInt32 i=0; i<current->providerCount; ++i )
        {
            if ( owner == current->owners[ i ] )
                ++owned;
        }
        const bool dropKernel = owner == current->kernelOwner && owned == current->providerCount;
        if ( 0 == owned && !dropKernel )
            continue;

        removed += owned;
        if ( dropKernel )
        {
            // an external kernel of this owner without providers of others: the id becomes free again
            Publish( (TMathBatchKernel) k, GUCEF_NULL );
            WaitForCallsToLeave( k );
            continue;
        }

        SProviderList* updated = GUCEF_NEW SProviderList( *current );
        UInt32 kept = 0;
        for ( UInt32 i=0; i<current->providerCount; ++i )
        {
            if ( owner == current->owners[ i ] )
                continue;
            updated->providers[ kept ] = current->providers[ i ];
            updated->owners[ kept ] = current->owners[ i ];
            ++kept;
        }
        updated->providerCount = kept;
        Publish( (TMathBatchKernel) k, updated );
        WaitForCallsToLeave( k );
    }
    return removed;
}

/*-------------------------------------------------------------------------*/

MT::CTAtomic< UInt32 >*
CMathBatchDispatcher::EnterCall( TMathBatchKernel kernel ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !IsValidKernel( kernel ) )
        return GUCEF_NULL;

    // Counted in the counter of the epoch read before and after the increment (sequentially consistent, as the epoch
    // change and the counter read of WaitForCallsToLeave): either the waiting thread sees this count, or this call sees
    // the new epoch and counts itself there, after the replacement list was published.
    for ( ;; )
    {
        const UInt32 epoch = m_epochs[ kernel ].Load( MT::MEMORDER_SEQ_CST );
        MT::CTAtomic< UInt32 >* counter = &m_activeCalls[ kernel ][ epoch & 1 ];
        counter->FetchAdd( 1, MT::MEMORDER_SEQ_CST );
        if ( epoch == m_epochs[ kernel ].Load( MT::MEMORDER_SEQ_CST ) )
            return counter;
        counter->FetchSub( 1, MT::MEMORDER_RELEASE );
    }
}

/*-------------------------------------------------------------------------*/

void
CMathBatchDispatcher::WaitForCallsToLeave( UInt32 kernel )
{
    // the calls that entered before this point are in the counter of the old epoch; new calls use the other counter,
    // so the old one drains even while calls keep coming. Every replacement waits, so the other counter is empty here.
    const UInt32 epoch = m_epochs[ kernel ].Load( MT::MEMORDER_SEQ_CST );
    m_epochs[ kernel ].Store( epoch + 1, MT::MEMORDER_SEQ_CST );
    MT::CTAtomic< UInt32 >& old = m_activeCalls[ kernel ][ epoch & 1 ];
    UInt32 spins = 0;
    while ( 0 != old.Load( MT::MEMORDER_SEQ_CST ) )
    {
        if ( ++spins < 1000 )
            MT::AtomicCpuRelax();
        else
            MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

void
CMathBatchDispatcher::SetMaxTier( TMathBatchTier tier )
{
    m_maxTier.Store( MinTier( (UInt32) tier, (UInt32) MATHBATCHTIER_ACCELERATOR ), MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

void
CMathBatchDispatcher::SetMaxTier( TMathBatchKernel kernel, TMathBatchTier tier )
{
    if ( IsValidKernel( kernel ) )
        m_kernelMaxTier[ kernel ].Store( MinTier( (UInt32) tier, (UInt32) MATHBATCHTIER_ACCELERATOR ), MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

TMathBatchTier
CMathBatchDispatcher::GetMaxTier( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return (TMathBatchTier) MinTier( (UInt32) GUCEF_MATH_BATCH_MAX_TIER_LEVEL, m_maxTier.Load( MT::MEMORDER_RELAXED ) );
}

/*-------------------------------------------------------------------------*/

TMathBatchTier
CMathBatchDispatcher::GetMaxTier( TMathBatchKernel kernel ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !IsValidKernel( kernel ) )
        return MATHBATCHTIER_SCALAR;
    return (TMathBatchTier) MinTier( (UInt32) GetMaxTier(), m_kernelMaxTier[ kernel ].Load( MT::MEMORDER_RELAXED ) );
}

/*-------------------------------------------------------------------------*/

const SMathBatchProvider*
CMathBatchDispatcher::SelectProvider( TMathBatchKernel kernel, UInt32 count, UInt32 flags, const void* in, const void* out ) const GUCEF_MATH_REALTIME_CONTEXT
{
    SSelection selection;
    return SelectNext( kernel, selection, count, flags, (size_t) in | (size_t) out );
}

/*-------------------------------------------------------------------------*/

const SMathBatchProvider*
CMathBatchDispatcher::SelectNextProvider( TMathBatchKernel kernel, SSelection& selection, UInt32 count, UInt32 flags, const void* in, const void* out ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return SelectNext( kernel, selection, count, flags, (size_t) in | (size_t) out );
}

/*-------------------------------------------------------------------------*/

const SMathBatchProvider*
CMathBatchDispatcher::SelectNext( TMathBatchKernel kernel, SSelection& selection, UInt32 count, UInt32 flags, size_t addressBits ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const SProviderList* list = static_cast< const SProviderList* >( selection.list );
    if ( GUCEF_NULL == list )
    {
        list = LoadList( kernel );
        if ( GUCEF_NULL == list )
            return GUCEF_NULL;
        selection.list = list;
        selection.next = 0;
    }

    UInt32 cap = (UInt32) GetMaxTier( kernel );
    if ( 0 != ( flags & MATHBATCH_REALTIME ) )
        cap = MinTier( cap, (UInt32) MATHBATCHTIER_SIMD );
    if ( 0 != ( flags & MATHBATCH_FORCE_SCALAR ) )
        cap = (UInt32) MATHBATCHTIER_SCALAR;
    const bool deterministic = 0 != ( flags & MATHBATCH_DETERMINISTIC ) || list->alwaysDeterministic;

    for ( UInt32 i=selection.next; i<list->providerCount; ++i )
    {
        const SMathBatchProvider& provider = list->providers[ i ];
        const UInt32 tier = (UInt32) provider.tier;
        if ( tier > cap )
            continue;
        if ( ( (UInt32) MATHBATCHTIER_MULTICORE == tier && 0 != ( flags & MATHBATCH_NO_MULTICORE ) ) ||
             ( (UInt32) MATHBATCHTIER_ACCELERATOR == tier && 0 != ( flags & MATHBATCH_NO_ACCELERATOR ) ) )
            continue;
        if ( count < provider.minCount || count > provider.maxCount )
            continue;
        if ( deterministic && 0 == ( provider.flags & MATHPROVIDER_BITEXACT ) )
            continue;
        if ( 0 != ( provider.flags & MATHPROVIDER_REQUIRES_ALIGNED_INPUT ) && 0 != ( addressBits & ( (size_t) provider.alignment - 1 ) ) )
            continue;
        selection.next = i + 1;
        return &provider;
    }
    selection.next = list->providerCount;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CMathBatchDispatcher::GetProviderCount( TMathBatchKernel kernel ) const
{
    const SProviderList* list = LoadList( kernel );
    return GUCEF_NULL != list ? list->providerCount : 0;
}

/*-------------------------------------------------------------------------*/

bool
CMathBatchDispatcher::GetProvider( TMathBatchKernel kernel, UInt32 index, SMathBatchProvider& out ) const
{
    const SProviderList* list = LoadList( kernel );
    if ( GUCEF_NULL == list || index >= list->providerCount )
        return false;
    out = list->providers[ index ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMathBatchDispatcher::RegisterExternalKernel( TMathBatchKernel kernel, const char* kernelId, bool alwaysDeterministic, const void* owner )
{GUCEF_TRACE;

    const UInt32 id = (UInt32) kernel;
    if ( id < (UInt32) MATHBATCHKERNEL_EXTERNAL_FIRST || id > (UInt32) MATHBATCHKERNEL_EXTERNAL_LAST || GUCEF_NULL == kernelId || GUCEF_NULL == owner )
        return false;
    const size_t length = strlen( kernelId );
    if ( 0 == length || length > MaxKernelIdLength )
        return false;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 k=0; k<MATHBATCHKERNEL_SLOT_COUNT; ++k )
    {
        const SProviderList* other = static_cast< const SProviderList* >( m_lists[ k ].Load( MT::MEMORDER_ACQUIRE ) );
        if ( GUCEF_NULL == other || 0 != strcmp( other->kernelId, kernelId ) )
            continue;
        // the same id: only the same kernel slot with the same determinism is accepted again
        return k == id && other->alwaysDeterministic == alwaysDeterministic;
    }
    if ( GUCEF_NULL != m_lists[ id ].Load( MT::MEMORDER_ACQUIRE ) )
        return false;

    SProviderList* list = GUCEF_NEW SProviderList;
    memset( list, 0, sizeof( SProviderList ) );
    memcpy( list->kernelId, kernelId, length );
    list->alwaysDeterministic = alwaysDeterministic;
    list->kernelOwner = owner;
    Publish( kernel, list );
    return true;
}

/*-------------------------------------------------------------------------*/

const char*
CMathBatchDispatcher::GetKernelId( TMathBatchKernel kernel ) const
{
    const SProviderList* list = LoadList( kernel );
    return GUCEF_NULL != list ? list->kernelId : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CMathBatchDispatcher::IsKernelAlwaysDeterministic( TMathBatchKernel kernel ) const
{
    const SProviderList* list = LoadList( kernel );
    return GUCEF_NULL != list && list->alwaysDeterministic;
}

/*-------------------------------------------------------------------------*/

UInt64
CMathBatchDispatcher::GetCpuIsaFeatures( void ) const
{
    return m_cpuFeatures;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
