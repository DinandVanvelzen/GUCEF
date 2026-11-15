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

#ifndef ASSETVIEWERLIB_CVIEWERAPP_H
#define ASSETVIEWERLIB_CVIEWERAPP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CEVENTAGGREGATETRIGGER_H
#include "gucefCORE_CEventAggregateTrigger.h"
#define GUCEF_CORE_CEVENTAGGREGATETRIGGER_H
#endif /* GUCEF_CORE_CEVENTAGGREGATETRIGGER_H ? */

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUCEF_GUI_CGENERALSETTINGS_H
#include "gucefGUI_CGeneralSettings.h"
#define GUCEF_GUI_CGENERALSETTINGS_H
#endif /* GUCEF_GUI_CGENERALSETTINGS_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUCEF_GUI_CIGUICONTEXT_H
#include "gucefGUI_CIGUIContext.h"
#define GUCEF_GUI_CIGUICONTEXT_H
#endif /* GUCEF_GUI_CIGUICONTEXT_H ? */

#ifndef ASSETVIEWERLIB_CASSETMOUNTER_H
#include "AssetViewerLib_CAssetMounter.h"
#define ASSETVIEWERLIB_CASSETMOUNTER_H
#endif /* ASSETVIEWERLIB_CASSETMOUNTER_H ? */

#ifndef ASSETVIEWERLIB_CASSETCAPTURE_H
#include "AssetViewerLib_CAssetCapture.h"
#define ASSETVIEWERLIB_CASSETCAPTURE_H
#endif /* ASSETVIEWERLIB_CASSETCAPTURE_H ? */

#ifndef ASSETVIEWERLIB_CASSETSOURCESSETTINGS_H
#include "AssetViewerLib_CAssetSourcesSettings.h"
#define ASSETVIEWERLIB_CASSETSOURCESSETTINGS_H
#endif /* ASSETVIEWERLIB_CASSETSOURCESSETTINGS_H ? */

#ifndef ASSETVIEWERLIB_CVIEWERCREDENTIALS_H
#include "AssetViewerLib_CViewerCredentials.h"
#define ASSETVIEWERLIB_CVIEWERCREDENTIALS_H
#endif /* ASSETVIEWERLIB_CVIEWERCREDENTIALS_H ? */

#ifndef ASSETVIEWERLIB_CVIEWERSHELL_H
#include "AssetViewerLib_CViewerShell.h"
#define ASSETVIEWERLIB_CVIEWERSHELL_H
#endif /* ASSETVIEWERLIB_CVIEWERSHELL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF { namespace GUI { class CGUIInputAdapter; } }
namespace GUCEF { namespace INPUT { class CInputContext; } }

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The asset viewer as an application. The command line:
 *
 *      <path>...                       folders, archives (zip, unitypackage, ...) or files of the disk to open at the start, as the open dialogs do
 *      --mount <mountPoint>=<source>   a folder, an archive (zip, unitypackage, ...) or the VFS path of an archive in an earlier mount; repeatable
 *      --open <vfs path>               an asset to show at the start
 *      --renderer <name>               OpenGL or Vulkan (the renderer of the video settings otherwise)
 *      --capture <folder> <vfs path>...  headless: a contact sheet PNG per asset into the folder, then exit
 *      --thumbnail-cache <folder>      where the thumbnails are kept (the cache folder of the user otherwise)
 *      --tile <pixels>                 the size of one view of a contact sheet (256)
 *      --offscreen                     no window on the screen (always when --capture runs without a display)
 *      --ingest <path>                 a folder or archive (VFS path or path of the disk) to add to the primary store after the start; repeatable
 *      --search <text>                 the text to search the primary store for once the ingests are done
 *      --screenshot <file.png>         writes the window to the file once the ingests and the search are done, then exits
 *      --bootstrap <file>, --config <file>, --log <file>, --help
 *
 *  With --capture no GUI is made: the window is only the place Ogre draws in, and it is Ogre's own headless surface when there is no
 *  display. The exit code is 0 when every sheet was written, 3 when one was not, 2 and above 3 for a start that failed.
 *
 *  Sign in: the sessions of the online stores that sign in on a web site are kept in a key file store and the passwords the owner saves in a
 *  keyring code store, both in the folder "Auth" of the settings folder (CViewerCredentials), installed for the store plugins from the
 *  start and locked when the viewer ends; the sign in itself is the business of the sources tab (CViewerSignIn).
 *
 *  Settings: section "AssetViewer" (thumbnailCache, captureTileSize, RecentLocations: the places that were opened lately), "AssetSources" (the primary store, the downloads, the formats
 *  to prefer, the idle lock of the keyring, see CAssetSourcesSettings), "VideoSettings", "GeneralSettings" and "Main/AppArgs"
 *  (minimalLogLevel, logFile). The section "Ogre" belongs to the Ogre support library.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CViewerApp : public CORE::CGloballyConfigurable ,
                                             public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CViewerApp > TEventCallback;

    static const CORE::CString ClassTypeName;
    static const CORE::CString AppName;             /**< "AssetViewer": the settings section and the names of the config files */
    static const CORE::CString WindowTitle;

    CViewerApp( int argc, char** argv );

    virtual ~CViewerApp() GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the command line; false when it cannot be read or help was asked for */
    bool ParseCommandLine( void );

    static void LogUsage( void );

    /** The viewer starts when the application runs and the VFS is ready (after the global config is loaded) */
    void RegisterEventHandlers( void );

    /** The console log of the general settings, as soon as the settings are loaded */
    void ApplyLogSettings( void );

    const CORE::CString& GetBootstrapConfigFile( void ) const;
    const CORE::CString& GetConfigFile( void ) const;
    Int32 GetMinimalLogLevel( void ) const;
    const CORE::CString& GetLogFile( void ) const;
    Int32 GetExitCode( void ) const;
    bool IsCapturing( void ) const;

    /** Whether the viewer wants to end: its window was closed, the capture is done or the start failed */
    bool IsFinished( void ) const;

    /**
     *  Takes everything the viewer made down: the input, the forms, the render paths, the window and the mounts. It is not done in a
     *  pulse of the application, as the windows and the GUI have not left the notifications of the pulse generator then: the program
     *  pulses the application itself until IsFinished(), calls this and then stops the application, while Ogre is still there
     *  (the destructor does it as well, when the program did not)
     */
    void Shutdown( void );

    /** What the command line asked for, for tests */
    const CORE::CString::StringVector& GetMountSpecs( void ) const;
    const CORE::CString::StringVector& GetCaptureAssets( void ) const;
    const CORE::CString::StringVector& GetOpenLocations( void ) const;
    GUI::CRecentLocations& GetRecentLocations( void );
    const CORE::CString& GetCaptureFolder( void ) const;
    const CORE::CString& GetOpenPath( void ) const;
    const CORE::CString& GetRenderer( void ) const;
    const CORE::CString& GetThumbnailCacheFolder( void ) const;
    const CORE::CString::StringVector& GetIngestPaths( void ) const;
    const CORE::CString& GetSearchText( void ) const;
    const CORE::CString& GetScreenshotFile( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& globalConfig ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& globalConfig ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    void OnWorkStartTrigger( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCloseRequest( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void KeepRecentLocations( void );
    void OnRecentLocationsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void Start( void );
    bool CreateViewerWindow( void );
    bool StartCapture( void );
    bool StartInteractive( void );
    bool CreateInput( void );
    void CaptureNext( void );
    void StepScript( void );
    void Stop( Int32 exitCode );

    CViewerApp( const CViewerApp& src );              /**< not implemented: the application is unique */
    CViewerApp& operator=( const CViewerApp& src );   /**< not implemented: the application is unique */

    private:

    int m_argc;
    char** m_argv;

    // settings and command line
    CORE::CString m_bootstrapConfigFile;
    CORE::CString m_configFile;
    CORE::CString m_logFile;
    CORE::CString m_renderer;
    CORE::CString m_openPath;
    CORE::CString m_captureFolder;
    CORE::CString m_thumbnailCacheFolder;
    CORE::CString m_screenshotFile;
    CORE::CString m_searchText;
    CORE::CString::StringVector m_ingestPaths;
    CORE::CString::StringVector m_mountSpecs;
    CORE::CString::StringVector m_captureAssets;
    CORE::CString::StringVector m_openLocations;
    CORE::CString m_settingsThumbnailCache;     /**< what the settings file says: a value given on the command line is for this run only and is not kept in it */
    CORE::CString m_settingsLogFile;
    UInt32 m_settingsTileSize;
    Int32 m_minimalLogLevel;
    UInt32 m_tileSize;
    bool m_offscreen;
    bool m_capturing;
    GUI::CVideoSettings m_videoSettings;
    GUI::CGeneralSettings m_generalSettings;
    CAssetSourcesSettings m_sourcesSettings;
    GUI::CRecentLocations m_recentLocations;
    CViewerCredentials m_credentials;           /**< the stores of the sessions and the saved passwords of the sign in of the online stores: installed from the start, before the store plugins load; locked when the viewer ends */

    // the running viewer
    CORE::CEventAggregateTrigger m_workStartTrigger;
    CAssetMounter m_mounter;
    GUI::TWindowContextPtr m_window;
    GUI::TGuiContextPtr m_gui;
    GUIDRIVEROGRE::COgreWindowContext* m_ogreWindow;
    CAssetCapture* m_capture;
    CViewerShell* m_shell;
    INPUT::CInputContext* m_input;
    GUI::CGUIInputAdapter* m_inputAdapter;
    UInt32 m_nextCapture;
    UInt32 m_captureFailures;
    UInt32 m_scriptStage;
    UInt32 m_scriptFrames;
    Int32 m_exitCode;
    bool m_started;
    bool m_stopRequested;
    bool m_recentDirty;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CVIEWERAPP_H ? */
