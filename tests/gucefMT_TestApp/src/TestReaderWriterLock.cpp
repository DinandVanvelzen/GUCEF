/*
 *  gucefMT_TestApp: GUCEF test application for the MT module
 *  Copyright (C) 2002 - 2022.  Dinand Vanvelzen
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

#include <vector>

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

#ifndef GUCEF_MT_DVRWLOCK_H
#include "gucefMT_DVRWLOCK.h"
#define GUCEF_MT_DVRWLOCK_H
#endif /* GUCEF_MT_DVRWLOCK_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#ifndef GUCEF_MT_CNOLOCK_H
#include "gucefMT_CNoLock.h"
#define GUCEF_MT_CNOLOCK_H
#endif /* GUCEF_MT_CNOLOCK_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#include "TestReaderWriterLock.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

#define GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS      250

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CRwLockTestData;

class CTestWriter : public MT::CActiveObject
{
    public:

    typedef MT::CReadWriteLock::TRWLockStates                           TRWLockStates;
    typedef CORE::CTSharedPtr< MT::CScopeWriterLock, MT::CNoLock >      CScopeWriterLockPtr;
    typedef std::vector< CScopeWriterLockPtr >                          TScopeWriterLockVector;

    CTestWriter( void );

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;    
    virtual void OnThreadStarted( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnThreadEnding( void* taskdata, bool willBeForced ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool WasOnThreadStartTriggered( void ) const;
    bool HadALockFailure( void ) const;
    TRWLockStates GetLastLockOperationResult( void ) const;
    MT::Float64 GetTimeSinceThreadStartInMs( void ) const;
    MT::Float64 GetCyclingDurationInMs( void ) const;
    MT::UInt32 GetCurrentReentrancyDepth( void ) const;

    void SignalCompletionOfWork( void );

    private:

    CRwLockTestData* m_testData;
    bool m_onThreadStartWasTriggered;
    bool m_onThreadEndingWasTriggered;
    TRWLockStates m_lastRwLockOpResult;
    bool m_rwLockFailed;
    bool m_isDone;
    MT::UInt64 m_ticksAtLastCycle;
    MT::UInt64 m_ticksAtThreadStart;
    MT::UInt32 m_currentReentrancyDepth;
    TScopeWriterLockVector m_scopeLocks;
};

/*-------------------------------------------------------------------------*/

class CTestReader : public MT::CActiveObject
{
    public:

    typedef MT::CReadWriteLock::TRWLockStates                           TRWLockStates;
    typedef CORE::CTSharedPtr< MT::CScopeReaderLock, MT::CNoLock >      CScopeReaderLockPtr;
    typedef std::vector< CScopeReaderLockPtr >                          TScopeReaderLockVector;

    CTestReader( void );

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;    
    virtual void OnThreadStarted( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnThreadEnding( void* taskdata, bool willBeForced ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool WasOnThreadStartTriggered( void ) const;
    bool HadALockFailure( void ) const;    
    TRWLockStates GetLastLockOperationResult( void ) const;
    MT::Float64 GetTimeSinceThreadStartInMs( void ) const;
    MT::Float64 GetCyclingDurationInMs( void ) const;
    MT::UInt32 GetCurrentReentrancyDepth( void ) const;

    void SignalCompletionOfWork( void );
    
    private:

    CRwLockTestData* m_testData;
    bool m_onThreadStartWasTriggered;
    bool m_onThreadEndingWasTriggered;
    TRWLockStates m_lastRwLockOpResult;
    bool m_rwLockFailed;
    bool m_isDone;
    MT::UInt64 m_ticksAtLastCycle;
    MT::UInt64 m_ticksAtThreadStart;
    MT::UInt32 m_currentReentrancyDepth;
    TScopeReaderLockVector m_scopeLocks;
};

/*-------------------------------------------------------------------------*/

class CTestReaderAndWriter : public MT::CActiveObject
{
    public:

    typedef MT::CReadWriteLock::TRWLockStates                           TRWLockStates;
    typedef CORE::CTSharedPtr< MT::CScopeReaderLock, MT::CNoLock >      CScopeReaderLockPtr;
    typedef CORE::CTSharedPtr< MT::CScopeWriterLock, MT::CNoLock >      CScopeWriterLockPtr;
    typedef std::pair< CScopeReaderLockPtr, CScopeWriterLockPtr >       TReadWriteScopeLockPtrs;
    typedef std::vector< TReadWriteScopeLockPtrs >                      TScopeReaderWriterLockVector;
    typedef std::vector< bool >                                         TBoolVector;

    CTestReaderAndWriter( void );

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;    
    virtual void OnThreadStarted( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnThreadEnding( void* taskdata, bool willBeForced ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool WasOnThreadStartTriggered( void ) const;
    bool HadALockFailure( void ) const;    
    TRWLockStates GetLastLockOperationResult( void ) const;
    MT::Float64 GetTimeSinceThreadStartInMs( void ) const;
    MT::Float64 GetCyclingDurationInMs( void ) const;
    MT::UInt32 GetCurrentReentrancyDepth( void ) const;

    void SignalCompletionOfWork( void );
    
    private:

    CRwLockTestData* m_testData;
    bool m_onThreadStartWasTriggered;
    bool m_onThreadEndingWasTriggered;
    TRWLockStates m_lastRwLockOpResult;
    bool m_rwLockFailed;
    bool m_isDone;
    MT::UInt64 m_ticksAtLastCycle;
    MT::UInt64 m_ticksAtThreadStart;
    MT::UInt32 m_currentReentrancyDepth;
    TScopeReaderWriterLockVector m_scopeLocks;
    TBoolVector m_depthIsWriterFlags;
};

/*-------------------------------------------------------------------------*/

class CRwLockTestData
{
    public:

    typedef std::vector< CTestWriter >              TTestWriterVector;
    typedef std::vector< CTestReader >              TTestReaderVector;
    typedef std::vector< CTestReaderAndWriter >     TTestReaderAndWriterVector;

    Float64 ticksPerMs;
    
    MT::CReadWriteLock rwLock;
    TTestWriterVector writers;
    TTestReaderVector readers;
    TTestReaderAndWriterVector readerAndWriters;
    UInt32 nrOfWriters;
    UInt32 nrOfReaders;
    UInt32 nrOfReaderAndWriters;

    UInt32 writerWorkDurationInMs;
    UInt32 readerWorkDurationInMs;
    UInt32 readerAndWriterWorkDurationInMs;

    bool writerWorkIsReentrant;
    UInt32 writerReentrancyDepth;
    bool readerWorkIsReentrant;
    UInt32 readerReentrancyDepth;
    bool readerAndWriterWorkIsReentrant;
    UInt32 readerAndWriterReentrancyDepth;

    bool useScopeObjects;
    bool readerAndWriterUsesTopLevelReader;
    Float32 readerAndWriterPercWrites;
    
    CRwLockTestData( bool writersHavePriority, bool usingScopeObjects );
    ~CRwLockTestData();

    bool Shutdown( void );
    bool Configure( void );

    bool DidAnyWriterHaveALockFailure( void ) const;
    bool DidAnyReaderHaveALockFailure( void ) const;
    bool DidAnyReaderWriterHaveALockFailure( void ) const;
    
    void SignalCompletionOfWorkToWriters( UInt32 nrOfWriters );
    void SignalCompletionOfWorkToReaders( UInt32 nrOfReaders );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CRwLockTestData::TTestWriterVector  TTestWriterVector;
typedef CRwLockTestData::TTestReaderVector  TTestReaderVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRwLockTestData::CRwLockTestData( bool writersHavePriority, bool usingScopeObjects )
    : ticksPerMs( MT::PrecisionTimerResolution() / 1000.0 )
    , rwLock( writersHavePriority )
    , writers()
    , readers()
    , readerAndWriters()
    , nrOfWriters( 0 )
    , nrOfReaders( 0 )
    , nrOfReaderAndWriters( 0 )
    , writerWorkDurationInMs( 0 )
    , readerWorkDurationInMs( 0 ) 
    , readerAndWriterWorkDurationInMs( 0 )
    , writerWorkIsReentrant( false )
    , writerReentrancyDepth( 0 )
    , readerWorkIsReentrant( false )
    , readerReentrancyDepth( 0 )
    , readerAndWriterWorkIsReentrant( false )
    , readerAndWriterReentrancyDepth( 0 )
    , useScopeObjects( usingScopeObjects )
    , readerAndWriterUsesTopLevelReader( false )
    , readerAndWriterPercWrites( 0.25 )
{GUCEF_TRACE;
        
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRwLockTestData: Setting up. writersHavePriority=" + CORE::ToString( writersHavePriority ) + " usingScopeObjects=" + CORE::ToString( usingScopeObjects ) );
}

/*-------------------------------------------------------------------------*/

bool
CRwLockTestData::Shutdown( void )
{GUCEF_TRACE;

    if ( !writers.empty() || !readers.empty() || !readerAndWriters.empty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRwLockTestData:Shutdown: Asking all threads to shut down politely" );

        // ask all threads to shut down politely
        // they can perform shutdown in paralell 
    
        TTestWriterVector::iterator w = writers.begin();
        while ( w != writers.end() )
        {
            (*w).Deactivate( false, false );
            ++w;
        }
    
        TTestReaderVector::iterator r = readers.begin();
        while ( r != readers.end() )
        {
            (*r).Deactivate( false, false );
            ++r;
        }
        
        TTestReaderAndWriterVector::iterator j = readerAndWriters.begin();
        while ( j != readerAndWriters.end() )
        {
            (*j).Deactivate( false, false );
            ++j;
        }

        // wait for the threads to be shut down
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRwLockTestData:Shutdown: Waiting for the threads to shut down ...." );
    
        w = writers.begin();
        while ( w != writers.end() )
        {
            MT::UInt32 waitResult = (*w).WaitForThreadToFinish( 30000 );
            if ( waitResult != GUCEF_THREAD_WAIT_OK         &&
                 waitResult != GUCEF_THREAD_WAIT_ABANDONEND  )
            {
                // go for another round
                w = writers.begin();
            }
            ++w;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRwLockTestData:Shutdown: Threads shut down completed for all writer workers" );

        r = readers.begin();
        while ( r != readers.end() )
        {
            MT::UInt32 waitResult = (*r).WaitForThreadToFinish( 30000 );
            if ( waitResult != GUCEF_THREAD_WAIT_OK         &&
                 waitResult != GUCEF_THREAD_WAIT_ABANDONEND  )
            {
                // go for another round
                r = readers.begin();
            }
            ++r;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRwLockTestData:Shutdown: Threads shut down completed for all reader workers" );

        j = readerAndWriters.begin();
        while ( j != readerAndWriters.end() )
        {
            MT::UInt32 waitResult = (*j).WaitForThreadToFinish( 30000 );
            if ( waitResult != GUCEF_THREAD_WAIT_OK         &&
                 waitResult != GUCEF_THREAD_WAIT_ABANDONEND  )
            {
                // go for another round
                j = readerAndWriters.begin();
            }
            ++j;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRwLockTestData:Shutdown: Threads shut down completed for all reader-writer combo workers" );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRwLockTestData:Shutdown: Threads shut down completed for all workers" );
    
        writers.clear();
        readers.clear();
        readerAndWriters.clear();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRwLockTestData::Configure( void )
{GUCEF_TRACE;

    // First shut down whatever is currently running
    if ( !Shutdown() )
        return false;

    // Now cleanly configure the desired number of readers and writers
    
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRwLockTestData:Shutdown: Configuring " + CORE::ToString( nrOfWriters ) + " writers" );
    
    writers.resize( nrOfWriters );
    TTestWriterVector::iterator w = writers.begin();
    while ( w != writers.end() )
    {
        if ( !(*w).Activate( this, true ) )
            return false;
        ++w;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRwLockTestData:Shutdown: Configuring " + CORE::ToString( nrOfReaders ) + " readers" );

    readers.resize( nrOfReaders );
    TTestReaderVector::iterator r = readers.begin();
    while ( r != readers.end() )
    {
        if ( !(*r).Activate( this, true ) )
            return false;
        ++r;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRwLockTestData:Shutdown: Configuring " + CORE::ToString( nrOfReaderAndWriters ) + " reader-writers" );

    readerAndWriters.resize( nrOfReaderAndWriters );
    TTestReaderAndWriterVector::iterator d = readerAndWriters.begin();
    while ( d != readerAndWriters.end() )
    {
        if ( !(*d).Activate( this, true ) )
            return false;
        ++d;
    }

    // To help reduce timing issues in the test on a slow system at least wait for the threads to start

    if ( !readers.empty() )
    {
        bool allReadersStarted = false;
        while ( !allReadersStarted )
        {
            allReadersStarted = true;
            TTestReaderVector::iterator r = readers.begin();
            while ( r != readers.end() )
            {
                if ( !(*r).WasOnThreadStartTriggered() )
                {
                    allReadersStarted = false;
                    break;
                }
                ++r;
            }
            if ( !allReadersStarted )
                MT::PrecisionDelay( 25 );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRwLockTestData:Shutdown: All readers have been started" );
    }

    if ( !writers.empty() )
    {
        bool allWritersStarted = false;
        while ( !allWritersStarted )
        {
            allWritersStarted = true;
            TTestWriterVector::iterator w = writers.begin();
            while ( w != writers.end() )
            {
                if ( !(*w).WasOnThreadStartTriggered() )
                {
                    allWritersStarted = false;
                    break;
                }
                ++w;
            }
            if ( !allWritersStarted )
                MT::PrecisionDelay( 25 );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRwLockTestData:Shutdown: All writers have been started" );
    }

    if ( !readerAndWriters.empty() )
    {
        bool allReaderAndWritersStarted = false;
        while ( !allReaderAndWritersStarted )
        {
            allReaderAndWritersStarted = true;
            TTestReaderAndWriterVector::iterator r = readerAndWriters.begin();
            while ( r != readerAndWriters.end() )
            {
                if ( !(*r).WasOnThreadStartTriggered() )
                {
                    allReaderAndWritersStarted = false;
                    break;
                }
                ++r;
            }
            if ( !allReaderAndWritersStarted )
                MT::PrecisionDelay( 25 );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRwLockTestData:Shutdown: All reader-writers have been started" );
    }

    // Now that we know all threads at least started we also know the very next call would 
    // have been to the reader writer. This will cause some to now be blocked on the lock.
    // We now rely on time passed since thread start to ensure that next instruction was executed
    // this part does rely on timing for the test to be correct but its only for a very small piece
    // of code that needs to execute

    if ( !readers.empty() )
    {
        bool allReadersLikelyInRwLock = false;
        while ( !allReadersLikelyInRwLock )
        {
            allReadersLikelyInRwLock = true;
            TTestReaderVector::iterator r = readers.begin();
            while ( r != readers.end() )
            {
                if ( GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS > (*r).GetTimeSinceThreadStartInMs() )
                {
                    allReadersLikelyInRwLock = false;
                    break;
                }
                ++r;
            }
            if ( !allReadersLikelyInRwLock )
                MT::PrecisionDelay( 25 );
        }
    }

    if ( !writers.empty() )
    {
        bool allWritersLikelyInRwLock = false;
        while ( !allWritersLikelyInRwLock )
        {
            allWritersLikelyInRwLock = true;
            TTestWriterVector::iterator w = writers.begin();
            while ( w != writers.end() )
            {
                if ( GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS > (*w).GetTimeSinceThreadStartInMs() )
                {
                    allWritersLikelyInRwLock = false;
                    break;
                }
                ++w;
            }
            if ( !allWritersLikelyInRwLock )
                MT::PrecisionDelay( 25 );
        }
    }

    if ( !readerAndWriters.empty() )
    {
        bool allReaderAndWritersLikelyInRwLock = false;
        while ( !allReaderAndWritersLikelyInRwLock )
        {
            allReaderAndWritersLikelyInRwLock = true;
            TTestReaderAndWriterVector::iterator r = readerAndWriters.begin();
            while ( r != readerAndWriters.end() )
            {
                if ( GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS > (*r).GetTimeSinceThreadStartInMs() )
                {
                    allReaderAndWritersLikelyInRwLock = false;
                    break;
                }
                ++r;
            }
            if ( !allReaderAndWritersLikelyInRwLock )
                MT::PrecisionDelay( 25 );
        }
    }

    return true;    
}

/*-------------------------------------------------------------------------*/

void
CRwLockTestData::SignalCompletionOfWorkToWriters( UInt32 nrOfWriters )
{GUCEF_TRACE;

    UInt32 signalCount = 0;
    TTestWriterVector::iterator i = writers.begin();
    while ( i != writers.end() )
    {
        if ( signalCount < nrOfReaders )
        {
            (*i).SignalCompletionOfWork();
            ++signalCount;
        }
        else
        {
            break;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CRwLockTestData::SignalCompletionOfWorkToReaders( UInt32 nrOfReaders )
{GUCEF_TRACE;

    UInt32 signalCount = 0;
    TTestReaderVector::iterator i = readers.begin();
    while ( i != readers.end() )
    {
        if ( signalCount < nrOfReaders )
        {
            (*i).SignalCompletionOfWork();
            ++signalCount;
        }
        else
        {
            break;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CRwLockTestData::DidAnyWriterHaveALockFailure( void ) const
{GUCEF_TRACE;

    TTestWriterVector::const_iterator i = writers.begin();
    while ( i != writers.end() )
    {
        if ( (*i).HadALockFailure() )
        {
            return true;
        }
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CRwLockTestData::DidAnyReaderHaveALockFailure( void ) const
{GUCEF_TRACE;

    TTestReaderVector::const_iterator i = readers.begin();
    while ( i != readers.end() )
    {
        if ( (*i).HadALockFailure() )
        {
            return true;
        }
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CRwLockTestData::DidAnyReaderWriterHaveALockFailure( void ) const
{GUCEF_TRACE;

    TTestReaderAndWriterVector::const_iterator i = readerAndWriters.begin();
    while ( i != readerAndWriters.end() )
    {
        if ( (*i).HadALockFailure() )
        {
            return true;
        }
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CRwLockTestData::~CRwLockTestData()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

CTestWriter::CTestWriter( void )
    : MT::CActiveObject()
    , m_testData( GUCEF_NULL )
    , m_onThreadStartWasTriggered( false )
    , m_onThreadEndingWasTriggered( false )
    , m_lastRwLockOpResult( TRWLockStates::RWLOCK_OPERATION_FAILED )
    , m_rwLockFailed( false )
    , m_isDone( false )
    , m_ticksAtLastCycle( 0 )
    , m_ticksAtThreadStart( 0 )
    , m_currentReentrancyDepth( 0 )
    , m_scopeLocks()
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/

bool
CTestWriter::WasOnThreadStartTriggered( void ) const
{GUCEF_TRACE;
        
    return m_onThreadStartWasTriggered;
}

/*-------------------------------------------------------------------------*/

bool
CTestWriter::HadALockFailure( void ) const
{GUCEF_TRACE;
        
    return m_rwLockFailed;
}

/*-------------------------------------------------------------------------*/

CTestWriter::TRWLockStates 
CTestWriter::GetLastLockOperationResult( void ) const
{GUCEF_TRACE;
        
    return m_lastRwLockOpResult;
}

/*-------------------------------------------------------------------------*/

void 
CTestWriter::SignalCompletionOfWork( void )
{GUCEF_TRACE;
        
    m_isDone = true;
}

/*-------------------------------------------------------------------------*/

MT::Float64 
CTestWriter::GetTimeSinceThreadStartInMs( void ) const
{GUCEF_TRACE;
    
    return ( MT::PrecisionTickCount() - m_ticksAtThreadStart ) / m_testData->ticksPerMs;
}

/*-------------------------------------------------------------------------*/

MT::Float64 
CTestWriter::GetCyclingDurationInMs( void ) const
{GUCEF_TRACE;
    
    return ( m_ticksAtLastCycle - m_ticksAtThreadStart ) / m_testData->ticksPerMs;
}

/*-------------------------------------------------------------------------*/

MT::UInt32 
CTestWriter::GetCurrentReentrancyDepth( void ) const
{GUCEF_TRACE;
        
    return m_currentReentrancyDepth;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CTestWriter::AsLockable( void ) const
{GUCEF_TRACE;
        
    return static_cast< const MT::CILockable* >( this );
}

/*-------------------------------------------------------------------------*/

bool
CTestWriter::OnThreadStart( void* taskdata )
{GUCEF_TRACE;
        
    m_testData = static_cast< CRwLockTestData* >( taskdata );    
    m_ticksAtThreadStart = m_ticksAtLastCycle = MT::PrecisionTickCount();
    m_onThreadStartWasTriggered = true;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CTestWriter:OnThreadStart: writer worker starting" );
    
    if ( m_testData->useScopeObjects )
    {
        m_scopeLocks.reserve( m_testData->writerReentrancyDepth );
        
        CScopeWriterLockPtr scopeLock;
        try
        {
            // Since scope objects area really intended for lock/unlock as your code stack flows we simulate a code callstack
            // simply by having the scope locks in a collections which represents the callstack
            scopeLock = CScopeWriterLockPtr( GUCEF_NEW MT::CScopeWriterLock( m_testData->rwLock ) );
            m_scopeLocks.push_back( scopeLock );
        }
        catch ( const timeout_exception& )
        {
            m_rwLockFailed = true;
        }

        if ( !scopeLock->IsScopeWriteLocked() )
        {
            m_rwLockFailed = true;
        }
    }
    else
    {
        m_lastRwLockOpResult = m_testData->rwLock.WriterStart();
        if ( TRWLockStates::RWLOCK_OPERATION_SUCCESS != m_lastRwLockOpResult )
            m_rwLockFailed = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/
    
void
CTestWriter::OnThreadStarted( void* taskdata ) 
{GUCEF_TRACE;
        
}

/*-------------------------------------------------------------------------*/

bool
CTestWriter::OnThreadCycle( void* taskdata ) 
{GUCEF_TRACE;
        
    MT::UInt64 newTickCount = MT::PrecisionTickCount();
    MT::Float64 timeDeltaInMs = ( newTickCount - m_ticksAtThreadStart ) / m_testData->ticksPerMs;

    if ( timeDeltaInMs >= m_testData->writerWorkDurationInMs )
        return true; // we are done

    if ( m_testData->writerWorkIsReentrant && ( m_currentReentrancyDepth < m_testData->writerReentrancyDepth ) )
    {
        // We already hold the lock to be able to get here
        // however we will now make a reentrant lock request from the same thread

        if ( m_testData->useScopeObjects )
        {        
            CScopeWriterLockPtr scopeLock;
            try
            {
                // Since scope objects area really intended for lock/unlock as your code stack flows we simulate a code callstack
                // simply by having the scope locks in a collections which represents the callstack
                scopeLock = CScopeWriterLockPtr( GUCEF_NEW MT::CScopeWriterLock( m_testData->rwLock ) );
                m_scopeLocks.push_back( scopeLock );
            }
            catch ( const timeout_exception& )
            {
                m_rwLockFailed = true;
            }

            if ( !scopeLock->IsScopeWriteLocked() )
            {
                m_rwLockFailed = true;
            }

            if ( !m_rwLockFailed )
            {
                ++m_currentReentrancyDepth;
            }
        }
        else
        {
            m_lastRwLockOpResult = m_testData->rwLock.WriterStart();
            if ( TRWLockStates::RWLOCK_OPERATION_SUCCESS != m_lastRwLockOpResult )
            {
                m_rwLockFailed = true;
            }
            else
            {
                ++m_currentReentrancyDepth;
            }
        }
    }
    
    m_ticksAtLastCycle = newTickCount;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CTestWriter::OnThreadEnding( void* taskdata, bool willBeForced )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CTestWriter:OnThreadStart: writer worker ending" );
    
    if ( m_testData->useScopeObjects )    
    {
        if ( m_testData->writerWorkIsReentrant )
        {
            ASSERT_TRUE( m_currentReentrancyDepth+1 == m_scopeLocks.size() );
            
            try
            {
                // We could just clear the entire lock collection but we are doing it intentionally verbose here
                // intent is to simulate a stack unroll
                for ( MT::UInt32 i=0; i<m_currentReentrancyDepth; ++i )
                {
                    // We must unroll the reentrancy on this thread
                    // similar to rolling up in the call stack
                    CScopeWriterLockPtr scopeLock = m_scopeLocks.back();
                    m_scopeLocks.pop_back();

                    // We are supposedly leaving the callstack scope so we end the life of the scope object
                    ASSERT_TRUE( 1 == scopeLock.GetReferenceCount() );
                    scopeLock.Unlink();
                }
            }
            catch ( const timeout_exception& )
            {
                m_rwLockFailed = true;
            }
        }

        m_scopeLocks.clear();
    }
    else
    {
        if ( m_testData->writerWorkIsReentrant )
        {
            for ( MT::UInt32 i=0; i<m_currentReentrancyDepth; ++i )
            {
                // We must unroll the reentrancy on this thread
                // similar to rolling up in the call stack
                m_lastRwLockOpResult = m_testData->rwLock.WriterStop();
                if ( TRWLockStates::RWLOCK_OPERATION_SUCCESS != m_lastRwLockOpResult )
                {
                    m_rwLockFailed = true;
                }
            }
        }

        m_lastRwLockOpResult = m_testData->rwLock.WriterStop();
        if ( TRWLockStates::RWLOCK_OPERATION_SUCCESS != m_lastRwLockOpResult )
        {
            m_rwLockFailed = true;
        }
    }

    m_onThreadEndingWasTriggered = true;
}

/*-------------------------------------------------------------------------*/

CTestReader::CTestReader( void )
    : MT::CActiveObject()
    , m_testData( GUCEF_NULL )
    , m_onThreadStartWasTriggered( false )
    , m_onThreadEndingWasTriggered( false )
    , m_lastRwLockOpResult( TRWLockStates::RWLOCK_OPERATION_FAILED )
    , m_rwLockFailed( false )
    , m_isDone( false )
    , m_ticksAtLastCycle( 0 )
    , m_ticksAtThreadStart( 0 )
    , m_currentReentrancyDepth( 0 )
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/

bool
CTestReader::WasOnThreadStartTriggered( void ) const
{GUCEF_TRACE;
        
    return m_onThreadStartWasTriggered;
}

/*-------------------------------------------------------------------------*/

bool
CTestReader::HadALockFailure( void ) const
{GUCEF_TRACE;
        
    return m_rwLockFailed;
}

/*-------------------------------------------------------------------------*/

CTestReader::TRWLockStates 
CTestReader::GetLastLockOperationResult( void ) const
{GUCEF_TRACE;
        
    return m_lastRwLockOpResult;
}

/*-------------------------------------------------------------------------*/

void 
CTestReader::SignalCompletionOfWork( void )
{GUCEF_TRACE;
        
    m_isDone = true;
}

/*-------------------------------------------------------------------------*/

MT::Float64 
CTestReader::GetTimeSinceThreadStartInMs( void ) const
{GUCEF_TRACE;
    
    return ( MT::PrecisionTickCount() - m_ticksAtThreadStart ) / m_testData->ticksPerMs;
}

/*-------------------------------------------------------------------------*/

MT::Float64 
CTestReader::GetCyclingDurationInMs( void ) const
{GUCEF_TRACE;
    
    return ( m_ticksAtLastCycle - m_ticksAtThreadStart ) / m_testData->ticksPerMs;
}

/*-------------------------------------------------------------------------*/

MT::UInt32 
CTestReader::GetCurrentReentrancyDepth( void ) const
{GUCEF_TRACE;
        
    return m_currentReentrancyDepth;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CTestReader::AsLockable( void ) const
{GUCEF_TRACE;
        
    return static_cast< const MT::CILockable* >( this );
}

/*-------------------------------------------------------------------------*/

bool
CTestReader::OnThreadStart( void* taskdata )
{GUCEF_TRACE;
        
    m_testData = static_cast< CRwLockTestData* >( taskdata );    
    m_ticksAtThreadStart = m_ticksAtLastCycle = MT::PrecisionTickCount();
    m_onThreadStartWasTriggered = true;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CTestWriter:OnThreadStart: reader worker starting" );
    
    if ( m_testData->useScopeObjects )
    {
        m_scopeLocks.reserve( m_testData->readerReentrancyDepth );
        
        CScopeReaderLockPtr scopeLock;
        try
        {
            // Since scope objects area really intended for lock/unlock as your code stack flows we simulate a code callstack
            // simply by having the scope locks in a collections which represents the callstack
            scopeLock = CScopeReaderLockPtr( GUCEF_NEW MT::CScopeReaderLock( m_testData->rwLock ) );
            m_scopeLocks.push_back( scopeLock );
        }
        catch ( const timeout_exception& )
        {
            m_rwLockFailed = true;
        }

        if ( !scopeLock->IsScopeReadLocked() )
        {
            m_rwLockFailed = true;
        }
    }
    else
    {
        m_lastRwLockOpResult = m_testData->rwLock.ReaderStart();    
        if ( TRWLockStates::RWLOCK_OPERATION_SUCCESS != m_lastRwLockOpResult )
            m_rwLockFailed = true;
    
    }
    return true;
}

/*-------------------------------------------------------------------------*/
    
void
CTestReader::OnThreadStarted( void* taskdata ) 
{GUCEF_TRACE;
        
}

/*-------------------------------------------------------------------------*/

bool
CTestReader::OnThreadCycle( void* taskdata ) 
{GUCEF_TRACE;
    
    MT::UInt64 newTickCount = MT::PrecisionTickCount();
    MT::Float64 timeDeltaInMs = ( newTickCount - m_ticksAtThreadStart ) / m_testData->ticksPerMs;

    if ( timeDeltaInMs >= m_testData->readerWorkDurationInMs )
        return true; // we are done

    if ( m_testData->readerWorkIsReentrant && ( m_currentReentrancyDepth < m_testData->readerReentrancyDepth ) )
    {
        if ( m_testData->useScopeObjects )
        {        
            CScopeReaderLockPtr scopeLock;
            try
            {
                // Since scope objects area really intended for lock/unlock as your code stack flows we simulate a code callstack
                // simply by having the scope locks in a collections which represents the callstack
                scopeLock = CScopeReaderLockPtr( GUCEF_NEW MT::CScopeReaderLock( m_testData->rwLock ) );
                m_scopeLocks.push_back( scopeLock );
            }
            catch ( const timeout_exception& )
            {
                m_rwLockFailed = true;
            }

            if ( !scopeLock->IsScopeReadLocked() )
            {
                m_rwLockFailed = true;
            }

            if ( !m_rwLockFailed )
            {
                ++m_currentReentrancyDepth;
            }
        }
        else
        {
            // We already hold the lock to be able to get here
            // however we will now make a reentrant lock request from the same thread
            m_lastRwLockOpResult = m_testData->rwLock.ReaderStart();
            if ( TRWLockStates::RWLOCK_OPERATION_SUCCESS != m_lastRwLockOpResult )
            {
                m_rwLockFailed = true;
            }
            else
            {
                ++m_currentReentrancyDepth;
            }
        }
    }

    m_ticksAtLastCycle = newTickCount;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CTestReader::OnThreadEnding( void* taskdata, bool willBeForced )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CTestWriter:OnThreadStart: reader worker ending" );
    
    if ( m_testData->useScopeObjects )    
    {
        if ( m_testData->readerWorkIsReentrant )
        {
            ASSERT_TRUE( m_currentReentrancyDepth+1 == m_scopeLocks.size() );
            
            try
            {
                // We could just clear the entire lock collection but we are doing it intentionally verbose here
                // intent is to simulate a stack unroll
                for ( MT::UInt32 i=0; i<m_currentReentrancyDepth; ++i )
                {
                    // We must unroll the reentrancy on this thread
                    // similar to rolling up in the call stack
                    CScopeReaderLockPtr scopeLock = m_scopeLocks.back();
                    m_scopeLocks.pop_back();

                    // We are supposedly leaving the callstack scope so we end the life of the scope object
                    ASSERT_TRUE( 1 == scopeLock.GetReferenceCount() );
                    scopeLock.Unlink();
                }
            
                
            }
            catch ( const timeout_exception& )
            {
                m_rwLockFailed = true;
            }
        }

        m_scopeLocks.clear();
    }
    else
    {
        if ( m_testData->readerWorkIsReentrant )
        {
            for ( MT::UInt32 i=0; i<m_currentReentrancyDepth; ++i )
            {
                // We must unroll the reentrancy on this thread
                // similar to rolling up in the call stack
                m_lastRwLockOpResult = m_testData->rwLock.ReaderStop();
                if ( TRWLockStates::RWLOCK_OPERATION_SUCCESS != m_lastRwLockOpResult )
                {
                    m_rwLockFailed = true;
                }
            } 
        }

        m_lastRwLockOpResult = m_testData->rwLock.ReaderStop();
        if ( TRWLockStates::RWLOCK_OPERATION_SUCCESS != m_lastRwLockOpResult )
            m_rwLockFailed = true;

    }
        
    m_onThreadEndingWasTriggered = true;
}
/*-------------------------------------------------------------------------*/

CTestReaderAndWriter::CTestReaderAndWriter( void )
    : MT::CActiveObject()
    , m_testData( GUCEF_NULL )
    , m_onThreadStartWasTriggered( false )
    , m_onThreadEndingWasTriggered( false )
    , m_lastRwLockOpResult( TRWLockStates::RWLOCK_OPERATION_FAILED )
    , m_rwLockFailed( false )
    , m_isDone( false )
    , m_ticksAtLastCycle( 0 )
    , m_ticksAtThreadStart( 0 )
    , m_currentReentrancyDepth( 0 )
    , m_scopeLocks()
    , m_depthIsWriterFlags()
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/

bool
CTestReaderAndWriter::WasOnThreadStartTriggered( void ) const
{GUCEF_TRACE;
        
    return m_onThreadStartWasTriggered;
}

/*-------------------------------------------------------------------------*/

bool
CTestReaderAndWriter::HadALockFailure( void ) const
{GUCEF_TRACE;
        
    return m_rwLockFailed;
}

/*-------------------------------------------------------------------------*/

CTestReaderAndWriter::TRWLockStates 
CTestReaderAndWriter::GetLastLockOperationResult( void ) const
{GUCEF_TRACE;
        
    return m_lastRwLockOpResult;
}

/*-------------------------------------------------------------------------*/

void 
CTestReaderAndWriter::SignalCompletionOfWork( void )
{GUCEF_TRACE;
        
    m_isDone = true;
}

/*-------------------------------------------------------------------------*/

MT::Float64 
CTestReaderAndWriter::GetTimeSinceThreadStartInMs( void ) const
{GUCEF_TRACE;
    
    return ( MT::PrecisionTickCount() - m_ticksAtThreadStart ) / m_testData->ticksPerMs;
}

/*-------------------------------------------------------------------------*/

MT::Float64 
CTestReaderAndWriter::GetCyclingDurationInMs( void ) const
{GUCEF_TRACE;
    
    return ( m_ticksAtLastCycle - m_ticksAtThreadStart ) / m_testData->ticksPerMs;
}

/*-------------------------------------------------------------------------*/

MT::UInt32 
CTestReaderAndWriter::GetCurrentReentrancyDepth( void ) const
{GUCEF_TRACE;
        
    return m_currentReentrancyDepth;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CTestReaderAndWriter::AsLockable( void ) const
{GUCEF_TRACE;
        
    return static_cast< const MT::CILockable* >( this );
}

/*-------------------------------------------------------------------------*/

bool
CTestReaderAndWriter::OnThreadStart( void* taskdata )
{GUCEF_TRACE;
        
    m_testData = static_cast< CRwLockTestData* >( taskdata );    
    m_ticksAtThreadStart = m_ticksAtLastCycle = MT::PrecisionTickCount();
    m_onThreadStartWasTriggered = true;
    
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CTestWriter:OnThreadStart: reader-writer worker starting" );
    
    if ( m_testData->useScopeObjects )
    {
        m_scopeLocks.reserve( m_testData->readerAndWriterReentrancyDepth );

        // Set up the flags to let us know if we should act as a reader or writer at each level of reentrancy
        // we alternate and we need to match as we simulate going up and down the stack        
        m_depthIsWriterFlags.reserve( m_testData->readerAndWriterReentrancyDepth );
        if ( m_testData->readerAndWriterReentrancyDepth > 0 )
        {
            m_depthIsWriterFlags.push_back( !m_testData->readerAndWriterUsesTopLevelReader );
            for ( UInt32 i=1; i<m_testData->readerAndWriterReentrancyDepth; ++i )
            {
                Float32 random = ( rand() / ( 1.0 * RAND_MAX ) );
                bool isWriter = random <= m_testData->readerAndWriterPercWrites;
                m_depthIsWriterFlags.push_back( isWriter );    
            }
        }
        
        if ( m_testData->readerAndWriterUsesTopLevelReader )
        {
            TReadWriteScopeLockPtrs scopeLockPtrs;
            try
            {
                // Since scope objects area really intended for lock/unlock as your code stack flows we simulate a code callstack
                // simply by having the scope locks in a collections which represents the callstack
                scopeLockPtrs.first = CScopeReaderLockPtr( GUCEF_NEW MT::CScopeReaderLock( m_testData->rwLock ) );
                m_scopeLocks.push_back( scopeLockPtrs );
            }
            catch ( const timeout_exception& )
            {
                m_rwLockFailed = true;
            }

            if ( !scopeLockPtrs.first->IsScopeReadLocked() )
            {
                m_rwLockFailed = true;
            }
        }
        else
        {
            TReadWriteScopeLockPtrs scopeLockPtrs;
            try
            {
                // Since scope objects area really intended for lock/unlock as your code stack flows we simulate a code callstack
                // simply by having the scope locks in a collections which represents the callstack
                scopeLockPtrs.second = CScopeWriterLockPtr( GUCEF_NEW MT::CScopeWriterLock( m_testData->rwLock ) );
                m_scopeLocks.push_back( scopeLockPtrs );
            }
            catch ( const timeout_exception& )
            {
                m_rwLockFailed = true;
            }

            if ( !scopeLockPtrs.second->IsScopeWriteLocked() )
            {
                m_rwLockFailed = true;
            }
        }
    }
    else
    {
        if ( m_testData->readerAndWriterUsesTopLevelReader )
        {
            m_lastRwLockOpResult = m_testData->rwLock.ReaderStart();
            if ( TRWLockStates::RWLOCK_OPERATION_SUCCESS != m_lastRwLockOpResult )
                m_rwLockFailed = true;
        }
        else
        {
            m_lastRwLockOpResult = m_testData->rwLock.WriterStart();
            if ( TRWLockStates::RWLOCK_OPERATION_SUCCESS != m_lastRwLockOpResult )
                m_rwLockFailed = true;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/
    
void
CTestReaderAndWriter::OnThreadStarted( void* taskdata ) 
{GUCEF_TRACE;
        
}

/*-------------------------------------------------------------------------*/

bool
CTestReaderAndWriter::OnThreadCycle( void* taskdata ) 
{GUCEF_TRACE;
        
    MT::UInt64 newTickCount = MT::PrecisionTickCount();
    MT::Float64 timeDeltaInMs = ( newTickCount - m_ticksAtThreadStart ) / m_testData->ticksPerMs;

    if ( timeDeltaInMs >= m_testData->readerAndWriterWorkDurationInMs )
        return true; // we are done

    if ( m_testData->readerAndWriterWorkIsReentrant && ( m_currentReentrancyDepth < m_testData->readerAndWriterReentrancyDepth ) )
    {
        // We already hold the lock to be able to get here
        // however we will now make a reentrant lock request from the same thread

        if ( m_testData->useScopeObjects )
        {        
            bool shouldActAsWriter = m_depthIsWriterFlags[ m_currentReentrancyDepth ];
            if ( shouldActAsWriter )
            {
                TReadWriteScopeLockPtrs scopeLockPtrs;
                try
                {
                    // Since scope objects area really intended for lock/unlock as your code stack flows we simulate a code callstack
                    // simply by having the scope locks in a collections which represents the callstack
                    scopeLockPtrs.second = CScopeWriterLockPtr( GUCEF_NEW MT::CScopeWriterLock( m_testData->rwLock ) );
                    m_scopeLocks.push_back( scopeLockPtrs );
                }
                catch ( const timeout_exception& )
                {
                    m_rwLockFailed = true;
                }

                if ( !scopeLockPtrs.second->IsScopeWriteLocked() )
                {
                    m_rwLockFailed = true;
                }
            }
            else
            {
                TReadWriteScopeLockPtrs scopeLockPtrs;
                try
                {
                    // Since scope objects area really intended for lock/unlock as your code stack flows we simulate a code callstack
                    // simply by having the scope locks in a collections which represents the callstack
                    scopeLockPtrs.first = CScopeReaderLockPtr( GUCEF_NEW MT::CScopeReaderLock( m_testData->rwLock ) );
                    m_scopeLocks.push_back( scopeLockPtrs );
                }
                catch ( const timeout_exception& )
                {
                    m_rwLockFailed = true;
                }

                if ( !scopeLockPtrs.first->IsScopeReadLocked() )
                {
                    m_rwLockFailed = true;
                }
            }

            if ( !m_rwLockFailed )
            {
                ++m_currentReentrancyDepth;
            }
        }
        else
        {
            m_lastRwLockOpResult = m_testData->rwLock.WriterStart();
            if ( TRWLockStates::RWLOCK_OPERATION_SUCCESS != m_lastRwLockOpResult )
            {
                m_rwLockFailed = true;
            }
            else
            {
                ++m_currentReentrancyDepth;
            }
        }
    }
    
    m_ticksAtLastCycle = newTickCount;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CTestReaderAndWriter::OnThreadEnding( void* taskdata, bool willBeForced )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CTestWriter:OnThreadStart: reader-writer worker ending" );
    
    if ( m_testData->useScopeObjects )    
    {
        if ( m_testData->readerAndWriterWorkIsReentrant )
        {
            ASSERT_TRUE( m_currentReentrancyDepth+1 == m_scopeLocks.size() );
            
            try
            {
                // We could just clear the entire lock collection but we are doing it intentionally verbose here
                // intent is to simulate a stack unroll
                for ( MT::UInt32 i=0; i<m_currentReentrancyDepth; ++i )
                {
                    // We must unroll the reentrancy on this thread
                    // similar to rolling up in the call stack
                    TReadWriteScopeLockPtrs scopeLocks = m_scopeLocks.back();
                    m_scopeLocks.pop_back();

                    // We are supposedly leaving the callstack scope so we end the life of the scope object
                    if ( scopeLocks.first.IsNULL() )
                    {
                        ASSERT_TRUE( 1 == scopeLocks.second.GetReferenceCount() );
                        scopeLocks.second.Unlink();
                    }
                    else
                    {
                        ASSERT_TRUE( 1 == scopeLocks.first.GetReferenceCount() );
                        scopeLocks.first.Unlink();
                    }
                }
            }
            catch ( const timeout_exception& )
            {
                m_rwLockFailed = true;
            }
        }

        m_scopeLocks.clear();
    }
    else
    {
        if ( m_testData->readerAndWriterWorkIsReentrant )
        {
            for ( MT::UInt32 i=0; i<m_currentReentrancyDepth; ++i )
            {
                // We must unroll the reentrancy on this thread
                // similar to rolling up in the call stack
                m_lastRwLockOpResult = m_testData->rwLock.WriterStop();
                if ( TRWLockStates::RWLOCK_OPERATION_SUCCESS != m_lastRwLockOpResult )
                {
                    m_rwLockFailed = true;
                }
            }
        }

        m_lastRwLockOpResult = m_testData->rwLock.WriterStop();
        if ( TRWLockStates::RWLOCK_OPERATION_SUCCESS != m_lastRwLockOpResult )
        {
            m_rwLockFailed = true;
        }
    }

    m_onThreadEndingWasTriggered = true;
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockSingleThreadTests_InitTest( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // start with a basic sanity check on the init state
        ASSERT_TRUE( GUCEF_NULL != testData.rwLock.AsLockable() );
        ASSERT_TRUE( 0 == testData.rwLock.ActiveReaderCount() );
        ASSERT_TRUE( 0 == testData.rwLock.ActiveWriterCount() );
        ASSERT_TRUE( 0 == testData.rwLock.QueuedReaderCount() );
        ASSERT_TRUE( 0 == testData.rwLock.QueuedWriterCount() );
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );

        // first just sanity check the test setup code
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 0;
        testData.writerWorkDurationInMs = 100;
        testData.readerWorkDurationInMs = 100;
        ASSERT_TRUE( testData.Configure() );
        ASSERT_TRUE( GUCEF_NULL != testData.rwLock.AsLockable() );
        ASSERT_TRUE( 0 == testData.rwLock.ActiveReaderCount() );
        ASSERT_TRUE( 0 == testData.rwLock.ActiveWriterCount() );
        ASSERT_TRUE( 0 == testData.rwLock.QueuedReaderCount() );
        ASSERT_TRUE( 0 == testData.rwLock.QueuedWriterCount() );
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        ASSERT_TRUE( testData.Shutdown() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockSingleThreadTests_SingleReader( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a singular reader (this thread)
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 0;
        testData.writerWorkDurationInMs = 100;
        testData.readerWorkDurationInMs = 100;
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStart() );
        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        ASSERT_TRUE( 1 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockSingleThreadTests_SingleWriter( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a singular writer (this thread)
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 0;
        testData.writerWorkDurationInMs = 100;
        testData.readerWorkDurationInMs = 100;
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStart() );
        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 1 == activeWriterCount );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStop() );
        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockSingleThreadTests_SingleReaderReentrant( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a singular re-entrant reader (this thread)
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 0;
        testData.writerWorkDurationInMs = 100;
        testData.readerWorkDurationInMs = 100;        
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
        UInt32 reentrancyDepth = 5;
        for ( UInt32 i=0; i<reentrancyDepth; ++i )
        {
            // note that the first pass is not re-entrant

            ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStart() );
            activeReaderCount = testData.rwLock.ActiveReaderCount();
            activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
            activeWriterCount = testData.rwLock.ActiveWriterCount();
            queuedReaderCount = testData.rwLock.QueuedReaderCount();
            queuedWriterCount = testData.rwLock.QueuedWriterCount();
            ASSERT_TRUE( 1 == activeReaderCount );
            ASSERT_TRUE( i == activeReaderReentrantCount );
            ASSERT_TRUE( 0 == activeWriterCount );
            ASSERT_TRUE( 0 == queuedReaderCount );
            ASSERT_TRUE( 0 == queuedWriterCount );
        }
        for ( UInt32 i=0; i<reentrancyDepth; ++i )
        {
            ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
            activeReaderCount = testData.rwLock.ActiveReaderCount();
            activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
            activeWriterCount = testData.rwLock.ActiveWriterCount();
            queuedReaderCount = testData.rwLock.QueuedReaderCount();
            queuedWriterCount = testData.rwLock.QueuedWriterCount();
            if ( i+2 == reentrancyDepth )
            {
                // last one, no more lock
                ASSERT_TRUE( 0 == activeReaderReentrantCount );
                ASSERT_TRUE( 1 == activeReaderCount );
            }
            else
            if ( i+1 == reentrancyDepth )
            {
                // last one, no more lock
                ASSERT_TRUE( 0 == activeReaderReentrantCount );
                ASSERT_TRUE( 0 == activeReaderCount );
            }
            else
            {
                ASSERT_TRUE( reentrancyDepth-(i+2) == activeReaderReentrantCount );
                ASSERT_TRUE( 1 == activeReaderCount );
            }
            ASSERT_TRUE( 0 == activeWriterCount );
            ASSERT_TRUE( 0 == queuedReaderCount );
            ASSERT_TRUE( 0 == queuedWriterCount );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockSingleThreadTests_SingleWriterReentrant( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a singular re-entrant writer (this thread)
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 0;
        testData.writerWorkDurationInMs = 100;
        testData.readerWorkDurationInMs = 100;        
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        UInt32 reentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == reentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
        UInt32 desiredReentrancyDepth = 5;
        for ( UInt32 i=0; i<=desiredReentrancyDepth; ++i )
        {
            ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStart() );
            activeReaderCount = testData.rwLock.ActiveReaderCount();
            activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
            activeWriterCount = testData.rwLock.ActiveWriterCount();
            queuedReaderCount = testData.rwLock.QueuedReaderCount();
            queuedWriterCount = testData.rwLock.QueuedWriterCount();
            reentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth(); 
            ASSERT_TRUE( 0 == activeReaderCount );
            ASSERT_TRUE( 0 == activeReaderReentrantCount );
            ASSERT_TRUE( 1 == activeWriterCount );
            ASSERT_TRUE( reentrancyDepth == i );
            ASSERT_TRUE( 0 == queuedReaderCount );
            ASSERT_TRUE( 0 == queuedWriterCount );
        }
        for ( UInt32 i=0; i<=desiredReentrancyDepth; ++i )
        {
            ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStop() );
            activeReaderCount = testData.rwLock.ActiveReaderCount();
            activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
            activeWriterCount = testData.rwLock.ActiveWriterCount();
            queuedReaderCount = testData.rwLock.QueuedReaderCount();
            queuedWriterCount = testData.rwLock.QueuedWriterCount();
            reentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth(); 
            ASSERT_TRUE( 0 == activeReaderCount );
            ASSERT_TRUE( 0 == activeReaderReentrantCount );
            if ( i < desiredReentrancyDepth )
            {
                ASSERT_TRUE( 1 == activeWriterCount );
                ASSERT_TRUE( reentrancyDepth == desiredReentrancyDepth-(i+1) );
            }
            else
            {
                ASSERT_TRUE( 0 == activeWriterCount );
                ASSERT_TRUE( 0 == reentrancyDepth );
            }
            ASSERT_TRUE( 0 == queuedReaderCount );
            ASSERT_TRUE( 0 == queuedWriterCount );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
}


/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockSingleThreadTests_SingleReaderWriterReentrant( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    typedef std::vector< bool >     TBoolVector;
    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a singular re-entrant writer (this thread)
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 0;
        testData.nrOfReaderAndWriters = 0;
        testData.writerWorkDurationInMs = 100;
        testData.readerWorkDurationInMs = 100;  
        testData.readerAndWriterWorkDurationInMs = 100;

        testData.readerAndWriterReentrancyDepth = 50;

        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        UInt32 activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        // Set up the flags to let us know if we should act as a reader or writer at each level of reentrancy
        // we alternate and we need to match as we simulate going up and down the stack
        TBoolVector depthIsWriterFlags;
        depthIsWriterFlags.reserve( testData.readerAndWriterReentrancyDepth+1 );
        if ( testData.readerAndWriterReentrancyDepth > 0 )
        {
            depthIsWriterFlags.push_back( !testData.readerAndWriterUsesTopLevelReader );
            for ( UInt32 i=0; i<testData.readerAndWriterReentrancyDepth; ++i )
            {
                Float32 random = ( rand() / ( 1.0 * RAND_MAX ) );
                bool isWriter = random <= testData.readerAndWriterPercWrites;
                depthIsWriterFlags.push_back( isWriter );    
            }
        }

        UInt32 writeCount = 0;
        UInt32 readCount = 0;
        for ( UInt32 i=0; i<=testData.readerAndWriterReentrancyDepth; ++i )
        {
            bool actAsWriter = depthIsWriterFlags[ i ];
            if ( actAsWriter )
            {
                ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStart() );
                ++writeCount;
            }
            else
            {
                ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStart() );
                ++readCount;
            }
            activeReaderCount = testData.rwLock.ActiveReaderCount();
            activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
            activeWriterCount = testData.rwLock.ActiveWriterCount();
            queuedReaderCount = testData.rwLock.QueuedReaderCount();
            queuedWriterCount = testData.rwLock.QueuedWriterCount();
            activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
            if ( readCount > 0 )
            {
                ASSERT_TRUE( 1 == activeReaderCount );
                ASSERT_TRUE( activeReaderReentrantCount == readCount-1 );
            }
            if ( writeCount > 0 )
            {
                ASSERT_TRUE( 1 == activeWriterCount );
                ASSERT_TRUE( activeWriterReentrancyDepth == writeCount-1 );
            }
            ASSERT_TRUE( 0 == queuedReaderCount );
            ASSERT_TRUE( 0 == queuedWriterCount );
        }


        for ( Int32 i=testData.readerAndWriterReentrancyDepth; i>=0; --i )
        {
            bool actAsWriter = depthIsWriterFlags[ i ];
            if ( actAsWriter )
            {
                ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStop() );
            }
            else
            {
                ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
            }
            activeReaderCount = testData.rwLock.ActiveReaderCount();
            activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
            activeWriterCount = testData.rwLock.ActiveWriterCount();
            queuedReaderCount = testData.rwLock.QueuedReaderCount();
            queuedWriterCount = testData.rwLock.QueuedWriterCount();
            activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth(); 

            if ( actAsWriter )
                --writeCount;
            else
                --readCount;

            if ( readCount > 0 )
            {
                ASSERT_TRUE( 1 == activeReaderCount );
                ASSERT_TRUE( activeReaderReentrantCount == (readCount-1) );
            }
            else
            {
                ASSERT_TRUE( 0 == activeReaderCount );
                ASSERT_TRUE( 0 == activeReaderReentrantCount );
            }
            if ( writeCount > 0 )
            {
                ASSERT_TRUE( 1 == activeWriterCount );
                ASSERT_TRUE( activeWriterReentrancyDepth == (writeCount-1) );
            }
            else
            {
                ASSERT_TRUE( 0 == activeWriterCount );
                ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
            }
            ASSERT_TRUE( 0 == queuedReaderCount );
            ASSERT_TRUE( 0 == queuedWriterCount );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockSingleThreadTests_SingleReaderWriterExplicitTransition( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a singular re-entrant writer (this thread)
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 0;
        testData.nrOfReaderAndWriters = 0;
        testData.writerWorkDurationInMs = 100;
        testData.readerWorkDurationInMs = 100;  
        testData.readerAndWriterWorkDurationInMs = 100;

        testData.readerAndWriterReentrancyDepth = 50;
        testData.readerAndWriterPercWrites = 0.5; // hardcoded below as constantly alternating

        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        UInt32 activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        // check if sanity checks work
        // we dont have a lock and as such should not be able to transition
        // cannot run in debug builds since the rw lock code itself will hit an assert
        #ifndef GUCEF_DEBUG_MODE
        ASSERT_FALSE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );
        ASSERT_FALSE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionReaderToWriter() );

        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
        #endif

        UInt32 writeCount = 0;
        UInt32 readCount = 0;
        bool actAsWriter = !testData.readerAndWriterUsesTopLevelReader;
        if ( actAsWriter )
        {
            ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStart() );
            ++writeCount;
        }
        else
        {
            ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStart() );
            ++readCount;
        }

        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( ( actAsWriter ? 0 : 1 ) == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( ( actAsWriter ? 1 : 0 ) == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        for ( UInt32 i=1; i<=testData.readerAndWriterReentrancyDepth; ++i )
        {
            bool lastWasWriter = actAsWriter;
            actAsWriter = !lastWasWriter;

            if ( lastWasWriter && !actAsWriter )
            {
                // we are a writer becoming a reader
                ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );
                --writeCount;
                ++readCount;
            }
            else
            if ( !lastWasWriter && actAsWriter )
            {
                // we are a reader becoming a writer
                ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionReaderToWriter() );
                --readCount;
                ++writeCount;
            }

            activeReaderCount = testData.rwLock.ActiveReaderCount();
            activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
            activeWriterCount = testData.rwLock.ActiveWriterCount();
            queuedReaderCount = testData.rwLock.QueuedReaderCount();
            queuedWriterCount = testData.rwLock.QueuedWriterCount();
            activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
            if ( readCount > 0 )
            {
                ASSERT_TRUE( 1 == activeReaderCount );
                ASSERT_TRUE( activeReaderReentrantCount == readCount-1 );
            }
            if ( writeCount > 0 )
            {
                ASSERT_TRUE( 1 == activeWriterCount );
                ASSERT_TRUE( activeWriterReentrancyDepth == writeCount-1 );
            }
            ASSERT_TRUE( 0 == queuedReaderCount );
            ASSERT_TRUE( 0 == queuedWriterCount );
        }

        // Since we constantly tranistioned back and forth we should zero out to after undoing the lock once
        // there should be no reentrancy in this case
        
        bool lastWasWriter = actAsWriter;
        if ( lastWasWriter )
        {
            ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStop() );
        }
        else
        {
            ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
        }

        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth(); 

        if ( lastWasWriter )
            --writeCount;
        else
            --readCount;

        ASSERT_TRUE( 0 == writeCount );
        ASSERT_TRUE( 0 == readCount );
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockSingleThreadTests_SingleReaderWriterExplicitTransitionReentrant( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a singular re-entrant reader & writer (this thread)
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 0;
        testData.nrOfReaderAndWriters = 0;
        testData.writerWorkDurationInMs = 100;
        testData.readerWorkDurationInMs = 100;  
        testData.readerAndWriterWorkDurationInMs = 100;
        testData.readerAndWriterReentrancyDepth = 50;
        testData.readerAndWriterPercWrites = 0.5;

        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        UInt32 activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStart() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStart() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStart() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStart() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStart() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStart() );
        
        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 1 == activeWriterCount );
        ASSERT_TRUE( 5 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStart() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStart() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStart() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStart() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStart() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStart() );

        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 1 == activeReaderCount );
        ASSERT_TRUE( 5 == activeReaderReentrantCount );
        ASSERT_TRUE( 1 == activeWriterCount );
        ASSERT_TRUE( 5 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );

        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 1 == activeReaderCount );
        ASSERT_TRUE( 8 == activeReaderReentrantCount );
        ASSERT_TRUE( 1 == activeWriterCount );
        ASSERT_TRUE( 2 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionReaderToWriter() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionReaderToWriter() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionReaderToWriter() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionReaderToWriter() );

        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 1 == activeReaderCount );
        ASSERT_TRUE( 4 == activeReaderReentrantCount );
        ASSERT_TRUE( 1 == activeWriterCount );
        ASSERT_TRUE( 6 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionReaderToWriter() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionReaderToWriter() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionReaderToWriter() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionReaderToWriter() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionReaderToWriter() );

        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 1 == activeWriterCount );
        ASSERT_TRUE( 11 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );

        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 1 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 1 == activeWriterCount );
        ASSERT_TRUE( 10 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );

        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 1 == activeReaderCount );
        ASSERT_TRUE( 9 == activeReaderReentrantCount );
        ASSERT_TRUE( 1 == activeWriterCount );
        ASSERT_TRUE( 1 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );

        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 1 == activeReaderCount );
        ASSERT_TRUE( 10 == activeReaderReentrantCount );
        ASSERT_TRUE( 1 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );

        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 1 == activeReaderCount );
        ASSERT_TRUE( 11 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );

        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 1 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );

        activeReaderCount = testData.rwLock.ActiveReaderCount();
        activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        activeWriterCount = testData.rwLock.ActiveWriterCount();
        queuedReaderCount = testData.rwLock.QueuedReaderCount();
        queuedWriterCount = testData.rwLock.QueuedWriterCount();
        activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockSingleThreadTests_SingleReaderWriterExplicitTransitionReentrantRandom( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    typedef std::vector< bool >     TBoolVector;
    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a singular re-entrant writer (this thread)
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 0;
        testData.nrOfReaderAndWriters = 0;
        testData.writerWorkDurationInMs = 100;
        testData.readerWorkDurationInMs = 100;  
        testData.readerAndWriterWorkDurationInMs = 100;

        testData.readerAndWriterReentrancyDepth = 50;
        testData.readerAndWriterPercWrites = 0.5;

        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        UInt32 activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 0 == activeReaderReentrantCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );

        // Set up the flags to let us know if we should act as a reader or writer at each level of reentrancy
        // we alternate and we need to match as we simulate going up and down the stack
        TBoolVector depthIsWriterFlags;
        depthIsWriterFlags.reserve( testData.readerAndWriterReentrancyDepth+1 );
        if ( testData.readerAndWriterReentrancyDepth > 0 )
        {
            depthIsWriterFlags.push_back( !testData.readerAndWriterUsesTopLevelReader );
            for ( UInt32 i=0; i<testData.readerAndWriterReentrancyDepth; ++i )
            {
                Float32 random = ( rand() / ( 1.0 * RAND_MAX ) );
                bool isWriter = random <= testData.readerAndWriterPercWrites;
                depthIsWriterFlags.push_back( isWriter );    
            }
        }

        UInt32 writeCount = 0;
        UInt32 readCount = 0;
        bool actAsWriter = depthIsWriterFlags[ 0 ];
        if ( actAsWriter )
        {
            ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStart() );
            ++writeCount;
        }
        else
        {
            ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStart() );
            ++readCount;
        }

        for ( UInt32 i=1; i<=testData.readerAndWriterReentrancyDepth; ++i )
        {
            bool lastWasWriter = actAsWriter;
            actAsWriter = depthIsWriterFlags[ i ];

            if ( lastWasWriter && actAsWriter )
            {
                // just a reentrant write
                ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStart() );
                ++writeCount;
            }
            else
            if ( !lastWasWriter && !actAsWriter )
            {
                // just a reentrant read
                ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStart() );
                ++readCount;
            }
            else
            if ( lastWasWriter && !actAsWriter )
            {
                // we are a writer becoming a reader
                ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionWriterToReader() );
                --writeCount;
                ++readCount;
            }
            else
            if ( !lastWasWriter && actAsWriter )
            {
                // we are a reader becoming a writer
                ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.TransitionReaderToWriter() );
                --readCount;
                ++writeCount;
            }

            activeReaderCount = testData.rwLock.ActiveReaderCount();
            activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
            activeWriterCount = testData.rwLock.ActiveWriterCount();
            queuedReaderCount = testData.rwLock.QueuedReaderCount();
            queuedWriterCount = testData.rwLock.QueuedWriterCount();
            activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
            if ( readCount > 0 )
            {
                ASSERT_TRUE( 1 == activeReaderCount );
                ASSERT_TRUE( activeReaderReentrantCount == readCount-1 );
            }
            if ( writeCount > 0 )
            {
                ASSERT_TRUE( 1 == activeWriterCount );
                ASSERT_TRUE( activeWriterReentrancyDepth == writeCount-1 );
            }
            ASSERT_TRUE( 0 == queuedReaderCount );
            ASSERT_TRUE( 0 == queuedWriterCount );
        }


        for ( Int32 i=testData.readerAndWriterReentrancyDepth; i>=0; --i )
        {
            bool actAsWriter = depthIsWriterFlags[ i ];
            if ( actAsWriter )
            {
                if ( writeCount > 0 )
                {
                    ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.WriterStop() );
                    --writeCount;
                }
            }
            else
            {
                if ( readCount > 0 )
                {
                    ASSERT_TRUE( MT::CReadWriteLock::TRWLockStates::RWLOCK_OPERATION_SUCCESS == testData.rwLock.ReaderStop() );
                    --readCount;
                }
            }
            activeReaderCount = testData.rwLock.ActiveReaderCount();
            activeReaderReentrantCount = testData.rwLock.ActiveReaderReentrancyDepth( MT::GetCurrentTaskID() );
            activeWriterCount = testData.rwLock.ActiveWriterCount();
            queuedReaderCount = testData.rwLock.QueuedReaderCount();
            queuedWriterCount = testData.rwLock.QueuedWriterCount();
            activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth(); 

            if ( readCount > 0 )
            {
                ASSERT_TRUE( 1 == activeReaderCount );
                if ( readCount > 1 )
                {
                    ASSERT_TRUE( activeReaderReentrantCount == (readCount-1) );
                }
                else
                {
                    ASSERT_TRUE( activeReaderReentrantCount == 0 );
                }
            }
            else
            {
                ASSERT_TRUE( 0 == activeReaderCount );
                ASSERT_TRUE( 0 == activeReaderReentrantCount );
            }
            if ( writeCount > 0 )
            {
                ASSERT_TRUE( 1 == activeWriterCount );
                if ( writeCount > 1 )
                {
                    ASSERT_TRUE( activeWriterReentrancyDepth == (writeCount-1) );
                }
                else
                {
                    ASSERT_TRUE( activeWriterReentrancyDepth == 0 );
                }
            }
            else
            {
                ASSERT_TRUE( 0 == activeWriterCount );
                ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
            }
            ASSERT_TRUE( 0 == queuedReaderCount );
            ASSERT_TRUE( 0 == queuedWriterCount );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockTests_SingleReader( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a singular reader
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 1;
        testData.writerReentrancyDepth = 0;
        testData.writerWorkIsReentrant = false;
        testData.readerReentrancyDepth = 0;
        testData.readerWorkIsReentrant = false;
        testData.writerWorkDurationInMs = 100;
        testData.readerWorkDurationInMs = GUCEF_UINT32MAX;
        ASSERT_TRUE( testData.Configure() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        ASSERT_TRUE( 1 == activeReaderCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );        
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        ASSERT_TRUE( false == testData.DidAnyReaderHaveALockFailure() );
        ASSERT_TRUE( false == testData.DidAnyWriterHaveALockFailure() );
        ASSERT_TRUE( testData.Shutdown() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockTests_SingleWriter( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a singular writer
        testData.nrOfWriters = 1;
        testData.nrOfReaders = 0;
        testData.writerReentrancyDepth = 0;
        testData.writerWorkIsReentrant = false;
        testData.readerReentrancyDepth = 0;
        testData.readerWorkIsReentrant = false;
        testData.writerWorkDurationInMs = GUCEF_UINT32MAX;
        testData.readerWorkDurationInMs = 100;
        ASSERT_TRUE( testData.Configure() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 1 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        ASSERT_TRUE( false == testData.DidAnyReaderHaveALockFailure() );
        ASSERT_TRUE( false == testData.DidAnyWriterHaveALockFailure() );
        ASSERT_TRUE( testData.Shutdown() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockTests_MultipleReaders( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a multiple readers all starting +- at the same time
        // they will keep running
        // Since we only have readers they should all end up running in parallel
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 50;
        testData.writerReentrancyDepth = 0;
        testData.writerWorkIsReentrant = false;
        testData.readerReentrancyDepth = 0;
        testData.readerWorkIsReentrant = false;
        testData.writerWorkDurationInMs = GUCEF_UINT32MAX;
        testData.readerWorkDurationInMs = GUCEF_UINT32MAX;
        ASSERT_TRUE( testData.Configure() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        ASSERT_TRUE( 50 == activeReaderCount );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        ASSERT_TRUE( false == testData.DidAnyReaderHaveALockFailure() );
        ASSERT_TRUE( false == testData.DidAnyWriterHaveALockFailure() );
        ASSERT_TRUE( testData.Shutdown() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockTests_MultipleWriters( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a multiple writers all starting +- at the same time
        // they will keep running. Since these are different writers only 1 should be allowed active
        // the rest should queue up
        testData.nrOfWriters = 50;
        testData.nrOfReaders = 0;
        testData.writerReentrancyDepth = 0;
        testData.writerWorkIsReentrant = false;
        testData.readerReentrancyDepth = 0;
        testData.readerWorkIsReentrant = false;
        testData.writerWorkDurationInMs = GUCEF_UINT32MAX;
        testData.readerWorkDurationInMs = GUCEF_UINT32MAX;
        ASSERT_TRUE( testData.Configure() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 1 == activeWriterCount );
        ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( testData.nrOfWriters-1 == queuedWriterCount );
        MT::PrecisionDelay( GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS * 2 );
        UInt32 cyclingWritersFound = 0;
        for ( UInt32 i=0; i<testData.nrOfWriters; ++i )
        {
            MT::Float64 cyclingDurationInMs = testData.writers[ i ].GetCyclingDurationInMs();
            if ( cyclingDurationInMs > GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS )
            {
                ++cyclingWritersFound;
            }
        }
        ASSERT_TRUE( 1 == cyclingWritersFound );
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        ASSERT_TRUE( false == testData.DidAnyReaderHaveALockFailure() );
        ASSERT_TRUE( false == testData.DidAnyWriterHaveALockFailure() );
        ASSERT_TRUE( testData.Shutdown() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockTests_MultipleReaderWritersReentrant( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a multiple writers all starting +- at the same time
        // they will keep running. Since these are different writers only 1 should be allowed active
        // the rest should queue up. We now also make the active writer reentrant
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 0;
        testData.nrOfReaderAndWriters = 50;
        testData.readerAndWriterPercWrites = 0.25;
        testData.readerAndWriterUsesTopLevelReader = false; // <- top lock will always be a write lock
        testData.writerReentrancyDepth = 0;
        testData.writerWorkIsReentrant = false;
        testData.readerReentrancyDepth = 0;
        testData.readerWorkIsReentrant = false;
        testData.readerAndWriterWorkIsReentrant = true;
        testData.readerAndWriterReentrancyDepth = 10;
        testData.writerWorkDurationInMs = GUCEF_UINT32MAX;
        testData.readerWorkDurationInMs = GUCEF_UINT32MAX;
        testData.readerAndWriterWorkDurationInMs = GUCEF_UINT32MAX;
        ASSERT_TRUE( testData.Configure() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        //ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 1 == activeWriterCount );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( queuedWriterCount > 0 );
        MT::PrecisionDelay( GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS * 2 );
        UInt32 cyclingReaderWritersFound = 0;
        UInt32 cyclingReaderWriterIndex = 0;
        for ( UInt32 i=0; i<testData.nrOfReaderAndWriters; ++i )
        {
            MT::Float64 cyclingDurationInMs = testData.readerAndWriters[ i ].GetCyclingDurationInMs();
            if ( cyclingDurationInMs > GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS )
            {
                ++cyclingReaderWritersFound;
                cyclingReaderWriterIndex = i;
            }
        }
        ASSERT_TRUE( 1 == cyclingReaderWritersFound );
        UInt32 activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( activeWriterReentrancyDepth > 0 );        
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        ASSERT_TRUE( false == testData.DidAnyReaderHaveALockFailure() );
        ASSERT_TRUE( false == testData.DidAnyWriterHaveALockFailure() );
        ASSERT_TRUE( false == testData.DidAnyReaderWriterHaveALockFailure() );
        ASSERT_TRUE( testData.readerAndWriterReentrancyDepth == testData.readerAndWriters[ cyclingReaderWriterIndex ].GetCurrentReentrancyDepth() );
        ASSERT_TRUE( testData.Shutdown() );
    }
    catch( ... )
    {
        ERRORHERE;
    }

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a multiple writers all starting +- at the same time
        // they will keep running. Since these are different writers only 1 should be allowed active
        // the rest should queue up. We now also make the active writer reentrant
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 0;
        testData.nrOfReaderAndWriters = 50;
        testData.readerAndWriterPercWrites = 0.25;
        testData.readerAndWriterUsesTopLevelReader = false; // <- top lock will always be a read lock
        testData.writerReentrancyDepth = 0;
        testData.writerWorkIsReentrant = false;
        testData.readerReentrancyDepth = 0;
        testData.readerWorkIsReentrant = false;
        testData.readerAndWriterWorkIsReentrant = true;
        testData.readerAndWriterReentrancyDepth = 10;
        testData.writerWorkDurationInMs = GUCEF_UINT32MAX;
        testData.readerWorkDurationInMs = GUCEF_UINT32MAX;
        testData.readerAndWriterWorkDurationInMs = GUCEF_UINT32MAX;
        ASSERT_TRUE( testData.Configure() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        //ASSERT_TRUE( 0 == activeReaderCount );
        //ASSERT_TRUE( 1 == activeWriterCount || 0 == activeWriterCount );
        //ASSERT_TRUE( 0 == queuedReaderCount );
        //ASSERT_TRUE( queuedWriterCount > 0 );
        MT::PrecisionDelay( GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS * 2 );
        UInt32 cyclingReaderWritersFound = 0;
        UInt32 cyclingReaderWriterIndex = 0;
        for ( UInt32 i=0; i<testData.nrOfReaderAndWriters; ++i )
        {
            MT::Float64 cyclingDurationInMs = testData.readerAndWriters[ i ].GetCyclingDurationInMs();
            if ( cyclingDurationInMs > GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS )
            {
                ++cyclingReaderWritersFound;
                cyclingReaderWriterIndex = i;
            }
        }
        ASSERT_TRUE( 1 == cyclingReaderWritersFound );
        UInt32 activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( activeWriterReentrancyDepth > 0 );        
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        ASSERT_TRUE( false == testData.DidAnyReaderHaveALockFailure() );
        ASSERT_TRUE( false == testData.DidAnyWriterHaveALockFailure() );
        ASSERT_TRUE( false == testData.DidAnyReaderWriterHaveALockFailure() );
        ASSERT_TRUE( testData.readerAndWriterReentrancyDepth == testData.readerAndWriters[ cyclingReaderWriterIndex ].GetCurrentReentrancyDepth() );
        ASSERT_TRUE( testData.Shutdown() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockTests_MultipleWritersReentrant( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a multiple writers all starting +- at the same time
        // they will keep running. Since these are different writers only 1 should be allowed active
        // the rest should queue up. We now also make the active writer reentrant
        testData.nrOfWriters = 50;
        testData.nrOfReaders = 0;
        testData.writerReentrancyDepth = 10;
        testData.writerWorkIsReentrant = true;
        testData.readerReentrancyDepth = 0;
        testData.readerWorkIsReentrant = false;
        testData.writerWorkDurationInMs = GUCEF_UINT32MAX;
        testData.readerWorkDurationInMs = GUCEF_UINT32MAX;
        ASSERT_TRUE( testData.Configure() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        ASSERT_TRUE( 0 == activeReaderCount );
        ASSERT_TRUE( 1 == activeWriterCount );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( testData.nrOfWriters-1 == queuedWriterCount );
        MT::PrecisionDelay( GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS * 2 );
        UInt32 cyclingWritersFound = 0;
        UInt32 cyclingWriterIndex = 0;
        for ( UInt32 i=0; i<testData.nrOfWriters; ++i )
        {
            MT::Float64 cyclingDurationInMs = testData.writers[ i ].GetCyclingDurationInMs();
            if ( cyclingDurationInMs > GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS )
            {
                ++cyclingWritersFound;
                cyclingWriterIndex = i;
            }
        }
        ASSERT_TRUE( 1 == cyclingWritersFound );
        UInt32 activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
        ASSERT_TRUE( testData.writerReentrancyDepth == testData.rwLock.ActiveWriterReentrancyDepth() );        
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        ASSERT_TRUE( false == testData.DidAnyReaderHaveALockFailure() );
        ASSERT_TRUE( false == testData.DidAnyWriterHaveALockFailure() );
        ASSERT_TRUE( testData.writerReentrancyDepth == testData.writers[ cyclingWriterIndex ].GetCurrentReentrancyDepth() );
        ASSERT_TRUE( testData.Shutdown() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockTests_MultipleReadersReentrant( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        CRwLockTestData testData( writersHavePriority, usingScopeObjects );

        // test with a multiple writers all starting +- at the same time
        // they will keep running. Since these are different writers only 1 should be allowed active
        // the rest should queue up. We now also make the active writer reentrant
        testData.nrOfWriters = 0;
        testData.nrOfReaders = 50;
        testData.writerReentrancyDepth = 0;
        testData.writerWorkIsReentrant = false;
        testData.readerReentrancyDepth = 10;
        testData.readerWorkIsReentrant = true;
        testData.writerWorkDurationInMs = GUCEF_UINT32MAX;
        testData.readerWorkDurationInMs = GUCEF_UINT32MAX;
        ASSERT_TRUE( testData.Configure() );
        UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
        UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
        UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
        UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
        ASSERT_TRUE( activeReaderCount >= testData.nrOfReaders );
        ASSERT_TRUE( 0 == activeWriterCount );
        ASSERT_TRUE( 0 == queuedReaderCount );
        ASSERT_TRUE( 0 == queuedWriterCount );
        MT::PrecisionDelay( GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS * 2 );
        UInt32 cyclingReadersFound = 0;
        for ( UInt32 i=0; i<testData.nrOfReaders; ++i )
        {
            MT::Float64 cyclingDurationInMs = testData.readers[ i ].GetCyclingDurationInMs();
            if ( cyclingDurationInMs > GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS )
            {
                ++cyclingReadersFound;
                UInt32 reentrancyDepth = testData.readers[ i ].GetCurrentReentrancyDepth();
                ASSERT_TRUE( testData.readerReentrancyDepth == reentrancyDepth);
            }
        }
        ASSERT_TRUE( testData.nrOfReaders == cyclingReadersFound );        
        ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
        ASSERT_TRUE( false == testData.DidAnyReaderHaveALockFailure() );
        ASSERT_TRUE( false == testData.DidAnyWriterHaveALockFailure() );
        ASSERT_TRUE( testData.Shutdown() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockTests_MultipleWritersMultipleReaders( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        bool hadATestRunWithActiveWriters = false;
        bool hadATestRunWithActiveReaders = false;

        do 
        {
            CRwLockTestData testData( writersHavePriority, usingScopeObjects );

            // test with a multiple readers and writers all starting +- at the same time
            // this test is heavily reliant on timing and as such we can only really check that the tallies make sense
            testData.nrOfWriters = 5;
            testData.nrOfReaders = 25;
            testData.writerReentrancyDepth = 0;
            testData.writerWorkIsReentrant = false;
            testData.readerReentrancyDepth = 0;
            testData.readerWorkIsReentrant = false;
            testData.writerWorkDurationInMs = GUCEF_UINT32MAX;
            testData.readerWorkDurationInMs = GUCEF_UINT32MAX;
            ASSERT_TRUE( testData.Configure() );
            UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
            UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
            UInt32 activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
            UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
            UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
            ASSERT_TRUE( testData.nrOfReaders == ( activeReaderCount + queuedReaderCount ) );
            ASSERT_TRUE( testData.nrOfWriters == ( activeWriterCount + queuedWriterCount ) );
            ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
            MT::PrecisionDelay( GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS * 2 );
            UInt32 cyclingWritersFound = 0;
            for ( UInt32 i=0; i<testData.nrOfWriters; ++i )
            {
                MT::Float64 cyclingDurationInMs = testData.writers[ i ].GetCyclingDurationInMs();
                if ( cyclingDurationInMs > GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS )
                {
                    ++cyclingWritersFound;
                }
            }
            UInt32 cyclingReadersFound = 0;
            for ( UInt32 i=0; i<testData.nrOfReaders; ++i )
            {
                MT::Float64 cyclingDurationInMs = testData.readers[ i ].GetCyclingDurationInMs();
                if ( cyclingDurationInMs > GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS )
                {
                    ++cyclingReadersFound;
                }
            }
            ASSERT_TRUE( activeWriterCount == cyclingWritersFound );
            ASSERT_TRUE( activeReaderCount == cyclingReadersFound );
            ASSERT_TRUE( cyclingWritersFound <= 1 );
            ASSERT_TRUE( ( activeReaderCount > 0 && 0 == activeWriterCount ) || ( activeWriterCount > 0 && 0 == activeReaderCount ) );
            ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
            ASSERT_TRUE( false == testData.DidAnyReaderHaveALockFailure() );
            ASSERT_TRUE( false == testData.DidAnyWriterHaveALockFailure() );

            if ( activeWriterCount > 0 )
                hadATestRunWithActiveWriters = true;
            if ( activeReaderCount > 0  )
                hadATestRunWithActiveReaders = true;

            ASSERT_TRUE( testData.Shutdown() );
        }
        while ( ( writersHavePriority && !hadATestRunWithActiveWriters ) || 
                ( !writersHavePriority && !hadATestRunWithActiveReaders ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockTests_MultipleWritersMultipleReaderWriters( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    try
    {       
        bool hadATestRunWithActiveWriters = false;
        bool hadATestRunWithActiveReaders = false;

        do 
        {
            CRwLockTestData testData( writersHavePriority, usingScopeObjects );

            // test with a multiple readers and writers all starting +- at the same time
            // this test is heavily reliant on timing and as such we can only really check that the tallies make sense
            testData.nrOfWriters = 5;
            testData.nrOfReaders = 0;
            testData.nrOfReaderAndWriters = 25;
            testData.writerReentrancyDepth = 0;
            testData.writerWorkIsReentrant = false;
            testData.readerReentrancyDepth = 0;
            testData.readerWorkIsReentrant = false;
            testData.writerWorkDurationInMs = GUCEF_UINT32MAX;
            testData.readerWorkDurationInMs = GUCEF_UINT32MAX;
            ASSERT_TRUE( testData.Configure() );
            UInt32 activeReaderCount = testData.rwLock.ActiveReaderCount();
            UInt32 activeWriterCount = testData.rwLock.ActiveWriterCount();
            UInt32 activeWriterReentrancyDepth = testData.rwLock.ActiveWriterReentrancyDepth();
            UInt32 queuedReaderCount = testData.rwLock.QueuedReaderCount();
            UInt32 queuedWriterCount = testData.rwLock.QueuedWriterCount();
            ASSERT_TRUE( testData.nrOfReaders == ( activeReaderCount + queuedReaderCount ) );
            ASSERT_TRUE( testData.nrOfWriters == ( activeWriterCount + queuedWriterCount ) );
            ASSERT_TRUE( 0 == activeWriterReentrancyDepth );
            MT::PrecisionDelay( GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS * 2 );
            UInt32 cyclingWritersFound = 0;
            for ( UInt32 i=0; i<testData.nrOfWriters; ++i )
            {
                MT::Float64 cyclingDurationInMs = testData.writers[ i ].GetCyclingDurationInMs();
                if ( cyclingDurationInMs > GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS )
                {
                    ++cyclingWritersFound;
                }
            }
            UInt32 cyclingReadersFound = 0;
            for ( UInt32 i=0; i<testData.nrOfReaders; ++i )
            {
                MT::Float64 cyclingDurationInMs = testData.readers[ i ].GetCyclingDurationInMs();
                if ( cyclingDurationInMs > GUCEF_RWLOCKTEST_TEST_THREAD_BOOTUP_TIME_IN_MS )
                {
                    ++cyclingReadersFound;
                }
            }
            ASSERT_TRUE( activeWriterCount == cyclingWritersFound );
            ASSERT_TRUE( activeReaderCount == cyclingReadersFound );
            ASSERT_TRUE( cyclingWritersFound <= 1 );
            ASSERT_TRUE( ( activeReaderCount > 0 && 0 == activeWriterCount ) || ( activeWriterCount > 0 && 0 == activeReaderCount ) );
            ASSERT_TRUE( writersHavePriority == testData.rwLock.DoWritersOverrule() );
            ASSERT_TRUE( false == testData.DidAnyReaderHaveALockFailure() );
            ASSERT_TRUE( false == testData.DidAnyWriterHaveALockFailure() );

            if ( activeWriterCount > 0 )
                hadATestRunWithActiveWriters = true;
            if ( activeReaderCount > 0  )
                hadATestRunWithActiveReaders = true;

            ASSERT_TRUE( testData.Shutdown() );
        }
        while ( ( writersHavePriority && !hadATestRunWithActiveWriters ) || 
                ( !writersHavePriority && !hadATestRunWithActiveReaders ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockSingleThreadTests( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    PerformReaderWriterLockSingleThreadTests_InitTest( writersHavePriority, usingScopeObjects );
    PerformReaderWriterLockSingleThreadTests_SingleReader( writersHavePriority, usingScopeObjects );
    PerformReaderWriterLockSingleThreadTests_SingleWriter( writersHavePriority, usingScopeObjects );
    PerformReaderWriterLockSingleThreadTests_SingleReaderReentrant( writersHavePriority, usingScopeObjects );
    PerformReaderWriterLockSingleThreadTests_SingleWriterReentrant( writersHavePriority, usingScopeObjects );
    PerformReaderWriterLockSingleThreadTests_SingleReaderWriterReentrant( writersHavePriority, usingScopeObjects );
    PerformReaderWriterLockSingleThreadTests_SingleReaderWriterExplicitTransition( writersHavePriority, usingScopeObjects );
    PerformReaderWriterLockSingleThreadTests_SingleReaderWriterExplicitTransitionReentrant( writersHavePriority, usingScopeObjects );
    PerformReaderWriterLockSingleThreadTests_SingleReaderWriterExplicitTransitionReentrantRandom( writersHavePriority, usingScopeObjects );
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockTests( bool writersHavePriority, bool usingScopeObjects )
{GUCEF_TRACE;

    PerformReaderWriterLockTests_SingleReader( writersHavePriority, usingScopeObjects );
    PerformReaderWriterLockTests_SingleWriter( writersHavePriority, usingScopeObjects );
    PerformReaderWriterLockTests_MultipleReaders( writersHavePriority, usingScopeObjects );
    PerformReaderWriterLockTests_MultipleWriters( writersHavePriority, usingScopeObjects );
    PerformReaderWriterLockTests_MultipleReadersReentrant( writersHavePriority, usingScopeObjects );
    PerformReaderWriterLockTests_MultipleWritersReentrant( writersHavePriority, usingScopeObjects );    
    PerformReaderWriterLockTests_MultipleWritersMultipleReaders( writersHavePriority, usingScopeObjects ); 
    PerformReaderWriterLockTests_MultipleReaderWritersReentrant( writersHavePriority, usingScopeObjects );
}

/*-------------------------------------------------------------------------*/

void
PerformReaderWriterLockTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Reader Writer lock TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "ReaderWriterLock" );

    // perform tests in the calling thread
    GUCEF_TESTFW_TESTCASE( "SingleThread: WritersPriority=true, ScopeObjects=false" )
        try { PerformReaderWriterLockSingleThreadTests( true, false ); } catch(...) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SingleThread: WritersPriority=true, ScopeObjects=true" )
        try { PerformReaderWriterLockSingleThreadTests( true, true ); } catch(...) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SingleThread: WritersPriority=false, ScopeObjects=false" )
        try { PerformReaderWriterLockSingleThreadTests( false, false ); } catch(...) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SingleThread: WritersPriority=false, ScopeObjects=true" )
        try { PerformReaderWriterLockSingleThreadTests( false, true ); } catch(...) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // spin up worker threads and run test workloads from that context
    GUCEF_TESTFW_TESTCASE( "MultiThread: WritersPriority=true, ScopeObjects=false" )
        try { PerformReaderWriterLockTests( true, false ); } catch(...) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MultiThread: WritersPriority=true, ScopeObjects=true" )
        try { PerformReaderWriterLockTests( true, true ); } catch(...) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    //GUCEF_TESTFW_TESTCASE( "MultiThread: WritersPriority=false, ScopeObjects=false" )
    //    try { PerformReaderWriterLockTests( false, false ); } catch(...) { ERRORHERE; }
    //GUCEF_TESTFW_TESTCASE_END

    //GUCEF_TESTFW_TESTCASE( "MultiThread: WritersPriority=false, ScopeObjects=true" )
    //    try { PerformReaderWriterLockTests( false, true ); } catch(...) { ERRORHERE; }
    //GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FINISHED Reader Writer lock TESTS" );
}

/*-------------------------------------------------------------------------*/
