/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/ 

#ifndef GUCEF_CORE_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_ESSENTIALS_H
#endif /* GUCEF_CORE_ESSENTIALS_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "gucefGUI_CVideoOptions.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
#include <windows.h>
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #ifndef GUCEF_CORE_DVCPPOSWRAP_H
  #include "DVCPPOSWRAP.h"
  #define GUCEF_CORE_DVCPPOSWRAP_H
  #endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
#include <emscripten.h>
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {
         
/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SDisplayModeSources
{
    MT::CMutex lock;
    GUCEF::map< CString, CVideoOptions::SDisplayModeSource > sources;
    CString selected;
};

/** Made once and kept: the plugins of window drivers unregister their source when they unload, which can be after the statics of this library are gone */
SDisplayModeSources&
GetDisplayModeSources( void )
{
    static SDisplayModeSources* sources = new SDisplayModeSources();
    return *sources;
}

/** The source that was selected, if there is one registered under that name */
bool
GetSelectedSource( CVideoOptions::SDisplayModeSource& source )
{
    SDisplayModeSources& sources = GetDisplayModeSources();
    MT::CScopeMutex lock( sources.lock );
    GUCEF::map< CString, CVideoOptions::SDisplayModeSource >::const_iterator i = sources.sources.find( sources.selected );
    if ( sources.selected.IsNULLOrEmpty() || i == sources.sources.end() )
        return false;
    source = (*i).second;
    return true;
}

CVideoOptions::TDisplayMode
MakeDisplayMode( UInt32 width, UInt32 height, UInt32 depth, UInt32 frequency )
{
    CVideoOptions::TDisplayMode mode;
    mode.widthInPixels = width;
    mode.heightInPixels = height;
    mode.depthInBits = depth;
    mode.frequency = frequency;
    return mode;
}

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

/*
 *  X11 and its RandR extension are loaded when asked for, so the library works on a system without them
 *  (a server, Wayland without XWayland). Only RandR 1.0 calls are used, they have the same form in every version.
 */
struct SXRRScreenSize
{
    int width;
    int height;
    int mwidth;
    int mheight;
};

class CX11Display
{
    public:

    typedef void* ( *TXOpenDisplay )( const char* );
    typedef int ( *TXCloseDisplay )( void* );
    typedef int ( *TXDefaultScreen )( void* );
    typedef int ( *TXDisplayWidth )( void*, int );
    typedef int ( *TXDefaultDepth )( void*, int );
    typedef unsigned long ( *TXRootWindow )( void*, int );
    typedef SXRRScreenSize* ( *TXRRSizes )( void*, int, int* );
    typedef short* ( *TXRRRates )( void*, int, int, int* );
    typedef void* ( *TXRRGetScreenInfo )( void*, unsigned long );
    typedef short ( *TXRRConfigCurrentRate )( void* );
    typedef void ( *TXRRFreeScreenConfigInfo )( void* );

    CX11Display( void )
        : m_x11( GUCEF_NULL ), m_xrandr( GUCEF_NULL ), m_display( GUCEF_NULL ), m_screen( 0 )
        , XCloseDisplay( GUCEF_NULL ), XDisplayWidth( GUCEF_NULL ), XDisplayHeight( GUCEF_NULL ), XDefaultDepth( GUCEF_NULL )
        , XRootWindow( GUCEF_NULL ), XRRSizes( GUCEF_NULL ), XRRRates( GUCEF_NULL ), XRRGetScreenInfo( GUCEF_NULL )
        , XRRConfigCurrentRate( GUCEF_NULL ), XRRFreeScreenConfigInfo( GUCEF_NULL )
    {
        m_x11 = CORE::LoadModuleDynamicly( "libX11.so.6" );
        if ( GUCEF_NULL == m_x11 )
            return;
        TXOpenDisplay xOpenDisplay = (TXOpenDisplay) Function( m_x11, "XOpenDisplay" );
        TXDefaultScreen xDefaultScreen = (TXDefaultScreen) Function( m_x11, "XDefaultScreen" );
        XCloseDisplay = (TXCloseDisplay) Function( m_x11, "XCloseDisplay" );
        XDisplayWidth = (TXDisplayWidth) Function( m_x11, "XDisplayWidth" );
        XDisplayHeight = (TXDisplayWidth) Function( m_x11, "XDisplayHeight" );
        XDefaultDepth = (TXDefaultDepth) Function( m_x11, "XDefaultDepth" );
        XRootWindow = (TXRootWindow) Function( m_x11, "XRootWindow" );
        if ( GUCEF_NULL == xOpenDisplay || GUCEF_NULL == xDefaultScreen || GUCEF_NULL == XCloseDisplay ||
             GUCEF_NULL == XDisplayWidth || GUCEF_NULL == XDisplayHeight || GUCEF_NULL == XDefaultDepth || GUCEF_NULL == XRootWindow )
            return;

        m_display = xOpenDisplay( GUCEF_NULL );
        if ( GUCEF_NULL == m_display )
            return;
        m_screen = xDefaultScreen( m_display );

        m_xrandr = CORE::LoadModuleDynamicly( "libXrandr.so.2" );
        if ( GUCEF_NULL != m_xrandr )
        {
            XRRSizes = (TXRRSizes) Function( m_xrandr, "XRRSizes" );
            XRRRates = (TXRRRates) Function( m_xrandr, "XRRRates" );
            XRRGetScreenInfo = (TXRRGetScreenInfo) Function( m_xrandr, "XRRGetScreenInfo" );
            XRRConfigCurrentRate = (TXRRConfigCurrentRate) Function( m_xrandr, "XRRConfigCurrentRate" );
            XRRFreeScreenConfigInfo = (TXRRFreeScreenConfigInfo) Function( m_xrandr, "XRRFreeScreenConfigInfo" );
        }
    }

    ~CX11Display()
    {
        if ( GUCEF_NULL != m_display )
            XCloseDisplay( m_display );
        if ( GUCEF_NULL != m_xrandr )
            CORE::UnloadModuleDynamicly( m_xrandr );
        if ( GUCEF_NULL != m_x11 )
            CORE::UnloadModuleDynamicly( m_x11 );
    }

    bool IsOpen( void ) const { return GUCEF_NULL != m_display; }

    bool HasRandR( void ) const { return GUCEF_NULL != XRRSizes && GUCEF_NULL != XRRRates; }

    /** The refresh rate the screen runs at, 0 when RandR cannot tell */
    UInt32 GetCurrentRate( void ) const
    {
        if ( GUCEF_NULL == XRRGetScreenInfo || GUCEF_NULL == XRRConfigCurrentRate || GUCEF_NULL == XRRFreeScreenConfigInfo )
            return 0;
        void* config = XRRGetScreenInfo( m_display, XRootWindow( m_display, m_screen ) );
        if ( GUCEF_NULL == config )
            return 0;
        short rate = XRRConfigCurrentRate( config );
        XRRFreeScreenConfigInfo( config );
        return rate > 0 ? (UInt32) rate : 0;
    }

    void* m_x11;
    void* m_xrandr;
    void* m_display;
    int m_screen;
    TXCloseDisplay XCloseDisplay;
    TXDisplayWidth XDisplayWidth;
    TXDisplayWidth XDisplayHeight;
    TXDefaultDepth XDefaultDepth;
    TXRootWindow XRootWindow;
    TXRRSizes XRRSizes;
    TXRRRates XRRRates;
    TXRRGetScreenInfo XRRGetScreenInfo;
    TXRRConfigCurrentRate XRRConfigCurrentRate;
    TXRRFreeScreenConfigInfo XRRFreeScreenConfigInfo;

    private:

    static void* Function( void* module, const char* name )
    {
        return CORE::GetFunctionAddress( module, name, 0 ).objPtr;
    }
};

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ? */

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CVideoOptions::CVideoOptions( void )
    : m_displayModes()
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/
    
CVideoOptions::CVideoOptions( const CVideoOptions& src )
    : m_displayModes( src.m_displayModes )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CVideoOptions::~CVideoOptions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CVideoOptions&
CVideoOptions::operator=( const CVideoOptions& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_displayModes = src.m_displayModes;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const CVideoOptions::TDisplayModeVector&
CVideoOptions::GetDisplayModes( void ) const
{GUCEF_TRACE;

    return m_displayModes;
}

/*-------------------------------------------------------------------------*/

void
CVideoOptions::Clear( void )
{GUCEF_TRACE;
    
    m_displayModes.clear();
}

/*-------------------------------------------------------------------------*/

void
CVideoOptions::AddDisplayMode( const TDisplayMode& displayMode )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_displayModes.size(); ++i )
    {
        const TDisplayMode& mode = m_displayModes[ i ];
        if ( mode.widthInPixels == displayMode.widthInPixels && mode.heightInPixels == displayMode.heightInPixels &&
             mode.depthInBits == displayMode.depthInBits && mode.frequency == displayMode.frequency )
            return;
    }
    m_displayModes.push_back( displayMode );
}

/*-------------------------------------------------------------------------*/

const CVideoOptions::TDisplayModeVector&
CVideoOptions::GetCommonGameResolutions( void )
{GUCEF_TRACE;

    static TDisplayModeVector resolutions;
    if ( resolutions.empty() )
    {
        static const UInt32 sizes[][ 2 ] = { { 640, 480 }, { 800, 600 }, { 1024, 768 }, { 1280, 720 }, { 1280, 800 }, { 1280, 960 },
                                             { 1280, 1024 }, { 1366, 768 }, { 1440, 900 }, { 1600, 900 }, { 1600, 1200 }, { 1680, 1050 },
                                             { 1920, 1080 }, { 1920, 1200 }, { 2560, 1080 }, { 2560, 1440 }, { 2560, 1600 }, { 3840, 1080 },
                                             { 3840, 1200 }, { 3440, 1440 }, { 3840, 1600 }, { 5120, 1440 }, { 3840, 2160 }, { 5120, 2160 } };
        for ( UInt32 i=0; i<sizeof( sizes ) / sizeof( sizes[ 0 ] ); ++i )
            resolutions.push_back( MakeDisplayMode( sizes[ i ][ 0 ], sizes[ i ][ 1 ], 32, 60 ) );
    }
    return resolutions;
}

/*-------------------------------------------------------------------------*/

void
CVideoOptions::GetCommonResolutions( bool fullscreen, TDisplayModeVector& resolutions ) const
{GUCEF_TRACE;

    const TDisplayModeVector& common = GetCommonGameResolutions();

    UInt32 maxWidth = 0, maxHeight = 0;
    for ( UInt32 i=0; i<(UInt32)m_displayModes.size(); ++i )
    {
        if ( m_displayModes[ i ].widthInPixels > maxWidth )
            maxWidth = m_displayModes[ i ].widthInPixels;
        if ( m_displayModes[ i ].heightInPixels > maxHeight )
            maxHeight = m_displayModes[ i ].heightInPixels;
    }

    for ( UInt32 c=0; c<(UInt32)common.size(); ++c )
    {
        const TDisplayMode& resolution = common[ c ];
        if ( !fullscreen )
        {
            if ( m_displayModes.empty() || ( resolution.widthInPixels <= maxWidth && resolution.heightInPixels <= maxHeight ) )
                resolutions.push_back( resolution );
            continue;
        }

        // the mode of the display with this size and the highest frequency, the deepest colours of those
        const TDisplayMode* best = GUCEF_NULL;
        for ( UInt32 i=0; i<(UInt32)m_displayModes.size(); ++i )
        {
            const TDisplayMode& mode = m_displayModes[ i ];
            if ( mode.widthInPixels != resolution.widthInPixels || mode.heightInPixels != resolution.heightInPixels )
                continue;
            if ( GUCEF_NULL == best || mode.frequency > best->frequency ||
                 ( mode.frequency == best->frequency && mode.depthInBits > best->depthInBits ) )
                best = &mode;
        }
        if ( GUCEF_NULL != best )
            resolutions.push_back( *best );
    }

    // the largest mode of the display is always offered: a display can have a size that no list of usual sizes holds
    const TDisplayMode* native = GUCEF_NULL;
    for ( UInt32 i=0; i<(UInt32)m_displayModes.size(); ++i )
    {
        const TDisplayMode& mode = m_displayModes[ i ];
        if ( GUCEF_NULL == native || mode.widthInPixels * mode.heightInPixels > native->widthInPixels * native->heightInPixels ||
             ( mode.widthInPixels * mode.heightInPixels == native->widthInPixels * native->heightInPixels && mode.widthInPixels > native->widthInPixels ) )
            native = &mode;
    }
    if ( GUCEF_NULL == native )
        return;

    const UInt64 nativeArea = (UInt64) native->widthInPixels * native->heightInPixels;
    UInt32 insertAt = (UInt32) resolutions.size();
    for ( UInt32 i=0; i<(UInt32)resolutions.size(); ++i )
    {
        if ( resolutions[ i ].widthInPixels == native->widthInPixels && resolutions[ i ].heightInPixels == native->heightInPixels )
            return;
        if ( insertAt == (UInt32) resolutions.size() && (UInt64) resolutions[ i ].widthInPixels * resolutions[ i ].heightInPixels > nativeArea )
            insertAt = i;
    }

    // at its highest frequency
    const TDisplayMode* best = native;
    for ( UInt32 i=0; i<(UInt32)m_displayModes.size(); ++i )
    {
        const TDisplayMode& mode = m_displayModes[ i ];
        if ( mode.widthInPixels == native->widthInPixels && mode.heightInPixels == native->heightInPixels &&
             ( mode.frequency > best->frequency || ( mode.frequency == best->frequency && mode.depthInBits > best->depthInBits ) ) )
            best = &mode;
    }
    resolutions.insert( resolutions.begin() + insertAt, *best );
}

/*-------------------------------------------------------------------------*/

void
CVideoOptions::RegisterDisplayModeSource( const CString& name, const SDisplayModeSource& source )
{GUCEF_TRACE;

    SDisplayModeSources& sources = GetDisplayModeSources();
    MT::CScopeMutex lock( sources.lock );
    sources.sources[ name ] = source;
}

/*-------------------------------------------------------------------------*/

void
CVideoOptions::UnregisterDisplayModeSource( const CString& name )
{GUCEF_TRACE;

    SDisplayModeSources& sources = GetDisplayModeSources();
    MT::CScopeMutex lock( sources.lock );
    sources.sources.erase( name );
}

/*-------------------------------------------------------------------------*/

void
CVideoOptions::SelectDisplayModeSource( const CString& name )
{GUCEF_TRACE;

    SDisplayModeSources& sources = GetDisplayModeSources();
    MT::CScopeMutex lock( sources.lock );
    sources.selected = name;
}

/*-------------------------------------------------------------------------*/

CString
CVideoOptions::GetSelectedDisplayModeSource( void )
{GUCEF_TRACE;

    SDisplayModeSources& sources = GetDisplayModeSources();
    MT::CScopeMutex lock( sources.lock );
    return sources.selected;
}

/*-------------------------------------------------------------------------*/

bool
CVideoOptions::RetrieveCurrentDisplayModeFromOS( TDisplayMode& displayMode )
{GUCEF_TRACE;

    SDisplayModeSource source;
    if ( GetSelectedSource( source ) && GUCEF_NULL != source.retrieveCurrentMode && source.retrieveCurrentMode( displayMode ) )
        return true;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    DEVMODE currentMode;
    memset( &currentMode, 0, sizeof( DEVMODE ) );
    currentMode.dmSize = sizeof( DEVMODE );
    if ( TRUE != EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &currentMode ) || 0 == currentMode.dmPelsWidth || 0 == currentMode.dmPelsHeight )
        return false;
    displayMode = MakeDisplayMode( currentMode.dmPelsWidth, currentMode.dmPelsHeight, currentMode.dmBitsPerPel, currentMode.dmDisplayFrequency );
    return true;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    CX11Display x11;
    if ( !x11.IsOpen() )
        return false;
    // a screen of no size has no display to show it (switched off, the session locked): there is no mode
    if ( x11.XDisplayWidth( x11.m_display, x11.m_screen ) <= 0 || x11.XDisplayHeight( x11.m_display, x11.m_screen ) <= 0 )
        return false;
    UInt32 rate = x11.GetCurrentRate();
    displayMode = MakeDisplayMode( (UInt32) x11.XDisplayWidth( x11.m_display, x11.m_screen )  ,
                                   (UInt32) x11.XDisplayHeight( x11.m_display, x11.m_screen ) ,
                                   (UInt32) x11.XDefaultDepth( x11.m_display, x11.m_screen )  ,
                                   rate > 0 ? rate : 60                                       );
    return true;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    int width = 0, height = 0;
    emscripten_get_screen_size( &width, &height );
    if ( width <= 0 || height <= 0 )
        return false;
    displayMode = MakeDisplayMode( (UInt32) width, (UInt32) height, 32, 60 );
    return true;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CVideoOptions::RetrieveOptionsFromOS( CVideoOptions& options )
{GUCEF_TRACE;

    SDisplayModeSource source;
    if ( GetSelectedSource( source ) && GUCEF_NULL != source.retrieveOptions )
    {
        CVideoOptions fromSource;
        if ( source.retrieveOptions( fromSource ) && !fromSource.GetDisplayModes().empty() )
        {
            options = fromSource;
            return true;
        }
    }

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    UInt32 i=0;
    DEVMODE displayMode;
    memset( &displayMode, 0, sizeof( DEVMODE ) );
    displayMode.dmSize = sizeof( DEVMODE );
    TDisplayMode newMode;
    
    options.Clear();
    while ( TRUE == EnumDisplaySettings( NULL         ,
                                         i            ,
                                         &displayMode ) )
    {
        newMode.widthInPixels = displayMode.dmPelsWidth; 
        newMode.heightInPixels = displayMode.dmPelsHeight;
        newMode.depthInBits = displayMode.dmBitsPerPel;
        newMode.frequency = displayMode.dmDisplayFrequency;
        options.m_displayModes.push_back( newMode );
        ++i;
    } 
    
    return true;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    CX11Display x11;
    if ( !x11.IsOpen() )
        return false;

    options.Clear();
    UInt32 depth = (UInt32) x11.XDefaultDepth( x11.m_display, x11.m_screen );
    if ( x11.HasRandR() )
    {
        int sizeCount = 0;
        SXRRScreenSize* sizes = x11.XRRSizes( x11.m_display, x11.m_screen, &sizeCount );
        for ( int s=0; GUCEF_NULL != sizes && s<sizeCount; ++s )
        {
            if ( sizes[ s ].width <= 0 || sizes[ s ].height <= 0 )
                continue;
            int rateCount = 0;
            short* rates = x11.XRRRates( x11.m_display, x11.m_screen, s, &rateCount );
            if ( GUCEF_NULL == rates || 0 == rateCount )
            {
                options.AddDisplayMode( MakeDisplayMode( (UInt32) sizes[ s ].width, (UInt32) sizes[ s ].height, depth, 60 ) );
                continue;
            }
            for ( int r=0; r<rateCount; ++r )
                if ( rates[ r ] > 0 )
                    options.AddDisplayMode( MakeDisplayMode( (UInt32) sizes[ s ].width, (UInt32) sizes[ s ].height, depth, (UInt32) rates[ r ] ) );
        }
    }

    // without RandR the screen as it is now is the one mode there is
    if ( options.m_displayModes.empty() )
    {
        TDisplayMode currentMode;
        if ( RetrieveCurrentDisplayModeFromOS( currentMode ) )
            options.AddDisplayMode( currentMode );
    }
    return !options.m_displayModes.empty();

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    // a page cannot change the mode of the screen: the screen as it is, is the one mode
    TDisplayMode currentMode;
    if ( !RetrieveCurrentDisplayModeFromOS( currentMode ) )
        return false;
    options.Clear();
    options.AddDisplayMode( currentMode );
    return true;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

void
CVideoOptions::GetDisplayModesForResolution( const UInt32 widthInPixels  , 
                                             const UInt32 heightInPixels ,
                                             TDisplayModeVector& modes   ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_displayModes.size(); ++i )
    {
        const TDisplayMode& displayMode = m_displayModes[ i ];
        if ( ( displayMode.widthInPixels == widthInPixels ) &&
             ( displayMode.heightInPixels == heightInPixels ) )
        {
            modes.push_back( displayMode );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CVideoOptions::AreSettingsPossible( const CVideoSettings& settings ) const
{GUCEF_TRACE;

    UInt32 widthInPixels = settings.GetResolutionWidthInPixels();
    UInt32 heightInPixels = settings.GetResolutionHeightInPixels();
    for ( UInt32 i=0; i<m_displayModes.size(); ++i )
    {
        const TDisplayMode& displayMode = m_displayModes[ i ];
        if ( ( displayMode.widthInPixels == widthInPixels ) &&
             ( displayMode.heightInPixels == heightInPixels ) )
        {
            if ( ( displayMode.depthInBits == settings.GetResolutionDepthInBits() ) &&
                 ( displayMode.frequency == settings.GetFrequency() )                )
            {
                return true;
            }
        }
    }
    return false;    
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
