/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_H
#include "gucefGEOMETRY.h"
#define GUCEF_GEOMETRY_H
#endif /* GUCEF_GEOMETRY_H ? */

#ifndef GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H
#include "gucefGEOMETRY_CGeometryGlobal.h"
#define GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H
#endif /* GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H ? */

#include "TestGeometryModule.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGeometryModuleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "GeometryModule: The global singleton exists and is stable" )
    try
    {
        GEOMETRY::CGeometryGlobal* global = GEOMETRY::CGeometryGlobal::Instance();
        ASSERT_TRUE( GUCEF_NULL != global );
        ASSERT_TRUE( global == GEOMETRY::CGeometryGlobal::Instance() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeometryModule: The module entry points are exported and the shape table is sized" )
    try
    {
        ASSERT_TRUE( GEOMETRY::GeometryModulePublicSpecProbe() );
        ASSERT_TRUE( GEOMETRY::GEOSHAPE_COUNT > GEOMETRY::GEOSHAPE_UNKNOWN );
        ASSERT_TRUE( GEOMETRY::FRUSTUMCLASSIFICATION_INSIDE > GEOMETRY::FRUSTUMCLASSIFICATION_OUTSIDE );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
