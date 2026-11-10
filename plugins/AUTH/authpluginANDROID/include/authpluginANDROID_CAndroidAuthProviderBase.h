/*
 *  authpluginANDROID: gucefAUTH plugin for logging in on Android devices
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

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHPROVIDERBASE_H
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHPROVIDERBASE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_AUTH_CIAUTHPROVIDER_H
#include "gucefAUTH_CIAuthProvider.h"
#define GUCEF_AUTH_CIAUTHPROVIDER_H
#endif /* GUCEF_AUTH_CIAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHBRIDGE_H
#include "authpluginANDROID_CAndroidAuthBridge.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHBRIDGE_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHBRIDGE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace ANDROIDAUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the Android providers share: a login is a request to the Java side through the bridge, answered by a callback
 *  on a Java thread. The base keeps the requests that wait, ends them when the requester cancels or the time is up
 *  (a timer on the pulse generator of the application, only while a request waits), and turns the answer into a
 *  CompleteLogin() or FailLogin() of the derived provider. One login at a time per provider.
 *
 *  The threads: starting a login, cancelling it and releasing the provider are done on the thread that pumps the pulse generator
 *  (the timer is touched there only); the answers arrive on any thread of the Java side and only end the login.
 *
 *  The locks: the provider's lock is never held while the bridge is called, because the bridge calls back into the
 *  provider (from a Java thread) when an answer arrives.
 */
class GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP CAndroidAuthProviderBase : public AUTH::CIAuthProvider ,
                                                                     public IAndroidAuthResultSink
{
    public:

    virtual ~CAndroidAuthProviderBase();

    /** The number of requests that wait for an answer */
    UInt32 GetPendingRequestCount( void ) const;

    /** The reason of a failed login that a status of the Java side stands for */
    static AUTH::TAuthFailureReason ToFailureReason( EAndroidAuthStatus status );

    virtual void OnAndroidAuthResult( const SAndroidAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    protected:

    struct SPending
    {
        UInt32 requestId;
        CORE::CFutureResultSource source;
        UInt64 deadlineInMs;
        bool interactive;
        CORE::CString nonce;
    };

    CAndroidAuthProviderBase( void );

    /**
     *  Begins a request. Under the lock of the provider canStart() is asked whether the provider can; then the request is
     *  made and startNative() calls Java without the lock. False when the login cannot start: for an interactive one the
     *  future then has the failure, for a silent one it has no future.
     */
    bool StartRequest( bool interactive, UInt32 timeoutInMs, const CORE::CString& nonce, CORE::CFutureResult& future );

    /** Under the lock: whether a login can be started, and if not why not */
    virtual bool CanStart( bool interactive, CORE::CString& reason ) const = 0;

    /** Without the lock: starts the Java side for the request. False when it did not start */
    virtual bool StartNative( UInt32 requestId, bool interactive, const CORE::CString& nonce ) = 0;

    /** Without the lock: the answer of the Java side for a request that waited. Ends the login with CompleteLogin() or FailLogin() */
    virtual void HandleResult( SPending& pending, const SAndroidAuthResult& result ) = 0;

    /** For the destructor of the derived class: stops waiting for answers and ends what waits, before the derived part is gone */
    void Shutdown( void );

    mutable MT::CMutex m_lock;

    private:

    class CPump : public CORE::CObserver
    {
        public:
        explicit CPump( CAndroidAuthProviderBase& owner );
        virtual ~CPump();
        virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
        protected:
        virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE;
        private:
        CAndroidAuthProviderBase& m_owner;
    };
    friend class CPump;

    typedef GUCEF::vector< SPending > TPendingVector;

    void OnTimer( void );
    void UpdateTimer( void );

    private:

    TPendingVector m_pending;
    bool m_shutDown;
    CORE::CTimer m_timer;
    CPump m_pump;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHPROVIDERBASE_H ? */
