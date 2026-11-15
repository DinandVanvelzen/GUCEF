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

typedef CORE::CString TString;
typedef GUCEF::vector< TString > TStrings;

/** The text commands of the overlay pass, in their order */
TStrings
OverlayTexts( GUI::CGUIContext* context )
{
    GUI::CDrawSurface::TCommandVector overlay;
    GUI::CWidgetPainter::PaintOverlay( *context, overlay );
    TStrings texts;
    for ( size_t i=0; i<overlay.size(); ++i )
        if ( GUI::CDrawSurface::COMMAND_TEXT == overlay[ i ].type )
            texts.push_back( overlay[ i ].text );
    return texts;
}

bool
OverlayShows( GUI::CGUIContext* context, const TString& text )
{
    TStrings texts = OverlayTexts( context );
    for ( size_t i=0; i<texts.size(); ++i )
        if ( texts[ i ] == text )
            return true;
    return false;
}

TStrings
Strings( const char* a, const char* b = GUCEF_NULL, const char* c = GUCEF_NULL )
{
    TStrings strings;
    strings.push_back( a );
    if ( GUCEF_NULL != b )
        strings.push_back( b );
    if ( GUCEF_NULL != c )
        strings.push_back( c );
    return strings;
}

bool
SameStrings( const GUCEF::vector< TString >& a, const TStrings& b )
{
    if ( a.size() != b.size() )
        return false;
    for ( size_t i=0; i<a.size(); ++i )
        if ( a[ i ] != b[ i ] )
            return false;
    return true;
}

/** The topics of a grid of two columns: the title of the second column and the first cell of the first row have one */
class CTestGridHelpProvider : public GUI::CIGridHelpProvider
{
    public:

    mutable UInt32 asked;

    CTestGridHelpProvider( void ) : GUI::CIGridHelpProvider(), asked( 0 ) {}

    virtual TString GetColumnHelpTopic( const GUI::CGridView& grid, UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE
    {
        ++asked;
        return 1 == column ? TString( "test.value" ) : TString();
    }

    virtual TString GetCellHelpTopic( const GUI::CGridView& grid, UInt32 row, UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE
    {
        ++asked;
        return 0 == row && 0 == column ? TString( "test.cell" ) : TString();
    }
};

const char* const DocumentA =
    "@id test.a\n"
    "@title Alpha\n"
    "@brief The first topic.\n"
    "@seealso test.c\n"
    "\n"
    "# Overview\n"
    "The first topic, see [[test.b]].\n"
    "\n"
    "- one\n"
    "- two\n";

const char* const DocumentB =
    "@id test.b\n"
    "@title Beta\n"
    "@brief The second topic.\n"
    "\n"
    "Back to [[test.a|the first]].\n";

const char* const DocumentC =
    "@id test.c\n"
    "@title Gamma\n"
    "@brief The third topic.\n"
    "\n"
    "Plain text.\n";

/** A catalogue of the three test documents, a document of the value of the grid and two briefs */
GUI::THelpSourcePtr
MakeTestSource( GUI::CHelpCatalogue*& catalogue )
{
    catalogue = GUCEF_NEW GUI::CHelpCatalogue();
    GUI::THelpSourcePtr source = catalogue->CreateSharedPtr();
    TString error;
    catalogue->AddDocumentText( DocumentA, "a.help", error );
    catalogue->AddDocumentText( DocumentB, "b.help", error );
    catalogue->AddDocumentText( DocumentC, "c.help", error );
    catalogue->AddDocumentText( "@id test.value\n@title The value\n@brief The value of the thing.\n\nWhat the value is.\n", "value.help", error );
    catalogue->AddBrief( "test.cell", "A cell brief", "test.value" );
    catalogue->AddBrief( "test.plain", "A brief without a topic", "" );
    return source;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiTooltipAndHelpTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.TooltipAndHelp" );

    GUCEF_TESTFW_TESTCASE( "Tooltip: shows after the delay and not before; a move restarts the rest; a button, the wheel and a key hide it until the pointer moves" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CTooltipService& tooltips = context->GetTooltipService();
        ASSERT_TRUE( 1000 == tooltips.GetShowDelayInMs() && tooltips.IsEnabled() );

        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* button = context->CreateWidget( "Button", "Explained", screen );
        button->SetProperty( "Position", "0,100 0,100" );
        button->SetProperty( "Size", "0,120 0,40" );
        button->SetProperty( "Tooltip", "Explained here" );
        context->FlushChanges();

        // rest: 999 ms is not enough, 1000 ms is
        context->InjectMouseMove( 150, 120 );
        ASSERT_TRUE( button == context->GetHovered() );
        context->RenderFrame( 999 );
        ASSERT_FALSE( OverlayShows( context, "Explained here" ) );
        context->RenderFrame( 1 );
        ASSERT_TRUE( OverlayShows( context, "Explained here" ) && tooltips.IsShowing() );

        // a move within the widget while it shows keeps it
        context->InjectMouseMove( 151, 121 );
        ASSERT_TRUE( OverlayShows( context, "Explained here" ) );

        // a press hides it, also after a long rest, until the pointer moves; then the rest starts again
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        context->RenderFrame( 2000 );
        ASSERT_FALSE( OverlayShows( context, "Explained here" ) );
        context->InjectMouseMove( 152, 120 );
        context->RenderFrame( 999 );
        ASSERT_FALSE( OverlayShows( context, "Explained here" ) );
        context->RenderFrame( 1 );
        ASSERT_TRUE( OverlayShows( context, "Explained here" ) );

        // the wheel and a key do the same
        context->InjectMouseWheel( 1 );
        context->RenderFrame( 5000 );
        ASSERT_FALSE( OverlayShows( context, "Explained here" ) );
        context->InjectMouseMove( 153, 120 );
        context->RenderFrame( 1000 );
        ASSERT_TRUE( OverlayShows( context, "Explained here" ) );
        PressKey( context, INPUT::KEYCODE_A );
        context->RenderFrame( 5000 );
        ASSERT_FALSE( OverlayShows( context, "Explained here" ) );

        // leaving the widget hides it, coming back waits for the delay again
        context->InjectMouseMove( 5, 5 );
        context->RenderFrame( 1000 );
        ASSERT_FALSE( OverlayShows( context, "Explained here" ) );
        context->InjectMouseMove( 150, 120 );
        context->RenderFrame( 500 );
        ASSERT_FALSE( OverlayShows( context, "Explained here" ) );
        context->RenderFrame( 500 );
        ASSERT_TRUE( OverlayShows( context, "Explained here" ) );

        // off: nothing shows; a delay of 300 ms
        tooltips.SetEnabled( false );
        context->RenderFrame( 5000 );
        ASSERT_FALSE( OverlayShows( context, "Explained here" ) );
        tooltips.SetEnabled( true );
        tooltips.SetShowDelayInMs( 300 );
        context->InjectMouseMove( 151, 120 );
        context->RenderFrame( 299 );
        ASSERT_FALSE( OverlayShows( context, "Explained here" ) );
        context->RenderFrame( 1 );
        ASSERT_TRUE( OverlayShows( context, "Explained here" ) );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tooltip: the box stays inside the picture at every edge, also of an ultrawide picture" )
    try
    {
        // a box of 100 x 40 in 640 x 480, the box at the pointer plus (12, 16)
        ASSERT_TRUE( GUI::CRect( 22, 26, 100, 40 ) == GUI::CTooltipLayout::Place( 10, 10, 100, 40, 640, 480, 12, 16 ) );
        // the right edge: moved left to end at 640
        ASSERT_TRUE( GUI::CRect( 540, 26, 100, 40 ) == GUI::CTooltipLayout::Place( 600, 10, 100, 40, 640, 480, 12, 16 ) );
        // the bottom: above the pointer, 470 - 40 - 4
        ASSERT_TRUE( GUI::CRect( 22, 426, 100, 40 ) == GUI::CTooltipLayout::Place( 10, 470, 100, 40, 640, 480, 12, 16 ) );
        // the corner
        ASSERT_TRUE( GUI::CRect( 540, 431, 100, 40 ) == GUI::CTooltipLayout::Place( 630, 475, 100, 40, 640, 480, 12, 16 ) );
        // the right edge of an ultrawide picture: 3440 - 300
        ASSERT_TRUE( GUI::CRect( 3140, 716, 300, 60 ) == GUI::CTooltipLayout::Place( 3430, 700, 300, 60, 3440, 1440, 12, 16 ) );
        // a box higher than the picture starts at its top; one wider than the picture at its left
        ASSERT_TRUE( GUI::CRect( 22, 0, 100, 500 ) == GUI::CTooltipLayout::Place( 10, 10, 100, 500, 640, 480, 12, 16 ) );
        ASSERT_TRUE( GUI::CRect( 0, 26, 700, 40 ) == GUI::CTooltipLayout::Place( 10, 10, 700, 40, 640, 480, 12, 16 ) );
        // a box that fits neither below (166 + 400) nor above (150 - 404) the pointer ends at the bottom of the picture: 480 - 400
        ASSERT_TRUE( GUI::CRect( 22, 80, 100, 400 ) == GUI::CTooltipLayout::Place( 10, 150, 100, 400, 640, 480, 12, 16 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tooltip: long text wraps at the words to the maximum width, a line break stays, a word wider than the room is split" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );

        // the test font: 6 pixels a character, 9 a line; 60 pixels hold 10 characters
        TStrings lines;
        GUI::CTooltipLayout::Wrap( *context, "Test", "aaa bbb ccc ddd", 60, lines );
        ASSERT_TRUE( SameStrings( lines, Strings( "aaa bbb", "ccc ddd" ) ) );
        lines.clear();
        GUI::CTooltipLayout::Wrap( *context, "Test", "line one\nline two", 1000, lines );
        ASSERT_TRUE( SameStrings( lines, Strings( "line one", "line two" ) ) );
        lines.clear();
        GUI::CTooltipLayout::Wrap( *context, "Test", "abcdefghijklmnop", 60, lines );
        ASSERT_TRUE( SameStrings( lines, Strings( "abcdefghij", "klmnop" ) ) );

        // the whole layout: a box of 68 pixels at most leaves 60 for the text; two lines of 42 pixels; at the bottom right corner
        GUI::CTooltipService settings;
        settings.SetMaxWidth( 68 );
        GUI::CTooltipLayout layout;
        ASSERT_TRUE( layout.Compute( *context, "Test", "aaa bbb ccc ddd", 600, 470, settings ) );
        ASSERT_TRUE( 2 == layout.lines.size() && 9 == layout.lineHeight );
        // width 42 + 2 x 4, height 2 x 9 + 2 x 2; x 612 + 50 passes 640: 590; y 486 + 22 passes 480: 470 - 22 - 4
        ASSERT_TRUE( GUI::CRect( 590, 444, 50, 22 ) == layout.box );

        GUI::CDrawSurface::TCommandVector commands;
        GUI::CTooltipPainter::Paint( layout, "Test", settings, commands );
        ASSERT_TRUE( 4 == commands.size() );
        ASSERT_TRUE( GUI::CDrawSurface::COMMAND_FILLRECT == commands[ 0 ].type && layout.box == commands[ 0 ].rect && settings.GetBackgroundColour() == commands[ 0 ].colour );
        ASSERT_TRUE( GUI::CDrawSurface::COMMAND_STROKERECT == commands[ 1 ].type && layout.box == commands[ 1 ].rect );
        ASSERT_TRUE( GUI::CDrawSurface::COMMAND_TEXT == commands[ 2 ].type && "aaa bbb" == commands[ 2 ].text && 594 == commands[ 2 ].x0 && 446 == commands[ 2 ].y0 );
        ASSERT_TRUE( GUI::CDrawSurface::COMMAND_TEXT == commands[ 3 ].type && "ccc ddd" == commands[ 3 ].text && 594 == commands[ 3 ].x0 && 455 == commands[ 3 ].y0 );
        ASSERT_FALSE( layout.Compute( *context, "Test", "", 10, 10, settings ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tooltip: the brief of a help topic of a widget, of an ancestor, of a grid title and of a grid cell, with the hint of the long explainer" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CHelpCatalogue* catalogue = GUCEF_NULL;
        GUI::THelpSourcePtr source = MakeTestSource( catalogue );
        GUI::CTooltipService& tooltips = context->GetTooltipService();

        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* card = context->CreateWidget( "Panel", "Card", screen );
        card->SetProperty( "Position", "0,10 0,10" );
        card->SetProperty( "Size", "0,200 0,80" );
        card->SetProperty( "HelpTopic", "test.value" );
        GUI::CWidget* inner = context->CreateWidget( "Label", "Inner", card );
        inner->SetProperty( "Size", "0,100 0,20" );
        context->FlushChanges();

        // without a source only the tooltips of the widgets
        ASSERT_TRUE( tooltips.GetTooltipText( *inner, 5, 5 ).IsNULLOrEmpty() );
        ASSERT_TRUE( "test.value" == tooltips.FindHelpTopicAt( *inner, 5, 5 ) && "test.value" == card->GetHelpTopicAt( 1, 1 ) );
        tooltips.SetHelpSource( source );
        ASSERT_TRUE( "The value of the thing." == tooltips.GetTooltipText( *inner, 5, 5 ) );
        tooltips.SetMoreHint( "F1: more" );
        ASSERT_TRUE( "The value of the thing.\nF1: more" == tooltips.GetTooltipText( *inner, 5, 5 ) );
        // a brief without a topic has no hint; the widget's own text stays and gets the hint of its topic
        inner->SetProperty( "HelpTopic", "test.plain" );
        ASSERT_TRUE( "A brief without a topic" == tooltips.GetTooltipText( *inner, 5, 5 ) );
        inner->SetProperty( "Tooltip", "Own text" );
        inner->SetProperty( "HelpTopic", "test.cell" );
        ASSERT_TRUE( "Own text\nF1: more" == tooltips.GetTooltipText( *inner, 5, 5 ) );
        ASSERT_TRUE( "test.value" == tooltips.ResolveTopic( "test.cell" ) && "test.a" == tooltips.ResolveTopic( "test.a" ) );
        ASSERT_TRUE( tooltips.ResolveTopic( "test.plain" ).IsNULLOrEmpty() && tooltips.ResolveTopic( "test.unknown" ).IsNULLOrEmpty() );

        // drawn: the brief and the hint each a line of their own
        inner->SetProperty( "Tooltip", "" );
        inner->SetProperty( "HelpTopic", "" );
        context->InjectMouseMove( 20, 20 );
        context->RenderFrame( 1000 );
        TStrings texts = OverlayTexts( context );
        ASSERT_TRUE( SameStrings( texts, Strings( "The value of the thing.", "F1: more" ) ) );

        // a grid: the title of the second column and the first cell through the provider; the other cells keep the Tooltip property
        GUI::CGridView* grid = static_cast< GUI::CGridView* >( context->CreateWidget( "GridView", "Grid", screen ) );
        grid->SetProperty( "Position", "0,10 0,200" );
        grid->SetProperty( "Size", "0,400 0,120" );
        grid->SetProperty( "Columns", "Name\nValue" );
        grid->SetProperty( "Tooltip", "The grid" );
        GUI::CGridView::TRowVector rows;
        GUI::CGridView::TStringVector cells;
        cells.push_back( "alpha" );
        cells.push_back( "1" );
        rows.push_back( cells );
        cells[ 0 ] = "beta";
        cells[ 1 ] = "2";
        rows.push_back( cells );
        grid->SetRows( rows );
        context->FlushChanges();
        GUI::CRect nameCell, valueCell;
        ASSERT_TRUE( grid->GetCellRect( 0, 0, nameCell ) && grid->GetCellRect( 0, 1, valueCell ) );
        const Int32 headerY = grid->GetRowHeight() / 2;
        const Int32 valueX = valueCell.left + valueCell.width / 2;
        const Int32 nameX = nameCell.left + nameCell.width / 2;
        const Int32 firstRowY = nameCell.top + nameCell.height / 2;

        // no provider: the grid as before
        ASSERT_TRUE( "The grid" == tooltips.GetTooltipText( *grid, valueX, headerY ) );
        CTestGridHelpProvider provider;
        grid->SetHelpProvider( &provider );
        ASSERT_TRUE( &provider == grid->GetHelpProvider() && 0 == provider.asked );
        ASSERT_TRUE( "test.value" == grid->GetColumnHelpTopic( 1 ) && grid->GetColumnHelpTopic( 0 ).IsNULLOrEmpty() );
        ASSERT_TRUE( "test.cell" == grid->GetCellHelpTopic( 0, 0 ) && grid->GetCellHelpTopic( 1, 0 ).IsNULLOrEmpty() );
        ASSERT_TRUE( "test.value" == grid->GetHelpTopicAt( valueX, headerY ) );
        ASSERT_TRUE( "The value of the thing.\nF1: more" == tooltips.GetTooltipText( *grid, valueX, headerY ) );
        ASSERT_TRUE( "A cell brief\nF1: more" == tooltips.GetTooltipText( *grid, nameX, firstRowY ) );
        // a part without a topic: the Tooltip property, and the HelpTopic property of the grid as its topic
        ASSERT_TRUE( "The grid" == tooltips.GetTooltipText( *grid, valueX, firstRowY ) );
        ASSERT_TRUE( grid->GetHelpTopicAt( valueX, firstRowY ).IsNULLOrEmpty() );
        grid->SetProperty( "HelpTopic", "test.b" );
        ASSERT_TRUE( "test.b" == grid->GetHelpTopicAt( valueX, firstRowY ) && "test.value" == grid->GetHelpTopicAt( valueX, headerY ) );
        grid->SetHelpProvider( GUCEF_NULL );
        ASSERT_TRUE( grid->GetColumnHelpTopic( 1 ).IsNULLOrEmpty() );
        context->DestroyWidget( screen );
        tooltips.SetHelpSource( GUI::THelpSourcePtr() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Help markup: header fields, headings, paragraphs, bullets, definitions, table rows, notes; inline bold and links; ids" )
    try
    {
        const char* const text =
            "\xEF\xBB\xBF@id test.topic\n"
            "@title A topic\n"
            "@group Tests\n"
            "@brief The brief, with [[test.other]].\n"
            "@seealso test.other, test.third\n"
            "@version 2\n"
            "@reviewed 2026-11\n"
            "@colour blue\n"
            "\n"
            "# First heading\n"
            "A paragraph that\n"
            "continues here with **bold** text.\n"
            "\n"
            "- bullet one\n"
            "  continued\n"
            "- bullet two\n"
            "Term :: the meaning of [[test.other|the other]]\n"
            "| a | [[test.other|the other]] |\n"
            "> A note\n"
            "that goes on\n"
            "## Second\r\n"
            "Last [[Bad Id]] and **open\n";
        GUI::CHelpDocument document;
        TString error;
        ASSERT_TRUE( GUI::CHelpMarkupParser::ParseDocument( text, document, error ) && error.IsNULLOrEmpty() );
        ASSERT_TRUE( "test.topic" == document.id && "A topic" == document.title && "Tests" == document.group );
        ASSERT_TRUE( "The brief, with [[test.other]]." == document.brief && "2" == document.version && "2026-11" == document.reviewed );
        ASSERT_TRUE( SameStrings( document.seeAlso, Strings( "test.other", "test.third" ) ) );
        ASSERT_TRUE( 9 == document.blocks.size() );
        if ( 9 == document.blocks.size() )
        {
            const GUI::CHelpDocument::TBlockVector& b = document.blocks;
            ASSERT_TRUE( GUI::CHelpDocument::BLOCK_HEADING == b[ 0 ].kind && 1 == b[ 0 ].level && "First heading" == b[ 0 ].text );
            ASSERT_TRUE( GUI::CHelpDocument::BLOCK_PARAGRAPH == b[ 1 ].kind && "A paragraph that continues here with **bold** text." == b[ 1 ].text );
            ASSERT_TRUE( GUI::CHelpDocument::BLOCK_BULLET == b[ 2 ].kind && "bullet one continued" == b[ 2 ].text );
            ASSERT_TRUE( GUI::CHelpDocument::BLOCK_BULLET == b[ 3 ].kind && "bullet two" == b[ 3 ].text );
            ASSERT_TRUE( GUI::CHelpDocument::BLOCK_DEFINITION == b[ 4 ].kind && "Term" == b[ 4 ].term && "the meaning of [[test.other|the other]]" == b[ 4 ].text );
            ASSERT_TRUE( GUI::CHelpDocument::BLOCK_TABLE_ROW == b[ 5 ].kind && SameStrings( b[ 5 ].cells, Strings( "a", "[[test.other|the other]]" ) ) );
            ASSERT_TRUE( GUI::CHelpDocument::BLOCK_NOTE == b[ 6 ].kind && "A note that goes on" == b[ 6 ].text );
            ASSERT_TRUE( GUI::CHelpDocument::BLOCK_HEADING == b[ 7 ].kind && 2 == b[ 7 ].level && "Second" == b[ 7 ].text );
            ASSERT_TRUE( GUI::CHelpDocument::BLOCK_PARAGRAPH == b[ 8 ].kind && "Last [[Bad Id]] and **open" == b[ 8 ].text );
        }
        // the links: see also first, then the inline ones, each once
        TStrings links;
        document.CollectLinks( links );
        ASSERT_TRUE( SameStrings( links, Strings( "test.other", "test.third" ) ) );

        // inline runs
        GUI::CHelpMarkupParser::TSpanVector spans;
        GUI::CHelpMarkupParser::ParseInline( "Pay **attention** to [[tax.niit]] and [[net.yield|the yield]].", spans );
        ASSERT_TRUE( 7 == spans.size() );
        if ( 7 == spans.size() )
        {
            ASSERT_TRUE( GUI::CHelpMarkupParser::SPAN_TEXT == spans[ 0 ].kind && "Pay " == spans[ 0 ].text );
            ASSERT_TRUE( GUI::CHelpMarkupParser::SPAN_BOLD == spans[ 1 ].kind && "attention" == spans[ 1 ].text );
            ASSERT_TRUE( GUI::CHelpMarkupParser::SPAN_TEXT == spans[ 2 ].kind && " to " == spans[ 2 ].text );
            ASSERT_TRUE( GUI::CHelpMarkupParser::SPAN_LINK == spans[ 3 ].kind && "tax.niit" == spans[ 3 ].target && spans[ 3 ].text.IsNULLOrEmpty() );
            ASSERT_TRUE( GUI::CHelpMarkupParser::SPAN_TEXT == spans[ 4 ].kind && " and " == spans[ 4 ].text );
            ASSERT_TRUE( GUI::CHelpMarkupParser::SPAN_LINK == spans[ 5 ].kind && "net.yield" == spans[ 5 ].target && "the yield" == spans[ 5 ].text );
            ASSERT_TRUE( GUI::CHelpMarkupParser::SPAN_TEXT == spans[ 6 ].kind && "." == spans[ 6 ].text );
        }
        spans.clear();
        GUI::CHelpMarkupParser::ParseInline( "Last [[Bad Id]] and **open", spans );
        ASSERT_TRUE( 1 == spans.size() && "Last [[Bad Id]] and **open" == spans[ 0 ].text );
        ASSERT_TRUE( "Pay attention to tax.niit and the yield." == GUI::CHelpMarkupParser::ToPlainText( "Pay **attention** to [[tax.niit]] and [[net.yield|the yield]].", GUCEF_NULL ) );

        // ids
        ASSERT_TRUE( GUI::CHelpMarkupParser::IsValidTopicId( "net.yield" ) && GUI::CHelpMarkupParser::IsValidTopicId( "a" ) && GUI::CHelpMarkupParser::IsValidTopicId( "9a-b_c" ) );
        ASSERT_FALSE( GUI::CHelpMarkupParser::IsValidTopicId( "Net.yield" ) || GUI::CHelpMarkupParser::IsValidTopicId( ".net" ) || GUI::CHelpMarkupParser::IsValidTopicId( "" ) );
        ASSERT_FALSE( GUI::CHelpMarkupParser::IsValidTopicId( "net yield" ) );
        TString longId;
        for ( UInt32 i=0; i<96; ++i )
            longId += "a";
        ASSERT_TRUE( GUI::CHelpMarkupParser::IsValidTopicId( longId ) );
        ASSERT_FALSE( GUI::CHelpMarkupParser::IsValidTopicId( longId + "a" ) );

        // refused: no id, no title, an id that is not valid
        ASSERT_FALSE( GUI::CHelpMarkupParser::ParseDocument( "@title No id\n\nText\n", document, error ) || error.IsNULLOrEmpty() );
        ASSERT_FALSE( GUI::CHelpMarkupParser::ParseDocument( "@id no.title\n\nText\n", document, error ) );
        ASSERT_FALSE( GUI::CHelpMarkupParser::ParseDocument( "@id Bad Id\n@title T\n\nText\n", document, error ) );

        // brief tables
        const char* const table =
            "# comment\n"
            "col.a | net.yield | The value of a.\n"
            "col.b |  | No topic here.\n"
            "bad line\n"
            "Bad.Id | | x\n"
            "col.c | Bad Topic | x\n"
            "col.d | net.yield |\n";
        GUI::CHelpMarkupParser::TBriefEntryVector entries;
        TStrings problems;
        ASSERT_TRUE( 2 == GUI::CHelpMarkupParser::ParseBriefTable( table, entries, problems ) );
        ASSERT_TRUE( 2 == entries.size() && 4 == problems.size() );
        if ( 2 == entries.size() && 4 == problems.size() )
        {
            ASSERT_TRUE( "col.a" == entries[ 0 ].id && "net.yield" == entries[ 0 ].topic && "The value of a." == entries[ 0 ].text );
            ASSERT_TRUE( "col.b" == entries[ 1 ].id && entries[ 1 ].topic.IsNULLOrEmpty() && "No topic here." == entries[ 1 ].text );
            ASSERT_TRUE( 0 <= problems[ 0 ].HasSubstr( "line 4" ) && 0 <= problems[ 3 ].HasSubstr( "line 7" ) );
        }

        // the plain text of a document
        GUI::CHelpDocument a;
        ASSERT_TRUE( GUI::CHelpMarkupParser::ParseDocument( DocumentA, a, error ) );
        ASSERT_TRUE( "Overview\nThe first topic, see test.b.\n\n- one\n- two" == a.ToPlainText( GUCEF_NULL ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Help catalogue: briefs of documents and of entries, the index in order, the check of what a reader can reach" )
    try
    {
        GUI::CHelpCatalogue* catalogue = GUCEF_NULL;
        GUI::THelpSourcePtr source = MakeTestSource( catalogue );
        ASSERT_TRUE( 4 == catalogue->GetTopicCount() && 2 == catalogue->GetBriefCount() );
        TString text, topic;
        ASSERT_TRUE( catalogue->GetBrief( "test.a", text, topic ) && "The first topic." == text && "test.a" == topic );
        ASSERT_TRUE( catalogue->GetBrief( "test.cell", text, topic ) && "A cell brief" == text && "test.value" == topic );
        ASSERT_FALSE( catalogue->GetBrief( "test.unknown", text, topic ) );
        ASSERT_TRUE( catalogue->HasTopic( "test.b" ) && !catalogue->HasTopic( "test.cell" ) );
        ASSERT_TRUE( "Beta" == catalogue->GetTopicTitle( "test.b" ) && "test.cell" == catalogue->GetTopicTitle( "test.cell" ) );
        ASSERT_TRUE( "Back to the first." == GUI::CHelpMarkupParser::ToPlainText( "Back to [[test.a|the first]].", catalogue ) );
        ASSERT_TRUE( "See Beta." == GUI::CHelpMarkupParser::ToPlainText( "See [[test.b]].", catalogue ) );

        // the index: by group, then by title
        catalogue->AddDocumentText( "@id test.z\n@title Zulu\n@group A group\n@brief Z.\n\nZ.\n", "z.help", text );
        GUI::CIHelpSource::TTopicEntryVector index;
        catalogue->ListTopics( index );
        ASSERT_TRUE( 5 == index.size() );
        if ( 5 == index.size() )
        {
            ASSERT_TRUE( "test.a" == index[ 0 ].id && "test.b" == index[ 1 ].id && "test.c" == index[ 2 ].id && "test.value" == index[ 3 ].id );
            ASSERT_TRUE( "test.z" == index[ 4 ].id && "A group" == index[ 4 ].group );
        }

        // complete: nothing to report
        TStrings problems;
        ASSERT_TRUE( catalogue->Validate( problems ) && problems.empty() );

        // what a reader cannot reach or a tooltip cannot show
        catalogue->AddDocumentText( "@id test.broken\n@title Broken\n@seealso test.missing\n\nSee [[test.gone]].\n", "broken.help", text );
        catalogue->AddBrief( "test.dangling", "Points nowhere", "test.none" );
        catalogue->AddBrief( "test.b", "Twice", "" );
        ASSERT_FALSE( catalogue->Validate( problems ) );
        bool missingBrief = false, seeAlso = false, link = false, dangling = false, twice = false;
        for ( size_t i=0; i<problems.size(); ++i )
        {
            missingBrief = missingBrief || 0 <= problems[ i ].HasSubstr( "test.broken has no brief" );
            seeAlso = seeAlso || 0 <= problems[ i ].HasSubstr( "sees also test.missing" );
            link = link || 0 <= problems[ i ].HasSubstr( "links to test.gone" );
            dangling = dangling || 0 <= problems[ i ].HasSubstr( "names the topic test.none" );
            twice = twice || 0 <= problems[ i ].HasSubstr( "test.b is both a topic and a brief" );
        }
        ASSERT_TRUE( missingBrief && seeAlso && link && dangling && twice && 5 == problems.size() );

        // refused: a document without a title, a brief without text or with an id that is not valid
        ASSERT_FALSE( catalogue->AddDocumentText( "@id test.nothing\n\nText\n", "nothing.help", text ) );
        ASSERT_TRUE( 0 <= text.HasSubstr( "nothing.help" ) );
        ASSERT_FALSE( catalogue->AddBrief( "test.empty", "", "" ) || catalogue->AddBrief( "Bad Id", "x", "" ) );
        catalogue->Clear();
        ASSERT_TRUE( 0 == catalogue->GetTopicCount() && 0 == catalogue->GetBriefCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Help history: back and forward, a visit drops what was ahead, a visit of the topic shown adds nothing, the maximum drops the oldest" )
    try
    {
        GUI::CHelpHistory history( 0 );
        ASSERT_TRUE( history.GetCurrent().IsNULLOrEmpty() && !history.CanGoBack() && !history.CanGoForward() && -1 == history.GetIndex() );
        history.Visit( "a" );
        history.Visit( "b" );
        history.Visit( "c" );
        ASSERT_TRUE( "c" == history.GetCurrent() && history.CanGoBack() && !history.CanGoForward() );
        ASSERT_TRUE( history.Back() && "b" == history.GetCurrent() && history.Back() && "a" == history.GetCurrent() );
        ASSERT_FALSE( history.CanGoBack() || history.Back() );
        ASSERT_TRUE( history.Forward() && "b" == history.GetCurrent() && history.CanGoForward() );
        history.Visit( "d" );
        ASSERT_TRUE( SameStrings( history.GetEntries(), Strings( "a", "b", "d" ) ) && !history.CanGoForward() && "d" == history.GetCurrent() );
        history.Visit( "d" );
        history.Visit( "" );
        ASSERT_TRUE( 3 == history.GetCount() );

        GUI::CHelpHistory bounded( 3 );
        const char* const visits[] = { "1", "2", "3", "4", "5" };
        for ( UInt32 i=0; i<5; ++i )
            bounded.Visit( visits[ i ] );
        ASSERT_TRUE( SameStrings( bounded.GetEntries(), Strings( "3", "4", "5" ) ) && 2 == bounded.GetIndex() );
        ASSERT_TRUE( bounded.Back() && bounded.Back() && "3" == bounded.GetCurrent() );
        bounded.SetMaximumEntries( 2 );
        ASSERT_TRUE( SameStrings( bounded.GetEntries(), Strings( "4", "5" ) ) && "4" == bounded.GetCurrent() && 2 == bounded.GetMaximumEntries() );
        bounded.Clear();
        ASSERT_TRUE( 0 == bounded.GetCount() && bounded.GetCurrent().IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Help viewer: opens a topic with its text and links, follows a link and an index entry, back and forward, not found, Escape closes; adopted by the context" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CHelpCatalogue* catalogue = GUCEF_NULL;
        GUI::THelpSourcePtr source = MakeTestSource( catalogue );

        GUI::CHelpViewerForm viewer( context );
        viewer.SetHelpSource( source );
        ASSERT_TRUE( viewer.Build( "Help" ) && !viewer.IsVisible() && !viewer.IsOpen() );
        const char* names[] = { "Back", "Forward", "TopicTitle", "Topics", "Document", "LinksTitle", "Links", "Reviewed", "Close" };
        for ( UInt32 i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
            ASSERT_TRUE( HasWidget( viewer, names[ i ] ) );
        CEventRecorder shown( &viewer, GUI::CHelpViewerForm::TopicShownEvent );
        CEventRecorder closed( &viewer, GUI::CHelpViewerForm::ClosedEvent );

        ASSERT_TRUE( viewer.Open( "test.a" ) && viewer.IsOpen() && viewer.IsVisible() && viewer.IsCurrentTopicFound() );
        ASSERT_TRUE( "test.a" == viewer.GetCurrentTopic() && 1 == shown.count );
        ASSERT_TRUE( "Alpha" == viewer.GetWidget( "TopicTitle" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "Overview\nThe first topic, see Beta.\n\n- one\n- two" == viewer.GetDocumentText() );
        ASSERT_TRUE( viewer.GetDocumentText() == viewer.GetWidget( "Document" )->GetProperty( "Text" ) );
        ASSERT_TRUE( SameStrings( viewer.GetLinkTopics(), Strings( "test.c", "test.b" ) ) );
        GUI::CListbox* links = viewer.GetWidgetAs< GUI::CListbox >( "Links" );
        ASSERT_TRUE( GUCEF_NULL != links && SameStrings( links->GetItems(), Strings( "Gamma", "Beta" ) ) );
        GUI::CListbox* topics = viewer.GetWidgetAs< GUI::CListbox >( "Topics" );
        ASSERT_TRUE( GUCEF_NULL != topics && 4 == topics->GetItems().size() && 0 == topics->GetSelectedIndex() );
        ASSERT_FALSE( viewer.GetWidget( "Back" )->IsEnabled() || viewer.GetWidget( "Forward" )->IsEnabled() );

        // a link, back and forward
        ASSERT_TRUE( viewer.OpenLink( 1 ) && "test.b" == viewer.GetCurrentTopic() && "Back to the first." == viewer.GetDocumentText() );
        ASSERT_TRUE( viewer.GetWidget( "Back" )->IsEnabled() );
        ClickWidget( context, viewer.GetWidget( "Back" ) );
        ASSERT_TRUE( "test.a" == viewer.GetCurrentTopic() && viewer.GetWidget( "Forward" )->IsEnabled() );
        ASSERT_TRUE( viewer.GoForward() && "test.b" == viewer.GetCurrentTopic() );
        // the keys go to the window itself (the button that was clicked may be disabled now)
        context->SetFocus( GUCEF_NULL );
        PressKey( context, INPUT::KEYCODE_LEFT, INPUT::KEYMOD_ALT );
        ASSERT_TRUE( "test.a" == viewer.GetCurrentTopic() );
        PressKey( context, INPUT::KEYCODE_RIGHT, INPUT::KEYMOD_ALT );
        ASSERT_TRUE( "test.b" == viewer.GetCurrentTopic() );

        // the index: Alpha, Beta, Gamma, The value; a click on an entry opens it
        ASSERT_TRUE( viewer.OpenIndexEntry( 2 ) && "test.c" == viewer.GetCurrentTopic() && 2 == topics->GetSelectedIndex() );
        context->FlushChanges();
        topics->SetSelectedIndex( 3 );
        ASSERT_TRUE( "test.value" == viewer.GetCurrentTopic() );

        // not found: said so, not in the history
        const UInt32 visited = viewer.GetHistory().GetCount();
        ASSERT_FALSE( viewer.ShowTopic( "test.nothere" ) );
        ASSERT_TRUE( "Not found" == viewer.GetWidget( "TopicTitle" )->GetProperty( "Text" ) && 0 <= viewer.GetDocumentText().HasSubstr( "test.nothere" ) );
        ASSERT_TRUE( visited == viewer.GetHistory().GetCount() && viewer.GetLinkTopics().empty() );

        // Escape closes
        context->SetFocus( GUCEF_NULL );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( !viewer.IsOpen() && !viewer.IsVisible() && 1 == closed.count );

        // a viewer made elsewhere and handed to the context
        GUI::CHelpViewerForm* adopted = GUCEF_NEW GUI::CHelpViewerForm( context );
        ASSERT_TRUE( context->AdoptForm( adopted ) && context->OwnsForm( adopted ) );
        adopted->SetHelpSource( source );
        ASSERT_TRUE( adopted->Open( "test.c" ) && adopted->IsVisible() );
        adopted->Close();
        context->DestroyForm( adopted );
        ASSERT_FALSE( context->OwnsForm( adopted ) );

        // in the place of a widget
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* placeholder = context->CreateWidget( "Panel", "Place", screen );
        placeholder->SetProperty( "Size", "0,600 0,400" );
        GUI::CHelpViewerForm inPlace;
        inPlace.SetHelpSource( source );
        ASSERT_TRUE( inPlace.Build( placeholder ) && placeholder == inPlace.GetRootWidget()->GetParentWidget() );
        ASSERT_TRUE( inPlace.Open( "test.b" ) && "Beta" == inPlace.GetWidget( "TopicTitle" )->GetProperty( "Text" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
