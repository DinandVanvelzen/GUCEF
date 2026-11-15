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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_COMPUTE_H
#include "gucefCOMPUTE.h"
#define GUCEF_COMPUTE_H
#endif /* GUCEF_COMPUTE_H ? */

#ifndef GUCEF_COMPUTE_TESTAPP_TESTCOMPUTESUPPORT_H
#include "TestComputeSupport.h"
#define GUCEF_COMPUTE_TESTAPP_TESTCOMPUTESUPPORT_H
#endif /* GUCEF_COMPUTE_TESTAPP_TESTCOMPUTESUPPORT_H ? */

#include "TestKernelRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::COMPUTE;
using namespace GUCEF::COMPUTE::TEST;

/*-------------------------------------------------------------------------*/

namespace {

const char g_embeddedData[] = "EMBEDDED-SPIRV";
int g_registryTestOwner = 0;
int g_otherOwner = 0;

CComputeKernelDecl
MakeTestDecl( const char* kernelId, UInt32 version )
{
    CComputeKernelDecl decl( kernelId, version );
    decl.GetSignature().SetWorkgroupSize( 64, 1, 1 );
    decl.GetSignature().AddBinding( 0, COMPUTEACCESS_READ_WRITE, ELEMENTTYPE_FLOAT32 );
    return decl;
}

CComputeKernelArtefact
MakeSpirvArtefact( const char* backend, const char* hash )
{
    CComputeKernelArtefact artefact;
    artefact.SetBackendName( backend );
    artefact.SetFormat( ARTEFACTFORMAT_SPIRV );
    artefact.SetEntryPoint( "main" );
    artefact.SetStaticData( g_embeddedData, sizeof( g_embeddedData ) - 1 );
    artefact.SetSourceHash( hash );
    return artefact;
}

CString
ArtefactText( const TComputeKernelArtefactPtr& artefact )
{
    if ( artefact.IsNULL() || GUCEF_NULL == artefact->GetData() )
        return CString();
    return CString( static_cast< const char* >( artefact->GetData() ), static_cast< UInt32 >( artefact->GetDataSizeInBytes() ) );
}

/** Registers and unregisters kernels of its own in a loop while others create kernels */
class CRegisteringBody : public CTestThreadBody
{
    public:
    CRegisteringBody( void ) : CTestThreadBody(), failures( 0 ) {}
    virtual void Run( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        CComputeKernelRegistry& registry = CComputeGlobal::Instance()->GetKernelRegistry();
        for ( UInt32 i=0; i<300; ++i )
        {
            if ( !registry.RegisterKernel( MakeTestDecl( "TEST.Churn.F32", 1 ), &g_otherOwner ) ||
                 !registry.RegisterArtefact( "TEST.Churn.F32", MakeSpirvArtefact( "testgpu", "" ) ) )
                failures.FetchAdd( 1 );
            if ( 1 != registry.UnregisterModuleKernels( &g_otherOwner ) )
                failures.FetchAdd( 1 );
        }
    }
    MT::CTAtomic< UInt32 > failures;
};

/** Creates kernels on a device while the registry changes */
class CKernelCreatingBody : public CTestThreadBody
{
    public:
    explicit CKernelCreatingBody( const TComputeDevicePtr& device ) : CTestThreadBody(), failures( 0 ), m_device( device ) {}
    virtual void Run( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<300; ++i )
        {
            TComputeKernelPtr kernel;
            if ( !m_device->CreateKernel( CCpuTestKernels::FillKernelId, kernel ) )
                failures.FetchAdd( 1 );
            TComputeKernelArtefactPtr artefact;
            CComputeGlobal::Instance()->GetKernelRegistry().GetArtefact( "TEST.Churn.F32", "testgpu", artefact );
            if ( !artefact.IsNULL() && ArtefactText( artefact ) != g_embeddedData )
                failures.FetchAdd( 1 );
        }
    }
    MT::CTAtomic< UInt32 > failures;
    private:
    TComputeDevicePtr m_device;
};

}

/*-------------------------------------------------------------------------*/

void
PerformKernelRegistryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "KernelRegistry" );

    CComputeKernelRegistry& registry = CComputeGlobal::Instance()->GetKernelRegistry();

    GUCEF_TESTFW_TESTCASE( "KernelRegistry: declarations and artefacts register once, invalid ones are refused" )
    try
    {
        const UInt32 before = registry.GetKernelCount();
        ASSERT_TRUE( registry.RegisterKernel( MakeTestDecl( "TEST.Plain.F32", 2 ), &g_registryTestOwner ) );
        ASSERT_FALSE( registry.RegisterKernel( MakeTestDecl( "TEST.Plain.F32", 2 ), &g_registryTestOwner ) );
        ASSERT_FALSE( registry.RegisterKernel( MakeTestDecl( "NotThreeParts", 1 ), &g_registryTestOwner ) );
        ASSERT_FALSE( registry.RegisterKernel( MakeTestDecl( "TEST..F32", 1 ), &g_registryTestOwner ) );
        ASSERT_TRUE( before + 1 == registry.GetKernelCount() );
        ASSERT_TRUE( registry.IsKernelRegistered( "TEST.Plain.F32" ) );

        TComputeKernelDeclPtr decl;
        ASSERT_TRUE( registry.TryGetKernel( "TEST.Plain.F32", decl ) );
        ASSERT_TRUE( 2 == decl->GetInterfaceVersion() );
        ASSERT_FALSE( registry.TryGetKernel( "TEST.Missing.F32", decl ) );

        ASSERT_TRUE( registry.RegisterArtefact( "TEST.Plain.F32", MakeSpirvArtefact( "TestGPU", "abc" ) ) );
        ASSERT_FALSE( registry.RegisterArtefact( "TEST.Missing.F32", MakeSpirvArtefact( "testgpu", "abc" ) ) );
        CComputeKernelArtefact invalid = MakeSpirvArtefact( "testgpu", "abc" );
        invalid.SetEntryPoint( "" );
        ASSERT_FALSE( registry.RegisterArtefact( "TEST.Plain.F32", invalid ) );
        ASSERT_FALSE( registry.RegisterArtefact( "TEST.Plain.F32", CComputeKernelArtefact::ForCpu( GUCEF_NULL ) ) );

        TComputeKernelArtefactPtr artefact;
        ASSERT_TRUE( registry.GetArtefact( "TEST.Plain.F32", "TESTGPU", artefact ) );
        ASSERT_TRUE( ArtefactText( artefact ) == g_embeddedData );
        ASSERT_TRUE( artefact->GetBackendName() == "testgpu" );
        ASSERT_FALSE( artefact->HasOwnedData() );
        ASSERT_TRUE( registry.HasArtefact( "TEST.Plain.F32", "testgpu" ) );
        ASSERT_FALSE( registry.HasArtefact( "TEST.Plain.F32", "cpu" ) );

        CString::StringVector ids;
        registry.GetKernelIds( ids );
        ASSERT_TRUE( ids.size() == registry.GetKernelCount() );

        // a declaration without a CPU artefact is not available on the CPU device
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeKernelPtr kernel;
        TComputeResult result = COMPUTERESULT_SUCCESS;
        ASSERT_FALSE( device->CreateKernel( "TEST.Plain.F32", kernel, &result ) );
        ASSERT_TRUE( COMPUTERESULT_KERNEL_NOT_FOUND == result );

        ASSERT_TRUE( 1 == registry.UnregisterModuleKernels( &g_registryTestOwner ) );
        ASSERT_TRUE( 0 == registry.UnregisterModuleKernels( &g_registryTestOwner ) );
        ASSERT_FALSE( registry.IsKernelRegistered( "TEST.Plain.F32" ) );
        ASSERT_TRUE( before == registry.GetKernelCount() );
    }
    catch( ... )
    {
        registry.UnregisterModuleKernels( &g_registryTestOwner );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "KernelRegistry: a kernel removed while a device uses it keeps working, new creations fail, re-registration builds anew" )
    try
    {
        CCpuTestKernels kernels;
        CComputeKernelRegistry& kernelRegistry = registry;
        // the global test kernels hold the ids, so this set lives under ids of its own: register a copy of the fill kernel
        TComputeDevicePtr device = CreateCpuTestDevice();
        TComputeKernelDeclPtr fillDecl;
        TComputeKernelArtefactPtr fillArtefact;
        ASSERT_TRUE( kernelRegistry.TryGetKernel( CCpuTestKernels::FillKernelId, fillDecl ) );
        ASSERT_TRUE( kernelRegistry.GetArtefact( CCpuTestKernels::FillKernelId, "cpu", fillArtefact ) );
        CComputeKernelDecl decl( *fillDecl );
        decl.SetKernelId( "TEST.RemovedFill.U32" );
        ASSERT_TRUE( kernelRegistry.RegisterKernel( decl, &g_registryTestOwner ) );
        ASSERT_TRUE( kernelRegistry.RegisterArtefact( "TEST.RemovedFill.U32", *fillArtefact ) );

        TComputeKernelPtr kernel;
        ASSERT_TRUE( device->CreateKernel( "TEST.RemovedFill.U32", kernel ) );
        ASSERT_TRUE( device->IsKernelAvailable( "TEST.RemovedFill.U32" ) );
        const UInt32 cached = device->GetKernelCacheSize();

        ASSERT_TRUE( 1 == kernelRegistry.UnregisterModuleKernels( &g_registryTestOwner ) );
        ASSERT_FALSE( device->IsKernelAvailable( "TEST.RemovedFill.U32" ) );
        TComputeKernelPtr again;
        TComputeResult result = COMPUTERESULT_SUCCESS;
        ASSERT_FALSE( device->CreateKernel( "TEST.RemovedFill.U32", again, &result ) );
        ASSERT_TRUE( COMPUTERESULT_KERNEL_NOT_FOUND == result );

        // the kernel that exists still runs: it holds its declaration and artefact
        ASSERT_TRUE( kernel->GetKernelId() == "TEST.RemovedFill.U32" );
        TComputeBufferPtr out = MakeBuffer( device, 64 * 4 );
        CCpuTestKernels::SFillConstants constants = { 0, 0, 5, 1 };
        CComputeArguments args;
        args.SetBuffer( 0, out );
        args.SetConstants( &constants, sizeof( constants ) );
        UInt32 values[ 64 ];
        CComputeCommandList list;
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == list.Dispatch( kernel, args, CComputeDispatchSize::ForItems( 64 ) ) );
        list.ReadToHost( out, 0, values, sizeof( values ) );
        ASSERT_TRUE( COMPUTERESULT_SUCCESS == device->GetDefaultQueue()->Execute( list ) );
        ASSERT_TRUE( 5 == values[ 0 ] && 68 == values[ 63 ] );

        // registered again: a new kernel, not the stale cached one
        ASSERT_TRUE( kernelRegistry.RegisterKernel( decl, &g_registryTestOwner ) );
        ASSERT_TRUE( kernelRegistry.RegisterArtefact( "TEST.RemovedFill.U32", *fillArtefact ) );
        ASSERT_TRUE( device->CreateKernel( "TEST.RemovedFill.U32", again ) );
        ASSERT_TRUE( again.GetPointerAlways() != kernel.GetPointerAlways() );
        ASSERT_TRUE( cached == device->GetKernelCacheSize() );
        device->ClearKernelCache();
        ASSERT_TRUE( 0 == device->GetKernelCacheSize() );
        kernelRegistry.UnregisterModuleKernels( &g_registryTestOwner );

        // a second set of test kernels cannot take ids that are registered already
        ASSERT_FALSE( kernels.Register( kernelRegistry ) );
        ASSERT_TRUE( kernels.IsRegistered() );
        ASSERT_TRUE( 0 == kernels.Unregister() );
        ASSERT_FALSE( kernels.IsRegistered() );
    }
    catch( ... )
    {
        registry.UnregisterModuleKernels( &g_registryTestOwner );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "KernelRegistry: an override from the VFS replaces the embedded artefact only with a matching interface version" )
    try
    {
        // the override files in a directory that is mounted into the VFS
        CString root = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( root, "ComputeKernelOverrides" );
        CString matching = root;
        CORE::AppendToPath( matching, "TEST.Override.F32" );
        CString mismatching = root;
        CORE::AppendToPath( mismatching, "TEST.OldOverride.F32" );
        CString dataOnly = root;
        CORE::AppendToPath( dataOnly, "TEST.DataOnly.F32" );
        ASSERT_TRUE( CORE::CreateDirs( matching ) && CORE::CreateDirs( mismatching ) && CORE::CreateDirs( dataOnly ) );

        CString file;
        file = matching; CORE::AppendToPath( file, "testgpu.ini" );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( file, "[ComputeKernelArtefact]\ninterfaceVersion=3\nformat=spirv\nentryPoint=mainOverride\nsourceHash=feed01\ncompiler=slangc test\n" ) );
        file = matching; CORE::AppendToPath( file, "testgpu.spv" );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( file, "OVERRIDE-SPIRV" ) );
        file = matching; CORE::AppendToPath( file, "cpu.ini" );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( file, "[ComputeKernelArtefact]\ninterfaceVersion=3\nformat=spirv\nentryPoint=main\n" ) );
        file = mismatching; CORE::AppendToPath( file, "testgpu.ini" );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( file, "[ComputeKernelArtefact]\ninterfaceVersion=2\nformat=spirv\nentryPoint=main\nsourceHash=old\n" ) );
        file = mismatching; CORE::AppendToPath( file, "testgpu.spv" );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( file, "OLD-SPIRV" ) );
        file = dataOnly; CORE::AppendToPath( file, "testgpu.ini" );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( file, "[ComputeKernelArtefact]\ninterfaceVersion=1\nformat=wgsl\nentryPoint=cs_main\n" ) );
        file = dataOnly; CORE::AppendToPath( file, "testgpu.wgsl" );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( file, "@compute fn cs_main() {}" ) );

        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        ASSERT_TRUE( vfs.MountArchive( "ComputeKernelOverrideTest", root, VFS::CVFS::FileSystemArchiveTypeName, "ComputeOverrides", false, false, root ) );

        ASSERT_TRUE( registry.RegisterKernel( MakeTestDecl( "TEST.Override.F32", 3 ), &g_registryTestOwner ) );
        ASSERT_TRUE( registry.RegisterArtefact( "TEST.Override.F32", MakeSpirvArtefact( "testgpu", "embedded01" ) ) );
        ASSERT_TRUE( registry.RegisterKernel( MakeTestDecl( "TEST.OldOverride.F32", 3 ), &g_registryTestOwner ) );
        ASSERT_TRUE( registry.RegisterArtefact( "TEST.OldOverride.F32", MakeSpirvArtefact( "testgpu", "embedded02" ) ) );
        ASSERT_TRUE( registry.RegisterKernel( MakeTestDecl( "TEST.DataOnly.F32", 1 ), &g_registryTestOwner ) );

        // no override root: the embedded artefacts
        TComputeKernelArtefactPtr artefact;
        ASSERT_TRUE( registry.GetArtefact( "TEST.Override.F32", "testgpu", artefact ) );
        ASSERT_TRUE( ArtefactText( artefact ) == g_embeddedData );
        ASSERT_FALSE( registry.HasArtefact( "TEST.DataOnly.F32", "testgpu" ) );

        CComputeSettings settings = CComputeGlobal::Instance()->GetSettings();
        settings.SetKernelOverrideRoot( "ComputeOverrides" );
        CLogCapture capture;
        CComputeGlobal::Instance()->ApplySettings( settings );
        ASSERT_TRUE( registry.GetOverrideRoot() == "ComputeOverrides" );

        ASSERT_TRUE( registry.GetArtefact( "TEST.Override.F32", "testgpu", artefact ) );
        if ( artefact.IsNULL() )
            throw 0;
        ASSERT_TRUE( ArtefactText( artefact ) == "OVERRIDE-SPIRV" );
        ASSERT_TRUE( artefact->HasOwnedData() );
        ASSERT_TRUE( artefact->GetEntryPoint() == "mainOverride" );
        ASSERT_TRUE( artefact->GetSourceHash() == "feed01" );
        ASSERT_TRUE( artefact->GetCompilerId() == "slangc test" );
        ASSERT_TRUE( 3 == artefact->GetInterfaceVersion() );
        ASSERT_TRUE( ARTEFACTFORMAT_SPIRV == artefact->GetFormat() );
        // looked up once: the same object the second time
        TComputeKernelArtefactPtr cachedArtefact;
        ASSERT_TRUE( registry.GetArtefact( "TEST.Override.F32", "testgpu", cachedArtefact ) );
        ASSERT_TRUE( cachedArtefact.GetPointerAlways() == artefact.GetPointerAlways() );

        // the version does not match: the embedded artefact stays
        ASSERT_TRUE( registry.GetArtefact( "TEST.OldOverride.F32", "testgpu", artefact ) );
        ASSERT_TRUE( ArtefactText( artefact ) == g_embeddedData );
        ASSERT_TRUE( artefact->GetSourceHash() == "embedded02" );

        // a kernel pack as data: no embedded artefact for the backend, the override adds one
        ASSERT_TRUE( registry.GetArtefact( "TEST.DataOnly.F32", "testgpu", artefact ) );
        if ( artefact.IsNULL() )
            throw 0;
        ASSERT_TRUE( ARTEFACTFORMAT_WGSL_SOURCE == artefact->GetFormat() );
        ASSERT_TRUE( artefact->GetEntryPoint() == "cs_main" );

        // the CPU backend has no overrides
        ASSERT_TRUE( registry.GetArtefact( CCpuTestKernels::FillKernelId, "cpu", artefact ) );
        ASSERT_TRUE( ARTEFACTFORMAT_CPU == artefact->GetFormat() );
        ASSERT_FALSE( registry.HasArtefact( "TEST.Override.F32", "cpu" ) );

        // both hashes are in the log, of the override used and of the one refused
        ASSERT_TRUE( capture.CountOccurrences( "feed01" ) >= 1 );
        ASSERT_TRUE( capture.CountOccurrences( "embedded01" ) >= 1 );
        ASSERT_TRUE( capture.CountOccurrences( "embedded02" ) >= 1 );
        ASSERT_TRUE( capture.CountOccurrences( "interface version 2" ) >= 1 );

        // switching the root off brings the embedded artefact back
        settings.SetKernelOverrideRoot( "" );
        CComputeGlobal::Instance()->ApplySettings( settings );
        ASSERT_TRUE( registry.GetArtefact( "TEST.Override.F32", "testgpu", artefact ) );
        ASSERT_TRUE( ArtefactText( artefact ) == g_embeddedData );
        ASSERT_FALSE( registry.HasArtefact( "TEST.DataOnly.F32", "testgpu" ) );
        registry.ClearOverrideCache();

        registry.UnregisterModuleKernels( &g_registryTestOwner );
        vfs.UnmountArchiveByName( "ComputeKernelOverrideTest" );
    }
    catch( ... )
    {
        registry.UnregisterModuleKernels( &g_registryTestOwner );
        VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( "ComputeKernelOverrideTest" );
        CComputeSettings reset = CComputeGlobal::Instance()->GetSettings();
        reset.SetKernelOverrideRoot( "" );
        CComputeGlobal::Instance()->ApplySettings( reset );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "KernelRegistry: the stale-artefact check compares the recorded source hash with the current one" )
    try
    {
        ASSERT_TRUE( registry.RegisterKernel( MakeTestDecl( "TEST.Stale.F32", 1 ), &g_registryTestOwner ) );
        ASSERT_TRUE( registry.RegisterArtefact( "TEST.Stale.F32", MakeSpirvArtefact( "testgpu", "AbC123" ) ) );
        ASSERT_TRUE( registry.IsArtefactSourceHashCurrent( "TEST.Stale.F32", "testgpu", "abc123" ) );
        ASSERT_FALSE( registry.IsArtefactSourceHashCurrent( "TEST.Stale.F32", "testgpu", "abc124" ) );
        ASSERT_TRUE( registry.IsArtefactSourceHashCurrent( "TEST.Stale.F32", "testgpu", "" ) );
        ASSERT_TRUE( registry.IsArtefactSourceHashCurrent( "TEST.Stale.F32", "vulkan", "abc124" ) );
        ASSERT_TRUE( registry.IsArtefactSourceHashCurrent( CCpuTestKernels::FillKernelId, "cpu", "abc124" ) );
        registry.UnregisterModuleKernels( &g_registryTestOwner );
    }
    catch( ... )
    {
        registry.UnregisterModuleKernels( &g_registryTestOwner );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "KernelRegistry: registration and removal on one thread while others create kernels and read artefacts" )
    try
    {
        TComputeDevicePtr device = CreateCpuTestDevice();
        CRegisteringBody registering;
        CKernelCreatingBody creatingA( device );
        CKernelCreatingBody creatingB( device );
        CTestThreadBody* bodies[ 3 ] = { &registering, &creatingA, &creatingB };
        ASSERT_TRUE( RunOnTestThreads( bodies, 3, 60000 ) );
        ASSERT_TRUE( 0 == registering.failures.Load() );
        ASSERT_TRUE( 0 == creatingA.failures.Load() && 0 == creatingB.failures.Load() );
        ASSERT_FALSE( registry.IsKernelRegistered( "TEST.Churn.F32" ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
