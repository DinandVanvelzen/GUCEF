/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
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

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#include "gucefCOMPUTE_CComputeGlobal.h"
#define GUCEF_COMPUTE_CCOMPUTEGLOBAL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEGLOBAL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H
#include "gucefCOMPUTE_CComputeKernelRegistry.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELREGISTRY_H ? */

#ifndef GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H
#include "gucefCOMPUTE_CCpuComputeKernel.h"
#define GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H
#endif /* GUCEF_COMPUTE_CCPUCOMPUTEKERNEL_H ? */

#include "gucefCOMPUTE_CCpuTestKernels.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CCpuTestKernels::FillKernelId = "COMPUTE.Fill.U32";
const char* CCpuTestKernels::SaxpyKernelId = "COMPUTE.Saxpy.F32";
const char* CCpuTestKernels::ReduceSumKernelId = "COMPUTE.ReduceSum.F32";
const char* CCpuTestKernels::ScanGroupsKernelId = "COMPUTE.ScanGroups.U32";
const char* CCpuTestKernels::AddGroupOffsetsKernelId = "COMPUTE.AddGroupOffsets.U32";
const char* CCpuTestKernels::HashKernelId = "COMPUTE.Hash.U32";
const UInt32 CCpuTestKernels::GroupSize = 256;

/*-------------------------------------------------------------------------*/

namespace {

/** The items [ first, end ) of a range of groups, limited by the item count */
inline void
GroupItems( const CCpuDispatchContext& context, UInt32 firstGroup, UInt32 groupCount, UInt32& first, UInt32& end )
{
    const UInt64 items = context.GetItemCount();
    const UInt64 begin = static_cast< UInt64 >( firstGroup ) * CCpuTestKernels::GroupSize;
    const UInt64 stop = begin + static_cast< UInt64 >( groupCount ) * CCpuTestKernels::GroupSize;
    first = static_cast< UInt32 >( begin < items ? begin : items );
    end = static_cast< UInt32 >( stop < items ? stop : items );
}

/*-------------------------------------------------------------------------*/

class CFillKernel : public CCpuComputeKernel
{
    public:
    virtual void ExecuteGroups( const CCpuDispatchContext& context, UInt32 firstGroup, UInt32 groupCount ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const CCpuTestKernels::SFillConstants& constants = context.GetConstantsAs< CCpuTestKernels::SFillConstants >();
        UInt32* out = context.GetBufferAs< UInt32 >( 0 );
        const UInt64 capacity = context.GetBufferElementCount< UInt32 >( 0 );
        UInt32 first = 0, end = 0;
        GroupItems( context, firstGroup, groupCount, first, end );
        for ( UInt32 i=first; i<end && i<capacity; ++i )
            out[ i ] = constants.base + i * constants.step;
    }
};

/*-------------------------------------------------------------------------*/

class CSaxpyKernel : public CCpuComputeKernel
{
    public:
    virtual void ExecuteGroups( const CCpuDispatchContext& context, UInt32 firstGroup, UInt32 groupCount ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const CCpuTestKernels::SSaxpyConstants& constants = context.GetConstantsAs< CCpuTestKernels::SSaxpyConstants >();
        const Float32* x = context.GetBufferAs< const Float32 >( 0 );
        Float32* y = context.GetBufferAs< Float32 >( 1 );
        UInt64 capacity = context.GetBufferElementCount< Float32 >( 0 );
        if ( context.GetBufferElementCount< Float32 >( 1 ) < capacity )
            capacity = context.GetBufferElementCount< Float32 >( 1 );
        UInt32 first = 0, end = 0;
        GroupItems( context, firstGroup, groupCount, first, end );
        for ( UInt32 i=first; i<end && i<capacity; ++i )
        {
            // rounded product, then the sum: no contraction into a fused multiply-add
            volatile Float32 product = constants.a * x[ i ];
            y[ i ] = product + y[ i ];
        }
    }
};

/*-------------------------------------------------------------------------*/

class CReduceSumKernel : public CCpuComputeKernel
{
    public:
    virtual void ExecuteGroups( const CCpuDispatchContext& context, UInt32 firstGroup, UInt32 groupCount ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const Float32* in = context.GetBufferAs< const Float32 >( 0 );
        Float32* partials = context.GetBufferAs< Float32 >( 1 );
        const UInt64 inCapacity = context.GetBufferElementCount< Float32 >( 0 );
        const UInt64 outCapacity = context.GetBufferElementCount< Float32 >( 1 );
        for ( UInt32 g=firstGroup; g<firstGroup+groupCount && g<outCapacity; ++g )
        {
            UInt32 first = 0, end = 0;
            GroupItems( context, g, 1, first, end );
            Float32 sum = 0.0f;
            for ( UInt32 i=first; i<end && i<inCapacity; ++i )
                sum += in[ i ];
            partials[ g ] = sum;
        }
    }
};

/*-------------------------------------------------------------------------*/

class CScanGroupsKernel : public CCpuComputeKernel
{
    public:
    virtual void ExecuteGroups( const CCpuDispatchContext& context, UInt32 firstGroup, UInt32 groupCount ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32* in = context.GetBufferAs< const UInt32 >( 0 );
        UInt32* out = context.GetBufferAs< UInt32 >( 1 );
        UInt32* totals = context.GetBufferAs< UInt32 >( 2 );
        UInt32* shared = static_cast< UInt32* >( context.GetScratch() );
        const UInt64 capacity = context.GetBufferElementCount< UInt32 >( 0 ) < context.GetBufferElementCount< UInt32 >( 1 ) ?
                                context.GetBufferElementCount< UInt32 >( 0 ) : context.GetBufferElementCount< UInt32 >( 1 );
        const UInt64 totalCapacity = context.GetBufferElementCount< UInt32 >( 2 );
        if ( GUCEF_NULL == shared || context.GetScratchSizeInBytes() < CCpuTestKernels::GroupSize * sizeof( UInt32 ) )
            return;

        for ( UInt32 g=firstGroup; g<firstGroup+groupCount; ++g )
        {
            UInt32 first = 0, end = 0;
            GroupItems( context, g, 1, first, end );
            if ( end > capacity )
                end = static_cast< UInt32 >( capacity );

            // load into the workgroup memory, scan there (Hillis-Steele, as a GPU group would), store
            const UInt32 count = end > first ? end - first : 0;
            for ( UInt32 i=0; i<CCpuTestKernels::GroupSize; ++i )
                shared[ i ] = i < count ? in[ first + i ] : 0;
            for ( UInt32 offset=1; offset<CCpuTestKernels::GroupSize; offset <<= 1 )
                for ( UInt32 i=CCpuTestKernels::GroupSize-1; i>=offset; --i )
                    shared[ i ] += shared[ i - offset ];
            for ( UInt32 i=0; i<count; ++i )
                out[ first + i ] = shared[ i ];
            if ( g < totalCapacity )
                totals[ g ] = shared[ CCpuTestKernels::GroupSize - 1 ];
        }
    }
};

/*-------------------------------------------------------------------------*/

class CAddGroupOffsetsKernel : public CCpuComputeKernel
{
    public:
    virtual void ExecuteGroups( const CCpuDispatchContext& context, UInt32 firstGroup, UInt32 groupCount ) const GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32* data = context.GetBufferAs< UInt32 >( 0 );
        const UInt32* offsets = context.GetBufferAs< const UInt32 >( 1 );
        const UInt64 capacity = context.GetBufferElementCount< UInt32 >( 0 );
        const UInt64 offsetCapacity = context.GetBufferElementCount< UInt32 >( 1 );
        UInt32 first = 0, end = 0;
        GroupItems( context, firstGroup, groupCount, first, end );
        for ( UInt32 i=first; i<end && i<capacity; ++i )
        {
            const UInt32 group = i / CCpuTestKernels::GroupSize;
            if ( group > 0 && group - 1 < offsetCapacity )
                data[ i ] += offsets[ group - 1 ];
        }
    }
};

/*-------------------------------------------------------------------------*/

class CHashKernel : public CCpuComputeKernel
{
    public:
    virtual void ExecuteGroups( const CCpuDispatchContext& context, UInt32 firstGroup, UInt32 groupCount ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const CCpuTestKernels::SHashConstants& constants = context.GetConstantsAs< CCpuTestKernels::SHashConstants >();
        const UInt32* in = context.GetBufferAs< const UInt32 >( 0 );
        UInt32* out = context.GetBufferAs< UInt32 >( 1 );
        const UInt64 capacity = context.GetBufferElementCount< UInt32 >( 0 ) < context.GetBufferElementCount< UInt32 >( 1 ) ?
                                context.GetBufferElementCount< UInt32 >( 0 ) : context.GetBufferElementCount< UInt32 >( 1 );
        UInt32 first = 0, end = 0;
        GroupItems( context, firstGroup, groupCount, first, end );
        for ( UInt32 i=first; i<end && i<capacity; ++i )
            out[ i ] = CCpuTestKernels::Hash( in[ i ] ^ constants.seed );
    }
};

/*-------------------------------------------------------------------------*/

CComputeKernelDecl
MakeDecl( const char* kernelId, TComputeDeterminism determinism, UInt32 constantsSize, const char* description )
{
    CComputeKernelDecl decl( kernelId, 1 );
    decl.GetSignature().SetWorkgroupSize( CCpuTestKernels::GroupSize, 1, 1 );
    decl.GetSignature().SetConstantsSizeInBytes( constantsSize );
    decl.SetDeterminism( determinism );
    decl.SetDescription( description );
    return decl;
}

}

/*-------------------------------------------------------------------------*/

UInt32
CCpuTestKernels::Hash( UInt32 value )
{GUCEF_TRACE;

    value ^= value >> 16;
    value *= 0x85EBCA6Bu;
    value ^= value >> 13;
    value *= 0xC2B2AE35u;
    value ^= value >> 16;
    return value;
}

/*-------------------------------------------------------------------------*/

CCpuTestKernels::CCpuTestKernels( void )
    : m_registry( GUCEF_NULL )
{GUCEF_TRACE;

    m_factories[ 0 ] = GUCEF_NEW CORE::CTFactory< CCpuComputeKernel, CFillKernel, MT::CMutex >();
    m_factories[ 1 ] = GUCEF_NEW CORE::CTFactory< CCpuComputeKernel, CSaxpyKernel, MT::CMutex >();
    m_factories[ 2 ] = GUCEF_NEW CORE::CTFactory< CCpuComputeKernel, CReduceSumKernel, MT::CMutex >();
    m_factories[ 3 ] = GUCEF_NEW CORE::CTFactory< CCpuComputeKernel, CScanGroupsKernel, MT::CMutex >();
    m_factories[ 4 ] = GUCEF_NEW CORE::CTFactory< CCpuComputeKernel, CAddGroupOffsetsKernel, MT::CMutex >();
    m_factories[ 5 ] = GUCEF_NEW CORE::CTFactory< CCpuComputeKernel, CHashKernel, MT::CMutex >();
}

/*-------------------------------------------------------------------------*/

CCpuTestKernels::~CCpuTestKernels()
{GUCEF_TRACE;

    Unregister();
    for ( UInt32 i=0; i<KernelCount; ++i )
    {
        GUCEF_DELETE m_factories[ i ];
        m_factories[ i ] = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CCpuTestKernels::Register( CComputeKernelRegistry& registry )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_registry )
        return false;

    CComputeKernelDecl decls[ KernelCount ];

    decls[ 0 ] = MakeDecl( FillKernelId, DETERMINISM_EXACT_INTEGER, sizeof( SFillConstants ), "out[ i ] = base + i * step" );
    decls[ 0 ].GetSignature().AddBinding( 0, COMPUTEACCESS_WRITE, ELEMENTTYPE_UINT32 );

    decls[ 1 ] = MakeDecl( SaxpyKernelId, DETERMINISM_FLOAT_STRICT, sizeof( SSaxpyConstants ), "y = a * x + y" );
    decls[ 1 ].GetSignature().AddBinding( 0, COMPUTEACCESS_READ, ELEMENTTYPE_FLOAT32 );
    decls[ 1 ].GetSignature().AddBinding( 1, COMPUTEACCESS_READ_WRITE, ELEMENTTYPE_FLOAT32 );
    decls[ 1 ].SetTolerance( CComputeTolerance( 1, 0.0, 0.0 ) );

    decls[ 2 ] = MakeDecl( ReduceSumKernelId, DETERMINISM_FLOAT_STRICT, GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE, "per group sums" );
    decls[ 2 ].GetSignature().AddBinding( 0, COMPUTEACCESS_READ, ELEMENTTYPE_FLOAT32 );
    decls[ 2 ].GetSignature().AddBinding( 1, COMPUTEACCESS_WRITE, ELEMENTTYPE_FLOAT32 );
    decls[ 2 ].SetTolerance( CComputeTolerance( 0, 0.0, 1.0e-5 ) );

    decls[ 3 ] = MakeDecl( ScanGroupsKernelId, DETERMINISM_EXACT_INTEGER, GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE, "inclusive scan per group" );
    decls[ 3 ].GetSignature().AddBinding( 0, COMPUTEACCESS_READ, ELEMENTTYPE_UINT32 );
    decls[ 3 ].GetSignature().AddBinding( 1, COMPUTEACCESS_WRITE, ELEMENTTYPE_UINT32 );
    decls[ 3 ].GetSignature().AddBinding( 2, COMPUTEACCESS_WRITE, ELEMENTTYPE_UINT32 );
    decls[ 3 ].GetSignature().SetWorkgroupMemoryInBytes( GroupSize * sizeof( UInt32 ) );

    decls[ 4 ] = MakeDecl( AddGroupOffsetsKernelId, DETERMINISM_EXACT_INTEGER, GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE, "adds scanned group totals" );
    decls[ 4 ].GetSignature().AddBinding( 0, COMPUTEACCESS_READ_WRITE, ELEMENTTYPE_UINT32 );
    decls[ 4 ].GetSignature().AddBinding( 1, COMPUTEACCESS_READ, ELEMENTTYPE_UINT32 );

    decls[ 5 ] = MakeDecl( HashKernelId, DETERMINISM_EXACT_INTEGER, sizeof( SHashConstants ), "integer hash" );
    decls[ 5 ].GetSignature().AddBinding( 0, COMPUTEACCESS_READ, ELEMENTTYPE_UINT32 );
    decls[ 5 ].GetSignature().AddBinding( 1, COMPUTEACCESS_WRITE, ELEMENTTYPE_UINT32 );

    m_registry = &registry;
    bool allRegistered = true;
    for ( UInt32 i=0; i<KernelCount; ++i )
    {
        if ( !registry.RegisterKernel( decls[ i ], this ) ||
             !registry.RegisterArtefact( decls[ i ].GetKernelId(), CComputeKernelArtefact::ForCpu( m_factories[ i ] ) ) )
            allRegistered = false;
    }
    return allRegistered;
}

/*-------------------------------------------------------------------------*/

UInt32
CCpuTestKernels::Unregister( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_registry )
        return 0;
    m_registry = GUCEF_NULL;

    // the registry of the global that exists now: after an unload of the module the one registered with is gone
    CComputeGlobal* global = CComputeGlobal::InstanceIfCreated();
    if ( GUCEF_NULL == global )
        return 0;
    return global->GetKernelRegistry().UnregisterModuleKernels( this );
}

/*-------------------------------------------------------------------------*/

bool
CCpuTestKernels::IsRegistered( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_registry;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
