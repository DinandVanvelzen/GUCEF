/*
 *  gucefDRNUTILSGUI_TestApp: GUCEF test application for the gucefDRNUTILSGUI module
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
//      INCLUDES                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDrnUtilsGuiFixtures.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformControlConfirmFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsGui.ControlConfirmForm" );

    GUCEF_TESTFW_TESTCASE( "ControlConfirmForm: asks modally, shows the action and the target, takes the reason of the field" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            DRNUTILSGUI::CControlConfirmForm form( context );
            CEventCount confirmed( &form, DRNUTILSGUI::CControlConfirmForm::ConfirmedEvent );
            CEventCount cancelled( &form, DRNUTILSGUI::CControlConfirmForm::CancelledEvent );

            ASSERT_FALSE( form.IsOpen() );
            ASSERT_TRUE( form.Ask( "Run now", "Run now on the node alpha", "Job job-1 (Fetch quotes)" ) );
            ASSERT_TRUE( form.IsOpen() );
            ASSERT_TRUE( "Run now on the node alpha" == TextOfWidget( form, "Action" ) );
            ASSERT_TRUE( "Job job-1 (Fetch quotes)" == TextOfWidget( form, "Target" ) );
            ASSERT_TRUE( "Run now" == form.GetRootWidget()->GetProperty( "Title" ) );
            ASSERT_TRUE( TextOfWidget( form, "Reason" ).IsNULLOrEmpty() );

            // the operator types the reason and presses Confirm: the form says it and carries the reason
            form.GetWidget( "Reason" )->SetProperty( "Text", "nightly check" );
            ClickWidget( context, form.GetWidget( "Confirm" ) );
            ASSERT_TRUE( !form.IsOpen() );
            ASSERT_TRUE( 1 == confirmed.count && 0 == cancelled.count );
            ASSERT_TRUE( "nightly check" == form.GetReason() );

            // a second question starts with an empty reason; Cancel answers no and sends no reason
            ASSERT_TRUE( form.Ask( "Disable", "Disable on the node alpha", "Job job-2" ) );
            ASSERT_TRUE( TextOfWidget( form, "Reason" ).IsNULLOrEmpty() );
            ClickWidget( context, form.GetWidget( "Cancel" ) );
            ASSERT_TRUE( !form.IsOpen() );
            ASSERT_TRUE( 1 == confirmed.count && 1 == cancelled.count );

            // answers of a form that is not asking are ignored
            form.Confirm( "late" );
            form.Cancel();
            ASSERT_TRUE( 1 == confirmed.count && 1 == cancelled.count );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
