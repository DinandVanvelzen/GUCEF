/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CSOCKETREADINESSWAITER_H
#define GUCEF_COMCORE_CSOCKETREADINESSWAITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CSocketReadinessWaiter;
/*-------------------------------------------------------------------------*/

/**
 *  Wakes the owners of sockets when their sockets become ready.
 *
 *  A socket is serviced at the pulses of its pulse generator. When that pulse generator belongs to a dedicated task,
 *  the task sleeps between its cycles and nothing ends that sleep when bytes arrive. The waiter is one task of the
 *  task manager (pool "gucef.comcore.readiness", no raw thread) that waits in the operating system for readiness of
 *  the sockets registered with it (epoll on Linux and Android, WSAPoll on Windows) and asks the pulse generator of
 *  each ready socket for an immediate pulse (CPulseGenerator::RequestImmediatePulseThroughDriver()). The socket is
 *  then serviced by its owner, on the owner's thread, as at any other pulse: the waiter never reads or writes.
 *
 *  Arming is one shot: Arm() asks for one notification; when it came the owner arms again after servicing the
 *  socket. A socket that stays readable is woken once per service pass, never in a loop of its own. Registrations
 *  are identified by ids that are never reused, so a descriptor number reused by a new socket is never mistaken for
 *  the old one. The registry lock is a leaf: nothing is called out under it.
 *
 *  The waiter only shortens waits: sockets keep their periodic pulses, so when the waiter is not available
 *  (platform without a backend, shut down, task refused) they are serviced as without it.
 *  Meant for pulse generators driven by a task delegator, see RequestImmediatePulseThroughDriver().
 */
class GUCEF_COMCORE_EXPORT_CPP CSocketReadinessWaiter : public CORE::CTaskConsumer ,
                                                         public CORE::CTSharedObjCreator< CSocketReadinessWaiter, MT::CMutex >
{
    public:

    typedef CORE::CTSharedObjCreator< CSocketReadinessWaiter, MT::CMutex >::TSharedPtrType  TSharedPtrType;

    typedef UInt64 TRegistrationId;     /**< 0 means none */

    static const UInt32 ReadinessRead;          /**< readable, also an error or a close of the peer */
    static const UInt32 ReadinessWrite;         /**< writable, also the end of a connect in progress */
    static const CORE::CString TaskType;
    static const CORE::CString ThreadPoolName;  /**< "gucef.comcore.readiness" */
    static const UInt32 WaitTimeoutInMs;        /**< longest OS wait, the time a task stopped from outside needs to end */

    /** Use CreateSharedObj(): the task holds a reference while it runs */
    CSocketReadinessWaiter( void );

    virtual ~CSocketReadinessWaiter() GUCEF_VIRTUAL_OVERRIDE;

    /** Whether this platform has a backend and the waiter was not shut down */
    bool IsAvailable( void ) const;

    /**
     *  Registers a socket, disarmed. Starts the waiter's task at the first registration.
     *
     *  @param socketDescriptor the operating system's socket descriptor (a SOCKET on Windows)
     *  @param owner the pulse generator that services the socket, driven by a task delegator
     *  @return the id of the registration, 0 when the waiter is not available
     */
    TRegistrationId Register( UInt64 socketDescriptor, const CORE::PulseGeneratorPtr& owner );

    /**
     *  Asks for one wake of the owner when the socket has any of the given readiness (ReadinessRead and / or
     *  ReadinessWrite). Readiness that is already there wakes at once.
     *
     *  @return false for an id that is not registered
     */
    bool Arm( TRegistrationId registrationId, UInt32 interest );

    /** Forgets the registration; call before the descriptor is closed. An id of 0 or an unknown id does nothing */
    void Unregister( TRegistrationId registrationId );

    /**
     *  Declares that every socket on the pulse generator uses the readiness wake, whatever its own setting
     *  (SetUseReadinessWake()), from its next connect, listen or open. For the owner of the pulse generator's driver,
     *  which knows that the driver takes pulse requests from other threads without pulsing inside them (a dedicated
     *  task that ends its wait on such a request). Holds a reference until DisableForPulseGenerator().
     */
    void EnableForPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator );

    /** Ends EnableForPulseGenerator(); registrations made meanwhile stay until their sockets close */
    void DisableForPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator );

    bool IsEnabledForPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator ) const;

    /** Drops every registration, ends the wait and stops the task. Registrations are refused from then on */
    void Shutdown( void );

    /** The pulse requests the waiter made */
    UInt64 GetWakeCount( void ) const;

    /** The returns from the operating system wait, timeouts included */
    UInt64 GetWaitCount( void ) const;

    UInt32 GetRegistrationCount( void ) const;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SRegistration;
    struct SBackend;
    typedef GUCEF::map< TRegistrationId, SRegistration* >  TRegistrationMap;
    typedef GUCEF::vector< CORE::PulseGeneratorPtr >  TPulseGeneratorVector;
    typedef GUCEF::vector< TRegistrationId >  TRegistrationIdVector;
    typedef GUCEF::map< const CORE::CPulseGenerator*, CORE::PulseGeneratorPtr >  TPulseGeneratorMap;

    CSocketReadinessWaiter( const CSocketReadinessWaiter& src );                /**< not implemented */
    CSocketReadinessWaiter& operator=( const CSocketReadinessWaiter& src );     /**< not implemented */

    bool EnsureTaskStarted( void );

    /** One operating system wait and the wakes it gives, on the waiter's task */
    void WaitOnce( UInt32 timeoutInMs );

    /** Ends a wait of the waiter's task, from any thread */
    void WakeWaiter( void );

    void ReleaseBackend( void );

    SBackend* m_backend;
    mutable MT::CMutex m_registryLock;      /**< leaf: guards the registrations and the backend's interest set */
    MT::CMutex m_taskLock;                  /**< guards the start and stop of the task, never taken under the registry lock */
    TRegistrationMap m_registrations;
    TPulseGeneratorMap m_enabledPulseGenerators;    /**< guarded by the registry lock */
    TRegistrationId m_lastRegistrationId;
    TRegistrationIdVector m_readyIds;               /**< used by the waiter's task only */
    TPulseGeneratorVector m_pulseGeneratorsToWake;  /**< used by the waiter's task only */
    MT::CTAtomic< bool > m_shutdown;
    MT::CTAtomic< bool > m_taskRunning;
    MT::CTAtomic< UInt64 > m_wakeCount;
    MT::CTAtomic< UInt64 > m_waitCount;
};

/*-------------------------------------------------------------------------*/

typedef CSocketReadinessWaiter::TSharedPtrType  CSocketReadinessWaiterPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CSOCKETREADINESSWAITER_H ? */
