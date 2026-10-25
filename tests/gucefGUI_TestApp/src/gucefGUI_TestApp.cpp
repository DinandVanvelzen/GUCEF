/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

/* the test framework is implemented in this source, before anything includes its header */
#define GUCEF_TESTFW_IMPL

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CALLBACK_H
#include "gucef_test_log_callback.h"
#define GUCEF_TEST_LOG_CALLBACK_H
#endif /* GUCEF_TEST_LOG_CALLBACK_H ? */

#include "gucefGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    try
    {
        GUCEF::CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( logFilename, "gucefGUI_TestApp_Log.txt" );
        GUCEF::CORE::CFileAccess logFileAccess( logFilename, "w" );
        GUCEF::CORE::CStdLogger logger( logFileAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        GUCEF::CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        GUCEF::GUI::CGuiGlobal::Instance();

        GUCEF_TESTFW_INIT( "gucefGUI_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( GUCEF::TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        GUCEF::CORE::CString xmlPath = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( xmlPath, "gucefGUI_TestApp_Results.xml" );

        PerformGuiCoreTests();
        PerformGuiWidgetTests();
        PerformGuiLayoutTests();
        PerformGuiMediaTests();
        PerformGuiRenderTests();
        PerformGuiInputTests();
        PerformGuiWindowTests();
        PerformGuiScrollPanelTests();
        PerformGuiPropertyTests();
        PerformGuiBrowserTests();
        PerformGuiFontTests();
        PerformGuiSettingsTests();
        PerformGuiFileDialogTests();

        GUCEF_TESTFW_WRITE_RESULTS( xmlPath.C_String() );
        return GUCEF_TESTFW_EXITCODE;
    }
    catch ( ... )
    {
        GUCEF::CORE::ShowErrorMessage( "Unknown exception"                                                                 ,
                                       "Unhandled exception during program execution, the application will now terminate"  );
    }
    return 1;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
