/*
 *  gucefCOMPUTE_TestApp: GUCEF test application for the Compute module
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

#ifndef GUCEF_COMPUTE_TESTAPP_TESTMATHBRIDGEHARNESS_H
#define GUCEF_COMPUTE_TESTAPP_TESTMATHBRIDGEHARNESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CMATHBATCHDISPATCHER_H
#include "gucefMATH_CMathBatchDispatcher.h"
#define GUCEF_MATH_CMATHBATCHDISPATCHER_H
#endif /* GUCEF_MATH_CMATHBATCHDISPATCHER_H ? */

#ifndef GUCEF_COMPUTE_H
#include "gucefCOMPUTE.h"
#define GUCEF_COMPUTE_H
#endif /* GUCEF_COMPUTE_H ? */

// the plugin is loaded by name and not linked: only its plain data header (the diagnostics the providers carry) is used
#include "../../../plugins/COMPUTE/computebridgeMATH/include/computebridgeMATH_SMathBridgeDiagnostics.h"
#include "../../../plugins/COMPUTE/computebridgeMATH/include/computebridgeMATH_SMathBridgeKernelInfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The harness of the MATH bridge suites (TestMathBridge.cpp) and benchmarks (TestMathBridgeBench.cpp): loading the
 *  bridge plugin with settings, the table of the bridged kernels with a function that runs one batch call of each kind
 *  of kernel on given arrays, and the data of the calls.
 */
namespace GUCEF {
namespace COMPUTE {
namespace TEST {

/** The arrays of one batch call; the kernel case uses the ones its kind of kernel has */
struct SMathCall
{
    const void* in0;
    UInt32 in0Stride;
    const void* in1;
    UInt32 in1Stride;
    void* out;
    UInt32 outStride;
    UInt32 count;
    const MATH::TFloat32Matrix4D* matrix;

    SMathCall( void );
};

enum EMathCaseShape
{
    MATHCASESHAPE_MATRIX_MAP = 0 ,      /**< matrix, one input array with a stride, one output array with a stride */
    MATHCASESHAPE_UNARY          ,      /**< one input array, one output array, both with strides */
    MATHCASESHAPE_BINARY         ,      /**< two input arrays, one output array, all with strides */
    MATHCASESHAPE_CONVERT        ,      /**< Float32 and Float16 arrays without strides */
    MATHCASESHAPE_GENERATE              /**< no input array: the output array is generated (noise fields) */
};

/** What a device (the host batch) is given for one call: the bindings, the constants block and the storage some of them point into */
struct SMathDeviceCall
{
    CComputeHostBatch::SHostBinding bindings[ 3 ];
    UInt32 bindingCount;
    UInt64 constants[ GUCEF_COMPUTE_MAX_CONSTANTS_SIZE / 8 ];
    UInt32 constantsSize;
    Float64 matrixFloat64[ 16 ];
    Int32 matrixFixed[ 16 ];

    SMathDeviceCall( void );
};

/** One bridged kernel as the tests call it */
struct SMathKernelCase
{
    MATH::TMathBatchKernel kernel;
    UInt32 shape;
    UInt32 in0Bytes;                    /**< bytes of one element of the first input */
    UInt32 in1Bytes;                    /**< of the second input, 0 for none */
    UInt32 outBytes;                    /**< of the output */
    bool strided;                       /**< the arrays have strides */
    bool hasMatrix;

    /** Runs one batch call through the dispatcher with the given call flags; returns its status */
    MATH::TMathStatus ( *run )( const MATH::CMathBatchDispatcher& dispatcher, UInt32 flags, const SMathCall& call );

    /** Fills 'count' elements of the input 'inputIndex' with test data (finite values, some special ones) */
    void ( *fill )( UInt32 inputIndex, void* data, UInt32 count, UInt32 strideBytes, UInt32 seed );

    /** The bindings and constants of a packed call for a device, by the rules of the kernel's declaration */
    void ( *makeDeviceCall )( const SMathCall& call, SMathDeviceCall& device );
};

/** The case of a kernel; null when the tests have none */
const SMathKernelCase* FindMathKernelCase( MATH::TMathBatchKernel kernel );

/** The kernels that have a MULTICORE provider right now (the plugin is loaded), in kernel order */
UInt32 GetMathMultiCoreKernels( MATH::TMathBatchKernel* kernels, UInt32 maxKernels );

/** The provider of a tier registered for a kernel; false when there is none */
bool FindMathProvider( MATH::TMathBatchKernel kernel, MATH::TMathBatchTier tier, MATH::SMathBatchProvider& provider );

/** The context of such a provider, with the diagnostics of the plugin; null when there is none */
const COMPUTEBRIDGEMATH::SMathBridgeProviderContext* FindMathProviderContext( MATH::TMathBatchKernel kernel, MATH::TMathBatchTier tier );

/**
 *  What the next load of the bridge plugin applies to every kernel the MATH dispatcher knows: a forced tier
 *  (PROVIDERTIER_NONE for none), and the minimum counts per tier (0: the cost model). Replaces the overrides of an
 *  earlier call; ClearMathBridgeSettings() removes them
 */
void SetMathBridgeSettings( TComputeProviderTier forcedTier, UInt32 minCountMultiCore = 0, UInt32 disabledTiers = 0 );
void ClearMathBridgeSettings( void );

/** Loads the plugin by name through the plugin control, false when it is not there or its load fails */
bool LoadMathBridgePlugin( void );

/** Unloads it again; true when it was loaded */
bool UnloadMathBridgePlugin( void );

/** The matrix the tests use: general (index 0) and singular (index 1) */
MATH::TFloat32Matrix4D MakeTestMatrix( UInt32 index );

/** A xorshift generator: deterministic test data */
class CTestRandom
{
    public:
    explicit CTestRandom( UInt32 seed ) : m_state( 0 == seed ? 0x9E3779B9u : seed ) {}
    UInt32 Next( void ) { m_state ^= m_state << 13; m_state ^= m_state >> 17; m_state ^= m_state << 5; return m_state; }
    Float32 NextFloat( Float32 scale ) { return ( static_cast< Float32 >( Next() >> 8 ) / 8388608.0f - 1.0f ) * scale; }
    private:
    UInt32 m_state;
};

/** The byte buffer of a test array: its start is the allocation plus 'misalignBytes' */
class CTestBuffer
{
    public:
    CTestBuffer( void ) : m_misalign( 0 ) {}
    void Resize( UInt64 bytes, UInt32 misalignBytes, UInt8 fill );
    UInt8* Data( void ) { return m_bytes.empty() ? GUCEF_NULL : &m_bytes[ 0 ] + m_misalign; }
    const UInt8* Data( void ) const { return m_bytes.empty() ? GUCEF_NULL : &m_bytes[ 0 ] + m_misalign; }
    UInt64 Size( void ) const { return m_bytes.size() - m_misalign; }
    private:
    GUCEF::vector< UInt8 > m_bytes;
    UInt32 m_misalign;
};

}  /* namespace TEST */
}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_TESTAPP_TESTMATHBRIDGEHARNESS_H ? */
