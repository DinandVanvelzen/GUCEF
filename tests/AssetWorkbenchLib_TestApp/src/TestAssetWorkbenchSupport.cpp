/*
 *  AssetWorkbenchLib_TestApp: Test application for the asset workbench library (AssetWorkbenchLib)
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

#include <stdlib.h>
#include <string.h>

#include <Ogre.h>

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#include "pluginglueOGRE_COgreGlobal.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#include "pluginglueOGRE_COgreRootSetup.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H ? */

#ifndef GUCEF_IMAGE_CPNGIMAGECODEC_H
#include "gucefIMAGE_CPngImageCodec.h"
#define GUCEF_IMAGE_CPNGIMAGECODEC_H
#endif /* GUCEF_IMAGE_CPNGIMAGECODEC_H ? */

#ifndef GUCEF_GUI_CGUIINPUTADAPTER_H
#include "gucefGUI_CGUIInputAdapter.h"
#define GUCEF_GUI_CGUIINPUTADAPTER_H
#endif /* GUCEF_GUI_CGUIINPUTADAPTER_H ? */

#ifndef GUCEF_GUI_STANDARDTHEMES_H
#include "gucefGUI_StandardThemes.h"
#define GUCEF_GUI_STANDARDTHEMES_H
#endif /* GUCEF_GUI_STANDARDTHEMES_H ? */

#include "TestAssetWorkbenchSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

GUI::TWindowContextPtr g_mainWindow;
const char* const GuiPluginGroup = "assetWorkbenchTestGuiPlugins";
const char* const OgrePluginGroup = "assetWorkbenchTestOgrePlugins";

bool
EnvIs( const char* name, char value )
{
    const char* text = ::getenv( name );
    return GUCEF_NULL != text && value == text[ 0 ];
}

bool
HasDisplay( void )
{
    const char* display = ::getenv( "DISPLAY" );
    return GUCEF_NULL != display && '\0' != display[ 0 ];
}

void
AddOgrePluginEntry( CORE::CDataNode& group, const CORE::CString& pluginName, bool loadFailAllowed )
{
    CORE::CDataNode* entry = group.AddChild( "PluginMetaData" );
    entry->SetAttribute( "Description", pluginName );
    entry->SetAttribute( "Path", "$MODULEDIR$" );
    entry->SetAttribute( "Filename", pluginName );
    entry->SetAttribute( "AltFilename", pluginName + "_d" );
    entry->SetAttribute( "Type", "OgrePlugin" );
    entry->SetAttribute( "LoadFailAllowed", loadFailAllowed );
}

bool
LoadGuiPlugin( const char* moduleName )
{
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    const CORE::CString path = CORE::CString( "$MODULEDIR$/" ) + moduleName;
    return pluginControl.AddPluginFromDir( path, GuiPluginGroup, true ) || pluginControl.AddPluginFromDir( path + "_d", GuiPluginGroup, true );
}

}

/*-------------------------------------------------------------------------*/

ETestWindowMode
GetTestWindowMode( void )
{GUCEF_TRACE;

    if ( EnvIs( "GUCEF_TEST_RENDER_GPU", '0' ) )
        return TESTWINDOWMODE_NONE;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    if ( HasDisplay() )
        return EnvIs( "GUCEF_TEST_X11_WINDOWS", '1' ) ? TESTWINDOWMODE_X11 : TESTWINDOWMODE_NONE;
    return TESTWINDOWMODE_OFFSCREEN;
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return TESTWINDOWMODE_WIN32;
    #else
    return TESTWINDOWMODE_NONE;
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetNoWindowReason( void )
{GUCEF_TRACE;

    if ( EnvIs( "GUCEF_TEST_RENDER_GPU", '0' ) )
        return "GUCEF_TEST_RENDER_GPU=0 skips the rendering tests";
    if ( HasDisplay() )
        return "windows on the X server of DISPLAY are made only with GUCEF_TEST_X11_WINDOWS=1 (a server for tests such as Xvfb)";
    return "the platform has no window mode for these tests";
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetTestRenderer( void )
{GUCEF_TRACE;

    // Ogre's headless surface is one of EGL: offscreen windows are OpenGL ones
    const char* renderer = ::getenv( "GUCEF_TEST_OGRE_RENDERER" );
    if ( GUCEF_NULL == renderer || '\0' == renderer[ 0 ] || TESTWINDOWMODE_OFFSCREEN == GetTestWindowMode() )
        return "OpenGL";
    return renderer;
}

/*-------------------------------------------------------------------------*/

bool
SetUpWorkbenchTests( void )
{GUCEF_TRACE;

    VFS::CVfsGlobal::Instance();
    IMAGE::CImageGlobal::Instance();
    GUI::CGuiGlobal::Instance();

    // the systems of the support library, as its plugin entry point makes them when the bootstrap configuration loads it
    if ( GUCEF_NULL == PLUGINGLUE::OGRE::COgreGlobal::Instance() )
        return false;

    // the image codecs come first: the Ogre plugin that hands them to Ogre makes its codecs of the ones that are there when it loads
    LoadGuiPlugin( "imgpluginFreeImage" );

    // the Ogre plugins as the bootstrap configuration lists them
    CORE::CDataNode config;
    CORE::CDataNode* group = config.AddChild( "PluginGroup" );
    group->SetAttribute( "GroupName", OgrePluginGroup );
    group->SetAttribute( "LoadImmediately", true );
    AddOgrePluginEntry( *group, "OgreRenderSystem_GL3Plus", true );
    AddOgrePluginEntry( *group, "OgreRenderSystem_Vulkan", true );
    AddOgrePluginEntry( *group, "OgrePlugin_GLSLangProgramManager", true );
    AddOgrePluginEntry( *group, "OgrePlugin_GucefVFS", true );
    AddOgrePluginEntry( *group, "OgreCodec_GucefIMAGE", true );
    AddOgrePluginEntry( *group, "OgreCodec_Assimp", true );
    AddOgrePluginEntry( *group, "OgrePlugin_DotScene", true );
    AddOgrePluginEntry( *group, "OgrePlugin_ParticleFX", true );
    CORE::CCoreGlobal::Instance()->GetPluginControl().LoadConfig( config );

    CORE::CString::StringVector renderSystems;
    renderSystems.push_back( "OpenGL 3+ Rendering Subsystem" );
    renderSystems.push_back( "Vulkan Rendering Subsystem" );
    PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetRootSetup().SetRenderSystems( renderSystems );

    // the window backend, the platform window driver, the GUI driver, the fonts and the archives of the VFS
    LoadGuiPlugin( "vfspluginLIBARCHIVE" );
    LoadGuiPlugin( "vfspluginUNITYPACKAGE" );
    LoadGuiPlugin( "fontpluginFREETYPE" );
    LoadGuiPlugin( "guidriverOgre" );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    LoadGuiPlugin( "guidriverXWinGL" );
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    LoadGuiPlugin( "guidriverWin32" );
    #endif
    LoadGuiPlugin( "guidriverImGuiOgre" );

    // what the catalogue of the primary store needs: JSON and compression for its files, the table codec and the database of the VFS tables
    LoadGuiPlugin( "dstorepluginJSONPARSER" );
    LoadGuiPlugin( "codecspluginZLIB" );
    // the sealing and key derivation of the credential stores of the sign in of the online stores
    LoadGuiPlugin( "codecspluginMONOCYPHER" );
    LoadGuiPlugin( "tablecodecpluginPARQUET" );
    LoadGuiPlugin( "dbpluginVFSTABLES" );
    return !GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( "Ogre" ).IsNULL();
}

/*-------------------------------------------------------------------------*/

void
TearDownWorkbenchTests( void )
{GUCEF_TRACE;

    // the order of an application: the windows, the GUI and window plugins (their GPU resources go while the Root exists), the
    // Root (Ogre shuts down while the code of its plugins is loaded), the Ogre plugin modules
    ReleaseMainTestWindow();
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    pluginControl.UnloadPluginGroup( GuiPluginGroup );
    PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetRootSetup().DestroyRoot();
    pluginControl.UnloadPluginGroup( OgrePluginGroup );
    PLUGINGLUE::OGRE::COgreGlobal::Deinstance();
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
GetMainTestWindow( void )
{GUCEF_TRACE;

    if ( !g_mainWindow.IsNULL() )
        return g_mainWindow;
    GUI::TWindowManagerBackendPtr backend = GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( "Ogre" );
    if ( backend.IsNULL() || TESTWINDOWMODE_NONE == GetTestWindowMode() )
        return GUI::TWindowContextPtr();

    GUI::CVideoSettings settings;
    // 1280 x 720, or the size that GUCEF_TEST_WINDOW_SIZE gives ("1600x900"), for the pictures that people look at
    UInt32 width = 1280;
    UInt32 height = 720;
    const char* size = ::getenv( "GUCEF_TEST_WINDOW_SIZE" );
    if ( GUCEF_NULL != size && '\0' != size[ 0 ] )
    {
        const CORE::CString text( size );
        const CORE::CString::StringVector parts = text.ParseElements( 'x', false );
        if ( 2 == parts.size() && CORE::StringToUInt32( parts[ 0 ], 0 ) >= 640 && CORE::StringToUInt32( parts[ 1 ], 0 ) >= 480 )
        {
            width = CORE::StringToUInt32( parts[ 0 ], 0 );
            height = CORE::StringToUInt32( parts[ 1 ], 0 );
        }
    }
    settings.SetResolution( width, height, 32 );
    settings.SetFullscreenState( false );
    settings.SetVSyncState( false );
    settings.SetRenderer( GetTestRenderer() );
    // the tests draw the frames themselves; the X11 window paces its own (60 a second) when the application pulses
    settings.SetFrequency( TESTWINDOWMODE_OFFSCREEN == GetTestWindowMode() ? 0 : 60 );
    CORE::CValueList params;
    if ( TESTWINDOWMODE_OFFSCREEN == GetTestWindowMode() )
        params.Set( GUIDRIVEROGRE::COgreWindowContext::PlatformBackendParameterName, GUIDRIVEROGRE::COgreWindowContext::OffscreenPlatformBackend );
    else
        params.Set( GUIDRIVEROGRE::COgreWindowContext::PlatformBackendParameterName, TESTWINDOWMODE_WIN32 == GetTestWindowMode() ? "Win32" : "XWinGL" );
    g_mainWindow = backend->CreateWindowContext( "WorkbenchTest", settings, &params );
    return g_mainWindow;
}

/*-------------------------------------------------------------------------*/

void
ReleaseMainTestWindow( void )
{GUCEF_TRACE;

    if ( g_mainWindow.IsNULL() )
        return;
    GUI::TWindowManagerBackendPtr backend = GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( "Ogre" );
    if ( !backend.IsNULL() )
        backend->DestroyWindowContext( g_mainWindow );
    g_mainWindow.Unlink();
}

/*-------------------------------------------------------------------------*/

GUIDRIVEROGRE::COgreWindowContext*
AsOgreWindow( GUI::TWindowContextPtr window )
{GUCEF_TRACE;

    if ( window.IsNULL() || GUIDRIVEROGRE::COgreWindowContext::ClassTypeName != window->GetClassTypeName() )
        return GUCEF_NULL;
    return static_cast< GUIDRIVEROGRE::COgreWindowContext* >( window.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CreateWorkbenchContext( void )
{GUCEF_TRACE;

    GUI::TWindowContextPtr window = GetMainTestWindow();
    if ( window.IsNULL() )
        return GUI::TGuiContextPtr();
    GUI::TGuiContextPtr contextPtr = GUI::CGuiGlobal::Instance()->GetGuiManager().CreateGUIContext( "ImGuiOgre", window );
    GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
    if ( GUCEF_NULL == context )
        return GUI::TGuiContextPtr();
    context->SetLogicalResolution( 0, 0 );
    if ( !GUI::CGuiGlobal::Instance()->MountData() || !context->RegisterFont( "WorkbenchFont", GUI::CGuiGlobal::DefaultUiFontPath, 14 ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CreateWorkbenchContext: The font of the forms cannot be made" );
    CORE::CDataNode theme;
    GUI::BuildDarkTheme( theme, "WorkbenchFont" );
    context->SetTheme( theme );
    return contextPtr;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
GetTestWorkDir( void )
{GUCEF_TRACE;

    CORE::CString dir = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( dir, "AssetWorkbenchLib_TestApp_Work" );
    return dir;
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeCleanTestDir( const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString dir = GetTestWorkDir();
    CORE::AppendToPath( dir, name );
    if ( CORE::DirExists( dir ) )
        CORE::DeleteDir( dir, true );
    CORE::CreatePathDirectories( dir + "/" );
    return dir;
}

/*-------------------------------------------------------------------------*/

bool
WriteTestFile( const CORE::CString& path, const CORE::CString& content )
{GUCEF_TRACE;

    CORE::CreatePathDirectories( path.SubstrToChar( '/', false ) );
    CORE::CFileAccess file( path, "wb" );
    if ( !file.IsValid() )
        return false;
    if ( content.IsNULLOrEmpty() )
        return true;
    return 1 == file.Write( content.C_String(), content.Length(), 1 );
}

/*-------------------------------------------------------------------------*/

void
GetPixel( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y, UInt8 rgba[ 4 ] )
{GUCEF_TRACE;

    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
    memcpy( rgba, data, 4 );
}

/*-------------------------------------------------------------------------*/

bool
PixelIs( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y, UInt8 r, UInt8 g, UInt8 b, Int32 tolerance )
{GUCEF_TRACE;

    if ( pixels.IsNULL() || x < 0 || y < 0 || x >= (Int32) pixels->GetWidthInPixels() || y >= (Int32) pixels->GetHeightInPixels() )
        return false;
    UInt8 rgba[ 4 ];
    GetPixel( pixels, x, y, rgba );
    const bool same = abs( (Int32) rgba[ 0 ] - r ) <= tolerance && abs( (Int32) rgba[ 1 ] - g ) <= tolerance && abs( (Int32) rgba[ 2 ] - b ) <= tolerance;
    if ( !same )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PixelIs: pixel " << x << "," << y << " is " << (UInt32) rgba[ 0 ] << "," << (UInt32) rgba[ 1 ] << ","
            << (UInt32) rgba[ 2 ] << "," << (UInt32) rgba[ 3 ] << ", expected " << (UInt32) r << "," << (UInt32) g << "," << (UInt32) b );
    }
    return same;
}

/*-------------------------------------------------------------------------*/

bool
PixelsDiffer( const IMAGE::TPixelMapPtr& pixels, Int32 x1, Int32 y1, Int32 x2, Int32 y2, Int32 tolerance )
{GUCEF_TRACE;

    if ( pixels.IsNULL() )
        return false;
    UInt8 a[ 4 ], b[ 4 ];
    GetPixel( pixels, x1, y1, a );
    GetPixel( pixels, x2, y2, b );
    return abs( (Int32) a[ 0 ] - b[ 0 ] ) > tolerance || abs( (Int32) a[ 1 ] - b[ 1 ] ) > tolerance || abs( (Int32) a[ 2 ] - b[ 2 ] ) > tolerance;
}

/*-------------------------------------------------------------------------*/

void
DumpPicture( const IMAGE::TPixelMapPtr& pixels, const CORE::CString& name )
{GUCEF_TRACE;

    const char* folder = ::getenv( "GUCEF_TEST_DUMP_DIR" );
    if ( GUCEF_NULL == folder || '\0' == folder[ 0 ] || pixels.IsNULL() )
        return;
    CORE::CreatePathDirectories( CORE::CString( folder ) + "/" );
    CORE::CString problem;
    IMAGE::CPngImageCodec::WritePixelMapToFile( *pixels, CORE::CString( folder ) + "/" + name + ".png", problem );
}

/*-------------------------------------------------------------------------*/

CThumbnailRecorder::CThumbnailRecorder( CORE::CNotifier* notifier, const CORE::CEvent& id )
    : CORE::CObserver()
    , count( 0 )
    , lastPath()
    , lastSize( 0 )
    , lastHadPixels( false )
    , m_eventId( id )
{GUCEF_TRACE;

    SubscribeTo( notifier, id );
}

/*-------------------------------------------------------------------------*/

CThumbnailRecorder::~CThumbnailRecorder()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CThumbnailRecorder::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( id != m_eventId )
        return;
    ++count;
    // the thumbnails: remember what the last one was about
    if ( GUCEF_NULL != eventData )
    {
        const GUI::CIThumbnailProvider::TThumbnailReadyEventData* ready = static_cast< const GUI::CIThumbnailProvider::TThumbnailReadyEventData* >( eventData );
        lastPath = ready->GetData().path;
        lastSize = ready->GetData().size;
        lastHadPixels = !ready->GetData().pixels.IsNULL();
    }
}
