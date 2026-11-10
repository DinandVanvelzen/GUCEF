/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

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

CORE::CDataNode*
AddWidget( CORE::CDataNode& parent, const char* type, const char* name, const char* position = GUCEF_NULL, const char* size = GUCEF_NULL )
{
    CORE::CDataNode* node = parent.AddChild( "Widget" );
    node->SetAttribute( "type", type );
    node->SetAttribute( "name", name );
    if ( GUCEF_NULL != position )
        node->SetAttribute( "position", position );
    if ( GUCEF_NULL != size )
        node->SetAttribute( "size", size );
    return node;
}

void
AddProperty( CORE::CDataNode& widget, const char* name, const char* value )
{
    CORE::CDataNode* node = widget.AddChild( "Property" );
    node->SetAttribute( "name", name );
    node->SetAttribute( "value", value );
}

/** A small menu screen: a panel with a title, a list (a string list property), two buttons and some unknown content */
void
MakeMenuLayout( CORE::CDataNode& layout )
{
    layout.Clear();
    layout.SetName( "GUILayout" );
    layout.SetAttribute( "version", "1" );
    CORE::CDataNode* panel = AddWidget( layout, "Panel", "Menu", "0.5,-100 0.5,-80", "0,200 0,160" );
    AddProperty( *panel, "BackgroundColour", "0,0,64,255" );
    // something an editor keeps; two attributes, so every codec keeps it a node of its own
    CORE::CDataNode* editorInfo = panel->AddChild( "EditorInfo" );
    editorInfo->SetAttribute( "author", "someone" );
    editorInfo->SetAttribute( "grid", "8" );
    CORE::CDataNode* title = AddWidget( *panel, "Label", "Title", "0,10 0,10" );
    AddProperty( *title, "Text", "Main menu" );
    AddProperty( *title, "FutureProperty", "kept" );
    CORE::CDataNode* list = AddWidget( *panel, "Listbox", "Choices", "0,10 0,40", "0,180 0,60" );
    AddProperty( *list, "Items", "New game\nLoad game" );
    CORE::CDataNode* ok = AddWidget( *panel, "Button", "Ok", "0,10 1,-30", "0,80 0,20" );
    AddProperty( *ok, "Text", "Ok" );
    CORE::CDataNode* hints = ok->AddChild( "BackendHints" );
    hints->SetAttribute( "driver", "Null" );
    AddProperty( *hints, "NullOnly", "yes" );
    CORE::CDataNode* otherHints = ok->AddChild( "BackendHints" );
    otherHints->SetAttribute( "driver", "SomeOtherDriver" );
    AddProperty( *otherHints, "OtherOnly", "yes" );
    CORE::CDataNode* cancel = AddWidget( *panel, "Button", "Cancel", "1,-90 1,-30", "0,80 0,20" );
    AddProperty( *cancel, "Text", "Cancel" );
}

const CORE::CDataNode*
FindWidgetNode( const CORE::CDataNode& parent, const char* name )
{
    CORE::CDataNode::const_iterator i = parent.ConstBegin();
    while ( i != parent.ConstEnd() )
    {
        const CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() == "Widget" && child->GetAttributeValue( "name" ).AsString() == name )
            return child;
        const CORE::CDataNode* found = FindWidgetNode( *child, name );
        if ( GUCEF_NULL != found )
            return found;
    }
    return GUCEF_NULL;
}

const CORE::CDataNode*
FindPropertyNode( const CORE::CDataNode& widget, const char* name )
{
    CORE::CDataNode::const_iterator i = widget.ConstBegin();
    while ( i != widget.ConstEnd() )
    {
        const CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() == "Property" && child->GetAttributeValue( "name" ).AsString() == name )
            return child;
    }
    return GUCEF_NULL;
}

/** Names, attributes and children (in order) the same, all the way down */
bool
DeepEqual( const CORE::CDataNode& a, const CORE::CDataNode& b )
{
    if ( !a.Compare( b, CORE::CDataNode::TStringSet() ) || a.GetNrOfDirectChildNodes() != b.GetNrOfDirectChildNodes() )
        return false;
    CORE::CDataNode::const_iterator i = a.ConstBegin();
    CORE::CDataNode::const_iterator j = b.ConstBegin();
    while ( i != a.ConstEnd() && j != b.ConstEnd() )
    {
        if ( !DeepEqual( *(*i), *(*j) ) )
            return false;
        ++i;
        ++j;
    }
    return true;
}

CORE::CString
TempPath( const char* name )
{
    CORE::CString path = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( path, name );
    return path;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiLayoutTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.Layout" );

    GUCEF_TESTFW_TESTCASE( "Layout: A gucefGUI layout builds the widgets, with lists, backend hints for this driver and found by name or path" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        CORE::CDataNode layout;
        MakeMenuLayout( layout );

        GUI::CForm form( context );
        ASSERT_TRUE( form.LoadLayout( layout ) );
        GUI::CWidget* root = form.GetRootWidget();
        ASSERT_TRUE( GUCEF_NULL != root && "Menu" == root->GetName() && "Panel" == root->GetType() );
        ASSERT_TRUE( 4 == root->GetChildren().size() );
        ASSERT_TRUE( root == form.GetWidget( "Menu" ) );
        ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "Ok" ) && form.GetWidget( "Ok" ) == form.GetWidget( "Ok" )->GetParentWidget()->GetChildWidget( "Ok" ) );
        ASSERT_TRUE( form.GetWidget( "Title" ) == root->FindWidget( "Title" ) );
        ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "Nothing" ) );
        ASSERT_TRUE( "Menu/Ok" == form.GetWidget( "Ok" )->GetNamePath() );

        GUI::CListbox* list = form.GetWidgetAs< GUI::CListbox >( "Choices" );
        ASSERT_TRUE( GUCEF_NULL != list && 2 == list->GetItems().size() && "Load game" == list->GetItems()[ 1 ] );
        ASSERT_TRUE( GUCEF_NULL == form.GetWidgetAs< GUI::CListbox >( "Ok" ) );
        ASSERT_TRUE( "Main menu" == form.GetWidget( "Title" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "kept" == form.GetWidget( "Title" )->GetProperty( "FutureProperty" ) );
        ASSERT_TRUE( "yes" == form.GetWidget( "Ok" )->GetProperty( "NullOnly" ) );
        ASSERT_FALSE( form.GetWidget( "Ok" )->IsPropertySet( "OtherOnly" ) );

        ASSERT_TRUE( form.Show() && form.IsVisible() );
        context->FlushChanges();
        ASSERT_TRUE( GUI::CRect( 220, 160, 200, 160 ) == root->GetRect() );
        ASSERT_TRUE( GUI::CRect( 230, 290, 80, 20 ) == form.GetWidget( "Ok" )->GetRect() );
        ASSERT_TRUE( GUI::CRect( 330, 290, 80, 20 ) == form.GetWidget( "Cancel" )->GetRect() );
        ASSERT_TRUE( form.Hide() && !form.IsVisible() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layout: Saving keeps unknown nodes and hints, writes only what differs from the defaults and follows added and removed widgets" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        CORE::CDataNode layout;
        MakeMenuLayout( layout );
        GUI::CForm form( context );
        ASSERT_TRUE( form.LoadLayout( layout ) );

        // what an editor would do
        form.GetWidget( "Title" )->SetProperty( "Text", "Welcome" );
        form.GetWidget( "Title" )->SetProperty( "TextColour", "255,255,255,255" );    // the default: not written
        form.GetWidget( "Ok" )->SetProperty( "Visible", "false" );
        context->DestroyWidget( form.GetWidget( "Cancel" ) );
        GUI::CWidget* added = context->CreateWidget( "Checkbox", "Music", form.GetRootWidget() );
        added->SetProperty( "Checked", "true" );
        added->SetProperty( "Position", "0,10 0,110" );

        CORE::CDataNode saved;
        ASSERT_TRUE( form.SaveLayout( saved ) );
        ASSERT_TRUE( "GUILayout" == saved.GetName() && "1" == saved.GetAttributeValue( "version" ).AsString() );
        ASSERT_TRUE( "GUILayout" == saved.GetAttributeValue( "format" ).AsString() );
        const CORE::CDataNode* menu = FindWidgetNode( saved, "Menu" );
        ASSERT_TRUE( GUCEF_NULL != menu && GUCEF_NULL != menu->FindChild( "EditorInfo" ) );
        ASSERT_TRUE( "0.5,-100 0.5,-80" == menu->GetAttributeValue( "position" ).AsString() );
        const CORE::CDataNode* title = FindWidgetNode( saved, "Title" );
        ASSERT_TRUE( "Welcome" == FindPropertyNode( *title, "Text" )->GetAttributeValue( "value" ).AsString() );
        ASSERT_TRUE( GUCEF_NULL == FindPropertyNode( *title, "TextColour" ) );
        ASSERT_TRUE( GUCEF_NULL != FindPropertyNode( *title, "FutureProperty" ) );
        const CORE::CDataNode* ok = FindWidgetNode( saved, "Ok" );
        ASSERT_TRUE( "false" == FindPropertyNode( *ok, "Visible" )->GetAttributeValue( "value" ).AsString() );
        ASSERT_TRUE( 2 == ok->FindChildrenOfType( "BackendHints" ).size() );
        ASSERT_TRUE( GUCEF_NULL == FindPropertyNode( *ok, "NullOnly" ) );
        ASSERT_TRUE( GUCEF_NULL == FindWidgetNode( saved, "Cancel" ) );
        const CORE::CDataNode* music = FindWidgetNode( saved, "Music" );
        ASSERT_TRUE( GUCEF_NULL != music && "Checkbox" == music->GetAttributeValue( "type" ).AsString() );
        ASSERT_TRUE( "true" == FindPropertyNode( *music, "Checked" )->GetAttributeValue( "value" ).AsString() );
        const CORE::CDataNode* choices = FindWidgetNode( saved, "Choices" );
        ASSERT_TRUE( "New game\nLoad game" == FindPropertyNode( *choices, "Items" )->GetAttributeValue( "value" ).AsString() );

        // loading what was saved gives the same widgets
        GUI::CForm again( context );
        ASSERT_TRUE( again.LoadLayout( saved ) );
        ASSERT_TRUE( "Welcome" == again.GetWidget( "Title" )->GetProperty( "Text" ) );
        ASSERT_FALSE( again.GetWidget( "Ok" )->IsVisible() );
        ASSERT_TRUE( "yes" == again.GetWidget( "Ok" )->GetProperty( "NullOnly" ) );
        ASSERT_TRUE( GUCEF_NULL == again.GetWidget( "Cancel" ) );
        ASSERT_TRUE( again.GetWidgetAs< GUI::CCheckbox >( "Music" )->GetCheckedState() );
        ASSERT_TRUE( "Load game" == again.GetWidgetAs< GUI::CListbox >( "Choices" )->GetItems()[ 1 ] );

        // and saving that again changes nothing
        CORE::CDataNode savedAgain;
        ASSERT_TRUE( again.SaveLayout( savedAgain ) );
        ASSERT_TRUE( DeepEqual( savedAgain, saved ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layout: Validation finds what does not fit the widget types, simplify and migrate tidy layouts" )
    try
    {
        CORE::CDataNode layout;
        MakeMenuLayout( layout );
        GUI::CFormLayout::TStringVector problems;
        // FutureProperty is not known to the Label type
        ASSERT_FALSE( GUI::CFormLayout::Validate( layout, problems ) );
        ASSERT_TRUE( 1 == problems.size() && problems[ 0 ].HasSubstr( "FutureProperty" ) >= 0 );

        CORE::CDataNode bad( "GUILayout" );
        CORE::CDataNode* root = AddWidget( bad, "Panel", "Root", "x,y" );
        AddProperty( *root, "Visible", "sometimes" );
        AddWidget( *root, "Spaceship", "Odd" );
        AddWidget( *root, "Label", "Twice" );
        AddWidget( *root, "Label", "Twice" );
        problems.clear();
        ASSERT_FALSE( GUI::CFormLayout::Validate( bad, problems ) );
        ASSERT_TRUE( 4 == problems.size() );

        problems.clear();
        CORE::CDataNode empty( "GUILayout" );
        ASSERT_FALSE( GUI::CFormLayout::Validate( empty, problems ) );
        problems.clear();
        CORE::CDataNode notALayout( "Something" );
        ASSERT_FALSE( GUI::CFormLayout::Validate( notALayout, problems ) );

        // defaults go, set values stay
        CORE::CDataNode verbose( "GUILayout" );
        CORE::CDataNode* panel = AddWidget( verbose, "Panel", "P", "0,0 0,0" );
        AddProperty( *panel, "Visible", "true" );
        AddProperty( *panel, "Enabled", "yes" );
        AddProperty( *panel, "BorderWidth", "3" );
        CORE::CDataNode* label = AddWidget( *panel, "Label", "L" );
        AddProperty( *label, "HTextAlign", "left" );
        ASSERT_TRUE( 4 == GUI::CFormLayout::Simplify( verbose ) );
        ASSERT_TRUE( 1 == panel->FindChildrenOfType( "Property" ).size() && GUCEF_NULL == panel->GetAttribute( "position" ) );
        ASSERT_TRUE( 0 == label->FindChildrenOfType( "Property" ).size() );

        // a layout without a version is taken as version 1; a newer one is left alone
        GUI::CFormLayout::TStringVector notes;
        ASSERT_TRUE( GUI::CFormLayout::Migrate( verbose, notes ) && 1 == notes.size() );
        ASSERT_TRUE( "1" == verbose.GetAttributeValue( "version" ).AsString() );
        verbose.SetAttribute( "version", "99" );
        notes.clear();
        ASSERT_FALSE( GUI::CFormLayout::Migrate( verbose, notes ) );
        problems.clear();
        GUI::CFormLayout::Validate( verbose, problems );
        ASSERT_TRUE( 1 == problems.size() && problems[ 0 ].HasSubstr( "newer" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layout: The same layout is written and read as XML and as JSON" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    if ( !LoadLayoutCodecs() )
        GUCEF_TESTFW_SKIP( "codec plugins are not loaded at run time in this WebAssembly build" );
    #endif
    try
    {
        ASSERT_TRUE( LoadLayoutCodecs() );
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        CORE::CDataNode layout;
        MakeMenuLayout( layout );
        GUI::CForm form( context );
        ASSERT_TRUE( form.LoadLayout( layout ) );

        const char* files[] = { "gucefGUI_TestLayout.xml", "gucefGUI_TestLayout.json" };
        for ( UInt32 f=0; f<2; ++f )
        {
            CORE::CString path = TempPath( files[ f ] );
            ASSERT_TRUE( form.SaveLayout( path ) );
            GUI::CForm loaded( context );
            ASSERT_TRUE( loaded.LoadLayout( path ) );
            ASSERT_TRUE( 4 == loaded.GetRootWidget()->GetChildren().size() );
            ASSERT_TRUE( "Main menu" == loaded.GetWidget( "Title" )->GetProperty( "Text" ) );
            ASSERT_TRUE( "New game" == loaded.GetWidgetAs< GUI::CListbox >( "Choices" )->GetItems()[ 0 ] );
            ASSERT_TRUE( 2 == loaded.GetWidgetAs< GUI::CListbox >( "Choices" )->GetItems().size() );
            ASSERT_TRUE( "0,0,64,255" == loaded.GetRootWidget()->GetProperty( "BackgroundColour" ) );
            ASSERT_TRUE( "yes" == loaded.GetWidget( "Ok" )->GetProperty( "NullOnly" ) );
            CORE::CDataNode reread;
            ASSERT_TRUE( GUI::CFormLayout::LoadTree( path, reread ) );
            ASSERT_TRUE( GUCEF_NULL != GUI::CFormLayout::FindLayoutRoot( reread ) );
            ASSERT_TRUE( GUCEF_NULL != FindWidgetNode( *GUI::CFormLayout::FindLayoutRoot( reread ), "Menu" )->FindChild( "EditorInfo" ) );
            CORE::DeleteFile( path );
        }

        // a file that is no gucefGUI layout goes to the driver's own loader; the null driver has none
        CORE::CString other = TempPath( "gucefGUI_NotALayout.xml" );
        CORE::CDataNode something( "Something" );
        ASSERT_TRUE( GUI::CFormLayout::SaveTree( something, other ) );
        GUI::CForm native( context );
        ASSERT_FALSE( native.LoadLayout( other ) );
        ASSERT_TRUE( GUCEF_NULL == native.GetRootWidget() );
        CORE::DeleteFile( other );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
