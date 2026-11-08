/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMCPREGISTRYBASE_H
#define GUCEF_COM_CMCPREGISTRYBASE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_COM_MCPTYPES_H
#include "gucefCOM_McpTypes.h"
#define GUCEF_COM_MCPTYPES_H
#endif /* GUCEF_COM_MCPTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the tool, resource and prompt registries share: the lock, the generation and the coalesced list change.
 *
 *  Every change increments the generation, so a holder of a list built from the registry can tell it is stale.
 *  ListChangedEvent is raised once on the next pulse of the pulse generator of the registry after one or more
 *  changes, from the thread that drives it: a plugin that registers twenty tools while it loads causes one.
 *
 *  Thread safety: an MT::CReadWriteLock. Reads take the shared lock and copy entry pointers, so a call that is
 *  running keeps its entry alive after an unregister and never holds the lock while it runs.
 *  Every event is raised on the thread that drives the pulse generator: a registration made on another thread has
 *  its registered or unregistered event queued and raised on the next pulse, in order, since a notifier is not to
 *  notify from two threads at once.
 */
class GUCEF_COM_PUBLIC_CPP CMcpRegistryBase : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ListChangedEvent;         /**< no event data */

    typedef CORE::TCloneableString TNameEventData;      /**< the name or URI of what registered or unregistered */

    static void RegisterEvents( void );

    /** @param pulseGenerator   the pulse generator whose thread raises ListChangedEvent; empty: the application's */
    explicit CMcpRegistryBase( const CORE::PulseGeneratorPtr& pulseGenerator = CORE::PulseGeneratorPtr() );

    virtual ~CMcpRegistryBase() GUCEF_VIRTUAL_OVERRIDE;

    /** Incremented by every change */
    UInt64 GetGeneration( void ) const;

    /** Whether a change has not been announced with ListChangedEvent yet */
    bool IsListChangePending( void ) const;

    /** Raises ListChangedEvent now when a change is pending, instead of on the next pulse. Call it on the pulse thread */
    void FlushListChanged( void );

    CORE::PulseGeneratorPtr GetPulseGenerator( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** To be called with the writer lock held, after a change */
    UInt64 OnChangedWhileLocked( void );

    /** Raises a name event now on the pulse thread, or queues it for the next pulse on any other thread */
    void RaiseNameEvent( const CORE::CEvent& eventId, const CORE::CString& name );

    bool IsPulseThread( void ) const;

    MT::CReadWriteLock m_rwLock;

    private:

    CMcpRegistryBase( const CMcpRegistryBase& src );
    CMcpRegistryBase& operator=( const CMcpRegistryBase& src );

    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void RaiseQueuedEvents( void );

    struct SQueuedEvent
    {
        CORE::CEvent eventId;
        CORE::CString name;
    };
    typedef GUCEF::vector< SQueuedEvent > TQueuedEventVector;

    CORE::PulseGeneratorPtr m_pulseGenerator;
    MT::CMutex m_queueLock;
    TQueuedEventVector m_queuedEvents;
    MT::CTAtomic< TThreadId > m_pulseThreadId;            /**< the thread that pulses, as the last pulse found it */
    GUCEF_SYSTEM_THREAD_ID_CODE( MT::CTAtomic< TSystemThreadId > m_pulseSystemThreadId; )   /**< display only, recorded with m_pulseThreadId */
    UInt64 m_generation;
    MT::CTAtomic< UInt32 > m_changePending;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPREGISTRYBASE_H ? */
