/*
 *  AssetViewerLib: the shell of the viewer: the asset tool forms of gucefGUI composed into the content browser, the views, the inspector and the report
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

#ifndef ASSETVIEWERLIB_CVIEWERSHELL_H
#define ASSETVIEWERLIB_CVIEWERSHELL_H

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

#ifndef ASSETVIEWERLIB_CASSETSOURCESPANE_H
#include "AssetViewerLib_CAssetSourcesPane.h"
#define ASSETVIEWERLIB_CASSETSOURCESPANE_H
#endif /* ASSETVIEWERLIB_CASSETSOURCESPANE_H ? */

#ifndef ASSETVIEWERLIB_CVIEWERSIGNIN_H
#include "AssetViewerLib_CViewerSignIn.h"
#define ASSETVIEWERLIB_CVIEWERSIGNIN_H
#endif /* ASSETVIEWERLIB_CVIEWERSIGNIN_H ? */

#ifndef ASSETVIEWERLIB_CASSETTYPERESOLVER_H
#include "AssetViewerLib_CAssetTypeResolver.h"
#define ASSETVIEWERLIB_CASSETTYPERESOLVER_H
#endif /* ASSETVIEWERLIB_CASSETTYPERESOLVER_H ? */

#ifndef ASSETVIEWERLIB_CASSETMOUNTER_H
#include "AssetViewerLib_CAssetMounter.h"
#define ASSETVIEWERLIB_CASSETMOUNTER_H
#endif /* ASSETVIEWERLIB_CASSETMOUNTER_H ? */

#ifndef ASSETVIEWERLIB_CASSETOPENER_H
#include "AssetViewerLib_CAssetOpener.h"
#define ASSETVIEWERLIB_CASSETOPENER_H
#endif /* ASSETVIEWERLIB_CASSETOPENER_H ? */

#ifndef ASSETVIEWERLIB_CASSETCAPTURE_H
#include "AssetViewerLib_CAssetCapture.h"
#define ASSETVIEWERLIB_CASSETCAPTURE_H
#endif /* ASSETVIEWERLIB_CASSETCAPTURE_H ? */

#ifndef ASSETVIEWERLIB_CTHUMBNAILPROVIDER_H
#include "AssetViewerLib_CThumbnailProvider.h"
#define ASSETVIEWERLIB_CTHUMBNAILPROVIDER_H
#endif /* ASSETVIEWERLIB_CTHUMBNAILPROVIDER_H ? */

#ifndef ASSETVIEWERLIB_CREPORTLOGGER_H
#include "AssetViewerLib_CReportLogger.h"
#define ASSETVIEWERLIB_CREPORTLOGGER_H
#endif /* ASSETVIEWERLIB_CREPORTLOGGER_H ? */

#ifndef ASSETVIEWERLIB_CMODELVIEW_H
#include "AssetViewerLib_CModelView.h"
#define ASSETVIEWERLIB_CMODELVIEW_H
#endif /* ASSETVIEWERLIB_CMODELVIEW_H ? */

#ifndef ASSETVIEWERLIB_CMATERIALVIEW_H
#include "AssetViewerLib_CMaterialView.h"
#define ASSETVIEWERLIB_CMATERIALVIEW_H
#endif /* ASSETVIEWERLIB_CMATERIALVIEW_H ? */

#ifndef ASSETVIEWERLIB_CTEXTUREVIEW_H
#include "AssetViewerLib_CTextureView.h"
#define ASSETVIEWERLIB_CTEXTUREVIEW_H
#endif /* ASSETVIEWERLIB_CTEXTUREVIEW_H ? */

#ifndef ASSETVIEWERLIB_CSCENEVIEW_H
#include "AssetViewerLib_CSceneView.h"
#define ASSETVIEWERLIB_CSCENEVIEW_H
#endif /* ASSETVIEWERLIB_CSCENEVIEW_H ? */

#ifndef ASSETVIEWERLIB_CPARTICLEPREVIEW_H
#include "AssetViewerLib_CParticlePreview.h"
#define ASSETVIEWERLIB_CPARTICLEPREVIEW_H
#endif /* ASSETVIEWERLIB_CPARTICLEPREVIEW_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

class CShellFrameListener;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The viewer as a window of a GUI context. Everything on it is one of the asset tool forms of gucefGUI, placed in panels:
 *    - left, in tabs: the content browser of the VFS (with thumbnails of the assets), the scene browser (the node tree of
 *      a scene, which is a content browser over CSceneTreeFileSystemAccess) and the asset sources (the primary store and the online
 *      stores, see CAssetSourcesPane; made by InitializeSources());
 *    - in the middle, in tabs: the 3D view (a viewport form on the render texture of the stage, and the time line of the
 *      animation clips below it), the texture (an image view form) and the UV layout of a sub mesh (a second image view form);
 *    - right: the property inspector, with the statistics and the editable properties of what is shown;
 *    - below: the report view with the warnings and errors that the viewer, Ogre and the plugins logged, and beside it the transfer
 *      queue with the downloads and the ingests of the asset sources.
 *  Opening an asset (OpenAsset(), a double click or a selection in the browser) chooses the view by the kind of the asset. The
 *  render texture of the stage is drawn in a frame listener of Ogre, when the camera, the animation or a switch changed it.
 *
 *  Opening: nothing is open when the viewer starts, so the content tab has an action strip at its top (CAssetOpener has the
 *  logic, the file system dialogs of gucefGUI the choosing): "Open folder...", "Open archive or pack..." and "Open file...", and a
 *  list of the places that were opened lately. The content browser has two roots: "Local files", the file system of the
 *  computer, where the viewer starts, and "VFS", the folders and packs that are mounted. A folder, archive or pack that is opened
 *  is mounted and shown in the VFS root; a file is opened in the view that suits it. Ctrl+O shows the dialog for a file and
 *  Ctrl+Shift+O the one for a folder. While nothing is open the 3D view says how to open something.
 *
 *  Extension points for later: the reports of the converter (a CReportViewForm per report) and the intermediate with its sidecar
 *  (another asset kind in OpenAsset() with its own view and inspector data) fit in the same panels.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CViewerShell : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CViewerShell > TEventCallback;

    /** The display names of the roots of the content browser */
    static const CORE::CString LocalFilesRootName;
    static const CORE::CString VfsRootName;

    /** The ids of the actions of the open strip */
    static const CORE::CString ActionOpenFolder;
    static const CORE::CString ActionOpenArchive;
    static const CORE::CString ActionOpenFile;

    explicit CViewerShell( CAssetMounter& mounter );

    virtual ~CViewerShell() GUCEF_VIRTUAL_OVERRIDE;

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
    void SetCredentials( CViewerCredentials* credentials );

    /** The manager of gucefAUTH that the stores that sign in with a provider of it use (not owned); before or after InitializeSources() */
    void SetAuthManager( AUTH::CAuthManager* manager );

    /** Shows an asset of the mounts in the view that suits its kind; false when it cannot be shown (the reason is in the report) */
    bool OpenAsset( const CORE::CString& vfsPath );

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

    /** The keys of the viewer: Ctrl+O shows the open file dialog, Ctrl+Shift+O the open folder dialog; true when the key was one of them */
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
    CViewerSignIn* GetSignIn( void ) const;
    CThumbnailProvider* GetThumbnailProvider( void ) const;

    CViewerStage& GetStage( void );
    COrbitController& GetController( void );
    CModelView& GetModelView( void );
    CMaterialView& GetMaterialView( void );
    CTextureView& GetTextureView( void );
    CSceneView& GetSceneView( void );

    /** The particle systems of the scene or particle script shown, played by the time line ("particles" clip) */
    CParticlePreview& GetParticlePreview( void );

    /** Which tab of the middle is in front: 0 the 3D view, 1 the texture, 2 the UV layout */
    Int32 GetCenterTab( void ) const;

    /** Which tab of the left side is in front: 0 the content, 1 the scene, 2 the asset sources */
    Int32 GetLeftTab( void ) const;
    void ShowLeftTab( Int32 tab );

    /** Whether the render texture is bound to the surface of the viewport form (needs the GUI driver ImGuiOgre) */
    bool IsStageBound( void ) const;

    private:

    CViewerShell( const CViewerShell& src );
    CViewerShell& operator=( const CViewerShell& src );

    bool BuildLayout( void );
    void UnloadViews( void );
    void BindStage( void );
    void SetModelToggles( bool forMaterial );
    void SetInspector( const CORE::CDataNode& data, const GUI::CPropertyInspectorForm::TDescriptorMap& descriptors );
    void RefreshInspector( void );
    void ShowCenterTab( Int32 tab );
    void ShowUvLayout( Int32 subMesh );
    bool ShowModel( const CORE::CString& vfsPath );
    bool ShowTexture( const CORE::CString& vfsPath );
    bool ShowMaterial( const CORE::CString& vfsPath );
    bool ShowScene( const CORE::CString& vfsPath );
    bool ShowSkeleton( const CORE::CString& vfsPath );
    bool ShowParticles( const CORE::CString& vfsPath );
    void FillTimeline( void );
    void ReportProblem( const CORE::CString& asset, const CORE::CString& message );
    void UpdateEmptyState( void );
    bool ShowOpenDialog( GUI::CFileSystemDialogForm*& dialog, const CORE::CString& kind );
    GUI::CFileSystemDialogForm* MakeOpenDialog( const CORE::CString& kind );
    void PrepareOpenDialog( GUI::CFileSystemDialogForm& dialog );
    bool IsOpenDialogShown( void ) const;
    bool OpenVfsSelection( const CORE::CString& vfsPath );

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
    void OnSourcesSignInRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSourcesSignInSettingsRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSourcesSessionExpired( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnOpenActionRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnOpenRecentChosen( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnOpenDialogAccepted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRootKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    CAssetMounter& m_mounter;
    GUI::CGUIContext* m_context;
    CAssetTypeResolver m_resolver;
    CReportLogger m_logger;
    CViewerStage m_stage;
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
    GUI::CWidget* m_sourcesHost;
    CAssetSourcesPane* m_sources;
    CViewerSignIn* m_signIn;
    CViewerCredentials* m_credentials;
    AUTH::CAuthManager* m_authManager;
    GUI::COpenActionStripForm* m_openStrip;
    GUI::CWidget* m_emptyState;
    GUI::CFileSystemDialogForm* m_folderDialog;
    GUI::CFileSystemDialogForm* m_archiveDialog;
    GUI::CFileSystemDialogForm* m_fileDialog;
    GUI::CRecentLocations m_ownRecent;
    GUI::CRecentLocations* m_recent;
    bool m_anythingOpened;
    CORE::CString m_browserRoot;

    IMAGE::CImage m_uvImage;
    EAssetKind m_shownKind;
    CORE::CString m_shownPath;
    bool m_stageBound;
    bool m_initialized;
    bool m_inspectorStale;
    UInt32 m_idleInMs;
    GUCEF::vector< CORE::CString > m_uvChoices;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CVIEWERSHELL_H ? */
