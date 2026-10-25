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

const Int32 TitleHeight = 24;

/** Counts one event of one widget; an event that carries a change of value (CValueChangedEventData) also keeps the last values */
class CCounter : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CCounter > TEventCallback;

    UInt32 count;
    CORE::CString lastOldValue;
    CORE::CString lastNewValue;

    CCounter( CORE::CNotifier* notifier, const CORE::CEvent& id, bool carriesValueChange = false )
        : CORE::CObserver(), count( 0 ), lastOldValue(), lastNewValue(), m_carriesValueChange( carriesValueChange )
    {
        TEventCallback callback( this, &CCounter::OnEvent );
        SubscribeTo( notifier, id, callback );
    }

    virtual ~CCounter() { UnsubscribeAllFromObserver(); }

    void OnEvent( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData )
    {
        ++count;
        if ( m_carriesValueChange && GUCEF_NULL != eventData )
        {
            const GUI::CValueChangedEventData* changed = static_cast< const GUI::CValueChangedEventData* >( eventData );
            lastOldValue = changed->oldValue;
            lastNewValue = changed->newValue;
        }
    }

    private:

    bool m_carriesValueChange;
};

/** A window as the root of a form: 300 x 200, its title bar 24 high, with a button and an edit box inside */
GUI::CWindow*
MakeWindow( GUI::CGUIContext* context, const char* position = "0,100 0,80" )
{
    GUI::CWindow* window = static_cast< GUI::CWindow* >( context->CreateWidget( "Window", "Window", GUCEF_NULL ) );
    window->SetProperty( "Position", position );
    window->SetProperty( "Size", "0,300 0,200" );
    window->SetIntProperty( "TitleHeight", TitleHeight );
    window->SetProperty( "Title", "Test" );
    window->SetProperty( "BackgroundColour", "10,10,10,255" );
    GUI::CWidget* button = context->CreateWidget( "Button", "Ok", window );
    button->SetProperty( "Position", "0,10 0,40" );
    button->SetProperty( "Size", "0,100 0,30" );
    button->SetProperty( "BackgroundColour", "200,0,0,255" );
    GUI::CWidget* edit = context->CreateWidget( "Editbox", "Name", window );
    edit->SetProperty( "Position", "0,10 0,100" );
    edit->SetProperty( "Size", "0,200 0,24" );
    context->ShowRoot( window, false );
    context->FlushChanges();
    return window;
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

void
Click( GUI::CGUIContext* context, Int32 x, Int32 y )
{
    Press( context, x, y );
    Release( context );
}

/** Drags from a point to another with the left button */
void
Drag( GUI::CGUIContext* context, Int32 fromX, Int32 fromY, Int32 toX, Int32 toY )
{
    Press( context, fromX, fromY );
    context->InjectMouseMove( toX, toY );
    Release( context );
    context->FlushChanges();
}

/** The pixel of a captured frame */
bool
PixelIs( const IMAGE::TPixelMapPtr& frame, Int32 x, Int32 y, UInt8 r, UInt8 g, UInt8 b )
{
    if ( frame.IsNULL() || x < 0 || y < 0 || x >= (Int32) frame->GetWidthInPixels() || y >= (Int32) frame->GetHeightInPixels() )
        return false;
    const UInt8* pixel = static_cast< const UInt8* >( frame->GetDataPtr() ) + ( (size_t) y * frame->GetWidthInPixels() + x ) * 4;
    return pixel[ 0 ] == r && pixel[ 1 ] == g && pixel[ 2 ] == b;
}

/** How many pixels of a rectangle of a frame have a colour */
UInt32
CountPixels( const IMAGE::TPixelMapPtr& frame, const GUI::CRect& rect, UInt8 r, UInt8 g, UInt8 b )
{
    UInt32 count = 0;
    for ( Int32 y=rect.top; y<rect.Bottom(); ++y )
        for ( Int32 x=rect.left; x<rect.Right(); ++x )
            if ( PixelIs( frame, x, y, r, g, b ) )
                ++count;
    return count;
}

GUI::CRect
Moved( const GUI::CRect& rect, Int32 dx, Int32 dy )
{
    return GUI::CRect( rect.left + dx, rect.top + dy, rect.width, rect.height );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiWindowTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.Windows" );

    GUCEF_TESTFW_TESTCASE( "Windows: Dragging the title bar moves the window by the pointer delta, keeps the relative part of the position and notifies once per move" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWindow* window = MakeWindow( context, "0.5,-150 0,80" );
        GUI::CRect start = window->GetRect();
        ASSERT_TRUE( 170 == start.left && 80 == start.top && 300 == start.width && 200 == start.height );
        ASSERT_TRUE( window->IsMovable() && window->IsCollapsible() && !window->IsCollapsed() && TitleHeight == window->GetTitleBarHeight() );

        CCounter moved( window, GUI::CWindow::MovedEvent, true );
        Press( context, 200, 90 );
        ASSERT_TRUE( window->IsDragging() );
        context->InjectMouseMove( 240, 125 );
        ASSERT_TRUE( 1 == moved.count );
        ASSERT_TRUE( "0.5,-110 0,115" == window->GetProperty( "Position" ) );
        context->FlushChanges();
        ASSERT_TRUE( window->GetRect().left == start.left + 40 && window->GetRect().top == start.top + 35 );
        ASSERT_TRUE( window->GetRect().width == 300 && window->GetRect().height == 200 );
        ASSERT_TRUE( "0.5,-150 0,80" == moved.lastOldValue && "0.5,-110 0,115" == moved.lastNewValue );

        // the pointer is captured: it can leave the title bar and the window, the window follows
        context->InjectMouseMove( 100, 400 );
        context->FlushChanges();
        ASSERT_TRUE( 2 == moved.count );
        ASSERT_TRUE( window->GetRect().left == start.left - 100 && window->GetRect().top == start.top + 310 );
        ASSERT_TRUE( window->IsDragging() );

        // a move without a change of position is no move
        context->InjectMouseMove( 100, 400 );
        ASSERT_TRUE( 2 == moved.count );

        Release( context );
        ASSERT_FALSE( window->IsDragging() );
        ASSERT_TRUE( window->IsMovedByUser() );
        GUI::CRect dropped = window->GetRect();
        context->InjectMouseMove( 300, 300 );
        context->FlushChanges();
        ASSERT_TRUE( window->GetRect() == dropped && 2 == moved.count );

        // a press on the body or on a child does not move it
        Press( context, dropped.left + 150, dropped.top + 120 );
        context->InjectMouseMove( dropped.left + 190, dropped.top + 150 );
        Release( context );
        context->FlushChanges();
        ASSERT_TRUE( window->GetRect() == dropped && 2 == moved.count );
        GUI::CWidget* button = window->GetChildWidget( "Ok" );
        Press( context, button->GetRect().left + 5, button->GetRect().top + 5 );
        context->InjectMouseMove( 10, 10 );
        Release( context );
        context->FlushChanges();
        ASSERT_TRUE( window->GetRect() == dropped && 2 == moved.count );

        // the right button does not move it
        context->InjectMouseMove( dropped.left + 50, dropped.top + 10 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_RIGHT, true );
        ASSERT_FALSE( window->IsDragging() );
        context->InjectMouseButton( GUI::MOUSEBUTTON_RIGHT, false );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Windows: The title bar stays inside the screen, a window wider than it covers it; a moved window is kept in reach when the parent shrinks" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWindow* window = MakeWindow( context );

        // far to the left and up: against the left and top edge
        Drag( context, 150, 90, -500, -500 );
        ASSERT_TRUE( 0 == window->GetRect().left && 0 == window->GetRect().top );
        // far to the right and down: the window inside horizontally, the title bar inside vertically
        Drag( context, 150, 10, 5000, 5000 );
        ASSERT_TRUE( 640 - 300 == window->GetRect().left );
        ASSERT_TRUE( 480 - TitleHeight == window->GetRect().top );
        ASSERT_TRUE( "0,340 0,456" == window->GetProperty( "Position" ) );
        ASSERT_TRUE( context->HitTest( 400, 470 ) == window );

        // the window moved by the user follows the screen when it shrinks
        context->SetLogicalResolution( 320, 240 );
        context->FlushChanges();
        ASSERT_TRUE( window->GetRect().left == 320 - 300 && window->GetRect().top == 240 - TitleHeight );
        ASSERT_TRUE( window->GetRect().Right() <= 320 );
        context->SetLogicalResolution( 640, 480 );
        context->FlushChanges();
        ASSERT_TRUE( window->GetRect().left == 320 - 300 && window->GetRect().top == 240 - TitleHeight );

        // a window wider than the screen can cover it and no more than that
        window->SetProperty( "Size", "0,700 0,200" );
        window->SetProperty( "Position", "0,0 0,0" );
        context->FlushChanges();
        Drag( context, 100, 10, 5000, 10 );
        ASSERT_TRUE( 0 == window->GetRect().left );
        Drag( context, 100, 10, -5000, 10 );
        ASSERT_TRUE( 640 - 700 == window->GetRect().left );

        // a window nobody moved is where the layout says, also when that is outside
        GUI::CWindow* other = static_cast< GUI::CWindow* >( context->CreateWidget( "Window", "Other", GUCEF_NULL ) );
        other->SetProperty( "Position", "0,-100 0,500" );
        other->SetProperty( "Size", "0,50 0,50" );
        context->ShowRoot( other, false );
        context->FlushChanges();
        ASSERT_TRUE( -100 == other->GetRect().left && 500 == other->GetRect().top );
        ASSERT_FALSE( other->IsMovedByUser() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Windows: A drag ends on release, and when the pointer is cancelled, the window hidden or disabled, or the button is no longer down" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWindow* window = MakeWindow( context );
        GUI::CRect start = window->GetRect();

        // the window lost the focus (the pointer capture is cancelled)
        CCounter lost( window, GUI::CWidget::MouseCaptureLostEvent );
        Press( context, 150, 90 );
        context->InjectMouseMove( 170, 100 );
        ASSERT_TRUE( window->IsDragging() );
        context->CancelPointerCapture();
        ASSERT_FALSE( window->IsDragging() );
        ASSERT_TRUE( 1 == lost.count );
        context->InjectMouseMove( 400, 400 );
        context->FlushChanges();
        ASSERT_TRUE( window->GetRect() == Moved( start, 20, 10 ) );
        Release( context );
        ASSERT_TRUE( 1 == lost.count );

        // hidden during the drag
        start = window->GetRect();
        Press( context, start.left + 50, start.top + 10 );
        ASSERT_TRUE( window->IsDragging() );
        window->SetVisibility( false );
        ASSERT_FALSE( window->IsDragging() );
        window->SetVisibility( true );
        context->InjectMouseMove( 400, 400 );
        context->FlushChanges();
        ASSERT_TRUE( window->GetRect() == start );
        Release( context );

        // disabled during the drag: nothing follows it
        Press( context, start.left + 50, start.top + 10 );
        ASSERT_TRUE( window->IsDragging() );
        window->SetEnabled( false );
        context->RenderFrame( 16 );
        ASSERT_FALSE( window->IsDragging() );
        window->SetEnabled( true );
        Release( context );

        // Movable off during a drag
        Press( context, start.left + 50, start.top + 10 );
        ASSERT_TRUE( window->IsDragging() );
        window->SetMovable( false );
        ASSERT_FALSE( window->IsDragging() );
        window->SetMovable( true );
        Release( context );

        // the button is no longer down while the pointer moves (the release went to another window)
        context->FlushChanges();
        start = window->GetRect();
        Press( context, start.left + 50, start.top + 10 );
        ASSERT_TRUE( window->IsDragging() );
        context->InjectMouseMove( start.left + 60, start.top + 10 );
        context->FlushChanges();
        ASSERT_TRUE( window->GetRect() == Moved( start, 10, 0 ) );
        Release( context );
        ASSERT_FALSE( window->IsDragging() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Windows: The fold button and a double click on the title bar collapse the window to its title bar and expand it again" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWindow* window = MakeWindow( context );
        GUI::CRect start = window->GetRect();
        CCounter changed( window, GUI::CWindow::CollapsedChangedEvent, true );
        CCounter moved( window, GUI::CWindow::MovedEvent );

        // the fold button is at the right end of the title bar
        GUI::CRect fold = window->GetFoldButtonRect();
        ASSERT_TRUE( fold.width == TitleHeight && fold.height == TitleHeight && fold.Right() == 300 && 0 == fold.top );
        Int32 foldX = start.left + fold.left + fold.width / 2;
        Int32 foldY = start.top + fold.top + fold.height / 2;

        // a click on it folds the window up: the title bar is all that is left, at the same place
        Click( context, foldX, foldY );
        context->FlushChanges();
        ASSERT_TRUE( window->IsCollapsed() && window->GetBoolProperty( "Collapsed" ) );
        ASSERT_TRUE( 1 == changed.count && "true" == changed.lastNewValue && "false" == changed.lastOldValue );
        ASSERT_TRUE( window->GetRect() == GUI::CRect( start.left, start.top, 300, TitleHeight ) );
        ASSERT_TRUE( 0 == moved.count );
        ASSERT_TRUE( "0,100 0,80" == window->GetProperty( "Position" ) && "0,300 0,200" == window->GetProperty( "Size" ) );

        // and again: everything as it was
        GUI::CRect foldCollapsed = window->GetFoldButtonRect();
        Click( context, start.left + foldCollapsed.left + 5, start.top + 5 );
        context->FlushChanges();
        ASSERT_FALSE( window->IsCollapsed() );
        ASSERT_TRUE( 2 == changed.count && "false" == changed.lastNewValue );
        ASSERT_TRUE( window->GetRect() == start );
        GUI::CWidget* button = window->GetChildWidget( "Ok" );
        ASSERT_TRUE( button->GetRect() == GUI::CRect( start.left + 10, start.top + 40, 100, 30 ) );

        // a double click on the title bar (not on the button) does the same
        context->RenderFrame( 1000 );
        Click( context, start.left + 60, start.top + 10 );
        Click( context, start.left + 60, start.top + 10 );
        context->FlushChanges();
        ASSERT_TRUE( window->IsCollapsed() && 3 == changed.count );
        ASSERT_TRUE( window->GetRect().height == TitleHeight );
        context->RenderFrame( 1000 );
        Click( context, start.left + 60, start.top + 10 );
        Click( context, start.left + 60, start.top + 10 );
        context->FlushChanges();
        ASSERT_FALSE( window->IsCollapsed() );
        ASSERT_TRUE( 4 == changed.count && window->GetRect() == start );

        // a single click on the title bar does nothing, a double click on the body does nothing
        context->RenderFrame( 1000 );
        Click( context, start.left + 60, start.top + 10 );
        context->RenderFrame( 1000 );
        Click( context, start.left + 150, start.top + 150 );
        Click( context, start.left + 150, start.top + 150 );
        context->FlushChanges();
        ASSERT_TRUE( 4 == changed.count && !window->IsCollapsed() );

        // the property does it too, and a repeated value is no change
        window->SetCollapsed( true );
        window->SetCollapsed( true );
        ASSERT_TRUE( 5 == changed.count );
        window->SetCollapsed( false );
        ASSERT_TRUE( 6 == changed.count && window->GetRect().height == 200 );
        context->FlushChanges();
        ASSERT_TRUE( window->GetRect() == start );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Windows: A collapsed window shows no children, they take no hits and no focus; moved while collapsed it stays where it was dropped" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWindow* window = MakeWindow( context );
        GUI::CRect start = window->GetRect();
        GUI::CWidget* button = window->GetChildWidget( "Ok" );
        GUI::CWidget* edit = window->GetChildWidget( "Name" );
        GUI::CRect buttonRect = button->GetRect();

        // drawn while expanded: the red button and the dark body
        IMAGE::TPixelMapPtr frame;
        ASSERT_TRUE( context->CaptureFrame( frame ) );
        ASSERT_TRUE( PixelIs( frame, buttonRect.left + 5, buttonRect.top + 5, 200, 0, 0 ) );
        ASSERT_TRUE( PixelIs( frame, start.left + 250, start.top + 150, 10, 10, 10 ) );

        ASSERT_TRUE( context->SetFocus( edit ) && edit->HasFocus() );
        ASSERT_TRUE( context->HitTest( buttonRect.left + 5, buttonRect.top + 5 ) == button );
        window->SetCollapsed( true );
        context->FlushChanges();

        // the focus went with it
        ASSERT_TRUE( GUCEF_NULL == context->GetFocus() && !edit->HasFocus() );
        ASSERT_FALSE( edit->IsEffectivelyVisible() || button->IsEffectivelyVisible() );
        ASSERT_FALSE( edit->IsFocusable() );
        ASSERT_FALSE( context->SetFocus( edit ) );
        ASSERT_TRUE( GUCEF_NULL == context->GetFocus() );
        // Tab finds nothing to focus below the window
        ASSERT_FALSE( context->Navigate( GUI::NAVIGATION_NEXT ) );

        // no hit on the children, nothing but the title bar is the window
        ASSERT_TRUE( GUCEF_NULL == context->HitTest( buttonRect.left + 5, buttonRect.top + 5 ) );
        ASSERT_TRUE( context->HitTest( start.left + 20, start.top + 5 ) == window );
        UInt32 clicked = 0;
        {
            CCounter activated( button, GUI::CWidget::ActivatedEvent );
            Click( context, buttonRect.left + 5, buttonRect.top + 5 );
            clicked = activated.count;
        }
        ASSERT_TRUE( 0 == clicked );

        // not drawn: the area is the background of the context, the title bar is still there
        ASSERT_TRUE( context->CaptureFrame( frame ) );
        ASSERT_FALSE( PixelIs( frame, buttonRect.left + 5, buttonRect.top + 5, 200, 0, 0 ) );
        ASSERT_FALSE( PixelIs( frame, start.left + 250, start.top + 150, 10, 10, 10 ) );
        ASSERT_TRUE( PixelIs( frame, start.left + 250, start.top + 150, 0, 0, 0 ) );
        ASSERT_TRUE( PixelIs( frame, start.left + 4, start.top + 12, 40, 60, 110 ) );

        // moving it while it is collapsed
        Drag( context, start.left + 40, start.top + 10, start.left + 40 + 30, start.top + 10 + 120 );
        ASSERT_TRUE( window->GetRect() == GUI::CRect( start.left + 30, start.top + 120, 300, TitleHeight ) );
        window->SetCollapsed( false );
        context->FlushChanges();
        ASSERT_TRUE( window->GetRect() == GUI::CRect( start.left + 30, start.top + 120, 300, 200 ) );
        ASSERT_TRUE( button->GetRect() == Moved( buttonRect, 30, 120 ) );
        ASSERT_TRUE( context->HitTest( button->GetRect().left + 5, button->GetRect().top + 5 ) == button );
        ASSERT_TRUE( context->SetFocus( edit ) );
        ASSERT_TRUE( context->CaptureFrame( frame ) );
        ASSERT_TRUE( PixelIs( frame, button->GetRect().left + 5, button->GetRect().top + 5, 200, 0, 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Windows: Movable and Collapsible off make a window that stays put and open; a window without a title bar has neither" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWindow* window = MakeWindow( context );
        GUI::CRect start = window->GetRect();

        window->SetMovable( false );
        Drag( context, start.left + 50, start.top + 10, start.left + 150, start.top + 100 );
        ASSERT_TRUE( window->GetRect() == start && !window->IsMovedByUser() );
        // the fold button and the double click still work
        window->SetCollapsible( false );
        ASSERT_TRUE( window->GetFoldButtonRect().IsEmpty() );
        context->RenderFrame( 1000 );
        Click( context, start.left + 290, start.top + 10 );
        Click( context, start.left + 60, start.top + 10 );
        Click( context, start.left + 60, start.top + 10 );
        context->FlushChanges();
        ASSERT_FALSE( window->IsCollapsed() );
        ASSERT_TRUE( window->GetRect() == start );
        // a property that says collapsed has no effect on a window that cannot collapse
        window->SetCollapsed( true );
        context->FlushChanges();
        ASSERT_FALSE( window->IsCollapsed() );
        ASSERT_TRUE( window->GetRect() == start );
        window->SetCollapsible( true );
        context->FlushChanges();
        ASSERT_TRUE( window->IsCollapsed() && window->GetRect().height == TitleHeight );
        window->SetCollapsed( false );

        // no title bar: nothing to hold and nothing to fold into
        window->SetBoolProperty( "TitleVisible", false );
        context->FlushChanges();
        ASSERT_FALSE( window->IsMovable() || window->IsCollapsible() );
        Drag( context, start.left + 50, start.top + 10, start.left + 150, start.top + 100 );
        ASSERT_TRUE( window->GetRect() == start );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Windows: Roots above a window and modal roots take the input, so the title bar below cannot be dragged" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWindow* window = MakeWindow( context );
        GUI::CRect start = window->GetRect();

        GUI::CWidget* cover = context->CreateWidget( "Panel", "Cover", GUCEF_NULL );
        cover->SetProperty( "Position", "0,90 0,70" );
        cover->SetProperty( "Size", "0,320 0,60" );
        context->ShowRoot( cover, false );
        context->FlushChanges();
        Drag( context, start.left + 50, start.top + 10, start.left + 150, start.top + 100 );
        ASSERT_TRUE( window->GetRect() == start );
        ASSERT_FALSE( window->IsDragging() );
        context->HideRoot( cover );

        // a modal root elsewhere: the window below it gets nothing
        GUI::CWidget* modal = context->CreateWidget( "Panel", "Modal", GUCEF_NULL );
        modal->SetProperty( "Position", "0,400 0,400" );
        modal->SetProperty( "Size", "0,100 0,50" );
        context->ShowRoot( modal, true );
        context->FlushChanges();
        Drag( context, start.left + 50, start.top + 10, start.left + 150, start.top + 100 );
        ASSERT_TRUE( window->GetRect() == start );

        // a modal window is moved, and while it is held the others get nothing either
        context->HideRoot( modal );
        context->ShowRoot( window, true );
        Drag( context, start.left + 50, start.top + 10, start.left + 70, start.top + 20 );
        ASSERT_TRUE( window->GetRect() == Moved( start, 20, 10 ) );
        context->HideRoot( window );
        context->ShowRoot( window, false );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Windows: MinSize and MaxSize limit the size the layout gives, a window of 90 percent stops growing on a large screen" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWindow* window = static_cast< GUI::CWindow* >( context->CreateWidget( "Window", "Window", GUCEF_NULL ) );
        window->SetProperty( "Position", "0.5,0 0.5,0" );
        window->SetProperty( "HAlign", "Center" );
        window->SetProperty( "VAlign", "Center" );
        window->SetProperty( "Size", "0.9,0 0.9,0" );
        window->SetProperty( "MinSize", "0,400 0,300" );
        window->SetProperty( "MaxSize", "0,1000 0,700" );
        context->ShowRoot( window, false );

        context->SetLogicalResolution( 640, 480 );
        context->FlushChanges();
        ASSERT_TRUE( 576 == window->GetRect().width && 432 == window->GetRect().height );
        ASSERT_TRUE( 32 == window->GetRect().left && 24 == window->GetRect().top );

        context->SetLogicalResolution( 1920, 1080 );
        context->FlushChanges();
        ASSERT_TRUE( 1000 == window->GetRect().width && 700 == window->GetRect().height );
        ASSERT_TRUE( 460 == window->GetRect().left && 190 == window->GetRect().top );

        context->SetLogicalResolution( 320, 240 );
        context->FlushChanges();
        // the minimum is more than the screen has: the window is the screen
        ASSERT_TRUE( 320 == window->GetRect().width && 240 == window->GetRect().height );

        // a half limit: only the height
        window->SetProperty( "MinSize", CORE::CString::Empty );
        window->SetProperty( "MaxSize", "0,0 0,100" );
        context->SetLogicalResolution( 640, 480 );
        context->FlushChanges();
        ASSERT_TRUE( 576 == window->GetRect().width && 100 == window->GetRect().height );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Windows: A size that is a part of the parent less more pixels than it has is no size, not a negative one" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWindow* window = MakeWindow( context );
        GUI::CWidget* label = context->CreateWidget( "Label", "Thin", window );
        label->SetProperty( "Position", "0,10 0,10" );
        label->SetProperty( "Size", "1,-1000 1,-1000" );
        context->FlushChanges();
        ASSERT_TRUE( 0 == label->GetRect().width && 0 == label->GetRect().height && label->GetRect().IsEmpty() );
        ASSERT_TRUE( label->GetRect().left == window->GetRect().left + 10 && label->GetRect().Right() == label->GetRect().left );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Windows: The title bar shows the fold button with a glyph for each state and a hover and pressed fill; the title makes room for it" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWindow* window = MakeWindow( context );
        window->SetProperty( "TitleBarColour", "40,60,110,255" );
        window->SetProperty( "TitleColour", "250,250,250,255" );
        GUI::CRect start = window->GetRect();
        GUI::CRect fold = window->GetFoldButtonRect();
        GUI::CRect foldOnScreen( start.left + fold.left, start.top + fold.top, fold.width, fold.height );

        IMAGE::TPixelMapPtr expanded;
        ASSERT_TRUE( context->CaptureFrame( expanded ) );
        UInt32 glyphExpanded = CountPixels( expanded, foldOnScreen, 250, 250, 250 );
        ASSERT_TRUE( glyphExpanded >= 10 && glyphExpanded <= 40 );
        ASSERT_TRUE( PixelIs( expanded, foldOnScreen.left + 1, foldOnScreen.top + 1, 40, 60, 110 ) );

        // hover and press: a fill over the button, the glyph on top
        context->InjectMouseMove( foldOnScreen.left + 3, foldOnScreen.top + 3 );
        context->FlushChanges();
        ASSERT_TRUE( window->GetBoolProperty( "FoldButtonHovered" ) );
        IMAGE::TPixelMapPtr hovered;
        ASSERT_TRUE( context->CaptureFrame( hovered ) );
        ASSERT_FALSE( PixelIs( hovered, foldOnScreen.left + 1, foldOnScreen.top + 1, 40, 60, 110 ) );
        ASSERT_TRUE( CountPixels( hovered, foldOnScreen, 250, 250, 250 ) == glyphExpanded );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        ASSERT_TRUE( window->GetBoolProperty( "FoldButtonPressed" ) );
        IMAGE::TPixelMapPtr pressed;
        ASSERT_TRUE( context->CaptureFrame( pressed ) );
        ASSERT_FALSE( PixelIs( pressed, foldOnScreen.left + 1, foldOnScreen.top + 1, 40, 60, 110 ) );
        context->InjectMouseMove( 600, 400 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_FALSE( window->IsCollapsed() );
        ASSERT_FALSE( window->GetBoolProperty( "FoldButtonPressed" ) || window->GetBoolProperty( "FoldButtonHovered" ) );

        // collapsed: the other direction, the glyph is mirrored
        window->SetCollapsed( true );
        context->FlushChanges();
        IMAGE::TPixelMapPtr collapsed;
        ASSERT_TRUE( context->CaptureFrame( collapsed ) );
        UInt32 glyphCollapsed = CountPixels( collapsed, foldOnScreen, 250, 250, 250 );
        ASSERT_TRUE( glyphCollapsed == glyphExpanded );
        bool different = false;
        for ( Int32 y=foldOnScreen.top; y<foldOnScreen.Bottom(); ++y )
            for ( Int32 x=foldOnScreen.left; x<foldOnScreen.Right(); ++x )
                if ( PixelIs( collapsed, x, y, 250, 250, 250 ) != PixelIs( expanded, x, y, 250, 250, 250 ) )
                    different = true;
        ASSERT_TRUE( different );

        // without the button no glyph
        window->SetCollapsible( false );
        context->FlushChanges();
        ASSERT_TRUE( context->CaptureFrame( collapsed ) );
        ASSERT_TRUE( 0 == CountPixels( collapsed, foldOnScreen, 250, 250, 250 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
