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
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_DRNUTILSGUI_H
#include "gucefDRNUTILSGUI.h"
#define GUCEF_DRNUTILSGUI_H
#endif /* GUCEF_DRNUTILSGUI_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDrnUtilsGuiSkeleton.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDrnUtilsGuiSkeletonTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsGuiSkeleton" );

    GUCEF_TESTFW_TESTCASE( "ModuleGlobal" )
    {
        ASSERT_TRUE( GUCEF_NULL != DRNUTILSGUI::CDrnUtilsGuiGlobal::Instance() );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "TimeDisplayDefaults" )
    {
        DRNUTILSGUI::CTimeDisplaySettings settings;
        ASSERT_TRUE( settings.GetDisplayTimeZone() == DRNUTILSGUI::CTimeDisplaySettings::LocalZoneName );
        ASSERT_TRUE( settings.GetShowZoneSuffix() );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "FormsConstruct" )
    {
        DRNUTILSGUI::CSchedulerForm schedulerForm;
        DRNUTILSGUI::CJobRunHistoryForm runHistoryForm;
        DRNUTILSGUI::CTaskManagerForm taskManagerForm;
        DRNUTILSGUI::CControlConfirmForm confirmForm;
        ASSERT_TRUE( !schedulerForm.IsOpen() );
        ASSERT_TRUE( !runHistoryForm.IsOpen() );
        ASSERT_TRUE( !taskManagerForm.IsOpen() );
        ASSERT_TRUE( !confirmForm.IsOpen() );
        ASSERT_TRUE( DRNUTILSGUI::CSchedulerForm::TAB_JOBS == schedulerForm.GetShownTab() );
        ASSERT_TRUE( DRNUTILSGUI::CTaskManagerForm::TAB_POOLS == taskManagerForm.GetShownTab() );
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
