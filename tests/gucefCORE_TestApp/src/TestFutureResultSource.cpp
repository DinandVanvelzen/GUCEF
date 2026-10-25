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

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

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

#include "TestFutureResultSource.h"

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

/** Data that names its class, as futures and event data do that are checked by their users */
class CNamedData : public CORE::CICloneable
{
    public:

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CNamedData( *this ); }
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return CloneableTypeName(); }
    static const char* CloneableTypeName( void ) { return "GUCEF::TEST::CNamedData"; }
};

class COtherNamedData : public CORE::CICloneable
{
    public:

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW COtherNamedData( *this ); }
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return CloneableTypeName(); }
    static const char* CloneableTypeName( void ) { return "GUCEF::TEST::COtherNamedData"; }
};

/** A thread GUCEF's task manager knows nothing about that does some work and then completes a source */
class CCompletingThread : public MT::CActiveObject
{
    public:

    enum EAction { ACTION_COMPLETE, ACTION_FAIL, ACTION_CANCEL };

    CCompletingThread( const CORE::CFutureResultSource& source, EAction action, UInt32 delayInMs )
        : MT::CActiveObject(), m_source( source ), m_action( action ), m_delayInMs( delayInMs ), m_won( false ), m_done( false )
    {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        if ( m_delayInMs > 0 )
            MT::PrecisionDelay( m_delayInMs );
        bool won = false;
        switch ( m_action )
        {
            case ACTION_COMPLETE : won = m_source.Complete(); break;
            case ACTION_FAIL     : won = m_source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "thread says no" ); break;
            default              : won = m_source.Cancel(); break;
        }
        m_won.Store( won, MT::MEMORDER_RELEASE );
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true; // stop cycling
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool IsDone( void ) const { return m_done.Load( MT::MEMORDER_ACQUIRE ); }
    bool Won( void ) const { return m_won.Load( MT::MEMORDER_ACQUIRE ); }

    bool WaitUntilDone( UInt32 timeoutInMs )
    {
        UInt32 waited = 0;
        while ( ( !IsDone() || IsActive() ) && waited < timeoutInMs )
        {
            MT::PrecisionDelay( 1 );
            ++waited;
        }
        return IsDone() && !IsActive();
    }

    private:

    CORE::CFutureResultSource m_source;
    EAction m_action;
    UInt32 m_delayInMs;
    MT::CTAtomic< bool > m_won;
    MT::CTAtomic< bool > m_done;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformBasicTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "FutureResultSource: Pending until completed, then ready" )
    try
    {
        CORE::CFutureResultSource source;
        CORE::CFutureResult future = source.GetFuture();
        ASSERT_TRUE( future.HasAFuture() );
        ASSERT_FALSE( future.IsReady() );
        ASSERT_FALSE( source.IsCompleted() );

        bool timedOut = false;
        try
        {
            future.Await( 30 );
        }
        catch ( const timeout_exception& )
        {
            timedOut = true;
        }
        ASSERT_TRUE( timedOut );

        ASSERT_TRUE( source.Complete() );
        ASSERT_TRUE( future.IsReady() );
        ASSERT_TRUE( source.IsCompleted() );
        future.Await( 1000 );
        CORE::CTaskPtr task = future.GetResult( 1000 );
        ASSERT_FALSE( task.IsNULL() );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == task->GetTaskStatus() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FutureResultSource: First completion wins" )
    try
    {
        CORE::CFutureResultSource source;
        CORE::CFutureResult future = source.GetFuture();
        ASSERT_TRUE( source.Fail( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED, "no room" ) );
        ASSERT_FALSE( source.Complete() );
        ASSERT_FALSE( source.Cancel() );
        ASSERT_FALSE( source.Fail( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_FALSE( source.SetStatus( CORE::TTaskStatus::TASKSTATUS_RUNNING ) );
        CORE::CTaskPtr task = future.GetResult( 1000 );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED == task->GetTaskStatus() );
        ASSERT_TRUE( task->IsTaskInErrorState() );
        ASSERT_TRUE( task->GetTaskStatusExtraInfo() == "no room" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FutureResultSource: A throttle status keeps the future pending, it can still complete when the condition clears" )
    try
    {
        CORE::CFutureResultSource source;
        CORE::CFutureResult future = source.GetFuture();

        // RESOURCE_LIMIT_REACHED and RESOURCE_NOT_AVAILABLE are not end states: Fail() refuses them
        ASSERT_FALSE( source.Fail( CORE::TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED ) );
        ASSERT_FALSE( source.Fail( CORE::TTaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE ) );
        ASSERT_FALSE( source.Fail( CORE::TTaskStatus::TASKSTATUS_RUNNING ) );           // not an error at all
        ASSERT_FALSE( future.IsReady() );

        // reported as the current status instead: the caller sees the error through HasNoFuture() while it lasts
        ASSERT_TRUE( source.SetStatus( CORE::TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED ) );
        ASSERT_FALSE( future.IsReady() );
        ASSERT_TRUE( future.HasNoFuture() );

        // the limit clears, the work goes on and finishes
        ASSERT_TRUE( source.SetStatus( CORE::TTaskStatus::TASKSTATUS_RUNNING ) );
        ASSERT_FALSE( future.HasNoFuture() );
        ASSERT_TRUE( source.Complete() );
        ASSERT_TRUE( future.IsReady() );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == future.GetResult( 1000 )->GetTaskStatus() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FutureResultSource: Status moves through the non end states and rejects bad requests" )
    try
    {
        CORE::CFutureResultSource source( CORE::TTaskStatus::TASKSTATUS_QUEUED );
        CORE::CFutureResult future = source.GetFuture();
        ASSERT_TRUE( source.SetStatus( CORE::TTaskStatus::TASKSTATUS_RUNNING ) );
        ASSERT_TRUE( source.SetStatus( CORE::TTaskStatus::TASKSTATUS_PAUSED ) );
        ASSERT_TRUE( source.SetStatus( CORE::TTaskStatus::TASKSTATUS_RESUMED ) );
        ASSERT_FALSE( source.SetStatus( CORE::TTaskStatus::TASKSTATUS_FINISHED ) );     // end states go through Complete()
        ASSERT_FALSE( source.SetStatus( CORE::TTaskStatus::TASKSTATUS_UNDEFINED ) );
        ASSERT_FALSE( source.Complete( CORE::TTaskStatus::TASKSTATUS_RUNNING ) );        // not an end state
        ASSERT_FALSE( future.IsReady() );
        ASSERT_TRUE( source.Complete( CORE::TTaskStatus::TASKSTATUS_STOPPED, "stopped by test" ) );
        ASSERT_TRUE( future.IsReady() );

        // an end state as initial state is not a pending future, it is turned into running
        CORE::CFutureResultSource odd( CORE::TTaskStatus::TASKSTATUS_FINISHED );
        ASSERT_FALSE( odd.GetFuture().IsReady() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FutureResultSource: Copies share one future, result data is delivered" )
    try
    {
        CORE::CFutureResultSource source;
        CORE::CFutureResultSource copy( source );
        CORE::CFutureResultSource assigned;
        assigned = source;
        CORE::CFutureResult future = source.GetFuture();

        CORE::TCloneableString* data = GUCEF_NEW CORE::TCloneableString( "the outcome" );
        ASSERT_TRUE( copy.Complete( data, true ) );
        ASSERT_TRUE( future.IsReady() );
        ASSERT_TRUE( assigned.IsCompleted() );
        ASSERT_FALSE( source.Complete() );

        CORE::CTaskPtr task = future.GetResult( 1000 );
        ASSERT_TRUE( task->HasTaskData() );
    #ifdef GUCEF_RTTI_IS_SUPPORTED
        CORE::TCloneableString* delivered = dynamic_cast< CORE::TCloneableString* >( task->GetTaskData() );
    #else
        // without RTTI there is no type check for this generic data class: this test put a TCloneableString in, the value check below verifies it
        CORE::TCloneableString* delivered = static_cast< CORE::TCloneableString* >( task->GetTaskData() );
    #endif
        ASSERT_TRUE( GUCEF_NULL != delivered );
        if ( GUCEF_NULL != delivered )
            ASSERT_TRUE( delivered->GetData() == "the outcome" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Cloneable type check: Works by name without RTTI, and agrees with RTTI where that is available" )
    try
    {
        CNamedData named;
        COtherNamedData other;
        CORE::TCloneableString text( "abc" );
        CORE::CICloneable* namedBase = &named;
        CORE::CICloneable* otherBase = &other;
        CORE::CICloneable* textBase = &text;

        // classes that do not say what they are never match
        ASSERT_TRUE( GUCEF_NULL == textBase->GetCloneableTypeName() );
        ASSERT_FALSE( textBase->IsCloneableType( "GUCEF::CORE::TCloneableString" ) );
        ASSERT_FALSE( textBase->IsCloneableType( GUCEF_NULL ) );
        ASSERT_TRUE( GUCEF_NULL == CORE::CICloneable::CloneableAs< CNamedData >( textBase ) );

        // classes that do say what they are match themselves only
        ASSERT_TRUE( namedBase->IsCloneableType( CNamedData::CloneableTypeName() ) );
        ASSERT_TRUE( &named == CORE::CICloneable::CloneableAs< CNamedData >( namedBase ) );
        ASSERT_TRUE( GUCEF_NULL == CORE::CICloneable::CloneableAs< CNamedData >( otherBase ) );
        ASSERT_TRUE( &other == CORE::CICloneable::CloneableAs< COtherNamedData >( otherBase ) );
        ASSERT_TRUE( GUCEF_NULL == CORE::CICloneable::CloneableAs< CNamedData >( static_cast< CORE::CICloneable* >( GUCEF_NULL ) ) );
        const CORE::CICloneable* constBase = namedBase;
        ASSERT_TRUE( GUCEF_NULL != CORE::CICloneable::CloneableAs< CNamedData >( constBase ) );

        // a clone says the same
        CORE::CICloneable* copy = namedBase->Clone();
        ASSERT_TRUE( GUCEF_NULL != CORE::CICloneable::CloneableAs< CNamedData >( copy ) );
        delete copy;

        // type names, as factories key on them: same for the same type, different for another, with or without RTTI
        ASSERT_TRUE( CORE::ToString< CNamedData >() == CORE::ToString< CNamedData >() );
        ASSERT_TRUE( CORE::ToString< CNamedData >() != CORE::ToString< COtherNamedData >() );
        ASSERT_TRUE( CORE::ToString< CNamedData >().HasSubstr( "CNamedData" ) > -1 );

        // the cutting of the name out of the compiler's function signature, whatever the compiler of this build
        ASSERT_TRUE( "my::Type" == CORE::TypeNameFromToStringSignature( "CString ToString() [with typeNameParam = my::Type]" ) );
        ASSERT_TRUE( "my::Type" == CORE::TypeNameFromToStringSignature( "CString ToString() [typeNameParam = my::Type]" ) );
        ASSERT_TRUE( "my::Type" == CORE::TypeNameFromToStringSignature( "CString ToString() [with typeNameParam = my::Type; CString = CUtf8String]" ) );
        ASSERT_TRUE( "class my::Type" == CORE::TypeNameFromToStringSignature( "class CString __cdecl GUCEF::CORE::ToString<class my::Type>(void)" ) );
        ASSERT_TRUE( "CString unknown()" == CORE::TypeNameFromToStringSignature( "CString unknown()" ) );

    #ifdef GUCEF_RTTI_IS_SUPPORTED
        // RTTI is on in this build: the RTTI answer is the same as the name based one
        ASSERT_TRUE( GUCEF_NULL != dynamic_cast< CNamedData* >( namedBase ) );
        ASSERT_TRUE( GUCEF_NULL == dynamic_cast< CNamedData* >( otherBase ) );
    #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FutureResultSource: A waiter can request cancellation and the worker sees it" )
    try
    {
        CORE::CFutureResultSource source;
        CORE::CFutureResult future = source.GetFuture();
        ASSERT_FALSE( source.IsCancellationRequested() );
        ASSERT_TRUE( future.RequestCancellation() );
        ASSERT_TRUE( source.IsCancellationRequested() );
        ASSERT_FALSE( future.IsReady() );                   // requesting is not the same as the work having ended
        ASSERT_TRUE( source.Cancel() );
        ASSERT_TRUE( future.IsReady() );
        ASSERT_FALSE( future.RequestCancellation() );       // nothing left to cancel
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformThreadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "FutureResultSource: Completed from a thread unknown to the task manager, waited on from another" )
    try
    {
        CORE::CFutureResultSource source;
        CORE::CFutureResult future = source.GetFuture();

        CCompletingThread thread( source, CCompletingThread::ACTION_COMPLETE, 50 );
        ASSERT_TRUE( thread.Activate( GUCEF_NULL, 0, 0 ) );
        future.Await( 10000 );
        ASSERT_TRUE( future.IsReady() );
        ASSERT_TRUE( thread.WaitUntilDone( 5000 ) );
        ASSERT_TRUE( thread.Won() );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == future.GetResult( 1000 )->GetTaskStatus() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FutureResultSource: Many threads race to complete, exactly one wins" )
    try
    {
        const UInt32 rounds = 25;
        const UInt32 threadCount = 6;
        for ( UInt32 round=0; round<rounds; ++round )
        {
            CORE::CFutureResultSource source;
            CORE::CFutureResult future = source.GetFuture();

            CCompletingThread* threads[ threadCount ];
            for ( UInt32 t=0; t<threadCount; ++t )
            {
                const CCompletingThread::EAction action = ( 0 == t % 3 ) ? CCompletingThread::ACTION_COMPLETE : ( ( 1 == t % 3 ) ? CCompletingThread::ACTION_FAIL : CCompletingThread::ACTION_CANCEL );
                threads[ t ] = GUCEF_NEW CCompletingThread( source, action, 0 );
            }
            for ( UInt32 t=0; t<threadCount; ++t )
                ASSERT_TRUE( threads[ t ]->Activate( GUCEF_NULL, 0, 0 ) );

            UInt32 winners = 0;
            bool allDone = true;
            for ( UInt32 t=0; t<threadCount; ++t )
            {
                allDone = threads[ t ]->WaitUntilDone( 10000 ) && allDone;
                if ( threads[ t ]->Won() )
                    ++winners;
            }
            ASSERT_TRUE( allDone );
            ASSERT_TRUE( 1 == winners );
            ASSERT_TRUE( future.IsReady() );
            for ( UInt32 t=0; t<threadCount; ++t )
                if ( threads[ t ]->IsDone() && !threads[ t ]->IsActive() )
                    GUCEF_DELETE threads[ t ];
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FutureResultSource: Future carries the failure from another thread" )
    try
    {
        CORE::CFutureResultSource source;
        CORE::CFutureResult future = source.GetFuture();
        CCompletingThread thread( source, CCompletingThread::ACTION_FAIL, 10 );
        ASSERT_TRUE( thread.Activate( GUCEF_NULL, 0, 0 ) );
        future.Await( 10000 );
        CORE::CTaskPtr task = future.GetResult( 1000 );
        ASSERT_TRUE( task->IsTaskInErrorState() );
        ASSERT_TRUE( task->GetTaskStatusExtraInfo() == "thread says no" );
        ASSERT_TRUE( thread.WaitUntilDone( 5000 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformFutureResultSourceTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING FutureResultSource TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "FutureResultSource" );

    PerformBasicTests();
    PerformThreadTests();
}

/*-------------------------------------------------------------------------*/
