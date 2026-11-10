/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#include "gucefGAMEGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const GUI::CColour NormalClock( 226, 229, 236, 255 );
static const GUI::CColour RedClock( 255, 80, 80, 255 );
static const GUI::CColour NormalBox( 28, 31, 38, 240 );
static const GUI::CColour RedBox( 120, 16, 16, 240 );

static CORE::CString
ClockText( GAMEGUI::CTurnWaitOverlayForm& form )
{
    return form.GetWidget( "Clock" )->GetProperty( "Text" );
}

static bool
IsClockColour( GAMEGUI::CTurnWaitOverlayForm& form, const GUI::CColour& colour )
{
    return colour == form.GetWidget( "Clock" )->GetColourProperty( "TextColour" );
}

static bool
IsBoxColour( GAMEGUI::CTurnWaitOverlayForm& form, const GUI::CColour& colour )
{
    return colour == form.GetRootWidget()->GetColourProperty( "BackgroundColour" );
}

/*-------------------------------------------------------------------------*/

void
PerformTurnWaitOverlayFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.TurnWaitOverlay" );

    GUCEF_TESTFW_TESTCASE( "TurnWaitOverlay: The message and the clock show in a box in the middle that is not modal, and the clock counts down with the frames" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CTurnWaitOverlayForm form( context );
        CEventCount expired( &form, GAMEGUI::CTurnWaitOverlayForm::TimeExpiredEvent );
        ASSERT_FALSE( form.IsOpen() || form.IsClockShown() );
        ASSERT_TRUE( 5000 == form.GetWarningThresholdMs() );

        ASSERT_TRUE( form.Open( "Waiting on player Bob to complete their turn", 12000, true ) );
        ASSERT_TRUE( form.IsOpen() && form.IsVisible() && form.IsClockShown() );
        ASSERT_FALSE( context->IsModalRoot( form.GetRootWidget() ) );
        ASSERT_TRUE( "Waiting on player Bob to complete their turn" == form.GetMessage() );
        ASSERT_TRUE( "Waiting on player Bob to complete their turn" == form.GetWidget( "Message" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "0:12" == ClockText( form ) && form.GetWidget( "Clock" )->IsVisible() );
        ASSERT_TRUE( 12000 == form.GetRemainingMs() );
        ASSERT_FALSE( form.IsWarning() || form.IsRed() );
        ASSERT_TRUE( IsClockColour( form, NormalClock ) && IsBoxColour( form, NormalBox ) );

        // the box is in the middle of the context
        context->FlushChanges();
        const GUI::CRect& rect = form.GetRootWidget()->GetRect();
        Int32 centerX = rect.left + rect.width / 2;
        Int32 centerY = rect.top + rect.height / 2;
        ASSERT_TRUE( centerX >= 639 && centerX <= 641 && centerY >= 479 && centerY <= 481 );
        ASSERT_TRUE( form.GetRootWidget()->IsEffectivelyVisible() );

        // the time left follows the frames, rounded up on the clock
        context->RenderFrame( 1000 );
        ASSERT_TRUE( 11000 == form.GetRemainingMs() && "0:11" == ClockText( form ) );
        context->RenderFrame( 1 );
        ASSERT_TRUE( 10999 == form.GetRemainingMs() && "0:11" == ClockText( form ) );
        context->RenderFrame( 999 );
        ASSERT_TRUE( 10000 == form.GetRemainingMs() && "0:10" == ClockText( form ) );
        context->RenderFrame( 0 );
        ASSERT_TRUE( 10000 == form.GetRemainingMs() );
        ASSERT_TRUE( 0 == expired.count );

        // the application can say it better, the message can change
        form.SetRemainingMs( 125000 );
        ASSERT_TRUE( 125000 == form.GetRemainingMs() && "2:05" == ClockText( form ) );
        form.SetMessage( "Waiting on player Carol to complete their turn" );
        ASSERT_TRUE( "Waiting on player Carol to complete their turn" == form.GetWidget( "Message" )->GetProperty( "Text" ) );
        form.SetRemainingMs( -50 );
        ASSERT_TRUE( 0 == form.GetRemainingMs() && "0:00" == ClockText( form ) );

        // close takes it away, the frames do nothing then
        form.Close();
        ASSERT_FALSE( form.IsOpen() || form.IsVisible() || form.IsClockShown() );
        UInt32 count = expired.count;
        context->RenderFrame( 1000 );
        ASSERT_TRUE( count == expired.count );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TurnWaitOverlay: From 5 seconds left the clock and the box flash red twice a second, and the widgets are only touched when the phase changes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CTurnWaitOverlayForm form( context );
        ASSERT_TRUE( form.Open( "Waiting", 6000, true ) );

        // one millisecond before the mark: normal; at the mark: red
        context->RenderFrame( 999 );
        ASSERT_TRUE( 5001 == form.GetRemainingMs() && !form.IsWarning() && !form.IsRed() );
        ASSERT_TRUE( IsClockColour( form, NormalClock ) && IsBoxColour( form, NormalBox ) );
        context->RenderFrame( 1 );
        ASSERT_TRUE( 5000 == form.GetRemainingMs() && form.IsWarning() && form.IsRed() && "0:05" == ClockText( form ) );
        ASSERT_TRUE( IsClockColour( form, RedClock ) && IsBoxColour( form, RedBox ) );

        // a mark set by hand stays while the phase is the same
        form.GetWidget( "Clock" )->SetProperty( "TextColour", "1,2,3,255" );
        context->RenderFrame( 100 );
        ASSERT_TRUE( 4900 == form.GetRemainingMs() && form.IsRed() && IsClockColour( form, GUI::CColour( 1, 2, 3, 255 ) ) );
        context->RenderFrame( 149 );
        ASSERT_TRUE( 4751 == form.GetRemainingMs() && form.IsRed() && IsClockColour( form, GUI::CColour( 1, 2, 3, 255 ) ) );

        // 250 ms later the phase changes: normal for 250 ms, red for 250 ms, ...
        context->RenderFrame( 1 );
        ASSERT_TRUE( 4750 == form.GetRemainingMs() && form.IsWarning() && !form.IsRed() );
        ASSERT_TRUE( IsClockColour( form, NormalClock ) && IsBoxColour( form, NormalBox ) );
        context->RenderFrame( 249 );
        ASSERT_TRUE( 4501 == form.GetRemainingMs() && !form.IsRed() );
        context->RenderFrame( 1 );
        ASSERT_TRUE( 4500 == form.GetRemainingMs() && form.IsRed() && IsClockColour( form, RedClock ) && IsBoxColour( form, RedBox ) );

        // frames of other sizes land in the same phases: 4500 left, 20 phases to go in 4500 ms, red at the even ones
        UInt32 toggles = 0;
        bool wasRed = form.IsRed();
        for ( UInt32 i=0; i<450; ++i )
        {
            context->RenderFrame( 10 );
            if ( form.IsRed() != wasRed )
            {
                ++toggles;
                wasRed = form.IsRed();
            }
            Int64 sinceMark = 5000 - form.GetRemainingMs();
            ASSERT_TRUE( form.IsRed() == ( 0 == ( sinceMark / 250 ) % 2 ) );
        }
        ASSERT_TRUE( 0 == form.GetRemainingMs() && 18 == toggles );      // phases 3..20 of the 21, each of them a change
        ASSERT_TRUE( form.IsRed() && "0:00" == ClockText( form ) );      // the last phase, the 5000th ms, is a red one

        // close puts the normal colours back for the next time
        form.Close();
        ASSERT_FALSE( form.IsRed() );
        ASSERT_TRUE( IsClockColour( form, NormalClock ) && IsBoxColour( form, NormalBox ) );
        ASSERT_TRUE( form.Open( "Waiting", 20000, true ) && !form.IsWarning() && !form.IsRed() && IsClockColour( form, NormalClock ) );

        // a time left set to below the mark flashes at once, and back above it the flashing is over
        form.SetRemainingMs( 4000 );
        ASSERT_TRUE( form.IsWarning() && form.IsRed() && IsClockColour( form, RedClock ) );
        form.SetRemainingMs( 4700 );
        ASSERT_TRUE( form.IsWarning() && !form.IsRed() );                // 300 ms into the mark: the second phase, normal
        form.SetRemainingMs( 4800 );
        ASSERT_TRUE( form.IsWarning() && form.IsRed() );                 // 200 ms into the mark: the first phase
        form.SetRemainingMs( 5001 );
        ASSERT_TRUE( !form.IsWarning() && !form.IsRed() && IsClockColour( form, NormalClock ) && IsBoxColour( form, NormalBox ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TurnWaitOverlay: The warning threshold can be set, and 0 never flashes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CTurnWaitOverlayForm form( context );
        form.SetWarningThresholdMs( 1000 );
        ASSERT_TRUE( 1000 == form.GetWarningThresholdMs() );
        ASSERT_TRUE( form.Open( "Waiting", 3000, true ) );
        context->RenderFrame( 1999 );
        ASSERT_TRUE( 1001 == form.GetRemainingMs() && !form.IsWarning() && !form.IsRed() );
        context->RenderFrame( 1 );
        ASSERT_TRUE( form.IsWarning() && form.IsRed() );
        context->RenderFrame( 250 );
        ASSERT_TRUE( form.IsWarning() && !form.IsRed() );

        // changed while it runs
        form.SetWarningThresholdMs( 0 );
        ASSERT_FALSE( form.IsWarning() || form.IsRed() );
        context->RenderFrame( 100 );
        ASSERT_FALSE( form.IsWarning() || form.IsRed() );
        form.SetWarningThresholdMs( 10150 );
        ASSERT_TRUE( form.IsWarning() && form.IsRed() );                 // 650 ms left, 9500 ms into the mark: phase 38
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TurnWaitOverlay: The end of the time is notified once, and again for a new time" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CTurnWaitOverlayForm form( context );
        CEventCount expired( &form, GAMEGUI::CTurnWaitOverlayForm::TimeExpiredEvent );
        ASSERT_TRUE( form.Open( "Waiting", 600, true ) );
        context->RenderFrame( 500 );
        ASSERT_TRUE( 0 == expired.count && 100 == form.GetRemainingMs() );
        context->RenderFrame( 100 );
        ASSERT_TRUE( 1 == expired.count && 0 == form.GetRemainingMs() && "0:00" == ClockText( form ) );
        context->RenderFrame( 100 );
        context->RenderFrame( 5000 );
        ASSERT_TRUE( 1 == expired.count && 0 == form.GetRemainingMs() );
        ASSERT_TRUE( form.IsOpen() && form.IsVisible() );               // the application decides what happens

        // a frame that is longer than the time left ends it too
        form.SetRemainingMs( 300 );
        context->RenderFrame( 5000 );
        ASSERT_TRUE( 2 == expired.count && 0 == form.GetRemainingMs() );

        // opening again arms it again
        ASSERT_TRUE( form.Open( "Waiting", 100, true ) );
        context->RenderFrame( 100 );
        ASSERT_TRUE( 3 == expired.count );

        // no time at all is over on the first frame
        ASSERT_TRUE( form.Open( "Waiting", 0, true ) );
        ASSERT_TRUE( 3 == expired.count );
        context->RenderFrame( 16 );
        ASSERT_TRUE( 4 == expired.count );
        context->RenderFrame( 16 );
        ASSERT_TRUE( 4 == expired.count );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TurnWaitOverlay: Without a clock only the message shows and nothing counts down" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CTurnWaitOverlayForm form( context );
        CEventCount expired( &form, GAMEGUI::CTurnWaitOverlayForm::TimeExpiredEvent );
        ASSERT_TRUE( form.Open( "Waiting on player Bob to complete their turn", 3000, false ) );
        ASSERT_TRUE( form.IsOpen() && form.IsVisible() && !form.IsClockShown() );
        ASSERT_TRUE( "Waiting on player Bob to complete their turn" == form.GetWidget( "Message" )->GetProperty( "Text" ) );
        ASSERT_FALSE( form.GetWidget( "Clock" )->IsVisible() );
        context->RenderFrame( 5000 );
        context->RenderFrame( 5000 );
        ASSERT_TRUE( 0 == expired.count && !form.IsWarning() && !form.IsRed() );
        ASSERT_TRUE( IsClockColour( form, NormalClock ) && IsBoxColour( form, NormalBox ) );

        // with a clock again the clock is back
        ASSERT_TRUE( form.Open( "Waiting", 8000, true ) );
        ASSERT_TRUE( form.IsClockShown() && form.GetWidget( "Clock" )->IsVisible() && "0:08" == ClockText( form ) );
        form.Close();
        ASSERT_TRUE( form.Open( "Waiting", 8000, false ) );
        ASSERT_FALSE( form.GetWidget( "Clock" )->IsVisible() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
