/*
 *  AssetWorkbenchLib: the tooltips of the workbench
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

#ifndef GUCEF_GUI_CTABCONTROL_H
#include "gucefGUI_CTabControl.h"
#define GUCEF_GUI_CTABCONTROL_H
#endif /* GUCEF_GUI_CTABCONTROL_H ? */

#include "AssetWorkbenchLib_CWorkbenchTooltips.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct STooltip
{
    const char* key;
    const char* text;
};

/**
 *  The tooltips of the workbench. The texts say what the control does, in the way of the tooltips of the other forms: a short sentence
 *  without a final period, the key of the control in brackets when it has one.
 */
const STooltip g_tooltips[] =
{
    { "MenuBar", "The menus of the workbench: File, View and Help (Ctrl+, opens the settings, F1 the help)" },
    { "LeftTabs/Content", "The files of this computer and the folders and packs that are mounted" },
    { "LeftTabs/Scene", "The nodes of the scene that is shown" },
    { "LeftTabs/Sources", "The primary store and the online stores, with their sign in, library and downloads" },
    { "CenterTabs/3D", "The 3D view with the time line of the animation clips" },
    { "CenterTabs/Image", "The picture that is shown" },
    { "CenterTabs/Texture", "The texture that is shown, as the GPU has it" },
    { "CenterTabs/UV layout", "The texture coordinates of the sub mesh that is selected" },
    { "ReportViewForm/Entries", "The warnings and errors that the workbench and its plugins found while loading" },
    { "ReportViewForm/Filter", "Shows only the entries that contain this text" },
    { "ShowError", "Shows or hides the errors" },
    { "ShowWarning", "Shows or hides the warnings" },
    { "ShowInfo", "Shows or hides the notes" },
    { "ContentBrowserForm/Folders", "The folders of this computer and of the mounted folders and packs" },
    { "ContentBrowserForm/Items", "The files and folders of the selected folder; a double click opens one" },
    { "ContentBrowserForm/Thumbnails", "Shows pictures of the assets instead of the list with details" },
    { "ContentBrowserForm/TypeFilter", "Shows only one kind of asset" },
    { "Actions/AddDisk", "Adds the folder or archive of the box above to the primary store" },
    { "Actions/AddToPrimary", "Downloads the selected item of the store and adds it to the primary store" },
    { "Actions/Format", "The format of the item that is imported, the best one first" },
    { "Actions/Import", "Downloads the selected item in this format into the download folder" },
    { "Actions/Keep", "What the primary store keeps of an added pack: the original, the converted files or both" },
    { "Actions/Location", "The storage location the primary store puts an added pack in" },
    { "AssetSearchForm/DeleteSearch", "Deletes the saved search that is chosen" },
    { "Choice_*", "Narrows the results to the assets with this value" },
    { "AssetSearchForm/FiltersToggle", "Shows or hides the filters of the search" },
    { "AssetSearchForm/NextPage", "The next page of results" },
    { "AssetSearchForm/PreviousPage", "The page of results before" },
    { "AssetSearchForm/Query", "The words to search for in the names, tags and descriptions of the assets" },
    { "Results/Grid", "The results of the search; a double click opens the asset" },
    { "AssetSearchForm/Saved", "The saved searches: choose one to run it again" },
    { "AssetSearchForm/SaveSearch", "Keeps this search to run again" },
    { "AssetSearchForm/Sort", "The order of the results" },
    { "AssetSearchForm/Suggestions", "Words that fit what you typed: choose one to use it" },
    { "AssetSearchForm/TilesToggle", "Shows the results as tiles or as a list" },
    { "SourcesHost/SignIn", "Signs in to the store that is selected" },
    { "SourcesHost/Stores", "The primary store and the online stores; choose one to see what it holds" },
    { "TimelineForm/Clips", "The animation clips of the asset" },
    { "TimelineForm/Loop", "Plays the clip again when it ends" },
    { "TimelineForm/Time", "The position in the clip" },
    { "ImageViewForm/Frame", "The frame or layer of the picture that is shown" },
    { "ImageViewForm/Level", "The mip level of the picture that is shown" },
    { "PropertyInspectorForm/Apply", "Applies the changes to the properties" },
    { "PropertyInspectorForm/Revert", "Puts the properties back as they were" },
    { "PathField/Path", "A path or the name of a file" },
    { "PropertyInspectorForm/Properties", "The properties of what is shown; the ones with a field can be changed" },
    { "DeveloperConsole/CommandLine", "A command: help lists them; the key left of 1 closes the console again" },
    { "FileSystemDialog/Accept", "Chooses what is selected" },
    { "FileSystemDialog/Back", "Back to the folder before" },
    { "FileSystemDialog/Cancel", "Closes the window without choosing" },
    { "FileSystemDialog/Entries", "The files and folders of this folder; a double click opens a folder or chooses a file" },
    { "FileSystemDialog/FileTypes", "Shows only the files of this kind" },
    { "FileSystemDialog/Forward", "On to the folder after" },
    { "FileSystemDialog/Name", "The name of the file or folder that is chosen" },
    { "FileSystemDialog/NewFolder", "Makes a new folder in this folder" },
    { "FileSystemDialog/Path", "The folder that is shown; type a path and press Enter to go there" },
    { "FileSystemDialog/Places", "Places on this computer that are often used" },
    { "FileSystemDialog/Roots", "The file systems that can be browsed" },
    { "FileSystemDialog/Search", "Finds files by name in this folder" },
    { "FileSystemDialog/SearchSubfolders", "Looks in the folders below this folder as well" },
    { "FileSystemDialog/ShowHidden", "Shows the files and folders that are hidden" },
    { "FileSystemDialog/Up", "Up to the folder this folder is in" },
    { "Content/Close", "Closes the help" },
    { "Content/Links", "The documents that this one links to" },
    { "Content/Topics", "All documents of the help" },
    { "ButtonRow/Cancel", "Closes the window without entering a code" },
    { "ButtonRow/Submit", "Uses the code that is entered" },
    { "KeyringUnlockForm/Code", "The code of the keyring; it is never kept or shown" },
    { "OAuthCodeEntryForm/Code", "The code that the web site of the store gave" },
    { "OAuthCodeEntryForm/CopyUrl", "Copies the address to the clipboard" },
    { "OAuthCodeEntryForm/OpenUrl", "Opens the address in the browser of this computer" },
    { "OAuthCodeEntryForm/Paste", "Pastes the code from the clipboard" },
    { "OAuthCodeEntryForm/Url", "The address to open in a browser to sign in" },
    { "SettingsForm/Ok", "Applies the changes and closes the window" },
    { "SettingsForm/Apply", "Applies the changes and keeps the window open" },
    { "SettingsForm/Cancel", "Puts everything back as it was and closes the window (Escape)" },
    { "Pages/General", "The log, the developer console and the log of what you do" },
    { "Pages/Video", "The renderer, the resolution, full screen and vertical sync" },
    { "Pages/Audio", "The audio driver, the device and the volumes" },
    { "Pages/Input", "The keys of the workbench" },
    { "Pages/Authentication", "The keyring for saved passwords and the sign in settings of the stores" },
    { "Pages/Sources", "The online stores, their sign in and connection test, and the folder with the store plugins" },
    { "DeveloperConsole", "Switches the developer console on or off; it drops down on the key left of 1" },
    { "ConsoleLogOwnWindow", "Shows the log in a window of its own and not in the console the program was started from" },
    { "ConsoleLogWindow", "Shows the log in a console window" },
    { "UserActionLog", "Keeps a log of what you do, for finding problems" },
    { "DisplayMode", "Window, full screen or a window without borders" },
    { "ImageUpscaler", "The method that makes pictures sharper when the resolution is higher" },
    { "MovieUpscaling", "How movies are shown at a higher resolution" },
    { "PresentFilter", "The filter that scales the picture of the window to the screen" },
    { "PresentMode", "How the picture of the window is scaled to the screen" },
    { "Renderer", "The renderer that draws the window: OpenGL or Vulkan; a change is used after a restart" },
    { "Resolution", "The size of the window or of the screen" },
    { "Sharpness", "How sharp the filter makes the picture" },
    { "StopProgress", "Stops the work that follows Apply" },
    { "VSync", "Waits for the screen before the next picture is drawn, which prevents tearing" },
    { "BufferSize", "The size of the audio buffer: smaller is quicker, bigger is safer" },
    { "Driver", "The audio driver that plays the sound" },
    { "MasterMute", "Switches all sound off" },
    { "MasterVolume", "The volume of all sound" },
    { "OutputDevice", "The device that plays the sound" },
    { "SampleRate", "The number of samples per second of the audio device" },
    { "SpeakerLayout", "The speakers that are connected" },
    { "TestSignal", "The sound that the test plays" },
    { "InputSettings/Actions", "The actions of the workbench with their keys; select one and press Change" },
    { "InputSettings/Change", "Press this and then the new key of the selected action; Escape cancels" },
    { "InputSettings/Reset", "Puts the key of the selected action back as it came with the workbench" },
    { "InputSettings/ResetAll", "Puts all keys back as they came with the workbench" },
    { "KeyringGroup/ChangeCode", "Changes the code of the keyring" },
    { "KeyringGroup/KeyringAction", "Sets the code of the keyring up, or unlocks the keyring" },
    { "KeyringGroup/LockKeyring", "Locks the keyring now" },
    { "StoreGroup/StoreChoice", "The online store whose sign in settings are opened" },
    { "StoreGroup/StoreSettings", "Sign in and out, saved credentials and the keyring code of the store" },
    { "PluginsGroup/LoadPlugins", "Loads the store plugins of the folder now, without a restart" },
    { "PluginsGroup/PluginsDir", "The folder with the store plugins; empty for none" },
    { "OpenPackPresentation", "When on, a pack that is opened shows what it is without more clicks: the portrait of a character, an effect in four views, the scene of the pack or a showcase of its models" },
    { "OpenPacksInResourceView", "When on, a pack that is opened starts in the resource view of the content browser: its resources by function (scenes, characters, models, effects, textures, ...) with a count, not the folders of the pack. The switch Files or Resources of the content browser takes you to the other view at any time" },
    { "PreferCookedOutput", "When on, a model, scene or pack that has current cooked Ogre output (the gucef-ogre3d files of its conversion) shows that output. Off, or when none exists or it is stale, the quick look loads the original files through the Assimp codec" },
    { "AutomaticThumbnails", "When on, a worthwhile render of a pack or a model is kept as its thumbnail for the next time; a thumbnail of the content itself always comes first" },
    { "ClearAutomaticThumbnails", "Deletes the automatic thumbnails; the thumbnails of the content stay" },
    { "StoreEnabled", "Whether the store shows in the sources tab" },
    { "StoreSignIn", "Starts the sign in of the store that is selected" },
    { "StoresGroup/Stores", "The online stores that the plugins made; choose one to set it up" },
    { "TestConnection", "Checks the shape of what the store answers, without showing any value of the store" },
    { "StateText", "What the connection to the remote store is doing" },
    { "Services", "The asset store services found on the local network; choose one to use its address" },
    { "Endpoint", "The address of the store service: host:port of its session listener, worker.lan:47400 by default" },
    { "LanDiscovery", "Looks for store services on the local network; a typed address always wins" },
    { "FindAgain", "Looks on the local network again for store services" },
    { "Connect", "Connects to the store service at the address; this computer has to be paired first" },
    { "Disconnect", "Ends the connection: the workbench uses the store of this computer again" },
    { "PairingCode", "The one-time code that the server shows when pairing is opened; it is not kept anywhere" },
    { "Pair", "Pairs this computer with the service at the address using the code, then connects" },
    { "SignInSettingsForm/ButtonRow/Close", "Closes the window" },
    { "ButtonRow/Close", "Closes the window" },
    { "ForgetCredentials", "Deletes the saved credentials of the store" },
    { "KeyringCode", "Sets the code of the keyring up or changes it" },
    { "LockNow", "Locks the keyring now" },
    { "SaveCredentials", "Keeps the credentials you sign in with in the keyring" },
    { "SignInSettingsForm/SignIn", "Signs in to the store" },
    { "SignInSettingsForm/SignOut", "Signs out of the store and forgets its session" },
    { GUCEF_NULL, GUCEF_NULL }
};

/** The widget types that are controls */
const char* const g_controlTypes[] =
{
    "Button", "Checkbox", "Combobox", "Editbox", "Listbox", "TabControl", "Slider", "Spinner", "GridView", "Treeview", "FoldablePanel", "MenuBar", "SecretInput", GUCEF_NULL
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWorkbenchTooltips::CWorkbenchTooltips( void )
    : GUI::CITooltipTextProvider()
    , m_texts()
{GUCEF_TRACE;

    for ( UInt32 i=0; GUCEF_NULL != g_tooltips[ i ].key; ++i )
        m_texts[ g_tooltips[ i ].key ] = g_tooltips[ i ].text;
}

/*-------------------------------------------------------------------------*/

CWorkbenchTooltips::~CWorkbenchTooltips()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchTooltips::FindText( const CORE::CString& key ) const
{GUCEF_TRACE;

    TTextMap::const_iterator i = m_texts.find( key );
    return i != m_texts.end() ? (*i).second : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchTooltips::GetEntryCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_texts.size();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchTooltips::FindTextOfWidget( const GUI::CWidget& widget, Int32 localX, Int32 localY ) const
{GUCEF_TRACE;

    const CORE::CString name = widget.GetName();
    if ( name.IsNULLOrEmpty() )
        return CORE::CString::Empty;

    // the tab under the pointer first
    const GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( const_cast< GUI::CWidget* >( &widget ) );
    if ( GUCEF_NULL != tabs )
    {
        const Int32 tab = tabs->GetTabAt( localX, localY );
        if ( tab >= 0 )
        {
            const CORE::CString tabText = FindText( name + "/" + tabs->GetTabTitle( (UInt32) tab ) );
            if ( !tabText.IsNULLOrEmpty() )
                return tabText;
        }
    }
    const GUI::CWidget* parent = widget.GetParentWidget();
    if ( GUCEF_NULL != parent && !parent->GetName().IsNULLOrEmpty() )
    {
        const CORE::CString text = FindText( parent->GetName() + "/" + name );
        if ( !text.IsNULLOrEmpty() )
            return text;
    }
    const CORE::CString text = FindText( name );
    if ( !text.IsNULLOrEmpty() )
        return text;
    // a family of names ("Choice_*"): the key with a star at its end
    for ( TTextMap::const_iterator i = m_texts.begin(); i != m_texts.end(); ++i )
    {
        const CORE::CString& key = (*i).first;
        if ( key.Length() > 1 && '*' == key[ key.Length() - 1 ] && 0 == name.HasSubstr( key.SubstrToIndex( key.Length() - 1, true ) ) )
            return (*i).second;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchTooltips::GetTooltipFor( const GUI::CWidget& widget, Int32 localX, Int32 localY ) const
{GUCEF_TRACE;

    return FindTextOfWidget( widget, localX, localY );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchTooltips::IsControl( const GUI::CWidget& widget )
{GUCEF_TRACE;

    const CORE::CString& type = widget.GetType();
    for ( UInt32 i=0; GUCEF_NULL != g_controlTypes[ i ]; ++i )
        if ( type == g_controlTypes[ i ] )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchTooltips::FindMissing( const GUI::CWidget& root, CORE::CString::StringVector& missing ) const
{GUCEF_TRACE;

    UInt32 found = 0;
    if ( IsControl( root ) && root.GetTooltipAt( 0, 0 ).IsNULLOrEmpty() )
    {
        // a tab control has the text of every tab, or one text of its own
        bool covered = !FindTextOfWidget( root, 0, 0 ).IsNULLOrEmpty();
        const GUI::CTabControl* tabs = GUI::CWidget::As< GUI::CTabControl >( const_cast< GUI::CWidget* >( &root ) );
        if ( GUCEF_NULL != tabs && !covered && tabs->GetTabCount() > 0 )
        {
            covered = true;
            for ( UInt32 t=0; t<tabs->GetTabCount(); ++t )
                covered = covered && !FindText( root.GetName() + "/" + tabs->GetTabTitle( t ) ).IsNULLOrEmpty();
        }
        if ( !covered )
        {
            missing.push_back( root.GetNamePath() + " (" + root.GetType() + ")" );
            ++found;
        }
    }
    const GUI::CWidget::TWidgetVector& children = root.GetChildren();
    for ( size_t i=0; i<children.size(); ++i )
        found += FindMissing( *children[ i ], missing );
    return found;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
