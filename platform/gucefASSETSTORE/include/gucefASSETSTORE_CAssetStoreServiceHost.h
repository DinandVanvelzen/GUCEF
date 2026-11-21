/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSTORESERVICEHOST_H
#define GUCEF_ASSETSTORE_CASSETSTORESERVICEHOST_H

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

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CISERVICERESOURCESOURCE_H
#include "gucefCORE_CIServiceResourceSource.h"
#define GUCEF_CORE_CISERVICERESOURCESOURCE_H
#endif /* GUCEF_CORE_CISERVICERESOURCESOURCE_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICEIMPL_H
#include "gucefCOMCORE_CRpcServiceImpl.h"
#define GUCEF_COMCORE_CRPCSERVICEIMPL_H
#endif /* GUCEF_COMCORE_CRPCSERVICEIMPL_H ? */

#ifndef GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#include "gucefCOMCORE_CTRpcMethodHandler.h"
#define GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#endif /* GUCEF_COMCORE_CTRPCMETHODHANDLER_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEHOST_H
#include "gucefDRNUTILS_CRemoteServiceHost.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEHOST_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEHOST_H ? */

#ifndef GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H
#include "gucefDRNUTILS_CCommandOutcomeCache.h"
#define GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H
#endif /* GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H ? */

#ifndef GUCEF_DRNUTILS_CCOMMANDJOURNAL_H
#include "gucefDRNUTILS_CCommandJournal.h"
#define GUCEF_DRNUTILS_CCOMMANDJOURNAL_H
#endif /* GUCEF_DRNUTILS_CCOMMANDJOURNAL_H ? */

#ifndef GUCEF_DRNUTILS_CUPLOADSTAGING_H
#include "gucefDRNUTILS_CUploadStaging.h"
#define GUCEF_DRNUTILS_CUPLOADSTAGING_H
#endif /* GUCEF_DRNUTILS_CUPLOADSTAGING_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTORESERVICEHOSTSETTINGS_H
#include "gucefASSETSTORE_CAssetStoreServiceHostSettings.h"
#include "gucefASSETSTORE_CAssetFileServingGate.h"
#define GUCEF_ASSETSTORE_CASSETSTORESERVICEHOSTSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESERVICEHOSTSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The server side of the remote asset store service (gucefASSETSTORE_RemoteService_Design.md, steps R2 and R3): it
 *  serves a CIAssetStoreService of this process (the in-process CLocalAssetStoreService) to the paired devices of a
 *  DRNUTILS::CRemoteServiceHost over the contracts of CAssetStoreRemoteContracts:
 *  - every call is carried out with CIAssetStoreService::Dispatch() on a thread of the RPC pool (the handlers block,
 *    the pulse thread never does); results travel in their DataNode form, failures as the status name and a message;
 *  - mutations: refused while the host drains; a command id is kept per device with the keyed hash of the request
 *    (DRNUTILS::CCommandOutcomeCache, and the monthly DRNUTILS::CCommandJournal on disk when a directory is set), so
 *    a repeat, also after a restart of the service, answers the first outcome (replayed) and a reuse for another request
 *    is refused; a repeat that arrives while the first one runs waits for it. The service itself gets the command id
 *    scoped to the device;
 *  - events of the service (catalogue changes, jobs with their progress at most every jobProgressIntervalMs, locations,
 *    stores, sign in prompts, thumbnails, notices) go to the event ring of every client session; a mutation answers
 *    once the events it caused are in the rings (two pulses of the host's pulse generator, at most eventBarrierMs), and
 *    every answer carries the next event sequence of the caller's session, so the client delivers them first;
 *  - files: blocks of at most maxReadBlockBytes as bytes; uploads through a DRNUTILS::CUploadStaging (a quota per
 *    device, SHA-256 checked at the commit) whose committed folder is ingested through the VFS mount of the staging;
 *  - secrets (the answer of a sign in prompt, the keyring code) come in a sensitive field and go to the service as
 *    AUTH::CSecretString; nothing of a call is logged but its operation.
 *  Access: only devices with one of the roles of the settings (default "owner") may call, and of those only the devices with a
 *  sign in role (default "owner"; never "mcp") may drive the sign in to the online stores and the keyring, and only their sessions
 *  receive the prompts of a sign in; a caller that is not an
 *  active paired device gets nothing (the host's access rules refuse it first).
 *  What the service does not offer it answers UNSUPPORTED with the reason (thumbnails and renders: R5; exports; local
 *  copies and offline pins, which are functions of a client; BeginUpload without the file list).
 *  Threading: Attach() and Detach() on the host's pulse thread; the handlers on the RPC pool; the events and the timer on
 *  the host's pulse generator.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreServiceHost : public CORE::CObservingNotifier ,
                                                          public CORE::CIServiceResourceSource
{
    public:

    CAssetStoreServiceHost( DRNUTILS::CRemoteServiceHost& host, const TAssetStoreServicePtr& service, const CAssetStoreServiceHostSettings& settings );

    virtual ~CAssetStoreServiceHost() GUCEF_VIRTUAL_OVERRIDE;

    /** The credential store that keeps the key of the command journal (provider "<serviceName>.host"); not owned, NULL for none */
    void SetCredentialStore( AUTH::ICredentialStore* store );

    /** A fixed key of the command journal (tests); without one and without a credential store the key is random per run */
    void SetJournalKey( const UInt8* key, UInt32 size );

    /** Registers the services and the access rules on the started host, opens the journal and the staging, follows the events. False with the reason */
    bool Attach( CORE::CString& error );

    /** Stops following the events, closes the journal and the staging; waits for the calls that run */
    void Detach( void );

    bool IsAttached( void ) const;

    const TAssetStoreServicePtr& GetService( void ) const;

    DRNUTILS::CRemoteServiceHost& GetHost( void );

    const CAssetStoreServiceHostSettings& GetSettings( void ) const;

    /** Calls being carried out now */
    UInt32 GetRunningCallCount( void ) const;

    /** Counters for an info page and the resource line; no names, no secrets */
    void GetInfo( CORE::CValueList& info ) const;

    // CIServiceResourceSource
    virtual CORE::CString GetResourceSourceName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetResourceCounters( CORE::CValueList& counters ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAssetStoreServiceHost > TEventCallback;
    typedef COMCORE::CTRpcMethodHandler< CAssetStoreServiceHost > THandler;

    struct SJobThrottle
    {
        TAssetJobState state;
        UInt64 lastSentMs;
        bool pending;
        CAssetJobEntry entry;
        SJobThrottle( void );
    };
    typedef GUCEF::map< CORE::CString, SJobThrottle > TJobThrottleMap;
    typedef GUCEF::map< CORE::CString, GUCEF::map< CORE::CString, UInt32 > > TUploadFileIndexMap;

    CAssetStoreServiceHost( const CAssetStoreServiceHost& src );                /**< not implemented */
    CAssetStoreServiceHost& operator=( const CAssetStoreServiceHost& src );     /**< not implemented */

    // the methods of the contracts
    COMCORE::TRpcHandlerResult OnQuery( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnCommand( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnSecret( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnFilesQuery( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnFilesCommand( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnReadBlock( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnWriteBlock( COMCORE::CRpcServerCall& call );

    COMCORE::TRpcHandlerResult HandleCall( COMCORE::CRpcServerCall& call, bool isCommand, bool filesContract );

    /** Answers a mutation from what the cache or the journal kept; true when it did (also for a refused reuse) */
    bool AnswerKnownCommand( COMCORE::CRpcServerCall& call, const DRNUTILS::CRemoteServicePrincipal& principal,
                             const CORE::CString& commandId, const CORE::CString& cacheHash, const CORE::CString& journalHash );

    void RememberCommand( const DRNUTILS::CRemoteServicePrincipal& principal, const CORE::CString& operation, const CORE::CString& commandId,
                          const CORE::CString& cacheHash, const CORE::CString& journalHash, const COMCORE::CRpcMessage& answer,
                          TAssetStoreStatus status, const CORE::CString& message, const CORE::CICloneable* result );

    /** Carries out a call for the device: what the host does itself (uploads, refusals), the rest through the service */
    TAssetStoreStatus Execute( const DRNUTILS::CRemoteServicePrincipal& principal, const CAssetServiceCall& call,
                               CORE::CICloneable*& result, CORE::CString& message );

    TAssetStoreStatus BeginUploadFiles( const DRNUTILS::CRemoteServicePrincipal& principal, const CAssetServiceCall& call, CAssetServiceResult& result );
    /** Whether the operation is one of the sign in and the keyring, which only the devices with a sign in role may use */
    static bool IsSignInOperation( const CORE::CString& operation );
    /** Whether the caller has one of the sign in roles of the settings (never the role "mcp" alone) */
    bool IsSignInPrincipal( const DRNUTILS::CRemoteServicePrincipal& principal ) const;
    /** Refuses the call of a device without a sign in role: counts it, logs the operation and the device and fails the call */
    void RefuseSignIn( COMCORE::CRpcServerCall& call, const DRNUTILS::CRemoteServicePrincipal& principal, const CORE::CString& operation );
    /**
     *  The role rule of the library sync (library sync design, section 7.1): a sync run is started, controlled and given
     *  marks and profiles by the devices with the owner role, never by the machine role; a dry run may also be started by
     *  the machine role; the reads (marks, profile, the plan in the job result) are for every role. Empty when the call is
     *  allowed, otherwise why not
     */
    CORE::CString GetSyncRefusal( const DRNUTILS::CRemoteServicePrincipal& principal, const CAssetServiceCall& call );
    /** Whether the caller has one of the learning roles of the settings: a machine learning consumer */
    bool IsLearningPrincipal( const DRNUTILS::CRemoteServicePrincipal& principal ) const;
    /** The caller as the licence gate of the files and thumbnails sees it */
    CAssetFileServingGate::SCaller MakeGateCaller( const DRNUTILS::CRemoteServicePrincipal& principal ) const;
    TAssetStoreStatus QueryUpload( const DRNUTILS::CRemoteServicePrincipal& principal, const CAssetServiceCall& call, CAssetFileStatPage& result, CORE::CString& message );
    TAssetStoreStatus CommitUpload( const DRNUTILS::CRemoteServicePrincipal& principal, const CAssetServiceCall& call, CAssetServiceResult& result );
    TAssetStoreStatus AbortUpload( const DRNUTILS::CRemoteServicePrincipal& principal, const CAssetServiceCall& call, CAssetServiceResult& result );

    /** Waits until the events a mutation caused went through the host's pulse thread (two pulses), at most eventBarrierMs */
    void WaitForEventBarrier( void );

    UInt64 GetNextEventSeq( const DRNUTILS::CRemoteServicePrincipal& principal ) const;

    bool BeginCommand( const CORE::CString& key );
    void EndCommand( const CORE::CString& key );

    void Publish( const char* kind, const CORE::CIDataNodeSerializable* record, const CORE::CString::StringVector& ids, UInt64 revision, bool signInSessionsOnly = false );

    void OnServiceEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    static TAssetStoreStatus StatusOfStagingDetail( const CORE::CString& detailCode );
    static UInt64 NowMs( void );

    private:

    DRNUTILS::CRemoteServiceHost& m_host;
    TAssetStoreServicePtr m_service;
    CAssetFileServingGate m_fileGate;
    CAssetStoreServiceHostSettings m_settings;
    AUTH::ICredentialStore* m_credentials;
    COMCORE::CRpcServiceImpl m_serviceImpl;
    COMCORE::CRpcServiceImpl m_filesImpl;
    DRNUTILS::CCommandOutcomeCache m_commands;
    DRNUTILS::CCommandJournal m_journal;
    DRNUTILS::CUploadStaging m_staging;
    CORE::CString m_stagingMountName;
    TUploadFileIndexMap m_uploadIndexes;
    TJobThrottleMap m_jobs;
    CORE::CString::StringSet m_inFlight;
    CORE::CTimer m_timer;
    MT::CTAtomic< UInt64 > m_pulses;
    MT::CTAtomic< UInt32 > m_running;
    MT::CTAtomic< UInt64 > m_calls;
    MT::CTAtomic< UInt64 > m_commandCount;
    MT::CTAtomic< UInt64 > m_replays;
    MT::CTAtomic< UInt64 > m_refusals;
    MT::CTAtomic< UInt64 > m_bytesRead;
    MT::CTAtomic< UInt64 > m_bytesWritten;
    MT::CTAtomic< UInt64 > m_eventsPublished;
    UInt64 m_lastExpiryMs;
    bool m_hasJournalKey;
    bool m_attached;
    bool m_journalOpen;
    bool m_stagingOpen;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTORESERVICEHOST_H ? */
