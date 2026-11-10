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

#ifndef GUCEF_INPUT_H
#include "gucefINPUT.h"
#define GUCEF_INPUT_H
#endif /* GUCEF_INPUT_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

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

/** Counts one routed event of one widget, in the phase the widget is the target of; for a value change it keeps the last new value */
class CCounter : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CCounter > TEventCallback;

    UInt32 count;
    CORE::CString lastValue;

    CCounter( CORE::CNotifier* notifier, const CORE::CEvent& id )
        : CORE::CObserver(), count( 0 ), lastValue()
    {
        TEventCallback callback( this, &CCounter::OnEvent );
        SubscribeTo( notifier, id, callback );
    }

    virtual ~CCounter() { UnsubscribeAllFromObserver(); }

    void OnEvent( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData )
    {
        // every event this counts is a routed one
        const GUI::CRoutedEventData* routed = static_cast< const GUI::CRoutedEventData* >( eventData );
        if ( GUCEF_NULL != routed && routed->source != notifier )
            return;
        ++count;
        if ( GUI::CWidget::ValueChangedEvent == id && GUCEF_NULL != eventData )
            lastValue = static_cast< const GUI::CValueChangedEventData* >( eventData )->newValue;
    }
};

/**
 *  A scroll panel of 200 x 100 at (10, 20) in a root of the whole picture, with the scroll bar 16 wide on its right,
 *  holding a label row of 30, a button of 50 at 200 and a group box below: 250 high and more
 */
GUI::CScrollPanel*
MakePanel( GUI::CGUIContext* context )
{
    GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
    root->SetProperty( "Size", "1,0 1,0" );
    context->ShowRoot( root, false );

    GUI::CScrollPanel* panel = static_cast< GUI::CScrollPanel* >( context->CreateWidget( "ScrollPanel", "Scroll", root ) );
    panel->SetProperty( "Position", "0,10 0,20" );
    panel->SetProperty( "Size", "0,200 0,100" );
    panel->SetProperty( "BackgroundColour", "20,20,20,255" );

    GUI::CWidget* top = context->CreateWidget( "Button", "Top", panel );
    top->SetProperty( "Position", "0,0 0,0" );
    top->SetProperty( "Size", "1,0 0,30" );
    top->SetProperty( "BackgroundColour", "200,0,0,255" );
    GUI::CWidget* middle = context->CreateWidget( "Button", "Middle", panel );
    middle->SetProperty( "Position", "0,0 0,200" );
    middle->SetProperty( "Size", "1,0 0,50" );
    middle->SetProperty( "BackgroundColour", "0,200,0,255" );
    context->FlushChanges();
    return panel;
}

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

bool
PixelIs( const IMAGE::TPixelMapPtr& frame, Int32 x, Int32 y, UInt8 r, UInt8 g, UInt8 b )
{
    if ( frame.IsNULL() || x < 0 || y < 0 || x >= (Int32) frame->GetWidthInPixels() || y >= (Int32) frame->GetHeightInPixels() )
        return false;
    const UInt8* pixel = static_cast< const UInt8* >( frame->GetDataPtr() ) + ( (size_t) y * frame->GetWidthInPixels() + x ) * 4;
    return pixel[ 0 ] == r && pixel[ 1 ] == g && pixel[ 2 ] == b;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiScrollPanelTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.ScrollPanel" );

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: The content is as high as the lowest child, relative sizes refer to the area left of the scroll bar" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CScrollPanel* panel = MakePanel( context );
        GUI::CWidget* top = panel->GetChildWidget( "Top" );
        GUI::CWidget* middle = panel->GetChildWidget( "Middle" );

        ASSERT_TRUE( "ScrollPanel" == panel->GetType() && "Panel" == GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().GetTypeInfo( "ScrollPanel" )->baseTypeName );
        ASSERT_TRUE( panel->GetBoolProperty( "Clip" ) && 0 == panel->GetScrollPosition() );
        ASSERT_TRUE( 250 == panel->GetContentHeight() && 100 == panel->GetViewportHeight() && 150 == panel->GetMaxScrollPosition() && panel->IsScrollable() );
        ASSERT_TRUE( top->GetRect() == GUI::CRect( 10, 20, 184, 30 ) );
        ASSERT_TRUE( middle->GetRect() == GUI::CRect( 10, 220, 184, 50 ) );
        ASSERT_TRUE( panel->GetViewportRect() == GUI::CRect( 0, 0, 184, 100 ) && panel->GetTrackRect() == GUI::CRect( 184, 0, 16, 100 ) );

        // room below the last child, and a child that is placed relative to the height of the area
        panel->SetIntProperty( "BottomPadding", 10 );
        context->FlushChanges();
        ASSERT_TRUE( 260 == panel->GetContentHeight() );
        GUI::CWidget* far = context->CreateWidget( "Button", "Far", panel );
        far->SetProperty( "Position", "0,0 2,0" );
        far->SetProperty( "Size", "0.5,0 0,20" );
        context->FlushChanges();
        ASSERT_TRUE( far->GetRect() == GUI::CRect( 10, 220, 92, 20 ) );
        ASSERT_TRUE( 260 == panel->GetContentHeight() );
        far->SetProperty( "Position", "0,0 3,0" );
        context->FlushChanges();
        ASSERT_TRUE( far->GetRect().top == 320 && 300 + 20 + 10 == panel->GetContentHeight() );

        // a hidden child does not count, a child that is removed neither
        far->SetVisibility( false );
        context->FlushChanges();
        ASSERT_TRUE( 260 == panel->GetContentHeight() );
        context->DestroyWidget( far );
        context->FlushChanges();
        ASSERT_TRUE( 260 == panel->GetContentHeight() );

        // everything fits: nothing to scroll, and no scroll bar thumb
        panel->SetProperty( "Size", "0,200 0,400" );
        context->FlushChanges();
        ASSERT_FALSE( panel->IsScrollable() );
        ASSERT_TRUE( 0 == panel->GetMaxScrollPosition() && panel->GetThumbRect().IsEmpty() );
        ASSERT_TRUE( 184 == top->GetRect().width );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: Scrolling moves the children, clamps at both ends and when the panel or the content change size" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CScrollPanel* panel = MakePanel( context );
        GUI::CWidget* top = panel->GetChildWidget( "Top" );
        GUI::CWidget* middle = panel->GetChildWidget( "Middle" );

        panel->SetScrollPosition( 60 );
        context->FlushChanges();
        ASSERT_TRUE( 60 == panel->GetScrollPosition() && 60 == panel->GetIntProperty( "ScrollPosition" ) );
        ASSERT_TRUE( top->GetRect().top == 20 - 60 && middle->GetRect().top == 220 - 60 );
        ASSERT_TRUE( top->GetRect().left == 10 && top->GetRect().width == 184 );
        ASSERT_TRUE( 250 == panel->GetContentHeight() );
        // the panel does not move
        ASSERT_TRUE( panel->GetRect() == GUI::CRect( 10, 20, 200, 100 ) );

        panel->SetScrollPosition( 1000 );
        context->FlushChanges();
        ASSERT_TRUE( 150 == panel->GetScrollPosition() && middle->GetRect().Bottom() == panel->GetRect().Bottom() );
        panel->SetIntProperty( "ScrollPosition", -40 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() && top->GetRect().top == 20 );

        // a taller panel: less to scroll, the position follows
        panel->SetScrollPosition( 150 );
        context->FlushChanges();
        panel->SetProperty( "Size", "0,200 0,160" );
        context->FlushChanges();
        ASSERT_TRUE( 90 == panel->GetMaxScrollPosition() && 90 == panel->GetScrollPosition() );
        ASSERT_TRUE( middle->GetRect().Bottom() == panel->GetRect().Bottom() );
        panel->SetProperty( "Size", "0,200 0,400" );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
        ASSERT_TRUE( top->GetRect().top == 20 );

        // a smaller content (a child that shrinks) does the same
        panel->SetProperty( "Size", "0,200 0,100" );
        panel->SetScrollPosition( 150 );
        context->FlushChanges();
        ASSERT_TRUE( 150 == panel->GetScrollPosition() );
        middle->SetProperty( "Position", "0,0 0,100" );
        context->FlushChanges();
        ASSERT_TRUE( 50 == panel->GetScrollPosition() && 50 == panel->GetMaxScrollPosition() );
        ASSERT_TRUE( middle->GetRect().top == 20 + 100 - 50 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: Children are cut off at the panel in the drawing and in the hit test, both follow the scroll position" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CScrollPanel* panel = MakePanel( context );
        GUI::CWidget* top = panel->GetChildWidget( "Top" );
        GUI::CWidget* middle = panel->GetChildWidget( "Middle" );
        CCounter topClicks( top, GUI::CWidget::ActivatedEvent );
        CCounter middleClicks( middle, GUI::CWidget::ActivatedEvent );

        // the middle button is below the panel: not drawn, not hit
        IMAGE::TPixelMapPtr frame;
        ASSERT_TRUE( context->CaptureFrame( frame ) );
        ASSERT_TRUE( PixelIs( frame, 50, 30, 200, 0, 0 ) );
        ASSERT_FALSE( PixelIs( frame, 50, 230, 0, 200, 0 ) );
        ASSERT_TRUE( context->HitTest( 50, 30 ) == top );
        ASSERT_TRUE( context->HitTest( 50, 230 ) != middle );

        // scrolled by 150: the middle button is in view, the top button is above the panel
        panel->SetScrollPosition( 150 );
        context->FlushChanges();
        ASSERT_TRUE( context->CaptureFrame( frame ) );
        ASSERT_TRUE( PixelIs( frame, 50, 100, 0, 200, 0 ) );
        ASSERT_FALSE( PixelIs( frame, 50, 30, 200, 0, 0 ) );
        // above the panel nothing of the top button shows
        ASSERT_FALSE( PixelIs( frame, 50, 5, 200, 0, 0 ) );
        ASSERT_TRUE( context->HitTest( 50, 100 ) == middle );
        ASSERT_TRUE( context->HitTest( 50, 10 ) != top );
        ASSERT_TRUE( context->HitTest( 50, 40 ) != top );

        // the click goes to what is there now
        context->InjectMouseMove( 50, 100 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( 1 == middleClicks.count && 0 == topClicks.count );
        context->InjectMouseMove( 50, 30 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( 1 == middleClicks.count && 0 == topClicks.count );

        // the scroll bar: a track and a thumb are drawn when there is something to scroll
        GUI::CRect track = panel->GetTrackRect();
        GUI::CRect thumb = panel->GetThumbRect();
        ASSERT_TRUE( thumb.width == track.width && thumb.top >= track.top && thumb.Bottom() <= track.Bottom() );
        ASSERT_TRUE( context->CaptureFrame( frame ) );
        ASSERT_TRUE( PixelIs( frame, 10 + track.left + 6, 20 + 1, 18, 20, 26 ) );
        ASSERT_TRUE( PixelIs( frame, 10 + thumb.left + 6, 20 + thumb.top + thumb.height / 2, 96, 106, 134 ) );
        panel->SetProperty( "Size", "0,200 0,400" );
        context->FlushChanges();
        ASSERT_TRUE( context->CaptureFrame( frame ) );
        ASSERT_FALSE( PixelIs( frame, 10 + track.left + 6, 20 + 1, 18, 20, 26 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: The wheel scrolls it over the panel and over its children, not over a child that has use for the wheel" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CScrollPanel* panel = MakePanel( context );
        CCounter changed( panel, GUI::CWidget::ValueChangedEvent );

        // over a button: it bubbles up to the panel
        context->InjectMouseMove( 50, 30 );
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( 48 == panel->GetScrollPosition() && 1 == changed.count && "48" == changed.lastValue );
        context->InjectMouseWheel( -2 );
        context->FlushChanges();
        ASSERT_TRUE( 144 == panel->GetScrollPosition() );
        context->InjectMouseWheel( -5 );
        context->FlushChanges();
        ASSERT_TRUE( 150 == panel->GetScrollPosition() );
        context->InjectMouseWheel( 1 );
        context->FlushChanges();
        ASSERT_TRUE( 102 == panel->GetScrollPosition() );
        context->InjectMouseWheel( 10 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );

        // over the scroll bar too
        context->InjectMouseMove( 10 + 194, 60 );
        context->InjectMouseWheel( -1 );
        ASSERT_TRUE( 48 == panel->GetScrollPosition() );

        // a step of its own
        panel->SetIntProperty( "ScrollStep", 10 );
        context->InjectMouseWheel( -1 );
        ASSERT_TRUE( 58 == panel->GetScrollPosition() );

        // a list box takes the wheel for itself
        GUI::CListbox* list = static_cast< GUI::CListbox* >( context->CreateWidget( "Listbox", "List", panel ) );
        list->SetProperty( "Position", "0,0 0,0" );
        list->SetProperty( "Size", "1,0 0,60" );
        list->SetProperty( "Items", "a\nb\nc\nd\ne\nf\ng\nh" );
        panel->SetScrollPosition( 0 );
        context->FlushChanges();
        context->InjectMouseMove( 50, 40 );
        context->InjectMouseWheel( -1 );
        ASSERT_TRUE( 0 == panel->GetScrollPosition() && 1 == list->GetIntProperty( "ScrollPosition" ) );

        // content that fits does not use the wheel, the widgets below it may
        panel->SetProperty( "Size", "0,200 0,500" );
        context->FlushChanges();
        context->InjectMouseMove( 50, 450 );
        context->InjectMouseWheel( -1 );
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: The wheel over a disabled widget, or one in a disabled box, scrolls the panel around it" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CScrollPanel* panel = MakePanel( context );

        // a disabled button takes no input of its own, but it does not stop the page from scrolling
        GUI::CWidget* top = panel->GetChildren()[ 0 ];
        top->SetEnabled( false );
        context->FlushChanges();
        context->InjectMouseMove( 50, 30 );
        ASSERT_FALSE( top->IsEffectivelyEnabled() );
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( 48 == panel->GetScrollPosition() );

        // in a disabled box: the box and everything in it
        GUI::CWidget* box = context->CreateWidget( "Panel", "Box", panel );
        box->SetProperty( "Position", "0,0 0,0" );
        box->SetProperty( "Size", "1,0 0,30" );
        GUI::CWidget* inBox = context->CreateWidget( "Button", "InBox", box );
        inBox->SetProperty( "Size", "1,0 1,0" );
        box->SetEnabled( false );
        panel->SetScrollPosition( 0 );
        context->FlushChanges();
        context->InjectMouseMove( 50, 30 );
        ASSERT_TRUE( context->HitTest( 50, 30 ) == inBox && !inBox->IsEffectivelyEnabled() );
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( 48 == panel->GetScrollPosition() );

        // the disabled widget itself still gets no click
        CCounter clicks( inBox, GUI::CWidget::MouseClickEvent );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( 0 == clicks.count );

        // a scroll panel that is disabled does not scroll, and neither does anything above it in a disabled window
        panel->SetScrollPosition( 0 );
        panel->SetEnabled( false );
        context->FlushChanges();
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: A widget that cannot scroll any further leaves the wheel to the scroll panel around it" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CScrollPanel* panel = MakePanel( context );

        // a list box with all its rows in view has nothing to scroll
        GUI::CListbox* list = static_cast< GUI::CListbox* >( context->CreateWidget( "Listbox", "List", panel ) );
        list->SetProperty( "Position", "0,0 0,0" );
        list->SetProperty( "Size", "1,0 0,90" );
        list->SetProperty( "Items", "a\nb" );
        context->FlushChanges();
        context->InjectMouseMove( 50, 40 );
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == list->GetIntProperty( "ScrollPosition" ) && 48 == panel->GetScrollPosition() );

        // one with more rows takes the wheel until its last row, then the panel gets it
        panel->SetScrollPosition( 0 );
        list->SetProperty( "Items", "a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\nl" );
        context->FlushChanges();
        const Int32 listMax = (Int32) list->GetItems().size() - list->GetVisibleRowCount();
        ASSERT_TRUE( listMax > 0 );
        for ( Int32 i=0; i<listMax; ++i )
            context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( listMax == list->GetIntProperty( "ScrollPosition" ) && 0 == panel->GetScrollPosition() );
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( listMax == list->GetIntProperty( "ScrollPosition" ) && 48 == panel->GetScrollPosition() );
        list->SetVisibility( false );

        // a scroll panel in a scroll panel: the inner one first, then the outer one
        GUI::CScrollPanel* inner = static_cast< GUI::CScrollPanel* >( context->CreateWidget( "ScrollPanel", "Inner", panel ) );
        inner->SetProperty( "Position", "0,0 0,0" );
        inner->SetProperty( "Size", "1,0 0,60" );
        GUI::CWidget* tall = context->CreateWidget( "Button", "Tall", inner );
        tall->SetProperty( "Position", "0,0 0,0" );
        tall->SetProperty( "Size", "1,0 0,100" );
        panel->SetScrollPosition( 0 );
        context->FlushChanges();
        ASSERT_TRUE( inner->IsScrollable() && 40 == inner->GetMaxScrollPosition() );
        context->InjectMouseMove( 50, 30 );
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( 40 == inner->GetScrollPosition() && 0 == panel->GetScrollPosition() );
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( 40 == inner->GetScrollPosition() && 48 == panel->GetScrollPosition() );

        // and back the other way: the inner panel moved up with the content of the outer one
        context->InjectMouseMove( 50, 25 );
        context->InjectMouseWheel( 1 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == inner->GetScrollPosition() && 48 == panel->GetScrollPosition() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: The thumb drags, a click on the track moves a page, the pointer stays captured" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CScrollPanel* panel = MakePanel( context );
        const Int32 panelLeft = 10, panelTop = 20;

        GUI::CRect thumb = panel->GetThumbRect();
        GUI::CRect track = panel->GetTrackRect();
        // 100 of 250 visible: the thumb is 40 of the 100 high track
        ASSERT_TRUE( 40 == thumb.height && 0 == thumb.top && 100 == track.height );

        // a click on the track below the thumb: a page down (90 of the 100 high viewport)
        context->InjectMouseMove( panelLeft + track.left + 6, panelTop + 90 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        context->FlushChanges();
        ASSERT_TRUE( 90 == panel->GetScrollPosition() );
        // and a page up above the thumb
        thumb = panel->GetThumbRect();
        ASSERT_TRUE( thumb.top > 0 );
        context->InjectMouseMove( panelLeft + track.left + 6, panelTop + 1 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );

        // dragging the thumb by half of the travel (30 of 60) scrolls half of the range
        thumb = panel->GetThumbRect();
        Press( context, panelLeft + thumb.left + 6, panelTop + thumb.top + 10 );
        ASSERT_TRUE( panel->IsThumbDragged() );
        context->InjectMouseMove( panelLeft + thumb.left + 6, panelTop + thumb.top + 10 + 30 );
        context->FlushChanges();
        ASSERT_TRUE( 75 == panel->GetScrollPosition() );
        ASSERT_TRUE( 30 == panel->GetThumbRect().top );
        // it is captured: the pointer far outside the panel, far below and to the left, goes to the end and no further
        context->InjectMouseMove( 5, 4000 );
        context->FlushChanges();
        ASSERT_TRUE( 150 == panel->GetScrollPosition() && 60 == panel->GetThumbRect().top );
        context->InjectMouseMove( 5, -4000 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
        Release( context );
        ASSERT_FALSE( panel->IsThumbDragged() );
        context->InjectMouseMove( panelLeft + 100, panelTop + 80 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );

        // a cancelled press ends the drag
        thumb = panel->GetThumbRect();
        Press( context, panelLeft + thumb.left + 6, panelTop + thumb.top + 10 );
        ASSERT_TRUE( panel->IsThumbDragged() );
        context->CancelPointerCapture();
        ASSERT_FALSE( panel->IsThumbDragged() );
        context->InjectMouseMove( panelLeft + thumb.left + 6, panelTop + thumb.top + 40 );
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
        Release( context );

        // the hover colour of the thumb
        context->InjectMouseMove( panelLeft + thumb.left + 6, panelTop + thumb.top + 5 );
        ASSERT_TRUE( panel->GetBoolProperty( "ThumbHovered" ) );
        context->InjectMouseMove( panelLeft + 50, panelTop + 50 );
        ASSERT_FALSE( panel->GetBoolProperty( "ThumbHovered" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: A child that gets the keyboard focus (Tab, a click, the application) is scrolled into view" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( root, false );
        GUI::CScrollPanel* panel = static_cast< GUI::CScrollPanel* >( context->CreateWidget( "ScrollPanel", "Scroll", root ) );
        panel->SetProperty( "Position", "0,10 0,20" );
        panel->SetProperty( "Size", "0,200 0,100" );
        GUI::CWidget* fields[ 4 ];
        for ( Int32 i=0; i<4; ++i )
        {
            fields[ i ] = context->CreateWidget( "Editbox", "Field" + CORE::Int32ToString( i ), panel );
            fields[ i ]->SetProperty( "Position", "0,4 0," + CORE::Int32ToString( 4 + i * 60 ) );
            fields[ i ]->SetProperty( "Size", "1,-8 0,30" );
        }
        context->FlushChanges();
        ASSERT_TRUE( panel->GetContentHeight() == 4 + 3 * 60 + 30 );

        // the first is in view, the application focuses the last
        ASSERT_TRUE( context->SetFocus( fields[ 0 ] ) );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
        ASSERT_TRUE( context->SetFocus( fields[ 3 ] ) );
        context->FlushChanges();
        const GUI::CRect& last = fields[ 3 ]->GetRect();
        ASSERT_TRUE( last.Bottom() <= panel->GetRect().Bottom() && last.top >= panel->GetRect().top );
        ASSERT_TRUE( panel->GetScrollPosition() == panel->GetMaxScrollPosition() );

        // Tab walks forwards and back, always in view
        ASSERT_TRUE( context->SetFocus( fields[ 0 ] ) );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
        for ( Int32 i=1; i<4; ++i )
        {
            context->InjectKey( INPUT::KEYCODE_TAB, true, 0 );
            context->InjectKey( INPUT::KEYCODE_TAB, false, 0 );
            context->FlushChanges();
            ASSERT_TRUE( context->GetFocus() == fields[ i ] );
            const GUI::CRect& rect = fields[ i ]->GetRect();
            ASSERT_TRUE( rect.top >= panel->GetRect().top && rect.Bottom() <= panel->GetRect().Bottom() );
        }
        for ( Int32 i=2; i>=0; --i )
        {
            context->InjectKey( INPUT::KEYCODE_TAB, true, INPUT::KEYMOD_SHIFT );
            context->InjectKey( INPUT::KEYCODE_TAB, false, INPUT::KEYMOD_SHIFT );
            context->FlushChanges();
            ASSERT_TRUE( context->GetFocus() == fields[ i ] );
            const GUI::CRect& rect = fields[ i ]->GetRect();
            ASSERT_TRUE( rect.top >= panel->GetRect().top && rect.Bottom() <= panel->GetRect().Bottom() );
        }
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );

        // everything in view: nothing scrolls
        panel->SetProperty( "Size", "0,200 0,400" );
        context->FlushChanges();
        ASSERT_TRUE( context->SetFocus( fields[ 3 ] ) );
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: Group boxes with a height of their own content stack inside, the widgets in them follow the scrolling" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( root, false );
        GUI::CScrollPanel* panel = static_cast< GUI::CScrollPanel* >( context->CreateWidget( "ScrollPanel", "Scroll", root ) );
        panel->SetProperty( "Position", "0,0 0,0" );
        panel->SetProperty( "Size", "0,300 0,150" );
        GUI::CWidget* groups[ 3 ];
        GUI::CWidget* fields[ 3 ];
        for ( Int32 i=0; i<3; ++i )
        {
            groups[ i ] = context->CreateWidget( "GroupBox", "Group" + CORE::Int32ToString( i ), panel );
            groups[ i ]->SetProperty( "Position", "0,4 0," + CORE::Int32ToString( 4 + i * 100 ) );
            groups[ i ]->SetProperty( "Size", "1,-8 0,0" );
            fields[ i ] = context->CreateWidget( "Editbox", "Field" + CORE::Int32ToString( i ), groups[ i ] );
            fields[ i ]->SetProperty( "Size", "1,0 0,40" );
        }
        context->FlushChanges();
        // a group box is as high as its child and its padding: 40 + 2 * 10
        ASSERT_TRUE( 60 == groups[ 0 ]->GetRect().height );
        ASSERT_TRUE( 4 + 200 + 60 == panel->GetContentHeight() );
        panel->SetScrollPosition( 100 );
        context->FlushChanges();
        ASSERT_TRUE( groups[ 1 ]->GetRect().top == 104 - 100 && fields[ 1 ]->GetRect().top == 104 - 100 + 10 );
        ASSERT_TRUE( context->HitTest( fields[ 1 ]->GetRect().left + 5, fields[ 1 ]->GetRect().top + 5 ) == fields[ 1 ] );
        // below the panel: cut off
        ASSERT_TRUE( context->HitTest( 20, 160 ) != fields[ 2 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: A box that is as high as its content counts the boxes in it, the content of a scroll panel follows" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( root, false );
        GUI::CScrollPanel* panel = static_cast< GUI::CScrollPanel* >( context->CreateWidget( "ScrollPanel", "Scroll", root ) );
        panel->SetProperty( "Size", "0,300 0,100" );
        GUI::CWidget* list = context->CreateWidget( "VBox", "List", panel );
        list->SetProperty( "Size", "1,0 0,0" );
        list->SetBoolProperty( "AutoHeight", true );
        list->SetIntProperty( "Padding", 4 );
        list->SetIntProperty( "Spacing", 6 );
        for ( Int32 i=0; i<3; ++i )
        {
            GUI::CWidget* group = context->CreateWidget( "GroupBox", "Group" + CORE::Int32ToString( i ), list );
            group->SetProperty( "Size", "1,0 0,0" );
            for ( Int32 r=0; r<2; ++r )
            {
                GUI::CWidget* row = context->CreateWidget( "Panel", "Row" + CORE::Int32ToString( i * 2 + r ), group );
                row->SetProperty( "Size", "1,0 0,30" );
            }
        }
        context->FlushChanges();

        // a group: 2 rows of 30 and a spacing of 4 between them, 10 around; the list: 3 groups, 2 spacings of 6, 4 around
        GUI::CWidget* group = list->GetChildWidget( "Group0" );
        ASSERT_TRUE( 60 + 4 + 20 == group->GetRect().height );
        ASSERT_TRUE( 3 * 84 + 2 * 6 + 8 == list->GetRect().height );
        ASSERT_TRUE( list->GetRect().height == panel->GetContentHeight() && panel->IsScrollable() );
        ASSERT_TRUE( list->GetRect().width == 300 - 16 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: The columns of a grid fit its width when asked to, a cell is cut off at its column" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        context->RegisterFont( "Block", MakeTestFont( 8, 10 ) );
        GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( root, false );
        GUI::CGridView* grid = static_cast< GUI::CGridView* >( context->CreateWidget( "GridView", "Grid", root ) );
        grid->SetProperty( "Font", "Block" );
        grid->SetProperty( "Size", "0,300 0,100" );
        grid->SetProperty( "Columns", "A\nB\nC" );
        GUCEF::vector< Int32 > explicitWidths;
        explicitWidths.push_back( 100 ); explicitWidths.push_back( 100 ); explicitWidths.push_back( 100 );
        grid->SetIntListProperty( "ColumnWidths", explicitWidths );
        grid->SetProperty( "Rows", "wwwwwwwwwwwwwwwwwwww\t1\t2" );
        context->FlushChanges();
        GUCEF::vector< Int32 > widths = grid->GetColumnWidths();
        ASSERT_TRUE( 3 == widths.size() && 100 == widths[ 0 ] && 100 == widths[ 2 ] );

        // relative to each other, together as wide as the grid
        explicitWidths[ 0 ] = 30; explicitWidths[ 1 ] = 60; explicitWidths[ 2 ] = 90;
        grid->SetIntListProperty( "ColumnWidths", explicitWidths );
        grid->SetBoolProperty( "FitColumns", true );
        context->FlushChanges();
        widths = grid->GetColumnWidths();
        ASSERT_TRUE( 3 == widths.size() && 50 == widths[ 0 ] && 100 == widths[ 1 ] && 150 == widths[ 2 ] );
        grid->SetProperty( "Size", "0,150 0,100" );
        context->FlushChanges();
        widths = grid->GetColumnWidths();
        ASSERT_TRUE( 25 == widths[ 0 ] && 50 == widths[ 1 ] && 75 == widths[ 2 ] && 150 == widths[ 0 ] + widths[ 1 ] + widths[ 2 ] );

        // the first cell is 160 wide in text and has 25 pixels: what is beyond its column is not drawn: of the second column only its own '1', a glyph of 8 x 10
        grid->SetProperty( "Size", "0,300 0,100" );
        grid->SetProperty( "Position", "0,0 0,0" );
        grid->SetProperty( "BackgroundColour", "0,0,0,255" );
        explicitWidths[ 0 ] = 50; explicitWidths[ 1 ] = 100; explicitWidths[ 2 ] = 150;
        grid->SetIntListProperty( "ColumnWidths", explicitWidths );
        context->FlushChanges();
        IMAGE::TPixelMapPtr frame;
        context->RenderFrame( 16 );
        ASSERT_TRUE( context->CaptureFrame( frame ) );
        const UInt8* data = static_cast< const UInt8* >( frame->GetDataPtr() );
        UInt32 inFirst = 0, inSecond = 0;
        const Int32 rowTop = grid->GetRowHeight();
        for ( Int32 y=rowTop; y<rowTop+10; ++y )
        {
            for ( Int32 x=0; x<50; ++x )
                if ( data[ ( (size_t) y * 640 + x ) * 4 ] > 100 )
                    ++inFirst;
            for ( Int32 x=50; x<100; ++x )
                if ( data[ ( (size_t) y * 640 + x ) * 4 ] > 100 )
                    ++inSecond;
        }
        ASSERT_TRUE( 48 * 10 == inFirst && 8 * 10 == inSecond );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: The scroll bar is 16 wide, its thumb is at least 32 long, track and thumb are drawn in their own colours for rest, hover and press" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CScrollPanel* panel = MakePanel( context );
        const Int32 panelLeft = 10, panelTop = 20;

        ASSERT_TRUE( 16 == panel->GetIntProperty( "ScrollBarWidth" ) );
        GUI::CRect track = panel->GetTrackRect();
        GUI::CRect thumb = panel->GetThumbRect();
        ASSERT_TRUE( track == GUI::CRect( 184, 0, 16, 100 ) && thumb.width == 16 && thumb.height >= 32 );

        // rest, hovered and pressed, each with its own colour, drawn inside the track with a margin around the thumb
        IMAGE::TPixelMapPtr frame;
        const Int32 thumbX = panelLeft + thumb.left + 8;
        const Int32 thumbY = panelTop + thumb.top + thumb.height / 2;
        ASSERT_TRUE( context->CaptureFrame( frame ) );
        ASSERT_TRUE( PixelIs( frame, thumbX, thumbY, 96, 106, 134 ) );
        ASSERT_TRUE( PixelIs( frame, panelLeft + thumb.left + 8, panelTop + thumb.top + 1, 18, 20, 26 ) );
        ASSERT_TRUE( PixelIs( frame, panelLeft + track.left + 8, panelTop + track.Bottom() - 1, 18, 20, 26 ) );
        context->InjectMouseMove( thumbX, thumbY );
        ASSERT_TRUE( panel->GetBoolProperty( "ThumbHovered" ) );
        ASSERT_TRUE( context->CaptureFrame( frame ) );
        ASSERT_TRUE( PixelIs( frame, thumbX, thumbY, 138, 152, 188 ) );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        ASSERT_TRUE( panel->IsThumbDragged() );
        ASSERT_TRUE( context->CaptureFrame( frame ) );
        ASSERT_TRUE( PixelIs( frame, thumbX, thumbY, 176, 192, 232 ) );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        context->InjectMouseMove( panelLeft + 20, panelTop + 20 );
        ASSERT_TRUE( context->CaptureFrame( frame ) );
        ASSERT_TRUE( PixelIs( frame, thumbX, thumbY, 96, 106, 134 ) );

        // a very long content: the thumb stays at 32, and it is flush with both ends of the track
        GUI::CWidget* tall = context->CreateWidget( "Panel", "Tall", panel );
        tall->SetProperty( "Position", "0,0 0,0" );
        tall->SetProperty( "Size", "1,0 0,20000" );
        context->FlushChanges();
        thumb = panel->GetThumbRect();
        ASSERT_TRUE( 32 == thumb.height && 0 == thumb.top );
        panel->SetScrollPosition( panel->GetMaxScrollPosition() );
        context->FlushChanges();
        thumb = panel->GetThumbRect();
        ASSERT_TRUE( 32 == thumb.height && thumb.Bottom() == track.Bottom() );

        // a panel that is not as high as the minimum: the thumb is the track
        panel->SetProperty( "Size", "0,200 0,20" );
        context->FlushChanges();
        thumb = panel->GetThumbRect();
        ASSERT_TRUE( thumb.height == panel->GetTrackRect().height && thumb.top == 0 );

        // a wider bar is a property: the children refer to what is left of the panel
        panel->SetProperty( "Size", "0,200 0,100" );
        panel->SetIntProperty( "ScrollBarWidth", 24 );
        context->FlushChanges();
        ASSERT_TRUE( panel->GetTrackRect() == GUI::CRect( 176, 0, 24, 100 ) && panel->GetViewportRect() == GUI::CRect( 0, 0, 176, 100 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: The whole width of the bar takes the pointer: the thumb grabs and the track pages at every column of it, the children left of it do not" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CScrollPanel* panel = MakePanel( context );
        const Int32 panelLeft = 10, panelTop = 20;
        const GUI::CRect track = panel->GetTrackRect();
        GUI::CRect thumb = panel->GetThumbRect();

        // every pixel column of the bar hits the panel itself, not a child, also next to the drawn thumb
        for ( Int32 x=track.left; x<track.Right(); ++x )
        {
            ASSERT_TRUE( context->HitTest( panelLeft + x, panelTop + 5 ) == panel );
            ASSERT_TRUE( context->HitTest( panelLeft + x, panelTop + 95 ) == panel );
        }
        ASSERT_TRUE( context->HitTest( panelLeft + track.left - 1, panelTop + 5 ) == panel->GetChildWidget( "Top" ) );

        // the thumb is grabbed at its outer pixel columns, which are not drawn as thumb
        for ( Int32 x=track.left; x<track.Right(); x+=track.width - 1 )
        {
            Press( context, panelLeft + x, panelTop + thumb.top + 5 );
            ASSERT_TRUE( panel->IsThumbDragged() );
            context->InjectMouseMove( panelLeft + x, panelTop + thumb.top + 5 + 20 );
            ASSERT_TRUE( panel->GetScrollPosition() > 0 );
            Release( context );
            ASSERT_FALSE( panel->IsThumbDragged() );
            panel->SetScrollPosition( 0 );
            context->FlushChanges();
        }

        // the track pages at the outer pixel columns too, by 90 of the 100 pixels of the viewport; a plain click on the thumb does nothing
        for ( Int32 x=track.left; x<track.Right(); x+=track.width - 1 )
        {
            context->InjectMouseMove( panelLeft + x, panelTop + 95 );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->FlushChanges();
            ASSERT_TRUE( 90 == panel->GetScrollPosition() );
            context->InjectMouseMove( panelLeft + x, panelTop + 1 );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->FlushChanges();
            ASSERT_TRUE( 0 == panel->GetScrollPosition() );
        }
        thumb = panel->GetThumbRect();
        context->InjectMouseMove( panelLeft + track.left + 8, panelTop + thumb.top + thumb.height / 2 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );

        // a right click on the bar is not a page
        context->InjectMouseMove( panelLeft + track.left + 8, panelTop + 95 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_RIGHT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_RIGHT, false );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );

        // clicks on the track again and again walk to the end and stop there, then back to the start
        GUI::CWidget* tall = context->CreateWidget( "Panel", "Tall", panel );
        tall->SetProperty( "Position", "0,0 0,0" );
        tall->SetProperty( "Size", "1,0 0,1000" );
        context->FlushChanges();
        ASSERT_TRUE( 900 == panel->GetMaxScrollPosition() );
        for ( Int32 click=1; click<=12; ++click )
        {
            context->InjectMouseMove( panelLeft + track.left + 8, panelTop + 99 );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->FlushChanges();
            ASSERT_TRUE( ( click < 10 ? click * 90 : 900 ) == panel->GetScrollPosition() );
        }
        ASSERT_TRUE( 68 == panel->GetThumbRect().top );
        for ( Int32 click=1; click<=12; ++click )
        {
            context->InjectMouseMove( panelLeft + track.left + 8, panelTop + 1 );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->FlushChanges();
            ASSERT_TRUE( ( click < 10 ? 900 - click * 90 : 0 ) == panel->GetScrollPosition() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: The thumb follows the pointer pixel by pixel over a very long content, and reaches both ends exactly" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CScrollPanel* panel = MakePanel( context );
        const Int32 panelLeft = 10, panelTop = 20;
        GUI::CWidget* tall = context->CreateWidget( "Panel", "Tall", panel );
        tall->SetProperty( "Position", "0,0 0,0" );
        tall->SetProperty( "Size", "1,0 0,1000" );
        context->FlushChanges();
        const GUI::CRect track = panel->GetTrackRect();
        const GUI::CRect thumb = panel->GetThumbRect();
        const Int32 travel = track.height - thumb.height;
        ASSERT_TRUE( 900 == panel->GetMaxScrollPosition() && 32 == thumb.height && 68 == travel );

        // grabbed 10 pixels below the top of the thumb
        const Int32 grabY = panelTop + thumb.top + 10;
        const Int32 x = panelLeft + track.left + 8;
        Press( context, x, grabY );
        ASSERT_TRUE( panel->IsThumbDragged() );
        Int32 previous = 0;
        for ( Int32 d=0; d<=travel; ++d )
        {
            context->InjectMouseMove( x, grabY + d );
            const Int32 position = panel->GetScrollPosition();
            const Int32 exact = ( d * 900 + travel / 2 ) / travel;
            ASSERT_TRUE( position >= exact - 1 && position <= exact + 1 );
            ASSERT_TRUE( position >= previous );
            // the thumb stays under the pointer: at the pixel it was grabbed at, give or take the rounding
            const Int32 under = panel->GetThumbRect().top - d;
            ASSERT_TRUE( under >= -1 && under <= 1 );
            previous = position;
        }
        ASSERT_TRUE( 900 == panel->GetScrollPosition() && panel->GetThumbRect().Bottom() == track.Bottom() );
        // and back, to exactly the start
        for ( Int32 d=travel; d>=0; --d )
            context->InjectMouseMove( x, grabY + d );
        ASSERT_TRUE( 0 == panel->GetScrollPosition() && 0 == panel->GetThumbRect().top );
        // sideways does not matter, the bar is grabbed
        context->InjectMouseMove( x - 300, grabY + 34 );
        ASSERT_TRUE( panel->GetScrollPosition() >= 440 && panel->GetScrollPosition() <= 460 );
        Release( context );
        ASSERT_FALSE( panel->IsThumbDragged() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: A column width keeps the children and the scroll bar in the middle of a wide panel, the wheel works over the margins on both sides" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CScrollPanel* panel = MakePanel( context );
        const Int32 panelLeft = 10, panelTop = 20;
        GUI::CWidget* top = panel->GetChildWidget( "Top" );
        panel->SetProperty( "Size", "0,600 0,100" );

        // no width of its own: the whole panel
        ASSERT_TRUE( 0 == panel->GetIntProperty( "MaxColumnWidth" ) );
        context->FlushChanges();
        ASSERT_TRUE( panel->GetColumnRect() == GUI::CRect( 0, 0, 600, 100 ) && panel->GetTrackRect() == GUI::CRect( 584, 0, 16, 100 ) );

        // 300 wide in the middle: 150 pixels of margin on both sides, the bar at the edge of the column
        panel->SetIntProperty( "MaxColumnWidth", 300 );
        context->FlushChanges();
        ASSERT_TRUE( panel->GetColumnRect() == GUI::CRect( 150, 0, 300, 100 ) );
        ASSERT_TRUE( panel->GetViewportRect() == GUI::CRect( 150, 0, 284, 100 ) && panel->GetTrackRect() == GUI::CRect( 434, 0, 16, 100 ) );
        ASSERT_TRUE( top->GetRect() == GUI::CRect( panelLeft + 150, panelTop, 284, 30 ) );
        ASSERT_TRUE( panel->GetThumbRect().left == 434 && panel->GetThumbRect().width == 16 );

        // drawn: the bar next to the column, the margins are the panel
        IMAGE::TPixelMapPtr frame;
        ASSERT_TRUE( context->CaptureFrame( frame ) );
        ASSERT_TRUE( PixelIs( frame, panelLeft + 442, panelTop + 98, 18, 20, 26 ) );
        ASSERT_TRUE( PixelIs( frame, panelLeft + 20, panelTop + 98, 20, 20, 20 ) && PixelIs( frame, panelLeft + 580, panelTop + 98, 20, 20, 20 ) );
        ASSERT_TRUE( PixelIs( frame, panelLeft + 200, panelTop + 10, 200, 0, 0 ) );
        ASSERT_FALSE( PixelIs( frame, panelLeft + 140, panelTop + 10, 200, 0, 0 ) );

        // the wheel over the margins and over the bar and over the children; a click or a press in the margins does nothing
        panel->SetScrollPosition( 0 );
        const Int32 marginXs[] = { 0, 20, 149, 451, 580, 599 };
        for ( UInt32 i=0; i<6; ++i )
        {
            panel->SetScrollPosition( 0 );
            context->FlushChanges();
            ASSERT_TRUE( context->HitTest( panelLeft + marginXs[ i ], panelTop + 50 ) == panel );
            context->InjectMouseMove( panelLeft + marginXs[ i ], panelTop + 50 );
            context->InjectMouseWheel( -1 );
            context->FlushChanges();
            ASSERT_TRUE( 48 == panel->GetScrollPosition() );
            context->InjectMouseWheel( 1 );
            context->FlushChanges();
            ASSERT_TRUE( 0 == panel->GetScrollPosition() );
            Press( context, panelLeft + marginXs[ i ], panelTop + 95 );
            Release( context );
            context->FlushChanges();
            ASSERT_TRUE( 0 == panel->GetScrollPosition() && !panel->IsThumbDragged() );
        }
        panel->SetScrollPosition( 0 );
        context->FlushChanges();
        context->InjectMouseMove( panelLeft + 300, panelTop + 20 );
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( 48 == panel->GetScrollPosition() );
        context->InjectMouseMove( panelLeft + 442, panelTop + 20 );
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( 96 == panel->GetScrollPosition() );

        // the bar of the column pages and drags, the same bar in the middle of the panel
        panel->SetScrollPosition( 0 );
        context->FlushChanges();
        const GUI::CRect track = panel->GetTrackRect();
        context->InjectMouseMove( panelLeft + track.left + 8, panelTop + 95 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        context->FlushChanges();
        ASSERT_TRUE( 90 == panel->GetScrollPosition() );
        panel->SetScrollPosition( 0 );
        context->FlushChanges();
        const GUI::CRect thumb = panel->GetThumbRect();
        Press( context, panelLeft + track.left + 8, panelTop + thumb.top + 5 );
        ASSERT_TRUE( panel->IsThumbDragged() );
        context->InjectMouseMove( panelLeft + track.left + 8, panelTop + thumb.top + 5 + 30 );
        ASSERT_TRUE( 75 == panel->GetScrollPosition() );
        Release( context );

        // a panel not as wide as the column is the column
        panel->SetScrollPosition( 0 );
        panel->SetProperty( "Size", "0,250 0,100" );
        context->FlushChanges();
        ASSERT_TRUE( panel->GetColumnRect() == GUI::CRect( 0, 0, 250, 100 ) && panel->GetViewportRect() == GUI::CRect( 0, 0, 234, 100 ) );
        ASSERT_TRUE( top->GetRect() == GUI::CRect( panelLeft, panelTop, 234, 30 ) );

        // an odd margin is one pixel more on the right, a width that is not positive is no limit
        panel->SetProperty( "Size", "0,301 0,100" );
        panel->SetIntProperty( "MaxColumnWidth", 200 );
        context->FlushChanges();
        ASSERT_TRUE( panel->GetColumnRect() == GUI::CRect( 50, 0, 200, 100 ) );
        panel->SetIntProperty( "MaxColumnWidth", -5 );
        context->FlushChanges();
        ASSERT_TRUE( panel->GetColumnRect() == GUI::CRect( 0, 0, 301, 100 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScrollPanel: PageUp, PageDown, Home and End scroll the panel of the focused control or else of the hovered one, not while an edit box has the focus" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CScrollPanel* panel = MakePanel( context );
        GUI::CWidget* top = panel->GetChildWidget( "Top" );
        GUI::CWidget* edit = context->CreateWidget( "Editbox", "Edit", panel );
        edit->SetProperty( "Position", "0,0 0,120" );
        edit->SetProperty( "Size", "1,0 0,30" );
        context->FlushChanges();
        ASSERT_TRUE( 250 == panel->GetContentHeight() && 150 == panel->GetMaxScrollPosition() );

        // the focus on a control in the panel
        ASSERT_TRUE( context->SetFocus( top ) );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
        context->InjectKey( INPUT::KEYCODE_PGDOWN, true, 0 );
        context->InjectKey( INPUT::KEYCODE_PGDOWN, false, 0 );
        context->FlushChanges();
        ASSERT_TRUE( 90 == panel->GetScrollPosition() );
        context->InjectKey( INPUT::KEYCODE_PGDOWN, true, 0 );
        context->FlushChanges();
        ASSERT_TRUE( 150 == panel->GetScrollPosition() );
        context->InjectKey( INPUT::KEYCODE_PGUP, true, 0 );
        context->FlushChanges();
        ASSERT_TRUE( 60 == panel->GetScrollPosition() );
        context->InjectKey( INPUT::KEYCODE_HOME, true, 0 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
        context->InjectKey( INPUT::KEYCODE_END, true, 0 );
        context->FlushChanges();
        ASSERT_TRUE( 150 == panel->GetScrollPosition() );
        // a key that is released does nothing
        context->InjectKey( INPUT::KEYCODE_HOME, false, 0 );
        ASSERT_TRUE( 150 == panel->GetScrollPosition() );

        // an edit box with the focus keeps its keys
        ASSERT_TRUE( context->SetFocus( edit ) );
        panel->SetScrollPosition( 0 );
        context->FlushChanges();
        context->InjectKey( INPUT::KEYCODE_PGDOWN, true, 0 );
        context->InjectKey( INPUT::KEYCODE_END, true, 0 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );

        // no focus: the panel the pointer is over; none under the pointer: nothing
        ASSERT_TRUE( context->SetFocus( GUCEF_NULL ) );
        ASSERT_TRUE( GUCEF_NULL == context->GetFocus() );
        context->InjectMouseMove( 600, 400 );
        context->InjectKey( INPUT::KEYCODE_PGDOWN, true, 0 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
        context->InjectMouseMove( 50, 60 );
        context->InjectKey( INPUT::KEYCODE_PGDOWN, true, 0 );
        context->FlushChanges();
        ASSERT_TRUE( 90 == panel->GetScrollPosition() );
        context->InjectKey( INPUT::KEYCODE_HOME, true, 0 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
        // the arrows scroll a step, with a pointer over it and nothing focused
        context->InjectKey( INPUT::KEYCODE_DOWN, true, 0 );
        context->FlushChanges();
        ASSERT_TRUE( 48 == panel->GetScrollPosition() && GUCEF_NULL == context->GetFocus() );
        context->InjectKey( INPUT::KEYCODE_UP, true, 0 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );

        // content that fits has nothing to scroll
        panel->SetProperty( "Size", "0,200 0,400" );
        context->FlushChanges();
        context->InjectKey( INPUT::KEYCODE_END, true, 0 );
        context->FlushChanges();
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
