/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#ifndef GUCEF_GUI_TESTASSETFORMSUPPORT_H
#define GUCEF_GUI_TESTASSETFORMSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_INPUT_H
#include "gucefINPUT.h"
#define GUCEF_INPUT_H
#endif /* GUCEF_INPUT_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#include "gucefCORE_CFileSystemEntryListIterator.h"
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */

#include "gucefGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUITEST {

/** Counts the events of a notifier and keeps a copy of the data of each */
class CEventRecorder : public CORE::CObserver
{
    public:

    CORE::CEvent eventId;
    UInt32 count;
    GUCEF::vector< CORE::CICloneable* > data;

    CEventRecorder( CORE::CNotifier* notifier, const CORE::CEvent& id )
        : CORE::CObserver(), eventId( id ), count( 0 ), data()
    {
        SubscribeTo( notifier, id );
    }

    virtual ~CEventRecorder()
    {
        UnsubscribeAllFromObserver();
        Reset();
    }

    void Reset( void )
    {
        for ( size_t i=0; i<data.size(); ++i )
            delete data[ i ];
        data.clear();
        count = 0;
    }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id != eventId )
            return;
        ++count;
        data.push_back( GUCEF_NULL != eventData ? eventData->Clone() : GUCEF_NULL );
    }

    /** The data of the last event as the type it was sent as, NULL without one */
    template< class T >
    T* Last( void ) const
    {
        return data.empty() ? GUCEF_NULL : static_cast< T* >( data.back() );
    }
};

/** A scene of one of the forms for the render tests: what it builds goes away with it */
class CFormScene
{
    public:

    virtual ~CFormScene() {}
};

typedef CFormScene* ( *TFormSceneBuilder )( GUI::CGUIContext* context );

/*-------------------------------------------------------------------------*/

/** The test font and the dark theme: what the forms are made for */
inline void
PrepareFormContext( GUI::CGUIContext* context )
{
    context->RegisterFont( "Test", MakeTestFont( 6, 9 ) );
    CORE::CDataNode theme;
    GUI::BuildDarkTheme( theme, "Test" );
    context->SetTheme( theme );
}

/*-------------------------------------------------------------------------*/

/** A panel as large as the context, to put a form in */
inline GUI::CWidget*
MakeScreen( GUI::CGUIContext* context )
{
    GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
    root->SetProperty( "Size", "1,0 1,0" );
    root->SetColourProperty( "BackgroundColour", GUI::CColour( 34, 36, 44, 255 ) );
    context->ShowRoot( root, false );
    return root;
}

/*-------------------------------------------------------------------------*/

inline void
ClickAt( GUI::CGUIContext* context, Int32 x, Int32 y, GUI::TMouseButton button = GUI::MOUSEBUTTON_LEFT )
{
    context->FlushChanges();
    context->InjectMouseMove( x, y );
    context->InjectMouseButton( button, true );
    context->InjectMouseButton( button, false );
}

/*-------------------------------------------------------------------------*/

inline void
ClickWidget( GUI::CGUIContext* context, GUI::CWidget* widget )
{
    context->FlushChanges();
    const GUI::CRect& rect = widget->GetRect();
    ClickAt( context, rect.left + rect.width / 2, rect.top + rect.height / 2 );
}

/*-------------------------------------------------------------------------*/

inline void
PressKey( GUI::CGUIContext* context, INPUT::KeyCode key, UInt32 modifiers = 0 )
{
    context->InjectKey( key, true, modifiers );
    context->InjectKey( key, false, modifiers );
}

/*-------------------------------------------------------------------------*/

inline void
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

/** The middle of a data row of a grid in the coordinates of the context (row 0 is below the header) */
inline void
RowPoint( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row, Int32 column, Int32& x, Int32& y )
{
    grid->ScrollToRow( row );
    context->FlushChanges();
    const GUI::CRect& rect = grid->GetRect();
    const Int32 rowHeight = grid->GetRowHeight();
    GUI::CRect cell;
    if ( grid->GetCellRect( row, column, cell ) )
        x = rect.left + cell.left + cell.width / 2;
    else
        x = rect.left + 12;
    y = rect.top + rowHeight + ( row - grid->GetIntProperty( "ScrollPosition" ) ) * rowHeight + rowHeight / 2;
}

/*-------------------------------------------------------------------------*/

inline void
ClickRow( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row, Int32 column = 0, UInt32 modifiers = 0 )
{
    Int32 x = 0, y = 0;
    RowPoint( context, grid, row, column, x, y );
    if ( 0 != modifiers )
        context->InjectKey( 0 != ( modifiers & INPUT::KEYMOD_CTRL ) ? INPUT::KEYCODE_LCONTROL : INPUT::KEYCODE_LSHIFT, true, modifiers );
    ClickAt( context, x, y );
    if ( 0 != modifiers )
        context->InjectKey( 0 != ( modifiers & INPUT::KEYMOD_CTRL ) ? INPUT::KEYCODE_LCONTROL : INPUT::KEYCODE_LSHIFT, false, 0 );
    context->RenderFrame( 500 );         // past the double click time: the next click is a click of its own
}

/*-------------------------------------------------------------------------*/

inline void
DoubleClickRow( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row, Int32 column = 0 )
{
    Int32 x = 0, y = 0;
    RowPoint( context, grid, row, column, x, y );
    ClickAt( context, x, y );
    ClickAt( context, x, y );
    context->RenderFrame( 300 );
}

/*-------------------------------------------------------------------------*/

/** A context of the null driver with the test font and theme, NULL when the driver cannot be loaded */
inline GUI::TGuiContextPtr
CreateFormContext( void )
{
    GUI::TGuiContextPtr contextPtr = CreateNullContext();
    GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
    if ( GUCEF_NULL != context )
        PrepareFormContext( context );
    return contextPtr;
}

/*-------------------------------------------------------------------------*/

/** Whether a widget with the name is somewhere below the root widget of the form */
inline bool
HasWidget( GUI::CForm& form, const CORE::CString& name )
{
    return GUCEF_NULL != form.GetWidget( name );
}


/*-------------------------------------------------------------------------*/

/** A file system in memory that only reads: paths like "/a/b", directories and files with a size, for the tests of what lists them */
class CTestContentFileSystem : public CORE::IFileSystemAccess
{
    public:

    struct SNode
    {
        bool isDirectory;
        UInt64 size;
    };
    typedef GUCEF::map< CORE::CString, SNode > TNodeMap;

    TNodeMap nodes;

    CTestContentFileSystem( void ) : CORE::IFileSystemAccess(), nodes()
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

    virtual CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "Content"; }
    virtual CORE::CString GetRootPath( void ) const GUCEF_VIRTUAL_OVERRIDE { return "/"; }
    virtual bool IsWriteable( const CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual CORE::TFileSystemAccessResult TryCreateDirectory( const CORE::CString& path, bool createParents ) GUCEF_VIRTUAL_OVERRIDE { return CORE::FILESYSTEMACCESS_READ_ONLY; }
    virtual CORE::TFileSystemAccessResult TryDeleteDirectory( const CORE::CString& path, bool deleteContents ) GUCEF_VIRTUAL_OVERRIDE { return CORE::FILESYSTEMACCESS_READ_ONLY; }
    virtual CORE::TFileSystemAccessResult TryMoveDirectory( const CORE::CString& oldPath, const CORE::CString& newPath ) GUCEF_VIRTUAL_OVERRIDE { return CORE::FILESYSTEMACCESS_READ_ONLY; }
    virtual CORE::TFileSystemAccessResult TryMoveFile( const CORE::CString& oldPath, const CORE::CString& newPath, bool overwrite ) GUCEF_VIRTUAL_OVERRIDE { return CORE::FILESYSTEMACCESS_READ_ONLY; }
    virtual CORE::TFileSystemAccessResult TryMoveFiles( const CORE::CString::StringVector& filePaths, const CORE::CString& destinationDir,
                                                        bool overwrite, TFileSystemAccessResultVector* perFileResults ) GUCEF_VIRTUAL_OVERRIDE { return CORE::FILESYSTEMACCESS_READ_ONLY; }
    virtual CORE::TFileSystemAccessResult TryRenameFile( const CORE::CString& path, const CORE::CString& newName ) GUCEF_VIRTUAL_OVERRIDE { return CORE::FILESYSTEMACCESS_READ_ONLY; }
    virtual CORE::TFileSystemAccessResult TryDeleteFile( const CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE { return CORE::FILESYSTEMACCESS_READ_ONLY; }

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
            entry.name = (*i).first.CutChars( (UInt32) ( (*i).first.HasChar( '/', false ) + 1 ), true );
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
        return CORE::FILESYSTEMACCESS_SUCCESS;
    }

    virtual CORE::TFileSystemAccessResult TryResolveSpecialDir( CORE::TSpecialDirs specialDir, CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return CORE::FILESYSTEMACCESS_NOT_SUPPORTED;
    }
};

/*-------------------------------------------------------------------------*/

/** A thumbnail provider that writes down what it is asked and answers when the test says so */
class CTestThumbnailProvider : public GUI::CIThumbnailProvider
{
    public:

    struct SRequest
    {
        CORE::CString path;
        UInt32 size;
    };

    GUCEF::vector< SRequest > requests;
    GUCEF::vector< SRequest > cancelled;
    bool answerAtOnce;

    CTestThumbnailProvider( void ) : GUI::CIThumbnailProvider(), requests(), cancelled(), answerAtOnce( false ) {}

    virtual void RequestThumbnail( const CORE::CString& path, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE
    {
        SRequest request;
        request.path = path;
        request.size = size;
        requests.push_back( request );
        if ( answerAtOnce )
            Answer( path, size, true );
    }

    virtual void CancelThumbnail( const CORE::CString& path, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE
    {
        SRequest request;
        request.path = path;
        request.size = size;
        cancelled.push_back( request );
    }

    /** The picture is ready: a square of a colour that comes from the path, or nothing for a path without one */
    void Answer( const CORE::CString& path, UInt32 size, bool withPicture )
    {
        TThumbnail thumbnail;
        thumbnail.path = path;
        thumbnail.size = size;
        if ( withPicture )
        {
            const UInt8 shade = (UInt8) ( 60 + ( path.Length() * 37 ) % 190 );
            thumbnail.pixels = MakeSolidImage( size / 2 + 1, size / 2 + 1, shade, (UInt8) ( 255 - shade ), 120, 255 );
        }
        TThumbnailReadyEventData eventData( thumbnail );
        NotifyObservers( ThumbnailReadyEvent, &eventData );
    }

    UInt32 CountRequests( const CORE::CString& path ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<requests.size(); ++i )
            if ( requests[ i ].path == path )
                ++count;
        return count;
    }
};

/*-------------------------------------------------------------------------*/

/** Names the types by extension and gives folders and textures a picture name */
class CTestTypeResolver : public GUI::CIContentTypeResolver
{
    public:

    UInt32 asked;

    CTestTypeResolver( void ) : GUI::CIContentTypeResolver(), asked( 0 ) {}

    virtual CORE::CString GetTypeName( const CORE::CString& path, bool isDirectory ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++asked;
        if ( isDirectory )
            return "Folder";
        const CORE::CString lower = path.Lowercase();
        if ( lower.HasSubstr( ".png" ) >= 0 || lower.HasSubstr( ".tga" ) >= 0 )
            return "Texture";
        if ( lower.HasSubstr( ".txt" ) >= 0 )
            return "Text";
        return CORE::CString::Empty;        // no opinion: the form says what it knows
    }

    virtual CORE::CString GetIconName( const CORE::CString& path, bool isDirectory ) GUCEF_VIRTUAL_OVERRIDE
    {
        return isDirectory ? "icon_folder" : CORE::CString::Empty;
    }
};

}; /* namespace GUITEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/** Draws a scene with the reference renderer and with the RmlUi and ImGui drivers and compares the pixels (TestGuiAssetFormRender.cpp) */
void RunFormRenderTests( const char* formName, GUCEF::GUITEST::TFormSceneBuilder builder );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_TESTASSETFORMSUPPORT_H ? */
