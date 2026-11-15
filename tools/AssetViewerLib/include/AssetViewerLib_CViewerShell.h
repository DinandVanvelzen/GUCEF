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

#ifndef ASSETVIEWERLIB_CASSETTYPERESOLVER_H
#include "AssetViewerLib_CAssetTypeResolver.h"
#define ASSETVIEWERLIB_CASSETTYPERESOLVER_H
#endif /* ASSETVIEWERLIB_CASSETTYPERESOLVER_H ? */

#ifndef ASSETVIEWERLIB_CASSETMOUNTER_H
#include "AssetViewerLib_CAssetMounter.h"
#define ASSETVIEWERLIB_CASSETMOUNTER_H
#endif /* ASSETVIEWERLIB_CASSETMOUNTER_H ? */

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
 *    - left, in tabs: the content browser of the VFS (with thumbnails of the assets), and the scene browser (the node tree of
 *      a scene, which is a content browser over CSceneTreeFileSystemAccess);
 *    - in the middle, in tabs: the 3D view (a viewport form on the render texture of the stage, and the time line of the
 *      animation clips below it), the texture (an image view form) and the UV layout of a sub mesh (a second image view form);
 *    - right: the property inspector, with the statistics and the editable properties of what is shown;
 *    - below: the report view with the warnings and errors that the viewer, Ogre and the plugins logged.
 *  Opening an asset (OpenAsset(), a double click or a selection in the browser) chooses the view by the kind of the asset. The
 *  render texture of the stage is drawn in a frame listener of Ogre, when the camera, the animation or a switch changed it.
 *
 *  Extension points for later: the reports of the converter (a CReportViewForm per report) and the intermediate with its sidecar
 *  (another asset kind in OpenAsset() with its own view and inspector data) fit in the same panels.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CViewerShell : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CViewerShell > TEventCallback;

    explicit CViewerShell( CAssetMounter& mounter );

    virtual ~CViewerShell() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Builds the forms in the context. 'capture' (not owned, it has to stay as long as the shell) makes the thumbnails of the
     *  content browser, with the cache in 'thumbnailCacheFolder' when that is not empty; without it the browser lists types only
     */
    bool Initialize( GUI::CGUIContext& context, CAssetCapture* capture, const CORE::CString& thumbnailCacheFolder );

    void Shutdown( void );

    bool IsInitialized( void ) const;

    /** Shows an asset of the mounts in the view that suits its kind; false when it cannot be shown (the reason is in the report) */
    bool OpenAsset( const CORE::CString& vfsPath );

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
    CThumbnailProvider* GetThumbnailProvider( void ) const;

    CViewerStage& GetStage( void );
    COrbitController& GetController( void );
    CModelView& GetModelView( void );
    CMaterialView& GetMaterialView( void );
    CTextureView& GetTextureView( void );
    CSceneView& GetSceneView( void );

    /** Which tab of the middle is in front: 0 the 3D view, 1 the texture, 2 the UV layout */
    Int32 GetCenterTab( void ) const;

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
    void FillTimeline( void );
    void ReportProblem( const CORE::CString& asset, const CORE::CString& message );

    void OnBrowserSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnBrowserItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSceneFolderChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSceneSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnToggleChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSurfaceResized( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnClipSelected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTimeChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPropertyChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

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
