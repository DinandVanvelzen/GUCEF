/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CKEYRINGSESSION_H
#define GUCEF_AUTH_CKEYRINGSESSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

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

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_AUTH_CFILECREDENTIALSTORE_H
#include "gucefAUTH_CFileCredentialStore.h"
#define GUCEF_AUTH_CFILECREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CFILECREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CKEYRINGSETTINGS_H
#include "gucefAUTH_CKeyringSettings.h"
#define GUCEF_AUTH_CKEYRINGSETTINGS_H
#endif /* GUCEF_AUTH_CKEYRINGSETTINGS_H ? */

#ifndef GUCEF_AUTH_CKEYRINGEVENTDATA_H
#include "gucefAUTH_CKeyringEventData.h"
#define GUCEF_AUTH_CKEYRINGEVENTDATA_H
#endif /* GUCEF_AUTH_CKEYRINGEVENTDATA_H ? */

#ifndef GUCEF_AUTH_CKEYRINGOPERATIONDATA_H
#include "gucefAUTH_CKeyringOperationData.h"
#define GUCEF_AUTH_CKEYRINGOPERATIONDATA_H
#endif /* GUCEF_AUTH_CKEYRINGOPERATIONDATA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CKeyringOperationTaskConsumer;

/*-------------------------------------------------------------------------*/

/**
 *  The unlock session of a credential store that is sealed with a keyring code (CFileCredentialStore in KEYMODE_KEYRING_CODE):
 *  what an application and its forms use to unlock it once per session, lock it again and change or forget the code.
 *
 *  Unlock. UnlockWithCode() is the blocking form (the derivation runs on the calling thread, for a tool without a user
 *  interface or a worker thread); UnlockWithCodeAsync() runs it on a thread of the pool (SetThreadPool(), else the default
 *  pool of the task manager) and gives a future that ends when it is done; the outcome is GetLastResult() and the events.
 *  The same holds for SetUpCode() and ChangeCode(). One operation with a code runs at a time: another one is KEYRING_BUSY.
 *
 *  Futures. The future of an operation that ran ends TASKSTATUS_FINISHED and holds the result of the operation; one that
 *  was refused without starting (KEYRING_BUSY, KEYRING_THROTTLED, no store) or could not be queued ends in an error status
 *  (HasNoFuture()) at once and holds why. A future that has ended is not a refusal: on a thread of the pool the operation
 *  can be done before the call returns. GetResultOfFuture() gives the result of either.
 *
 *  Wrong codes. A wrong code is reported as such (WrongCodeEvent), apart from tampering (FailedEvent with KEYRING_TAMPERED).
 *  After CKeyringSettings::freeAttempts wrong codes in a row every further attempt waits (KEYRING_THROTTLED without trying,
 *  GetRetryWaitInMs() says how long), doubling up to maxRetryDelayInMs; RetryAllowedEvent says when the wait is over. The
 *  wait is a time stamp, nothing sleeps: the thread of a form is never held up. The count is per session, in memory.
 *
 *  Locking. The keyring locks on LockNow(), after idleLockInSeconds without use of the key by the store (any read or change
 *  counts; 0 is never), when the application ends (CGUCEFApplication::AppShutdownEvent), on Shutdown() and when the session
 *  goes; the store also locks itself when another instance sealed the file with another code or it was forgotten.
 *  LockedEvent says why.
 *
 *  Threads. The state can be asked from any thread. The events are raised on the thread that pumps the pulse generator of
 *  the timer of the session (the outcome of an asynchronous operation at its next tick, within checkIntervalInMs) and, for
 *  the blocking forms, on the calling thread when they return. The store is not owned: it outlives the session or is taken
 *  away with GUCEF_NULL first; the session waits for an operation that runs before it lets go of the store.
 *
 *  Secrets. The codes are CSecretString, wiped when the operation is done; nothing of a code or of the derived key is
 *  logged, kept by the session or part of an event.
 */
class GUCEF_AUTH_EXPORT_CPP CKeyringSession : public CORE::CObservingNotifier ,
                                              public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent UnlockedEvent;        /**< CKeyringEventData: the keyring is unlocked (an unlock, a set up) */
    static const CORE::CEvent LockedEvent;          /**< CKeyringEventData: the keyring locked, lockReason says why */
    static const CORE::CEvent WrongCodeEvent;       /**< CKeyringEventData: a wrong code; failedAttempts and retryWaitInMs */
    static const CORE::CEvent RetryAllowedEvent;    /**< CKeyringEventData: the wait after wrong codes is over */
    static const CORE::CEvent CodeSetUpEvent;       /**< CKeyringEventData: a code was set up, the keyring is new and unlocked */
    static const CORE::CEvent CodeChangedEvent;     /**< CKeyringEventData: the code was changed, the keyring was sealed again */
    static const CORE::CEvent ForgottenEvent;       /**< CKeyringEventData: the keyring and its saved credentials were deleted */
    static const CORE::CEvent FailedEvent;          /**< CKeyringEventData: an operation failed for another reason than a wrong code: result */

    static void RegisterEvents( void );

    CKeyringSession( void );

    /** A session of the store, not owned */
    explicit CKeyringSession( CFileCredentialStore* store );

    virtual ~CKeyringSession() GUCEF_VIRTUAL_OVERRIDE;

    /** The store, not owned; GUCEF_NULL lets go of it (after an operation that runs). Locks the store it lets go of */
    void SetStore( CFileCredentialStore* store );
    CFileCredentialStore* GetStore( void ) const;

    void SetSettings( const CKeyringSettings& settings );
    CKeyringSettings GetSettings( void ) const;

    /** The pool of the asynchronous operations; a NULL pointer is the default pool of the task manager */
    void SetThreadPool( CORE::ThreadPoolPtr threadPool );

    /** @name State: any thread */
    ///@{
    bool IsLocked( void ) const;            /**< no store, or the keyring of the store is locked */
    bool IsSetUp( void ) const;             /**< the store has a keyring file */
    bool IsBusy( void ) const;              /**< an operation with a code runs */
    UInt32 GetFailedAttemptCount( void ) const;
    UInt32 GetRetryWaitInMs( void ) const;  /**< how long the next attempt has to wait after wrong codes, 0 for none */
    TKeyringResult GetLastResult( void ) const;
    ///@}

    /** @name Operations with a code */
    ///@{
    // the asynchronous forms are virtual: a derived session can wrap them, for example a test double that waits for the outcome
    TKeyringResult UnlockWithCode( const CSecretString& code );
    virtual CORE::CFutureResult UnlockWithCodeAsync( const CSecretString& code );

    /** Sets up the first code: an empty keyring, unlocked */
    TKeyringResult SetUpCode( const CSecretString& code );
    virtual CORE::CFutureResult SetUpCodeAsync( const CSecretString& code );

    /** Changes the code: the current one is required, also while unlocked; wrong current codes count as wrong codes */
    TKeyringResult ChangeCode( const CSecretString& currentCode, const CSecretString& newCode );
    virtual CORE::CFutureResult ChangeCodeAsync( const CSecretString& currentCode, const CSecretString& newCode );

    /**
     *  The result that the future of an asynchronous operation holds once it ended: the result of the operation when it
     *  ran (TASKSTATUS_FINISHED), why it did not start when it was refused (HasNoFuture()); KEYRING_FAILED when it could
     *  not be queued, was dropped by the pool, has not ended yet or is not a future of a keyring operation
     */
    static TKeyringResult GetResultOfFuture( const CORE::CFutureResult& future );
    ///@}

    /** Locks the keyring now (the person asked for it) */
    void LockNow( void );

    /** Deletes the keyring and the credentials saved in it (forgetting the code); a new code can be set up afterwards */
    bool Forget( void );

    /** Locks and ends what the session does; an operation that runs is let finish and locked again. For the end of the application */
    void Shutdown( void );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CKeyringOperationTaskConsumer;

    typedef CORE::CTEventHandlerFunctor< CKeyringSession > TEventCallback;

    struct SPendingEvent
    {
        CORE::CEvent eventId;
        CKeyringEventData data;
    };
    typedef GUCEF::vector< SPendingEvent > TPendingEventVector;

    CKeyringSession( const CKeyringSession& src );
    CKeyringSession& operator=( const CKeyringSession& src );

    void Construct( void );
    void OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnApplicationShutdown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    /** Takes the right to run an operation: KEYRING_OK, or BUSY, THROTTLED, FAILED (no store) */
    TKeyringResult BeginOperation( bool verifiesCode );
    TKeyringResult PerformOperation( CKeyringOperationData::EOperation operation, const CSecretString& code, const CSecretString& newCode );
    CORE::CFutureResult QueueOperation( CKeyringOperationData::EOperation operation, const CSecretString& code, const CSecretString& newCode );

    /** Called by CKeyringOperationTaskConsumer on the pool thread */
    void RunOperation( CKeyringOperationData& data );
    void AbandonOperation( CKeyringOperationData& data );

    void QueueEventLocked( const CORE::CEvent& eventId, const CKeyringEventData& data );
    CKeyringEventData MakeEventDataLocked( TKeyringResult result ) const;
    void FlushEvents( void );
    void CheckState( void );
    void UpdateTimer( void );
    void WaitForOperation( void );

    private:

    mutable MT::CMutex m_lock;
    CFileCredentialStore* m_store;
    CKeyringSettings m_settings;
    CORE::ThreadPoolPtr m_threadPool;
    CORE::CTimer m_timer;
    bool m_busy;
    bool m_shutDown;
    bool m_knownUnlocked;               /**< what the events said last: unlocked */
    UInt32 m_failedAttempts;
    UInt64 m_retryAtMs;                 /**< monotonic time before which no code is tried, 0 for none */
    bool m_retryAllowedPending;         /**< RetryAllowedEvent is to be raised when the wait is over */
    TKeyringResult m_lastResult;
    TPendingEventVector m_pendingEvents;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CKEYRINGSESSION_H ? */
