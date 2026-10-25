/*
 *  gucefGUI_WaylandTestApp: Tests of the Wayland window and input drivers against a fake compositor
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

#include <pthread.h>
#include <string.h>
#include <time.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CLOGSTREAMSCOPE_H
#include "gucefCORE_CLogStreamScope.h"
#define GUCEF_CORE_CLOGSTREAMSCOPE_H
#endif /* GUCEF_CORE_CLOGSTREAMSCOPE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_H
#include "pluginglueWAYLAND.h"
#define GUCEF_PLUGINGLUE_WAYLAND_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_H
#include "guidriverWAYLAND.h"
#define GUCEF_GUIDRIVERWAYLAND_H
#endif /* GUCEF_GUIDRIVERWAYLAND_H ? */

#include "TestWaylandCommon.h"
#include "TestWaylandActivation.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::WAYLAND;
using namespace GUCEF::GUIDRIVERWAYLAND;
using namespace GUCEF::TESTWAYLAND;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A fake compositor, a connection to it and a surface of the application on that connection */
struct SActivationRig
{
    CFakeWayland fake;
    CWaylandConnection connection;
    struct wl_surface* surface;
    UInt32 surfaceId;

    explicit SActivationRig( bool offerActivation = true )
        : fake(), connection( fake.GetApi() ), surface( GUCEF_NULL ), surfaceId( 0 )
    {
        fake.OfferDefaultGlobals();
        if ( offerActivation )
            fake.OfferGlobal( "xdg_activation_v1", 1 );
        connection.SetPumpOnPulse( false );
    }

    ~SActivationRig()
    {
        if ( GUCEF_NULL != surface && connection.IsConnected() )
            gucef_wl_surface_destroy( &connection.GetApi(), surface );
        connection.Disconnect();
    }

    bool Connect( void )
    {
        UseNoCursorTheme( connection );
        if ( !connection.Connect() )
            return false;
        surface = gucef_wl_compositor_create_surface( &connection.GetApi(), connection.GetCompositor() );
        surfaceId = connection.RegisterSurface( surface );
        return GUCEF_NULL != surface && 0 != surfaceId;
    }

    /** A click: the pointer enters, the button goes down and up, each with a serial of its own */
    void Click( UInt32 enterSerial, UInt32 downSerial, UInt32 upSerial )
    {
        SFakeProxy* proxy = (SFakeProxy*) surface;
        fake.SendPointerEnter( enterSerial, proxy, 10.0, 10.0 );
        fake.SendPointerButton( downSerial, 1000, 0x110, 1 );
        fake.SendPointerButton( upSerial, 1010, 0x110, 0 );
    }
};

Int32
IndexOfPrefix( const CFakeWayland& fake, const char* prefix )
{
    const size_t length = strlen( prefix );
    for ( UInt32 i=0; i<fake.requests.size(); ++i )
        if ( 0 == strncmp( fake.requests[ i ].C_String(), prefix, length ) )
            return (Int32) i;
    return -1;
}

UInt64
MonotonicTimeInMs( void )
{
    struct timespec now;
    ::clock_gettime( CLOCK_MONOTONIC, &now );
    return (UInt64) now.tv_sec * 1000 + (UInt64) now.tv_nsec / 1000000;
}

/** Keeps every message of every type */
class CLogCaptureAll : public CORE::CILogger
{
    public:

    CLogCaptureAll( void ) : CORE::CILogger(), m_text() {}

    CORE::CString m_text;

    virtual void Log( const CORE::TLogMsgType, const Int32, const CORE::CString& logMessage, const UInt32, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { m_text += logMessage; m_text += '\n'; }
    virtual void Log( const CORE::TLogMsgType type, const Int32 level, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE { Log( type, level, logMessage.ToString(), threadId, timestamp ); }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType type, const Int32 level, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE { Log( type, level, logMessage, threadId, timestamp ); }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType type, const Int32 level, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE { Log( type, level, logMessage.ToString(), threadId, timestamp ); }
    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void SetMinimalLogLevel( const Int32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::LOGLEVEL_EVERYTHING; }
};

struct SThreadAsk
{
    CWaylandActivationTokenProvider* provider;
    bool result;
    CORE::CString token;
};

void*
AskFromThread( void* argument )
{
    SThreadAsk* ask = static_cast< SThreadAsk* >( argument );
    CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_XDG_ACTIVATION;
    ask->result = ask->provider->GetActivationToken( ask->token, kind, 200 );
    return GUCEF_NULL;
}

/** Asks for a token in the handler of a button, which is what a click on a login button does */
class CClickAsker : public IWaylandSeatListener
{
    public:

    CWaylandActivationTokenProvider* provider;
    GUCEF::vector< CORE::CString > events;
    CORE::CString token;
    bool gotToken;

    CClickAsker( void ) : provider( GUCEF_NULL ), events(), token(), gotToken( false ) {}

    virtual void OnPointerButton( UInt32, UInt32, UInt32, UInt32 state )
    {
        if ( 0 == state )
            return;
        events.push_back( "button-begin" );
        CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_STARTUP_ID;
        gotToken = provider->GetActivationToken( token, kind, 200 ) && CORE::ACTIVATIONTOKEN_XDG_ACTIVATION == kind;
        events.push_back( "button-end" );
    }

    virtual void OnPointerMotion( UInt32, wl_fixed_t, wl_fixed_t ) { events.push_back( "motion" ); }
};

struct SClickScript
{
    CFakeWayland* fake;
    UInt32 step;
};

void
ThunkClickDown( void* argument )
{
    CFakeWayland* fake = static_cast< CFakeWayland* >( argument );
    fake->SendPointerButton( 7001, 2000, 0x110, 1 );
}

void
ThunkMotionAfter( void* argument )
{
    CFakeWayland* fake = static_cast< CFakeWayland* >( argument );
    fake->SendPointerMotion( 2001, 20.0, 30.0 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformWaylandActivationTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Wayland activation TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "WaylandActivation" );

    GUCEF_TESTFW_TESTCASE( "Test: xdg_activation_v1 is bound when the compositor offers it; without it the provider has no token and sends no request" )
        try
        {
            {
                SActivationRig rig( false );
                ASSERT_TRUE( rig.Connect() );
                ASSERT_TRUE( GUCEF_NULL == rig.connection.GetActivationManager() );
                CWaylandActivationTokenProvider provider( rig.connection, rig.surfaceId, "redux" );
                ASSERT_FALSE( provider.IsSupported() );
                rig.fake.ClearRequests();
                CORE::CString token;
                CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_STARTUP_ID;
                ASSERT_FALSE( provider.GetActivationToken( token, kind, 200 ) );
                ASSERT_TRUE( token.IsNULLOrEmpty() && CORE::ACTIVATIONTOKEN_XDG_ACTIVATION == kind );
                ASSERT_TRUE( 0 == rig.fake.requests.size() && 0 == rig.fake.queuesCreated );
            }
            {
                SActivationRig rig( true );
                ASSERT_TRUE( rig.Connect() );
                ASSERT_TRUE( GUCEF_NULL != rig.connection.GetActivationManager() );
                ASSERT_TRUE( rig.fake.Requested( "wl_registry.bind(6, \"xdg_activation_v1\", 1)" ) );
                CWaylandActivationTokenProvider provider( rig.connection, rig.surfaceId, "redux" );
                ASSERT_TRUE( provider.IsSupported() );
                rig.connection.Disconnect();
                ASSERT_FALSE( provider.IsSupported() );
                ASSERT_TRUE( rig.fake.Requested( "xdg_activation_v1.destroy()" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The serial for a token is the one of the last pointer button or key event: an enter or a motion after it does not replace it" )
        try
        {
            SActivationRig rig;
            ASSERT_TRUE( rig.Connect() );
            ASSERT_TRUE( 0 == rig.connection.GetLastActivationSerial() );

            rig.fake.SendPointerEnter( 100, (SFakeProxy*) rig.surface, 10.0, 10.0 );
            ASSERT_TRUE( 100 == rig.connection.GetLastInputSerial() && 0 == rig.connection.GetLastActivationSerial() );
            rig.fake.SendPointerButton( 4711, 1000, 0x110, 1 );
            ASSERT_TRUE( 4711 == rig.connection.GetLastActivationSerial() );
            rig.fake.SendPointerMotion( 1001, 11.0, 12.0 );
            rig.fake.SendPointerEnter( 4800, (SFakeProxy*) rig.surface, 11.0, 12.0 );
            ASSERT_TRUE( 4800 == rig.connection.GetLastInputSerial() && 4711 == rig.connection.GetLastActivationSerial() );

            rig.fake.SendKeyboardKey( 4900, 1010, 30, 1 );
            ASSERT_TRUE( 4900 == rig.connection.GetLastActivationSerial() );
            rig.fake.SendPointerButton( 4950, 1020, 0x110, 0 );
            ASSERT_TRUE( 4950 == rig.connection.GetLastActivationSerial() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A token is requested with the serial and seat of the last click, the application id and the window, committed, and the object is destroyed after the answer" )
        try
        {
            SActivationRig rig;
            ASSERT_TRUE( rig.Connect() );
            rig.Click( 100, 4711, 4712 );
            CWaylandActivationTokenProvider provider( rig.connection, rig.surfaceId, "redux" );
            rig.fake.ClearRequests();

            CORE::CString token;
            CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_STARTUP_ID;
            ASSERT_TRUE( provider.GetActivationToken( token, kind, CORE::ActivationTokenMaxWaitInMs ) );
            ASSERT_TRUE( "fake-activation-token-0001" == token && CORE::ACTIVATIONTOKEN_XDG_ACTIVATION == kind );

            // the order of the sequence of the protocol: the object, what it is made from, the commit, the end of it
            const Int32 get = IndexOfPrefix( rig.fake, "xdg_activation_v1.get_activation_token(" );
            const Int32 serial = IndexOfPrefix( rig.fake, "xdg_activation_token_v1.set_serial(4712, <wl_seat#" );
            const Int32 appId = IndexOfPrefix( rig.fake, "xdg_activation_token_v1.set_app_id(\"redux\")" );
            const Int32 surface = IndexOfPrefix( rig.fake, "xdg_activation_token_v1.set_surface(<wl_surface#" );
            const Int32 commit = IndexOfPrefix( rig.fake, "xdg_activation_token_v1.commit(" );
            const Int32 destroy = IndexOfPrefix( rig.fake, "xdg_activation_token_v1.destroy(" );
            ASSERT_TRUE( get >= 0 && serial > get && appId > serial && surface > appId && commit > surface && destroy > commit );
            ASSERT_TRUE( 1 == rig.fake.CountRequestsWithPrefix( "xdg_activation_v1.get_activation_token(" ) );
            ASSERT_TRUE( 1 == rig.fake.queuesCreated && 1 == rig.fake.queuesDestroyed );

            // the surface in the request is the one of the window
            SFakeProxy* windowSurface = (SFakeProxy*) rig.surface;
            ASSERT_TRUE( rig.fake.requests[ surface ].HasSubstr( "#" + CORE::UInt32ToString( windowSurface->id ) + ">" ) >= 0 );

            // an empty application id is left out; each request makes a new token object on a queue of its own
            CWaylandActivationTokenProvider anonymous( rig.connection, rig.surfaceId, CORE::CString::Empty );
            rig.fake.ClearRequests();
            ASSERT_TRUE( anonymous.GetActivationToken( token, kind, 200 ) );
            ASSERT_TRUE( 0 == rig.fake.CountRequestsWithPrefix( "xdg_activation_token_v1.set_app_id(" ) );
            ASSERT_TRUE( 1 == rig.fake.CountRequestsWithPrefix( "xdg_activation_token_v1.commit(" ) && 2 == rig.fake.queuesCreated && 2 == rig.fake.queuesDestroyed );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: With no click yet the enter serial is used and the seat serial is left out when there is none at all" )
        try
        {
            SActivationRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandActivationTokenProvider provider( rig.connection, rig.surfaceId, "redux" );
            CORE::CString token;
            CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_STARTUP_ID;

            // no input at all: the token is still made, without a serial
            rig.fake.ClearRequests();
            ASSERT_TRUE( provider.GetActivationToken( token, kind, 200 ) );
            ASSERT_TRUE( 0 == rig.fake.CountRequestsWithPrefix( "xdg_activation_token_v1.set_serial(" ) );

            // only an enter
            rig.fake.SendPointerEnter( 321, (SFakeProxy*) rig.surface, 5.0, 5.0 );
            rig.fake.ClearRequests();
            ASSERT_TRUE( provider.GetActivationToken( token, kind, 200 ) );
            ASSERT_TRUE( rig.fake.requests.size() > 0 && IndexOfPrefix( rig.fake, "xdg_activation_token_v1.set_serial(321, " ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An answer that arrives on the socket is read and dispatched on the queue of the request" )
        try
        {
            SActivationRig rig;
            rig.fake.activationFromSocket = true;
            rig.fake.activationToken = "token-over-the-socket";
            ASSERT_TRUE( rig.Connect() );
            rig.Click( 100, 4711, 4712 );
            CWaylandActivationTokenProvider provider( rig.connection, rig.surfaceId, "redux" );
            CORE::CString token;
            CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_STARTUP_ID;
            const UInt32 readsBefore = rig.fake.readEventsCalls;
            ASSERT_TRUE( provider.GetActivationToken( token, kind, 200 ) );
            ASSERT_TRUE( "token-over-the-socket" == token );
            ASSERT_TRUE( rig.fake.readEventsCalls > readsBefore );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A compositor that does not answer costs the time given and no more, nothing is asked with no time, and the connection keeps working" )
        try
        {
            SActivationRig rig;
            rig.fake.activationAnswers = false;
            ASSERT_TRUE( rig.Connect() );
            rig.Click( 100, 4711, 4712 );
            CWaylandActivationTokenProvider provider( rig.connection, rig.surfaceId, "redux" );
            CORE::CString token = "stale";
            CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_STARTUP_ID;

            const UInt64 before = MonotonicTimeInMs();
            ASSERT_FALSE( provider.GetActivationToken( token, kind, 50 ) );
            const UInt64 waited = MonotonicTimeInMs() - before;
            ASSERT_TRUE( waited >= 45 && waited < 1000 );
            ASSERT_TRUE( token.IsNULLOrEmpty() && CORE::ACTIVATIONTOKEN_XDG_ACTIVATION == kind );
            // the object and the queue are gone again: a late answer finds no proxy
            ASSERT_TRUE( 1 == rig.fake.CountRequestsWithPrefix( "xdg_activation_token_v1.destroy(" ) && 1 == rig.fake.queuesCreated && 1 == rig.fake.queuesDestroyed );

            // no time to wait, no request
            rig.fake.ClearRequests();
            ASSERT_FALSE( provider.GetActivationToken( token, kind, 0 ) );
            ASSERT_TRUE( 0 == rig.fake.requests.size() );

            // and the compositor that answers later does not make the next token wrong
            rig.fake.activationAnswers = true;
            rig.fake.activationToken = "second-token";
            ASSERT_TRUE( provider.GetActivationToken( token, kind, 200 ) );
            ASSERT_TRUE( "second-token" == token );
            ASSERT_TRUE( rig.connection.IsHealthy() && rig.connection.Pump() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An empty token, another thread, a window that is gone and a library without event queues all give no token" )
        try
        {
            {
                SActivationRig rig;
                rig.fake.activationToken.Clear();
                ASSERT_TRUE( rig.Connect() );
                CWaylandActivationTokenProvider provider( rig.connection, rig.surfaceId, "redux" );
                CORE::CString token;
                CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_XDG_ACTIVATION;
                ASSERT_FALSE( provider.GetActivationToken( token, kind, 200 ) );
                ASSERT_TRUE( token.IsNULLOrEmpty() );
            }
            {
                // the thread that made the provider is the only one that may ask
                SActivationRig rig;
                ASSERT_TRUE( rig.Connect() );
                CWaylandActivationTokenProvider provider( rig.connection, rig.surfaceId, "redux" );
                rig.fake.ClearRequests();
                SThreadAsk ask;
                ask.provider = &provider;
                ask.result = true;
                pthread_t thread;
                ASSERT_TRUE( 0 == ::pthread_create( &thread, GUCEF_NULL, AskFromThread, &ask ) );
                ::pthread_join( thread, GUCEF_NULL );
                ASSERT_FALSE( ask.result );
                ASSERT_TRUE( ask.token.IsNULLOrEmpty() && 0 == rig.fake.requests.size() );
            }
            {
                // a window that is not registered (any more) has no token
                SActivationRig rig;
                ASSERT_TRUE( rig.Connect() );
                CWaylandActivationTokenProvider provider( rig.connection, rig.surfaceId, "redux" );
                rig.connection.UnregisterSurface( rig.surfaceId );
                rig.fake.ClearRequests();
                CORE::CString token;
                CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_XDG_ACTIVATION;
                ASSERT_FALSE( provider.GetActivationToken( token, kind, 200 ) );
                ASSERT_TRUE( 0 == rig.fake.requests.size() );
                rig.surfaceId = 0;
            }
            {
                SActivationRig rig;
                rig.fake.RemoveQueueFunctions();
                ASSERT_TRUE( rig.Connect() );
                CWaylandActivationTokenProvider provider( rig.connection, rig.surfaceId, "redux" );
                ASSERT_FALSE( provider.IsSupported() );
                rig.fake.ClearRequests();
                CORE::CString token;
                CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_XDG_ACTIVATION;
                ASSERT_FALSE( provider.GetActivationToken( token, kind, 200 ) );
                ASSERT_TRUE( 0 == rig.fake.requests.size() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Asked from inside the handler of a click the answer comes without dispatching the events that follow; those are dispatched when the handler is done" )
        try
        {
            SActivationRig rig;
            ASSERT_TRUE( rig.Connect() );
            CWaylandActivationTokenProvider provider( rig.connection, rig.surfaceId, "redux" );
            CClickAsker asker;
            asker.provider = &provider;
            rig.connection.AddSeatListener( &asker );

            // the button and the motion behind it arrive in one dispatch of the connection, as they do when the user clicks
            rig.fake.Post( ThunkClickDown, &rig.fake );
            rig.fake.Post( ThunkMotionAfter, &rig.fake );
            ASSERT_TRUE( rig.connection.Pump() );
            rig.connection.RemoveSeatListener( &asker );

            ASSERT_TRUE( asker.gotToken && "fake-activation-token-0001" == asker.token );
            ASSERT_TRUE( 3 == asker.events.size() && "button-begin" == asker.events[ 0 ] && "button-end" == asker.events[ 1 ] && "motion" == asker.events[ 2 ] );
            ASSERT_TRUE( 1 == rig.fake.CountRequestsWithPrefix( "xdg_activation_token_v1.set_serial(7001, " ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The window driver installs the provider for the launcher while its window exists, takes it away with the window and leaves the one of somebody else alone" )
        try
        {
            ASSERT_TRUE( GUCEF_NULL == CORE::GetActivationTokenProvider() );
            CFakeWayland fake;
            CFakeGraphics graphics;
            CWaylandConnection connection( fake.GetApi() );
            fake.OfferDefaultGlobals();
            fake.OfferGlobal( "xdg_activation_v1", 1 );
            connection.SetPumpOnPulse( false );
            UseNoCursorTheme( connection );
            ASSERT_TRUE( connection.Connect() );

            GUI::CVideoSettings settings;
            settings.SetResolution( 1280, 720, 32 );
            settings.SetVSyncState( true );
            settings.SetFullscreenState( false );
            settings.SetFrequency( 60 );
            SWaylandWindowParams params;
            params.appId = "redux";

            {
                CWaylandWindowContext window( connection, graphics.GetRuntime() );
                ASSERT_TRUE( window.Initialize( "Redux", settings, params ) );
                CORE::CIActivationTokenProvider* installed = CORE::GetActivationTokenProvider();
                ASSERT_TRUE( GUCEF_NULL != installed );

                // the launcher asks it, and what it asks is what the compositor is told
                fake.SendPointerEnter( 100, (SFakeProxy*) connection.FindSurface( window.GetSurfaceId() )->surface, 5.0, 5.0 );
                fake.SendPointerButton( 8123, 1000, 0x110, 1 );
                fake.ClearRequests();
                CORE::CString token;
                CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_STARTUP_ID;
                ASSERT_TRUE( installed->GetActivationToken( token, kind, 200 ) );
                ASSERT_TRUE( "fake-activation-token-0001" == token && CORE::ACTIVATIONTOKEN_XDG_ACTIVATION == kind );
                ASSERT_TRUE( IndexOfPrefix( fake, "xdg_activation_token_v1.set_serial(8123, " ) >= 0 && IndexOfPrefix( fake, "xdg_activation_token_v1.set_app_id(\"redux\")" ) >= 0 );

                // a second window does not take over
                CWaylandWindowContext other( connection, graphics.GetRuntime() );
                ASSERT_TRUE( other.Initialize( "Other", settings, params ) );
                ASSERT_TRUE( installed == CORE::GetActivationTokenProvider() );
                other.Shutdown();
                ASSERT_TRUE( installed == CORE::GetActivationTokenProvider() );
            }
            ASSERT_TRUE( GUCEF_NULL == CORE::GetActivationTokenProvider() );

            // a provider of somebody else stays
            CORE::CIActivationTokenProvider* foreign = GUCEF_NULL;
            class CForeign : public CORE::CIActivationTokenProvider
            {
                public:
                virtual bool GetActivationToken( CORE::CString&, CORE::TActivationTokenKind&, UInt32 ) GUCEF_VIRTUAL_OVERRIDE { return false; }
            } foreignProvider;
            foreign = &foreignProvider;
            CORE::SetActivationTokenProvider( foreign );
            {
                CWaylandWindowContext window( connection, graphics.GetRuntime() );
                ASSERT_TRUE( window.Initialize( "Redux", settings, params ) );
                ASSERT_TRUE( foreign == CORE::GetActivationTokenProvider() );
            }
            ASSERT_TRUE( foreign == CORE::GetActivationTokenProvider() );
            CORE::SetActivationTokenProvider( GUCEF_NULL );
            connection.Disconnect();
        }
        catch ( ... )
        {
            CORE::SetActivationTokenProvider( GUCEF_NULL );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The token is not written to the log, the reason there is none is" )
        try
        {
            CORE::CLogStreamScope::FlushLogs();
            CLogCaptureAll capture;
            CORE::CLogManager& logManager = CORE::CCoreGlobal::Instance()->GetLogManager();
            logManager.AddLogger( &capture );
            {
                SActivationRig rig;
                rig.fake.activationToken = "SECRET-WAYLAND-TOKEN-9d2e";
                ASSERT_TRUE( rig.Connect() );
                rig.Click( 100, 4711, 4712 );
                CWaylandActivationTokenProvider provider( rig.connection, rig.surfaceId, "redux" );
                CORE::CString token;
                CORE::TActivationTokenKind kind = CORE::ACTIVATIONTOKEN_STARTUP_ID;
                ASSERT_TRUE( provider.GetActivationToken( token, kind, 200 ) );
                ASSERT_TRUE( "SECRET-WAYLAND-TOKEN-9d2e" == token );
                rig.fake.activationAnswers = false;
                ASSERT_FALSE( provider.GetActivationToken( token, kind, 20 ) );
            }
            CORE::CLogStreamScope::FlushLogs();
            logManager.RemoveLogger( &capture );
            ASSERT_TRUE( capture.m_text.HasSubstr( "SECRET-WAYLAND-TOKEN-9d2e" ) < 0 );
            ASSERT_TRUE( capture.m_text.HasSubstr( "No activation token" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Wayland activation TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
