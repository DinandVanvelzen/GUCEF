/*
 *  AssetWorkbenchLib: the shell of the workbench: the asset tool forms of gucefGUI composed into the content browser, the views, the inspector and the report
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSHELL_H
#define ASSETWORKBENCHLIB_CWORKBENCHSHELL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CCONTENTBROWSERFORM_H
#include "gucefGUI_CContentBrowserForm.h"
#define GUCEF_GUI_CCONTENTBROWSERFORM_H
#endif /* GUCEF_GUI_CCONTENTBROWSERFORM_H ? */

#ifndef GUCEF_GUI_CVIEWPORTFORM_H
#include "gucefGUI_CViewportForm.h"
#define GUCEF_GUI_CVIEWPORTFORM_H
#endif /* GUCEF_GUI_CVIEWPORTFORM_H ? */

#ifndef GUCEF_GUI_CSPLITTER_H
#include "gucefGUI_CSplitter.h"
#define GUCEF_GUI_CSPLITTER_H
#endif /* GUCEF_GUI_CSPLITTER_H ? */

#ifndef GUCEF_GUI_CFOLDABLEPANEL_H
#include "gucefGUI_CFoldablePanel.h"
#define GUCEF_GUI_CFOLDABLEPANEL_H
#endif /* GUCEF_GUI_CFOLDABLEPANEL_H ? */

#ifndef GUCEF_GUI_CLAYOUTMANAGER_H
#include "gucefGUI_CLayoutManager.h"
#define GUCEF_GUI_CLAYOUTMANAGER_H
#endif /* GUCEF_GUI_CLAYOUTMANAGER_H ? */

#ifndef GUCEF_GUI_CDEVELOPERCONSOLEFORM_H
#include "gucefGUI_CDeveloperConsoleForm.h"
#define GUCEF_GUI_CDEVELOPERCONSOLEFORM_H
#endif /* GUCEF_GUI_CDEVELOPERCONSOLEFORM_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H
#include "gucefGUI_CKeyboardShortcutMap.h"
#define GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H
#endif /* GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H ? */

#ifndef ASSETWORKBENCHLIB_CVIEWPRESETTRACKER_H
#include "AssetWorkbenchLib_CViewPresetTracker.h"
#define ASSETWORKBENCHLIB_CVIEWPRESETTRACKER_H
#endif /* ASSETWORKBENCHLIB_CVIEWPRESETTRACKER_H ? */

#ifndef GUCEF_GUI_CTIMELINEFORM_H
#include "gucefGUI_CTimelineForm.h"
#define GUCEF_GUI_CTIMELINEFORM_H
#endif /* GUCEF_GUI_CTIMELINEFORM_H ? */

#ifndef GUCEF_GUI_CIMAGEVIEWFORM_H
#include "gucefGUI_CImageViewForm.h"
#define GUCEF_GUI_CIMAGEVIEWFORM_H
#endif /* GUCEF_GUI_CIMAGEVIEWFORM_H ? */

#ifndef GUCEF_GUI_CREPORTVIEWFORM_H
#include "gucefGUI_CReportViewForm.h"
#define GUCEF_GUI_CREPORTVIEWFORM_H
#endif /* GUCEF_GUI_CREPORTVIEWFORM_H ? */

#ifndef GUCEF_GUI_CPROPERTYINSPECTORFORM_H
#include "gucefGUI_CPropertyInspectorForm.h"
#define GUCEF_GUI_CPROPERTYINSPECTORFORM_H
#endif /* GUCEF_GUI_CPROPERTYINSPECTORFORM_H ? */

#ifndef GUCEF_GUI_CTRANSFERQUEUEFORM_H
#include "gucefGUI_CTransferQueueForm.h"
#define GUCEF_GUI_CTRANSFERQUEUEFORM_H
#endif /* GUCEF_GUI_CTRANSFERQUEUEFORM_H ? */

#ifndef GUCEF_GUI_COPENACTIONSTRIPFORM_H
#include "gucefGUI_COpenActionStripForm.h"
#define GUCEF_GUI_COPENACTIONSTRIPFORM_H
#endif /* GUCEF_GUI_COPENACTIONSTRIPFORM_H ? */

#ifndef GUCEF_GUI_CRECENTLOCATIONS_H
#include "gucefGUI_CRecentLocations.h"
#define GUCEF_GUI_CRECENTLOCATIONS_H
#endif /* GUCEF_GUI_CRECENTLOCATIONS_H ? */

#ifndef GUCEF_GUI_CFILESYSTEMDIALOGFORM_H
#include "gucefGUI_CFileSystemDialogForm.h"
#define GUCEF_GUI_CFILESYSTEMDIALOGFORM_H
#endif /* GUCEF_GUI_CFILESYSTEMDIALOGFORM_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETSOURCESPANE_H
#include "AssetWorkbenchLib_CAssetSourcesPane.h"
#define ASSETWORKBENCHLIB_CASSETSOURCESPANE_H
#endif /* ASSETWORKBENCHLIB_CASSETSOURCESPANE_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSIGNIN_H
#include "AssetWorkbenchLib_CWorkbenchSignIn.h"
#define ASSETWORKBENCHLIB_CWORKBENCHSIGNIN_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHSIGNIN_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSTORECONNECTION_H
#include "AssetWorkbenchLib_CWorkbenchStoreConnection.h"
#define ASSETWORKBENCHLIB_CWORKBENCHSTORECONNECTION_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHSTORECONNECTION_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H
#include "AssetWorkbenchLib_CWorkbenchJobRegistry.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBSFORM_H
#include "AssetWorkbenchLib_CWorkbenchJobsForm.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBSFORM_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBSFORM_H ? */

#ifndef ASSETWORKBENCHLIB_CBULKCONVERSIONFORM_H
#include "AssetWorkbenchLib_CBulkConversionForm.h"
#define ASSETWORKBENCHLIB_CBULKCONVERSIONFORM_H
#endif /* ASSETWORKBENCHLIB_CBULKCONVERSIONFORM_H ? */

#ifndef ASSETWORKBENCHLIB_CLIBRARYSYNCFORM_H
#include "AssetWorkbenchLib_CLibrarySyncForm.h"
#define ASSETWORKBENCHLIB_CLIBRARYSYNCFORM_H
#endif /* ASSETWORKBENCHLIB_CLIBRARYSYNCFORM_H ? */

#ifndef ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H
#include "AssetWorkbenchLib_CCookedOutputResolver.h"
#define ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H
#endif /* ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHREDOACTIONS_H
#include "AssetWorkbenchLib_CWorkbenchRedoActions.h"
#define ASSETWORKBENCHLIB_CWORKBENCHREDOACTIONS_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHREDOACTIONS_H ? */

#ifndef GUCEF_GUI_CACTIVITYSERVICE_H
#include "gucefGUI_CActivityService.h"
#define GUCEF_GUI_CACTIVITYSERVICE_H
#endif /* GUCEF_GUI_CACTIVITYSERVICE_H ? */

#ifndef GUCEF_GUI_CACTIVITYINDICATORFORM_H
#include "gucefGUI_CActivityIndicatorForm.h"
#define GUCEF_GUI_CACTIVITYINDICATORFORM_H
#endif /* GUCEF_GUI_CACTIVITYINDICATORFORM_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETLOADER_H
#include "AssetWorkbenchLib_CAssetLoader.h"
#define ASSETWORKBENCHLIB_CASSETLOADER_H
#endif /* ASSETWORKBENCHLIB_CASSETLOADER_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#include "AssetWorkbenchLib_CAssetTypeResolver.h"
#define ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#endif /* ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETPACKSUMMARY_H
#include "AssetWorkbenchLib_CAssetPackSummary.h"
#define ASSETWORKBENCHLIB_CASSETPACKSUMMARY_H
#endif /* ASSETWORKBENCHLIB_CASSETPACKSUMMARY_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETMOUNTER_H
#include "AssetWorkbenchLib_CAssetMounter.h"
#define ASSETWORKBENCHLIB_CASSETMOUNTER_H
#endif /* ASSETWORKBENCHLIB_CASSETMOUNTER_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETOPENER_H
#include "AssetWorkbenchLib_CAssetOpener.h"
#define ASSETWORKBENCHLIB_CASSETOPENER_H
#endif /* ASSETWORKBENCHLIB_CASSETOPENER_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETCAPTURE_H
#include "AssetWorkbenchLib_CAssetCapture.h"
#define ASSETWORKBENCHLIB_CASSETCAPTURE_H
#endif /* ASSETWORKBENCHLIB_CASSETCAPTURE_H ? */

#ifndef ASSETWORKBENCHLIB_CTHUMBNAILPROVIDER_H
#include "AssetWorkbenchLib_CThumbnailProvider.h"
#define ASSETWORKBENCHLIB_CTHUMBNAILPROVIDER_H
#endif /* ASSETWORKBENCHLIB_CTHUMBNAILPROVIDER_H ? */

#ifndef ASSETWORKBENCHLIB_CREPORTLOGGER_H
#include "AssetWorkbenchLib_CReportLogger.h"
#define ASSETWORKBENCHLIB_CREPORTLOGGER_H
#endif /* ASSETWORKBENCHLIB_CREPORTLOGGER_H ? */

#ifndef ASSETWORKBENCHLIB_CMODELVIEW_H
#include "AssetWorkbenchLib_CModelView.h"
#define ASSETWORKBENCHLIB_CMODELVIEW_H
#endif /* ASSETWORKBENCHLIB_CMODELVIEW_H ? */

#ifndef ASSETWORKBENCHLIB_CMATERIALVIEW_H
#include "AssetWorkbenchLib_CMaterialView.h"
#define ASSETWORKBENCHLIB_CMATERIALVIEW_H
#endif /* ASSETWORKBENCHLIB_CMATERIALVIEW_H ? */

#ifndef ASSETWORKBENCHLIB_CTEXTUREVIEW_H
#include "AssetWorkbenchLib_CTextureView.h"
#define ASSETWORKBENCHLIB_CTEXTUREVIEW_H
#endif /* ASSETWORKBENCHLIB_CTEXTUREVIEW_H ? */

#ifndef ASSETWORKBENCHLIB_CSCENEVIEW_H
#include "AssetWorkbenchLib_CSceneView.h"
#define ASSETWORKBENCHLIB_CSCENEVIEW_H
#endif /* ASSETWORKBENCHLIB_CSCENEVIEW_H ? */

#ifndef ASSETWORKBENCHLIB_CPARTICLEPREVIEW_H
#include "AssetWorkbenchLib_CParticlePreview.h"
#define ASSETWORKBENCHLIB_CPARTICLEPREVIEW_H
#endif /* ASSETWORKBENCHLIB_CPARTICLEPREVIEW_H ? */

#ifndef ASSETWORKBENCHLIB_CSHOWCASEVIEW_H
#include "AssetWorkbenchLib_CShowcaseView.h"
#define ASSETWORKBENCHLIB_CSHOWCASEVIEW_H
#endif /* ASSETWORKBENCHLIB_CSHOWCASEVIEW_H ? */

#ifndef ASSETWORKBENCHLIB_CPACKPRESENTATIONRESOLVER_H
#include "AssetWorkbenchLib_CPackPresentationResolver.h"
#define ASSETWORKBENCHLIB_CPACKPRESENTATIONRESOLVER_H
#endif /* ASSETWORKBENCHLIB_CPACKPRESENTATIONRESOLVER_H ? */

#ifndef ASSETWORKBENCHLIB_CRESOURCEVIEWFILESYSTEMACCESS_H
#include "AssetWorkbenchLib_CResourceViewFileSystemAccess.h"
#define ASSETWORKBENCHLIB_CRESOURCEVIEWFILESYSTEMACCESS_H
#endif /* ASSETWORKBENCHLIB_CRESOURCEVIEWFILESYSTEMACCESS_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXRUNRESULT_H
#include "AssetConverterLib_CPackIndexRunResult.h"
#define ASSETCONVERTERLIB_CPACKINDEXRUNRESULT_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXRUNRESULT_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H
#include "AssetWorkbenchLib_CWorkbenchPreferences.h"
#define ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H ? */

#ifndef ASSETWORKBENCHLIB_CPACKASSETCONVERTER_H
#include "AssetWorkbenchLib_CPackAssetConverter.h"
#define ASSETWORKBENCHLIB_CPACKASSETCONVERTER_H
#endif /* ASSETWORKBENCHLIB_CPACKASSETCONVERTER_H ? */

#ifndef ASSETWORKBENCHLIB_CAUTOTHUMBNAILSTORE_H
#include "AssetWorkbenchLib_CAutoThumbnailStore.h"
#define ASSETWORKBENCHLIB_CAUTOTHUMBNAILSTORE_H
#endif /* ASSETWORKBENCHLIB_CAUTOTHUMBNAILSTORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

class CShellFrameListener;
class CPackPresentationJob;
class CPackIndexJob;
class CResourceViewThumbnailProvider;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The workbench as a window of a GUI context. Everything on it is one of the asset tool forms of gucefGUI, placed in panels:
 *    - left, in tabs: the content browser of the VFS (with thumbnails of the assets), the scene browser (the node tree of
 *      a scene, which is a content browser over CSceneTreeFileSystemAccess) and the asset sources (the primary store and the online
 *      stores, see CAssetSourcesPane; made by InitializeSources());
 *    - in the middle, in tabs: the 3D view (a viewport form on the render texture of the stage, and the time line of the
 *      animation clips below it), the texture (an image view form) and the UV layout of a sub mesh (a second image view form);
 *    - right: the property inspector, with the statistics and the editable properties of what is shown;
 *    - below: the report view with the warnings and errors that the workbench, Ogre and the plugins logged, and beside it the transfer
 *      queue with the downloads and the ingests of the asset sources.
 *  Opening an asset (OpenAsset(), a double click or a selection in the browser) chooses the view by the kind of the asset. The
 *  render texture of the stage is drawn in a frame listener of Ogre, when the camera, the animation or a switch changed it.
 *
 *  Opening: nothing is open when the workbench starts, so the content tab has an action strip at its top (CAssetOpener has the
 *  logic, the file system dialogs of gucefGUI the choosing): "Open folder...", "Open archive or pack..." and "Open file...", and a
 *  list of the places that were opened lately. The content browser has two roots: "Local files", the file system of the
 *  computer, where the workbench starts, and "VFS", the folders and packs that are mounted. A folder, archive or pack that is opened
 *  is mounted and shown in the VFS root; a file is opened in the view that suits it. Ctrl+O shows the dialog for a file and
 *  Ctrl+Shift+O the one for a folder. While nothing is open the 3D view says how to open something.
 *
 *  Extension points for later: the reports of the converter (a CReportViewForm per report) and the intermediate with its sidecar
 *  (another asset kind in OpenAsset() with its own view and inspector data) fit in the same panels.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchShell : public CORE::CObservingNotifier
{
    public:

    typedef CORE::CTEventHandlerFunctor< CWorkbenchShell > TEventCallback;

    /** An action the shell does not do itself (the settings, the help, the developer console): TCloneableString, the id of the action. From a key or from the menu */
    static const CORE::CEvent ActionRequestedEvent;
    /** The shell changed a preference (the choice of a pack, the switch of the presentation, the automatic thumbnails): the application keeps the settings */
    static const CORE::CEvent PreferencesChangedEvent;

    static void RegisterEvents( void );

    /** The display names of the roots of the content browser */
    static const CORE::CString LocalFilesRootName;
    static const CORE::CString VfsRootName;
    static const CORE::CString ResourcesRootName;       /**< the root of the resource view (F17) of the content browser */
    static const CORE::CString ActionReIndex;           /**< the action that builds the pack index of the pack that is shown again */

    /** The ids of the actions of the open strip */
    static const CORE::CString ActionOpenFolder;
    static const CORE::CString ActionOpenArchive;
    static const CORE::CString ActionConvertForOgre;    /**< "ConvertForOgre": Convert for Ogre of what is shown */
    static const CORE::CString ActionOpenFile;

    explicit CWorkbenchShell( CAssetMounter& mounter );

    virtual ~CWorkbenchShell() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Builds the forms in the context. 'capture' (not owned, it has to stay as long as the shell) makes the thumbnails of the
     *  content browser, with the cache in 'thumbnailCacheFolder' when that is not empty; without it the browser lists types only
     */
    bool Initialize( GUI::CGUIContext& context, CAssetCapture* capture, const CORE::CString& thumbnailCacheFolder );

    void Shutdown( void );

    bool IsInitialized( void ) const;

    /**
     *  Makes the asset sources of the left tab from the settings: opens the primary store, starts the downloads and fills the page
     *  with the stores. An asset that is opened from the page is shown in the views, a folder in the content browser. False
     *  when the primary store cannot be opened (the page then says so); without this call the tab is empty
     */
    bool InitializeSources( const CAssetSourcesSettings& settings );

    /**
     *  The credential stores of the sign in of the online stores (not owned; they have to be there as long as the shell, before the
     *  sources are made): the saved passwords and the keyring code. Without them a sign in can only be done with the browser window,
     *  and nothing is saved
     */
    void SetCredentials( CWorkbenchCredentials* credentials );

    /** The manager of gucefAUTH that the stores that sign in with a provider of it use (not owned); before or after InitializeSources() */
    void SetAuthManager( AUTH::CAuthManager* manager );

    /**
     *  Shows an asset of the mounts in the view that suits its kind; false when it cannot be shown (the reason is in the report).
     *  The synchronous form, on the calling thread: for the command line and the tests. The interface uses OpenAssetAsync()
     */
    bool OpenAsset( const CORE::CString& vfsPath );

    /**
     *  The asynchronous form of OpenAsset(): the file is read, decoded and imported on a thread of a pool (CAssetLoader), and the views
     *  are made on the thread of the renderer in pieces within a budget per frame (Update()), so the interface keeps drawing. The
     *  previous asset stays shown until the new one is ready. Opening something else meanwhile drops this result; the activity service
     *  shows the operation (label: the name of the asset) when it takes longer than the threshold, with a cancel button. The future
     *  is that of the preparation task; the asset is shown when IsOpening() is false again (LoadFinished of GetLoader())
     */
    CORE::CFutureResult OpenAssetAsync( const CORE::CString& vfsPath );

    /** Whether an asset is being opened with OpenAssetAsync() */
    bool IsOpening( void ) const;

    /** Ends the asset that is being opened: nothing of it is shown, the previous asset stays */
    void CancelOpening( void );

    /**
     *  Whether the selections and double clicks of the interface open with OpenAssetAsync() (on in the application) or OpenAsset()
     *  (off by default, so that a test that selects a file finds it shown at once)
     */
    void SetAsyncOpenEnabled( bool enabled );

    bool IsAsyncOpenEnabled( void ) const;

    /** The service that tells the user about what takes time; its indicator is in the status strip at the bottom of the window */
    GUI::CActivityService& GetActivityService( void );

    GUI::CActivityIndicatorForm* GetActivityIndicator( void ) const;

    CAssetLoader& GetLoader( void );

    /**
     *  Opens a folder, an archive or pack, or a file of the disk, as the open dialogs and the command line do (see CAssetOpener):
     *  it is mounted, remembered in the recent places, and shown: a folder or archive in the VFS root of the content browser, a file
     *  in the view that suits it. 'showInBrowser' false leaves the content browser where it is for a file (a double click on a file
     *  in the files of the computer). False when it cannot be opened (the reason is in the report)
     */
    bool OpenLocation( const CORE::CString& diskPath, bool showInBrowser = true );

    /** The dialogs of the open strip, as its buttons show them (the first call makes them); false when the dialog cannot be shown */
    bool ShowOpenFolderDialog( void );
    bool ShowOpenArchiveDialog( void );
    bool ShowOpenFileDialog( void );

    /** The keys of the workbench: those of the table of the shortcuts (GetActionTable()), by default Ctrl+O the open file dialog and Ctrl+Shift+O the open folder dialog; true when the key was one of them */
    bool HandleKey( INPUT::KeyCode keyCode, UInt32 modifiers );

    /** The places that were opened lately (not owned; the list of the shell itself until another is given); it has to be there as long as the shell has it */
    void SetRecentLocations( GUI::CRecentLocations* recent );

    GUI::CRecentLocations* GetRecentLocations( void ) const;

    /** What the 3D view says while nothing is open, and whether it says so now */
    static const CORE::CString& GetEmptyStateText( void );
    bool IsEmptyStateShown( void ) const;

    /** Takes what is shown away */
    void CloseAsset( void );

    EAssetKind GetShownKind( void ) const;
    const CORE::CString& GetShownPath( void ) const;

    /** Work for the thread of the GUI between two frames: the report log, and the thumbnails within the budget in milliseconds */
    void Update( UInt32 thumbnailBudgetInMs );

    /** The log that feeds the report view (the application hands it to the log manager) */
    CReportLogger& GetLogger( void );

    /** Called at the start of every frame of Ogre: draws the render texture of the stage when something changed it */
    void OnOgreFrameStarted( UInt32 deltaInMs );

    /*
     *  The parts, for tests and for applications that go beyond this
     */

    GUI::CContentBrowserForm* GetBrowser( void ) const;
    GUI::CContentBrowserForm* GetSceneBrowser( void ) const;
    GUI::CViewportForm* GetViewport( void ) const;
    GUI::CTimelineForm* GetTimeline( void ) const;
    GUI::CPropertyInspectorForm* GetInspector( void ) const;
    GUI::CImageViewForm* GetImageView( void ) const;
    GUI::CImageViewForm* GetUvView( void ) const;
    GUI::CReportViewForm* GetReport( void ) const;
    GUI::CTransferQueueForm* GetTransfers( void ) const;
    GUI::COpenActionStripForm* GetOpenStrip( void ) const;
    GUI::CFileSystemDialogForm* GetFolderDialog( void ) const;
    GUI::CFileSystemDialogForm* GetArchiveDialog( void ) const;
    GUI::CFileSystemDialogForm* GetFileDialog( void ) const;
    CAssetSourcesPane* GetSources( void ) const;

    /** The sign in of the online stores: the forms and the routing; NULL before InitializeSources() */
    CWorkbenchSignIn* GetSignIn( void ) const;

    /**
     *  The connection to a remote asset store service (made by InitializeSources() or here, with the credential store that keeps the pairing; the key file
     *  store of the credentials of the workbench by default). When it is connected the sources tab, the transfers, the jobs and the bulk conversion work on the
     *  service it opened; when it ends they go back to the store of this process
     */
    CWorkbenchStoreConnection* GetStoreConnection( void ) const;
    bool InitializeStoreConnection( AUTH::ICredentialStore* store = GUCEF_NULL );
    CThumbnailProvider* GetThumbnailProvider( void ) const;

    CWorkbenchStage& GetStage( void );
    COrbitController& GetController( void );
    CModelView& GetModelView( void );
    CMaterialView& GetMaterialView( void );
    CTextureView& GetTextureView( void );
    CSceneView& GetSceneView( void );

    /** The particle systems of the scene or particle script shown, played by the time line ("particles" clip) */
    CParticlePreview& GetParticlePreview( void );

    /** Which tab of the middle is in front: 0 the 3D view, 1 the texture, 2 the UV layout */
    Int32 GetCenterTab( void ) const;

    /** The title of a tab of the centre: "3D", "Image" or "Texture", "UV layout" */
    CORE::CString GetCenterTabTitle( UInt32 index ) const;

    /**
     *  Shows what a folder of the VFS is in the inspector: a recognised pack (Unity, Godot or Unreal project, MetaHuman package, model pack) with
     *  its engine, version, the files per kind, the size and the entry points. A folder that is no pack is shown only when nothing is open.
     *  False when it shows nothing.
     */
    bool ShowFolderSummary( const CORE::CString& vfsFolder );

    const CAssetPackSummary& GetPackSummary( void ) const;

    /** Which tab of the left side is in front: 0 the content, 1 the scene, 2 the asset sources */
    Int32 GetLeftTab( void ) const;
    void ShowLeftTab( Int32 tab );

    /** Whether the render texture is bound to the surface of the viewport form (needs the GUI driver ImGuiOgre) */
    bool IsStageBound( void ) const;

    /*
     *  The arrangement of the window: splitters between the resources (left), the views (middle) and the inspector (right), and
     *  between those and the report row; the left and the right side fold into a strip of their title
     */

    /** The names of the layouts of the shell, which the layout manager has as presets: the sizes of the splitters and what is folded */
    static const CORE::CString LayoutBrowse;        /**< the resources and the inspector open, as the window starts */
    static const CORE::CString LayoutReview;        /**< the resources folded, a bigger inspector and report */
    static const CORE::CString LayoutPreview;       /**< both sides folded: the view has almost the whole width */

    /** The names of the widgets of the arrangement, by which the layout state keeps them */
    static const CORE::CString MainSplitName;       /**< the main area above the report row */
    static const CORE::CString SideSplitName;       /**< the resources and the rest */
    static const CORE::CString ViewSplitName;       /**< the views and the inspector */
    static const CORE::CString LeftPanelName;
    static const CORE::CString RightPanelName;

    /** The keys of the actions: F9 folds the resources, F10 the inspector, F11 switches to the Preview layout and back */
    static const CORE::CString ActionFoldLeft;
    static const CORE::CString ActionFoldRight;
    static const CORE::CString ActionPreview;
    /** More actions with a key: Ctrl+, the settings, F1 the help of the place, the key left of 1 the developer console (the ids of the open actions are above) */
    static const CORE::CString ActionSettings;
    static const CORE::CString ActionHelp;
    static const CORE::CString ActionConsole;
    static const CORE::CString ActionConnectStore;      /**< "ConnectStore": File, Connect to store...: the settings layer opens the page Remote store */

    /** An action of the table of the shortcuts: its id, the text of the settings, and the key the workbench ships it with */
    struct SActionInfo
    {
        const char* actionId;
        const char* title;
        INPUT::KeyCode defaultKey;
        UInt32 defaultModifiers;
    };
    typedef struct SActionInfo TActionInfo;

    /** The actions that have a key, in the order the settings list them */
    static const TActionInfo* GetActionTable( UInt32& count );

    /**
     *  Does the action as its key or its menu item does: the folds, the layouts and the open dialogs here, the settings, the help and the
     *  developer console by raising ActionRequestedEvent. "Layout:<name>" puts the window in the layout of that name. False for an action
     *  that is none
     */
    bool ExecuteAction( const CORE::CString& actionId );

    /** The GUI context the shell was made in, NULL before Initialize() */
    GUI::CGUIContext* GetGUIContext( void ) const;

    /** The menu bar at the top of the window, NULL before Initialize() */
    GUI::CWidget* GetMenuBar( void ) const;

    /** The text of the menu item as the menu bar has it ("File/Settings...") for the action; empty for an action that has none */
    CORE::CString GetMenuPathOfAction( const CORE::CString& actionId ) const;

    /** The developer console that gets the key of the console and the text that reached no one (not owned; NULL for none) */
    void SetDeveloperConsole( GUI::CDeveloperConsoleForm* console );
    GUI::CDeveloperConsoleForm* GetDeveloperConsole( void ) const;

    /**
     *  The manager that keeps the layout: the application gives its own, which it keeps in the settings of the user; the shell
     *  has one of its own until then (NULL gives it back). The built in layouts are added to it and its stored layout is applied.
     */
    void SetLayoutManager( GUI::CLayoutManager* layout );
    GUI::CLayoutManager& GetLayoutManager( void );

    /** Puts on one of the layouts (Browse, Review, Preview, or one of the user); false for a name that is none */
    bool ApplyLayoutPreset( const CORE::CString& name );

    GUI::CFoldablePanel* GetLeftPanel( void ) const;
    GUI::CFoldablePanel* GetRightPanel( void ) const;
    GUI::CSplitter* GetMainSplitter( void ) const;
    GUI::CSplitter* GetSideSplitter( void ) const;
    GUI::CSplitter* GetViewSplitter( void ) const;

    void ToggleLeftSide( void );
    void ToggleRightSide( void );

    /** The Preview layout, or the layout from before it when it is on */
    void TogglePreviewLayout( void );

    GUI::CKeyboardShortcutMap& GetShortcuts( void );

    /*
     *  The view presets of the 3D view: Frame, the fixed views, Four views; the list shows Custom when the camera left the preset
     */

    CViewPresetTracker& GetViewPresetTracker( void );

    /** The text the view list shows now */
    CORE::CString GetViewPresetLabel( void ) const;

    /** Chooses a preset by its name as the user would in the view list (Frame, Front, Side, Top, Three quarter, Four views); false for a name that is none */
    bool ApplyViewPreset( const CORE::CString& name );

    /** Whether the four view layout is on */
    bool IsFourViewsShown( void ) const;

    /** The view preset an asset of the kind is shown with when it is opened, empty for the usual one (Frame); an application or a pack profile sets it */
    void SetDefaultViewPreset( EAssetKind kind, const CORE::CString& presetName );
    const CORE::CString& GetDefaultViewPreset( EAssetKind kind ) const;

    /*
     *  Pack presentations: what a pack that is opened shows without more clicks
     */

    /** The preferences the shell reads and writes (the switches, the choice per pack); without a call it has some of its own */
    void SetPreferences( CWorkbenchPreferences* preferences );
    CWorkbenchPreferences& GetPreferences( void );
    /** Whether a pack that is opened shows its presentation: the command line override when there is one, else the preference (on by default) */
    bool IsPackPresentationEnabled( void ) const;
    /** Switches the preference; it is kept (PreferencesChangedEvent) */
    void SetPackPresentationEnabled( bool enabled );
    /** What the command line says for this run: -1 nothing, 0 off, 1 on; it is not kept */
    void SetPackPresentationOverride( Int32 value );
    Int32 GetPackPresentationOverride( void ) const;
    /** The kind of presentation for the packs of this run (the command line): used when the user did not choose one; not kept. Empty for none */
    void SetPresentationKindOverride( const CORE::CString& kindId );
    /** The presentation of the pack at a VFS folder, as an open does: nothing happens when the presentation is switched off and the request is no explicit one. False when the folder is no recognised pack or nothing was started */
    bool OpenPack( const CORE::CString& vfsFolder, bool explicitRequest );
    /** Shows the presentation of a kind (CPackPresentation::GetKindId(), empty for the remembered choice or the default); a kind that was named is the choice of the user and is remembered */
    bool ShowPackPresentation( const CORE::CString& vfsFolder, const CORE::CString& kindId, bool userChoice );
    /** Forgets the choice for the pack that is shown and shows its default */
    bool ResetPackPresentation( void );
    bool HasPackPresentation( void ) const;
    const CPackPresentation& GetPackPresentation( void ) const;
    /** The kinds that the selector offers for the pack that is shown */
    const GUCEF::vector< CPackPresentation::EKind >& GetOfferedPresentations( void ) const;
    /** The VFS folder of the pack that was opened last */
    const CORE::CString& GetPackRoot( void ) const;
    /** The key of the pack in the settings: its place on the disk */
    CORE::CString GetPackKey( const CORE::CString& vfsFolder ) const;
    CShowcaseView& GetShowcase( void );
    /** Where the conversions of presentations are kept (a folder of the disk) */
    const CORE::CString& GetPackCacheFolder( void ) const;
    /** Loads more models into the showcase (48 more than before); false when the pack has no more */
    bool LoadMoreOfPresentation( void );
    /** Switches the automatic thumbnails (the preference and the store); kept like the other preferences */
    void SetAutomaticThumbnailsEnabled( bool enabled );

    /*
     *  The source of what is shown: the cooked Ogre output (gucef-ogre3d) of a model, scene or pack when it exists and is current, else the
     *  quick look through the Assimp codec (CCookedOutputResolver)
     */

    /** Whether a model, scene or pack with current cooked output shows that (on by default); the preference, kept like the others */
    bool IsCookedOutputPreferred( void ) const;
    void SetCookedOutputPreferred( bool preferred );
    /** The source of the model, scene or pack that is shown, as the inspector says it ("cooked Ogre v1 output (gucef-ogre3d)", "quick look: ..."); empty when nothing of that kind is shown */
    CORE::CString GetShownSourceDescription( void ) const;
    /** What the cooked output of the shown asset or pack is (absent, stale, current); an absent output with no reason when nothing was looked for */
    const CCookedOutputResolver::SCookedOutput& GetShownCookedOutput( void ) const;
    /** The line the console command source prints: what is shown, from where, what the cooked output is, and the setting */
    CORE::CString GetSourceReport( void ) const;
    /**
     *  Convert for Ogre: starts the conversion of what is shown into cooked Ogre output (gucef-ogre3d) as a job, and switches the view
     *  to that output when it is done. A pack or asset of the primary store is a convert_pack job of the asset store service (the bulk
     *  conversion, scope the shown asset or the pack); a pack of a local source with an importer is converted into the cache folder as
     *  a request of the loader. False, with the reason in the report, when there is nothing to convert or no way to do it
     */
    bool ConvertForOgre( void );
    /** Whether Convert for Ogre was started and the view still waits for its result */
    bool IsConvertForOgrePending( void ) const;

    /** The automatic thumbnails: the renders of presentations that the content browser shows when there is no thumbnail of a better kind */
    CAutoThumbnailStore& GetAutoThumbnails( void );
    /** Takes a render of the 3D view now as the automatic thumbnail of the pack or model that is shown; false when it is not worthwhile or switched off */
    bool CaptureAutoThumbnail( void );
    /** Deletes the automatic thumbnails; the number deleted */
    UInt32 ClearAutoThumbnails( void );

    /*
     *  The resource view (F17): the pack that is opened as its resources by function, in the content browser beside the files
     */

    /** Whether a pack that is opened starts in the resource view: the command line override when there is one, else the preference (on by default) */
    bool IsOpenPacksInResourceView( void ) const;
    /** Switches the preference; it is kept (PreferencesChangedEvent) */
    void SetOpenPacksInResourceView( bool enabled );
    /** What the command line says for this run (--resource-view, --files-view): -1 nothing, 0 files, 1 resources; it is not kept */
    void SetResourceViewOverride( Int32 value );
    Int32 GetResourceViewOverride( void ) const;
    /** The folder of the resource view the browser goes to once the index of the next pack that is opened is shown (the command line: --resource-path); not kept */
    void SetResourceViewStartPath( const CORE::CString& path );
    /** Whether the content browser shows the resource view now */
    bool IsResourceViewShown( void ) const;
    /** Switches the content browser between the files and the resources of the pack; the choice is kept */
    void ShowResourceView( bool shown );
    /** The file system the resource view shows: the index of the pack that was indexed last */
    CResourceViewFileSystemAccess& GetResourceView( void );
    /** The VFS folder of the pack the resource view shows */
    const CORE::CString& GetResourceViewPack( void ) const;
    /** Gets the index of the pack at the VFS folder in the background (read when fresh, built when missing or stale) and shows it in the resource view; 'redo' forces the build. False when nothing was started */
    bool StartPackIndex( const CORE::CString& vfsFolder, bool redo );
    /** Builds the index of the pack that the resource view shows again (the Re-index action); false without such a pack */
    bool ReIndexPack( void );
    /** Whether an index is being got now */
    bool IsIndexing( void ) const;
    CAssetLoader& GetIndexLoader( void );
    /** What the last index job did: the work gate's decision and the counters */
    const ASSETCONVERTER::CPackIndexRunResult& GetLastIndexResult( void ) const;
    /** The resource a path of the resource view names, as the inspector shows it; false for a folder or a path of the files */
    bool GetResourceOfPath( const CORE::CString& browserPath, ASSETCONVERTER::CPackIndexResource& resource ) const;
    /** Called by the index job on the thread of the renderer: shows the index */
    void ApplyPackIndex( const CORE::CString& vfsFolder, ASSETCONVERTER::CPackIndexDocument& document, const ASSETCONVERTER::CPackIndexRunResult& result );

    private:

    class COpenJob;
    friend class COpenJob;
    friend class CPackPresentationJob;
    friend class CPackIndexJob;

    CWorkbenchShell( const CWorkbenchShell& src );
    CWorkbenchShell& operator=( const CWorkbenchShell& src );

    bool BuildLayout( void );
    void UnloadViews( void );
    void BindStage( void );
    void SetModelToggles( bool forMaterial );
    void SetInspector( const CORE::CDataNode& data, const GUI::CPropertyInspectorForm::TDescriptorMap& descriptors );
    void RefreshInspector( void );
    void ShowCenterTab( Int32 tab );
    void ShowUvLayout( Int32 subMesh );
    bool ShowModel( const CORE::CString& vfsPath );
    bool ShowModelLoad( const CORE::CString& vfsPath, CModelView::SPreparedMesh* prepared );
    void ShowModelFinish( void );
    bool ProbeCooked( const CORE::CString& vfsPath, CCookedOutputResolver::SCookedOutput& cooked ) const;
    void NoteSource( const CORE::CString& vfsPath, const CCookedOutputResolver::SCookedOutput& cooked, bool probed, bool usedCooked );
    bool ShowCooked( const CCookedOutputResolver::SCookedOutput& cooked, CModelView::SPreparedMesh* prepared, bool& isScene );
    void OnBulkNotification( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    bool ReopenShown( const CORE::CString& path );
    bool ShowTexture( const CORE::CString& vfsPath );
    void ShowTextureFinish( void );
    /** The second tab of the centre is called Image for a picture and Texture for GPU-ready data */
    void UpdateImageTabTitle( void );
    bool ShowOther( EAssetKind kind, const CORE::CString& vfsPath );
    void FinishOpen( EAssetKind kind, const CORE::CString& vfsPath, bool shown );
    public:

    bool OpenAssetInteractive( const CORE::CString& vfsPath );

    /**
     *  A pack as a file in the VFS (a package of an engine, a zip, a character package): it is mounted as a pack of its own and
     *  presented as a pack is, not looked at as an asset. False when it cannot be mounted
     */
    bool OpenPackageFile( const CORE::CString& vfsPath );

    /** Whether the file at this VFS path is a pack in one file (a package or an archive), which OpenPackageFile() takes */
    static bool IsPackageFile( const CORE::CString& vfsPath );
    void OnLoadFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    bool ShowMaterial( const CORE::CString& vfsPath );
    bool ShowScene( const CORE::CString& vfsPath, bool switchLeftTab = true );
    bool ShowSkeleton( const CORE::CString& vfsPath );
    bool ShowParticles( const CORE::CString& vfsPath );
    void FillTimeline( void );
    void ReportProblem( const CORE::CString& asset, const CORE::CString& message );
    void UpdateEmptyState( void );

    private:

    bool ShowOpenDialog( GUI::CFileSystemDialogForm*& dialog, const CORE::CString& kind );
    GUI::CFileSystemDialogForm* MakeOpenDialog( const CORE::CString& kind );
    void PrepareOpenDialog( GUI::CFileSystemDialogForm& dialog );
    bool IsOpenDialogShown( void ) const;
    public:
    /** What a choice in the VFS dialog does: a pack is presented, an asset opened, a folder entered */
    bool OpenVfsSelection( const CORE::CString& vfsPath );
    private:

    void OnBrowserSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnBrowserFolderChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnBrowserItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSceneFolderChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSceneSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnToggleChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSurfaceResized( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnClipSelected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTimeChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPlayStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void StartParticleClip( void );
    void OnPropertyChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSourcesAssetOpenRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSourcesFolderOpenRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSourcesRemoteContentChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSourcesSignInRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSourcesSignInSettingsRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSourcesSessionExpired( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnOpenActionRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnOpenRecentChosen( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnOpenDialogAccepted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRootKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnShortcutActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRootTextInput( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnMenuItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnJobSummaryActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnJobResultRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnBulkTemplatesChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void UpdateJobSummary( UInt32 deltaInMs );
    void AttachJobs( void );
    void OnStoreServiceReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStoreServiceLost( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void DetachService( void );
    void ReattachService( void );
    void BuildMenuBar( void );
    void UpdateMenuBar( void );
    void BindShortcuts( void );

    void RegisterLayoutPresets( GUI::CLayoutManager& layout );
    void ResetViewPreset( EAssetKind kind );
    void ApplyViewLayout( void );
    void UpdateViewPreset( void );
    void ShowViewPresetLabel( void );
    void FinishPackPresentation( const CPackPresentation& presentation );
    void FinishShowcase( void );
    void ApplyPresentationCamera( const CPackPresentation& presentation );
    void ApplyPresentationView( const CORE::CString& presetName );
    /** Frames the box that the running effect filled, so that a small flame fills 40 percent of every view; nothing when no effect is shown */
    void FrameEffectExtent( void );
    void PlayFirstClip( void );
    void AppendPresentationInspector( CORE::CDataNode& data ) const;
    void AddPresentationToggle( GUI::CViewportForm::TToggleVector& toggles );
    void RaisePreferencesChanged( void );
    void ReleaseConversion( void );
    void KeepConversion( const CPackAssetConverter::SRequest& request );
    bool StartPackPresentation( const CORE::CString& vfsFolder, const CORE::CString& requestedId, bool userChoice, CPackPresentation::EKind failedKind, const CORE::CString& failedReason );
    void UpdateAutoThumbnailCapture( UInt32 deltaInMs );
    void OnBrowserFolderActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnBrowserFilterChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnBrowserViewSwitched( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    /** The VFS path of what a path of the current root of the browser names: that of the resource in the resource view, the path itself elsewhere */
    CORE::CString ResolveBrowserPath( const CORE::CString& path ) const;
    public:

    /*
     *  The jobs (F15) and the bulk conversion (F8)
     */

    /** The long operations of the workbench itself (open, index, presentation, thumbnails), which the jobs list shows next to those of the service */
    CWorkbenchJobRegistry& GetJobRegistry( void );

    /** The jobs page (a window, made on first use; needs InitializeSources() for the jobs of the service); false when it cannot be shown. 'jobId' is selected in it */
    bool ShowJobs( const CORE::CString& jobId = CORE::CString::Empty );

    CWorkbenchJobsForm* GetJobsForm( void );

    /** The jobs summary of the status area: the counts of the jobs list, running, queued and failed */
    CWorkbenchJobSummary GetJobSummary( void ) const;
    CORE::CString GetJobSummaryText( void ) const;
    /** Brings the summary button of the status area up to date now (it follows the list by itself every 250 ms of frame time) */
    void RefreshJobSummary( void );
    GUI::CWidget* GetJobSummaryButton( void ) const;

    /** The job of the activity indicator that shows now: the jobs page opens on it. Empty when the indicator shows nothing that is a job */
    CORE::CString GetJobIdOfActivity( UInt32 activityId ) const;

    /** The bulk conversion panel (a window, made on first use) with the scope of what is selected now: the pack of the sources tab, the selection or the folder of the browser */
    bool ShowBulkConversion( void );

    CBulkConversionForm* GetBulkConversionForm( void );
    CBulkConversionController* GetBulkConversionController( void ) const;

    /** The library sync panel (a window, made on first use): the online stores of the service with their plan, sync and progress */
    bool ShowLibrarySync( void );

    CLibrarySyncForm* GetLibrarySyncForm( void );
    CLibrarySyncController* GetLibrarySyncController( void ) const;

    /** The scope of the browser as bulk conversion settings: the selected assets, else the folder, else the pack of the sources tab */
    CBulkConversionSettings MakeBulkScopeOfBrowser( void ) const;

    /** Opens what a finished job made (its result folder) in the content browser */
    bool OpenJobResult( const CORE::CString& jobId );

    /**
     *  A redo action (Re-index, Re-analyse, Re-import, Re-convert, Re-thumbnail) on the pack that is in the resource view (the index steps run
     *  here as local jobs) or the pack that is selected in the sources tab (a job of the service); false when there is no pack
     */
    bool RedoPack( CWorkbenchRedoActions::ERedoAction action );

    /** A redo action as a job of the service for a pack of the store, with the asset globs of its scope (empty: the whole pack) */
    bool RedoStorePack( CWorkbenchRedoActions::ERedoAction action, const CORE::CString& packId, const CORE::CString::StringVector& assetGlobs );

    /** A local index job on a folder of the VFS with the redo of the action, registered as a job */
    bool RedoFolder( CWorkbenchRedoActions::ERedoAction action, const CORE::CString& vfsFolder );

    private:

    void ShowResourceFolderSummary( const CORE::CString& path );
    void AppendResourceInspector( CORE::CDataNode& data ) const;

    private:

    CAssetMounter& m_mounter;
    GUI::CGUIContext* m_context;
    CAssetTypeResolver m_resolver;
    CAssetPackSummary m_packSummary;
    CReportLogger m_logger;
    GUI::CActivityService m_activity;
    CWorkbenchJobRegistry m_jobRegistry;
    CAssetLoader m_loader;
    CWorkbenchStage m_stage;
    COrbitController m_controller;
    CModelView m_model;
    CMaterialView m_material;
    CTextureView m_texture;
    CSceneView m_scene;
    CParticlePreview m_particles;
    bool m_particleClip;
    UInt32 m_particleTimeInMs;
    UInt32 m_particleInspectorMs;
    CThumbnailProvider* m_thumbnails;
    CShellFrameListener* m_frameListener;

    GUI::CWidget* m_root;
    GUI::CWidget* m_menuBar;
    UInt32 m_menuBarHeight;
    bool m_menuBarHeightKnown;
    GUI::CDeveloperConsoleForm* m_console;
    GUI::CWidget* m_leftTabs;
    GUI::CWidget* m_centerTabs;
    GUI::CContentBrowserForm* m_browser;
    GUI::CContentBrowserForm* m_sceneBrowser;
    GUI::CViewportForm* m_viewport;
    GUI::CTimelineForm* m_timeline;
    GUI::CPropertyInspectorForm* m_inspector;
    GUI::CImageViewForm* m_imageView;
    GUI::CImageViewForm* m_uvView;
    GUI::CReportViewForm* m_report;
    GUI::CTransferQueueForm* m_transfers;
    GUI::CActivityIndicatorForm* m_activityIndicator;
    GUI::CWidget* m_jobSummaryButton;
    GUI::CWidget* m_activityHost;
    GUI::CWidget* m_statusHost;
    GUI::CWidget* m_jobsHost;
    CWorkbenchJobsForm* m_jobsForm;
    CBulkConversionForm* m_bulkForm;
    CBulkConversionController* m_bulkController;
    CLibrarySyncForm* m_syncForm;
    CLibrarySyncController* m_syncController;
    UInt32 m_jobSummaryWaitMs;
    CWorkbenchJobSummary m_shownSummary;
    GUI::CWidget* m_sourcesHost;
    CAssetSourcesPane* m_sources;
    CWorkbenchSignIn* m_signIn;
    CWorkbenchStoreConnection* m_storeConnection;
    CWorkbenchCredentials* m_credentials;
    AUTH::CAuthManager* m_authManager;
    GUI::COpenActionStripForm* m_openStrip;
    GUI::CWidget* m_emptyState;
    GUI::CFileSystemDialogForm* m_folderDialog;
    GUI::CFileSystemDialogForm* m_archiveDialog;
    GUI::CFileSystemDialogForm* m_fileDialog;
    GUI::CRecentLocations m_ownRecent;
    GUI::CRecentLocations* m_recent;
    bool m_anythingOpened;
    bool m_asyncOpen;
    bool m_openStepFailed;
    CORE::CString m_openingPath;
    CORE::CString m_browserRoot;

    IMAGE::CImage m_uvImage;
    EAssetKind m_shownKind;
    CORE::CString m_shownPath;
    bool m_stageBound;
    bool m_initialized;
    bool m_inspectorStale;
    UInt32 m_idleInMs;
    GUCEF::vector< CORE::CString > m_uvChoices;

    GUI::CSplitter* m_mainSplit;
    GUI::CSplitter* m_sideSplit;
    GUI::CSplitter* m_viewSplit;
    GUI::CFoldablePanel* m_leftPanel;
    GUI::CFoldablePanel* m_rightPanel;
    GUI::CLayoutManager m_ownLayout;
    GUI::CLayoutManager* m_layout;
    GUI::CLayoutState m_beforePreview;
    GUI::CKeyboardShortcutMap m_shortcuts;
    CViewPresetTracker m_viewPreset;
    bool m_updatingViewLabel;
    CORE::CString m_defaultViewPresets[ ASSETKIND_COUNT ];
    CShowcaseView m_showcase;
    CWorkbenchPreferences m_ownPreferences;
    CWorkbenchPreferences* m_preferences;
    Int32 m_presentationOverride;
    CPackPresentation m_presentation;
    bool m_hasPresentation;
    CORE::CString m_packRoot;
    CORE::CString m_packKey;
    CORE::CString m_packSource;
    CORE::CString m_packCacheFolder;
    GUCEF::vector< CPackPresentation::EKind > m_offered;
    bool m_toggleForMaterial;
    bool m_presentationJobActive;
    CPackPresentation::EKind m_presentationJobKind;
    UInt32 m_presentationRetries;
    UInt32 m_presentationMaxAssets;
    UInt32 m_autoCaptureFrames;
    CAutoThumbnailStore m_autoThumbnails;
    UInt32 m_autoCaptureCountdownInMs;
    bool m_autoCapturePending;
    CORE::CString m_autoCaptureKey;
    UInt32 m_navigatingItself;
    UInt64 m_presentationStartTick;
    UInt32 m_poolSerial;
    CORE::CString m_kindOverride;
    CPackAssetConverter::SRequest m_conversion;
    bool m_hasConversion;
    CAssetLoader m_indexLoader;
    CORE::TFileSystemAccessPtr m_resourceViewHold;
    CResourceViewFileSystemAccess* m_resourceView;
    Int32 m_resourceViewOverride;
    CORE::CString m_resourceViewPack;
    CORE::CString m_indexingFolder;
    bool m_indexAutoSwitch;
    ASSETCONVERTER::CPackIndexRunResult m_lastIndexResult;
    ASSETCONVERTER::CPackIndexResource m_selectedResource;
    CORE::CString m_selectedResourcePath;
    bool m_hasSelectedResource;
    CORE::CString m_resourceStartPath;
    CResourceViewThumbnailProvider* m_resourceThumbs;
    CAssetCapture* m_capture;
    CORE::CString m_sourceText;
    CCookedOutputResolver::SCookedOutput m_cooked;
    bool m_hasCooked;
    CORE::CString m_cookedFor;
    bool m_cookPending;
    CORE::CString m_cookPendingPath;
    bool m_cookForPresentation;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHSHELL_H ? */
