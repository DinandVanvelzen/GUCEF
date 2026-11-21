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

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

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

GUI::TGuiContextPtr
CreateLayoutLintContext( void )
{
    GUI::TGuiContextPtr contextPtr = CreateNullContext();
    GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
    if ( GUCEF_NULL != context )
    {
        context->SetLogicalResolution( 640, 480 );
        context->RegisterFont( "Test", GUITEST::MakeLayoutLintFont() );
        CORE::CDataNode theme;
        GUI::BuildDarkTheme( theme, "Test" );
        context->SetTheme( theme );
    }
    return contextPtr;
}

/*-------------------------------------------------------------------------*/

namespace {

using namespace GUITEST;

typedef CORE::CString TString;

const Int32 LintGlyphWidth = GUITEST::LayoutLintGlyphWidth;
const Int32 LintLineHeight = GUITEST::LayoutLintLineHeight;

GUI::TGuiContextPtr
CreateLintContext( void )
{
    return CreateLayoutLintContext();
}

GUI::CWidget*
Add( GUI::CGUIContext* context, GUI::CWidget* parent, const char* type, const char* name, const char* position, const char* size, const char* text = GUCEF_NULL )
{
    GUI::CWidget* widget = context->CreateWidget( type, name, parent );
    if ( GUCEF_NULL == widget )
        return GUCEF_NULL;
    widget->SetProperty( "Position", position );
    widget->SetProperty( "Size", size );
    if ( GUCEF_NULL != text )
        widget->SetProperty( "Text", text );
    return widget;
}

/** The findings of a lint run with the default options (layout first) */
GUI::TLayoutLintFindingVector
Lint( GUI::CGUIContext* context, const GUI::CLayoutLintOptions& options = GUI::CLayoutLintOptions() )
{
    GUI::TLayoutLintFindingVector findings;
    GUI::CLayoutLint lint;
    lint.Run( *context, options, findings );
    return findings;
}

/** How many findings of the rule are about a widget whose path ends with the name (empty: any widget) */
UInt32
Count( const GUI::TLayoutLintFindingVector& findings, GUI::ELayoutLintRule rule, const TString& name = TString::Empty )
{
    UInt32 count = 0;
    for ( UInt32 i=0; i<(UInt32) findings.size(); ++i )
    {
        if ( findings[ i ].rule != rule )
            continue;
        const TString& path = findings[ i ].widgetPath;
        if ( name.IsNULLOrEmpty() || ( path.Length() >= name.Length() && path.SubstrFromRange( path.Length() - name.Length(), path.Length() ) == name ) )
            ++count;
    }
    return count;
}

/** The severity of the first finding of the rule, LAYOUTLINTSEVERITY_COUNT when there is none */
GUI::ELayoutLintSeverity
SeverityOf( const GUI::TLayoutLintFindingVector& findings, GUI::ELayoutLintRule rule )
{
    for ( UInt32 i=0; i<(UInt32) findings.size(); ++i )
        if ( findings[ i ].rule == rule )
            return findings[ i ].severity;
    return GUI::LAYOUTLINTSEVERITY_COUNT;
}

void
LogFindings( const char* title, const GUI::TLayoutLintFindingVector& findings )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, title << ": " << GUI::CLayoutLint::Report( findings, 20 ) );
}

/** The text commands of the painter for one widget */
GUI::CDrawSurface::TCommandVector
TextCommands( GUI::CGUIContext* context, GUI::CWidget* widget )
{
    context->FlushChanges();
    GUI::CDrawSurface::TCommandVector commands, texts;
    GUI::CWidgetPainter::Paint( *widget, *context, commands );
    for ( UInt32 i=0; i<(UInt32) commands.size(); ++i )
        if ( GUI::CDrawSurface::COMMAND_TEXT == commands[ i ].type )
            texts.push_back( commands[ i ] );
    return texts;
}

/*
 *  The baseline of the forms: the realistic font, the matrix, the counts and the budget
 */

/**
 *  The findings (warnings and errors, summed over the combinations of the matrix) each form is still allowed, per rule.
 *  Recorded by the first run of the lint (G0) and lowered as the layout steps fix the forms; a form that is not listed is
 *  allowed none. Never raise a number: fix the form, or suppress an intended case with LintIgnore and its reason.
 */
const char* const FormLintBudget =
    "# scope rule count: the warnings and errors of the forms after G1 (Noto Sans 16 px, 15 combinations)\n"
    "Timeline L01 7\n"
    "Timeline L04 2\n"
    "ReportView L01 8\n"
    "ReportView L04 4\n"
    "ReportView L10 4\n"
    "ReportView L15 60\n"
    "TransferQueue L15 24\n"
    "AssetSearch L01 18\n"
    "AssetSearch L02 8\n"
    "AssetSearch L03 8\n"
    "AssetSearch L04 10\n"
    "AssetSearch L10 28\n"
    "AssetSearch L15 45\n"
    "Viewport L02 5\n"
    "Viewport L04 2\n"
    "ContentBrowser L04 10\n"
    "ContentBrowserHost448 L04 11\n"
    "ContentBrowserHost448 L05 31\n"
    "ContentBrowserHost448 L12 56\n"
    "ContentBrowserHost280 L04 53\n"
    "ContentBrowserHost280 L05 45\n"
    "ActivityIndicator L02 6\n"
    "ActivityIndicator L03 6\n"
    "ActivityIndicator L04 5\n"
    "ActivityIndicator L12 1\n"
    "ImageView L01 31\n"
    "ImageView L04 4\n"
    "ImageView L10 9\n"
    "OpenActionStrip L01 3\n"
    "OpenActionStrip L02 1\n"
    "FileOpenDialog L01 10\n"
    "FileOpenDialog L04 23\n"
    "FileOpenDialog L05 1\n"
    "FileOpenDialog L08 2\n"
    ;

/** Whether the lint runs of the forms use the user interface font (Noto Sans by the FreeType plugin); else the 9 x 22 font */
bool g_lintWithUiFont = false;

/** Registers the font of the forms as "Test" at the scale: Noto Sans at 16 px times the scale when it can, else the 9 x 22 font */
bool
RegisterFormLintFont( GUI::CGUIContext* context, Float32 scale )
{
    const UInt32 pixels = (UInt32) ( 16.0f * scale + 0.5f );
    if ( LoadFreeTypeFontPlugin() && GUI::CGuiGlobal::Instance()->MountData() && context->RegisterFont( "Test", GUI::CGuiGlobal::DefaultUiFontPath, pixels ) )
    {
        g_lintWithUiFont = true;
        return true;
    }
    g_lintWithUiFont = false;
    return context->RegisterFont( "Test", MakeTestFont( (Int32) ( LintGlyphWidth * scale ), (Int32) ( LintLineHeight * scale ) ) );
}

/** The sweep of the forms: the text scale registers the font of the forms again */
class CFormLintSweep : public GUI::CLayoutLintSweep
{
    protected:

    virtual bool ApplyTextScale( GUI::CGUIContext& context, Float32 scale ) GUCEF_VIRTUAL_OVERRIDE
    {
        return RegisterFormLintFont( &context, scale );
    }
};

/** Appends a text to the file GUCEF_TEST_LAYOUT_LINT_REPORT names, when it is set: the baseline for the design document */
void
AppendToLintReport( const TString& text )
{
    const char* path = getenv( "GUCEF_TEST_LAYOUT_LINT_REPORT" );
    if ( GUCEF_NULL == path || '\0' == *path )
        return;
    CORE::CFileAccess file( path, "ab" );
    if ( file.IsValid() )
        file.Write( text.C_String(), 1, text.Length() );
}

/** A scene of the open action strip: three ways to open and two recent places, at the top of the screen */
class COpenStripScene : public CFormScene
{
    public:

    GUI::CRecentLocations recent;
    GUI::COpenActionStripForm strip;

    explicit COpenStripScene( GUI::CGUIContext* context ) : CFormScene(), recent(), strip( context )
    {
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* placeholder = context->CreateWidget( "Panel", "Strip", screen );
        placeholder->SetProperty( "Size", "1,0 0," + CORE::UInt32ToString( GUI::COpenActionStripForm::GetPreferredHeight() ) );
        strip.AddAction( "folder", "Open folder...", "a folder" );
        strip.AddAction( "archive", "Open archive or pack...", "an archive" );
        strip.AddAction( "file", "Open file...", "a file" );
        strip.Build( placeholder );
        recent.Add( "/home/user/assets/textures/stone" );
        recent.Add( "/home/user/assets/packs/castle.zip" );
        strip.SetRecentLocations( &recent );
        context->FlushChanges();
    }

    virtual ~COpenStripScene()
    {
        strip.SetRecentLocations( GUCEF_NULL );
    }
};

CFormScene*
BuildOpenStripScene( GUI::CGUIContext* context )
{
    return new COpenStripScene( context );
}

/** A scene of the file open dialog over a small tree of folders and files */
class CFileDialogScene : public CFormScene
{
    public:

    GUI::CFileOpenDialogForm dialog;

    explicit CFileDialogScene( GUI::CGUIContext* context ) : CFormScene(), dialog( context )
    {
        MakeScreen( context );
        CTestContentFileSystem* fs = GUCEF_NEW CTestContentFileSystem();
        fs->AddDirectory( "/home" );
        fs->AddDirectory( "/home/assets" );
        fs->AddDirectory( "/home/assets/textures" );
        fs->AddFile( "/home/assets/readme.txt", 2048 );
        fs->AddFile( "/home/assets/a_texture_with_a_rather_long_descriptive_file_name_of_the_kind_packs_ship.png", 650000000 );
        fs->AddFile( "/home/assets/rock.png", 120 );
        dialog.AddRoot( "Computer", fs->CreateSharedPtr(), "/home/assets" );
        dialog.Open();
        context->FlushChanges();
    }

    virtual ~CFileDialogScene()
    {
        dialog.Cancel();
    }
};

CFormScene*
BuildFileDialogScene( GUI::CGUIContext* context )
{
    return new CFileDialogScene( context );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE FORMS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
RunFormLintBaseline( const char* scope, GUITEST::TFormSceneBuilder builder )
{GUCEF_TRACE;

    CORE::CString title = CORE::CString( "LayoutLint: The " ) + scope + " stays within its budget of findings over the matrix of sizes, scales and pseudo-localised text";
    GUCEF_TESTFW_TESTCASE( title.C_String() )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        context->SetLogicalResolution( 640, 480 );
        ASSERT_TRUE( RegisterFormLintFont( context, 1.0f ) );
        CORE::CDataNode theme;
        GUI::BuildDarkTheme( theme, "Test" );
        context->SetTheme( theme );
        GUITEST::CFormScene* scene = builder( context );
        ASSERT_TRUE( GUCEF_NULL != scene );
        if ( GUCEF_NULL == scene )
            return;

        GUI::CLayoutLintMatrix matrix;
        matrix.SetTextScaleSupported( true );
        CFormLintSweep sweep;
        sweep.Run( *context, matrix, GUI::CLayoutLintOptions() );
        for ( UInt32 c=0; c<matrix.GetCombinationCount(); ++c )
        {
            const GUI::CLayoutLintMatrix::TCombination& combination = matrix.GetCombination( c );
            if ( 0 == c || ( 1600 == combination.width && 900 == combination.height && !combination.pseudoLocalized ) )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LayoutLint " << scope << " at " << matrix.DescribeCombination( c ) << ": " << GUI::CLayoutLint::Report( sweep.GetFindings( c ), 40 ) );
        }

        // the counts per rule against the budget of the form: errors and warnings may only go down
        GUI::CLayoutLintBudget budget;
        CORE::CString problems, improvements;
        ASSERT_TRUE( budget.LoadFromText( FormLintBudget, problems ) );
        const bool within = budget.Check( scope, sweep.GetRuleCounts(), problems, improvements );
        const TString summary = TString( "== " ) + scope + ( g_lintWithUiFont ? " (Noto Sans 16 px)" : " (test font 9 x 22)" ) + ": " +
                                CORE::UInt32ToString( sweep.GetCount( GUI::LAYOUTLINTSEVERITY_ERROR ) ) + " errors, " +
                                CORE::UInt32ToString( sweep.GetCount( GUI::LAYOUTLINTSEVERITY_WARNING ) ) + " warnings, " +
                                CORE::UInt32ToString( sweep.GetCount( GUI::LAYOUTLINTSEVERITY_INFO ) ) + " info over " +
                                CORE::UInt32ToString( matrix.GetCombinationCount() ) + " combinations\n" + sweep.GetSummary() +
                                GUI::CLayoutLintBudget::FormatCounts( scope, sweep.GetRuleCounts() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LayoutLint baseline " << summary );
        AppendToLintReport( summary );
        if ( !improvements.IsNULLOrEmpty() )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LayoutLint: " << improvements );
        // the budget holds the counts with the user interface font; with the 9 x 22 font (no FreeType plugin) they are reported only
        if ( !within )
            GUCEF_LOG( g_lintWithUiFont ? CORE::LOGLEVEL_CRITICAL : CORE::LOGLEVEL_NORMAL, "LayoutLint: over the budget:\n" << problems );
        ASSERT_TRUE( within || !g_lintWithUiFont );

        delete scene;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiLayoutLintTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.LayoutLint" );

    GUCEF_TESTFW_TESTCASE( "LayoutLint: Rules, severities and their names and ids convert both ways" )
    try
    {
        for ( UInt32 r=0; r<(UInt32) GUI::LAYOUTLINTRULE_COUNT; ++r )
        {
            const GUI::ELayoutLintRule rule = (GUI::ELayoutLintRule) r;
            GUI::ELayoutLintRule back = GUI::LAYOUTLINTRULE_COUNT;
            ASSERT_TRUE( GUI::StringToLayoutLintRule( GUI::LayoutLintRuleToString( rule ), back ) && back == rule );
            back = GUI::LAYOUTLINTRULE_COUNT;
            ASSERT_TRUE( GUI::StringToLayoutLintRule( GUI::LayoutLintRuleToId( rule ).Lowercase(), back ) && back == rule );
        }
        ASSERT_TRUE( "L01" == GUI::LayoutLintRuleToId( GUI::LAYOUTLINTRULE_OVERLAP ) && "TextClipped" == GUI::LayoutLintRuleToString( GUI::LAYOUTLINTRULE_TEXTCLIPPED ) );
        ASSERT_TRUE( "L17" == GUI::LayoutLintRuleToId( GUI::LAYOUTLINTRULE_ABSOLUTEINFLOW ) );
        GUI::ELayoutLintRule unknown = GUI::LAYOUTLINTRULE_OVERLAP;
        ASSERT_FALSE( GUI::StringToLayoutLintRule( "L99", unknown ) );
        ASSERT_TRUE( GUI::LAYOUTLINTRULE_OVERLAP == unknown );
        GUI::ELayoutLintSeverity severity = GUI::LAYOUTLINTSEVERITY_INFO;
        ASSERT_TRUE( GUI::StringToLayoutLintSeverity( "error", severity ) && GUI::LAYOUTLINTSEVERITY_ERROR == severity );
        ASSERT_TRUE( "Warning" == GUI::LayoutLintSeverityToString( GUI::LAYOUTLINTSEVERITY_WARNING ) );
        ASSERT_TRUE( GUI::LAYOUTLINTSEVERITY_INFO == GUI::GetLayoutLintRuleDefaultSeverity( GUI::LAYOUTLINTRULE_EMPTYVIEWPORT ) );
        ASSERT_FALSE( GUI::IsLayoutLintRuleComplete( GUI::LAYOUTLINTRULE_BELOWMINIMUM ) );
        ASSERT_TRUE( GUI::IsLayoutLintRuleComplete( GUI::LAYOUTLINTRULE_TEXTCLIPPED ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: A clean screen has no findings, and the lint changes nothing in the widgets" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* label = Add( context, screen, "Label", "Title", "0,16 0,16", "0,300 0,28", "Settings" );
        Add( context, screen, "Button", "Ok", "0,16 0,60", "0,100 0,32", "OK" );
        Add( context, screen, "Button", "Cancel", "0,132 0,60", "0,100 0,32", "Cancel" );
        GUI::TLayoutLintFindingVector findings = Lint( context );
        LogFindings( "clean screen", findings );
        ASSERT_TRUE( findings.empty() );
        ASSERT_TRUE( "Settings" == label->GetProperty( "Text" ) );
        ASSERT_TRUE( 0 == GUI::CLayoutLint::CountAtLeast( findings, GUI::LAYOUTLINTSEVERITY_INFO ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: L01 Overlap: overlapping controls are an error; a decoration below, LintIgnore and a suppression are not" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* backdrop = Add( context, screen, "Panel", "Backdrop", "0,10 0,10", "0,300 0,200" );
        backdrop->SetProperty( "BackgroundColour", "50,50,60,255" );
        Add( context, screen, "Button", "First", "0,20 0,20", "0,100 0,32", "First" );
        Add( context, screen, "Button", "Second", "0,80 0,30", "0,100 0,32", "Second" );
        GUI::TLayoutLintFindingVector findings = Lint( context );
        LogFindings( "overlap", findings );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_OVERLAP ) && 1 == Count( findings, GUI::LAYOUTLINTRULE_OVERLAP, "Second" ) );
        ASSERT_TRUE( GUI::LAYOUTLINTSEVERITY_ERROR == SeverityOf( findings, GUI::LAYOUTLINTRULE_OVERLAP ) );
        ASSERT_TRUE( "Screen/First" == findings[ 0 ].otherPath && findings[ 0 ].Describe().HasSubstr( "L01 Overlap (Error): Screen/Second" ) == 0 );

        // a suppression of the options, by a glob over the name path
        GUI::CLayoutLintOptions options;
        ASSERT_TRUE( options.AddSuppression( GUI::LAYOUTLINTRULE_OVERLAP, "Screen/Sec*" ) );
        ASSERT_TRUE( 0 == Count( Lint( context, options ), GUI::LAYOUTLINTRULE_OVERLAP ) );

        // LintIgnore on the widget, by id or by name
        context->GetRootWidgets()[ 0 ]->GetChildWidget( "Second" )->SetProperty( "LintIgnore", "L10, Overlap" );
        ASSERT_TRUE( 0 == Count( Lint( context ), GUI::LAYOUTLINTRULE_OVERLAP ) );

        // a rule that is off
        context->GetRootWidgets()[ 0 ]->GetChildWidget( "Second" )->SetProperty( "LintIgnore", "" );
        options.ClearSuppressions();
        options.EnableRule( GUI::LAYOUTLINTRULE_OVERLAP, false );
        ASSERT_TRUE( 0 == Count( Lint( context, options ), GUI::LAYOUTLINTRULE_OVERLAP ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: L02 OutsideParent, L03 OutsideScreen and L14 ContentUnreachable" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* box = Add( context, screen, "Panel", "Box", "0,10 0,10", "0,200 0,100" );
        box->SetProperty( "BackgroundColour", "50,50,60,255" );
        Add( context, box, "Label", "Wide", "0,150 0,10", "0,100 0,24", "Name" );
        Add( context, screen, "Label", "Edge", "0,600 0,400", "0,100 0,24", "Status" );
        GUI::TLayoutLintFindingVector findings = Lint( context );
        LogFindings( "outside", findings );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_OUTSIDEPARENT, "Wide" ) );
        ASSERT_TRUE( GUI::LAYOUTLINTSEVERITY_WARNING == SeverityOf( findings, GUI::LAYOUTLINTRULE_OUTSIDEPARENT ) );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_OUTSIDESCREEN, "Edge" ) );

        // a parent that cuts it off: an error
        box->SetProperty( "Clip", "true" );
        findings = Lint( context );
        ASSERT_TRUE( GUI::LAYOUTLINTSEVERITY_ERROR == SeverityOf( findings, GUI::LAYOUTLINTRULE_OUTSIDEPARENT ) );

        // below the end of a scroll panel is scrolled to, not outside
        GUI::CWidget* scroll = Add( context, screen, "ScrollPanel", "Scroll", "0,300 0,10", "0,200 0,100" );
        Add( context, scroll, "Label", "Far", "0,0 0,300", "0,100 0,24", "Far down" );
        findings = Lint( context );
        ASSERT_TRUE( 0 == Count( findings, GUI::LAYOUTLINTRULE_OUTSIDEPARENT, "Far" ) && 0 == Count( findings, GUI::LAYOUTLINTRULE_CONTENTUNREACHABLE, "Far" ) );

        // inside its parent, but beyond an ancestor further up that cuts it off and does not scroll that way
        GUI::CWidget* inner = Add( context, box, "Panel", "Inner", "0,0 0,0", "0,400 0,40" );
        Add( context, inner, "Label", "Hidden", "0,250 0,0", "0,100 0,24", "Lost" );
        findings = Lint( context );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_CONTENTUNREACHABLE, "Hidden" ) );
        ASSERT_TRUE( 0 == Count( findings, GUI::LAYOUTLINTRULE_OUTSIDEPARENT, "Hidden" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: L04 TextClipped: a silent cut is an error; an ellipsis, a scroll or enough room is not" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* clipped = Add( context, screen, "Label", "Clipped", "0,10 0,10", "0,60 0,28", "A long text here" );
        clipped->SetProperty( "TextOverflow", "Clip" );
        Add( context, screen, "Label", "Elided", "0,10 0,50", "0,60 0,28", "A long text here" );
        Add( context, screen, "Label", "Fits", "0,10 0,90", "0,200 0,28", "A long text here" );
        Add( context, screen, "Editbox", "Field", "0,10 0,130", "0,60 0,28", "typed text that scrolls" );
        GUI::TLayoutLintFindingVector findings = Lint( context );
        LogFindings( "text clipped", findings );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_TEXTCLIPPED, "Clipped" ) );
        ASSERT_TRUE( 0 == Count( findings, GUI::LAYOUTLINTRULE_TEXTCLIPPED, "Elided" ) && 0 == Count( findings, GUI::LAYOUTLINTRULE_TEXTCLIPPED, "Fits" ) );
        ASSERT_TRUE( 0 == Count( findings, GUI::LAYOUTLINTRULE_TEXTCLIPPED, "Field" ) );
        ASSERT_TRUE( 0 == Count( findings, GUI::LAYOUTLINTRULE_TEXTELIDEDNOTOOLTIP ) );
        for ( UInt32 i=0; i<(UInt32) findings.size(); ++i )
            if ( GUI::LAYOUTLINTRULE_TEXTCLIPPED == findings[ i ].rule )
                ASSERT_TRUE( "A long text here" == findings[ i ].text && 144 == findings[ i ].rect.width && findings[ i ].detail.HasSubstr( "needs 144 px width, has 60" ) >= 0 );

        // vertically: a label lower than its line, by a height that is a part of its parent only (no one line minimum then)
        Add( context, screen, "Label", "Low", "0,300 0,10", "0,200 0.02,0", "Low" );
        findings = Lint( context );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_TEXTCLIPPED, "Low" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: L05 TextElidedNoTooltip and L12 NumberElided on text drawn shortened by hand" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CDrawSurface* surface = static_cast< GUI::CDrawSurface* >( Add( context, screen, "DrawSurface", "Surface", "0,10 0,10", "0,300 0,100" ) );
        ASSERT_TRUE( GUCEF_NULL != surface );
        surface->Clear();
        surface->DrawString( "Test", "Long tex...", 0, 0, GUI::CColour( 255, 255, 255, 255 ) );
        surface->DrawString( "Test", "194...", 0, 30, GUI::CColour( 255, 255, 255, 255 ) );
        surface->Commit();
        GUI::TLayoutLintFindingVector findings = Lint( context );
        LogFindings( "elided by hand", findings );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_NUMBERELIDED, "Surface" ) );
        ASSERT_TRUE( 2 == Count( findings, GUI::LAYOUTLINTRULE_TEXTELIDEDNOTOOLTIP, "Surface" ) );

        // with a tooltip the shortened text says what it is
        surface->SetProperty( "Tooltip", "Long text, 194 MiB" );
        findings = Lint( context );
        ASSERT_TRUE( 0 == Count( findings, GUI::LAYOUTLINTRULE_TEXTELIDEDNOTOOLTIP ) && 1 == Count( findings, GUI::LAYOUTLINTRULE_NUMBERELIDED ) );
        ASSERT_TRUE( GUI::CLayoutLint::LooksNumeric( "4.0 KiB" ) && GUI::CLayoutLint::LooksNumeric( "-12,5%" ) && GUI::CLayoutLint::LooksNumeric( "$1.24M" ) );
        ASSERT_FALSE( GUI::CLayoutLint::LooksNumeric( "static-m..." ) || GUI::CLayoutLint::LooksNumeric( "Medieval We..." ) || GUI::CLayoutLint::LooksNumeric( "" ) );
        ASSERT_TRUE( GUI::CLayoutLint::EndsWithEllipsis( "abc..." ) && !GUI::CLayoutLint::EndsWithEllipsis( "abc.." ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: L06 BelowMinimum, L08 ZeroSize and the one line minimum height of single line text" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );

        // a pixel height below the line is raised to one line; the top stays for VAlign Top, the centre for Center
        GUI::CWidget* low = Add( context, screen, "Label", "Low", "0,10 0,10", "0,200 0,11", "Ingest: into the primary store" );
        GUI::CWidget* centred = Add( context, screen, "Label", "Centred", "0,10 0,100", "0,200 0,10", "Centred" );
        centred->SetProperty( "VAlign", "Center" );
        GUI::CWidget* rule = Add( context, screen, "Label", "Rule", "0,10 0,150", "0,200 0,2" );
        rule->SetProperty( "BackgroundColour", "90,90,90,255" );
        GUI::CWidget* field = Add( context, screen, "Editbox", "Field", "0,10 0,200", "0,200 0,12" );
        context->FlushChanges();
        const Int32 minimum = LintLineHeight - LintLineHeight / 8;                   // the line less its leading: 20 of 22
        ASSERT_TRUE( minimum == context->GetMinimumLineHeight( "Test" ) );
        ASSERT_TRUE( 10 == low->GetRect().top && minimum == low->GetRect().height );
        ASSERT_TRUE( 100 - minimum / 2 == centred->GetRect().top && minimum == centred->GetRect().height );
        ASSERT_TRUE( 2 == rule->GetRect().height );                                    // no text: a rule keeps its height
        ASSERT_TRUE( minimum == field->GetRect().height );

        // a height that is only a part of the parent is not raised: the lint says it is below the minimum
        Add( context, screen, "Label", "Squeezed", "0,300 0,10", "0,200 0.02,0", "Squeezed" );
        Add( context, screen, "Button", "Sliver", "0,300 0,100", "0,1 0,32", "Sliver" );
        GUI::TLayoutLintFindingVector findings = Lint( context );
        LogFindings( "minimum", findings );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_BELOWMINIMUM, "Squeezed" ) && 0 == Count( findings, GUI::LAYOUTLINTRULE_BELOWMINIMUM, "Low" ) );
        ASSERT_TRUE( GUI::LAYOUTLINTSEVERITY_WARNING == SeverityOf( findings, GUI::LAYOUTLINTRULE_BELOWMINIMUM ) );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_ZEROSIZE, "Sliver" ) && 0 == Count( findings, GUI::LAYOUTLINTRULE_ZEROSIZE, "Rule" ) );
        ASSERT_TRUE( 0 == Count( findings, GUI::LAYOUTLINTRULE_SMALLTARGET, "Sliver" ) );    // reported once, as having no size
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: L07 SmallTarget with the spacing exception, and L10 TightSpacing" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        Add( context, screen, "Button", "SmallA", "0,10 0,10", "0,20 0,20" );
        Add( context, screen, "Button", "SmallB", "0,30 0,10", "0,20 0,20" );
        Add( context, screen, "Button", "Alone", "0,300 0,300", "0,20 0,20" );
        Add( context, screen, "Button", "Spaced", "0,100 0,10", "0,20 0,20" );
        Add( context, screen, "Button", "Spaced2", "0,124 0,10", "0,20 0,20" );
        GUI::CWidget* disabled = Add( context, screen, "Button", "Disabled", "0,400 0,10", "0,20 0,20" );
        disabled->SetProperty( "Enabled", "false" );
        Add( context, screen, "Button", "Next", "0,402 0,10", "0,20 0,20" );
        GUI::TLayoutLintFindingVector findings = Lint( context );
        LogFindings( "targets", findings );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_SMALLTARGET, "SmallA" ) && 1 == Count( findings, GUI::LAYOUTLINTRULE_SMALLTARGET, "SmallB" ) );
        ASSERT_TRUE( 0 == Count( findings, GUI::LAYOUTLINTRULE_SMALLTARGET, "Alone" ) );
        ASSERT_TRUE( 0 == Count( findings, GUI::LAYOUTLINTRULE_SMALLTARGET, "Spaced" ) && 0 == Count( findings, GUI::LAYOUTLINTRULE_SMALLTARGET, "Spaced2" ) );
        ASSERT_TRUE( 0 == Count( findings, GUI::LAYOUTLINTRULE_SMALLTARGET, "Disabled" ) );     // not a target while disabled

        // controls side by side: 2 px apart is tight, 8 px is not, touching is tight unless both are a segmented group
        GUI::CWidget* left = Add( context, screen, "Button", "Left", "0,10 0,100", "0,60 0,32", "One" );
        GUI::CWidget* right = Add( context, screen, "Button", "Right", "0,72 0,100", "0,60 0,32", "Two" );
        Add( context, screen, "Button", "Far", "0,140 0,100", "0,60 0,32", "Three" );
        findings = Lint( context );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_TIGHTSPACING, "Right" ) && 0 == Count( findings, GUI::LAYOUTLINTRULE_TIGHTSPACING, "Far" ) );
        ASSERT_TRUE( GUI::LAYOUTLINTSEVERITY_WARNING == SeverityOf( findings, GUI::LAYOUTLINTRULE_TIGHTSPACING ) );
        right->SetProperty( "Position", "0,70 0,100" );
        left->SetProperty( "StyleClass", "Segmented" );
        right->SetProperty( "StyleClass", "Segmented" );
        ASSERT_TRUE( 0 == Count( Lint( context ), GUI::LAYOUTLINTRULE_TIGHTSPACING, "Right" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: L09 ViewportTooSmall and L16 EmptyViewport for lists, grids and text boxes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        const char* items = "one\ntwo\nthree\nfour\nfive\nsix\nseven\neight\nnine\nten";
        GUI::CWidget* small = Add( context, screen, "Listbox", "Small", "0,10 0,10", "0,200 0,30" );
        small->SetProperty( "Items", items );
        GUI::CWidget* enough = Add( context, screen, "Listbox", "Enough", "0,220 0,10", "0,200 0,100" );
        enough->SetProperty( "Items", items );
        GUI::CWidget* two = Add( context, screen, "Listbox", "Two", "0,430 0,10", "0,200 0,50" );
        two->SetProperty( "Items", "one\ntwo" );
        Add( context, screen, "Listbox", "Empty", "0,10 0,200", "0,200 0,100" );
        GUI::CGridView* grid = static_cast< GUI::CGridView* >( Add( context, screen, "GridView", "Header", "0,220 0,200", "0,200 0,26" ) );
        grid->SetProperty( "Columns", "Name\nSize" );
        grid->SetProperty( "Rows", "a,1\nb,2\nc,3\nd,4" );
        GUI::TLayoutLintFindingVector findings = Lint( context );
        LogFindings( "viewports", findings );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_VIEWPORTTOOSMALL, "Small" ) && 0 == Count( findings, GUI::LAYOUTLINTRULE_VIEWPORTTOOSMALL, "Enough" ) );
        ASSERT_TRUE( 0 == Count( findings, GUI::LAYOUTLINTRULE_VIEWPORTTOOSMALL, "Two" ) );     // shows all it has
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_VIEWPORTTOOSMALL, "Header" ) );  // only the header row, as the report view had
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_EMPTYVIEWPORT, "Empty" ) && GUI::LAYOUTLINTSEVERITY_INFO == SeverityOf( findings, GUI::LAYOUTLINTRULE_EMPTYVIEWPORT ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: L11 EstimatedText, L13 TabUnreachable, L15 LowContrast and L17 AbsoluteInFlow" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* unknown = Add( context, screen, "Label", "Unknown", "0,10 0,10", "0,200 0,28", "abc" );
        unknown->SetProperty( "Font", "NoSuchFont" );
        GUI::TLayoutLintFindingVector findings = Lint( context );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_ESTIMATEDTEXT, "Unknown" ) );

        // a strip of ten tabs in 5 pixels: no tab has room and there is no overflow button
        GUI::CWidget* tabs = Add( context, screen, "TabControl", "Tabs", "0,10 0,60", "0,5 0,40" );
        tabs->SetProperty( "Tabs", "A\nB\nC\nD\nE\nF\nG\nH\nI\nJ" );
        GUI::CWidget* wide = Add( context, screen, "TabControl", "Wide", "0,100 0,60", "0,400 0,40" );
        wide->SetProperty( "Tabs", "A\nB" );
        findings = Lint( context );
        LogFindings( "tabs", findings );
        ASSERT_TRUE( 10 == Count( findings, GUI::LAYOUTLINTRULE_TABUNREACHABLE, "Tabs" ) && 0 == Count( findings, GUI::LAYOUTLINTRULE_TABUNREACHABLE, "Wide" ) );

        // dark text on the dark screen; a disabled one is only info; white text is fine
        GUI::CWidget* dim = Add( context, screen, "Label", "Dim", "0,10 0,120", "0,200 0,28", "Dim" );
        dim->SetProperty( "TextColour", "45,45,50,255" );
        Add( context, screen, "Label", "Bright", "0,10 0,160", "0,200 0,28", "Bright" );
        GUI::CWidget* off = Add( context, screen, "Label", "Off", "0,10 0,200", "0,200 0,28", "Off" );
        off->SetProperty( "Enabled", "false" );
        off->SetProperty( "DisabledTextColour", "45,45,50,255" );
        findings = Lint( context );
        LogFindings( "contrast", findings );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_LOWCONTRAST, "Dim" ) && 0 == Count( findings, GUI::LAYOUTLINTRULE_LOWCONTRAST, "Bright" ) );
        for ( UInt32 i=0; i<(UInt32) findings.size(); ++i )
            if ( GUI::LAYOUTLINTRULE_LOWCONTRAST == findings[ i ].rule )
                ASSERT_TRUE( findings[ i ].widgetPath.HasSubstr( "Dim" ) >= 0 ? GUI::LAYOUTLINTSEVERITY_WARNING == findings[ i ].severity : GUI::LAYOUTLINTSEVERITY_INFO == findings[ i ].severity );
        ASSERT_TRUE( GUI::CLayoutLint::GetContrastRatio( GUI::CColour( 0, 0, 0 ), GUI::CColour( 255, 255, 255 ) ) > 20.9f );
        ASSERT_TRUE( GUI::CLayoutLint::GetContrastRatio( GUI::CColour( 118, 118, 118 ), GUI::CColour( 255, 255, 255 ) ) > 4.5f );

        // a plain panel of seven buttons placed by pixels: a candidate for boxes
        GUI::CWidget* toolbar = Add( context, screen, "Panel", "Toolbar", "0,10 0,300", "0,620 0,40" );
        for ( UInt32 b=0; b<7; ++b )
            Add( context, toolbar, "Button", ( "B" + CORE::UInt32ToString( b ) ).C_String(), ( "0," + CORE::UInt32ToString( b * 80 ) + " 0,4" ).C_String(), "0,72 0,32", "Tool" );
        findings = Lint( context );
        ASSERT_TRUE( 1 == Count( findings, GUI::LAYOUTLINTRULE_ABSOLUTEINFLOW, "Toolbar" ) && GUI::LAYOUTLINTSEVERITY_INFO == SeverityOf( findings, GUI::LAYOUTLINTRULE_ABSOLUTEINFLOW ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: Findings are sorted by severity, the report counts them and Run() counts the failures" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        Add( context, screen, "Listbox", "Empty", "0,10 0,10", "0,200 0,100" );
        Add( context, screen, "Button", "A", "0,10 0,200", "0,100 0,32", "A" );
        Add( context, screen, "Button", "B", "0,112 0,200", "0,100 0,32", "B" );
        Add( context, screen, "Button", "C", "0,150 0,200", "0,100 0,32", "C" );
        GUI::TLayoutLintFindingVector findings;
        GUI::CLayoutLint lint;
        GUI::CLayoutLintOptions options;
        const UInt32 failures = lint.Run( *context, options, findings );
        LogFindings( "sorted", findings );
        ASSERT_TRUE( failures == GUI::CLayoutLint::CountAtLeast( findings, GUI::LAYOUTLINTSEVERITY_ERROR ) && failures >= 1 );
        for ( UInt32 i=1; i<(UInt32) findings.size(); ++i )
            ASSERT_TRUE( findings[ i - 1 ].severity >= findings[ i ].severity );
        options.strict = true;
        ASSERT_TRUE( lint.Run( *context, options, findings ) == GUI::CLayoutLint::CountAtLeast( findings, GUI::LAYOUTLINTSEVERITY_WARNING ) );
        const CORE::CString report = GUI::CLayoutLint::Report( findings, 1 );
        ASSERT_TRUE( report.HasSubstr( "errors, " ) > 0 && report.HasSubstr( "more" ) > 0 );
        UInt32 counts[ GUI::LAYOUTLINTRULE_COUNT ] = { 0 };
        GUI::CLayoutLint::CountByRule( findings, GUI::LAYOUTLINTSEVERITY_INFO, counts );
        ASSERT_TRUE( 1 == counts[ GUI::LAYOUTLINTRULE_EMPTYVIEWPORT ] && counts[ GUI::LAYOUTLINTRULE_OVERLAP ] >= 1 );

        // only the subtree of a widget
        options.root = screen->GetChildWidget( "Empty" );
        lint.Run( *context, options, findings );
        ASSERT_TRUE( 1 == findings.size() && GUI::LAYOUTLINTRULE_EMPTYVIEWPORT == findings[ 0 ].rule );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: The budget reads its lines, allows up to its limits, says what can be lowered and starts new scopes at zero" )
    try
    {
        GUI::CLayoutLintBudget budget;
        CORE::CString problems;
        ASSERT_TRUE( budget.LoadFromText( "# a comment\n\nFormA L04 3\nFormA Overlap 1\n", problems ) && problems.IsNULLOrEmpty() );
        ASSERT_FALSE( budget.LoadFromText( "FormB L99 2\nFormB L04\n", problems ) );
        ASSERT_TRUE( problems.HasSubstr( "L99" ) >= 0 );
        ASSERT_TRUE( 3 == budget.GetLimit( "FormA", GUI::LAYOUTLINTRULE_TEXTCLIPPED ) && 1 == budget.GetLimit( "FormA", GUI::LAYOUTLINTRULE_OVERLAP ) );
        ASSERT_TRUE( 0 == budget.GetLimit( "FormC", GUI::LAYOUTLINTRULE_OVERLAP ) );

        UInt32 counts[ GUI::LAYOUTLINTRULE_COUNT ] = { 0 };
        counts[ GUI::LAYOUTLINTRULE_TEXTCLIPPED ] = 2;
        counts[ GUI::LAYOUTLINTRULE_OVERLAP ] = 1;
        CORE::CString over, lower;
        ASSERT_TRUE( budget.Check( "FormA", counts, over, lower ) && over.IsNULLOrEmpty() && lower.HasSubstr( "L04" ) >= 0 );
        counts[ GUI::LAYOUTLINTRULE_OVERLAP ] = 2;
        ASSERT_FALSE( budget.Check( "FormA", counts, over, lower ) );
        ASSERT_TRUE( over.HasSubstr( "L01" ) >= 0 );
        over.Clear();
        ASSERT_FALSE( budget.Check( "FormC", counts, over, lower ) );     // a new scope is allowed nothing
        ASSERT_TRUE( "FormA L01 2\nFormA L04 2\n" == GUI::CLayoutLintBudget::FormatCounts( "FormA", counts ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: The matrix has the sizes from 640 x 480 to ultrawide, pseudo-localised text, the render scale and the stresses" )
    try
    {
        GUI::CLayoutLintMatrix matrix;
        ASSERT_TRUE( 14 == matrix.GetCombinationCount() );
        ASSERT_TRUE( "640x480" == matrix.DescribeCombination( 0 ) && "3440x1440" == matrix.DescribeCombination( 5 ) && "1600x900 pseudo" == matrix.DescribeCombination( 9 ) );
        ASSERT_TRUE( "1920x1080 render scale 2" == matrix.DescribeCombination( 12 ) && "1920x1080 pseudo line spacing 1.5" == matrix.DescribeCombination( 13 ) );
        matrix.SetTextScaleSupported( true );
        ASSERT_TRUE( 15 == matrix.GetCombinationCount() && "1920x1080 text scale 2" == matrix.DescribeCombination( 14 ) );
        ASSERT_TRUE( 2.0f == matrix.GetCombination( 14 ).textScale );

        // a form minimum leaves the smaller sizes out
        GUI::CLayoutLintMatrix minimum;
        minimum.SetMinimumSize( 1000, 700 );
        ASSERT_TRUE( "1000x700" == minimum.DescribeCombination( 0 ) && "1024x768" == minimum.DescribeCombination( 1 ) );

        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* label = Add( context, screen, "Label", "Name", "0,10 0,10", "0,300 0,28", "Save" );
        matrix.ApplyCombination( 13, *context );
        context->FlushChanges();
        ASSERT_TRUE( 1920 == context->GetLogicalWidth() && 1080 == context->GetLogicalHeight() && 1920 == screen->GetRect().width );
        ASSERT_TRUE( 33 == context->GetLineHeight( "Test" ) );                     // the line spacing stress of 1.5
        GUI::CDrawSurface::TCommandVector texts = TextCommands( context, label );
        ASSERT_TRUE( 1 == texts.size() && "[S\xC3\xA1v\xC3\xA9\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7]" == texts[ 0 ].text );
        ASSERT_TRUE( "Save" == label->GetProperty( "Text" ) );                       // what is shown changes, the property does not
        matrix.ApplyCombination( 12, *context );
        ASSERT_TRUE( 2 == context->GetRenderScale() && GUCEF_NULL == context->GetDisplayTextTransform() );
        matrix.Restore( *context );
        context->FlushChanges();
        ASSERT_TRUE( 640 == context->GetLogicalWidth() && 1 == context->GetRenderScale() && LintLineHeight == context->GetLineHeight( "Test" ) );
        texts = TextCommands( context, label );
        ASSERT_TRUE( 1 == texts.size() && "Save" == texts[ 0 ].text );

        // a sweep over the matrix adds up the findings; without a way to scale the fonts it skips the text scale
        Add( context, screen, "Button", "A", "0,10 0,100", "0,100 0,32", "A" );
        Add( context, screen, "Button", "B", "0,60 0,100", "0,100 0,32", "B" );
        GUI::CLayoutLintSweep sweep;
        const UInt32 errors = sweep.Run( *context, matrix, GUI::CLayoutLintOptions() );
        ASSERT_TRUE( errors == sweep.GetCount( GUI::LAYOUTLINTSEVERITY_ERROR ) && 14 == sweep.GetRuleCounts()[ GUI::LAYOUTLINTRULE_OVERLAP ] );
        ASSERT_TRUE( sweep.GetSummary().HasSubstr( "1920x1080 text scale 2: skipped" ) >= 0 && sweep.GetFindings( 14 ).empty() );
        ASSERT_TRUE( 1 == Count( sweep.GetFindings( 3 ), GUI::LAYOUTLINTRULE_OVERLAP, "B" ) );
        ASSERT_TRUE( 640 == context->GetLogicalWidth() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: Pseudo-localisation lengthens, accents and brackets letters, and leaves numbers alone" )
    try
    {
        GUI::CPseudoLocalizer pseudo;
        ASSERT_TRUE( "[S\xC3\xA1v\xC3\xA9\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7]" == pseudo.Transform( "Save" ) );       // short: 100 % more
        ASSERT_TRUE( "12.5" == pseudo.Transform( "12.5" ) && "0:00.000 / 0:00.000" == pseudo.Transform( "0:00.000 / 0:00.000" ) );
        ASSERT_TRUE( pseudo.Transform( CORE::CString::Empty ).IsNULLOrEmpty() );

        // 20 characters: 40 % more, 8 dots; the bracket shows a cut
        const CORE::CString long20 = pseudo.Transform( "Add to primary store" );
        const char* bytes = long20.C_String();
        const size_t length = strlen( bytes );
        UInt32 dots = 0;
        for ( size_t i=0; i+1<length; ++i )
            if ( 0xC2 == (unsigned char) bytes[ i ] && 0xB7 == (unsigned char) bytes[ i + 1 ] )
                ++dots;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Pseudo-localised: " << dots << " dots in " << (UInt32) length << " bytes, CString length " << long20.Length() );
        ASSERT_TRUE( 8 == dots && length > 2 && '[' == bytes[ 0 ] && ']' == bytes[ length - 1 ] );
        pseudo.SetBrackets( false );
        pseudo.SetAccents( false );
        ASSERT_TRUE( "Save\xC2\xB7\xC2\xB7\xC2\xB7\xC2\xB7" == pseudo.TransformText( "Save" ) );
        pseudo.SetLineSpacing( 0.5f );
        ASSERT_TRUE( 1.0f == pseudo.GetLineSpacing() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TextOverflow: Labels, buttons, check boxes and combo boxes shorten with an ellipsis by default, with the whole text as tooltip" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* label = Add( context, screen, "Label", "Label", "0,10 0,10", "0,90 0,28", "A long text here" );
        GUI::CWidget* button = Add( context, screen, "Button", "Button", "0,10 0,50", "0,62 0,32", "Delete all" );
        GUI::CWidget* check = Add( context, screen, "Checkbox", "Check", "0,10 0,100", "0,80 0,28", "Show hidden" );
        GUI::CWidget* combo = Add( context, screen, "Combobox", "Combo", "0,10 0,140", "0,100 0,28", "Relevance first" );
        GUI::CWidget* fits = Add( context, screen, "Button", "Fits", "0,200 0,50", "0,62 0,32", "OK" );
        GUI::CWidget* group = Add( context, screen, "GroupBox", "Group", "0,10 0,200", "0,120 0,100" );
        group->SetProperty( "Caption", "Add folder or archive to primary store" );

        GUI::CDrawSurface::TCommandVector texts = TextCommands( context, label );
        ASSERT_TRUE( 1 == texts.size() && "A long..." == texts[ 0 ].text && 0 != ( texts[ 0 ].textFlags & GUI::CDrawSurface::TEXTFLAG_ELIDED ) );
        ASSERT_TRUE( 90 == texts[ 0 ].textArea.width );
        texts = TextCommands( context, button );                                           // inside the padding of 4 on both sides: 54 px
        ASSERT_TRUE( 1 == texts.size() && "Del..." == texts[ 0 ].text && texts[ 0 ].x0 >= 4 );
        texts = TextCommands( context, fits );
        ASSERT_TRUE( 1 == texts.size() && "OK" == texts[ 0 ].text && 0 == texts[ 0 ].textFlags );
        texts = TextCommands( context, check );
        ASSERT_TRUE( 1 == texts.size() && 0 != ( texts[ 0 ].textFlags & GUI::CDrawSurface::TEXTFLAG_ELIDED ) );
        texts = TextCommands( context, combo );
        ASSERT_TRUE( 1 == texts.size() && 0 != ( texts[ 0 ].textFlags & GUI::CDrawSurface::TEXTFLAG_ELIDED ) );
        texts = TextCommands( context, group );
        ASSERT_TRUE( 1 == texts.size() && 0 != ( texts[ 0 ].textFlags & GUI::CDrawSurface::TEXTFLAG_ELIDED ) );

        // the tooltip of a shortened text is the whole text; above the widget's own tooltip when it has one; none when it fits
        GUI::CTooltipService& tooltips = context->GetTooltipService();
        CORE::CString full;
        ASSERT_TRUE( GUI::CWidgetPainter::IsOwnTextElided( *label, *context, full ) && "A long text here" == full );
        ASSERT_TRUE( "A long text here" == tooltips.GetTooltipText( *label, 5, 5 ) );
        button->SetProperty( "Tooltip", "Removes every entry" );
        ASSERT_TRUE( "Delete all\nRemoves every entry" == tooltips.GetTooltipText( *button, 5, 5 ) );
        ASSERT_TRUE( "Add folder or archive to primary store" == tooltips.GetTooltipText( *group, 5, 5 ) );
        ASSERT_TRUE( tooltips.GetTooltipText( *fits, 5, 5 ).IsNULLOrEmpty() );
        ASSERT_FALSE( GUI::CWidgetPainter::IsOwnTextElided( *fits, *context, full ) );

        // the old way stays available: Clip cuts at the edge without an ellipsis
        label->SetProperty( "TextOverflow", "Clip" );
        texts = TextCommands( context, label );
        ASSERT_TRUE( 1 == texts.size() && "A long text here" == texts[ 0 ].text && 0 == texts[ 0 ].textFlags );
        ASSERT_TRUE( tooltips.GetTooltipText( *label, 5, 5 ).IsNULLOrEmpty() );

        // a path shortened in the middle keeps its start and its extension
        label->SetProperty( "TextOverflow", "EllipsisMiddle" );
        label->SetProperty( "Size", "0,180 0,28" );
        label->SetProperty( "Text", "/home/user/assets/textures/stone_wall_diffuse.png" );
        texts = TextCommands( context, label );
        ASSERT_TRUE( 1 == texts.size() && texts[ 0 ].text.HasSubstr( "/home" ) == 0 && texts[ 0 ].text.HasSubstr( "..." ) > 0 );
        ASSERT_TRUE( texts[ 0 ].text.SubstrFromRange( texts[ 0 ].text.Length() - 4, texts[ 0 ].text.Length() ) == ".png" && texts[ 0 ].text.Length() * LintGlyphWidth <= 180 );

        // wrapped text that is higher than its label ends with "..." on its last line
        GUI::CWidget* wrapped = Add( context, screen, "Label", "Wrapped", "0,300 0,10", "0,90 0,44", "one two three four five six seven" );
        wrapped->SetProperty( "WordWrap", "true" );
        texts = TextCommands( context, wrapped );
        ASSERT_TRUE( 2 == texts.size() && 0 != ( texts[ 1 ].textFlags & GUI::CDrawSurface::TEXTFLAG_ELIDED ) && GUI::CLayoutLint::EndsWithEllipsis( texts[ 1 ].text ) );
        ASSERT_TRUE( 0 == Count( Lint( context ), GUI::LAYOUTLINTRULE_TEXTCLIPPED, "Wrapped" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TextOverflow: FitText shortens at the end, the start or the middle and keeps the extension of a file name" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        bool cut = false;
        ASSERT_TRUE( "abc..." == GUI::CWidgetPainter::FitText( *context, "Test", "abcdefghij", 54, GUI::TEXTOVERFLOW_ELLIPSISEND, cut ) && cut );
        ASSERT_TRUE( "...hij" == GUI::CWidgetPainter::FitText( *context, "Test", "abcdefghij", 54, GUI::TEXTOVERFLOW_ELLIPSISSTART, cut ) && cut );
        ASSERT_TRUE( "abcdefghij" == GUI::CWidgetPainter::FitText( *context, "Test", "abcdefghij", 54, GUI::TEXTOVERFLOW_CLIP, cut ) && !cut );
        ASSERT_TRUE( "abcdefghij" == GUI::CWidgetPainter::FitText( *context, "Test", "abcdefghij", 90, GUI::TEXTOVERFLOW_ELLIPSISMIDDLE, cut ) && !cut );
        const CORE::CString middle = GUI::CWidgetPainter::FitText( *context, "Test", "sound_effect_sword_hit.wav", 135, GUI::TEXTOVERFLOW_ELLIPSISMIDDLE, cut );
        ASSERT_TRUE( cut && 15 == middle.Length() && middle.HasSubstr( "sou" ) == 0 && middle.HasSubstr( "...") > 0 && middle.HasSubstr( ".wav" ) == 11 );
        ASSERT_TRUE( GUI::CWidgetPainter::FitText( *context, "Test", "abcdefghij", 20, GUI::TEXTOVERFLOW_ELLIPSISMIDDLE, cut ).IsNULLOrEmpty() && cut );
        ASSERT_TRUE( GUI::TEXTOVERFLOW_ELLIPSISEND == GUI::StringToTextOverflow( "Ellipsis", GUI::TEXTOVERFLOW_CLIP ) );
        ASSERT_TRUE( GUI::TEXTOVERFLOW_ELLIPSISMIDDLE == GUI::StringToTextOverflow( "EllipsisMiddle", GUI::TEXTOVERFLOW_CLIP ) );
        ASSERT_TRUE( GUI::TEXTOVERFLOW_CLIP == GUI::StringToTextOverflow( "Nonsense", GUI::TEXTOVERFLOW_CLIP ) );
        ASSERT_TRUE( "EllipsisStart" == GUI::TextOverflowToString( GUI::TEXTOVERFLOW_ELLIPSISSTART ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: The debug overlay is off until switched on, then outlines the findings and runs again after a layout" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        Add( context, screen, "Button", "First", "0,20 0,20", "0,100 0,32", "First" );
        GUI::CWidget* second = Add( context, screen, "Button", "Second", "0,80 0,30", "0,100 0,32", "Second" );
        context->FlushChanges();

        UInt32 mode = 0;
        ASSERT_TRUE( GUI::CLayoutDebugOverlay::ParseMode( "overlay,log", mode ) && ( GUI::CLayoutDebugOverlay::MODE_OVERLAY | GUI::CLayoutDebugOverlay::MODE_LOG ) == mode );
        ASSERT_TRUE( GUI::CLayoutDebugOverlay::ParseMode( "on", mode ) && GUI::CLayoutDebugOverlay::MODE_OVERLAY == mode );
        ASSERT_TRUE( GUI::CLayoutDebugOverlay::ParseMode( "off", mode ) && GUI::CLayoutDebugOverlay::MODE_OFF == mode );
        ASSERT_FALSE( GUI::CLayoutDebugOverlay::ParseMode( "loud", mode ) );

        GUI::CLayoutDebugOverlay& overlay = context->GetLayoutDebugOverlay();
        GUI::CDrawSurface::TCommandVector commands;
        GUI::CWidgetPainter::PaintOverlay( *context, commands );
        ASSERT_FALSE( overlay.IsOn() );
        ASSERT_TRUE( commands.empty() && overlay.GetFindings().empty() );

        overlay.SetMode( GUI::CLayoutDebugOverlay::MODE_OVERLAY );
        commands.clear();
        GUI::CWidgetPainter::PaintOverlay( *context, commands );
        ASSERT_TRUE( 1 == Count( overlay.GetFindings(), GUI::LAYOUTLINTRULE_OVERLAP, "Second" ) );
        bool outlined = false, labelled = false;
        for ( UInt32 i=0; i<(UInt32) commands.size(); ++i )
        {
            if ( GUI::CDrawSurface::COMMAND_STROKERECT == commands[ i ].type && commands[ i ].rect == second->GetRect() && 230 == commands[ i ].colour.r )
                outlined = true;
            if ( GUI::CDrawSurface::COMMAND_TEXT == commands[ i ].type && "L01" == commands[ i ].text )
                labelled = true;
        }
        ASSERT_TRUE( outlined && labelled );

        // after a layout that moves it apart there is nothing to show
        second->SetProperty( "Position", "0,200 0,30" );
        context->FlushChanges();
        commands.clear();
        GUI::CWidgetPainter::PaintOverlay( *context, commands );
        ASSERT_TRUE( 0 == Count( overlay.GetFindings(), GUI::LAYOUTLINTRULE_OVERLAP ) );
        overlay.SetMode( GUI::CLayoutDebugOverlay::MODE_OFF );
        ASSERT_TRUE( overlay.GetFindings().empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutLint: The developer console command gui.lint switches the overlay of the layers and writes what the lint found" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        Add( context, screen, "Button", "First", "0,20 0,200", "0,100 0,32", "First" );
        Add( context, screen, "Button", "Second", "0,80 0,210", "0,100 0,32", "Second" );
        GUI::CDeveloperConsoleForm console( context );
        console.SetSlideTimeInMs( 10 );
        ASSERT_TRUE( console.Build() && console.Open() );
        context->RenderFrame( 50 );
        GUI::CEditbox* line = console.GetWidgetAs< GUI::CEditbox >( "CommandLine" );
        GUI::CTextbox* output = console.GetWidgetAs< GUI::CTextbox >( "Output" );
        ASSERT_TRUE( GUCEF_NULL != line && GUCEF_NULL != output );
        if ( GUCEF_NULL == line || GUCEF_NULL == output )
            return;
        const char* const commands[] = { "gui.lint overlay,log", "gui.lint report", "gui.lint loud", "gui.lint off" };
        for ( UInt32 i=0; i<4; ++i )
        {
            context->SetFocus( line );
            line->SetText( commands[ i ] );
            PressKey( context, INPUT::KEYCODE_RETURN );
            if ( 0 == i )
            {
                ASSERT_TRUE( ( GUI::CLayoutDebugOverlay::MODE_OVERLAY | GUI::CLayoutDebugOverlay::MODE_LOG ) == context->GetLayoutDebugOverlay().GetMode() );
                GUI::CDrawSurface::TCommandVector overlay;
                GUI::CWidgetPainter::PaintOverlay( *context, overlay );     // the lint runs with the next overlay pass
            }
        }
        const CORE::CString text = output->GetTextAsString();
        ASSERT_TRUE( text.HasSubstr( "Layout lint on for 1 layers" ) >= 0 && text.HasSubstr( "L01 Overlap" ) >= 0 );
        ASSERT_TRUE( text.HasSubstr( "Usage: gui.lint" ) >= 0 && text.HasSubstr( "Layout lint off" ) >= 0 );
        ASSERT_FALSE( context->GetLayoutDebugOverlay().IsOn() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // the forms that have no test file of their own here; the others call RunFormLintBaseline() from theirs
    RunFormLintBaseline( "OpenActionStrip", &BuildOpenStripScene );
    RunFormLintBaseline( "FileOpenDialog", &BuildFileDialogScene );
}

/*-------------------------------------------------------------------------*/
