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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestAssetFormSupport.h"

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

using namespace GUITEST;

/** A small tree of folders and files, and a folder with many */
CORE::TFileSystemAccessPtr
MakeContent( UInt32 bigCount = 0 )
{
    CTestContentFileSystem* fs = GUCEF_NEW CTestContentFileSystem();
    fs->AddDirectory( "/textures" );
    fs->AddDirectory( "/textures/detail" );
    fs->AddDirectory( "/meshes" );
    fs->AddDirectory( "/empty" );
    fs->AddFile( "/textures/rock.png", 100 );
    fs->AddFile( "/textures/sand.png", 2000 );
    fs->AddFile( "/textures/moss.tga", 300000 );
    fs->AddFile( "/textures/readme.txt", 10 );
    fs->AddFile( "/textures/.secret.png", 5 );
    fs->AddFile( "/textures/detail/one.png", 1 );
    fs->AddFile( "/meshes/tree.obj", 500 );
    fs->AddFile( "/meshes/rock.obj", 600 );
    fs->AddFile( "/a.png", 10 );
    fs->AddFile( "/b.txt", 20 );
    fs->AddFile( "/c.dat", 30 );
    if ( bigCount > 0 )
    {
        fs->AddDirectory( "/big" );
        for ( UInt32 i=0; i<bigCount; ++i )
        {
            CORE::CString number = CORE::UInt32ToString( i );
            while ( number.Length() < 4 )
                number = "0" + number;
            fs->AddFile( "/big/f" + number + ".png", 100 + i );
        }
    }
    return fs->CreateSharedPtr();
}

bool
Contains( const CORE::CString::StringVector& names, const CORE::CString& name )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return true;
    return false;
}

/** The names of the tree items the folders widget has */
CORE::CString::StringVector
TreeItems( GUI::CForm& form )
{
    CORE::CString::StringVector items;
    GUI::CWidget* tree = form.GetWidget( "Folders" );
    if ( GUCEF_NULL != tree )
    {
        const GUI::CWidget::TStringVector list = tree->GetStringListProperty( GUI::EWidgetProperty_Items );
        for ( size_t i=0; i<list.size(); ++i )
            items.push_back( list[ i ] );
    }
    return items;
}

class CContentScene : public CFormScene
{
    public:

    GUI::CContentBrowserForm form;
    CTestThumbnailProvider provider;
    CTestTypeResolver resolver;

    explicit CContentScene( GUI::CGUIContext* context ) : CFormScene(), form( context ), provider(), resolver()
    {
        context->RegisterImage( "icon_folder", MakeSolidImage( 8, 8, 200, 170, 60, 255 ) );
        GUI::CWidget* screen = MakeScreen( context );
        form.Build( screen );
        provider.answerAtOnce = true;
        form.SetThumbnailProvider( &provider );
        form.SetContentTypeResolver( &resolver );
        form.AddRoot( "Content", MakeContent() );
        form.SetThumbnailSize( 64 );
        form.NavigateTo( "Content", "/textures" );
        form.SetViewMode( GUI::CContentBrowserForm::VIEWMODE_THUMBNAILS );
        context->RenderFrame( 16 );
        form.Update( 0 );
        CORE::CString::StringVector names;
        names.push_back( "sand.png" );
        form.SelectNames( names );
    }
};

CFormScene*
BuildContentScene( GUI::CGUIContext* context )
{
    return new CContentScene( context );
}

/**
 *  The browser in a host of a fixed width at the left of the screen, as the workbench has it in its left panel: 448 pixels at 1600 wide,
 *  280 at 1000 wide. The tool bar has to put its controls in one, two or three rows so that none is drawn over another
 */
class CHostedContentScene : public CFormScene
{
    public:

    GUI::CContentBrowserForm form;
    CTestThumbnailProvider provider;
    CTestTypeResolver resolver;

    CHostedContentScene( GUI::CGUIContext* context, UInt32 hostWidth ) : CFormScene(), form( context ), provider(), resolver()
    {
        context->RegisterImage( "icon_folder", MakeSolidImage( 8, 8, 200, 170, 60, 255 ) );
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* host = context->CreateWidget( "Panel", "Host", screen );
        host->SetProperty( "Position", "0,0 0,0" );
        host->SetProperty( "Size", "0," + CORE::UInt32ToString( hostWidth ) + " 1,0" );
        form.Build( host );
        provider.answerAtOnce = true;
        form.SetThumbnailProvider( &provider );
        form.SetContentTypeResolver( &resolver );
        form.AddRoot( "Content", MakeContent() );
        form.NavigateTo( "Content", "/textures" );
        context->RenderFrame( 16 );
        form.Update( 0 );
    }
};

CFormScene*
BuildContentHost448Scene( GUI::CGUIContext* context )
{
    return new CHostedContentScene( context, 448 );
}

CFormScene*
BuildContentHost280Scene( GUI::CGUIContext* context )
{
    return new CHostedContentScene( context, 280 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiContentBrowserFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.ContentBrowserForm" );

    GUCEF_TESTFW_TESTCASE( "Content browser: The form builds as a window and in the place of a widget, lists the folder of a root and its tree, folders first" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CContentBrowserForm form( context );
        ASSERT_FALSE( form.AddRoot( "", MakeContent() ) );
        ASSERT_TRUE( form.AddRoot( "Content", MakeContent() ) );        // before the form is built
        ASSERT_FALSE( form.AddRoot( "Content", MakeContent() ) );       // a name once
        ASSERT_TRUE( form.Build( "Content" ) && form.IsVisible() );
        const char* names[] = { "Back", "Forward", "Up", "Filter", "TypeFilter", "Thumbnails", "Folders", "Items", "Thumbs" };
        for ( UInt32 i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
            ASSERT_TRUE( HasWidget( form, names[ i ] ) );

        ASSERT_TRUE( "Content" == form.GetCurrentRootName() && "/" == form.GetCurrentDirectory() );
        ASSERT_TRUE( 6 == form.GetListedCount() );
        CORE::CString::StringVector listed = form.GetListedNames();
        ASSERT_TRUE( "empty/" == listed[ 0 ] && "meshes/" == listed[ 1 ] && "textures/" == listed[ 2 ] && "a.png" == listed[ 3 ] && "b.txt" == listed[ 4 ] && "c.dat" == listed[ 5 ] );
        ASSERT_TRUE( form.GetListedItem( 2 ).isDirectory && "/textures" == form.GetListedItem( 2 ).path && 10 == form.GetListedItem( 3 ).size );
        ASSERT_TRUE( "Folder" == form.GetListedItem( 0 ).typeName && "PNG file" == form.GetListedItem( 3 ).typeName && "DAT file" == form.GetListedItem( 5 ).typeName );

        // the details in a grid: name, type, size
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Items" );
        ASSERT_TRUE( 6 == grid->GetRowCount() && 4 == grid->GetColumnCount() && "Modified" == grid->GetColumnTitle( 3 ) );
        ASSERT_TRUE( "textures/" == grid->GetText( 0, 2 ) && "a.png" == grid->GetText( 0, 3 ) && "PNG file" == grid->GetText( 1, 3 ) && "10 B" == grid->GetText( 2, 3 ) );

        // the tree has the root and its folders, in a form that opens
        const CORE::CString::StringVector tree = TreeItems( form );
        ASSERT_TRUE( Contains( tree, "Content" ) && Contains( tree, "Content/textures" ) && Contains( tree, "Content/meshes" ) && Contains( tree, "Content/empty" ) );
        ASSERT_TRUE( "Content" == form.GetWidgetAs< GUI::CTreeview >( "Folders" )->GetSelectedItem() );
        ASSERT_FALSE( form.GetWidget( "Back" )->IsEnabled() || form.GetWidget( "Forward" )->IsEnabled() || form.GetWidget( "Up" )->IsEnabled() );

        // in the place of a widget of another form
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* placeholder = context->CreateWidget( "Panel", "Place", screen );
        placeholder->SetProperty( "Size", "0,500 0,400" );
        GUI::CContentBrowserForm inPlace;
        ASSERT_FALSE( inPlace.Build( static_cast< GUI::CWidget* >( GUCEF_NULL ) ) );
        ASSERT_TRUE( inPlace.Build( placeholder ) && placeholder == inPlace.GetRootWidget()->GetParentWidget() );
        ASSERT_TRUE( inPlace.AddRoot( "Other", MakeContent() ) && 6 == inPlace.GetListedCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Content browser: Back, forward and up, entering folders and the tree move through the folders, with FolderChangedEvent" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CContentBrowserForm form( context );
        ASSERT_TRUE( form.Build() );
        ASSERT_TRUE( form.AddRoot( "Content", MakeContent() ) );
        context->RenderFrame( 16 );
        CEventRecorder folderChanged( &form, GUI::CContentBrowserForm::FolderChangedEvent );

        ASSERT_TRUE( form.NavigateTo( "", "/textures" ) );
        ASSERT_TRUE( 1 == folderChanged.count && "/textures" == static_cast< CORE::TCloneableString* >( folderChanged.data[ 0 ] )->GetData() );
        ASSERT_TRUE( "/textures" == form.GetCurrentDirectory() && 5 == form.GetListedCount() );
        ASSERT_FALSE( form.NavigateTo( "", "/textures/rock.png" ) );        // a file is no folder
        ASSERT_FALSE( form.NavigateTo( "", "/nothing" ) );
        ASSERT_FALSE( form.NavigateTo( "Unknown", "/" ) );
        ASSERT_TRUE( 1 == folderChanged.count );
        ASSERT_TRUE( form.GetWidget( "Back" )->IsEnabled() && form.GetWidget( "Up" )->IsEnabled() && !form.GetWidget( "Forward" )->IsEnabled() );

        // up, back, forward
        ASSERT_TRUE( form.GoUp() && "/" == form.GetCurrentDirectory() );
        ASSERT_FALSE( form.GoUp() );                                       // the top
        ASSERT_TRUE( form.GoBack() && "/textures" == form.GetCurrentDirectory() );
        ASSERT_TRUE( form.GetWidget( "Forward" )->IsEnabled() );
        ASSERT_TRUE( form.GoBack() && "/" == form.GetCurrentDirectory() );
        ASSERT_FALSE( form.GoBack() );
        ASSERT_TRUE( form.GoForward() && "/textures" == form.GetCurrentDirectory() );
        ASSERT_TRUE( form.GoForward() && "/" == form.GetCurrentDirectory() );
        ASSERT_FALSE( form.GoForward() );

        // the buttons
        form.NavigateTo( "", "/textures/detail" );
        ClickWidget( context, form.GetWidget( "Up" ) );
        ASSERT_TRUE( "/textures" == form.GetCurrentDirectory() );
        ClickWidget( context, form.GetWidget( "Back" ) );
        ASSERT_TRUE( "/textures/detail" == form.GetCurrentDirectory() );
        ClickWidget( context, form.GetWidget( "Back" ) );
        ASSERT_TRUE( "/" == form.GetCurrentDirectory() );
        ClickWidget( context, form.GetWidget( "Forward" ) );
        ASSERT_TRUE( "/textures/detail" == form.GetCurrentDirectory() );
        folderChanged.Reset();

        // a double click on a folder enters it, a double click on a file notifies ItemActivatedEvent with its path
        form.NavigateTo( "", "/" );
        context->RenderFrame( 16 );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Items" );
        CEventRecorder activated( &form, GUI::CContentBrowserForm::ItemActivatedEvent );
        folderChanged.Reset();
        DoubleClickRow( context, grid, 2 );
        ASSERT_TRUE( "/textures" == form.GetCurrentDirectory() && 1 == folderChanged.count && 0 == activated.count );
        context->RenderFrame( 16 );
        CORE::CString::StringVector listed = form.GetListedNames();
        ASSERT_TRUE( 5 == listed.size() && "detail/" == listed[ 0 ] && "rock.png" == listed[ 3 ] );
        DoubleClickRow( context, grid, 3 );
        ASSERT_TRUE( 1 == activated.count && "/textures/rock.png" == static_cast< CORE::TCloneableString* >( activated.data[ 0 ] )->GetData() );
        ASSERT_TRUE( form.ActivateItem( 0 ) && "/textures/detail" == form.GetCurrentDirectory() );
        ASSERT_FALSE( form.ActivateItem( 99 ) );

        // Enter on a file, and the keys that go up and back
        form.NavigateTo( "", "/meshes" );
        context->RenderFrame( 16 );
        context->SetFocus( grid );
        PressKey( context, INPUT::KEYCODE_HOME );
        activated.Reset();
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( 1 == activated.count && "/meshes/rock.obj" == static_cast< CORE::TCloneableString* >( activated.data[ 0 ] )->GetData() );
        PressKey( context, INPUT::KEYCODE_BACK );
        ASSERT_TRUE( "/" == form.GetCurrentDirectory() );
        PressKey( context, INPUT::KEYCODE_LEFT, INPUT::KEYMOD_ALT );
        ASSERT_TRUE( "/meshes" == form.GetCurrentDirectory() );
        PressKey( context, INPUT::KEYCODE_RIGHT, INPUT::KEYMOD_ALT );
        ASSERT_TRUE( "/" == form.GetCurrentDirectory() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Content browser: The tree opens folders when they are opened, goes to a folder that is chosen, and follows the folder that is open" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CContentBrowserForm form( context );
        ASSERT_TRUE( form.Build() );
        ASSERT_TRUE( form.AddRoot( "Content", MakeContent() ) );
        context->RenderFrame( 16 );
        GUI::CTreeview* tree = form.GetWidgetAs< GUI::CTreeview >( "Folders" );
        CEventRecorder folderChanged( &form, GUI::CContentBrowserForm::FolderChangedEvent );

        // a folder that was not read has an arrow, and its folders are there when it opens
        CORE::CString::StringVector items = TreeItems( form );
        ASSERT_TRUE( Contains( items, "Content/textures/..." ) && !Contains( items, "Content/textures/detail" ) );
        tree->SetExpanded( "Content/textures", true );
        items = TreeItems( form );
        ASSERT_TRUE( Contains( items, "Content/textures/detail" ) && !Contains( items, "Content/textures/..." ) );
        ASSERT_TRUE( 0 == folderChanged.count );

        // the user chooses a folder in the tree
        tree->ChangeValueByUser( GUI::EWidgetProperty_SelectedItem, "Content/textures/detail" );
        ASSERT_TRUE( "/textures/detail" == form.GetCurrentDirectory() && 1 == folderChanged.count );
        ASSERT_TRUE( 1 == form.GetListedCount() && "one.png" == form.GetListedNames()[ 0 ] );
        tree->ChangeValueByUser( GUI::EWidgetProperty_SelectedItem, "Content/textures/..." );        // the arrow's place holder is no folder
        ASSERT_TRUE( 1 == folderChanged.count );
        tree->ChangeValueByUser( GUI::EWidgetProperty_SelectedItem, "Content/textures/detail" );     // where it is: nothing changes
        ASSERT_TRUE( 1 == folderChanged.count );

        // a folder that is opened with the functions is in the tree, with the folders on its way
        ASSERT_TRUE( form.NavigateTo( "", "/meshes" ) );
        ASSERT_TRUE( "Content/meshes" == tree->GetSelectedItem() );
        ASSERT_TRUE( form.NavigateTo( "", "/textures/detail" ) && "Content/textures/detail" == tree->GetSelectedItem() );
        ASSERT_TRUE( tree->IsExpanded( "Content" ) && tree->IsExpanded( "Content/textures" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Content browser: The filter line (a glob, or a part of a name), the type filter and the hidden items choose what is listed" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CContentBrowserForm form( context );
        ASSERT_TRUE( form.Build() );
        ASSERT_TRUE( form.AddRoot( "Content", MakeContent(), "/textures" ) );
        context->RenderFrame( 16 );
        ASSERT_TRUE( "/textures" == form.GetCurrentDirectory() && 5 == form.GetListedCount() && !form.GetShowHidden() );     // detail/, moss, readme, rock, sand
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Items" );

        // a glob, in any letter case; the folders stay
        form.SetFilter( "*.PNG" );
        ASSERT_TRUE( "*.PNG" == form.GetFilter() && 3 == form.GetListedCount() && 3 == grid->GetRowCount() );
        CORE::CString::StringVector listed = form.GetListedNames();
        ASSERT_TRUE( "detail/" == listed[ 0 ] && "rock.png" == listed[ 1 ] && "sand.png" == listed[ 2 ] );

        // a text without wildcards finds names that have it
        form.SetFilter( "oc" );
        ASSERT_TRUE( 2 == form.GetListedCount() && "rock.png" == form.GetListedNames()[ 1 ] );
        form.SetFilter( "" );
        ASSERT_TRUE( 5 == form.GetListedCount() );

        // typed in the line by the user
        TypeInto( context, form.GetWidget( "Filter" ), "r*", false );
        ASSERT_TRUE( "r*" == form.GetFilter() && 3 == form.GetListedCount() );        // detail/, readme.txt, rock.png
        form.SetFilter( "" );

        // the type filter: the types of the files of the folder
        const CORE::CString::StringVector choices = form.GetTypeFilterChoices();
        ASSERT_TRUE( 3 == choices.size() && Contains( choices, "PNG file" ) && Contains( choices, "TGA file" ) && Contains( choices, "TXT file" ) );
        form.SetTypeFilter( "PNG file" );
        ASSERT_TRUE( 3 == form.GetListedCount() && "PNG file" == form.GetTypeFilter() );
        GUI::CCombobox* types = form.GetWidgetAs< GUI::CCombobox >( "TypeFilter" );
        ASSERT_TRUE( 1 == types->GetSelectedIndex() );
        types->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "0" );
        ASSERT_TRUE( form.GetTypeFilter().IsNULLOrEmpty() && 5 == form.GetListedCount() );
        types->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "3" );
        ASSERT_TRUE( 2 == form.GetListedCount() && "TXT file" == form.GetTypeFilter() );          // detail/ and readme.txt
        form.SetTypeFilter( "" );

        // a type that is not in the next folder is let go
        form.SetTypeFilter( "TGA file" );
        form.NavigateTo( "", "/meshes" );
        ASSERT_TRUE( form.GetTypeFilter().IsNULLOrEmpty() && 2 == form.GetListedCount() );
        form.NavigateTo( "", "/textures" );

        // hidden items
        form.SetShowHidden( true );
        ASSERT_TRUE( form.GetShowHidden() && 6 == form.GetListedCount() && Contains( form.GetListedNames(), ".secret.png" ) );
        form.SetShowHidden( false );
        ASSERT_TRUE( 5 == form.GetListedCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Content browser: A content type resolver names the types and the pictures of what is listed, and the type filter offers its types" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CContentBrowserForm form( context );
        ASSERT_TRUE( form.Build() );
        ASSERT_TRUE( form.AddRoot( "Content", MakeContent() ) );
        CTestTypeResolver resolver;
        form.SetContentTypeResolver( &resolver );
        ASSERT_TRUE( &resolver == form.GetContentTypeResolver() && resolver.asked >= 6 );
        ASSERT_TRUE( "Folder" == form.GetListedItem( 0 ).typeName && "icon_folder" == form.GetListedItem( 0 ).iconName );
        ASSERT_TRUE( "Texture" == form.GetListedItem( 3 ).typeName && "Text" == form.GetListedItem( 4 ).typeName && "DAT file" == form.GetListedItem( 5 ).typeName );
        ASSERT_TRUE( form.GetListedItem( 3 ).iconName.IsNULLOrEmpty() );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Items" );
        ASSERT_TRUE( "Texture" == grid->GetText( 1, 3 ) );
        const CORE::CString::StringVector choices = form.GetTypeFilterChoices();
        ASSERT_TRUE( 3 == choices.size() && "DAT file" == choices[ 0 ] && "Text" == choices[ 1 ] && "Texture" == choices[ 2 ] );
        form.SetTypeFilter( "Texture" );
        ASSERT_TRUE( 4 == form.GetListedCount() && "a.png" == form.GetListedNames()[ 3 ] );

        // the details sort by the type column
        form.SetTypeFilter( "" );
        context->RenderFrame( 16 );
        const GUI::CRect& rect = grid->GetRect();
        ClickAt( context, rect.left + rect.width * 55 / 100, rect.top + 4 );        // the title of the type column
        ASSERT_TRUE( "c.dat" == form.GetListedNames()[ 3 ] && "a.png" == form.GetListedNames()[ 5 ] );
        context->RenderFrame( 300 );
        ClickAt( context, rect.left + rect.width * 55 / 100, rect.top + 4 );
        ASSERT_TRUE( "a.png" == form.GetListedNames()[ 3 ] && "c.dat" == form.GetListedNames()[ 5 ] );
        form.SetContentTypeResolver( GUCEF_NULL );
        bool foundPng = false;
        for ( UInt32 i=0; i<form.GetListedCount(); ++i )
            foundPng = foundPng || "PNG file" == form.GetListedItem( i ).typeName;
        ASSERT_TRUE( foundPng );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Content browser: Selecting items in the list is notified, by clicks, keys and functions, and the names and paths follow" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CContentBrowserForm form( context );
        ASSERT_TRUE( form.Build() );
        ASSERT_TRUE( form.AddRoot( "Content", MakeContent(), "/textures" ) );
        context->RenderFrame( 16 );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Items" );
        CEventRecorder selection( &form, GUI::CContentBrowserForm::SelectionChangedEvent );

        ClickRow( context, grid, 2 );
        ASSERT_TRUE( 1 == selection.count && 1 == form.GetSelection().size() && 2 == form.GetSelection()[ 0 ] );
        ASSERT_TRUE( 1 == form.GetSelectedPaths().size() && "/textures/readme.txt" == form.GetSelectedPaths()[ 0 ] );
        ClickRow( context, grid, 4, 0, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 2 == selection.count && 2 == form.GetSelection().size() && 4 == form.GetSelection()[ 1 ] );
        ClickRow( context, grid, 1, 0, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( form.GetSelection().size() >= 2 );
        context->SetFocus( grid );
        PressKey( context, INPUT::KEYCODE_END );
        ASSERT_TRUE( 1 == form.GetSelection().size() && 4 == form.GetSelection()[ 0 ] );

        // by the functions: no change, no event; names that are not there are refused
        const UInt32 count = selection.count;
        CORE::CString::StringVector names;
        names.push_back( "rock.png" );
        names.push_back( "detail/" );
        ASSERT_TRUE( form.SelectNames( names ) && count + 1 == selection.count );
        ASSERT_TRUE( 2 == form.GetSelection().size() && 0 == form.GetSelection()[ 0 ] );
        ASSERT_TRUE( form.SelectNames( names ) && count + 1 == selection.count );
        names.push_back( "nothing.png" );
        ASSERT_FALSE( form.SelectNames( names ) );
        GUI::CContentBrowserForm::TInt32Vector rows;
        rows.push_back( 9 );
        ASSERT_FALSE( form.SelectItems( rows ) );
        rows[ 0 ] = 3;
        ASSERT_TRUE( form.SelectItems( rows ) && 1 == form.GetSelection().size() );
        ASSERT_TRUE( "/textures/rock.png" == form.GetSelectedPaths()[ 0 ] );

        // a change of the folder or of the filters ends the selection
        const UInt32 before = selection.count;
        form.SetFilter( "zzz" );
        ASSERT_TRUE( form.GetSelection().empty() && before + 1 == selection.count );
        form.SetFilter( "" );
        form.NavigateTo( "", "/meshes" );
        ASSERT_TRUE( form.GetSelection().empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Content browser: The thumbnail view makes tiles for the items in view, asks the provider for the files only, and shows a thumbnail that arrives later" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CContentBrowserForm form( context );
        ASSERT_TRUE( form.Build() );
        ASSERT_TRUE( form.AddRoot( "Content", MakeContent() ) );
        CTestThumbnailProvider provider;
        form.SetThumbnailProvider( &provider );
        ASSERT_TRUE( &provider == form.GetThumbnailProvider() );
        ASSERT_TRUE( 96 == form.GetThumbnailSize() && GUI::CContentBrowserForm::VIEWMODE_DETAILS == form.GetViewMode() );
        context->RenderFrame( 16 );
        ASSERT_TRUE( 0 == provider.requests.size() && 0 == form.GetRealizedTileCount() );      // nothing is asked for while the list is shown

        // the check box of the tool bar changes the view
        ClickWidget( context, form.GetWidget( "Thumbnails" ) );
        ASSERT_TRUE( GUI::CContentBrowserForm::VIEWMODE_THUMBNAILS == form.GetViewMode() );
        ASSERT_FALSE( form.GetWidget( "Items" )->IsVisible() );
        ASSERT_TRUE( form.GetWidget( "Thumbs" )->IsVisible() );
        context->RenderFrame( 16 );
        form.Update( 0 );
        ASSERT_TRUE( 6 == form.GetRealizedTileCount() && form.GetTilesPerRow() >= 2 );
        ASSERT_TRUE( 3 == provider.requests.size() && 3 == form.GetThumbnailRequestCount() );
        ASSERT_TRUE( 1 == provider.CountRequests( "/a.png" ) && 1 == provider.CountRequests( "/b.txt" ) && 1 == provider.CountRequests( "/c.dat" ) );
        ASSERT_TRUE( 0 == provider.CountRequests( "/textures" ) && 96 == provider.requests[ 0 ].size );

        // a tile has the name, and the type while it has no thumbnail
        GUI::CWidget* tile = form.GetWidget( "Tile_3" );
        ASSERT_TRUE( GUCEF_NULL != tile && tile->IsVisible() && "a.png" == tile->GetChildWidget( "Name" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "PNG file" == tile->GetChildWidget( "Kind" )->GetProperty( "Text" ) && tile->GetChildWidget( "Kind" )->IsVisible() && !tile->GetChildWidget( "Image" )->IsVisible() );
        ASSERT_TRUE( "textures/" == form.GetWidget( "Tile_2" )->GetChildWidget( "Name" )->GetProperty( "Text" ) );

        // the render goes on, nothing is asked twice
        context->RenderFrame( 16 );
        form.Update( 16 );
        ASSERT_TRUE( 3 == provider.requests.size() );

        // the answer comes later: the tile shows the picture
        provider.Answer( "/a.png", 96, true );
        ASSERT_TRUE( form.HasThumbnail( "/a.png" ) && !form.HasThumbnail( "/b.txt" ) );
        ASSERT_TRUE( tile->GetChildWidget( "Image" )->IsVisible() && !tile->GetChildWidget( "Kind" )->IsVisible() );
        provider.Answer( "/b.txt", 96, false );                      // nothing to make: the tile keeps its type
        ASSERT_FALSE( form.HasThumbnail( "/b.txt" ) );
        ASSERT_TRUE( form.GetWidget( "Tile_4" )->GetChildWidget( "Kind" )->IsVisible() );
        provider.Answer( "/c.dat", 48, true );                       // another size than the one asked for
        provider.Answer( "/unknown.png", 96, true );                 // a path that was not asked for
        ASSERT_FALSE( form.HasThumbnail( "/c.dat" ) || form.HasThumbnail( "/unknown.png" ) );
        ASSERT_TRUE( 3 == form.GetThumbnailRequestCount() );

        // another size: the thumbnails are asked for again
        form.SetThumbnailSize( 64 );
        ASSERT_TRUE( 64 == form.GetThumbnailSize() && !form.HasThumbnail( "/a.png" ) );
        ASSERT_TRUE( 6 == provider.requests.size() && 64 == provider.requests.back().size );
        form.SetThumbnailSize( 5 );
        ASSERT_TRUE( GUI::CContentBrowserForm::MIN_THUMBNAIL_SIZE == form.GetThumbnailSize() );
        form.SetThumbnailSize( 5000 );
        ASSERT_TRUE( GUI::CContentBrowserForm::MAX_THUMBNAIL_SIZE == form.GetThumbnailSize() );
        form.SetThumbnailSize( 64 );

        // leaving the folder gives up what was not answered
        provider.cancelled.clear();
        form.NavigateTo( "", "/textures" );
        ASSERT_TRUE( provider.cancelled.size() >= 1 );
        context->RenderFrame( 16 );
        form.Update( 0 );
        ASSERT_TRUE( 1 == provider.CountRequests( "/textures/rock.png" ) && 0 == provider.CountRequests( "/textures/detail" ) );

        // a provider that answers inside the request
        CTestThumbnailProvider quick;
        quick.answerAtOnce = true;
        form.SetThumbnailProvider( &quick );
        form.NavigateTo( "", "/meshes" );
        context->RenderFrame( 16 );
        form.Update( 0 );
        ASSERT_TRUE( form.HasThumbnail( "/meshes/tree.obj" ) && form.HasThumbnail( "/meshes/rock.obj" ) );
        form.SetThumbnailProvider( GUCEF_NULL );
        ASSERT_TRUE( GUCEF_NULL == form.GetThumbnailProvider() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Content browser: A folder with many items makes the tiles that are in view only, and the ones that scroll into view when it scrolls" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CContentBrowserForm form( context );
        ASSERT_TRUE( form.Build() );
        ASSERT_TRUE( form.AddRoot( "Content", MakeContent( 400 ) ) );
        CTestThumbnailProvider provider;
        form.SetThumbnailProvider( &provider );
        ASSERT_TRUE( form.NavigateTo( "", "/big" ) && 400 == form.GetListedCount() );
        form.SetViewMode( GUI::CContentBrowserForm::VIEWMODE_THUMBNAILS );
        context->RenderFrame( 16 );
        form.Update( 0 );
        const UInt32 realized = form.GetRealizedTileCount();
        ASSERT_TRUE( realized > 3 && realized < 40 && provider.requests.size() == realized );
        ASSERT_TRUE( 1 == provider.CountRequests( "/big/f0000.png" ) && 0 == provider.CountRequests( "/big/f0399.png" ) );

        // scrolled to the end: the last items are there
        GUI::CScrollPanel* thumbs = GUI::CWidget::ToScrollPanel( form.GetWidget( "Thumbs" ) );
        context->RenderFrame( 16 );
        ASSERT_TRUE( GUCEF_NULL != thumbs && thumbs->GetContentHeight() > 5000 );
        thumbs->SetScrollPosition( thumbs->GetMaxScrollPosition() );
        context->RenderFrame( 16 );
        ASSERT_TRUE( 1 == provider.CountRequests( "/big/f0399.png" ) );
        ASSERT_TRUE( form.GetRealizedTileCount() < 40 && form.GetRealizedTileCount() > 3 );

        // the keys move through the tiles and bring the selected one into view
        thumbs->SetScrollPosition( 0 );
        context->RenderFrame( 16 );
        ClickWidget( context, form.GetWidget( "Tile_0" ) );
        ASSERT_TRUE( 1 == form.GetSelection().size() && 0 == form.GetSelection()[ 0 ] );
        PressKey( context, INPUT::KEYCODE_END );
        ASSERT_TRUE( 399 == form.GetSelection()[ 0 ] && thumbs->GetScrollPosition() > 5000 );
        PressKey( context, INPUT::KEYCODE_HOME );
        ASSERT_TRUE( 0 == form.GetSelection()[ 0 ] && 0 == thumbs->GetScrollPosition() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Content browser: Tiles are selected by clicks with Ctrl and Shift and by the arrow keys, and opened by a double click or Enter" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CContentBrowserForm form( context );
        ASSERT_TRUE( form.Build() );
        ASSERT_TRUE( form.AddRoot( "Content", MakeContent() ) );
        form.SetViewMode( GUI::CContentBrowserForm::VIEWMODE_THUMBNAILS );
        context->RenderFrame( 16 );
        form.Update( 0 );
        CEventRecorder selection( &form, GUI::CContentBrowserForm::SelectionChangedEvent );
        CEventRecorder activated( &form, GUI::CContentBrowserForm::ItemActivatedEvent );

        ClickWidget( context, form.GetWidget( "Tile_3" ) );
        ASSERT_TRUE( 1 == selection.count && 1 == form.GetSelection().size() && 3 == form.GetSelection()[ 0 ] );
        context->RenderFrame( 500 );
        context->InjectKey( INPUT::KEYCODE_LCONTROL, true, INPUT::KEYMOD_CTRL );
        ClickWidget( context, form.GetWidget( "Tile_5" ) );
        context->InjectKey( INPUT::KEYCODE_LCONTROL, false, 0 );
        ASSERT_TRUE( 2 == form.GetSelection().size() && 3 == form.GetSelection()[ 0 ] && 5 == form.GetSelection()[ 1 ] );
        context->RenderFrame( 500 );
        context->InjectKey( INPUT::KEYCODE_LCONTROL, true, INPUT::KEYMOD_CTRL );
        ClickWidget( context, form.GetWidget( "Tile_3" ) );
        context->InjectKey( INPUT::KEYCODE_LCONTROL, false, 0 );
        ASSERT_TRUE( 1 == form.GetSelection().size() && 5 == form.GetSelection()[ 0 ] );
        context->RenderFrame( 500 );
        context->InjectKey( INPUT::KEYCODE_LSHIFT, true, INPUT::KEYMOD_SHIFT );
        ClickWidget( context, form.GetWidget( "Tile_1" ) );
        context->InjectKey( INPUT::KEYCODE_LSHIFT, false, 0 );
        ASSERT_TRUE( 3 == form.GetSelection().size() && 1 == form.GetSelection()[ 0 ] && 3 == form.GetSelection()[ 2 ] );
        context->RenderFrame( 500 );

        // the keys
        ClickWidget( context, form.GetWidget( "Tile_3" ) );
        PressKey( context, INPUT::KEYCODE_RIGHT );
        ASSERT_TRUE( 4 == form.GetSelection()[ 0 ] );
        PressKey( context, INPUT::KEYCODE_LEFT );
        PressKey( context, INPUT::KEYCODE_LEFT );
        ASSERT_TRUE( 2 == form.GetSelection()[ 0 ] );
        const UInt32 columns = form.GetTilesPerRow();
        PressKey( context, INPUT::KEYCODE_DOWN );
        ASSERT_TRUE( ( 2 + (Int32) columns < 5 ? 2 + (Int32) columns : 5 ) == form.GetSelection()[ 0 ] );
        PressKey( context, INPUT::KEYCODE_UP );
        ASSERT_TRUE( 2 == form.GetSelection()[ 0 ] );
        PressKey( context, INPUT::KEYCODE_END );
        ASSERT_TRUE( 5 == form.GetSelection()[ 0 ] );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( 1 == activated.count && "/c.dat" == static_cast< CORE::TCloneableString* >( activated.data[ 0 ] )->GetData() );

        // a double click: on a file it activates it, on a folder it opens it
        context->RenderFrame( 500 );
        ClickWidget( context, form.GetWidget( "Tile_3" ) );
        ClickWidget( context, form.GetWidget( "Tile_3" ) );
        ASSERT_TRUE( 2 == activated.count && "/a.png" == static_cast< CORE::TCloneableString* >( activated.data[ 1 ] )->GetData() );
        context->RenderFrame( 500 );
        ClickWidget( context, form.GetWidget( "Tile_2" ) );
        ClickWidget( context, form.GetWidget( "Tile_2" ) );
        ASSERT_TRUE( "/textures" == form.GetCurrentDirectory() && 2 == activated.count );
        context->RenderFrame( 16 );
        form.Update( 0 );
        ASSERT_TRUE( 5 == form.GetRealizedTileCount() && form.GetSelection().empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Content browser: The actions of the items are a context menu of the tiles and of the list: a right click selects the item and opens the menu, a choice is notified for the selection, folders and a browser without actions get no menu" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CContentBrowserForm form( context );
        ASSERT_TRUE( form.Build() );
        ASSERT_TRUE( form.AddRoot( "Content", MakeContent() ) );
        form.SetViewMode( GUI::CContentBrowserForm::VIEWMODE_THUMBNAILS );
        context->RenderFrame( 16 );
        form.Update( 0 );
        CEventRecorder chosen( &form, GUI::CContentBrowserForm::ItemActionEvent );

        // no actions: a right click selects, there is no menu
        GUI::CWidget* fileTile = form.GetWidget( "Tile_3" );
        GUI::CWidget* folderTile = form.GetWidget( "Tile_2" );
        ASSERT_TRUE( GUCEF_NULL != fileTile && GUCEF_NULL != folderTile );
        if ( GUCEF_NULL == fileTile || GUCEF_NULL == folderTile )
            return;
        context->FlushChanges();
        const GUI::CRect fileRect = fileTile->GetRect();
        const GUI::CRect folderRect = folderTile->GetRect();
        ClickAt( context, fileRect.left + fileRect.width / 2, fileRect.top + fileRect.height / 2, GUI::MOUSEBUTTON_RIGHT );
        ASSERT_TRUE( !form.IsItemActionsOpen() && GUCEF_NULL == context->GetPopup() );

        GUI::TContextMenuItemVector actions;
        actions.push_back( GUI::CContextMenuItem( "open", "Open", "Enter" ) );
        actions.push_back( GUI::CContextMenuItem::Separator() );
        actions.push_back( GUI::CContextMenuItem( "remove", "Remove", CORE::CString::Empty, false ) );
        form.SetItemActions( actions );
        ASSERT_TRUE( 3 == form.GetItemActions().size() );

        // a right click on a file: it is the selection and its menu opens; the first item is chosen with a click on it
        context->RenderFrame( 500 );
        ClickAt( context, fileRect.left + fileRect.width / 2, fileRect.top + fileRect.height / 2, GUI::MOUSEBUTTON_RIGHT );
        ASSERT_TRUE( 1 == form.GetSelection().size() && 3 == form.GetSelection()[ 0 ] && form.IsItemActionsOpen() );
        GUI::CPopupMenu* menu = GUI::CWidget::As< GUI::CPopupMenu >( context->GetPopup() );
        ASSERT_TRUE( GUCEF_NULL != menu && 3 == menu->GetItems().size() );
        if ( GUCEF_NULL == menu )
            return;
        context->FlushChanges();
        context->RenderFrame( 500 );
        const GUI::CRect menuRect = menu->GetRect();
        ClickAt( context, menuRect.left + 10, menuRect.top + menu->GetRowHeight() / 2 );
        ASSERT_TRUE( 1 == chosen.count && "open" == static_cast< CORE::TCloneableString* >( chosen.data[ 0 ] )->GetData() );
        ASSERT_TRUE( !form.IsItemActionsOpen() && 1 == form.GetSelectedPaths().size() && "/a.png" == form.GetSelectedPaths()[ 0 ] );

        // a disabled action is not chosen; a folder has no menu
        ASSERT_FALSE( form.ChooseItemAction( "remove" ) );
        ASSERT_FALSE( form.ChooseItemAction( "unknown" ) );
        context->RenderFrame( 500 );
        ClickAt( context, folderRect.left + folderRect.width / 2, folderRect.top + folderRect.height / 2, GUI::MOUSEBUTTON_RIGHT );
        ASSERT_TRUE( !form.IsItemActionsOpen() && 1 == chosen.count );

        // the list of the details view: its context menu has the same actions
        form.SetViewMode( GUI::CContentBrowserForm::VIEWMODE_DETAILS );
        context->RenderFrame( 16 );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Items" );
        ASSERT_TRUE( GUCEF_NULL != grid );
        if ( GUCEF_NULL == grid )
            return;
        ASSERT_TRUE( grid->OpenContextMenu( 4, 0, 100, 100 ) );
        ASSERT_TRUE( form.IsItemActionsOpen() && 1 == form.GetSelection().size() && 4 == form.GetSelection()[ 0 ] );
        ASSERT_TRUE( grid->ChooseContextMenuItem( "open" ) );
        ASSERT_TRUE( 2 == chosen.count && "open" == static_cast< CORE::TCloneableString* >( chosen.data[ 1 ] )->GetData() && "/b.txt" == form.GetSelectedPaths()[ 0 ] );
        ASSERT_FALSE( grid->OpenContextMenu( 0, 0, 100, 100 ) );        // a folder
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Content browser: A layout of the form saves and loads again: the new form browses the roots it is given, and the tiles are made again" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CContentBrowserForm first( context );
        ASSERT_TRUE( first.Build() );
        CORE::CDataNode layout;
        ASSERT_TRUE( first.SaveLayout( layout ) );

        GUI::CContentBrowserForm second( context );
        ASSERT_TRUE( second.AddRoot( "Content", MakeContent() ) );
        ASSERT_TRUE( second.LoadLayout( layout ) && second.Show() );
        ASSERT_TRUE( HasWidget( second, "Folders" ) && HasWidget( second, "Items" ) && HasWidget( second, "Thumbs" ) );
        ASSERT_TRUE( 6 == second.GetListedCount() && 6 == second.GetWidgetAs< GUI::CGridView >( "Items" )->GetRowCount() );
        context->RenderFrame( 16 );
        CEventRecorder selection( &second, GUI::CContentBrowserForm::SelectionChangedEvent );
        ClickRow( context, second.GetWidgetAs< GUI::CGridView >( "Items" ), 3 );
        ASSERT_TRUE( 1 == selection.count );
        CEventRecorder folderChanged( &second, GUI::CContentBrowserForm::FolderChangedEvent );
        DoubleClickRow( context, second.GetWidgetAs< GUI::CGridView >( "Items" ), 1 );
        ASSERT_TRUE( 1 == folderChanged.count && "/meshes" == second.GetCurrentDirectory() );

        // loaded again while it shows tiles: they are made again in the new widgets
        second.SetViewMode( GUI::CContentBrowserForm::VIEWMODE_THUMBNAILS );
        context->RenderFrame( 16 );
        second.Update( 0 );
        ASSERT_TRUE( 2 == second.GetRealizedTileCount() );
        ASSERT_TRUE( second.LoadLayout( layout ) );
        context->RenderFrame( 16 );
        second.Update( 0 );
        ASSERT_TRUE( 2 == second.GetRealizedTileCount() && "/meshes" == second.GetCurrentDirectory() );
        ASSERT_TRUE( second.GetWidget( "Thumbs" )->IsVisible() && !second.GetWidget( "Items" )->IsVisible() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Content browser: An alternate root gets a switch in the tool bar, the switch and the functions go between the two views and remember the folder of the files, and the filter and the opening of a folder are notified" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CContentBrowserForm form( context );
        ASSERT_TRUE( form.Build() );
        ASSERT_TRUE( form.AddRoot( "Content", MakeContent() ) && form.AddRoot( "Grouped", MakeContent() ) );
        context->RenderFrame( 16 );
        form.Update( 0 );
        GUI::CWidget* viewSwitch = form.GetWidget( "ViewSwitch" );
        ASSERT_TRUE( GUCEF_NULL != viewSwitch && !viewSwitch->GetBoolProperty( GUI::EWidgetProperty_Visible ) );
        ASSERT_TRUE( form.GetAlternateRootName().IsNULLOrEmpty() && !form.IsAlternateViewActive() );
        ASSERT_FALSE( form.SetAlternateRoot( "Nothing", "Files", "Resources" ) );

        // the switch is there when there is an alternate root
        ASSERT_TRUE( form.SetAlternateRoot( "Grouped", "Files", "Resources" ) && "Grouped" == form.GetAlternateRootName() );
        context->RenderFrame( 16 );
        form.Update( 0 );
        ASSERT_TRUE( viewSwitch->GetBoolProperty( GUI::EWidgetProperty_Visible ) && !form.IsAlternateViewActive() );
        GUI::CCombobox* combobox = form.GetWidgetAs< GUI::CCombobox >( "ViewSwitch" );
        ASSERT_TRUE( GUCEF_NULL != combobox && 0 == combobox->GetSelectedIndex() );

        // to the alternate view and back: the folder of the files is where it was
        CEventRecorder switched( &form, GUI::CContentBrowserForm::ViewSwitchedEvent );
        ASSERT_TRUE( form.NavigateTo( "Content", "/textures" ) && "Content" == form.GetCurrentRootName() );
        form.SetAlternateViewActive( true );
        ASSERT_TRUE( form.IsAlternateViewActive() && "Grouped" == form.GetCurrentRootName() && "/" == form.GetCurrentDirectory() && 1 == combobox->GetSelectedIndex() );
        ASSERT_TRUE( 0 == switched.count );
        form.SetAlternateViewActive( false );
        ASSERT_TRUE( !form.IsAlternateViewActive() && "Content" == form.GetCurrentRootName() && "/textures" == form.GetCurrentDirectory() && 0 == combobox->GetSelectedIndex() );

        // the person's choice with the switch is notified; going to the root another way moves the switch
        combobox->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "1" );
        ASSERT_TRUE( 1 == switched.count && form.IsAlternateViewActive() && "Grouped" == form.GetCurrentRootName() );
        combobox->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "0" );
        ASSERT_TRUE( 2 == switched.count && !form.IsAlternateViewActive() && "/textures" == form.GetCurrentDirectory() );
        ASSERT_TRUE( form.NavigateTo( "Grouped", "/" ) && 1 == combobox->GetSelectedIndex() && form.IsAlternateViewActive() );
        ASSERT_TRUE( 2 == switched.count );

        // the filter line and the opening of a folder tell the application
        CEventRecorder filterChanged( &form, GUI::CContentBrowserForm::FilterChangedEvent );
        form.SetFilter( "rock" );
        ASSERT_TRUE( 1 == filterChanged.count );
        form.SetFilter( "rock" );
        ASSERT_TRUE( 1 == filterChanged.count );
        form.SetFilter( CORE::CString::Empty );
        ASSERT_TRUE( 2 == filterChanged.count );
        form.NavigateTo( "", "/" );
        context->RenderFrame( 16 );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Items" );
        CEventRecorder folderActivated( &form, GUI::CContentBrowserForm::FolderActivatedEvent );
        CEventRecorder fileActivated( &form, GUI::CContentBrowserForm::ItemActivatedEvent );
        DoubleClickRow( context, grid, 2 );
        ASSERT_TRUE( 1 == folderActivated.count && 0 == fileActivated.count && "/textures" == static_cast< CORE::TCloneableString* >( folderActivated.data[ 0 ] )->GetData() );
        ASSERT_TRUE( "/textures" == form.GetCurrentDirectory() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Content browser: A root with an order of its own lists its folders by rank in the list and in the tree, and the alternate view gives the tree and the list the room for long names" )
    try
    {
        class COrderOfTheTest : public GUI::CIContentItemOrder
        {
            public:
            virtual Int32 GetRank( const CORE::CString& path, bool isDirectory ) const
            {
                return isDirectory ? ( "/textures" == path ? 1 : ( "/meshes" == path ? 2 : ( "/empty" == path ? 3 : 0 ) ) ) : 0;
            }
        };
        COrderOfTheTest order;
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CContentBrowserForm form( context );
        ASSERT_TRUE( form.Build() );
        ASSERT_TRUE( form.AddRoot( "Content", MakeContent() ) && form.AddRoot( "Ordered", MakeContent() ) );
        ASSERT_FALSE( form.SetRootOrder( "Nothing", &order ) );
        ASSERT_TRUE( form.SetRootOrder( "Ordered", &order ) );
        context->RenderFrame( 16 );

        // by name in the root without an order, by rank in the root with one; the files stay behind the folders, by name
        ASSERT_TRUE( form.NavigateTo( "Content", "/" ) );
        CORE::CString::StringVector names = form.GetListedNames();
        ASSERT_TRUE( 6 == names.size() && "empty/" == names[ 0 ] && "meshes/" == names[ 1 ] && "textures/" == names[ 2 ] && "a.png" == names[ 3 ] );
        ASSERT_TRUE( form.NavigateTo( "Ordered", "/" ) );
        names = form.GetListedNames();
        ASSERT_TRUE( 6 == names.size() && "textures/" == names[ 0 ] && "meshes/" == names[ 1 ] && "empty/" == names[ 2 ] && "a.png" == names[ 3 ] );
        // the tree of the root has the same order
        form.GoBack();
        form.NavigateTo( "Ordered", "/" );
        context->RenderFrame( 16 );
        const CORE::CString::StringVector tree = TreeItems( form );
        size_t texturesAt = tree.size(), emptyAt = tree.size();
        for ( size_t i=0; i<tree.size(); ++i )
        {
            if ( tree[ i ].HasSubstr( "textures" ) >= 0 && texturesAt == tree.size() && tree[ i ].HasSubstr( "detail" ) < 0 ) texturesAt = i;
            if ( tree[ i ].HasSubstr( "empty" ) >= 0 ) emptyAt = i;
        }
        ASSERT_TRUE( texturesAt < emptyAt && emptyAt < tree.size() );

        // the alternate view: the tree has more room than in the files view
        ASSERT_TRUE( form.SetAlternateRoot( "Ordered", "Files", "Resources" ) );
        form.NavigateTo( "Content", "/" );
        context->RenderFrame( 16 );
        form.Update( 0 );
        context->RenderFrame( 16 );
        const Int32 filesTree = form.GetWidget( "Folders" )->GetRect().width;
        form.SetAlternateViewActive( true );
        context->RenderFrame( 16 );
        form.Update( 0 );
        context->RenderFrame( 16 );
        const Int32 alternateTree = form.GetWidget( "Folders" )->GetRect().width;
        ASSERT_TRUE( alternateTree > filesTree + 20 );
        form.SetAlternateViewActive( false );
        context->RenderFrame( 16 );
        form.Update( 0 );
        context->RenderFrame( 16 );
        ASSERT_TRUE( form.GetWidget( "Folders" )->GetRect().width == filesTree );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    RunFormRenderTests( "content browser", &BuildContentScene );
    RunFormLintBaseline( "ContentBrowser", &BuildContentScene );
    RunFormLintBaseline( "ContentBrowserHost448", &BuildContentHost448Scene );
    RunFormLintBaseline( "ContentBrowserHost280", &BuildContentHost280Scene );
}

/*-------------------------------------------------------------------------*/
