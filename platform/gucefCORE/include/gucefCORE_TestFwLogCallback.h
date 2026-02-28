/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic types
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

#ifndef GUCEF_CORE_TESTFWLOGCALLBACK_H
#define GUCEF_CORE_TESTFWLOGCALLBACK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_LOGGINGMACROS_H
#include "gucefCORE_LoggingMacros.h"
#define GUCEF_CORE_LOGGINGMACROS_H
#endif /* GUCEF_CORE_LOGGINGMACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reusable callback that routes test framework events to the GUCEF logging system.
 *  Pass to GUCEF_TESTFW_SET_CALLBACK() in any test application's main().
 */
inline void TestFrameworkLogCallback(
    GUCEF::TESTFW::ETestEvent eventType,
    const char* suiteName,
    const char* caseName,
    const char* message,
    const char* file,
    Int32 line,
    UInt32 failureCount,
    void* /*userData*/ )
{GUCEF_TRACE;

    switch ( eventType )
    {
        case GUCEF::TESTFW::GUCEF_TESTFW_EVENT_INIT:
            GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "Test Framework initialized: " << ( message ? message : "" ) );
            break;
        case GUCEF::TESTFW::GUCEF_TESTFW_EVENT_SUITE_BEGIN:
            GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "Suite BEGIN: " << ( suiteName ? suiteName : "" ) );
            break;
        case GUCEF::TESTFW::GUCEF_TESTFW_EVENT_SUITE_END:
            GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "Suite END: " << ( suiteName ? suiteName : "" ) );
            break;
        case GUCEF::TESTFW::GUCEF_TESTFW_EVENT_CASE_BEGIN:
            GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "  Test BEGIN: " << ( caseName ? caseName : "" ) );
            break;
        case GUCEF::TESTFW::GUCEF_TESTFW_EVENT_CASE_END:
            if ( failureCount > 0 )
                { GUCEF_WARNING_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "  Test END: " << ( caseName ? caseName : "" ) << " - FAILED (" << failureCount << " failures)" ); }
            else
                { GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "  Test END: " << ( caseName ? caseName : "" ) << " - PASSED" ); }
            break;
        case GUCEF::TESTFW::GUCEF_TESTFW_EVENT_FAILURE:
            GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "    ASSERTION FAILED: " << ( message ? message : "" ) << " at " << ( file ? file : "" ) << ":" << line );
            break;
        case GUCEF::TESTFW::GUCEF_TESTFW_EVENT_EXCEPTION:
            GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "    EXCEPTION: " << ( message ? message : "" ) << " at " << ( file ? file : "" ) << ":" << line );
            break;
        case GUCEF::TESTFW::GUCEF_TESTFW_EVENT_RESULTS_WRITTEN:
            GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "Test results written to: " << ( message ? message : "" ) << " (" << failureCount << " total failures)" );
            break;
        default:
            break;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTFWLOGCALLBACK_H ? */
