/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CEXTERNALTHREADSCOPE_H
#include "gucefCORE_CExternalThreadScope.h"
#define GUCEF_CORE_CEXTERNALTHREADSCOPE_H
#endif /* GUCEF_CORE_CEXTERNALTHREADSCOPE_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestExternalThreads.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Stands in for an operating system or driver thread: GUCEF did not create it through the task manager */
class CForeignThread : public MT::CActiveObject
{
    public:

    CForeignThread( UInt32 nestedRegistrations )
        : MT::CActiveObject(), m_nested( nestedRegistrations ), m_registered( false ), m_release( false ), m_done( false ), m_threadId( 0 )
    {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        {
            CORE::CExternalThreadScope scope( "test foreign thread" );
            CORE::CExternalThreadScope* extra[ 4 ] = { GUCEF_NULL, GUCEF_NULL, GUCEF_NULL, GUCEF_NULL };
            for ( UInt32 i=0; i<m_nested && i<4; ++i )
                extra[ i ] = GUCEF_NEW CORE::CExternalThreadScope( "test foreign thread" );

            m_threadId.Store( MT::GetCurrentTaskID(), MT::MEMORDER_RELEASE );
            m_registered.Store( scope.IsRegistered(), MT::MEMORDER_RELEASE );

            UInt32 waited = 0;
            while ( !m_release.Load( MT::MEMORDER_ACQUIRE ) && waited < 10000 )
            {
                MT::PrecisionDelay( 1 );
                ++waited;
            }
            for ( UInt32 i=0; i<4; ++i )
                GUCEF_DELETE extra[ i ];
        }
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true; // stop cycling
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool WaitUntilRegistered( UInt32 timeoutInMs ) { return WaitFor( m_registered, timeoutInMs ); }
    void Release( void ) { m_release.Store( true, MT::MEMORDER_RELEASE ); }
    bool WaitUntilDone( UInt32 timeoutInMs )
    {
        bool done = WaitFor( m_done, timeoutInMs );
        UInt32 waited = 0;
        while ( IsActive() && waited < timeoutInMs ) { MT::PrecisionDelay( 1 ); ++waited; }
        return done && !IsActive();
    }
    UInt32 GetThreadIdOfThread( void ) const { return m_threadId.Load( MT::MEMORDER_ACQUIRE ); }

    private:

    static bool WaitFor( const MT::CTAtomic< bool >& flag, UInt32 timeoutInMs )
    {
        UInt32 waited = 0;
        while ( !flag.Load( MT::MEMORDER_ACQUIRE ) && waited < timeoutInMs ) { MT::PrecisionDelay( 1 ); ++waited; }
        return flag.Load( MT::MEMORDER_ACQUIRE );
    }

    UInt32 m_nested;
    MT::CTAtomic< bool > m_registered;
    MT::CTAtomic< bool > m_release;
    MT::CTAtomic< bool > m_done;
    MT::CTAtomic< UInt32 > m_threadId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformExternalThreadTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING ExternalThreads TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "ExternalThreads" );

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();

    GUCEF_TESTFW_TESTCASE( "ExternalThreads: A foreign thread shows up in the task manager while it is registered and not after" )
    try
    {
        const UInt32 before = taskManager.GetNrOfExternalThreads();
        CORE::CString::StringSet namesBefore;
        taskManager.GetAllThreadPoolNames( namesBefore );
        ASSERT_TRUE( namesBefore.end() == namesBefore.find( CORE::CTaskManager::ExternalThreadPoolName ) || before > 0 );

        CForeignThread thread( 0 );
        ASSERT_TRUE( thread.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( thread.WaitUntilRegistered( 10000 ) );
        const UInt32 threadId = thread.GetThreadIdOfThread();

        ASSERT_TRUE( taskManager.GetNrOfExternalThreads() == before + 1 );

        CORE::CThreadInfo info;
        CORE::CString poolName;
        ASSERT_TRUE( taskManager.GetThreadInfo( threadId, info, poolName ) );
        ASSERT_TRUE( poolName == CORE::CTaskManager::ExternalThreadPoolName );
        ASSERT_TRUE( info.GetIsExternal() );
        ASSERT_TRUE( info.GetThreadName() == "test foreign thread" );
        ASSERT_TRUE( info.GetThreadId() == threadId );

        CORE::CTaskManager::TThreadInfoMap all;
        taskManager.GetAllThreadInfo( all );
        ASSERT_TRUE( all.end() != all.find( threadId ) );

        CORE::CString::StringSet names;
        taskManager.GetAllThreadPoolNames( names );
        ASSERT_TRUE( names.end() != names.find( CORE::CTaskManager::ExternalThreadPoolName ) );

        CORE::CThreadPoolInfo poolInfo;
        ASSERT_TRUE( taskManager.GetThreadPoolInfo( CORE::CTaskManager::ExternalThreadPoolName, poolInfo ) );
        ASSERT_TRUE( poolInfo.GetThreadPoolName() == CORE::CTaskManager::ExternalThreadPoolName );
        ASSERT_TRUE( poolInfo.GetActiveNrOfDedicatedThreads() == before + 1 );

        CORE::CTaskManager::TThreadPoolInfoMap allPools;
        taskManager.GetAllThreadPoolInfo( allPools );
        ASSERT_TRUE( allPools.end() != allPools.find( CORE::CTaskManager::ExternalThreadPoolName ) );

        // it is a tracking pool only: nothing can be scheduled on it
        ASSERT_TRUE( taskManager.GetThreadPool( CORE::CTaskManager::ExternalThreadPoolName ).IsNULL() );

        thread.Release();
        ASSERT_TRUE( thread.WaitUntilDone( 10000 ) );
        ASSERT_TRUE( taskManager.GetNrOfExternalThreads() == before );
        ASSERT_FALSE( taskManager.GetThreadInfo( threadId, info, poolName ) && info.GetIsExternal() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ExternalThreads: Registrations are counted per thread" )
    try
    {
        const UInt32 before = taskManager.GetNrOfExternalThreads();
        CForeignThread thread( 3 );     // registers 4 times in total
        ASSERT_TRUE( thread.Activate( GUCEF_NULL, 0, 0 ) );
        ASSERT_TRUE( thread.WaitUntilRegistered( 10000 ) );
        MT::PrecisionDelay( 20 );       // let the nested registrations happen
        ASSERT_TRUE( taskManager.GetNrOfExternalThreads() == before + 1 );   // still one thread
        thread.Release();
        ASSERT_TRUE( thread.WaitUntilDone( 10000 ) );
        ASSERT_TRUE( taskManager.GetNrOfExternalThreads() == before );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ExternalThreads: Unregistering a thread that was never registered is refused" )
    try
    {
        // this very thread is not registered
        ASSERT_FALSE( taskManager.UnregisterExternalThread() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ExternalThreads: Thread info with name and external flag survives serialization" )
    try
    {
        CORE::CThreadInfo info;
        info.SetThreadId( 77 );
        info.SetThreadName( "audio device" );
        info.SetIsExternal( true );

        CORE::CDataNode node;
        CORE::CDataNodeSerializableSettings settings;
        ASSERT_TRUE( info.Serialize( node, settings ) );

        CORE::CThreadInfo restored;
        ASSERT_TRUE( restored.Deserialize( node, settings ) );
        ASSERT_TRUE( restored.GetThreadId() == 77 );
        ASSERT_TRUE( restored.GetThreadName() == "audio device" );
        ASSERT_TRUE( restored.GetIsExternal() );

        CORE::CThreadInfo copy( restored );
        ASSERT_TRUE( copy.GetIsExternal() && copy.GetThreadName() == "audio device" );
        copy.Clear();
        ASSERT_FALSE( copy.GetIsExternal() );
        ASSERT_TRUE( copy.GetThreadName().IsNULLOrEmpty() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
