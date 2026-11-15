/*
 *  computebridgeMATH: GUCEF compute bridge plugin of gucefMATH: multi-core and accelerator providers for the batch kernels
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H
#include "gucefCOMPUTE_CComputeCommandList.h"
#define GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H
#endif /* GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H ? */

#include "computebridgeMATH_CMathCostModel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTEBRIDGEMATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const Float64 CMathCostModel::DefaultForkJoinNs = 15000.0;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMathCostModel::SDeviceTransfer::SDeviceTransfer( void )
    : latencyNs( 50000.0 )
    , bytesPerNs( 12.0 )
    , measured( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMathCostModel::CMathCostModel( void )
    : m_forkJoinNs( DefaultForkJoinNs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMathCostModel::SetForkJoinCostNs( Float64 forkJoinNs )
{GUCEF_TRACE;

    m_forkJoinNs = forkJoinNs > 0.0 ? forkJoinNs : DefaultForkJoinNs;
}

/*-------------------------------------------------------------------------*/

Float64
CMathCostModel::GetForkJoinCostNs( void ) const
{GUCEF_TRACE;

    return m_forkJoinNs;
}

/*-------------------------------------------------------------------------*/

namespace {

UInt32
ToCount( Float64 items )
{GUCEF_TRACE;

    if ( !( items < 4294967295.0 ) )
        return CMathCostModel::NeverCount;
    return items < 1.0 ? 1 : static_cast< UInt32 >( ceil( items ) );
}

/** The factor by which the threads of the job system speed up the kernel: all of them, up to what the memory bandwidth allows */
Float64
MultiCoreSpeedup( const SMathBridgeKernelInfo& info, UInt32 workerCount )
{GUCEF_TRACE;

    const Float64 threads = static_cast< Float64 >( workerCount ) + 1.0;
    return threads < info.maxMultiCoreSpeedup ? threads : info.maxMultiCoreSpeedup;
}

}

/*-------------------------------------------------------------------------*/

UInt32
CMathCostModel::ComputeMultiCoreMinCount( const SMathBridgeKernelInfo& info, UInt32 workerCount ) const
{GUCEF_TRACE;

    const Float64 speedup = MultiCoreSpeedup( info, workerCount );
    if ( speedup <= 1.05 || info.simdNsPerItem <= 0.0 )
        return NeverCount;

    // fixed + n c / S < n c   gives   n > fixed / ( c ( 1 - 1 / S ) )
    const UInt32 byCost = ToCount( m_forkJoinNs / ( info.simdNsPerItem * ( 1.0 - 1.0 / speedup ) ) );

    // below two chunks there is nothing to run at the same time
    const UInt64 twoChunks = static_cast< UInt64 >( info.grainItems ) * 2;
    if ( NeverCount == byCost )
        return NeverCount;
    return byCost > twoChunks ? byCost : static_cast< UInt32 >( twoChunks );
}

/*-------------------------------------------------------------------------*/

UInt32
CMathCostModel::ComputeAcceleratorMinCount( const SMathBridgeKernelInfo& info, const SDeviceTransfer& transfer, UInt32 workerCount ) const
{GUCEF_TRACE;

    if ( info.simdNsPerItem <= 0.0 || info.acceleratorSpeedup <= 0.0 )
        return NeverCount;

    // best CPU tier per item: the multi-core throughput where that provider is registered, else one thread
    const Float64 speedup = MultiCoreSpeedup( info, workerCount );
    const Float64 cpuNsPerItem = info.simdNsPerItem / ( speedup > 1.05 ? speedup : 1.0 );
    const Float64 busNsPerItem = transfer.bytesPerNs > 0.0 ? static_cast< Float64 >( info.bytesPerItem ) / transfer.bytesPerNs : 0.0;
    const Float64 deviceNsPerItem = info.simdNsPerItem / info.acceleratorSpeedup;

    // latency + n ( bus + device ) < n cpu   gives   n > latency / ( cpu - bus - device )
    const Float64 gainPerItem = cpuNsPerItem - busNsPerItem - deviceNsPerItem;
    if ( gainPerItem <= 0.0 )
        return NeverCount;
    return ToCount( transfer.latencyNs / gainPerItem );
}

/*-------------------------------------------------------------------------*/

bool
CMathCostModel::MeasureDeviceTransfer( COMPUTE::CComputeDevice& device, SDeviceTransfer& transfer )
{GUCEF_TRACE;

    transfer = SDeviceTransfer();
    if ( device.IsLost() )
        return false;

    const bool unified = 0 != ( device.GetCapabilities().GetFeatures() & ( COMPUTE::COMPUTEFEATURE_UNIFIED_MEMORY | COMPUTE::COMPUTEFEATURE_MAPPABLE_DEVICE_LOCAL_MEMORY ) );
    const UInt32 smallBytes = 256;
    const UInt32 largeBytes = 4 * 1024 * 1024;

    COMPUTE::TComputeBufferPtr buffer;
    COMPUTE::TComputeResult result = COMPUTE::COMPUTERESULT_SUCCESS;
    const COMPUTE::TComputeMemoryLocation location = unified ? COMPUTE::MEMORYLOCATION_UNIFIED : COMPUTE::MEMORYLOCATION_DEVICE;
    if ( !device.CreateBuffer( COMPUTE::CComputeBufferDesc( largeBytes, COMPUTE::BUFFERUSAGE_STORAGE | COMPUTE::BUFFERUSAGE_COPY_SRC | COMPUTE::BUFFERUSAGE_COPY_DST, location ), buffer, &result ) )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: unable to make a buffer to measure the transfer cost of a device, result " << (UInt32) result );
        return false;
    }
    COMPUTE::TComputeQueuePtr queue = device.GetDefaultQueue();
    if ( queue.IsNULL() )
        return false;

    GUCEF::vector< UInt8 > host( largeBytes, 0 );

    // the time of a round trip of 'bytes' bytes, best of a few runs: the least disturbed one
    const UInt32 runs = 3;
    Float64 smallNs = 0.0;
    Float64 largeNs = 0.0;
    for ( UInt32 pass=0; pass<2; ++pass )
    {
        const UInt32 bytes = 0 == pass ? smallBytes : largeBytes;
        COMPUTE::CComputeCommandList list;
        if ( COMPUTE::COMPUTERESULT_SUCCESS != list.WriteFromHost( buffer, 0, &host[ 0 ], bytes ) ||
             COMPUTE::COMPUTERESULT_SUCCESS != list.ReadToHost( buffer, 0, &host[ 0 ], bytes ) )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: unable to record the copies that measure the transfer cost of a device" );
            return false;
        }

        // the timer has a resolution of about a millisecond: each measurement spans several submissions
        const UInt32 repeats = 0 == pass ? 64 : 4;
        Float64 best = 0.0;
        for ( UInt32 run=0; run<runs; ++run )
        {
            const UInt64 start = MT::PrecisionTickCount();
            for ( UInt32 r=0; r<repeats; ++r )
            {
                result = queue->Execute( list );
                if ( COMPUTE::COMPUTERESULT_SUCCESS != result )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "computebridgeMATH: the copies that measure the transfer cost of a device failed with result " << (UInt32) result );
                    return false;
                }
            }
            const Float64 ns = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) * 1000000.0 / repeats;
            if ( 0 == run || ns < best )
                best = ns;
        }
        if ( 0 == pass )
            smallNs = best;
        else
            largeNs = best;
    }

    transfer.latencyNs = smallNs > 0.0 ? smallNs : transfer.latencyNs;
    if ( unified )
    {
        transfer.bytesPerNs = 0.0;
    }
    else
    {
        // both directions: 2 x bytes over the time beyond the fixed latency
        const Float64 transferNs = largeNs - smallNs;
        transfer.bytesPerNs = transferNs > 0.0 ? ( 2.0 * static_cast< Float64 >( largeBytes ) ) / transferNs : transfer.bytesPerNs;
    }
    transfer.measured = true;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
