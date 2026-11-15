/*
 *  AssetViewerLib: the application of the viewer: command line, settings, mounts, the window and the capture or the interactive session
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

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_INPUT_H
#include "gucefINPUT.h"
#include "gucefINPUT_CInputGlobal.h"
#include "CInputController.h"
#include "CInputContext.h"
#define GUCEF_INPUT_H
#endif /* GUCEF_INPUT_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#include "gucefGUI_CGUIInputAdapter.h"
#include "gucefGUI_CGeneralSettingsForm.h"
#include "gucefGUI_StandardThemes.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefAUTH_CAuthGlobal.h"

#include "AssetViewerLib_CViewerApp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CViewerApp::ClassTypeName = "GUCEF::ASSETVIEWER::CViewerApp";
const CORE::CString CViewerApp::AppName = "AssetViewer";
const CORE::CString CViewerApp::WindowTitle = "Asset viewer";

namespace {

const char* const WindowBackendName = "Ogre";                   /**< the window backend of guidriverOgre */
const char* const GuiDriverName = "ImGuiOgre";                  /**< the GUI driver of guidriverImGuiOgre */
const char* const FontName = "ViewerFont";
const UInt32 FontPixelSize = 16;
const UInt32 DefaultTileSize = 256;
const UInt32 ThumbnailTileSize = 128;
const UInt32 ThumbnailBudgetInMs = 6;
const UInt32 ScriptWaitPulses = 3000;            /**< how long --ingest and --search wait for the store before the script goes on */

/** The input driver of the platform window that the Ogre window is made on */
CORE::CString
PlatformInputDriverName( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    return "XWINMSG";
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return "MSWINMSG";
    #else
    return CORE::CString::Empty;
    #endif
}

bool
HasDisplay( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    const char* display = ::getenv( "DISPLAY" );
    return GUCEF_NULL != display && '\0' != display[ 0 ];
    #else
    return true;
    #endif
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CViewerApp::CViewerApp( int argc, char** argv )
    : CORE::CGloballyConfigurable( false )
    , CORE::CObserver()
    , m_argc( argc )
    , m_argv( argv )
    , m_bootstrapConfigFile()
    , m_configFile()
    , m_logFile()
    , m_renderer()
    , m_openPath()
    , m_captureFolder()
    , m_screenshotFile()
    , m_searchText()
    , m_ingestPaths()
    , m_thumbnailCacheFolder()
    , m_mountSpecs()
    , m_captureAssets()
    , m_openLocations()
    , m_settingsThumbnailCache()
    , m_settingsLogFile()
    , m_settingsTileSize( DefaultTileSize )
    , m_minimalLogLevel( CORE::LOGLEVEL_NORMAL )
    , m_tileSize( DefaultTileSize )
    , m_offscreen( false )
    , m_capturing( false )
    , m_videoSettings()
    , m_generalSettings()
    , m_sourcesSettings()
    , m_recentLocations()
    , m_credentials()
    , m_workStartTrigger()
    , m_mounter()
    , m_window()
    , m_gui()
    , m_ogreWindow( GUCEF_NULL )
    , m_capture( GUCEF_NULL )
    , m_shell( GUCEF_NULL )
    , m_input( GUCEF_NULL )
    , m_inputAdapter( GUCEF_NULL )
    , m_nextCapture( 0 )
    , m_captureFailures( 0 )
    , m_scriptStage( 0 )
    , m_scriptFrames( 0 )
    , m_exitCode( 0 )
    , m_started( false )
    , m_stopRequested( false )
    , m_recentDirty( false )
{GUCEF_TRACE;

    m_videoSettings.SetResolution( 1600, 900, 32 );
    m_videoSettings.SetFullscreenState( false );
    m_videoSettings.SetFrequency( 60 );

    // before the plugins of the stores are loaded: they take the stores of the sessions and of the saved passwords when they make their provider
    m_credentials.Install();
}

/*-------------------------------------------------------------------------*/

CViewerApp::~CViewerApp()
{GUCEF_TRACE;

    Shutdown();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CViewerApp::LogUsage( void )
{GUCEF_TRACE;

    GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, "Usage: " << AppName << " [options] [<path>...]\n"
        "  <path>                             a folder, an archive (zip, 7z, tar, unitypackage, ...) or a file (a model, a picture) to open at the\n"
        "                                     start, as \"Open folder\", \"Open archive or pack\" and \"Open file\" do; repeat for more. Without any\n"
        "                                     path the viewer shows the files of this computer: use the buttons of the Content tab or Ctrl+O\n"
        "  --mount <mountPoint>=<source>      mount a folder, an archive (zip, unitypackage, ...) or the VFS path of an archive in\n"
        "                                     an earlier mount (a package inside a zip); repeat for more mounts\n"
        "  --open <vfs path>                  show this asset at the start\n"
        "  --renderer <name>                  draw with this renderer (OpenGL, Vulkan) instead of the one of the video settings\n"
        "  --capture <folder> <vfs path>...   headless: write a contact sheet PNG of every asset into the folder and exit\n"
        "  --tile <pixels>                    the size of one view of a contact sheet (default " << DefaultTileSize << ")\n"
        "  --thumbnail-cache <folder>         where the thumbnails of the content browser are kept\n"
        "  --offscreen                        draw without a window on the screen (always so with --capture and no display)\n"
        "  --ingest <path>                    add a folder or archive to the primary store after the start; repeat for more\n"
        "  --search <text>                    search the primary store for the text once the ingests are done\n"
        "  --screenshot <file.png>            write the window to the file once the ingests and the search are done, then exit\n"
        "  --bootstrap <file>                 the bootstrap config (plugins) to use instead of the one next to the executable\n"
        "  --config <file>                    the settings file to use instead of the settings of the user\n"
        "  --log <file>                       also write the log to this file\n"
        "  --help                             this text\n"
        "Examples:\n"
        "  " << AppName << " ~/Downloads/Shops.zip                 browse a pack as a folder\n"
        "  " << AppName << " ~/Assets/Town ~/Assets/Props.zip     open a folder and a pack\n"
        "  " << AppName << " ~/Assets/Town/House.fbx             show a model (its folder is mounted)\n"
        "  " << AppName << " --mount town=~/Assets/Town --open town/House.fbx" );
}

/*-------------------------------------------------------------------------*/

bool
CViewerApp::ParseCommandLine( void )
{GUCEF_TRACE;

    for ( int i=1; i<m_argc; ++i )
    {
        const CORE::CString arg( m_argv[ i ] );
        const bool hasValue = i+1 < m_argc;
        if ( "--help" == arg || "-h" == arg )
            return false;
        else if ( "--offscreen" == arg )
            m_offscreen = true;
        else if ( "--mount" == arg && hasValue )
            m_mountSpecs.push_back( m_argv[ ++i ] );
        else if ( "--open" == arg && hasValue )
            m_openPath = m_argv[ ++i ];
        else if ( "--ingest" == arg && hasValue )
            m_ingestPaths.push_back( m_argv[ ++i ] );
        else if ( "--search" == arg && hasValue )
            m_searchText = m_argv[ ++i ];
        else if ( "--screenshot" == arg && hasValue )
            m_screenshotFile = m_argv[ ++i ];
        else if ( "--renderer" == arg && hasValue )
            m_renderer = m_argv[ ++i ];
        else if ( "--thumbnail-cache" == arg && hasValue )
            m_thumbnailCacheFolder = m_argv[ ++i ];
        else if ( "--tile" == arg && hasValue )
            m_tileSize = CORE::StringToUInt32( m_argv[ ++i ], DefaultTileSize );
        else if ( "--bootstrap" == arg && hasValue )
            m_bootstrapConfigFile = m_argv[ ++i ];
        else if ( "--config" == arg && hasValue )
            m_configFile = m_argv[ ++i ];
        else if ( "--log" == arg && hasValue )
            m_logFile = m_argv[ ++i ];
        else if ( arg.Length() > 0 && '-' != arg[ 0 ] )
            m_openLocations.push_back( arg );
        else if ( "--capture" == arg && hasValue )
        {
            // the folder, then the assets up to the next option
            m_capturing = true;
            m_captureFolder = m_argv[ ++i ];
            while ( i+1 < m_argc && 0 != strncmp( m_argv[ i+1 ], "--", 2 ) )
                m_captureAssets.push_back( m_argv[ ++i ] );
        }
        else
        {
            GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": Unknown option or option without its value: " << arg );
            return false;
        }
    }
    if ( m_capturing && m_captureAssets.empty() )
    {
        GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": --capture needs a folder and at least one asset" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CViewerApp::RegisterEventHandlers( void )
{GUCEF_TRACE;

    CORE::CGUCEFApplication& application = CORE::CCoreGlobal::Instance()->GetApplication();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    m_workStartTrigger.SubscribeAndAddEventToTriggerCriterea( &application, CORE::CGUCEFApplication::FirstCycleEvent );
    m_workStartTrigger.SubscribeAndAddEventToTriggerCriterea( &vfs, VFS::CVFS::VfsInitializationCompletedEvent );

    TEventCallback callback( this, &CViewerApp::OnWorkStartTrigger );
    SubscribeTo( &m_workStartTrigger, CORE::CEventAggregateTrigger::AggregateTriggerEvent, callback );
}

/*-------------------------------------------------------------------------*/

void
CViewerApp::ApplyLogSettings( void )
{GUCEF_TRACE;

    GUI::CGeneralSettingsForm::ApplyGeneralSettings( m_generalSettings, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& CViewerApp::GetBootstrapConfigFile( void ) const { return m_bootstrapConfigFile; }
const CORE::CString& CViewerApp::GetConfigFile( void ) const { return m_configFile; }
Int32 CViewerApp::GetMinimalLogLevel( void ) const { return m_minimalLogLevel; }
const CORE::CString& CViewerApp::GetLogFile( void ) const { return m_logFile; }
Int32 CViewerApp::GetExitCode( void ) const { return m_exitCode; }
bool CViewerApp::IsCapturing( void ) const { return m_capturing; }
bool CViewerApp::IsFinished( void ) const { return m_stopRequested; }
const CORE::CString::StringVector& CViewerApp::GetMountSpecs( void ) const { return m_mountSpecs; }
const CORE::CString::StringVector& CViewerApp::GetCaptureAssets( void ) const { return m_captureAssets; }
const CORE::CString::StringVector& CViewerApp::GetOpenLocations( void ) const { return m_openLocations; }
GUI::CRecentLocations& CViewerApp::GetRecentLocations( void ) { return m_recentLocations; }
const CORE::CString& CViewerApp::GetCaptureFolder( void ) const { return m_captureFolder; }
const CORE::CString& CViewerApp::GetOpenPath( void ) const { return m_openPath; }
const CORE::CString& CViewerApp::GetRenderer( void ) const { return m_renderer; }
const CORE::CString& CViewerApp::GetThumbnailCacheFolder( void ) const { return m_thumbnailCacheFolder; }
const CORE::CString::StringVector& CViewerApp::GetIngestPaths( void ) const { return m_ingestPaths; }
const CORE::CString& CViewerApp::GetSearchText( void ) const { return m_searchText; }
const CORE::CString& CViewerApp::GetScreenshotFile( void ) const { return m_screenshotFile; }

/*-------------------------------------------------------------------------*/

bool
CViewerApp::SaveConfig( CORE::CDataNode& globalConfig ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = globalConfig.Structure( AppName, '/' );
    section->SetAttribute( "thumbnailCache", m_settingsThumbnailCache );
    section->SetAttribute( "captureTileSize", m_settingsTileSize );
    m_recentLocations.Save( *section );
    CORE::CDataNode* appArgs = globalConfig.Structure( "Main/AppArgs", '/' );
    appArgs->SetAttribute( "minimalLogLevel", m_minimalLogLevel );
    appArgs->SetAttribute( "logFile", m_settingsLogFile );
    return m_videoSettings.SaveConfig( globalConfig ) && m_generalSettings.SaveConfig( globalConfig ) && m_sourcesSettings.Save( globalConfig );
}

/*-------------------------------------------------------------------------*/

bool
CViewerApp::LoadConfig( const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    // the command line goes over the settings: only what it did not give is taken from them
    const CORE::CDataNode* section = globalConfig.FindChild( AppName );
    if ( GUCEF_NULL != section )
    {
        m_settingsThumbnailCache = section->GetAttributeValueOrChildValueByName( "thumbnailCache" ).AsString( CORE::CString::Empty, true );
        m_settingsTileSize = section->GetAttributeValueOrChildValueByName( "captureTileSize" ).AsUInt32( DefaultTileSize, true );
        if ( m_thumbnailCacheFolder.IsNULLOrEmpty() )
            m_thumbnailCacheFolder = m_settingsThumbnailCache;
        if ( DefaultTileSize == m_tileSize )
            m_tileSize = m_settingsTileSize;
        m_recentLocations.Load( *section );
    }
    const CORE::CDataNode* appArgs = globalConfig.Search( "Main/AppArgs", '/', true, true );
    if ( GUCEF_NULL != appArgs )
    {
        m_minimalLogLevel = appArgs->GetAttributeValueOrChildValueByName( "minimalLogLevel" ).AsInt32( m_minimalLogLevel, true );
        m_settingsLogFile = appArgs->GetAttributeValueOrChildValueByName( "logFile" ).AsString( CORE::CString::Empty, true );
        if ( m_logFile.IsNULLOrEmpty() )
            m_logFile = m_settingsLogFile;
    }
    m_videoSettings.LoadConfig( globalConfig );
    m_generalSettings.LoadConfig( globalConfig );
    if ( !m_sourcesSettings.Load( globalConfig ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The settings of the asset sources cannot be read completely, defaults are used for what is wrong" );
    m_credentials.ApplySettings( m_sourcesSettings );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CViewerApp::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

void
CViewerApp::OnWorkStartTrigger( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_started )
        return;
    m_started = true;

    TEventCallback callback( this, &CViewerApp::OnPulse );
    SubscribeTo( CORE::CCoreGlobal::Instance()->GetPulseGenerator().GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, callback );
    Start();
}

/*-------------------------------------------------------------------------*/

void
CViewerApp::Start( void )
{GUCEF_TRACE;

    if ( m_mountSpecs.empty() && m_capturing )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": There is nothing mounted, give --mount <mountPoint>=<folder or archive>" );
    }
    else
    if ( m_mountSpecs.empty() && m_openLocations.empty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, AppName << ": Nothing is open: use the buttons of the Content tab, Ctrl+O, or give a path on the command line" );
    }
    if ( m_capturing && !m_openLocations.empty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The paths on the command line are for the viewer with a window; --capture takes its mounts from --mount" );
    }
    m_mounter.MountAll( m_mountSpecs );
    if ( m_capturing && 0 == m_mounter.GetMountCount() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": Nothing could be mounted, there is nothing to capture" );
        Stop( 4 );
        return;
    }
    if ( !CreateViewerWindow() )
    {
        Stop( 2 );
        return;
    }
    if ( m_capturing ? !StartCapture() : !StartInteractive() )
        Stop( m_capturing ? 5 : 3 );
}

/*-------------------------------------------------------------------------*/

bool
CViewerApp::CreateViewerWindow( void )
{GUCEF_TRACE;

    GUI::TWindowManagerBackendPtr backend = GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( WindowBackendName );
    if ( backend.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": There is no window backend " << WindowBackendName << " (is the plugin guidriverOgre loaded?)" );
        return false;
    }

    // Ogre's own headless surface where there is no display to make a window on, and where it was asked for
    const bool offscreen = m_offscreen || ( m_capturing && !HasDisplay() );
    CORE::CValueList params;
    if ( offscreen )
        params.Set( GUIDRIVEROGRE::COgreWindowContext::PlatformBackendParameterName, GUIDRIVEROGRE::COgreWindowContext::OffscreenPlatformBackend );

    GUI::CVideoSettings windowSettings( m_videoSettings );
    if ( !m_renderer.IsNULLOrEmpty() )
        windowSettings.SetRenderer( m_renderer );
    if ( m_capturing )
    {
        // the window is only the place that Ogre draws in
        windowSettings.SetResolution( 320, 240, 32 );
        windowSettings.SetVSyncState( false );
    }
    if ( offscreen )
    {
        windowSettings.SetFrequency( 0 );
        if ( !windowSettings.GetRenderer().IsNULLOrEmpty() && !windowSettings.GetRenderer().Equals( "OpenGL", false ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": Without a display only the renderer OpenGL can draw, " << windowSettings.GetRenderer() << " needs a window" );
    }
    m_window = backend->CreateWindowContext( WindowTitle, windowSettings, &params );
    if ( m_window.IsNULL() || GUIDRIVEROGRE::COgreWindowContext::ClassTypeName != m_window->GetClassTypeName() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The window could not be made" );
        return false;
    }
    m_ogreWindow = static_cast< GUIDRIVEROGRE::COgreWindowContext* >( m_window.GetPointerAlways() );

    CORE::CString::StringVector renderers;
    backend->GetAvailableRenderers( renderers );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": Renderers " << CORE::CString().Combine( renderers, ',' ) << ", asked for "
        << ( windowSettings.GetRenderer().IsNULLOrEmpty() ? CORE::CString( "automatic" ) : windowSettings.GetRenderer() ) << ", drawing with " << m_window->GetProperty( "RENDERER" ) );

    TEventCallback callback( this, &CViewerApp::OnCloseRequest );
    SubscribeTo( m_window.GetPointerAlways(), GUI::CWindowContext::WindowContextCloseRequestEvent, callback );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CViewerApp::StartCapture( void )
{GUCEF_TRACE;

    m_capture = GUCEF_NEW CAssetCapture( m_ogreWindow );
    if ( !m_capture->Initialize( m_tileSize ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The render path of the capture cannot be made" );
        return false;
    }
    const CORE::CString folder = CORE::RelativePath( m_captureFolder );
    if ( !CORE::DirExists( folder ) && !CORE::CreateDirs( folder ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The folder " << folder << " cannot be made" );
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, AppName << ": Capturing " << (UInt32) m_captureAssets.size() << " assets to " << folder << ", views of " << m_tileSize << " pixels" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CViewerApp::CaptureNext( void )
{GUCEF_TRACE;

    if ( m_nextCapture >= (UInt32) m_captureAssets.size() )
    {
        Stop( 0 == m_captureFailures ? 0 : 3 );
        return;
    }
    const CORE::CString asset = m_captureAssets[ m_nextCapture++ ];
    IMAGE::TPixelMapPtr sheet;
    bool done = false;
    if ( m_capture->CaptureContactSheet( m_mounter, asset, sheet ) )
    {
        const CORE::CString file = CORE::CombinePath( CORE::RelativePath( m_captureFolder ), CAssetCapture::MakeSheetFileName( asset ) );
        done = CAssetCapture::WritePng( sheet, file );
        if ( done )
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": Wrote " << file << " (" << sheet->GetWidthInPixels() << "x" << sheet->GetHeightInPixels() << ") for " << asset );
    }
    if ( !done )
    {
        ++m_captureFailures;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": No contact sheet for " << asset );
    }
}

/*-------------------------------------------------------------------------*/

bool
CViewerApp::StartInteractive( void )
{GUCEF_TRACE;

    m_gui = GUI::CGuiGlobal::Instance()->GetGuiManager().CreateGUIContext( GuiDriverName, m_window );
    GUI::CGUIContext* context = m_gui.IsNULL() ? GUCEF_NULL : m_gui->AsGUIContext();
    if ( GUCEF_NULL == context )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": There is no GUI driver " << GuiDriverName << " (is the plugin guidriverImGuiOgre loaded?)" );
        return false;
    }

    // the forms follow the real resolution of the window
    context->SetLogicalResolution( 0, 0 );
    if ( !GUI::CGuiGlobal::Instance()->MountData() || !context->RegisterFont( FontName, GUI::CGuiGlobal::DefaultUiFontPath, FontPixelSize ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The font of the forms cannot be made (is the plugin fontpluginFREETYPE loaded?)" );
    CORE::CDataNode theme;
    GUI::BuildDarkTheme( theme, FontName );
    context->SetTheme( theme );

    // the thumbnails are made by a render path of their own, the size of a thumbnail tile
    m_capture = GUCEF_NEW CAssetCapture( m_ogreWindow );
    if ( !m_capture->Initialize( ThumbnailTileSize ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The render path of the thumbnails cannot be made, the content browser shows no thumbnails" );
        GUCEF_DELETE m_capture;
        m_capture = GUCEF_NULL;
    }

    CORE::CString cache = m_thumbnailCacheFolder;
    if ( cache.IsNULLOrEmpty() )
        cache = "$CACHEDIR$/AssetViewer/thumbnails";
    m_shell = GUCEF_NEW CViewerShell( m_mounter );
    if ( !m_shell->Initialize( *context, m_capture, cache ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The forms of the viewer cannot be made" );
        return false;
    }
    if ( !CreateInput() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The window has no input, the viewer cannot be used" );
    m_shell->SetCredentials( &m_credentials );
    m_shell->SetAuthManager( &AUTH::CAuthGlobal::Instance()->GetManager() );
    if ( !m_shell->InitializeSources( m_sourcesSettings ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The primary store is not available, the asset sources show it" );

    // the places of the command line are opened as the dialogs do, then the asset of --open
    m_shell->SetRecentLocations( &m_recentLocations );
    TEventCallback recentCallback( this, &CViewerApp::OnRecentLocationsChanged );
    SubscribeTo( &m_recentLocations, GUI::CRecentLocations::ChangedEvent, recentCallback );
    for ( size_t i=0; i<m_openLocations.size(); ++i )
        m_shell->OpenLocation( m_openLocations[ i ] );
    if ( !m_openPath.IsNULLOrEmpty() )
        m_shell->OpenAsset( m_openPath );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, AppName << ": Started, drawing with " << m_window->GetProperty( "RENDERER" ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CViewerApp::CreateInput( void )
{GUCEF_TRACE;

    GUI::CGUIContext* context = m_gui.IsNULL() ? GUCEF_NULL : m_gui->AsGUIContext();
    const CORE::CString driverName = PlatformInputDriverName();
    if ( GUCEF_NULL == context || driverName.IsNULLOrEmpty() || m_offscreen )
        return false;

    // the input of the platform window that the Ogre window forwards its native handles of
    CORE::CValueList params;
    params.Set( "WINDOW", m_window->GetProperty( "WINDOW" ) );
    m_input = INPUT::CInputGlobal::Instance()->GetInputController().CreateContext( driverName, params );
    if ( GUCEF_NULL == m_input )
        return false;
    m_inputAdapter = GUCEF_NEW GUI::CGUIInputAdapter( *context, m_input->GetID() );
    m_inputAdapter->Attach();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CViewerApp::KeepRecentLocations( void )
{GUCEF_TRACE;

    // only the list of recent places is written to the settings file of the user: the rest of the file stays as it is
    CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
    const CORE::CString file = configStore.GetConsolidatedConfigFile();
    if ( file.IsNULLOrEmpty() )
        return;
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( configStore.GetConsolidatedConfigFileCodecType(), codec, false ) || codec.IsNULL() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, AppName << ": There is no codec to keep the recent places in the settings with" );
        return;
    }
    CORE::CDataNode settings;
    if ( !codec->BuildDataTree( &settings, file ) )
        settings.SetName( AppName );
    CORE::CDataNode* section = settings.Structure( AppName, '/' );
    if ( GUCEF_NULL == section || !m_recentLocations.Save( *section ) || !codec->StoreDataTree( &settings, file ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, AppName << ": The recent places cannot be kept in the settings file " << file );
    }
}

/*-------------------------------------------------------------------------*/

void
CViewerApp::OnRecentLocationsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // kept in the settings of the user from the next pulse on
    m_recentDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CViewerApp::OnCloseRequest( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Stop( 0 );
}

/*-------------------------------------------------------------------------*/

void
CViewerApp::Stop( Int32 exitCode )
{GUCEF_TRACE;

    // the program sees it and takes the viewer down outside of the pulses (see Shutdown())
    if ( m_stopRequested )
        return;
    m_stopRequested = true;
    m_exitCode = exitCode;
}

/*-------------------------------------------------------------------------*/

void
CViewerApp::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a stop is the program's to carry out, outside of this notification (see Shutdown())
    if ( m_stopRequested )
        return;
    if ( m_recentDirty )
    {
        m_recentDirty = false;
        KeepRecentLocations();
    }
    if ( m_capturing && GUCEF_NULL != m_capture )
        CaptureNext();
    else
    if ( GUCEF_NULL != m_shell )
    {
        m_shell->Update( ThumbnailBudgetInMs );
        StepScript();
    }
}

/*-------------------------------------------------------------------------*/

void
CViewerApp::StepScript( void )
{GUCEF_TRACE;

    // the steps of --ingest, --search and --screenshot: ingest, search, let the forms settle, write the window
    CAssetSourcesPane* sources = m_shell->GetSources();
    if ( GUCEF_NULL == sources || ( m_ingestPaths.empty() && m_searchText.IsNULLOrEmpty() && m_screenshotFile.IsNULLOrEmpty() ) )
        return;
    switch ( m_scriptStage )
    {
        case 0:
        {
            if ( !m_ingestPaths.empty() || !m_searchText.IsNULLOrEmpty() )
                m_shell->ShowLeftTab( 2 );
            for ( size_t i=0; i<m_ingestPaths.size(); ++i )
                if ( !sources->AddDiskPack( m_ingestPaths[ i ] ) )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": " << m_ingestPaths[ i ] << " cannot be added to the primary store" );
            m_scriptStage = 1;
            break;
        }
        case 1:
        {
            // the ingests have ended when the transfer queue holds an ended ingest for each of them
            GUI::CITransferQueueModel::TEntryVector entries;
            sources->GetTransferModel()->GetEntries( entries );
            UInt32 ended = 0;
            for ( size_t i=0; i<entries.size(); ++i )
                if ( entries[ i ].id.HasSubstr( CAssetStoreTransferModel::IngestIdPrefix ) == 0 && entries[ i ].IsEnded() )
                    ++ended;
            if ( ended >= sources->GetStartedIngestCount() || ++m_scriptFrames > ScriptWaitPulses )
            {
                m_scriptFrames = 0;
                sources->GetSearchForm()->SetText( m_searchText );
                sources->GetSearchForm()->Search();
                m_scriptStage = 2;
            }
            break;
        }
        case 2:
        {
            if ( !sources->GetSearchForm()->IsSearching() || ++m_scriptFrames > ScriptWaitPulses )
            {
                m_scriptStage = 3;
                m_scriptFrames = 0;
            }
            break;
        }
        case 3:
        {
            // a few frames for the forms to lay out and draw what arrived
            if ( ++m_scriptFrames < 20 )
                break;
            if ( !m_screenshotFile.IsNULLOrEmpty() && GUCEF_NULL != m_ogreWindow )
            {
                IMAGE::TPixelMapPtr pixels;
                if ( m_ogreWindow->RenderFrame( &pixels ) && !pixels.IsNULL() && CAssetCapture::WritePng( pixels, CORE::RelativePath( m_screenshotFile ) ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": Wrote the window to " << m_screenshotFile << " (" << pixels->GetWidthInPixels() << "x" << pixels->GetHeightInPixels() << ")" );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The window cannot be written to " << m_screenshotFile );
                    Stop( 6 );
                    return;
                }
            }
            if ( !m_screenshotFile.IsNULLOrEmpty() )
                Stop( 0 );
            m_scriptStage = 4;
            break;
        }
        default:
        {
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CViewerApp::Shutdown( void )
{GUCEF_TRACE;

    // the order of an Ogre application: the input, the viewer and its GUI, the render paths, then the window; the Root goes
    // when the Ogre support library unloads
    if ( GUCEF_NULL != m_inputAdapter )
    {
        m_inputAdapter->Detach();
        GUCEF_DELETE m_inputAdapter;
        m_inputAdapter = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_input )
    {
        INPUT::CInputGlobal::Instance()->GetInputController().DestroyContext( m_input );
        m_input = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_shell )
    {
        m_shell->Shutdown();
        GUCEF_DELETE m_shell;
        m_shell = GUCEF_NULL;
    }
    m_gui.Unlink();
    if ( GUCEF_NULL != m_capture )
    {
        m_capture->Shutdown();
        GUCEF_DELETE m_capture;
        m_capture = GUCEF_NULL;
    }
    if ( !m_window.IsNULL() )
    {
        UnsubscribeFrom( *m_window );
        GUI::TWindowManagerBackendPtr backend = GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( WindowBackendName );
        if ( !backend.IsNULL() )
            backend->DestroyWindowContext( m_window );
        m_window.Unlink();
    }
    m_ogreWindow = GUCEF_NULL;
    m_mounter.UnmountAll();
    // the saved passwords are locked again when the viewer ends
    m_credentials.Shutdown();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
