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

#ifndef GUCEF_COMCORE_CSERVICEFINDER_H
#define GUCEF_COMCORE_CSERVICEFINDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTLIST_H
#include "gucefCOMCORE_CDiscoveryClientList.h"
#define GUCEF_COMCORE_CDISCOVERYCLIENTLIST_H
#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTLIST_H ? */

#ifndef GUCEF_COMCORE_CSERVICECANDIDATE_H
#include "gucefCOMCORE_CServiceCandidate.h"
#define GUCEF_COMCORE_CSERVICECANDIDATE_H
#endif /* GUCEF_COMCORE_CSERVICECANDIDATE_H ? */

#ifndef GUCEF_COMCORE_CSERVICEFINDERSETTINGS_H
#include "gucefCOMCORE_CServiceFinderSettings.h"
#define GUCEF_COMCORE_CSERVICEFINDERSETTINGS_H
#endif /* GUCEF_COMCORE_CSERVICEFINDERSETTINGS_H ? */

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

/**
 *  Finds services of a type on the local network (announced with CServiceAdvertiser): asks with every discovery client
 *  its settings name and keeps what they hear as candidates, merged by service type and instance id across the
 *  clients. A candidate goes when the last client lost it (its TTL ran out or the service said goodbye).
 *
 *  Candidates are offers, not trust: show them next to the configured address of the service, and authenticate
 *  whatever answers at their endpoints (for example by the pinned key of a paired service). The filter of the settings
 *  keeps only matching candidates, for example only the service with the fingerprint of a pinned key.
 *
 *  Two ways to use it:
 *  - as an object: Start(), the events, GetCandidates(), Stop(). Its clients run on the pulse generator given at
 *    construction, else the global one; all calls on the thread of that pulse generator.
 *  - once: FindOnce() looks for findTimeoutMs on the calling thread with a pulse generator of its own and returns
 *    the candidates; FindOnceAsync() does the same as a task on a thread pool and returns a future, whose result
 *    GetFindOnceResult() reads.
 */
class GUCEF_COMCORE_EXPORT_CPP CServiceFinder : public CORE::CObservingNotifier ,
                                                public virtual CORE::CIConfigurable
{
    public:

    static const CORE::CEvent CandidateFoundEvent;      /**< TServiceCandidateEventData: a new candidate */
    static const CORE::CEvent CandidateUpdatedEvent;    /**< TServiceCandidateEventData: a known candidate with changed content */
    static const CORE::CEvent CandidateLostEvent;       /**< TServiceCandidateEventData: the last client lost the candidate */

    static void RegisterEvents( void );

    typedef CDiscoveryClientList::TDiscoveryClientVector TDiscoveryClientVector;
    typedef GUCEF::vector< CServiceCandidate > TServiceCandidateVector;

    CServiceFinder( void );
    explicit CServiceFinder( const CORE::PulseGeneratorPtr& pulseGenerator );
    explicit CServiceFinder( const CServiceFinderSettings& settings );
    CServiceFinder( const CORE::PulseGeneratorPtr& pulseGenerator, const CServiceFinderSettings& settings );
    virtual ~CServiceFinder();

    /** Starts looking for the service type of the settings; what the clients know already is reported at once */
    bool Start( void );

    /** Stops looking; the candidates are forgotten without events */
    void Stop( void );

    /** Asks the network again */
    bool Refresh( void );

    bool IsActive( void ) const;

    void GetCandidates( TServiceCandidateVector& candidates ) const;
    UInt32 GetCandidateCount( void ) const;
    bool GetCandidate( const CORE::CString& instanceId, CServiceCandidate& candidate ) const;

    /** Applies to the candidates heard from now on; candidates that no longer match are reported lost */
    void SetFilter( const CServiceCandidateFilter& filter );

    /** Looks with the client too, also when the finder runs already. Clients added by hand replace the default types */
    bool AddDiscoveryClient( CIDiscoveryClientPtr client );
    const TDiscoveryClientVector& GetDiscoveryClients( void ) const;

    void SetSettings( const CServiceFinderSettings& settings );
    const CServiceFinderSettings& GetSettings( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Looks for the service type of the settings until findMinCandidates candidates are known or findTimeoutMs has
     *  passed. Blocks the calling thread and does all of the work on it, with a pulse generator of its own.
     *
     *  @return whether findMinCandidates candidates were found (with findMinCandidates 0: whether the look could start);
     *          the candidates found are given either way
     */
    static bool FindOnce( const CServiceFinderSettings& settings, TServiceCandidateVector& candidates );

    /**
     *  FindOnce() as a task on the given thread pool, else on the pool named in the settings, else on the default
     *  pool. The future ends when the look ends; GetFindOnceResult() reads its candidates.
     */
    static CORE::CFutureResult FindOnceAsync( const CServiceFinderSettings& settings, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() );

    /**
     *  The result of a future of FindOnceAsync() that is ready.
     *
     *  @return false while the future is not ready or when it did not come from FindOnceAsync(); found tells whether
     *          FindOnce() reached its minimum
     */
    static bool GetFindOnceResult( const CORE::CFutureResult& future, TServiceCandidateVector& candidates, bool& found );

    private:

    typedef CORE::CTEventHandlerFunctor< CServiceFinder > TEventCallback;

    typedef GUCEF::set< CIDiscoveryClient* > TClientSet;

    struct SKnownCandidate
    {
        CServiceCandidate candidate;
        TClientSet sources;
    };
    typedef struct SKnownCandidate TKnownCandidate;
    typedef GUCEF::map< CORE::CString, TKnownCandidate > TKnownCandidateMap;

    CServiceFinder( const CServiceFinder& src );
    CServiceFinder& operator=( const CServiceFinder& src );

    bool EnsureClients( void );
    bool PrepareClient( CIDiscoveryClient& client ) const;
    bool StartClient( CIDiscoveryClient& client );
    void SubscribeToClient( CIDiscoveryClient& client );
    CIDiscoveryClient* FindClient( CORE::CNotifier* notifier );
    CORE::CString GetBrowsedServiceType( void ) const;

    bool OnEntryFound( CIDiscoveryClient* client, const CIDiscoveryEntry& entry );
    bool OnEntryLost( CIDiscoveryClient* client, const CIDiscoveryEntry& entry );

    void OnClientEntryFound( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnClientEntryLost( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnClientError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    CORE::PulseGeneratorPtr m_pulseGenerator;
    CServiceFinderSettings m_settings;
    TDiscoveryClientVector m_clients;
    bool m_clientsCreated;
    bool m_active;
    CORE::CString m_browsedServiceType;
    TKnownCandidateMap m_candidates;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CSERVICEFINDER_H ? */
