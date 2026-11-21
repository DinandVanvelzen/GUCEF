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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
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

#ifndef GUCEF_AUTH_CWEBSESSIONSIGNIN_H
#include "gucefAUTH_CWebSessionSignIn.h"
#define GUCEF_AUTH_CWEBSESSIONSIGNIN_H
#endif /* GUCEF_AUTH_CWEBSESSIONSIGNIN_H ? */

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#ifndef GUCEF_GUI_CMENUBAR_H
#include "gucefGUI_CMenuBar.h"
#define GUCEF_GUI_CMENUBAR_H
#endif /* GUCEF_GUI_CMENUBAR_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHHELP_H
#include "AssetWorkbenchLib_CWorkbenchHelp.h"
#define ASSETWORKBENCHLIB_CWORKBENCHHELP_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHHELP_H ? */

#ifndef ASSETWORKBENCHLIB_CRESOURCEVIEWTHUMBNAILPROVIDER_H
#include "AssetWorkbenchLib_CResourceViewThumbnailProvider.h"
#define ASSETWORKBENCHLIB_CRESOURCEVIEWTHUMBNAILPROVIDER_H
#endif /* ASSETWORKBENCHLIB_CRESOURCEVIEWTHUMBNAILPROVIDER_H ? */

#ifndef ASSETWORKBENCHLIB_CPACKINDEXJOB_H
#include "AssetWorkbenchLib_CPackIndexJob.h"
#define ASSETWORKBENCHLIB_CPACKINDEXJOB_H
#endif /* ASSETWORKBENCHLIB_CPACKINDEXJOB_H ? */

#include "AssetWorkbenchLib_CWorkbenchShell.h"
#include "gucefVFS_CVfsGlobal.h"
#include "gucefASSETSTORE_CPackStorageLayout.h"
#include "AssetWorkbenchLib_CPackPresentationJob.h"
#include "AssetWorkbenchLib_CPackAssetConverter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The frame listener of Ogre that gives the shell its turn at the start of every frame */
class CShellFrameListener : public Ogre::FrameListener
{
    public:

    explicit CShellFrameListener( CWorkbenchShell& shell ) : m_shell( shell ) {}

    virtual bool frameStarted( const Ogre::FrameEvent& evt ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_shell.OnOgreFrameStarted( (UInt32) ( evt.timeSinceLastFrame * 1000.0f ) );
        return true;
    }

    private:

    CWorkbenchShell& m_shell;
};

namespace {

const UInt32 TimelineHeight = 120;

/** An effect fills this part of the narrower side of every view, and is framed as if it were at least this big when it has not run (or is smaller) */
const Float32 EffectFillFraction = 0.4f;
const Float32 MinimumEffectFrame = 0.1f;

/** A model of the showcase is at least this part of the biggest footprint, scaled up for it */
const Float32 ShowcaseMinimumShare = 0.55f;

/** The camera of the showcase of many models: turned 20 degrees, 35 degrees above the ground */
const Float32 ShowcaseYaw = 0.3490659f;
const Float32 ShowcasePitch = 0.6108652f;
const Float32 EffectObserveSeconds = 2.0f;

// the start layout: the sizes of the splitters (a part of the room less the handle) that give the resources, the views and the
// inspector the widths they had when the window was divided by fractions of its width, with the height of the main area for the strips
const char* const MainSplitRatio = "0.74";
const char* const SideSplitRatio = "0.2812";
const char* const ViewSplitRatio = "0.683";
const char* const SplitHandleSize = "5";
const char* const SideHeaderHeight = "22";

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
const char* const TogglePresentation = "presentation";

const char* const ActionPackPresentation = "PackPresentation";
const char* const ActionPresentationPrefix = "Presentation:";

/** The field of view of the camera of the workbench, which a portrait changes for its own */
const Float32 DefaultFieldOfView = 45.0f;

/** Binds a key without modifiers to an action unless an action has it already (the existing binding is the owner of the key) */
void
BindShortcut( GUI::CKeyboardShortcutMap& shortcuts, const CORE::CString& action, INPUT::KeyCode key, UInt32 modifiers )
{
    const CORE::CString owner = shortcuts.FindAction( key, modifiers );
    if ( !owner.IsNULLOrEmpty() && owner != action )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: The key " << GUI::CKeyboardShortcutMap::DescribeChord( key, modifiers ) << " is the key of " << owner << " already and is not given to " << action );
        return;
    }
    shortcuts.Bind( action, key, modifiers );
}

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

const CORE::CString CWorkbenchShell::LayoutBrowse = "Browse";
const CORE::CString CWorkbenchShell::LayoutReview = "Review";
const CORE::CString CWorkbenchShell::LayoutPreview = "Preview";
const CORE::CString CWorkbenchShell::MainSplitName = "MainSplit";
const CORE::CString CWorkbenchShell::SideSplitName = "SideSplit";
const CORE::CString CWorkbenchShell::ViewSplitName = "ViewSplit";
const CORE::CString CWorkbenchShell::LeftPanelName = "LeftSide";
const CORE::CString CWorkbenchShell::RightPanelName = "RightSide";
const CORE::CString CWorkbenchShell::ActionFoldLeft = "FoldLeftSide";
const CORE::CString CWorkbenchShell::ActionFoldRight = "FoldRightSide";
const CORE::CString CWorkbenchShell::ActionPreview = "PreviewLayout";
const UInt32 MenuBarHeight = 24;

/** The window is at least this high (logical pixels) for the menu bar to take its row: in a lower window every pixel is for the views, and the keys (Ctrl+, settings, F1 help, the key left of 1 console) have the actions */
const UInt32 MenuBarMinWindowHeight = 800;

/** The menu: the path of an item as the menu bar has it, and the action it does */
struct SMenuItem
{
    const char* path;
    const char* actionId;
};

const SMenuItem g_menuItems[] =
{
    { "File/Open folder...", "folder" },
    { "File/Open archive or pack...", "archive" },
    { "File/Open file...", "file" },
    { "File/Connect to store...", "ConnectStore" },
    { "File/Settings...", "Settings" },
    { "View/Resources", "FoldLeftSide" },
    { "View/Inspector", "FoldRightSide" },
    { "View/Preview layout", "PreviewLayout" },
    { "View/Layout Browse", "Layout:Browse" },
    { "View/Layout Review", "Layout:Review" },
    { "View/Developer console", "DeveloperConsole" },
    { "View/Resource view", "ResourceView" },
    { "View/Re-index pack", "ReIndex" },
    { "View/Pack presentation", "PackPresentation" },
    { "View/Default presentation", "Presentation:default" },
    { "View/More models", "PresentationMore" },
    { "Jobs/Jobs overview...", "Jobs" },
    { "Jobs/Bulk conversion...", "BulkConversion" },
    { "Jobs/Library sync...", "LibrarySync" },
    { "Jobs/Re-index", "Redo:reindex" },
    { "Jobs/Re-analyse", "Redo:reanalyse" },
    { "Jobs/Re-import", "Redo:reimport" },
    { "Jobs/Re-convert", "Redo:reconvert" },
    { "Jobs/Re-thumbnail", "Redo:rethumbnail" },
    { "Jobs/Convert for Ogre", "ConvertForOgre" },
    { "Help/Help of this place", "Help" },
    { "Help/Keyboard shortcuts", "Help:workbench.shortcuts" },
    { GUCEF_NULL, GUCEF_NULL }
};

/** The actions with a key, in the order the settings list them */
const CWorkbenchShell::TActionInfo g_actions[] =
{
    { "file", "Open a file", INPUT::KEYCODE_O, INPUT::KEYMOD_CTRL },
    { "folder", "Open a folder", INPUT::KEYCODE_O, INPUT::KEYMOD_CTRL | INPUT::KEYMOD_SHIFT },
    { "FoldLeftSide", "Fold or unfold the resources", INPUT::KEYCODE_F9, 0 },
    { "FoldRightSide", "Fold or unfold the inspector", INPUT::KEYCODE_F10, 0 },
    { "PreviewLayout", "Preview layout on or off", INPUT::KEYCODE_F11, 0 },
    { "Settings", "Open the settings", INPUT::KEYCODE_COMMA, INPUT::KEYMOD_CTRL },
    { "Help", "Help of the place you are at", INPUT::KEYCODE_F1, 0 },
    { "DeveloperConsole", "Open or close the developer console", INPUT::KEYCODE_GRAVE, 0 }
};

const CORE::CString CWorkbenchShell::ActionSettings = "Settings";
const CORE::CString CWorkbenchShell::ActionHelp = "Help";
const CORE::CString CWorkbenchShell::ActionConsole = "DeveloperConsole";
const CORE::CString CWorkbenchShell::ActionConnectStore = "ConnectStore";
const CORE::CEvent CWorkbenchShell::ActionRequestedEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchShell::ActionRequestedEvent";
const CORE::CEvent CWorkbenchShell::PreferencesChangedEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchShell::PreferencesChangedEvent";
const CORE::CString CWorkbenchShell::LocalFilesRootName = "Local files";
const CORE::CString CWorkbenchShell::VfsRootName = "VFS";
const CORE::CString CWorkbenchShell::ResourcesRootName = "Resources";
const CORE::CString CWorkbenchShell::ActionReIndex = "ReIndex";
const CORE::CString CWorkbenchShell::ActionOpenFolder = "folder";
const CORE::CString CWorkbenchShell::ActionOpenArchive = "archive";
const CORE::CString CWorkbenchShell::ActionOpenFile = "file";
const CORE::CString CWorkbenchShell::ActionConvertForOgre = "ConvertForOgre";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWorkbenchShell::CWorkbenchShell( CAssetMounter& mounter )
    : CORE::CObservingNotifier()
    , m_mounter( mounter )
    , m_context( GUCEF_NULL )
    , m_resolver()
    , m_logger()
    , m_activity()
    , m_jobRegistry()
    , m_loader( &m_activity )
    , m_stage( "Workbench" )
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
    , m_menuBar( GUCEF_NULL )
    , m_menuBarHeight( 0 )
    , m_menuBarHeightKnown( false )
    , m_console( GUCEF_NULL )
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
    , m_activityIndicator( GUCEF_NULL )
    , m_jobSummaryButton( GUCEF_NULL )
    , m_activityHost( GUCEF_NULL )
    , m_statusHost( GUCEF_NULL )
    , m_jobsHost( GUCEF_NULL )
    , m_jobsForm( GUCEF_NULL )
    , m_bulkForm( GUCEF_NULL )
    , m_bulkController( GUCEF_NULL )
    , m_syncForm( GUCEF_NULL )
    , m_syncController( GUCEF_NULL )
    , m_jobSummaryWaitMs( 0 )
    , m_shownSummary()
    , m_sourcesHost( GUCEF_NULL )
    , m_sources( GUCEF_NULL )
    , m_signIn( GUCEF_NULL )
    , m_storeConnection( GUCEF_NULL )
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
    , m_asyncOpen( false )
    , m_openStepFailed( false )
    , m_openingPath()
    , m_browserRoot()
    , m_uvImage()
    , m_shownKind( ASSETKIND_OTHER )
    , m_shownPath()
    , m_stageBound( false )
    , m_initialized( false )
    , m_inspectorStale( false )
    , m_idleInMs( 0 )
    , m_uvChoices()
    , m_mainSplit( GUCEF_NULL )
    , m_sideSplit( GUCEF_NULL )
    , m_viewSplit( GUCEF_NULL )
    , m_leftPanel( GUCEF_NULL )
    , m_rightPanel( GUCEF_NULL )
    , m_ownLayout()
    , m_layout( &m_ownLayout )
    , m_beforePreview()
    , m_shortcuts()
    , m_viewPreset()
    , m_updatingViewLabel( false )
    , m_showcase( m_stage )
    , m_ownPreferences()
    , m_preferences( &m_ownPreferences )
    , m_presentationOverride( -1 )
    , m_presentation()
    , m_hasPresentation( false )
    , m_packRoot()
    , m_packKey()
    , m_packSource()
    , m_packCacheFolder()
    , m_offered()
    , m_toggleForMaterial( false )
    , m_presentationJobActive( false )
    , m_presentationJobKind( CPackPresentation::KIND_SUMMARY )
    , m_presentationRetries( 0 )
    , m_presentationMaxAssets( 48 )
    , m_autoCaptureFrames( 0 )
    , m_autoThumbnails()
    , m_autoCaptureCountdownInMs( 0 )
    , m_autoCapturePending( false )
    , m_autoCaptureKey()
    , m_navigatingItself( 0 )
    , m_presentationStartTick( 0 )
    , m_poolSerial( 0 )
    , m_kindOverride()
    , m_conversion()
    , m_hasConversion( false )
    , m_indexLoader( &m_activity )
    , m_resourceViewHold()
    , m_resourceView( GUCEF_NULL )
    , m_resourceViewOverride( -1 )
    , m_resourceViewPack()
    , m_indexingFolder()
    , m_indexAutoSwitch( false )
    , m_lastIndexResult()
    , m_selectedResource()
    , m_selectedResourcePath()
    , m_hasSelectedResource( false )
    , m_resourceStartPath()
    , m_resourceThumbs( GUCEF_NULL )
    , m_capture( GUCEF_NULL )
    , m_sourceText()
    , m_cooked()
    , m_hasCooked( false )
    , m_cookedFor()
    , m_cookPending( false )
    , m_cookPendingPath()
    , m_cookForPresentation( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWorkbenchShell::~CWorkbenchShell()
{GUCEF_TRACE;

    Shutdown();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::BuildLayout( void )
{GUCEF_TRACE;

    m_root = m_context->CreateWidget( "Panel", "AssetWorkbench", GUCEF_NULL );
    if ( GUCEF_NULL == m_root )
        return false;
    m_root->SetProperty( GUI::EWidgetProperty_Size, "1,0 1,0" );
    m_root->SetProperty( GUI::EWidgetProperty_BackgroundColour, "30,32,40,255" );
    m_root->SetProperty( GUI::EWidgetProperty_HelpTopic, CWorkbenchHelp::TopicOverview );
    BuildMenuBar();

    // the arrangement: the main area above the row of the report, in the main area the resources (left) next to the views and the
    // inspector (right); the sides fold into a strip and the splitters between them are the user's to move
    m_mainSplit = GUI::CWidget::As< GUI::CSplitter >( m_context->CreateWidget( "Splitter", MainSplitName, m_root ) );
    if ( GUCEF_NULL == m_mainSplit )
        return false;
    m_mainSplit->SetProperty( GUI::EWidgetProperty_Position, "0,0 0,0" );
    m_mainSplit->SetProperty( GUI::EWidgetProperty_Size, "1,0 1,0" );
    m_mainSplit->SetProperty( GUI::EWidgetProperty_Orientation, "Vertical" );
    m_mainSplit->SetProperty( GUI::EWidgetProperty_DefaultSplitRatio, MainSplitRatio );
    m_mainSplit->SetProperty( GUI::EWidgetProperty_SplitRatio, MainSplitRatio );
    m_mainSplit->SetProperty( GUI::EWidgetProperty_HandleSize, SplitHandleSize );
    m_mainSplit->SetProperty( GUI::EWidgetProperty_FirstMinSize, "362" );   // the tabs keep their room in a low window
    m_mainSplit->SetProperty( GUI::EWidgetProperty_SecondMinSize, "72" );
    m_mainSplit->SetProperty( GUI::EWidgetProperty_BackgroundColour, "0,0,0,0" );
    m_sideSplit = GUI::CWidget::As< GUI::CSplitter >( m_context->CreateWidget( "Splitter", SideSplitName, m_mainSplit ) );
    GUI::CWidget* bottomRow = MakePanel( *m_context, "BottomRow", m_mainSplit, "0,0 0,0", "1,0 1,0" );
    if ( GUCEF_NULL == m_sideSplit || GUCEF_NULL == bottomRow )
        return false;
    m_sideSplit->SetProperty( GUI::EWidgetProperty_Size, "1,0 1,0" );
    m_sideSplit->SetProperty( GUI::EWidgetProperty_DefaultSplitRatio, SideSplitRatio );
    m_sideSplit->SetProperty( GUI::EWidgetProperty_SplitRatio, SideSplitRatio );
    m_sideSplit->SetProperty( GUI::EWidgetProperty_HandleSize, SplitHandleSize );
    m_sideSplit->SetProperty( GUI::EWidgetProperty_FirstMinSize, "160" );
    m_sideSplit->SetProperty( GUI::EWidgetProperty_SecondMinSize, "320" );
    m_sideSplit->SetProperty( GUI::EWidgetProperty_BackgroundColour, "0,0,0,0" );
    m_leftPanel = GUI::CWidget::As< GUI::CFoldablePanel >( m_context->CreateWidget( "FoldablePanel", LeftPanelName, m_sideSplit ) );
    m_viewSplit = GUI::CWidget::As< GUI::CSplitter >( m_context->CreateWidget( "Splitter", ViewSplitName, m_sideSplit ) );
    if ( GUCEF_NULL == m_leftPanel || GUCEF_NULL == m_viewSplit )
        return false;
    m_leftPanel->SetProperty( GUI::EWidgetProperty_Size, "1,0 1,0" );
    m_leftPanel->SetProperty( GUI::EWidgetProperty_Dock, "Left" );
    m_leftPanel->SetProperty( GUI::EWidgetProperty_TitleHeight, SideHeaderHeight );
    m_leftPanel->SetProperty( GUI::EWidgetProperty_Title, "Resources" );
    m_leftPanel->SetProperty( GUI::EWidgetProperty_Tooltip, "Fold the resources away or bring them back (F9)" );
    m_viewSplit->SetProperty( GUI::EWidgetProperty_Size, "1,0 1,0" );
    m_viewSplit->SetProperty( GUI::EWidgetProperty_DefaultSplitRatio, ViewSplitRatio );
    m_viewSplit->SetProperty( GUI::EWidgetProperty_SplitRatio, ViewSplitRatio );
    m_viewSplit->SetProperty( GUI::EWidgetProperty_HandleSize, SplitHandleSize );
    m_viewSplit->SetProperty( GUI::EWidgetProperty_FirstMinSize, "240" );
    m_viewSplit->SetProperty( GUI::EWidgetProperty_SecondMinSize, "140" );
    m_viewSplit->SetProperty( GUI::EWidgetProperty_BackgroundColour, "0,0,0,0" );

    // left: the browsers of the files and of the nodes of a scene
    GUI::CWidget* leftPages[ 3 ] = { GUCEF_NULL, GUCEF_NULL, GUCEF_NULL };
    m_leftTabs = MakeTabs( *m_context, "LeftTabs", m_leftPanel, "0,0 0,0", "1,0 1,0", "Content\nScene\nSources", 3, leftPages );
    // middle: the 3D view with its time line, the texture, and the UV layout
    GUI::CWidget* centerPages[ 3 ] = { GUCEF_NULL, GUCEF_NULL, GUCEF_NULL };
    m_centerTabs = MakeTabs( *m_context, "CenterTabs", m_viewSplit, "0,0 0,0", "1,0 1,0", "3D\nImage\nUV layout", 3, centerPages );
    m_rightPanel = GUI::CWidget::As< GUI::CFoldablePanel >( m_context->CreateWidget( "FoldablePanel", RightPanelName, m_viewSplit ) );
    if ( GUCEF_NULL == m_rightPanel )
        return false;
    m_rightPanel->SetProperty( GUI::EWidgetProperty_Size, "1,0 1,0" );
    m_rightPanel->SetProperty( GUI::EWidgetProperty_Dock, "Right" );
    m_rightPanel->SetProperty( GUI::EWidgetProperty_TitleHeight, SideHeaderHeight );
    m_rightPanel->SetProperty( GUI::EWidgetProperty_Title, "Inspector" );
    m_rightPanel->SetProperty( GUI::EWidgetProperty_Tooltip, "Fold the inspector away or bring it back (F10)" );
    GUI::CWidget* inspectorHost = MakePanel( *m_context, "InspectorHost", m_rightPanel, "0,0 0,0", "1,0 1,0" );
    // the strip at the bottom right is the status area: the activity indicator shows there when something takes time (the report keeps its height)
    const CORE::CString statusHeight = CORE::UInt32ToString( GUI::CActivityIndicatorForm::GetPreferredHeight() );
    const CORE::CString transferSize = "0.38,0 1,-" + statusHeight;
    GUI::CWidget* reportHost = MakePanel( *m_context, "ReportHost", bottomRow, "0,0 0,0", "0.62,0 1,0" );
    GUI::CWidget* transferHost = MakePanel( *m_context, "TransferHost", bottomRow, "0.62,0 0,0", transferSize.C_String() );
    const CORE::CString statusPosition = "0.62,0 1,-" + statusHeight;
    const CORE::CString statusSize = "0.38,0 0," + statusHeight;
    GUI::CWidget* statusHost = MakePanel( *m_context, "StatusHost", bottomRow, statusPosition.C_String(), statusSize.C_String() );
    // the status strip holds the activity indicator and, beside it, the summary of the jobs (a click opens the jobs page)
    GUI::CWidget* activityHost = MakePanel( *m_context, "ActivityHost", statusHost, "0,0 0,0", "0.5,0 1,0" );
    if ( GUCEF_NULL == m_leftTabs || GUCEF_NULL == m_centerTabs || GUCEF_NULL == inspectorHost || GUCEF_NULL == reportHost || GUCEF_NULL == transferHost || GUCEF_NULL == statusHost || GUCEF_NULL == activityHost )
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

    // F1 opens the help of the area the focus is in
    leftPages[ 0 ]->SetProperty( GUI::EWidgetProperty_HelpTopic, CWorkbenchHelp::TopicOpening );
    leftPages[ 1 ]->SetProperty( GUI::EWidgetProperty_HelpTopic, CWorkbenchHelp::TopicViews );
    leftPages[ 2 ]->SetProperty( GUI::EWidgetProperty_HelpTopic, CWorkbenchHelp::TopicImporting );
    for ( UInt32 i=0; i<3; ++i )
        centerPages[ i ]->SetProperty( GUI::EWidgetProperty_HelpTopic, CWorkbenchHelp::TopicViews );
    m_rightPanel->SetProperty( GUI::EWidgetProperty_HelpTopic, CWorkbenchHelp::TopicViews );
    transferHost->SetProperty( GUI::EWidgetProperty_HelpTopic, CWorkbenchHelp::TopicImporting );

    m_browser = GUCEF_NEW GUI::CContentBrowserForm( m_context );
    m_sceneBrowser = GUCEF_NEW GUI::CContentBrowserForm( m_context );
    m_viewport = GUCEF_NEW GUI::CViewportForm( m_context );
    m_timeline = GUCEF_NEW GUI::CTimelineForm( m_context );
    m_inspector = GUCEF_NEW GUI::CPropertyInspectorForm( m_context );
    m_imageView = GUCEF_NEW GUI::CImageViewForm( m_context );
    m_uvView = GUCEF_NEW GUI::CImageViewForm( m_context );
    m_report = GUCEF_NEW GUI::CReportViewForm( m_context );
    m_transfers = GUCEF_NEW GUI::CTransferQueueForm( m_context );
    m_activityIndicator = GUCEF_NEW GUI::CActivityIndicatorForm( m_context );
    m_openStrip = GUCEF_NEW GUI::COpenActionStripForm( m_context );
    m_openStrip->AddAction( ActionOpenFolder, "Open folder...", "Browse a folder of this computer or of the VFS (Ctrl+Shift+O)" );
    m_openStrip->AddAction( ActionOpenArchive, "Open archive or pack...", "Browse a zip, 7z, rar, tar or Unity package as a folder" );
    m_openStrip->AddAction( ActionOpenFile, "Open file...", "Show a model, scene or picture; an archive is browsed (Ctrl+O)" );
    const bool built = m_openStrip->Build( openHost ) && m_browser->Build( browserHost ) && m_sceneBrowser->Build( sceneHost ) && m_viewport->Build( viewportHost ) &&
                       m_timeline->Build( timelineHost ) && m_inspector->Build( inspectorHost ) && m_imageView->Build( imageHost ) &&
                       m_uvView->Build( uvHost ) && m_report->Build( reportHost ) && m_transfers->Build( transferHost ) && m_activityIndicator->Build( activityHost );
    if ( !built )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchShell: The forms of the workbench cannot be built" );
        return false;
    }

    m_activityHost = activityHost;
    m_statusHost = statusHost;
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
CWorkbenchShell::Initialize( GUI::CGUIContext& context, CAssetCapture* capture, const CORE::CString& thumbnailCacheFolder )
{GUCEF_TRACE;

    if ( m_initialized )
        return true;
    m_context = &context;

    // the workbench's own log: warnings and errors of what is loaded
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &m_logger );

    if ( !m_stage.Initialize( 512, 512 ) || !BuildLayout() )
    {
        Shutdown();
        return false;
    }

    m_browser->SetContentTypeResolver( &m_resolver );

    // the presentations of packs: their conversions are kept next to the thumbnails, and the particles open in the four views
    m_autoThumbnails.SetFolder( thumbnailCacheFolder );
    m_autoThumbnails.SetEnabled( m_preferences->automaticThumbnails );
    m_packCacheFolder = thumbnailCacheFolder.IsNULLOrEmpty() ? CORE::RelativePath( "$CACHEDIR$/AssetWorkbench/packs" ) : CORE::CString( thumbnailCacheFolder + "_packs" );
    SetDefaultViewPreset( ASSETKIND_PARTICLE, CViewPresetTracker::GetPresetName( CViewPresetTracker::PRESET_FOUR_VIEWS ) );

    // the files of the computer are where the workbench starts, with the folder of the user as the first folder; the mounts are the second root
    CORE::CString home;
    if ( !CORE::TryResolveSpecialDir( CORE::SPECIALDIR_HOME_DIR, home ) )
        home.Clear();
    m_browser->AddRoot( LocalFilesRootName, CORE::TFileSystemAccessPtr( GUCEF_NEW CORE::CNativeFileSystemAccess( LocalFilesRootName ) ), home );
    m_browser->AddRoot( VfsRootName, CORE::TFileSystemAccessPtr( GUCEF_NEW VFS::CVfsFileSystemAccess( VfsRootName ) ) );
    // the resources of a pack by function (F17): a root of its own beside the files, with the switch in the tool bar of the browser
    m_resourceView = GUCEF_NEW CResourceViewFileSystemAccess( ResourcesRootName );
    m_resourceViewHold = CORE::TFileSystemAccessPtr( m_resourceView );
    m_browser->AddRoot( ResourcesRootName, m_resourceViewHold );
    m_browser->SetAlternateRoot( ResourcesRootName, "Files", "Resources" );
    m_browser->SetRootOrder( ResourcesRootName, m_resourceView );
    m_indexLoader.SetThreadPoolName( CAssetLoader::DefaultThreadPoolName + "_index" );
    if ( m_mounter.GetMountCount() > 0 )
    {
        // something was mounted before the workbench started (--mount): that is what is shown
        m_anythingOpened = true;
        m_browser->NavigateTo( VfsRootName, "/" );
    }
    m_openStrip->SetRecentLocations( m_recent );
    if ( GUCEF_NULL != capture )
    {
        // the pictures of models follow the setting: cooked output or the quick look
        capture->SetPreferCookedFlag( &m_preferences->preferCookedOutput );
        m_capture = capture;
        m_thumbnails = GUCEF_NEW CThumbnailProvider( m_mounter, *capture );
        m_thumbnails->SetCacheFolder( thumbnailCacheFolder );
        m_thumbnails->SetAutoThumbnails( &m_autoThumbnails );
        // the resource view gets the thumbnails that exist, for the files of the pack
        m_resourceThumbs = GUCEF_NEW CResourceViewThumbnailProvider( *m_thumbnails, *m_resourceView );
        m_browser->SetThumbnailProvider( m_resourceThumbs );
    }
    // the files of the computer are a list (name, type, size, date), the mounts show thumbnails when they can be made
    m_browserRoot = m_browser->GetCurrentRootName();
    m_browser->SetViewMode( LocalFilesRootName != m_browserRoot && ResourcesRootName != m_browserRoot && GUCEF_NULL != m_thumbnails ? GUI::CContentBrowserForm::VIEWMODE_THUMBNAILS : GUI::CContentBrowserForm::VIEWMODE_DETAILS );
    m_sceneBrowser->AddRoot( "Scene", m_scene.GetFileSystem() );

    // the reports that came before the report view did
    m_report->AddEntry( GUI::CReportViewForm::SEVERITY_INFO, CORE::CString::Empty, "Asset workbench started", "Renderer: " + CORE::CString( Ogre::Root::getSingleton().getRenderSystem()->getName() ) );
    if ( !m_anythingOpened )
        m_report->AddEntry( GUI::CReportViewForm::SEVERITY_INFO, CORE::CString::Empty, "Nothing is open", GetEmptyStateText() );
    UpdateEmptyState();

    TEventCallback selectionCallback( this, &CWorkbenchShell::OnBrowserSelectionChanged );
    SubscribeTo( m_browser, GUI::CContentBrowserForm::SelectionChangedEvent, selectionCallback );
    TEventCallback folderCallback( this, &CWorkbenchShell::OnBrowserFolderChanged );
    SubscribeTo( m_browser, GUI::CContentBrowserForm::FolderChangedEvent, folderCallback );
    TEventCallback activatedCallback( this, &CWorkbenchShell::OnBrowserItemActivated );
    SubscribeTo( m_browser, GUI::CContentBrowserForm::ItemActivatedEvent, activatedCallback );
    TEventCallback folderActivatedCallback( this, &CWorkbenchShell::OnBrowserFolderActivated );
    SubscribeTo( m_browser, GUI::CContentBrowserForm::FolderActivatedEvent, folderActivatedCallback );
    TEventCallback filterChangedCallback( this, &CWorkbenchShell::OnBrowserFilterChanged );
    SubscribeTo( m_browser, GUI::CContentBrowserForm::FilterChangedEvent, filterChangedCallback );
    TEventCallback viewSwitchedCallback( this, &CWorkbenchShell::OnBrowserViewSwitched );
    SubscribeTo( m_browser, GUI::CContentBrowserForm::ViewSwitchedEvent, viewSwitchedCallback );
    TEventCallback sceneFolderCallback( this, &CWorkbenchShell::OnSceneFolderChanged );
    SubscribeTo( m_sceneBrowser, GUI::CContentBrowserForm::FolderChangedEvent, sceneFolderCallback );
    TEventCallback sceneSelectionCallback( this, &CWorkbenchShell::OnSceneSelectionChanged );
    SubscribeTo( m_sceneBrowser, GUI::CContentBrowserForm::SelectionChangedEvent, sceneSelectionCallback );
    TEventCallback toggleCallback( this, &CWorkbenchShell::OnToggleChanged );
    SubscribeTo( m_viewport, GUI::CViewportForm::ToggleChangedEvent, toggleCallback );
    TEventCallback resizedCallback( this, &CWorkbenchShell::OnSurfaceResized );
    SubscribeTo( m_viewport, GUI::CViewportForm::SurfaceResizedEvent, resizedCallback );
    TEventCallback clipCallback( this, &CWorkbenchShell::OnClipSelected );
    SubscribeTo( m_timeline, GUI::CTimelineForm::ClipSelectedEvent, clipCallback );
    TEventCallback timeCallback( this, &CWorkbenchShell::OnTimeChanged );
    SubscribeTo( m_timeline, GUI::CTimelineForm::TimeChangedEvent, timeCallback );
    TEventCallback playCallback( this, &CWorkbenchShell::OnPlayStateChanged );
    SubscribeTo( m_timeline, GUI::CTimelineForm::PlayStateChangedEvent, playCallback );
    TEventCallback propertyCallback( this, &CWorkbenchShell::OnPropertyChanged );
    SubscribeTo( m_inspector, GUI::CPropertyInspectorForm::PropertyChangedEvent, propertyCallback );
    TEventCallback actionCallback( this, &CWorkbenchShell::OnOpenActionRequested );
    SubscribeTo( m_openStrip, GUI::COpenActionStripForm::ActionRequestedEvent, actionCallback );
    TEventCallback recentCallback( this, &CWorkbenchShell::OnOpenRecentChosen );
    SubscribeTo( m_openStrip, GUI::COpenActionStripForm::RecentChosenEvent, recentCallback );
    TEventCallback keyCallback( this, &CWorkbenchShell::OnRootKeyDown );
    SubscribeTo( m_root, GUI::CWidget::KeyDownEvent, keyCallback );

    // the keys of the workbench; a chord that is taken by another action is not given away
    BindShortcuts();
    TEventCallback textInputCallback( this, &CWorkbenchShell::OnRootTextInput );
    SubscribeTo( m_root, GUI::CWidget::TextInputEvent, textInputCallback );
    TEventCallback shortcutCallback( this, &CWorkbenchShell::OnShortcutActivated );
    SubscribeTo( &m_shortcuts, GUI::CKeyboardShortcutMap::ShortcutActivatedEvent, shortcutCallback );
    SetLayoutManager( GUCEF_NULL );

    // what takes time is shown by the activity service: the busy cursor, and the indicator in the status strip
    m_activity.Attach( m_context );
    m_activityIndicator->SetService( &m_activity );
    m_jobRegistry.SetActivityService( &m_activity );
    m_loader.SetJobRegistry( &m_jobRegistry, "open" );
    m_indexLoader.SetJobRegistry( &m_jobRegistry, "index" );
    TEventCallback loadFinishedCallback( this, &CWorkbenchShell::OnLoadFinished );
    SubscribeTo( &m_loader, CAssetLoader::LoadFinishedEvent, loadFinishedCallback );

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
CWorkbenchShell::Shutdown( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_frameListener )
    {
        if ( GUCEF_NULL != Ogre::Root::getSingletonPtr() )
            Ogre::Root::getSingleton().removeFrameListener( m_frameListener );
        GUCEF_DELETE m_frameListener;
        m_frameListener = GUCEF_NULL;
    }
    UnsubscribeAllFromObserver();
    if ( GUCEF_NULL != m_capture )
        m_capture->SetPreferCookedFlag( GUCEF_NULL );
    m_capture = GUCEF_NULL;
    m_ownLayout.Detach();
    if ( m_layout != &m_ownLayout )
        m_layout->Detach();
    m_layout = &m_ownLayout;
    CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &m_logger );

    // what is being opened is let go of before the views and the stage it would have gone to
    m_loader.Cancel();
    m_loader.Update( 0 );
    m_indexLoader.Cancel();
    m_indexLoader.Update( 0 );
    m_loader.SetJobRegistry( GUCEF_NULL, CORE::CString::Empty );
    m_indexLoader.SetJobRegistry( GUCEF_NULL, CORE::CString::Empty );
    m_activity.Detach();
    if ( GUCEF_NULL != m_activityIndicator )
        m_activityIndicator->SetService( GUCEF_NULL );
    GUCEF_DELETE m_activityIndicator;   m_activityIndicator = GUCEF_NULL;

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

    // the connection goes first: its service is let go of by the sources, the transfers and the bulk conversion while they are still there
    GUCEF_DELETE m_storeConnection; m_storeConnection = GUCEF_NULL;
    GUCEF_DELETE m_bulkForm;        m_bulkForm = GUCEF_NULL;
    GUCEF_DELETE m_bulkController;  m_bulkController = GUCEF_NULL;
    GUCEF_DELETE m_syncForm;        m_syncForm = GUCEF_NULL;
    GUCEF_DELETE m_syncController;  m_syncController = GUCEF_NULL;
    GUCEF_DELETE m_jobsForm;        m_jobsForm = GUCEF_NULL;
    m_jobSummaryButton = GUCEF_NULL;
    m_activityHost = GUCEF_NULL;
    m_jobsHost = GUCEF_NULL;
    m_statusHost = GUCEF_NULL;
    GUCEF_DELETE m_signIn;          m_signIn = GUCEF_NULL;
    GUCEF_DELETE m_sources;         m_sources = GUCEF_NULL;
    GUCEF_DELETE m_folderDialog;    m_folderDialog = GUCEF_NULL;
    GUCEF_DELETE m_archiveDialog;   m_archiveDialog = GUCEF_NULL;
    GUCEF_DELETE m_fileDialog;      m_fileDialog = GUCEF_NULL;
    GUCEF_DELETE m_openStrip;       m_openStrip = GUCEF_NULL;
    m_emptyState = GUCEF_NULL;
    m_menuBar = GUCEF_NULL;
    m_console = GUCEF_NULL;
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
    if ( GUCEF_NULL != m_browser )
        m_browser->SetThumbnailProvider( GUCEF_NULL );
    GUCEF_DELETE m_resourceThumbs;  m_resourceThumbs = GUCEF_NULL;
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
CWorkbenchShell::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_initialized;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::BindStage( void )
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
CWorkbenchShell::UnloadViews( void )
{GUCEF_TRACE;

    m_particles.Clear();
    m_particleClip = false;
    m_showcase.Unload();
    m_hasPresentation = false;
    m_autoCapturePending = false;
    m_controller.SetFieldOfView( DefaultFieldOfView );
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
    m_sourceText.Clear();
    m_hasCooked = false;
    m_cooked = CCookedOutputResolver::SCookedOutput();
    m_cookedFor.Clear();
    UpdateImageTabTitle();
    ReleaseConversion();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::CloseAsset( void )
{GUCEF_TRACE;

    UnloadViews();
    if ( GUCEF_NULL != m_inspector )
        m_inspector->SetDataCopy( CORE::CDataNode() );
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

EAssetKind
CWorkbenchShell::GetShownKind( void ) const
{GUCEF_TRACE;

    return m_shownKind;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchShell::GetShownPath( void ) const
{GUCEF_TRACE;

    return m_shownPath;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ReportProblem( const CORE::CString& asset, const CORE::CString& message )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchShell: " << asset << ": " << message );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::SetInspector( const CORE::CDataNode& data, const GUI::CPropertyInspectorForm::TDescriptorMap& descriptors )
{GUCEF_TRACE;

    m_inspector->ClearDescriptors();
    m_inspector->SetDescriptors( descriptors );
    m_inspector->SetDataCopy( data );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::RefreshInspector( void )
{GUCEF_TRACE;

    CORE::CDataNode data;
    GUI::CPropertyInspectorForm::TDescriptorMap descriptors;
    switch ( m_shownKind )
    {
        case ASSETKIND_MODEL:
        {
            m_model.WriteInspectorData( data );
            m_model.GetInspectorDescriptors( descriptors );
            if ( !m_sourceText.IsNULLOrEmpty() )
                data.SetAttribute( "Source", m_sourceText );
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
            if ( !m_sourceText.IsNULLOrEmpty() )
                data.SetAttribute( "Source", m_sourceText );
            break;
        }
        case ASSETKIND_PARTICLE:
        {
            data.SetName( "Particles" );
            data.SetAttribute( "Name", m_shownPath );
            m_particles.WriteInspectorData( data );
            break;
        }
        case ASSETKIND_IMAGE:
        case ASSETKIND_TEXTURE:
        {
            m_texture.WriteInspectorData( data );
            break;
        }
        case ASSETKIND_FOLDER:
        {
            // the presentation of a pack that has no 3D view of its own: the summary of the pack and what is shown of it
            if ( !m_hasPresentation )
                return;
            m_packSummary.WriteInspectorData( data );
            break;
        }
        default:
        {
            return;
        }
    }
    AppendResourceInspector( data );
    AppendPresentationInspector( data );
    SetInspector( data, descriptors );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ShowCenterTab( Int32 tab )
{GUCEF_TRACE;

    GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( m_centerTabs );
    if ( GUCEF_NULL != tabs )
        tabs->SetActiveTab( (UInt32) tab );
}

/*-------------------------------------------------------------------------*/

Int32
CWorkbenchShell::GetCenterTab( void ) const
{GUCEF_TRACE;

    GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( m_centerTabs );
    return GUCEF_NULL != tabs ? tabs->GetActiveTab() : -1;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchShell::GetCenterTabTitle( UInt32 index ) const
{GUCEF_TRACE;

    GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( m_centerTabs );
    return GUCEF_NULL != tabs ? tabs->GetTabTitle( index ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ShowFolderSummary( const CORE::CString& vfsFolder )
{GUCEF_TRACE;

    if ( !m_initialized || !m_packSummary.Read( CAssetMounter::NormalizePath( vfsFolder ) ) )
        return false;
    if ( !m_packSummary.IsRecognisedPack() && ASSETKIND_OTHER != m_shownKind )
        return false;
    CORE::CDataNode data;
    m_packSummary.WriteInspectorData( data );
    SetInspector( data, GUI::CPropertyInspectorForm::TDescriptorMap() );
    return true;
}

/*-------------------------------------------------------------------------*/

const CAssetPackSummary&
CWorkbenchShell::GetPackSummary( void ) const
{GUCEF_TRACE;

    return m_packSummary;
}

/*-------------------------------------------------------------------------*/

Int32
CWorkbenchShell::GetLeftTab( void ) const
{GUCEF_TRACE;

    GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( m_leftTabs );
    return GUCEF_NULL != tabs ? tabs->GetActiveTab() : -1;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ShowLeftTab( Int32 tab )
{GUCEF_TRACE;

    GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( m_leftTabs );
    if ( GUCEF_NULL != tabs && tab >= 0 )
        tabs->SetActiveTab( (UInt32) tab );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::SetModelToggles( bool forMaterial )
{GUCEF_TRACE;

    m_toggleForMaterial = forMaterial;
    GUI::CViewportForm::TToggleVector toggles;

    GUI::CViewportForm::TToggle display( ToggleDisplay, "Display", "How the surfaces are drawn" );
    for ( UInt32 i=0; i<(UInt32) CModelView::DISPLAYMODE_COUNT; ++i )
        display.choices.push_back( CModelView::GetDisplayModeName( static_cast< CModelView::EDisplayMode >( i ) ) );
    display.choice = 0;
    toggles.push_back( display );

    GUI::CViewportForm::TToggle lighting( ToggleLighting, "Light", "The light preset" );
    for ( UInt32 i=0; i<(UInt32) CWorkbenchStage::LIGHTPRESET_COUNT; ++i )
        lighting.choices.push_back( CWorkbenchStage::GetLightPresetName( static_cast< CWorkbenchStage::ELightPreset >( i ) ) );
    lighting.choice = (Int32) m_stage.GetLightPreset();
    toggles.push_back( lighting );

    GUI::CViewportForm::TToggle view( ToggleView, "View", "Frame the model, look at it from a fixed side (keys 1 to 4, F) or see four views at once; Custom when the camera left the preset" );
    view.choices.push_back( "Frame" );
    view.choices.push_back( "Front" );
    view.choices.push_back( "Side" );
    view.choices.push_back( "Top" );
    view.choices.push_back( "Three quarter" );
    view.choices.push_back( CViewPresetTracker::GetPresetName( CViewPresetTracker::PRESET_FOUR_VIEWS ) );
    view.choice = m_viewPreset.IsCustom() ? 0 : (Int32) m_viewPreset.GetPreset();
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
    AddPresentationToggle( toggles );
    m_viewport->SetToggles( toggles );
    ShowViewPresetLabel();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::FillTimeline( void )
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
CWorkbenchShell::IsPackageFile( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    const CORE::CString path = CAssetMounter::NormalizePath( vfsPath );
    return ASSETKIND_PACKAGE == CAssetTypeResolver::GetKind( path ) || !CAssetMounter::GetArchiveType( path ).IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::OpenPackageFile( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;
    const CORE::CString path = CAssetMounter::NormalizePath( vfsPath );
    CORE::CString mountPoint;
    // a file of a folder of the disk is mounted from the disk, as the open dialogs do (an archive made from a resource of the VFS needs the
    // resource to be read from several places, which a file read into memory cannot); else the VFS path is the source
    CORE::CString source = path;
    for ( UInt32 i=0; i<m_mounter.GetMountCount(); ++i )
    {
        const CAssetMounter::SMount& mount = m_mounter.GetMount( i );
        if ( mount.adopted || mount.archiveType != VFS::CVFS::FileSystemArchiveTypeName || path.Length() <= mount.mountPoint.Length() + 1 )
            continue;
        if ( path.SubstrFromRange( 0, mount.mountPoint.Length() ) == mount.mountPoint && '/' == path[ mount.mountPoint.Length() ] )
        {
            const CORE::CString onDisk = CAssetMounter::NormalizeDiskPath( mount.source ) + "/" + path.CutChars( mount.mountPoint.Length() + 1, true );
            if ( CORE::FileExists( onDisk ) )
                source = onDisk;
        }
    }
    // a pack that was opened already is the same mount: its presentation is shown again
    if ( !m_mounter.MountUnique( source, mountPoint ) )
    {
        ReportProblem( path, "The package " + path + " cannot be mounted as a pack" );
        return false;
    }
    m_anythingOpened = true;
    UpdateEmptyState();
    m_browser->Refresh();
    ++m_navigatingItself;
    m_browser->NavigateTo( VfsRootName, "/" + mountPoint );
    --m_navigatingItself;
    ShowLeftTab( 0 );
    m_report->AddEntry( GUI::CReportViewForm::SEVERITY_INFO, path, "Opened as " + mountPoint + " in the VFS" );
    OpenPack( mountPoint, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::OpenAsset( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;
    const CORE::CString path = CAssetMounter::NormalizePath( vfsPath );
    const EAssetKind kind = CAssetTypeResolver::GetKind( path );
    // a package is a pack: it is presented, there is nothing to view
    if ( IsPackageFile( path ) )
        return OpenPackageFile( path );
    UnloadViews();

    bool shown = false;
    EAssetKind shownKind = kind;
    switch ( kind )
    {
        case ASSETKIND_MODEL:
        {
            // the cooked Ogre output of the asset when it exists and is current, else the quick look through the Assimp codec
            CCookedOutputResolver::SCookedOutput cooked;
            const bool probed = ProbeCooked( path, cooked );
            if ( probed )
                CCookedOutputResolver::Verify( cooked );
            const bool useCooked = probed && cooked.IsCurrent();
            NoteSource( path, cooked, probed, useCooked );
            if ( useCooked )
            {
                bool isScene = false;
                shown = ShowCooked( cooked, GUCEF_NULL, isScene );
                if ( shown && !isScene )
                    ShowModelFinish();
                if ( shown && isScene )
                    shownKind = ASSETKIND_SCENE;
                if ( !shown )
                {
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchShell: The cooked output of " << path << " cannot be shown: the quick look follows" );
                    UnloadViews();
                    NoteSource( path, cooked, probed, false );
                }
            }
            if ( !shown )
            {
                shownKind = kind;
                shown = ShowModel( path );
            }
            break;
        }
        case ASSETKIND_IMAGE:
        case ASSETKIND_TEXTURE:     { shown = ShowTexture( path ); break; }
        default:                    { shown = ShowOther( kind, path ); break; }
    }
    FinishOpen( shownKind, path, shown );
    return shown;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ShowOther( EAssetKind kind, const CORE::CString& path )
{GUCEF_TRACE;

    switch ( kind )
    {
        case ASSETKIND_MATERIAL:    return ShowMaterial( path );
        case ASSETKIND_SCENE:       return ShowScene( path );
        case ASSETKIND_SKELETON:    return ShowSkeleton( path );
        case ASSETKIND_PARTICLE:    return ShowParticles( path );
        default:
        {
            ReportProblem( path, "There is no view for this kind of file" );
            return false;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      OPENING WITHOUT BLOCKING THE INTERFACE                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The two halves of opening an asset of the workbench (CIAssetLoadJob). Prepare() runs on a thread of a pool and touches nothing
 *  of the views: a model is read and imported (CModelView::PrepareMesh()), a picture is decoded into a view of its own, the other
 *  kinds are read ahead. Step() runs on the thread of the renderer: the first piece takes the previous asset away and makes the
 *  new one (the Ogre objects of a model, the picture of a texture), the second fills the panels.
 */
class CWorkbenchShell::COpenJob : public CIAssetLoadJob
{
    public:

    COpenJob( CWorkbenchShell& shell, const CORE::CString& path, EAssetKind kind )
        : CIAssetLoadJob()
        , m_shell( shell )
        , m_path( path )
        , m_kind( kind )
        , m_group()
        , m_relative()
        , m_mesh()
        , m_texture()
        , m_step( 0 )
        , m_problem()
        , m_cooked()
        , m_probed( false )
        , m_useCooked( false )
        , m_cookedIsScene( false )
    {GUCEF_TRACE;

        // the mounts are read here, on the thread of the interface
        m_group = shell.m_mounter.GetResourceGroup( path, m_relative );
        // the cooked output of a model: what is there is looked up here (the cache entry is small), the source is compared in Prepare()
        if ( ASSETKIND_MODEL == kind )
        {
            m_probed = shell.ProbeCooked( path, m_cooked );
            if ( m_probed )
            {
                m_cookedIsScene = "scene" == CAssetTypeResolver::GetExtension( m_cooked.mainFile ).Lowercase();
                // the group of the cooked file is made on this thread; the file is in the mount of the original (a store) or in one of its own
                m_cookedGroup = shell.m_mounter.GetResourceGroup( m_cooked.mainFile, m_cookedRelative );
            }
        }
    }

    virtual bool Prepare( CAssetLoadControl& control ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        switch ( m_kind )
        {
            case ASSETKIND_MODEL:
            {
                if ( m_group.IsNULLOrEmpty() )
                {
                    m_problem = "The file is not in a mount";
                    return false;
                }
                if ( m_probed )
                {
                    // reads the source: here, not on the thread of the interface
                    CCookedOutputResolver::Verify( m_cooked );
                    m_useCooked = m_cooked.IsCurrent() && !m_cookedGroup.IsNULLOrEmpty();
                    if ( m_useCooked && !m_cookedIsScene )
                    {
                        if ( CModelView::PrepareMesh( m_cookedGroup, m_cookedRelative, m_mesh, &control ) )
                            return !control.IsCancelled();
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchShell: The cooked mesh " << m_cooked.mainFile << " cannot be prepared: the quick look follows" );
                        m_useCooked = false;
                    }
                    if ( m_useCooked )
                        return !control.IsCancelled();
                }
                if ( !CModelView::PrepareMesh( m_group, m_relative, m_mesh, &control ) )
                {
                    m_problem = control.IsCancelled() ? CORE::CString( "Cancelled" ) : CORE::CString( "The model cannot be loaded" );
                    return false;
                }
                return true;
            }
            case ASSETKIND_IMAGE:
            case ASSETKIND_TEXTURE:
            {
                if ( !m_texture.Load( m_path ) )
                {
                    m_problem = "The picture cannot be loaded: " + m_texture.GetInfo().note;
                    return false;
                }
                return !control.IsCancelled();
            }
            default:
            {
                // scripts and scenes are small; they are read once here so that the mount has them ready for the thread of the renderer
                if ( !m_group.IsNULLOrEmpty() )
                {
                    try
                    {
                        Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource( m_relative.STL_String(), m_group.STL_String() );
                        char buffer[ 16384 ];
                        while ( !stream->eof() && !control.IsCancelled() )
                            if ( 0 == stream->read( buffer, sizeof( buffer ) ) )
                                break;
                    }
                    catch ( const Ogre::Exception& )
                    {
                        // the view reports it
                    }
                }
                return !control.IsCancelled();
            }
        }
    }

    virtual EStepResult Step( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_shell.m_openStepFailed = false;
        switch ( m_kind )
        {
            case ASSETKIND_MODEL:
            {
                if ( m_useCooked && m_cookedIsScene )
                {
                    m_shell.UnloadViews();
                    m_shell.NoteSource( m_path, m_cooked, m_probed, true );
                    bool isScene = false;
                    if ( !m_shell.ShowCooked( m_cooked, GUCEF_NULL, isScene ) )
                        return Failed();
                    m_shell.FinishOpen( ASSETKIND_SCENE, m_path, true );
                    return STEP_DONE;
                }
                if ( 0 == m_step++ )
                {
                    m_shell.UnloadViews();
                    m_shell.NoteSource( m_path, m_cooked, m_probed, m_useCooked );
                    bool isScene = false;
                    const bool loaded = m_useCooked ? m_shell.ShowCooked( m_cooked, &m_mesh, isScene ) : m_shell.ShowModelLoad( m_path, &m_mesh );
                    if ( !loaded )
                        return Failed();
                    return STEP_MORE;
                }
                m_shell.ShowModelFinish();
                m_shell.FinishOpen( m_kind, m_path, true );
                return STEP_DONE;
            }
            case ASSETKIND_IMAGE:
            case ASSETKIND_TEXTURE:
            {
                if ( 0 == m_step++ )
                {
                    m_shell.UnloadViews();
                    m_shell.m_texture.Adopt( m_texture );
                    return STEP_MORE;
                }
                m_shell.ShowTextureFinish();
                m_shell.FinishOpen( m_kind, m_path, true );
                return STEP_DONE;
            }
            default:
            {
                m_shell.UnloadViews();
                const bool shown = m_shell.ShowOther( m_kind, m_path );
                m_shell.FinishOpen( m_kind, m_path, shown );
                if ( !shown )
                {
                    m_shell.m_openStepFailed = true;
                    m_problem = "The asset cannot be shown";
                    return STEP_FAILED;
                }
                return STEP_DONE;
            }
        }
    }

    virtual void Discard( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CModelView::DiscardPrepared( m_mesh );
        m_texture.Unload();
        // a request that was ended between its pieces leaves a half made asset
        if ( m_step > 0 && m_shell.m_initialized )
            m_shell.UnloadViews();
        m_step = 0;
    }

    virtual CORE::CString GetProblem( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_problem;
    }

    private:

    EStepResult Failed( void )
    {GUCEF_TRACE;

        m_problem = "The asset cannot be shown";
        m_shell.m_openStepFailed = true;
        m_shell.FinishOpen( m_kind, m_path, false );
        return STEP_FAILED;
    }

    CWorkbenchShell& m_shell;
    CORE::CString m_path;
    EAssetKind m_kind;
    CORE::CString m_group;
    CORE::CString m_relative;
    CModelView::SPreparedMesh m_mesh;
    CTextureView m_texture;
    UInt32 m_step;
    CORE::CString m_problem;
    CCookedOutputResolver::SCookedOutput m_cooked;
    bool m_probed;
    bool m_useCooked;
    bool m_cookedIsScene;
    CORE::CString m_cookedGroup;
    CORE::CString m_cookedRelative;
};

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWorkbenchShell::OpenAssetAsync( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    if ( !m_initialized )
        return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );
    const CORE::CString path = CAssetMounter::NormalizePath( vfsPath );
    const EAssetKind kind = CAssetTypeResolver::GetKind( path );
    // a pack has no asset to open in the background: the opening of its presentation is its own request of the loader (OpenAssetInteractive() tells it apart)
    if ( IsPackageFile( path ) )
    {
        OpenPackageFile( path );
        return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );
    }
    if ( !CAssetTypeResolver::HasView( path ) )
    {
        ReportProblem( path, "There is no view for this kind of file" );
        return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );
    }
    TAssetLoadJobPtr job( GUCEF_NEW COpenJob( *this, path, kind ) );
    // the label is what the user sees when it takes longer than the threshold: the name of the asset
    const Int32 slash = path.HasChar( '/', false );
    const CORE::CString label = "Opening " + ( slash < 0 ? path : path.SubstrFromRange( (UInt32) slash+1, path.Length() ) );
    // the request that is in flight is ended by this one (and told about); the new path is known after that
    m_loader.DescribeNext( "open", CORE::CString::Empty, path );
    const CORE::CFutureResult future = m_loader.Begin( job, label );
    m_openingPath = path;
    return future;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::OpenAssetInteractive( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    if ( IsPackageFile( vfsPath ) )
        return OpenPackageFile( vfsPath );
    if ( !m_asyncOpen )
        return OpenAsset( vfsPath );
    // the result is shown later; true when it was started
    return !OpenAssetAsync( vfsPath ).HasNoFuture();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::CancelOpening( void )
{GUCEF_TRACE;

    m_loader.Cancel();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnLoadFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    switch ( m_loader.GetLastOutcome() )
    {
        case CAssetLoader::OUTCOME_FAILED:
        {
            // a failure of a piece of the renderer was reported where it happened
            if ( !m_openStepFailed )
            {
                ReportProblem( m_openingPath, m_loader.GetLastProblem() );
                UpdateEmptyState();
            }
            // a presentation that did not work (a scene or a character that cannot be converted) gives way to the generated showcase, and says why
            if ( m_presentationJobActive && CPackPresentation::KIND_SUMMARY != m_presentationJobKind && CPackPresentation::KIND_MODEL_GRID != m_presentationJobKind && m_presentationRetries < 2 )
            {
                ++m_presentationRetries;
                m_presentationJobActive = false;
                const CORE::CString root = m_packRoot;
                StartPackPresentation( root, CORE::CString::Empty, false, m_presentationJobKind, m_loader.GetLastProblem() );
                return;
            }
            break;
        }
        case CAssetLoader::OUTCOME_CANCELLED:
        {
            // another asset that was opened meanwhile is no news: the user did that
            if ( "Another asset was opened" != m_loader.GetLastProblem() )
                m_report->AddEntry( GUI::CReportViewForm::SEVERITY_INFO, m_openingPath, "Opening was cancelled" );
            break;
        }
        default:
            break;
    }
    m_openStepFailed = false;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::FinishOpen( EAssetKind kind, const CORE::CString& path, bool shown )
{GUCEF_TRACE;

    if ( shown )
    {
        m_shownKind = kind;
        m_shownPath = path;
        m_anythingOpened = true;
        m_report->AddEntry( GUI::CReportViewForm::SEVERITY_INFO, path, "Opened as " + CAssetTypeResolver::GetKindName( kind ) );
        // the default view of a model is worth a thumbnail when it has none: taken when the view has settled
        if ( ASSETKIND_MODEL == kind && m_autoThumbnails.IsEnabled() )
        {
            m_autoCapturePending = true;
            m_autoCaptureKey = path;
            m_autoCaptureCountdownInMs = 0;
            m_autoCaptureFrames = 0;
        }
    }
    UpdateEmptyState();
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ShowModel( const CORE::CString& path )
{GUCEF_TRACE;

    if ( !ShowModelLoad( path, GUCEF_NULL ) )
        return false;
    ShowModelFinish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ShowModelLoad( const CORE::CString& path, CModelView::SPreparedMesh* prepared )
{GUCEF_TRACE;

    CORE::CString relative;
    const CORE::CString group = m_mounter.GetResourceGroup( path, relative );
    if ( group.IsNULLOrEmpty() )
    {
        ReportProblem( path, "The file is not in a mount" );
        return false;
    }
    const bool loaded = GUCEF_NULL != prepared ? m_model.LoadMesh( group, relative, *prepared ) : m_model.LoadMesh( group, relative );
    if ( !loaded )
    {
        ReportProblem( path, "The model cannot be loaded" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ShowModelFinish( void )
{GUCEF_TRACE;

    m_model.FrameInController( m_controller );
    m_model.Update( m_controller );
    m_stage.ApplyController( m_controller );
    ResetViewPreset( ASSETKIND_MODEL );
    SetModelToggles( false );
    m_shownKind = ASSETKIND_MODEL;
    FillTimeline();
    m_stage.SetWireframe( false );
    RefreshInspector();
    ShowCenterTab( CENTERTAB_3D );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ShowMaterial( const CORE::CString& path )
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
    ResetViewPreset( ASSETKIND_MATERIAL );
    SetModelToggles( true );
    m_timeline->ClearClips();
    m_shownKind = ASSETKIND_MATERIAL;
    RefreshInspector();
    ShowCenterTab( CENTERTAB_3D );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ShowScene( const CORE::CString& path, bool switchLeftTab )
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
    COrbitController::SVector3 low, high;
    if ( m_scene.GetBoundingBox( low, high ) )
        m_controller.SetBoundsBox( low, high );
    else
        m_controller.SetBounds( center, radius );
    // a sky or other background that is left out of the frame stays in view
    m_controller.SetMinimumFarClip( 1.5f * m_scene.GetReach() );
    m_stage.SetGridUnit( radius > 0.0f ? radius / 4.0f : 0.2f );
    m_stage.ApplyController( m_controller );
    ResetViewPreset( ASSETKIND_SCENE );
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
    if ( switchLeftTab && GUCEF_NULL != tabs )
        tabs->SetActiveTab( 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ShowTexture( const CORE::CString& path )
{GUCEF_TRACE;

    if ( !m_texture.Load( path ) )
    {
        ReportProblem( path, "The picture cannot be loaded: " + m_texture.GetInfo().note );
        return false;
    }
    ShowTextureFinish();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::UpdateImageTabTitle( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_centerTabs )
        return;
    const CORE::CString titles = ASSETKIND_TEXTURE == m_shownKind ? CORE::CString( "3D\nTexture\nUV layout" ) : CORE::CString( "3D\nImage\nUV layout" );
    if ( titles != m_centerTabs->GetProperty( GUI::EWidgetProperty_Tabs ) )
        m_centerTabs->SetProperty( GUI::EWidgetProperty_Tabs, titles );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ShowTextureFinish( void )
{GUCEF_TRACE;

    m_imageView->SetFrameMeaning( m_texture.GetInfo().isCube ? GUI::CImageViewForm::FRAMEMEANING_CUBE_FACES : GUI::CImageViewForm::FRAMEMEANING_FRAMES );
    m_imageView->SetImage( m_texture.GetImage() );
    m_imageView->FitToView();
    m_shownKind = ASSETKIND_TEXTURE == CAssetTypeResolver::GetKind( m_texture.GetInfo().path ) ? ASSETKIND_TEXTURE : ASSETKIND_IMAGE;
    UpdateImageTabTitle();
    RefreshInspector();
    ShowCenterTab( CENTERTAB_IMAGE );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ShowSkeleton( const CORE::CString& path )
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
CWorkbenchShell::ShowUvLayout( Int32 subMesh )
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
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: Sub mesh " << subMesh << " has no texture coordinates to show" );
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
CWorkbenchShell::OnBrowserSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the files of the computer are opened by a double click: a click on one of them does not mount its folder
    if ( LocalFilesRootName == m_browser->GetCurrentRootName() )
        return;
    const GUI::CContentBrowserForm::TInt32Vector selection = m_browser->GetSelection();
    if ( 1 != selection.size() )
        return;
    const GUI::CContentBrowserForm::TItem& item = m_browser->GetListedItem( (UInt32) selection[ 0 ] );
    if ( ResourcesRootName == m_browser->GetCurrentRootName() )
    {
        // a resource is shown where it is in the pack; a group is summarised
        if ( item.isDirectory )
        {
            ShowResourceFolderSummary( item.path );
            return;
        }
        ASSETCONVERTER::CPackIndexResource resource;
        if ( !GetResourceOfPath( item.path, resource ) )
            return;
        m_selectedResource = resource;
        m_selectedResourcePath = CAssetMounter::NormalizePath( m_resourceView->GetVfsPathOf( resource ) );
        m_hasSelectedResource = true;
        if ( !CAssetTypeResolver::HasView( m_selectedResourcePath ) || m_selectedResourcePath == m_shownPath )
        {
            if ( m_selectedResourcePath == m_shownPath )
                RefreshInspector();
            return;
        }
        OpenAssetInteractive( m_selectedResourcePath );
        return;
    }
    if ( item.isDirectory )
    {
        ShowFolderSummary( item.path );
        return;
    }
    if ( item.isDirectory || !CAssetTypeResolver::HasView( item.path ) )
        return;
    if ( CAssetMounter::NormalizePath( item.path ) == m_shownPath )
        return;
    OpenAssetInteractive( item.path );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnBrowserFolderChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_resourceThumbs )
        m_resourceThumbs->SetResourceMode( ResourcesRootName == m_browser->GetCurrentRootName() );

    // entering the root of a pack (the double click on it) is the request for its presentation, whatever the switch says; the places the
    // shell navigates to itself are no request
    const CORE::TCloneableString* entered = static_cast< const CORE::TCloneableString* >( eventData );
    if ( 0 == m_navigatingItself && GUCEF_NULL != entered && VfsRootName == m_browser->GetCurrentRootName() )
    {
        const CORE::CString folder = CAssetMounter::NormalizePath( entered->GetData() );
        CAssetMounter::SMount mount;
        CORE::CString relative;
        if ( !folder.IsNULLOrEmpty() && m_mounter.FindMount( folder, mount, relative ) && relative.IsNULLOrEmpty() && folder != m_packRoot )
            OpenPack( folder, true );
    }
    // going from the files of the computer to the mounts or back changes how the files are listed; within a root the choice of the user stays
    if ( m_browserRoot == m_browser->GetCurrentRootName() )
        return;
    m_browserRoot = m_browser->GetCurrentRootName();
    m_browser->SetViewMode( LocalFilesRootName != m_browserRoot && ResourcesRootName != m_browserRoot && GUCEF_NULL != m_thumbnails ? GUI::CContentBrowserForm::VIEWMODE_THUMBNAILS : GUI::CContentBrowserForm::VIEWMODE_DETAILS );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnBrowserItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
    if ( ResourcesRootName == m_browser->GetCurrentRootName() )
    {
        // a double click on a resource opens it where it is in the pack
        ASSETCONVERTER::CPackIndexResource resource;
        if ( !GetResourceOfPath( path->GetData(), resource ) )
            return;
        m_selectedResource = resource;
        m_selectedResourcePath = CAssetMounter::NormalizePath( m_resourceView->GetVfsPathOf( resource ) );
        m_hasSelectedResource = true;
        if ( m_selectedResourcePath == m_shownPath )
            return;
        OpenAssetInteractive( m_selectedResourcePath );
        return;
    }
    if ( CAssetMounter::NormalizePath( path->GetData() ) == m_shownPath )
        return;
    // the double click on the root of a pack (or the folder of one) is the request for its presentation, whatever the switch says
    CAssetMounter::SMount mount;
    CORE::CString relative;
    if ( !CAssetTypeResolver::HasView( path->GetData() ) && m_mounter.FindMount( CAssetMounter::NormalizePath( path->GetData() ), mount, relative ) )
    {
        if ( OpenPack( path->GetData(), true ) )
            return;
    }
    OpenAssetInteractive( path->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnSceneFolderChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* path = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == path || ASSETKIND_SCENE != m_shownKind )
        return;
    if ( m_scene.Select( path->GetData() ) )
        RefreshInspector();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnSceneSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( ASSETKIND_SCENE != m_shownKind )
        return;
    const CORE::CString::StringVector paths = m_sceneBrowser->GetSelectedPaths();
    if ( 1 == paths.size() && m_scene.Select( paths[ 0 ] ) )
        RefreshInspector();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnToggleChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
        m_stage.SetLightPreset( static_cast< CWorkbenchStage::ELightPreset >( toggle.choice ) );
    else
    if ( ToggleView == toggle.id )
    {
        // the label that the shell put there itself (Custom, or the preset the camera came back to) is no choice of the user
        if ( m_updatingViewLabel || toggle.choice < 0 || toggle.choice >= (Int32) CViewPresetTracker::PRESET_COUNT )
            return;
        m_viewPreset.Apply( static_cast< CViewPresetTracker::EPreset >( toggle.choice ), m_controller );
        ApplyViewLayout();
    }
    else
    if ( TogglePresentation == toggle.id )
    {
        // the first choice puts the default back, the others are the kinds the pack offers; what the shell put there itself is no choice
        if ( m_updatingViewLabel || m_packRoot.IsNULLOrEmpty() )
            return;
        if ( 0 == toggle.choice )
            ResetPackPresentation();
        else
        if ( toggle.choice > 0 && (size_t) toggle.choice <= m_offered.size() )
            ShowPackPresentation( m_packRoot, CPackPresentation::GetKindId( m_offered[ (size_t) toggle.choice - 1 ] ), true );
        return;
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
CWorkbenchShell::OnSurfaceResized( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    BindStage();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnClipSelected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableInt32* index = static_cast< const CORE::TCloneableInt32* >( eventData );
    if ( GUCEF_NULL != index && ASSETKIND_MODEL == m_shownKind )
        m_model.SelectClip( index->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnTimeChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
CWorkbenchShell::OnPlayStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableBool* playing = static_cast< const CORE::TCloneableBool* >( eventData );
    if ( GUCEF_NULL != playing && m_particleClip )
        m_particles.SetPlaying( playing->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::StartParticleClip( void )
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
CWorkbenchShell::ShowParticles( const CORE::CString& path )
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
    FrameEffectExtent();
    if ( !m_controller.HasBoundsBox() )
        m_stage.SetGridUnit( radius > 0.0f ? radius / 4.0f : 0.2f );
    m_stage.ApplyController( m_controller );
    ResetViewPreset( ASSETKIND_PARTICLE );
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
CWorkbenchShell::OnPropertyChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
CWorkbenchShell::InitializeSources( const CAssetSourcesSettings& settings )
{GUCEF_TRACE;

    if ( !m_initialized || GUCEF_NULL != m_sources )
        return false;
    m_sources = GUCEF_NEW CAssetSourcesPane( m_mounter );
    const bool opened = m_sources->Open( settings );
    if ( !m_sources->Initialize( *m_context, m_sourcesHost, m_inspector, m_transfers, m_thumbnails ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchShell: The asset sources cannot be shown" );
        GUCEF_DELETE m_sources;
        m_sources = GUCEF_NULL;
        return false;
    }
    TEventCallback assetCallback( this, &CWorkbenchShell::OnSourcesAssetOpenRequested );
    SubscribeTo( m_sources, CAssetSourcesPane::AssetOpenRequestedEvent, assetCallback );
    TEventCallback folderCallback( this, &CWorkbenchShell::OnSourcesFolderOpenRequested );
    SubscribeTo( m_sources, CAssetSourcesPane::FolderOpenRequestedEvent, folderCallback );

    // the files of a remote store: the fetches are operations of the activity indicator and jobs of the jobs list, the pictures come from the service
    m_sources->SetFeedback( &m_activity, &m_jobRegistry );
    {
        CORE::CString remoteCache = m_packCacheFolder;
        if ( remoteCache.Length() > 6 && "_packs" == remoteCache.SubstrFromRange( remoteCache.Length() - 6, remoteCache.Length() ) )
            remoteCache = remoteCache.SubstrFromRange( 0, remoteCache.Length() - 6 ) + "_remote";
        else
        if ( remoteCache.Length() > 6 && "/packs" == remoteCache.SubstrFromRange( remoteCache.Length() - 6, remoteCache.Length() ) )
            remoteCache = remoteCache.SubstrFromRange( 0, remoteCache.Length() - 6 ) + "/remote";
        m_sources->SetRemoteCacheFolder( remoteCache );
    }
    if ( GUCEF_NULL != m_thumbnails )
        m_thumbnails->SetRemoteSource( m_sources->GetRemoteThumbnails() );
    TEventCallback remoteChangedCallback( this, &CWorkbenchShell::OnSourcesRemoteContentChanged );
    SubscribeTo( m_sources, CAssetSourcesPane::RemoteContentChangedEvent, remoteChangedCallback );

    // the sign in of the online stores: the forms and the routing of the buttons of the page
    m_signIn = GUCEF_NEW CWorkbenchSignIn();
    if ( m_signIn->Initialize( *m_context, m_report, m_credentials ) )
    {
        m_signIn->SetAuthManager( m_authManager );
        TEventCallback signInCallback( this, &CWorkbenchShell::OnSourcesSignInRequested );
        SubscribeTo( m_sources, CAssetSourcesPane::SignInRequestedEvent, signInCallback );
        TEventCallback settingsCallback( this, &CWorkbenchShell::OnSourcesSignInSettingsRequested );
        SubscribeTo( m_sources, CAssetSourcesPane::SignInSettingsRequestedEvent, settingsCallback );
        TEventCallback expiredCallback( this, &CWorkbenchShell::OnSourcesSessionExpired );
        SubscribeTo( m_sources, CAssetSourcesPane::SessionExpiredEvent, expiredCallback );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchShell: The sign in of the online stores cannot be shown" );
        GUCEF_DELETE m_signIn;
        m_signIn = GUCEF_NULL;
    }
    InitializeStoreConnection( GUCEF_NULL != m_credentials ? &m_credentials->GetSessionStore() : GUCEF_NULL );
    AttachJobs();
    return opened;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::SetCredentials( CWorkbenchCredentials* credentials )
{GUCEF_TRACE;

    m_credentials = credentials;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::SetAuthManager( AUTH::CAuthManager* manager )
{GUCEF_TRACE;

    m_authManager = manager;
    if ( GUCEF_NULL != m_signIn )
        m_signIn->SetAuthManager( manager );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnSourcesAssetOpenRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* path = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != path )
        OpenAsset( path->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnSourcesRemoteContentChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // folders and files of a remote store arrived: the folder of the content browser is read again
    if ( GUCEF_NULL != m_browser )
        m_browser->Refresh();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnSourcesFolderOpenRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
CWorkbenchShell::OnSourcesSignInRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the route of the kind of the store (a browser window for a web session, the sign in service of gucefAUTH) is the business of CWorkbenchSignIn
    const CORE::TCloneableString* storeId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != storeId && GUCEF_NULL != m_signIn )
        m_signIn->SignIn( storeId->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnSourcesSignInSettingsRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* storeId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != storeId && GUCEF_NULL != m_signIn )
        m_signIn->OpenSettings( storeId->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnSourcesSessionExpired( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* storeId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != storeId && GUCEF_NULL != m_signIn )
        m_signIn->OnSessionExpired( storeId->GetData() );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchShell::GetEmptyStateText( void )
{GUCEF_TRACE;

    static const CORE::CString text = "Open a folder, archive or pack to start: use the buttons on the Content tab, press Ctrl+O, or start the workbench with a path.";
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::IsEmptyStateShown( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_emptyState && m_emptyState->IsVisible();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::UpdateEmptyState( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_emptyState )
        m_emptyState->SetVisibility( !m_anythingOpened && ASSETKIND_OTHER == m_shownKind );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::SetRecentLocations( GUI::CRecentLocations* recent )
{GUCEF_TRACE;

    m_recent = GUCEF_NULL != recent ? recent : &m_ownRecent;
    if ( GUCEF_NULL != m_openStrip )
        m_openStrip->SetRecentLocations( m_recent );
}

/*-------------------------------------------------------------------------*/

GUI::CRecentLocations*
CWorkbenchShell::GetRecentLocations( void ) const
{GUCEF_TRACE;

    return m_recent;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::OpenLocation( const CORE::CString& diskPath, bool showInBrowser )
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
        ++m_navigatingItself;
        m_browser->NavigateTo( VfsRootName, "/" + result.vfsFolder );
        --m_navigatingItself;
        ShowLeftTab( 0 );
    }
    if ( isFile )
        return OpenAssetInteractive( result.vfsAsset );
    m_report->AddEntry( GUI::CReportViewForm::SEVERITY_INFO, result.diskPath, "Opened as " + result.mountPoint + " in the VFS" );
    // a pack that is recognised shows what it is without more clicks, unless the user switched that off
    OpenPack( result.vfsFolder, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::OpenVfsSelection( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    // chosen in the VFS root of a dialog: it is mounted already
    const CORE::CString path = CAssetMounter::NormalizePath( vfsPath );
    if ( path.IsNULLOrEmpty() )
        return false;
    m_anythingOpened = true;
    if ( IsPackageFile( path ) )
        return OpenPackageFile( path );
    if ( CAssetTypeResolver::HasView( path ) )
    {
        const Int32 slash = path.HasChar( '/', false );
        m_browser->NavigateTo( VfsRootName, slash > 0 ? "/" + path.SubstrFromRange( 0, (UInt32) slash ) : CORE::CString( "/" ) );
        ShowLeftTab( 0 );
        return OpenAssetInteractive( path );
    }
    ++m_navigatingItself;
    m_browser->NavigateTo( VfsRootName, "/" + path );
    --m_navigatingItself;
    ShowLeftTab( 0 );
    UpdateEmptyState();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::IsOpenDialogShown( void ) const
{GUCEF_TRACE;

    return ( GUCEF_NULL != m_folderDialog && m_folderDialog->IsOpen() ) ||
           ( GUCEF_NULL != m_archiveDialog && m_archiveDialog->IsOpen() ) ||
           ( GUCEF_NULL != m_fileDialog && m_fileDialog->IsOpen() );
}

/*-------------------------------------------------------------------------*/

GUI::CFileSystemDialogForm*
CWorkbenchShell::MakeOpenDialog( const CORE::CString& kind )
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
        dialog->AddFileType( "Images and textures", textures );
        dialog->AddFileType( "Archives and packs", archives );
    }

    if ( !dialog->Build() )
    {
        GUCEF_DELETE dialog;
        return GUCEF_NULL;
    }
    TEventCallback acceptedCallback( this, &CWorkbenchShell::OnOpenDialogAccepted );
    SubscribeTo( dialog, GUI::CFileSystemDialogForm::AcceptedEvent, acceptedCallback );
    return dialog;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::PrepareOpenDialog( GUI::CFileSystemDialogForm& dialog )
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
CWorkbenchShell::ShowOpenDialog( GUI::CFileSystemDialogForm*& dialog, const CORE::CString& kind )
{GUCEF_TRACE;

    if ( !m_initialized || IsOpenDialogShown() )
        return false;
    if ( GUCEF_NULL == dialog )
        dialog = MakeOpenDialog( kind );
    if ( GUCEF_NULL == dialog )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchShell: The dialog to open something cannot be made" );
        return false;
    }
    PrepareOpenDialog( *dialog );
    return dialog->Open();
}

/*-------------------------------------------------------------------------*/

bool CWorkbenchShell::ShowOpenFolderDialog( void ) { return ShowOpenDialog( m_folderDialog, FolderDialogKind ); }
bool CWorkbenchShell::ShowOpenArchiveDialog( void ) { return ShowOpenDialog( m_archiveDialog, ArchiveDialogKind ); }
bool CWorkbenchShell::ShowOpenFileDialog( void ) { return ShowOpenDialog( m_fileDialog, FileDialogKind ); }

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::HandleKey( INPUT::KeyCode keyCode, UInt32 modifiers )
{GUCEF_TRACE;

    // the key of the developer console first: it is typed in a text field that has the focus
    if ( GUCEF_NULL != m_console && 0 == ( modifiers & ( INPUT::KEYMOD_CTRL | INPUT::KEYMOD_ALT ) ) && m_console->HandleKey( keyCode, modifiers ) )
        return true;
    return m_shortcuts.HandleKey( keyCode, modifiers );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::RegisterLayoutPresets( GUI::CLayoutManager& layout )
{GUCEF_TRACE;

    // Browse is the window as it starts, Review gives the room to the inspector and the report, Preview folds both sides
    GUI::CLayoutState browse;
    browse.SetSplitRatio( MainSplitName, 0.74f );
    browse.SetSplitRatio( SideSplitName, 0.2812f );
    browse.SetSplitRatio( ViewSplitName, 0.683f );
    browse.SetFolded( LeftPanelName, false );
    browse.SetFolded( RightPanelName, false );
    layout.AddPreset( LayoutBrowse, browse );

    GUI::CLayoutState review;
    review.SetSplitRatio( MainSplitName, 0.62f );
    review.SetSplitRatio( SideSplitName, 0.2812f );
    review.SetSplitRatio( ViewSplitName, 0.55f );
    review.SetFolded( LeftPanelName, true );
    review.SetFolded( RightPanelName, false );
    layout.AddPreset( LayoutReview, review );

    GUI::CLayoutState preview;
    preview.SetSplitRatio( MainSplitName, 0.74f );
    preview.SetSplitRatio( SideSplitName, 0.2812f );
    preview.SetSplitRatio( ViewSplitName, 0.683f );
    preview.SetFolded( LeftPanelName, true );
    preview.SetFolded( RightPanelName, true );
    layout.AddPreset( LayoutPreview, preview );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::SetLayoutManager( GUI::CLayoutManager* layout )
{GUCEF_TRACE;

    m_layout->Detach();
    m_layout = GUCEF_NULL != layout ? layout : &m_ownLayout;
    if ( GUCEF_NULL == m_root )
        return;
    RegisterLayoutPresets( *m_layout );
    m_layout->Attach( m_root );
}

/*-------------------------------------------------------------------------*/

GUI::CLayoutManager&
CWorkbenchShell::GetLayoutManager( void )
{GUCEF_TRACE;

    return *m_layout;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ApplyLayoutPreset( const CORE::CString& name )
{GUCEF_TRACE;

    return m_layout->ApplyPreset( name );
}

/*-------------------------------------------------------------------------*/

GUI::CFoldablePanel* CWorkbenchShell::GetLeftPanel( void ) const { return m_leftPanel; }
GUI::CFoldablePanel* CWorkbenchShell::GetRightPanel( void ) const { return m_rightPanel; }
GUI::CSplitter* CWorkbenchShell::GetMainSplitter( void ) const { return m_mainSplit; }
GUI::CSplitter* CWorkbenchShell::GetSideSplitter( void ) const { return m_sideSplit; }
GUI::CSplitter* CWorkbenchShell::GetViewSplitter( void ) const { return m_viewSplit; }
GUI::CKeyboardShortcutMap& CWorkbenchShell::GetShortcuts( void ) { return m_shortcuts; }

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ToggleLeftSide( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_leftPanel )
        m_leftPanel->ToggleFolded();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ToggleRightSide( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_rightPanel )
        m_rightPanel->ToggleFolded();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::TogglePreviewLayout( void )
{GUCEF_TRACE;

    if ( LayoutPreview == m_layout->GetActivePreset() )
    {
        // back to what it was before
        if ( m_beforePreview.IsEmpty() )
            m_layout->ApplyPreset( LayoutBrowse );
        else
            m_layout->ApplyState( m_beforePreview );
        return;
    }
    m_layout->CaptureState( m_beforePreview );
    m_layout->ApplyPreset( LayoutPreview );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnShortcutActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* action = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == action )
        return;
    // a key of the developer console without modifiers is the console's own (HandleKey()): it is in the map to be listed and not to be taken by
    // another action, and it is not typed away from a text field; a chord with modifiers is an action like the others
    if ( ActionConsole == action->GetData() )
    {
        INPUT::KeyCode key = INPUT::KEYCODE_UNASSIGNED;
        UInt32 modifiers = 0;
        if ( !m_shortcuts.GetChord( ActionConsole, key, modifiers ) || 0 == modifiers )
            return;
    }
    ExecuteAction( action->GetData() );
}

/*-------------------------------------------------------------------------*/

CWorkbenchJobRegistry&
CWorkbenchShell::GetJobRegistry( void )
{GUCEF_TRACE;

    return m_jobRegistry;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::AttachJobs( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_sources )
        return;
    CAssetStoreTransferModel* model = m_sources->GetTransferModel();
    if ( GUCEF_NULL != model )
        model->SetLocalJobs( &m_jobRegistry );
    ASSETSTORE::CIAssetStoreService* service = m_sources->GetService();
    if ( GUCEF_NULL != service && GUCEF_NULL == m_bulkController )
    {
        m_bulkController = GUCEF_NEW CBulkConversionController( *service );
        TEventCallback notified( this, &CWorkbenchShell::OnBulkNotification );
        SubscribeTo( m_bulkController, CBulkConversionController::NotificationEvent, notified );
    }
    if ( GUCEF_NULL != m_jobsForm )
    {
        m_jobsForm->SetModel( model );
        m_jobsForm->SetService( service );
    }
    if ( GUCEF_NULL != m_bulkForm )
    {
        m_bulkForm->SetController( m_bulkController );
        m_bulkForm->SetModel( model );
    }
    if ( GUCEF_NULL != service && GUCEF_NULL == m_syncController )
        m_syncController = GUCEF_NEW CLibrarySyncController( *service );
    if ( GUCEF_NULL != m_syncForm )
        m_syncForm->SetController( m_syncController );
}

/*-------------------------------------------------------------------------*/

CWorkbenchStoreConnection*
CWorkbenchShell::GetStoreConnection( void ) const
{GUCEF_TRACE;

    return m_storeConnection;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::InitializeStoreConnection( AUTH::ICredentialStore* store )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_storeConnection )
    {
        if ( GUCEF_NULL != store )
            m_storeConnection->SetCredentialStore( store );
        return true;
    }
    m_storeConnection = GUCEF_NEW CWorkbenchStoreConnection();
    if ( !m_storeConnection->Initialize( store ) )
    {
        GUCEF_DELETE m_storeConnection;
        m_storeConnection = GUCEF_NULL;
        return false;
    }
    TEventCallback readyCallback( this, &CWorkbenchShell::OnStoreServiceReady );
    SubscribeTo( m_storeConnection, CWorkbenchStoreConnection::ServiceReadyEvent, readyCallback );
    TEventCallback lostCallback( this, &CWorkbenchShell::OnStoreServiceLost );
    SubscribeTo( m_storeConnection, CWorkbenchStoreConnection::ServiceLostEvent, lostCallback );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnStoreServiceReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_sources || GUCEF_NULL == m_storeConnection )
        return;
    // what works on the model and the service of the page lets go first: the page deletes its model when it takes the other service
    DetachService();
    const bool taken = m_sources->UseService( m_storeConnection->GetService(), m_storeConnection->GetActiveEndpoint(), &m_storeConnection->GetServiceInfo() );
    ReattachService();
    if ( taken )
    {
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: the workbench works on the remote store " << m_storeConnection->GetStoreLabel() );
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnStoreServiceLost( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_sources )
        return;
    DetachService();
    const bool taken = m_sources->UseLocalService();
    ReattachService();
    if ( taken )
    {
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: the workbench works on the store of this computer again" );
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::DetachService( void )
{GUCEF_TRACE;

    // the jobs page, the bulk conversion and its controller hold the model and the service of the page
    if ( GUCEF_NULL != m_jobsForm )
    {
        m_jobsForm->SetModel( GUCEF_NULL );
        m_jobsForm->SetService( GUCEF_NULL );
    }
    if ( GUCEF_NULL != m_bulkForm )
    {
        m_bulkForm->SetController( GUCEF_NULL );
        m_bulkForm->SetModel( GUCEF_NULL );
    }
    GUCEF_DELETE m_bulkController;
    m_bulkController = GUCEF_NULL;
    if ( GUCEF_NULL != m_syncForm )
        m_syncForm->SetController( GUCEF_NULL );
    GUCEF_DELETE m_syncController;
    m_syncController = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ReattachService( void )
{GUCEF_TRACE;

    // the jobs page and the bulk conversion work on the service of the page again: a new controller for it
    AttachJobs();
}

/*-------------------------------------------------------------------------*/

CWorkbenchJobsForm*
CWorkbenchShell::GetJobsForm( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_jobsForm && m_initialized )
    {
        m_jobsForm = GUCEF_NEW CWorkbenchJobsForm( m_context );
        if ( !m_jobsForm->Build() )
        {
            GUCEF_DELETE m_jobsForm;
            m_jobsForm = GUCEF_NULL;
            return GUCEF_NULL;
        }
        m_jobsForm->GetRootWidget()->SetProperty( GUI::EWidgetProperty_HelpTopic, CWorkbenchHelp::TopicImporting );
        TEventCallback resultCallback( this, &CWorkbenchShell::OnJobResultRequested );
        SubscribeTo( m_jobsForm, CWorkbenchJobsForm::OpenResultRequestedEvent, resultCallback );
        AttachJobs();
    }
    return m_jobsForm;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ShowJobs( const CORE::CString& jobId )
{GUCEF_TRACE;

    CWorkbenchJobsForm* form = GetJobsForm();
    if ( GUCEF_NULL == form || !form->Open() )
        return false;
    if ( !jobId.IsNULLOrEmpty() )
        form->SelectJob( jobId );
    return true;
}

/*-------------------------------------------------------------------------*/

CWorkbenchJobSummary
CWorkbenchShell::GetJobSummary( void ) const
{GUCEF_TRACE;

    CAssetStoreTransferModel* model = GUCEF_NULL != m_sources ? m_sources->GetTransferModel() : GUCEF_NULL;
    if ( GUCEF_NULL != model )
        return model->GetSummary();
    ASSETSTORE::TAssetJobEntryVector jobs;
    m_jobRegistry.GetJobs( jobs );
    return CWorkbenchJobSummary::Count( jobs );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchShell::GetJobSummaryText( void ) const
{GUCEF_TRACE;

    return GetJobSummary().ToText();
}

/*-------------------------------------------------------------------------*/

GUI::CWidget*
CWorkbenchShell::GetJobSummaryButton( void ) const
{GUCEF_TRACE;

    return m_jobSummaryButton;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::RefreshJobSummary( void )
{GUCEF_TRACE;

    m_jobSummaryWaitMs = 1000000;
    UpdateJobSummary( 0 );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::UpdateJobSummary( UInt32 deltaInMs )
{GUCEF_TRACE;

    m_jobSummaryWaitMs += deltaInMs;
    if ( m_jobSummaryWaitMs < 250 )
        return;
    m_jobSummaryWaitMs = 0;
    const CWorkbenchJobSummary summary = GetJobSummary();
    if ( summary == m_shownSummary && ( GUCEF_NULL == m_jobSummaryButton || !m_jobSummaryButton->GetProperty( GUI::EWidgetProperty_Text ).IsNULLOrEmpty() ) )
        return;
    m_shownSummary = summary;
    // shown while something needs attention (running, waiting or failed jobs); the jobs menu and the console open the page at any time
    const bool any = ( summary.running + summary.queued + summary.failed ) > 0;
    if ( any && GUCEF_NULL == m_jobsHost )
    {
        // made when the first job comes: without jobs the status strip is the indicator alone
        m_jobsHost = MakePanel( *m_context, "JobsHost", m_statusHost, "0.5,0 0,0", "0.5,-1 1,0" );
        m_jobSummaryButton = GUCEF_NULL != m_jobsHost ? m_context->CreateWidget( "Button", "JobsSummary", m_jobsHost ) : GUCEF_NULL;
        if ( GUCEF_NULL != m_jobSummaryButton )
        {
            m_jobSummaryButton->SetProperty( GUI::EWidgetProperty_Position, "0,0 0,0" );
            m_jobSummaryButton->SetProperty( GUI::EWidgetProperty_Size, "1,0 1,0" );
            m_jobSummaryButton->SetProperty( GUI::EWidgetProperty_Tooltip, "The jobs of the workbench and of the asset store service: click to open the jobs page" );
            TEventCallback summaryCallback( this, &CWorkbenchShell::OnJobSummaryActivated );
            SubscribeTo( m_jobSummaryButton, GUI::CWidget::ActivatedEvent, summaryCallback );
        }
    }
    if ( GUCEF_NULL == m_jobSummaryButton )
        return;
    m_activityHost->SetProperty( GUI::EWidgetProperty_Size, any ? "0.5,0 1,0" : "1,0 1,0" );
    m_jobsHost->SetVisibility( any );
    m_jobSummaryButton->SetProperty( GUI::EWidgetProperty_Text, summary.ToShortText() );
    m_jobSummaryButton->SetProperty( GUI::EWidgetProperty_Tooltip, summary.ToText() + ": click to open the jobs page of the workbench and of the asset store service" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchShell::GetJobIdOfActivity( UInt32 activityId ) const
{GUCEF_TRACE;

    const UInt32 number = m_jobRegistry.FindByActivity( activityId );
    return 0 == number ? CORE::CString::Empty : CWorkbenchJobRegistry::IdPrefix + CORE::UInt32ToString( number );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnJobSummaryActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the indicator that shows now is the job to select
    GUI::CActivityService::TActivityInfoVector shown;
    m_activity.GetShownOperations( shown );
    ShowJobs( shown.empty() ? CORE::CString::Empty : GetJobIdOfActivity( shown[ shown.size() - 1 ].id ) );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnJobResultRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* jobId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != jobId )
        OpenJobResult( jobId->GetData() );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::OpenJobResult( const CORE::CString& jobId )
{GUCEF_TRACE;

    CAssetStoreTransferModel* model = GUCEF_NULL != m_sources ? m_sources->GetTransferModel() : GUCEF_NULL;
    ASSETSTORE::CAssetJobEntry job;
    if ( GUCEF_NULL == model || !model->GetJob( jobId, job ) || !job.IsEnded() || job.folder.IsNULLOrEmpty() )
        return false;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: Opening the result of the job " << jobId << ": " << job.folder );
    return OpenLocation( job.folder, true );
}

/*-------------------------------------------------------------------------*/

CBulkConversionForm*
CWorkbenchShell::GetBulkConversionForm( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_bulkForm && m_initialized )
    {
        m_bulkForm = GUCEF_NEW CBulkConversionForm( m_context );
        if ( !m_bulkForm->Build() )
        {
            GUCEF_DELETE m_bulkForm;
            m_bulkForm = GUCEF_NULL;
            return GUCEF_NULL;
        }
        m_bulkForm->GetRootWidget()->SetProperty( GUI::EWidgetProperty_HelpTopic, CWorkbenchHelp::TopicImporting );
        m_bulkForm->SetPreferences( m_preferences );
        TEventCallback changedCallback( this, &CWorkbenchShell::OnBulkTemplatesChanged );
        SubscribeTo( m_bulkForm, CBulkConversionForm::TemplatesChangedEvent, changedCallback );
        AttachJobs();
    }
    return m_bulkForm;
}

/*-------------------------------------------------------------------------*/

CBulkConversionController*
CWorkbenchShell::GetBulkConversionController( void ) const
{GUCEF_TRACE;

    return m_bulkController;
}

/*-------------------------------------------------------------------------*/

CLibrarySyncForm*
CWorkbenchShell::GetLibrarySyncForm( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_syncForm && m_initialized )
    {
        m_syncForm = GUCEF_NEW CLibrarySyncForm( m_context );
        if ( !m_syncForm->Build() )
        {
            GUCEF_DELETE m_syncForm;
            m_syncForm = GUCEF_NULL;
            return GUCEF_NULL;
        }
        m_syncForm->GetRootWidget()->SetProperty( GUI::EWidgetProperty_HelpTopic, CWorkbenchHelp::TopicImporting );
        AttachJobs();
    }
    return m_syncForm;
}

/*-------------------------------------------------------------------------*/

CLibrarySyncController*
CWorkbenchShell::GetLibrarySyncController( void ) const
{GUCEF_TRACE;

    return m_syncController;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ShowLibrarySync( void )
{GUCEF_TRACE;

    CLibrarySyncForm* form = GetLibrarySyncForm();
    return GUCEF_NULL != form && form->Open();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnBulkTemplatesChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RaisePreferencesChanged();
}

/*-------------------------------------------------------------------------*/

CBulkConversionSettings
CWorkbenchShell::MakeBulkScopeOfBrowser( void ) const
{GUCEF_TRACE;

    CBulkConversionSettings settings;
    if ( GUCEF_NULL != m_sources )
        settings.packId = m_sources->GetSelectedPackId();
    if ( GUCEF_NULL == m_browser )
        return settings;
    // the paths of the pack: relative to the root of the pack the browser shows
    const CORE::CString root = IsResourceViewShown() ? m_resourceViewPack : m_packRoot;
    const CORE::CString::StringVector selected = m_browser->GetSelectedPaths();
    for ( size_t i=0; i<selected.size(); ++i )
    {
        ASSETCONVERTER::CPackIndexResource resource;
        CORE::CString path;
        if ( IsResourceViewShown() && GetResourceOfPath( selected[ i ], resource ) )
            path = resource.path;
        else
        if ( !root.IsNULLOrEmpty() )
        {
            const CORE::CString normal = CAssetMounter::NormalizePath( selected[ i ] );
            if ( 0 == normal.HasSubstr( root + "/" ) )
                path = normal.CutChars( (UInt32) root.Length() + 1, true );
        }
        if ( !path.IsNULLOrEmpty() )
            settings.assets.push_back( path );
    }
    if ( !settings.assets.empty() )
    {
        settings.scopeKind = CBulkConversionSettings::ScopeSelection;
        return settings;
    }
    if ( IsResourceViewShown() )
    {
        CORE::CString groupId;
        bool isGroupFolder = false;
        if ( CResourceViewFileSystemAccess::GetGroupOfPath( m_browser->GetCurrentDirectory(), groupId, isGroupFolder ) && !groupId.IsNULLOrEmpty() && GUCEF_NULL != m_resourceView )
        {
            const ASSETCONVERTER::CPackIndexResource::TResourceVector& all = m_resourceView->GetManifest().resources;
            for ( size_t i=0; i<all.size(); ++i )
                if ( all[ i ].group == groupId )
                    settings.assets.push_back( all[ i ].path );
            if ( !settings.assets.empty() )
            {
                settings.scopeKind = CBulkConversionSettings::ScopeGroup;
                settings.groupId = groupId;
            }
        }
    }
    return settings;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ShowBulkConversion( void )
{GUCEF_TRACE;

    CBulkConversionForm* form = GetBulkConversionForm();
    if ( GUCEF_NULL == form || !form->Open() )
        return false;
    form->SetSettings( MakeBulkScopeOfBrowser() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::RedoStorePack( CWorkbenchRedoActions::ERedoAction action, const CORE::CString& packId, const CORE::CString::StringVector& assetGlobs )
{GUCEF_TRACE;

    ASSETSTORE::CIAssetStoreService* service = GUCEF_NULL != m_sources ? m_sources->GetService() : GUCEF_NULL;
    if ( GUCEF_NULL == service || packId.IsNULLOrEmpty() )
        return false;
    ASSETSTORE::CAssetPipelineRequest request;
    CWorkbenchRedoActions::MakePipelineRequest( action, packId, assetGlobs, request );
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: " << CWorkbenchRedoActions::MakeConfirmation( action, "the pack " + packId, 0 ) );
    if ( !service->IsInProcess() )
        return service->StartPipelineJobAsync( CORE::CString::Empty, request ).HasAFuture();
    ASSETSTORE::CAssetServiceResult result;
    const bool started = ASSETSTORE::ASSETSTORE_STATUS_OK == service->StartPipelineJob( CORE::CString::Empty, request, result );
    if ( !started )
        m_report->AddEntry( GUI::CReportViewForm::SEVERITY_WARNING, packId, CORE::CString( CWorkbenchRedoActions::GetInfo( action ).menuText ) + " was refused", result.message );
    return started;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::RedoFolder( CWorkbenchRedoActions::ERedoAction action, const CORE::CString& vfsFolder )
{GUCEF_TRACE;

    ASSETCONVERTER::CPackRedoRequest redo;
    if ( !m_initialized || !CWorkbenchRedoActions::MakeLocalRedo( action, redo ) )
        return false;
    const CORE::CString root = CAssetMounter::NormalizePath( vfsFolder );
    if ( root.IsNULLOrEmpty() )
        return false;
    TAssetLoadJobPtr job( GUCEF_NEW CPackIndexJob( *this, root, redo ) );
    CAssetMounter::SMount mount;
    CORE::CString relative;
    const CORE::CString label = CORE::CString( CWorkbenchRedoActions::GetInfo( action ).menuText ) + " " + ( m_mounter.FindMount( root, mount, relative ) ? mount.mountPoint : root );
    m_indexingFolder = root;
    m_indexAutoSwitch = false;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: " << CWorkbenchRedoActions::MakeConfirmation( action, root, 0 ) );
    m_indexLoader.DescribeNext( "index", CWorkbenchRedoActions::GetJobSubKind( action ), root );
    return !m_indexLoader.Begin( job, label ).HasNoFuture();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::RedoPack( CWorkbenchRedoActions::ERedoAction action )
{GUCEF_TRACE;

    // the index steps on the pack of the resource view run here; the others, and a pack that is only in the store, are jobs of the service
    if ( CWorkbenchRedoActions::IsLocal( action ) && !m_resourceViewPack.IsNULLOrEmpty() )
        return RedoFolder( action, m_resourceViewPack );
    CORE::CString::StringVector globs;
    const CBulkConversionSettings scope = MakeBulkScopeOfBrowser();
    if ( CBulkConversionSettings::ScopeSelection == scope.scopeKind || CBulkConversionSettings::ScopeGroup == scope.scopeKind )
        globs = scope.assets;
    return RedoStorePack( action, scope.packId, globs );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::RegisterEvents( void )
{GUCEF_TRACE;

    ActionRequestedEvent.Initialize();
    PreferencesChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

const CWorkbenchShell::TActionInfo*
CWorkbenchShell::GetActionTable( UInt32& count )
{GUCEF_TRACE;

    count = (UInt32) ( sizeof( g_actions ) / sizeof( g_actions[ 0 ] ) );
    return g_actions;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::BindShortcuts( void )
{GUCEF_TRACE;

    UInt32 count = 0;
    const TActionInfo* actions = GetActionTable( count );
    for ( UInt32 i=0; i<count; ++i )
        BindShortcut( m_shortcuts, actions[ i ].actionId, actions[ i ].defaultKey, actions[ i ].defaultModifiers );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ExecuteAction( const CORE::CString& actionId )
{GUCEF_TRACE;

    if ( ActionFoldLeft == actionId )
        ToggleLeftSide();
    else
    if ( ActionFoldRight == actionId )
        ToggleRightSide();
    else
    if ( ActionPreview == actionId )
        TogglePreviewLayout();
    else
    if ( ActionConvertForOgre == actionId )
        ConvertForOgre();
    else
    if ( ActionOpenFile == actionId )
    {
        if ( !IsOpenDialogShown() )
            ShowOpenFileDialog();
    }
    else
    if ( ActionOpenFolder == actionId )
    {
        if ( !IsOpenDialogShown() )
            ShowOpenFolderDialog();
    }
    else
    if ( ActionOpenArchive == actionId )
    {
        if ( !IsOpenDialogShown() )
            ShowOpenArchiveDialog();
    }
    else
    if ( 0 == actionId.HasSubstr( "Layout:" ) )
        return ApplyLayoutPreset( actionId.SubstrToIndex( 7, false ) );
    else
    if ( "ResourceView" == actionId )
        ShowResourceView( !IsResourceViewShown() );
    else
    if ( "Jobs" == actionId )
        return ShowJobs();
    else
    if ( "BulkConversion" == actionId )
        return ShowBulkConversion();
    else
    if ( "LibrarySync" == actionId )
        return ShowLibrarySync();
    else
    if ( 0 == actionId.HasSubstr( "Redo:" ) )
    {
        CWorkbenchRedoActions::ERedoAction redo = CWorkbenchRedoActions::REDO_REINDEX;
        return CWorkbenchRedoActions::ParseActionId( actionId, redo ) && RedoPack( redo );
    }
    else
    if ( ActionReIndex == actionId )
        return ReIndexPack();
    else
    if ( ActionPackPresentation == actionId )
        SetPackPresentationEnabled( !IsPackPresentationEnabled() );
    else
    if ( "PresentationMore" == actionId )
        return LoadMoreOfPresentation();
    else
    if ( 0 == actionId.HasSubstr( ActionPresentationPrefix ) )
    {
        const CORE::CString kind = actionId.SubstrFromRange( (UInt32) CORE::CString( ActionPresentationPrefix ).Length(), actionId.Length() );
        if ( m_packRoot.IsNULLOrEmpty() )
            return false;
        // a menu item is an explicit request, whatever the switch of the presentation says
        return "default" == kind ? ResetPackPresentation() : ShowPackPresentation( m_packRoot, kind, true );
    }
    else
    if ( ActionSettings == actionId || ActionHelp == actionId || ActionConsole == actionId || ActionConnectStore == actionId || 0 == actionId.HasSubstr( "Help:" ) )
    {
        CORE::TCloneableString data( actionId );
        NotifyObservers( ActionRequestedEvent, &data );
    }
    else
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::BuildMenuBar( void )
{GUCEF_TRACE;

    m_menuBar = m_context->CreateWidget( "MenuBar", "MenuBar", m_root );
    if ( GUCEF_NULL == m_menuBar )
        return;
    m_menuBar->SetProperty( GUI::EWidgetProperty_Position, "0,0 0,0" );
    m_menuBar->SetProperty( GUI::EWidgetProperty_Size, "1,0 0," + CORE::UInt32ToString( MenuBarHeight ) );
    GUI::TStringVector items;
    for ( UInt32 i=0; GUCEF_NULL != g_menuItems[ i ].path; ++i )
        items.push_back( g_menuItems[ i ].path );
    m_menuBar->SetProperty( GUI::EWidgetProperty_Items, GUI::JoinStringList( items ) );
    TEventCallback itemCallback( this, &CWorkbenchShell::OnMenuItemActivated );
    SubscribeTo( m_menuBar, GUI::CWidget::ItemActivatedEvent, itemCallback );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::UpdateMenuBar( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_menuBar || GUCEF_NULL == m_mainSplit || GUCEF_NULL == m_context )
        return;
    // the height of the row is that of the text of the menus, so that a larger font does not clip them
    UInt32 height = 0;
    if ( m_context->GetLogicalHeight() >= MenuBarMinWindowHeight )
    {
        Int32 naturalWidth = 0, naturalHeight = 0;
        height = m_menuBar->GetNaturalSize( naturalWidth, naturalHeight ) && naturalHeight > (Int32) MenuBarHeight ? (UInt32) naturalHeight : MenuBarHeight;
    }
    if ( height == m_menuBarHeight && m_menuBarHeightKnown )
        return;
    m_menuBarHeightKnown = true;
    m_menuBarHeight = height;
    m_menuBar->SetBoolProperty( GUI::EWidgetProperty_Visible, height > 0 );
    if ( height > 0 )
        m_menuBar->SetProperty( GUI::EWidgetProperty_Size, "1,0 0," + CORE::UInt32ToString( height ) );
    m_mainSplit->SetProperty( GUI::EWidgetProperty_Position, "0,0 0," + CORE::UInt32ToString( height ) );
    m_mainSplit->SetProperty( GUI::EWidgetProperty_Size, "1,0 1,-" + CORE::UInt32ToString( height ) );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnMenuItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const GUI::CItemEventData* item = CORE::CICloneable::CloneableAs< GUI::CItemEventData >( eventData );
    if ( GUCEF_NULL == item )
        return;
    for ( UInt32 i=0; GUCEF_NULL != g_menuItems[ i ].path; ++i )
    {
        if ( item->item == g_menuItems[ i ].path )
        {
            ExecuteAction( g_menuItems[ i ].actionId );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchShell::GetMenuPathOfAction( const CORE::CString& actionId ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; GUCEF_NULL != g_menuItems[ i ].path; ++i )
        if ( actionId == g_menuItems[ i ].actionId )
            return g_menuItems[ i ].path;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

GUI::CGUIContext*
CWorkbenchShell::GetGUIContext( void ) const
{GUCEF_TRACE;

    return m_context;
}

/*-------------------------------------------------------------------------*/

GUI::CWidget*
CWorkbenchShell::GetMenuBar( void ) const
{GUCEF_TRACE;

    return m_menuBar;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::SetDeveloperConsole( GUI::CDeveloperConsoleForm* console )
{GUCEF_TRACE;

    m_console = console;
}

/*-------------------------------------------------------------------------*/

GUI::CDeveloperConsoleForm*
CWorkbenchShell::GetDeveloperConsole( void ) const
{GUCEF_TRACE;

    return m_console;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnRootTextInput( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CTextEventData* text = CORE::CICloneable::CloneableAs< GUI::CTextEventData >( eventData );
    if ( GUCEF_NULL == text || text->handled || GUI::EVENTPHASE_BUBBLE == text->phase || GUCEF_NULL == m_console )
        return;
    // the character of the console key that reached the window, or the character a layout has on another key
    if ( m_console->HandleText( text->text ) )
        text->handled = true;
}

/*-------------------------------------------------------------------------*/

CViewPresetTracker&
CWorkbenchShell::GetViewPresetTracker( void )
{GUCEF_TRACE;

    return m_viewPreset;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchShell::GetViewPresetLabel( void ) const
{GUCEF_TRACE;

    return m_viewPreset.GetLabel();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::IsFourViewsShown( void ) const
{GUCEF_TRACE;

    return m_viewPreset.IsFourViews();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ApplyViewPreset( const CORE::CString& name )
{GUCEF_TRACE;

    CViewPresetTracker::EPreset preset = CViewPresetTracker::PRESET_FRAME;
    if ( !CViewPresetTracker::TryParsePreset( name, preset ) || CViewPresetTracker::PRESET_CUSTOM == preset )
        return false;
    m_viewPreset.Apply( preset, m_controller );
    ApplyViewLayout();
    ShowViewPresetLabel();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::SetDefaultViewPreset( EAssetKind kind, const CORE::CString& presetName )
{GUCEF_TRACE;

    if ( (Int32) kind >= 0 && (Int32) kind < (Int32) ASSETKIND_COUNT )
        m_defaultViewPresets[ kind ] = presetName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchShell::GetDefaultViewPreset( EAssetKind kind ) const
{GUCEF_TRACE;

    return (Int32) kind >= 0 && (Int32) kind < (Int32) ASSETKIND_COUNT ? m_defaultViewPresets[ kind ] : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ResetViewPreset( EAssetKind kind )
{GUCEF_TRACE;

    // a new asset starts framed; the four views stay while they are on, and the kind may name a preset of its own
    CViewPresetTracker::EPreset preset = m_viewPreset.IsFourViews() ? CViewPresetTracker::PRESET_FOUR_VIEWS : CViewPresetTracker::PRESET_FRAME;
    CViewPresetTracker::EPreset named = CViewPresetTracker::PRESET_FRAME;
    if ( !GetDefaultViewPreset( kind ).IsNULLOrEmpty() && CViewPresetTracker::TryParsePreset( GetDefaultViewPreset( kind ), named ) && CViewPresetTracker::PRESET_CUSTOM != named )
        preset = named;
    m_viewPreset.Apply( preset, m_controller );
    ApplyViewLayout();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ApplyViewLayout( void )
{GUCEF_TRACE;

    if ( !m_stage.IsInitialized() )
        return;
    const bool wanted = m_viewPreset.IsFourViews();
    if ( !m_stage.SetViewLayout( wanted ? CWorkbenchViewLayout::MakeFourViews() : CWorkbenchViewLayout::MakeSingle() ) )
        return;
    m_stage.ApplyController( m_controller );
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::UpdateViewPreset( void )
{GUCEF_TRACE;

    if ( m_viewPreset.Update( m_controller ) )
        ShowViewPresetLabel();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ShowViewPresetLabel( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_viewport )
        return;
    // what the shell puts in the list itself is no choice of the user: no preset is applied because of it
    m_updatingViewLabel = true;
    if ( m_viewPreset.IsCustom() )
        m_viewport->SetToggleDisplayText( ToggleView, m_viewPreset.GetLabel() );
    else
        m_viewport->SetToggleChoiceSilently( ToggleView, (Int32) m_viewPreset.GetPreset() );
    m_updatingViewLabel = false;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnRootKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CKeyEventData* key = CORE::CICloneable::CloneableAs< GUI::CKeyEventData >( eventData );
    if ( GUCEF_NULL == key || key->handled || GUI::EVENTPHASE_BUBBLE == key->phase )
        return;
    if ( HandleKey( key->keyCode, key->modifiers ) )
        key->handled = true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnOpenActionRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
CWorkbenchShell::OnOpenRecentChosen( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* path = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != path )
        OpenLocation( path->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnOpenDialogAccepted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
CWorkbenchShell::Update( UInt32 thumbnailBudgetInMs )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;
    UpdateMenuBar();
    UpdateJobSummary( 0 );
    m_logger.Pump( m_report );
    m_loader.Update();
    m_indexLoader.Update();
    m_presentationJobActive = m_presentationJobActive && m_loader.IsBusy();
    // the conversion of the scene of a pack that takes longer than the preference allows is cancelled (the importer stops within a moment,
    // what it did is dropped) and the generated showcase is shown; it loads in another thread pool, as the one of the conversion is busy
    // until the importer has seen the cancel
    if ( m_presentationJobActive && CPackPresentation::KIND_SCENE_DEMO == m_presentationJobKind && m_loader.IsBusy() && m_presentationRetries < 2 &&
         ( MT::PrecisionTickCount() - m_presentationStartTick ) * 1000 / MT::PrecisionTimerResolution() > (UInt64) m_preferences->maxSceneSeconds * 1000 )
    {
        ++m_presentationRetries;
        m_presentationJobActive = false;
        const CORE::CString root = m_packRoot;
        const CORE::CString reason = "The conversion of the scene took longer than " + CORE::UInt32ToString( m_preferences->maxSceneSeconds ) + " s and was cancelled";
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchShell: " << reason );
        m_loader.Cancel();
        m_loader.SetThreadPoolName( CAssetLoader::DefaultThreadPoolName + "_" + CORE::UInt32ToString( ++m_poolSerial ) );
        StartPackPresentation( root, CORE::CString::Empty, false, CPackPresentation::KIND_SCENE_DEMO, reason );
    }
    if ( GUCEF_NULL != m_thumbnails )
        m_thumbnails->Update( thumbnailBudgetInMs );
    if ( GUCEF_NULL != m_sources )
        m_sources->Update( thumbnailBudgetInMs );
}

/*-------------------------------------------------------------------------*/

CReportLogger&
CWorkbenchShell::GetLogger( void )
{GUCEF_TRACE;

    return m_logger;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnOgreFrameStarted( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( !m_initialized || !m_stage.IsInitialized() )
        return;
    UpdateJobSummary( deltaInMs );
    if ( m_controller.ConsumeChanged() )
    {
        if ( ASSETKIND_MODEL == m_shownKind )
            m_model.Update( m_controller );
        else
        if ( ASSETKIND_MATERIAL == m_shownKind )
            m_material.GetPreview().Update( m_controller );
        m_stage.ApplyController( m_controller );
        UpdateViewPreset();
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
    UpdateAutoThumbnailCapture( deltaInMs );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      PACK PRESENTATIONS                                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CWorkbenchShell::SetPreferences( CWorkbenchPreferences* preferences )
{GUCEF_TRACE;

    m_preferences = GUCEF_NULL != preferences ? preferences : &m_ownPreferences;
    if ( GUCEF_NULL != m_capture )
        m_capture->SetPreferCookedFlag( &m_preferences->preferCookedOutput );
    m_autoThumbnails.SetEnabled( m_preferences->automaticThumbnails );
}

/*-------------------------------------------------------------------------*/

CWorkbenchPreferences&
CWorkbenchShell::GetPreferences( void )
{GUCEF_TRACE;

    return *m_preferences;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ReleaseConversion( void )
{GUCEF_TRACE;

    // the folder of a conversion is a mount while what it made is shown (the pictures of a model load when it is drawn first)
    if ( !m_hasConversion )
        return;
    m_mounter.Forget( CPackAssetConverter::GetCacheMountPoint( m_conversion ) );
    CPackAssetConverter::Unmount( m_conversion );
    m_hasConversion = false;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::KeepConversion( const CPackAssetConverter::SRequest& request )
{GUCEF_TRACE;

    ReleaseConversion();
    m_conversion = request;
    m_hasConversion = true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::RaisePreferencesChanged( void )
{GUCEF_TRACE;

    NotifyObservers( PreferencesChangedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::IsPackPresentationEnabled( void ) const
{GUCEF_TRACE;

    return m_presentationOverride >= 0 ? 0 != m_presentationOverride : m_preferences->openPackPresentation;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::SetPackPresentationEnabled( bool enabled )
{GUCEF_TRACE;

    m_preferences->openPackPresentation = enabled;
    // a switch of the user in this run, from the menu or the console, is the last word
    m_presentationOverride = -1;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: The presentation of a pack that is opened is " << ( enabled ? "on" : "off" ) );
    RaisePreferencesChanged();
}

/*-------------------------------------------------------------------------*/

void CWorkbenchShell::SetPackPresentationOverride( Int32 value ) { m_presentationOverride = value; }
Int32 CWorkbenchShell::GetPackPresentationOverride( void ) const { return m_presentationOverride; }
void CWorkbenchShell::SetPresentationKindOverride( const CORE::CString& kindId ) { m_kindOverride = kindId; }

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::SetAutomaticThumbnailsEnabled( bool enabled )
{GUCEF_TRACE;

    m_preferences->automaticThumbnails = enabled;
    m_autoThumbnails.SetEnabled( enabled );
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: Automatic thumbnails are " << ( enabled ? "on" : "off" ) );
    RaisePreferencesChanged();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchShell::GetPackKey( const CORE::CString& vfsFolder ) const
{GUCEF_TRACE;

    const CORE::CString path = CAssetMounter::NormalizePath( vfsFolder );
    CAssetMounter::SMount mount;
    CORE::CString relative;
    if ( m_mounter.FindMount( path, mount, relative ) )
        return CPackPresentationResolver::MakePackKey( mount.source + ( relative.IsNULLOrEmpty() ? CORE::CString::Empty : "/" + relative ) );
    return CPackPresentationResolver::MakePackKey( path );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::OpenPack( const CORE::CString& vfsFolder, bool explicitRequest )
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;
    // the index of the pack for the resource view is got whatever the switch of the presentation says; a folder that is no pack ends it at once
    StartPackIndex( vfsFolder, false );
    if ( !explicitRequest && !IsPackPresentationEnabled() )
        return false;
    return ShowPackPresentation( vfsFolder, CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ShowPackPresentation( const CORE::CString& vfsFolder, const CORE::CString& kindId, bool userChoice )
{GUCEF_TRACE;

    m_presentationRetries = 0;
    m_presentationMaxAssets = 48;
    return StartPackPresentation( vfsFolder, kindId, userChoice, CPackPresentation::KIND_SUMMARY, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ResetPackPresentation( void )
{GUCEF_TRACE;

    if ( m_packRoot.IsNULLOrEmpty() )
        return false;
    m_preferences->ClearPackPresentation( m_packKey );
    RaisePreferencesChanged();
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: The presentation of " << m_packRoot << " is back to its default" );
    return ShowPackPresentation( m_packRoot, CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::StartPackPresentation( const CORE::CString& vfsFolder, const CORE::CString& requestedId, bool userChoice, CPackPresentation::EKind failedKind, const CORE::CString& failedReason )
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;
    const CORE::CString root = CAssetMounter::NormalizePath( vfsFolder );
    if ( root.IsNULLOrEmpty() || !m_packSummary.Read( root ) || !m_packSummary.IsRecognisedPack() )
        return false;
    const CORE::CString key = GetPackKey( root );
    CAssetMounter::SMount mount;
    CORE::CString relative;
    const bool inMount = m_mounter.FindMount( root, mount, relative );

    // what the user chose before, until something else is asked for; a user choice is kept for the next time
    CORE::CString kindId = requestedId;
    if ( kindId.IsNULLOrEmpty() && failedReason.IsNULLOrEmpty() && !m_preferences->GetPackPresentation( key, kindId ) )
        kindId = m_kindOverride;
    if ( userChoice && !requestedId.IsNULLOrEmpty() )
    {
        m_preferences->SetPackPresentation( key, requestedId );
        RaisePreferencesChanged();
    }
    CPackPresentationResolver::SOptions options;
    CPackPresentationResolver::DetectCapabilities( options.capabilities );
    options.maxAssets = m_presentationMaxAssets;
    options.requestedId = kindId;
    options.failedKind = failedKind;
    options.failedReason = failedReason;

    // the scene of the pack converts whatever its size: the conversion reports its progress, it can be cancelled, and its result is kept
    // for the next open of the pack
    const ASSETCONVERTER::CAssetPackProfile& profile = m_packSummary.GetProfile();
    CPackPresentation presentation;
    CPackPresentationResolver::Resolve( m_packSummary.GetListing(), profile, root, options, presentation );
    m_offered.clear();
    CPackPresentationResolver::GetOffered( m_packSummary.GetListing(), profile, root, options, m_offered );
    m_packRoot = root;
    m_packKey = key;
    m_packSource = inMount ? mount.source : CORE::CString::Empty;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: Presentation of " << root << ": " << CPackPresentation::GetKindName( presentation.kind ) << ". " << presentation.why );

    TAssetLoadJobPtr job( GUCEF_NEW CPackPresentationJob( *this, presentation, m_packSource, m_packCacheFolder ) );
    const CORE::CString label = "Opening " + ( inMount ? mount.mountPoint : root );
    m_presentationJobActive = true;
    m_presentationJobKind = presentation.kind;
    m_presentationStartTick = MT::PrecisionTickCount();
    m_openingPath = root;
    // a presentation always loads in the background: a pack takes time, and the interface goes on with the activity indicator
    m_loader.DescribeNext( "presentation", CORE::CString::Empty, root );
    return !m_loader.Begin( job, label ).HasNoFuture();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::LoadMoreOfPresentation( void )
{GUCEF_TRACE;

    if ( !m_hasPresentation || 0 == m_presentation.moreAssets )
        return false;
    m_presentationMaxAssets += 48;
    return StartPackPresentation( m_packRoot, CPackPresentation::GetKindId( m_presentation.kind ), false, CPackPresentation::KIND_SUMMARY, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool CWorkbenchShell::HasPackPresentation( void ) const { return m_hasPresentation; }
const CPackPresentation& CWorkbenchShell::GetPackPresentation( void ) const { return m_presentation; }
const GUCEF::vector< CPackPresentation::EKind >& CWorkbenchShell::GetOfferedPresentations( void ) const { return m_offered; }
const CORE::CString& CWorkbenchShell::GetPackRoot( void ) const { return m_packRoot; }
CShowcaseView& CWorkbenchShell::GetShowcase( void ) { return m_showcase; }
const CORE::CString& CWorkbenchShell::GetPackCacheFolder( void ) const { return m_packCacheFolder; }
CAutoThumbnailStore& CWorkbenchShell::GetAutoThumbnails( void ) { return m_autoThumbnails; }

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ApplyPresentationView( const CORE::CString& presetName )
{GUCEF_TRACE;

    // an effect is framed by the box it filled while it ran, so that a small flame fills a good part of every view
    FrameEffectExtent();
    CViewPresetTracker::EPreset preset = CViewPresetTracker::PRESET_FRAME;
    if ( !presetName.IsNULLOrEmpty() && CViewPresetTracker::TryParsePreset( presetName, preset ) && CViewPresetTracker::PRESET_CUSTOM != preset )
    {
        m_viewPreset.Apply( preset, m_controller );
        ApplyViewLayout();
        m_stage.ApplyController( m_controller );
        ShowViewPresetLabel();
    }
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::FrameEffectExtent( void )
{GUCEF_TRACE;

    if ( 0 == m_particles.GetSystemCount() || m_model.IsLoaded() )
        return;
    COrbitController::SVector3 minimum, maximum;
    // how far the effect reaches is seen by running it for a moment, once: the time goes back to where it was
    if ( !m_particles.HasObservedRun() )
        m_particles.ObserveRun( EffectObserveSeconds );
    if ( !m_particles.GetBoundingBox( minimum, maximum, MinimumEffectFrame ) )
        return;
    m_controller.SetBoundsBox( minimum, maximum, EffectFillFraction );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: The effect is framed by the box (" << minimum.x << ", " << minimum.y << ", " << minimum.z << ") to (" << maximum.x << ", " << maximum.y << ", " << maximum.z << ")" );
    m_stage.SetGridUnit( m_controller.GetBoundsRadius() > 0.0f ? m_controller.GetBoundsRadius() / 2.0f : 0.1f );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ApplyPresentationCamera( const CPackPresentation& presentation )
{GUCEF_TRACE;

    if ( CPackPresentation::KIND_PORTRAIT != presentation.kind )
        return;
    // the lights of a studio, whatever the character was shown with before
    m_stage.SetLightPreset( CWorkbenchStage::LIGHTPRESET_THREE_POINT );
    if ( !presentation.camera.valid )
        return;
    const CPackPresentation::SCameraHint& hint = presentation.camera;
    const Float32 dx = hint.eye[ 0 ] - hint.target[ 0 ], dy = hint.eye[ 1 ] - hint.target[ 1 ], dz = hint.eye[ 2 ] - hint.target[ 2 ];
    const Float32 distance = sqrtf( dx * dx + dy * dy + dz * dz );
    if ( distance < 1e-5f )
        return;
    m_controller.SetFieldOfView( hint.fovDegrees );
    m_controller.SetTarget( COrbitController::SVector3( hint.target[ 0 ], hint.target[ 1 ], hint.target[ 2 ] ) );
    m_controller.SetAngles( atan2f( dx, dz ), asinf( dy / distance ) );
    m_controller.SetDistance( distance );
    m_stage.ApplyController( m_controller );
    m_stage.MarkDirty();
    UpdateViewPreset();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::PlayFirstClip( void )
{GUCEF_TRACE;

    if ( m_model.GetClips().empty() || GUCEF_NULL == m_timeline )
        return;
    m_timeline->SelectClip( 0 );
    m_timeline->SetLooping( true );
    m_timeline->Play();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::FinishShowcase( void )
{GUCEF_TRACE;

    CPackShowcaseLayout::SOptions options;
    options.minShare = ShowcaseMinimumShare;
    m_showcase.Arrange( options );
    // the box of the arrangement (the ground it stands on, the tallest model and the names in front of the first row) fills the view
    Float32 minX = 0.0f, maxX = 0.0f, minZ = 0.0f, maxZ = 0.0f, maxHeight = 0.0f;
    m_showcase.GetLayout().GetExtent( minX, maxX, minZ, maxZ, maxHeight );
    const Float32 pitch = m_showcase.GetLayout().GetPitch();
    m_controller.SetBoundsBox( COrbitController::SVector3( minX, 0.0f, minZ ), COrbitController::SVector3( maxX, maxHeight, maxZ + 0.2f * pitch ), 0.95f, true );
    m_timeline->ClearClips();
    m_shownKind = ASSETKIND_FOLDER;
    ApplyPresentationView( "Three quarter" );
    // a wide, flat arrangement fills a wide view better from a flatter angle and with the corner turned less than the three quarter view
    // does (that view is limited by the height of the arrangement): the names in front of the models also face the camera more
    m_controller.SetAngles( ShowcaseYaw, ShowcasePitch );
    m_controller.Frame();
    m_stage.ApplyController( m_controller );
    UpdateViewPreset();
    SetModelToggles( false );
    ShowCenterTab( CENTERTAB_3D );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      COOKED OUTPUT                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CWorkbenchShell::IsCookedOutputPreferred( void ) const
{GUCEF_TRACE;

    return m_preferences->preferCookedOutput;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::SetCookedOutputPreferred( bool preferred )
{GUCEF_TRACE;

    m_preferences->preferCookedOutput = preferred;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: The cooked output is " << ( preferred ? "preferred over the quick look" : "not preferred: the quick look is shown" ) );
    RaisePreferencesChanged();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchShell::GetShownSourceDescription( void ) const
{GUCEF_TRACE;

    if ( m_hasPresentation )
        return m_presentation.GetSourceDescription();
    if ( ASSETKIND_MODEL == m_shownKind && m_model.IsLoaded() )
        return m_sourceText.IsNULLOrEmpty() ? CModelView::GetSourceDescription( m_model.GetMeshName() ) : m_sourceText;
    if ( ASSETKIND_SCENE == m_shownKind )
        return m_sourceText;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CCookedOutputResolver::SCookedOutput&
CWorkbenchShell::GetShownCookedOutput( void ) const
{GUCEF_TRACE;

    return m_cooked;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::IsConvertForOgrePending( void ) const
{GUCEF_TRACE;

    return m_cookPending;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchShell::GetSourceReport( void ) const
{GUCEF_TRACE;

    const CORE::CString shown = m_hasPresentation ? m_packRoot : m_shownPath;
    const CORE::CString setting = CORE::CString( "Prefer cooked output is " ) + ( m_preferences->preferCookedOutput ? "on" : "off" );
    if ( shown.IsNULLOrEmpty() )
        return "Nothing is shown. " + setting;
    const CORE::CString source = GetShownSourceDescription();
    CORE::CString text = shown + ": source: " + ( source.IsNULLOrEmpty() ? CORE::CString( "not a model, scene or pack" ) : source );
    if ( m_hasCooked && m_cookedFor == shown )
        text += ". " + CCookedOutputResolver::Describe( m_cooked );
    return text + ". " + setting;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ProbeCooked( const CORE::CString& vfsPath, CCookedOutputResolver::SCookedOutput& cooked ) const
{GUCEF_TRACE;

    cooked = CCookedOutputResolver::SCookedOutput();
    if ( !m_preferences->preferCookedOutput )
        return false;
    const CORE::CString extension = CAssetTypeResolver::GetExtension( vfsPath ).Lowercase();
    if ( CCookedOutputResolver::IsCookedMainExtension( extension ) )
        return false;
    CCookedOutputResolver::LocateInStore( vfsPath, CORE::CString::Empty, CCookedOutputResolver::SOptions(), cooked );
    return cooked.IsLocated();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::NoteSource( const CORE::CString& vfsPath, const CCookedOutputResolver::SCookedOutput& cooked, bool probed, bool usedCooked )
{GUCEF_TRACE;

    m_cooked = cooked;
    m_hasCooked = true;
    m_cookedFor = vfsPath;
    if ( usedCooked )
    {
        m_sourceText = CCookedOutputResolver::GetCookedLabel();
        return;
    }
    m_sourceText.Clear();
    if ( CCookedOutputResolver::IsCookedMainExtension( CAssetTypeResolver::GetExtension( vfsPath ) ) )
        return;
    const CORE::CString base = CModelView::GetSourceDescription( vfsPath );
    if ( !m_preferences->preferCookedOutput )
    {
        m_sourceText = base + "; Prefer cooked output is off";
        return;
    }
    if ( !probed && cooked.reason.IsNULLOrEmpty() )
    {
        m_sourceText = base;
        return;
    }
    const bool convertible = "store" == cooked.origin && !cooked.IsCurrent();
    m_sourceText = base + "; " + CCookedOutputResolver::Describe( cooked ) + ( convertible ? CORE::CString( ", use Convert for Ogre" ) : CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ShowCooked( const CCookedOutputResolver::SCookedOutput& cooked, CModelView::SPreparedMesh* prepared, bool& isScene )
{GUCEF_TRACE;

    isScene = "scene" == CAssetTypeResolver::GetExtension( cooked.mainFile ).Lowercase();
    CORE::CString relative;
    const CORE::CString group = m_mounter.GetResourceGroup( cooked.mainFile, relative );
    if ( group.IsNULLOrEmpty() )
    {
        ReportProblem( cooked.mainFile, "The cooked file is not in a mount" );
        return false;
    }
    // the scene and the materials name their meshes and pictures by file name: the folder of the cooked profile is a place of the group
    if ( !cooked.cookedFolder.IsNULLOrEmpty() )
        m_mounter.AddResourceLocation( cooked.cookedFolder + "/" + cooked.profile );
    if ( isScene )
        return ShowScene( cooked.mainFile, false );
    // the materials of the cooked files are scripts next to the mesh
    CParticlePreview::ParseMaterialsNear( group, relative );
    return ShowModelLoad( cooked.mainFile, prepared );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ReopenShown( const CORE::CString& path )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return false;
    // the cooked files are new: the resource group of their mount is made again to see them
    m_mounter.InvalidateResourceGroup( path );
    // a file of a remote store: the new files are fetched first, then the views open it (the page asks for it)
    if ( GUCEF_NULL != m_sources && m_sources->IsUsingRemoteService() && !( m_hasPresentation && path == m_packRoot ) && m_sources->ReopenRemotePath( path ) )
        return true;
    if ( m_hasPresentation && path == m_packRoot )
        return ShowPackPresentation( path, CPackPresentation::GetKindId( m_presentation.kind ), false );
    return OpenAssetInteractive( path );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnBulkNotification( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_cookPending )
        return;
    const CORE::TCloneableString* text = static_cast< const CORE::TCloneableString* >( eventData );
    m_cookPending = false;
    const CORE::CString path = m_cookPendingPath;
    m_cookPendingPath.Clear();
    m_report->AddEntry( GUI::CReportViewForm::SEVERITY_INFO, path, "Convert for Ogre finished", GUCEF_NULL != text ? text->GetData() : CORE::CString::Empty );
    // the view switches to the cooked output when it is there and current; else it shows what it shows (the report has the job)
    ReopenShown( path );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ConvertForOgre( void )
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;
    const CORE::CString target = m_hasPresentation ? m_packRoot : m_shownPath;
    if ( target.IsNULLOrEmpty() )
    {
        m_report->AddEntry( GUI::CReportViewForm::SEVERITY_WARNING, CORE::CString::Empty, "Convert for Ogre: nothing is shown", "Open a model, scene or pack first" );
        return false;
    }
    CORE::CString root, packFolder, relative;
    if ( CCookedOutputResolver::SplitStorePath( target, root, packFolder, relative ) )
    {
        // a pack of the primary store: a convert_pack job of the service (the bulk conversion), scope the shown asset or the pack
        ASSETSTORE::CIAssetStoreService* service = GUCEF_NULL != m_sources ? m_sources->GetService() : GUCEF_NULL;
        CORE::CString packId = GUCEF_NULL != m_sources ? m_sources->GetSelectedPackId() : CORE::CString::Empty;
        ASSETSTORE::CAssetPackRecord pack;
        if ( GUCEF_NULL == service || GUCEF_NULL == m_bulkController || packId.IsNULLOrEmpty() ||
             ASSETSTORE::ASSETSTORE_STATUS_OK != service->GetPack( packId, pack ) || ASSETSTORE::CPackStorageLayout( pack.rootPath ).GetPackFolder() != packFolder )
        {
            m_report->AddEntry( GUI::CReportViewForm::SEVERITY_WARNING, target, "Convert for Ogre: the pack is not known", "Select the pack in the Sources tab: the conversion is a job of the asset store service" );
            return false;
        }
        CBulkConversionSettings settings;
        settings.packId = packId;
        settings.scopeKind = relative.IsNULLOrEmpty() ? CBulkConversionSettings::ScopePack : CBulkConversionSettings::ScopeSelection;
        if ( !relative.IsNULLOrEmpty() )
            settings.assets.push_back( relative );
        settings.intermediate = true;
        settings.ogreRuntime = true;
        settings.cookProfile = CCookedOutputResolver::SOptions().profile;
        if ( !m_bulkController->RequestPlan( settings ) || !m_bulkController->CanStart() || 0 == m_bulkController->Start() )
        {
            m_report->AddEntry( GUI::CReportViewForm::SEVERITY_WARNING, target, "Convert for Ogre was not started", m_bulkController->GetProblem() );
            return false;
        }
        m_cookPending = true;
        m_cookPendingPath = target;
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: Convert for Ogre: the job of the pack " << packId << " is started, the view switches to the cooked output when it is done" );
        return true;
    }
    // a pack of a local source with an importer: converted into the cache folder, shown when it is done
    if ( m_hasPresentation && !m_presentation.converter.IsNULLOrEmpty() )
    {
        m_cookForPresentation = true;
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: Convert for Ogre: converting " << target << " with the " << m_presentation.converter << " converter" );
        const CORE::CString kind = CPackPresentation::GetKindId( m_presentation.kind );
        return StartPackPresentation( target, kind, false, CPackPresentation::KIND_SUMMARY, CORE::CString::Empty );
    }
    m_report->AddEntry( GUI::CReportViewForm::SEVERITY_WARNING, target, "Convert for Ogre is not possible here", "The asset is not in a pack of the primary store and no converter plugin reads its pack" );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::FinishPackPresentation( const CPackPresentation& presentation )
{GUCEF_TRACE;

    m_presentation = presentation;
    m_hasPresentation = true;
    m_shownPath = m_packRoot;
    if ( ASSETKIND_OTHER == m_shownKind )
        m_shownKind = ASSETKIND_FOLDER;
    m_anythingOpened = true;
    SetModelToggles( m_toggleForMaterial );
    RefreshInspector();
    CORE::CString detail = presentation.why;
    if ( presentation.moreAssets > 0 )
        detail += ". " + CORE::UInt32ToString( presentation.moreAssets ) + " more can be loaded";
    detail += ". Source: " + presentation.GetSourceDescription();
    m_report->AddEntry( GUI::CReportViewForm::SEVERITY_INFO, m_packRoot, "Presentation: " + CPackPresentation::GetKindName( presentation.kind ), detail );
    if ( !presentation.fallbackReason.IsNULLOrEmpty() && presentation.kind != presentation.wantedKind )
        m_report->AddEntry( GUI::CReportViewForm::SEVERITY_WARNING, m_packRoot, CPackPresentation::GetKindName( presentation.wantedKind ) + " is not shown", presentation.fallbackReason );
    UpdateEmptyState();
    m_stage.MarkDirty();
    // a picture worth keeping is taken when the presentation has settled
    const bool is3D = CPackPresentation::KIND_PORTRAIT == presentation.kind || CPackPresentation::KIND_EFFECT_FOUR_VIEW == presentation.kind ||
                      CPackPresentation::KIND_SCENE_DEMO == presentation.kind || CPackPresentation::KIND_MODEL_GRID == presentation.kind ||
                      CPackPresentation::KIND_ANIMATION_PLAYER == presentation.kind;
    m_autoCapturePending = is3D && m_autoThumbnails.IsEnabled();
    m_autoCaptureKey.Clear();
    m_autoCaptureCountdownInMs = 0;
    m_autoCaptureFrames = 0;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::AppendPresentationInspector( CORE::CDataNode& data ) const
{GUCEF_TRACE;

    if ( !m_hasPresentation )
        return;
    CORE::CDataNode* node = data.AddChild( "Presentation" );
    node->SetAttribute( "Kind", CPackPresentation::GetKindName( m_presentation.kind ) );
    node->SetAttribute( "Why", m_presentation.why );
    node->SetAttribute( "Needs", m_presentation.requirement );
    node->SetAttribute( "Chosen by", m_presentation.userChoice ? "you" : "default of the pack type" );
    node->SetAttribute( "Source", m_presentation.GetSourceDescription() );
    if ( !m_presentation.fallbackReason.IsNULLOrEmpty() )
        node->SetAttribute( "Note", m_presentation.fallbackReason );
    node->SetAttribute( "Assets", (UInt32) m_presentation.assets.size() );
    if ( m_presentation.moreAssets > 0 )
        node->SetAttribute( "More to load", m_presentation.moreAssets );
    if ( m_showcase.IsLoaded() )
        m_showcase.WriteInspectorData( data );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::AddPresentationToggle( GUI::CViewportForm::TToggleVector& toggles )
{GUCEF_TRACE;

    if ( !m_hasPresentation || m_offered.size() < 2 )
        return;
    GUI::CViewportForm::TToggle present( TogglePresentation, "Present", "What this pack shows: the default of its type, or another presentation; the first choice puts the default back (the choice is remembered per pack)" );
    present.choices.push_back( "Default" );
    Int32 choice = 0;
    for ( size_t i=0; i<m_offered.size(); ++i )
    {
        present.choices.push_back( CPackPresentation::GetKindName( m_offered[ i ] ) );
        if ( m_presentation.userChoice && m_offered[ i ] == m_presentation.kind )
            choice = (Int32) i + 1;
    }
    present.choice = choice;
    toggles.push_back( present );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::UpdateAutoThumbnailCapture( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( !m_autoCapturePending )
        return;
    if ( m_loader.IsBusy() || ( GUCEF_NULL != m_activityIndicator && m_activityIndicator->IsIndicatorShown() ) )
    {
        m_autoCaptureCountdownInMs = 0;
        m_autoCaptureFrames = 0;
        return;
    }
    m_autoCaptureCountdownInMs += deltaInMs;
    ++m_autoCaptureFrames;
    if ( m_autoCaptureFrames < 6 || m_autoCaptureCountdownInMs < 60 )
        return;
    m_autoCapturePending = false;
    CaptureAutoThumbnail();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::CaptureAutoThumbnail( void )
{GUCEF_TRACE;

    m_autoCapturePending = false;
    if ( !m_autoThumbnails.IsEnabled() || !m_stage.IsInitialized() )
        return false;
    const bool forPack = m_hasPresentation && m_autoCaptureKey.IsNULLOrEmpty();
    const CORE::CString hash = forPack ? CAutoThumbnailStore::HashPack( m_packRoot ) : CThumbnailProvider::HashContent( m_autoCaptureKey );
    if ( hash.IsNULLOrEmpty() )
        return false;
    const UInt32 thumbnailJob = m_jobRegistry.Begin( "thumbnail", CORE::CString::Empty, "Thumbnail of " + ( forPack ? m_packRoot : m_autoCaptureKey ), forPack ? m_packRoot : m_autoCaptureKey, false );
    IMAGE::TPixelMapPtr pixels;
    if ( !m_stage.ReadPixels( pixels ) )
    {
        m_jobRegistry.End( thumbnailJob, ASSETSTORE::ASSETJOB_STATE_FAILED, "The render cannot be read" );
        return false;
    }
    const bool loadFailed = CAssetLoader::OUTCOME_FAILED == m_loader.GetLastOutcome() || CAssetLoader::OUTCOME_CANCELLED == m_loader.GetLastOutcome();
    // what the views show of a file of a remote store is the picture of it on the service as well (once per run), so that the other devices have it
    if ( !forPack && !loadFailed && CAutoThumbnailStore::IsWorthwhile( pixels ) && GUCEF_NULL != m_sources && GUCEF_NULL != m_sources->GetRemoteThumbnails() && m_sources->GetRemoteThumbnails()->IsRemotePath( m_autoCaptureKey ) )
    {
        IMAGE::TPixelMapPtr fitted;
        if ( CAutoThumbnailStore::FitToSquare( pixels, CAutoThumbnailStore::DefaultSize, fitted ) )
            m_sources->GetRemoteThumbnails()->UploadRender( m_autoCaptureKey, fitted );
    }
    CORE::CString reason;
    const bool stored = m_autoThumbnails.Store( hash, pixels, CAssetLoader::OUTCOME_FAILED == m_loader.GetLastOutcome() || CAssetLoader::OUTCOME_CANCELLED == m_loader.GetLastOutcome(), reason );
    m_jobRegistry.End( thumbnailJob, stored ? ASSETSTORE::ASSETJOB_STATE_DONE : ASSETSTORE::ASSETJOB_STATE_FAILED, stored ? CORE::CString( "kept" ) : reason );
    if ( stored )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: Kept the render of " << ( forPack ? m_packRoot : m_autoCaptureKey ) << " as its automatic thumbnail" );
    }
    else
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: No automatic thumbnail of " << ( forPack ? m_packRoot : m_autoCaptureKey ) << ": " << reason << " (the last request ended as " << (Int32) m_loader.GetLastOutcome() << ": " << m_loader.GetLastProblem() << ")" );
    }
    return stored;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchShell::ClearAutoThumbnails( void )
{GUCEF_TRACE;

    const UInt32 cleared = m_autoThumbnails.Clear();
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: " << cleared << " automatic thumbnails were cleared" );
    if ( GUCEF_NULL != m_browser )
        m_browser->Refresh();
    return cleared;
}

/*-------------------------------------------------------------------------*/

GUI::CContentBrowserForm* CWorkbenchShell::GetBrowser( void ) const { return m_browser; }
GUI::CContentBrowserForm* CWorkbenchShell::GetSceneBrowser( void ) const { return m_sceneBrowser; }
GUI::CViewportForm* CWorkbenchShell::GetViewport( void ) const { return m_viewport; }
GUI::CTimelineForm* CWorkbenchShell::GetTimeline( void ) const { return m_timeline; }
GUI::CPropertyInspectorForm* CWorkbenchShell::GetInspector( void ) const { return m_inspector; }
GUI::CImageViewForm* CWorkbenchShell::GetImageView( void ) const { return m_imageView; }
GUI::CImageViewForm* CWorkbenchShell::GetUvView( void ) const { return m_uvView; }
GUI::CReportViewForm* CWorkbenchShell::GetReport( void ) const { return m_report; }
GUI::CTransferQueueForm* CWorkbenchShell::GetTransfers( void ) const { return m_transfers; }
GUI::COpenActionStripForm* CWorkbenchShell::GetOpenStrip( void ) const { return m_openStrip; }
GUI::CFileSystemDialogForm* CWorkbenchShell::GetFolderDialog( void ) const { return m_folderDialog; }
GUI::CFileSystemDialogForm* CWorkbenchShell::GetArchiveDialog( void ) const { return m_archiveDialog; }
GUI::CFileSystemDialogForm* CWorkbenchShell::GetFileDialog( void ) const { return m_fileDialog; }
CAssetSourcesPane* CWorkbenchShell::GetSources( void ) const { return m_sources; }
CWorkbenchSignIn* CWorkbenchShell::GetSignIn( void ) const { return m_signIn; }
CThumbnailProvider* CWorkbenchShell::GetThumbnailProvider( void ) const { return m_thumbnails; }
CWorkbenchStage& CWorkbenchShell::GetStage( void ) { return m_stage; }
COrbitController& CWorkbenchShell::GetController( void ) { return m_controller; }
CModelView& CWorkbenchShell::GetModelView( void ) { return m_model; }
CMaterialView& CWorkbenchShell::GetMaterialView( void ) { return m_material; }
CTextureView& CWorkbenchShell::GetTextureView( void ) { return m_texture; }
CSceneView& CWorkbenchShell::GetSceneView( void ) { return m_scene; }
CParticlePreview& CWorkbenchShell::GetParticlePreview( void ) { return m_particles; }
bool CWorkbenchShell::IsStageBound( void ) const { return m_stageBound; }
GUI::CActivityService& CWorkbenchShell::GetActivityService( void ) { return m_activity; }
GUI::CActivityIndicatorForm* CWorkbenchShell::GetActivityIndicator( void ) const { return m_activityIndicator; }
CAssetLoader& CWorkbenchShell::GetLoader( void ) { return m_loader; }
bool CWorkbenchShell::IsOpening( void ) const { return m_loader.IsBusy(); }
bool CWorkbenchShell::IsAsyncOpenEnabled( void ) const { return m_asyncOpen; }
void CWorkbenchShell::SetAsyncOpenEnabled( bool enabled ) { m_asyncOpen = enabled; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE RESOURCE VIEW                                                  //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CWorkbenchShell::IsOpenPacksInResourceView( void ) const
{GUCEF_TRACE;

    return m_resourceViewOverride >= 0 ? 0 != m_resourceViewOverride : m_preferences->openPacksInResourceView;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::SetOpenPacksInResourceView( bool enabled )
{GUCEF_TRACE;

    m_preferences->openPacksInResourceView = enabled;
    // a switch of the user in this run is the last word
    m_resourceViewOverride = -1;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: A pack that is opened starts in the " << ( enabled ? "resource view" : "files view" ) );
    RaisePreferencesChanged();
}

/*-------------------------------------------------------------------------*/

void CWorkbenchShell::SetResourceViewOverride( Int32 value ) { m_resourceViewOverride = value; }
void CWorkbenchShell::SetResourceViewStartPath( const CORE::CString& path ) { m_resourceStartPath = path; }
Int32 CWorkbenchShell::GetResourceViewOverride( void ) const { return m_resourceViewOverride; }
CResourceViewFileSystemAccess& CWorkbenchShell::GetResourceView( void ) { return *m_resourceView; }
const CORE::CString& CWorkbenchShell::GetResourceViewPack( void ) const { return m_resourceViewPack; }
bool CWorkbenchShell::IsIndexing( void ) const { return m_indexLoader.IsBusy(); }
CAssetLoader& CWorkbenchShell::GetIndexLoader( void ) { return m_indexLoader; }
const ASSETCONVERTER::CPackIndexRunResult& CWorkbenchShell::GetLastIndexResult( void ) const { return m_lastIndexResult; }

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::IsResourceViewShown( void ) const
{GUCEF_TRACE;

    return m_initialized && GUCEF_NULL != m_browser && m_browser->IsAlternateViewActive();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ShowResourceView( bool shown )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;
    m_browser->SetAlternateViewActive( shown );
    m_preferences->resourceViewShown = shown;
    RaisePreferencesChanged();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::StartPackIndex( const CORE::CString& vfsFolder, bool redo )
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;
    const CORE::CString root = CAssetMounter::NormalizePath( vfsFolder );
    if ( root.IsNULLOrEmpty() )
        return false;
    ASSETCONVERTER::CPackRedoRequest request;
    request.reindex = redo;
    TAssetLoadJobPtr job( GUCEF_NEW CPackIndexJob( *this, root, request ) );
    CAssetMounter::SMount mount;
    CORE::CString relative;
    const CORE::CString label = "Indexing " + ( m_mounter.FindMount( root, mount, relative ) ? mount.mountPoint : root );
    m_indexingFolder = root;
    m_indexAutoSwitch = !redo;
    m_indexLoader.DescribeNext( "index", CORE::CString::Empty, root );
    return !m_indexLoader.Begin( job, label ).HasNoFuture();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::ReIndexPack( void )
{GUCEF_TRACE;

    if ( m_resourceViewPack.IsNULLOrEmpty() )
        return false;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: The index of " << m_resourceViewPack << " is built again" );
    return StartPackIndex( m_resourceViewPack, true );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ApplyPackIndex( const CORE::CString& vfsFolder, ASSETCONVERTER::CPackIndexDocument& document, const ASSETCONVERTER::CPackIndexRunResult& result )
{GUCEF_TRACE;

    m_lastIndexResult = result;
    const bool wasShown = IsResourceViewShown();
    m_resourceView->AdoptPack( document, result.indexFolder, vfsFolder );
    m_resourceView->SetSearch( m_browser->GetFilter() );
    m_resourceViewPack = vfsFolder;
    m_hasSelectedResource = false;
    m_report->AddEntry( GUI::CReportViewForm::SEVERITY_INFO, vfsFolder, "Pack index", result.ToSummaryText() );
    if ( m_indexAutoSwitch && IsOpenPacksInResourceView() && !wasShown )
        m_browser->SetAlternateViewActive( true );
    else
    if ( wasShown )
        m_browser->NavigateTo( ResourcesRootName, CORE::CString::Empty );
    if ( !m_resourceStartPath.IsNULLOrEmpty() && m_browser->IsAlternateViewActive() )
    {
        m_browser->NavigateTo( ResourcesRootName, m_resourceStartPath );
        m_resourceStartPath.Clear();
    }
    m_indexAutoSwitch = false;
    if ( ResourcesRootName == m_browser->GetCurrentRootName() )
        m_browser->Refresh();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchShell::GetResourceOfPath( const CORE::CString& browserPath, ASSETCONVERTER::CPackIndexResource& resource ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_resourceView && m_resourceView->TryResolve( browserPath, resource );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchShell::ResolveBrowserPath( const CORE::CString& path ) const
{GUCEF_TRACE;

    ASSETCONVERTER::CPackIndexResource resource;
    if ( ResourcesRootName == m_browser->GetCurrentRootName() && GetResourceOfPath( path, resource ) )
        return CAssetMounter::NormalizePath( m_resourceView->GetVfsPathOf( resource ) );
    return path;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::ShowResourceFolderSummary( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString text;
    UInt32 count = 0;
    UInt64 bytes = 0;
    if ( !m_resourceView->GetFolderSummary( path, text, count, bytes ) )
        return;
    CORE::CDataNode data;
    data.SetName( "Resources" );
    data.SetAttribute( "Group", text );
    data.SetAttribute( "Resources", CORE::UInt32ToString( count ) );
    data.SetAttribute( "Size", CAssetPackSummary::FormatSize( bytes ) );
    const ASSETCONVERTER::CPackIndexDocument& manifest = m_resourceView->GetManifest();
    data.SetAttribute( "Pack", manifest.identity.name );
    if ( !manifest.profile.packTypeName.IsNULLOrEmpty() )
        data.SetAttribute( "Type", manifest.profile.packTypeName );
    SetInspector( data, GUI::CPropertyInspectorForm::TDescriptorMap() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::AppendResourceInspector( CORE::CDataNode& data ) const
{GUCEF_TRACE;

    if ( !m_hasSelectedResource || m_selectedResourcePath != m_shownPath )
        return;
    CORE::CDataNode* node = data.AddChild( "Resource" );
    node->SetAttribute( "Group", m_resourceView->GetGroupTextOf( m_selectedResource ) );
    node->SetAttribute( "Pack path", m_selectedResource.path );
    node->SetAttribute( "Kind", m_selectedResource.kind );
    if ( !m_selectedResource.format.IsNULLOrEmpty() )
        node->SetAttribute( "Format", m_selectedResource.format );
    node->SetAttribute( "File size", CAssetPackSummary::FormatSize( m_selectedResource.size ) );
    UInt64 width = 0, height = 0, triangles = 0;
    if ( m_selectedResource.GetStat( "width", width ) && m_selectedResource.GetStat( "height", height ) )
        node->SetAttribute( "Pixels", CORE::UInt64ToString( width ) + " x " + CORE::UInt64ToString( height ) );
    if ( m_selectedResource.GetStat( "triangles", triangles ) )
        node->SetAttribute( "Triangles", CORE::UInt64ToString( triangles ) );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnBrowserFolderActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the double click on a group of the resource view opens the presentation of that group: effects play in four views, models stand in a grid, ...
    const CORE::TCloneableString* path = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == path || ResourcesRootName != m_browser->GetCurrentRootName() || m_resourceViewPack.IsNULLOrEmpty() )
        return;
    CORE::CString groupId;
    bool isGroupFolder = false;
    if ( !CResourceViewFileSystemAccess::GetGroupOfPath( path->GetData(), groupId, isGroupFolder ) || !isGroupFolder )
        return;
    static const char* const presentations[][ 2 ] =
    {
        { "scenes", "scene" }, { "characters", "portrait" }, { "static-models", "grid" }, { "animations", "animation" }, { "visual-effects", "effect" },
        { "materials", "material" }, { "textures", "images" }, { "audio", "audio" }
    };
    CORE::CString kindId = "summary";
    for ( size_t i=0; i<sizeof( presentations ) / sizeof( presentations[ 0 ] ); ++i )
        if ( groupId == presentations[ i ][ 0 ] )
            kindId = presentations[ i ][ 1 ];
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchShell: The group " << groupId << " of " << m_resourceViewPack << " opens its presentation: " << kindId );
    ShowPackPresentation( m_resourceViewPack, kindId, false );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnBrowserFilterChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // in the resource view the filter line searches the whole pack, not only the folder that is open: the root lists what matches in every group
    if ( !m_initialized || GUCEF_NULL == m_resourceView || m_resourceView->GetSearch() == m_browser->GetFilter() )
        return;
    m_resourceView->SetSearch( m_browser->GetFilter() );
    if ( ResourcesRootName == m_browser->GetCurrentRootName() )
        m_browser->Refresh();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchShell::OnBrowserViewSwitched( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the choice of the person with the switch of the tool bar is kept
    m_preferences->resourceViewShown = m_browser->IsAlternateViewActive();
    RaisePreferencesChanged();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
