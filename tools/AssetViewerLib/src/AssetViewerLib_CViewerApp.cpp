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
    , m_thumbnailCacheFolder()
    , m_mountSpecs()
    , m_captureAssets()
    , m_minimalLogLevel( CORE::LOGLEVEL_NORMAL )
    , m_tileSize( DefaultTileSize )
    , m_offscreen( false )
    , m_capturing( false )
    , m_videoSettings()
    , m_generalSettings()
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
    , m_exitCode( 0 )
    , m_started( false )
    , m_stopRequested( false )
{GUCEF_TRACE;

    m_videoSettings.SetResolution( 1600, 900, 32 );
    m_videoSettings.SetFullscreenState( false );
    m_videoSettings.SetFrequency( 60 );
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

    GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, "Usage: " << AppName << " [options]\n"
        "  --mount <mountPoint>=<source>      mount a folder, an archive (zip, unitypackage, ...) or the VFS path of an archive in\n"
        "                                     an earlier mount (a package inside a zip); repeat for more mounts\n"
        "  --open <vfs path>                  show this asset at the start\n"
        "  --renderer <name>                  draw with this renderer (OpenGL, Vulkan) instead of the one of the video settings\n"
        "  --capture <folder> <vfs path>...   headless: write a contact sheet PNG of every asset into the folder and exit\n"
        "  --tile <pixels>                    the size of one view of a contact sheet (default " << DefaultTileSize << ")\n"
        "  --thumbnail-cache <folder>         where the thumbnails of the content browser are kept\n"
        "  --offscreen                        draw without a window on the screen (always so with --capture and no display)\n"
        "  --bootstrap <file>                 the bootstrap config (plugins) to use instead of the one next to the executable\n"
        "  --config <file>                    the settings file to use instead of the settings of the user\n"
        "  --log <file>                       also write the log to this file\n"
        "  --help                             this text" );
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
const CORE::CString& CViewerApp::GetCaptureFolder( void ) const { return m_captureFolder; }
const CORE::CString& CViewerApp::GetOpenPath( void ) const { return m_openPath; }
const CORE::CString& CViewerApp::GetRenderer( void ) const { return m_renderer; }
const CORE::CString& CViewerApp::GetThumbnailCacheFolder( void ) const { return m_thumbnailCacheFolder; }

/*-------------------------------------------------------------------------*/

bool
CViewerApp::SaveConfig( CORE::CDataNode& globalConfig ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = globalConfig.Structure( AppName, '/' );
    section->SetAttribute( "thumbnailCache", m_thumbnailCacheFolder );
    section->SetAttribute( "captureTileSize", m_tileSize );
    CORE::CDataNode* appArgs = globalConfig.Structure( "Main/AppArgs", '/' );
    appArgs->SetAttribute( "minimalLogLevel", m_minimalLogLevel );
    appArgs->SetAttribute( "logFile", m_logFile );
    return m_videoSettings.SaveConfig( globalConfig ) && m_generalSettings.SaveConfig( globalConfig );
}

/*-------------------------------------------------------------------------*/

bool
CViewerApp::LoadConfig( const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    // the command line goes over the settings: only what it did not give is taken from them
    const CORE::CDataNode* section = globalConfig.FindChild( AppName );
    if ( GUCEF_NULL != section )
    {
        if ( m_thumbnailCacheFolder.IsNULLOrEmpty() )
            m_thumbnailCacheFolder = section->GetAttributeValueOrChildValueByName( "thumbnailCache" ).AsString( CORE::CString::Empty, true );
        if ( DefaultTileSize == m_tileSize )
            m_tileSize = section->GetAttributeValueOrChildValueByName( "captureTileSize" ).AsUInt32( DefaultTileSize, true );
    }
    const CORE::CDataNode* appArgs = globalConfig.Search( "Main/AppArgs", '/', true, true );
    if ( GUCEF_NULL != appArgs )
    {
        m_minimalLogLevel = appArgs->GetAttributeValueOrChildValueByName( "minimalLogLevel" ).AsInt32( m_minimalLogLevel, true );
        if ( m_logFile.IsNULLOrEmpty() )
            m_logFile = appArgs->GetAttributeValueOrChildValueByName( "logFile" ).AsString( CORE::CString::Empty, true );
    }
    m_videoSettings.LoadConfig( globalConfig );
    m_generalSettings.LoadConfig( globalConfig );
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

    if ( m_mountSpecs.empty() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": There is nothing mounted, give --mount <mountPoint>=<folder or archive>" );
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
    if ( m_capturing && GUCEF_NULL != m_capture )
        CaptureNext();
    else
    if ( GUCEF_NULL != m_shell )
        m_shell->Update( ThumbnailBudgetInMs );
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
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
