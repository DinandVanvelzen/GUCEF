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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <Ogre.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#include "gucefCORE_CNativeFileSystemAccess.h"
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */

#ifndef GUCEF_GUI_CFILEOPENDIALOGFORM_H
#include "gucefGUI_CFileOpenDialogForm.h"
#define GUCEF_GUI_CFILEOPENDIALOGFORM_H
#endif /* GUCEF_GUI_CFILEOPENDIALOGFORM_H ? */

#ifndef GUCEF_GUI_CDIRECTORYOPENDIALOGFORM_H
#include "gucefGUI_CDirectoryOpenDialogForm.h"
#define GUCEF_GUI_CDIRECTORYOPENDIALOGFORM_H
#endif /* GUCEF_GUI_CDIRECTORYOPENDIALOGFORM_H ? */

#ifndef GUCEF_VFS_CVFSFILESYSTEMACCESS_H
#include "gucefVFS_CVfsFileSystemAccess.h"
#define GUCEF_VFS_CVFSFILESYSTEMACCESS_H
#endif /* GUCEF_VFS_CVFSFILESYSTEMACCESS_H ? */

#ifndef GUCEF_GUI_CTABCONTROL_H
#include "gucefGUI_CTabControl.h"
#define GUCEF_GUI_CTABCONTROL_H
#endif /* GUCEF_GUI_CTABCONTROL_H ? */

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

#ifndef GUIDRIVERIMGUIOGRE_CIMGUIOGRERENDERCONTEXT_H
#include "guidriverImGuiOgre_CImGuiOgreRenderContext.h"
#define GUIDRIVERIMGUIOGRE_CIMGUIOGRERENDERCONTEXT_H
#endif /* GUIDRIVERIMGUIOGRE_CIMGUIOGRERENDERCONTEXT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CWEBSESSIONSIGNIN_H
#include "gucefASSETSTORE_CWebSessionSignIn.h"
#define GUCEF_ASSETSTORE_CWEBSESSIONSIGNIN_H
#endif /* GUCEF_ASSETSTORE_CWEBSESSIONSIGNIN_H ? */

#include "AssetViewerLib_CViewerShell.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The frame listener of Ogre that gives the shell its turn at the start of every frame */
class CShellFrameListener : public Ogre::FrameListener
{
    public:

    explicit CShellFrameListener( CViewerShell& shell ) : m_shell( shell ) {}

    virtual bool frameStarted( const Ogre::FrameEvent& evt ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_shell.OnOgreFrameStarted( (UInt32) ( evt.timeSinceLastFrame * 1000.0f ) );
        return true;
    }

    private:

    CViewerShell& m_shell;
};

namespace {

const UInt32 TimelineHeight = 120;

const char* const FolderDialogKind = "folder";
const char* const ArchiveDialogKind = "archive";
const char* const FileDialogKind = "file";

enum ECenterTab
{
    CENTERTAB_3D = 0 ,
    CENTERTAB_IMAGE  ,
    CENTERTAB_UV
};

const char* const ToggleDisplay = "display";
const char* const ToggleLighting = "lighting";
const char* const ToggleView = "view";
const char* const ToggleGrid = "grid";
const char* const ToggleAxes = "axes";
const char* const ToggleBounds = "bounds";
const char* const ToggleBones = "bones";
const char* const ToggleUv = "uv";
const char* const ToggleShape = "shape";
const char* const ToggleUpAxis = "up";
const char* const TogglePlacement = "place";

GUI::CWidget*
MakePanel( GUI::CGUIContext& context, const CORE::CString& name, GUI::CWidget* parent, const char* position, const char* size )
{
    GUI::CWidget* panel = context.CreateWidget( "Panel", name, parent );
    if ( GUCEF_NULL == panel )
        return GUCEF_NULL;
    panel->SetProperty( GUI::EWidgetProperty_Position, position );
    panel->SetProperty( GUI::EWidgetProperty_Size, size );
    panel->SetProperty( GUI::EWidgetProperty_BackgroundColour, "0,0,0,0" );
    return panel;
}

GUI::CWidget*
MakeTabs( GUI::CGUIContext& context, const CORE::CString& name, GUI::CWidget* parent, const char* position, const char* size, const CORE::CString& titles, UInt32 pageCount, GUI::CWidget** pages )
{
    GUI::CWidget* tabs = context.CreateWidget( "TabControl", name, parent );
    if ( GUCEF_NULL == tabs )
        return GUCEF_NULL;
    tabs->SetProperty( GUI::EWidgetProperty_Position, position );
    tabs->SetProperty( GUI::EWidgetProperty_Size, size );
    tabs->SetProperty( GUI::EWidgetProperty_Tabs, titles );
    // a title wider than its tab (a narrow window, a larger font) is shortened with "...", its tooltip the whole title
    tabs->SetProperty( GUI::EWidgetProperty_TextOverflow, "Ellipsis" );
    for ( UInt32 i=0; i<pageCount; ++i )
        pages[ i ] = context.CreateWidget( "TabContentPane", name + "Page" + CORE::UInt32ToString( i ), tabs );
    return tabs;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CViewerShell::LocalFilesRootName = "Local files";
const CORE::CString CViewerShell::VfsRootName = "VFS";
const CORE::CString CViewerShell::ActionOpenFolder = "folder";
const CORE::CString CViewerShell::ActionOpenArchive = "archive";
const CORE::CString CViewerShell::ActionOpenFile = "file";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CViewerShell::CViewerShell( CAssetMounter& mounter )
    : CORE::CObserver()
    , m_mounter( mounter )
    , m_context( GUCEF_NULL )
    , m_resolver()
    , m_logger()
    , m_stage( "Viewer" )
    , m_controller()
    , m_model( m_stage )
    , m_material( m_stage )
    , m_texture()
    , m_scene( m_stage )
    , m_particles()
    , m_particleClip( false )
    , m_particleTimeInMs( 0 )
    , m_particleInspectorMs( 0 )
    , m_thumbnails( GUCEF_NULL )
    , m_frameListener( GUCEF_NULL )
    , m_root( GUCEF_NULL )
    , m_leftTabs( GUCEF_NULL )
    , m_centerTabs( GUCEF_NULL )
    , m_browser( GUCEF_NULL )
    , m_sceneBrowser( GUCEF_NULL )
    , m_viewport( GUCEF_NULL )
    , m_timeline( GUCEF_NULL )
    , m_inspector( GUCEF_NULL )
    , m_imageView( GUCEF_NULL )
    , m_uvView( GUCEF_NULL )
    , m_report( GUCEF_NULL )
    , m_transfers( GUCEF_NULL )
    , m_sourcesHost( GUCEF_NULL )
    , m_sources( GUCEF_NULL )
    , m_signIn( GUCEF_NULL )
    , m_credentials( GUCEF_NULL )
    , m_authManager( GUCEF_NULL )
    , m_openStrip( GUCEF_NULL )
    , m_emptyState( GUCEF_NULL )
    , m_folderDialog( GUCEF_NULL )
    , m_archiveDialog( GUCEF_NULL )
    , m_fileDialog( GUCEF_NULL )
    , m_ownRecent()
    , m_recent( &m_ownRecent )
    , m_anythingOpened( false )
    , m_browserRoot()
    , m_uvImage()
    , m_shownKind( ASSETKIND_OTHER )
    , m_shownPath()
    , m_stageBound( false )
    , m_initialized( false )
    , m_inspectorStale( false )
    , m_idleInMs( 0 )
    , m_uvChoices()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CViewerShell::~CViewerShell()
{GUCEF_TRACE;

    Shutdown();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::BuildLayout( void )
{GUCEF_TRACE;

    m_root = m_context->CreateWidget( "Panel", "AssetViewer", GUCEF_NULL );
    if ( GUCEF_NULL == m_root )
        return false;
    m_root->SetProperty( GUI::EWidgetProperty_Size, "1,0 1,0" );
    m_root->SetProperty( GUI::EWidgetProperty_BackgroundColour, "30,32,40,255" );

    // left: the browsers of the files and of the nodes of a scene
    GUI::CWidget* leftPages[ 3 ] = { GUCEF_NULL, GUCEF_NULL, GUCEF_NULL };
    m_leftTabs = MakeTabs( *m_context, "LeftTabs", m_root, "0,0 0,0", "0.28,0 0.74,0", "Content\nScene\nSources", 3, leftPages );
    // middle: the 3D view with its time line, the texture, and the UV layout
    GUI::CWidget* centerPages[ 3 ] = { GUCEF_NULL, GUCEF_NULL, GUCEF_NULL };
    m_centerTabs = MakeTabs( *m_context, "CenterTabs", m_root, "0.28,0 0,0", "0.48,0 0.74,0", "3D\nTexture\nUV layout", 3, centerPages );
    GUI::CWidget* inspectorHost = MakePanel( *m_context, "InspectorHost", m_root, "0.76,0 0,0", "0.24,0 0.74,0" );
    GUI::CWidget* reportHost = MakePanel( *m_context, "ReportHost", m_root, "0,0 0.74,0", "0.62,0 0.26,0" );
    GUI::CWidget* transferHost = MakePanel( *m_context, "TransferHost", m_root, "0.62,0 0.74,0", "0.38,0 0.26,0" );
    if ( GUCEF_NULL == m_leftTabs || GUCEF_NULL == m_centerTabs || GUCEF_NULL == inspectorHost || GUCEF_NULL == reportHost || GUCEF_NULL == transferHost )
        return false;
    for ( UInt32 i=0; i<3; ++i )
        if ( GUCEF_NULL == leftPages[ i ] )
            return false;
    for ( UInt32 i=0; i<2; ++i )
        if ( GUCEF_NULL == leftPages[ i ] )
            return false;
    for ( UInt32 i=0; i<3; ++i )
        if ( GUCEF_NULL == centerPages[ i ] )
            return false;

    const CORE::CString timelineSize = "1,0 0," + CORE::UInt32ToString( TimelineHeight );
    const CORE::CString timelinePosition = "0,0 1,-" + CORE::UInt32ToString( TimelineHeight );
    const CORE::CString viewportSize = "1,0 1,-" + CORE::UInt32ToString( TimelineHeight );
    GUI::CWidget* viewportHost = MakePanel( *m_context, "ViewportHost", centerPages[ CENTERTAB_3D ], "0,0 0,0", viewportSize.C_String() );
    GUI::CWidget* timelineHost = MakePanel( *m_context, "TimelineHost", centerPages[ CENTERTAB_3D ], timelinePosition.C_String(), timelineSize.C_String() );
    GUI::CWidget* imageHost = MakePanel( *m_context, "ImageHost", centerPages[ CENTERTAB_IMAGE ], "0,0 0,0", "1,0 1,0" );
    GUI::CWidget* uvHost = MakePanel( *m_context, "UvHost", centerPages[ CENTERTAB_UV ], "0,0 0,0", "1,0 1,0" );
    // the content tab: the strip with the ways to open something, and the browser below it
    const CORE::CString stripHeight = CORE::UInt32ToString( GUI::COpenActionStripForm::GetPreferredHeight() );
    const CORE::CString stripSize = "1,0 0," + stripHeight;
    const CORE::CString browserPosition = "0,0 0," + stripHeight;
    const CORE::CString browserSize = "1,0 1,-" + stripHeight;
    GUI::CWidget* openHost = MakePanel( *m_context, "OpenStripHost", leftPages[ 0 ], "0,0 0,0", stripSize.C_String() );
    GUI::CWidget* browserHost = MakePanel( *m_context, "BrowserHost", leftPages[ 0 ], browserPosition.C_String(), browserSize.C_String() );
    GUI::CWidget* sceneHost = MakePanel( *m_context, "SceneHost", leftPages[ 1 ], "0,0 0,0", "1,0 1,0" );
    m_sourcesHost = MakePanel( *m_context, "SourcesHost", leftPages[ 2 ], "0,0 0,0", "1,0 1,0" );
    if ( GUCEF_NULL == openHost || GUCEF_NULL == m_sourcesHost || GUCEF_NULL == viewportHost || GUCEF_NULL == timelineHost || GUCEF_NULL == imageHost || GUCEF_NULL == uvHost || GUCEF_NULL == browserHost || GUCEF_NULL == sceneHost )
        return false;

    m_browser = GUCEF_NEW GUI::CContentBrowserForm( m_context );
    m_sceneBrowser = GUCEF_NEW GUI::CContentBrowserForm( m_context );
    m_viewport = GUCEF_NEW GUI::CViewportForm( m_context );
    m_timeline = GUCEF_NEW GUI::CTimelineForm( m_context );
    m_inspector = GUCEF_NEW GUI::CPropertyInspectorForm( m_context );
    m_imageView = GUCEF_NEW GUI::CImageViewForm( m_context );
    m_uvView = GUCEF_NEW GUI::CImageViewForm( m_context );
    m_report = GUCEF_NEW GUI::CReportViewForm( m_context );
    m_transfers = GUCEF_NEW GUI::CTransferQueueForm( m_context );
    m_openStrip = GUCEF_NEW GUI::COpenActionStripForm( m_context );
    m_openStrip->AddAction( ActionOpenFolder, "Open folder...", "Browse a folder of this computer or of the VFS (Ctrl+Shift+O)" );
    m_openStrip->AddAction( ActionOpenArchive, "Open archive or pack...", "Browse a zip, 7z, rar, tar or Unity package as a folder" );
    m_openStrip->AddAction( ActionOpenFile, "Open file...", "Show a model, scene or picture; an archive is browsed (Ctrl+O)" );
    const bool built = m_openStrip->Build( openHost ) && m_browser->Build( browserHost ) && m_sceneBrowser->Build( sceneHost ) && m_viewport->Build( viewportHost ) &&
                       m_timeline->Build( timelineHost ) && m_inspector->Build( inspectorHost ) && m_imageView->Build( imageHost ) &&
                       m_uvView->Build( uvHost ) && m_report->Build( reportHost ) && m_transfers->Build( transferHost );
    if ( !built )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CViewerShell: The forms of the viewer cannot be built" );
        return false;
    }

    // what the 3D view says while nothing is open, over the viewport
    m_emptyState = m_context->CreateWidget( "Label", "EmptyState", viewportHost );
    if ( GUCEF_NULL != m_emptyState )
    {
        m_emptyState->SetProperty( GUI::EWidgetProperty_Position, "0,0 0,0" );
        m_emptyState->SetProperty( GUI::EWidgetProperty_Size, "1,0 1,0" );
        m_emptyState->SetProperty( GUI::EWidgetProperty_BackgroundColour, "0,0,0,0" );
        m_emptyState->SetProperty( GUI::EWidgetProperty_TextColour, "176,182,196,255" );
        m_emptyState->SetProperty( GUI::EWidgetProperty_HTextAlign, "Center" );
        m_emptyState->SetProperty( GUI::EWidgetProperty_VTextAlign, "Center" );
        m_emptyState->SetProperty( GUI::EWidgetProperty_WordWrap, "true" );
        m_emptyState->SetProperty( GUI::EWidgetProperty_Text, GetEmptyStateText() );
    }
    return m_context->ShowRoot( m_root, false );
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::Initialize( GUI::CGUIContext& context, CAssetCapture* capture, const CORE::CString& thumbnailCacheFolder )
{GUCEF_TRACE;

    if ( m_initialized )
        return true;
    m_context = &context;

    // the viewer's own log: warnings and errors of what is loaded
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &m_logger );

    if ( !m_stage.Initialize( 512, 512 ) || !BuildLayout() )
    {
        Shutdown();
        return false;
    }

    m_browser->SetContentTypeResolver( &m_resolver );

    // the files of the computer are where the viewer starts, with the folder of the user as the first folder; the mounts are the second root
    CORE::CString home;
    if ( !CORE::TryResolveSpecialDir( CORE::SPECIALDIR_HOME_DIR, home ) )
        home.Clear();
    m_browser->AddRoot( LocalFilesRootName, CORE::TFileSystemAccessPtr( GUCEF_NEW CORE::CNativeFileSystemAccess( LocalFilesRootName ) ), home );
    m_browser->AddRoot( VfsRootName, CORE::TFileSystemAccessPtr( GUCEF_NEW VFS::CVfsFileSystemAccess( VfsRootName ) ) );
    if ( m_mounter.GetMountCount() > 0 )
    {
        // something was mounted before the viewer started (--mount): that is what is shown
        m_anythingOpened = true;
        m_browser->NavigateTo( VfsRootName, "/" );
    }
    m_openStrip->SetRecentLocations( m_recent );
    if ( GUCEF_NULL != capture )
    {
        m_thumbnails = GUCEF_NEW CThumbnailProvider( m_mounter, *capture );
        m_thumbnails->SetCacheFolder( thumbnailCacheFolder );
        m_browser->SetThumbnailProvider( m_thumbnails );
    }
    // the files of the computer are a list (name, type, size, date), the mounts show thumbnails when they can be made
    m_browserRoot = m_browser->GetCurrentRootName();
    m_browser->SetViewMode( LocalFilesRootName != m_browserRoot && GUCEF_NULL != m_thumbnails ? GUI::CContentBrowserForm::VIEWMODE_THUMBNAILS : GUI::CContentBrowserForm::VIEWMODE_DETAILS );
    m_sceneBrowser->AddRoot( "Scene", m_scene.GetFileSystem() );

    // the reports that came before the report view did
    m_report->AddEntry( GUI::CReportViewForm::SEVERITY_INFO, CORE::CString::Empty, "Asset viewer started", "Renderer: " + CORE::CString( Ogre::Root::getSingleton().getRenderSystem()->getName() ) );
    if ( !m_anythingOpened )
        m_report->AddEntry( GUI::CReportViewForm::SEVERITY_INFO, CORE::CString::Empty, "Nothing is open", GetEmptyStateText() );
    UpdateEmptyState();

    TEventCallback selectionCallback( this, &CViewerShell::OnBrowserSelectionChanged );
    SubscribeTo( m_browser, GUI::CContentBrowserForm::SelectionChangedEvent, selectionCallback );
    TEventCallback folderCallback( this, &CViewerShell::OnBrowserFolderChanged );
    SubscribeTo( m_browser, GUI::CContentBrowserForm::FolderChangedEvent, folderCallback );
    TEventCallback activatedCallback( this, &CViewerShell::OnBrowserItemActivated );
    SubscribeTo( m_browser, GUI::CContentBrowserForm::ItemActivatedEvent, activatedCallback );
    TEventCallback sceneFolderCallback( this, &CViewerShell::OnSceneFolderChanged );
    SubscribeTo( m_sceneBrowser, GUI::CContentBrowserForm::FolderChangedEvent, sceneFolderCallback );
    TEventCallback sceneSelectionCallback( this, &CViewerShell::OnSceneSelectionChanged );
    SubscribeTo( m_sceneBrowser, GUI::CContentBrowserForm::SelectionChangedEvent, sceneSelectionCallback );
    TEventCallback toggleCallback( this, &CViewerShell::OnToggleChanged );
    SubscribeTo( m_viewport, GUI::CViewportForm::ToggleChangedEvent, toggleCallback );
    TEventCallback resizedCallback( this, &CViewerShell::OnSurfaceResized );
    SubscribeTo( m_viewport, GUI::CViewportForm::SurfaceResizedEvent, resizedCallback );
    TEventCallback clipCallback( this, &CViewerShell::OnClipSelected );
    SubscribeTo( m_timeline, GUI::CTimelineForm::ClipSelectedEvent, clipCallback );
    TEventCallback timeCallback( this, &CViewerShell::OnTimeChanged );
    SubscribeTo( m_timeline, GUI::CTimelineForm::TimeChangedEvent, timeCallback );
    TEventCallback playCallback( this, &CViewerShell::OnPlayStateChanged );
    SubscribeTo( m_timeline, GUI::CTimelineForm::PlayStateChangedEvent, playCallback );
    TEventCallback propertyCallback( this, &CViewerShell::OnPropertyChanged );
    SubscribeTo( m_inspector, GUI::CPropertyInspectorForm::PropertyChangedEvent, propertyCallback );
    TEventCallback actionCallback( this, &CViewerShell::OnOpenActionRequested );
    SubscribeTo( m_openStrip, GUI::COpenActionStripForm::ActionRequestedEvent, actionCallback );
    TEventCallback recentCallback( this, &CViewerShell::OnOpenRecentChosen );
    SubscribeTo( m_openStrip, GUI::COpenActionStripForm::RecentChosenEvent, recentCallback );
    TEventCallback keyCallback( this, &CViewerShell::OnRootKeyDown );
    SubscribeTo( m_root, GUI::CWidget::KeyDownEvent, keyCallback );

    m_viewport->SetController( &m_controller );
    m_controller.SetBounds( COrbitController::SVector3(), 1.0f );
    m_stage.ApplyController( m_controller );

    m_frameListener = GUCEF_NEW CShellFrameListener( *this );
    Ogre::Root::getSingleton().addFrameListener( m_frameListener );

    m_initialized = true;
    BindStage();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::Shutdown( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_frameListener )
    {
        if ( GUCEF_NULL != Ogre::Root::getSingletonPtr() )
            Ogre::Root::getSingleton().removeFrameListener( m_frameListener );
        GUCEF_DELETE m_frameListener;
        m_frameListener = GUCEF_NULL;
    }
    UnsubscribeAllFromObserver();
    CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &m_logger );

    if ( GUCEF_NULL != m_viewport )
        m_viewport->SetController( GUCEF_NULL );
    if ( GUCEF_NULL != m_browser )
    {
        m_browser->SetThumbnailProvider( GUCEF_NULL );
        m_browser->SetContentTypeResolver( GUCEF_NULL );
    }
    UnloadViews();
    if ( m_stageBound && GUCEF_NULL != m_viewport && GUCEF_NULL != m_viewport->GetSurface() )
        GUIDRIVERIMGUIOGRE::CImGuiOgreRenderContext::ClearOgreTexture( *m_viewport->GetSurface() );
    m_stageBound = false;

    GUCEF_DELETE m_signIn;          m_signIn = GUCEF_NULL;
    GUCEF_DELETE m_sources;         m_sources = GUCEF_NULL;
    GUCEF_DELETE m_folderDialog;    m_folderDialog = GUCEF_NULL;
    GUCEF_DELETE m_archiveDialog;   m_archiveDialog = GUCEF_NULL;
    GUCEF_DELETE m_fileDialog;      m_fileDialog = GUCEF_NULL;
    GUCEF_DELETE m_openStrip;       m_openStrip = GUCEF_NULL;
    m_emptyState = GUCEF_NULL;
    GUCEF_DELETE m_transfers;       m_transfers = GUCEF_NULL;
    m_sourcesHost = GUCEF_NULL;
    GUCEF_DELETE m_report;          m_report = GUCEF_NULL;
    GUCEF_DELETE m_uvView;          m_uvView = GUCEF_NULL;
    GUCEF_DELETE m_imageView;       m_imageView = GUCEF_NULL;
    GUCEF_DELETE m_inspector;       m_inspector = GUCEF_NULL;
    GUCEF_DELETE m_timeline;        m_timeline = GUCEF_NULL;
    GUCEF_DELETE m_viewport;        m_viewport = GUCEF_NULL;
    GUCEF_DELETE m_sceneBrowser;    m_sceneBrowser = GUCEF_NULL;
    GUCEF_DELETE m_browser;         m_browser = GUCEF_NULL;
    GUCEF_DELETE m_thumbnails;      m_thumbnails = GUCEF_NULL;
    m_root = GUCEF_NULL;
    m_leftTabs = GUCEF_NULL;
    m_centerTabs = GUCEF_NULL;
    m_stage.Shutdown();
    m_context = GUCEF_NULL;
    m_initialized = false;
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_initialized;
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::BindStage( void )
{GUCEF_TRACE;

    if ( !m_initialized || GUCEF_NULL == m_viewport || GUCEF_NULL == m_viewport->GetSurface() )
        return;
    UInt32 width = 0, height = 0;
    m_viewport->GetSurfaceSize( width, height );
    if ( 0 == width || 0 == height )
        return;
    if ( !m_stage.Resize( width, height ) )
        return;
    m_stageBound = GUIDRIVERIMGUIOGRE::CImGuiOgreRenderContext::SetOgreTexture( *m_viewport->GetSurface(), m_stage.GetRenderTexture(), GUI::CRect( 0, 0, (Int32) width, (Int32) height ) );
    m_stage.ApplyController( m_controller );
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::UnloadViews( void )
{GUCEF_TRACE;

    m_particles.Clear();
    m_particleClip = false;
    m_model.Unload();
    m_material.Unload();
    m_scene.Unload();
    m_texture.Unload();
    if ( GUCEF_NULL != m_imageView )
        m_imageView->SetImage( GUCEF_NULL );
    if ( GUCEF_NULL != m_uvView )
        m_uvView->SetImage( GUCEF_NULL );
    m_uvImage.Assign( IMAGE::CImage::TFrameList() );
    if ( GUCEF_NULL != m_timeline )
        m_timeline->ClearClips();
    if ( GUCEF_NULL != m_sceneBrowser && m_shownKind == ASSETKIND_SCENE )
        m_sceneBrowser->Refresh();
    m_shownKind = ASSETKIND_OTHER;
    m_shownPath.Clear();
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::CloseAsset( void )
{GUCEF_TRACE;

    UnloadViews();
    if ( GUCEF_NULL != m_inspector )
        m_inspector->SetDataCopy( CORE::CDataNode() );
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

EAssetKind
CViewerShell::GetShownKind( void ) const
{GUCEF_TRACE;

    return m_shownKind;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CViewerShell::GetShownPath( void ) const
{GUCEF_TRACE;

    return m_shownPath;
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::ReportProblem( const CORE::CString& asset, const CORE::CString& message )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CViewerShell: " << asset << ": " << message );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::SetInspector( const CORE::CDataNode& data, const GUI::CPropertyInspectorForm::TDescriptorMap& descriptors )
{GUCEF_TRACE;

    m_inspector->ClearDescriptors();
    m_inspector->SetDescriptors( descriptors );
    m_inspector->SetDataCopy( data );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::RefreshInspector( void )
{GUCEF_TRACE;

    CORE::CDataNode data;
    GUI::CPropertyInspectorForm::TDescriptorMap descriptors;
    switch ( m_shownKind )
    {
        case ASSETKIND_MODEL:
        {
            m_model.WriteInspectorData( data );
            m_model.GetInspectorDescriptors( descriptors );
            break;
        }
        case ASSETKIND_MATERIAL:
        {
            m_material.WriteInspectorData( data );
            m_material.GetInspectorDescriptors( descriptors );
            break;
        }
        case ASSETKIND_SCENE:
        {
            m_scene.WriteInspectorData( data );
            m_particles.WriteInspectorData( data );
            break;
        }
        case ASSETKIND_PARTICLE:
        {
            data.SetName( "Particles" );
            data.SetAttribute( "Name", m_shownPath );
            m_particles.WriteInspectorData( data );
            break;
        }
        case ASSETKIND_TEXTURE:
        {
            m_texture.WriteInspectorData( data );
            break;
        }
        default:
        {
            return;
        }
    }
    SetInspector( data, descriptors );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::ShowCenterTab( Int32 tab )
{GUCEF_TRACE;

    GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( m_centerTabs );
    if ( GUCEF_NULL != tabs )
        tabs->SetActiveTab( (UInt32) tab );
}

/*-------------------------------------------------------------------------*/

Int32
CViewerShell::GetCenterTab( void ) const
{GUCEF_TRACE;

    GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( m_centerTabs );
    return GUCEF_NULL != tabs ? tabs->GetActiveTab() : -1;
}

/*-------------------------------------------------------------------------*/

Int32
CViewerShell::GetLeftTab( void ) const
{GUCEF_TRACE;

    GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( m_leftTabs );
    return GUCEF_NULL != tabs ? tabs->GetActiveTab() : -1;
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::ShowLeftTab( Int32 tab )
{GUCEF_TRACE;

    GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( m_leftTabs );
    if ( GUCEF_NULL != tabs && tab >= 0 )
        tabs->SetActiveTab( (UInt32) tab );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::SetModelToggles( bool forMaterial )
{GUCEF_TRACE;

    GUI::CViewportForm::TToggleVector toggles;

    GUI::CViewportForm::TToggle display( ToggleDisplay, "Display", "How the surfaces are drawn" );
    for ( UInt32 i=0; i<(UInt32) CModelView::DISPLAYMODE_COUNT; ++i )
        display.choices.push_back( CModelView::GetDisplayModeName( static_cast< CModelView::EDisplayMode >( i ) ) );
    display.choice = 0;
    toggles.push_back( display );

    GUI::CViewportForm::TToggle lighting( ToggleLighting, "Light", "The light preset" );
    for ( UInt32 i=0; i<(UInt32) CViewerStage::LIGHTPRESET_COUNT; ++i )
        lighting.choices.push_back( CViewerStage::GetLightPresetName( static_cast< CViewerStage::ELightPreset >( i ) ) );
    lighting.choice = (Int32) m_stage.GetLightPreset();
    toggles.push_back( lighting );

    GUI::CViewportForm::TToggle view( ToggleView, "View", "Frame the model or look at it from a fixed side (keys 1 to 4, F)" );
    view.choices.push_back( "Frame" );
    view.choices.push_back( "Front" );
    view.choices.push_back( "Side" );
    view.choices.push_back( "Top" );
    view.choices.push_back( "Three quarter" );
    view.choice = 0;
    toggles.push_back( view );

    if ( forMaterial )
    {
        GUI::CViewportForm::TToggle shape( ToggleShape, "Shape", "What the material is shown on" );
        for ( UInt32 i=0; i<(UInt32) CMaterialView::SHAPE_COUNT; ++i )
            shape.choices.push_back( CMaterialView::GetShapeName( static_cast< CMaterialView::EShape >( i ) ) );
        shape.choice = (Int32) m_material.GetShape();
        toggles.push_back( shape );
    }

    toggles.push_back( GUI::CViewportForm::TToggle( ToggleGrid, "Grid", "The grid on the ground", m_stage.IsGridVisible() ) );
    toggles.push_back( GUI::CViewportForm::TToggle( ToggleAxes, "Axes", "The axes of the world", m_stage.IsAxesVisible() ) );
    toggles.push_back( GUI::CViewportForm::TToggle( ToggleBounds, "Bounds", "The bounding box", false ) );
    if ( !forMaterial )
    {
        toggles.push_back( GUI::CViewportForm::TToggle( ToggleBones, "Bones", "The bones of the skeleton", false ) );

        // automatic says what the format of the file gave
        GUI::CViewportForm::TToggle up( ToggleUpAxis, "Up", "The axis of the file that points up; automatic: Z for 3D printing files (stl, 3mf, amf), Y for the others" );
        for ( UInt32 i=0; i<(UInt32) CModelView::UPAXIS_COUNT; ++i )
        {
            const CModelView::EUpAxis upAxis = static_cast< CModelView::EUpAxis >( i );
            CORE::CString name = CModelView::GetUpAxisName( upAxis );
            if ( CModelView::UPAXIS_AUTOMATIC == upAxis && m_model.IsLoaded() )
                name += CORE::CString( " (" ) + CModelView::GetUpAxisName( m_model.GetEffectiveUpAxis() ) + ")";
            up.choices.push_back( name );
        }
        up.choice = (Int32) m_model.GetUpAxis();
        toggles.push_back( up );

        GUI::CViewportForm::TToggle place( TogglePlacement, "Place", "Where the model stands; automatic: 3D printing files on the grid, the others as authored" );
        for ( UInt32 i=0; i<(UInt32) CModelView::PLACEMENT_COUNT; ++i )
        {
            const CModelView::EPlacement placement = static_cast< CModelView::EPlacement >( i );
            CORE::CString name = CModelView::GetPlacementName( placement );
            if ( CModelView::PLACEMENT_AUTOMATIC == placement && m_model.IsLoaded() )
                name += CORE::CString( " (" ) + CModelView::GetPlacementName( m_model.GetEffectivePlacement() ) + ")";
            place.choices.push_back( name );
        }
        place.choice = (Int32) m_model.GetPlacement();
        toggles.push_back( place );

        GUI::CViewportForm::TToggle uv( ToggleUv, "UV", "The texture coordinates of a sub mesh" );
        uv.choices.push_back( "Off" );
        for ( UInt32 i=0; i<m_model.GetSubMeshCount(); ++i )
            uv.choices.push_back( "Submesh " + CORE::UInt32ToString( i ) );
        uv.choice = 0;
        toggles.push_back( uv );
    }
    m_viewport->SetToggles( toggles );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::FillTimeline( void )
{GUCEF_TRACE;

    GUI::CTimelineForm::TClipVector clips;
    const CModelView::TClipVector& modelClips = m_model.GetClips();
    for ( size_t i=0; i<modelClips.size(); ++i )
        clips.push_back( GUI::CTimelineForm::TClip( modelClips[ i ].name, modelClips[ i ].lengthInMs ) );
    m_timeline->SetClips( clips );
    if ( !clips.empty() )
        m_timeline->SelectClip( 0 );
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::OpenAsset( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;
    const CORE::CString path = CAssetMounter::NormalizePath( vfsPath );
    const EAssetKind kind = CAssetTypeResolver::GetKind( path );
    UnloadViews();

    bool shown = false;
    switch ( kind )
    {
        case ASSETKIND_MODEL:       { shown = ShowModel( path ); break; }
        case ASSETKIND_TEXTURE:     { shown = ShowTexture( path ); break; }
        case ASSETKIND_MATERIAL:    { shown = ShowMaterial( path ); break; }
        case ASSETKIND_SCENE:       { shown = ShowScene( path ); break; }
        case ASSETKIND_SKELETON:    { shown = ShowSkeleton( path ); break; }
        case ASSETKIND_PARTICLE:    { shown = ShowParticles( path ); break; }
        default:
        {
            ReportProblem( path, "There is no view for this kind of file" );
            break;
        }
    }
    if ( shown )
    {
        m_shownKind = kind;
        m_shownPath = path;
        m_anythingOpened = true;
        m_report->AddEntry( GUI::CReportViewForm::SEVERITY_INFO, path, "Opened as " + CAssetTypeResolver::GetKindName( kind ) );
    }
    UpdateEmptyState();
    m_stage.MarkDirty();
    return shown;
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::ShowModel( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString relative;
    const CORE::CString group = m_mounter.GetResourceGroup( path, relative );
    if ( group.IsNULLOrEmpty() )
    {
        ReportProblem( path, "The file is not in a mount" );
        return false;
    }
    if ( !m_model.LoadMesh( group, relative ) )
    {
        ReportProblem( path, "The model cannot be loaded" );
        return false;
    }
    m_model.FrameInController( m_controller );
    m_model.Update( m_controller );
    m_stage.ApplyController( m_controller );
    SetModelToggles( false );
    m_shownKind = ASSETKIND_MODEL;
    FillTimeline();
    m_stage.SetWireframe( false );
    RefreshInspector();
    ShowCenterTab( CENTERTAB_3D );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::ShowMaterial( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString relative;
    const CORE::CString group = m_mounter.GetResourceGroup( path, relative );
    if ( group.IsNULLOrEmpty() || !m_material.LoadScript( group, relative ) )
    {
        ReportProblem( path, "The material script cannot be loaded" );
        return false;
    }
    m_material.GetPreview().FrameInController( m_controller );
    m_stage.ApplyController( m_controller );
    SetModelToggles( true );
    m_timeline->ClearClips();
    m_shownKind = ASSETKIND_MATERIAL;
    RefreshInspector();
    ShowCenterTab( CENTERTAB_3D );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::ShowScene( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString relative;
    const CORE::CString group = m_mounter.GetResourceGroup( path, relative );
    if ( group.IsNULLOrEmpty() || !m_scene.Load( group, relative ) )
    {
        ReportProblem( path, "The scene cannot be loaded" );
        return false;
    }
    COrbitController::SVector3 center;
    Float32 radius = 1.0f;
    m_scene.GetBoundingSphere( center, radius );
    m_controller.SetBounds( center, radius );
    m_stage.SetGridUnit( radius > 0.0f ? radius / 4.0f : 0.2f );
    m_stage.ApplyController( m_controller );
    SetModelToggles( true );
    m_timeline->ClearClips();
    if ( m_particles.Collect( m_scene.GetRoot() ) > 0 )
        StartParticleClip();
    m_sceneBrowser->Refresh();
    m_sceneBrowser->NavigateTo( "Scene", "/" );
    m_scene.Select( "/" );
    m_shownKind = ASSETKIND_SCENE;
    RefreshInspector();
    ShowCenterTab( CENTERTAB_3D );
    GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( m_leftTabs );
    if ( GUCEF_NULL != tabs )
        tabs->SetActiveTab( 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::ShowTexture( const CORE::CString& path )
{GUCEF_TRACE;

    if ( !m_texture.Load( path ) )
    {
        ReportProblem( path, "The picture cannot be loaded: " + m_texture.GetInfo().note );
        return false;
    }
    m_imageView->SetFrameMeaning( m_texture.GetInfo().isCube ? GUI::CImageViewForm::FRAMEMEANING_CUBE_FACES : GUI::CImageViewForm::FRAMEMEANING_FRAMES );
    m_imageView->SetImage( m_texture.GetImage() );
    m_imageView->FitToView();
    m_shownKind = ASSETKIND_TEXTURE;
    RefreshInspector();
    ShowCenterTab( CENTERTAB_IMAGE );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::ShowSkeleton( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString relative;
    const CORE::CString group = m_mounter.GetResourceGroup( path, relative );
    if ( group.IsNULLOrEmpty() )
    {
        ReportProblem( path, "The file is not in a mount" );
        return false;
    }
    try
    {
        Ogre::SkeletonPtr skeleton = Ogre::static_pointer_cast< Ogre::Skeleton >( Ogre::SkeletonManager::getSingleton().load( relative.STL_String(), group.STL_String() ) );
        CORE::CDataNode data;
        data.SetName( "Skeleton" );
        data.SetAttribute( "Name", path );
        CORE::CDataNode* counts = data.AddChild( "Statistics" );
        counts->SetAttribute( "Bones", CORE::UInt32ToString( skeleton->getNumBones() ) );
        counts->SetAttribute( "Animations", CORE::UInt32ToString( skeleton->getNumAnimations() ) );
        CORE::CDataNode* bones = data.AddChild( "Bones" );
        for ( unsigned short i=0; i<skeleton->getNumBones(); ++i )
        {
            Ogre::Bone* bone = skeleton->getBone( i );
            bones->SetAttribute( CORE::CString( bone->getName() ), bone->getParent() ? CORE::CString( "child of " ) + bone->getParent()->getName() : CORE::CString( "root" ) );
        }
        CORE::CDataNode* clips = data.AddChild( "Clips" );
        for ( unsigned short i=0; i<skeleton->getNumAnimations(); ++i )
        {
            Ogre::Animation* animation = skeleton->getAnimation( i );
            clips->SetAttribute( CORE::CString( animation->getName() ), GUI::CPropertyRowDescriptor::NumberToText( animation->getLength(), 2 ) + " s" );
        }
        SetInspector( data, GUI::CPropertyInspectorForm::TDescriptorMap() );
        Ogre::SkeletonManager::getSingleton().remove( skeleton );
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        ReportProblem( path, "The skeleton cannot be loaded: " + CORE::CString( e.getDescription() ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::ShowUvLayout( Int32 subMesh )
{GUCEF_TRACE;

    if ( subMesh < 0 )
    {
        m_uvView->SetImage( GUCEF_NULL );
        ShowCenterTab( CENTERTAB_3D );
        return;
    }
    IMAGE::TPixelMapPtr layout;
    if ( !m_model.RenderUvLayout( (UInt32) subMesh, 1024, layout ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CViewerShell: Sub mesh " << subMesh << " has no texture coordinates to show" );
        m_uvView->SetImage( GUCEF_NULL );
        return;
    }
    m_uvImage.Assign( layout );
    m_uvView->SetImage( &m_uvImage );
    m_uvView->FitToView();
    ShowCenterTab( CENTERTAB_UV );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnBrowserSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the files of the computer are opened by a double click: a click on one of them does not mount its folder
    if ( LocalFilesRootName == m_browser->GetCurrentRootName() )
        return;
    const GUI::CContentBrowserForm::TInt32Vector selection = m_browser->GetSelection();
    if ( 1 != selection.size() )
        return;
    const GUI::CContentBrowserForm::TItem& item = m_browser->GetListedItem( (UInt32) selection[ 0 ] );
    if ( item.isDirectory || ASSETKIND_OTHER == CAssetTypeResolver::GetKind( item.path ) )
        return;
    if ( CAssetMounter::NormalizePath( item.path ) == m_shownPath )
        return;
    OpenAsset( item.path );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnBrowserFolderChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // going from the files of the computer to the mounts or back changes how the files are listed; within a root the choice of the user stays
    if ( m_browserRoot == m_browser->GetCurrentRootName() )
        return;
    m_browserRoot = m_browser->GetCurrentRootName();
    m_browser->SetViewMode( LocalFilesRootName != m_browserRoot && GUCEF_NULL != m_thumbnails ? GUI::CContentBrowserForm::VIEWMODE_THUMBNAILS : GUI::CContentBrowserForm::VIEWMODE_DETAILS );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnBrowserItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* path = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == path )
        return;
    if ( LocalFilesRootName == m_browser->GetCurrentRootName() )
    {
        // an archive or pack is entered like a folder, an asset is shown where the browser is
        OpenLocation( path->GetData(), CAssetOpener::OPENKIND_ARCHIVE == CAssetOpener::Classify( path->GetData() ) );
        return;
    }
    if ( CAssetMounter::NormalizePath( path->GetData() ) == m_shownPath )
        return;
    OpenAsset( path->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnSceneFolderChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* path = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == path || ASSETKIND_SCENE != m_shownKind )
        return;
    if ( m_scene.Select( path->GetData() ) )
        RefreshInspector();
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnSceneSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( ASSETKIND_SCENE != m_shownKind )
        return;
    const CORE::CString::StringVector paths = m_sceneBrowser->GetSelectedPaths();
    if ( 1 == paths.size() && m_scene.Select( paths[ 0 ] ) )
        RefreshInspector();
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnToggleChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const GUI::CViewportForm::TToggleChangedEventData* change = static_cast< const GUI::CViewportForm::TToggleChangedEventData* >( eventData );
    if ( GUCEF_NULL == change )
        return;
    const GUI::CViewportForm::TToggleChange& toggle = change->GetData();
    if ( ToggleDisplay == toggle.id )
    {
        // the display modes are of a model (a material is drawn on a shape of Ogre, which takes them as well)
        CModelView& view = ASSETKIND_MATERIAL == m_shownKind ? m_material.GetPreview() : m_model;
        view.SetDisplayMode( static_cast< CModelView::EDisplayMode >( toggle.choice ) );
    }
    else
    if ( ToggleLighting == toggle.id )
        m_stage.SetLightPreset( static_cast< CViewerStage::ELightPreset >( toggle.choice ) );
    else
    if ( ToggleView == toggle.id )
    {
        if ( toggle.choice <= 0 )
            m_controller.Frame();
        else
            m_controller.ApplyPreset( static_cast< COrbitController::EViewPreset >( toggle.choice - 1 ) );
    }
    else
    if ( ToggleGrid == toggle.id )
        m_stage.SetGridVisible( toggle.checked );
    else
    if ( ToggleAxes == toggle.id )
        m_stage.SetAxesVisible( toggle.checked );
    else
    if ( ToggleBounds == toggle.id )
        ( ASSETKIND_MATERIAL == m_shownKind ? m_material.GetPreview() : m_model ).SetBoundsVisible( toggle.checked );
    else
    if ( ToggleBones == toggle.id )
        m_model.SetSkeletonVisible( toggle.checked );
    else
    if ( ToggleUpAxis == toggle.id || TogglePlacement == toggle.id )
    {
        if ( ToggleUpAxis == toggle.id )
            m_model.SetUpAxis( static_cast< CModelView::EUpAxis >( toggle.choice ) );
        else
            m_model.SetPlacement( static_cast< CModelView::EPlacement >( toggle.choice ) );
        // the model moved: look at it again
        m_model.FrameInController( m_controller );
        m_model.Update( m_controller );
        m_stage.ApplyController( m_controller );
    }
    else
    if ( ToggleUv == toggle.id )
        ShowUvLayout( toggle.choice - 1 );
    else
    if ( ToggleShape == toggle.id )
    {
        m_material.SetShape( static_cast< CMaterialView::EShape >( toggle.choice ) );
        m_material.GetPreview().FrameInController( m_controller );
        RefreshInspector();
    }
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnSurfaceResized( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    BindStage();
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnClipSelected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableInt32* index = static_cast< const CORE::TCloneableInt32* >( eventData );
    if ( GUCEF_NULL != index && ASSETKIND_MODEL == m_shownKind )
        m_model.SelectClip( index->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnTimeChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const GUI::CTimelineForm::TTimeChangedEventData* change = static_cast< const GUI::CTimelineForm::TTimeChangedEventData* >( eventData );
    if ( GUCEF_NULL != change && m_particleClip )
    {
        // the particles follow the time line: on while it plays (also across a loop), from the start after a scrub or a
        // change while it does not play
        const UInt32 timeInMs = change->GetData().timeInMs;
        if ( change->GetData().scrubbed || !m_timeline->IsPlaying() )
            m_particles.SetTime( 0.001f * (Float32) timeInMs );
        else
        if ( timeInMs >= m_particleTimeInMs )
            m_particles.Advance( 0.001f * (Float32) ( timeInMs - m_particleTimeInMs ) );
        else
            m_particles.Advance( 0.001f * (Float32) ( m_timeline->GetClipLength() - m_particleTimeInMs + timeInMs ) );
        m_particleTimeInMs = timeInMs;
        m_stage.MarkDirty();
        if ( timeInMs < m_particleInspectorMs || timeInMs >= m_particleInspectorMs + 500 )
        {
            m_particleInspectorMs = timeInMs;
            RefreshInspector();
        }
        return;
    }
    if ( GUCEF_NULL != change && ASSETKIND_MODEL == m_shownKind )
        m_model.SetClipTime( change->GetData().timeInMs );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnPlayStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableBool* playing = static_cast< const CORE::TCloneableBool* >( eventData );
    if ( GUCEF_NULL != playing && m_particleClip )
        m_particles.SetPlaying( playing->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::StartParticleClip( void )
{GUCEF_TRACE;

    GUI::CTimelineForm::TClipVector clips;
    clips.push_back( GUI::CTimelineForm::TClip( "particles", m_particles.GetLengthInMs() ) );
    m_timeline->SetClips( clips );
    m_timeline->SelectClip( 0 );
    m_timeline->SetLooping( true );
    m_particleClip = true;
    m_particleTimeInMs = 0;
    m_particleInspectorMs = 0;
    m_timeline->Play();
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::ShowParticles( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString relative;
    const CORE::CString group = m_mounter.GetResourceGroup( path, relative );
    CORE::CString::StringVector templates;
    if ( group.IsNULLOrEmpty() || !CParticlePreview::ParseParticleScript( group, relative, templates ) )
    {
        ReportProblem( path, "The particle script cannot be loaded (is the Ogre plugin OgrePlugin_ParticleFX loaded?)" );
        return false;
    }
    if ( 0 == m_particles.CreateFromTemplates( m_stage.GetSceneManager(), m_stage.GetContentNode(), templates ) )
    {
        ReportProblem( path, "No particle system of the script can be made" );
        return false;
    }
    m_particles.Advance( 2.0f );
    COrbitController::SVector3 center;
    Float32 radius = 1.0f;
    if ( !m_particles.GetBoundingSphere( center, radius ) )
        center = COrbitController::SVector3();
    m_controller.SetBounds( center, radius );
    m_stage.SetGridUnit( radius > 0.0f ? radius / 4.0f : 0.2f );
    m_stage.ApplyController( m_controller );
    SetModelToggles( true );
    m_timeline->ClearClips();
    StartParticleClip();
    m_particles.SetTime( 0.0f );
    m_shownKind = ASSETKIND_PARTICLE;
    m_shownPath = path;
    RefreshInspector();
    ShowCenterTab( CENTERTAB_3D );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnPropertyChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const GUI::CPropertyInspectorForm::TPropertyChangedEventData* change = static_cast< const GUI::CPropertyInspectorForm::TPropertyChangedEventData* >( eventData );
    if ( GUCEF_NULL == change )
        return;
    const GUI::CPropertyInspectorForm::TPropertyChange& property = change->GetData();
    if ( ASSETKIND_MODEL == m_shownKind )
        m_model.ApplyProperty( property.path, property.newValue );
    else
    if ( ASSETKIND_MATERIAL == m_shownKind )
        m_material.ApplyProperty( property.path, property.newValue );
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::InitializeSources( const CAssetSourcesSettings& settings )
{GUCEF_TRACE;

    if ( !m_initialized || GUCEF_NULL != m_sources )
        return false;
    m_sources = GUCEF_NEW CAssetSourcesPane( m_mounter );
    const bool opened = m_sources->Open( settings );
    if ( !m_sources->Initialize( *m_context, m_sourcesHost, m_inspector, m_transfers, m_thumbnails ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CViewerShell: The asset sources cannot be shown" );
        GUCEF_DELETE m_sources;
        m_sources = GUCEF_NULL;
        return false;
    }
    TEventCallback assetCallback( this, &CViewerShell::OnSourcesAssetOpenRequested );
    SubscribeTo( m_sources, CAssetSourcesPane::AssetOpenRequestedEvent, assetCallback );
    TEventCallback folderCallback( this, &CViewerShell::OnSourcesFolderOpenRequested );
    SubscribeTo( m_sources, CAssetSourcesPane::FolderOpenRequestedEvent, folderCallback );

    // the sign in of the online stores: the forms and the routing of the buttons of the page
    m_signIn = GUCEF_NEW CViewerSignIn();
    if ( m_signIn->Initialize( *m_context, m_report, m_credentials ) )
    {
        m_signIn->SetAuthManager( m_authManager );
        TEventCallback signInCallback( this, &CViewerShell::OnSourcesSignInRequested );
        SubscribeTo( m_sources, CAssetSourcesPane::SignInRequestedEvent, signInCallback );
        TEventCallback settingsCallback( this, &CViewerShell::OnSourcesSignInSettingsRequested );
        SubscribeTo( m_sources, CAssetSourcesPane::SignInSettingsRequestedEvent, settingsCallback );
        TEventCallback expiredCallback( this, &CViewerShell::OnSourcesSessionExpired );
        SubscribeTo( m_sources, CAssetSourcesPane::SessionExpiredEvent, expiredCallback );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CViewerShell: The sign in of the online stores cannot be shown" );
        GUCEF_DELETE m_signIn;
        m_signIn = GUCEF_NULL;
    }
    return opened;
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::SetCredentials( CViewerCredentials* credentials )
{GUCEF_TRACE;

    m_credentials = credentials;
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::SetAuthManager( AUTH::CAuthManager* manager )
{GUCEF_TRACE;

    m_authManager = manager;
    if ( GUCEF_NULL != m_signIn )
        m_signIn->SetAuthManager( manager );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnSourcesAssetOpenRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* path = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != path )
        OpenAsset( path->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnSourcesFolderOpenRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* path = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == path || GUCEF_NULL == m_browser )
        return;
    m_anythingOpened = true;
    UpdateEmptyState();
    m_browser->Refresh();
    m_browser->NavigateTo( VfsRootName, "/" + path->GetData() );
    GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( m_leftTabs );
    if ( GUCEF_NULL != tabs )
        tabs->SetActiveTab( 0 );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnSourcesSignInRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the route of the kind of the store (a browser window for a web session, the sign in service of gucefAUTH) is the business of CViewerSignIn
    const CORE::TCloneableString* storeId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != storeId && GUCEF_NULL != m_signIn )
        m_signIn->SignIn( storeId->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnSourcesSignInSettingsRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* storeId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != storeId && GUCEF_NULL != m_signIn )
        m_signIn->OpenSettings( storeId->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnSourcesSessionExpired( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* storeId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != storeId && GUCEF_NULL != m_signIn )
        m_signIn->OnSessionExpired( storeId->GetData() );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CViewerShell::GetEmptyStateText( void )
{GUCEF_TRACE;

    static const CORE::CString text = "Open a folder, archive or pack to start: use the buttons on the Content tab, press Ctrl+O, or start the viewer with a path.";
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::IsEmptyStateShown( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_emptyState && m_emptyState->IsVisible();
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::UpdateEmptyState( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_emptyState )
        m_emptyState->SetVisibility( !m_anythingOpened && ASSETKIND_OTHER == m_shownKind );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::SetRecentLocations( GUI::CRecentLocations* recent )
{GUCEF_TRACE;

    m_recent = GUCEF_NULL != recent ? recent : &m_ownRecent;
    if ( GUCEF_NULL != m_openStrip )
        m_openStrip->SetRecentLocations( m_recent );
}

/*-------------------------------------------------------------------------*/

GUI::CRecentLocations*
CViewerShell::GetRecentLocations( void ) const
{GUCEF_TRACE;

    return m_recent;
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::OpenLocation( const CORE::CString& diskPath, bool showInBrowser )
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;

    CAssetOpener opener( m_mounter );
    CAssetOpener::SOpenResult result;
    if ( !opener.Open( diskPath, result ) )
    {
        ReportProblem( diskPath, result.problem );
        // a place that is gone is not offered again
        m_recent->Remove( diskPath );
        return false;
    }
    m_recent->Add( result.diskPath );
    m_anythingOpened = true;
    UpdateEmptyState();

    // folders and archives are shown in the VFS root of the browser, which lists the new mount
    const bool isFile = CAssetOpener::OPENKIND_FILE == result.kind;
    if ( showInBrowser || !isFile )
    {
        m_browser->Refresh();
        m_browser->NavigateTo( VfsRootName, "/" + result.vfsFolder );
        ShowLeftTab( 0 );
    }
    if ( isFile )
        return OpenAsset( result.vfsAsset );
    m_report->AddEntry( GUI::CReportViewForm::SEVERITY_INFO, result.diskPath, "Opened as " + result.mountPoint + " in the VFS" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::OpenVfsSelection( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    // chosen in the VFS root of a dialog: it is mounted already
    const CORE::CString path = CAssetMounter::NormalizePath( vfsPath );
    if ( path.IsNULLOrEmpty() )
        return false;
    m_anythingOpened = true;
    if ( ASSETKIND_OTHER != CAssetTypeResolver::GetKind( path ) )
    {
        const Int32 slash = path.HasChar( '/', false );
        m_browser->NavigateTo( VfsRootName, slash > 0 ? "/" + path.SubstrFromRange( 0, (UInt32) slash ) : CORE::CString( "/" ) );
        ShowLeftTab( 0 );
        return OpenAsset( path );
    }
    m_browser->NavigateTo( VfsRootName, "/" + path );
    ShowLeftTab( 0 );
    UpdateEmptyState();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::IsOpenDialogShown( void ) const
{GUCEF_TRACE;

    return ( GUCEF_NULL != m_folderDialog && m_folderDialog->IsOpen() ) ||
           ( GUCEF_NULL != m_archiveDialog && m_archiveDialog->IsOpen() ) ||
           ( GUCEF_NULL != m_fileDialog && m_fileDialog->IsOpen() );
}

/*-------------------------------------------------------------------------*/

GUI::CFileSystemDialogForm*
CViewerShell::MakeOpenDialog( const CORE::CString& kind )
{GUCEF_TRACE;

    GUI::CFileSystemDialogForm* dialog = GUCEF_NULL;
    if ( FolderDialogKind == kind )
        dialog = GUCEF_NEW GUI::CDirectoryOpenDialogForm( m_context );
    else
        dialog = GUCEF_NEW GUI::CFileOpenDialogForm( m_context );

    // the files of the computer first, then the mounts; nothing is changed by what is chosen
    CORE::CString home;
    if ( !CORE::TryResolveSpecialDir( CORE::SPECIALDIR_HOME_DIR, home ) )
        home.Clear();
    dialog->AddRoot( LocalFilesRootName, CORE::TFileSystemAccessPtr( GUCEF_NEW CORE::CNativeFileSystemAccess( LocalFilesRootName ) ), home );
    dialog->AddRoot( VfsRootName, CORE::TFileSystemAccessPtr( GUCEF_NEW VFS::CVfsFileSystemAccess( VfsRootName ) ) );
    GUI::CFileSystemDialogForm::TOptions& options = dialog->GetOptions();
    options.readOnly = true;
    options.mustExist = true;
    options.multiSelect = false;
    options.initialRoot = LocalFilesRootName;

    if ( FolderDialogKind == kind )
    {
        options.title = "Open a folder";
        options.acceptLabel = "Open folder";
    }
    else
    if ( ArchiveDialogKind == kind )
    {
        options.title = "Open an archive or pack";
        options.acceptLabel = "Open";
        options.includeAllFilesType = true;
        CORE::CString::StringVector archives;
        CAssetOpener::GetArchivePatterns( archives );
        dialog->AddFileType( "Archives and packs", archives );
    }
    else
    {
        options.title = "Open a file";
        options.acceptLabel = "Open";
        options.includeAllFilesType = true;
        CORE::CString::StringVector supported, models, textures, archives;
        CAssetOpener::GetSupportedPatterns( supported );
        CAssetOpener::GetModelPatterns( models );
        CAssetOpener::GetTexturePatterns( textures );
        CAssetOpener::GetArchivePatterns( archives );
        dialog->AddFileType( "All supported", supported );
        dialog->AddFileType( "3D models, scenes and materials", models );
        dialog->AddFileType( "Textures and images", textures );
        dialog->AddFileType( "Archives and packs", archives );
    }

    if ( !dialog->Build() )
    {
        GUCEF_DELETE dialog;
        return GUCEF_NULL;
    }
    TEventCallback acceptedCallback( this, &CViewerShell::OnOpenDialogAccepted );
    SubscribeTo( dialog, GUI::CFileSystemDialogForm::AcceptedEvent, acceptedCallback );
    return dialog;
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::PrepareOpenDialog( GUI::CFileSystemDialogForm& dialog )
{GUCEF_TRACE;

    // the places pane: the folders of the user that are there, the mounts, and the folders of the places that were opened lately
    dialog.ClearPlaces();
    const struct { CORE::TSpecialDirs dir; const char* label; } userFolders[] =
    {
        { CORE::SPECIALDIR_HOME_DIR, "Home" }, { CORE::SPECIALDIR_DESKTOP_DIR, "Desktop" },
        { CORE::SPECIALDIR_MY_DOCUMENTS_DIR, "Documents" }, { CORE::SPECIALDIR_MY_DOWNLOADS_DIR, "Downloads" }
    };
    CORE::CString::StringVector userPaths;
    for ( size_t u=0; u<sizeof( userFolders ) / sizeof( userFolders[ 0 ] ); ++u )
    {
        CORE::CString userPath;
        if ( !CORE::TryResolveSpecialDir( userFolders[ u ].dir, userPath ) )
            continue;
        userPath = CAssetMounter::NormalizeDiskPath( userPath );
        bool known = userPath.IsNULLOrEmpty() || !CORE::DirExists( userPath );
        for ( size_t k=0; k<userPaths.size() && !known; ++k )
            known = userPaths[ k ] == userPath;
        if ( known )
            continue;
        userPaths.push_back( userPath );
        dialog.AddPlace( userFolders[ u ].label, LocalFilesRootName, userPath );
    }
    dialog.AddPlace( "Mounted (VFS)", VfsRootName, "/" );
    CORE::CString::StringVector folders;
    CORE::CString startFolder;
    for ( UInt32 i=0; i<m_recent->GetCount(); ++i )
    {
        const CORE::CString path = CAssetMounter::NormalizeDiskPath( m_recent->Get( i ) );
        const CORE::CString folder = CORE::DirExists( path ) ? path : CORE::StripLastSubDir( path ).ReplaceChar( '\\', '/' );
        if ( folder.IsNULLOrEmpty() || !CORE::DirExists( folder ) )
            continue;
        bool known = false;
        for ( size_t f=0; f<folders.size() && !known; ++f )
            known = folders[ f ] == folder;
        if ( known )
            continue;
        if ( folders.empty() )
            startFolder = folder;
        folders.push_back( folder );
        dialog.AddPlace( "Recent: " + GUI::CRecentLocations::GetShortName( folder ), LocalFilesRootName, folder );
    }

    // it starts where the last place was, and else in the folder of the user
    dialog.GetOptions().initialRoot = LocalFilesRootName;
    dialog.GetOptions().initialDirectory = startFolder;
}

/*-------------------------------------------------------------------------*/

bool
CViewerShell::ShowOpenDialog( GUI::CFileSystemDialogForm*& dialog, const CORE::CString& kind )
{GUCEF_TRACE;

    if ( !m_initialized || IsOpenDialogShown() )
        return false;
    if ( GUCEF_NULL == dialog )
        dialog = MakeOpenDialog( kind );
    if ( GUCEF_NULL == dialog )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CViewerShell: The dialog to open something cannot be made" );
        return false;
    }
    PrepareOpenDialog( *dialog );
    return dialog->Open();
}

/*-------------------------------------------------------------------------*/

bool CViewerShell::ShowOpenFolderDialog( void ) { return ShowOpenDialog( m_folderDialog, FolderDialogKind ); }
bool CViewerShell::ShowOpenArchiveDialog( void ) { return ShowOpenDialog( m_archiveDialog, ArchiveDialogKind ); }
bool CViewerShell::ShowOpenFileDialog( void ) { return ShowOpenDialog( m_fileDialog, FileDialogKind ); }

/*-------------------------------------------------------------------------*/

bool
CViewerShell::HandleKey( INPUT::KeyCode keyCode, UInt32 modifiers )
{GUCEF_TRACE;

    const bool ctrl = 0 != ( modifiers & INPUT::KEYMOD_CTRL );
    const bool shift = 0 != ( modifiers & INPUT::KEYMOD_SHIFT );
    const bool alt = 0 != ( modifiers & INPUT::KEYMOD_ALT );
    if ( !ctrl || alt || INPUT::KEYCODE_O != keyCode )
        return false;
    if ( IsOpenDialogShown() )
        return true;
    if ( shift )
        ShowOpenFolderDialog();
    else
        ShowOpenFileDialog();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnRootKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CKeyEventData* key = CORE::CICloneable::CloneableAs< GUI::CKeyEventData >( eventData );
    if ( GUCEF_NULL == key || key->handled || GUI::EVENTPHASE_BUBBLE == key->phase )
        return;
    if ( HandleKey( key->keyCode, key->modifiers ) )
        key->handled = true;
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnOpenActionRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* action = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == action )
        return;
    if ( ActionOpenFolder == action->GetData() )
        ShowOpenFolderDialog();
    else
    if ( ActionOpenArchive == action->GetData() )
        ShowOpenArchiveDialog();
    else
    if ( ActionOpenFile == action->GetData() )
        ShowOpenFileDialog();
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnOpenRecentChosen( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* path = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != path )
        OpenLocation( path->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnOpenDialogAccepted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CFileSystemDialogForm* dialog = static_cast< GUI::CFileSystemDialogForm* >( notifier );
    if ( ( dialog != m_folderDialog && dialog != m_archiveDialog && dialog != m_fileDialog ) || dialog->GetSelection().empty() )
        return;
    const GUI::CFileSystemDialogForm::TSelection& selection = dialog->GetSelection().front();
    if ( VfsRootName == selection.rootName )
        OpenVfsSelection( selection.path );
    else
        OpenLocation( selection.path );
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::Update( UInt32 thumbnailBudgetInMs )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;
    m_logger.Pump( m_report );
    if ( GUCEF_NULL != m_thumbnails )
        m_thumbnails->Update( thumbnailBudgetInMs );
    if ( GUCEF_NULL != m_sources )
        m_sources->Update( thumbnailBudgetInMs );
}

/*-------------------------------------------------------------------------*/

CReportLogger&
CViewerShell::GetLogger( void )
{GUCEF_TRACE;

    return m_logger;
}

/*-------------------------------------------------------------------------*/

void
CViewerShell::OnOgreFrameStarted( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( !m_initialized || !m_stage.IsInitialized() )
        return;
    if ( m_controller.ConsumeChanged() )
    {
        if ( ASSETKIND_MODEL == m_shownKind )
            m_model.Update( m_controller );
        else
        if ( ASSETKIND_MATERIAL == m_shownKind )
            m_material.GetPreview().Update( m_controller );
        m_stage.ApplyController( m_controller );
        m_inspectorStale = true;
        m_idleInMs = 0;
    }
    else
    if ( m_inspectorStale )
    {
        // the distance and the level of detail in the inspector follow the camera once it has come to rest
        m_idleInMs += deltaInMs;
        if ( m_idleInMs > 300 )
        {
            m_inspectorStale = false;
            if ( ASSETKIND_MODEL == m_shownKind )
                RefreshInspector();
        }
    }
    if ( m_stageBound && m_stage.IsDirty() )
        m_stage.Render();
}

/*-------------------------------------------------------------------------*/

GUI::CContentBrowserForm* CViewerShell::GetBrowser( void ) const { return m_browser; }
GUI::CContentBrowserForm* CViewerShell::GetSceneBrowser( void ) const { return m_sceneBrowser; }
GUI::CViewportForm* CViewerShell::GetViewport( void ) const { return m_viewport; }
GUI::CTimelineForm* CViewerShell::GetTimeline( void ) const { return m_timeline; }
GUI::CPropertyInspectorForm* CViewerShell::GetInspector( void ) const { return m_inspector; }
GUI::CImageViewForm* CViewerShell::GetImageView( void ) const { return m_imageView; }
GUI::CImageViewForm* CViewerShell::GetUvView( void ) const { return m_uvView; }
GUI::CReportViewForm* CViewerShell::GetReport( void ) const { return m_report; }
GUI::CTransferQueueForm* CViewerShell::GetTransfers( void ) const { return m_transfers; }
GUI::COpenActionStripForm* CViewerShell::GetOpenStrip( void ) const { return m_openStrip; }
GUI::CFileSystemDialogForm* CViewerShell::GetFolderDialog( void ) const { return m_folderDialog; }
GUI::CFileSystemDialogForm* CViewerShell::GetArchiveDialog( void ) const { return m_archiveDialog; }
GUI::CFileSystemDialogForm* CViewerShell::GetFileDialog( void ) const { return m_fileDialog; }
CAssetSourcesPane* CViewerShell::GetSources( void ) const { return m_sources; }
CViewerSignIn* CViewerShell::GetSignIn( void ) const { return m_signIn; }
CThumbnailProvider* CViewerShell::GetThumbnailProvider( void ) const { return m_thumbnails; }
CViewerStage& CViewerShell::GetStage( void ) { return m_stage; }
COrbitController& CViewerShell::GetController( void ) { return m_controller; }
CModelView& CViewerShell::GetModelView( void ) { return m_model; }
CMaterialView& CViewerShell::GetMaterialView( void ) { return m_material; }
CTextureView& CViewerShell::GetTextureView( void ) { return m_texture; }
CSceneView& CViewerShell::GetSceneView( void ) { return m_scene; }
CParticlePreview& CViewerShell::GetParticlePreview( void ) { return m_particles; }
bool CViewerShell::IsStageBound( void ) const { return m_stageBound; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
