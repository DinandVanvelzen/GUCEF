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

#ifndef GUCEF_CORE_CTABSTRACTFACTORYREGISTRY_H
#define GUCEF_CORE_CTABSTRACTFACTORYREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <set>

#ifndef GUCEF_MT_LOCKSTATUS_H
#include "gucefMT_LockStatus.h"
#define GUCEF_MT_LOCKSTATUS_H
#endif /* GUCEF_MT_LOCKSTATUS_H ? */

#ifndef GUCEF_TIMEOUT_EXCEPTION_H
#include "gucef_timeout_exception.h"
#define GUCEF_TIMEOUT_EXCEPTION_H
#endif /* GUCEF_TIMEOUT_EXCEPTION_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CABSTRACTFACTORYENTRYTRACKER_H
#include "gucefCORE_CAbstractFactoryEntryTracker.h"
#define GUCEF_CORE_CABSTRACTFACTORYENTRYTRACKER_H
#endif /* GUCEF_CORE_CABSTRACTFACTORYENTRYTRACKER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEMPLATES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The registry that every abstract factory is built on: the concrete factories (TFactory, a CTFactoryBase or a
 *  CTFactoryBaseWithParam) registered per selection criterion, and everything an abstract factory does except making a
 *  product, which the forms add with the signature of their concrete factories (CTAbstractFactory,
 *  CTAbstractFactoryWithParam, CTBasicAbstractFactory, CTBasicAbstractFactoryWithParam). TBase is the base the forms
 *  share: CAbstractFactoryBase for the notifying forms, CBasicAbstractFactoryBase for the lightweight ones. The
 *  bookkeeping is in CAbstractFactoryEntryTracker.
 *
 *  Note that when using this template it is mandatory for BaseClassType classes to define the following member
 *  function: SelectionCriteriaType GetType( void ) const;
 *
 *  Threads: the registry is guarded by its lock (a LockType member). The registry locks that member itself, without
 *  virtual calls, so the end of a create on one thread never calls into an abstract factory that another thread is
 *  destroying; the virtual Lock() / Unlock() lock the same member for code that shares it (AsLockable(), or a derived
 *  class that forwards its own Lock() to the form, as CDataDrivenDStoreCodecFactory does). The lock is held only to look up or change the registry and the
 *  bookkeeping: never while a product is made, while a removal waits, while logging or while the hooks below run. A
 *  create takes its concrete factory under the lock and records the create on it by the identity of its thread, makes
 *  the product without the lock, and ends the create on every way out (also when the product constructor throws). A
 *  product constructor may therefore take other locks, while a thread that holds them creates through the same registry.
 *
 *  Removal: UnregisterConcreteFactory(), a replacement by RegisterConcreteFactory(), UnregisterAllConcreteFactories()
 *  and the destructor take the concrete factory out of the registry, so creates that start afterwards do not find it,
 *  and wait until no create through it is under way on another thread; then the owner of a concrete factory that the
 *  registry does not own may destroy it, after its products (a product goes through the concrete factory that made
 *  it). They wait for the creates through the removed concrete factory only. A removal from within a create through
 *  the same concrete factory on the same thread (a product constructor that unregisters or replaces its own type)
 *  waits for the creates of other threads through it but not for the creates of its own thread, which would never end;
 *  the last of those lets go of it, and its owner does not destroy it before that.
 *
 *  Removal under the lock of the caller: code outside the registry may hold the registry lock through Lock()
 *  (AsLockable() with MT::CObjectScopeLock, or a derived class that shares the lock, as CDataDrivenDStoreCodecFactory
 *  does with its registry of codec meta data, whose operations hold it while they notify their observers). A removal
 *  on a thread that holds the lock that way cannot wait for the creates through the removed concrete factory on other
 *  threads, as ending a create takes the lock. It does not wait for them (no deadlock): it logs an error naming the
 *  type, and the concrete factory is let go of when the last of those creates ends, as for a removal from within a
 *  create of its own thread. An owned one is retired (kept until the registry is destroyed); the owner of one that is
 *  not owned must not destroy it before those creates ended, which is after the lock is released. Lock() and Unlock()
 *  record the holding thread and how many times it holds the lock, under the lock, so a derived class that overrides
 *  them forwards to them to share the lock. A lock type whose lock does not apply (MT::CNoLock) records nothing and
 *  Unlock() costs one comparison; the locking of the registry itself is not recorded, so Create() does not pay for it.
 *  Destroying the registry while holding its lock is not supported.
 *
 *  Replacement: RegisterConcreteFactory() for a type that has another concrete factory registered replaces it; the new
 *  one serves the creates that start from then on. The registry never deletes a replaced concrete factory, also not
 *  when it owns its concrete factories: it goes back to whoever registered it. Registering the concrete factory that is
 *  registered for the type already changes nothing.
 *
 *  Owned concrete factories (assumeFactoryOwnership): one that is removed while no create through it is under way is
 *  deleted at once, so its products must be gone by then. One that is removed while creates through it are under way is
 *  kept until the registry is destroyed, as the products of those creates go through it.
 *
 *  Waits: removals whose waits close a cycle (product constructors on two threads that unregister each other's type)
 *  would wait for each other forever. When the registry owns the concrete factory whose removal closes the cycle that
 *  removal does not wait; with one it does not own it waits and logs the cycle as an error. Cycles are detected within
 *  one registry only: a cycle through removals in two registries is not detected and does not end. A removal that waits
 *  logs a warning after 5 seconds and every 30 seconds after that, see CAbstractFactoryEntryTracker.
 *
 *  Class type names: the concrete class type name of every registered concrete factory, as it gave it at registration,
 *  maps to its selection criterion; replaced and removed ones no longer do. When the concrete factories of two types
 *  have the same concrete class, the name maps to the type registered last, and to the other one once that is removed.
 *
 *  Hooks: after every registration and every removal the registry calls OnConcreteFactoryRegistered() or
 *  OnConcreteFactoryUnregistered() for the type, without the lock, on the thread of the operation: once per
 *  RegisterConcreteFactory() (also for a replacement and a repeat), once per entry that UnregisterConcreteFactory(),
 *  UnregisterAllConcreteFactories() or a destructor removes. They do nothing here; the notifying forms send their events
 *  from them, a lightweight one may override them. As for any virtual function, a destructor calls the hook of its own
 *  class, not the override of a derived class: the entries left at destruction reach no hook of a derived class. The
 *  notifying forms still send their events for them, from their own destructor; the lightweight forms give no signal
 *  per entry. A derived class that wants its hooks for those entries calls UnregisterAllConcreteFactories() in its own
 *  destructor.
 *
 *  Order of the hooks: those of one operation run in its order (UnregisterAllConcreteFactories() and the destructor in
 *  key order, once every entry was let go of) and those of the operations of one thread in the order of the operations.
 *  The hooks of operations on different threads, also for one type, may run in an order other than the one in which
 *  the operations took effect, as they run after the operation and without the lock (after any wait of a removal): when
 *  one thread registers a type while another unregisters it, the unregistered hook may come first although the
 *  registration took effect last. The registry itself holds the outcome of the operation that took effect last.
 *
 *  Exceptions out of the hooks (in the notifying forms out of observers of the events): the operation is complete when
 *  its hook runs, so the exception leaves an operation whose entry is registered, or removed and let go of.
 *  UnregisterAllConcreteFactories() lets go of every entry first, then calls the hook of every entry, also after an
 *  earlier one threw, and passes the first exception on; the later ones are logged as errors. A destructor passes none
 *  on (a destructor that throws ends the process): it logs each one as an error.
 *
 *  Destruction: the registry is not destroyed from within a create through it. If it is, that is logged as an error,
 *  the creates under way on that thread end without it, and the owned concrete factories they may use are left alone.
 *  A class derived from it that can be destroyed while creates are under way calls UnregisterAllConcreteFactories() in
 *  its own destructor, as those creates still use the derived object until they end.
 *
 *  Lock time limits: the public operations wait for the lock up to GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS and throw
 *  timeout_exception after that where locks have time limits; the end of a create, the waits of removals,
 *  UnregisterAllConcreteFactories() and the destructor wait without a limit and never throw.
 */
template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
class CTAbstractFactoryRegistry : public TBase
{
    public:

    typedef SelectionCriteriaType                                                                                           TSelectionCriteriaType;
    typedef CTCloneableObj< SelectionCriteriaType >                                                                         TKeyContainer;
    typedef TFactoryType                                                                                                    TFactory;
    typedef GUCEF::set< SelectionCriteriaType, std::less< SelectionCriteriaType >, gucef_allocator< SelectionCriteriaType > > TKeySet;
    typedef typename TFactoryType::TProductPtr                                                                              TProductPtr;

    /**
     *  Registers the concrete factory for the given type. When another concrete factory is registered for the type it
     *  is replaced; the registration then returns once the creates through the replaced one that are under way on other
     *  threads ended, and the replaced one is never deleted by the registry, see the class documentation
     */
    void RegisterConcreteFactory( const SelectionCriteriaType& selectedType ,
                                  TFactory* concreteFactory                 );

    /**
     *  Removes the concrete factory of the given type; returns once the creates through it that are under way on other
     *  threads ended, see the class documentation
     */
    void UnregisterConcreteFactory( const SelectionCriteriaType& selectedType );

    /**
     *  Removes every concrete factory; returns once the creates through them that are under way on other threads
     *  ended, see the class documentation
     */
    void UnregisterAllConcreteFactories( void );

    bool IsConstructible( const SelectionCriteriaType& selectedType ) const;

    bool IsClassConstructible( const CString& classTypeName ) const;

    /**
     *  Returns the concete class's type name as produced by a factory linked to the given selection criterea
     */
    CString GetConcreteClassTypeNameForSelectionCriterea( const SelectionCriteriaType& selectedType ) const;

    /**
     *  Returns the factory linked selection criterea for the given concete class's type name
     */
    SelectionCriteriaType GetSelectionCritereaForConcreteClassTypeName( const CString& classTypeName ) const;

    void ObtainKeySet( TKeySet& keySet ) const;

    protected:

    /**
     *  The one place where the registry chooses how it locks: every acquisition of the registry lock goes through this
     *  scope lock, which locks the LockType member directly. The first form, for the public operations, waits up to
     *  GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS and throws timeout_exception after that where the lock has time limits; the
     *  second waits without a limit and never throws. A lock that does not apply (MT::CNoLock, LOCKSTATUS_NOT_APPLICABLE)
     *  is not released.
     */
    class CRegistryScopeLock
    {
        public:

        explicit CRegistryScopeLock( const CTAbstractFactoryRegistry& registry )
            : m_lock( registry.m_lock ), m_unlockNeeded( false )
        {
            if ( Take( GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) == MT::LOCKSTATUS_WAIT_TIMEOUT )
                throw timeout_exception();
        }

        CRegistryScopeLock( const CTAbstractFactoryRegistry& registry, bool waitWithoutTimeLimit )
            : m_lock( registry.m_lock ), m_unlockNeeded( false )
        {
            Take( waitWithoutTimeLimit ? GUCEF_MT_INFINITE_LOCK_TIMEOUT : GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS );
        }

        ~CRegistryScopeLock()
        {
            if ( m_unlockNeeded )
                m_lock.Unlock();
        }

        private:

        MT::TLockStatus Take( UInt32 lockWaitTimeoutInMs )
        {
            const MT::TLockStatus lockStatus = m_lock.Lock( lockWaitTimeoutInMs );
            m_unlockNeeded = ( MT::LOCKSTATUS_OPERATION_SUCCESS == lockStatus || MT::LOCKSTATUS_ABANDONED == lockStatus );
            return lockStatus;
        }

        CRegistryScopeLock( const CRegistryScopeLock& src );              /**< not implemented */
        CRegistryScopeLock& operator=( const CRegistryScopeLock& src );   /**< not implemented */

        const LockType& m_lock;
        bool m_unlockNeeded;
    };

    /**
     *  The twin of CRegistryScopeLock for the queries, which only read the registry: the place where a lock type that
     *  tells readers from writers would take the read lock. The lock types in use have one kind of lock.
     */
    class CRegistryScopeReadOnlyLock
    {
        public:

        explicit CRegistryScopeReadOnlyLock( const CTAbstractFactoryRegistry& registry )
            : m_lock( registry )
        {}

        private:

        CRegistryScopeReadOnlyLock( const CRegistryScopeReadOnlyLock& src );              /**< not implemented */
        CRegistryScopeReadOnlyLock& operator=( const CRegistryScopeReadOnlyLock& src );   /**< not implemented */

        CRegistryScopeLock m_lock;
    };

    /**
     *  A create through the registry, for the forms: takes the concrete factory of the type under the lock and records
     *  the create, and ends the create when it goes out of scope, on every way out of the create. GetFactory() is null
     *  when no concrete factory is registered for the type.
     */
    class CCreateScope
    {
        public:

        CCreateScope( const CTAbstractFactoryRegistry& registry, const SelectionCriteriaType& selectedType );

        ~CCreateScope();

        TFactory* GetFactory( void ) const { return m_factory; }

        private:

        CCreateScope( const CCreateScope& src );              /**< not implemented */
        CCreateScope& operator=( const CCreateScope& src );   /**< not implemented */

        const CTAbstractFactoryRegistry& m_registry;
        CAbstractFactoryEntryTracker::CEntry* m_entry;
        TFactory* m_factory;
    };
    friend class CRegistryScopeLock;
    friend class CRegistryScopeReadOnlyLock;
    friend class CCreateScope;

    explicit CTAbstractFactoryRegistry( const bool assumeFactoryOwnership );

    /** For a TBase whose constructor takes whether it sends events (CAbstractFactoryBase) */
    CTAbstractFactoryRegistry( const bool assumeFactoryOwnership, const bool useEventing );

    /** Copies the class type names only, not the concrete factories */
    CTAbstractFactoryRegistry( const CTAbstractFactoryRegistry& src );

    virtual ~CTAbstractFactoryRegistry();

    /** Does nothing: concrete factories are not copied */
    CTAbstractFactoryRegistry& operator=( const CTAbstractFactoryRegistry& src );

    /**
     *  Locks the registry lock for code outside the registry (AsLockable(), a derived class that shares the lock) and
     *  records the hold, see the class documentation; a derived class that overrides it forwards to it to share the lock
     */
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Ends a hold of Lock(), see there */
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Called after every registration, see the class documentation; does nothing here */
    virtual void OnConcreteFactoryRegistered( const SelectionCriteriaType& selectedType );

    /** Called after every removal of an entry, see the class documentation; does nothing here */
    virtual void OnConcreteFactoryUnregistered( const SelectionCriteriaType& selectedType );

    /**
     *  Removes every entry, waits for the creates through them on other threads and lets go of them, then calls the
     *  hook per entry, see the class documentation for exceptions out of the hooks. Logs per entry and for all of them,
     *  except in a destructor.
     */
    void RemoveAllEntries( bool inDestructor );

    /** Resolves a concrete class type name to the type of its registered concrete factory */
    bool TryGetSelectionCriteriaForClassTypeName( const CString& classTypeName, SelectionCriteriaType& selectedType ) const;

    private:

    typedef CAbstractFactoryEntryTracker::CEntry                        TEntry;
    typedef std::pair< const SelectionCriteriaType, TEntry* >           TFactoryEntryPair;
    typedef GUCEF::map< SelectionCriteriaType, TEntry*, std::less< SelectionCriteriaType >, gucef_allocator< TFactoryEntryPair > >  TFactoryList;
    typedef std::pair< const CString, SelectionCriteriaType >           TStringToSCPair;
    typedef GUCEF::map< CString, SelectionCriteriaType, std::less< CString >, gucef_allocator< TStringToSCPair > >  TClassTypeNameMap;

    /**
     *  Waits, without the lock, until no create through the removed entry is under way on another thread. Returns true
     *  when the caller lets go of the entry now, false when the entry is let go of by the last create through it (only
     *  creates of this thread remain, the wait closed a cycle and the concrete factory is retired, or this thread holds
     *  the registry lock through Lock() so the creates of other threads cannot end while it waits).
     */
    bool WaitForCreatesOfRemovedEntry( TEntry* entry );

    /**
     *  Calls the hook of every removed entry, each guarded: an exception does not keep the hooks of the other entries
     *  from running. In a destructor every exception is logged and none is passed on; otherwise the first one is passed
     *  on once all hooks ran and the later ones are logged.
     */
    void CallUnregisteredHooks( const TFactoryList& removed, bool inDestructor );

    /** Under the lock: the class type name no longer maps to the type; it maps to another type that has it, if any */
    void EraseClassTypeNameLocked( const CString& classTypeName, const SelectionCriteriaType& selectedType );

    TFactoryList m_concreteFactoryList;
    TClassTypeNameMap m_concreteFactoryTypeMap;
    mutable CAbstractFactoryEntryTracker m_tracker;
    LockType m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::CCreateScope::CCreateScope( const CTAbstractFactoryRegistry& registry          ,
                                                                                                                const SelectionCriteriaType& selectedType )
    : m_registry( registry )
    , m_entry( GUCEF_NULL )
    , m_factory( GUCEF_NULL )
{GUCEF_TRACE;

    CRegistryScopeLock lock( registry );
    typename TFactoryList::const_iterator i( registry.m_concreteFactoryList.find( selectedType ) );
    if ( i != registry.m_concreteFactoryList.end() )
    {
        m_entry = (*i).second;
        m_factory = static_cast< TFactory* >( registry.m_tracker.BeginCreateLocked( m_entry ) );
    }
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::CCreateScope::~CCreateScope()
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_entry )
        return;

    // A registry destroyed from within this create handed the entry over to its creates: the registry is gone
    if ( CAbstractFactoryEntryTracker::EndCreateIfOrphaned( m_entry ) )
        return;

    bool releaseEntry = false;
    {
        CRegistryScopeLock lock( m_registry, true );
        releaseEntry = m_registry.m_tracker.EndCreateLocked( m_entry );
    }
    if ( releaseEntry )
        CAbstractFactoryEntryTracker::ReleaseEntry( m_entry );
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::CTAbstractFactoryRegistry( const bool assumeFactoryOwnership )
    : TBase()
    , m_concreteFactoryList()
    , m_concreteFactoryTypeMap()
    , m_tracker( assumeFactoryOwnership )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::CTAbstractFactoryRegistry( const bool assumeFactoryOwnership ,
                                                                                                              const bool useEventing            )
    : TBase( useEventing )
    , m_concreteFactoryList()
    , m_concreteFactoryTypeMap()
    , m_tracker( assumeFactoryOwnership )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::CTAbstractFactoryRegistry( const CTAbstractFactoryRegistry& src )
    : TBase( src )
    , m_concreteFactoryList()   // Wont copy raw factory pointers, too much risk of bad access. only copy 'config' style info
    , m_concreteFactoryTypeMap( src.m_concreteFactoryTypeMap )
    , m_tracker( src.m_tracker.GetAssumeFactoryOwnership() )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::~CTAbstractFactoryRegistry()
{GUCEF_TRACE;

    // The notifying forms removed their entries already, sending their events
    RemoveAllEntries( true );

    // Wait for the creates of other threads through entries let go of without waiting; what remains are creates of this
    // thread: the registry is destroyed from within a create through it
    CAbstractFactoryEntryTracker::CRemovalWait wait( GUCEF_NULL );
    UInt32 orphanedEntries = 0;
    for ( ;; )
    {
        bool waitForOtherThreads = false;
        {
            CRegistryScopeLock lock( *this, true );
            waitForOtherThreads = m_tracker.HasPendingCreatesOfOtherThreadsLocked();
            if ( !waitForOtherThreads )
                orphanedEntries = m_tracker.OrphanPendingEntriesLocked();
        }
        if ( !waitForOtherThreads )
            break;
        m_tracker.ContinueRemovalWait( wait, CAbstractFactoryEntryTracker::REMOVALWAITSTEP_WAIT, this->GetClassTypeName() );
    }
    m_tracker.DisposeRetiredFactories( orphanedEntries, this->GetClassTypeName() );
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >&
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::operator=( const CTAbstractFactoryRegistry& /* src */ )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
bool
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::WaitForCreatesOfRemovedEntry( TEntry* entry )
{GUCEF_TRACE;

    CAbstractFactoryEntryTracker::CRemovalWait wait( entry );
    for ( ;; )
    {
        CAbstractFactoryEntryTracker::ERemovalWaitStep step = CAbstractFactoryEntryTracker::REMOVALWAITSTEP_DONE;
        {
            CRegistryScopeLock lock( *this, true );
            step = m_tracker.TakeRemovalWaitStepLocked( wait );
        }
        if ( CAbstractFactoryEntryTracker::REMOVALWAITSTEP_DONE == step )
            return true;
        if ( CAbstractFactoryEntryTracker::REMOVALWAITSTEP_LEFT == step )
            return false;
        if ( !m_tracker.ContinueRemovalWait( wait, step, this->GetClassTypeName() ) )
            return false;
    }
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
void
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::EraseClassTypeNameLocked( const CString& classTypeName                ,
                                                                                                             const SelectionCriteriaType& selectedType )
{GUCEF_TRACE;

    typename TClassTypeNameMap::iterator n( m_concreteFactoryTypeMap.find( classTypeName ) );
    if ( n == m_concreteFactoryTypeMap.end() || !( (*n).second == selectedType ) )
        return;
    m_concreteFactoryTypeMap.erase( n );

    // Another registered concrete factory of the same concrete class
    typename TFactoryList::const_iterator i( m_concreteFactoryList.begin() );
    for ( ; i != m_concreteFactoryList.end(); ++i )
    {
        if ( CAbstractFactoryEntryTracker::GetConcreteClassTypeName( (*i).second ) == classTypeName )
        {
            m_concreteFactoryTypeMap[ classTypeName ] = (*i).first;
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
bool
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::TryGetSelectionCriteriaForClassTypeName( const CString& classTypeName       ,
                                                                                                                            SelectionCriteriaType& selectedType ) const
{GUCEF_TRACE;

    CRegistryScopeReadOnlyLock lock( *this );
    typename TClassTypeNameMap::const_iterator i( m_concreteFactoryTypeMap.find( classTypeName ) );
    if ( i == m_concreteFactoryTypeMap.end() )
        return false;
    selectedType = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
void
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::ObtainKeySet( TKeySet& keySet ) const
{GUCEF_TRACE;

    CRegistryScopeReadOnlyLock lock( *this );
    typename TFactoryList::const_iterator i( m_concreteFactoryList.begin() );
    while ( i != m_concreteFactoryList.end() )
    {
        keySet.insert( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
bool
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::IsConstructible( const SelectionCriteriaType& selectedType ) const
{GUCEF_TRACE;

    CRegistryScopeReadOnlyLock lock( *this );
    return m_concreteFactoryList.find( selectedType ) != m_concreteFactoryList.end();
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
bool
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::IsClassConstructible( const CString& classTypeName ) const
{GUCEF_TRACE;

    CRegistryScopeReadOnlyLock lock( *this );
    return m_concreteFactoryTypeMap.find( classTypeName ) != m_concreteFactoryTypeMap.end();
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
CString
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::GetConcreteClassTypeNameForSelectionCriterea( const SelectionCriteriaType& selectedType ) const
{GUCEF_TRACE;

    CRegistryScopeReadOnlyLock lock( *this );
    typename TFactoryList::const_iterator i( m_concreteFactoryList.find( selectedType ) );
    if ( i != m_concreteFactoryList.end() )
    {
        return static_cast< TFactory* >( CAbstractFactoryEntryTracker::GetFactory( (*i).second ) )->GetConcreteClassTypeName();
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
SelectionCriteriaType
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::GetSelectionCritereaForConcreteClassTypeName( const CString& classTypeName ) const
{GUCEF_TRACE;

    CRegistryScopeReadOnlyLock lock( *this );
    typename TClassTypeNameMap::const_iterator i( m_concreteFactoryTypeMap.find( classTypeName ) );
    if ( i != m_concreteFactoryTypeMap.end() )
    {
        return (*i).second;
    }
    return SelectionCriteriaType();
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
void
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::RegisterConcreteFactory( const SelectionCriteriaType& selectedType ,
                                                                                                            TFactory* concreteFactory                 )
{GUCEF_TRACE;

    const CString typeName( ToString( selectedType ) );
    TEntry* replacedEntry = GUCEF_NULL;
    CString concreteClassTypeName;
    {
        CRegistryScopeLock lock( *this );
        concreteClassTypeName = concreteFactory->GetConcreteClassTypeName();
        typename TFactoryList::iterator i = m_concreteFactoryList.find( selectedType );
        if ( i == m_concreteFactoryList.end() )
        {
            m_concreteFactoryList[ selectedType ] = m_tracker.NewEntryLocked( concreteFactory, typeName, concreteClassTypeName );
        }
        else
        {
            const CString previousClassTypeName = CAbstractFactoryEntryTracker::GetConcreteClassTypeName( (*i).second );
            if ( CAbstractFactoryEntryTracker::GetFactory( (*i).second ) != concreteFactory )
            {
                // The entry of the replaced concrete factory is removed: the creates under way use the concrete factory
                // they took, and the registry does not delete it
                replacedEntry = (*i).second;
                m_tracker.DisownEntryLocked( replacedEntry );
                (*i).second = m_tracker.NewEntryLocked( concreteFactory, typeName, concreteClassTypeName );
            }
            else
            {
                m_tracker.SetConcreteClassTypeNameLocked( (*i).second, concreteClassTypeName );
            }
            if ( previousClassTypeName != concreteClassTypeName )
                EraseClassTypeNameLocked( previousClassTypeName, selectedType );
        }
        m_concreteFactoryTypeMap[ concreteClassTypeName ] = selectedType;
    }

    // A create that took the replaced concrete factory before it was replaced may still use it
    if ( GUCEF_NULL != replacedEntry && WaitForCreatesOfRemovedEntry( replacedEntry ) )
        CAbstractFactoryEntryTracker::ReleaseEntry( replacedEntry );

    CAbstractFactoryEntryTracker::LogRegistration( typeName, concreteClassTypeName, GUCEF_NULL != replacedEntry );
    OnConcreteFactoryRegistered( selectedType );
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
void
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::UnregisterConcreteFactory( const SelectionCriteriaType& selectedType )
{GUCEF_TRACE;

    TEntry* entry = GUCEF_NULL;
    {
        CRegistryScopeLock lock( *this );
        typename TFactoryList::iterator i = m_concreteFactoryList.find( selectedType );
        if ( i == m_concreteFactoryList.end() )
            return;
        entry = (*i).second;
        m_concreteFactoryList.erase( i );
        EraseClassTypeNameLocked( CAbstractFactoryEntryTracker::GetConcreteClassTypeName( entry ), selectedType );
    }

    // A create that took the concrete factory before it was removed may still use it. The texts are taken first, as the
    // last create through the entry lets go of it when the removal does not wait.
    const CString typeName( CAbstractFactoryEntryTracker::GetTypeName( entry ) );
    const CString concreteClassTypeName( CAbstractFactoryEntryTracker::GetConcreteClassTypeName( entry ) );
    const bool waited = WaitForCreatesOfRemovedEntry( entry );
    if ( waited )
        CAbstractFactoryEntryTracker::ReleaseEntry( entry );

    CAbstractFactoryEntryTracker::LogUnregistration( typeName, concreteClassTypeName, waited );
    OnConcreteFactoryUnregistered( selectedType );
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
void
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::RemoveAllEntries( bool inDestructor )
{GUCEF_TRACE;

    TFactoryList removed;
    {
        CRegistryScopeLock lock( *this, true );
        removed.swap( m_concreteFactoryList );
        typename TFactoryList::iterator n = removed.begin();
        for ( ; n != removed.end(); ++n )
            EraseClassTypeNameLocked( CAbstractFactoryEntryTracker::GetConcreteClassTypeName( (*n).second ), (*n).first );
    }

    // A create that took a concrete factory before it was removed may still use it. Every entry is waited for and let
    // go of before any hook runs, so a hook that throws leaves no entry behind.
    typename TFactoryList::iterator i = removed.begin();
    for ( ; i != removed.end(); ++i )
    {
        TEntry* entry = (*i).second;
        (*i).second = GUCEF_NULL;
        const CString typeName( CAbstractFactoryEntryTracker::GetTypeName( entry ) );
        const CString concreteClassTypeName( CAbstractFactoryEntryTracker::GetConcreteClassTypeName( entry ) );
        const bool waited = WaitForCreatesOfRemovedEntry( entry );
        if ( waited )
            CAbstractFactoryEntryTracker::ReleaseEntry( entry );

        if ( !inDestructor )
            CAbstractFactoryEntryTracker::LogUnregistration( typeName, concreteClassTypeName, waited );
    }
    if ( !inDestructor )
        CAbstractFactoryEntryTracker::LogUnregistrationOfAll();

    CallUnregisteredHooks( removed, inDestructor );
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
void
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::CallUnregisteredHooks( const TFactoryList& removed ,
                                                                                                          bool inDestructor         )
{GUCEF_TRACE;

    typename TFactoryList::const_iterator i = removed.begin();
    for ( ; i != removed.end(); ++i )
    {
        try
        {
            OnConcreteFactoryUnregistered( (*i).first );
        }
        catch ( ... )
        {
            // A destructor passes no exception on, that would end the process
            if ( inDestructor )
            {
                CAbstractFactoryEntryTracker::LogExceptionFromRemovalHook( this->GetClassTypeName(), ToString( (*i).first ), true );
                continue;
            }

            // The hooks of the remaining entries still run, then the first exception is passed on
            typename TFactoryList::const_iterator n = i;
            for ( ++n; n != removed.end(); ++n )
            {
                try
                {
                    OnConcreteFactoryUnregistered( (*n).first );
                }
                catch ( ... )
                {
                    CAbstractFactoryEntryTracker::LogExceptionFromRemovalHook( this->GetClassTypeName(), ToString( (*n).first ), false );
                }
            }
            throw;
        }
    }
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
void
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::UnregisterAllConcreteFactories( void )
{GUCEF_TRACE;

    RemoveAllEntries( false );
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
void
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::OnConcreteFactoryRegistered( const SelectionCriteriaType& /* selectedType */ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
void
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::OnConcreteFactoryUnregistered( const SelectionCriteriaType& /* selectedType */ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
MT::TLockStatus
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    // Code outside the registry holds the registry lock: the hold is recorded, under the lock, so a removal on this
    // thread does not wait for creates that cannot end while it is held. A lock that was not taken (MT::CNoLock does not
    // apply) is not recorded.
    const MT::TLockStatus lockStatus = m_lock.Lock( lockWaitTimeoutInMs );
    if ( MT::LOCKSTATUS_OPERATION_SUCCESS == lockStatus || MT::LOCKSTATUS_ABANDONED == lockStatus )
        m_tracker.OnLockTakenByCallerLocked();
    return lockStatus;
}

/*-------------------------------------------------------------------------*/

template< typename SelectionCriteriaType, class TFactoryType, class LockType, class TBase >
MT::TLockStatus
CTAbstractFactoryRegistry< SelectionCriteriaType, TFactoryType, LockType, TBase >::Unlock( void ) const
{GUCEF_TRACE;

    // The record ends before the lock is released, while no other thread can take it
    m_tracker.OnLockReleasedByCallerLocked();
    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTABSTRACTFACTORYREGISTRY_H ? */
