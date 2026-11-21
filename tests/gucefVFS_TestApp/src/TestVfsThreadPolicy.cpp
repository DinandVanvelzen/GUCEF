/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestThreads.h"

#include "TestVfsThreadPolicy.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Asks the registry for the rule of the thread it runs on, and sets or clears one when told to */
class CPolicyProbe : public CTestThreadBody
{
    public:

    VFS::CVfsThreadPolicyRegistry* registry;
    MT::CTAtomic< UInt32 > seenAction;
    MT::CTAtomic< UInt32 > seenCount;
    VFS::EMissAction setAction;
    bool doSet;
    MT::CTAtomic< UInt32 > setOk;

    CPolicyProbe( VFS::CVfsThreadPolicyRegistry* r )
        : registry( r ), seenAction( 99 ), seenCount( 0 ), setAction( VFS::MISSACTION_WAIT ), doSet( false ), setOk( 0 ) {}

    virtual void Run( UInt32 /* index */ ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( doSet )
        {
            VFS::CVfsThreadPolicy policy( setAction );
            if ( registry->SetThreadPolicy( MT::GetCurrentThreadID(), policy ) )
                setOk.Store( 1 );
        }
        seenAction.Store( (UInt32) registry->GetCurrent().missAction );
        seenCount.Store( registry->GetRegisteredThreadCount() );
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVfsThreadPolicyTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsThreadPolicy" );

    GUCEF_TESTFW_TESTCASE( "ThreadPolicy: the rule of a thread has WAIT, serving stale metadata and opening partly held files as its defaults; only WAIT waits" )
    try
    {
        VFS::CVfsThreadPolicy policy;
        ASSERT_TRUE( VFS::MISSACTION_WAIT == policy.missAction );
        ASSERT_TRUE( policy.serveStaleMetadata );
        ASSERT_FALSE( policy.openRequiresCompleteFile );
        ASSERT_FALSE( policy.NeverWaits() );
        ASSERT_TRUE( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ).NeverWaits() );
        ASSERT_TRUE( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL ).NeverWaits() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ThreadPolicyRegistry: a thread without a rule gets the default, and asking for it takes no lock while no thread has a rule" )
    try
    {
        VFS::CVfsThreadPolicyRegistry registry;
        ASSERT_TRUE( 0 == registry.GetRegisteredThreadCount() );
        const UInt64 before = registry.GetLockAcquisitionCount();
        for ( UInt32 i=0; i<1000; ++i )
        {
            ASSERT_TRUE( VFS::MISSACTION_WAIT == registry.GetCurrent().missAction );
        }
        ASSERT_TRUE( before == registry.GetLockAcquisitionCount() );

        // a rule for a thread: that one is found, and every thread then looks in the table
        ASSERT_TRUE( registry.SetThreadPolicy( MT::GetCurrentThreadID(), VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL ) ) );
        ASSERT_TRUE( 1 == registry.GetRegisteredThreadCount() );
        ASSERT_TRUE( VFS::MISSACTION_FAIL == registry.GetCurrent().missAction );
        ASSERT_TRUE( registry.GetLockAcquisitionCount() > before );
        registry.ClearThreadPolicy( MT::GetCurrentThreadID() );
        ASSERT_TRUE( 0 == registry.GetRegisteredThreadCount() );
        ASSERT_TRUE( VFS::MISSACTION_WAIT == registry.GetCurrent().missAction );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ThreadPolicyRegistry: the rule of one thread does not apply to another, several threads can have their own, and a rule is replaced and cleared by thread id" )
    try
    {
        VFS::CVfsThreadPolicyRegistry registry;
        ASSERT_TRUE( registry.SetThreadPolicy( MT::GetCurrentThreadID(), VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) ) );

        // another thread: no rule of its own, it waits
        CPolicyProbe probeNoRule( &registry );
        {
            CTestThreads threads( probeNoRule, 1 );
            ASSERT_TRUE( threads.Start() );
            ASSERT_TRUE( threads.Join( 10000 ) );
        }
        ASSERT_TRUE( (UInt32) VFS::MISSACTION_WAIT == probeNoRule.seenAction.Load() );

        // another thread with a rule of its own: it sees its own, this one still sees its own
        CPolicyProbe probeOwnRule( &registry );
        probeOwnRule.doSet = true;
        probeOwnRule.setAction = VFS::MISSACTION_FAIL;
        {
            CTestThreads threads( probeOwnRule, 1 );
            ASSERT_TRUE( threads.Start() );
            ASSERT_TRUE( threads.Join( 10000 ) );
        }
        ASSERT_TRUE( 1 == probeOwnRule.setOk.Load() );
        ASSERT_TRUE( (UInt32) VFS::MISSACTION_FAIL == probeOwnRule.seenAction.Load() );
        ASSERT_TRUE( VFS::MISSACTION_FAIL_AND_FETCH == registry.GetCurrent().missAction );
        ASSERT_TRUE( 2 == registry.GetRegisteredThreadCount() );

        // replaced by the same id, cleared by the id: the owner of the registration removes it
        ASSERT_TRUE( registry.SetThreadPolicy( MT::GetCurrentThreadID(), VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL ) ) );
        ASSERT_TRUE( VFS::MISSACTION_FAIL == registry.GetCurrent().missAction );
        ASSERT_TRUE( 2 == registry.GetRegisteredThreadCount() );
        registry.ClearThreadPolicy( MT::GetCurrentThreadID() );
        ASSERT_TRUE( 1 == registry.GetRegisteredThreadCount() );
        ASSERT_TRUE( VFS::MISSACTION_WAIT == registry.GetCurrent().missAction );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ThreadPolicyRegistry: the table is bounded, a thread beyond the bound is refused and keeps what it had" )
    try
    {
        VFS::CVfsThreadPolicyRegistry registry;
        for ( UInt32 i=0; i<VFS::CVfsThreadPolicyRegistry::MaxThreads; ++i )
        {
            ASSERT_TRUE( registry.SetThreadPolicy( (TThreadId) ( 1000000 + i ), VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL ) ) );
        }
        ASSERT_TRUE( VFS::CVfsThreadPolicyRegistry::MaxThreads == registry.GetRegisteredThreadCount() );
        ASSERT_FALSE( registry.SetThreadPolicy( MT::GetCurrentThreadID(), VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL ) ) );
        ASSERT_TRUE( VFS::MISSACTION_WAIT == registry.GetCurrent().missAction );
        // a thread that has one can still change it
        ASSERT_TRUE( registry.SetThreadPolicy( (TThreadId) 1000000, VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) ) );
        registry.ClearThreadPolicy( (TThreadId) 1000000 );
        ASSERT_TRUE( registry.SetThreadPolicy( MT::GetCurrentThreadID(), VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL ) ) );
        registry.ClearThreadPolicy( MT::GetCurrentThreadID() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ThreadPolicyScope: the rule holds for the scope, nested scopes restore what the outer one had, and a registration of the thread comes back after the scope" )
    try
    {
        VFS::CVfsThreadPolicyRegistry registry;
        ASSERT_TRUE( VFS::MISSACTION_WAIT == registry.GetCurrent().missAction );
        {
            VFS::CVfsThreadPolicyScope outer( registry, VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) );
            ASSERT_TRUE( VFS::MISSACTION_FAIL_AND_FETCH == registry.GetCurrent().missAction );
            {
                VFS::CVfsThreadPolicy inner( VFS::MISSACTION_FAIL );
                inner.openRequiresCompleteFile = true;
                VFS::CVfsThreadPolicyScope innerScope( registry, inner );
                ASSERT_TRUE( VFS::MISSACTION_FAIL == registry.GetCurrent().missAction );
                ASSERT_TRUE( registry.GetCurrent().openRequiresCompleteFile );
            }
            ASSERT_TRUE( VFS::MISSACTION_FAIL_AND_FETCH == registry.GetCurrent().missAction );
            ASSERT_FALSE( registry.GetCurrent().openRequiresCompleteFile );
        }
        ASSERT_TRUE( VFS::MISSACTION_WAIT == registry.GetCurrent().missAction );
        ASSERT_TRUE( 0 == registry.GetRegisteredThreadCount() );

        // a registration made before the scope is what is back after it
        ASSERT_TRUE( registry.SetThreadPolicy( MT::GetCurrentThreadID(), VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL ) ) );
        {
            VFS::CVfsThreadPolicyScope scope( registry, VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) );
            ASSERT_TRUE( VFS::MISSACTION_FAIL_AND_FETCH == registry.GetCurrent().missAction );
        }
        ASSERT_TRUE( VFS::MISSACTION_FAIL == registry.GetCurrent().missAction );
        registry.ClearThreadPolicy( MT::GetCurrentThreadID() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ThreadPolicyScope: a scope of the VFS registry applies to the calling thread only and ends with the scope" )
    try
    {
        VFS::CVfsThreadPolicyRegistry& registry = VFS::CVfsGlobal::Instance()->GetThreadPolicyRegistry();
        ASSERT_TRUE( VFS::MISSACTION_WAIT == registry.GetCurrent().missAction );
        CPolicyProbe probe( &registry );
        {
            VFS::CVfsThreadPolicyScope scope( VFS::CVfsThreadPolicy( VFS::MISSACTION_FAIL_AND_FETCH ) );
            ASSERT_TRUE( VFS::MISSACTION_FAIL_AND_FETCH == registry.GetCurrent().missAction );
            CTestThreads threads( probe, 1 );
            ASSERT_TRUE( threads.Start() );
            ASSERT_TRUE( threads.Join( 10000 ) );
        }
        ASSERT_TRUE( (UInt32) VFS::MISSACTION_WAIT == probe.seenAction.Load() );
        ASSERT_TRUE( VFS::MISSACTION_WAIT == registry.GetCurrent().missAction );
        ASSERT_TRUE( 0 == registry.GetRegisteredThreadCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ThreadPolicyScope: threads that each use scopes at once see their own rule" )
    try
    {
        VFS::CVfsThreadPolicyRegistry registry;

        class CScopedThreads : public CTestThreadBody
        {
            public:
            VFS::CVfsThreadPolicyRegistry* registry;
            MT::CTAtomic< UInt32 > wrong;
            explicit CScopedThreads( VFS::CVfsThreadPolicyRegistry* r ) : registry( r ), wrong( 0 ) {}
            virtual void Run( UInt32 index ) GUCEF_VIRTUAL_OVERRIDE
            {
                const VFS::EMissAction mine = ( index % 2 ) ? VFS::MISSACTION_FAIL : VFS::MISSACTION_FAIL_AND_FETCH;
                for ( UInt32 i=0; i<2000; ++i )
                {
                    VFS::CVfsThreadPolicyScope scope( *registry, VFS::CVfsThreadPolicy( mine ) );
                    if ( registry->GetCurrent().missAction != mine )
                        wrong.FetchAdd( 1 );
                }
            }
        } body( &registry );

        CTestThreads threads( body, 4 );
        ASSERT_TRUE( threads.Start() );
        ASSERT_TRUE( threads.Join( 60000 ) );
        ASSERT_TRUE( 0 == body.wrong.Load() );
        ASSERT_TRUE( 0 == registry.GetRegisteredThreadCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
