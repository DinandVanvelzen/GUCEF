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

#ifndef GUCEF_COMPUTEBRIDGEMATH_MATHKERNELTRAITS_H
#define GUCEF_COMPUTEBRIDGEMATH_MATHKERNELTRAITS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MATH_CMATHBATCHDISPATCHER_H
#include "gucefMATH_CMathBatchDispatcher.h"
#define GUCEF_MATH_CMATHBATCHDISPATCHER_H
#endif /* GUCEF_MATH_CMATHBATCHDISPATCHER_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H
#include "gucefCOMPUTE_CComputeKernelSignature.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNELSIGNATURE_H ? */

#ifndef GUCEF_COMPUTE_CCPUDISPATCHCONTEXT_H
#include "gucefCOMPUTE_CCpuDispatchContext.h"
#define GUCEF_COMPUTE_CCPUDISPATCHCONTEXT_H
#endif /* GUCEF_COMPUTE_CCPUDISPATCHCONTEXT_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEHOSTBATCH_H
#include "gucefCOMPUTE_CComputeHostBatch.h"
#define GUCEF_COMPUTE_CCOMPUTEHOSTBATCH_H
#endif /* GUCEF_COMPUTE_CCOMPUTEHOSTBATCH_H ? */

#ifndef GUCEF_COMPUTEBRIDGEMATH_ETYPES_H
#include "computebridgeMATH_ETypes.h"
#define GUCEF_COMPUTEBRIDGEMATH_ETYPES_H
#endif /* GUCEF_COMPUTEBRIDGEMATH_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTEBRIDGEMATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The traits of the bridged kernels. One struct per kernel, derived from the shape of its arguments, tells the generic
 *  provider code (CTMathMulticoreProvider, CTMathAcceleratorProvider) and the CPU kernel (CTMathBatchCpuKernel)
 *  everything that differs between kernels. The contract of a kernel struct K:
 *
 *      typedef ... TArgs;                          the argument struct of the MATH kernel (gucefMATH_MathBatchTypes.h)
 *      static MATH::TMathBatchKernel Kernel()      the kernel id; the string id comes from the dispatcher
 *      static UInt32 GetCount( args )              the item count of a call
 *      static const void* GetInput( args )         the pointers the dispatcher checks alignment against
 *      static void* GetOutput( args )
 *      static bool CanSlice( args )                false for a call that cannot be cut into independent chunks (in place conversions)
 *      static void Slice( args, first, count, out) the arguments of items [ first, first + count )
 *
 *  and for the device form (the CPU kernel and the ACCELERATOR providers), where arrays are packed (stride equals
 *  element size) in storage buffers and parameters travel in the constants block:
 *
 *      static void Describe( signature )           bindings and constants size
 *      static UInt32 GetConstantsSize()            including the 8 byte header
 *      static void PackConstants( args, block )    writes after the header
 *      static UInt32 MakeBindings( args, bindings) the host bindings of a call, 0 when the call is not packed
 *      static UInt64 GetItemCapacity( context )    items the bound buffers hold
 *      static void RunDevice( context, first, count, dispatcher )   a CPU kernel run over items [ first, first + count )
 *
 *  The device form calls the MATH dispatcher with MATHBATCH_NO_MULTICORE and MATHBATCH_NO_ACCELERATOR, so a CPU kernel
 *  is the best single thread provider (SIMD where there is one) and nothing recurses.
 */

/** Bytes of the header of every constants block (base item and item count) */
enum { MATHBRIDGE_CONSTANTS_HEADER = GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE };

/** What the CPU kernel and the nested calls pass to the dispatcher: below the MULTICORE tier, bit-exact providers only */
enum { MATHBRIDGE_NESTED_FLAGS = MATH::MATHBATCH_NO_MULTICORE | MATH::MATHBATCH_NO_ACCELERATOR | MATH::MATHBATCH_DETERMINISTIC };

/*-------------------------------------------------------------------------*/

inline const void*
MathBridgeAdvance( const void* base, UInt32 strideBytes, UInt32 items )
{
    return static_cast< const UInt8* >( base ) + static_cast< size_t >( static_cast< UInt64 >( items ) * strideBytes );
}

inline void*
MathBridgeAdvance( void* base, UInt32 strideBytes, UInt32 items )
{
    return static_cast< UInt8* >( base ) + static_cast< size_t >( static_cast< UInt64 >( items ) * strideBytes );
}

/** The items that fit the range bound to 'slot' when it holds elements of 'elementBytes' */
inline UInt64
MathBridgeBindingCapacity( const COMPUTE::CCpuDispatchContext& context, UInt32 slot, UInt32 elementBytes )
{
    return context.GetBufferSizeInBytes( slot ) / elementBytes;
}

inline UInt64
MathBridgeMinimum( UInt64 a, UInt64 b )
{
    return a < b ? a : b;
}

/*-------------------------------------------------------------------------*/

/**
 *  A matrix applied to an array: TransformPoints3 / Vectors3 / Normals3 / Vectors4. One input array in slot 0, one
 *  output array in slot 1, the 16 column-major Float32 of the matrix in the constants.
 */
template < typename TArgsType, MATH::TMathBatchKernel kernelId, UInt32 inputBytes, UInt32 outputBytes >
struct STMathMatrixMapKernel
{
    typedef TArgsType TArgs;

    static MATH::TMathBatchKernel Kernel( void ) { return kernelId; }
    static UInt32 GetCount( const TArgs& args ) { return args.count; }
    static const void* GetInput( const TArgs& args ) { return args.in; }
    static void* GetOutput( const TArgs& args ) { return args.out; }
    static bool CanSlice( const TArgs& ) { return true; }

    static void Slice( const TArgs& args, UInt32 first, UInt32 count, TArgs& sliced )
    {
        sliced = args;
        sliced.in = MathBridgeAdvance( args.in, args.inStrideBytes, first );
        sliced.out = MathBridgeAdvance( args.out, args.outStrideBytes, first );
        sliced.count = count;
    }

    static void Describe( COMPUTE::CComputeKernelSignature& signature )
    {
        signature.AddBinding( 0, COMPUTE::COMPUTEACCESS_READ, COMPUTE::ELEMENTTYPE_FLOAT32 );
        signature.AddBinding( 1, COMPUTE::COMPUTEACCESS_WRITE, COMPUTE::ELEMENTTYPE_FLOAT32 );
        signature.SetConstantsSizeInBytes( GetConstantsSize() );
    }

    static UInt32 GetConstantsSize( void ) { return MATHBRIDGE_CONSTANTS_HEADER + 16 * sizeof( Float32 ); }

    static void PackConstants( const TArgs& args, void* block )
    {
        memcpy( static_cast< UInt8* >( block ) + MATHBRIDGE_CONSTANTS_HEADER, args.matrix->GetData(), 16 * sizeof( Float32 ) );
    }

    static UInt32 MakeBindings( const TArgs& args, COMPUTE::CComputeHostBatch::SHostBinding* bindings )
    {
        if ( args.inStrideBytes != inputBytes || args.outStrideBytes != outputBytes )
            return 0;
        bindings[ 0 ] = COMPUTE::CComputeHostBatch::SHostBinding::Input( 0, args.in, static_cast< UInt64 >( args.count ) * inputBytes );
        bindings[ 1 ] = COMPUTE::CComputeHostBatch::SHostBinding::Output( 1, args.out, static_cast< UInt64 >( args.count ) * outputBytes );
        return 2;
    }

    static UInt64 GetItemCapacity( const COMPUTE::CCpuDispatchContext& context )
    {
        return MathBridgeMinimum( MathBridgeBindingCapacity( context, 0, inputBytes ), MathBridgeBindingCapacity( context, 1, outputBytes ) );
    }

    static void RunDevice( const COMPUTE::CCpuDispatchContext& context, UInt32 first, UInt32 count, const MATH::CMathBatchDispatcher& dispatcher )
    {
        const UInt8* constants = static_cast< const UInt8* >( context.GetConstants() ) + MATHBRIDGE_CONSTANTS_HEADER;
        Float32 values[ 16 ];
        memcpy( values, constants, sizeof( values ) );
        const MATH::TFloat32Matrix4D matrix = MATH::TFloat32Matrix4D::FromColumnMajor( values );

        TArgs args;
        args.matrix = &matrix;
        args.in = MathBridgeAdvance( static_cast< const void* >( context.GetBufferData( 0 ) ), inputBytes, first );
        args.inStrideBytes = inputBytes;
        args.out = MathBridgeAdvance( context.GetBufferData( 1 ), outputBytes, first );
        args.outStrideBytes = outputBytes;
        args.count = count;
        dispatcher.Execute( kernelId, args, count, MATHBRIDGE_NESTED_FLAGS, args.in, args.out );
    }
};

/*-------------------------------------------------------------------------*/

/** One array mapped to another without parameters: QuaternionsToMatrices, NormalizeVectors3. Input slot 0, output slot 1 */
template < typename TArgsType, MATH::TMathBatchKernel kernelId, UInt32 inputBytes, UInt32 outputBytes >
struct STMathUnaryMapKernel
{
    typedef TArgsType TArgs;

    static MATH::TMathBatchKernel Kernel( void ) { return kernelId; }
    static UInt32 GetCount( const TArgs& args ) { return args.count; }
    static const void* GetInput( const TArgs& args ) { return args.in; }
    static void* GetOutput( const TArgs& args ) { return args.out; }
    static bool CanSlice( const TArgs& ) { return true; }

    static void Slice( const TArgs& args, UInt32 first, UInt32 count, TArgs& sliced )
    {
        sliced = args;
        sliced.in = MathBridgeAdvance( args.in, args.inStrideBytes, first );
        sliced.out = MathBridgeAdvance( args.out, args.outStrideBytes, first );
        sliced.count = count;
    }

    static void Describe( COMPUTE::CComputeKernelSignature& signature )
    {
        signature.AddBinding( 0, COMPUTE::COMPUTEACCESS_READ, COMPUTE::ELEMENTTYPE_FLOAT32 );
        signature.AddBinding( 1, COMPUTE::COMPUTEACCESS_WRITE, COMPUTE::ELEMENTTYPE_FLOAT32 );
    }

    static UInt32 GetConstantsSize( void ) { return MATHBRIDGE_CONSTANTS_HEADER; }
    static void PackConstants( const TArgs&, void* ) {}

    static UInt32 MakeBindings( const TArgs& args, COMPUTE::CComputeHostBatch::SHostBinding* bindings )
    {
        if ( args.inStrideBytes != inputBytes || args.outStrideBytes != outputBytes )
            return 0;
        bindings[ 0 ] = COMPUTE::CComputeHostBatch::SHostBinding::Input( 0, args.in, static_cast< UInt64 >( args.count ) * inputBytes );
        bindings[ 1 ] = COMPUTE::CComputeHostBatch::SHostBinding::Output( 1, args.out, static_cast< UInt64 >( args.count ) * outputBytes );
        return 2;
    }

    static UInt64 GetItemCapacity( const COMPUTE::CCpuDispatchContext& context )
    {
        return MathBridgeMinimum( MathBridgeBindingCapacity( context, 0, inputBytes ), MathBridgeBindingCapacity( context, 1, outputBytes ) );
    }

    static void RunDevice( const COMPUTE::CCpuDispatchContext& context, UInt32 first, UInt32 count, const MATH::CMathBatchDispatcher& dispatcher )
    {
        TArgs args;
        args.in = MathBridgeAdvance( static_cast< const void* >( context.GetBufferData( 0 ) ), inputBytes, first );
        args.inStrideBytes = inputBytes;
        args.out = MathBridgeAdvance( context.GetBufferData( 1 ), outputBytes, first );
        args.outStrideBytes = outputBytes;
        args.count = count;
        dispatcher.Execute( kernelId, args, count, MATHBRIDGE_NESTED_FLAGS, args.in, args.out );
    }
};

/*-------------------------------------------------------------------------*/

/** Two input arrays and one output array without parameters: MultiplyMatrices4, DotProducts3. Inputs in slots 0 and 1, output in slot 2 */
template < typename TArgsType, MATH::TMathBatchKernel kernelId, UInt32 firstBytes, UInt32 secondBytes, UInt32 outputBytes,
           COMPUTE::TComputeElementType elementType = COMPUTE::ELEMENTTYPE_FLOAT32, UInt32 features = 0 >
struct STMathBinaryMapKernel
{
    typedef TArgsType TArgs;

    static MATH::TMathBatchKernel Kernel( void ) { return kernelId; }
    static UInt32 GetCount( const TArgs& args ) { return args.count; }
    static const void* GetInput( const TArgs& args ) { return args.a; }
    static void* GetOutput( const TArgs& args ) { return args.out; }
    static bool CanSlice( const TArgs& ) { return true; }

    static void Slice( const TArgs& args, UInt32 first, UInt32 count, TArgs& sliced )
    {
        sliced = args;
        sliced.a = MathBridgeAdvance( args.a, args.aStrideBytes, first );
        sliced.b = MathBridgeAdvance( args.b, args.bStrideBytes, first );
        sliced.out = MathBridgeAdvance( args.out, args.outStrideBytes, first );
        sliced.count = count;
    }

    static void Describe( COMPUTE::CComputeKernelSignature& signature )
    {
        signature.AddBinding( 0, COMPUTE::COMPUTEACCESS_READ, elementType );
        signature.AddBinding( 1, COMPUTE::COMPUTEACCESS_READ, elementType );
        signature.AddBinding( 2, COMPUTE::COMPUTEACCESS_WRITE, elementType );
        signature.SetRequiredFeatures( features );
    }

    static UInt32 GetConstantsSize( void ) { return MATHBRIDGE_CONSTANTS_HEADER; }
    static void PackConstants( const TArgs&, void* ) {}

    static UInt32 MakeBindings( const TArgs& args, COMPUTE::CComputeHostBatch::SHostBinding* bindings )
    {
        if ( args.aStrideBytes != firstBytes || args.bStrideBytes != secondBytes || args.outStrideBytes != outputBytes )
            return 0;
        bindings[ 0 ] = COMPUTE::CComputeHostBatch::SHostBinding::Input( 0, args.a, static_cast< UInt64 >( args.count ) * firstBytes );
        bindings[ 1 ] = COMPUTE::CComputeHostBatch::SHostBinding::Input( 1, args.b, static_cast< UInt64 >( args.count ) * secondBytes );
        bindings[ 2 ] = COMPUTE::CComputeHostBatch::SHostBinding::Output( 2, args.out, static_cast< UInt64 >( args.count ) * outputBytes );
        return 3;
    }

    static UInt64 GetItemCapacity( const COMPUTE::CCpuDispatchContext& context )
    {
        return MathBridgeMinimum( MathBridgeMinimum( MathBridgeBindingCapacity( context, 0, firstBytes ), MathBridgeBindingCapacity( context, 1, secondBytes ) ),
                                  MathBridgeBindingCapacity( context, 2, outputBytes ) );
    }

    static void RunDevice( const COMPUTE::CCpuDispatchContext& context, UInt32 first, UInt32 count, const MATH::CMathBatchDispatcher& dispatcher )
    {
        TArgs args;
        args.a = MathBridgeAdvance( static_cast< const void* >( context.GetBufferData( 0 ) ), firstBytes, first );
        args.aStrideBytes = firstBytes;
        args.b = MathBridgeAdvance( static_cast< const void* >( context.GetBufferData( 1 ) ), secondBytes, first );
        args.bStrideBytes = secondBytes;
        args.out = MathBridgeAdvance( context.GetBufferData( 2 ), outputBytes, first );
        args.outStrideBytes = outputBytes;
        args.count = count;
        dispatcher.Execute( kernelId, args, count, MATHBRIDGE_NESTED_FLAGS, args.a, args.out );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  The conversions between Float32 and Float16 (contiguous arrays, no strides). The element types are those of the
 *  arguments. In place (out equal to in) the MATH providers walk the array in an order that depends on the element
 *  sizes, which chunks that run at the same time cannot keep: such a call is not sliced (CanSlice), the MULTICORE
 *  provider declines it and the dispatcher continues with the next tier.
 */
template < typename TArgsType, MATH::TMathBatchKernel kernelId, typename TInput, typename TOutput, COMPUTE::TComputeElementType inputElement, COMPUTE::TComputeElementType outputElement >
struct STMathConvertKernel
{
    typedef TArgsType TArgs;

    static MATH::TMathBatchKernel Kernel( void ) { return kernelId; }
    static UInt32 GetCount( const TArgs& args ) { return args.count; }
    static const void* GetInput( const TArgs& args ) { return args.in; }
    static void* GetOutput( const TArgs& args ) { return args.out; }
    static bool CanSlice( const TArgs& args ) { return static_cast< const void* >( args.in ) != static_cast< const void* >( args.out ); }

    static void Slice( const TArgs& args, UInt32 first, UInt32 count, TArgs& sliced )
    {
        sliced.in = args.in + first;
        sliced.out = args.out + first;
        sliced.count = count;
    }

    static void Describe( COMPUTE::CComputeKernelSignature& signature )
    {
        signature.AddBinding( 0, COMPUTE::COMPUTEACCESS_READ, inputElement );
        signature.AddBinding( 1, COMPUTE::COMPUTEACCESS_WRITE, outputElement );
    }

    static UInt32 GetConstantsSize( void ) { return MATHBRIDGE_CONSTANTS_HEADER; }
    static void PackConstants( const TArgs&, void* ) {}

    static UInt32 MakeBindings( const TArgs& args, COMPUTE::CComputeHostBatch::SHostBinding* bindings )
    {
        bindings[ 0 ] = COMPUTE::CComputeHostBatch::SHostBinding::Input( 0, args.in, static_cast< UInt64 >( args.count ) * sizeof( TInput ) );
        bindings[ 1 ] = COMPUTE::CComputeHostBatch::SHostBinding::Output( 1, args.out, static_cast< UInt64 >( args.count ) * sizeof( TOutput ) );
        return 2;
    }

    static UInt64 GetItemCapacity( const COMPUTE::CCpuDispatchContext& context )
    {
        return MathBridgeMinimum( MathBridgeBindingCapacity( context, 0, sizeof( TInput ) ), MathBridgeBindingCapacity( context, 1, sizeof( TOutput ) ) );
    }

    static void RunDevice( const COMPUTE::CCpuDispatchContext& context, UInt32 first, UInt32 count, const MATH::CMathBatchDispatcher& dispatcher )
    {
        TArgs args;
        args.in = static_cast< const TInput* >( static_cast< const void* >( context.GetBufferData( 0 ) ) ) + first;
        args.out = static_cast< TOutput* >( context.GetBufferData( 1 ) ) + first;
        args.count = count;
        dispatcher.Execute( kernelId, args, count, MATHBRIDGE_NESTED_FLAGS, args.in, args.out );
    }
};

/**
 *  A matrix applied to an array for the kinds whose matrix does not fit the constants block (Float64: 128 bytes) and for
 *  fixed point: as STMathMatrixMapKernel, but the matrix is a third binding, slot 2, read only. 'TMatrix' is the matrix
 *  type of the arguments, 'elementType' the element type of every binding, 'features' what the device has to offer.
 */
template < typename TArgsType, MATH::TMathBatchKernel kernelId, UInt32 inputBytes, UInt32 outputBytes, typename TMatrix, COMPUTE::TComputeElementType elementType, UInt32 features >
struct STMathMatrixBufferMapKernel
{
    typedef TArgsType TArgs;
    typedef typename TMatrix::TValue TValue;

    static MATH::TMathBatchKernel Kernel( void ) { return kernelId; }
    static UInt32 GetCount( const TArgs& args ) { return args.count; }
    static const void* GetInput( const TArgs& args ) { return args.in; }
    static void* GetOutput( const TArgs& args ) { return args.out; }
    static bool CanSlice( const TArgs& ) { return true; }

    static void Slice( const TArgs& args, UInt32 first, UInt32 count, TArgs& sliced )
    {
        sliced = args;
        sliced.in = MathBridgeAdvance( args.in, args.inStrideBytes, first );
        sliced.out = MathBridgeAdvance( args.out, args.outStrideBytes, first );
        sliced.count = count;
    }

    static void Describe( COMPUTE::CComputeKernelSignature& signature )
    {
        signature.AddBinding( 0, COMPUTE::COMPUTEACCESS_READ, elementType );
        signature.AddBinding( 1, COMPUTE::COMPUTEACCESS_WRITE, elementType );
        signature.AddBinding( 2, COMPUTE::COMPUTEACCESS_READ, elementType );
        signature.SetRequiredFeatures( features );
    }

    static UInt32 GetConstantsSize( void ) { return MATHBRIDGE_CONSTANTS_HEADER; }
    static void PackConstants( const TArgs&, void* ) {}

    static UInt32 MakeBindings( const TArgs& args, COMPUTE::CComputeHostBatch::SHostBinding* bindings )
    {
        if ( args.inStrideBytes != inputBytes || args.outStrideBytes != outputBytes )
            return 0;
        bindings[ 0 ] = COMPUTE::CComputeHostBatch::SHostBinding::Input( 0, args.in, static_cast< UInt64 >( args.count ) * inputBytes );
        bindings[ 1 ] = COMPUTE::CComputeHostBatch::SHostBinding::Output( 1, args.out, static_cast< UInt64 >( args.count ) * outputBytes );
        bindings[ 2 ] = COMPUTE::CComputeHostBatch::SHostBinding::Input( 2, args.matrix->GetData(), 16 * sizeof( TValue ) );
        return 3;
    }

    static UInt64 GetItemCapacity( const COMPUTE::CCpuDispatchContext& context )
    {
        if ( context.GetBufferSizeInBytes( 2 ) < 16 * sizeof( TValue ) )
            return 0;
        return MathBridgeMinimum( MathBridgeBindingCapacity( context, 0, inputBytes ), MathBridgeBindingCapacity( context, 1, outputBytes ) );
    }

    static void RunDevice( const COMPUTE::CCpuDispatchContext& context, UInt32 first, UInt32 count, const MATH::CMathBatchDispatcher& dispatcher )
    {
        TValue values[ 16 ];
        memcpy( values, context.GetBufferData( 2 ), sizeof( values ) );
        const TMatrix matrix = TMatrix::FromColumnMajor( values );

        TArgs args;
        args.matrix = &matrix;
        args.in = MathBridgeAdvance( static_cast< const void* >( context.GetBufferData( 0 ) ), inputBytes, first );
        args.inStrideBytes = inputBytes;
        args.out = MathBridgeAdvance( context.GetBufferData( 1 ), outputBytes, first );
        args.outStrideBytes = outputBytes;
        args.count = count;
        dispatcher.Execute( kernelId, args, count, MATHBRIDGE_NESTED_FLAGS, args.in, args.out );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  VbapGains2D: azimuths in, GetSpeakerCount() gains per source out. The panner is an object of the host: the device
 *  form (the CPU kernel, which runs in this process) carries its address in the constants after the header, followed
 *  by the target power; devices elsewhere have no artefact of this kernel. Azimuths in slot 0, gains in slot 1.
 */
struct SMathVbapGains2DF32Kernel
{
    typedef MATH::SMathVbapGains2DF32Args TArgs;

    /** The constants after the header */
    struct SConstants
    {
        const MATH::TFloat32Vbap2D* vbap;
        Float32 targetPower;
        UInt32 padding;
    };

    static MATH::TMathBatchKernel Kernel( void ) { return MATH::MATHBATCHKERNEL_VBAP_GAINS2D_F32; }
    static UInt32 GetCount( const TArgs& args ) { return args.count; }
    static const void* GetInput( const TArgs& args ) { return args.azimuths; }
    static void* GetOutput( const TArgs& args ) { return args.gains; }
    static bool CanSlice( const TArgs& ) { return true; }

    static void Slice( const TArgs& args, UInt32 first, UInt32 count, TArgs& sliced )
    {
        sliced = args;
        sliced.azimuths = MathBridgeAdvance( args.azimuths, args.azimuthStrideBytes, first );
        sliced.gains = MathBridgeAdvance( args.gains, args.gainsStrideBytes, first );
        sliced.count = count;
    }

    static void Describe( COMPUTE::CComputeKernelSignature& signature )
    {
        signature.AddBinding( 0, COMPUTE::COMPUTEACCESS_READ, COMPUTE::ELEMENTTYPE_FLOAT32 );
        signature.AddBinding( 1, COMPUTE::COMPUTEACCESS_WRITE, COMPUTE::ELEMENTTYPE_FLOAT32 );
        signature.SetConstantsSizeInBytes( GetConstantsSize() );
    }

    static UInt32 GetConstantsSize( void ) { return MATHBRIDGE_CONSTANTS_HEADER + sizeof( SConstants ); }

    static void PackConstants( const TArgs& args, void* block )
    {
        SConstants constants;
        constants.vbap = args.vbap;
        constants.targetPower = args.targetPower;
        constants.padding = 0;
        memcpy( static_cast< UInt8* >( block ) + MATHBRIDGE_CONSTANTS_HEADER, &constants, sizeof( constants ) );
    }

    static UInt32 MakeBindings( const TArgs& args, COMPUTE::CComputeHostBatch::SHostBinding* bindings )
    {
        const UInt32 gainBytes = GUCEF_NULL != args.vbap ? args.vbap->GetSpeakerCount() * sizeof( Float32 ) : 0;
        if ( 0 == gainBytes || args.azimuthStrideBytes != sizeof( Float32 ) || args.gainsStrideBytes != gainBytes )
            return 0;
        bindings[ 0 ] = COMPUTE::CComputeHostBatch::SHostBinding::Input( 0, args.azimuths, static_cast< UInt64 >( args.count ) * sizeof( Float32 ) );
        bindings[ 1 ] = COMPUTE::CComputeHostBatch::SHostBinding::Output( 1, args.gains, static_cast< UInt64 >( args.count ) * gainBytes );
        return 2;
    }

    static UInt64 GetItemCapacity( const COMPUTE::CCpuDispatchContext& context )
    {
        SConstants constants;
        memcpy( &constants, static_cast< const UInt8* >( context.GetConstants() ) + MATHBRIDGE_CONSTANTS_HEADER, sizeof( constants ) );
        const UInt32 gainBytes = GUCEF_NULL != constants.vbap ? constants.vbap->GetSpeakerCount() * sizeof( Float32 ) : 0;
        if ( 0 == gainBytes )
            return 0;
        return MathBridgeMinimum( MathBridgeBindingCapacity( context, 0, sizeof( Float32 ) ), MathBridgeBindingCapacity( context, 1, gainBytes ) );
    }

    static void RunDevice( const COMPUTE::CCpuDispatchContext& context, UInt32 first, UInt32 count, const MATH::CMathBatchDispatcher& dispatcher )
    {
        SConstants constants;
        memcpy( &constants, static_cast< const UInt8* >( context.GetConstants() ) + MATHBRIDGE_CONSTANTS_HEADER, sizeof( constants ) );
        const UInt32 gainBytes = constants.vbap->GetSpeakerCount() * sizeof( Float32 );

        TArgs args;
        args.vbap = constants.vbap;
        args.azimuths = MathBridgeAdvance( static_cast< const void* >( context.GetBufferData( 0 ) ), sizeof( Float32 ), first );
        args.azimuthStrideBytes = sizeof( Float32 );
        args.targetPower = constants.targetPower;
        args.gains = MathBridgeAdvance( context.GetBufferData( 1 ), gainBytes, first );
        args.gainsStrideBytes = gainBytes;
        args.count = count;
        dispatcher.Execute( Kernel(), args, count, MATHBRIDGE_NESTED_FLAGS, args.azimuths, args.gains );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  NoiseField2D and NoiseField3D: a grid of samples of a noise object of the host, 'count' values from item 'baseItem'
 *  of the grid into 'out'. Chunks adjust baseItem, out and count. The device form carries the address of the noise
 *  object and the grid parameters in the constants after the header (devices elsewhere have no artefact of this kernel,
 *  as with VbapGains2D); the output is slot 0. The same traits serve 2D and 3D: the 3D arguments have the extra z origin and step.
 */
template < typename TArgsType, MATH::TMathBatchKernel kernelId, bool is3D >
struct STMathNoiseFieldKernel
{
    typedef TArgsType TArgs;

    /** The constants after the header */
    struct SConstants
    {
        const void* noise;
        UInt32 noiseType;
        UInt32 width;
        UInt32 height;
        UInt32 baseItem;        /**< the item of the grid that item 0 of the dispatch is */
        Float32 origin[ 3 ];
        Float32 step[ 3 ];
        UInt32 padding;
    };

    static MATH::TMathBatchKernel Kernel( void ) { return kernelId; }
    static UInt32 GetCount( const TArgs& args ) { return args.count; }
    static const void* GetInput( const TArgs& args ) { return args.out; }
    static void* GetOutput( const TArgs& args ) { return args.out; }
    static bool CanSlice( const TArgs& ) { return true; }

    static void Slice( const TArgs& args, UInt32 first, UInt32 count, TArgs& sliced )
    {
        sliced = args;
        sliced.baseItem = args.baseItem + first;
        sliced.out = args.out + first;
        sliced.count = count;
    }

    static void Describe( COMPUTE::CComputeKernelSignature& signature )
    {
        signature.AddBinding( 0, COMPUTE::COMPUTEACCESS_WRITE, COMPUTE::ELEMENTTYPE_FLOAT32 );
        signature.SetConstantsSizeInBytes( GetConstantsSize() );
    }

    static UInt32 GetConstantsSize( void ) { return MATHBRIDGE_CONSTANTS_HEADER + sizeof( SConstants ); }

    static void PackConstants( const TArgs& args, void* block )
    {
        SConstants constants;
        memset( &constants, 0, sizeof( constants ) );
        constants.noise = args.noise;
        constants.noiseType = args.noiseType;
        constants.width = args.width;
        constants.height = args.height;
        constants.baseItem = args.baseItem;
        Pack( args, constants );
        memcpy( static_cast< UInt8* >( block ) + MATHBRIDGE_CONSTANTS_HEADER, &constants, sizeof( constants ) );
    }

    static UInt32 MakeBindings( const TArgs& args, COMPUTE::CComputeHostBatch::SHostBinding* bindings )
    {
        if ( GUCEF_NULL == args.noise )
            return 0;
        bindings[ 0 ] = COMPUTE::CComputeHostBatch::SHostBinding::Output( 0, args.out, static_cast< UInt64 >( args.count ) * sizeof( Float32 ) );
        return 1;
    }

    static UInt64 GetItemCapacity( const COMPUTE::CCpuDispatchContext& context )
    {
        return MathBridgeBindingCapacity( context, 0, sizeof( Float32 ) );
    }

    static void RunDevice( const COMPUTE::CCpuDispatchContext& context, UInt32 first, UInt32 count, const MATH::CMathBatchDispatcher& dispatcher )
    {
        SConstants constants;
        memcpy( &constants, static_cast< const UInt8* >( context.GetConstants() ) + MATHBRIDGE_CONSTANTS_HEADER, sizeof( constants ) );

        TArgs args;
        memset( &args, 0, sizeof( args ) );
        args.noise = constants.noise;
        args.noiseType = constants.noiseType;
        args.width = constants.width;
        args.height = constants.height;
        args.baseItem = constants.baseItem + first;
        args.out = static_cast< Float32* >( context.GetBufferData( 0 ) ) + first;
        args.count = count;
        Unpack( constants, args );
        dispatcher.Execute( kernelId, args, count, MATHBRIDGE_NESTED_FLAGS, args.out, args.out );
    }

    private:

    static void Pack( const MATH::SMathNoiseField2DF32Args& args, SConstants& constants )
    {
        constants.origin[ 0 ] = args.originX; constants.origin[ 1 ] = args.originY;
        constants.step[ 0 ] = args.stepX; constants.step[ 1 ] = args.stepY;
    }

    static void Pack( const MATH::SMathNoiseField3DF32Args& args, SConstants& constants )
    {
        constants.origin[ 0 ] = args.originX; constants.origin[ 1 ] = args.originY; constants.origin[ 2 ] = args.originZ;
        constants.step[ 0 ] = args.stepX; constants.step[ 1 ] = args.stepY; constants.step[ 2 ] = args.stepZ;
    }

    static void Unpack( const SConstants& constants, MATH::SMathNoiseField2DF32Args& args )
    {
        args.originX = constants.origin[ 0 ]; args.originY = constants.origin[ 1 ];
        args.stepX = constants.step[ 0 ]; args.stepY = constants.step[ 1 ];
    }

    static void Unpack( const SConstants& constants, MATH::SMathNoiseField3DF32Args& args )
    {
        args.originX = constants.origin[ 0 ]; args.originY = constants.origin[ 1 ]; args.originZ = constants.origin[ 2 ];
        args.stepX = constants.step[ 0 ]; args.stepY = constants.step[ 1 ]; args.stepZ = constants.step[ 2 ];
    }
};

/*-------------------------------------------------------------------------*/

typedef STMathMatrixMapKernel< MATH::SMathTransformPoints3F32Args, MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F32, 12, 12 > SMathTransformPoints3F32Kernel;
typedef STMathMatrixMapKernel< MATH::SMathTransformVectors3F32Args, MATH::MATHBATCHKERNEL_TRANSFORM_VECTORS3_F32, 12, 12 > SMathTransformVectors3F32Kernel;
typedef STMathMatrixMapKernel< MATH::SMathTransformNormals3F32Args, MATH::MATHBATCHKERNEL_TRANSFORM_NORMALS3_F32, 12, 12 > SMathTransformNormals3F32Kernel;
typedef STMathMatrixMapKernel< MATH::SMathTransformVectors4F32Args, MATH::MATHBATCHKERNEL_TRANSFORM_VECTORS4_F32, 16, 16 > SMathTransformVectors4F32Kernel;
typedef STMathBinaryMapKernel< MATH::SMathMultiplyMatrices4F32Args, MATH::MATHBATCHKERNEL_MULTIPLY_MATRICES4_F32, 64, 64, 64 > SMathMultiplyMatrices4F32Kernel;
typedef STMathUnaryMapKernel< MATH::SMathQuaternionsToMatricesF32Args, MATH::MATHBATCHKERNEL_QUATERNIONS_TO_MATRICES_F32, 16, 64 > SMathQuaternionsToMatricesF32Kernel;
typedef STMathBinaryMapKernel< MATH::SMathDotProducts3F32Args, MATH::MATHBATCHKERNEL_DOT_PRODUCTS3_F32, 12, 12, 4 > SMathDotProducts3F32Kernel;
typedef STMathUnaryMapKernel< MATH::SMathNormalizeVectors3F32Args, MATH::MATHBATCHKERNEL_NORMALIZE_VECTORS3_F32, 12, 12 > SMathNormalizeVectors3F32Kernel;
typedef STMathConvertKernel< MATH::SMathConvertF32ToF16Args, MATH::MATHBATCHKERNEL_CONVERT_F32_TO_F16, Float32, MATH::CFloat16, COMPUTE::ELEMENTTYPE_FLOAT32, COMPUTE::ELEMENTTYPE_FLOAT16 > SMathConvertF32ToF16Kernel;
typedef STMathConvertKernel< MATH::SMathConvertF16ToF32Args, MATH::MATHBATCHKERNEL_CONVERT_F16_TO_F32, MATH::CFloat16, Float32, COMPUTE::ELEMENTTYPE_FLOAT16, COMPUTE::ELEMENTTYPE_FLOAT32 > SMathConvertF16ToF32Kernel;

typedef STMathMatrixBufferMapKernel< MATH::SMathTransformPoints3F64Args, MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F64, 24, 24, MATH::TFloat64Matrix4D, COMPUTE::ELEMENTTYPE_FLOAT64, COMPUTE::COMPUTEFEATURE_FLOAT64 > SMathTransformPoints3F64Kernel;
typedef STMathMatrixBufferMapKernel< MATH::SMathTransformVectors3F64Args, MATH::MATHBATCHKERNEL_TRANSFORM_VECTORS3_F64, 24, 24, MATH::TFloat64Matrix4D, COMPUTE::ELEMENTTYPE_FLOAT64, COMPUTE::COMPUTEFEATURE_FLOAT64 > SMathTransformVectors3F64Kernel;
typedef STMathBinaryMapKernel< MATH::SMathMultiplyMatrices4F64Args, MATH::MATHBATCHKERNEL_MULTIPLY_MATRICES4_F64, 128, 128, 128, COMPUTE::ELEMENTTYPE_FLOAT64, COMPUTE::COMPUTEFEATURE_FLOAT64 > SMathMultiplyMatrices4F64Kernel;
typedef STMathMatrixBufferMapKernel< MATH::SMathTransformPoints3Fx32Args, MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_FX32, 12, 12, MATH::TFixed32Matrix4D, COMPUTE::ELEMENTTYPE_INT32, COMPUTE::COMPUTEFEATURE_INT64 > SMathTransformPoints3Fx32Kernel;
typedef STMathBinaryMapKernel< MATH::SMathMultiplyMatrices4Fx32Args, MATH::MATHBATCHKERNEL_MULTIPLY_MATRICES4_FX32, 64, 64, 64, COMPUTE::ELEMENTTYPE_INT32, COMPUTE::COMPUTEFEATURE_INT64 > SMathMultiplyMatrices4Fx32Kernel;
typedef STMathNoiseFieldKernel< MATH::SMathNoiseField2DF32Args, MATH::MATHBATCHKERNEL_NOISE_FIELD2D_F32, false > SMathNoiseField2DF32Kernel;
typedef STMathNoiseFieldKernel< MATH::SMathNoiseField3DF32Args, MATH::MATHBATCHKERNEL_NOISE_FIELD3D_F32, true > SMathNoiseField3DF32Kernel;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTEBRIDGEMATH_MATHKERNELTRAITS_H ? */
