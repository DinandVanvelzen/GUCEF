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

#ifndef GUCEF_AUTH_CWEBSESSIONSIGNIN_H
#define GUCEF_AUTH_CWEBSESSIONSIGNIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */
#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */
#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */
#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */
#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */
#ifndef GUCEF_WEB_CHTTPCOOKIEJAR_H
#include "gucefWEB_CHttpCookieJar.h"
#define GUCEF_WEB_CHTTPCOOKIEJAR_H
#endif /* GUCEF_WEB_CHTTPCOOKIEJAR_H ? */
#ifndef GUCEF_WEB_CHTTPRESPONSEDATA_H
#include "gucefWEB_CHttpResponseData.h"
#define GUCEF_WEB_CHTTPRESPONSEDATA_H
#endif /* GUCEF_WEB_CHTTPRESPONSEDATA_H ? */
#ifndef GUCEF_WEB_ETYPES_H
#include "gucefWEB_ETypes.h"
#define GUCEF_WEB_ETYPES_H
#endif /* GUCEF_WEB_ETYPES_H ? */
#ifndef GUCEF_AUTH_CWEBSESSIONSIGNINSETTINGS_H
#include "gucefAUTH_CWebSessionSignInSettings.h"
#define GUCEF_AUTH_CWEBSESSIONSIGNINSETTINGS_H
#endif /* GUCEF_AUTH_CWEBSESSIONSIGNINSETTINGS_H ? */
#ifndef GUCEF_AUTH_CWEBSIGNINCREDENTIALS_H
#include "gucefAUTH_CWebSignInCredentials.h"
#define GUCEF_AUTH_CWEBSIGNINCREDENTIALS_H
#endif /* GUCEF_AUTH_CWEBSIGNINCREDENTIALS_H ? */
#ifndef GUCEF_AUTH_CWEBSESSION_H
#include "gucefAUTH_CWebSession.h"
#define GUCEF_AUTH_CWEBSESSION_H
#endif /* GUCEF_AUTH_CWEBSESSION_H ? */
#ifndef GUCEF_WEB_CWEBSESSIONENDEDCHECK_H
#include "gucefWEB_CWebSessionEndedCheck.h"
#define GUCEF_WEB_CWEBSESSIONENDEDCHECK_H
#endif /* GUCEF_WEB_CWEBSESSIONENDEDCHECK_H ? */
#ifndef GUCEF_AUTH_CIWEBSIGNINBROWSER_H
#include "gucefAUTH_CIWebSignInBrowser.h"
#define GUCEF_AUTH_CIWEBSIGNINBROWSER_H
#endif /* GUCEF_AUTH_CIWEBSIGNINBROWSER_H ? */
#ifndef GUCEF_WEB_CWEBSIGNINEVENTDATA_H
#include "gucefWEB_CWebSignInEventData.h"
#define GUCEF_WEB_CWEBSIGNINEVENTDATA_H
#endif /* GUCEF_WEB_CWEBSIGNINEVENTDATA_H ? */
#ifndef GUCEF_WEB_CWEBSIGNINRESULT_H
#include "gucefWEB_CWebSignInResult.h"
#define GUCEF_WEB_CWEBSIGNINRESULT_H
#endif /* GUCEF_WEB_CWEBSIGNINRESULT_H ? */

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

/**
 *  Signs in to a store that only has a login on its web site, through a browser (section 12.1 of the design), and
 *  keeps the web session: the cookies of the store's domains.
 *
 *  A sign in starts a browser (CIWebSignInBrowser; by default CChromeSignInBrowser: Chrome, Chromium, Edge or Brave with
 *  a profile folder used only for the store and a debugging port on the loopback interface), opens the start page of
 *  the login script and runs the script:
 *  - with saved credentials (SetCredentialStore(), SaveCredentials()) or credentials given for this sign in
 *    (SetCredentials()) the fill steps type them in: automatic sign in;
 *  - without them, or when the page does not show what the script expects, an interactive sign in hands the rest
 *    over to the owner in the browser window (OwnerActionNeededEvent) and waits until the signed in condition holds;
 *  - a personCheck step that holds (a captcha, a code sent by e-mail, a second factor) hands over in the same way and
 *    the sign in continues when the signed in condition appears; a failCheck step that holds ends it.
 *  Then the cookies of the store's domains are taken as the session (CWebSession), kept in the session store
 *  (SetSessionStore(), a gucefAUTH credential store: sealed, never in the clear) and the browser is closed (unless
 *  WEB::CWebSignInBrowserSettings::keepOpen). The session goes into the cookie jar of the store plugin with ApplySessionTo().
 *
 *  An unattended sign in (WEB::WEBSIGNIN_MODE_UNATTENDED, used by RenewSession()) has a browser without a window, needs
 *  saved credentials and ends with WEB::WEBSIGNIN_STATUS_OWNER_ACTION_NEEDED where a person would have to act. So a window
 *  only appears when the owner starts a sign in; SignInStartingEvent is raised before the browser starts, so an
 *  application can say that a window is about to open.
 *
 *  The session ended when the store says so: CheckResponse() with every answer to a request made with the session
 *  (WEB::CIWebSessionEndedCheck: 401 or a redirect to the login page by default, from the script) forgets the session,
 *  raises SessionEndedEvent and has the browser cookies cleared before the next sign in.
 *
 *  Threading: SignIn() runs on the calling thread and blocks it for as long as the sign in takes (minutes when the owner
 *  acts): never call it on the thread of a user interface; SignInAsync() runs it as a task on a thread pool (the pool
 *  of SetThreadPoolName(), "WebSignIn" by default) and returns a CFutureResult whose result data is a WEB::CWebSignInResult.
 *  The browser is driven on the thread that runs the sign in, with a pulse generator of its own. Events are raised on
 *  the thread of the global pulse generator. Cancel() and the session members can be used from any thread. One sign
 *  in at a time per object.
 *
 *  Nothing secret is logged: not the credentials, not the cookies; the log says which step runs and why it stopped.
 *
 *  Events:
 *    SignInStartingEvent       WEB::CWebSignInEventData: a sign in starts, a browser (window, unless headless) opens next
 *    ProgressEvent             WEB::CWebSignInEventData: what the sign in does now
 *    OwnerActionNeededEvent    WEB::CWebSignInEventData: "owner action needed": the message says what to do in the window
 *    SignedInEvent             WEB::CWebSignInResult
 *    SignInFailedEvent         WEB::CWebSignInResult: failed, timed out, cancelled, owner action needed (unattended)
 *    SessionEndedEvent         CORE::TCloneableString: the store id
 */
class GUCEF_AUTH_EXPORT_CPP CWebSessionSignIn : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent SignInStartingEvent;
    static const CORE::CEvent ProgressEvent;
    static const CORE::CEvent OwnerActionNeededEvent;
    static const CORE::CEvent SignedInEvent;
    static const CORE::CEvent SignInFailedEvent;
    static const CORE::CEvent SessionEndedEvent;

    static const CORE::CString DefaultThreadPoolName;      /**< "WebSignIn" */

    static void RegisterEvents( void );

    CWebSessionSignIn( void );
    explicit CWebSessionSignIn( const CWebSessionSignInSettings& settings );
    virtual ~CWebSessionSignIn() GUCEF_VIRTUAL_OVERRIDE;

    /** False while a sign in runs or when the settings are not valid (the problems say why) */
    bool SetSettings( const CWebSessionSignInSettings& settings, CORE::CString::StringVector* problems = GUCEF_NULL );
    CWebSessionSignInSettings GetSettings( void ) const;
    CORE::CString GetStoreId( void ) const;

    /** Reads the login script of the settings from a JSON file (WEB::CWebLoginScript::LoadFromFile()) */
    bool LoadLoginScript( const CORE::CString& path, CORE::CString::StringVector& problems );

    /** The browser to drive instead of a CChromeSignInBrowser per sign in, for tests; not owned, GUCEF_NULL for the default */
    void SetBrowser( CIWebSignInBrowser* browser );

    /** Where saved credentials are read from and saved to (CWebSignInCredentials); not owned, GUCEF_NULL for none */
    void SetCredentialStore( AUTH::ICredentialStore* credentialStore );

    /** Where the session is kept between runs of the application; not owned, GUCEF_NULL for none */
    void SetSessionStore( AUTH::ICredentialStore* sessionStore );

    /** The check of the answers of the store; not owned, GUCEF_NULL for the default made from the script */
    void SetSessionEndedCheck( const WEB::CIWebSessionEndedCheck* check );

    /** Credentials for the sign ins of this object, kept in memory only (wiped on Clear and destruction); they come before saved ones */
    void SetCredentials( const CWebSignInCredentials& credentials );
    void ClearCredentials( void );

    /** Saves the credentials in the credential store under the store id */
    bool SaveCredentials( const CWebSignInCredentials& credentials );

    /** Whether credentials for every slot of the script are given or saved */
    bool HasCredentials( void ) const;

    /** Removes the saved credentials of the store */
    bool ForgetCredentials( void );

    /**
     *  Signs in on the calling thread, which it blocks until done (see the class comment)
     *  @param message  why it failed, or a remark; never a secret
     */
    WEB::TWebSignInStatus SignIn( WEB::TWebSignInMode mode, CORE::CString* message = GUCEF_NULL );

    /** Signs in as a task; the future completes with a WEB::CWebSignInResult or fails with "status: message" */
    CORE::CFutureResult SignInAsync( WEB::TWebSignInMode mode );

    /** Ends a sign in that runs, from any thread: it ends with WEB::WEBSIGNIN_STATUS_CANCELLED and closes the browser */
    void Cancel( void );

    bool IsSignInRunning( void ) const;

    /** Waits until no async sign in of this object runs; false when the time ran out */
    bool WaitForAsyncSignIn( UInt32 timeoutInMs );

    void SetThreadPoolName( const CORE::CString& threadPoolName );
    CORE::CString GetThreadPoolName( void ) const;

    /** Whether there is a session with at least one cookie */
    bool HasSession( void ) const;

    /** A copy of the session; false when there is none */
    bool GetSession( CWebSession& session ) const;

    /** Reads the session of the store from the session store; true when there is one */
    bool LoadSavedSession( void );

    /** Puts the cookies of the session into the jar; the number put in */
    UInt32 ApplySessionTo( WEB::CHttpCookieJar& jar ) const;

    /** Forgets the session, here and in the session store; the browser cookies are cleared before the next sign in */
    bool ForgetSession( void );

    /**
     *  Checks an answer of the store to a request made with the session; when it means the session ended, forgets it
     *  and raises SessionEndedEvent. Any thread
     *  @return whether the session ended
     */
    bool CheckResponse( const WEB::CHttpResponseData& response );

    /**
     *  Ends the session the store no longer takes (what CheckResponse() does when an answer says so), for a caller that
     *  found out in another way, for example a check of the store that a refused request was not a refusal of the item:
     *  forgets the session and raises SessionEndedEvent. The reason is logged; it must hold no secret. Any thread
     */
    void EndSession( const CORE::CString& reason );

    /**
     *  Takes a session that was signed in somewhere else (the browser of the user on another machine handed its cookies
     *  over) as the session of the store, in place of a sign in: only the cookies of the domains of the login script
     *  (CWebLoginScript::IsStoreCookieDomain()) with a value and not expired are kept, the store id is that of the
     *  settings, the user agent is kept with it; it is saved in the session store when the settings say so and
     *  SignedInEvent is raised (CWebSignInResult, handedOver). Refused while a sign in runs, when the settings have no
     *  store id or no cookie domain, and when no cookie is left. Nothing of a value is logged. Any thread
     *  @param keptCookies   the cookies taken over
     *  @param problem       why it was refused, for people; never a value
     */
    bool AdoptSession( const CWebSession& session, UInt32& keptCookies, CORE::CString& problem );

    /**
     *  Takes the cookies of the store's domains that the store renewed in its answers (Set-Cookie into the jar the
     *  requests of the session use) into the session: a cookie of the session whose value or expiry changed, and a new
     *  cookie of a domain of the login script. The session store is written only when something changed. Cookies are never
     *  removed here (an ended session is CheckResponse()'s). Any thread
     *  @return whether the session changed
     */
    bool UpdateSessionCookies( const WEB::CHttpCookieJar& jar );

    /**
     *  After the store refused the session: forgets it and signs in again without the owner (unattended), when the
     *  settings allow it and credentials for every slot are given or saved; otherwise WEB::WEBSIGNIN_STATUS_NOT_SIGNED_IN:
     *  the owner has to sign in. When a sign in runs already it waits for that one and uses its session, so requests
     *  that find the session ended at the same time sign in once. Blocks the calling thread
     */
    WEB::TWebSignInStatus RenewSession( CORE::CString* message = GUCEF_NULL );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CWebSessionSignInTaskConsumer;

    struct SRunContext;

    /** The sign in itself, on the calling thread; the run must have been begun (TryBeginRun()) */
    WEB::TWebSignInStatus RunSignIn( WEB::TWebSignInMode mode, bool startNoticeRaised, WEB::CWebSignInResult& result );
    WEB::TWebSignInStatus RunScript( SRunContext& context );
    WEB::TWebSignInStatus RunStep( SRunContext& context, UInt32 stepIndex, bool& signedIn );
    WEB::TWebSignInStatus CheckRunning( SRunContext& context );
    WEB::TWebSignInStatus WaitForCondition( SRunContext& context, const WEB::CWebLoginScriptCondition& condition, UInt32 timeoutInMs, bool checkWatchers );
    WEB::TWebSignInStatus WaitForSignedIn( SRunContext& context, UInt32 timeoutInMs );
    WEB::TWebSignInStatus HandOver( SRunContext& context, const CORE::CString& prompt, const CORE::CString& kind );
    WEB::TWebSignInStatus Stuck( SRunContext& context, WEB::TWebSignInStatus unattendedStatus, const CORE::CString& reason );
    WEB::TWebSignInStatus CheckWatchers( SRunContext& context );
    Int32 CheckCondition( SRunContext& context, const WEB::CWebLoginScriptCondition& condition );
    bool IsSignedInNow( SRunContext& context, Int32& state );
    bool CaptureSession( SRunContext& context, CWebSession& session );
    void Pause( SRunContext& context, UInt32 milliseconds );
    void Progress( SRunContext& context, UInt32 stepNumber, const CORE::CString& message );
    void Notify( const CORE::CEvent& eventId, CORE::CICloneable& eventData );
    WEB::CWebSignInEventData MakeEventData( const CWebSessionSignInSettings& settings, WEB::TWebSignInMode mode ) const;

    bool TryBeginRun( void );
    void EndRun( void );
    void AsyncEnded( void );
    void LoadCredentials( const CWebSessionSignInSettings& settings, CWebSignInCredentials& credentials ) const;

    CWebSessionSignIn( const CWebSessionSignIn& src );              /**< not implemented */
    CWebSessionSignIn& operator=( const CWebSessionSignIn& src );   /**< not implemented */

    private:

    mutable MT::CMutex m_dataLock;
    CWebSessionSignInSettings m_settings;
    CIWebSignInBrowser* m_browser;
    AUTH::ICredentialStore* m_credentialStore;
    AUTH::ICredentialStore* m_sessionStore;
    const WEB::CIWebSessionEndedCheck* m_sessionEndedCheck;
    WEB::CWebSessionEndedCheck m_defaultSessionEndedCheck;
    CWebSignInCredentials m_credentials;
    CWebSession m_session;
    bool m_clearBrowserCookies;
    bool m_running;
    CORE::CString m_threadPoolName;
    MT::CTAtomic< UInt32 > m_cancelRequested;
    MT::CTAtomic< UInt32 > m_pendingAsync;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CWEBSESSIONSIGNIN_H ? */
