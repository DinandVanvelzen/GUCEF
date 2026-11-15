/*
 *  guidriverOgre_TestApp: Test application for the window backend Ogre (guidriverOgre)
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

#include <string.h>

#include <Ogre.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUIDRIVEROGRE_COGREPLATFORMWINDOWSUPPORT_H
#include "guidriverOgre_COgrePlatformWindowSupport.h"
#define GUIDRIVEROGRE_COGREPLATFORMWINDOWSUPPORT_H
#endif /* GUIDRIVEROGRE_COGREPLATFORMWINDOWSUPPORT_H ? */

#ifndef GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#include "guidriverOgre_COgreWindowContext.h"
#define GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#endif /* GUIDRIVEROGRE_COGREWINDOWCONTEXT_H ? */

#ifndef GUIDRIVEROGRE_COGRESURFACESTATE_H
#include "guidriverOgre_COgreSurfaceState.h"
#define GUIDRIVEROGRE_COGRESURFACESTATE_H
#endif /* GUIDRIVEROGRE_COGRESURFACESTATE_H ? */

#include "TestOgrePlatform.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::GUIDRIVEROGRE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A platform window that is only its properties: what a window of a platform driver offers, on any build platform */
class CTestPlatformWindow : public GUI::CWindowContext
{
    public:

    CTestPlatformWindow( void ) : GUI::CWindowContext(), m_properties(), m_guiContext() {}

    void Set( const char* name, const CORE::CString& value ) { m_properties[ name ] = value; }

    virtual GUI::UInt32 GetWidth( void ) const GUCEF_VIRTUAL_OVERRIDE { return 640; }
    virtual GUI::UInt32 GetHeight( void ) const GUCEF_VIRTUAL_OVERRIDE { return 480; }
    virtual GUI::UInt32 GetID( void ) const GUCEF_VIRTUAL_OVERRIDE { return 1; }
    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual GUI::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "TestPlatformWindow"; }
    virtual void SetGuiContext( GUI::TGuiContextPtr& context ) GUCEF_VIRTUAL_OVERRIDE { m_guiContext = context; }
    virtual GUI::TGuiContextPtr GetGuiContext( void ) GUCEF_VIRTUAL_OVERRIDE { return m_guiContext; }

    virtual GUI::CString GetProperty( const GUI::CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CString::StringMap::const_iterator i = m_properties.find( propertyName );
        return i != m_properties.end() ? (*i).second : GUI::CString::Empty;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTestPlatformWindow";
        return classTypeName;
    }

    private:

    CORE::CString::StringMap m_properties;
    GUI::TGuiContextPtr m_guiContext;
};

/*-------------------------------------------------------------------------*/

bool
HasParam( const Ogre::NameValuePairList& params, const char* name, const Ogre::String& value )
{
    Ogre::NameValuePairList::const_iterator i = params.find( name );
    return i != params.end() && (*i).second == value;
}

/*-------------------------------------------------------------------------*/

bool
HasParam( const Ogre::NameValuePairList& params, const char* name )
{
    return params.end() != params.find( name );
}

/*-------------------------------------------------------------------------*/

GUI::CVideoSettings
MakeSettings( bool vsync, UInt32 antiAliasing, UInt32 frequency )
{
    GUI::CVideoSettings settings;
    settings.SetResolution( 640, 480, 32 );
    settings.SetVSyncState( vsync );
    settings.SetAntiAliasingFactor( antiAliasing );
    settings.SetFrequency( frequency );
    return settings;
}

/*-------------------------------------------------------------------------*/

/** Window parameters with only PlatformBackend; CValueList::Set() adds a value to a key that has one, so every case has its own list */
CORE::CValueList
MakeBackendParams( const char* backend )
{
    CORE::CValueList params;
    params.Set( COgrePlatformWindowSupport::PlatformBackendParameterName, backend );
    return params;
}

/*-------------------------------------------------------------------------*/

/** A pointer as the properties of the window drivers give it (CORE::PointerToString), and as Ogre takes it (decimal) */
Ogre::String
AsOgreHandle( const void* pointer )
{
    return Ogre::StringConverter::toString( (size_t) pointer );
}

}

/*-------------------------------------------------------------------------*/

void
PerformOgrePlatformTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "OgrePlatform" );

    GUCEF_TESTFW_TESTCASE( "Platform backend: the build platform chooses it, the window parameter PlatformBackend overrides it in any letter case" )
        try
        {
            ASSERT_TRUE( "XWinGL" == COgrePlatformWindowSupport::GetDefaultPlatformBackendForPlatform( GUCEF_PLATFORM_LINUX ) );
            ASSERT_TRUE( "Win32" == COgrePlatformWindowSupport::GetDefaultPlatformBackendForPlatform( GUCEF_PLATFORM_MSWIN ) );
            ASSERT_TRUE( "AndroidGLES" == COgrePlatformWindowSupport::GetDefaultPlatformBackendForPlatform( GUCEF_PLATFORM_ANDROID ) );
            ASSERT_TRUE( "Emscripten" == COgrePlatformWindowSupport::GetDefaultPlatformBackendForPlatform( GUCEF_PLATFORM_WASM_EMSCRIPTEN ) );
            ASSERT_TRUE( "Offscreen" == COgrePlatformWindowSupport::GetDefaultPlatformBackendForPlatform( GUCEF_PLATFORM_APPLE_MACOS ) );
            ASSERT_TRUE( COgrePlatformWindowSupport::GetDefaultPlatformBackendForPlatform( GUCEF_PLATFORM ) == COgrePlatformWindowSupport::GetDefaultPlatformBackend() );
            ASSERT_TRUE( COgreWindowContext::PlatformBackendParameterName == COgrePlatformWindowSupport::PlatformBackendParameterName );
            ASSERT_TRUE( COgreWindowContext::OffscreenPlatformBackend == COgrePlatformWindowSupport::OffscreenBackend );

            // no parameter, no value: the default
            ASSERT_TRUE( "Win32" == COgrePlatformWindowSupport::SelectPlatformBackend( GUCEF_NULL, "Win32" ) );
            CORE::CValueList params;
            ASSERT_TRUE( "XWinGL" == COgrePlatformWindowSupport::SelectPlatformBackend( &params, "XWinGL" ) );
            CORE::CValueList blank = MakeBackendParams( "  " );
            ASSERT_TRUE( "XWinGL" == COgrePlatformWindowSupport::SelectPlatformBackend( &blank, "XWinGL" ) );

            // the known names as the backends write them
            CORE::CValueList wayland = MakeBackendParams( "wayland" );
            ASSERT_TRUE( "Wayland" == COgrePlatformWindowSupport::SelectPlatformBackend( &wayland, "XWinGL" ) );
            CORE::CValueList offscreen = MakeBackendParams( "OFFSCREEN" );
            ASSERT_TRUE( "Offscreen" == COgrePlatformWindowSupport::SelectPlatformBackend( &offscreen, "XWinGL" ) );
            CORE::CValueList android = MakeBackendParams( "androidgles" );
            ASSERT_TRUE( "AndroidGLES" == COgrePlatformWindowSupport::SelectPlatformBackend( &android, "XWinGL" ) );

            // an unknown name stays as it is given and is not supported
            CORE::CValueList cocoa = MakeBackendParams( "Cocoa" );
            ASSERT_TRUE( "Cocoa" == COgrePlatformWindowSupport::SelectPlatformBackend( &cocoa, "XWinGL" ) );
            ASSERT_FALSE( COgrePlatformWindowSupport::IsKnownPlatformBackend( "Cocoa" ) );
            ASSERT_FALSE( COgrePlatformWindowSupport::IsKnownPlatformBackend( "wayland" ) );
            ASSERT_TRUE( COgrePlatformWindowSupport::IsKnownPlatformBackend( "Wayland" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Parameters: the video settings give title, vsync, FSAA and displayFrequency; on Wayland Ogre's swap does not wait" )
        try
        {
            Ogre::NameValuePairList params;
            COgrePlatformWindowSupport::AddVideoSettingParams( "XWinGL", MakeSettings( true, 4, 60 ), "Game", params );
            ASSERT_TRUE( HasParam( params, "title", "Game" ) );
            ASSERT_TRUE( HasParam( params, "vsync", "true" ) );
            ASSERT_TRUE( HasParam( params, "FSAA", "4" ) );
            ASSERT_TRUE( HasParam( params, "displayFrequency", "60" ) );

            Ogre::NameValuePairList plain;
            COgrePlatformWindowSupport::AddVideoSettingParams( "Win32", MakeSettings( false, 1, 0 ), "Game", plain );
            ASSERT_TRUE( HasParam( plain, "vsync", "false" ) );
            ASSERT_FALSE( HasParam( plain, "FSAA" ) );
            ASSERT_FALSE( HasParam( plain, "displayFrequency" ) );

            // the frame callbacks of the Wayland window are the vertical sync
            Ogre::NameValuePairList wayland;
            COgrePlatformWindowSupport::AddVideoSettingParams( "Wayland", MakeSettings( true, 0, 60 ), "Game", wayland );
            ASSERT_TRUE( HasParam( wayland, "vsync", "false" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Parameters: the native handles of every platform window, from its properties" )
        try
        {
            CORE::CString problem;
            int dummy[ 4 ] = { 0, 0, 0, 0 };

            // X11: the window as a number
            CTestPlatformWindow x11;
            x11.Set( "WINDOW", "4194310" );
            Ogre::NameValuePairList x11Params;
            ASSERT_TRUE( COgrePlatformWindowSupport::AddNativeWindowParams( "XWinGL", x11, x11Params, problem ) );
            ASSERT_TRUE( HasParam( x11Params, "externalWindowHandle", "4194310" ) );
            ASSERT_TRUE( 1 == x11Params.size() );

            // Wayland: the display and the surface, pointers as decimal numbers, and no externalWindowHandle (Ogre's Wayland window refuses it)
            CTestPlatformWindow wayland;
            wayland.Set( "DISPLAY", CORE::PointerToString( &dummy[ 0 ] ) );
            wayland.Set( "WL_SURFACE", CORE::PointerToString( &dummy[ 1 ] ) );
            wayland.Set( "WINDOW", "7" );
            Ogre::NameValuePairList waylandParams;
            ASSERT_TRUE( COgrePlatformWindowSupport::AddNativeWindowParams( "Wayland", wayland, waylandParams, problem ) );
            ASSERT_TRUE( HasParam( waylandParams, "externalWlDisplay", AsOgreHandle( &dummy[ 0 ] ) ) );
            ASSERT_TRUE( HasParam( waylandParams, "externalWlSurface", AsOgreHandle( &dummy[ 1 ] ) ) );
            ASSERT_FALSE( HasParam( waylandParams, "externalWindowHandle" ) );
            ASSERT_FALSE( HasParam( waylandParams, "parentWindowHandle" ) );

            // Win32: the HWND
            CTestPlatformWindow win32;
            win32.Set( "WINDOW", CORE::PointerToString( &dummy[ 2 ] ) );
            Ogre::NameValuePairList win32Params;
            ASSERT_TRUE( COgrePlatformWindowSupport::AddNativeWindowParams( "Win32", win32, win32Params, problem ) );
            ASSERT_TRUE( HasParam( win32Params, "externalWindowHandle", AsOgreHandle( &dummy[ 2 ] ) ) );

            // Android: the ANativeWindow, the AConfiguration, and the context that outlives the native window
            CTestPlatformWindow android;
            android.Set( "WINDOW", CORE::PointerToString( &dummy[ 3 ] ) );
            android.Set( "ANDROID_CONFIG", CORE::PointerToString( &dummy[ 0 ] ) );
            Ogre::NameValuePairList androidParams;
            ASSERT_TRUE( COgrePlatformWindowSupport::AddNativeWindowParams( "AndroidGLES", android, androidParams, problem ) );
            ASSERT_TRUE( HasParam( androidParams, "externalWindowHandle", AsOgreHandle( &dummy[ 3 ] ) ) );
            ASSERT_TRUE( HasParam( androidParams, "androidConfig", AsOgreHandle( &dummy[ 0 ] ) ) );
            ASSERT_TRUE( HasParam( androidParams, "preserveContext", "true" ) );

            // the web: the CSS selector of the canvas as it is
            CTestPlatformWindow web;
            web.Set( "WINDOW", "#canvas" );
            Ogre::NameValuePairList webParams;
            ASSERT_TRUE( COgrePlatformWindowSupport::AddNativeWindowParams( "Emscripten", web, webParams, problem ) );
            ASSERT_TRUE( HasParam( webParams, "externalWindowHandle", "#canvas" ) );

            // offscreen: nothing native
            CTestPlatformWindow none;
            Ogre::NameValuePairList offscreenParams;
            ASSERT_TRUE( COgrePlatformWindowSupport::AddNativeWindowParams( "Offscreen", none, offscreenParams, problem ) );
            ASSERT_TRUE( offscreenParams.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Parameters: a platform window without its native handle, or of an unknown backend, gives no parameters and a reason" )
        try
        {
            CTestPlatformWindow empty;
            const char* backends[] = { "XWinGL", "Wayland", "Win32", "AndroidGLES", "Emscripten", "Cocoa" };
            for ( UInt32 i=0; i<sizeof( backends ) / sizeof( backends[ 0 ] ); ++i )
            {
                CORE::CString problem;
                Ogre::NameValuePairList params;
                ASSERT_FALSE( COgrePlatformWindowSupport::AddNativeWindowParams( backends[ i ], empty, params, problem ) );
                ASSERT_FALSE( problem.IsNULLOrEmpty() );
            }

            // a Wayland window with a surface but no display
            int dummy = 0;
            CTestPlatformWindow halfWayland;
            halfWayland.Set( "WL_SURFACE", CORE::PointerToString( &dummy ) );
            CORE::CString problem;
            Ogre::NameValuePairList params;
            ASSERT_FALSE( COgrePlatformWindowSupport::AddNativeWindowParams( "Wayland", halfWayland, params, problem ) );

            // an Android app in the background has no native window (WINDOW is empty)
            CTestPlatformWindow paused;
            paused.Set( "ANDROID_CONFIG", CORE::PointerToString( &dummy ) );
            ASSERT_FALSE( COgrePlatformWindowSupport::AddNativeWindowParams( "AndroidGLES", paused, params, problem ) );
            ASSERT_TRUE( "0" == COgrePlatformWindowSupport::PointerPropertyToHandle( "" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Resize and pacing: Ogre sizes what it owns and reads what it was given; the platform window paces, offscreen windows pace themselves" )
        try
        {
            ASSERT_TRUE( COgrePlatformWindowSupport::RESIZEMETHOD_RESIZE == COgrePlatformWindowSupport::GetResizeMethod( "XWinGL" ) );
            ASSERT_TRUE( COgrePlatformWindowSupport::RESIZEMETHOD_RESIZE == COgrePlatformWindowSupport::GetResizeMethod( "Wayland" ) );
            ASSERT_TRUE( COgrePlatformWindowSupport::RESIZEMETHOD_RESIZE == COgrePlatformWindowSupport::GetResizeMethod( "Offscreen" ) );
            ASSERT_TRUE( COgrePlatformWindowSupport::RESIZEMETHOD_FROM_NATIVE == COgrePlatformWindowSupport::GetResizeMethod( "Win32" ) );
            ASSERT_TRUE( COgrePlatformWindowSupport::RESIZEMETHOD_FROM_NATIVE == COgrePlatformWindowSupport::GetResizeMethod( "AndroidGLES" ) );
            ASSERT_TRUE( COgrePlatformWindowSupport::RESIZEMETHOD_FROM_NATIVE == COgrePlatformWindowSupport::GetResizeMethod( "Emscripten" ) );

            ASSERT_TRUE( COgrePlatformWindowSupport::DoesPlatformPaceFrames( "Wayland" ) );
            ASSERT_TRUE( COgrePlatformWindowSupport::DoesPlatformPaceFrames( "Emscripten" ) );
            ASSERT_FALSE( COgrePlatformWindowSupport::DoesPlatformPaceFrames( "Offscreen" ) );

            ASSERT_TRUE( 0 == COgrePlatformWindowSupport::GetPulseIntervalInMs( 0 ) );
            ASSERT_TRUE( 16 == COgrePlatformWindowSupport::GetPulseIntervalInMs( 60 ) );
            ASSERT_TRUE( 1 == COgrePlatformWindowSupport::GetPulseIntervalInMs( 1000 ) );
            ASSERT_TRUE( 1 == COgrePlatformWindowSupport::GetPulseIntervalInMs( 5000 ) );

            ASSERT_TRUE( COgrePlatformWindowSupport::CanLoseSurface( "AndroidGLES" ) );
            ASSERT_FALSE( COgrePlatformWindowSupport::CanLoseSurface( "Wayland" ) );
            ASSERT_FALSE( COgrePlatformWindowSupport::CanLoseSurface( "Win32" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Surface: lost and back again keeps the render window; nothing is drawn in between, and stray events change nothing" )
        try
        {
            COgreSurfaceState state;
            ASSERT_TRUE( COgreSurfaceState::SURFACESTATE_NO_WINDOW == state.GetState() );
            ASSERT_FALSE( state.CanDraw() );

            // a loss without a render window is not Ogre's business
            ASSERT_TRUE( COgreSurfaceState::SURFACEACTION_NONE == state.OnSurfaceLost() );
            ASSERT_TRUE( COgreSurfaceState::SURFACEACTION_NONE == state.OnSurfaceRestored( true ) );

            state.OnRenderWindowCreated();
            ASSERT_TRUE( state.CanDraw() );
            ASSERT_TRUE( COgreSurfaceState::SURFACEACTION_NONE == state.OnSurfaceRestored( true ) );    // nothing was lost

            // pause: Ogre lets go of the surface once
            ASSERT_TRUE( COgreSurfaceState::SURFACEACTION_RELEASE_SURFACE == state.OnSurfaceLost() );
            ASSERT_FALSE( state.CanDraw() );
            ASSERT_TRUE( COgreSurfaceState::SURFACESTATE_LOST == state.GetState() );
            ASSERT_TRUE( COgreSurfaceState::SURFACEACTION_NONE == state.OnSurfaceLost() );
            ASSERT_TRUE( 1 == state.GetLossCount() );

            // a restore that names no native window waits for one that does
            ASSERT_TRUE( COgreSurfaceState::SURFACEACTION_NONE == state.OnSurfaceRestored( false ) );
            ASSERT_FALSE( state.CanDraw() );

            // resume: Ogre gets the new native window
            ASSERT_TRUE( COgreSurfaceState::SURFACEACTION_ADOPT_SURFACE == state.OnSurfaceRestored( true ) );
            ASSERT_TRUE( state.CanDraw() );
            ASSERT_TRUE( 1 == state.GetRestoreCount() );

            // again, then the window goes while the surface is away
            ASSERT_TRUE( COgreSurfaceState::SURFACEACTION_RELEASE_SURFACE == state.OnSurfaceLost() );
            ASSERT_TRUE( 2 == state.GetLossCount() );
            state.OnRenderWindowDestroyed();
            ASSERT_TRUE( COgreSurfaceState::SURFACESTATE_NO_WINDOW == state.GetState() );
            ASSERT_TRUE( COgreSurfaceState::SURFACEACTION_NONE == state.OnSurfaceRestored( true ) );
            ASSERT_TRUE( 0 == strcmp( "NoWindow", COgreSurfaceState::ToString( state.GetState() ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
