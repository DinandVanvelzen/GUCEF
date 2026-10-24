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

void
PerformGameGuiGlobalTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.Global" );

    GUCEF_TESTFW_TESTCASE( "Global: The module sets up the systems it builds on" )
    try
    {
        GAMEGUI::CGameGuiGlobal* global = GAMEGUI::CGameGuiGlobal::Instance();
        ASSERT_TRUE( GUCEF_NULL != global && global == GAMEGUI::CGameGuiGlobal::Instance() );
        ASSERT_TRUE( GUCEF_NULL != MULTIPLAY::CMultiplayGlobal::Instance() );
        ASSERT_TRUE( GUCEF_NULL != GUI::CGuiGlobal::Instance() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Global: Every form builds its layout on the null driver in the dark theme and starts hidden" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CServerBrowserForm browser( context );
        ASSERT_TRUE( browser.Build() && GUCEF_NULL != browser.GetRootWidget() && !browser.IsVisible() );
        GAMEGUI::CCreateLobbyForm create( context );
        ASSERT_TRUE( create.Build() && GUCEF_NULL != create.GetRootWidget() && !create.IsVisible() && GUCEF_NULL != create.GetSettingsPanel() );
        GAMEGUI::CLobbyForm lobby( context );
        ASSERT_TRUE( lobby.Build() && GUCEF_NULL != lobby.GetRootWidget() && !lobby.IsVisible() );
        ASSERT_TRUE( GUCEF_NULL != lobby.GetChatForm() && GUCEF_NULL != lobby.GetSettingsPanel() && GUCEF_NULL != lobby.GetVotePrompt() );
        GAMEGUI::CVotePromptForm vote( context );
        ASSERT_TRUE( vote.Build() && !vote.IsVisible() );
        GAMEGUI::CConnectingForm connecting( context );
        ASSERT_TRUE( connecting.Build() && !connecting.IsVisible() );
        GAMEGUI::CDirectConnectForm direct( context );
        ASSERT_TRUE( direct.Build() && !direct.IsVisible() );
        GAMEGUI::CPlayerProfileForm profile( context );
        ASSERT_TRUE( profile.Build() && !profile.IsVisible() );

        // the embeddable forms fill a widget of another form
        GUI::CWidget* holder = context->CreateWidget( "Panel", "Holder", GUCEF_NULL );
        ASSERT_TRUE( GUCEF_NULL != holder );
        GAMEGUI::CChatForm chat;
        ASSERT_TRUE( chat.Build( holder ) && GUCEF_NULL != chat.GetRootWidget() && chat.GetRootWidget()->GetParentWidget() == holder );
        GAMEGUI::CLobbySettingsPanelForm panel;
        ASSERT_TRUE( panel.Build( holder ) && GUCEF_NULL != panel.GetRootWidget() );
        ASSERT_FALSE( chat.Build( GUCEF_NULL ) || panel.Build( GUCEF_NULL ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Global: The helpers of the forms format what the lists show" )
    try
    {
        ASSERT_TRUE( GAMEGUI::FormatPing( -1 ).IsNULLOrEmpty() );
        ASSERT_TRUE( "35 ms" == GAMEGUI::FormatPing( 35 ) );
        ASSERT_TRUE( "2/4" == GAMEGUI::FormatPlayers( 2, 4 ) && "2" == GAMEGUI::FormatPlayers( 2, 0 ) );
        ASSERT_TRUE( "a b" == GAMEGUI::TrimText( "  a b \t" ) );
        CORE::CDateTime time;
        ASSERT_TRUE( -1 == GAMEGUI::GetSecondsUntil( time ) );
        time = CORE::CDateTime::NowUTCDateTime();
        time.AddSeconds( 30 );
        Int32 left = GAMEGUI::GetSecondsUntil( time );
        ASSERT_TRUE( left >= 29 && left <= 30 );
        time.AddSeconds( -60 );
        ASSERT_TRUE( 0 == GAMEGUI::GetSecondsUntil( time ) );
        CORE::CString::StringVector lines;
        lines.push_back( "a" );
        lines.push_back( "b" );
        ASSERT_TRUE( "a\nb" == GAMEGUI::JoinLines( lines ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
