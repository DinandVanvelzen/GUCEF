/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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
#ifndef GUCEF_GUI_CCONTENTBROWSERFORM_H
#define GUCEF_GUI_CCONTENTBROWSERFORM_H

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

#ifndef GUCEF_GUI_CITHUMBNAILPROVIDER_H
#include "gucefGUI_CIThumbnailProvider.h"
#define GUCEF_GUI_CITHUMBNAILPROVIDER_H
#endif /* GUCEF_GUI_CITHUMBNAILPROVIDER_H ? */

#ifndef GUCEF_GUI_CICONTENTTYPERESOLVER_H
#include "gucefGUI_CIContentTypeResolver.h"
#define GUCEF_GUI_CICONTENTTYPERESOLVER_H
#endif /* GUCEF_GUI_CICONTENTTYPERESOLVER_H ? */

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
class CTreeview;
class CScrollPanel;

/**
 *  A browser of folders and what is in them: a tree of the folders of the roots beside a list of the items of the folder
 *  that is open, shown as details (name, type, size, modified, sortable by its columns) or as thumbnails, with a filter line,
 *  a type filter, and back, forward and up.
 *
 *  The roots are file systems (CORE::IFileSystemAccess, AddRoot(), as in the file system dialogs: the VFS for the viewer,
 *  the file system of the computer, ...). The folders of the tree are read when they are opened. What the items are is the
 *  application's: an optional CIContentTypeResolver names the type of a path and a picture for it, an optional
 *  CIThumbnailProvider makes the thumbnails of the thumbnail view. The form asks for the thumbnails of the tiles in view
 *  only, and shows each when it arrives (ThumbnailReadyEvent of the provider): the GUI thread never waits for a thumbnail.
 *
 *  The filter line is a glob (CORE::CGlobPattern): letters in any case, and a text without wildcards finds the names that
 *  have it. The type filter lists the types of the files of the folder; choosing one shows the files of that type (and the
 *  folders). Folders come first, then the files, each by name, or by the column that was clicked.
 *
 *  Events: SelectionChangedEvent (no data, read GetSelection()), ItemActivatedEvent (TCloneableString with the path of the
 *  file: a double click or Enter on a file; on a folder these open it) and FolderChangedEvent (TCloneableString with the new
 *  folder). The thumbnail view realizes a tile for the items in view and a few more; Update() does it after the view scrolled
 *  or changed size, and the form calls it on every FrameStartedEvent of its context.
 *
 *  Keys: the list has the keys of a grid (arrows, Home, End, type to find, Enter); in the thumbnail view the arrows move
 *  the selection a tile (Up and Down a row), Home and End go to the first and the last, Enter opens, and Ctrl and Shift with a
 *  click add to the selection and extend it. Backspace goes up and Alt+Left and Alt+Right go back and forward.
 *
 *  The form builds its layout in code, as a window (Build()) or in the place of a widget of another form (Build( placeholder )).
 */
class GUCEF_GUI_PUBLIC_CPP CContentBrowserForm : public CFormEx
{
    public:

    static const CORE::CEvent SelectionChangedEvent;
    static const CORE::CEvent ItemActivatedEvent;
    static const CORE::CEvent FolderChangedEvent;

    static void RegisterEvents( void );

    enum EViewMode
    {
        VIEWMODE_DETAILS = 0    ,
        VIEWMODE_THUMBNAILS
    };

    /** What the thumbnail view uses */
    enum EThumbnailLayout
    {
        DEFAULT_THUMBNAIL_SIZE = 96     ,
        MIN_THUMBNAIL_SIZE = 32         ,
        MAX_THUMBNAIL_SIZE = 256        ,
        TILE_PADDING = 6                ,
        TILE_LABEL_HEIGHT = 24
    };

    /** An item of the folder that is open */
    struct GUCEF_GUI_PUBLIC_CPP SItem
    {
        CString name;
        CString path;               /**< a path of the file system of its root */
        bool isDirectory;
        bool hasSize;
        UInt64 size;
        bool hasModified;
        CORE::CDateTime modified;
        bool hidden;
        CString typeName;
        CString iconName;

        SItem( void );
    };
    typedef struct SItem TItem;
    typedef GUCEF::vector< TItem > TItemVector;
    typedef GUCEF::vector< Int32 > TInt32Vector;

    CContentBrowserForm( void );

    explicit CContentBrowserForm( CIGUIContext* context );

    virtual ~CContentBrowserForm();

    /** Makes the form as a window of the context; false when it has no context */
    bool Build( const CString& title = "Content" );

    /** Makes the form in the place of 'placeholder', which it fills (as a settings page is built in its pane) */
    bool Build( CWidget* placeholder );

    /*
     *  What it browses
     */

    /** A file system to browse, with the folder it opens in (empty: its own start); the first root added is where the form starts */
    bool AddRoot( const CString& displayName, CORE::TFileSystemAccessPtr access, const CString& initialDirectory = CString::Empty );

    /** How many file systems the form browses and the display name of each, in the order they were added */
    UInt32 GetRootCount( void ) const;

    const CString& GetRootName( UInt32 index ) const;

    /** The maker of the thumbnails (not owned, NULL for none: the tiles show their type); it has to be there while the form has it */
    void SetThumbnailProvider( CIThumbnailProvider* provider );

    CIThumbnailProvider* GetThumbnailProvider( void ) const;

    /** The namer of the types (not owned, NULL for none: folders and the extensions of the files) */
    void SetContentTypeResolver( CIContentTypeResolver* resolver );

    CIContentTypeResolver* GetContentTypeResolver( void ) const;

    /*
     *  Navigation
     */

    /** Goes to a folder of a root (by its display name, empty for the current root; an empty directory is the start of the root); false when it is not a folder */
    bool NavigateTo( const CString& rootName, const CString& directory );

    bool GoBack( void );

    bool GoForward( void );

    bool GoUp( void );

    /** Reads the folder again */
    void Refresh( void );

    const CString& GetCurrentRootName( void ) const;

    const CString& GetCurrentDirectory( void ) const;

    /*
     *  The view
     */

    void SetViewMode( EViewMode mode );

    EViewMode GetViewMode( void ) const;

    /** The longest side of the thumbnails in pixels, kept between MIN_THUMBNAIL_SIZE and MAX_THUMBNAIL_SIZE; thumbnails of another size are asked for again */
    void SetThumbnailSize( UInt32 size );

    UInt32 GetThumbnailSize( void ) const;

    /** The filter line: a glob; empty for everything */
    void SetFilter( const CString& glob );

    const CString& GetFilter( void ) const;

    /** Only the files of the type (and the folders); empty for all types */
    void SetTypeFilter( const CString& typeName );

    const CString& GetTypeFilter( void ) const;

    /** The types the type filter offers: those of the files of the folder, ordered */
    const CString::StringVector& GetTypeFilterChoices( void ) const;

    void SetShowHidden( bool showHidden );

    bool GetShowHidden( void ) const;

    /** Brings the thumbnail tiles up to date after the view scrolled or changed size; only the tiles in view and a row more are made */
    void Update( UInt32 deltaInMs );

    /*
     *  What is listed and chosen
     */

    /** The items that pass the filters, in the order of the list */
    UInt32 GetListedCount( void ) const;

    const TItem& GetListedItem( UInt32 index ) const;

    /** The names of the listed items in order, with "/" after the folders */
    CString::StringVector GetListedNames( void ) const;

    /** The selected items, as indices of the listed items in ascending order */
    TInt32Vector GetSelection( void ) const;

    bool SelectItems( const TInt32Vector& items );

    /** Selects the listed items with these names (as GetListedNames() gives them) */
    bool SelectNames( const CString::StringVector& names );

    /** The paths of the selected items */
    CString::StringVector GetSelectedPaths( void ) const;

    /** Opens an item as a double click does: a folder is entered, for a file ItemActivatedEvent is notified */
    bool ActivateItem( UInt32 index );

    /** The thumbnail view: how many tiles are made now, and the picture of a tile (NULL while it has none) */
    UInt32 GetRealizedTileCount( void ) const;

    /** Whether a thumbnail arrived for the path at the current size */
    bool HasThumbnail( const CString& path ) const;

    /** How many thumbnails were asked of the provider, and how many were answered with a picture, since the form was made */
    UInt32 GetThumbnailRequestCount( void ) const;

    /** The room of a tile of the current size and the number of tiles in a row of the thumbnail view as it is laid out now */
    UInt32 GetTilesPerRow( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SRoot
    {
        CString name;
        CORE::TFileSystemAccessPtr access;
        CString initialDirectory;
        CString topDirectory;
    };
    typedef GUCEF::vector< SRoot > TRootVector;

    struct SLocation
    {
        UInt32 root;
        CString directory;
    };
    typedef GUCEF::vector< SLocation > TLocationVector;

    /** A folder in the tree */
    struct STreeNode
    {
        CString treePath;           /**< the path of the item in the tree widget */
        CString label;
        Int32 parent;               /**< index in m_tree, -1 for the node of a root */
        bool isPlaceholder;         /**< the "..." that makes the arrow of a folder that was not read yet */
        bool loaded;
        bool removed;               /**< a placeholder that was replaced by the folders that were read */
        SLocation location;
    };
    typedef GUCEF::vector< STreeNode > TTreeNodeVector;

    /** A tile of the thumbnail view */
    struct STile
    {
        CWidget* panel;
        Int32 item;                 /**< the listed item it shows, -1 for none */
    };
    typedef GUCEF::vector< STile > TTileVector;

    typedef CORE::CTEventHandlerFunctor< CContentBrowserForm > TEventCallback;

    void OnBackActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnForwardActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnUpActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFilterTextChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTypeFilterValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnThumbnailsChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnThumbnailsUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTreeValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTreePropertyChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnItemsSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnItemsColumnClicked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnItemsItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTilesMouseClick( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTilesKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRootKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnThumbnailReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title );
    CORE::IFileSystemAccess* GetCurrentAccess( void ) const;
    void GoTo( UInt32 root, const CString& directory, bool remember );
    void ReadDirectory( void );
    void SortItems( void );
    void ApplyFilters( void );
    void UpdateTypeFilterChoices( void );
    void ShowItems( void );
    void ShowDetails( void );
    void ShowViewMode( void );
    void UpdateButtons( void );
    void RealizeTiles( void );
    void BindTile( UInt32 slot, Int32 item );
    void ShowTileSelection( void );
    void RequestThumbnails( void );
    CWidget* MakeTile( UInt32 slot );
    void SetSelectionRows( const TInt32Vector& items, bool notify );
    void SelectByTile( Int32 item, UInt32 modifiers );
    void MoveTileSelection( Int32 step, bool absolute );
    void NotifySelectionChanged( void );
    CString TypeNameOf( const TItem& item ) const;

    /** The tree */
    void ResetTree( void );
    Int32 FindTreeNode( const CString& treePath ) const;
    Int32 AddTreeNode( Int32 parent, const CString& label, UInt32 root, const CString& directory );
    void LoadTreeChildren( Int32 node );
    void RevealInTree( UInt32 root, const CString& directory );
    void ShowTree( void );

    CContentBrowserForm( const CContentBrowserForm& src );
    CContentBrowserForm& operator=( const CContentBrowserForm& src );

    private:

    TRootVector m_roots;
    UInt32 m_currentRoot;
    CString m_currentDirectory;
    TLocationVector m_back;
    TLocationVector m_forward;
    TItemVector m_items;                    /**< what the folder has */
    GUCEF::vector< UInt32 > m_listed;       /**< the items that pass the filters, in the order of the list */
    Int32 m_sortColumn;
    bool m_sortAscending;
    CString m_filter;
    CORE::CGlobPattern m_filterPattern;
    CString m_typeFilter;
    CString::StringVector m_typeChoices;
    bool m_showHidden;
    EViewMode m_viewMode;
    UInt32 m_thumbnailSize;
    TInt32Vector m_selection;               /**< the selected listed items, ascending */
    Int32 m_anchorItem;
    TTreeNodeVector m_tree;
    TTileVector m_tiles;
    GUCEF::map< CString, IMAGE::TPixelMapPtr > m_thumbnails;     /**< by path, for the current size */
    GUCEF::set< CString > m_requested;
    UInt32 m_requestCount;
    CIThumbnailProvider* m_provider;
    CIContentTypeResolver* m_resolver;
    Int32 m_lastScroll;
    Int32 m_lastViewportWidth;
    Int32 m_lastViewportHeight;
    UInt32 m_tilesPerRow;
    bool m_tilesDirty;
    bool m_updatingWidgets;
    bool m_frameSubscribed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CCONTENTBROWSERFORM_H ? */
