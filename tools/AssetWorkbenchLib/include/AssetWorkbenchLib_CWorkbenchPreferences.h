/*
 *  AssetWorkbenchLib: the preferences of the owner that the settings window keeps: the folder of private store plugins, the stores that are switched off and the key bindings
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H
#define ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H
#include "gucefGUI_CKeyboardShortcutMap.h"
#define GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H
#endif /* GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H ? */

#ifndef ASSETWORKBENCHLIB_CBULKCONVERSIONSETTINGS_H
#include "AssetWorkbenchLib_CBulkConversionSettings.h"
#define ASSETWORKBENCHLIB_CBULKCONVERSIONSETTINGS_H
#endif /* ASSETWORKBENCHLIB_CBULKCONVERSIONSETTINGS_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

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
 *  What the owner sets in the settings window of the workbench that no shared settings class of gucefGUI keeps, as attributes and
 *  children of the section "AssetWorkbench" of the settings of the user (next to the layout and the recent places):
 *
 *    - "PrivatePluginsDir": the folder with the asset store plugins that are not part of the build (empty by default; it is the owner's
 *      to set, never part of a source repository), see ASSETSTORE::CAssetStorePluginLoader;
 *    - "DisabledStores": the ids of the online stores that are switched off (a store that is not listed is on);
 *    - "SettingsVersion": which version of the settings the file was written by, so that a new setting can start from its own default;
 *    - "Shortcuts": the key chords that the owner changed, one entry "action=chord" per action ("fold.left=F8"); an action that is not
 *      listed has the chord the workbench ships.
 *    - "RemoteEndpoint", "RemotePairingPort", "RemoteLanDiscovery", "RemoteConnectAtStart": how the workbench reaches a remote asset store service
 *      (CWorkbenchStoreConnection): the address worker.lan:47400, the pairing port, the look on the local network and the connect at the start.
 *
 *  No credential, token or cookie is ever kept here: those live in the credential stores of the sign in (CWorkbenchCredentials).
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchPreferences
{
    public:

    static const CORE::CString PrivatePluginsDirName;   /**< "PrivatePluginsDir" */
    static const CORE::CString DisabledStoresName;      /**< "DisabledStores" */
    static const CORE::CString ShortcutsName;           /**< "Shortcuts" */
    static const CORE::CString SettingsVersionName;     /**< "SettingsVersion" */
    static const CORE::CString OpenPackPresentationName;    /**< "OpenPackPresentation" */
    static const CORE::CString AutomaticThumbnailsName;     /**< "AutomaticThumbnails" */
    static const CORE::CString OpenPacksInResourceViewName; /**< "OpenPacksInResourceView" */
    static const CORE::CString PreferCookedOutputName;      /**< "PreferCookedOutput" */
    static const CORE::CString ResourceViewShownName;       /**< "ResourceViewShown" */
    static const CORE::CString PackPresentationsName;       /**< "PackPresentations" */
    static const CORE::CString BulkTemplatesName;           /**< "BulkTemplates" */
    static const CORE::CString MaxSceneSecondsName;         /**< "MaxSceneSeconds" */
    static const CORE::CString RemoteEndpointName;          /**< "RemoteEndpoint" */
    static const CORE::CString RemotePairingPortName;       /**< "RemotePairingPort" */
    static const CORE::CString RemoteLanDiscoveryName;      /**< "RemoteLanDiscovery" */
    static const CORE::CString RemoteConnectAtStartName;    /**< "RemoteConnectAtStart" */
    static const CORE::CString DefaultRemoteEndpoint;       /**< "worker.lan:47400": the session port of the asset store service on the main node of the cluster */
    static const UInt32 DefaultRemotePairingPort = 47401;
    static const UInt32 CurrentSettingsVersion = 2;     /**< 1: the developer console exists and is on until the owner switches it off; 2: the scene of a pack converts for up to 600 seconds, whatever size it has */
    static const UInt32 DefaultMaxSceneSeconds = 600;   /**< the default of maxSceneSeconds */

    static const UInt32 MaxPackPresentations = 200;    /**< the choices kept per pack */
    static const UInt32 MaxBulkTemplates = 50;          /**< the named bulk conversion settings kept */
    UInt32 settingsVersion;                             /**< the version of the settings that were read; 0 for settings of before the versions */
    CORE::CString privatePluginsDir;
    CORE::CString::StringVector disabledStores;
    CORE::CString::StringVector shortcuts;              /**< "action=chord" */
    bool openPackPresentation;                          /**< a pack that is opened shows its presentation without more clicks (on by default) */
    UInt32 maxSceneSeconds;                             /**< how long a presentation waits for the conversion of the scene of a pack, whatever the size of the scene, before it cancels the conversion and shows the generated showcase instead: 600; the result of a conversion is kept, a next open of the pack is a cache hit */
    bool automaticThumbnails;                           /**< a worthwhile render of a pack or model is kept as its automatic thumbnail (on by default) */
    bool preferCookedOutput;                            /**< a model, scene or pack that has current cooked Ogre output (gucef-ogre3d) shows that instead of the quick look through the Assimp codec (on by default) */
    bool openPacksInResourceView;                       /**< a pack that is opened starts in the resource view of the content browser: its resources by function, not its files (on by default) */
    bool resourceViewShown;                             /**< the switch Files or Resources of the content browser as the person left it */
    CORE::CString remoteEndpoint;                       /**< "host:port" of the session listener of the remote asset store service (never a secret); worker.lan:47400 by default */
    UInt32 remotePairingPort;                           /**< the port of the pairing listener of that service: 47401 */
    bool remoteLanDiscovery;                            /**< look for asset store services on the local network (on by default; a configured endpoint always wins) */
    bool remoteConnectAtStart;                          /**< connect to the remote store when the workbench starts, when this computer is paired (off by default: the store of this process is the default) */
    CORE::CString::StringVector packPresentations;      /**< "packKey=presentationId": the last choice per pack */
    TBulkConversionSettingsVector bulkTemplates;        /**< the named settings of the bulk conversion panel (job templates) */

    CWorkbenchPreferences( void );

    bool IsStoreEnabled( const CORE::CString& storeId ) const;

    void SetStoreEnabled( const CORE::CString& storeId, bool enabled );

    /** The presentation chosen last for the pack with this key; false when there is none */
    bool GetPackPresentation( const CORE::CString& packKey, CORE::CString& presentationId ) const;
    void SetPackPresentation( const CORE::CString& packKey, const CORE::CString& presentationId );
    /** The template of that name; false when there is none */
    bool GetBulkTemplate( const CORE::CString& name, CBulkConversionSettings& settings ) const;

    /** Keeps the settings under their name (a template of that name is replaced); false without a name or when the list is full */
    bool SetBulkTemplate( const CBulkConversionSettings& settings );

    bool RemoveBulkTemplate( const CORE::CString& name );

    void GetBulkTemplateNames( CORE::CString::StringVector& names ) const;

    /** Forgets the choice of a pack: its default applies again */
    void ClearPackPresentation( const CORE::CString& packKey );

    /** Keeps the chords of the actions of the map, except the ids in 'leaveOut', as the entries of 'shortcuts' */
    void CaptureShortcuts( const GUI::CKeyboardShortcutMap& map, const CORE::CString::StringVector& leaveOut );

    /** Binds the chords of 'shortcuts' in the map; the number bound */
    UInt32 ApplyShortcuts( GUI::CKeyboardShortcutMap& map ) const;

    /** Writes the preferences into the section (the section "AssetWorkbench" of the settings) */
    void Save( CORE::CDataNode& section ) const;

    /** Reads the preferences of the section; what it does not hold keeps its value */
    void Load( const CORE::CDataNode& section );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H ? */
