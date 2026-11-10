/*
 *  gucefAUTHGUI_TestApp: Application for testing gucefAUTHGUI
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

#include "gucefAUTHGUI_TestCommon.h"
#include "TestLayoutUtils.h"
#include "TestLoginFixture.h"
#include "TestAccountFixture.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** The widest the page of the account gets: the column of the content and the scroll bar; the tab of the settings window is wider than that only on a very wide screen */
static const Int32 MaxColumn = 1400;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformFormLayoutTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUTHGUI.Layouts" );

    GUCEF_TESTFW_TESTCASE( "Layouts: The login form keeps its rows, the device code, the accounts and the button apart from 600 x 900 to 5120 x 1440" )
    try
    {
        // the sizes of the forms, the ultra wide 32:9 of 3840 x 1080 and a screen smaller than the others
        const SScreenSize loginSizes[] = { { 640, 480 }, { 800, 600 }, { 1024, 768 }, { 1242, 952 }, { 1920, 1080 }, { 3440, 1440 }, { 3840, 1080 }, { 5120, 1440 }, { 600, 900 }, { 2560, 720 } };
        for ( UInt32 s=0; s<10; ++s )
        {
            const UInt32 screenWidth = loginSizes[ s ].width;
            const UInt32 screenHeight = loginSizes[ s ].height;
            const CORE::CString where = SizeName( screenWidth, screenHeight );
            CLoginFixture fixture( screenWidth, screenHeight );
            ASSERT_TRUE( fixture.IsReady() );
            if ( !fixture.IsReady() )
                return;
            AUTHGUI::CLoginForm& form = fixture.form;
            CFakeLoginProvider* google = fixture.AddProvider( "google" );
            CFakeLoginProvider* microsoft = fixture.AddProvider( "microsoft" );
            CFakeLoginProvider* openId2 = fixture.AddProvider( "fakeopenid" );
            CFakeLoginProvider* aws = fixture.AddProvider( "aws-cognito" );
            ASSERT_TRUE( GUCEF_NULL != google && GUCEF_NULL != microsoft && GUCEF_NULL != openId2 && GUCEF_NULL != aws );
            if ( GUCEF_NULL == google || GUCEF_NULL == microsoft || GUCEF_NULL == openId2 || GUCEF_NULL == aws )
                return;
            google->accountNamespace = "google";
            google->subject = "109876543210123456789";
            microsoft->accountNamespace = "microsoft";
            openId2->accountNamespace = "fakeopenid";
            openId2->subject = "4200000000";
            openId2->displayName = "OpenIdPlayer";
            aws->accountNamespace = "aws";
            aws->subject = "eu-west-1_AbCdEfGhI:2f6c9d1e-3333-4444-5555-666677778888";

            // a guest first
            ASSERT_TRUE( form.Open() );
            fixture.Settle();
            GUI::CWidget* window = form.GetRootWidget();
            GUI::CRect rect = window->GetRect();
            Int32 wantedWidth = (Int32) ( ( screenWidth * 9 + 5 ) / 10 );
            Int32 wantedHeight = (Int32) ( ( screenHeight * 9 + 5 ) / 10 );
            wantedWidth = wantedWidth < 400 ? 400 : ( wantedWidth > 640 ? 640 : wantedWidth );
            wantedHeight = wantedHeight < 380 ? 380 : ( wantedHeight > 820 ? 820 : wantedHeight );
            ASSERT_TRUE( rect.width == wantedWidth && rect.height == wantedHeight );
            ASSERT_TRUE( Inside( rect, GUI::CRect( 0, 0, (Int32) screenWidth, (Int32) screenHeight ) ) );
            Int32 centerX = rect.left + rect.width / 2, centerY = rect.top + rect.height / 2;
            ASSERT_TRUE( centerX >= (Int32) screenWidth / 2 - 1 && centerX <= (Int32) screenWidth / 2 + 1 && centerY >= (Int32) screenHeight / 2 - 1 && centerY <= (Int32) screenHeight / 2 + 1 );
            CORE::CString problem = FindLayoutProblem( window );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the login form at " << where.C_String() << ": " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() );
            problem = FindBarEdgeProblem( window );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() );
            IMAGE::TPixelMapPtr frame = CaptureFrame( fixture.context );
            ASSERT_TRUE( OnlyBlackOutside( frame, rect ) && CountLit( frame, rect ) > 500 );
            WriteShot( frame, "login-guest-" + where );

            // busy: a device code, a failure, an account that is linked
            google->mode = CFakeLoginProvider::MODE_DEVICE_CODE;
            microsoft->mode = CFakeLoginProvider::MODE_FAIL;
            openId2->mode = CFakeLoginProvider::MODE_INSTANT;
            aws->mode = CFakeLoginProvider::MODE_INSTANT;
            ASSERT_TRUE( form.Login( "fakeopenid" ) && form.Login( "aws-cognito" ) );
            fixture.Settle();
            ASSERT_TRUE( form.Login( "google" ) && form.Login( "microsoft" ) );
            fixture.Settle();
            ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_SHOWING_DEVICE_CODE == form.GetProgress( "google" ) && AUTHGUI::CLoginForm::LOGINPROGRESS_FAILED == form.GetProgress( "microsoft" ) );
            ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_DONE == form.GetProgress( "fakeopenid" ) );
            window = form.GetRootWidget();
            ASSERT_TRUE( window->GetRect() == rect );
            ASSERT_TRUE( "Linked: AWS, Fakeopenid" == form.GetStateText() || "Linked: Fakeopenid, AWS" == form.GetStateText() );
            problem = FindLayoutProblem( window );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the busy login form at " << where.C_String() << ": " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() );

            // the body scrolls when it does not fit; the last control comes into view, nothing gets out of order
            GUI::CScrollPanel* body = GUI::CWidget::ToScrollPanel( form.GetWidget( "Body" ) );
            ASSERT_TRUE( GUCEF_NULL != body && body->GetRect().Bottom() <= form.GetWidget( "ButtonRow" )->GetRect().top );
            if ( GUCEF_NULL == body )
                return;
            ASSERT_TRUE( ( body->GetContentHeight() > body->GetViewportHeight() ) == body->IsScrollable() );
            // the providers that were logged in with last come first, then the others by name
            CORE::CString::StringVector shown;
            form.GetShownProviders( shown );
            UInt32 googleRow = 99;
            for ( UInt32 g=0; g<(UInt32)shown.size(); ++g )
                if ( "google" == shown[ g ] )
                    googleRow = g;
            ASSERT_TRUE( 4 == shown.size() && googleRow < 4 );
            const CORE::CString googleNumber = CORE::UInt32ToString( googleRow );
            GUI::CWidget* codeField = form.GetWidget( "Code" + googleNumber );
            GUI::CWidget* urlField = form.GetWidget( "Url" + googleNumber );
            GUI::CWidget* openButton = form.GetWidget( "Open" + googleNumber );
            ASSERT_TRUE( GUCEF_NULL != codeField && GUCEF_NULL != urlField && GUCEF_NULL != openButton );
            if ( GUCEF_NULL == codeField || GUCEF_NULL == urlField || GUCEF_NULL == openButton )
                return;
            ASSERT_TRUE( codeField->GetRect().width >= 150 && urlField->GetRect().width >= 150 && openButton->GetRect().width >= 80 );
            frame = CaptureFrame( fixture.context );
            ASSERT_TRUE( OnlyBlackOutside( frame, rect ) && CountLit( frame, rect ) > 500 );
            WriteShot( frame, "login-busy-" + where );
            body->SetScrollPosition( body->GetMaxScrollPosition() );
            fixture.context->FlushChanges();
            ASSERT_TRUE( FindLayoutProblem( window ).IsNULLOrEmpty() );
            GUI::CWidget* last = form.GetWidget( "AutoLogin" );
            ASSERT_TRUE( GUCEF_NULL != last && Inside( last->GetRect(), body->GetRect() ) );
            if ( body->IsScrollable() )
                WriteShot( CaptureFrame( fixture.context ), "login-busy-scrolled-" + where );
            body->SetScrollPosition( 0 );
            fixture.context->FlushChanges();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layouts: The account page in the settings window keeps its controls apart and inside from 600 x 900 to 5120 x 1440" )
    try
    {
        UInt32 marginsTried = 0;
        const SScreenSize pageSizes[] = { { 640, 480 }, { 800, 600 }, { 1024, 768 }, { 1242, 952 }, { 1920, 1080 }, { 3440, 1440 }, { 3840, 1080 }, { 5120, 1440 }, { 600, 900 }, { 2560, 720 } };
        for ( UInt32 s=0; s<10; ++s )
        {
            const UInt32 screenWidth = pageSizes[ s ].width;
            const UInt32 screenHeight = pageSizes[ s ].height;
            const CORE::CString where = SizeName( screenWidth, screenHeight );
            CAccountFixture fixture( screenWidth, screenHeight );
            ASSERT_TRUE( fixture.IsReady() );
            if ( !fixture.IsReady() )
                return;
            AUTHGUI::CAccountSettingsForm& page = *fixture.page;
            CFakeLoginProvider* google = fixture.AddProvider( "google" );
            CFakeLoginProvider* microsoft = fixture.AddProvider( "microsoft" );
            CFakeLoginProvider* openId2 = fixture.AddProvider( "fakeopenid" );
            CFakeLoginProvider* aws = fixture.AddProvider( "aws-cognito" );
            ASSERT_TRUE( GUCEF_NULL != google && GUCEF_NULL != microsoft && GUCEF_NULL != openId2 && GUCEF_NULL != aws );
            if ( GUCEF_NULL == google || GUCEF_NULL == microsoft || GUCEF_NULL == openId2 || GUCEF_NULL == aws )
                return;
            google->accountNamespace = "google";
            google->subject = "109876543210123456789";
            microsoft->accountNamespace = "microsoft";
            openId2->accountNamespace = "fakeopenid";
            openId2->subject = "4200000000";
            openId2->displayName = "OpenIdPlayer";
            aws->accountNamespace = "aws";
            aws->subject = "eu-west-1_AbCdEfGhI:2f6c9d1e-3333-4444-5555-666677778888";

            // a guest first
            ASSERT_TRUE( fixture.Open() );
            GUI::CWidget* window = fixture.settings.GetRootWidget();
            ASSERT_TRUE( Inside( window->GetRect(), GUI::CRect( 0, 0, (Int32) screenWidth, (Int32) screenHeight ) ) );
            CORE::CString problem = FindLayoutProblem( window );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the account page at " << where.C_String() << ": " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() );
            IMAGE::TPixelMapPtr frame = CaptureFrame( fixture.context );
            ASSERT_TRUE( OnlyBlackOutside( frame, window->GetRect() ) && CountLit( frame, window->GetRect() ) > 500 );
            WriteShot( frame, "account-page-guest-" + where );

            // the buttons that log in are as wide as the content: no half width buttons in a wide page
            {
                GUI::CWidget* providersHeader = page.GetWidget( "ProvidersHeader" );
                ASSERT_TRUE( GUCEF_NULL != providersHeader );
                for ( UInt32 b=0; b<4; ++b )
                {
                    GUI::CWidget* login = page.GetWidget( "Login" + CORE::UInt32ToString( b ) );
                    ASSERT_TRUE( GUCEF_NULL != login && GUCEF_NULL != providersHeader && login->GetRect().width == providersHeader->GetRect().width );
                }
            }

            // busy: a device code, a failure, accounts that are linked, a name that was typed
            google->mode = CFakeLoginProvider::MODE_DEVICE_CODE;
            microsoft->mode = CFakeLoginProvider::MODE_FAIL;
            openId2->mode = CFakeLoginProvider::MODE_INSTANT;
            aws->mode = CFakeLoginProvider::MODE_INSTANT;
            ASSERT_TRUE( page.Login( "fakeopenid" ) && page.Login( "aws-cognito" ) );
            fixture.Settle();
            ASSERT_TRUE( page.Login( "google" ) && page.Login( "microsoft" ) );
            fixture.Settle();
            ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_SHOWING_DEVICE_CODE == page.GetProgress( "google" ) && AUTHGUI::CAccountsPanel::LOGINPROGRESS_FAILED == page.GetProgress( "microsoft" ) );
            ASSERT_TRUE( page.SetDisplayNameText( "Alice the second" ) );
            fixture.Settle();
            ASSERT_TRUE( "Alice the second" == page.GetDisplayNameText() );
            problem = FindLayoutProblem( window );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the busy account page at " << where.C_String() << ": " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() );
            frame = CaptureFrame( fixture.context );
            ASSERT_TRUE( OnlyBlackOutside( frame, window->GetRect() ) && CountLit( frame, window->GetRect() ) > 500 );
            WriteShot( frame, "account-page-busy-" + where );

            // the body scrolls when it does not fit; the last control comes into view, nothing gets out of order
            GUI::CScrollPanel* body = GUI::CWidget::ToScrollPanel( page.GetWidget( "Body" ) );
            ASSERT_TRUE( GUCEF_NULL != body );
            if ( GUCEF_NULL == body )
                return;
            ASSERT_TRUE( ( body->GetContentHeight() > body->GetViewportHeight() ) == body->IsScrollable() );
            body->SetScrollPosition( body->GetMaxScrollPosition() );
            fixture.context->FlushChanges();
            ASSERT_TRUE( FindLayoutProblem( window ).IsNULLOrEmpty() );
            GUI::CWidget* last = page.GetWidget( "AutoLogin" );
            ASSERT_TRUE( GUCEF_NULL != last && Inside( last->GetRect(), body->GetRect() ) );
            if ( body->IsScrollable() )
                WriteShot( CaptureFrame( fixture.context ), "account-page-busy-scrolled-" + where );
            body->SetScrollPosition( 0 );
            fixture.context->FlushChanges();

            // the column is as wide as the tab, up to a generous limit, and in the middle
            GUI::CWidget* list = page.GetWidget( "BodyList" );
            ASSERT_TRUE( GUCEF_NULL != list && list->GetRect().width <= MaxColumn - 16 && list->GetRect().width >= 300 );

            // the scroll bar is next to the column at its right edge, not at the edge of the window: the margins at both sides are as wide
            const GUI::CRect bodyRect = body->GetRect();
            const GUI::CRect track = body->GetTrackRect();
            ASSERT_TRUE( 16 == track.width && track.height == bodyRect.height );
            ASSERT_TRUE( bodyRect.left + track.left == list->GetRect().Right() );
            const Int32 leftMargin = list->GetRect().left - bodyRect.left;
            const Int32 rightMargin = bodyRect.Right() - ( bodyRect.left + track.Right() );
            ASSERT_TRUE( leftMargin >= 0 && rightMargin >= 0 && leftMargin - rightMargin <= 1 && rightMargin - leftMargin <= 1 );
            ASSERT_TRUE( bodyRect.width <= MaxColumn || leftMargin == ( bodyRect.width - MaxColumn ) / 2 );
            // no lane in the tab: where the tab is not wider than the limit the content has all of it, the scroll bar at the right edge
            if ( bodyRect.width <= MaxColumn )
                ASSERT_TRUE( 0 == leftMargin && 0 == rightMargin && bodyRect.left + track.Right() == bodyRect.Right() );

            // the controls fill the column: the rows of the providers, the log in buttons too, from the left edge of the text to the right one
            GUI::CWidget* header = page.GetWidget( "ProvidersHeader" );
            ASSERT_TRUE( GUCEF_NULL != header && header->GetRect().left - list->GetRect().left <= 16 && list->GetRect().Right() - header->GetRect().Right() <= 16 );
            GUI::CWidget* unavailable = page.GetWidget( "UnavailableLogin0" );
            if ( GUCEF_NULL != unavailable )
                ASSERT_TRUE( unavailable->GetRect().width == header->GetRect().width );

            ASSERT_TRUE( track.Right() + rightMargin == bodyRect.width && list->GetRect().width + track.width + 2 * leftMargin <= bodyRect.width + 1 );

            // the wheel scrolls the page over the margins on both sides of the column, and a press there does not grab anything
            if ( body->IsScrollable() && leftMargin >= 8 )
            {
                const Int32 y = bodyRect.top + bodyRect.height / 2;
                const Int32 xs[] = { bodyRect.left + 2, bodyRect.left + leftMargin / 2, bodyRect.Right() - 2, bodyRect.left + track.Right() + rightMargin / 2 };
                for ( UInt32 m=0; m<4; ++m )
                {
                    body->SetScrollPosition( 0 );
                    fixture.context->FlushChanges();
                    ASSERT_TRUE( fixture.context->HitTest( xs[ m ], y ) == body );
                    fixture.context->InjectMouseMove( xs[ m ], y );
                    fixture.context->InjectMouseWheel( -1 );
                    fixture.context->FlushChanges();
                    ASSERT_TRUE( body->GetScrollPosition() > 0 );
                    fixture.context->InjectMouseWheel( 1 );
                    fixture.context->FlushChanges();
                    ASSERT_TRUE( 0 == body->GetScrollPosition() );
                    fixture.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
                    ASSERT_FALSE( body->IsThumbDragged() );
                    fixture.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
                    ASSERT_TRUE( 0 == body->GetScrollPosition() );
                }
                ++marginsTried;
            }
            fixture.settings.Cancel();
        }
        // the sizes wider than the column have margins; a wide and low one has a page that scrolls too
        ASSERT_TRUE( marginsTried >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
