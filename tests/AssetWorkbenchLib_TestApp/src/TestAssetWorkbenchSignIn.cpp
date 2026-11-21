/*
 *  AssetWorkbenchLib_TestApp: tests of the AssetWorkbenchLib module
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


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <Ogre.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

#ifndef GUCEF_AUTH_CIAUTHPROVIDER_H
#include "gucefAUTH_CIAuthProvider.h"
#define GUCEF_AUTH_CIAUTHPROVIDER_H
#endif /* GUCEF_AUTH_CIAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

#ifndef GUCEF_WEB_CWEBLOGINSCRIPT_H
#include "gucefWEB_CWebLoginScript.h"
#define GUCEF_WEB_CWEBLOGINSCRIPT_H
#endif /* GUCEF_WEB_CWEBLOGINSCRIPT_H ? */

#ifndef GUCEF_ASSETSTORE_CWEBSESSIONASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CWebSessionAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CWEBSESSIONASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CWEBSESSIONASSETSTOREPROVIDER_H ? */

#include "AssetWorkbenchLib.h"

#include "TestAssetWorkbenchSupport.h"
#include "TestFakeSourceStore.h"
#include "TestFakeLoginSite.h"
#include "TestFakeSignInBrowser.h"
#include "TestAssetWorkbenchSignIn.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETWORKBENCH;
using namespace AUTHTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

using WEB::CWebLoginScript;
using WEB::CWebLoginScriptStep;

const char* const WebStoreId = "fakeweb";
const char* const TestEmail = "owner@example.test";
const char* const TestPassword = "Pa55-w0rd-of-the-workbench-test";
const char* const KeyringCode = "keyring code of the workbench 7q4d";
const char* const WrongKeyringCode = "keyring code of the workbench 7q4x";
const char* const NewKeyringCode = "Another-Workbench-Keyring-Code-91";
const char* const OAuthCode = "4/0AbCdEfGhIjKlMnOpQrStUvWx-workbench-code";
const char* const OAuthUrl = "https://accounts.example.com/o/authorize?client_id=workbench&state=workbench-state-3f";

/** A login script for the fake login site: the fill steps, the person checks, the signed in state */
WEB::CWebLoginScript
MakeSiteScript( const CFakeLoginSite& site )
{
    WEB::CWebLoginScript script;
    script.name = "Fake login site";
    script.startUrl = site.GetBaseUrl() + "/login";
    script.cookieDomains.push_back( "127.0.0.1" );
    script.credentialSlots.push_back( "username" );
    script.credentialSlots.push_back( "password" );
    script.signedIn.selector = "#account";
    script.signedIn.cookie = "sid";
    script.sessionEndedStatusCodes.push_back( 401 );
    script.sessionEndedRedirectPattern = site.GetBaseUrl() + "/login*";
    script.stepTimeoutInMs = 3000;

    WEB::CWebLoginScriptStep step;
    step.action = WEB::CWebLoginScriptStep::ACTION_FILL;
    step.selector = "input[name=email]";
    step.credential = "username";
    script.steps.push_back( step );
    step.selector = "input[name=\"password\"]";
    step.credential = "password";
    script.steps.push_back( step );

    step = WEB::CWebLoginScriptStep();
    step.action = WEB::CWebLoginScriptStep::ACTION_SUBMIT;
    step.selector = "form#login";
    script.steps.push_back( step );

    step = WEB::CWebLoginScriptStep();
    step.action = WEB::CWebLoginScriptStep::ACTION_FAIL_CHECK;
    step.condition.selector = ".login-error";
    step.reason = "The site refused the e-mail address or password";
    script.steps.push_back( step );

    step = WEB::CWebLoginScriptStep();
    step.action = WEB::CWebLoginScriptStep::ACTION_PERSON_CHECK;
    step.condition.selector = "#captcha";
    step.kind = "captcha";
    step.prompt = "Solve the puzzle in the browser window";
    script.steps.push_back( step );

    step = WEB::CWebLoginScriptStep();
    step.action = WEB::CWebLoginScriptStep::ACTION_WAIT_FOR_SIGNED_IN;
    step.timeoutInMs = 8000;
    script.steps.push_back( step );
    return script;
}

/** A store on the fake login site, on the provider base class of the web session stores: its library needs the session */
class CWorkbenchTestWebProvider : public ASSETSTORE::CWebSessionAssetStoreProvider
{
    public:

    static const CFakeLoginSite*& Site( void ) { static const CFakeLoginSite* site = GUCEF_NULL; return site; }

    /** The store refuses the session whatever it is, as a store that ends sessions at once does */
    static bool& AlwaysExpired( void ) { static bool expired = false; return expired; }

    CWorkbenchTestWebProvider( void ) : ASSETSTORE::CWebSessionAssetStoreProvider() {}

    virtual ~CWorkbenchTestWebProvider()
    {
        WaitForAsyncOperations();
        SignalUpcomingDestruction();
    }

    virtual const CORE::CString& GetProviderId( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString id = WebStoreId;
        return id;
    }

    virtual CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "Fake web store"; }

    virtual ASSETSTORE::TAssetStoreStatus QueryLibrary( const ASSETSTORE::CAssetStoreQuery& query, ASSETSTORE::CAssetStoreLibraryPage& page ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !IsSignedIn() )
            return ASSETSTORE::ASSETSTORE_STATUS_NOT_SIGNED_IN;
        const ASSETSTORE::TAssetStoreStatus status = AskSite();
        if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
            return status;
        ASSETSTORE::CAssetStoreItem item;
        item.providerId = WebStoreId;
        item.itemId = "pack-1";
        item.title = "Pack one";
        page.items.push_back( item );
        return ASSETSTORE::ASSETSTORE_STATUS_OK;
    }

    virtual ASSETSTORE::TAssetStoreStatus GetItem( const CORE::CString& itemId, ASSETSTORE::CAssetStoreItem& item ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !IsSignedIn() )
            return ASSETSTORE::ASSETSTORE_STATUS_NOT_SIGNED_IN;
        const ASSETSTORE::TAssetStoreStatus status = AskSite();
        if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
            return status;
        item.providerId = WebStoreId;
        item.itemId = itemId;
        item.title = "Pack one";
        ASSETSTORE::CAssetStoreFormat format;
        format.formatId = "gltf";
        format.name = "glTF";
        format.group = "3d-exchange";
        ASSETSTORE::CAssetStoreFile file;
        file.fileId = "pack-1.gltf";
        file.name = "pack-1.gltf";
        file.size = 1000;
        format.files.push_back( file );
        item.formats.push_back( format );
        return ASSETSTORE::ASSETSTORE_STATUS_OK;
    }

    virtual ASSETSTORE::TAssetStoreStatus PlanDownload( const CORE::CString& itemId, const CORE::CString& formatId, const CORE::CString::StringVector& fileIds,
                                                        ASSETSTORE::CAssetStoreDownloadPlan& plan ) GUCEF_VIRTUAL_OVERRIDE { return ASSETSTORE::ASSETSTORE_STATUS_UNSUPPORTED; }

    private:

    /** The request of the plugin: the library of the site with the cookies of the session; every answer goes to CheckResponse() */
    ASSETSTORE::TAssetStoreStatus AskSite( void )
    {
        if ( AlwaysExpired() )
        {
            WEB::CHttpResponseData ended;
            ended.statusCode = 401;
            CheckResponse( ended );
            return ASSETSTORE::ASSETSTORE_STATUS_SESSION_EXPIRED;
        }
        WEB::CHttpRequestData request;
        request.SetUrl( Site()->GetBaseUrl() + "/api/library" );
        request.requestType = "GET";
        WEB::CHttpClientRequestOptions options;
        options.cookieJar = GetCookieJar();
        options.followRedirects = false;
        options.totalTimeoutInMs = 10000;
        WEB::CHTTPClient client;
        WEB::CHttpResponseData response;
        if ( !client.Execute( request, options, response ) )
            return ASSETSTORE::ASSETSTORE_STATUS_NETWORK_ERROR;
        if ( CheckResponse( response ) )
            return ASSETSTORE::ASSETSTORE_STATUS_SESSION_EXPIRED;
        return 200 == response.statusCode ? ASSETSTORE::ASSETSTORE_STATUS_OK : ASSETSTORE::ASSETSTORE_STATUS_HTTP_ERROR;
    }
};

typedef CORE::CTFactory< ASSETSTORE::CIAssetStoreProvider, CWorkbenchTestWebProvider, MT::CMutex > TWebFactory;

/*-------------------------------------------------------------------------*/

/** A login provider of gucefAUTH that asks for a page and waits for a code: the login ends when the test says so */
class CFakeAuthProvider : public AUTH::CIAuthProvider
{
    public:

    CORE::CString name;
    UInt32 beginCount;

    CFakeAuthProvider( void ) : AUTH::CIAuthProvider(), name( "fake-auth" ), beginCount( 0 ), m_source(), m_pending( false ) {}
    virtual ~CFakeAuthProvider() {}

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE { return name; }
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return AUTH::AUTHCAP_BROWSER; }
    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool TrySilentLogin( CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        future = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
        return false;
    }

    virtual bool BeginLogin( const AUTH::CAuthOptions& options, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++beginCount;
        m_source = CORE::CFutureResultSource();
        future = m_source.GetFuture();
        m_pending = true;
        AUTH::CAuthActionRequired action;
        action.kind = AUTH::AUTHACTION_OPEN_URL;
        action.url = OAuthUrl;
        action.expiresAtMs = AUTH::GetEpochTimeInMs() + 600000;
        RaiseActionRequired( action );
        return true;
    }

    bool Succeed( void )
    {
        if ( !m_pending )
            return false;
        m_pending = false;
        AUTH::CAuthResult result;
        result.accountRef = AUTH::CAccountRef( "fake", "owner-1" );
        result.displayName = "Fake owner";
        result.accessToken.Set( "access-token-of-the-workbench-test" );
        result.refreshToken.Set( "refresh-token-of-the-workbench-test" );
        result.expiresAtMs = AUTH::GetEpochTimeInMs() + 3600000;
        return CompleteLogin( m_source, result );
    }

    bool Fail( const CORE::CString& message )
    {
        if ( !m_pending )
            return false;
        m_pending = false;
        return FailLogin( m_source, AUTH::AUTHFAIL_DENIED, message );
    }

    bool HonourCancel( void )
    {
        if ( !m_pending || !m_source.IsCancellationRequested() )
            return false;
        m_pending = false;
        return FailLogin( m_source, AUTH::AUTHFAIL_CANCELLED, "cancelled" );
    }

    bool IsPending( void ) const { return m_pending; }

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CFakeAuthProvider";
        return classTypeName;
    }

    private:

    CORE::CFutureResultSource m_source;
    bool m_pending;
};

typedef CORE::CTFactory< AUTH::CIAuthProvider, CFakeAuthProvider, MT::CMutex > TFakeAuthFactory;

/** An engine that takes the code the way the test says, and remembers what it was given */
class CFakeCodeEngine : public AUTH::IAuthFlowEngine
{
    public:

    AUTH::TCodeCheck answer;
    UInt32 count;
    CORE::CString lastText;

    CFakeCodeEngine( void ) : AUTH::IAuthFlowEngine(), answer( AUTH::CODECHECK_ACCEPTED ), count( 0 ), lastText() {}

    virtual bool IsSupported( const AUTH::CAuthProviderDescriptor& descriptor, const AUTH::CAuthProviderSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool StartInteractive( const AUTH::CAuthProviderDescriptor& descriptor, const AUTH::CAuthProviderSettings& settings, const AUTH::CAuthOptions& options, AUTH::IAuthFlowSink& sink, CORE::CFutureResultSource& result ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool StartRefresh( const AUTH::CAuthProviderDescriptor& descriptor, const AUTH::CAuthProviderSettings& settings, const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken, AUTH::IAuthFlowSink& sink, CORE::CFutureResultSource& result ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Revoke( const AUTH::CAuthProviderDescriptor& descriptor, const AUTH::CAuthProviderSettings& settings, const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Verify( const AUTH::CAuthProviderDescriptor& descriptor, const AUTH::CAuthProviderSettings& settings, const AUTH::CAuthProof& proof, AUTH::CAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool SubmitAuthorizationCode( const AUTH::CSecretString& text ) GUCEF_VIRTUAL_OVERRIDE
    {
        AUTH::TCodeCheck check = AUTH::CODECHECK_REFUSED;
        return SubmitAuthorizationCode( text, check );
    }

    virtual bool SubmitAuthorizationCode( const AUTH::CSecretString& text, AUTH::TCodeCheck& check ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++count;
        lastText = text.Reveal();
        check = answer;
        return AUTH::CODECHECK_ACCEPTED == check;
    }
};

/*-------------------------------------------------------------------------*/

/** Counts one event of one notifier */
class CEventCounter : public CORE::CObserver
{
    public:

    UInt32 count;

    CEventCounter( CORE::CNotifier* notifier, const CORE::CEvent& id ) : CORE::CObserver(), count( 0 ), m_id( id )
    {
        SubscribeTo( notifier, id );
    }

    virtual ~CEventCounter() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id == m_id )
            ++count;
    }

    private:

    CORE::CEvent m_id;
};

/*-------------------------------------------------------------------------*/

/**
 *  The workbench with its asset sources and the sign in of the online stores: the fake web store on the fake login site (a browser that
 *  is a test double), the fake store that signs in with a provider of gucefAUTH, the credential stores in a folder of the test with
 *  cheap Argon2id parameters, and the shell on the real GUI of the window
 */
class CSignInFixture
{
    public:

    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUIDRIVEROGRE::COgreWindowContext* window;
    CFakeLoginSite site;
    CFakeSignInBrowser browser;
    CAssetMounter mounter;
    CORE::CString dir;
    CORE::CString name;
    CAssetSourcesSettings settings;
    CWorkbenchCredentials* credentials;
    CWorkbenchShell* shell;
    TWebFactory webFactory;
    ASSETSTORE::TAssetStoreProviderPtr webPtr;
    CWorkbenchTestWebProvider* web;
    SOURCESTEST::TFakeSourceFactory fakeFactory;
    ASSETSTORE::TAssetStoreProviderPtr fakePtr;
    SOURCESTEST::CFakeSourceProvider* fake;
    AUTH::CAuthManager* authManager;
    bool withWeb;
    bool withFake;
    bool ready;
    bool sourcesOpened;

    CSignInFixture( const CORE::CString& testName, bool useWeb, bool useFake )
        : contextPtr(), context( GUCEF_NULL ), window( GUCEF_NULL ), site(), browser(), mounter(), dir(), name( testName ), settings()
        , credentials( GUCEF_NULL ), shell( GUCEF_NULL ), webFactory(), webPtr(), web( GUCEF_NULL ), fakeFactory(), fakePtr(), fake( GUCEF_NULL )
        , authManager( GUCEF_NULL ), withWeb( useWeb ), withFake( useFake ), ready( false ), sourcesOpened( false )
    {
        if ( GetMainTestWindow().IsNULL() )
            return;
        window = AsOgreWindow( GetMainTestWindow() );
        dir = MakeCleanTestDir( name );
        if ( !site.Start() )
            return;
        site.SetAccount( TestEmail, TestPassword );
        CWorkbenchTestWebProvider::Site() = &site;
        CWorkbenchTestWebProvider::AlwaysExpired() = false;

        const CORE::CString storeDir = dir + "/store";
        CORE::CreateDirs( storeDir );
        const CORE::CString root = "avstore/" + name;
        settings.primaryStore.locations[ 0 ].vfsRoot = root;
        settings.primaryStore.locations[ 0 ].fileSystemPath = storeDir;
        settings.primaryStore.database.database = root + "/catalogue";
        settings.primaryStore.analysisConcurrency = 2;

        contextPtr = CreateWorkbenchContext();
        context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL == context )
            return;
        BuildStores();
        ready = BuildShell();
    }

    ~CSignInFixture()
    {
        TearDownShell();
        TearDownStores();
        site.Stop();
        contextPtr.Unlink();
        mounter.UnmountAll();
    }

    /** The credential stores of the workbench, the web store that takes them (a restart of the workbench makes them again) and the fake store of gucefAUTH */
    void BuildStores( void )
    {
        credentials = GUCEF_NEW CWorkbenchCredentials();
        credentials->Open( dir + "/auth" );
        credentials->GetKeyringStore().SetKeyringParameters( 64, 1, 1 );
        AUTH::CKeyringSettings keyring = credentials->GetKeyringSession().GetSettings();
        keyring.checkIntervalInMs = 5;
        credentials->GetKeyringSession().SetSettings( keyring );
        credentials->Install();

        if ( withWeb )
        {
            webPtr = webFactory.Create();
            web = static_cast< CWorkbenchTestWebProvider* >( webPtr.GetPointerAlways() );
            AUTH::CWebSessionSignInSettings signInSettings;
            signInSettings.storeId = WebStoreId;
            signInSettings.script = MakeSiteScript( site );
            signInSettings.signInTimeoutInMs = 30000;
            signInSettings.handOverTimeoutInMs = 20000;
            signInSettings.pollIntervalInMs = 20;
            signInSettings.browser.profileDirectory = dir + "/profile";
            web->ConfigureWebSignIn( signInSettings, GUCEF_NULL, GUCEF_NULL );
            web->GetWebSessionSignIn()->SetBrowser( &browser );
            ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().AddProvider( webPtr );
        }
        if ( withFake )
        {
            fakePtr = fakeFactory.Create();
            fake = static_cast< SOURCESTEST::CFakeSourceProvider* >( fakePtr.GetPointerAlways() );
            fake->SetSignedIn( false );
            ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().AddProvider( fakePtr );
        }
    }

    bool BuildShell( void )
    {
        shell = GUCEF_NEW CWorkbenchShell( mounter );
        if ( !shell->Initialize( *context, GUCEF_NULL, MakeCleanTestDir( name + "_cache" ) ) )
            return false;
        shell->SetCredentials( credentials );
        if ( GUCEF_NULL != authManager )
            shell->SetAuthManager( authManager );
        sourcesOpened = shell->InitializeSources( settings );
        shell->ShowLeftTab( 2 );
        return sourcesOpened && GUCEF_NULL != shell->GetSignIn();
    }

    void TearDownShell( void )
    {
        if ( GUCEF_NULL != shell )
        {
            shell->Shutdown();
            GUCEF_DELETE shell;
            shell = GUCEF_NULL;
        }
    }

    void TearDownStores( void )
    {
        if ( withWeb )
        {
            ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( WebStoreId );
            webPtr.Unlink();
            web = GUCEF_NULL;
        }
        if ( withFake )
        {
            ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( SOURCESTEST::CFakeSourceProvider::ProviderIdText );
            fakePtr.Unlink();
            fake = GUCEF_NULL;
        }
        if ( GUCEF_NULL != credentials )
        {
            credentials->Shutdown();
            GUCEF_DELETE credentials;
            credentials = GUCEF_NULL;
        }
    }

    /** The workbench ends and starts again on the same folders: nothing but what the files hold is left */
    bool Restart( void )
    {
        TearDownShell();
        TearDownStores();
        BuildStores();
        ready = BuildShell();
        return ready;
    }

    /** Frames of the window with the pulses of the application: the shell has its turn, the events of the stores and of the sign in arrive */
    IMAGE::TPixelMapPtr Frames( UInt32 count )
    {
        IMAGE::TPixelMapPtr pixels;
        for ( UInt32 i=0; i<count; ++i )
        {
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            shell->Update( 5 );
            window->RenderFrame( &pixels );
            MT::PrecisionDelay( 2 );
        }
        return pixels;
    }

    template < typename TCondition >
    bool WaitFor( TCondition condition, UInt32 maxFrames = 800 )
    {
        for ( UInt32 i=0; i<maxFrames; ++i )
        {
            if ( condition() )
                return true;
            Frames( 1 );
            MT::PrecisionDelay( 5 );
        }
        return condition();
    }

    CAssetSourcesPane& Pane( void ) { return *shell->GetSources(); }
    CWorkbenchSignIn& SignIn( void ) { return *shell->GetSignIn(); }
    AUTH::CWebSessionSignIn* WebSignIn( void ) { return web->GetWebSessionSignIn(); }

    /** The store of the web (or of gucefAUTH) as the page shows it: selected, with its line */
    bool SelectStore( const CORE::CString& storeId )
    {
        shell->ShowLeftTab( 2 );
        if ( !Pane().SelectStore( storeId ) )
            return false;
        Frames( 3 );
        return true;
    }

    /** The click of a person on a control of the page */
    void Click( GUI::CWidget* widget )
    {
        context->FlushChanges();
        const GUI::CRect rect = widget->GetRect();
        context->InjectMouseMove( rect.left + rect.width / 2, rect.top + rect.height / 2 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
    }

    /** The person types in a field of a form */
    void Type( GUI::CForm* form, const CORE::CString& field, const CORE::CString& text )
    {
        context->FlushChanges();
        GUI::CWidget* widget = form->GetWidget( field );
        if ( GUCEF_NULL == widget )
            return;
        context->SetFocus( widget );
        context->InjectText( text );
    }

    /** What the person does in the window of the browser: the e-mail address and the password, then the button */
    void PersonSignsIn( const char* password = TestPassword )
    {
        browser.PersonFill( "input[name=email]", TestEmail );
        browser.PersonFill( "input[name=\"password\"]", password );
        browser.PersonClick( "button#sign-in" );
    }

    /** The keyring of the workbench set up with the code and the credentials of the site saved in it; locked again when asked */
    bool PrepareSavedCredentials( bool lockAgain )
    {
        if ( AUTH::KEYRING_OK != credentials->GetKeyringSession().SetUpCode( AUTH::CSecretString( KeyringCode ) ) )
            return false;
        AUTH::CWebSignInCredentials saved;
        saved.Set( "username", AUTH::CSecretString( TestEmail ) );
        saved.Set( "password", AUTH::CSecretString( TestPassword ) );
        if ( !WebSignIn()->SaveCredentials( saved ) )
            return false;
        if ( lockAgain )
            credentials->GetKeyringSession().LockNow();
        return true;
    }

    /** The line of the web store on the page */
    CORE::CString WebLine( void ) { return Pane().GetStoreLine( StoreIndex( WebStoreId ) ); }

    UInt32 StoreIndex( const CORE::CString& storeId )
    {
        for ( UInt32 i=0; i<Pane().GetStoreCount(); ++i )
            if ( Pane().GetStoreId( i ) == storeId )
                return i;
        return 0;
    }

    bool ReportHas( const CORE::CString& text, GUI::CReportViewForm::ESeverity* severity = GUCEF_NULL )
    {
        GUI::CReportViewForm* report = shell->GetReport();
        for ( UInt32 i=0; i<report->GetEntryCount(); ++i )
        {
            const GUI::CReportViewForm::TEntry& entry = report->GetEntry( i );
            if ( entry.message.HasSubstr( text ) >= 0 || entry.details.HasSubstr( text ) >= 0 )
            {
                if ( GUCEF_NULL != severity )
                    *severity = entry.severity;
                return true;
            }
        }
        return false;
    }

    CORE::CString BoxText( GUI::CMessageBoxForm* box )
    {
        GUI::CWidget* text = box->GetWidget( "Text" );
        return GUCEF_NULL == text ? CORE::CString::Empty : text->GetProperty( "Text" );
    }

    private:

    CSignInFixture( const CSignInFixture& src );
    CSignInFixture& operator=( const CSignInFixture& src );
};

/*-------------------------------------------------------------------------*/

void
CollectWidgetTexts( GUI::CWidget* widget, CORE::CString::StringVector& texts )
{
    texts.push_back( widget->GetProperty( "Text" ) );
    texts.push_back( widget->GetProperty( "Tooltip" ) );
    const GUI::CWidget::TWidgetVector& children = widget->GetChildren();
    for ( UInt32 i=0; i<(UInt32) children.size(); ++i )
        CollectWidgetTexts( children[ i ], texts );
}

/** Whether a text of any widget of the context, hidden forms included, holds the text */
bool
AnyWidgetTextHas( GUI::CGUIContext* context, const CORE::CString& text )
{
    CORE::CString::StringVector texts;
    const GUI::CWidget::TWidgetVector& roots = context->GetRootWidgets();
    for ( UInt32 i=0; i<(UInt32) roots.size(); ++i )
        CollectWidgetTexts( roots[ i ], texts );
    for ( size_t i=0; i<texts.size(); ++i )
        if ( !text.IsNULLOrEmpty() && texts[ i ].HasSubstr( text ) >= 0 )
            return true;
    return false;
}

/** Whether the bytes of a file of the folder hold the text */
bool
FolderHolds( const CORE::CString& folder, const CORE::CString& text )
{
    const char* names[] = { "credentials.bin", "credentials.key", "credentials.lock", "keyring.bin" };
    for ( UInt32 i=0; i<4; ++i )
    {
        CORE::CString path = folder;
        CORE::AppendToPath( path, names[ i ] );
        CORE::CDynamicBuffer content;
        if ( !CORE::FileExists( path ) || !content.LoadContentFromFile( path ) || 0 == content.GetDataSize() || text.IsNULLOrEmpty() )
            continue;
        const UInt32 textSize = text.ByteSize() - 1;
        const char* bytes = content.AsConstTypePtr< char >();
        for ( UInt32 b=0; b+textSize<=content.GetDataSize(); ++b )
            if ( 0 == memcmp( bytes + b, text.C_String(), textSize ) )
                return true;
    }
    return false;
}

/** Whether a file of the folder is there */
bool
FolderHas( const CORE::CString& folder, const char* name )
{
    CORE::CString path = folder;
    CORE::AppendToPath( path, name );
    return CORE::FileExists( path );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

#define REQUIRE_READY( fixture )                                                                                   \
    if ( GetMainTestWindow().IsNULL() )                                                                            \
        GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );                                                       \
    GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsKeyringAvailable(), "The argon2id or xchacha20-poly1305 codec is not available" ); \
    ASSERT_TRUE( fixture.ready );                                                                                  \
    if ( !fixture.ready )                                                                                          \
        throw CORE::CString( "the fixture is not ready" )

static void
TestSignInCredentialStores( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SignIn: The workbench makes a key file store for the sessions and a keyring code store for the passwords in one folder, hands them to the asset store module, and takes them away again" )
    try
    {
        GUCEF_TESTFW_SKIP_IF( !AUTH::CFileCredentialStore::IsKeyringAvailable(), "The argon2id or xchacha20-poly1305 codec is not available" );
        ASSETSTORE::CAssetStoreGlobal* global = ASSETSTORE::CAssetStoreGlobal::Instance();
        const CORE::CString dir = MakeCleanTestDir( "signin_stores" );
        {
            CWorkbenchCredentials credentials;
            ASSERT_TRUE( CWorkbenchCredentials::GetDefaultFolder().HasSubstr( "AssetWorkbench" ) >= 0 && CWorkbenchCredentials::GetDefaultFolder().HasSubstr( "Auth" ) >= 0 );
            ASSERT_TRUE( credentials.Open( dir + "/auth" ) && credentials.GetFolder().HasSubstr( "auth" ) >= 0 );
            ASSERT_FALSE( credentials.Open( CORE::CString::Empty ) );
            ASSERT_FALSE( credentials.IsInstalled() );
            credentials.Install();
            ASSERT_TRUE( credentials.IsInstalled() );
            ASSERT_TRUE( global->GetWebSessionStore() == &credentials.GetSessionStore() && global->GetWebCredentialStore() == &credentials.GetKeyringStore() );
            ASSERT_TRUE( AUTH::CFileCredentialStore::KEYMODE_KEY_FILE == credentials.GetSessionStore().GetKeyMode() );
            ASSERT_TRUE( AUTH::CFileCredentialStore::KEYMODE_KEYRING_CODE == credentials.GetKeyringStore().GetKeyMode() );
            ASSERT_TRUE( credentials.GetSessionStore().GetDirectory() == credentials.GetKeyringStore().GetDirectory() );
            ASSERT_TRUE( &credentials.GetKeyringStore() == credentials.GetKeyringSession().GetStore() );

            // the idle lock and the folder come from the settings
            CAssetSourcesSettings settings;
            ASSERT_TRUE( 900 == settings.keyringIdleLockInSeconds && settings.credentialFolder.IsNULLOrEmpty() );
            ASSERT_TRUE( 900 == credentials.GetKeyringSession().GetSettings().idleLockInSeconds );
            settings.keyringIdleLockInSeconds = 37;
            settings.credentialFolder = dir + "/other";
            credentials.ApplySettings( settings );
            ASSERT_TRUE( 37 == credentials.GetKeyringSession().GetSettings().idleLockInSeconds );
            ASSERT_TRUE( credentials.GetFolder().HasSubstr( "other" ) >= 0 );
            settings.keyringIdleLockInSeconds = 0;
            settings.credentialFolder.Clear();
            credentials.ApplySettings( settings );
            ASSERT_TRUE( 0 == credentials.GetKeyringSession().GetSettings().idleLockInSeconds && credentials.GetFolder().HasSubstr( "other" ) >= 0 );

            // the keyring locks when the workbench ends, the stores leave the asset store module
            credentials.Open( dir + "/auth" );
            credentials.GetKeyringStore().SetKeyringParameters( 64, 1, 1 );
            ASSERT_TRUE( AUTH::KEYRING_OK == credentials.GetKeyringSession().SetUpCode( AUTH::CSecretString( KeyringCode ) ) );
            ASSERT_FALSE( credentials.GetKeyringSession().IsLocked() );
            ASSERT_TRUE( credentials.GetKeyringStore().Store( "assetstore-web-credentials", "fakeweb/username", AUTH::CSecretString( TestEmail ) ) );
            ASSERT_TRUE( credentials.GetSessionStore().Store( "assetstore-web-session", "fakeweb", AUTH::CSecretString( "cookie-jar-of-the-test" ) ) );
            credentials.Shutdown();
            ASSERT_FALSE( credentials.IsInstalled() );
            ASSERT_TRUE( GUCEF_NULL == global->GetWebSessionStore() && GUCEF_NULL == global->GetWebCredentialStore() );
            ASSERT_TRUE( credentials.GetKeyringSession().IsLocked() && credentials.GetKeyringSession().IsSetUp() );
            // the session store is read without the code, the passwords are not
            AUTH::CSecretString secret;
            ASSERT_TRUE( credentials.GetSessionStore().Load( "assetstore-web-session", "fakeweb", secret ) );
            ASSERT_FALSE( credentials.GetKeyringStore().Load( "assetstore-web-credentials", "fakeweb/username", secret ) );
            ASSERT_TRUE( FolderHas( dir + "/auth", "keyring.bin" ) && FolderHas( dir + "/auth", "credentials.bin" ) );
            ASSERT_FALSE( FolderHolds( dir + "/auth", TestEmail ) || FolderHolds( dir + "/auth", "cookie-jar-of-the-test" ) );
        }
        ASSERT_TRUE( GUCEF_NULL == global->GetWebSessionStore() );

        // only the folder and the idle lock are settings: no code, password or cookie
        CAssetSourcesSettings written;
        written.keyringIdleLockInSeconds = 42;
        written.credentialFolder = "$APPCONFIGDIR$/elsewhere";
        CORE::CDataNode config;
        ASSERT_TRUE( written.Save( config ) );
        CAssetSourcesSettings read;
        ASSERT_TRUE( read.Load( config ) );
        ASSERT_TRUE( 42 == read.keyringIdleLockInSeconds && "$APPCONFIGDIR$/elsewhere" == read.credentialFolder );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSignInManual( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SignIn: Sign in without saved credentials: a message box says a window opens (Cancel starts nothing), then the browser window, the prompt for the owner, the owner signs in, the row and the report follow" )
    try
    {
        CSignInFixture fixture( "signin_manual", true, false );
        REQUIRE_READY( fixture );
        CWorkbenchSignIn& signIn = fixture.SignIn();
        ASSERT_TRUE( fixture.SelectStore( WebStoreId ) );
        ASSERT_TRUE( "Fake web store - sign in required" == fixture.WebLine() && "Sign in required" == fixture.Pane().GetStoreStatusText() );
        ASSERT_TRUE( fixture.Pane().GetControl( "SignIn" )->GetBoolProperty( "Visible" ) && fixture.Pane().GetControl( "SignInSettings" )->GetBoolProperty( "Visible" ) );
        DumpPicture( fixture.Frames( 3 ), "signin-01-sources-signed-out" );

        // the notice before the window: Cancel and nothing starts
        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_TRUE( signIn.GetQuestionBox()->IsOpen() );
        ASSERT_TRUE( fixture.BoxText( signIn.GetQuestionBox() ).HasSubstr( "A browser window opens" ) >= 0 && fixture.BoxText( signIn.GetQuestionBox() ).HasSubstr( "Fake web store" ) >= 0 );
        DumpPicture( fixture.Frames( 3 ), "signin-02-browser-window-notice" );
        signIn.GetQuestionBox()->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_CANCEL );
        fixture.Frames( 5 );
        ASSERT_TRUE( 0 == fixture.browser.GetStartCount() && !fixture.WebSignIn()->IsSignInRunning() );
        ASSERT_TRUE( fixture.ReportHas( "The sign in was not started" ) );

        // OK: the window opens and the owner is asked to sign in there (no credentials are saved to fill in)
        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_TRUE( signIn.GetQuestionBox()->IsOpen() );
        signIn.GetQuestionBox()->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return signIn.GetNoticeBox()->IsOpen(); } ) );
        ASSERT_TRUE( 1 == fixture.browser.GetStartCount() && !fixture.browser.GetLastSettings().headless );
        ASSERT_TRUE( !fixture.BoxText( signIn.GetNoticeBox() ).IsNULLOrEmpty() && fixture.BoxText( signIn.GetNoticeBox() ).HasSubstr( "browser window" ) >= 0 );
        GUI::CReportViewForm::ESeverity severity = GUI::CReportViewForm::SEVERITY_INFO;
        ASSERT_TRUE( fixture.ReportHas( "Owner action needed", &severity ) && GUI::CReportViewForm::SEVERITY_WARNING == severity );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return fixture.ReportHas( "A browser window opens to sign in to" ); } ) );
        ASSERT_TRUE( fixture.WebSignIn()->IsSignInRunning() && !fixture.web->IsSignedIn() );
        DumpPicture( fixture.Frames( 3 ), "signin-03-owner-prompt" );

        // the owner signs in in the window of the store
        fixture.PersonSignsIn();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "Fake web store - signed in" == fixture.WebLine() && !signIn.GetNoticeBox()->IsOpen() && fixture.ReportHas( "Signed in" ); } ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !fixture.Pane().GetControl( "SignIn" )->GetBoolProperty( "Visible" ); } ) );
        ASSERT_TRUE( fixture.Pane().GetControl( "SignInSettings" )->GetBoolProperty( "Visible" ) );
        ASSERT_TRUE( fixture.ReportHas( "Signed in", &severity ) && GUI::CReportViewForm::SEVERITY_INFO == severity );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "1 items" == fixture.Pane().GetStoreStatusText(); } ) );
        DumpPicture( fixture.Frames( 3 ), "signin-04-sources-signed-in" );
        ASSERT_TRUE( 1 == fixture.browser.GetStartCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSignInFailures( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SignIn: A refused password ends the sign in with the outcome in a message box and in the report, a window that the owner closes ends it in the report; the row stays signed out" )
    try
    {
        CSignInFixture fixture( "signin_failed", true, false );
        REQUIRE_READY( fixture );
        CWorkbenchSignIn& signIn = fixture.SignIn();
        ASSERT_TRUE( fixture.PrepareSavedCredentials( false ) );
        fixture.site.SetAccount( TestEmail, "another password than the saved one" );
        ASSERT_TRUE( fixture.SelectStore( WebStoreId ) );

        // the saved password is refused by the automatic sign in, then by the sign in with the window the owner asks for
        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return signIn.GetQuestionBox()->IsOpen(); } ) );
        ASSERT_TRUE( fixture.BoxText( signIn.GetQuestionBox() ).HasSubstr( "refused" ) >= 0 );
        signIn.GetQuestionBox()->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_YES );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return signIn.GetNoticeBox()->IsOpen(); } ) );
        ASSERT_TRUE( fixture.BoxText( signIn.GetNoticeBox() ).HasSubstr( "did not succeed" ) >= 0 && fixture.BoxText( signIn.GetNoticeBox() ).HasSubstr( "refused" ) >= 0 );
        GUI::CReportViewForm::ESeverity severity = GUI::CReportViewForm::SEVERITY_INFO;
        ASSERT_TRUE( fixture.ReportHas( "The sign in did not succeed", &severity ) && GUI::CReportViewForm::        SEVERITY_FAILURE == severity );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !fixture.WebSignIn()->IsSignInRunning(); } ) );
        ASSERT_TRUE( "Fake web store - sign in required" == fixture.WebLine() && 2 == fixture.browser.GetStartCount() );
        signIn.GetNoticeBox()->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );
        ASSERT_FALSE( signIn.GetQuestionBox()->IsOpen() );

        // the owner closes the window: no message box, an entry in the report
        ASSERT_TRUE( fixture.credentials->GetKeyringStore().IsKeyringSetUp() && fixture.WebSignIn()->ForgetCredentials() );
        const UInt32 entries = fixture.shell->GetReport()->GetEntryCount();
        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_TRUE( signIn.GetQuestionBox()->IsOpen() );
        signIn.GetQuestionBox()->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return signIn.GetNoticeBox()->IsOpen(); } ) );
        signIn.GetNoticeBox()->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );
        fixture.browser.PersonClosesWindow();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !fixture.WebSignIn()->IsSignInRunning() && fixture.shell->GetReport()->GetEntryCount() > entries + 2; } ) );
        ASSERT_TRUE( fixture.ReportHas( "The sign in did not succeed: cancelled" ) );
        ASSERT_FALSE( signIn.GetNoticeBox()->IsOpen() );
        ASSERT_TRUE( "Fake web store - sign in required" == fixture.WebLine() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSignInAutomatic( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SignIn: Saved credentials behind a locked keyring: the keyring form asks for the code (a wrong code, then the right one), then the sign in runs by itself without a window and without a message box" )
    try
    {
        CSignInFixture fixture( "signin_auto", true, false );
        REQUIRE_READY( fixture );
        CWorkbenchSignIn& signIn = fixture.SignIn();
        ASSERT_TRUE( fixture.PrepareSavedCredentials( true ) );
        ASSERT_TRUE( fixture.credentials->GetKeyringSession().IsLocked() && !fixture.WebSignIn()->HasCredentials() );
        CEventCounter questions( signIn.GetQuestionBox(), GUI::CMessageBoxForm::ClosedEvent );
        ASSERT_TRUE( fixture.SelectStore( WebStoreId ) );

        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_TRUE( signIn.GetUnlockForm()->IsOpen() && AUTHGUI::CKeyringUnlockForm::MODE_UNLOCK == signIn.GetUnlockForm()->GetMode() );
        ASSERT_FALSE( signIn.GetQuestionBox()->IsOpen() );
        DumpPicture( fixture.Frames( 3 ), "signin-05-unlock-form" );

        fixture.Type( signIn.GetUnlockForm(), "Code", WrongKeyringCode );
        ASSERT_TRUE( signIn.GetUnlockForm()->Submit() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "Wrong code." == signIn.GetUnlockForm()->GetMessageText(); } ) );
        ASSERT_TRUE( signIn.GetUnlockForm()->IsOpen() && fixture.credentials->GetKeyringSession().IsLocked() && 0 == fixture.browser.GetStartCount() );

        fixture.Type( signIn.GetUnlockForm(), "Code", KeyringCode );
        ASSERT_TRUE( signIn.GetUnlockForm()->Submit() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "Fake web store - signed in" == fixture.WebLine(); } ) );
        ASSERT_TRUE( !signIn.GetUnlockForm()->IsOpen() && !fixture.credentials->GetKeyringSession().IsLocked() );
        ASSERT_TRUE( 0 == questions.count && !signIn.GetQuestionBox()->IsOpen() && !signIn.GetNoticeBox()->IsOpen() );
        ASSERT_TRUE( 1 == fixture.browser.GetStartCount() && fixture.browser.GetLastSettings().headless && 2 == fixture.browser.GetFillCount() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return fixture.ReportHas( "Signing in with the saved credentials" ) && fixture.ReportHas( "Signed in" ); } ) );

        // signed out and in again: the keyring is open, no form at all
        ASSERT_TRUE( signIn.SignOut( WebStoreId ) );
        ASSERT_TRUE( "Fake web store - sign in required" == fixture.WebLine() );
        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_FALSE( signIn.GetUnlockForm()->IsOpen() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "Fake web store - signed in" == fixture.WebLine(); } ) );
        ASSERT_TRUE( 2 == fixture.browser.GetStartCount() && 0 == questions.count );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSignInUnlockCancelled( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SignIn: When the owner gives up the keyring form, the sign in with the window is offered and the saved credentials stay locked" )
    try
    {
        CSignInFixture fixture( "signin_cancel", true, false );
        REQUIRE_READY( fixture );
        CWorkbenchSignIn& signIn = fixture.SignIn();
        ASSERT_TRUE( fixture.PrepareSavedCredentials( true ) );
        ASSERT_TRUE( fixture.SelectStore( WebStoreId ) );
        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_TRUE( signIn.GetUnlockForm()->IsOpen() );
        signIn.GetUnlockForm()->Cancel();
        ASSERT_TRUE( !signIn.GetUnlockForm()->IsOpen() && signIn.GetQuestionBox()->IsOpen() );
        ASSERT_TRUE( fixture.BoxText( signIn.GetQuestionBox() ).HasSubstr( "A browser window opens" ) >= 0 );
        ASSERT_TRUE( fixture.credentials->GetKeyringSession().IsLocked() && 0 == fixture.browser.GetStartCount() );

        // go on with the window: the saved credentials cannot be read, the owner types them
        signIn.GetQuestionBox()->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return signIn.GetNoticeBox()->IsOpen(); } ) );
        ASSERT_TRUE( 1 == fixture.browser.GetStartCount() && !fixture.browser.GetLastSettings().headless && 0 == fixture.browser.GetFillCount() );
        fixture.PersonSignsIn();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "Fake web store - signed in" == fixture.WebLine(); } ) );
        ASSERT_TRUE( fixture.credentials->GetKeyringSession().IsLocked() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSignInAutomaticNeedsOwner( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SignIn: An automatic sign in that ends because a person has to act is offered again with the window, the saved credentials are filled in and the owner does the step" )
    try
    {
        CSignInFixture fixture( "signin_autoowner", true, false );
        REQUIRE_READY( fixture );
        CWorkbenchSignIn& signIn = fixture.SignIn();
        ASSERT_TRUE( fixture.PrepareSavedCredentials( false ) );
        fixture.site.SetChallenge( true );
        ASSERT_TRUE( fixture.SelectStore( WebStoreId ) );
        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_FALSE( signIn.GetUnlockForm()->IsOpen() );

        // the automatic sign in runs without a window and meets the puzzle
        ASSERT_TRUE( fixture.WaitFor( [&]() { return signIn.GetQuestionBox()->IsOpen(); } ) );
        ASSERT_TRUE( fixture.browser.GetLastSettings().headless && 2 == fixture.browser.GetFillCount() );
        ASSERT_TRUE( fixture.BoxText( signIn.GetQuestionBox() ).HasSubstr( "could not finish by itself" ) >= 0 && fixture.BoxText( signIn.GetQuestionBox() ).HasSubstr( "Sign in with a browser window?" ) >= 0 );
        GUI::CReportViewForm::ESeverity severity = GUI::CReportViewForm::SEVERITY_INFO;
        ASSERT_TRUE( fixture.ReportHas( "The sign in did not succeed", &severity ) && GUI::CReportViewForm::        SEVERITY_FAILURE == severity );
        DumpPicture( fixture.Frames( 3 ), "signin-06-automatic-needs-owner" );

        // the answer No leaves the row as it is; Yes goes on with the window
        signIn.GetQuestionBox()->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_NO );
        fixture.Frames( 5 );
        ASSERT_TRUE( 1 == fixture.browser.GetStartCount() && "Fake web store - sign in required" == fixture.WebLine() );
        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return signIn.GetQuestionBox()->IsOpen(); } ) );
        signIn.GetQuestionBox()->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_YES );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return fixture.BoxText( signIn.GetNoticeBox() ).HasSubstr( "Solve the puzzle in the browser window" ) >= 0; } ) );
        ASSERT_TRUE( 3 == fixture.browser.GetStartCount() && !fixture.browser.GetLastSettings().headless && 6 == fixture.browser.GetFillCount() );
        ASSERT_TRUE( signIn.GetNoticeBox()->IsOpen() );
        ASSERT_TRUE( fixture.ReportHas( "Owner action needed: Solve the puzzle in the browser window", &severity ) && GUI::CReportViewForm::SEVERITY_WARNING == severity );
        DumpPicture( fixture.Frames( 3 ), "signin-10-owner-prompt-puzzle" );
        fixture.browser.PersonFill( "#answer", "person" );
        fixture.browser.PersonClick( "#solve" );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "Fake web store - signed in" == fixture.WebLine() && !signIn.GetNoticeBox()->IsOpen(); } ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSignInRestart( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SignIn: The session is reused after the workbench was started again, without the keyring code; signing out forgets it for good" )
    try
    {
        CSignInFixture fixture( "signin_restart", true, false );
        REQUIRE_READY( fixture );
        ASSERT_TRUE( fixture.PrepareSavedCredentials( true ) );
        ASSERT_TRUE( fixture.SelectStore( WebStoreId ) );
        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_TRUE( fixture.SignIn().GetUnlockForm()->IsOpen() );
        fixture.Type( fixture.SignIn().GetUnlockForm(), "Code", KeyringCode );
        ASSERT_TRUE( fixture.SignIn().GetUnlockForm()->Submit() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "Fake web store - signed in" == fixture.WebLine(); } ) );

        // the workbench ends (the keyring locks) and starts again on the same folder
        ASSERT_TRUE( fixture.Restart() );
        ASSERT_TRUE( fixture.credentials->GetKeyringSession().IsLocked() && fixture.credentials->GetKeyringSession().IsSetUp() );
        ASSERT_TRUE( fixture.web->IsSignedIn() );
        ASSERT_TRUE( fixture.SelectStore( WebStoreId ) );
        ASSERT_TRUE( "Fake web store - signed in" == fixture.WebLine() && !fixture.Pane().GetControl( "SignIn" )->GetBoolProperty( "Visible" ) );
        ASSERT_TRUE( !fixture.SignIn().GetUnlockForm()->IsOpen() && 1 == fixture.browser.GetStartCount() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "1 items" == fixture.Pane().GetStoreStatusText(); } ) );

        // signed out: gone from the store, also after another start; the saved credentials are not touched
        ASSERT_TRUE( fixture.SignIn().SignOut( WebStoreId ) );
        ASSERT_TRUE( "Fake web store - sign in required" == fixture.WebLine() );
        ASSERT_TRUE( fixture.Restart() );
        ASSERT_FALSE( fixture.web->IsSignedIn() );
        ASSERT_TRUE( fixture.credentials->GetKeyringSession().IsSetUp() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSignInSessionEnded( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SignIn: An action of the library that finds the session over signs in again once, by itself with the saved credentials, and is tried again once; when that does not help the error stays and nothing loops" )
    try
    {
        CSignInFixture fixture( "signin_ended", true, false );
        REQUIRE_READY( fixture );
        ASSERT_TRUE( fixture.PrepareSavedCredentials( false ) );
        ASSERT_TRUE( fixture.SelectStore( WebStoreId ) );
        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "Fake web store - signed in" == fixture.WebLine() && "1 items" == fixture.Pane().GetStoreStatusText(); } ) );
        ASSERT_TRUE( 1 == fixture.browser.GetStartCount() );

        // the store ends the session: the library is read again, finds out, the workbench signs in again unattended and reads once more
        fixture.site.ExpireSessions();
        CEventCounter expired( fixture.shell->GetSources(), CAssetSourcesPane::SessionExpiredEvent );
        CEventCounter reads( fixture.Pane().GetLibrary( WebStoreId ), ASSETSTORE::CAssetStoreLibraryFileSystemAccess::LibraryChangedEvent );
        ASSERT_TRUE( fixture.Pane().GetLibrary( WebStoreId )->Refresh() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 1 == expired.count && 2 == fixture.browser.GetStartCount(); } ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 1 == reads.count && "Fake web store - signed in" == fixture.WebLine() && "1 items" == fixture.Pane().GetStoreStatusText(); } ) );
        ASSERT_TRUE( fixture.browser.GetLastSettings().headless && 1 == expired.count );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return fixture.ReportHas( "The session ended" ) && !fixture.Pane().GetControl( "SignIn" )->GetBoolProperty( "Visible" ); } ) );
        ASSERT_TRUE( !fixture.SignIn().GetQuestionBox()->IsOpen() );

        // the store refuses every session: one sign in again, one more try, then the error, and no more sign ins
        CWorkbenchTestWebProvider::AlwaysExpired() = true;
        ASSERT_TRUE( fixture.Pane().GetLibrary( WebStoreId )->Refresh() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 2 == expired.count; } ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return fixture.Pane().GetStoreStatusText().HasSubstr( "did not help" ) >= 0; } ) );
        const UInt32 starts = fixture.browser.GetStartCount();
        ASSERT_TRUE( 3 == starts );
        fixture.Frames( 60 );
        ASSERT_TRUE( starts == fixture.browser.GetStartCount() && 2 == expired.count );
        ASSERT_TRUE( "Fake web store - session ended" == fixture.WebLine() && fixture.Pane().GetControl( "SignIn" )->GetBoolProperty( "Visible" ) );
        CWorkbenchTestWebProvider::AlwaysExpired() = false;

        // the owner signs in and the actions have their try again
        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "Fake web store - signed in" == fixture.WebLine() && "1 items" == fixture.Pane().GetStoreStatusText(); } ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSignInItemDetails( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SignIn: The details of an item that find the session over are asked again once after the sign in, and then show" )
    try
    {
        CSignInFixture fixture( "signin_item", true, false );
        REQUIRE_READY( fixture );
        ASSERT_TRUE( fixture.PrepareSavedCredentials( false ) );
        ASSERT_TRUE( fixture.SelectStore( WebStoreId ) );
        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "1 items" == fixture.Pane().GetStoreStatusText(); } ) );

        CEventCounter expired( fixture.shell->GetSources(), CAssetSourcesPane::SessionExpiredEvent );
        fixture.site.ExpireSessions();
        ASSERT_TRUE( fixture.Pane().SelectItem( WebStoreId, "pack-1" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return 1 == expired.count && 2 == fixture.browser.GetStartCount(); } ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !fixture.Pane().GetFormatChoices().empty(); } ) );
        ASSERT_TRUE( 1 == expired.count && "gltf" == fixture.Pane().GetSelectedFormatId() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSignInSettings( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SignIn: The sign in settings of a store: save credentials (a keyring code is set up first), lock now, forget saved credentials (the code is asked), change the keyring code, sign in and sign out; no secret reaches a log, a widget or a file" )
    try
    {
        TESTFW::CLogTextCapture capture;
        CSignInFixture fixture( "signin_settings", true, false );
        REQUIRE_READY( fixture );
        CWorkbenchSignIn& signIn = fixture.SignIn();
        AUTHGUI::CSignInSettingsForm* settings = signIn.GetSettingsForm();
        AUTHGUI::CKeyringUnlockForm* unlock = signIn.GetUnlockForm();
        AUTH::CKeyringSession& session = fixture.credentials->GetKeyringSession();
        ASSERT_TRUE( fixture.SelectStore( WebStoreId ) );

        // from the row of the store: the panel with its state
        fixture.Click( fixture.Pane().GetControl( "SignInSettings" ) );
        ASSERT_TRUE( settings->IsOpen() && WebStoreId == signIn.GetSettingsStoreId() );
        ASSERT_TRUE( "Not signed in" == settings->GetStatusText() && AUTHGUI::CSignInSettingsState::KEYRING_NOT_SET_UP == settings->GetState().keyringState );
        ASSERT_TRUE( 2 == settings->GetState().credentialSlots.size() && "Fake web store" == settings->GetState().serviceName );
        ASSERT_TRUE( "Set up keyring code..." == settings->GetWidget( "KeyringCode" )->GetProperty( "Text" ) );
        DumpPicture( fixture.Frames( 3 ), "signin-07-settings-panel" );

        // Save credentials without a keyring: the form for a new code first, then the save
        fixture.Type( settings, "Slot0", TestEmail );
        fixture.Type( settings, "Slot1", TestPassword );
        ASSERT_TRUE( settings->SaveCredentials() );
        ASSERT_TRUE( unlock->IsOpen() && AUTHGUI::CKeyringUnlockForm::MODE_SET_UP == unlock->GetMode() );
        ASSERT_TRUE( 0 == settings->GetWidgetAs< GUI::CSecretInput >( "Slot0" )->GetValueLength() && 0 == settings->GetWidgetAs< GUI::CSecretInput >( "Slot1" )->GetValueLength() );
        fixture.Type( unlock, "Code", KeyringCode );
        fixture.Type( unlock, "Confirm", KeyringCode );
        DumpPicture( fixture.Frames( 3 ), "signin-08-settings-and-set-up-form" );
        ASSERT_TRUE( unlock->Submit() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "Credentials saved." == settings->GetMessageText(); } ) );
        ASSERT_TRUE( !unlock->IsOpen() && !session.IsLocked() && session.IsSetUp() );
        ASSERT_TRUE( fixture.WebSignIn()->HasCredentials() );
        ASSERT_TRUE( AUTHGUI::CSignInSettingsState::KEYRING_UNLOCKED == settings->GetState().keyringState && settings->GetState().hasSavedCredentials );
        ASSERT_TRUE( "Change keyring code..." == settings->GetWidget( "KeyringCode" )->GetProperty( "Text" ) && settings->IsActionEnabled( "LockNow" ) );
        ASSERT_TRUE( FolderHas( fixture.dir + "/auth", "keyring.bin" ) );

        // Lock now: the saved credentials cannot be read
        ASSERT_TRUE( settings->LockNow() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return session.IsLocked(); } ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return AUTHGUI::CSignInSettingsState::KEYRING_LOCKED == settings->GetState().keyringState; } ) );
        ASSERT_TRUE( !fixture.WebSignIn()->HasCredentials() && !settings->IsActionEnabled( "LockNow" ) );

        // Forget while locked: the code is asked, a wrong code first, then the credentials are gone
        ASSERT_TRUE( settings->ForgetCredentials() );
        ASSERT_TRUE( unlock->IsOpen() && AUTHGUI::CKeyringUnlockForm::MODE_UNLOCK == unlock->GetMode() );
        fixture.Type( unlock, "Code", WrongKeyringCode );
        ASSERT_TRUE( unlock->Submit() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "Wrong code." == unlock->GetMessageText(); } ) );
        fixture.Type( unlock, "Code", KeyringCode );
        ASSERT_TRUE( unlock->Submit() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "The saved credentials are forgotten." == settings->GetMessageText(); } ) );
        ASSERT_TRUE( !fixture.WebSignIn()->HasCredentials() && !settings->GetState().hasSavedCredentials );

        // the owner gives up the code while saving: nothing is saved and nothing stays in memory
        session.LockNow();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return session.IsLocked(); } ) );
        fixture.Type( settings, "Slot0", TestEmail );
        fixture.Type( settings, "Slot1", TestPassword );
        ASSERT_TRUE( settings->SaveCredentials() );
        ASSERT_TRUE( unlock->IsOpen() );
        unlock->Cancel();
        ASSERT_TRUE( settings->GetMessageText().HasSubstr( "Not saved" ) >= 0 && !fixture.WebSignIn()->HasCredentials() );

        // change the keyring code: the current one and the new one twice
        ASSERT_TRUE( settings->KeyringCode() );
        ASSERT_TRUE( unlock->IsOpen() && AUTHGUI::CKeyringUnlockForm::MODE_CHANGE_CODE == unlock->GetMode() );
        fixture.Type( unlock, "Current", KeyringCode );
        fixture.Type( unlock, "Code", NewKeyringCode );
        fixture.Type( unlock, "Confirm", NewKeyringCode );
        ASSERT_TRUE( unlock->Submit() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !unlock->IsOpen(); } ) );
        ASSERT_TRUE( !session.IsLocked() );

        // saving with the keyring open needs no form; the new code is the one that unlocks
        fixture.Type( settings, "Slot0", TestEmail );
        fixture.Type( settings, "Slot1", TestPassword );
        ASSERT_TRUE( settings->SaveCredentials() );
        ASSERT_FALSE( unlock->IsOpen() );
        ASSERT_TRUE( "Credentials saved." == settings->GetMessageText() && fixture.WebSignIn()->HasCredentials() );
        session.LockNow();
        ASSERT_TRUE( fixture.WaitFor( [&]() { return session.IsLocked(); } ) );
        ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == session.UnlockWithCode( AUTH::CSecretString( KeyringCode ) ) );
        ASSERT_TRUE( AUTH::KEYRING_OK == session.UnlockWithCode( AUTH::CSecretString( NewKeyringCode ) ) && fixture.WebSignIn()->HasCredentials() );

        // Sign in from the panel: the saved credentials make it automatic; Sign out takes the session away again
        ASSERT_TRUE( settings->SignIn() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return settings->GetState().signedIn && "Signed in" == settings->GetStatusText(); } ) );
        ASSERT_TRUE( !settings->IsActionEnabled( "SignIn" ) && settings->IsActionEnabled( "SignOut" ) );
        ASSERT_TRUE( "Fake web store - signed in" == fixture.WebLine() );
        ASSERT_TRUE( settings->SignOut() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return !settings->GetState().signedIn && "Not signed in" == settings->GetStatusText(); } ) );
        ASSERT_TRUE( "Fake web store - sign in required" == fixture.WebLine() && !fixture.web->IsSignedIn() );
        settings->CloseWindow();
        ASSERT_TRUE( signIn.GetSettingsStoreId().IsNULLOrEmpty() );

        // none of the secrets is in a log, a text of a widget (the forms that are closed too) or a file of the credential stores
        const CORE::CString secrets[] = { TestEmail, TestPassword, KeyringCode, WrongKeyringCode, NewKeyringCode };
        const CORE::CString logged = capture.GetText();
        ASSERT_TRUE( logged.Length() > 0 );
        for ( UInt32 i=0; i<5; ++i )
        {
            ASSERT_FALSE( logged.HasSubstr( secrets[ i ] ) >= 0 );
            ASSERT_FALSE( AnyWidgetTextHas( fixture.context, secrets[ i ] ) );
            ASSERT_FALSE( FolderHolds( fixture.dir + "/auth", secrets[ i ] ) );
        }
        const CORE::CString::StringVector sessionValues = fixture.site.GetIssuedSessionValues();
        ASSERT_TRUE( !sessionValues.empty() );
        for ( size_t i=0; i<sessionValues.size(); ++i )
        {
            ASSERT_FALSE( logged.HasSubstr( sessionValues[ i ] ) >= 0 );
            ASSERT_FALSE( AnyWidgetTextHas( fixture.context, sessionValues[ i ] ) );
            ASSERT_FALSE( FolderHolds( fixture.dir + "/auth", sessionValues[ i ] ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

namespace {

/** Takes the place of the browser of the system: remembers the addresses */
class CRecordingLauncher : public CORE::CIBrowserLauncherHandler
{
    public:

    CORE::CString::StringVector urls;
    CORE::CIBrowserLauncherHandler* previous;

    CRecordingLauncher( void ) : CORE::CIBrowserLauncherHandler(), urls(), previous( GUCEF_NULL )
    {
        previous = CORE::SetBrowserLauncherHandler( this );
    }

    virtual ~CRecordingLauncher() { CORE::SetBrowserLauncherHandler( previous ); }

    virtual bool OpenUrl( const CORE::CString& url ) GUCEF_VIRTUAL_OVERRIDE
    {
        urls.push_back( url );
        return true;
    }
};

}

static void
TestSignInOAuthCodeEntry( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SignIn: A store that signs in with a provider of gucefAUTH that shows a code: the address, copy and open in the browser, the code pasted, each refusal in its own words, taken, and the login ends; cancel and a refused code too" )
    try
    {
        TESTFW::CLogTextCapture capture;
        CRecordingLauncher launcher;
        CSignInFixture fixture( "signin_oauth", false, true );
        REQUIRE_READY( fixture );
        CWorkbenchSignIn& signIn = fixture.SignIn();

        // the manager of gucefAUTH of the workbench, with a provider that asks for a page and waits, and the redirect mode that shows a code
        TFakeAuthFactory authFactory;
        AUTH::CMemoryCredentialStore authCredentials;
        AUTH::CAuthManager manager;
        manager.SetCredentialStore( &authCredentials );
        AUTH::CIAuthProviderPtr product = authFactory.Create();
        CFakeAuthProvider* provider = static_cast< CFakeAuthProvider* >( product.GetPointerAlways() );
        ASSERT_TRUE( manager.AddProvider( product ) );
        AUTH::CAuthSettings authSettings;
        AUTH::CAuthProviderSettings providerSettings;
        providerSettings.redirectMode = "code-entry";
        authSettings.providers[ "fake-auth" ] = providerSettings;
        manager.SetSettings( authSettings );
        CFakeCodeEngine engine;
        fixture.shell->SetAuthManager( &manager );
        signIn.SetFlowEngine( &engine );

        AUTHGUI::COAuthCodeEntryForm* form = signIn.GetCodeEntryForm();
        ASSERT_TRUE( fixture.SelectStore( "fakesrc" ) );
        ASSERT_TRUE( "Fake Store - sign in required" == fixture.Pane().GetStoreLine( 1 ) );
        fixture.Click( fixture.Pane().GetControl( "SignIn" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return form->IsOpen(); } ) );
        ASSERT_TRUE( 1 == provider->beginCount );
        ASSERT_TRUE( OAuthUrl == form->GetAuthorizationUrl() && "Fake Store" == form->GetProviderDisplayName() );
        ASSERT_TRUE( fixture.ReportHas( "Owner action needed: sign in on the page of the store" ) );
        DumpPicture( fixture.Frames( 3 ), "signin-09-code-entry" );

        // the address: copied and opened in the browser
        ASSERT_TRUE( form->CanOpenUrl() && form->OpenUrl() );
        ASSERT_TRUE( 1 == launcher.urls.size() && OAuthUrl == launcher.urls[ 0 ] );

        // the outcomes that are not taken keep the form open, each with its words; the code that is taken closes it
        const AUTH::TCodeCheck refusals[] = { AUTH::CODECHECK_NO_LOGIN_WAITING, AUTH::CODECHECK_AMBIGUOUS, AUTH::CODECHECK_MALFORMED, AUTH::CODECHECK_REFUSED };
        CORE::CString::StringVector messages;
        for ( UInt32 i=0; i<4; ++i )
        {
            engine.answer = refusals[ i ];
            fixture.Type( form, "Code", OAuthCode );
            ASSERT_FALSE( form->Submit() );
            ASSERT_TRUE( form->IsOpen() && form->GetMessageText() == AUTHGUI::COAuthCodeEntryForm::CheckToText( refusals[ i ] ) );
            for ( size_t j=0; j<messages.size(); ++j )
                ASSERT_TRUE( messages[ j ] != form->GetMessageText() );
            messages.push_back( form->GetMessageText() );
        }
        ASSERT_TRUE( 4 == engine.count && OAuthCode == engine.lastText );
        ASSERT_FALSE( AnyWidgetTextHas( fixture.context, OAuthCode ) );
        engine.answer = AUTH::CODECHECK_ACCEPTED;
        fixture.Type( form, "Code", OAuthCode );
        ASSERT_TRUE( form->Submit() );
        ASSERT_TRUE( !form->IsOpen() && fixture.ReportHas( "The code was accepted" ) );

        // the login goes on at the provider and ends through the manager
        ASSERT_TRUE( provider->Succeed() );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return fixture.ReportHas( "Signed in" ); } ) );
        fixture.fake->SetSignedIn( true );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return "Fake Store - signed in" == fixture.Pane().GetStoreLine( 1 ); } ) );
        ASSERT_FALSE( manager.IsLoginPending( "fake-auth" ) );

        // cancel: the login is asked to stop
        fixture.fake->SetSignedIn( false );
        ASSERT_TRUE( signIn.SignIn( "fakesrc" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return form->IsOpen(); } ) && 2 == provider->beginCount );
        form->Cancel();
        ASSERT_TRUE( fixture.ReportHas( "The sign in was cancelled" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return provider->HonourCancel() || !manager.IsLoginPending( "fake-auth" ); } ) );
        fixture.Frames( 5 );
        ASSERT_FALSE( manager.IsLoginPending( "fake-auth" ) );

        // a code that the provider refuses at its end: the form closes, a message box says so
        ASSERT_TRUE( signIn.SignIn( "fakesrc" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return form->IsOpen(); } ) && 3 == provider->beginCount );
        fixture.Type( form, "Code", OAuthCode );
        ASSERT_TRUE( form->Submit() );
        ASSERT_TRUE( provider->Fail( "The store refused the code" ) );
        ASSERT_TRUE( fixture.WaitFor( [&]() { return signIn.GetNoticeBox()->IsOpen(); } ) );
        ASSERT_TRUE( fixture.BoxText( signIn.GetNoticeBox() ).HasSubstr( "The store refused the code" ) >= 0 && !form->IsOpen() );
        ASSERT_TRUE( fixture.ReportHas( "The sign in did not succeed" ) );

        // the code and the state of the address were in no log
        const CORE::CString logged = capture.GetText();
        ASSERT_FALSE( logged.HasSubstr( OAuthCode ) >= 0 || logged.HasSubstr( "workbench-state-3f" ) >= 0 );
        fixture.shell->SetAuthManager( GUCEF_NULL );
        signIn.SetFlowEngine( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSignInWithoutService( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SignIn: Without a sign in service for a store of gucefAUTH, or without credential stores, the owner is told and nothing is saved; a store without a sign in has no buttons" )
    try
    {
        CSignInFixture fixture( "signin_noservice", true, true );
        REQUIRE_READY( fixture );
        CWorkbenchSignIn& signIn = fixture.SignIn();
        ASSERT_TRUE( fixture.SelectStore( "fakesrc" ) );
        ASSERT_FALSE( signIn.SignIn( "fakesrc" ) );
        ASSERT_TRUE( signIn.GetNoticeBox()->IsOpen() && fixture.BoxText( signIn.GetNoticeBox() ).HasSubstr( "sign in service" ) >= 0 );
        ASSERT_FALSE( signIn.SignIn( "unknown-store" ) || signIn.OpenSettings( "unknown-store" ) );
        ASSERT_FALSE( signIn.OnSessionExpired( "unknown-store" ) );

        // the primary store has no sign in
        ASSERT_TRUE( fixture.Pane().SelectStore( "primary" ) );
        fixture.Frames( 3 );
        ASSERT_FALSE( fixture.Pane().GetControl( "SignIn" )->GetBoolProperty( "Visible" ) || fixture.Pane().GetControl( "SignInSettings" )->GetBoolProperty( "Visible" ) );
        ASSERT_FALSE( signIn.OpenSettings( "primary" ) );

        // a workbench without credential stores: the settings say so, saving is refused, the sign in goes on by the window only
        fixture.TearDownShell();
        fixture.shell = GUCEF_NEW CWorkbenchShell( fixture.mounter );
        ASSERT_TRUE( fixture.shell->Initialize( *fixture.context, GUCEF_NULL, MakeCleanTestDir( "signin_noservice_cache2" ) ) );
        ASSERT_TRUE( fixture.shell->InitializeSources( fixture.settings ) && GUCEF_NULL != fixture.shell->GetSignIn() );
        fixture.shell->ShowLeftTab( 2 );
        CWorkbenchSignIn& plain = *fixture.shell->GetSignIn();
        ASSERT_TRUE( plain.OpenSettings( WebStoreId ) );
        ASSERT_TRUE( AUTHGUI::CSignInSettingsState::KEYRING_UNAVAILABLE == plain.GetSettingsForm()->GetState().keyringState );
        ASSERT_FALSE( plain.GetSettingsForm()->IsActionEnabled( "SaveCredentials" ) || plain.GetSettingsForm()->IsActionEnabled( "KeyringCode" ) );
        ASSERT_TRUE( plain.GetSettingsForm()->IsActionEnabled( "SignIn" ) );
        plain.GetSettingsForm()->CloseWindow();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      MODULE TESTS                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetWorkbenchSignInTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbenchLib.SignIn" );

    TestSignInCredentialStores();
    TestSignInManual();
    TestSignInFailures();
    TestSignInAutomatic();
    TestSignInUnlockCancelled();
    TestSignInAutomaticNeedsOwner();
    TestSignInRestart();
    TestSignInSessionEnded();
    TestSignInItemDetails();
    TestSignInSettings();
    TestSignInOAuthCodeEntry();
    TestSignInWithoutService();
}

/*-------------------------------------------------------------------------*/
