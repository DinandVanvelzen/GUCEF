/*
 *  guidriverOgre: gucefGUI window backend that gives Ogre render windows on the platform windows
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

#include <OgreStringConverter.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "guidriverOgre_COgrePlatformWindowSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEROGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString COgrePlatformWindowSupport::XWinGLBackend = "XWinGL";
const CORE::CString COgrePlatformWindowSupport::WaylandBackend = "Wayland";
const CORE::CString COgrePlatformWindowSupport::Win32Backend = "Win32";
const CORE::CString COgrePlatformWindowSupport::AndroidGLESBackend = "AndroidGLES";
const CORE::CString COgrePlatformWindowSupport::EmscriptenBackend = "Emscripten";
const CORE::CString COgrePlatformWindowSupport::OffscreenBackend = "Offscreen";
const CORE::CString COgrePlatformWindowSupport::PlatformBackendParameterName = "PlatformBackend";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
COgrePlatformWindowSupport::GetDefaultPlatformBackendForPlatform( UInt32 gucefPlatform )
{GUCEF_TRACE;

    switch ( gucefPlatform )
    {
        case GUCEF_PLATFORM_LINUX: return XWinGLBackend;
        case GUCEF_PLATFORM_MSWIN: return Win32Backend;
        case GUCEF_PLATFORM_ANDROID: return AndroidGLESBackend;
        case GUCEF_PLATFORM_WASM_EMSCRIPTEN: return EmscriptenBackend;
        default: return OffscreenBackend;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
COgrePlatformWindowSupport::GetDefaultPlatformBackend( void )
{GUCEF_TRACE;

    return GetDefaultPlatformBackendForPlatform( GUCEF_PLATFORM );
}

/*-------------------------------------------------------------------------*/

CORE::CString
COgrePlatformWindowSupport::SelectPlatformBackend( const CORE::CValueList* params      ,
                                                   const CORE::CString& defaultBackend )
{GUCEF_TRACE;

    if ( GUCEF_NULL == params || !params->HasKey( PlatformBackendParameterName ) )
        return defaultBackend;
    const CORE::CString asked = params->GetValueAlways( PlatformBackendParameterName ).AsString( CORE::CString::Empty, true ).Trim( true ).Trim( false );
    if ( asked.IsNULLOrEmpty() )
        return defaultBackend;

    const CORE::CString* known[] = { &XWinGLBackend, &WaylandBackend, &Win32Backend, &AndroidGLESBackend, &EmscriptenBackend, &OffscreenBackend };
    for ( UInt32 i=0; i<sizeof( known ) / sizeof( known[ 0 ] ); ++i )
        if ( asked.Equals( *known[ i ], false ) )
            return *known[ i ];
    return asked;
}

/*-------------------------------------------------------------------------*/

bool
COgrePlatformWindowSupport::IsKnownPlatformBackend( const CORE::CString& backendName )
{GUCEF_TRACE;

    return XWinGLBackend == backendName || WaylandBackend == backendName || Win32Backend == backendName ||
           AndroidGLESBackend == backendName || EmscriptenBackend == backendName || OffscreenBackend == backendName;
}

/*-------------------------------------------------------------------------*/

bool
COgrePlatformWindowSupport::DoesPlatformPaceFrames( const CORE::CString& backendName )
{GUCEF_TRACE;

    return OffscreenBackend != backendName;
}

/*-------------------------------------------------------------------------*/

COgrePlatformWindowSupport::EResizeMethod
COgrePlatformWindowSupport::GetResizeMethod( const CORE::CString& backendName )
{GUCEF_TRACE;

    // Ogre owns the X11 child window, the wl_egl_window and the offscreen surface; it was given the HWND, the
    // ANativeWindow and the canvas, whose size the platform window has already set
    if ( Win32Backend == backendName || AndroidGLESBackend == backendName || EmscriptenBackend == backendName )
        return RESIZEMETHOD_FROM_NATIVE;
    return RESIZEMETHOD_RESIZE;
}

/*-------------------------------------------------------------------------*/

bool
COgrePlatformWindowSupport::CanLoseSurface( const CORE::CString& backendName )
{GUCEF_TRACE;

    return AndroidGLESBackend == backendName;
}

/*-------------------------------------------------------------------------*/

UInt32
COgrePlatformWindowSupport::GetPulseIntervalInMs( UInt32 frequency )
{GUCEF_TRACE;

    if ( 0 == frequency )
        return 0;
    return frequency < 1000 ? 1000 / frequency : 1;
}

/*-------------------------------------------------------------------------*/

Ogre::String
COgrePlatformWindowSupport::PointerPropertyToHandle( const CORE::CString& pointerProperty )
{GUCEF_TRACE;

    void* pointer = CORE::StringToPointer( pointerProperty );
    return Ogre::StringConverter::toString( (size_t) pointer );
}

/*-------------------------------------------------------------------------*/

void
COgrePlatformWindowSupport::AddVideoSettingParams( const CORE::CString& backendName          ,
                                                   const GUI::CVideoSettings& videoSettings ,
                                                   const CORE::CString& title               ,
                                                   Ogre::NameValuePairList& miscParams      )
{GUCEF_TRACE;

    miscParams[ "title" ] = title.C_String();

    // on Wayland the frame callbacks of the platform window pace the frames: a swap that waits for the monitor as well
    // would wait twice
    const bool vsync = videoSettings.GetVSyncState() && WaylandBackend != backendName;
    miscParams[ "vsync" ] = vsync ? "true" : "false";
    if ( videoSettings.GetAntiAliasingFactor() > 1 )
        miscParams[ "FSAA" ] = Ogre::StringConverter::toString( videoSettings.GetAntiAliasingFactor() );
    if ( videoSettings.GetFrequency() > 0 )
        miscParams[ "displayFrequency" ] = Ogre::StringConverter::toString( videoSettings.GetFrequency() );
}

/*-------------------------------------------------------------------------*/

bool
COgrePlatformWindowSupport::AddNativeWindowParams( const CORE::CString& backendName          ,
                                                   const GUI::CWindowContext& platformWindow ,
                                                   Ogre::NameValuePairList& miscParams       ,
                                                   CORE::CString& problem                    )
{GUCEF_TRACE;

    if ( XWinGLBackend == backendName )
    {
        const UInt32 window = CORE::StringToUInt32( platformWindow.GetProperty( "WINDOW" ), 0 );
        if ( 0 == window )
        {
            problem = "the XWinGL window has no X11 window";
            return false;
        }
        miscParams[ "externalWindowHandle" ] = Ogre::StringConverter::toString( window );
        return true;
    }
    if ( WaylandBackend == backendName )
    {
        const CORE::CString display = platformWindow.GetProperty( "DISPLAY" );
        const CORE::CString surface = platformWindow.GetProperty( "WL_SURFACE" );
        if ( GUCEF_NULL == CORE::StringToPointer( display ) || GUCEF_NULL == CORE::StringToPointer( surface ) )
        {
            problem = "the Wayland window has no wl_display or no wl_surface";
            return false;
        }
        miscParams[ "externalWlDisplay" ] = PointerPropertyToHandle( display );
        miscParams[ "externalWlSurface" ] = PointerPropertyToHandle( surface );
        return true;
    }
    if ( Win32Backend == backendName )
    {
        const CORE::CString window = platformWindow.GetProperty( "WINDOW" );
        if ( GUCEF_NULL == CORE::StringToPointer( window ) )
        {
            problem = "the Win32 window has no HWND";
            return false;
        }
        miscParams[ "externalWindowHandle" ] = PointerPropertyToHandle( window );
        return true;
    }
    if ( AndroidGLESBackend == backendName )
    {
        const CORE::CString window = platformWindow.GetProperty( "WINDOW" );
        if ( GUCEF_NULL == CORE::StringToPointer( window ) )
        {
            problem = "the Android window has no native window (the app has no window while it is in the background)";
            return false;
        }
        miscParams[ "externalWindowHandle" ] = PointerPropertyToHandle( window );
        const CORE::CString config = platformWindow.GetProperty( "ANDROID_CONFIG" );
        if ( GUCEF_NULL != CORE::StringToPointer( config ) )
            miscParams[ "androidConfig" ] = PointerPropertyToHandle( config );

        // the EGL context outlives the native window when the app is paused, the resources in it stay
        miscParams[ "preserveContext" ] = "true";
        return true;
    }
    if ( EmscriptenBackend == backendName )
    {
        const CORE::CString canvas = platformWindow.GetProperty( "WINDOW" );
        if ( canvas.IsNULLOrEmpty() )
        {
            problem = "the Emscripten window has no canvas";
            return false;
        }
        miscParams[ "externalWindowHandle" ] = canvas.C_String();
        return true;
    }
    if ( OffscreenBackend == backendName )
        return true;

    problem = "Ogre render windows on windows of the platform backend \"" + backendName + "\" are not supported";
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
