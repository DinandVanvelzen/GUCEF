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

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_CSplitter.h"
#include "gucefGUI_CFoldablePanel.h"
#include "gucefGUI_CLayoutState.h"
#include "gucefGUI_CLayoutManager.h"
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

/** Counts the events of one notifier */
class CCounter : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CCounter > TEventCallback;

    UInt32 count;

    CCounter( CORE::CNotifier* notifier, const CORE::CEvent& id )
        : CORE::CObserver(), count( 0 )
    {
        TEventCallback callback( this, &CCounter::OnEvent );
        SubscribeTo( notifier, id, callback );
    }

    virtual ~CCounter() { UnsubscribeAllFromObserver(); }

    void OnEvent( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) { ++count; }
};

bool
Near( Float32 a, Float32 b, Float32 tolerance = 0.002f )
{
    return a - b <= tolerance && b - a <= tolerance;
}

/** A splitter of the whole 640 x 480 screen with two panels as panes */
struct SSplitFixture
{
    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUI::CWidget* screen;
    GUI::CSplitter* splitter;
    GUI::CWidget* first;
    GUI::CWidget* second;

    SSplitFixture( bool sideBySide = true )
        : contextPtr( CreateNullContext() ), context( static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() ) )
        , screen( GUCEF_NULL ), splitter( GUCEF_NULL ), first( GUCEF_NULL ), second( GUCEF_NULL )
    {
        screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        screen->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( screen, false );
        splitter = static_cast< GUI::CSplitter* >( context->CreateWidget( "Splitter", "Split", screen ) );
        splitter->SetProperty( "Size", "1,0 1,0" );
        splitter->SetProperty( "Orientation", sideBySide ? "Horizontal" : "Vertical" );
        first = context->CreateWidget( "Panel", "First", splitter );
        second = context->CreateWidget( "Panel", "Second", splitter );
        first->SetProperty( "Size", "1,0 1,0" );
        second->SetProperty( "Size", "1,0 1,0" );
        context->FlushChanges();
    }
};

void
Press( GUI::CGUIContext* context, Int32 x, Int32 y )
{
    context->InjectMouseMove( x, y );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
}

void
Release( GUI::CGUIContext* context )
{
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
}

void
Click( GUI::CGUIContext* context, Int32 x, Int32 y )
{
    Press( context, x, y );
    Release( context );
}

GUI::CColour
PixelAt( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y )
{
    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
    return GUI::CColour( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiSplitterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.Splitter" );

    GUCEF_TESTFW_TESTCASE( "Splitter: the panes share the room less the handle by the ratio, vertical and horizontal" )
    try
    {
        SSplitFixture h( true );
        // 640 less the handle of 6 is 634, half of it for the first pane
        ASSERT_TRUE( 317 == h.splitter->GetFirstPaneExtent() && 6 == h.splitter->GetHandleExtent() && 317 == h.splitter->GetSecondPaneExtent() );
        ASSERT_TRUE( GUI::CRect( 0, 0, 317, 480 ) == h.first->GetRect() );
        ASSERT_TRUE( GUI::CRect( 323, 0, 317, 480 ) == h.second->GetRect() );
        ASSERT_TRUE( GUI::CRect( 317, 0, 6, 480 ) == h.splitter->GetHandleRect() );

        h.splitter->SetSplitRatio( 0.25f );
        h.context->FlushChanges();
        ASSERT_TRUE( GUI::CRect( 0, 0, 159, 480 ) == h.first->GetRect() );
        ASSERT_TRUE( 159 + 6 == h.second->GetRect().left );

        SSplitFixture v( false );
        ASSERT_TRUE( 237 == v.splitter->GetFirstPaneExtent() );
        ASSERT_TRUE( GUI::CRect( 0, 0, 640, 237 ) == v.first->GetRect() );
        ASSERT_TRUE( GUI::CRect( 0, 243, 640, 237 ) == v.second->GetRect() );
        ASSERT_TRUE( GUI::CRect( 0, 237, 640, 6 ) == v.splitter->GetHandleRect() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Splitter: dragging the handle moves it by the pointer delta, never past the minimum sizes of both panes" )
    try
    {
        SSplitFixture h( true );
        h.splitter->SetIntProperty( "FirstMinSize", 50 );
        h.splitter->SetIntProperty( "SecondMinSize", 70 );
        h.context->FlushChanges();
        CCounter changed( h.splitter, GUI::CSplitter::SizeChangedEvent );

        // grab the handle in its middle (x 319), move 100 pixels to the right
        Press( h.context, 319, 200 );
        ASSERT_TRUE( h.splitter->IsDragging() );
        h.context->InjectMouseMove( 419, 200 );
        h.context->FlushChanges();
        ASSERT_TRUE( 417 == h.first->GetRect().width && 417 + 6 == h.second->GetRect().left );
        ASSERT_TRUE( changed.count >= 1 );

        // far to the right: the second pane keeps its minimum
        h.context->InjectMouseMove( 5000, 200 );
        h.context->FlushChanges();
        ASSERT_TRUE( 70 == h.second->GetRect().width && 640 - 70 - 6 == h.first->GetRect().width );
        // far to the left: the first pane keeps its minimum
        h.context->InjectMouseMove( -4000, 200 );
        h.context->FlushChanges();
        ASSERT_TRUE( 50 == h.first->GetRect().width );
        Release( h.context );
        ASSERT_FALSE( h.splitter->IsDragging() );

        // a press beside the handle does not drag
        const UInt32 before = changed.count;
        Press( h.context, 300, 200 );
        h.context->InjectMouseMove( 400, 200 );
        Release( h.context );
        ASSERT_TRUE( before == changed.count && 50 == h.first->GetRect().width );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Splitter: a double click on the handle goes back to the default ratio" )
    try
    {
        SSplitFixture h( true );
        h.splitter->SetFloatProperty( "DefaultSplitRatio", 0.3f );
        h.splitter->SetSplitRatio( 0.3f );
        h.context->FlushChanges();
        const Int32 defaultWidth = h.first->GetRect().width;
        ASSERT_TRUE( 190 == defaultWidth );

        Press( h.context, h.first->GetRect().width + 3, 100 );
        h.context->InjectMouseMove( 400, 100 );
        Release( h.context );
        h.context->FlushChanges();
        ASSERT_TRUE( h.first->GetRect().width > 300 );

        const Int32 x = h.first->GetRect().width + 3;
        Click( h.context, x, 100 );
        Click( h.context, x, 100 );
        h.context->FlushChanges();
        ASSERT_TRUE( defaultWidth == h.first->GetRect().width );
        ASSERT_TRUE( Near( 0.3f, h.splitter->GetSplitRatio() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Splitter: the arrow keys along the splitter move the handle by a step while it has the focus, within the minimum sizes" )
    try
    {
        SSplitFixture h( true );
        h.splitter->SetIntProperty( "FirstMinSize", 100 );
        h.context->FlushChanges();
        ASSERT_TRUE( h.splitter->IsFocusable() );
        ASSERT_TRUE( h.splitter->SetFocus() );
        const Int32 start = h.first->GetRect().width;
        h.context->InjectNavigation( GUI::NAVIGATION_RIGHT );
        h.context->FlushChanges();
        ASSERT_TRUE( start + GUI::CSplitter::KEYSTEP == h.first->GetRect().width );
        h.context->InjectNavigation( GUI::NAVIGATION_LEFT );
        h.context->InjectNavigation( GUI::NAVIGATION_LEFT );
        h.context->FlushChanges();
        ASSERT_TRUE( start - GUI::CSplitter::KEYSTEP == h.first->GetRect().width );
        // up and down mean nothing for panes side by side
        h.context->InjectNavigation( GUI::NAVIGATION_DOWN );
        h.context->FlushChanges();
        ASSERT_TRUE( start - GUI::CSplitter::KEYSTEP == h.first->GetRect().width );
        for ( UInt32 i=0; i<40; ++i )
            h.context->InjectNavigation( GUI::NAVIGATION_LEFT );
        h.context->FlushChanges();
        ASSERT_TRUE( 100 == h.first->GetRect().width );

        // above each other, up and down
        SSplitFixture v( false );
        ASSERT_TRUE( v.splitter->SetFocus() );
        const Int32 startHeight = v.first->GetRect().height;
        v.context->InjectNavigation( GUI::NAVIGATION_DOWN );
        v.context->FlushChanges();
        ASSERT_TRUE( startHeight + GUI::CSplitter::KEYSTEP == v.first->GetRect().height );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Splitter: the pointer over the handle is a resize cursor of the right direction and the arrow again beside it" )
    try
    {
        SSplitFixture h( true );
        h.context->SetSystemCursorShape( GUI::CURSORSHAPE_ARROW );
        h.context->InjectMouseMove( 319, 100 );
        ASSERT_TRUE( GUI::CURSORSHAPE_RESIZE_EW == h.context->GetSystemCursorShape() && h.splitter->IsHandleActive() );
        h.context->InjectMouseMove( 100, 100 );
        ASSERT_TRUE( GUI::CURSORSHAPE_ARROW == h.context->GetSystemCursorShape() && !h.splitter->IsHandleActive() );

        SSplitFixture v( false );
        v.context->InjectMouseMove( 100, 240 );
        ASSERT_TRUE( GUI::CURSORSHAPE_RESIZE_NS == v.context->GetSystemCursorShape() );
        v.context->InjectMouseMove( 100, 20 );
        ASSERT_TRUE( GUI::CURSORSHAPE_ARROW == v.context->GetSystemCursorShape() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Splitter: a foldable pane folds into its strip with a click and the other pane gets the room, unfolding brings the old size back" )
    try
    {
        SSplitFixture h( true );
        GUI::CWidget* first = h.first;
        h.context->DestroyWidget( first );
        GUI::CFoldablePanel* panel = static_cast< GUI::CFoldablePanel* >( h.context->CreateWidget( "FoldablePanel", "Left", h.splitter ) );
        panel->SetProperty( "Size", "1,0 1,0" );
        panel->SetProperty( "Title", "Files" );
        panel->SetProperty( "Dock", "Left" );
        GUI::CWidget* child = h.context->CreateWidget( "Button", "Inside", panel );
        child->SetProperty( "Size", "0,50 0,20" );
        // a foldable panel made after the second pane is the second child: put the panes in order
        h.second->SetParentWidget( GUCEF_NULL );
        h.second->SetParentWidget( h.splitter );
        h.splitter->SetSplitRatio( 0.4f );
        h.context->FlushChanges();
        ASSERT_TRUE( panel == h.splitter->GetFirstPane() && h.second == h.splitter->GetSecondPane() );
        const GUI::CRect open = panel->GetRect();
        ASSERT_TRUE( 254 == open.width && !panel->IsFolded() );
        // the button is below the 24 pixel header
        ASSERT_TRUE( 24 == child->GetRect().top && GUI::CRect( 0, 0, 254, 24 ) == panel->GetHeaderRect() );

        CCounter folded( panel, GUI::CFoldablePanel::FoldedChangedEvent );
        CCounter collapsed( h.splitter, GUI::CSplitter::CollapsedChangedEvent );
        Click( h.context, 20, 10 );
        h.context->FlushChanges();
        ASSERT_TRUE( panel->IsFolded() && 1 == folded.count && 1 == collapsed.count );
        ASSERT_TRUE( 24 == panel->GetRect().width && 480 == panel->GetRect().height );
        ASSERT_TRUE( 24 == h.second->GetRect().left && 640 - 24 == h.second->GetRect().width );
        ASSERT_TRUE( h.splitter->IsFirstPaneFolded() && h.splitter->GetHandleRect().IsEmpty() );
        ASSERT_FALSE( child->IsEffectivelyVisible() && panel->AreChildrenShown() );

        // the strip folds out again
        Click( h.context, 10, 300 );
        h.context->FlushChanges();
        ASSERT_TRUE( !panel->IsFolded() && 2 == folded.count && 2 == collapsed.count );
        ASSERT_TRUE( open == panel->GetRect() && Near( 0.4f, h.splitter->GetSplitRatio() ) );

        // by code as well
        ASSERT_TRUE( panel->SetFolded( true ) );
        h.context->FlushChanges();
        ASSERT_TRUE( 24 == panel->GetRect().width );
        ASSERT_TRUE( panel->ToggleFolded() );
        h.context->FlushChanges();
        ASSERT_TRUE( open == panel->GetRect() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Splitter: a foldable panel outside a splitter folds to the strip at its dock edge" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        screen->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( screen, false );
        GUI::CFoldablePanel* right = static_cast< GUI::CFoldablePanel* >( context->CreateWidget( "FoldablePanel", "Right", screen ) );
        right->SetProperty( "Position", "1,-200 0,0" );
        right->SetProperty( "Size", "0,200 1,0" );
        right->SetProperty( "Dock", "Right" );
        GUI::CFoldablePanel* bottom = static_cast< GUI::CFoldablePanel* >( context->CreateWidget( "FoldablePanel", "Bottom", screen ) );
        bottom->SetProperty( "Position", "0,0 1,-100" );
        bottom->SetProperty( "Size", "0,300 0,100" );
        bottom->SetProperty( "Dock", "Bottom" );
        context->FlushChanges();
        ASSERT_TRUE( GUI::CRect( 440, 0, 200, 480 ) == right->GetRect() && GUI::CRect( 0, 380, 300, 100 ) == bottom->GetRect() );
        right->SetFolded( true );
        bottom->SetFolded( true );
        context->FlushChanges();
        ASSERT_TRUE( GUI::CRect( 616, 0, 24, 480 ) == right->GetRect() );
        ASSERT_TRUE( GUI::CRect( 0, 456, 300, 24 ) == bottom->GetRect() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutState: sizes and folded states are captured, applied and written and read through a settings node, with unknown and missing keys" )
    try
    {
        SSplitFixture h( true );
        GUI::CFoldablePanel* panel = static_cast< GUI::CFoldablePanel* >( h.context->CreateWidget( "FoldablePanel", "SidePanel", h.screen ) );
        panel->SetProperty( "Size", "0,100 0,100" );
        h.splitter->SetSplitRatio( 0.7f );
        panel->SetFolded( true );
        h.context->FlushChanges();

        GUI::CLayoutState state;
        ASSERT_TRUE( 2 == state.Capture( *h.screen ) );
        Float32 ratio = 0.0f;
        bool folded = false;
        ASSERT_TRUE( state.TryGetSplitRatio( "Split", ratio ) && Near( 0.7f, ratio ) && state.TryGetFolded( "SidePanel", folded ) && folded );

        CORE::CDataNode settings( "Config" );
        CORE::CDataNode* section = settings.Structure( "App", '/' );
        ASSERT_TRUE( state.Save( *section ) );
        GUI::CLayoutState loaded;
        ASSERT_TRUE( loaded.Load( *section ) && loaded == state );

        // through JSON, as an application config file has it
        if ( LoadLayoutCodecs() )
        {
            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) );
            CORE::CDynamicBuffer buffer;
            CORE::CDynamicBufferAccess output( buffer );
            ASSERT_TRUE( codec->StoreDataTree( &settings, &output ) );
            CORE::CDynamicBufferAccess input( buffer );
            CORE::CDataNode parsed;
            ASSERT_TRUE( codec->BuildDataTree( &parsed, &input ) );
            const CORE::CDataNode* parsedSection = parsed.Search( "App", '/', true, true );
            GUI::CLayoutState fromJson;
            ASSERT_TRUE( GUCEF_NULL != parsedSection && fromJson.Load( *parsedSection ) );
            ASSERT_TRUE( 1 == fromJson.GetSplitRatios().size() && 1 == fromJson.GetFolded().size() );
            ASSERT_TRUE( fromJson.TryGetSplitRatio( "Split", ratio ) && Near( 0.7f, ratio ) && fromJson.TryGetFolded( "SidePanel", folded ) && folded );
        }

        // applied to a window that is shaped otherwise
        h.splitter->SetSplitRatio( 0.1f );
        panel->SetFolded( false );
        ASSERT_TRUE( 2 == state.Apply( *h.screen ) );
        ASSERT_TRUE( Near( 0.7f, h.splitter->GetSplitRatio() ) && panel->IsFolded() );

        // what is not understood is skipped: an unknown node, an entry without a name, a ratio that is no number or out of range, an unknown widget
        CORE::CDataNode odd( "Layout" );
        CORE::CDataNode* entry = odd.AddChild( "Splitter" );
        entry->SetAttribute( "name", "Split" );
        entry->SetAttribute( "ratio", "0.2" );
        entry = odd.AddChild( "Splitter" );
        entry->SetAttribute( "ratio", "0.9" );
        entry = odd.AddChild( "Splitter" );
        entry->SetAttribute( "name", "Broken" );
        entry->SetAttribute( "ratio", "wide" );
        entry = odd.AddChild( "Splitter" );
        entry->SetAttribute( "name", "TooBig" );
        entry->SetAttribute( "ratio", "7" );
        entry = odd.AddChild( "Splitter" );
        entry->SetAttribute( "name", "Gone" );
        entry->SetAttribute( "ratio", "0.4" );
        entry = odd.AddChild( "FromTheFuture" );
        entry->SetAttribute( "name", "Split" );
        entry = odd.AddChild( "Panel" );
        entry->SetAttribute( "name", "SidePanel" );
        GUI::CLayoutState partial;
        ASSERT_TRUE( partial.Load( odd ) );
        ASSERT_TRUE( 2 == partial.GetSplitRatios().size() && 0 == partial.GetFolded().size() );
        // the panel has no entry, so it stays as it is; the unknown widget "Gone" is ignored
        ASSERT_TRUE( 1 == partial.Apply( *h.screen ) );
        ASSERT_TRUE( Near( 0.2f, h.splitter->GetSplitRatio() ) && panel->IsFolded() );

        GUI::CLayoutState empty;
        ASSERT_FALSE( empty.Load( CORE::CDataNode( "Nothing" ) ) );
        ASSERT_TRUE( 0 == empty.Apply( *h.screen ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LayoutManager: presets apply without a change event, a change by hand clears the active preset, the layout is saved and restored" )
    try
    {
        SSplitFixture h( true );
        GUI::CFoldablePanel* panel = static_cast< GUI::CFoldablePanel* >( h.context->CreateWidget( "FoldablePanel", "SidePanel", h.screen ) );
        panel->SetProperty( "Size", "0,100 0,100" );
        h.context->FlushChanges();

        GUI::CLayoutManager manager;
        CCounter changed( &manager, GUI::CLayoutManager::LayoutChangedEvent );
        CCounter applied( &manager, GUI::CLayoutManager::PresetAppliedEvent );
        ASSERT_TRUE( manager.Attach( h.screen ) && 2 == manager.GetWatchedCount() );

        GUI::CLayoutState browse, preview;
        browse.SetSplitRatio( "Split", 0.5f );
        browse.SetFolded( "SidePanel", false );
        preview.SetSplitRatio( "Split", 0.9f );
        preview.SetFolded( "SidePanel", true );
        manager.AddPreset( "Browse", browse );
        manager.AddPreset( "Preview", preview );
        ASSERT_TRUE( manager.HasPreset( "Preview" ) && 2 == manager.GetPresetNames().size() && !manager.ApplyPreset( "Nothing" ) );

        ASSERT_TRUE( manager.ApplyPreset( "Preview" ) );
        ASSERT_TRUE( Near( 0.9f, h.splitter->GetSplitRatio() ) && panel->IsFolded() );
        ASSERT_TRUE( 0 == changed.count && 1 == applied.count && "Preview" == manager.GetActivePreset() );

        // by hand
        h.splitter->SetSplitRatio( 0.6f );
        ASSERT_TRUE( 1 == changed.count && manager.GetActivePreset().IsNULLOrEmpty() );
        ASSERT_TRUE( manager.ApplyPreset( "Browse" ) );
        ASSERT_TRUE( 1 == changed.count && "Browse" == manager.GetActivePreset() && !panel->IsFolded() );

        // the user shapes it and names it; presets of the application are not saved, those of the user are
        h.splitter->SetSplitRatio( 0.35f );
        panel->SetFolded( true );
        ASSERT_TRUE( manager.AddPresetFromCurrent( "Mine" ) );
        CORE::CDataNode config( "Config" );
        CORE::CDataNode* section = config.Structure( "App", '/' );
        ASSERT_TRUE( manager.Save( *section ) );

        SSplitFixture other( true );
        GUI::CFoldablePanel* otherPanel = static_cast< GUI::CFoldablePanel* >( other.context->CreateWidget( "FoldablePanel", "SidePanel", other.screen ) );
        otherPanel->SetProperty( "Size", "0,100 0,100" );
        GUI::CLayoutManager restored;
        // loaded before the window is attached, as an application reads its settings first
        ASSERT_TRUE( restored.Load( *section ) );
        ASSERT_TRUE( Near( 0.5f, other.splitter->GetSplitRatio() ) );
        ASSERT_TRUE( restored.Attach( other.screen ) );
        ASSERT_TRUE( Near( 0.35f, other.splitter->GetSplitRatio() ) && otherPanel->IsFolded() );
        ASSERT_TRUE( restored.HasPreset( "Mine" ) && !restored.HasPreset( "Browse" ) );
        ASSERT_TRUE( restored.ApplyPreset( "Mine" ) && Near( 0.35f, other.splitter->GetSplitRatio() ) );

        GUI::CLayoutManager none;
        ASSERT_FALSE( none.Load( CORE::CDataNode( "Empty" ) ) );
        ASSERT_FALSE( none.Attach( GUCEF_NULL ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Splitter: the handle is drawn between the panes and lighter under the pointer, a folded strip shows its header colour" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        screen->SetProperty( "Size", "1,0 1,0" );
        screen->SetProperty( "BackgroundColour", "0,0,0,255" );
        context->ShowRoot( screen, false );
        GUI::CSplitter* splitter = static_cast< GUI::CSplitter* >( context->CreateWidget( "Splitter", "Split", screen ) );
        splitter->SetProperty( "Size", "1,0 1,0" );
        splitter->SetProperty( "HandleSize", "10" );
        splitter->SetProperty( "HandleColour", "200,0,0,255" );
        splitter->SetProperty( "HoverColour", "0,200,0,255" );
        GUI::CWidget* first = context->CreateWidget( "Panel", "First", splitter );
        GUI::CWidget* second = context->CreateWidget( "Panel", "Second", splitter );
        first->SetProperty( "Size", "1,0 1,0" );
        first->SetProperty( "BackgroundColour", "0,0,200,255" );
        second->SetProperty( "Size", "1,0 1,0" );
        second->SetProperty( "BackgroundColour", "0,0,100,255" );
        context->FlushChanges();
        // 640 less 10 is 630: the handle is at x 315 to 324
        ASSERT_TRUE( GUI::CRect( 315, 0, 10, 480 ) == splitter->GetHandleRect() );

        context->RenderFrame( 16 );
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( context->CaptureFrame( pixels ) && !pixels.IsNULL() && 640 == pixels->GetWidthInPixels() );
        // the corner of the handle, away from the grip dots at its middle
        const GUI::CColour handle = PixelAt( pixels, 317, 20 );
        ASSERT_TRUE( handle.r > 190 && handle.g < 10 && handle.b < 10 );
        const GUI::CColour left = PixelAt( pixels, 100, 100 );
        const GUI::CColour right = PixelAt( pixels, 500, 100 );
        ASSERT_TRUE( 200 == left.b && 100 == right.b );

        context->InjectMouseMove( 320, 100 );
        context->RenderFrame( 16 );
        ASSERT_TRUE( context->CaptureFrame( pixels ) && !pixels.IsNULL() );
        const GUI::CColour hot = PixelAt( pixels, 317, 20 );
        ASSERT_TRUE( hot.g > 190 && hot.r < 10 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
