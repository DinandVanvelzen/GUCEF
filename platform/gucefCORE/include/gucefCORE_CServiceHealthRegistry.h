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

#ifndef GUCEF_CORE_CSERVICEHEALTHREGISTRY_H
#define GUCEF_CORE_CSERVICEHEALTHREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CISERVICEHEALTHCHECK_H
#include "gucefCORE_CIServiceHealthCheck.h"
#define GUCEF_CORE_CISERVICEHEALTHCHECK_H
#endif /* GUCEF_CORE_CISERVICEHEALTHCHECK_H ? */

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

/**
 *  The health of a service process as the probes of a container orchestrator ask for it (Kubernetes liveness,
 *  readiness and startup probes), kept apart from how it is served (gucefWEB serves it over HTTP):
 *
 *  - liveness:  the thread that drives the pulse generator of the registry evaluated the checks within the
 *               liveness staleness window (default 10 s). A hung pulse thread fails liveness without a check of
 *               its own. Liveness never depends on an outside dependency, so a database that is down never gets
 *               the process restarted.
 *  - readiness: the service said it started (SetStarted()), the cached outcome of every readiness check passes
 *               (WARN passes) and the cache is not older than the staleness window.
 *  - startup:   the service said it started.
 *
 *  The checks are evaluated on the pulse thread every check interval (default 1 s) and their outcome is cached with
 *  the time of its last change; GetLiveness(), GetReadiness() and GetStartup() read the cache only, from any thread.
 *  The registry starts to listen to the pulse generator on first use, so a process that does not use it pays nothing.
 *
 *  One registry per process is owned by CCoreGlobal (GetServiceHealthRegistry()), on the global pulse generator.
 *  Further instances are for tests and for processes that serve the health of several parts apart.
 *  Thread safe.
 */
class GUCEF_CORE_PUBLIC_CPP CServiceHealthRegistry : public CObserver
{
    public:

    static const UInt32 DefaultCheckIntervalInMs;   /**< 1000 */
    static const UInt32 DefaultStalenessInMs;       /**< 10000 */

    /** The cached outcome of one check */
    struct SCheckOutcome
    {
        CString name;
        TServiceHealthState state;
        CString detail;
        UInt64 sinceUtcMs;              /**< when the check got this state, ms since the Unix epoch (UTC) */

        SCheckOutcome( void ) : name(), state( HEALTH_FAIL ), detail(), sinceUtcMs( 0 ) {}
    };
    typedef GUCEF::vector< SCheckOutcome > TCheckOutcomeVector;

    /** The answer to one of the probes */
    struct SHealthReport
    {
        TServiceHealthState state;      /**< the probe passes when this is not HEALTH_FAIL */
        bool isStale;                   /**< the checks were not evaluated within the staleness window */
        bool isStarted;                 /**< what SetStarted() said last */
        UInt64 evaluatedAtUtcMs;        /**< when the checks were evaluated last, ms since the Unix epoch (UTC); 0 = never */
        TCheckOutcomeVector checks;     /**< readiness: the outcome of every check */

        SHealthReport( void ) : state( HEALTH_FAIL ), isStale( true ), isStarted( false ), evaluatedAtUtcMs( 0 ), checks() {}

        bool Passes( void ) const { return HEALTH_FAIL != state; }
    };

    CServiceHealthRegistry( void );

    explicit CServiceHealthRegistry( PulseGeneratorPtr pulseGenerator );

    virtual ~CServiceHealthRegistry();

    /** The pulse generator on whose thread the checks are evaluated */
    void SetPulseGenerator( PulseGeneratorPtr pulseGenerator );

    PulseGeneratorPtr GetPulseGenerator( void ) const;

    /** Adds a check; the caller keeps ownership and unregisters it before it goes. False for GUCEF_NULL or a check that is registered already */
    bool RegisterReadinessCheck( CIServiceHealthCheck* check );

    /**
     *  Removes a check. When the checks are being evaluated on another thread at that moment, waits until that is
     *  done, so the check may be destroyed once this returns. From within a Check() (itself or another check) it
     *  returns at once: a check that is no longer registered is not called again and its outcome is dropped
     */
    bool UnregisterReadinessCheck( CIServiceHealthCheck* check );

    UInt32 GetReadinessCheckCount( void ) const;

    /** Whether the service finished its start; startup and readiness fail until it says so */
    void SetStarted( bool started );

    bool IsStarted( void ) const;

    /** What /info shows: version, build time, start time, host and application specific fields. No secrets */
    void SetInfo( const CValueList& info );

    CValueList GetInfo( void ) const;

    void SetCheckIntervalInMs( UInt32 checkIntervalInMs );

    UInt32 GetCheckIntervalInMs( void ) const;

    /** How old the last evaluation may be before liveness and readiness fail; 0 means it never goes stale */
    void SetStalenessInMs( UInt32 stalenessInMs );

    UInt32 GetStalenessInMs( void ) const;

    /** Evaluates every readiness check now, on the calling thread, and caches the outcome. The pulse does this at the check interval */
    void EvaluateChecks( void );

    void GetLiveness( SHealthReport& report ) const;

    void GetReadiness( SHealthReport& report ) const;

    void GetStartup( SHealthReport& report ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::vector< CIServiceHealthCheck* > TCheckVector;
    typedef CTEventHandlerFunctor< CServiceHealthRegistry > TEventCallback;

    CServiceHealthRegistry( const CServiceHealthRegistry& src );            /**< not implemented */
    CServiceHealthRegistry& operator=( const CServiceHealthRegistry& src ); /**< not implemented */

    void OnPulse( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData );

    void ListenToPulses( void );

    bool IsStaleWhileLocked( void ) const;

    void FillCommonWhileLocked( SHealthReport& report ) const;

    private:

    mutable MT::CMutex m_dataLock;          /**< everything below except the evaluation itself */
    MT::CMutex m_evaluationLock;            /**< held while the checks run, so an unregistered check is not in use */
    PulseGeneratorPtr m_pulseGenerator;
    bool m_listening;
    TCheckVector m_checks;
    TCheckOutcomeVector m_outcomes;         /**< in the order of m_checks */
    bool m_started;
    CValueList m_info;
    UInt32 m_checkIntervalInMs;
    UInt32 m_stalenessInMs;
    bool m_evaluated;
    UInt64 m_lastEvaluationTicks;           /**< MT::PrecisionTickCount() of the last evaluation */
    UInt64 m_lastEvaluationUtcMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSERVICEHEALTHREGISTRY_H ? */
