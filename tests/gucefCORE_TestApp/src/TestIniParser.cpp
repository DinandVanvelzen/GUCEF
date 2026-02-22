/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_CINIPARSER_H
#include "gucefCORE_CIniParser.h"
#define GUCEF_CORE_CINIPARSER_H
#endif /* GUCEF_CORE_CINIPARSER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestIniParser.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const CORE::CString testIni1 =
"[SectionHeader1]\n"
"myKey1=myValue1\n"
"myKey2=myValue2\n\n"
"[SectionHeader2]\n"
"myKey3=myValue3\n"
"myKey4=myValue4";

static const CORE::CString testIni2 =
"[SectionHeader1\\SubSection1]\n"
"myKey1=myValue1\n"
"myKey2=myValue2\n\n"
"[SectionHeader1\\SubSection2]\n"
"myKey3=myValue3\n"
"myKey4=myValue4"
"[SectionHeader2]\n"
"myKey5=myValue5\n"
"myKey6=myValue6";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformIniParserTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CIniParser TESTS" );
    
    GUCEF_TESTFW_SUITE_SCOPE( "CIniParser" );

    GUCEF_TESTFW_TESTCASE( "Test: INI file parsing" )
        try
        {
            CORE::CIniParser iniParser;
            ASSERT_TRUE( iniParser.LoadFrom( testIni1 ) );
            ASSERT_TRUE( iniParser.LoadFrom( testIni2 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CIniParser TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
