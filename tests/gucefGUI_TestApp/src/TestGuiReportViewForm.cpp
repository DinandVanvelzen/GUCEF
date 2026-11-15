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

void
FillReport( GUI::CReportViewForm& form )
{
    form.AddEntry( GUI::CReportViewForm::SEVERITY_INFO, "textures/rock.png", "Converted", "Converted to BC7 in 0.4 s" );
    form.AddEntry( GUI::CReportViewForm::SEVERITY_WARNING, "textures/sand.png", "Not a power of two", "The size 100x60 was kept\nMips are made for it" );
    form.AddEntry( GUI::CReportViewForm::SEVERITY_ERROR, "meshes/tree.obj", "No normals", "The mesh has no normals and none can be made" );
    form.AddEntry( GUI::CReportViewForm::SEVERITY_INFO, "meshes/rock.obj", "Converted", "" );
    form.AddEntry( GUI::CReportViewForm::SEVERITY_ERROR, "textures/moss.png", "Unreadable", "End of file at 1024" );
    form.Refresh();
}

class CReportScene : public CFormScene
{
    public:

    GUI::CReportViewForm form;

    explicit CReportScene( GUI::CGUIContext* context ) : CFormScene(), form( context )
    {
        GUI::CWidget* screen = MakeScreen( context );
        form.Build( screen );
        FillReport( form );
        form.SelectEntry( 2 );
    }
};

CFormScene*
BuildReportScene( GUI::CGUIContext* context )
{
    return new CReportScene( context );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiReportViewFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.ReportViewForm" );

    GUCEF_TESTFW_TESTCASE( "Report: The form builds as a window and in the place of a widget, with the entries in a grid and a colour per severity" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CReportViewForm form( context );
        ASSERT_TRUE( form.Build( "Problems" ) && form.IsVisible() );
        ASSERT_TRUE( "Problems" == form.GetRootWidget()->GetProperty( "Title" ) );
        const char* names[] = { "ShowInfo", "ShowWarning", "ShowError", "Filter", "Entries", "Details", "Status" };
        for ( UInt32 i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
            ASSERT_TRUE( HasWidget( form, names[ i ] ) );
        FillReport( form );

        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Entries" );
        ASSERT_TRUE( 5 == grid->GetRowCount() && 4 == grid->GetColumnCount() );
        ASSERT_TRUE( "Severity" == grid->GetColumnTitle( 0 ) && "Details" == grid->GetColumnTitle( 3 ) );
        ASSERT_TRUE( "info" == grid->GetText( 0, 0 ) && "textures/sand.png" == grid->GetText( 1, 1 ) && "No normals" == grid->GetText( 2, 2 ) );
        ASSERT_TRUE( "5 of 5 shown, 2 info, 1 warning, 2 error" == form.GetWidget( "Status" )->GetProperty( "Text" ) );

        // the rows have the colour of their severity
        GUI::CColour colour;
        ASSERT_TRUE( grid->GetRowColour( 0, colour ) && colour == form.GetSeverityColour( GUI::CReportViewForm::SEVERITY_INFO ) );
        ASSERT_TRUE( grid->GetRowColour( 1, colour ) && colour == form.GetSeverityColour( GUI::CReportViewForm::SEVERITY_WARNING ) );
        ASSERT_TRUE( grid->GetRowColour( 2, colour ) && colour == form.GetSeverityColour( GUI::CReportViewForm::SEVERITY_ERROR ) );
        ASSERT_TRUE( form.GetSeverityColour( GUI::CReportViewForm::SEVERITY_INFO ) != form.GetSeverityColour( GUI::CReportViewForm::SEVERITY_ERROR ) );

        // and the painter of the grid draws a marker and the text in that colour
        context->RenderFrame( 16 );
        GUI::CDrawSurface::TCommandVector commands;
        GUI::CWidgetPainter::Paint( *grid, *context, commands );
        bool markerFound = false, textInColour = false;
        const GUI::CColour error = form.GetSeverityColour( GUI::CReportViewForm::SEVERITY_ERROR );
        for ( size_t i=0; i<commands.size(); ++i )
        {
            if ( GUI::CDrawSurface::COMMAND_FILLRECT == commands[ i ].type && commands[ i ].colour == error && 4 == commands[ i ].rect.width )
                markerFound = true;
            if ( GUI::CDrawSurface::COMMAND_TEXT == commands[ i ].type && commands[ i ].colour == error && "No normals" == commands[ i ].text )
                textInColour = true;
        }
        ASSERT_TRUE( markerFound && textInColour );

        // in the place of a widget of another form
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* placeholder = context->CreateWidget( "Panel", "Place", screen );
        placeholder->SetProperty( "Size", "0,500 0,400" );
        GUI::CReportViewForm inPlace;
        ASSERT_TRUE( inPlace.Build( placeholder ) && placeholder == inPlace.GetRootWidget()->GetParentWidget() );
        inPlace.AddEntry( GUI::CReportViewForm::SEVERITY_WARNING, "a", "b" );
        inPlace.Refresh();
        ASSERT_TRUE( 1 == inPlace.GetWidgetAs< GUI::CGridView >( "Entries" )->GetRowCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Report: Entries come as a data node list or one by one, and the grid follows at most once every refresh interval while they arrive" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CReportViewForm form( context );
        ASSERT_TRUE( form.Build() );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Entries" );

        CORE::CDataNode list( "Report" );
        CORE::CDataNode* a = list.AddChild( "Entry" );
        a->SetAttribute( "severity", "Warning" );
        a->SetAttribute( "asset", "x/y.png" );
        a->SetAttribute( "message", "Odd size" );
        a->SetAttribute( "owner", "the artist" );
        CORE::CDataNode* b = list.AddChild( "Entry" );
        b->SetAttribute( "severity", "2" );
        b->SetAttribute( "asset", "x/z.png" );
        b->SetAttribute( "message", "Missing" );
        b->SetAttribute( "details", "No such file" );
        CORE::CDataNode* c = list.AddChild( "Entry" );
        c->SetAttribute( "severity", "nonsense" );
        c->SetAttribute( "message", "Odd severity" );
        form.SetEntries( list );
        ASSERT_TRUE( 3 == form.GetEntryCount() && 3 == grid->GetRowCount() );
        ASSERT_TRUE( GUI::CReportViewForm::SEVERITY_WARNING == form.GetEntry( 0 ).severity && "the artist" == form.GetEntry( 0 ).GetValue( "owner" ) );
        ASSERT_TRUE( GUI::CReportViewForm::SEVERITY_ERROR == form.GetEntry( 1 ).severity && "No such file" == form.GetEntry( 1 ).details );
        ASSERT_TRUE( GUI::CReportViewForm::SEVERITY_INFO == form.GetEntry( 2 ).severity );

        // one by one: the counts are up to date at once, the grid after the interval
        form.ClearEntries();
        ASSERT_TRUE( 0 == grid->GetRowCount() );
        for ( UInt32 i=0; i<50; ++i )
            form.AddEntry( GUI::CReportViewForm::SEVERITY_INFO, "asset" + CORE::UInt32ToString( i ), "message" );
        ASSERT_TRUE( 50 == form.GetEntryCount() && 50 == form.GetShownCount() && 50 == form.GetEntryCount( GUI::CReportViewForm::SEVERITY_INFO ) );
        ASSERT_TRUE( 0 == grid->GetRowCount() );
        form.Update( 40 );
        ASSERT_TRUE( 0 == grid->GetRowCount() );
        form.Update( 70 );
        ASSERT_TRUE( 50 == grid->GetRowCount() );
        form.AddEntry( GUI::CReportViewForm::SEVERITY_ERROR, "late", "late" );
        ASSERT_TRUE( 50 == grid->GetRowCount() );
        context->RenderFrame( 150 );                  // the frame time of the context does it
        ASSERT_TRUE( 51 == grid->GetRowCount() );
        form.Refresh();
        ASSERT_TRUE( 51 == grid->GetRowCount() && 51 == form.GetEntryCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Report: The severity boxes and the filter line choose the entries, and the details of the selected entry are shown" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CReportViewForm form( context );
        ASSERT_TRUE( form.Build() );
        FillReport( form );
        context->RenderFrame( 16 );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Entries" );

        // a severity box of the user
        ClickWidget( context, form.GetWidget( "ShowInfo" ) );
        ASSERT_FALSE( form.IsSeverityShown( GUI::CReportViewForm::SEVERITY_INFO ) );
        ASSERT_TRUE( 3 == form.GetShownCount() && 3 == grid->GetRowCount() && "warning" == grid->GetText( 0, 0 ) );
        ASSERT_TRUE( 2 == form.GetShownEntryIndex( 1 ) && 4 == form.GetShownEntryIndex( 2 ) );
        form.SetSeverityShown( GUI::CReportViewForm::SEVERITY_WARNING, false );
        ASSERT_FALSE( form.GetWidget( "ShowWarning" )->GetBoolProperty( GUI::EWidgetProperty_Checked ) );
        ASSERT_TRUE( 2 == grid->GetRowCount() );
        form.SetSeverityShown( GUI::CReportViewForm::SEVERITY_INFO, true );
        form.SetSeverityShown( GUI::CReportViewForm::SEVERITY_WARNING, true );
        ASSERT_TRUE( 5 == grid->GetRowCount() );

        // the filter line: any column, any letter case
        TypeInto( context, form.GetWidget( "Filter" ), "ROCK", false );
        ASSERT_TRUE( "ROCK" == form.GetFilter() && 2 == form.GetShownCount() && 2 == grid->GetRowCount() );
        ASSERT_TRUE( 0 == form.GetShownEntryIndex( 0 ) && 3 == form.GetShownEntryIndex( 1 ) );
        form.SetFilter( "end of file" );
        ASSERT_TRUE( 1 == form.GetShownCount() && 4 == form.GetShownEntryIndex( 0 ) );
        ASSERT_TRUE( "end of file" == form.GetWidget( "Filter" )->GetProperty( "Text" ) );
        form.SetFilter( "" );
        ASSERT_TRUE( 5 == form.GetShownCount() );

        // the details of the selected entry
        ASSERT_TRUE( form.SelectEntry( 2 ) && 2 == form.GetSelectedEntry() );
        const CORE::CString details = form.GetWidget( "Details" )->GetProperty( "Text" );
        ASSERT_TRUE( details.HasSubstr( "No normals" ) >= 0 && details.HasSubstr( "meshes/tree.obj" ) >= 0 && details.HasSubstr( "none can be made" ) >= 0 );
        ASSERT_FALSE( form.SelectEntry( 9 ) );
        form.SetSeverityShown( GUI::CReportViewForm::SEVERITY_ERROR, false );
        ASSERT_TRUE( -1 == form.GetSelectedEntry() );       // the selected entry is not shown any more
        ASSERT_TRUE( form.GetWidget( "Details" )->GetProperty( "Text" ).IsNULLOrEmpty() );

        // a click on a row selects its entry
        form.SetSeverityShown( GUI::CReportViewForm::SEVERITY_ERROR, true );
        ClickRow( context, grid, 4 );
        ASSERT_TRUE( 4 == form.GetSelectedEntry() );
        ASSERT_TRUE( form.GetWidget( "Details" )->GetProperty( "Text" ).HasSubstr( "End of file at 1024" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Report: A double click or Enter on an entry notifies EntryActivatedEvent with its index; the keys of the grid move the selection" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CReportViewForm form( context );
        ASSERT_TRUE( form.Build() );
        FillReport( form );
        form.SetSeverityShown( GUI::CReportViewForm::SEVERITY_INFO, false );        // entries 1, 2 and 4 remain
        context->RenderFrame( 16 );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Entries" );
        CEventRecorder activated( &form, GUI::CReportViewForm::EntryActivatedEvent );

        DoubleClickRow( context, grid, 1 );
        ASSERT_TRUE( 1 == activated.count && 2 == static_cast< CORE::TCloneableInt32* >( activated.data[ 0 ] )->GetData() );
        context->SetFocus( grid );
        PressKey( context, INPUT::KEYCODE_DOWN );
        ASSERT_TRUE( 4 == form.GetSelectedEntry() );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( 2 == activated.count && 4 == static_cast< CORE::TCloneableInt32* >( activated.data[ 1 ] )->GetData() );
        PressKey( context, INPUT::KEYCODE_UP );
        PressKey( context, INPUT::KEYCODE_UP );
        ASSERT_TRUE( 1 == form.GetSelectedEntry() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Report: The column set is the application's: keys, titles and relative widths, with values of other attributes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CReportViewForm form( context );
        ASSERT_TRUE( form.Build() );
        GUI::CReportViewForm::TColumnVector columns;
        columns.push_back( GUI::CReportViewForm::TColumn( "asset", "Name", 3 ) );
        columns.push_back( GUI::CReportViewForm::TColumn( "owner", "Owner", 1 ) );
        form.SetColumns( columns );
        GUI::CReportViewForm::TEntry entry( GUI::CReportViewForm::SEVERITY_WARNING, "a.png", "message" );
        entry.values[ "owner" ] = "Sam";
        form.AddEntry( entry );
        form.Refresh();
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Entries" );
        ASSERT_TRUE( 2 == grid->GetColumnCount() && "Owner" == grid->GetColumnTitle( 1 ) );
        ASSERT_TRUE( "a.png" == grid->GetText( 0, 0 ) && "Sam" == grid->GetText( 1, 0 ) );
        const GUCEF::vector< Int32 > widths = grid->GetIntListProperty( GUI::EWidgetProperty_ColumnWidths );
        ASSERT_TRUE( 2 == widths.size() && 3 == widths[ 0 ] && 1 == widths[ 1 ] );

        // the filter looks in the columns that are there
        form.SetFilter( "sam" );
        ASSERT_TRUE( 1 == form.GetShownCount() );
        form.SetFilter( "message" );
        ASSERT_TRUE( 0 == form.GetShownCount() );

        // default columns when none are given
        form.SetColumns( GUI::CReportViewForm::TColumnVector() );
        ASSERT_TRUE( 4 == form.GetColumns().size() && "severity" == form.GetColumns()[ 0 ].key );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Report: A layout of the form saves and loads again: the new form shows its entries as the first one does" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CReportViewForm first( context );
        ASSERT_TRUE( first.Build() );
        FillReport( first );
        CORE::CDataNode layout;
        ASSERT_TRUE( first.SaveLayout( layout ) );

        GUI::CReportViewForm second( context );
        ASSERT_TRUE( second.LoadLayout( layout ) && second.Show() );
        second.AddEntry( GUI::CReportViewForm::SEVERITY_ERROR, "b", "c", "d" );
        second.Refresh();
        GUI::CGridView* grid = second.GetWidgetAs< GUI::CGridView >( "Entries" );
        ASSERT_TRUE( 1 == grid->GetRowCount() && "error" == grid->GetText( 0, 0 ) );
        context->RenderFrame( 16 );
        CEventRecorder activated( &second, GUI::CReportViewForm::EntryActivatedEvent );
        DoubleClickRow( context, grid, 0 );
        ASSERT_TRUE( 1 == activated.count );
        ClickWidget( context, second.GetWidget( "ShowError" ) );
        ASSERT_TRUE( 0 == second.GetShownCount() );

        // the same form loaded again keeps what it has
        ASSERT_TRUE( first.LoadLayout( layout ) );
        ASSERT_TRUE( 5 == first.GetWidgetAs< GUI::CGridView >( "Entries" )->GetRowCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ReportView: In a low host the entries keep rows in view and the details take what is left below the status" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLayoutLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        // the heights the report row of the asset viewer has at 900 and at 720 high
        const Int32 heights[] = { 234, 187 };
        const Int32 rows[] = { 3, 2 };
        for ( UInt32 h=0; h<2; ++h )
        {
            context->SetLogicalResolution( 992, (UInt32) heights[ h ] );
            GUI::CWidget* screen = MakeScreen( context );
            GUI::CReportViewForm form( context );
            ASSERT_TRUE( form.Build( screen ) );
            for ( UInt32 i=0; i<25; ++i )
                form.AddEntry( GUI::CReportViewForm::SEVERITY_WARNING, "asset" + CORE::UInt32ToString( i ), "a message", "details" );
            form.Refresh();
            context->FlushChanges();
            GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Entries" );
            GUI::CWidget* status = form.GetWidget( "Status" );
            GUI::CWidget* details = form.GetWidget( "Details" );
            ASSERT_TRUE( GUCEF_NULL != grid && GUCEF_NULL != status && GUCEF_NULL != details );
            if ( GUCEF_NULL == grid || GUCEF_NULL == status || GUCEF_NULL == details )
                return;
            ASSERT_TRUE( grid->GetVisibleRowCount() >= rows[ h ] );
            ASSERT_TRUE( status->GetRect().top >= grid->GetRect().Bottom() && details->GetRect().top >= status->GetRect().Bottom() );
            ASSERT_TRUE( details->GetRect().Bottom() <= heights[ h ] && details->GetRect().height >= GUITEST::LayoutLintLineHeight );

            GUI::TLayoutLintFindingVector findings;
            GUI::CLayoutLint lint;
            lint.Run( *context, GUI::CLayoutLintOptions(), findings );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ReportView at 992x" << heights[ h ] << ": " << GUI::CLayoutLint::Report( findings, 20 ) );
            UInt32 counts[ GUI::LAYOUTLINTRULE_COUNT ] = { 0 };
            GUI::CLayoutLint::CountByRule( findings, GUI::LAYOUTLINTSEVERITY_INFO, counts );
            ASSERT_TRUE( 0 == counts[ GUI::LAYOUTLINTRULE_OVERLAP ] && ( 0 == counts[ GUI::LAYOUTLINTRULE_VIEWPORTTOOSMALL ] || 187 == heights[ h ] ) );
            context->DestroyWidget( screen );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    RunFormRenderTests( "report view", &BuildReportScene );
    RunFormLintBaseline( "ReportView", &BuildReportScene );
}

/*-------------------------------------------------------------------------*/
