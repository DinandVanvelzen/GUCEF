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

#ifndef GUCEF_MATH_CMATHBATCHDISPATCHER_H
#define GUCEF_MATH_CMATHBATCHDISPATCHER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MATH_MATHBATCHTYPES_H
#include "gucefMATH_MathBatchTypes.h"
#define GUCEF_MATH_MATHBATCHTYPES_H
#endif /* GUCEF_MATH_MATHBATCHTYPES_H ? */

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
 *  The batch dispatch table (section 5.6 of the design): per kernel an immutable list of providers, sorted by
 *  descending tier, published with a release store and read with one acquire load. Selection (SelectProvider, Execute)
 *  takes no lock and allocates nothing. Registration and unregistration are serialised by a mutex, copy the list of the
 *  kernel, change the copy and publish it; replaced lists are kept until the dispatcher is destroyed, so a provider
 *  removed during a call is never freed under that call. Calls made through Execute are counted per kernel (two
 *  counters per kernel that alternate with an epoch, atomics only); UnregisterProviders returns only after every call
 *  that could still reach a removed provider has left it, so the owner may unload the provider's code right after.
 *
 *  Selection for one call: the effective cap is the lowest of the build cap (GUCEF_MATH_BATCH_MAX_TIER), the application
 *  cap (SetMaxTier) and the kernel cap (SetMaxTier for the kernel); MATHBATCH_REALTIME lowers it to SIMD,
 *  MATHBATCH_FORCE_SCALAR to SCALAR; MATHBATCH_NO_MULTICORE and MATHBATCH_NO_ACCELERATOR remove those tiers. Determinism
 *  is required with MATHBATCH_DETERMINISTIC and always for kernels registered as always deterministic (fixed point).
 *  The first provider in the list with tier at most the cap, minCount <= count <= maxCount, MATHPROVIDER_BITEXACT when
 *  determinism is required and its alignment satisfied by every pointer and byte stride of the call is called; a failing
 *  provider makes Execute continue with the next eligible one. The SCALAR provider of every MATH kernel terminates the
 *  chain. Execute rejects calls whose byte strides are smaller than an element (MathBatchArgumentsValid).
 *
 *  One dispatcher is owned by CMathGlobal (GetBatchDispatcher) and serves the batch functions; further instances are
 *  independent (tests, tools). Not copyable.
 */
class GUCEF_MATH_PUBLIC_CPP CMathBatchDispatcher
{
    public:

    /** The most providers one kernel can hold */
    static const UInt32 MaxProvidersPerKernel = 8;

    /** The longest kernel id of an external kernel, without the terminating zero */
    static const UInt32 MaxKernelIdLength = 63;

    /** Position in the provider list of one call, for Execute and SelectNextProvider; start with Reset() */
    struct SSelection
    {
        const void* list;
        UInt32 next;

        SSelection( void ) : list( GUCEF_NULL ), next( 0 ) {}
    };

    /** The MATH kernels are known and have no providers yet; reads the CPU features once (CORE::GetCpuIsaFeatures) */
    CMathBatchDispatcher( void );

    /** Frees every provider list, the replaced ones too; no call may be running */
    ~CMathBatchDispatcher();

    /**
     *  Registers the providers compiled into gucefMATH with this dispatcher as owner: the SCALAR provider of every MATH
     *  kernel and, per kernel, the best SIMD provider whose CPU features are present and within 'allowedCpuFeatures'
     *  (SSE2, AVX2, F16C on x86, NEON on AArch64, WASM128 on WebAssembly with SIMD). Returns the number registered. Not realtime-safe.
     */
    UInt32 RegisterBuiltInProviders( UInt64 allowedCpuFeatures = ~(UInt64) 0 );

    /**
     *  Adds a provider to a known kernel. False for an unknown kernel, a NULL function or owner, a tier above
     *  ACCELERATOR, minCount above maxCount, an alignment that is not a power of two with
     *  MATHPROVIDER_REQUIRES_ALIGNED_INPUT, CPU features this CPU lacks, or a full list. Not realtime-safe.
     */
    bool RegisterProvider( TMathBatchKernel kernel, const SMathBatchProvider& provider, const void* owner );

    /**
     *  Removes every provider registered with 'owner' and returns how many; external kernels registered by 'owner' that
     *  have no providers left are unregistered too. A plugin calls it before its code is unloaded: it returns only after
     *  every Execute call that selected a removed provider has left it. Not realtime-safe; it waits, so it must not be
     *  called from inside a provider, and a provider must not register or unregister providers.
     */
    UInt32 UnregisterProviders( const void* owner );

    /** The application cap for every kernel; also realtime-safe */
    void SetMaxTier( TMathBatchTier tier );

    /** The cap of one kernel; also realtime-safe */
    void SetMaxTier( TMathBatchKernel kernel, TMathBatchTier tier );

    /** The lower of the build cap and the application cap */
    TMathBatchTier GetMaxTier( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The lowest of the build cap, the application cap and the cap of the kernel */
    TMathBatchTier GetMaxTier( TMathBatchKernel kernel ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The provider a call would use first, NULL when none is eligible (alignment checked for 'in' and 'out' only).
     *  Realtime-safe: no lock, no allocation. For diagnostics: only Execute keeps a provider from being unregistered
     *  while it is called.
     */
    const SMathBatchProvider* SelectProvider( TMathBatchKernel kernel, UInt32 count, UInt32 flags, const void* in, const void* out ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The next eligible provider of one call, after those already returned for 'selection' (a default constructed
     *  SSelection starts at the first). The list read by the first call is used for the whole selection.
     */
    const SMathBatchProvider* SelectNextProvider( TMathBatchKernel kernel, SSelection& selection, UInt32 count, UInt32 flags, const void* in, const void* out ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Runs a call: the eligible providers in order until one returns MATHSTATUS_OK. TArgs is the argument struct of the
     *  kernel; the alignment check uses 'in', 'out' and every pointer and stride of the arguments
     *  (MathBatchAlignmentBits). Returns MATHSTATUS_OK, the status of the last provider that failed, or
     *  MATHSTATUS_INVALID_OPERAND when no provider is eligible or a byte stride is smaller than an element
     *  (MathBatchArgumentsValid; nothing is written then). Lock-free; realtime-safe when the providers that can be
     *  reached are (MATHBATCH_REALTIME). The call is counted, see UnregisterProviders.
     */
    template < typename TArgs >
    TMathStatus Execute( TMathBatchKernel kernel, const TArgs& args, UInt32 count, UInt32 flags, const void* in, const void* out ) const;

    /** The number of providers of a kernel */
    UInt32 GetProviderCount( TMathBatchKernel kernel ) const;

    /** A copy of provider 'index' of a kernel in selection order; false when there is none */
    bool GetProvider( TMathBatchKernel kernel, UInt32 index, SMathBatchProvider& out ) const;

    /**
     *  Makes a kernel id of the external range known (gucefGEOMETRY uses 256 to 319). 'kernelId' (at most
     *  MaxKernelIdLength characters) is copied and must be unique; registering the same id again for the same kernel
     *  succeeds. Not realtime-safe.
     */
    bool RegisterExternalKernel( TMathBatchKernel kernel, const char* kernelId, bool alwaysDeterministic, const void* owner );

    /** "MATH.TransformPoints3.F32" and the ids of registered external kernels; NULL for unknown kernels */
    const char* GetKernelId( TMathBatchKernel kernel ) const;

    /** Whether every call of the kernel requires determinism (fixed-point kernels) */
    bool IsKernelAlwaysDeterministic( TMathBatchKernel kernel ) const;

    /** The CPU features read at construction, the ones registration checks against */
    UInt64 GetCpuIsaFeatures( void ) const;

    private:

    /** Counts one Execute call of a kernel for the time of the call (paired EnterCall / leave) */
    class CCallScope
    {
        public:

        CCallScope( const CMathBatchDispatcher& dispatcher, TMathBatchKernel kernel ) GUCEF_MATH_REALTIME_CONTEXT
            : m_counter( dispatcher.EnterCall( kernel ) )
        {
        }

        ~CCallScope() GUCEF_MATH_REALTIME_CONTEXT
        {
            if ( GUCEF_NULL != m_counter )
                m_counter->FetchSub( 1, MT::MEMORDER_RELEASE );
        }

        private:

        MT::CTAtomic< UInt32 >* m_counter;

        CCallScope( const CCallScope& src );                /**< not implemented */
        CCallScope& operator=( const CCallScope& src );     /**< not implemented */
    };
    friend class CCallScope;

    struct SProviderList;

    typedef GUCEF::vector< SProviderList* > TProviderListVector;

    static bool IsValidKernel( TMathBatchKernel kernel );

    const SProviderList* LoadList( TMathBatchKernel kernel ) const GUCEF_MATH_REALTIME_CONTEXT;

    const SMathBatchProvider* SelectNext( TMathBatchKernel kernel, SSelection& selection, UInt32 count, UInt32 flags, size_t addressBits ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Counts a call of the kernel in the counter of the current epoch; the returned counter is decremented when it ends */
    MT::CTAtomic< UInt32 >* EnterCall( TMathBatchKernel kernel ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** With m_lock held, after a list of the kernel was replaced: starts a new epoch and waits for the calls of the old one */
    void WaitForCallsToLeave( UInt32 kernel );

    void Publish( TMathBatchKernel kernel, SProviderList* list );

    CMathBatchDispatcher( const CMathBatchDispatcher& src );               /**< not implemented */
    CMathBatchDispatcher& operator=( const CMathBatchDispatcher& src );    /**< not implemented */

    private:

    MT::CTAtomic< const void* > m_lists[ MATHBATCHKERNEL_SLOT_COUNT ];     /**< const SProviderList*, published */
    MT::CTAtomic< UInt32 > m_kernelMaxTier[ MATHBATCHKERNEL_SLOT_COUNT ];
    MT::CTAtomic< UInt32 > m_maxTier;
    MT::CTAtomic< UInt32 > m_epochs[ MATHBATCHKERNEL_SLOT_COUNT ];             /**< per kernel, advanced by UnregisterProviders */
    mutable MT::CTAtomic< UInt32 > m_activeCalls[ MATHBATCHKERNEL_SLOT_COUNT ][ 2 ];  /**< per kernel and epoch parity: Execute calls inside */
    UInt64 m_cpuFeatures;
    MT::CMutex m_lock;                                                      /**< serialises registration */
    TProviderListVector m_allLists;                                         /**< every list ever published, freed at destruction */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename TArgs >
TMathStatus
CMathBatchDispatcher::Execute( TMathBatchKernel kernel, const TArgs& args, UInt32 count, UInt32 flags, const void* in, const void* out ) const
{
    typedef TMathStatus ( *TFunction )( void* context, const TArgs& args );

    if ( !MathBatchArgumentsValid( kernel, args ) )
        return MATHSTATUS_INVALID_OPERAND;
    const size_t addressBits = (size_t) in | (size_t) out | MathBatchAlignmentBits( args );

    const CCallScope scope( *this, kernel );
    TMathStatus status = MATHSTATUS_INVALID_OPERAND;
    SSelection selection;
    const SMathBatchProvider* provider = SelectNext( kernel, selection, count, flags, addressBits );
    while ( GUCEF_NULL != provider )
    {
        status = reinterpret_cast< TFunction >( provider->function )( provider->context, args );
        if ( MATHSTATUS_OK == status )
            return status;
        provider = SelectNext( kernel, selection, count, flags, addressBits );
    }
    return status;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CMATHBATCHDISPATCHER_H ? */
