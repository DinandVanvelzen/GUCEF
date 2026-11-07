/*
 *  gucefGUI_TestApp: Tests for the GUI module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#include "gucefCORE_CNativeFileSystemAccess.h"
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */

#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#include "gucefCORE_CFileSystemEntryListIterator.h"
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/**
 *  A file system in memory for the dialog tests: paths like "/a/b", directories and files with a size. Its special
 *  directories: home (/home), downloads (/home/dl), desktop (also /home: left out by the dialog as the same place)
 *  and music (/missing, not there: left out too).
 */
class CMemoryFileSystem : public CORE::IFileSystemAccess
{
    public:

    struct SNode
    {
        bool isDirectory;
        UInt64 size;
    };
    typedef GUCEF::map< CORE::CString, SNode > TNodeMap;

    TNodeMap nodes;
    bool writeable;

    CMemoryFileSystem( void )
        : CORE::IFileSystemAccess(), nodes(), writeable( true )
    {
        AddDirectory( "/" );
    }

    static CORE::CString Parent( const CORE::CString& path )
    {
        Int32 slash = path.HasChar( '/', false );
        if ( slash <= 0 )
            return "/";
        return path.SubstrToIndex( (UInt32) slash, true );
    }

    static CORE::CString Join( const CORE::CString& dir, const CORE::CString& name )
    {
        return "/" == dir ? "/" + name : dir + "/" + name;
    }

    void AddDirectory( const CORE::CString& path )
    {
        SNode node;
        node.isDirectory = true;
        node.size = 0;
        nodes[ path ] = node;
    }

    void AddFile( const CORE::CString& path, UInt64 size )
    {
        SNode node;
        node.isDirectory = false;
        node.size = size;
        nodes[ path ] = node;
    }

    bool IsDir( const CORE::CString& path ) const
    {
        TNodeMap::const_iterator i = nodes.find( path );
        return i != nodes.end() && (*i).second.isDirectory;
    }

    bool IsFile( const CORE::CString& path ) const
    {
        TNodeMap::const_iterator i = nodes.find( path );
        return i != nodes.end() && !(*i).second.isDirectory;
    }

    virtual CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "Memory"; }
    virtual CORE::CString GetRootPath( void ) const GUCEF_VIRTUAL_OVERRIDE { return "/"; }
    virtual bool IsWriteable( const CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE { return writeable; }

    virtual CORE::TFileSystemAccessResult TryCreateDirectory( const CORE::CString& path, bool createParents ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !writeable ) return CORE::FILESYSTEMACCESS_READ_ONLY;
        if ( nodes.find( path ) != nodes.end() ) return CORE::FILESYSTEMACCESS_ALREADY_EXISTS;
        if ( !IsDir( Parent( path ) ) )
        {
            if ( !createParents ) return CORE::FILESYSTEMACCESS_PARENT_NOT_FOUND;
            TryCreateDirectory( Parent( path ), true );
        }
        AddDirectory( path );
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }

    virtual CORE::TFileSystemAccessResult TryDeleteDirectory( const CORE::CString& path, bool deleteContents ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !IsDir( path ) ) return CORE::FILESYSTEMACCESS_NOT_FOUND;
        CORE::CString prefix = path + "/";
        for ( TNodeMap::iterator i = nodes.begin(); i != nodes.end(); )
        {
            if ( (*i).first.StartsWith( prefix ) )
            {
                if ( !deleteContents ) return CORE::FILESYSTEMACCESS_DIRECTORY_NOT_EMPTY;
                nodes.erase( i++ );
            }
            else
                ++i;
        }
        nodes.erase( path );
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }

    virtual CORE::TFileSystemAccessResult TryMoveDirectory( const CORE::CString& oldPath, const CORE::CString& newPath ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !IsDir( oldPath ) ) return CORE::FILESYSTEMACCESS_NOT_FOUND;
        if ( nodes.find( newPath ) != nodes.end() ) return CORE::FILESYSTEMACCESS_ALREADY_EXISTS;
        TNodeMap moved;
        CORE::CString prefix = oldPath + "/";
        for ( TNodeMap::iterator i = nodes.begin(); i != nodes.end(); )
        {
            if ( (*i).first == oldPath || (*i).first.StartsWith( prefix ) )
            {
                moved[ newPath + (*i).first.CutChars( oldPath.Length(), true ) ] = (*i).second;
                nodes.erase( i++ );
            }
            else
                ++i;
        }
        nodes.insert( moved.begin(), moved.end() );
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }

    virtual CORE::TFileSystemAccessResult TryMoveFile( const CORE::CString& oldPath, const CORE::CString& newPath, bool overwrite ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !IsFile( oldPath ) ) return CORE::FILESYSTEMACCESS_NOT_FOUND;
        if ( nodes.find( newPath ) != nodes.end() && !overwrite ) return CORE::FILESYSTEMACCESS_ALREADY_EXISTS;
        nodes[ newPath ] = nodes[ oldPath ];
        nodes.erase( oldPath );
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }

    virtual CORE::TFileSystemAccessResult TryMoveFiles( const CORE::CString::StringVector& filePaths, const CORE::CString& destinationDir,
                                                        bool overwrite, TFileSystemAccessResultVector* perFileResults ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( size_t i=0; i<filePaths.size(); ++i )
            TryMoveFile( filePaths[ i ], Join( destinationDir, CORE::ExtractFilename( filePaths[ i ] ) ), overwrite );
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }

    virtual CORE::TFileSystemAccessResult TryRenameFile( const CORE::CString& path, const CORE::CString& newName ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !CORE::CNativeFileSystemAccess::IsValidEntryName( newName ) ) return CORE::FILESYSTEMACCESS_INVALID_NAME;
        return TryMoveFile( path, Join( Parent( path ), newName ), false );
    }

    virtual CORE::TFileSystemAccessResult TryDeleteFile( const CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !IsFile( path ) ) return CORE::FILESYSTEMACCESS_NOT_FOUND;
        nodes.erase( path );
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }

    virtual CORE::TFileSystemAccessResult TryGetMetaData( const CORE::CString& path, CORE::CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE
    {
        TNodeMap::const_iterator i = nodes.find( path );
        if ( i == nodes.end() ) return CORE::FILESYSTEMACCESS_NOT_FOUND;
        metaData.resourceExists = true;
        metaData.hasResourceSizeInBytes = !(*i).second.isDirectory;
        metaData.resourceSizeInBytes = (*i).second.size;
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }

    virtual CORE::TFileSystemAccessResult TryGetEntryType( const CORE::CString& path, CORE::TFileSystemEntryType& entryType ) const GUCEF_VIRTUAL_OVERRIDE
    {
        entryType = IsDir( path ) ? CORE::FILESYSTEMENTRYTYPE_DIRECTORY : IsFile( path ) ? CORE::FILESYSTEMENTRYTYPE_FILE : CORE::FILESYSTEMENTRYTYPE_NONE;
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }

    virtual CORE::TFileSystemAccessResult TryGetFileSystemIterator( const CORE::CString& dirPath, CORE::TFileSystemIteratorPtr& iterator ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !IsDir( dirPath ) ) return IsFile( dirPath ) ? CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY : CORE::FILESYSTEMACCESS_NOT_FOUND;
        CORE::CFileSystemEntryListIterator::TEntryVector entries;
        for ( TNodeMap::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
        {
            if ( "/" == (*i).first || Parent( (*i).first ) != dirPath )
                continue;
            CORE::CFileSystemEntryListIterator::CEntry entry;
            entry.name = CORE::ExtractFilename( (*i).first );
            entry.path = (*i).first;
            entry.isDirectory = (*i).second.isDirectory;
            entry.isFile = !(*i).second.isDirectory;
            entry.hasMetaData = true;
            entry.metaData.hasResourceSizeInBytes = entry.isFile;
            entry.metaData.resourceSizeInBytes = (*i).second.size;
            entries.push_back( entry );
        }
        iterator = CORE::TFileSystemIteratorPtr( GUCEF_NEW CORE::CFileSystemEntryListIterator( dirPath, entries ) );
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }

    virtual CORE::TFileSystemAccessResult ListAvailableSpecialDirs( CORE::TSpecialDirSet& specialDirs ) const GUCEF_VIRTUAL_OVERRIDE
    {
        specialDirs.clear();
        specialDirs.insert( CORE::SPECIALDIR_HOME_DIR );
        specialDirs.insert( CORE::SPECIALDIR_MY_DOWNLOADS_DIR );
        specialDirs.insert( CORE::SPECIALDIR_DESKTOP_DIR );
        specialDirs.insert( CORE::SPECIALDIR_MY_MUSIC_DIR );
        specialDirs.insert( CORE::SPECIALDIR_TEMP_DIR );
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }

    virtual CORE::TFileSystemAccessResult TryResolveSpecialDir( CORE::TSpecialDirs specialDir, CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE
    {
        switch ( specialDir )
        {
            case CORE::SPECIALDIR_HOME_DIR:         path = "/home"; return CORE::FILESYSTEMACCESS_SUCCESS;
            case CORE::SPECIALDIR_DESKTOP_DIR:      path = "/home"; return CORE::FILESYSTEMACCESS_SUCCESS;
            case CORE::SPECIALDIR_MY_DOWNLOADS_DIR: path = "/home/dl"; return CORE::FILESYSTEMACCESS_SUCCESS;
            case CORE::SPECIALDIR_MY_MUSIC_DIR:     path = "/missing"; return CORE::FILESYSTEMACCESS_SUCCESS;
            case CORE::SPECIALDIR_TEMP_DIR:         path = "/tmp"; return CORE::FILESYSTEMACCESS_SUCCESS;
            default:                                return CORE::FILESYSTEMACCESS_NOT_SUPPORTED;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** The memory file system of the dialog tests */
CORE::TFileSystemAccessPtr
MakeMemoryFileSystem( CMemoryFileSystem** memory )
{
    CMemoryFileSystem* fs = GUCEF_NEW CMemoryFileSystem();
    fs->AddDirectory( "/home" );
    fs->AddDirectory( "/home/dl" );
    fs->AddDirectory( "/home/dl/games" );
    fs->AddDirectory( "/home/dl/games/itv" );
    fs->AddDirectory( "/tmp" );
    fs->AddFile( "/home/dl/ITVGOLD.CUE", 120 );
    fs->AddFile( "/home/dl/ITVGOLD.BIN", 650000000 );
    fs->AddFile( "/home/dl/notes.txt", 2048 );
    fs->AddFile( "/home/dl/.hidden", 1 );
    fs->AddFile( "/home/dl/games/itv/DISC.ISO", 400000000 );
    fs->AddFile( "/home/dl/games/itv/readme.txt", 10 );
    fs->AddFile( "/home/save1.gam", 300 );
    if ( GUCEF_NULL != memory )
        *memory = fs;
    return fs->CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

class CEventCount : public CORE::CObserver
{
    public:

    CORE::CEvent eventId;
    UInt32 count;

    CEventCount( CORE::CNotifier* notifier, const CORE::CEvent& id )
        : CORE::CObserver(), eventId( id ), count( 0 )
    {
        SubscribeTo( notifier, id );
    }

    virtual ~CEventCount() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id == eventId )
            ++count;
    }
};

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CreateDialogContext( void )
{
    GUI::TGuiContextPtr contextPtr = CreateNullContext();
    GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
    if ( GUCEF_NULL != context )
    {
        context->SetLogicalResolution( 1280, 960 );
        context->RegisterFont( "Test", MakeTestFont( 8, 10 ) );
        CORE::CDataNode theme;
        GUI::BuildDarkTheme( theme, "Test" );
        context->SetTheme( theme );
    }
    return contextPtr;
}

/*-------------------------------------------------------------------------*/

void
ClickAt( GUI::CGUIContext* context, Int32 x, Int32 y, GUI::TMouseButton button = GUI::MOUSEBUTTON_LEFT )
{
    context->FlushChanges();
    context->InjectMouseMove( x, y );
    context->InjectMouseButton( button, true );
    context->InjectMouseButton( button, false );
}

/*-------------------------------------------------------------------------*/

void
ClickWidget( GUI::CGUIContext* context, GUI::CWidget* widget )
{
    context->FlushChanges();
    const GUI::CRect& rect = widget->GetRect();
    ClickAt( context, rect.left + rect.width / 2, rect.top + rect.height / 2 );
}

/*-------------------------------------------------------------------------*/

/** The middle of a data row of a grid (row 0 is below the header) */
void
RowPoint( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row, Int32& x, Int32& y )
{
    grid->ScrollToRow( row );
    context->FlushChanges();
    const GUI::CRect& rect = grid->GetRect();
    Int32 rowHeight = grid->GetRowHeight();
    x = rect.left + 12;
    y = rect.top + rowHeight + ( row - grid->GetIntProperty( "ScrollPosition" ) ) * rowHeight + rowHeight / 2;
}

/*-------------------------------------------------------------------------*/

void
ClickRow( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row, UInt32 modifiers = 0 )
{
    Int32 x = 0, y = 0;
    RowPoint( context, grid, row, x, y );
    if ( 0 != modifiers )
        context->InjectKey( 0 != ( modifiers & INPUT::KEYMOD_CTRL ) ? INPUT::KEYCODE_LCONTROL : INPUT::KEYCODE_LSHIFT, true, modifiers );
    ClickAt( context, x, y );
    if ( 0 != modifiers )
        context->InjectKey( 0 != ( modifiers & INPUT::KEYMOD_CTRL ) ? INPUT::KEYCODE_LCONTROL : INPUT::KEYCODE_LSHIFT, false, 0 );
    context->RenderFrame( 500 );         // past the double click time: the next click is a click of its own
}

/*-------------------------------------------------------------------------*/

void
DoubleClickRow( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row )
{
    Int32 x = 0, y = 0;
    RowPoint( context, grid, row, x, y );
    ClickAt( context, x, y );
    ClickAt( context, x, y );
    context->RenderFrame( 300 );
}

/*-------------------------------------------------------------------------*/

void
PressKey( GUI::CGUIContext* context, INPUT::KeyCode key, UInt32 modifiers = 0 )
{
    context->InjectKey( key, true, modifiers );
    context->InjectKey( key, false, modifiers );
}

/*-------------------------------------------------------------------------*/

void
TypeInto( GUI::CGUIContext* context, GUI::CWidget* field, const CORE::CString& text, bool enter )
{
    context->FlushChanges();
    context->SetFocus( field );
    field->SetProperty( "Text", CORE::CString::Empty );
    context->InjectText( text );
    if ( enter )
        PressKey( context, INPUT::KEYCODE_RETURN );
}

/*-------------------------------------------------------------------------*/

bool
Contains( const CORE::CString::StringVector& names, const CORE::CString& name )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** Renders frames until the search of a dialog ended, at most a few seconds */
bool
FinishSearch( GUI::CGUIContext* context, GUI::CFileSystemDialogForm& dialog )
{
    for ( UInt32 i=0; i<2000 && dialog.IsSearching(); ++i )
    {
        context->RenderFrame( 5 );
        MT::PrecisionDelay( 2 );
    }
    return !dialog.IsSearching();
}

/*-------------------------------------------------------------------------*/

void
PerformGridViewTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileDialogs: The grid selects one row with a click, several with Ctrl and Shift, moves by keys, finds by typing and sorts by a column" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateDialogContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        screen->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( screen, false );
        GUI::CGridView* grid = static_cast< GUI::CGridView* >( context->CreateWidget( "GridView", "Grid", screen ) );
        grid->SetProperty( "Position", "0,100 0,100" );
        grid->SetProperty( "Size", "0,400 0,130" );
        grid->SetProperty( "RowHeight", "20" );
        grid->SetProperty( "Font", "Test" );
        grid->SetProperty( "Columns", "Name\nSize" );
        GUI::CGridView::TRowVector rows;
        const char* names[] = { "delta", "alpha", "charlie", "bravo", "echo", "foxtrot", "golf", "hotel" };
        const char* sizes[] = { "40", "100", "3", "20", "5", "60", "7", "8" };
        for ( UInt32 i=0; i<8; ++i )
        {
            GUI::CGridView::TStringVector cells;
            cells.push_back( names[ i ] );
            cells.push_back( sizes[ i ] );
            rows.push_back( cells );
        }
        grid->SetRows( rows );
        ASSERT_TRUE( 8 == grid->GetRowCount() && "charlie" == grid->GetText( 0, 2 ) && -1 == grid->GetSelectedRow() );
        context->FlushChanges();
        ASSERT_TRUE( 5 == grid->GetVisibleRowCount() );

        // without MultiSelect: one row, Ctrl changes nothing about that
        CEventCount changes( grid, GUI::CGridView::SelectionChangedEvent );
        ClickRow( context, grid, 1 );
        ASSERT_TRUE( 1 == grid->GetSelectedRow() && 1 == grid->GetSelectedRows().size() && grid->IsRowSelected( 1 ) );
        ClickRow( context, grid, 3, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 3 == grid->GetSelectedRow() && 1 == grid->GetSelectedRows().size() && !grid->IsRowSelected( 1 ) );
        ASSERT_TRUE( changes.count >= 2 );

        // keys move the selection and scroll
        context->SetFocus( grid );
        PressKey( context, INPUT::KEYCODE_DOWN );
        ASSERT_TRUE( 4 == grid->GetSelectedRow() );
        PressKey( context, INPUT::KEYCODE_END );
        ASSERT_TRUE( 7 == grid->GetSelectedRow() && grid->GetIntProperty( "ScrollPosition" ) >= 3 );
        PressKey( context, INPUT::KEYCODE_HOME );
        ASSERT_TRUE( 0 == grid->GetSelectedRow() && 0 == grid->GetIntProperty( "ScrollPosition" ) );
        PressKey( context, INPUT::KEYCODE_PGDOWN );
        ASSERT_TRUE( 4 == grid->GetSelectedRow() );
        PressKey( context, INPUT::KEYCODE_PGUP );
        ASSERT_TRUE( 0 == grid->GetSelectedRow() );

        // typing finds the next row that starts with the text
        context->InjectText( "g" );
        ASSERT_TRUE( 6 == grid->GetSelectedRow() );
        context->InjectText( "E" );
        ASSERT_TRUE( 4 == grid->GetSelectedRow() );

        // Enter and a double click activate the row
        CEventCount activated( grid, GUI::CWidget::ItemActivatedEvent );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( 1 == activated.count );
        DoubleClickRow( context, grid, 2 );
        ASSERT_TRUE( 2 == activated.count );

        // MultiSelect: a click selects one, Ctrl adds and removes, Shift selects a range, Shift + keys extend it
        grid->SetAllowMultiSelect( true );
        ClickRow( context, grid, 1 );
        ClickRow( context, grid, 3, INPUT::KEYMOD_CTRL );
        GUI::CGridView::TInt32Vector selected = grid->GetSelectedRows();
        ASSERT_TRUE( 2 == selected.size() && 1 == selected[ 0 ] && 3 == selected[ 1 ] );
        ClickRow( context, grid, 1, INPUT::KEYMOD_CTRL );
        selected = grid->GetSelectedRows();
        ASSERT_TRUE( 1 == selected.size() && 3 == selected[ 0 ] );
        ClickRow( context, grid, 0 );
        ClickRow( context, grid, 2, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( 3 == grid->GetSelectedRows().size() && grid->IsRowSelected( 0 ) && grid->IsRowSelected( 1 ) && grid->IsRowSelected( 2 ) );
        context->SetFocus( grid );
        PressKey( context, INPUT::KEYCODE_DOWN, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( 4 == grid->GetSelectedRows().size() && 3 == grid->GetSelectedRow() );
        PressKey( context, INPUT::KEYCODE_DOWN );
        ASSERT_TRUE( 1 == grid->GetSelectedRows().size() && 4 == grid->GetSelectedRow() );

        // a click on a column title: an event, a sort when the grid is sortable; the selection stays with its row
        CEventCount columnClicks( grid, GUI::CGridView::ColumnClickedEvent );
        const GUI::CRect& rect = grid->GetRect();
        ClickAt( context, rect.left + 10, rect.top + 5 );
        ASSERT_TRUE( 1 == columnClicks.count && "delta" == grid->GetText( 0, 0 ) );     // not sortable: order kept
        context->RenderFrame( 300 );
        grid->SetBoolProperty( "Sortable", true );
        ClickAt( context, rect.left + 10, rect.top + 5 );
        ASSERT_TRUE( "alpha" == grid->GetText( 0, 0 ) && "hotel" == grid->GetText( 0, 7 ) );
        ASSERT_TRUE( 0 == grid->GetIntProperty( "SortColumn" ) && grid->GetBoolProperty( "SortAscending" ) );
        ASSERT_TRUE( "echo" == grid->GetText( 0, grid->GetSelectedRow() ) );
        context->RenderFrame( 300 );
        ClickAt( context, rect.left + 10, rect.top + 5 );
        ASSERT_TRUE( "hotel" == grid->GetText( 0, 0 ) && !grid->GetBoolProperty( "SortAscending" ) );
        context->RenderFrame( 300 );
        grid->SortRows( 1, true );          // numbers by their value
        ASSERT_TRUE( "charlie" == grid->GetText( 0, 0 ) && "alpha" == grid->GetText( 0, 7 ) );

        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** The text commands of a widget as the painter makes them */
GUI::CDrawSurface::TCommandVector
PaintedCommands( GUI::CGUIContext* context, GUI::CWidget* widget )
{
    GUI::CDrawSurface::TCommandVector commands;
    context->FlushChanges();
    GUI::CWidgetPainter::Paint( *widget, *context, commands );
    return commands;
}

/*-------------------------------------------------------------------------*/

/** The first text command with the text, NULL when none */
const GUI::CDrawSurface::TCommand*
FindTextCommand( const GUI::CDrawSurface::TCommandVector& commands, const CORE::CString& text )
{
    for ( size_t i=0; i<commands.size(); ++i )
        if ( GUI::CDrawSurface::COMMAND_TEXT == commands[ i ].type && text == commands[ i ].text )
            return &commands[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

Int32
TextWidth( GUI::CGUIContext* context, const CORE::CString& text )
{
    Int32 width = 0, height = 0;
    context->MeasureText( "Test", text, 0, width, height );
    return width;
}

/*-------------------------------------------------------------------------*/

/** The smallest and largest x of the line commands in [top, bottom); false when there is none */
bool
LineSpan( const GUI::CDrawSurface::TCommandVector& commands, Int32 top, Int32 bottom, Int32& minX, Int32& maxX )
{
    bool found = false;
    for ( size_t i=0; i<commands.size(); ++i )
    {
        const GUI::CDrawSurface::TCommand& command = commands[ i ];
        if ( GUI::CDrawSurface::COMMAND_LINE != command.type || command.y0 < top || command.y0 >= bottom )
            continue;
        const Int32 low = command.x0 < command.x1 ? command.x0 : command.x1;
        const Int32 high = command.x0 < command.x1 ? command.x1 : command.x0;
        if ( !found || low < minX ) minX = low;
        if ( !found || high > maxX ) maxX = high;
        found = true;
    }
    return found;
}

/*-------------------------------------------------------------------------*/

/** Whether a fill of exactly the rectangle is among the commands */
bool
HasFill( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CRect& rect, GUI::CColour* colour = GUCEF_NULL )
{
    for ( size_t i=0; i<commands.size(); ++i )
    {
        const GUI::CDrawSurface::TCommand& command = commands[ i ];
        if ( GUI::CDrawSurface::COMMAND_FILLRECT == command.type && command.rect.left == rect.left && command.rect.top == rect.top &&
             command.rect.width == rect.width && command.rect.height == rect.height )
        {
            if ( GUCEF_NULL != colour )
                *colour = command.colour;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
PerformGridViewPresentationTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileDialogs: The grid aligns columns, marks the sorted column as a whole, cuts long cells with an ellipsis and shows them whole as a tooltip" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateDialogContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        screen->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( screen, false );
        GUI::CGridView* grid = static_cast< GUI::CGridView* >( context->CreateWidget( "GridView", "Grid", screen ) );
        grid->SetProperty( "Position", "0,100 0,100" );
        grid->SetProperty( "Size", "0,400 0,130" );
        grid->SetProperty( "RowHeight", "20" );
        grid->SetProperty( "Font", "Test" );
        grid->SetProperty( "Columns", "Name\nValue\nFlags" );
        grid->SetProperty( "ColumnWidths", "100,100,200" );
        grid->SetProperty( "ColumnAlignments", "Left\nRight" );
        grid->SetProperty( "Tooltip", "The grid" );
        const CORE::CString longFlags = "not seeded, date unknown, to review, frequency unknown";
        GUI::CGridView::TRowVector rows;
        GUI::CGridView::TStringVector cells;
        cells.push_back( "alpha" ); cells.push_back( "1.50" ); cells.push_back( "short" );
        rows.push_back( cells );
        cells.clear();
        cells.push_back( "bravo" ); cells.push_back( "12,345.00" ); cells.push_back( longFlags );
        rows.push_back( cells );
        grid->SetRows( rows );
        ASSERT_TRUE( "Left" == grid->GetColumnAlignment( 0 ) && "Right" == grid->GetColumnAlignment( 1 ) && "Left" == grid->GetColumnAlignment( 2 ) );
        ASSERT_TRUE( "Left" == grid->GetColumnAlignment( 7 ) );

        // a right aligned column: the titles and cells end at the padding before the right edge of the column; the others start at the left padding
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );
        const Int32 rightEdge = 200 - GUI::CGridView::CELL_PADDING_RIGHT;
        const GUI::CDrawSurface::TCommand* value = FindTextCommand( commands, "1.50" );
        const GUI::CDrawSurface::TCommand* bigValue = FindTextCommand( commands, "12,345.00" );
        const GUI::CDrawSurface::TCommand* valueTitle = FindTextCommand( commands, "Value" );
        const GUI::CDrawSurface::TCommand* name = FindTextCommand( commands, "alpha" );
        ASSERT_TRUE( GUCEF_NULL != value && GUCEF_NULL != bigValue && GUCEF_NULL != valueTitle && GUCEF_NULL != name );
        if ( GUCEF_NULL == value || GUCEF_NULL == bigValue || GUCEF_NULL == valueTitle || GUCEF_NULL == name )
            return;
        ASSERT_TRUE( value->x0 + TextWidth( context, "1.50" ) == rightEdge );
        ASSERT_TRUE( bigValue->x0 + TextWidth( context, "12,345.00" ) == rightEdge );
        ASSERT_TRUE( valueTitle->x0 + TextWidth( context, "Value" ) == rightEdge );
        ASSERT_TRUE( GUI::CGridView::CELL_PADDING_LEFT == name->x0 );

        // Clip (the default): the long cell is drawn whole and cut by the clip of its column
        ASSERT_TRUE( GUCEF_NULL != FindTextCommand( commands, longFlags ) );
        ASSERT_TRUE( grid->IsCellTextCut( 2, 1 ) && !grid->IsCellTextCut( 2, 0 ) && !grid->IsCellTextCut( 0, 0 ) );
        ASSERT_TRUE( "The grid" == grid->GetTooltipAt( 250, 45 ) );

        // the sorted column: its whole title cell is tinted and the arrow stands beside its title, inside its column, on the side away from the edge it is aligned to
        grid->SetIntProperty( "SortColumn", 1 );
        grid->SetBoolProperty( "SortAscending", false );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( HasFill( commands, GUI::CRect( 100, 0, 100, 20 ) ) );
        ASSERT_TRUE( !HasFill( commands, GUI::CRect( 0, 0, 100, 20 ) ) && !HasFill( commands, GUI::CRect( 200, 0, 200, 20 ) ) );
        valueTitle = FindTextCommand( commands, "Value" );
        Int32 minX = 0, maxX = 0;
        ASSERT_TRUE( GUCEF_NULL != valueTitle && LineSpan( commands, 0, 20, minX, maxX ) );
        if ( GUCEF_NULL == valueTitle )
            return;
        ASSERT_TRUE( minX >= 100 && maxX < valueTitle->x0 && valueTitle->x0 - maxX <= GUI::CGridView::SORT_ARROW_ROOM );
        ASSERT_TRUE( valueTitle->x0 + TextWidth( context, "Value" ) == rightEdge );

        grid->SetIntProperty( "SortColumn", 0 );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( HasFill( commands, GUI::CRect( 0, 0, 100, 20 ) ) && !HasFill( commands, GUI::CRect( 100, 0, 100, 20 ) ) );
        const Int32 nameTitleEnd = GUI::CGridView::CELL_PADDING_LEFT + TextWidth( context, "Name" );
        ASSERT_TRUE( LineSpan( commands, 0, 20, minX, maxX ) );
        ASSERT_TRUE( minX > nameTitleEnd && minX - nameTitleEnd <= 6 && maxX < 100 );
        // the cells of the sorted column are not tinted
        ASSERT_TRUE( !HasFill( commands, GUI::CRect( 0, 20, 100, 20 ) ) );

        // Ellipsis: the long cell is cut with "..." to the room of its column, the short ones are whole; the tooltip over the cut one is its whole text
        grid->SetProperty( "TextOverflow", "Ellipsis" );
        commands = PaintedCommands( context, grid );
        bool found = false;
        for ( size_t i=0; i<commands.size(); ++i )
        {
            const GUI::CDrawSurface::TCommand& command = commands[ i ];
            if ( GUI::CDrawSurface::COMMAND_TEXT != command.type || command.y0 != 40 || command.x0 < 200 )
                continue;
            found = true;
            ASSERT_TRUE( command.text.Length() > 3 && command.text.Length() < longFlags.Length() );
            ASSERT_TRUE( "..." == command.text.SubstrFromRange( command.text.Length() - 3, command.text.Length() ) );
            ASSERT_TRUE( longFlags.StartsWith( command.text.SubstrFromRange( 0, command.text.Length() - 3 ) ) );
            ASSERT_TRUE( TextWidth( context, command.text ) <= grid->GetCellTextRoom( 2, 1 ) );
            ASSERT_TRUE( TextWidth( context, command.text ) > grid->GetCellTextRoom( 2, 1 ) - 2 * TextWidth( context, "W" ) );
        }
        ASSERT_TRUE( found && GUCEF_NULL != FindTextCommand( commands, "short" ) && GUCEF_NULL != FindTextCommand( commands, "12,345.00" ) );
        ASSERT_TRUE( longFlags == grid->GetTooltipAt( 250, 45 ) );
        ASSERT_TRUE( "The grid" == grid->GetTooltipAt( 250, 25 ) && "The grid" == grid->GetTooltipAt( 50, 45 ) && "The grid" == grid->GetTooltipAt( 250, 5 ) );

        // the tooltip shows when the pointer rests on the cut cell
        const GUI::CRect& rect = grid->GetRect();
        context->InjectMouseMove( rect.left + 250, rect.top + 45 );
        context->RenderFrame( 800 );
        GUI::CDrawSurface::TCommandVector overlay;
        GUI::CWidgetPainter::PaintOverlay( *context, overlay );
        ASSERT_TRUE( GUCEF_NULL != FindTextCommand( overlay, longFlags ) );

        // the grid that has the keyboard focus says so with an outline of the colour of the focus, the others not
        commands = PaintedCommands( context, grid );
        bool outlined = false;
        for ( size_t i=0; i<commands.size(); ++i )
            outlined = outlined || ( GUI::CDrawSurface::COMMAND_STROKERECT == commands[ i ].type && 0 == commands[ i ].rect.left && 400 == commands[ i ].rect.width );
        ASSERT_TRUE( !outlined );
        context->SetFocus( grid );
        commands = PaintedCommands( context, grid );
        for ( size_t i=0; i<commands.size(); ++i )
            outlined = outlined || ( GUI::CDrawSurface::COMMAND_STROKERECT == commands[ i ].type && 0 == commands[ i ].rect.left && 400 == commands[ i ].rect.width && 130 == commands[ i ].rect.height );
        ASSERT_TRUE( outlined );
        context->SetFocus( GUCEF_NULL );

        // the fitting itself: whole when it fits, nothing when not even the dots fit
        bool cut = true;
        ASSERT_TRUE( "short" == GUI::CWidgetPainter::FitText( *context, "Test", "short", 1000, cut ) && !cut );
        ASSERT_TRUE( GUI::CWidgetPainter::FitText( *context, "Test", "short", 2, cut ).IsNULLOrEmpty() && cut );
        ASSERT_TRUE( "..." == GUI::CWidgetPainter::FitText( *context, "Test", "short", TextWidth( context, "..." ), cut ) && cut );

        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FileDialogs: Rows of a list get a colour of their own with a marker, and long rows an ellipsis with the whole row as tooltip" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateDialogContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        screen->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( screen, false );
        GUI::CListbox* list = static_cast< GUI::CListbox* >( context->CreateWidget( "Listbox", "Notes", screen ) );
        list->SetProperty( "Position", "0,50 0,50" );
        list->SetProperty( "Size", "0,300 0,100" );
        list->SetProperty( "RowHeight", "20" );
        list->SetProperty( "Font", "Test" );
        list->SetProperty( "TextColour", "200,200,200,255" );
        const CORE::CString longNote = "77 positions are from the broker's positions export: their opening lots are not in the ledger";
        list->SetProperty( "Items", "plain\nwarning\n" + longNote );

        // no colours: the rows are as before
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, list );
        const GUI::CDrawSurface::TCommand* plain = FindTextCommand( commands, "plain" );
        ASSERT_TRUE( !list->HasItemColours() && GUCEF_NULL != plain && GUI::CListbox::ITEM_PADDING_LEFT == plain->x0 );

        // a colour for the second row: its marker and its text in the colour; every row makes room for the marker
        list->SetProperty( "ItemColours", "\n255,170,60,255" );
        GUI::CColour colour;
        ASSERT_TRUE( list->HasItemColours() && list->GetItemColour( 1, colour ) && 255 == colour.r && 170 == colour.g && !list->GetItemColour( 0, colour ) && !list->GetItemColour( 2, colour ) );
        commands = PaintedCommands( context, list );
        plain = FindTextCommand( commands, "plain" );
        const GUI::CDrawSurface::TCommand* warning = FindTextCommand( commands, "warning" );
        ASSERT_TRUE( GUCEF_NULL != plain && GUCEF_NULL != warning );
        if ( GUCEF_NULL == plain || GUCEF_NULL == warning )
            return;
        const Int32 textLeft = GUI::CListbox::ITEM_PADDING_LEFT + GUI::CListbox::ITEM_MARKER_ROOM;
        ASSERT_TRUE( textLeft == plain->x0 && textLeft == warning->x0 );
        ASSERT_TRUE( 255 == warning->colour.r && 170 == warning->colour.g && 60 == warning->colour.b );
        ASSERT_TRUE( 200 == plain->colour.r && 200 == plain->colour.g );
        GUI::CColour marker;
        ASSERT_TRUE( HasFill( commands, GUI::CRect( GUI::CListbox::ITEM_PADDING_LEFT, 20 + 3, GUI::CListbox::ITEM_MARKER_WIDTH, 20 - 6 ), &marker ) && 170 == marker.g );
        ASSERT_TRUE( !HasFill( commands, GUI::CRect( GUI::CListbox::ITEM_PADDING_LEFT, 3, GUI::CListbox::ITEM_MARKER_WIDTH, 20 - 6 ) ) );

        // a row wider than the list: cut with "..." when asked, the tooltip over it is the whole row
        ASSERT_TRUE( GUCEF_NULL != FindTextCommand( commands, longNote ) );
        list->SetProperty( "TextOverflow", "Ellipsis" );
        commands = PaintedCommands( context, list );
        ASSERT_TRUE( GUCEF_NULL == FindTextCommand( commands, longNote ) );
        bool found = false;
        for ( size_t i=0; i<commands.size(); ++i )
            if ( GUI::CDrawSurface::COMMAND_TEXT == commands[ i ].type && 40 <= commands[ i ].y0 && commands[ i ].y0 < 60 )
            {
                found = true;
                ASSERT_TRUE( "..." == commands[ i ].text.SubstrFromRange( commands[ i ].text.Length() - 3, commands[ i ].text.Length() ) );
                ASSERT_TRUE( TextWidth( context, commands[ i ].text ) <= list->GetItemTextRoom() );
            }
        ASSERT_TRUE( found );
        ASSERT_TRUE( longNote == list->GetTooltipAt( 100, 45 ) && list->GetTooltipAt( 100, 5 ).IsNULLOrEmpty() );

        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FileDialogs: The new standard properties of grids and lists have their names and descriptors" )
    try
    {
        ASSERT_TRUE( GUI::EWidgetProperty_ColumnAlignments == GUI::StringToWidgetProperty( "ColumnAlignments" ) );
        ASSERT_TRUE( GUI::EWidgetProperty_ItemColours == GUI::StringToWidgetProperty( "ItemColours" ) );
        ASSERT_TRUE( GUI::EWidgetProperty_TextOverflow == GUI::StringToWidgetProperty( "TextOverflow" ) );
        ASSERT_TRUE( "TextOverflow" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_TextOverflow ) );
        // the lookup is a binary search: every name is found again
        for ( UInt32 i=1; i<(UInt32)GUI::EWidgetProperty_Count; ++i )
            ASSERT_TRUE( (GUI::EWidgetProperty) i == GUI::StringToWidgetProperty( GUI::WidgetPropertyToString( (GUI::EWidgetProperty) i ) ) );
        GUI::CWidgetTypeRegistry& registry = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry();
        ASSERT_TRUE( GUCEF_NULL != registry.FindProperty( "GridView", "ColumnAlignments" ) && GUCEF_NULL != registry.FindProperty( "GridView", "TextOverflow" ) );
        ASSERT_TRUE( GUCEF_NULL != registry.FindProperty( "Listbox", "ItemColours" ) && GUCEF_NULL != registry.FindProperty( "Listbox", "TextOverflow" ) );
        CORE::CString normalized;
        ASSERT_TRUE( registry.FindProperty( "GridView", "TextOverflow" )->Validate( "Ellipsis", normalized ) );
        ASSERT_TRUE( !registry.FindProperty( "GridView", "TextOverflow" )->Validate( "Wrap", normalized ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformMessageBoxTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileDialogs: The message box answers with the button chosen, Enter and Escape, and asks for a line of text" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateDialogContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CMessageBoxForm box( context );
        CEventCount closed( &box, GUI::CMessageBoxForm::ClosedEvent );

        ASSERT_TRUE( box.Open( "Question", "Replace the file?", GUI::CMessageBoxForm::MESSAGEBOXBUTTONS_YES_NO ) );
        ASSERT_TRUE( box.IsOpen() && box.IsVisible() && context->IsModalRoot( box.GetRootWidget() ) );
        ASSERT_TRUE( "Replace the file?" == box.GetWidget( "Text" )->GetProperty( "Text" ) && !box.GetWidget( "Input" )->IsVisible() );
        ASSERT_TRUE( !box.GetWidget( "Button1" )->IsVisible() && "Yes" == box.GetWidget( "Button2" )->GetProperty( "Text" ) && "No" == box.GetWidget( "Button3" )->GetProperty( "Text" ) );
        ClickWidget( context, box.GetWidget( "Button2" ) );
        ASSERT_TRUE( !box.IsOpen() && !box.IsVisible() && GUI::CMessageBoxForm::MESSAGEBOXRESULT_YES == box.GetResult() && 1 == closed.count );

        // Escape: the last button
        ASSERT_TRUE( box.Open( "Question", "Go on?", GUI::CMessageBoxForm::MESSAGEBOXBUTTONS_YES_NO_CANCEL ) );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( GUI::CMessageBoxForm::MESSAGEBOXRESULT_CANCEL == box.GetResult() && 2 == closed.count );
        ASSERT_TRUE( box.Open( "Note", "Done.", GUI::CMessageBoxForm::MESSAGEBOXBUTTONS_OK ) );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK == box.GetResult() );

        // a line of text: Enter is the first button
        ASSERT_TRUE( box.OpenInput( "New folder", "The name:", "New folder", GUI::CMessageBoxForm::MESSAGEBOXBUTTONS_OK_CANCEL ) );
        ASSERT_TRUE( box.GetWidget( "Input" )->IsVisible() && "New folder" == box.GetInputText() && context->GetFocus() == box.GetWidget( "Input" ) );
        TypeInto( context, box.GetWidget( "Input" ), "Saves", true );
        ASSERT_TRUE( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK == box.GetResult() && "Saves" == box.GetInputText() && !box.IsOpen() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformOpenDialogTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileDialogs: The open file dialog lists its roots, browses by double click, Up, Back, Forward and a typed path, and opens the chosen file" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateDialogContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CMemoryFileSystem* memory = GUCEF_NULL;
        CORE::TFileSystemAccessPtr fs = MakeMemoryFileSystem( &memory );
        CMemoryFileSystem* other = GUCEF_NULL;
        CORE::TFileSystemAccessPtr otherFs = MakeMemoryFileSystem( &other );
        other->AddFile( "/only-here.txt", 5 );

        GUI::CFileOpenDialogForm dialog( context );
        ASSERT_FALSE( dialog.Open() );                       // nothing to browse
        ASSERT_TRUE( dialog.AddRoot( "Computer", fs, "/home" ) && dialog.AddRoot( "Other", otherFs ) );
        ASSERT_FALSE( dialog.AddRoot( "Computer", fs ) );    // names are unique
        CEventCount accepted( &dialog, GUI::CFileSystemDialogForm::AcceptedEvent );
        CEventCount cancelled( &dialog, GUI::CFileSystemDialogForm::CancelledEvent );
        CEventCount moved( &dialog, GUI::CFileSystemDialogForm::DirectoryChangedEvent );
        ASSERT_TRUE( dialog.Open() );
        ASSERT_TRUE( dialog.IsOpen() && dialog.IsVisible() && context->IsModalRoot( dialog.GetRootWidget() ) );
        ASSERT_TRUE( "Open" == dialog.GetRootWidget()->GetProperty( "Title" ) && "Open" == dialog.GetWidget( "Accept" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "Computer" == dialog.GetCurrentRootName() && "/home" == dialog.GetCurrentDirectory() && "/home" == dialog.GetWidget( "Path" )->GetProperty( "Text" ) );
        GUI::CListbox* roots = dialog.GetWidgetAs< GUI::CListbox >( "Roots" );
        ASSERT_TRUE( GUCEF_NULL != roots && 2 == roots->GetItems().size() && 0 == roots->GetSelectedIndex() );
        ASSERT_FALSE( dialog.GetWidget( "Places" )->IsVisible() );       // no places, no special directories asked

        // folders first, then the files
        CORE::CString::StringVector listed = dialog.GetListedNames();
        ASSERT_TRUE( 2 == listed.size() && "dl/" == listed[ 0 ] && "save1.gam" == listed[ 1 ] );
        GUI::CGridView* grid = dialog.GetWidgetAs< GUI::CGridView >( "Entries" );
        ASSERT_TRUE( GUCEF_NULL != grid && "Folder" == grid->GetText( 2, 0 ) && "GAM file" == grid->GetText( 2, 1 ) && "300 B" == grid->GetText( 1, 1 ) );

        // into a folder with a double click; Back, Forward and Up
        DoubleClickRow( context, grid, 0 );
        ASSERT_TRUE( "/home/dl" == dialog.GetCurrentDirectory() );
        listed = dialog.GetListedNames();
        ASSERT_TRUE( 4 == listed.size() && "games/" == listed[ 0 ] && Contains( listed, "ITVGOLD.CUE" ) && Contains( listed, "notes.txt" ) );
        ASSERT_FALSE( Contains( listed, ".hidden" ) );                    // hidden files are not shown
        ClickWidget( context, dialog.GetWidget( "ShowHidden" ) );
        ASSERT_TRUE( Contains( dialog.GetListedNames(), ".hidden" ) );
        ClickWidget( context, dialog.GetWidget( "ShowHidden" ) );
        ClickWidget( context, dialog.GetWidget( "Back" ) );
        ASSERT_TRUE( "/home" == dialog.GetCurrentDirectory() );
        ClickWidget( context, dialog.GetWidget( "Forward" ) );
        ASSERT_TRUE( "/home/dl" == dialog.GetCurrentDirectory() );
        ClickWidget( context, dialog.GetWidget( "Up" ) );
        ASSERT_TRUE( "/home" == dialog.GetCurrentDirectory() );
        ASSERT_TRUE( dialog.GoUp() && "/" == dialog.GetCurrentDirectory() );
        ASSERT_FALSE( dialog.GoUp() || dialog.GetWidget( "Up" )->IsEnabled() );     // the top
        ASSERT_TRUE( moved.count >= 5 );

        // a typed path: a folder of this root, a file of the other root (the dialog moves there and picks it)
        TypeInto( context, dialog.GetWidget( "Path" ), "/home/dl/games", true );
        ASSERT_TRUE( "/home/dl/games" == dialog.GetCurrentDirectory() );
        TypeInto( context, dialog.GetWidget( "Path" ), "/only-here.txt", true );
        ASSERT_TRUE( "Other" == dialog.GetCurrentRootName() && "/" == dialog.GetCurrentDirectory() && "only-here.txt" == dialog.GetName() );
        TypeInto( context, dialog.GetWidget( "Path" ), "/nowhere", true );
        ASSERT_TRUE( dialog.GetMessageBox().IsOpen() );                     // told, the dialog stays
        dialog.GetMessageBox().Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );

        // the root list switches the root
        ASSERT_TRUE( roots->SetSelectedIndex( 0 ) && "Computer" == dialog.GetCurrentRootName() && "/home" == dialog.GetCurrentDirectory() );

        // a file that is not there is refused; a folder typed in the name field is opened; a file is accepted
        ASSERT_TRUE( dialog.NavigateTo( "", "/home/dl" ) );
        TypeInto( context, dialog.GetWidget( "Name" ), "missing.cue", true );
        ASSERT_TRUE( dialog.IsOpen() && dialog.GetMessageBox().IsOpen() && 0 == accepted.count );
        dialog.GetMessageBox().Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );
        TypeInto( context, dialog.GetWidget( "Name" ), "games", true );
        ASSERT_TRUE( "/home/dl/games" == dialog.GetCurrentDirectory() && dialog.IsOpen() );
        ASSERT_TRUE( dialog.GoBack() );
        CORE::CString::StringVector pick;
        pick.push_back( "ITVGOLD.CUE" );
        ASSERT_TRUE( dialog.SelectNames( pick ) && "ITVGOLD.CUE" == dialog.GetName() );
        ClickWidget( context, dialog.GetWidget( "Accept" ) );
        ASSERT_TRUE( 1 == accepted.count && !dialog.IsOpen() && !dialog.IsVisible() );
        ASSERT_TRUE( 1 == dialog.GetSelection().size() && "/home/dl/ITVGOLD.CUE" == dialog.GetSelection()[ 0 ].path );
        ASSERT_TRUE( "Computer" == dialog.GetSelection()[ 0 ].rootName && !dialog.GetSelection()[ 0 ].isDirectory && fs == dialog.GetSelection()[ 0 ].access );

        // again: a double click on a file opens it; Escape cancels
        ASSERT_TRUE( dialog.Open() && dialog.NavigateTo( "", "/home/dl" ) );
        listed = dialog.GetListedNames();
        Int32 notes = -1;
        for ( UInt32 i=0; i<(UInt32)listed.size(); ++i )
            if ( "notes.txt" == listed[ i ] )
                notes = (Int32) i;
        DoubleClickRow( context, grid, notes );
        ASSERT_TRUE( 2 == accepted.count && "/home/dl/notes.txt" == dialog.GetSelection()[ 0 ].path );
        ASSERT_TRUE( dialog.Open() );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( 1 == cancelled.count && !dialog.IsOpen() && dialog.GetSelection().empty() );

        // several files at once
        dialog.GetOptions().multiSelect = true;
        GUI::CFileOpenDialogForm many( context );
        many.GetOptions().multiSelect = true;
        many.AddRoot( "Computer", fs, "/home/dl" );
        CEventCount manyAccepted( &many, GUI::CFileSystemDialogForm::AcceptedEvent );
        ASSERT_TRUE( many.Open() );
        CORE::CString::StringVector two;
        two.push_back( "ITVGOLD.BIN" );
        two.push_back( "notes.txt" );
        ASSERT_TRUE( many.SelectNames( two ) && many.Accept() );
        ASSERT_TRUE( 1 == manyAccepted.count && 2 == many.GetSelection().size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformSearchTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileDialogs: File types and the user's search filter the list; a glob in the name field searches; a search through the subfolders runs on the pool and can be stopped" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateDialogContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CORE::TFileSystemAccessPtr fs = MakeMemoryFileSystem( GUCEF_NULL );
        GUI::CFileOpenDialogForm dialog( context );
        dialog.AddRoot( "Computer", fs, "/home/dl" );
        CORE::CString::StringVector disc;
        disc.push_back( "*.cue" );
        disc.push_back( "*.iso" );
        dialog.AddFileType( "Game disc", disc, "cue" );
        CORE::CString::StringVector text;
        text.push_back( "*.txt" );
        dialog.AddFileType( "Text", text );
        CEventCount typeChanges( &dialog, GUI::CFileSystemDialogForm::FileTypeChangedEvent );
        CEventCount searchChanges( &dialog, GUI::CFileSystemDialogForm::SearchChangedEvent );
        ASSERT_TRUE( dialog.Open() );

        // the types: "Game disc (*.cue, *.iso)", "Text (*.txt)", "All files (*)"; folders are always listed
        GUI::CCombobox* types = dialog.GetWidgetAs< GUI::CCombobox >( "FileTypes" );
        GUI::CCombobox::TStringVector typeItems;
        ASSERT_TRUE( GUCEF_NULL != types && types->GetListItems( typeItems ) );
        ASSERT_TRUE( 3 == typeItems.size() && "Game disc (*.cue, *.iso)" == typeItems[ 0 ] && "All files (*)" == typeItems[ 2 ] );
        CORE::CString::StringVector listed = dialog.GetListedNames();
        ASSERT_TRUE( 2 == listed.size() && "games/" == listed[ 0 ] && "ITVGOLD.CUE" == listed[ 1 ] );     // the type matches without case
        dialog.SetSelectedFileType( 1 );
        listed = dialog.GetListedNames();
        ASSERT_TRUE( 2 == listed.size() && "notes.txt" == listed[ 1 ] && 1 == typeChanges.count && 1 == types->GetSelectedIndex() );
        dialog.SetSelectedFileType( 2 );
        ASSERT_TRUE( 4 == dialog.GetListedNames().size() );

        // the search of this folder; type and search together
        TypeInto( context, dialog.GetWidget( "Search" ), "itv*", false );
        listed = dialog.GetListedNames();
        ASSERT_TRUE( 2 == listed.size() && Contains( listed, "ITVGOLD.CUE" ) && Contains( listed, "ITVGOLD.BIN" ) && searchChanges.count >= 1 );
        dialog.SetSelectedFileType( 0 );
        listed = dialog.GetListedNames();
        ASSERT_TRUE( 1 == listed.size() && "ITVGOLD.CUE" == listed[ 0 ] );
        dialog.SetSelectedFileType( 2 );

        // a glob typed in the name field is a search, not a name
        TypeInto( context, dialog.GetWidget( "Name" ), "*.{txt,cue}", true );
        ASSERT_TRUE( dialog.IsOpen() && dialog.GetName().IsNULLOrEmpty() && "*.{txt,cue}" == dialog.GetWidget( "Search" )->GetProperty( "Text" ) );
        listed = dialog.GetListedNames();
        ASSERT_TRUE( 2 == listed.size() && Contains( listed, "notes.txt" ) && Contains( listed, "ITVGOLD.CUE" ) );

        // through the subfolders: read on the pool, the results show the path below the folder
        dialog.SetSearch( "*.{iso,txt}", true );
        ASSERT_TRUE( FinishSearch( context, dialog ) );
        listed = dialog.GetListedNames();
        ASSERT_TRUE( 3 == listed.size() && Contains( listed, "games/itv/DISC.ISO" ) && Contains( listed, "games/itv/readme.txt" ) && Contains( listed, "notes.txt" ) );
        ASSERT_TRUE( CORE::CString( dialog.GetWidget( "Status" )->GetProperty( "Text" ) ).HasSubstr( "3 found" ) >= 0 );

        // a pool without worker threads, as in a build without threads (the browser without pthreads): the search still finishes
        const CORE::CString noWorkersPoolName = "gucefGUI_TestApp.NoWorkers";
        CORE::ThreadPoolPtr noWorkers = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( noWorkersPoolName );
        ASSERT_TRUE( !noWorkers.IsNULL() );
        noWorkers->SetDesiredMaxTotalNrOfThreads( 0 );
        ASSERT_TRUE( 0 == noWorkers->GetActiveNrOfThreads() );
        dialog.GetOptions().threadPoolName = noWorkersPoolName;
        dialog.SetSearch( "*.{iso,txt}", true );
        ASSERT_TRUE( FinishSearch( context, dialog ) && 3 == dialog.GetListedNames().size() );
        dialog.GetOptions().threadPoolName.Clear();
        CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterThreadPool( noWorkersPoolName );

        dialog.SetSearch( "games/**/*.iso", true );
        ASSERT_TRUE( FinishSearch( context, dialog ) );
        listed = dialog.GetListedNames();
        ASSERT_TRUE( 1 == listed.size() && "games/itv/DISC.ISO" == listed[ 0 ] );

        // a result is opened as the file it is
        CORE::CString::StringVector pick;
        pick.push_back( "games/itv/DISC.ISO" );
        ASSERT_TRUE( dialog.SelectNames( pick ) && dialog.Accept() && "/home/dl/games/itv/DISC.ISO" == dialog.GetSelection()[ 0 ].path );

        // stopping: Escape stops a running search before it cancels the dialog
        ASSERT_TRUE( dialog.Open() );
        dialog.SetSearch( "*", true );
        if ( dialog.IsSearching() )
        {
            PressKey( context, INPUT::KEYCODE_ESCAPE );
            ASSERT_TRUE( !dialog.IsSearching() && dialog.IsOpen() );
        }
        dialog.SetSearch( CORE::CString::Empty, false );
        ASSERT_TRUE( 4 == dialog.GetListedNames().size() );
        dialog.Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformSaveDialogTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileDialogs: The save file dialog adds the extension of the type, keeps strict types, confirms replacing, and makes, renames and deletes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateDialogContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CMemoryFileSystem* memory = GUCEF_NULL;
        CORE::TFileSystemAccessPtr fs = MakeMemoryFileSystem( &memory );
        GUI::CFileSaveDialogForm dialog( context );
        dialog.GetOptions().allowRename = true;
        dialog.GetOptions().allowDelete = true;
        dialog.AddRoot( "Computer", fs, "/home" );
        CORE::CString::StringVector saves;
        saves.push_back( "*.gam" );
        dialog.AddFileType( "Saved game", saves, "gam" );
        CEventCount accepted( &dialog, GUI::CFileSystemDialogForm::AcceptedEvent );
        dialog.GetOptions().initialName = "game2";
        ASSERT_TRUE( dialog.Open() );
        ASSERT_TRUE( "Save as" == dialog.GetRootWidget()->GetProperty( "Title" ) && "Save" == dialog.GetWidget( "Accept" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "File name:" == dialog.GetWidget( "NameLabel" )->GetProperty( "Text" ) && "game2" == dialog.GetName() );

        // the extension of the type is added
        ClickWidget( context, dialog.GetWidget( "Accept" ) );
        ASSERT_TRUE( 1 == accepted.count && "/home/game2.gam" == dialog.GetSelection()[ 0 ].path );

        // an existing file: asked first, No keeps the dialog open, Yes saves
        ASSERT_TRUE( dialog.Open() );
        dialog.SetName( "save1" );
        ASSERT_FALSE( dialog.Accept() );
        ASSERT_TRUE( dialog.GetMessageBox().IsOpen() && dialog.IsOpen() );
        dialog.GetMessageBox().Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_NO );
        ASSERT_TRUE( dialog.IsOpen() && 1 == accepted.count );
        ASSERT_FALSE( dialog.Accept() );
        ClickWidget( context, dialog.GetMessageBox().GetWidget( "Button2" ) );     // Yes
        ASSERT_TRUE( 2 == accepted.count && "/home/save1.gam" == dialog.GetSelection()[ 0 ].path && !dialog.IsOpen() );

        // strict types: another extension is refused
        dialog.GetOptions().strictFileTypes = true;
        ASSERT_TRUE( dialog.Open() );
        dialog.SetName( "notes.txt" );
        ASSERT_FALSE( dialog.Accept() );
        ASSERT_TRUE( dialog.GetMessageBox().IsOpen() );
        dialog.GetMessageBox().Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );
        dialog.GetOptions().strictFileTypes = false;

        // a new folder (the button), renamed and deleted through the menu of the list
        ClickWidget( context, dialog.GetWidget( "NewFolder" ) );
        ASSERT_TRUE( dialog.GetMessageBox().IsOpen() && "New folder" == dialog.GetMessageBox().GetInputText() );
        TypeInto( context, dialog.GetMessageBox().GetWidget( "Input" ), "Saves", true );
        ASSERT_TRUE( memory->IsDir( "/home/Saves" ) && Contains( dialog.GetListedNames(), "Saves/" ) );
        GUI::CGridView* grid = dialog.GetWidgetAs< GUI::CGridView >( "Entries" );
        ASSERT_TRUE( "Saves" == grid->GetText( 0, grid->GetSelectedRow() ) );     // the new folder is selected
        Int32 x = 0, y = 0;
        RowPoint( context, grid, grid->GetSelectedRow(), x, y );
        ClickAt( context, x, y, GUI::MOUSEBUTTON_RIGHT );
        GUI::CPopupMenu* menu = GUI::CWidget::As< GUI::CPopupMenu >( context->GetPopup() );
        ASSERT_TRUE( GUCEF_NULL != menu );
        if ( GUCEF_NULL != menu )
        {
            GUI::CPopupMenu::TStringVector items = menu->GetItems();
            ASSERT_TRUE( 5 == items.size() && "New folder" == items[ 0 ] && "Rename" == items[ 1 ] && "Delete" == items[ 2 ] && "Refresh" == items[ 4 ] );
            context->FlushChanges();
            const GUI::CRect& menuRect = menu->GetRect();
            ClickAt( context, menuRect.left + 10, menuRect.top + menu->GetRowHeight() + menu->GetRowHeight() / 2 );      // Rename
            ASSERT_TRUE( dialog.GetMessageBox().IsOpen() && "Saves" == dialog.GetMessageBox().GetInputText() );
            TypeInto( context, dialog.GetMessageBox().GetWidget( "Input" ), "Games", true );
            ASSERT_TRUE( memory->IsDir( "/home/Games" ) && !memory->IsDir( "/home/Saves" ) );
            context->RenderFrame( 300 );
            RowPoint( context, grid, grid->GetSelectedRow(), x, y );
            ClickAt( context, x, y, GUI::MOUSEBUTTON_RIGHT );
            context->FlushChanges();
            const GUI::CRect& again = menu->GetRect();
            ClickAt( context, again.left + 10, again.top + 2 * menu->GetRowHeight() + menu->GetRowHeight() / 2 );          // Delete
            ASSERT_TRUE( dialog.GetMessageBox().IsOpen() );
            dialog.GetMessageBox().Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_YES );
            ASSERT_FALSE( memory->IsDir( "/home/Games" ) || Contains( dialog.GetListedNames(), "Games/" ) );
        }

        // read-only: no new folder, nothing is saved
        dialog.Cancel();
        memory->writeable = false;
        ASSERT_TRUE( dialog.Open() );
        ASSERT_FALSE( dialog.GetWidget( "NewFolder" )->IsEnabled() );
        dialog.SetName( "x" );
        ASSERT_FALSE( dialog.Accept() );
        ASSERT_TRUE( dialog.GetMessageBox().IsOpen() );
        dialog.GetMessageBox().Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );
        dialog.Cancel();
        memory->writeable = true;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformDirectoryDialogTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileDialogs: The directory dialogs choose folders only; the directory save dialog makes the folder and marks the files that are there" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateDialogContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CMemoryFileSystem* memory = GUCEF_NULL;
        CORE::TFileSystemAccessPtr fs = MakeMemoryFileSystem( &memory );

        GUI::CDirectoryOpenDialogForm open( context );
        open.GetOptions().multiSelect = true;
        open.AddRoot( "Computer", fs, "/home/dl" );
        ASSERT_TRUE( open.Open() );
        ASSERT_TRUE( "Select a folder" == open.GetRootWidget()->GetProperty( "Title" ) && "Select folder" == open.GetWidget( "Accept" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "Folder:" == open.GetWidget( "NameLabel" )->GetProperty( "Text" ) && GUCEF_NULL == open.GetWidget( "FileTypes" ) );
        CORE::CString::StringVector listed = open.GetListedNames();
        ASSERT_TRUE( 1 == listed.size() && "games/" == listed[ 0 ] );                 // folders only
        ASSERT_TRUE( open.Accept() && 1 == open.GetSelection().size() && "/home/dl" == open.GetSelection()[ 0 ].path && open.GetSelection()[ 0 ].isDirectory );
        ASSERT_TRUE( open.Open() );
        CORE::CString::StringVector games;
        games.push_back( "games/" );
        ASSERT_TRUE( open.SelectNames( games ) && open.Accept() && "/home/dl/games" == open.GetSelection()[ 0 ].path );
        open.GetOptions().showFilesForContext = true;
        ASSERT_TRUE( open.Open() && 4 == open.GetListedNames().size() );
        open.SetName( "nothere" );
        ASSERT_FALSE( open.Accept() );
        open.GetMessageBox().Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );
        open.Cancel();

        GUI::CDirectorySaveDialogForm save( context );
        save.GetOptions().filesToSave.push_back( "notes.txt" );
        save.GetOptions().filesToSave.push_back( "new.txt" );
        save.AddRoot( "Computer", fs, "/home/dl" );
        ASSERT_TRUE( save.Open() );
        ASSERT_TRUE( "Save here" == save.GetWidget( "Accept" )->GetProperty( "Text" ) );
        CORE::CString status = save.GetWidget( "Status" )->GetProperty( "Text" );
        ASSERT_TRUE( status.HasSubstr( "notes.txt (exists)" ) >= 0 && status.HasSubstr( "new.txt (exists)" ) < 0 );
        ASSERT_FALSE( save.Accept() );                                                  // notes.txt is there: asked
        ASSERT_TRUE( save.GetMessageBox().IsOpen() );
        save.GetMessageBox().Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_YES );
        ASSERT_TRUE( "/home/dl" == save.GetSelection()[ 0 ].path );
        ASSERT_TRUE( save.Open() );
        save.SetName( "exports/today" );
        ASSERT_TRUE( save.Accept() && memory->IsDir( "/home/dl/exports/today" ) && "/home/dl/exports/today" == save.GetSelection()[ 0 ].path );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformPlacesAndChoicesTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileDialogs: Special directories and app places in the places pane, the app's choices, and Open folder" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateDialogContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CORE::TFileSystemAccessPtr fs = MakeMemoryFileSystem( GUCEF_NULL );
        GUI::CFileOpenDialogForm dialog( context );
        dialog.GetOptions().showSpecialDirs = true;
        dialog.GetOptions().acceptDirectoriesToo = true;
        dialog.AddRoot( "Computer", fs, "/" );
        dialog.AddPlace( "Saved games", "Computer", "/home" );
        ASSERT_FALSE( dialog.AddPlace( "Nowhere", "No root", "/" ) );
        dialog.SetSpecialDirLabel( CORE::SPECIALDIR_MY_DOWNLOADS_DIR, "Downloaded" );
        CORE::CString::StringVector speeds;
        speeds.push_back( "Slow" );
        speeds.push_back( "Fast" );
        dialog.AddChoice( "readonly", "Open read-only" );
        dialog.AddChoice( "speed", "Speed", speeds );
        CEventCount accepted( &dialog, GUI::CFileSystemDialogForm::AcceptedEvent );
        ASSERT_TRUE( dialog.Open() );

        // the app's place, then home and downloads; desktop is home again and music is not there: left out; temp is not asked for
        GUI::CListbox* places = dialog.GetWidgetAs< GUI::CListbox >( "Places" );
        ASSERT_TRUE( GUCEF_NULL != places && places->IsVisible() && dialog.GetWidget( "PlacesLabel" )->IsVisible() );
        GUI::CListbox::TStringVector labels = places->GetItems();
        ASSERT_TRUE( 3 == labels.size() && "Saved games" == labels[ 0 ] && "Home" == labels[ 1 ] && "Downloaded" == labels[ 2 ] );
        ClickWidget( context, places );
        context->FlushChanges();
        const GUI::CRect& placesRect = places->GetRect();
        ClickAt( context, placesRect.left + 10, placesRect.top + 2 * places->GetRowHeight() + places->GetRowHeight() / 2 );
        ASSERT_TRUE( "/home/dl" == dialog.GetCurrentDirectory() );
        ASSERT_TRUE( "Home" == GUI::CFileSystemDialogForm::GetStandardSpecialDirLabel( CORE::SPECIALDIR_HOME_DIR ) );

        // asked for temp only
        dialog.Cancel();
        dialog.GetOptions().specialDirsToShow.insert( CORE::SPECIALDIR_TEMP_DIR );
        ASSERT_TRUE( dialog.Open() );
        labels = places->GetItems();
        ASSERT_TRUE( 2 == labels.size() && "Temporary files" == labels[ 1 ] );

        // the choices: a check box and a list, read back after accepting
        ASSERT_TRUE( GUCEF_NULL != dialog.GetWidgetAs< GUI::CCheckbox >( "Choice0" ) && GUCEF_NULL != dialog.GetWidgetAs< GUI::CCombobox >( "Choice1" ) );
        ClickWidget( context, dialog.GetWidget( "Choice0" ) );
        dialog.SetChoice( "speed", "Fast" );

        // Open folder: the folder that is selected, else the one shown
        ASSERT_TRUE( GUCEF_NULL != dialog.GetWidget( "OpenFolder" ) );
        ASSERT_TRUE( dialog.NavigateTo( "", "/home/dl" ) );
        ClickWidget( context, dialog.GetWidget( "OpenFolder" ) );
        ASSERT_TRUE( 1 == accepted.count && "/home/dl" == dialog.GetSelection()[ 0 ].path && dialog.GetSelection()[ 0 ].isDirectory );
        ASSERT_TRUE( "true" == dialog.GetChoice( "readonly" ) && "Fast" == dialog.GetChoice( "speed" ) && dialog.GetChoice( "none" ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformNativeDialogTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileDialogs: The open dialog on the native file system of a temporary folder" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateDialogContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefGUI_TestApp_FileDialogs" );
        CORE::DeleteDir( dir, true );
        ASSERT_TRUE( CORE::CreateDirs( CORE::CombinePath( dir, "sub" ) ) );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "disc.cue" ), "FILE \"disc.bin\" BINARY" ) );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "sub/deep.iso" ), "iso" ) );

        CORE::TFileSystemAccessPtr native = ( GUCEF_NEW CORE::CNativeFileSystemAccess( "This computer" ) )->CreateSharedPtr();
        GUI::CFileOpenDialogForm dialog( context );
        dialog.AddRoot( "This computer", native, dir );
        ASSERT_TRUE( dialog.Open() );
        CORE::CString::StringVector listed = dialog.GetListedNames();
        ASSERT_TRUE( 2 == listed.size() && "sub/" == listed[ 0 ] && "disc.cue" == listed[ 1 ] );
        dialog.SetSearch( "*.iso", true );
        ASSERT_TRUE( FinishSearch( context, dialog ) );
        listed = dialog.GetListedNames();
        ASSERT_TRUE( 1 == listed.size() && "sub/deep.iso" == listed[ 0 ] );
        ASSERT_TRUE( dialog.SelectNames( listed ) && dialog.Accept() );
        ASSERT_TRUE( CORE::CombinePath( dir, "sub/deep.iso" ) == dialog.GetSelection()[ 0 ].path );
        CORE::DeleteDir( dir, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPathFieldTests( void )
{
    GUCEF_TESTFW_TESTCASE( "FileDialogs: A path field takes the place of a widget: a line and a button whose dialog puts the folder or file chosen in the line" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateDialogContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        screen->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( screen, false );
        GUI::CWidget* placeholder = context->CreateWidget( "Panel", "Location", screen );
        placeholder->SetProperty( "Position", "0,20 0,40" );
        placeholder->SetProperty( "Size", "0,400 0,30" );

        // a folder: the line fills what the button leaves
        GUI::CPathFieldForm field( GUI::CPathFieldForm::PATHKIND_DIRECTORY );
        CMemoryFileSystem* memory = GUCEF_NULL;
        ASSERT_TRUE( field.AddRoot( "Computer", MakeMemoryFileSystem( &memory ), "/home" ) );
        ASSERT_TRUE( field.Build( placeholder ) );
        context->FlushChanges();
        GUI::CWidget* line = field.GetWidget( "Path" );
        GUI::CWidget* browse = field.GetWidget( "Browse" );
        ASSERT_TRUE( GUCEF_NULL != line && GUCEF_NULL != browse && field.GetRootWidget()->GetParentWidget() == placeholder );
        ASSERT_TRUE( line->GetRect().left == 20 && browse->GetRect().Right() == 420 && line->GetRect().Right() < browse->GetRect().left );
        field.SetPath( "/home" );
        ASSERT_TRUE( "/home" == field.GetPath() && GUI::CPathFieldForm::PATHKIND_DIRECTORY == field.GetPathKind() );

        // the button opens the dialog; the folder chosen goes in the line as a change of the user, the containing form sees it
        CEventCount changed( screen, GUI::CWidget::ValueChangedEvent );
        CEventCount chosen( &field, GUI::CPathFieldForm::PathChosenEvent );
        ClickWidget( context, browse );
        GUI::CFileSystemDialogForm* dialog = field.GetDialog();
        ASSERT_TRUE( GUCEF_NULL != dialog && dialog->IsOpen() && GUI::CFileSystemDialogForm::MODE_OPEN_DIRECTORY == dialog->GetMode() );
        ASSERT_TRUE( dialog->NavigateTo( "Computer", "/home/dl/games" ) && dialog->AcceptCurrentDirectory() );
        ASSERT_TRUE( "/home/dl/games" == field.GetPath() && 1 == chosen.count && changed.count >= 1 && !dialog->IsOpen() );

        // a file
        GUI::CWidget* filePlaceholder = context->CreateWidget( "Panel", "File", screen );
        filePlaceholder->SetProperty( "Size", "0,300 0,30" );
        GUI::CPathFieldForm fileField( GUI::CPathFieldForm::PATHKIND_FILE );
        ASSERT_TRUE( fileField.AddRoot( "Computer", MakeMemoryFileSystem( GUCEF_NULL ), "/home/dl" ) );
        ASSERT_TRUE( fileField.Build( filePlaceholder ) && fileField.OpenDialog() );
        ASSERT_TRUE( GUI::CFileSystemDialogForm::MODE_OPEN_FILE == fileField.GetDialog()->GetMode() );
        fileField.GetDialog()->SetName( "ITVGOLD.CUE" );
        ASSERT_TRUE( fileField.GetDialog()->Accept() && "/home/dl/ITVGOLD.CUE" == fileField.GetPath() );

        // the dialog starts in the folder of the path, or the nearest folder of it that is still there
        const CORE::CString temp = CORE::RelativePath( "$TEMPDIR$" ).ReplaceChar( '\\', '/' );
        ASSERT_TRUE( temp == GUI::CPathFieldForm::NearestExistingFolder( temp + "/gucefGUI_TestApp_NoSuchDir/deeper" ) );
        ASSERT_TRUE( GUI::CPathFieldForm::NearestExistingFolder( CORE::CString::Empty ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformGuiFileDialogTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.FileDialogs" );

    PerformGridViewTests();
    PerformGridViewPresentationTests();
    PerformMessageBoxTests();
    PerformOpenDialogTests();
    PerformSearchTests();
    PerformSaveDialogTests();
    PerformDirectoryDialogTests();
    PerformPlacesAndChoicesTests();
    PerformNativeDialogTests();
    PerformPathFieldTests();
}

/*-------------------------------------------------------------------------*/
