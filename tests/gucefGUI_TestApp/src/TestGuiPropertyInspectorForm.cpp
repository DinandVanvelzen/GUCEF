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

/** A tree of values of all kinds, as a material of some tool would have */
void
MakeMaterial( CORE::CDataNode& root )
{
    root.SetName( "Material" );
    root.SetAttribute( "Name", "Rock" );
    root.SetAttribute( "Roughness", CORE::CVariant( 0.5f ) );
    root.SetAttribute( "Metallic", CORE::CVariant( true ) );
    root.SetAttribute( "Layers", CORE::CVariant( (Int32) 3 ) );
    root.SetAttribute( "Tint", "255,128,0,255" );
    root.SetAttribute( "Offset", "0.5, 0.25" );
    root.SetAttribute( "Shader", "standard" );
    root.SetAttribute( "Texture", "textures/rock.png" );
    root.SetAttribute( "Note", "no descriptor" );
    CORE::CDataNode* surface = root.AddChild( "Surface" );
    surface->SetAttribute( "Scale", CORE::CVariant( 2.0f ) );
    CORE::CDataNode* detail = surface->AddChild( "Detail" );
    detail->SetAttribute( "Strength", CORE::CVariant( 0.1f ) );
    surface->AddChildWithValue( "Layer", CORE::CVariant( "A" ) );
    surface->AddChildWithValue( "Layer", CORE::CVariant( "B" ) );
}

void
DescribeMaterial( GUI::CPropertyInspectorForm& form )
{
    CORE::CString::StringVector shaders;
    shaders.push_back( "standard" );
    shaders.push_back( "toon" );
    shaders.push_back( "unlit" );
    form.SetDescriptor( "@Name", GUI::CPropertyRowDescriptor::Text() );
    form.SetDescriptor( "@Roughness", GUI::CPropertyRowDescriptor::NumberInRange( 0.0, 1.0, 2 ).WithTooltip( "How rough the surface is" ) );
    form.SetDescriptor( "@Metallic", GUI::CPropertyRowDescriptor::Boolean() );
    form.SetDescriptor( "@Layers", GUI::CPropertyRowDescriptor::NumberInRange( 1.0, 8.0, 0 ) );
    form.SetDescriptor( "@Tint", GUI::CPropertyRowDescriptor::Colour() );
    form.SetDescriptor( "@Offset", GUI::CPropertyRowDescriptor::Vector( 2 ) );
    form.SetDescriptor( "@Shader", GUI::CPropertyRowDescriptor::Choice( shaders ) );
    form.SetDescriptor( "@Texture", GUI::CPropertyRowDescriptor::Path( false ).WithLabel( "Texture file" ) );
    form.SetDescriptor( "Surface@Scale", GUI::CPropertyRowDescriptor::NumberInRange( 0.0, 10.0, 1 ).WithUnit( "m" ) );
    form.SetDescriptor( "Surface/Detail@Strength", GUI::CPropertyRowDescriptor::Number( 3 ).AsReadOnly() );
    form.SetDescriptor( "Surface/Layer", GUI::CPropertyRowDescriptor::Text() );
    form.SetDescriptor( "Surface/Layer[1]", GUI::CPropertyRowDescriptor::Text() );
}

/** The row of a path in the grid, -1 when it is not shown */
Int32
RowOf( GUI::CPropertyInspectorForm& form, const CORE::CString& path )
{
    return form.FindRow( path );
}

CORE::CString
CellText( GUI::CGridView* grid, Int32 row, Int32 column )
{
    GUI::SCellContent content;
    if ( !grid->GetCellContent( row, column, content ) )
        return CORE::CString::Empty;
    return content.text;
}

class CInspectorScene : public CFormScene
{
    public:

    GUI::CPropertyInspectorForm form;
    CORE::CDataNode material;

    explicit CInspectorScene( GUI::CGUIContext* context ) : CFormScene(), form( context ), material()
    {
        GUI::CWidget* screen = MakeScreen( context );
        form.Build( screen );
        MakeMaterial( material );
        DescribeMaterial( form );
        form.SetData( &material );
        form.SetGroupExpanded( "Surface/Detail", false );
        form.SelectPath( "@Roughness" );
    }
};

CFormScene*
BuildInspectorScene( GUI::CGUIContext* context )
{
    return new CInspectorScene( context );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiPropertyInspectorFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.PropertyInspectorForm" );

    GUCEF_TESTFW_TESTCASE( "Inspector: A descriptor says what an editor needs: its range, choices, text forms and unit, and what it accepts" )
    try
    {
        const GUI::CPropertyRowDescriptor roughness = GUI::CPropertyRowDescriptor::NumberInRange( 0.0, 1.0, 2 );
        CORE::CString problem;
        ASSERT_TRUE( roughness.Validate( "0.5", problem ) && problem.IsNULLOrEmpty() );
        ASSERT_FALSE( roughness.Validate( "1.5", problem ) );
        ASSERT_TRUE( problem.HasSubstr( "at most" ) >= 0 );
        ASSERT_FALSE( roughness.Validate( "-1", problem ) );
        ASSERT_FALSE( roughness.Validate( "abc", problem ) );
        ASSERT_FALSE( roughness.Validate( "", problem ) );
        ASSERT_TRUE( "0.50" == roughness.Normalize( "0.5" ) && "0.50" == roughness.Normalize( " .5 " ) );
        ASSERT_TRUE( "0.5 m" == GUI::CPropertyRowDescriptor::Number( 1 ).WithUnit( "m" ).FormatForDisplay( "0.5" ) );
        ASSERT_TRUE( GUI::CPropertyRowDescriptor::Number( 1 ).WithUnit( "m" ).Validate( "0.5 m", problem ) );

        const GUI::CPropertyRowDescriptor whole = GUI::CPropertyRowDescriptor::NumberInRange( 1.0, 8.0, 0 );
        ASSERT_TRUE( whole.Validate( "3", problem ) && "3" == whole.Normalize( "3" ) );
        ASSERT_FALSE( whole.Validate( "2.5", problem ) );
        ASSERT_TRUE( problem.HasSubstr( "whole" ) >= 0 );

        ASSERT_TRUE( GUI::CPropertyRowDescriptor::Boolean().Validate( "yes", problem ) && "true" == GUI::CPropertyRowDescriptor::Boolean().Normalize( "yes" ) );
        ASSERT_FALSE( GUI::CPropertyRowDescriptor::Boolean().Validate( "maybe", problem ) );
        CORE::CString::StringVector options;
        options.push_back( "a" );
        options.push_back( "b" );
        ASSERT_TRUE( GUI::CPropertyRowDescriptor::Choice( options ).Validate( "b", problem ) );
        ASSERT_FALSE( GUI::CPropertyRowDescriptor::Choice( options ).Validate( "c", problem ) );
        ASSERT_TRUE( GUI::CPropertyRowDescriptor::Colour().Validate( "#ff8800", problem ) && "255,136,0,255" == GUI::CPropertyRowDescriptor::Colour().Normalize( "#ff8800" ) );
        ASSERT_FALSE( GUI::CPropertyRowDescriptor::Colour().Validate( "reddish", problem ) );
        const GUI::CPropertyRowDescriptor vector3 = GUI::CPropertyRowDescriptor::Vector( 3 );
        ASSERT_TRUE( vector3.Validate( "1,2,3", problem ) && "1, 2, 3" == vector3.Normalize( "1,2,3" ) );
        ASSERT_FALSE( vector3.Validate( "1,2", problem ) );
        ASSERT_FALSE( vector3.Validate( "1,x,3", problem ) );
        ASSERT_TRUE( 2 == GUI::CPropertyRowDescriptor::Vector( 1 ).componentCount && 4 == GUI::CPropertyRowDescriptor::Vector( 9 ).componentCount );
        ASSERT_TRUE( GUI::CPropertyRowDescriptor::Path( true ).pathIsDirectory && GUI::CPropertyRowDescriptor::Text().Validate( "anything", problem ) );
        ASSERT_TRUE( GUI::CPropertyRowDescriptor::Text().AsReadOnly().readOnly && !GUI::CPropertyRowDescriptor::Text().readOnly );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Inspector: The form builds as a window and in the place of a widget, and shows the tree as rows of groups and values with their paths" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CPropertyInspectorForm form( context );
        ASSERT_TRUE( form.Build( "Material" ) && form.IsVisible() );
        ASSERT_TRUE( "Material" == form.GetRootWidget()->GetProperty( "Title" ) && GUCEF_NULL != form.GetGrid() && GUCEF_NULL != form.GetPathField() );
        const char* names[] = { "Properties", "PathEditor", "Apply", "Revert" };
        for ( UInt32 i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
            ASSERT_TRUE( HasWidget( form, names[ i ] ) );

        CORE::CDataNode material;
        MakeMaterial( material );
        DescribeMaterial( form );
        form.SetData( &material );
        context->RenderFrame( 16 );
        ASSERT_TRUE( 15 == form.GetRowCount() );
        const char* paths[] = { "@Layers", "@Metallic", "@Name", "@Note", "@Offset", "@Roughness", "@Shader", "@Texture", "@Tint",
                                "Surface", "Surface@Scale", "Surface/Detail", "Surface/Detail@Strength", "Surface/Layer", "Surface/Layer[1]" };
        for ( UInt32 i=0; i<15; ++i )
            ASSERT_TRUE( paths[ i ] == form.GetRowPath( i ) && (Int32) i == form.FindRow( paths[ i ] ) );
        ASSERT_TRUE( form.IsGroupRow( 9 ) && form.IsGroupRow( 11 ) && !form.IsGroupRow( 0 ) && !form.IsGroupRow( 14 ) );
        ASSERT_TRUE( -1 == form.FindRow( "Nothing" ) && form.GetRowPath( 99 ).IsNULLOrEmpty() );

        // the first column names the rows (a label of the descriptor, indented below a group), the second shows the value by its kind
        GUI::CGridView* grid = form.GetGrid();
        ASSERT_TRUE( 15 == grid->GetRowCount() && 2 == grid->GetColumnCount() && grid->IsCellMode() );
        ASSERT_TRUE( "Layers" == CellText( grid, 0, 0 ) && "3" == CellText( grid, 0, 1 ) );
        ASSERT_TRUE( "Texture file" == CellText( grid, 7, 0 ) && "textures/rock.png" == CellText( grid, 7, 1 ) );
        ASSERT_TRUE( "- Surface" == CellText( grid, 9, 0 ) && CellText( grid, 9, 1 ).IsNULLOrEmpty() );
        ASSERT_TRUE( "  Scale" == CellText( grid, 10, 0 ) && "2.0 m" == CellText( grid, 10, 1 ) );
        ASSERT_TRUE( "  - Detail" == CellText( grid, 11, 0 ) && "    Strength" == CellText( grid, 12, 0 ) && "0.100" == CellText( grid, 12, 1 ) );
        ASSERT_TRUE( "  Layer[1]" == CellText( grid, 14, 0 ) && "B" == CellText( grid, 14, 1 ) );
        ASSERT_TRUE( "0.50" == CellText( grid, 5, 1 ) && "255,128,0,255" == CellText( grid, 8, 1 ) && "0.5, 0.25" == CellText( grid, 4, 1 ) );
        GUI::SCellContent check;
        ASSERT_TRUE( grid->GetCellContent( 1, 1, check ) && GUI::SCellContent::KIND_CHECK_ON == check.kind );
        ASSERT_TRUE( "no descriptor" == CellText( grid, 3, 1 ) );

        // values by path
        ASSERT_TRUE( "Rock" == form.GetValue( "@Name" ) && "2.0" == form.GetValue( "Surface@Scale" ) && "A" == form.GetValue( "Surface/Layer" ) );
        ASSERT_TRUE( form.GetValue( "Surface" ).IsNULLOrEmpty() && form.GetValue( "@Nothing" ).IsNULLOrEmpty() );
        ASSERT_TRUE( NULL != form.GetDescriptor( "@Name" ) && NULL == form.GetDescriptor( "@Note" ) );

        // in the place of a widget of another form
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* placeholder = context->CreateWidget( "Panel", "Place", screen );
        placeholder->SetProperty( "Size", "0,500 0,400" );
        GUI::CPropertyInspectorForm inPlace;
        ASSERT_FALSE( inPlace.Build( static_cast< GUI::CWidget* >( GUCEF_NULL ) ) );
        ASSERT_TRUE( inPlace.Build( placeholder ) && placeholder == inPlace.GetRootWidget()->GetParentWidget() );
        inPlace.SetDataCopy( material );
        ASSERT_TRUE( 15 == inPlace.GetRowCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Inspector: Values change by descriptor in the copy, in the type they had, with PropertyChangedEvent; Apply writes them back, Revert takes the source again" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CPropertyInspectorForm form( context );
        ASSERT_TRUE( form.Build() );
        CORE::CDataNode material;
        MakeMaterial( material );
        DescribeMaterial( form );
        form.SetData( &material );
        CEventRecorder changed( &form, GUI::CPropertyInspectorForm::PropertyChangedEvent );
        CEventRecorder applied( &form, GUI::CPropertyInspectorForm::AppliedEvent );
        ASSERT_FALSE( form.IsModified() );
        ASSERT_FALSE( form.GetWidget( "Apply" )->IsEnabled() );

        CORE::CString problem;
        ASSERT_TRUE( form.SetValue( "@Roughness", "0.75", &problem ) );
        ASSERT_TRUE( 1 == changed.count );
        GUI::CPropertyInspectorForm::TPropertyChange change = changed.Last< GUI::CPropertyInspectorForm::TPropertyChangedEventData >()->GetData();
        ASSERT_TRUE( "@Roughness" == change.path && "0.50" == change.oldValue && "0.75" == change.newValue );
        ASSERT_TRUE( form.IsModified() && form.GetWidget( "Apply" )->IsEnabled() && form.GetWidget( "Revert" )->IsEnabled() );

        // a number stays a number, a bool a bool
        ASSERT_TRUE( form.GetData().GetAttributeValue( "Roughness" ).IsFloat() && form.GetData().GetAttributeValue( "Roughness" ).AsFloat32() == 0.75f );
        ASSERT_TRUE( form.SetValue( "@Layers", "5" ) && form.GetData().GetAttributeValue( "Layers" ).IsInteger() && 5 == form.GetData().GetAttributeValue( "Layers" ).AsInt32() );
        ASSERT_TRUE( form.SetValue( "@Metallic", "no" ) && form.GetData().GetAttributeValue( "Metallic" ).IsBoolean() && !form.GetData().GetAttributeValue( "Metallic" ).AsBool() );
        ASSERT_TRUE( "false" == form.GetValue( "@Metallic" ) );
        ASSERT_TRUE( form.SetValue( "@Name", "Basalt" ) && "Basalt" == form.GetData().GetAttributeValue( "Name" ).AsString() );
        ASSERT_TRUE( form.SetValue( "@Shader", "toon" ) && form.SetValue( "@Tint", "#102030" ) && "16,32,48,255" == form.GetValue( "@Tint" ) );
        ASSERT_TRUE( form.SetValue( "@Offset", "1,2" ) && "1, 2" == form.GetValue( "@Offset" ) );
        ASSERT_TRUE( form.SetValue( "Surface/Layer[1]", "C" ) && "C" == form.GetValue( "Surface/Layer[1]" ) );
        ASSERT_TRUE( form.SetValue( "Surface@Scale", "3.5 m" ) && "3.5" == form.GetValue( "Surface@Scale" ) && 3.5f == form.GetData().FindChild( "Surface" )->GetAttributeValue( "Scale" ).AsFloat32() );
        const UInt32 events = changed.count;

        // refusals say why, change nothing and notify nothing
        ASSERT_FALSE( form.SetValue( "@Roughness", "2", &problem ) );
        ASSERT_TRUE( problem.HasSubstr( "at most" ) >= 0 && "0.75" == form.GetValue( "@Roughness" ) );
        ASSERT_FALSE( form.SetValue( "@Layers", "2.5", &problem ) );
        ASSERT_FALSE( form.SetValue( "@Shader", "glass", &problem ) );
        ASSERT_FALSE( form.SetValue( "@Tint", "nope", &problem ) );
        ASSERT_FALSE( form.SetValue( "@Offset", "1,2,3", &problem ) );
        ASSERT_FALSE( form.SetValue( "@Note", "x", &problem ) );
        ASSERT_TRUE( problem.HasSubstr( "cannot be changed" ) >= 0 );
        ASSERT_FALSE( form.SetValue( "Surface/Detail@Strength", "0.5", &problem ) );
        ASSERT_TRUE( problem.HasSubstr( "read only" ) >= 0 );
        ASSERT_FALSE( form.SetValue( "Surface", "x", &problem ) );
        ASSERT_FALSE( form.SetValue( "@Missing", "x", &problem ) );
        ASSERT_TRUE( events == changed.count );
        ASSERT_TRUE( form.SetValue( "@Name", "Basalt" ) && events == changed.count );        // the same value: no change

        // the source is untouched until the copy is applied
        ASSERT_TRUE( "Rock" == material.GetAttributeValue( "Name" ).AsString() && 0.5f == material.GetAttributeValue( "Roughness" ).AsFloat32() );
        ASSERT_TRUE( form.Apply() && 1 == applied.count );
        ASSERT_TRUE( "Basalt" == material.GetAttributeValue( "Name" ).AsString() && 0.75f == material.GetAttributeValue( "Roughness" ).AsFloat32() );
        ASSERT_TRUE( 5 == material.GetAttributeValue( "Layers" ).AsInt32() && !material.GetAttributeValue( "Metallic" ).AsBool() );
        ASSERT_FALSE( form.IsModified() || form.GetWidget( "Apply" )->IsEnabled() );

        // revert: another change goes, the source is what is shown again
        ASSERT_TRUE( form.SetValue( "@Name", "Granite" ) && form.IsModified() );
        ASSERT_TRUE( form.Revert() && "Basalt" == form.GetValue( "@Name" ) && !form.IsModified() );
        ASSERT_TRUE( 1 == applied.count );

        // a click on the buttons
        context->RenderFrame( 16 );
        ASSERT_TRUE( form.SetValue( "@Name", "Marble" ) );
        ClickWidget( context, form.GetWidget( "Apply" ) );
        ASSERT_TRUE( 2 == applied.count && "Marble" == material.GetAttributeValue( "Name" ).AsString() );
        ASSERT_TRUE( form.SetValue( "@Name", "Slate" ) );
        ClickWidget( context, form.GetWidget( "Revert" ) );
        ASSERT_TRUE( "Marble" == form.GetValue( "@Name" ) && "Marble" == material.GetAttributeValue( "Name" ).AsString() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Inspector: Live editing writes every change to the source at once; a copy without a source has nothing to apply to" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CPropertyInspectorForm form( context );
        ASSERT_TRUE( form.Build() );
        CORE::CDataNode material;
        MakeMaterial( material );
        DescribeMaterial( form );
        form.SetData( &material );
        CEventRecorder applied( &form, GUI::CPropertyInspectorForm::AppliedEvent );
        form.SetLiveEditing( true );
        ASSERT_TRUE( form.IsLiveEditing() );
        ASSERT_TRUE( form.SetValue( "@Name", "Live" ) );
        ASSERT_TRUE( "Live" == material.GetAttributeValue( "Name" ).AsString() && 1 == applied.count && !form.IsModified() );
        ASSERT_FALSE( form.GetWidget( "Apply" )->IsEnabled() );
        form.SetLiveEditing( false );
        ASSERT_TRUE( form.SetValue( "@Name", "Later" ) && "Live" == material.GetAttributeValue( "Name" ).AsString() );
        form.SetLiveEditing( true );          // what waited goes now
        ASSERT_TRUE( "Later" == material.GetAttributeValue( "Name" ).AsString() && 2 == applied.count );

        GUI::CPropertyInspectorForm copyForm( context );
        ASSERT_TRUE( copyForm.Build() );
        DescribeMaterial( copyForm );
        copyForm.SetDataCopy( material );
        ASSERT_TRUE( copyForm.SetValue( "@Name", "Copy" ) && "Copy" == copyForm.GetData().GetAttributeValue( "Name" ).AsString() );
        ASSERT_FALSE( copyForm.Apply() || copyForm.Revert() );
        ASSERT_TRUE( "Later" == material.GetAttributeValue( "Name" ).AsString() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Inspector: The user edits values in the grid with the editor of their kind, and the changes are notified and routed as changes of the user" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CPropertyInspectorForm form( context );
        ASSERT_TRUE( form.Build() );
        CORE::CDataNode material;
        MakeMaterial( material );
        DescribeMaterial( form );
        form.SetData( &material );
        context->RenderFrame( 16 );
        GUI::CGridView* grid = form.GetGrid();
        CEventRecorder changed( &form, GUI::CPropertyInspectorForm::PropertyChangedEvent );
        CEventRecorder routed( grid, GUI::CWidget::ValueChangedEvent );
        CEventRecorder refused( grid, GUI::CGridView::CellEditRefusedEvent );
        const UInt32 routedBefore = routed.count;

        // a number: typing on the cell opens the editor with what is typed, Enter commits
        ClickRow( context, grid, RowOf( form, "@Roughness" ), 1 );
        ASSERT_TRUE( "@Roughness" == form.GetSelectedPath() );
        context->SetFocus( grid );
        context->InjectText( "0.8" );
        ASSERT_TRUE( grid->IsEditing() );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_FALSE( grid->IsEditing() );
        ASSERT_TRUE( "0.80" == form.GetValue( "@Roughness" ) && 1 == changed.count );
        ASSERT_TRUE( 0.8f == form.GetData().GetAttributeValue( "Roughness" ).AsFloat32() );
        ASSERT_TRUE( routed.count > routedBefore );
        bool routedByPath = false;
        for ( size_t i=0; i<routed.data.size(); ++i )
        {
            GUI::CValueChangedEventData* data = static_cast< GUI::CValueChangedEventData* >( routed.data[ i ] );
            if ( GUCEF_NULL != data && "Value:@Roughness" == data->GetPropertyName() && "0.80" == data->newValue )
                routedByPath = true;
        }
        ASSERT_TRUE( routedByPath );
        ASSERT_FALSE( grid->IsPropertySet( CORE::CString( "Value:@Roughness" ) ) );        // nothing is left on the grid, a layout stays clean
        ASSERT_TRUE( "0.80" == CellText( grid, RowOf( form, "@Roughness" ), 1 ) );

        // a value out of its range is refused with the reason, the editor stays open, Escape gives it up
        ClickRow( context, grid, RowOf( form, "@Roughness" ), 1 );
        context->InjectText( "7" );
        ASSERT_TRUE( grid->IsEditing() );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( grid->IsEditing() && refused.count >= 1 && "0.80" == form.GetValue( "@Roughness" ) );
        ASSERT_TRUE( grid->GetCellEditor()->HasError() && grid->GetCellEditor()->GetError().HasSubstr( "at most" ) >= 0 );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ASSERT_FALSE( grid->IsEditing() );
        ASSERT_TRUE( 1 == changed.count );

        // a bool is a check box: Space flips it
        ASSERT_TRUE( form.SelectPath( "@Metallic" ) && "true" == form.GetValue( "@Metallic" ) );
        context->SetFocus( grid );
        PressKey( context, INPUT::KEYCODE_SPACE );
        ASSERT_TRUE( "false" == form.GetValue( "@Metallic" ) && 2 == changed.count && !grid->IsEditing() );
        ASSERT_TRUE( "@Metallic" == changed.Last< GUI::CPropertyInspectorForm::TPropertyChangedEventData >()->GetData().path );

        // and a click on the check box does the same
        ClickRow( context, grid, RowOf( form, "@Metallic" ), 1 );
        ASSERT_TRUE( "true" == form.GetValue( "@Metallic" ) && 3 == changed.count );
        PressKey( context, INPUT::KEYCODE_SPACE );
        ASSERT_TRUE( "false" == form.GetValue( "@Metallic" ) && 4 == changed.count );

        // a choice is a list: F2 opens it, a letter picks, Enter commits
        ClickRow( context, grid, RowOf( form, "@Shader" ), 1 );
        context->SetFocus( grid );
        PressKey( context, INPUT::KEYCODE_F2 );
        ASSERT_TRUE( grid->IsEditing() && GUI::CGridCellEditor::KIND_CHOICE == grid->GetCellEditor()->GetKind() && 3 == grid->GetCellEditor()->GetChoices().size() );
        context->InjectText( "u" );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( "unlit" == form.GetValue( "@Shader" ) && 5 == changed.count );

        // a whole number refuses a fraction before it reaches the form; the vector and the colour are text with a form
        ClickRow( context, grid, RowOf( form, "@Layers" ), 1 );
        context->InjectText( "2.5" );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( grid->IsEditing() && "3" == form.GetValue( "@Layers" ) );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ClickRow( context, grid, RowOf( form, "@Offset" ), 1 );
        context->InjectText( "9" );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( grid->IsEditing() && grid->GetCellEditor()->HasError() && "0.5, 0.25" == form.GetValue( "@Offset" ) );
        context->InjectText( "," );
        context->InjectText( "4" );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( "9, 4" == form.GetValue( "@Offset" ) && !grid->IsEditing() );
        ClickRow( context, grid, RowOf( form, "@Tint" ), 1 );
        context->InjectText( "1,2,3" );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( "1,2,3,255" == form.GetValue( "@Tint" ) );

        // values without a descriptor and read only values cannot be edited, groups neither
        const UInt32 count = changed.count;
        ClickRow( context, grid, RowOf( form, "@Note" ), 1 );
        context->InjectText( "x" );
        ASSERT_FALSE( grid->IsEditing() );
        ClickRow( context, grid, RowOf( form, "Surface/Detail@Strength" ), 1 );
        context->InjectText( "1" );
        ASSERT_FALSE( grid->IsEditing() );
        ASSERT_TRUE( count == changed.count && "no descriptor" == form.GetValue( "@Note" ) );

        // the unit does not stop a number from being typed
        ClickRow( context, grid, RowOf( form, "Surface@Scale" ), 1 );
        context->InjectText( "4" );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( "4.0" == form.GetValue( "Surface@Scale" ) && "4.0 m" == CellText( grid, RowOf( form, "Surface@Scale" ), 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Inspector: Groups fold by a click and by keys, and also by the function, below groups as well; the cursor stays with its row" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CPropertyInspectorForm form( context );
        ASSERT_TRUE( form.Build() );
        CORE::CDataNode material;
        MakeMaterial( material );
        DescribeMaterial( form );
        form.SetData( &material );
        context->RenderFrame( 16 );
        GUI::CGridView* grid = form.GetGrid();
        ASSERT_TRUE( form.IsGroupExpanded( "Surface" ) && form.IsGroupExpanded( "Surface/Detail" ) && !form.IsGroupExpanded( "@Name" ) );

        // a click on the group row folds it
        ClickRow( context, grid, 9, 0 );
        ASSERT_FALSE( form.IsGroupExpanded( "Surface" ) );
        ASSERT_TRUE( 10 == form.GetRowCount() && "+ Surface" == CellText( grid, 9, 0 ) && -1 == form.FindRow( "Surface@Scale" ) );
        ClickRow( context, grid, 9, 0 );
        ASSERT_TRUE( form.IsGroupExpanded( "Surface" ) && 15 == form.GetRowCount() );

        // the keys, with the cursor on a group row
        ASSERT_TRUE( form.SelectPath( "Surface/Detail" ) && "Surface/Detail" == form.GetSelectedPath() );
        context->SetFocus( grid );
        PressKey( context, INPUT::KEYCODE_LEFT );
        ASSERT_FALSE( form.IsGroupExpanded( "Surface/Detail" ) );
        ASSERT_TRUE( 14 == form.GetRowCount() && "Surface/Detail" == form.GetSelectedPath() );
        PressKey( context, INPUT::KEYCODE_RIGHT );
        ASSERT_TRUE( form.IsGroupExpanded( "Surface/Detail" ) && 15 == form.GetRowCount() );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_FALSE( form.IsGroupExpanded( "Surface/Detail" ) );
        PressKey( context, INPUT::KEYCODE_SPACE );
        ASSERT_TRUE( form.IsGroupExpanded( "Surface/Detail" ) );

        // a group folded under a group that folds: both come back as they were
        ASSERT_TRUE( form.SetGroupExpanded( "Surface/Detail", false ) && form.SetGroupExpanded( "Surface", false ) );
        ASSERT_TRUE( 10 == form.GetRowCount() );
        ASSERT_TRUE( form.SetGroupExpanded( "Surface", true ) );
        ASSERT_TRUE( 14 == form.GetRowCount() && !form.IsGroupExpanded( "Surface/Detail" ) && -1 == form.FindRow( "Surface/Detail@Strength" ) );
        ASSERT_FALSE( form.SetGroupExpanded( "@Name", false ) );

        // a value of a folded group is not selected
        ASSERT_FALSE( form.SelectPath( "Surface/Detail@Strength" ) );
        ASSERT_TRUE( form.SelectPath( "Surface/Layer" ) && "Surface/Layer" == form.GetSelectedPath() );
        const Int32 layerRow = form.FindRow( "Surface/Layer" );
        form.SetGroupExpanded( "Surface/Detail", true );
        ASSERT_TRUE( layerRow + 1 == form.FindRow( "Surface/Layer" ) && "Surface/Layer" == form.GetSelectedPath() );
        form.SetGroupExpanded( "Surface/Detail", false );
        ASSERT_TRUE( layerRow == form.FindRow( "Surface/Layer" ) && "Surface/Layer" == form.GetSelectedPath() );

        // a group that folds takes the cursor with it when the cursor is below it
        ASSERT_TRUE( form.SelectPath( "Surface@Scale" ) );
        form.SetGroupExpanded( "Surface", false );
        ASSERT_TRUE( form.GetSelectedPath().IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Inspector: A path value is edited with a path field below the list, which takes the place of a widget of the form" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CPropertyInspectorForm form( context );
        ASSERT_TRUE( form.Build() );
        CORE::CDataNode material;
        MakeMaterial( material );
        DescribeMaterial( form );
        form.SetData( &material );
        context->RenderFrame( 16 );
        GUI::CPathFieldForm* pathField = form.GetPathField();
        ASSERT_TRUE( GUCEF_NULL != pathField && "PathHolder" == pathField->GetRootWidget()->GetParentWidget()->GetName() );
        ASSERT_FALSE( form.GetWidget( "PathEditor" )->IsVisible() );

        CEventRecorder changed( &form, GUI::CPropertyInspectorForm::PropertyChangedEvent );
        ASSERT_TRUE( form.SelectPath( "@Texture" ) );
        ASSERT_TRUE( form.GetWidget( "PathEditor" )->IsVisible() && "textures/rock.png" == pathField->GetPath() );
        ASSERT_TRUE( "Texture file" == form.GetWidget( "PathLabel" )->GetProperty( "Text" ) );

        // typed in the line, Enter gives it to the value
        context->RenderFrame( 16 );
        TypeInto( context, pathField->GetWidget( "Path" ), "textures/sand.png", true );
        ASSERT_TRUE( "textures/sand.png" == form.GetValue( "@Texture" ) && 1 == changed.count );
        ASSERT_TRUE( "textures/sand.png" == form.GetData().GetAttributeValue( "Texture" ).AsString() );

        // chosen with the dialog of the field: its choice is the value
        const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefGUI_TestApp_InspectorPath" );
        CORE::DeleteDir( dir, true );
        ASSERT_TRUE( CORE::CreateDirs( dir ) && CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "chosen.png" ), "x" ) );
        ASSERT_TRUE( pathField->OpenDialog() );
        GUI::CFileSystemDialogForm* dialog = pathField->GetDialog();
        ASSERT_TRUE( GUCEF_NULL != dialog && dialog->NavigateTo( "This computer", dir ) );
        CORE::CString::StringVector pick;
        pick.push_back( "chosen.png" );
        ASSERT_TRUE( dialog->SelectNames( pick ) && dialog->Accept() );
        ASSERT_TRUE( form.GetValue( "@Texture" ).HasSubstr( "gucefGUI_TestApp_InspectorPath" ) >= 0 && form.GetValue( "@Texture" ).HasSubstr( "chosen.png" ) >= 0 && 2 == changed.count );
        CORE::DeleteDir( dir, true );

        // another kind of row: the strip goes; a read only path has none either
        ASSERT_TRUE( form.SelectPath( "@Name" ) );
        ASSERT_FALSE( form.GetWidget( "PathEditor" )->IsVisible() );
        form.SetDescriptor( "@Texture", GUI::CPropertyRowDescriptor::Path( false ).AsReadOnly() );
        ASSERT_TRUE( form.SelectPath( "@Texture" ) );
        ASSERT_FALSE( form.GetWidget( "PathEditor" )->IsVisible() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Inspector: The tooltip of the descriptor shows over its row, and a layout of the form saves and loads again to a form that works the same" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CPropertyInspectorForm first( context );
        ASSERT_TRUE( first.Build() );
        CORE::CDataNode material;
        MakeMaterial( material );
        DescribeMaterial( first );
        first.SetData( &material );
        context->RenderFrame( 16 );

        GUI::CGridView* grid = first.GetGrid();
        Int32 x = 0, y = 0;
        RowPoint( context, grid, first.FindRow( "@Roughness" ), 1, x, y );
        context->InjectMouseMove( x, y );
        ASSERT_TRUE( "How rough the surface is" == grid->GetProperty( GUI::EWidgetProperty_Tooltip ) );
        RowPoint( context, grid, first.FindRow( "@Name" ), 1, x, y );
        context->InjectMouseMove( x, y );
        ASSERT_TRUE( grid->GetProperty( GUI::EWidgetProperty_Tooltip ).IsNULLOrEmpty() );

        // the layout holds the widgets, not the values: nothing of an edit stays in it
        first.SetValue( "@Name", "Edited" );
        CORE::CDataNode layout;
        ASSERT_TRUE( first.SaveLayout( layout ) );
        GUI::CPropertyInspectorForm second( context );
        ASSERT_TRUE( second.LoadLayout( layout ) && second.Show() );
        ASSERT_TRUE( HasWidget( second, "Properties" ) && HasWidget( second, "Apply" ) && GUCEF_NULL != second.GetPathField() );
        ASSERT_TRUE( 0 == second.GetRowCount() );
        CORE::CDataNode other;
        MakeMaterial( other );
        DescribeMaterial( second );
        second.SetData( &other );
        ASSERT_TRUE( 15 == second.GetRowCount() && "Rock" == second.GetValue( "@Name" ) );
        context->RenderFrame( 16 );
        CEventRecorder changed( &second, GUI::CPropertyInspectorForm::PropertyChangedEvent );
        ASSERT_TRUE( second.SelectPath( "@Metallic" ) );
        context->SetFocus( second.GetGrid() );
        PressKey( context, INPUT::KEYCODE_SPACE );
        ASSERT_TRUE( 1 == changed.count && "false" == second.GetValue( "@Metallic" ) );

        // the same form loaded again: the rows and the values are there
        ASSERT_TRUE( second.LoadLayout( layout ) );
        ASSERT_TRUE( 15 == second.GetRowCount() && "false" == second.GetValue( "@Metallic" ) && GUCEF_NULL != second.GetPathField() );
        ASSERT_TRUE( second.SetValue( "@Name", "Again" ) && "Again" == second.GetGrid()->GetText( 1, second.FindRow( "@Name" ) ).SubstrToIndex( 5, true ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    RunFormRenderTests( "property inspector", &BuildInspectorScene );
    RunFormLintBaseline( "PropertyInspector", &BuildInspectorScene );
}

/*-------------------------------------------------------------------------*/
