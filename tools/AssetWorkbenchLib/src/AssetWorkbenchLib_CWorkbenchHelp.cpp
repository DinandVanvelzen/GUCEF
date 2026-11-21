/*
 *  AssetWorkbenchLib: the help of the workbench
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "AssetWorkbenchLib_CWorkbenchHelp.h"

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

const CORE::CString CWorkbenchHelp::TopicOverview = "workbench.overview";
const CORE::CString CWorkbenchHelp::TopicOpening = "workbench.opening";
const CORE::CString CWorkbenchHelp::TopicViews = "workbench.views";
const CORE::CString CWorkbenchHelp::TopicImporting = "workbench.importing";
const CORE::CString CWorkbenchHelp::TopicSettings = "workbench.settings";
const CORE::CString CWorkbenchHelp::TopicConsole = "workbench.console";
const CORE::CString CWorkbenchHelp::TopicShortcuts = "workbench.shortcuts";

// The documents, in the markup of CHelpMarkupParser: one place to read and review the help of the workbench
namespace {

const char* const g_document_overview =
    "@id workbench.overview\n"
    "@title The asset workbench\n"
    "@group Workbench\n"
    "@brief The workbench opens folders, archives and packs of assets, shows their models, textures, materials and scenes, and keeps a catalogue of the packs you add.\n"
    "@seealso workbench.opening, workbench.views, workbench.importing, workbench.settings, workbench.shortcuts\n"
    "@version 1\n"
    "\n"
    "# What it is for\n"
    "The workbench is a viewer and a catalogue for asset packs. You open a folder, an archive or a single file, look at what is in it in 3D, as a picture or as a material, and decide what to keep.\n"
    "\n"
    "# The window\n"
    "- Left: the content browser with the files of this computer and the folders and packs that are mounted, the scene browser with the nodes of the scene that is shown, and the sources: the primary store and the online stores ([[workbench.importing]]).\n"
    "- Middle: the 3D view with its time line, the picture or texture, and the layout of the UV coordinates of a mesh ([[workbench.views]]).\n"
    "- Right: the inspector with the statistics and the properties of what is shown.\n"
    "- Below: the report with the warnings and errors that were found, the transfers of the sources, and the strip that shows what takes time.\n"
    "\n"
    "# Where to start\n"
    "Choose File and Open folder, Open archive or pack, or Open file, or use the buttons at the top of the content tab ([[workbench.opening]]). Press F1 at any place for the help of that place, and Ctrl+, for the settings ([[workbench.settings]]).\n"
    "\n"
    "> Nothing is changed in the folders and archives that you open: the workbench only reads them.\n";

const char* const g_document_opening =
    "@id workbench.opening\n"
    "@title Opening assets\n"
    "@group Workbench\n"
    "@brief Open a folder, an archive or pack, or a file from the content tab; the places you opened lately are listed to open again.\n"
    "@seealso workbench.views, workbench.shortcuts\n"
    "@version 1\n"
    "\n"
    "# The ways to open\n"
    "Open folder :: browses a folder of this computer or of the virtual file system (Ctrl+Shift+O).\n"
    "Open archive or pack :: browses a zip, 7z, rar or tar archive, a Unity package or a similar pack as if it was a folder.\n"
    "Open file :: shows a model, a scene or a picture, and browses an archive (Ctrl+O).\n"
    "\n"
    "# What happens then\n"
    "A folder, an archive or a pack is mounted and shown in the VFS root of the content browser. A file is shown in the view that suits it: a model in the 3D view, a picture in the picture tab, a material as a sphere. While something is opening the activity indicator at the bottom right shows its name, with a button to cancel it.\n"
    "\n"
    "# What a pack shows\n"
    "A pack that is recognised (a MetaHuman package, a Unity package or project, a Godot project, an Unreal project, a pack of models) shows what it is as soon as it is opened. A character opens on its portrait, an effect pack plays the first effect in the four view layout, a pack with a scene of its own (the demo, sample or main scene) shows that scene, and other packs show a showcase: their models in a grid grouped by folder, with their names. The View menu and the Present list of the 3D view choose another presentation, and Default puts back the one of the pack; the choice is remembered for the pack. The first 48 models are loaded; View, Presentation: load more models loads more. Settings, General, Show a presentation when a pack is opened, switches this off: a pack is then only mounted and listed, and the double click on its root in the content browser still shows the presentation. The command line has --no-presentation and --presentation.\n"
    "\n"
    "# The resource view\n"
    "The content browser has a switch, Files or Resources. Resources groups what is in the pack by function instead of by folder: Scenes, Characters, Static models, Animations, Visual effects, Materials, Textures and images, Audio, Prefabs, Shaders, Fonts and Other, each with a count, then the sub-groups inside it (the folders of the pack and the usual prefixes of the names, such as buildings, props and vehicles). Selecting a resource shows it where it is in the pack, and the inspector says its group and its path in the pack. A double click opens it; a double click on a group opens the presentation of that group. The filter line searches the whole pack, not only the folder that is open. The files that are no content (metadata, caches, previews of the export) are not listed. The index behind the view is made once, in the background, and kept; the View menu, Re-index pack, and the command reindex make it again. Settings, General, Open packs in the resource view, decides which view a pack that is opened starts in. The command line has --resource-view and --files-view.\n"
    "\n"
    "# Automatic thumbnails\n"
    "When a pack or a model has no thumbnail of its own and the render of its presentation is worthwhile, the render is kept as the thumbnail for the next time. A thumbnail of the content itself always comes first. Settings, General, Create automatic thumbnails switches this off, and Clear automatic thumbnails deletes them; the command thumbnails does the same in the developer console.\n"
    "\n"
    "# The content browser\n"
    "The content browser has two roots: Local files, the file system of this computer, and VFS, the folders and packs that are mounted. Select a file to see its properties in the inspector, double click to open it. The files of this computer are listed with their type, size and date, the mounts show thumbnails when they can be made.\n"
    "\n"
    "# Places opened lately\n"
    "The list next to the buttons holds the places you opened lately; choose one to open it again.\n";

const char* const g_document_views =
    "@id workbench.views\n"
    "@title Views and layouts\n"
    "@group Workbench\n"
    "@brief The 3D view with its view presets and the four views layout, the picture and UV tabs, and the layouts Browse, Review and Preview with F9, F10 and F11.\n"
    "@seealso workbench.opening, workbench.shortcuts\n"
    "@version 1\n"
    "\n"
    "# The views\n"
    "- 3D: models, scenes, skeletons and particles, with the time line below it for animation clips.\n"
    "- Image or Texture: a picture, or the data a texture holds.\n"
    "- UV layout: the texture coordinates of the sub mesh that is selected.\n"
    "\n"
    "# View presets\n"
    "The view list of the 3D view holds Frame, Front, Side, Top, Three quarter and Four views. Frame fits the whole asset in view. The fixed views look at the asset from one side. Four views shows the fixed views next to each other. When you move the camera yourself the list says Custom. With the 3D view in focus the keys F and 1 to 4 choose Frame, Front, Side, Top and Three quarter.\n"
    "\n"
    "# Layouts\n"
    "Browse: the resources and the inspector are open, as the workbench starts. Review: the resources are folded away and the inspector and the report are bigger. Preview: both sides are folded and the view has almost the whole width.\n"
    "- F9 folds the resources away or brings them back.\n"
    "- F10 does the same for the inspector.\n"
    "- F11 switches to the Preview layout and back to the layout you had.\n"
    "Drag the splitters between the parts to your own sizes; the workbench keeps the layout for the next start.\n";

const char* const g_document_importing =
    "@id workbench.importing\n"
    "@title Importing and conversion\n"
    "@group Workbench\n"
    "@brief The sources tab: the primary store with its catalogue and conversion of packs, and the online stores with their sign in, library and downloads.\n"
    "@seealso workbench.settings\n"
    "@version 1\n"
    "\n"
    "# The primary store\n"
    "The primary store is the catalogue of the packs you keep. Add a folder or an archive to it, search it, and open the assets of a result. A pack that has to be converted (conversion pending) is converted with Convert, one pack or every pack that is waiting, as a transfer of the queue. Packs of Unreal projects are listed and browsed, an import of them is not built yet.\n"
    "\n"
    "# The online stores\n"
    "An online store shows the library of your account when you are signed in. Select an item, choose a format and press Import to download it into the download folder, or Add to primary store to download it and ingest it. Downloads are in the transfer queue below, where they can be paused and cancelled.\n"
    "\n"
    "# Sign in\n"
    "Press Sign in at the store. The sign in method depends on the store. Passwords that you let the workbench save are kept in the keyring, which is locked with a code of your own; sessions and passwords are never in the settings or in the log. Sign in settings shows the state of the keyring and the saved credentials of the store.\n"
    "\n"
    "# Jobs\n"
    "Choose Jobs and Jobs overview for the jobs page: every job of the asset store service (in client mode those of the server, which go on when the workbench is closed) and every long operation of the workbench itself, with its kind, scope, state, progress, times, duration and result. Show and Kind filter the list, Finished is the history. Cancel, Resume (a cancelled or failed run goes on where it stopped), Retry failed (only the assets that failed), Remove, Report (the result per asset) and Open result work on the selected job. The summary at the bottom right counts the running, queued and failed jobs and opens the page.\n"
    "\n"
    "# Bulk conversion\n"
    "Choose Jobs and Bulk conversion for a panel that converts a scope (the selection, a folder, a pack, a group of the resource view or the packs of a saved search) to the intermediate glTF with its sidecar and to the Ogre runtime files. Press Plan first: it lists what would be done per asset with the reason, and refuses a pack whose licence does not allow it. Start makes a job of the service. Templates keep the settings under a name.\n"
    "\n"
    "# Doing a step again\n"
    "Choose Jobs and then Re-index, Re-analyse, Re-import, Re-convert or Re-thumbnail to redo that step of the work on the pack in the resource view or the pack selected in the sources tab, as a job. Work that is done is kept and used until you ask for it again.\n"
    "\n"
    "# No store installed\n"
    "The workbench has no online store of its own: the store plugins are separate. Set the folder with the plugins in Settings, Sources, and restart the workbench.\n";

const char* const g_document_settings =
    "@id workbench.settings\n"
    "@title Settings\n"
    "@group Workbench\n"
    "@brief General, Video, Audio, Input, Authentication, Sources and Remote store: open them with Ctrl+, or from the File menu; OK and Apply keep them for the next start.\n"
    "@seealso workbench.shortcuts, workbench.console, workbench.importing\n"
    "@version 1\n"
    "\n"
    "# Opening\n"
    "Choose File and Settings, or press Ctrl+, (the key can be changed on the Input page). OK applies and closes, Apply applies and stays, Cancel and Escape put everything back.\n"
    "\n"
    "# The pages\n"
    "General :: the log in a console window, the developer console and the log of what you do, whether a pack that is opened shows its presentation, and the automatic thumbnails.\n"
    "Video :: the renderer (OpenGL or Vulkan), the resolution, full screen and vertical sync. A new renderer is used the next time the workbench starts.\n"
    "Audio :: the audio driver, device and volumes.\n"
    "Input :: the keys of the workbench; select an action, press Change and then the key. A key that another action has is refused. Reset puts the key back that came with the workbench.\n"
    "Authentication :: the keyring that protects the saved passwords of the online stores: set it up, unlock, lock and change the code, and the sign in settings of each store.\n"
    "Sources :: the online stores: whether a store is on, how it signs in, the state of the keyring, Sign in and Test connection, and the folder with the store plugins.\n"
    "Remote store :: the asset store service of a server (the k3s service): the stores found on the network, its address, the pairing code and the Pair button, Connect and Disconnect, and what the service is. When it is connected the sources tab, the search, the details, the jobs and the downloads work on that service.\n"
    "\n"
    "# Where they are kept\n"
    "The settings are in the settings file of the workbench in your user folder. Credentials are not: they are only in the keyring.\n";

const char* const g_document_shortcuts =
    "@id workbench.shortcuts\n"
    "@title Keyboard shortcuts\n"
    "@group Workbench\n"
    "@brief The keys of the workbench and what they do; change them in Settings, Input.\n"
    "@seealso workbench.settings, workbench.views\n"
    "@version 1\n"
    "\n"
    "# Keys\n"
    "The list of keys is made from the key bindings that are in effect.\n";

const char* const g_document_console =
    "@id workbench.console\n"
    "@title The developer console\n"
    "@group Workbench\n"
    "@brief A console that drops down over the window on the ` key (the key left of 1) or from the View menu, for commands of the workbench.\n"
    "@seealso workbench.settings\n"
    "@version 1\n"
    "\n"
    "# Opening\n"
    "Press the ` key, the key left of 1, or choose View and Developer console. The key does nothing while you type in a text field. Turn the console off or change its key in Settings, General and Input.\n"
    "\n"
    "# Commands\n"
    "help :: lists the commands.\n"
    "open <path> :: opens a folder, archive or file of this computer, or an asset of the VFS.\n"
    "convert [pending | <pack id>] :: converts a pack of the primary store, or every pack that is waiting.\n"
    "jobs [open | all | active | running | queued | failed | done | cancelled | finished] [kind] :: lists the jobs of the asset store service and of the workbench, or opens the jobs page.\n"
    "connect [host:port] :: connects to the remote asset store service (this computer has to be paired); without an address the address of the settings is used.\n"
    "pair <code> :: pairs this computer with the service at the address of the settings with the one-time code of a pairing window of the server, then connects. The code stays in the console output: it is good once.\n"
    "disconnect :: ends the connection; the workbench uses the store of this computer again.\n"
    "remote :: tells the state of the connection to the remote store.\n"
    "redo <reindex | reanalyse | reimport | reconvert | rethumbnail> :: does a step of the work on a pack again, as a job.\n"
    "bulk [open | plan <pack id> | start <pack id>] :: opens the bulk conversion panel, or plans or starts the conversion of a pack as a job.\n"    "layout <Browse | Review | Preview> :: puts the window in a layout.\n"
    "layout lint [on | off | report] :: switches the layout debug overlay of the window, or lists what the layout lint found.\n"
    "gui.lint [on | off | overlay | log | report] :: the same lint, as the GUI library has it.\n";

const char* const* GetDocumentTexts( UInt32& count )
{GUCEF_TRACE;

    static const char* const texts[] =
    {
        g_document_overview, g_document_opening, g_document_views, g_document_importing, g_document_settings, g_document_console, g_document_shortcuts
    };
    count = (UInt32) ( sizeof( texts ) / sizeof( texts[ 0 ] ) );
    return texts;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWorkbenchHelp::CWorkbenchHelp( void )
    : m_source()
    , m_catalogue( GUCEF_NEW GUI::CHelpCatalogue() )
{GUCEF_TRACE;

    m_source = m_catalogue->CreateSharedPtr();
    UInt32 count = 0;
    const char* const* texts = GetDocumentTexts( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CString error;
        if ( !m_catalogue->AddDocumentText( texts[ i ], "the help of the workbench", error ) )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchHelp: A document of the help is refused: " << error );
    }
}

/*-------------------------------------------------------------------------*/

CWorkbenchHelp::~CWorkbenchHelp()
{GUCEF_TRACE;

    // the shared pointer owns the catalogue
    m_catalogue = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const GUI::THelpSourcePtr&
CWorkbenchHelp::GetSource( void ) const
{GUCEF_TRACE;

    return m_source;
}

/*-------------------------------------------------------------------------*/

GUI::CHelpCatalogue&
CWorkbenchHelp::GetCatalogue( void )
{GUCEF_TRACE;

    return *m_catalogue;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchHelp::SetShortcutRows( const CORE::CString::StringVector& actions, const CORE::CString::StringVector& chords )
{GUCEF_TRACE;

    GUI::CHelpDocument document;
    if ( !m_catalogue->GetTopic( TopicShortcuts, document ) )
        return;
    // the rows of an earlier call go, the text of the document stays
    GUI::CHelpDocument::TBlockVector kept;
    for ( size_t i=0; i<document.blocks.size(); ++i )
        if ( GUI::CHelpDocument::BLOCK_TABLE_ROW != document.blocks[ i ].kind )
            kept.push_back( document.blocks[ i ] );
    document.blocks = kept;
    for ( size_t i=0; i<actions.size() && i<chords.size(); ++i )
    {
        GUI::CHelpDocument::TBlock row( GUI::CHelpDocument::BLOCK_TABLE_ROW, CORE::CString::Empty );
        row.cells.push_back( actions[ i ] );
        row.cells.push_back( chords[ i ] );
        document.blocks.push_back( row );
    }
    m_catalogue->AddDocument( document );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchHelp::FindTopicOfWidget( const GUI::CWidget* widget ) const
{GUCEF_TRACE;

    for ( const GUI::CWidget* current = widget; GUCEF_NULL != current; current = current->GetParentWidget() )
    {
        const CORE::CString& topic = current->GetPropertyRef( GUI::EWidgetProperty_HelpTopic );
        if ( !topic.IsNULLOrEmpty() && m_catalogue->HasTopic( topic ) )
            return topic;
    }
    return TopicOverview;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchHelp::Validate( CORE::CString::StringVector& problems ) const
{GUCEF_TRACE;

    return m_catalogue->Validate( problems );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
