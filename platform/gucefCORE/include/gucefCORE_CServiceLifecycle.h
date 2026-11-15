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

#ifndef GUCEF_CORE_CSERVICELIFECYCLE_H
#define GUCEF_CORE_CSERVICELIFECYCLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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

/** The states of a service process */
enum EServiceLifecycleState
{
    SERVICELIFECYCLE_STARTING = 0 ,     /**< process start: settings, the data directory, the endpoints that answer health */
    SERVICELIFECYCLE_LOCK_WAIT ,        /**< a single writer lock is held by another holder with a recent heartbeat; retried */
    SERVICELIFECYCLE_LOADING ,          /**< the lock is ours (or none is needed); stores and listeners are being opened */
    SERVICELIFECYCLE_READY ,            /**< serving; no condition is set */
    SERVICELIFECYCLE_DEGRADED ,         /**< serving with at least one condition (a database unreachable, a location offline) */
    SERVICELIFECYCLE_READ_ONLY ,        /**< the writer lock was lost: writes are refused, the process ends for a restart */
    SERVICELIFECYCLE_DRAINING ,         /**< a stop was requested: running work completes within the drain time, new work is refused */
    SERVICELIFECYCLE_STOPPED ,          /**< the end of an orderly stop */
    SERVICELIFECYCLE_FAILED             /**< the service cannot go on; the reason is kept */
};
typedef enum EServiceLifecycleState TServiceLifecycleState;

/** "STARTING", "LOCK_WAIT", "LOADING", "READY", "DEGRADED", "READ_ONLY", "DRAINING", "STOPPED", "FAILED" */
GUCEF_CORE_PUBLIC_CPP const char* ServiceLifecycleStateToString( TServiceLifecycleState state );

/** False for a text that is none of the names */
GUCEF_CORE_PUBLIC_CPP bool ServiceLifecycleStateFromString( const CString& text, TServiceLifecycleState& state );

/*-------------------------------------------------------------------------*/

/** What a state change was: the data of CServiceLifecycle::StateChangedEvent */
struct GUCEF_CORE_PUBLIC_CPP SServiceLifecycleChange
{
    TServiceLifecycleState from;
    TServiceLifecycleState to;
    CString reason;

    SServiceLifecycleChange( void );
};
typedef CTCloneableObj< SServiceLifecycleChange > TServiceLifecycleChangeEventData;

/*-------------------------------------------------------------------------*/

/**
 *  The state machine of a service process: the one place its state lives, and the rules of which state may follow:
 *      STARTING   -> LOCK_WAIT, LOADING, DRAINING, FAILED
 *      LOCK_WAIT  -> LOADING, DRAINING, FAILED
 *      LOADING    -> READY, DEGRADED, LOCK_WAIT, DRAINING, FAILED
 *      READY      -> DEGRADED, READ_ONLY, DRAINING, FAILED
 *      DEGRADED   -> READY, READ_ONLY, DRAINING, FAILED
 *      READ_ONLY  -> DRAINING, FAILED
 *      DRAINING   -> STOPPED, FAILED
 *      STOPPED, FAILED: final
 *  Conditions are named reasons for DEGRADED: while serving, setting the first condition moves READY to DEGRADED and
 *  clearing the last one moves it back. BeginDrain() moves to DRAINING with a deadline on the monotonic clock.
 *  Transition() refuses a change the rules do not allow and changes nothing. Thread safe: the state is read from the
 *  threads of the health endpoints. StateChangedEvent is raised after the change, outside the lock, on the thread
 *  that made it (through the CTSGNotifier: on the pulse generator given at construction when that is not the caller's).
 */
class GUCEF_CORE_PUBLIC_CPP CServiceLifecycle : public CTSGNotifier
{
    public:

    static const CEvent StateChangedEvent;          /**< TServiceLifecycleChangeEventData */
    static const CEvent ConditionsChangedEvent;     /**< no data: GetConditions() changed */

    static void RegisterEvents( void );

    /** Events go to the pulse generator; GUCEF_NULL: the global one */
    explicit CServiceLifecycle( PulseGeneratorPtr pulseGenerator = PulseGeneratorPtr() );
    virtual ~CServiceLifecycle() GUCEF_VIRTUAL_OVERRIDE;

    TServiceLifecycleState GetState( void ) const;

    /** STOPPED and FAILED: nothing follows */
    bool IsFinal( void ) const;

    /** READY or DEGRADED (and READ_ONLY for reads): whether clients are served */
    bool IsServing( void ) const;

    /** The reason of the last change, empty when it had none; for FAILED the reason of the failure */
    CString GetReason( void ) const;

    /** Milliseconds in the current state, and since construction (monotonic) */
    UInt64 GetStateAgeInMs( void ) const;
    UInt64 GetUptimeInMs( void ) const;

    /** True when the change was made; false (nothing changed) for a change the rules do not allow */
    bool Transition( TServiceLifecycleState to, const CString& reason = CString::Empty );

    /** FAILED with the reason from any state that is not final; false when it was final already */
    bool Fail( const CString& reason );

    /** Sets or replaces a condition (name: a code such as "DATABASE_UNAVAILABLE"; text: for people) */
    void SetCondition( const CString& name, const CString& text );

    /** Clears a condition; false when it was not set */
    bool ClearCondition( const CString& name );

    /** The conditions, name to text */
    CString::StringMap GetConditions( void ) const;

    /** DRAINING with the deadline now + drainMs; false when the state does not allow it */
    bool BeginDrain( const CString& reason, UInt32 drainMs );

    /** While DRAINING: milliseconds left of the drain time (0 when it is over); 0 in other states */
    UInt64 GetDrainRemainingMs( void ) const;

    /** While DRAINING: whether the drain time is over */
    bool IsDrainExpired( void ) const;

    /** Whether the rules allow the change; a state never follows itself */
    static bool IsTransitionAllowed( TServiceLifecycleState from, TServiceLifecycleState to );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CServiceLifecycle( const CServiceLifecycle& src );              /**< not implemented */
    CServiceLifecycle& operator=( const CServiceLifecycle& src );   /**< not implemented */

    bool TransitionLocked( TServiceLifecycleState to, const CString& reason, SServiceLifecycleChange& change );
    void RaiseChange( const SServiceLifecycleChange& change );

    mutable MT::CMutex m_stateLock;
    TServiceLifecycleState m_state;
    CString m_reason;
    UInt64 m_createdAtMs;
    UInt64 m_stateSinceMs;
    UInt64 m_drainDeadlineMs;
    CString::StringMap m_conditions;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSERVICELIFECYCLE_H ? */
