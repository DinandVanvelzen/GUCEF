/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_GUI_CLISTBOX_H
#include "gucefGUI_CListBox.h"
#define GUCEF_GUI_CLISTBOX_H
#endif /* GUCEF_GUI_CLISTBOX_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CCHECKBOX_H
#include "gucefGUI_CCheckbox.h"
#define GUCEF_GUI_CCHECKBOX_H
#endif /* GUCEF_GUI_CCHECKBOX_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#ifndef GUCEF_GUI_CPOPUPMENU_H
#include "gucefGUI_CPopupMenu.h"
#define GUCEF_GUI_CPOPUPMENU_H
#endif /* GUCEF_GUI_CPOPUPMENU_H ? */

#include "gucefGUI_CFileSystemDialogForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CFileSystemDialogForm::AcceptedEvent = "GUCEF::GUI::CFileSystemDialogForm::AcceptedEvent";
const CORE::CEvent CFileSystemDialogForm::CancelledEvent = "GUCEF::GUI::CFileSystemDialogForm::CancelledEvent";
const CORE::CEvent CFileSystemDialogForm::SelectionChangedEvent = "GUCEF::GUI::CFileSystemDialogForm::SelectionChangedEvent";
const CORE::CEvent CFileSystemDialogForm::DirectoryChangedEvent = "GUCEF::GUI::CFileSystemDialogForm::DirectoryChangedEvent";
const CORE::CEvent CFileSystemDialogForm::FileTypeChangedEvent = "GUCEF::GUI::CFileSystemDialogForm::FileTypeChangedEvent";
const CORE::CEvent CFileSystemDialogForm::SearchChangedEvent = "GUCEF::GUI::CFileSystemDialogForm::SearchChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Folders read at the same time by a search through the subfolders */
const UInt32 MaxConcurrentSearchListings = 4;

/** A search stops when it found this many */
const UInt32 MaxSearchResults = 10000;

const char* const MenuNewFolder = "New folder";
const char* const MenuRename = "Rename";
const char* const MenuDelete = "Delete";
const char* const MenuRefresh = "Refresh";

/** The special directories the places pane shows unless the app picks others, in the order it shows them */
const CORE::TSpecialDirs UsualSpecialDirs[] = { CORE::SPECIALDIR_HOME_DIR, CORE::SPECIALDIR_DESKTOP_DIR, CORE::SPECIALDIR_MY_DOCUMENTS_DIR,
                                                CORE::SPECIALDIR_MY_DOWNLOADS_DIR, CORE::SPECIALDIR_MY_MUSIC_DIR, CORE::SPECIALDIR_MY_PICTURES_DIR,
                                                CORE::SPECIALDIR_MY_VIDEO_DIR };

/*-------------------------------------------------------------------------*/

bool
IsSeparator( UInt32 c )
{
    return '/' == c || '\\' == c;
}

/*-------------------------------------------------------------------------*/

CString
JoinPath( const CString& directory, const CString& name )
{
    if ( directory.IsNULLOrEmpty() )
        return name;
    if ( IsSeparator( directory[ directory.Length()-1 ] ) )
        return directory + name;
    return directory + "/" + name;
}

/*-------------------------------------------------------------------------*/

/** The directory above a path; empty when the path is a top of the file system without one ("" or a drive) */
CString
ParentOf( const CString& path )
{
    if ( path.IsNULLOrEmpty() || "/" == path )
        return CString::Empty;
    CString trimmed = path;
    while ( trimmed.Length() > 1 && IsSeparator( trimmed[ trimmed.Length()-1 ] ) )
        trimmed = trimmed.CutChars( 1, false );
    Int32 last = -1;
    for ( Int32 i=(Int32)trimmed.Length()-1; i>=0; --i )
    {
        if ( IsSeparator( trimmed[ i ] ) )
        {
            last = i;
            break;
        }
    }
    if ( last < 0 )
        return CString::Empty;
    if ( 0 == last )
        return "/";
    CString parent = trimmed.SubstrToIndex( (UInt32) last, true );
    if ( 2 == parent.Length() && ':' == parent[ 1 ] )
        parent += "\\";             // "C:" is the drive, "C:\" its top
    return parent;
}

/*-------------------------------------------------------------------------*/

/** The last part of a path */
CString
NameOf( const CString& path )
{
    Int32 last = -1;
    for ( Int32 i=(Int32)path.Length()-1; i>=0; --i )
    {
        if ( IsSeparator( path[ i ] ) )
        {
            last = i;
            break;
        }
    }
    return last < 0 ? path : path.CutChars( (UInt32) last + 1, true );
}

/*-------------------------------------------------------------------------*/

bool
IsAbsolute( const CString& path )
{
    if ( path.IsNULLOrEmpty() )
        return false;
    if ( IsSeparator( path[ 0 ] ) || '$' == path[ 0 ] )
        return true;
    return path.Length() >= 2 && ':' == path[ 1 ];
}

/*-------------------------------------------------------------------------*/

CString
ExtensionOf( const CString& name )
{
    Int32 dot = name.HasChar( '.', false );
    return dot > 0 ? name.CutChars( (UInt32) dot + 1, true ) : CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString
TwoDigits( Int32 value )
{
    return value < 10 ? "0" + CORE::Int32ToString( value ) : CORE::Int32ToString( value );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFileSystemDialogForm::SOptions::SOptions( void )
    : title()
    , acceptLabel()
    , initialRoot()
    , initialDirectory()
    , initialName()
    , multiSelect( false )
    , mustExist( true )
    , confirmOverwrite( true )
    , strictFileTypes( false )
    , includeAllFilesType( true )
    , acceptDirectoriesToo( false )
    , showFilesForContext( false )
    , showHidden( false )
    , allowCreateDirectory( true )
    , allowRename( false )
    , allowDelete( false )
    , readOnly( false )
    , showSpecialDirs( false )
    , specialDirsToShow()
    , filesToSave()
    , threadPoolName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::RegisterEvents( void )
{GUCEF_TRACE;

    AcceptedEvent.Initialize();
    CancelledEvent.Initialize();
    SelectionChangedEvent.Initialize();
    DirectoryChangedEvent.Initialize();
    FileTypeChangedEvent.Initialize();
    SearchChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CFileSystemDialogForm::CFileSystemDialogForm( EMode mode, CIGUIContext* context )
    : CFormEx( context )
    , m_mode( mode )
    , m_options()
    , m_roots()
    , m_places()
    , m_appPlaces()
    , m_specialDirLabels()
    , m_fileTypes()
    , m_selectedFileType( 0 )
    , m_choices()
    , m_currentRoot( 0 )
    , m_currentDirectory()
    , m_back()
    , m_forward()
    , m_entries()
    , m_listed()
    , m_sortColumn( 0 )
    , m_sortAscending( true )
    , m_search()
    , m_searchSubfolders( false )
    , m_searching( false )
    , m_searchTasks()
    , m_searchQueue()
    , m_searchFound( 0 )
    , m_selection()
    , m_pendingSelection()
    , m_pendingQuestion( QUESTION_NONE )
    , m_pendingPath()
    , m_open( false )
    , m_updatingWidgets( false )
    , m_messageBox( context )
    , m_contextMenu( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
    TEventCallback messageBoxClosedCallback( this, &CFileSystemDialogForm::OnMessageBoxClosed );
    SubscribeTo( &m_messageBox, CMessageBoxForm::ClosedEvent, messageBoxClosedCallback );
    if ( MODE_OPEN_DIRECTORY == mode || MODE_SAVE_DIRECTORY == mode )
        m_options.mustExist = MODE_OPEN_DIRECTORY == mode;
}

/*-------------------------------------------------------------------------*/

CFileSystemDialogForm::~CFileSystemDialogForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    StopSearch();
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != m_contextMenu && GUCEF_NULL != context )
        context->DestroyWidget( m_contextMenu );
    m_contextMenu = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CFileSystemDialogForm::EMode
CFileSystemDialogForm::GetMode( void ) const
{GUCEF_TRACE;

    return m_mode;
}

/*-------------------------------------------------------------------------*/

CFileSystemDialogForm::TOptions&
CFileSystemDialogForm::GetOptions( void )
{GUCEF_TRACE;

    return m_options;
}

/*-------------------------------------------------------------------------*/

const CFileSystemDialogForm::TOptions&
CFileSystemDialogForm::GetOptions( void ) const
{GUCEF_TRACE;

    return m_options;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::IsFileMode( void ) const
{GUCEF_TRACE;

    return MODE_OPEN_FILE == m_mode || MODE_SAVE_FILE == m_mode;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::IsSaveMode( void ) const
{GUCEF_TRACE;

    return MODE_SAVE_FILE == m_mode || MODE_SAVE_DIRECTORY == m_mode;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::AddRoot( const CString& displayName, CORE::TFileSystemAccessPtr access, const CString& initialDirectory )
{GUCEF_TRACE;

    if ( access.IsNULL() || displayName.IsNULLOrEmpty() )
        return false;
    for ( UInt32 i=0; i<(UInt32)m_roots.size(); ++i )
        if ( m_roots[ i ].name == displayName )
            return false;
    SRoot root;
    root.name = displayName;
    root.access = access;
    root.initialDirectory = initialDirectory.IsNULLOrEmpty() ? access->GetRootPath() : initialDirectory;
    m_roots.push_back( root );
    UpdatePanes();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::AddPlace( const CString& displayName, const CString& rootName, const CString& directory )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_roots.size(); ++i )
    {
        if ( m_roots[ i ].name == rootName )
        {
            SPlace place;
            place.label = displayName;
            place.root = i;
            place.directory = directory;
            m_appPlaces.push_back( place );
            UpdatePanes();
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CString
CFileSystemDialogForm::GetStandardSpecialDirLabel( CORE::TSpecialDirs specialDir )
{GUCEF_TRACE;

    switch ( specialDir )
    {
        case CORE::SPECIALDIR_HOME_DIR:             return "Home";
        case CORE::SPECIALDIR_DESKTOP_DIR:          return "Desktop";
        case CORE::SPECIALDIR_MY_DOCUMENTS_DIR:     return "Documents";
        case CORE::SPECIALDIR_MY_DOWNLOADS_DIR:     return "Downloads";
        case CORE::SPECIALDIR_MY_MUSIC_DIR:         return "Music";
        case CORE::SPECIALDIR_MY_PICTURES_DIR:      return "Pictures";
        case CORE::SPECIALDIR_MY_VIDEO_DIR:         return "Videos";
        case CORE::SPECIALDIR_MY_GAMES_DIR:         return "Games";
        case CORE::SPECIALDIR_TEMPLATES_DIR:        return "Templates";
        case CORE::SPECIALDIR_TEMP_DIR:             return "Temporary files";
        case CORE::SPECIALDIR_MODULE_DIR:           return "Program folder";
        case CORE::SPECIALDIR_CURRENT_WORK_DIR:     return "Working folder";
        case CORE::SPECIALDIR_APPDATA_DIR:          return "Application data";
        case CORE::SPECIALDIR_LOCAL_APPDATA_DIR:    return "Local application data";
        case CORE::SPECIALDIR_CACHE_DIR:            return "Cache";
        case CORE::SPECIALDIR_RECENT_DIR:           return "Recent";
        case CORE::SPECIALDIR_PROGRAMS_DIR:         return "Programs";
        default:                                    return CORE::SpecialDirToString( specialDir );
    }
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::SetSpecialDirLabel( CORE::TSpecialDirs specialDir, const CString& label )
{GUCEF_TRACE;

    m_specialDirLabels[ (Int32) specialDir ] = label;
    UpdatePanes();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::AddFileType( const CString& description, const CString::StringVector& patterns, const CString& defaultExtension )
{GUCEF_TRACE;

    TFileType fileType;
    fileType.description = description;
    fileType.patterns = patterns;
    fileType.defaultExtension = defaultExtension;
    while ( fileType.defaultExtension.Length() > 0 && '.' == fileType.defaultExtension[ 0 ] )
        fileType.defaultExtension = fileType.defaultExtension.CutChars( 1, true );
    m_fileTypes.push_back( fileType );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::SetSelectedFileType( UInt32 index )
{GUCEF_TRACE;

    UInt32 count = (UInt32) m_fileTypes.size() + ( m_options.includeAllFilesType ? 1 : 0 );
    if ( index >= count )
        return;
    m_selectedFileType = index;
    CCombobox* types = GetWidgetAs< CCombobox >( "FileTypes" );
    if ( GUCEF_NULL != types && types->GetSelectedIndex() != (Int32) index )
    {
        m_updatingWidgets = true;
        types->SetSelectedIndex( (Int32) index );
        m_updatingWidgets = false;
    }
    if ( GUCEF_NULL != GetRootWidget() )
        ShowEntries();
    NotifyObservers( FileTypeChangedEvent );
}

/*-------------------------------------------------------------------------*/

UInt32
CFileSystemDialogForm::GetSelectedFileType( void ) const
{GUCEF_TRACE;

    return m_selectedFileType;
}

/*-------------------------------------------------------------------------*/

const CFileSystemDialogForm::TFileTypeVector&
CFileSystemDialogForm::GetFileTypes( void ) const
{GUCEF_TRACE;

    return m_fileTypes;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::AddChoice( const CString& id, const CString& label, const CString::StringVector& options )
{GUCEF_TRACE;

    SChoice choice;
    choice.id = id;
    choice.label = label;
    choice.options = options;
    m_choices.push_back( choice );
}

/*-------------------------------------------------------------------------*/

CString
CFileSystemDialogForm::GetChoice( const CString& id )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_choices.size(); ++i )
    {
        if ( m_choices[ i ].id != id )
            continue;
        CWidget* widget = GetWidget( "Choice" + CORE::UInt32ToString( i ) );
        if ( GUCEF_NULL == widget )
            return CString::Empty;
        if ( m_choices[ i ].options.empty() )
            return CORE::BoolToString( widget->GetBoolProperty( EWidgetProperty_Checked ) );
        CCombobox* list = CWidget::As< CCombobox >( widget );
        return GUCEF_NULL != list ? list->GetText() : CString::Empty;
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::SetChoice( const CString& id, const CString& value )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_choices.size(); ++i )
    {
        if ( m_choices[ i ].id != id )
            continue;
        CWidget* widget = GetWidget( "Choice" + CORE::UInt32ToString( i ) );
        if ( GUCEF_NULL == widget )
            return;
        if ( m_choices[ i ].options.empty() )
        {
            widget->SetProperty( EWidgetProperty_Checked, CORE::BoolToString( CORE::StringToBool( value, false ) ) );
            return;
        }
        CCombobox* list = CWidget::As< CCombobox >( widget );
        for ( UInt32 o=0; GUCEF_NULL != list && o<(UInt32)m_choices[ i ].options.size(); ++o )
            if ( m_choices[ i ].options[ o ] == value )
                list->SetSelectedIndex( (Int32) o );
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    const bool fileMode = IsFileMode();
    CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "FileSystemDialog", "0.05,0 0.05,0", "0.9,0 0.9,0" );
    CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, m_options.title );

    // the tool bar: back, forward, up, the path, the search
    struct SButtonSpec { const char* name; const char* text; const char* position; const char* size; };
    static const SButtonSpec toolButtons[] = { { "Back", "Back", "0,16 0,48", "0,72 0,34" },
                                               { "Forward", "Forward", "0,92 0,48", "0,84 0,34" },
                                               { "Up", "Up", "0,180 0,48", "0,56 0,34" } };
    for ( UInt32 i=0; i<sizeof( toolButtons ) / sizeof( toolButtons[ 0 ] ); ++i )
    {
        CORE::CDataNode* button = CFormLayout::AddWidgetNode( *window, "Button", toolButtons[ i ].name, toolButtons[ i ].position, toolButtons[ i ].size );
        CFormLayout::SetWidgetNodeProperty( *button, EWidgetProperty_Text, toolButtons[ i ].text );
    }
    CFormLayout::AddWidgetNode( *window, "Editbox", "Path", "0,248 0,48", "1,-834 0,34" );
    CORE::CDataNode* searchLabel = CFormLayout::AddWidgetNode( *window, "Label", "SearchLabel", "1,-576 0,52", "0,64 0,28" );
    CFormLayout::SetWidgetNodeProperty( *searchLabel, EWidgetProperty_Text, "Search" );
    CFormLayout::AddWidgetNode( *window, "Editbox", "Search", "1,-508 0,48", "0,200 0,34" );
    CORE::CDataNode* subfolders = CFormLayout::AddWidgetNode( *window, "Checkbox", "SearchSubfolders", "1,-300 0,48", "0,156 0,34" );
    CFormLayout::SetWidgetNodeProperty( *subfolders, EWidgetProperty_Text, "Subfolders" );
    CORE::CDataNode* hidden = CFormLayout::AddWidgetNode( *window, "Checkbox", "ShowHidden", "1,-136 0,48", "0,120 0,34" );
    CFormLayout::SetWidgetNodeProperty( *hidden, EWidgetProperty_Text, "Hidden" );

    // the roots and the places on the left
    CORE::CDataNode* rootsLabel = CFormLayout::AddWidgetNode( *window, "Label", "RootsLabel", "0,16 0,94", "0,240 0,26" );
    CFormLayout::SetWidgetNodeProperty( *rootsLabel, EWidgetProperty_Text, "Locations" );
    CFormLayout::AddWidgetNode( *window, "Listbox", "Roots", "0,16 0,122", "0,240 0.35,-60" );
    CORE::CDataNode* placesLabel = CFormLayout::AddWidgetNode( *window, "Label", "PlacesLabel", "0,16 0.35,70", "0,240 0,26" );
    CFormLayout::SetWidgetNodeProperty( *placesLabel, EWidgetProperty_Text, "Places" );
    CFormLayout::AddWidgetNode( *window, "Listbox", "Places", "0,16 0.35,98", "0,240 0.65,-264" );

    // the folder
    CORE::CDataNode* entries = CFormLayout::AddWidgetNode( *window, "GridView", "Entries", "0,272 0,94", "1,-288 1,-288" );
    CFormLayout::SetWidgetNodeProperty( *entries, EWidgetProperty_Columns, "Name\nSize\nType\nModified" );
    CFormLayout::SetWidgetNodeProperty( *entries, EWidgetProperty_MultiSelect, CORE::BoolToString( m_options.multiSelect && !IsSaveMode() ) );
    CFormLayout::AddWidgetNode( *window, "Label", "Status", "0,272 1,-188", "1,-288 0,26" );

    // the name of what is chosen, and the file types
    CORE::CDataNode* nameLabel = CFormLayout::AddWidgetNode( *window, "Label", "NameLabel", "0,16 1,-140", "0,120 0,28" );
    CFormLayout::SetWidgetNodeProperty( *nameLabel, EWidgetProperty_Text, fileMode ? "File name:" : "Folder:" );
    CFormLayout::AddWidgetNode( *window, "Editbox", "Name", "0,140 1,-144", fileMode ? "1,-480 0,34" : "1,-156 0,34" );
    if ( fileMode )
    {
        CORE::CDataNode* types = CFormLayout::AddWidgetNode( *window, "Combobox", "FileTypes", "1,-328 1,-144", "0,312 0,34" );
        CFormLayout::SetWidgetNodeProperty( *types, EWidgetProperty_DropDownRows, "8" );
    }

    // the buttons, and the app's choices between the new folder button and them
    CORE::CDataNode* newFolder = CFormLayout::AddWidgetNode( *window, "Button", "NewFolder", "0,16 1,-82", "0,140 0,38" );
    CFormLayout::SetWidgetNodeProperty( *newFolder, EWidgetProperty_Text, "New folder" );
    Int32 x = 172;
    for ( UInt32 i=0; i<(UInt32)m_choices.size(); ++i )
    {
        const CString name = "Choice" + CORE::UInt32ToString( i );
        if ( m_choices[ i ].options.empty() )
        {
            CORE::CDataNode* box = CFormLayout::AddWidgetNode( *window, "Checkbox", name, "0," + CORE::Int32ToString( x ) + " 1,-80", "0,200 0,34" );
            CFormLayout::SetWidgetNodeProperty( *box, EWidgetProperty_Text, m_choices[ i ].label );
            x += 212;
        }
        else
        {
            CORE::CDataNode* label = CFormLayout::AddWidgetNode( *window, "Label", "ChoiceLabel" + CORE::UInt32ToString( i ), "0," + CORE::Int32ToString( x ) + " 1,-76", "0,120 0,28" );
            CFormLayout::SetWidgetNodeProperty( *label, EWidgetProperty_Text, m_choices[ i ].label );
            CORE::CDataNode* list = CFormLayout::AddWidgetNode( *window, "Combobox", name, "0," + CORE::Int32ToString( x + 124 ) + " 1,-80", "0,180 0,34" );
            CString items;
            for ( UInt32 o=0; o<(UInt32)m_choices[ i ].options.size(); ++o )
                items += ( o > 0 ? "\n" : "" ) + m_choices[ i ].options[ o ];
            CFormLayout::SetWidgetNodeProperty( *list, EWidgetProperty_Items, items );
            x += 316;
        }
    }
    if ( MODE_OPEN_FILE == m_mode && m_options.acceptDirectoriesToo )
    {
        CORE::CDataNode* openFolder = CFormLayout::AddWidgetNode( *window, "Button", "OpenFolder", "1,-424 1,-82", "0,160 0,38" );
        CFormLayout::SetWidgetNodeProperty( *openFolder, EWidgetProperty_Text, "Open folder" );
    }
    CORE::CDataNode* accept = CFormLayout::AddWidgetNode( *window, "Button", "Accept", "1,-252 1,-82", "0,112 0,38" );
    CFormLayout::SetWidgetNodeProperty( *accept, EWidgetProperty_Text, m_options.acceptLabel );
    CFormLayout::SetWidgetNodeProperty( *accept, EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* cancel = CFormLayout::AddWidgetNode( *window, "Button", "Cancel", "1,-128 1,-82", "0,112 0,38" );
    CFormLayout::SetWidgetNodeProperty( *cancel, EWidgetProperty_Text, "Cancel" );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    // every widget the dialog listens to is subscribed to where it is looked up: the widgets of a layout that is loaded again are new
    CWidget* widget = GetWidget( "Back" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnBackActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Forward" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnForwardActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Up" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnUpActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "NewFolder" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnNewFolderActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "OpenFolder" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnOpenFolderActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Accept" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnAcceptActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Cancel" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnCancelActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Path" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnPathTextEntered );
        SubscribeTo( widget, CEditbox::TextEnteredEvent, callback );
    }
    widget = GetWidget( "Name" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnNameTextEntered );
        SubscribeTo( widget, CEditbox::TextEnteredEvent, callback );
    }
    widget = GetWidget( "Search" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnSearchTextChanged );
        SubscribeTo( widget, CEditbox::TextChangedEvent, callback );
    }
    widget = GetWidget( "SearchSubfolders" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback checkedCallback( this, &CFileSystemDialogForm::OnSearchSubfoldersChecked );
        SubscribeTo( widget, CCheckbox::BoxCheckedEvent, checkedCallback );
        TEventCallback uncheckedCallback( this, &CFileSystemDialogForm::OnSearchSubfoldersUnchecked );
        SubscribeTo( widget, CCheckbox::BoxUncheckedEvent, uncheckedCallback );
    }
    widget = GetWidget( "ShowHidden" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback checkedCallback( this, &CFileSystemDialogForm::OnShowHiddenChecked );
        SubscribeTo( widget, CCheckbox::BoxCheckedEvent, checkedCallback );
        TEventCallback uncheckedCallback( this, &CFileSystemDialogForm::OnShowHiddenUnchecked );
        SubscribeTo( widget, CCheckbox::BoxUncheckedEvent, uncheckedCallback );
    }
    widget = GetWidget( "Roots" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnRootsSelectionChanged );
        SubscribeTo( widget, CListbox::SelectionChangedEvent, callback );
    }
    widget = GetWidget( "Places" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnPlacesSelectionChanged );
        SubscribeTo( widget, CListbox::SelectionChangedEvent, callback );
    }
    widget = GetWidget( "FileTypes" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnFileTypesValueChanged );
        SubscribeTo( widget, CWidget::ValueChangedEvent, callback );
    }
    widget = GetWidget( "Entries" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback selectionCallback( this, &CFileSystemDialogForm::OnEntriesSelectionChanged );
        SubscribeTo( widget, CGridView::SelectionChangedEvent, selectionCallback );
        TEventCallback columnCallback( this, &CFileSystemDialogForm::OnEntriesColumnClicked );
        SubscribeTo( widget, CGridView::ColumnClickedEvent, columnCallback );
        TEventCallback itemCallback( this, &CFileSystemDialogForm::OnEntriesItemActivated );
        SubscribeTo( widget, CWidget::ItemActivatedEvent, itemCallback );
        TEventCallback clickCallback( this, &CFileSystemDialogForm::OnEntriesMouseClick );
        SubscribeTo( widget, CWidget::MouseClickEvent, clickCallback );
    }
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnNavigation );
        SubscribeTo( root, CWidget::NavigationEvent, callback );
    }
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context )
    {
        TEventCallback callback( this, &CFileSystemDialogForm::OnFrameStarted );
        SubscribeTo( context, CGUIContext::FrameStartedEvent, callback );
    }

    // the file types
    CCombobox* types = GetWidgetAs< CCombobox >( "FileTypes" );
    if ( GUCEF_NULL != types )
    {
        CCombobox::TStringVector items;
        for ( UInt32 i=0; i<(UInt32)m_fileTypes.size(); ++i )
        {
            CString patterns;
            for ( UInt32 p=0; p<(UInt32)m_fileTypes[ i ].patterns.size(); ++p )
                patterns += ( p > 0 ? ", " : "" ) + m_fileTypes[ i ].patterns[ p ];
            items.push_back( patterns.IsNULLOrEmpty() ? m_fileTypes[ i ].description : m_fileTypes[ i ].description + " (" + patterns + ")" );
        }
        if ( m_options.includeAllFilesType )
            items.push_back( "All files (*)" );
        m_updatingWidgets = true;
        types->SetListItems( items );
        types->SetSelectedIndex( (Int32) m_selectedFileType );
        m_updatingWidgets = false;
    }
    UpdatePanes();
}

/*-------------------------------------------------------------------------*/

CORE::IFileSystemAccess*
CFileSystemDialogForm::GetCurrentAccess( void ) const
{GUCEF_TRACE;

    if ( m_currentRoot >= (UInt32) m_roots.size() || m_roots[ m_currentRoot ].access.IsNULL() )
        return GUCEF_NULL;
    return const_cast< CORE::IFileSystemAccess* >( m_roots[ m_currentRoot ].access.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

CString
CFileSystemDialogForm::ResolvePath( const CString& nameOrPath ) const
{GUCEF_TRACE;

    if ( nameOrPath.IsNULLOrEmpty() )
        return m_currentDirectory;
    if ( IsAbsolute( nameOrPath ) )
        return nameOrPath;
    return JoinPath( m_currentDirectory, nameOrPath );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::Open( void )
{GUCEF_TRACE;

    if ( m_roots.empty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemDialogForm: There is no file system to browse, add a root first" );
        return false;
    }
    if ( m_options.title.IsNULLOrEmpty() )
        m_options.title = MODE_OPEN_FILE == m_mode ? "Open" : MODE_SAVE_FILE == m_mode ? "Save as" : MODE_OPEN_DIRECTORY == m_mode ? "Select a folder" : "Choose where to save";
    if ( m_options.acceptLabel.IsNULLOrEmpty() )
        m_options.acceptLabel = MODE_OPEN_FILE == m_mode ? "Open" : MODE_SAVE_FILE == m_mode ? "Save" : MODE_OPEN_DIRECTORY == m_mode ? "Select folder" : "Save here";
    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;

    GetRootWidget()->SetProperty( EWidgetProperty_Title, m_options.title );
    CWidget* accept = GetWidget( "Accept" );
    if ( GUCEF_NULL != accept )
        accept->SetProperty( EWidgetProperty_Text, m_options.acceptLabel );
    CWidget* hidden = GetWidget( "ShowHidden" );
    if ( GUCEF_NULL != hidden )
        hidden->SetProperty( EWidgetProperty_Checked, CORE::BoolToString( m_options.showHidden ) );

    // where to start
    UInt32 root = 0;
    for ( UInt32 i=0; i<(UInt32)m_roots.size(); ++i )
        if ( m_roots[ i ].name == m_options.initialRoot )
            root = i;
    CString directory = m_options.initialDirectory.IsNULLOrEmpty() ? m_roots[ root ].initialDirectory : m_options.initialDirectory;
    m_back.clear();
    m_forward.clear();
    m_selection.clear();
    m_open = true;
    UpdatePanes();
    GoTo( root, directory, false );
    SetName( m_options.initialName );
    UpdateButtons();
    return ShowModal();
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::GoTo( UInt32 root, const CString& directory, bool remember )
{GUCEF_TRACE;

    if ( root >= (UInt32) m_roots.size() )
        return;
    StopSearch();
    if ( remember && ( root != m_currentRoot || directory != m_currentDirectory ) )
    {
        SLocation here;
        here.root = m_currentRoot;
        here.directory = m_currentDirectory;
        m_back.push_back( here );
        m_forward.clear();
    }
    m_currentRoot = root;
    m_currentDirectory = directory;
    ReadDirectory();

    m_updatingWidgets = true;
    CWidget* path = GetWidget( "Path" );
    if ( GUCEF_NULL != path )
        path->SetProperty( EWidgetProperty_Text, m_currentDirectory );
    CListbox* roots = GetWidgetAs< CListbox >( "Roots" );
    if ( GUCEF_NULL != roots && roots->GetSelectedIndex() != (Int32) root )
        roots->SetSelectedIndex( (Int32) root );
    CListbox* places = GetWidgetAs< CListbox >( "Places" );
    if ( GUCEF_NULL != places )
        places->SetSelectedIndex( -1 );
    m_updatingWidgets = false;

    UpdateButtons();
    NotifyObservers( DirectoryChangedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::NavigateTo( const CString& rootName, const CString& directory )
{GUCEF_TRACE;

    UInt32 root = m_currentRoot;
    if ( !rootName.IsNULLOrEmpty() )
    {
        bool found = false;
        for ( UInt32 i=0; i<(UInt32)m_roots.size(); ++i )
        {
            if ( m_roots[ i ].name == rootName )
            {
                root = i;
                found = true;
            }
        }
        if ( !found )
            return false;
    }
    if ( root >= (UInt32) m_roots.size() )
        return false;
    CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
    if ( CORE::FILESYSTEMACCESS_SUCCESS != m_roots[ root ].access->TryGetEntryType( directory, type ) || CORE::FILESYSTEMENTRYTYPE_DIRECTORY != type )
        return false;
    GoTo( root, directory, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::GoBack( void )
{GUCEF_TRACE;

    if ( m_back.empty() )
        return false;
    SLocation here;
    here.root = m_currentRoot;
    here.directory = m_currentDirectory;
    SLocation there = m_back.back();
    m_back.pop_back();
    m_forward.push_back( here );
    GoTo( there.root, there.directory, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::GoForward( void )
{GUCEF_TRACE;

    if ( m_forward.empty() )
        return false;
    SLocation here;
    here.root = m_currentRoot;
    here.directory = m_currentDirectory;
    SLocation there = m_forward.back();
    m_forward.pop_back();
    m_back.push_back( here );
    GoTo( there.root, there.directory, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::GoUp( void )
{GUCEF_TRACE;

    CORE::IFileSystemAccess* access = GetCurrentAccess();
    if ( GUCEF_NULL == access || m_currentDirectory == access->GetRootPath() )
        return false;
    CString parent = ParentOf( m_currentDirectory );
    if ( parent.IsNULLOrEmpty() && !access->GetRootPath().IsNULLOrEmpty() )
        return false;
    GoTo( m_currentRoot, parent, true );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::Refresh( void )
{GUCEF_TRACE;

    if ( m_searching || ( m_searchSubfolders && m_search.IsValid() && !m_search.GetPattern().IsNULLOrEmpty() ) )
        StartSearch();
    else
        ReadDirectory();
}

/*-------------------------------------------------------------------------*/

const CString&
CFileSystemDialogForm::GetCurrentRootName( void ) const
{GUCEF_TRACE;

    return m_currentRoot < (UInt32) m_roots.size() ? m_roots[ m_currentRoot ].name : CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CString&
CFileSystemDialogForm::GetCurrentDirectory( void ) const
{GUCEF_TRACE;

    return m_currentDirectory;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::ReadDirectory( void )
{GUCEF_TRACE;

    m_entries.clear();
    CORE::IFileSystemAccess* access = GetCurrentAccess();
    if ( GUCEF_NULL != access )
    {
        CORE::TFileSystemIteratorPtr iterator;
        CORE::TFileSystemAccessResult result = access->TryGetFileSystemIterator( m_currentDirectory, iterator );
        if ( CORE::FILESYSTEMACCESS_SUCCESS == result && !iterator.IsNULL() )
        {
            if ( iterator->FindFirst( m_currentDirectory ) )
            {
                do
                {
                    SEntry entry;
                    entry.name = iterator->GetResourceName();
                    entry.path = iterator->GetResourcePath();
                    if ( entry.path.IsNULLOrEmpty() )
                        entry.path = JoinPath( m_currentDirectory, entry.name );
                    entry.isDirectory = iterator->IsADirectory();
                    CORE::CResourceMetaData metaData;
                    const bool hasMetaData = iterator->TryReadMetaData( metaData );
                    entry.hasSize = hasMetaData && metaData.hasResourceSizeInBytes && !entry.isDirectory;
                    entry.size = entry.hasSize ? metaData.resourceSizeInBytes : 0;
                    entry.hasModified = hasMetaData && metaData.hasModifiedDateTime;
                    if ( entry.hasModified )
                        entry.modified = metaData.modifiedDateTime;
                    entry.hidden = ( entry.name.Length() > 0 && '.' == entry.name[ 0 ] ) || ( hasMetaData && metaData.hasIsHidden && metaData.isHidden );
                    m_entries.push_back( entry );
                }
                while ( iterator->FindNext() );
                iterator->FindClose();
            }
        }
        else
        if ( m_open )
            Tell( DescribeResult( result ) + "\n" + m_currentDirectory );
    }
    SortEntries();
    ShowEntries();
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::FitsFileType( const CString& name ) const
{GUCEF_TRACE;

    if ( !IsFileMode() || m_selectedFileType >= (UInt32) m_fileTypes.size() )
        return true;   // no types, or "All files"
    const TFileType& fileType = m_fileTypes[ m_selectedFileType ];
    if ( fileType.patterns.empty() )
        return true;
    for ( UInt32 i=0; i<(UInt32)fileType.patterns.size(); ++i )
        if ( "*" == fileType.patterns[ i ] || CORE::CGlobPattern::IsMatch( fileType.patterns[ i ], name ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::IsShown( const SEntry& entry ) const
{GUCEF_TRACE;

    if ( entry.hidden && !m_options.showHidden )
        return false;
    if ( !entry.isDirectory )
    {
        if ( !IsFileMode() && !m_options.showFilesForContext )
            return false;
        if ( IsFileMode() && !FitsFileType( NameOf( entry.name ) ) )
            return false;
    }
    if ( m_searchSubfolders && !m_search.GetPattern().IsNULLOrEmpty() )
        return true;   // the search chose the results already
    if ( !m_search.GetPattern().IsNULLOrEmpty() )
        return m_search.IsMatch( entry.name );
    return true;
}

/*-------------------------------------------------------------------------*/

namespace {

/** Folders first, then by the column; equal entries keep their order (a stable sort) */
class CEntryOrder
{
    public:

    CEntryOrder( Int32 column, bool ascending ) : m_column( column ), m_ascending( ascending ) {}

    template< class TEntry >
    bool operator()( const TEntry& a, const TEntry& b ) const
    {
        if ( a.isDirectory != b.isDirectory )
            return a.isDirectory;
        Int32 compared = 0;
        if ( 1 == m_column )
            compared = a.size < b.size ? -1 : ( b.size < a.size ? 1 : 0 );
        else
        if ( 2 == m_column )
        {
            const CString typeA = ExtensionOf( a.name ).Lowercase(), typeB = ExtensionOf( b.name ).Lowercase();
            compared = typeA < typeB ? -1 : ( typeB < typeA ? 1 : 0 );
        }
        else
        if ( 3 == m_column )
            compared = a.modified < b.modified ? -1 : ( b.modified < a.modified ? 1 : 0 );
        if ( 0 == compared )
        {
            const CString nameA = a.name.Lowercase(), nameB = b.name.Lowercase();
            compared = nameA < nameB ? -1 : ( nameB < nameA ? 1 : 0 );
        }
        return m_ascending ? compared < 0 : compared > 0;
    }

    private:

    Int32 m_column;
    bool m_ascending;
};

}

void
CFileSystemDialogForm::SortEntries( void )
{GUCEF_TRACE;

    std::stable_sort( m_entries.begin(), m_entries.end(), CEntryOrder( m_sortColumn, m_sortAscending ) );
}

/*-------------------------------------------------------------------------*/

CString
CFileSystemDialogForm::FormatSize( UInt64 size )
{GUCEF_TRACE;

    if ( size < 1024 )
        return CORE::UInt64ToString( size ) + " B";
    if ( size < 1024 * 1024 )
        return CORE::UInt64ToString( ( size + 512 ) / 1024 ) + " KB";
    if ( size < (UInt64) 1024 * 1024 * 1024 )
        return CORE::UInt64ToString( ( size + 512 * 1024 ) / ( 1024 * 1024 ) ) + " MB";
    return CORE::UInt64ToString( ( size + (UInt64) 512 * 1024 * 1024 ) / ( (UInt64) 1024 * 1024 * 1024 ) ) + " GB";
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::ShowEntries( void )
{GUCEF_TRACE;

    m_listed.clear();
    CGridView::TRowVector rows;
    for ( UInt32 i=0; i<(UInt32)m_entries.size(); ++i )
    {
        const SEntry& entry = m_entries[ i ];
        if ( !IsShown( entry ) )
            continue;
        m_listed.push_back( i );
        CGridView::TStringVector cells;
        cells.push_back( entry.name );
        cells.push_back( entry.hasSize ? FormatSize( entry.size ) : CString::Empty );
        CString extension = ExtensionOf( NameOf( entry.name ) );
        cells.push_back( entry.isDirectory ? CString( "Folder" ) : extension.IsNULLOrEmpty() ? CString( "File" ) : extension.Uppercase() + " file" );
        cells.push_back( entry.hasModified ? entry.modified.ToIso8601DateString( true ) + " " + TwoDigits( entry.modified.GetHours() ) + ":" + TwoDigits( entry.modified.GetMinutes() ) : CString::Empty );
        rows.push_back( cells );
    }

    CGridView* grid = GetWidgetAs< CGridView >( "Entries" );
    if ( GUCEF_NULL != grid )
    {
        m_updatingWidgets = true;
        Int32 width = grid->GetRect().width;
        if ( width > 0 )
        {
            GUCEF::vector< Int32 > widths;
            widths.push_back( width * 50 / 100 );
            widths.push_back( width * 13 / 100 );
            widths.push_back( width * 15 / 100 );
            widths.push_back( width - widths[ 0 ] - widths[ 1 ] - widths[ 2 ] );
            grid->SetIntListProperty( EWidgetProperty_ColumnWidths, widths );
        }
        grid->SetRows( rows );
        grid->SetIntProperty( EWidgetProperty_SortColumn, m_sortColumn );
        grid->SetBoolProperty( EWidgetProperty_SortAscending, m_sortAscending );
        m_updatingWidgets = false;
    }
    UpdateStatus();
    UpdateButtons();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::UpdateStatus( void )
{GUCEF_TRACE;

    CWidget* status = GetWidget( "Status" );
    if ( GUCEF_NULL == status )
        return;
    CString text;
    if ( m_searching )
        text = "Searching... " + CORE::UInt32ToString( (UInt32) m_listed.size() ) + " found";
    else
    if ( m_searchSubfolders && !m_search.GetPattern().IsNULLOrEmpty() )
        text = CORE::UInt32ToString( (UInt32) m_listed.size() ) + " found in the subfolders";
    else
        text = CORE::UInt32ToString( (UInt32) m_listed.size() ) + ( 1 == m_listed.size() ? " item" : " items" );

    if ( MODE_SAVE_DIRECTORY == m_mode && !m_options.filesToSave.empty() )
    {
        // what will be saved here, the names already here marked
        text += "   Will save: ";
        CORE::IFileSystemAccess* access = GetCurrentAccess();
        for ( UInt32 i=0; i<(UInt32)m_options.filesToSave.size(); ++i )
        {
            CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
            bool exists = GUCEF_NULL != access && CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryType( JoinPath( m_currentDirectory, m_options.filesToSave[ i ] ), type )
                          && CORE::FILESYSTEMENTRYTYPE_NONE != type;
            text += ( i > 0 ? ", " : "" ) + m_options.filesToSave[ i ] + ( exists ? " (exists)" : "" );
        }
    }
    status->SetProperty( EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::UpdatePanes( void )
{GUCEF_TRACE;

    // the places: the app's, then the special directories of the roots
    m_places = m_appPlaces;
    if ( m_options.showSpecialDirs )
    {
        CORE::TSpecialDirSet wanted = m_options.specialDirsToShow;
        if ( wanted.empty() )
            wanted.insert( UsualSpecialDirs, UsualSpecialDirs + sizeof( UsualSpecialDirs ) / sizeof( UsualSpecialDirs[ 0 ] ) );

        // the usual order first, then the rest by id
        GUCEF::vector< CORE::TSpecialDirs > order( UsualSpecialDirs, UsualSpecialDirs + sizeof( UsualSpecialDirs ) / sizeof( UsualSpecialDirs[ 0 ] ) );
        CORE::TSpecialDirSet::const_iterator w = wanted.begin();
        while ( w != wanted.end() )
        {
            if ( std::find( order.begin(), order.end(), (*w) ) == order.end() )
                order.push_back( (*w) );
            ++w;
        }

        UInt32 rootsWithSpecialDirs = 0;
        TPlaceVector specials;
        for ( UInt32 r=0; r<(UInt32)m_roots.size(); ++r )
        {
            CORE::TSpecialDirSet available;
            if ( CORE::FILESYSTEMACCESS_SUCCESS != m_roots[ r ].access->ListAvailableSpecialDirs( available ) || available.empty() )
                continue;
            bool any = false;
            for ( UInt32 o=0; o<(UInt32)order.size(); ++o )
            {
                if ( wanted.find( order[ o ] ) == wanted.end() || available.find( order[ o ] ) == available.end() )
                    continue;
                CString path;
                CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
                if ( CORE::FILESYSTEMACCESS_SUCCESS != m_roots[ r ].access->TryResolveSpecialDir( order[ o ], path ) ||
                     CORE::FILESYSTEMACCESS_SUCCESS != m_roots[ r ].access->TryGetEntryType( path, type ) ||
                     CORE::FILESYSTEMENTRYTYPE_DIRECTORY != type )
                    continue;

                // a place already listed (Linux gives the home directory for user directories that are not set up) is left out
                bool duplicate = false;
                for ( UInt32 s=0; s<(UInt32)specials.size(); ++s )
                    if ( specials[ s ].root == r && specials[ s ].directory == path )
                        duplicate = true;
                if ( duplicate )
                    continue;

                SPlace place;
                GUCEF::map< Int32, CString >::const_iterator label = m_specialDirLabels.find( (Int32) order[ o ] );
                place.label = label != m_specialDirLabels.end() ? (*label).second : GetStandardSpecialDirLabel( order[ o ] );
                place.root = r;
                place.directory = path;
                specials.push_back( place );
                any = true;
            }
            if ( any )
                ++rootsWithSpecialDirs;
        }
        for ( UInt32 s=0; s<(UInt32)specials.size(); ++s )
        {
            if ( rootsWithSpecialDirs > 1 )
                specials[ s ].label += " (" + m_roots[ specials[ s ].root ].name + ")";
            m_places.push_back( specials[ s ] );
        }
    }

    m_updatingWidgets = true;
    CListbox* roots = GetWidgetAs< CListbox >( "Roots" );
    if ( GUCEF_NULL != roots )
    {
        CListbox::TStringVector names;
        for ( UInt32 i=0; i<(UInt32)m_roots.size(); ++i )
            names.push_back( m_roots[ i ].name );
        roots->SetItems( names );
        roots->SetSelectedIndex( m_currentRoot < (UInt32) m_roots.size() ? (Int32) m_currentRoot : -1 );
    }
    CListbox* places = GetWidgetAs< CListbox >( "Places" );
    if ( GUCEF_NULL != places )
    {
        CListbox::TStringVector labels;
        for ( UInt32 i=0; i<(UInt32)m_places.size(); ++i )
            labels.push_back( m_places[ i ].label );
        places->SetItems( labels );
        places->SetSelectedIndex( -1 );
        places->SetVisibility( !m_places.empty() );
    }
    CWidget* placesLabel = GetWidget( "PlacesLabel" );
    if ( GUCEF_NULL != placesLabel )
        placesLabel->SetVisibility( !m_places.empty() );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::UpdateButtons( void )
{GUCEF_TRACE;

    CORE::IFileSystemAccess* access = GetCurrentAccess();
    CWidget* back = GetWidget( "Back" );
    if ( GUCEF_NULL != back )
        back->SetEnabled( !m_back.empty() );
    CWidget* forward = GetWidget( "Forward" );
    if ( GUCEF_NULL != forward )
        forward->SetEnabled( !m_forward.empty() );
    CWidget* up = GetWidget( "Up" );
    if ( GUCEF_NULL != up )
        up->SetEnabled( GUCEF_NULL != access && m_currentDirectory != access->GetRootPath() &&
                        !( ParentOf( m_currentDirectory ).IsNULLOrEmpty() && !access->GetRootPath().IsNULLOrEmpty() ) );
    CWidget* newFolder = GetWidget( "NewFolder" );
    if ( GUCEF_NULL != newFolder )
    {
        newFolder->SetVisibility( m_options.allowCreateDirectory && !m_options.readOnly );
        newFolder->SetEnabled( GUCEF_NULL != access && access->IsWriteable( m_currentDirectory ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::SetSearch( const CString& glob, bool includeSubfolders )
{GUCEF_TRACE;

    StopSearch();
    m_search.SetPattern( glob, false );
    m_searchSubfolders = includeSubfolders;

    m_updatingWidgets = true;
    CWidget* search = GetWidget( "Search" );
    if ( GUCEF_NULL != search && search->GetProperty( EWidgetProperty_Text ) != glob )
        search->SetProperty( EWidgetProperty_Text, glob );
    CWidget* subfolders = GetWidget( "SearchSubfolders" );
    if ( GUCEF_NULL != subfolders && subfolders->GetBoolProperty( EWidgetProperty_Checked ) != includeSubfolders )
        subfolders->SetProperty( EWidgetProperty_Checked, CORE::BoolToString( includeSubfolders ) );
    m_updatingWidgets = false;

    if ( !glob.IsNULLOrEmpty() && includeSubfolders )
        StartSearch();
    else
        ReadDirectory();
    NotifyObservers( SearchChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::StartSearch( void )
{GUCEF_TRACE;

    StopSearch();
    m_entries.clear();
    m_searchQueue.clear();
    m_searchQueue.push_back( m_currentDirectory );
    m_searchFound = 0;
    m_searching = true;
    ShowEntries();
    PollSearch();
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::QueueSearchDirectory( const CString& directory )
{GUCEF_TRACE;

    CORE::IFileSystemAccess* access = GetCurrentAccess();
    if ( GUCEF_NULL == access )
        return false;
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    if ( CORE::FILESYSTEMACCESS_SUCCESS != access->TryGetFileSystemIteratorAsync( directory, future, m_options.threadPoolName ) )
        return false;
    m_searchTasks.push_back( SSearchTask( directory, future ) );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::PollSearch( void )
{GUCEF_TRACE;

    if ( !m_searching )
        return;

    // a pool without worker threads (a build without threads, as in a browser without pthreads) only does a task when another
    // thread pitches in: here the GUI thread reads a folder per frame
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool(
            m_options.threadPoolName.IsNULLOrEmpty() ? CORE::CTaskManager::DefaultThreadPoolName : m_options.threadPoolName );
    if ( !pool.IsNULL() && 0 == pool->GetActiveNrOfThreads() )
        pool->CarryOutQueuedTasksIfAny( 1 );

    // read the folders that are done
    bool found = false;
    CString prefix = m_currentDirectory;
    if ( !prefix.IsNULLOrEmpty() && !IsSeparator( prefix[ prefix.Length()-1 ] ) )
        prefix += "/";
    for ( UInt32 t=0; t<(UInt32)m_searchTasks.size(); )
    {
        SSearchTask& task = m_searchTasks[ t ];
        if ( !task.future.IsReady() )
        {
            ++t;
            continue;
        }
        CORE::TFileSystemIteratorPtr listing;
        if ( CORE::IFileSystemAccess::TryGetAsyncIterator( task.future, listing ) && listing->FindFirst( task.directory ) )
        {
            do
            {
                CString name = listing->GetResourceName();
                CString path = listing->GetResourcePath();
                if ( path.IsNULLOrEmpty() )
                    path = JoinPath( task.directory, name );
                bool hidden = name.Length() > 0 && '.' == name[ 0 ];
                if ( hidden && !m_options.showHidden )
                    continue;
                if ( listing->IsADirectory() )
                    m_searchQueue.push_back( path );
                CString below = path.StartsWith( prefix ) ? path.CutChars( prefix.Length(), true ) : name;
                if ( m_search.IsMatch( name ) || m_search.IsMatch( below ) )
                {
                    SEntry entry;
                    entry.name = below;
                    entry.path = path;
                    entry.isDirectory = listing->IsADirectory();
                    CORE::CResourceMetaData metaData;
                    const bool hasMetaData = listing->TryReadMetaData( metaData );
                    entry.hasSize = hasMetaData && metaData.hasResourceSizeInBytes && !entry.isDirectory;
                    entry.size = entry.hasSize ? metaData.resourceSizeInBytes : 0;
                    entry.hasModified = hasMetaData && metaData.hasModifiedDateTime;
                    if ( entry.hasModified )
                        entry.modified = metaData.modifiedDateTime;
                    entry.hidden = hidden;
                    m_entries.push_back( entry );
                    ++m_searchFound;
                    found = true;
                }
            }
            while ( listing->FindNext() && m_searchFound < MaxSearchResults );
        }
        m_searchTasks.erase( m_searchTasks.begin() + t );
    }

    // more folders to read, a few at the same time
    while ( m_searchTasks.size() < MaxConcurrentSearchListings && !m_searchQueue.empty() && m_searchFound < MaxSearchResults )
    {
        CString directory = m_searchQueue.front();
        m_searchQueue.erase( m_searchQueue.begin() );
        QueueSearchDirectory( directory );
    }
    if ( m_searchTasks.empty() && ( m_searchQueue.empty() || m_searchFound >= MaxSearchResults ) )
    {
        m_searching = false;
        m_searchQueue.clear();
        found = true;
    }
    if ( found )
    {
        SortEntries();
        ShowEntries();
    }
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::StopSearch( void )
{GUCEF_TRACE;

    for ( UInt32 t=0; t<(UInt32)m_searchTasks.size(); ++t )
        m_searchTasks[ t ].future.RequestCancellation();
    m_searchTasks.clear();
    m_searchQueue.clear();
    if ( m_searching )
    {
        m_searching = false;
        UpdateStatus();
    }
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::IsSearching( void ) const
{GUCEF_TRACE;

    return m_searching;
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CFileSystemDialogForm::GetListedNames( void ) const
{GUCEF_TRACE;

    CString::StringVector names;
    for ( UInt32 i=0; i<(UInt32)m_listed.size(); ++i )
    {
        const SEntry& entry = m_entries[ m_listed[ i ] ];
        names.push_back( entry.isDirectory ? entry.name + "/" : entry.name );
    }
    return names;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::SelectNames( const CString::StringVector& names )
{GUCEF_TRACE;

    CGridView* grid = GetWidgetAs< CGridView >( "Entries" );
    if ( GUCEF_NULL == grid )
        return false;
    CGridView::TInt32Vector rows;
    for ( UInt32 i=0; i<(UInt32)m_listed.size(); ++i )
    {
        const SEntry& entry = m_entries[ m_listed[ i ] ];
        for ( UInt32 n=0; n<(UInt32)names.size(); ++n )
            if ( names[ n ] == entry.name || names[ n ] == entry.name + "/" )
                rows.push_back( (Int32) i );
    }
    if ( rows.size() != names.size() )
        return false;
    if ( !grid->GetAllowMultiSelect() && rows.size() > 1 )
        return false;
    grid->SetSelectedRows( rows );
    if ( !rows.empty() )
        grid->ScrollToRow( rows.front() );
    OnSelectionChanged();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::SetName( const CString& name )
{GUCEF_TRACE;

    CWidget* field = GetWidget( "Name" );
    if ( GUCEF_NULL != field )
    {
        m_updatingWidgets = true;
        field->SetProperty( EWidgetProperty_Text, name );
        m_updatingWidgets = false;
    }
}

/*-------------------------------------------------------------------------*/

CString
CFileSystemDialogForm::GetName( void )
{GUCEF_TRACE;

    CWidget* field = GetWidget( "Name" );
    return GUCEF_NULL != field ? field->GetProperty( EWidgetProperty_Text ) : CString::Empty;
}

/*-------------------------------------------------------------------------*/

CMessageBoxForm&
CFileSystemDialogForm::GetMessageBox( void )
{GUCEF_TRACE;

    return m_messageBox;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnSelectionChanged( void )
{GUCEF_TRACE;

    CGridView* grid = GetWidgetAs< CGridView >( "Entries" );
    if ( GUCEF_NULL == grid )
        return;

    // a chosen file goes in the name field of the file dialogs
    Int32 row = grid->GetSelectedRow();
    if ( IsFileMode() && row >= 0 && row < (Int32) m_listed.size() )
    {
        const SEntry& entry = m_entries[ m_listed[ row ] ];
        if ( !entry.isDirectory )
        {
            CGridView::TInt32Vector rows = grid->GetSelectedRows();
            if ( rows.size() > 1 )
                SetName( CString::Empty );
            else
                SetName( entry.name );
        }
    }
    NotifyObservers( SelectionChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::Enter( const SEntry& entry )
{GUCEF_TRACE;

    if ( entry.isDirectory )
    {
        GoTo( m_currentRoot, entry.path, true );
        return;
    }
    if ( IsFileMode() )
    {
        SetName( entry.name );
        Accept();
    }
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnRowsActivated( void )
{GUCEF_TRACE;

    CGridView* grid = GetWidgetAs< CGridView >( "Entries" );
    Int32 row = GUCEF_NULL != grid ? grid->GetSelectedRow() : -1;
    if ( row >= 0 && row < (Int32) m_listed.size() )
        Enter( m_entries[ m_listed[ row ] ] );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnNameEntered( void )
{GUCEF_TRACE;

    CString name = GetName();
    if ( CORE::CGlobPattern::HasGlobCharacters( name ) )
    {
        // a glob in the name field is a search, as in the dialogs of Windows and Qt
        CWidget* subfolders = GetWidget( "SearchSubfolders" );
        SetName( CString::Empty );
        SetSearch( name, GUCEF_NULL != subfolders && subfolders->GetBoolProperty( EWidgetProperty_Checked ) );
        return;
    }
    Accept();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnPathEntered( void )
{GUCEF_TRACE;

    CWidget* field = GetWidget( "Path" );
    if ( GUCEF_NULL == field )
        return;
    CString path = field->GetProperty( EWidgetProperty_Text );
    if ( path.IsNULLOrEmpty() )
        return;

    // the current root first, then the others
    for ( UInt32 n=0; n<(UInt32)m_roots.size(); ++n )
    {
        UInt32 root = 0 == n ? m_currentRoot : ( n <= m_currentRoot ? n - 1 : n );
        CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
        if ( CORE::FILESYSTEMACCESS_SUCCESS != m_roots[ root ].access->TryGetEntryType( path, type ) )
            continue;
        if ( CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type )
        {
            GoTo( root, path, true );
            return;
        }
        if ( CORE::FILESYSTEMENTRYTYPE_FILE == type )
        {
            GoTo( root, ParentOf( path ), true );
            CString::StringVector names;
            names.push_back( NameOf( path ) );
            SelectNames( names );
            SetName( NameOf( path ) );
            return;
        }
    }
    Tell( "There is no folder or file \"" + path + "\"." );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnPlaceChosen( Int32 index )
{GUCEF_TRACE;

    if ( index < 0 || index >= (Int32) m_places.size() )
        return;
    GoTo( m_places[ index ].root, m_places[ index ].directory, true );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnContextMenu( Int32 x, Int32 y )
{GUCEF_TRACE;

    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == context )
        return;
    if ( GUCEF_NULL == m_contextMenu )
    {
        m_contextMenu = static_cast< CPopupMenu* >( context->CreateWidget( "PopupMenu", "FileSystemDialog#Menu", GUCEF_NULL ) );
        if ( GUCEF_NULL == m_contextMenu )
            return;
        TEventCallback itemCallback( this, &CFileSystemDialogForm::OnContextMenuItemActivated );
        SubscribeTo( m_contextMenu, CWidget::ItemActivatedEvent, itemCallback );
    }
    CPopupMenu::TStringVector items;
    CORE::IFileSystemAccess* access = GetCurrentAccess();
    const bool writeable = !m_options.readOnly && GUCEF_NULL != access && access->IsWriteable( m_currentDirectory );
    if ( writeable && m_options.allowCreateDirectory )
        items.push_back( MenuNewFolder );
    CGridView* grid = GetWidgetAs< CGridView >( "Entries" );
    if ( writeable && GUCEF_NULL != grid && grid->GetSelectedRow() >= 0 )
    {
        if ( m_options.allowRename )
            items.push_back( MenuRename );
        if ( m_options.allowDelete )
            items.push_back( MenuDelete );
    }
    if ( !items.empty() )
        items.push_back( "-" );
    items.push_back( MenuRefresh );
    m_contextMenu->SetItems( items );
    m_contextMenu->ShowAt( x, y );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnContextMenuItem( const CString& item )
{GUCEF_TRACE;

    CGridView* grid = GetWidgetAs< CGridView >( "Entries" );
    Int32 row = GUCEF_NULL != grid ? grid->GetSelectedRow() : -1;
    if ( MenuNewFolder == item )
    {
        CString name = "New folder";
        Ask( QUESTION_NEW_FOLDER, "The name of the new folder:", &name );
    }
    else
    if ( MenuRename == item && row >= 0 && row < (Int32) m_listed.size() )
    {
        const SEntry& entry = m_entries[ m_listed[ row ] ];
        m_pendingPath = entry.path;
        CString name = NameOf( entry.path );
        Ask( QUESTION_RENAME, "The new name of \"" + name + "\":", &name );
    }
    else
    if ( MenuDelete == item && row >= 0 && row < (Int32) m_listed.size() )
    {
        const SEntry& entry = m_entries[ m_listed[ row ] ];
        m_pendingPath = entry.path;
        Ask( QUESTION_DELETE, "Delete \"" + NameOf( entry.path ) + "\"" + ( entry.isDirectory ? " and everything in it?" : "?" ), GUCEF_NULL );
    }
    else
    if ( MenuRefresh == item )
        Refresh();
}

/*-------------------------------------------------------------------------*/

CString
CFileSystemDialogForm::DescribeResult( CORE::TFileSystemAccessResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case CORE::FILESYSTEMACCESS_SUCCESS:                return "Done.";
        case CORE::FILESYSTEMACCESS_PARTIAL_SUCCESS:        return "Only some of it could be done.";
        case CORE::FILESYSTEMACCESS_NOT_FOUND:              return "It is not there (any more).";
        case CORE::FILESYSTEMACCESS_PARENT_NOT_FOUND:       return "The folder it should be in is not there.";
        case CORE::FILESYSTEMACCESS_ALREADY_EXISTS:         return "There already is something with that name.";
        case CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY:        return "That is not a folder.";
        case CORE::FILESYSTEMACCESS_NOT_A_FILE:             return "That is not a file.";
        case CORE::FILESYSTEMACCESS_DIRECTORY_NOT_EMPTY:    return "The folder is not empty.";
        case CORE::FILESYSTEMACCESS_ACCESS_DENIED:          return "You do not have the rights to do that here.";
        case CORE::FILESYSTEMACCESS_READ_ONLY:              return "This place cannot be changed.";
        case CORE::FILESYSTEMACCESS_INVALID_PATH:           return "That is not a usable path.";
        case CORE::FILESYSTEMACCESS_INVALID_NAME:           return "That is not a usable name.";
        case CORE::FILESYSTEMACCESS_SAME_PATH:              return "That is where it is already.";
        case CORE::FILESYSTEMACCESS_CROSS_DEVICE:           return "It cannot be moved to another drive this way.";
        case CORE::FILESYSTEMACCESS_IN_USE:                 return "It is in use.";
        case CORE::FILESYSTEMACCESS_NO_SPACE:               return "There is no room left.";
        case CORE::FILESYSTEMACCESS_TOO_LONG:               return "The name or path is too long.";
        case CORE::FILESYSTEMACCESS_NOT_SUPPORTED:          return "This place cannot do that.";
        case CORE::FILESYSTEMACCESS_QUEUEING_FAILED:        return "It could not be started.";
        case CORE::FILESYSTEMACCESS_CANCELLED:              return "It was stopped.";
        case CORE::FILESYSTEMACCESS_TIMEOUT:                return "It took too long.";
        default:                                            return "It could not be done.";
    }
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::Tell( const CString& text )
{GUCEF_TRACE;

    Ask( QUESTION_INFO, text, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::Ask( EPendingQuestion question, const CString& text, const CString* input )
{GUCEF_TRACE;

    m_pendingQuestion = question;
    if ( GUCEF_NULL != input )
        m_messageBox.OpenInput( m_options.title, text, *input, CMessageBoxForm::MESSAGEBOXBUTTONS_OK_CANCEL );
    else
    if ( QUESTION_INFO == question )
        m_messageBox.Open( m_options.title, text, CMessageBoxForm::MESSAGEBOXBUTTONS_OK );
    else
        m_messageBox.Open( m_options.title, text, CMessageBoxForm::MESSAGEBOXBUTTONS_YES_NO );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnAnswer( CMessageBoxForm::EMessageBoxResult result )
{GUCEF_TRACE;

    EPendingQuestion question = m_pendingQuestion;
    m_pendingQuestion = QUESTION_NONE;
    const bool yes = CMessageBoxForm::MESSAGEBOXRESULT_YES == result || CMessageBoxForm::MESSAGEBOXRESULT_OK == result;
    CORE::IFileSystemAccess* access = GetCurrentAccess();
    switch ( question )
    {
        case QUESTION_OVERWRITE:
        {
            if ( yes )
                Finish( m_pendingSelection );
            m_pendingSelection.clear();
            break;
        }
        case QUESTION_NEW_FOLDER:
        {
            CString name = m_messageBox.GetInputText();
            if ( !yes || GUCEF_NULL == access || name.IsNULLOrEmpty() )
                break;
            CORE::TFileSystemAccessResult made = access->TryCreateDirectory( JoinPath( m_currentDirectory, name ), false );
            if ( CORE::FILESYSTEMACCESS_SUCCESS != made )
            {
                Tell( DescribeResult( made ) );
                break;
            }
            ReadDirectory();
            CString::StringVector names;
            names.push_back( name );
            SelectNames( names );
            break;
        }
        case QUESTION_RENAME:
        {
            CString name = m_messageBox.GetInputText();
            if ( !yes || GUCEF_NULL == access || name.IsNULLOrEmpty() || name == NameOf( m_pendingPath ) )
                break;
            CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
            access->TryGetEntryType( m_pendingPath, type );
            CORE::TFileSystemAccessResult renamed = CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type ?
                access->TryMoveDirectory( m_pendingPath, JoinPath( ParentOf( m_pendingPath ), name ) ) :
                access->TryRenameFile( m_pendingPath, name );
            if ( CORE::FILESYSTEMACCESS_SUCCESS != renamed )
            {
                Tell( DescribeResult( renamed ) );
                break;
            }
            ReadDirectory();
            CString::StringVector names;
            names.push_back( name );
            SelectNames( names );
            break;
        }
        case QUESTION_DELETE:
        {
            if ( !yes || GUCEF_NULL == access )
                break;
            CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
            access->TryGetEntryType( m_pendingPath, type );
            CORE::TFileSystemAccessResult deleted = CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type ?
                access->TryDeleteDirectory( m_pendingPath, true ) : access->TryDeleteFile( m_pendingPath );
            if ( CORE::FILESYSTEMACCESS_SUCCESS != deleted )
                Tell( DescribeResult( deleted ) );
            ReadDirectory();
            break;
        }
        default:
            break;
    }
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::ValidateAccept( const TSelectionVector& selection, CString& problem )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::CollectSelection( TSelectionVector& selection, CString& problem, bool& askOverwrite )
{GUCEF_TRACE;

    selection.clear();
    askOverwrite = false;
    CORE::IFileSystemAccess* access = GetCurrentAccess();
    if ( GUCEF_NULL == access )
        return false;

    SSelection chosen;
    chosen.rootName = m_roots[ m_currentRoot ].name;
    chosen.access = m_roots[ m_currentRoot ].access;
    chosen.isDirectory = false;

    CGridView* grid = GetWidgetAs< CGridView >( "Entries" );
    CGridView::TInt32Vector rows;
    if ( GUCEF_NULL != grid )
        rows = grid->GetSelectedRows();
    const CString name = GetName();

    if ( MODE_OPEN_FILE == m_mode )
    {
        // the files chosen in the list, else the name; a folder is opened in the dialog
        for ( UInt32 i=0; i<(UInt32)rows.size(); ++i )
        {
            if ( rows[ i ] < 0 || rows[ i ] >= (Int32) m_listed.size() )
                continue;
            const SEntry& entry = m_entries[ m_listed[ rows[ i ] ] ];
            if ( entry.isDirectory )
                continue;
            chosen.path = entry.path;
            selection.push_back( chosen );
        }
        if ( !selection.empty() && ( name.IsNULLOrEmpty() || rows.size() > 1 || NameOf( selection.front().path ) == name ) )
            return true;
        selection.clear();
        if ( name.IsNULLOrEmpty() )
        {
            Int32 row = GUCEF_NULL != grid ? grid->GetSelectedRow() : -1;
            if ( row >= 0 && row < (Int32) m_listed.size() && m_entries[ m_listed[ row ] ].isDirectory )
            {
                GoTo( m_currentRoot, m_entries[ m_listed[ row ] ].path, true );
                return false;
            }
            problem = "Choose a file.";
            return false;
        }
        const CString path = ResolvePath( name );
        CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
        access->TryGetEntryType( path, type );
        if ( CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type )
        {
            SetName( CString::Empty );
            GoTo( m_currentRoot, path, true );
            return false;
        }
        if ( CORE::FILESYSTEMENTRYTYPE_NONE == type && m_options.mustExist )
        {
            problem = "There is no file \"" + name + "\".";
            return false;
        }
        chosen.path = path;
        selection.push_back( chosen );
        return true;
    }

    if ( MODE_SAVE_FILE == m_mode )
    {
        if ( name.IsNULLOrEmpty() )
        {
            problem = "Type a name for the file.";
            return false;
        }
        CString path = ResolvePath( name );
        CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
        access->TryGetEntryType( path, type );
        if ( CORE::FILESYSTEMENTRYTYPE_DIRECTORY == type )
        {
            SetName( CString::Empty );
            GoTo( m_currentRoot, path, true );
            return false;
        }

        // the extension of the chosen type when the name has none
        const TFileType* fileType = m_selectedFileType < (UInt32) m_fileTypes.size() ? &m_fileTypes[ m_selectedFileType ] : GUCEF_NULL;
        if ( ExtensionOf( NameOf( path ) ).IsNULLOrEmpty() && GUCEF_NULL != fileType && !fileType->defaultExtension.IsNULLOrEmpty() )
        {
            path += "." + fileType->defaultExtension;
            type = CORE::FILESYSTEMENTRYTYPE_NONE;
            access->TryGetEntryType( path, type );
        }
        if ( m_options.strictFileTypes && GUCEF_NULL != fileType && !FitsFileType( NameOf( path ) ) )
        {
            CString patterns;
            for ( UInt32 p=0; p<(UInt32)fileType->patterns.size(); ++p )
                patterns += ( p > 0 ? ", " : "" ) + fileType->patterns[ p ];
            problem = "The name must fit " + patterns + ".";
            return false;
        }
        if ( m_options.readOnly || !access->IsWriteable( ParentOf( path ).IsNULLOrEmpty() ? m_currentDirectory : ParentOf( path ) ) )
        {
            problem = DescribeResult( CORE::FILESYSTEMACCESS_READ_ONLY );
            return false;
        }
        askOverwrite = CORE::FILESYSTEMENTRYTYPE_FILE == type && m_options.confirmOverwrite;
        chosen.path = path;
        selection.push_back( chosen );
        return true;
    }

    if ( MODE_OPEN_DIRECTORY == m_mode )
    {
        chosen.isDirectory = true;
        for ( UInt32 i=0; i<(UInt32)rows.size(); ++i )
        {
            if ( rows[ i ] < 0 || rows[ i ] >= (Int32) m_listed.size() )
                continue;
            const SEntry& entry = m_entries[ m_listed[ rows[ i ] ] ];
            if ( !entry.isDirectory )
                continue;
            chosen.path = entry.path;
            selection.push_back( chosen );
        }
        if ( !selection.empty() && name.IsNULLOrEmpty() )
            return true;
        selection.clear();
        const CString path = name.IsNULLOrEmpty() ? m_currentDirectory : ResolvePath( name );
        CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
        access->TryGetEntryType( path, type );
        if ( CORE::FILESYSTEMENTRYTYPE_DIRECTORY != type && m_options.mustExist )
        {
            problem = "There is no folder \"" + ( name.IsNULLOrEmpty() ? path : name ) + "\".";
            return false;
        }
        chosen.path = path;
        selection.push_back( chosen );
        return true;
    }

    // MODE_SAVE_DIRECTORY: a folder to save into, made when it is not there
    chosen.isDirectory = true;
    CString path = m_currentDirectory;
    if ( !name.IsNULLOrEmpty() )
        path = ResolvePath( name );
    else
    if ( 1 == rows.size() && rows[ 0 ] >= 0 && rows[ 0 ] < (Int32) m_listed.size() && m_entries[ m_listed[ rows[ 0 ] ] ].isDirectory )
        path = m_entries[ m_listed[ rows[ 0 ] ] ].path;
    CORE::TFileSystemEntryType type = CORE::FILESYSTEMENTRYTYPE_NONE;
    access->TryGetEntryType( path, type );
    if ( CORE::FILESYSTEMENTRYTYPE_FILE == type )
    {
        problem = DescribeResult( CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY );
        return false;
    }
    if ( m_options.readOnly || !access->IsWriteable( path ) )
    {
        problem = DescribeResult( CORE::FILESYSTEMACCESS_READ_ONLY );
        return false;
    }
    if ( CORE::FILESYSTEMENTRYTYPE_NONE == type )
    {
        CORE::TFileSystemAccessResult made = access->TryCreateDirectory( path, true );
        if ( CORE::FILESYSTEMACCESS_SUCCESS != made )
        {
            problem = DescribeResult( made );
            return false;
        }
    }
    for ( UInt32 i=0; i<(UInt32)m_options.filesToSave.size() && m_options.confirmOverwrite; ++i )
    {
        CORE::TFileSystemEntryType there = CORE::FILESYSTEMENTRYTYPE_NONE;
        if ( CORE::FILESYSTEMACCESS_SUCCESS == access->TryGetEntryType( JoinPath( path, m_options.filesToSave[ i ] ), there ) && CORE::FILESYSTEMENTRYTYPE_NONE != there )
            askOverwrite = true;
    }
    chosen.path = path;
    selection.push_back( chosen );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::Accept( void )
{GUCEF_TRACE;

    TSelectionVector selection;
    CString problem;
    bool askOverwrite = false;
    if ( !CollectSelection( selection, problem, askOverwrite ) )
    {
        if ( !problem.IsNULLOrEmpty() )
            Tell( problem );
        return false;
    }
    if ( !ValidateAccept( selection, problem ) )
    {
        Tell( problem.IsNULLOrEmpty() ? CString( "That cannot be chosen here." ) : problem );
        return false;
    }
    if ( askOverwrite )
    {
        m_pendingSelection = selection;
        if ( MODE_SAVE_DIRECTORY == m_mode )
            Ask( QUESTION_OVERWRITE, "Some of the files are there already. Replace them?", GUCEF_NULL );
        else
            Ask( QUESTION_OVERWRITE, "\"" + NameOf( selection.front().path ) + "\" is there already. Replace it?", GUCEF_NULL );
        return false;
    }
    Finish( selection );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::AcceptCurrentDirectory( void )
{GUCEF_TRACE;

    CORE::IFileSystemAccess* access = GetCurrentAccess();
    if ( GUCEF_NULL == access )
        return false;
    SSelection chosen;
    chosen.rootName = m_roots[ m_currentRoot ].name;
    chosen.access = m_roots[ m_currentRoot ].access;
    chosen.isDirectory = true;
    chosen.path = m_currentDirectory;
    CGridView* grid = GetWidgetAs< CGridView >( "Entries" );
    Int32 row = GUCEF_NULL != grid ? grid->GetSelectedRow() : -1;
    if ( row >= 0 && row < (Int32) m_listed.size() && m_entries[ m_listed[ row ] ].isDirectory )
        chosen.path = m_entries[ m_listed[ row ] ].path;
    TSelectionVector selection;
    selection.push_back( chosen );
    CString problem;
    if ( !ValidateAccept( selection, problem ) )
    {
        Tell( problem.IsNULLOrEmpty() ? CString( "That cannot be chosen here." ) : problem );
        return false;
    }
    Finish( selection );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::Finish( const TSelectionVector& selection )
{GUCEF_TRACE;

    m_selection = selection;
    Close();
    NotifyObservers( AcceptedEvent );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::Cancel( void )
{GUCEF_TRACE;

    m_selection.clear();
    Close();
    NotifyObservers( CancelledEvent );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::Close( void )
{GUCEF_TRACE;

    StopSearch();
    m_open = false;
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != m_contextMenu && GUCEF_NULL != context && context->GetPopup() == m_contextMenu )
        context->ClosePopup();
    Hide();
}

/*-------------------------------------------------------------------------*/

const CFileSystemDialogForm::TSelectionVector&
CFileSystemDialogForm::GetSelection( void ) const
{GUCEF_TRACE;

    return m_selection;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDialogForm::IsListening( void ) const
{GUCEF_TRACE;

    return m_open && !m_updatingWidgets;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnMessageBoxClosed( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    OnAnswer( m_messageBox.GetResult() );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_searching )
        PollSearch();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnBackActivated( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_open )
        GoBack();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnForwardActivated( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_open )
        GoForward();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnUpActivated( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_open )
        GoUp();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnNewFolderActivated( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_open )
        OnContextMenuItem( MenuNewFolder );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnOpenFolderActivated( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_open )
        AcceptCurrentDirectory();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnAcceptActivated( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_open )
        Accept();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnCancelActivated( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_open )
        Cancel();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnContextMenuItemActivated( CORE::CNotifier* notifier    ,
                                                   const CORE::CEvent& eventId  ,
                                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    CItemEventData* item = static_cast< CItemEventData* >( eventData );
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && context->GetPopup() == m_contextMenu )
        context->ClosePopup();
    OnContextMenuItem( item->item );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnEntriesItemActivated( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_open )
        return;
    CRoutedEventData* routed = static_cast< CRoutedEventData* >( eventData );
    if ( GUCEF_NULL == routed || EVENTPHASE_TUNNEL != routed->phase )
        OnRowsActivated();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnEntriesMouseClick( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_open || GUCEF_NULL == eventData )
        return;
    CPointerEventData* pointer = static_cast< CPointerEventData* >( eventData );
    if ( MOUSEBUTTON_RIGHT == pointer->button && EVENTPHASE_TUNNEL != pointer->phase )
        OnContextMenu( pointer->x, pointer->y );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnEntriesSelectionChanged( CORE::CNotifier* notifier    ,
                                                  const CORE::CEvent& eventId  ,
                                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsListening() )
        OnSelectionChanged();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnEntriesColumnClicked( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !IsListening() || GUCEF_NULL == eventData )
        return;
    CGridView::TColumnClickedEventData* column = static_cast< CGridView::TColumnClickedEventData* >( eventData );
    Int32 clicked = (Int32) column->GetData().column;
    m_sortAscending = clicked == m_sortColumn ? !m_sortAscending : true;
    m_sortColumn = clicked;
    SortEntries();
    ShowEntries();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnPathTextEntered( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsListening() )
        OnPathEntered();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnNameTextEntered( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsListening() )
        OnNameEntered();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnSearchTextChanged( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !IsListening() )
        return;
    CWidget* search = GetWidget( "Search" );
    CWidget* subfolders = GetWidget( "SearchSubfolders" );
    if ( GUCEF_NULL != search )
        SetSearch( search->GetProperty( EWidgetProperty_Text ), GUCEF_NULL != subfolders && subfolders->GetBoolProperty( EWidgetProperty_Checked ) );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::ApplySearchSubfolders( bool includeSubfolders )
{GUCEF_TRACE;

    CWidget* search = GetWidget( "Search" );
    SetSearch( GUCEF_NULL != search ? search->GetProperty( EWidgetProperty_Text ) : CString::Empty, includeSubfolders );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnSearchSubfoldersChecked( CORE::CNotifier* notifier    ,
                                                  const CORE::CEvent& eventId  ,
                                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsListening() )
        ApplySearchSubfolders( true );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnSearchSubfoldersUnchecked( CORE::CNotifier* notifier    ,
                                                    const CORE::CEvent& eventId  ,
                                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsListening() )
        ApplySearchSubfolders( false );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::ApplyShowHidden( bool showHidden )
{GUCEF_TRACE;

    m_options.showHidden = showHidden;
    ShowEntries();
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnShowHiddenChecked( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsListening() )
        ApplyShowHidden( true );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnShowHiddenUnchecked( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsListening() )
        ApplyShowHidden( false );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnRootsSelectionChanged( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CWidget* roots = GetWidget( "Roots" );
    if ( !IsListening() || GUCEF_NULL == roots )
        return;
    Int32 index = roots->GetIntProperty( EWidgetProperty_SelectedIndex );
    if ( index >= 0 && index < (Int32) m_roots.size() && (UInt32) index != m_currentRoot )
        GoTo( (UInt32) index, m_roots[ index ].initialDirectory, true );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnPlacesSelectionChanged( CORE::CNotifier* notifier    ,
                                                 const CORE::CEvent& eventId  ,
                                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CWidget* places = GetWidget( "Places" );
    if ( IsListening() && GUCEF_NULL != places )
        OnPlaceChosen( places->GetIntProperty( EWidgetProperty_SelectedIndex ) );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnFileTypesValueChanged( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CWidget* types = GetWidget( "FileTypes" );
    if ( !IsListening() || GUCEF_NULL == types )
        return;
    Int32 selected = types->GetIntProperty( EWidgetProperty_SelectedIndex );
    if ( selected >= 0 && (UInt32) selected != m_selectedFileType )
        SetSelectedFileType( (UInt32) selected );
}

/*-------------------------------------------------------------------------*/

void
CFileSystemDialogForm::OnNavigation( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !IsListening() || GUCEF_NULL == eventData )
        return;
    CNavigationEventData* navigation = static_cast< CNavigationEventData* >( eventData );
    if ( NAVIGATION_CANCEL == navigation->direction && EVENTPHASE_TUNNEL != navigation->phase && !m_messageBox.IsOpen() )
    {
        navigation->handled = true;
        if ( m_searching )
            StopSearch();
        else
            Cancel();
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CFileSystemDialogForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CFileSystemDialogForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
