/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_CAbstractFactoryEntryTracker.h"

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
  #undef min
  #undef max
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <time.h>
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
  #include <emscripten.h>
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAbstractFactoryEntryTracker::CEntry
{
    public:

    CEntry( CICloneable* concreteFactory, const CString& type, const CString& concreteClassType, bool ownsConcreteFactory )
        : factory( concreteFactory ), typeName( type ), concreteClassTypeName( concreteClassType ), creatingThreads(),
          ownsFactory( ownsConcreteFactory ), factoryRetired( false ), releaseWhenIdle( false ), orphaned( false )
    {}

    CICloneable* factory;
    CString typeName;                                           /**< the selection criterion as text, for the logs */
    CString concreteClassTypeName;                              /**< as the concrete factory gave it at registration */
    GUCEF::vector< TAbstractFactoryThreadId > creatingThreads;  /**< the thread of each create under way, under the lock */
    bool ownsFactory;                                           /**< letting go of the entry deletes the concrete factory */
    bool factoryRetired;                                        /**< the owned concrete factory went to the retired ones */
    bool releaseWhenIdle;                                       /**< let go of without waiting: the last create lets go of it */
    bool orphaned;                                              /**< handed over to its creates by a destroyed abstract factory */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt32 g_firstLongRemovalWaitWarningAfterInMs = 5000;
static UInt32 g_longRemovalWaitWarningIntervalInMs = 30000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Milliseconds since an arbitrary fixed point, from a clock that a change of the wall clock does not move
 */
static UInt64
GetMonotonicTimeInMs( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    if ( FALSE == ::QueryPerformanceFrequency( &frequency ) || FALSE == ::QueryPerformanceCounter( &counter ) || 0 >= frequency.QuadPart )
        return (UInt64) ::GetTickCount();
    const UInt64 ticks = (UInt64) counter.QuadPart;
    const UInt64 ticksPerSecond = (UInt64) frequency.QuadPart;
    return ( ticks / ticksPerSecond ) * 1000 + ( ( ticks % ticksPerSecond ) * 1000 ) / ticksPerSecond;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    struct timespec now;
    ::clock_gettime( CLOCK_MONOTONIC, &now );
    return (UInt64) now.tv_sec * 1000 + (UInt64) now.tv_nsec / 1000000;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    return (UInt64) ::emscripten_get_now();

    #else
    #error unsupported platform
    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAbstractFactoryEntryTracker::CAbstractFactoryEntryTracker( bool assumeFactoryOwnership )
    : m_removalWaits()
    , m_pendingEntries()
    , m_retiredFactories()
    , m_lockHolder()
    , m_lockHolderDepth( 0 )
    , m_assumeFactoryOwnership( assumeFactoryOwnership )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAbstractFactoryEntryTracker::~CAbstractFactoryEntryTracker()
{GUCEF_TRACE;

    // The abstract factory disposed of its entries and retired concrete factories, see DisposeRetiredFactories()
}

/*-------------------------------------------------------------------------*/

bool
CAbstractFactoryEntryTracker::GetAssumeFactoryOwnership( void ) const
{GUCEF_TRACE;

    return m_assumeFactoryOwnership;
}

/*-------------------------------------------------------------------------*/

CAbstractFactoryEntryTracker::CEntry*
CAbstractFactoryEntryTracker::NewEntryLocked( CICloneable* concreteFactory         ,
                                              const CString& typeName              ,
                                              const CString& concreteClassTypeName )
{GUCEF_TRACE;

    return GUCEF_NEW CEntry( concreteFactory, typeName, concreteClassTypeName, m_assumeFactoryOwnership );
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::DisownEntryLocked( CEntry* entry )
{GUCEF_TRACE;

    entry->ownsFactory = false;
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::SetConcreteClassTypeNameLocked( CEntry* entry, const CString& concreteClassTypeName )
{GUCEF_TRACE;

    entry->concreteClassTypeName = concreteClassTypeName;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CAbstractFactoryEntryTracker::GetFactory( const CEntry* entry )
{GUCEF_TRACE;

    return entry->factory;
}

/*-------------------------------------------------------------------------*/

const CString&
CAbstractFactoryEntryTracker::GetTypeName( const CEntry* entry )
{GUCEF_TRACE;

    return entry->typeName;
}

/*-------------------------------------------------------------------------*/

const CString&
CAbstractFactoryEntryTracker::GetConcreteClassTypeName( const CEntry* entry )
{GUCEF_TRACE;

    return entry->concreteClassTypeName;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CAbstractFactoryEntryTracker::BeginCreateLocked( CEntry* entry )
{GUCEF_TRACE;

    entry->creatingThreads.push_back( GetAbstractFactoryThreadId() );
    return entry->factory;
}

/*-------------------------------------------------------------------------*/

bool
CAbstractFactoryEntryTracker::EndCreateLocked( CEntry* entry )
{GUCEF_TRACE;

    // The create of this thread that started last: creates through the same entry nest on one thread
    const TAbstractFactoryThreadId thisThread = GetAbstractFactoryThreadId();
    GUCEF::vector< TAbstractFactoryThreadId >& threads = entry->creatingThreads;
    for ( size_t i=threads.size(); i>0; --i )
    {
        if ( threads[ i-1 ] == thisThread )
        {
            threads.erase( threads.begin() + ( i-1 ) );
            break;
        }
    }
    if ( entry->releaseWhenIdle && threads.empty() )
    {
        RemovePendingEntryLocked( entry );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAbstractFactoryEntryTracker::EndCreateIfOrphaned( CEntry* entry )
{GUCEF_TRACE;

    if ( !entry->orphaned )
        return false;

    // Only creates of this thread remain under way through an orphaned entry, nobody else uses it
    const TAbstractFactoryThreadId thisThread = GetAbstractFactoryThreadId();
    GUCEF::vector< TAbstractFactoryThreadId >& threads = entry->creatingThreads;
    for ( size_t i=threads.size(); i>0; --i )
    {
        if ( threads[ i-1 ] == thisThread )
        {
            threads.erase( threads.begin() + ( i-1 ) );
            break;
        }
    }
    if ( threads.empty() )
        ReleaseEntry( entry );
    return true;
}

/*-------------------------------------------------------------------------*/

CAbstractFactoryEntryTracker::ERemovalWaitStep
CAbstractFactoryEntryTracker::TakeRemovalWaitStepLocked( CRemovalWait& wait )
{GUCEF_TRACE;

    CEntry* entry = wait.entry;
    const GUCEF::vector< TAbstractFactoryThreadId >& threads = entry->creatingThreads;
    if ( threads.empty() )
    {
        EndRemovalWaitLocked( wait );
        return REMOVALWAITSTEP_DONE;
    }

    // The products of the creates under way go through the concrete factory
    RetireOwnedFactoryLocked( entry );

    bool createOfOtherThread = false;
    for ( size_t i=0; i<threads.size() && !createOfOtherThread; ++i )
        createOfOtherThread = ( threads[ i ] != wait.thread );

    // Only creates of this thread remain: the removal comes from within them and waiting would never end
    if ( !createOfOtherThread )
    {
        EndRemovalWaitLocked( wait );
        entry->releaseWhenIdle = true;
        AddPendingEntryLocked( entry );
        return REMOVALWAITSTEP_LEFT;
    }

    // This thread holds the registry lock through Lock() (AsLockable(), or a class that shares the lock): the creates of
    // other threads take that lock to end, so they cannot end while the removal waits and waiting would never end
    if ( 0 < m_lockHolderDepth && m_lockHolder == wait.thread )
    {
        EndRemovalWaitLocked( wait );
        entry->releaseWhenIdle = true;
        AddPendingEntryLocked( entry );
        return REMOVALWAITSTEP_LEFT_LOCK_HELD;
    }

    if ( !wait.listed )
    {
        m_removalWaits.push_back( &wait );
        wait.listed = true;
    }

    // A cycle of removals that wait for each other's creates, which this removal can leave when the abstract factory
    // keeps the concrete factory
    const bool waitCycle = IsRemovalWaitCycleLocked( entry, wait.thread );
    if ( waitCycle && entry->factoryRetired )
    {
        EndRemovalWaitLocked( wait );
        entry->releaseWhenIdle = true;
        AddPendingEntryLocked( entry );
        return REMOVALWAITSTEP_LEFT_CYCLE;
    }
    return waitCycle ? REMOVALWAITSTEP_WAIT_IN_CYCLE : REMOVALWAITSTEP_WAIT;
}

/*-------------------------------------------------------------------------*/

bool
CAbstractFactoryEntryTracker::ContinueRemovalWait( CRemovalWait& wait                 ,
                                                   ERemovalWaitStep step              ,
                                                   const CString& abstractFactoryName ) const
{GUCEF_TRACE;

    // The entry and its texts stay while the removal has not let go of it, the leave of a cycle excepted
    if ( REMOVALWAITSTEP_LEFT_CYCLE == step )
        return false;

    // The entry was let go of without waiting, but the creates through it cannot end before this thread releases the
    // registry lock, nor can creates of this thread end during this call: the entry and its texts stay for the log
    if ( REMOVALWAITSTEP_LEFT_LOCK_HELD == step )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_CRITICAL, "TAbstractFactory<>: " << abstractFactoryName << ": the removal of the concrete factory for type \""
            << wait.entry->typeName << "\" with concrete class type \"" << wait.entry->concreteClassTypeName
            << "\" runs on a thread that holds the registry lock (through AsLockable() or Lock()) while creates through it are under way on other "
               "threads, which cannot end before that lock is released: the removal does not wait for them, the concrete factory is let go of when "
               "the last of them ends (an owned one is kept until the abstract factory is destroyed) and must not be destroyed before then" );
        return false;
    }

    if ( REMOVALWAITSTEP_WAIT_IN_CYCLE == step && !wait.cycleLogged && GUCEF_NULL != wait.entry )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_CRITICAL, "TAbstractFactory<>: " << abstractFactoryName << ": the removal of the concrete factory for type \""
            << wait.entry->typeName << "\" with concrete class type \"" << wait.entry->concreteClassTypeName
            << "\" waits for creates whose threads wait in removals for a create of this thread: the removals wait for each other and do not end" );
        wait.cycleLogged = true;
    }

    const UInt64 now = GetMonotonicTimeInMs();
    if ( !wait.timing )
    {
        wait.waitStartInMs = now;
        wait.nextWarningAfterInMs = g_firstLongRemovalWaitWarningAfterInMs;
        wait.timing = true;
    }
    const UInt64 waitedInMs = now - wait.waitStartInMs;
    if ( waitedInMs >= wait.nextWarningAfterInMs )
    {
        if ( GUCEF_NULL != wait.entry )
        {
            GUCEF_WARNING_LOG( LOGLEVEL_IMPORTANT, "TAbstractFactory<>: " << abstractFactoryName << ": the removal of the concrete factory for type \""
                << wait.entry->typeName << "\" with concrete class type \"" << wait.entry->concreteClassTypeName
                << "\" has waited " << waitedInMs << " ms for the creates through it that are under way" );
        }
        else
        {
            GUCEF_WARNING_LOG( LOGLEVEL_IMPORTANT, "TAbstractFactory<>: " << abstractFactoryName << ": the destruction of the abstract factory has waited "
                << waitedInMs << " ms for the creates under way through concrete factories removed without waiting" );
        }
        wait.nextWarningAfterInMs = waitedInMs + g_longRemovalWaitWarningIntervalInMs;
    }
    MT::ThreadDelay( 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAbstractFactoryEntryTracker::IsRemovalWaitCycleLocked( const CEntry* entry, TAbstractFactoryThreadId thisThread ) const
{GUCEF_TRACE;

    // A cycle takes the removal wait of this thread and at least one other
    if ( m_removalWaits.size() < 2 )
        return false;

    // The threads of the creates this removal waits for (not those of its own thread), then the threads of the creates
    // their removals wait for
    GUCEF::vector< TAbstractFactoryThreadId > threadsToVisit;
    for ( size_t i=0; i<entry->creatingThreads.size(); ++i )
    {
        if ( entry->creatingThreads[ i ] != thisThread )
            threadsToVisit.push_back( entry->creatingThreads[ i ] );
    }
    GUCEF::vector< TAbstractFactoryThreadId > visitedThreads;
    while ( !threadsToVisit.empty() )
    {
        const TAbstractFactoryThreadId thread = threadsToVisit.back();
        threadsToVisit.pop_back();
        if ( thread == thisThread )
            return true;

        bool visited = false;
        for ( size_t i=0; i<visitedThreads.size() && !visited; ++i )
            visited = ( visitedThreads[ i ] == thread );
        if ( visited )
            continue;
        visitedThreads.push_back( thread );

        for ( size_t w=0; w<m_removalWaits.size(); ++w )
        {
            if ( m_removalWaits[ w ]->thread == thread )
            {
                const GUCEF::vector< TAbstractFactoryThreadId >& waitedFor = m_removalWaits[ w ]->entry->creatingThreads;
                for ( size_t i=0; i<waitedFor.size(); ++i )
                {
                    if ( waitedFor[ i ] != thread )
                        threadsToVisit.push_back( waitedFor[ i ] );
                }
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::EndRemovalWaitLocked( CRemovalWait& wait )
{GUCEF_TRACE;

    if ( !wait.listed )
        return;
    for ( size_t w=0; w<m_removalWaits.size(); ++w )
    {
        if ( m_removalWaits[ w ] == &wait )
        {
            m_removalWaits.erase( m_removalWaits.begin() + w );
            break;
        }
    }
    wait.listed = false;
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::OnLockTakenByCallerLocked( void )
{GUCEF_TRACE;

    // The lock is recursive per thread: while it is held, only the holding thread takes it again
    const TAbstractFactoryThreadId thisThread = GetAbstractFactoryThreadId();
    if ( 0 < m_lockHolderDepth && m_lockHolder == thisThread )
    {
        ++m_lockHolderDepth;
        return;
    }
    m_lockHolder = thisThread;
    m_lockHolderDepth = 1;
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::EndLockHoldOfCallerLocked( void )
{GUCEF_TRACE;

    if ( m_lockHolder == GetAbstractFactoryThreadId() )
        --m_lockHolderDepth;
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::RetireOwnedFactoryLocked( CEntry* entry )
{GUCEF_TRACE;

    if ( entry->ownsFactory )
    {
        m_retiredFactories.push_back( entry->factory );
        entry->ownsFactory = false;
        entry->factoryRetired = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::AddPendingEntryLocked( CEntry* entry )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_pendingEntries.size(); ++i )
    {
        if ( m_pendingEntries[ i ] == entry )
            return;
    }
    m_pendingEntries.push_back( entry );
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::RemovePendingEntryLocked( CEntry* entry )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_pendingEntries.size(); ++i )
    {
        if ( m_pendingEntries[ i ] == entry )
        {
            m_pendingEntries.erase( m_pendingEntries.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::ReleaseEntry( CEntry* entry )
{GUCEF_TRACE;

    if ( entry->ownsFactory )
        GUCEF_DELETE entry->factory;
    entry->factory = GUCEF_NULL;
    GUCEF_DELETE entry;
}

/*-------------------------------------------------------------------------*/

bool
CAbstractFactoryEntryTracker::HasPendingCreatesOfOtherThreadsLocked( void ) const
{GUCEF_TRACE;

    const TAbstractFactoryThreadId thisThread = GetAbstractFactoryThreadId();
    for ( size_t e=0; e<m_pendingEntries.size(); ++e )
    {
        const GUCEF::vector< TAbstractFactoryThreadId >& threads = m_pendingEntries[ e ]->creatingThreads;
        for ( size_t i=0; i<threads.size(); ++i )
        {
            if ( threads[ i ] != thisThread )
                return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CAbstractFactoryEntryTracker::OrphanPendingEntriesLocked( void )
{GUCEF_TRACE;

    const UInt32 orphaned = (UInt32) m_pendingEntries.size();
    for ( size_t e=0; e<m_pendingEntries.size(); ++e )
        m_pendingEntries[ e ]->orphaned = true;
    m_pendingEntries.clear();
    return orphaned;
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::DisposeRetiredFactories( UInt32 orphanedEntries               ,
                                                       const CString& abstractFactoryName   )
{GUCEF_TRACE;

    if ( 0 < orphanedEntries )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "TAbstractFactory<>: " << abstractFactoryName << ": destroyed from within a create through it, which is not supported: "
            << orphanedEntries << " concrete factories have creates of this thread under way, those creates end without the abstract factory; the "
            << (UInt32) m_retiredFactories.size() << " owned concrete factories retired under creates are not deleted" );
    }
    else
    {
        for ( size_t i=0; i<m_retiredFactories.size(); ++i )
            GUCEF_DELETE m_retiredFactories[ i ];
    }
    m_retiredFactories.clear();
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::LogRegistration( const CString& typeName              ,
                                               const CString& concreteClassTypeName ,
                                               bool replaced                        )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "TAbstractFactory<>: Registered concrete factory for type \"" << typeName << "\" with concrete class type \""
        << concreteClassTypeName << ( replaced ? "\", it replaced the one registered before" : "\"" ) );
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::LogUnregistration( const CString& typeName              ,
                                                 const CString& concreteClassTypeName ,
                                                 bool waited                          )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "TAbstractFactory<>: Unregistered concrete factory for type \"" << typeName << "\" with concrete class type \""
        << concreteClassTypeName << ( waited ? "\"" : "\" without waiting for the creates through it, it is let go when the last of them ends" ) );
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::LogUnregistrationOfAll( void )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "TAbstractFactory<>: Unregistered all concrete factories" );
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::LogExceptionFromRemovalHook( const CString& abstractFactoryName ,
                                                           const CString& typeName            ,
                                                           bool inDestructor                  )
{GUCEF_TRACE;

    if ( inDestructor )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_CRITICAL, "TAbstractFactory<>: " << abstractFactoryName << ": an exception left the notification of the removal of the concrete factory for type \""
            << typeName << "\" in the destructor of the abstract factory: it is not passed on, as a destructor that throws ends the process" );
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_CRITICAL, "TAbstractFactory<>: " << abstractFactoryName << ": an exception left the notification of the removal of the concrete factory for type \""
            << typeName << "\" after an earlier exception of the same removal of all concrete factories: only the first exception is passed on" );
    }
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::LogCreate( const CString& typeName, bool created )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "TAbstractFactory<>: " << ( created ? "Created" : "Failed to create" ) << " concrete type \"" << typeName << "\"" );
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::SetLongRemovalWaitWarningTimes( UInt32 firstWarningAfterInMs, UInt32 warningIntervalInMs )
{GUCEF_TRACE;

    g_firstLongRemovalWaitWarningAfterInMs = firstWarningAfterInMs;
    g_longRemovalWaitWarningIntervalInMs = warningIntervalInMs;
}

/*-------------------------------------------------------------------------*/

void
CAbstractFactoryEntryTracker::GetLongRemovalWaitWarningTimes( UInt32& firstWarningAfterInMs, UInt32& warningIntervalInMs )
{GUCEF_TRACE;

    firstWarningAfterInMs = g_firstLongRemovalWaitWarningAfterInMs;
    warningIntervalInMs = g_longRemovalWaitWarningIntervalInMs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
