/*
 *  AssetWorkbenchLib: the settings layer of the workbench: the settings window with its pages, the developer console, the help window and the tooltips
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSETTINGSLAYER_H
#define ASSETWORKBENCHLIB_CWORKBENCHSETTINGSLAYER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CSETTINGSFORM_H
#include "gucefGUI_CSettingsForm.h"
#define GUCEF_GUI_CSETTINGSFORM_H
#endif /* GUCEF_GUI_CSETTINGSFORM_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGSFORM_H
#include "gucefGUI_CVideoSettingsForm.h"
#define GUCEF_GUI_CVIDEOSETTINGSFORM_H
#endif /* GUCEF_GUI_CVIDEOSETTINGSFORM_H ? */

#ifndef GUCEF_GUI_CGENERALSETTINGSFORM_H
#include "gucefGUI_CGeneralSettingsForm.h"
#define GUCEF_GUI_CGENERALSETTINGSFORM_H
#endif /* GUCEF_GUI_CGENERALSETTINGSFORM_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHGENERALSETTINGSFORM_H
#include "AssetWorkbenchLib_CWorkbenchGeneralSettingsForm.h"
#define ASSETWORKBENCHLIB_CWORKBENCHGENERALSETTINGSFORM_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHGENERALSETTINGSFORM_H ? */

#ifndef GUCEF_GUI_CAUDIOSETTINGSFORM_H
#include "gucefGUI_CAudioSettingsForm.h"
#define GUCEF_GUI_CAUDIOSETTINGSFORM_H
#endif /* GUCEF_GUI_CAUDIOSETTINGSFORM_H ? */

#ifndef GUCEF_GUI_CINPUTSETTINGSFORM_H
#include "gucefGUI_CInputSettingsForm.h"
#define GUCEF_GUI_CINPUTSETTINGSFORM_H
#endif /* GUCEF_GUI_CINPUTSETTINGSFORM_H ? */

#ifndef GUCEF_GUI_CHELPVIEWERFORM_H
#include "gucefGUI_CHelpViewerForm.h"
#define GUCEF_GUI_CHELPVIEWERFORM_H
#endif /* GUCEF_GUI_CHELPVIEWERFORM_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADER_H
#include "gucefASSETSTORE_CAssetStorePluginLoader.h"
#define GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADER_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSHELL_H
#include "AssetWorkbenchLib_CWorkbenchShell.h"
#define ASSETWORKBENCHLIB_CWORKBENCHSHELL_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHSHELL_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H
#include "AssetWorkbenchLib_CWorkbenchPreferences.h"
#define ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHHELP_H
#include "AssetWorkbenchLib_CWorkbenchHelp.h"
#define ASSETWORKBENCHLIB_CWORKBENCHHELP_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHHELP_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHTOOLTIPS_H
#include "AssetWorkbenchLib_CWorkbenchTooltips.h"
#define ASSETWORKBENCHLIB_CWORKBENCHTOOLTIPS_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHTOOLTIPS_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHCONSOLECOMMANDS_H
#include "AssetWorkbenchLib_CWorkbenchConsoleCommands.h"
#define ASSETWORKBENCHLIB_CWORKBENCHCONSOLECOMMANDS_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHCONSOLECOMMANDS_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHAUTHSETTINGSFORM_H
#include "AssetWorkbenchLib_CWorkbenchAuthSettingsForm.h"
#define ASSETWORKBENCHLIB_CWORKBENCHAUTHSETTINGSFORM_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHAUTHSETTINGSFORM_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSOURCESSETTINGSFORM_H
#include "AssetWorkbenchLib_CWorkbenchSourcesSettingsForm.h"
#define ASSETWORKBENCHLIB_CWORKBENCHSOURCESSETTINGSFORM_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHSOURCESSETTINGSFORM_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHREMOTESTOREFORM_H
#include "AssetWorkbenchLib_CWorkbenchRemoteStoreForm.h"
#define ASSETWORKBENCHLIB_CWORKBENCHREMOTESTOREFORM_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHREMOTESTOREFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What is not a view of an asset, over the views: the settings window, the developer console, the help window and the tooltips.
 *  The layer is made after the shell and goes before it. It works in the GUI context of the shell (the context already has the theme and
 *  the resolution of the window) and follows the structure of the settings layers of the other applications:
 *
 *    - The settings window (CSettingsForm) with the pages General, Video and Audio of gucefGUI, Input (the keys of the shell as the
 *      shortcut map has them, CInputSettingsForm), Authentication and Sources. The layer holds the settings in effect (SetVideoSettings(),
 *      SetGeneralSettings(), SetAudioSettings()); after Apply or OK it reads them back from the pages and tells the application with
 *      SettingsAppliedEvent, which keeps them in the settings of the user.
 *    - The developer console, made over the window: it opens on its key (the key left of 1) and from the menu, not while a text field has
 *      the focus, and runs the commands of CWorkbenchConsoleCommands.
 *    - The help: the catalogue of CWorkbenchHelp as the help source of the tooltip service of the context and of the help window; F1 opens
 *      the help window at the document of the place the focus is in, and in the settings window at the settings.
 *    - The tooltips of CWorkbenchTooltips for every control.
 *    - The folder of private store plugins of the preferences: LoadPrivatePlugins() loads the asset store plugins of it, at the start of the
 *      application and when the person presses Load now.
 *
 *  The actions of the keys and of the menu reach the layer as ActionRequestedEvent of the shell: Settings, Help, DeveloperConsole.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchSettingsLayer : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent SettingsAppliedEvent;     /**< Apply or OK of the settings window: the application keeps the settings */
    static void RegisterEvents( void );

    /** The shell and the preferences the layer works with; neither is owned and both outlive the layer */
    CWorkbenchSettingsLayer( CWorkbenchShell& shell, CWorkbenchPreferences& preferences );

    virtual ~CWorkbenchSettingsLayer() GUCEF_VIRTUAL_OVERRIDE;

    /** Makes the settings window with its pages, the console, the help and the tooltips; false when the shell is not initialized or a form cannot be made */
    bool Create( void );

    bool IsCreated( void ) const;

    /*
     *  The settings in effect
     */
    void SetVideoSettings( const GUI::CVideoSettings& settings );
    const GUI::CVideoSettings& GetVideoSettings( void ) const;
    void SetGeneralSettings( const GUI::CGeneralSettings& settings );
    const GUI::CGeneralSettings& GetGeneralSettings( void ) const;
    void SetAudioSettings( const AUDIO::CAudioSettings& settings );
    const AUDIO::CAudioSettings& GetAudioSettings( void ) const;

    /** The renderers the window manager backend offers and the one in use, and the window the video page applies to */
    void SetRenderers( const CORE::CString::StringVector& rendererNames, const CORE::CString& rendererInUse );
    void SetWindow( const CORE::CString& windowManagerBackendName, GUI::TWindowContextPtr window );

    /** The general settings as at the start: the console log window and the developer console with its key */
    void ApplyGeneralSettings( void );

    /*
     *  The settings window
     */

    /** Shows the settings window with the page of that title in front (the one that was in front when there is none such, the first at the first time) */
    bool Open( const CORE::CString& pageTitle = CORE::CString::Empty );
    bool IsOpen( void ) const;
    GUI::CSettingsForm* GetForm( void );
    GUI::CVideoSettingsForm* GetVideoPage( void );
    CWorkbenchGeneralSettingsForm* GetGeneralPage( void );
    GUI::CAudioSettingsForm* GetAudioPage( void );
    GUI::CInputSettingsForm* GetInputPage( void );
    CWorkbenchAuthSettingsForm* GetAuthPage( void );
    CWorkbenchSourcesSettingsForm* GetSourcesPage( void );
    CWorkbenchRemoteStoreForm* GetRemotePage( void );

    /*
     *  The developer console
     */
    GUI::CDeveloperConsoleForm* GetConsole( void );
    CWorkbenchConsoleCommands* GetCommands( void );
    bool ToggleConsole( void );

    /*
     *  Help and tooltips
     */
    CWorkbenchHelp& GetHelp( void );
    CWorkbenchTooltips& GetTooltips( void );

    /** Opens the help window at the document; the overview for a document that is not there */
    bool OpenHelp( const CORE::CString& topicId );

    /** F1: the help window at the document of the place the focus is in; in the settings window the settings */
    bool OpenHelpForFocus( void );

    /** The help window, NULL before it was opened the first time */
    GUI::CHelpViewerForm* GetHelpViewer( void ) const;

    /** The document F1 would open now */
    CORE::CString GetHelpTopicForFocus( void ) const;

    /*
     *  The private store plugins
     */

    /** Loads the asset store plugins of the folder of the preferences; false when no folder is set, it is not there or a plugin was refused */
    bool LoadPrivatePlugins( void );

    /** The same for another folder, for the folder that is typed in the settings window and not applied yet */
    bool LoadPrivatePlugins( const CORE::CString& directory );

    /** What the last load did */
    const ASSETSTORE::CAssetStorePluginLoadReport& GetPluginReport( void ) const;

    /** The host that inspects and loads the plugin libraries; the system host by default (not owned, for tests). NULL gives the system host back */
    void SetPluginHost( ASSETSTORE::CIAssetStorePluginHost* host );

    /**
     *  After the sources are made (or made again): the pages get the sign in, and the stores that the preferences switch off are taken out
     *  of the sources tab
     */
    void BindSources( void );

    /** The text of the keyboard shortcuts document and of the settings: the keys of the shell now */
    void UpdateShortcutHelp( void );

    private:

    typedef CORE::CTEventHandlerFunctor< CWorkbenchSettingsLayer > TEventCallback;

    void OnShellAction( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFormSettingsApplied( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnGeneralApplied( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnClearAutomaticThumbnails( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPluginsLoadRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFormKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    bool PrepareOpen( void );
    void SyncConsoleKeyToShortcuts( void );
    void SyncShortcutsToConsoleKey( void );
    CORE::CString GetServiceNote( void ) const;

    CWorkbenchSettingsLayer( const CWorkbenchSettingsLayer& src );              /**< not implemented */
    CWorkbenchSettingsLayer& operator=( const CWorkbenchSettingsLayer& src );   /**< not implemented */

    private:

    CWorkbenchShell& m_shell;
    CWorkbenchPreferences& m_preferences;
    GUI::CGUIContext* m_context;
    GUI::CSettingsForm* m_form;
    GUI::CVideoSettingsForm* m_videoPage;
    CWorkbenchGeneralSettingsForm* m_generalPage;
    GUI::CAudioSettingsForm* m_audioPage;
    GUI::CInputSettingsForm* m_inputPage;
    CWorkbenchAuthSettingsForm* m_authPage;
    CWorkbenchSourcesSettingsForm* m_sourcesPage;
    CWorkbenchRemoteStoreForm* m_remotePage;
    GUI::CDeveloperConsoleForm* m_console;
    CWorkbenchConsoleCommands* m_commands;
    GUI::CHelpViewerForm* m_helpViewer;
    CWorkbenchHelp m_help;
    CWorkbenchTooltips m_tooltips;
    GUI::CVideoSettings m_video;
    GUI::CGeneralSettings m_general;
    AUDIO::CAudioSettings m_audio;
    ASSETSTORE::CAssetStorePluginLoadReport m_pluginReport;
    ASSETSTORE::CIAssetStorePluginHost* m_pluginHost;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHSETTINGSLAYER_H ? */
