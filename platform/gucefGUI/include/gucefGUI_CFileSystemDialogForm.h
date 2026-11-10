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

#ifndef GUCEF_GUI_CFILESYSTEMDIALOGFORM_H
#define GUCEF_GUI_CFILESYSTEMDIALOGFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_IFILESYSTEMACCESS_H
#include "gucefCORE_IFileSystemAccess.h"
#define GUCEF_CORE_IFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_IFILESYSTEMACCESS_H ? */

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CMESSAGEBOXFORM_H
#include "gucefGUI_CMessageBoxForm.h"
#define GUCEF_GUI_CMESSAGEBOXFORM_H
#endif /* GUCEF_GUI_CMESSAGEBOXFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CGridView;
class CPopupMenu;

/**
 *  The base of the file system dialogs: CFileOpenDialogForm, CFileSaveDialogForm, CDirectoryOpenDialogForm and
 *  CDirectorySaveDialogForm. Each of those is named by what the user is asked to do and fixes the mode, so it looks the part:
 *  the dialogs differ in what they list, what the accept button says and what is checked before they accept.
 *
 *  What they share:
 *  - roots: file systems to browse (AddRoot(), an IFileSystemAccess each), in a pane on the left; app bookmarks (AddPlace())
 *    and, when switched on, the special directories of the roots (home, documents, ...) in a pane below
 *  - navigation: back, forward, up, a path to type, a details list (name, size, type, modified) sortable by its columns,
 *    keyboard navigation and type to find
 *  - the user's search: a glob (CGlobPattern) that filters the folder, or with "Search subfolders" every folder below it,
 *    read on the threads of a pool (IFileSystemAccess::TryGetFileSystemIteratorAsync); a glob typed in the name field does
 *    the same
 *  - the app's file types (file dialogs): a list of descriptions with patterns; a file shows when it fits the type and the
 *    search
 *  - making, renaming and deleting folders and files where allowed, choices of the app (check boxes and lists) next to the
 *    buttons
 *
 *  Open() shows the form modal; it closes on accept or cancel and notifies AcceptedEvent or CancelledEvent, after which
 *  GetSelection() gives what was chosen. The layout is built in code and made for BuildDarkTheme() in a context at the
 *  resolution of the window.
 */
class GUCEF_GUI_PUBLIC_CPP CFileSystemDialogForm : public CFormEx
{
    public:

    static const CORE::CEvent AcceptedEvent;
    static const CORE::CEvent CancelledEvent;
    static const CORE::CEvent SelectionChangedEvent;
    static const CORE::CEvent DirectoryChangedEvent;
    static const CORE::CEvent FileTypeChangedEvent;
    static const CORE::CEvent SearchChangedEvent;

    static void RegisterEvents( void );

    /** What the dialog asks for, fixed by the derived form */
    enum EMode
    {
        MODE_OPEN_FILE = 0,
        MODE_SAVE_FILE,
        MODE_OPEN_DIRECTORY,
        MODE_SAVE_DIRECTORY
    };

    /** A kind of file the app offers, e.g. "Game disc" with "*.cue" and "*.iso" */
    struct GUCEF_GUI_PUBLIC_CPP SFileType
    {
        CString description;
        CString::StringVector patterns;     /**< globs; empty or "*" for every file */
        CString defaultExtension;           /**< without a dot; the save dialog adds it to a name without one */
    };
    typedef struct SFileType TFileType;
    typedef GUCEF::vector< TFileType > TFileTypeVector;

    /** A file or folder the user chose */
    struct GUCEF_GUI_PUBLIC_CPP SSelection
    {
        CString rootName;
        CORE::TFileSystemAccessPtr access;
        CString path;                       /**< a path of that file system */
        bool isDirectory;
    };
    typedef struct SSelection TSelection;
    typedef GUCEF::vector< TSelection > TSelectionVector;

    /** The options; the forms set the defaults that suit them */
    struct GUCEF_GUI_PUBLIC_CPP SOptions
    {
        CString title;
        CString acceptLabel;
        CString initialRoot;                    /**< display name of a root; empty for the first one */
        CString initialDirectory;               /**< in that root; empty for its own initial directory */
        CString initialName;
        bool multiSelect;                       /**< open dialogs */
        bool mustExist;                         /**< open dialogs: what is chosen must be there */
        bool confirmOverwrite;                  /**< save dialogs: ask before an existing file is replaced */
        bool strictFileTypes;                   /**< save file: only the extensions of the file types */
        bool includeAllFilesType;               /**< file dialogs: "All files (*)" after the app's types */
        bool acceptDirectoriesToo;              /**< open file: an "Open folder" button accepts a folder too */
        bool showFilesForContext;               /**< directory dialogs: list files as well (they cannot be chosen) */
        bool showHidden;
        bool allowCreateDirectory;
        bool allowRename;
        bool allowDelete;
        bool readOnly;                          /**< nothing is changed: no new folder, rename or delete */
        bool showSpecialDirs;                   /**< the special directories of the roots in the places pane */
        CORE::TSpecialDirSet specialDirsToShow; /**< which of them; empty for the usual ones (home, desktop, documents, ...) */
        CString::StringVector filesToSave;      /**< directory save: names that will be written there, clashes are marked */
        CString threadPoolName;                 /**< the pool the search through the subfolders reads on; empty for the default pool */

        SOptions( void );
    };
    typedef struct SOptions TOptions;

    virtual ~CFileSystemDialogForm();

    EMode GetMode( void ) const;

    TOptions& GetOptions( void );

    const TOptions& GetOptions( void ) const;

    /** A file system to browse; the first root added is where the dialog starts unless the options name another */
    bool AddRoot( const CString& displayName, CORE::TFileSystemAccessPtr access, const CString& initialDirectory = CString::Empty );

    /** A bookmark of the app in the places pane */
    bool AddPlace( const CString& displayName, const CString& rootName, const CString& directory );

    /** A label for a special directory in the places pane, instead of the standard one */
    void SetSpecialDirLabel( CORE::TSpecialDirs specialDir, const CString& label );

    static CString GetStandardSpecialDirLabel( CORE::TSpecialDirs specialDir );

    /** File dialogs: the kinds of files offered, in order */
    void AddFileType( const CString& description, const CString::StringVector& patterns, const CString& defaultExtension = CString::Empty );

    void SetSelectedFileType( UInt32 index );

    UInt32 GetSelectedFileType( void ) const;

    const TFileTypeVector& GetFileTypes( void ) const;

    /** A choice of the app next to the buttons: a check box without options, a list with them */
    void AddChoice( const CString& id, const CString& label, const CString::StringVector& options = CString::EmptyStringVector );

    /** "true" / "false" for a check box, the option for a list */
    CString GetChoice( const CString& id );

    void SetChoice( const CString& id, const CString& value );

    /** Makes the window; Open() does this when it was not done */
    bool Build( void );

    /** Shows the dialog in its initial root and directory, modal */
    bool Open( void );

    bool IsOpen( void ) const;

    /** Accepts as the accept button does: false when the choice does not fit (a message tells the user) */
    bool Accept( void );

    /** The folder as the "Open folder" button of the file open dialog does */
    bool AcceptCurrentDirectory( void );

    void Cancel( void );

    /** What was chosen, after AcceptedEvent */
    const TSelectionVector& GetSelection( void ) const;

    /** Goes to a directory of a root (by its display name, empty for the current root) */
    bool NavigateTo( const CString& rootName, const CString& directory );

    bool GoBack( void );

    bool GoForward( void );

    bool GoUp( void );

    void Refresh( void );

    const CString& GetCurrentRootName( void ) const;

    const CString& GetCurrentDirectory( void ) const;

    /** The user's search: a glob, recursive through the subfolders when asked */
    void SetSearch( const CString& glob, bool includeSubfolders );

    /** Stops a recursive search that is running */
    void StopSearch( void );

    bool IsSearching( void ) const;

    /** The entries the list shows now, in its order: their names, with "/" after folders (search results: the path below the folder) */
    CString::StringVector GetListedNames( void ) const;

    /** Selects the listed entries with these names (as GetListedNames() gives them) */
    bool SelectNames( const CString::StringVector& names );

    /** The name field */
    void SetName( const CString& name );

    CString GetName( void );

    /** The message box the dialog uses to ask and to tell (for tests and applications that answer it) */
    CMessageBoxForm& GetMessageBox( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    CFileSystemDialogForm( EMode mode, CIGUIContext* context );

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  A last check of what is about to be accepted, for applications that make their own dialog: false refuses it, with the
     *  problem shown to the user
     */
    virtual bool ValidateAccept( const TSelectionVector& selection, CString& problem );

    private:

    struct SRoot
    {
        CString name;
        CORE::TFileSystemAccessPtr access;
        CString initialDirectory;
    };
    typedef GUCEF::vector< SRoot > TRootVector;

    struct SPlace
    {
        CString label;
        UInt32 root;
        CString directory;
    };
    typedef GUCEF::vector< SPlace > TPlaceVector;

    struct SLocation
    {
        UInt32 root;
        CString directory;
    };
    typedef GUCEF::vector< SLocation > TLocationVector;

    struct SEntry
    {
        CString name;           /**< what the list shows in the name column (search results: the path below the folder) */
        CString path;
        bool isDirectory;
        bool hasSize;
        UInt64 size;
        bool hasModified;
        CORE::CDateTime modified;
        bool hidden;
    };
    typedef GUCEF::vector< SEntry > TEntryVector;

    struct SChoice
    {
        CString id;
        CString label;
        CString::StringVector options;
    };
    typedef GUCEF::vector< SChoice > TChoiceVector;

    enum EPendingQuestion
    {
        QUESTION_NONE = 0,
        QUESTION_INFO,
        QUESTION_OVERWRITE,
        QUESTION_NEW_FOLDER,
        QUESTION_RENAME,
        QUESTION_DELETE
    };

    void BuildLayout( CORE::CDataNode& layout );
    bool IsFileMode( void ) const;
    bool IsSaveMode( void ) const;
    CString ResolvePath( const CString& nameOrPath ) const;
    CORE::IFileSystemAccess* GetCurrentAccess( void ) const;

    void ReadDirectory( void );
    void ShowEntries( void );
    bool IsShown( const SEntry& entry ) const;
    bool FitsFileType( const CString& name ) const;
    void SortEntries( void );
    void UpdatePanes( void );
    void UpdateButtons( void );
    void UpdateStatus( void );
    void GoTo( UInt32 root, const CString& directory, bool remember );
    void Enter( const SEntry& entry );

    typedef CORE::CTEventHandlerFunctor< CFileSystemDialogForm > TEventCallback;

    /** Whether the widgets of the dialog are listened to: it is open and no widget is being filled by the dialog itself */
    bool IsListening( void ) const;

    /** The event handlers: one per event of one widget, the ones that are there only while the dialog is open check that first */
    void OnBackActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnForwardActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnUpActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNewFolderActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnOpenFolderActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnAcceptActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCancelActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPathTextEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNameTextEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSearchTextChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSearchSubfoldersChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSearchSubfoldersUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnShowHiddenChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnShowHiddenUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRootsSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPlacesSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFileTypesValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEntriesSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEntriesColumnClicked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEntriesItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEntriesMouseClick( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnContextMenuItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnMessageBoxClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void ApplySearchSubfolders( bool includeSubfolders );
    void ApplyShowHidden( bool showHidden );

    void OnRowsActivated( void );
    void OnNameEntered( void );
    void OnPathEntered( void );
    void OnPlaceChosen( Int32 index );
    void OnSelectionChanged( void );
    void OnContextMenu( Int32 x, Int32 y );
    void OnContextMenuItem( const CString& item );
    void StartSearch( void );
    void PollSearch( void );
    bool QueueSearchDirectory( const CString& directory );
    void Tell( const CString& text );
    void Ask( EPendingQuestion question, const CString& text, const CString* input );
    void OnAnswer( CMessageBoxForm::EMessageBoxResult result );
    bool CollectSelection( TSelectionVector& selection, CString& problem, bool& askOverwrite );
    void Finish( const TSelectionVector& selection );
    void Close( void );
    static CString DescribeResult( CORE::TFileSystemAccessResult result );
    static CString FormatSize( UInt64 size );

    CFileSystemDialogForm( const CFileSystemDialogForm& src );
    CFileSystemDialogForm& operator=( const CFileSystemDialogForm& src );

    private:

    struct SSearchTask
    {
        CString directory;
        CORE::CFutureResult future;

        SSearchTask( const CString& searchDirectory, const CORE::CFutureResult& listing )
            : directory( searchDirectory ), future( listing ) {}
    };
    typedef GUCEF::vector< SSearchTask > TSearchTaskVector;

    EMode m_mode;
    TOptions m_options;
    TRootVector m_roots;
    TPlaceVector m_places;              /**< the app's places and the special directories, as the places pane lists them */
    TPlaceVector m_appPlaces;
    GUCEF::map< Int32, CString > m_specialDirLabels;
    TFileTypeVector m_fileTypes;
    UInt32 m_selectedFileType;
    TChoiceVector m_choices;
    UInt32 m_currentRoot;
    CString m_currentDirectory;
    TLocationVector m_back;
    TLocationVector m_forward;
    TEntryVector m_entries;             /**< what the folder (or the search) has */
    GUCEF::vector< UInt32 > m_listed;   /**< the entries the list shows, in its order */
    Int32 m_sortColumn;
    bool m_sortAscending;
    CORE::CGlobPattern m_search;
    bool m_searchSubfolders;
    bool m_searching;
    TSearchTaskVector m_searchTasks;
    CString::StringVector m_searchQueue;
    UInt32 m_searchFound;
    TSelectionVector m_selection;
    TSelectionVector m_pendingSelection;
    EPendingQuestion m_pendingQuestion;
    CString m_pendingPath;
    bool m_open;
    bool m_updatingWidgets;
    CMessageBoxForm m_messageBox;
    CPopupMenu* m_contextMenu;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CFILESYSTEMDIALOGFORM_H ? */
