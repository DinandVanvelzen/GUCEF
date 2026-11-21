/*
 *  AssetWorkbenchLib: the application of the workbench: command line, settings, mounts, the window and the capture or the interactive session
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

#include "AssetWorkbenchLib_CWorkbenchApp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CWorkbenchApp::ClassTypeName = "GUCEF::ASSETWORKBENCH::CWorkbenchApp";
const CORE::CString CWorkbenchApp::AppName = "AssetWorkbench";
const CORE::CString CWorkbenchApp::WindowTitle = "Asset workbench";

namespace {

const char* const WindowBackendName = "Ogre";                   /**< the window backend of guidriverOgre */
const char* const GuiDriverName = "ImGuiOgre";                  /**< the GUI driver of guidriverImGuiOgre */
const char* const FontName = "WorkbenchFont";
const UInt32 FontPixelSize = 16;
const UInt32 DefaultTileSize = 256;

// a change of the layout is kept when it did not change for this long
const UInt64 LayoutQuietTimeInMs = 600;
const UInt32 ThumbnailTileSize = 128;
const UInt32 ThumbnailBudgetInMs = 6;
const UInt32 ScriptWaitPulses = 3000;            /**< how long --ingest and --search wait for the store before the script goes on */
const UInt32 OpenWaitPulses = 60000;           /**< how long a screenshot waits for what is opening (a conversion of a pack can take minutes) */

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

CWorkbenchApp::CWorkbenchApp( int argc, char** argv )
    : CORE::CGloballyConfigurable( false )
    , CORE::CObserver()
    , m_argc( argc )
    , m_argv( argv )
    , m_bootstrapConfigFile()
    , m_configFile()
    , m_logFile()
    , m_renderer()
    , m_openPath()
    , m_screenshotDelayInMs( 0 )
    , m_screenshotDelayStart( 0 )
    , m_openWaitPulses( 0 )
    , m_presentationKind()
    , m_presentationOverride( -1 )
    , m_resourceViewOverride( -1 )
    , m_resourcePath()
    , m_captureFolder()
    , m_screenshotFile()
    , m_searchText()
    , m_openResult( false )
    , m_resultThumbnails( false )
    , m_downloadFolder()
    , m_resultActionsDone( false )
    , m_storeEndpoint()
    , m_storePairCode()
    , m_storeScriptStarted( false )
    , m_storeScriptDone( false )
    , m_storeScriptFrames( 0 )
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
    , m_audioSettings()
    , m_preferences()
    , m_sourcesSettings()
    , m_recentLocations()
    , m_layout()
    , m_layoutChangedTick( 0 )
    , m_credentials()
    , m_workStartTrigger()
    , m_mounter()
    , m_window()
    , m_gui()
    , m_ogreWindow( GUCEF_NULL )
    , m_capture( GUCEF_NULL )
    , m_shell( GUCEF_NULL )
    , m_layer( GUCEF_NULL )
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
    , m_layoutDirty( false )
    , m_settingsDirty( false )
{GUCEF_TRACE;

    m_videoSettings.SetResolution( 1600, 900, 32 );
    m_videoSettings.SetFullscreenState( false );
    m_videoSettings.SetFrequency( 60 );

    // the developer console is a tool of the workbench: it is on until the owner switches it off
    m_generalSettings.SetDeveloperConsole( true );

    // before the plugins of the stores are loaded: they take the stores of the sessions and of the saved passwords when they make their provider
    m_credentials.Install();
}

/*-------------------------------------------------------------------------*/

CWorkbenchApp::~CWorkbenchApp()
{GUCEF_TRACE;

    Shutdown();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchApp::LogUsage( void )
{GUCEF_TRACE;

    GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, "Usage: " << AppName << " [options] [<path>...]\n"
        "  <path>                             a folder, an archive (zip, 7z, tar, unitypackage, ...) or a file (a model, a picture) to open at the\n"
        "                                     start, as \"Open folder\", \"Open archive or pack\" and \"Open file\" do; repeat for more. Without any\n"
        "                                     path the workbench shows the files of this computer: use the buttons of the Content tab or Ctrl+O\n"
        "  --mount <mountPoint>=<source>      mount a folder, an archive (zip, unitypackage, ...) or the VFS path of an archive in\n"
        "                                     an earlier mount (a package inside a zip); repeat for more mounts\n"
        "  --open <vfs path>                  show this asset at the start\n"
        "  --presentation / --no-presentation  show / do not show the presentation of a pack that is opened, for this run (the setting says otherwise\n"
        "                                     by default: on); --open of a folder of the VFS shows its presentation\n"
        "  --resource-view / --files-view     a pack that is opened starts in the resource view (its resources by function) / in its files, for this run\n"
        "                                     (the setting says otherwise by default: the resource view; not kept)\n"
        "  --resource-path <path>             the folder of the resource view to show first, such as /static-models/props (with --open of a pack)\n"
        "  --presentation-kind <id>           the presentation of the packs of this run: portrait, effect, scene, grid, animation, material, images\n"
        "                                     or summary, instead of the default of the pack (not kept)\n"
        "  --view <name>                      the view preset of the 3D view: Frame, Front, Side, Top, Three quarter or Four views (with --capture:\n"
        "                                     Four views writes the sheet of the four view layout instead of the contact sheet)\n"
        "  --layout <name>                    the layout of the window at the start: Browse, Review or Preview\n"
        "  --renderer <name>                  draw with this renderer (OpenGL, Vulkan) instead of the one of the video settings\n"
        "  --capture <folder> <vfs path>...   headless: write a contact sheet PNG of every asset into the folder and exit\n"
        "  --tile <pixels>                    the size of one view of a contact sheet (default " << DefaultTileSize << ")\n"
        "  --thumbnail-cache <folder>         where the thumbnails of the content browser are kept\n"
        "  --offscreen                        draw without a window on the screen (always so with --capture and no display)\n"
        "  --ingest <path>                    add a folder or archive to the primary store after the start; repeat for more\n"
        "  --search <text>                    search the primary store for the text once the ingests are done\n"
        "  --open-result                      with --search: open the first result of the search (a remote store fetches its files first)\n"
        "  --result-thumbnails                with --search: show the results as thumbnails\n"
        "  --download-result <folder>         with --search: copy the files of the first result to the folder (a remote store)\n"
        "  --screenshot-after <ms>            with --screenshot: wait this long after what is opening is shown (an effect plays on)\n"
        "  --store-endpoint <host:port>       connect to the remote asset store service at this address (this computer has to be paired); the address of the\n"
        "                                     settings otherwise (worker.lan:47400)\n"
        "  --store-pair-code <code>           pair this computer with the service with the one-time code of a pairing window of the server, then connect\n"
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
CWorkbenchApp::ParseCommandLine( void )
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
        else if ( "--view" == arg && hasValue )
            m_viewName = m_argv[ ++i ];
        else if ( "--presentation-kind" == arg && hasValue )
            m_presentationKind = m_argv[ ++i ];
        else if ( "--no-presentation" == arg )
            m_presentationOverride = 0;
        else if ( "--resource-path" == arg && hasValue )
            m_resourcePath = m_argv[ ++i ];
        else if ( "--files-view" == arg )
            m_resourceViewOverride = 0;
        else if ( "--resource-view" == arg )
            m_resourceViewOverride = 1;
        else if ( "--presentation" == arg )
            m_presentationOverride = 1;
        else if ( "--layout" == arg && hasValue )
            m_layoutName = m_argv[ ++i ];
        else if ( "--ingest" == arg && hasValue )
            m_ingestPaths.push_back( m_argv[ ++i ] );
        else if ( "--search" == arg && hasValue )
            m_searchText = m_argv[ ++i ];
        else if ( "--open-result" == arg )
            m_openResult = true;
        else if ( "--result-thumbnails" == arg )
            m_resultThumbnails = true;
        else if ( "--download-result" == arg && hasValue )
            m_downloadFolder = m_argv[ ++i ];
        else if ( "--screenshot-after" == arg && hasValue )
            m_screenshotDelayInMs = CORE::StringToUInt32( m_argv[ ++i ], 0 );
        else if ( "--screenshot" == arg && hasValue )
            m_screenshotFile = m_argv[ ++i ];
        else if ( "--store-endpoint" == arg && hasValue )
            m_storeEndpoint = m_argv[ ++i ];
        else if ( "--store-pair-code" == arg && hasValue )
        {
            m_storePairCode.Set( CORE::CString( m_argv[ ++i ] ) );
            // the code does not stay in the argument list that other users of this computer can read
            for ( char* c = m_argv[ i ]; '\0' != *c; ++c )
                *c = 'x';
        }
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
CWorkbenchApp::RegisterEventHandlers( void )
{GUCEF_TRACE;

    CORE::CGUCEFApplication& application = CORE::CCoreGlobal::Instance()->GetApplication();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    m_workStartTrigger.SubscribeAndAddEventToTriggerCriterea( &application, CORE::CGUCEFApplication::FirstCycleEvent );
    m_workStartTrigger.SubscribeAndAddEventToTriggerCriterea( &vfs, VFS::CVFS::VfsInitializationCompletedEvent );

    TEventCallback callback( this, &CWorkbenchApp::OnWorkStartTrigger );
    SubscribeTo( &m_workStartTrigger, CORE::CEventAggregateTrigger::AggregateTriggerEvent, callback );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchApp::ApplyLogSettings( void )
{GUCEF_TRACE;

    GUI::CGeneralSettingsForm::ApplyGeneralSettings( m_generalSettings, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& CWorkbenchApp::GetBootstrapConfigFile( void ) const { return m_bootstrapConfigFile; }
const CORE::CString& CWorkbenchApp::GetConfigFile( void ) const { return m_configFile; }
Int32 CWorkbenchApp::GetMinimalLogLevel( void ) const { return m_minimalLogLevel; }
const CORE::CString& CWorkbenchApp::GetLogFile( void ) const { return m_logFile; }
Int32 CWorkbenchApp::GetExitCode( void ) const { return m_exitCode; }
bool CWorkbenchApp::IsCapturing( void ) const { return m_capturing; }
bool CWorkbenchApp::IsFinished( void ) const { return m_stopRequested; }
const CORE::CString::StringVector& CWorkbenchApp::GetMountSpecs( void ) const { return m_mountSpecs; }
const CORE::CString::StringVector& CWorkbenchApp::GetCaptureAssets( void ) const { return m_captureAssets; }
const CORE::CString::StringVector& CWorkbenchApp::GetOpenLocations( void ) const { return m_openLocations; }
GUI::CRecentLocations& CWorkbenchApp::GetRecentLocations( void ) { return m_recentLocations; }
const CORE::CString& CWorkbenchApp::GetCaptureFolder( void ) const { return m_captureFolder; }
const CORE::CString& CWorkbenchApp::GetOpenPath( void ) const { return m_openPath; }
const CORE::CString& CWorkbenchApp::GetRenderer( void ) const { return m_renderer; }
const CORE::CString& CWorkbenchApp::GetThumbnailCacheFolder( void ) const { return m_thumbnailCacheFolder; }
const CORE::CString::StringVector& CWorkbenchApp::GetIngestPaths( void ) const { return m_ingestPaths; }
const CORE::CString& CWorkbenchApp::GetSearchText( void ) const { return m_searchText; }
const CORE::CString& CWorkbenchApp::GetStoreEndpoint( void ) const { return m_storeEndpoint; }
bool CWorkbenchApp::HasStorePairCode( void ) const { return !m_storePairCode.IsEmpty(); }
const CORE::CString& CWorkbenchApp::GetScreenshotFile( void ) const { return m_screenshotFile; }
const CWorkbenchPreferences& CWorkbenchApp::GetPreferences( void ) const { return m_preferences; }
const GUI::CGeneralSettings& CWorkbenchApp::GetGeneralSettings( void ) const { return m_generalSettings; }
CWorkbenchSettingsLayer* CWorkbenchApp::GetSettingsLayer( void ) const { return m_layer; }

/*-------------------------------------------------------------------------*/

bool
CWorkbenchApp::SaveConfig( CORE::CDataNode& globalConfig ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = globalConfig.Structure( AppName, '/' );
    section->SetAttribute( "thumbnailCache", m_settingsThumbnailCache );
    section->SetAttribute( "captureTileSize", m_settingsTileSize );
    m_recentLocations.Save( *section );
    m_layout.Save( *section );
    CWorkbenchPreferences preferences( m_preferences );
    preferences.settingsVersion = CWorkbenchPreferences::CurrentSettingsVersion;
    preferences.Save( *section );
    CORE::CDataNode* appArgs = globalConfig.Structure( "Main/AppArgs", '/' );
    appArgs->SetAttribute( "minimalLogLevel", m_minimalLogLevel );
    appArgs->SetAttribute( "logFile", m_settingsLogFile );
    return m_videoSettings.SaveConfig( globalConfig ) && m_generalSettings.SaveConfig( globalConfig ) && m_audioSettings.SaveConfig( globalConfig ) && m_sourcesSettings.Save( globalConfig );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchApp::LoadConfig( const CORE::CDataNode& globalConfig )
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
        m_layout.Load( *section );
        m_preferences.Load( *section );
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
    m_audioSettings.LoadConfig( globalConfig );
    if ( m_preferences.settingsVersion < 1 )
    {
        // settings of before the developer console of the workbench: what they say about the console was never a choice
        m_generalSettings.SetDeveloperConsole( true );
    }
    if ( m_preferences.settingsVersion < 2 )
    {
        // the 60 seconds of before were the default, not a choice: the scene of a pack gets the time it needs
        m_preferences.maxSceneSeconds = CWorkbenchPreferences::DefaultMaxSceneSeconds;
    }
    m_preferences.settingsVersion = CWorkbenchPreferences::CurrentSettingsVersion;
    if ( !m_sourcesSettings.Load( globalConfig ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The settings of the asset sources cannot be read completely, defaults are used for what is wrong" );
    m_credentials.ApplySettings( m_sourcesSettings );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchApp::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchApp::OnWorkStartTrigger( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_started )
        return;
    m_started = true;

    TEventCallback callback( this, &CWorkbenchApp::OnPulse );
    SubscribeTo( CORE::CCoreGlobal::Instance()->GetPulseGenerator().GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, callback );
    Start();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchApp::Start( void )
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
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The paths on the command line are for the workbench with a window; --capture takes its mounts from --mount" );
    }
    m_mounter.MountAll( m_mountSpecs );
    if ( m_capturing && 0 == m_mounter.GetMountCount() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": Nothing could be mounted, there is nothing to capture" );
        Stop( 4 );
        return;
    }
    if ( !CreateWorkbenchWindow() )
    {
        Stop( 2 );
        return;
    }
    if ( m_capturing ? !StartCapture() : !StartInteractive() )
        Stop( m_capturing ? 5 : 3 );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchApp::CreateWorkbenchWindow( void )
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

    TEventCallback callback( this, &CWorkbenchApp::OnCloseRequest );
    SubscribeTo( m_window.GetPointerAlways(), GUI::CWindowContext::WindowContextCloseRequestEvent, callback );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchApp::StartCapture( void )
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
CWorkbenchApp::CaptureNext( void )
{GUCEF_TRACE;

    if ( m_nextCapture >= (UInt32) m_captureAssets.size() )
    {
        Stop( 0 == m_captureFailures ? 0 : 3 );
        return;
    }
    const CORE::CString asset = m_captureAssets[ m_nextCapture++ ];
    IMAGE::TPixelMapPtr sheet;
    bool done = false;
    CViewPresetTracker::EPreset viewPreset = CViewPresetTracker::PRESET_FRAME;
    const bool fourViews = !m_viewName.IsNULLOrEmpty() && CViewPresetTracker::TryParsePreset( m_viewName, viewPreset ) && CViewPresetTracker::PRESET_FOUR_VIEWS == viewPreset;
    if ( m_capture->CaptureContactSheet( m_mounter, asset, sheet, fourViews ) )
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
CWorkbenchApp::StartInteractive( void )
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
        cache = "$CACHEDIR$/AssetWorkbench/thumbnails";
    m_shell = GUCEF_NEW CWorkbenchShell( m_mounter );
    if ( !m_shell->Initialize( *context, m_capture, cache ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The forms of the workbench cannot be made" );
        return false;
    }
    if ( !CreateInput() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The window has no input, the workbench cannot be used" );
    // the layout of the last session comes back, and is kept again when the user shapes it
    m_shell->SetPreferences( &m_preferences );
    m_shell->SetPackPresentationOverride( m_presentationOverride );
    m_shell->SetResourceViewOverride( m_resourceViewOverride );
    m_shell->SetResourceViewStartPath( m_resourcePath );
    m_shell->SetPresentationKindOverride( m_presentationKind );
    TEventCallback preferencesCallback( this, &CWorkbenchApp::OnSettingsApplied );
    SubscribeTo( m_shell, CWorkbenchShell::PreferencesChangedEvent, preferencesCallback );
    m_shell->SetLayoutManager( &m_layout );
    TEventCallback layoutCallback( this, &CWorkbenchApp::OnLayoutChanged );
    SubscribeTo( &m_layout, GUI::CLayoutManager::LayoutChangedEvent, layoutCallback );
    if ( !m_layoutName.IsNULLOrEmpty() && !m_shell->ApplyLayoutPreset( m_layoutName ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": There is no layout " << m_layoutName );
    m_shell->SetCredentials( &m_credentials );
    m_shell->SetAuthManager( &AUTH::CAuthGlobal::Instance()->GetManager() );

    // the settings window, the developer console, the help and the tooltips; then the private store plugins, before the sources look for stores
    m_layer = GUCEF_NEW CWorkbenchSettingsLayer( *m_shell, m_preferences );
    m_layer->SetVideoSettings( m_videoSettings );
    m_layer->SetGeneralSettings( m_generalSettings );
    m_layer->SetAudioSettings( m_audioSettings );
    if ( !m_layer->Create() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The settings window, the console or the help cannot be made, the workbench works without them" );
    }
    else
    {
        CORE::CString::StringVector renderers;
        GUI::TWindowManagerBackendPtr backend = GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( WindowBackendName );
        if ( !backend.IsNULL() )
            backend->GetAvailableRenderers( renderers );
        m_layer->SetRenderers( renderers, m_window->GetProperty( "RENDERER" ) );
        m_layer->SetWindow( WindowBackendName, m_window );
        m_layer->ApplyGeneralSettings();
        TEventCallback settingsCallback( this, &CWorkbenchApp::OnSettingsApplied );
        SubscribeTo( m_layer, CWorkbenchSettingsLayer::SettingsAppliedEvent, settingsCallback );
    }
    m_layer->LoadPrivatePlugins();
    if ( !m_shell->InitializeSources( m_sourcesSettings ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The primary store is not available, the asset sources show it" );
    m_layer->BindSources();
    ConfigureStoreConnection();

    // the places of the command line are opened as the dialogs do, then the asset of --open
    m_shell->SetRecentLocations( &m_recentLocations );
    TEventCallback recentCallback( this, &CWorkbenchApp::OnRecentLocationsChanged );
    SubscribeTo( &m_recentLocations, GUI::CRecentLocations::ChangedEvent, recentCallback );
    for ( size_t i=0; i<m_openLocations.size(); ++i )
        m_shell->OpenLocation( m_openLocations[ i ] );
    if ( !m_openPath.IsNULLOrEmpty() )
    {
        // a pack (a folder of the VFS) shows its presentation, as the user asked for it by naming it; an asset is shown as it is
        if ( VFS::CVfsGlobal::Instance()->GetVfs().DirExists( CAssetMounter::NormalizePath( m_openPath ) ) )
            m_shell->OpenPack( m_openPath, true );
        else
            m_shell->OpenAsset( m_openPath );
    }
    if ( !m_viewName.IsNULLOrEmpty() && !m_shell->ApplyViewPreset( m_viewName ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": There is no view preset " << m_viewName );
    // what the command line named is shown at once; from here on the interface opens in the background and keeps drawing
    m_shell->SetAsyncOpenEnabled( true );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, AppName << ": Started, drawing with " << m_window->GetProperty( "RENDERER" ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchApp::CreateInput( void )
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
CWorkbenchApp::KeepRecentLocations( void )
{GUCEF_TRACE;

    // only the list of recent places and the layout of the window are written to the settings file of the user: the rest of the file stays as it is
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
    if ( GUCEF_NULL != section )
    {
        // what the settings window changes is kept with them: the video, general and audio settings, the stores and the preferences
        m_preferences.settingsVersion = CWorkbenchPreferences::CurrentSettingsVersion;
        m_preferences.Save( *section );
        if ( m_settingsDirty )
        {
            m_videoSettings.SaveConfig( settings );
            m_generalSettings.SaveConfig( settings );
            m_audioSettings.SaveConfig( settings );
        }
    }
    if ( GUCEF_NULL == section || !m_recentLocations.Save( *section ) || !m_layout.Save( *section ) || !codec->StoreDataTree( &settings, file ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, AppName << ": The recent places, the layout and the settings cannot be kept in the settings file " << file );
    }
    m_settingsDirty = false;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchApp::OnRecentLocationsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // kept in the settings of the user from the next pulse on
    m_recentDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchApp::OnLayoutChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // kept in the settings of the user once the user let go of the splitter: not for every pixel of a drag
    m_layoutDirty = true;
    m_layoutChangedTick = MT::PrecisionTickCount();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchApp::OnSettingsApplied( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_layer )
        return;
    // what the window applied is the application's now, and is kept in the settings of the user from the next pulse on
    m_videoSettings = m_layer->GetVideoSettings();
    m_generalSettings = m_layer->GetGeneralSettings();
    m_audioSettings = m_layer->GetAudioSettings();
    m_settingsDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchApp::OnCloseRequest( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Stop( 0 );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchApp::Stop( Int32 exitCode )
{GUCEF_TRACE;

    // the program sees it and takes the workbench down outside of the pulses (see Shutdown())
    if ( m_stopRequested )
        return;
    m_stopRequested = true;
    m_exitCode = exitCode;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchApp::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a stop is the program's to carry out, outside of this notification (see Shutdown())
    if ( m_stopRequested )
        return;
    const UInt64 frequency = MT::PrecisionTimerResolution();
    if ( m_recentDirty || m_settingsDirty || ( m_layoutDirty && ( 0 == frequency || ( MT::PrecisionTickCount() - m_layoutChangedTick ) * 1000 / frequency >= LayoutQuietTimeInMs ) ) )
    {
        m_recentDirty = false;
        m_layoutDirty = false;
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
CWorkbenchApp::ConfigureStoreConnection( void )
{GUCEF_TRACE;

    CWorkbenchStoreConnection* connection = m_shell->GetStoreConnection();
    if ( GUCEF_NULL == connection )
        return;
    // the settings of the owner (worker.lan:47400 by default), then what the command line says for this run
    CWorkbenchStoreConnection::SSettings settings = connection->GetSettings();
    settings.endpoint = m_storeEndpoint.IsNULLOrEmpty() ? m_preferences.remoteEndpoint : m_storeEndpoint;
    settings.pairingPort = m_preferences.remotePairingPort;
    settings.lanDiscovery = m_preferences.remoteLanDiscovery;
    connection->SetSettings( settings );

    if ( !m_storePairCode.IsEmpty() )
    {
        connection->Pair( m_storePairCode );
        m_storePairCode.Clear();
        m_storeScriptStarted = true;
    }
    else
    if ( !m_storeEndpoint.IsNULLOrEmpty() || m_preferences.remoteConnectAtStart )
    {
        if ( connection->IsPaired() )
            m_storeScriptStarted = connection->Connect();
        else
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": This computer is not paired with a store service, so it does not connect: use --store-pair-code or Settings, Remote store" );
    }
    if ( !m_storeScriptStarted && m_preferences.remoteLanDiscovery )
        connection->Discover();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchApp::StepStoreConnection( void )
{GUCEF_TRACE;

    // the script of the command line goes on once the connection to the remote store was made or has failed
    if ( !m_storeScriptStarted || m_storeScriptDone )
        return true;
    CWorkbenchStoreConnection* connection = m_shell->GetStoreConnection();
    if ( GUCEF_NULL == connection )
    {
        m_storeScriptDone = true;
        return true;
    }
    const CWorkbenchStoreConnection::TState state = connection->GetState();
    if ( CWorkbenchStoreConnection::STATE_CONNECTED == state )
    {
        // a few frames for the sources tab to take the store and ask it
        if ( ++m_storeScriptFrames < 30 )
            return false;
        m_storeScriptDone = true;
        return true;
    }
    if ( CWorkbenchStoreConnection::STATE_DISCONNECTED == state || ++m_storeScriptFrames > ScriptWaitPulses )
    {
        if ( !connection->GetErrorText().IsNULLOrEmpty() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The remote store is not connected: " << connection->GetErrorText() );
        m_storeScriptDone = true;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchApp::StepScript( void )
{GUCEF_TRACE;

    // the steps of --ingest, --search and --screenshot: ingest, search, let the forms settle, write the window
    if ( !StepStoreConnection() )
        return;
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
            if ( !m_resultActionsDone )
            {
                // the actions on the results of the search: thumbnails, open, download
                m_resultActionsDone = true;
                GUI::CAssetSearchForm* form = sources->GetSearchForm();
                if ( m_resultThumbnails )
                    form->SetThumbnailView( true );
                CORE::CString key;
                for ( UInt32 i=0; i<form->GetShownRowCount() && key.IsNULLOrEmpty(); ++i )
                    if ( 0 != form->GetShownRow( i ).key.HasSubstr( "pack:" ) )
                        key = form->GetShownRow( i ).key;
                if ( !key.IsNULLOrEmpty() && ( m_openResult || !m_downloadFolder.IsNULLOrEmpty() ) )
                {
                    form->SelectResult( key );
                    if ( m_openResult )
                        sources->OpenResult( key );
                    CORE::CString message;
                    if ( !m_downloadFolder.IsNULLOrEmpty() && !sources->DownloadSelected( m_downloadFolder, message ) )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": The download cannot start: " << message );
                }
            }
            // what is opening (a pack with its presentation, the files of a remote result) is waited for, as long as the other steps wait
            if ( ( m_shell->IsOpening() || sources->IsRemoteOpenBusy() || ( GUCEF_NULL != sources->GetFetchJobs() && sources->GetFetchJobs()->GetActiveCount() > 0 ) ) && ++m_openWaitPulses < OpenWaitPulses )
            {
                m_scriptFrames = 0;
                break;
            }
            // a few frames for the forms to lay out and draw what arrived
            if ( ++m_scriptFrames < 20 )
                break;
            // an effect plays on: the picture is taken this long after it is shown
            if ( m_screenshotDelayInMs > 0 )
            {
                if ( 0 == m_screenshotDelayStart )
                    m_screenshotDelayStart = MT::PrecisionTickCount();
                if ( ( MT::PrecisionTickCount() - m_screenshotDelayStart ) * 1000 / MT::PrecisionTimerResolution() < m_screenshotDelayInMs )
                    break;
            }
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
CWorkbenchApp::Shutdown( void )
{GUCEF_TRACE;

    // the order of an Ogre application: the input, the workbench and its GUI, the render paths, then the window; the Root goes
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
        // a change of the layout that is not kept yet is kept now, while the window is still there to read it from
        if ( m_layoutDirty || m_settingsDirty )
        {
            m_layoutDirty = false;
            KeepRecentLocations();
        }
        GUCEF_DELETE m_layer;
        m_layer = GUCEF_NULL;
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
    // the saved passwords are locked again when the workbench ends
    m_credentials.Shutdown();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
