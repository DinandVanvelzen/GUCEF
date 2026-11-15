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

#ifndef GUCEF_COMPUTE_CCOMPUTESETTINGS_H
#define GUCEF_COMPUTE_CCOMPUTESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_COMPUTE_MACROS_H
#include "gucefCOMPUTE_macros.h"
#define GUCEF_COMPUTE_MACROS_H
#endif /* GUCEF_COMPUTE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of the compute module: which backends are used and in what order, the preferred device, the number
 *  of CPU workers, overrides of the batch providers per kernel, the VFS root of kernel overrides, validation, and
 *  whether f64 kernels may run on GPUs. Applied with CComputeGlobal::ApplySettings().
 *
 *  Saved as the section "ComputeSettings" of a config tree (attributes enabledBackends, disabledBackends and
 *  backendOrder as comma separated lists, preferredDevice, maxCpuWorkers, kernelOverrideRoot, validation,
 *  allowFloat64OnGpu; one child "Kernel" per kernel override with the attributes id, forcedTier, disabledTiers
 *  (comma separated tier names) and minCountScalar, minCountSimd, minCountMulticore, minCountAccelerator).
 *  Values that a loaded tree does not have keep their value.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeSettings : public CORE::CIConfigurable
{
    public:

    static const CString ConfigSectionName;     /**< "ComputeSettings" */

    /**
     *  How a bridge plugin selects batch providers for one kernel, replacing the thresholds of its cost model:
     *  a forced tier (PROVIDERTIER_NONE for none), tiers that are not used (bit 1 << tier), and the smallest batch per
     *  tier (0: the cost model decides).
     */
    struct GUCEF_COMPUTE_PUBLIC_CPP SKernelProviderOverride
    {
        TComputeProviderTier forcedTier;
        UInt32 disabledTiers;
        UInt32 minCount[ PROVIDERTIER_COUNT ];

        SKernelProviderOverride( void );
        bool IsTierDisabled( TComputeProviderTier tier ) const;
        bool operator==( const SKernelProviderOverride& other ) const;
    };

    typedef GUCEF::map< CString, SKernelProviderOverride > TKernelProviderOverrideMap;

    CComputeSettings( void );

    CComputeSettings( const CComputeSettings& src );

    virtual ~CComputeSettings() GUCEF_VIRTUAL_OVERRIDE;

    CComputeSettings& operator=( const CComputeSettings& src );

    /** Only these backends are used (names, without regard for case); empty for all */
    const CString::StringSet& GetEnabledBackends( void ) const;
    void SetEnabledBackends( const CString::StringSet& backendNames );

    /** These backends are never used */
    const CString::StringSet& GetDisabledBackends( void ) const;
    void SetDisabledBackends( const CString::StringSet& backendNames );

    /** Backends in the order of preference; backends not listed come after, by their priority */
    const CString::StringVector& GetBackendOrder( void ) const;
    void SetBackendOrder( const CString::StringVector& backendNames );

    /** The identifier (CComputeDeviceInfo::GetIdentifier()) of the device that wins when it meets the requirements; empty for none */
    const CString& GetPreferredDevice( void ) const;
    void SetPreferredDevice( const CString& deviceIdentifier );

    /** The most CPU workers of the CPU backend; 0 for the logical CPUs minus one */
    UInt32 GetMaxCpuWorkerCount( void ) const;
    void SetMaxCpuWorkerCount( UInt32 count );

    /** The VFS path below which kernel overrides are looked for; empty for none */
    const CString& GetKernelOverrideRoot( void ) const;
    void SetKernelOverrideRoot( const CString& vfsRoot );

    /** Signature checks and the validation layers of backends for devices created through the registry */
    bool GetValidation( void ) const;
    void SetValidation( bool validation );

    /** Whether kernels that need f64 may run on GPUs that are not fast at it; off by default */
    bool GetAllowFloat64OnGpu( void ) const;
    void SetAllowFloat64OnGpu( bool allow );

    void SetKernelProviderOverride( const CString& kernelId, const SKernelProviderOverride& providerOverride );
    bool GetKernelProviderOverride( const CString& kernelId, SKernelProviderOverride& providerOverride ) const;
    bool RemoveKernelProviderOverride( const CString& kernelId );
    const TKernelProviderOverrideMap& GetKernelProviderOverrides( void ) const;

    /** Adds the section "ComputeSettings" to the tree, replacing the one that was there */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the section from the tree, or the tree itself when it is that section. False when there is no such section */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CString::StringSet m_enabledBackends;
    CString::StringSet m_disabledBackends;
    CString::StringVector m_backendOrder;
    CString m_preferredDevice;
    UInt32 m_maxCpuWorkerCount;
    CString m_kernelOverrideRoot;
    bool m_validation;
    bool m_allowFloat64OnGpu;
    TKernelProviderOverrideMap m_kernelOverrides;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTESETTINGS_H ? */
