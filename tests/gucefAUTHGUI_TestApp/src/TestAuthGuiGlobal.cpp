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
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAuthGuiGlobalTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUTHGUI.Global" );

    GUCEF_TESTFW_TESTCASE( "Global: The module sets up the systems it builds on and nothing of the game" )
    try
    {
        AUTHGUI::CAuthGuiGlobal* global = AUTHGUI::CAuthGuiGlobal::Instance();
        ASSERT_TRUE( GUCEF_NULL != global && global == AUTHGUI::CAuthGuiGlobal::Instance() );
        ASSERT_TRUE( GUCEF_NULL != AUTH::CAuthGlobal::Instance() );
        ASSERT_TRUE( GUCEF_NULL != GUI::CGuiGlobal::Instance() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Global: The page \"Account\" is in the page factory of the settings form, a tool adds it by name" )
    try
    {
        AUTHGUI::CAuthGuiGlobal::Instance();
        ASSERT_TRUE( GUI::CSettingsForm::GetPageFactory().IsConstructible( AUTHGUI::CAuthGuiGlobal::AccountPageName ) );

        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CSettingsForm settings( context );
        GUI::CSettingsPageForm* page = settings.AddPage( "Account" );
        ASSERT_TRUE( GUCEF_NULL != page && "Account" == page->GetTitle() && page == settings.GetPage( "Account" ) );
        ASSERT_TRUE( GUCEF_NULL != page && "GUCEF::AUTHGUI::CAccountSettingsForm" == page->GetClassTypeName() );

        // a null manager and no profile: the page is made and is usable
        ASSERT_TRUE( settings.Build() && settings.Open( "Account" ) );
        ASSERT_TRUE( "Account" == settings.GetSelectedPageTitle() );
        settings.Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
