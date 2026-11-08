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

#ifndef GUCEF_CORE_CABSTRACTFACTORYENTRYTRACKER_H
#define GUCEF_CORE_CABSTRACTFACTORYENTRYTRACKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The identity of a thread as the abstract factories record it: the one place where they name the thread identity of
 *  the platform (MT::GetCurrentThreadID(), whose values only are compared, never with an id of the operating system).
 *  It is compared with ==, copied and kept in GUCEF::vector, and nothing assumes its width.
 */
typedef TThreadId     TAbstractFactoryThreadId;

/** The identity of the calling thread, see TAbstractFactoryThreadId */
inline TAbstractFactoryThreadId
GetAbstractFactoryThreadId( void )
{
    return MT::GetCurrentThreadID();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The bookkeeping that every abstract factory (CTAbstractFactoryRegistry and the forms built on it) shares: an entry
 *  per registered concrete factory with the thread of every create through it that is under way, the removals that wait
 *  for those creates, the entries let go of without waiting, the owned concrete factories retired under creates, the
 *  detection of cycles of removals, the warnings of removals that wait long, and every text the abstract factories log.
 *
 *  It never locks. The members whose names end in Locked are called with the registry lock of the abstract factory
 *  held; the others need no lock. The concrete factories are kept as CICloneable, whose destructor is virtual, so the
 *  tracker deletes an owned one without knowing its type.
 *
 *  Rules (see also CTAbstractFactoryRegistry):
 *  - a removal waits for the creates through the removed entry on other threads; when only creates of its own thread
 *    remain (a removal from within such a create) it does not wait and the last of those creates lets go of the entry;
 *  - an owned concrete factory removed while creates through it are under way is retired: kept until the abstract
 *    factory is destroyed, as the products of those creates go through it;
 *  - removals that wait for each other's creates (a cycle) do not end; the removal that closes a cycle does not wait
 *    when its concrete factory is retired (owned), else it waits and the cycle is logged as an error. Cycles are found
 *    within one abstract factory only: the removals that wait are listed per abstract factory;
 *  - a removal that waits logs a warning after a time and again at an interval (5 s and 30 s by default), measured
 *    with a monotonic clock;
 *  - a removal on a thread that holds the registry lock the way code outside the registry takes it (the virtual Lock()
 *    of the abstract factory, see OnLockTakenByCallerLocked()) does not wait for creates of other threads, which take
 *    that lock to end: it is let go of without waiting, as from within a create of its own thread, and logs an error.
 */
class GUCEF_CORE_PUBLIC_CPP CAbstractFactoryEntryTracker
{
    public:

    /** A registered concrete factory and the creates through it; defined in the implementation only */
    class CEntry;

    /** What a look at the creates through a removed entry, under the lock, found */
    enum ERemovalWaitStep
    {
        REMOVALWAITSTEP_DONE ,              /**< no create through it is under way: the remover lets go of the entry */
        REMOVALWAITSTEP_LEFT ,              /**< only creates of the remover's thread: it does not wait, the last of them lets go of the entry */
        REMOVALWAITSTEP_LEFT_CYCLE ,        /**< the wait closes a cycle and the concrete factory is retired: it does not wait */
        REMOVALWAITSTEP_LEFT_LOCK_HELD ,    /**< the remover's thread holds the registry lock, the creates of other threads cannot end: it does not wait, the last of them lets go of the entry */
        REMOVALWAITSTEP_WAIT ,              /**< creates of other threads are under way: wait */
        REMOVALWAITSTEP_WAIT_IN_CYCLE       /**< as REMOVALWAITSTEP_WAIT, in a cycle of removals that wait for each other */
    };

    /** The state of one removal that waits, on the stack of the removing thread */
    class CRemovalWait
    {
        public:

        explicit CRemovalWait( CEntry* removedEntry )
            : entry( removedEntry ), thread( GetAbstractFactoryThreadId() ), waitStartInMs( 0 ), nextWarningAfterInMs( 0 ),
              listed( false ), timing( false ), cycleLogged( false )
        {}

        CEntry* entry;                          /**< the removed entry; null for the wait of a destructor */
        TAbstractFactoryThreadId thread;        /**< the removing thread */
        UInt64 waitStartInMs;
        UInt64 nextWarningAfterInMs;
        bool listed;                            /**< in the list of removals that wait */
        bool timing;
        bool cycleLogged;
    };

    explicit CAbstractFactoryEntryTracker( bool assumeFactoryOwnership );

    ~CAbstractFactoryEntryTracker();

    bool GetAssumeFactoryOwnership( void ) const;

    /** A new entry for the concrete factory; it owns the concrete factory when the tracker owns concrete factories */
    CEntry* NewEntryLocked( CICloneable* concreteFactory                ,
                            const CString& typeName                     ,
                            const CString& concreteClassTypeName        );

    /** The entry no longer owns its concrete factory: a replaced concrete factory goes back to whoever registered it */
    void DisownEntryLocked( CEntry* entry );

    void SetConcreteClassTypeNameLocked( CEntry* entry, const CString& concreteClassTypeName );

    static CICloneable* GetFactory( const CEntry* entry );

    static const CString& GetTypeName( const CEntry* entry );

    static const CString& GetConcreteClassTypeName( const CEntry* entry );

    /** Records a create of this thread through the entry; returns its concrete factory */
    CICloneable* BeginCreateLocked( CEntry* entry );

    /**
     *  Takes the create of this thread that started last off the entry. Returns true when it was the last create
     *  through an entry that was let go of without waiting: the caller then releases the entry, without the lock.
     */
    bool EndCreateLocked( CEntry* entry );

    /**
     *  Without the lock: ends a create through an entry that a destroyed abstract factory handed over to its creates
     *  (orphaned), and deletes the entry with the last of them. Returns false, and does nothing, for any other entry.
     *  It reads one flag of the entry, which only the destructor of the abstract factory writes, on this thread.
     */
    static bool EndCreateIfOrphaned( CEntry* entry );

    /** One look at the creates through the removed entry of the wait, see ERemovalWaitStep */
    ERemovalWaitStep TakeRemovalWaitStepLocked( CRemovalWait& wait );

    /**
     *  The registry lock as code outside the registry takes it: the virtual Lock() / Unlock() of the abstract factory,
     *  which AsLockable() with MT::CObjectScopeLock uses, and a derived class that forwards its own Lock() to the form's
     *  (CDataDrivenDStoreCodecFactory). Called with that lock held, right after it was taken and right before it is
     *  released; only when it was taken, so a lock type whose Lock() does not apply (MT::CNoLock) records nothing. It
     *  records the holding thread and how many times it holds the lock that way: a removal on that thread cannot wait for
     *  the creates of other threads, which take the lock to end (REMOVALWAITSTEP_LEFT_LOCK_HELD). The lock the registry
     *  takes for itself is not recorded.
     */
    void OnLockTakenByCallerLocked( void );

    /** See OnLockTakenByCallerLocked(); does nothing when no hold is recorded (one comparison) */
    void OnLockReleasedByCallerLocked( void )
    {
        if ( 0 < m_lockHolderDepth )
            EndLockHoldOfCallerLocked();
    }

    /**
     *  Without the lock, after a step that was not REMOVALWAITSTEP_DONE or REMOVALWAITSTEP_LEFT: logs what the step
     *  calls for (a cycle, a long wait, a removal under the lock held by its thread) and pauses 1 ms. Returns false when
     *  the removal stops waiting.
     */
    bool ContinueRemovalWait( CRemovalWait& wait                  ,
                              ERemovalWaitStep step               ,
                              const CString& abstractFactoryName  ) const;

    /** Deletes the entry, and its concrete factory when the entry owns it */
    static void ReleaseEntry( CEntry* entry );

    /** Whether a create of another thread is under way through an entry that was let go of without waiting */
    bool HasPendingCreatesOfOtherThreadsLocked( void ) const;

    /**
     *  For the destructor of the abstract factory, once only creates of this thread remain under way through entries
     *  let go of without waiting (it is destroyed from within such a create): hands those entries over to their creates
     *  (see EndCreateIfOrphaned()) and returns how many there were
     */
    UInt32 OrphanPendingEntriesLocked( void );

    /**
     *  For the destructor of the abstract factory: deletes the retired concrete factories, or, when entries were
     *  orphaned, logs the error and leaves them alone, as the products of those creates may still go through them
     */
    void DisposeRetiredFactories( UInt32 orphanedEntries                ,
                                  const CString& abstractFactoryName    );

    static void LogRegistration( const CString& typeName              ,
                                 const CString& concreteClassTypeName ,
                                 bool replaced                        );

    static void LogUnregistration( const CString& typeName              ,
                                   const CString& concreteClassTypeName ,
                                   bool waited                          );

    static void LogUnregistrationOfAll( void );

    /**
     *  An exception left the hook of a removal (in the notifying forms an observer of the unregistered event) and is not
     *  passed on: in a destructor, or after an earlier exception of the same removal of all concrete factories
     */
    static void LogExceptionFromRemovalHook( const CString& abstractFactoryName ,
                                             const CString& typeName            ,
                                             bool inDestructor                  );

    /** A debug log of every create, used by the forms in debug builds */
    static void LogCreate( const CString& typeName, bool created );

    /**
     *  The times after which a removal that waits logs a warning, and the interval of the warnings after that, for every
     *  abstract factory of the process. Set them before abstract factories are used concurrently (the tests shorten them).
     */
    static void SetLongRemovalWaitWarningTimes( UInt32 firstWarningAfterInMs, UInt32 warningIntervalInMs );

    static void GetLongRemovalWaitWarningTimes( UInt32& firstWarningAfterInMs, UInt32& warningIntervalInMs );

    private:

    CAbstractFactoryEntryTracker( const CAbstractFactoryEntryTracker& src );              /**< not implemented */
    CAbstractFactoryEntryTracker& operator=( const CAbstractFactoryEntryTracker& src );   /**< not implemented */

    void RetireOwnedFactoryLocked( CEntry* entry );

    void EndRemovalWaitLocked( CRemovalWait& wait );

    bool IsRemovalWaitCycleLocked( const CEntry* entry, TAbstractFactoryThreadId thisThread ) const;

    void AddPendingEntryLocked( CEntry* entry );

    void RemovePendingEntryLocked( CEntry* entry );

    void EndLockHoldOfCallerLocked( void );

    typedef GUCEF::vector< CRemovalWait* >  TRemovalWaits;
    typedef GUCEF::vector< CEntry* >        TEntryVector;
    typedef GUCEF::vector< CICloneable* >   TFactoryVector;

    TRemovalWaits m_removalWaits;           /**< the removals that wait, on the stacks of their threads */
    TEntryVector m_pendingEntries;          /**< entries let go of without waiting, until the last create through them ends */
    TFactoryVector m_retiredFactories;      /**< owned concrete factories kept until the abstract factory is destroyed */
    TAbstractFactoryThreadId m_lockHolder;  /**< the thread that holds the registry lock through Lock(), while m_lockHolderDepth is not 0 */
    UInt32 m_lockHolderDepth;               /**< how many times m_lockHolder holds the registry lock through Lock() */
    bool m_assumeFactoryOwnership;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CABSTRACTFACTORYENTRYTRACKER_H ? */
