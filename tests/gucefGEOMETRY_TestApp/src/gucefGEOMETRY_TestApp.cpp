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

#define GUCEF_TESTFW_IMPL

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CALLBACK_H
#include "gucef_test_log_callback.h"
#define GUCEF_TEST_LOG_CALLBACK_H
#endif /* GUCEF_TEST_LOG_CALLBACK_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#include "TestClosestPoints.h"
#include "TestContainment.h"
#include "TestConvexDecomposition.h"
#include "TestDelaunay.h"
#include "TestEpa.h"
#include "TestExactKindShapes.h"
#include "TestFitting.h"
#include "TestFrustum.h"
#include "TestGeoObjects.h"
#include "TestGeoPlane.h"
#include "TestGeoPolygonQueries.h"
#include "TestGeoSnapshots.h"
#include "TestGeoSpace.h"
#include "TestGeoSpaceConversions.h"
#include "TestGeometryModule.h"
#include "TestGjk.h"
#include "TestGridTraversal.h"
#include "TestSpatialStructures.h"
#include "TestSimplification.h"
#include "TestPointInPolygon.h"
#include "TestPolygonBooleans.h"
#include "TestPolygonOffset.h"
#include "TestKdTree.h"
#include "TestHeightField.h"
#include "TestGeometryBatchKernels.h"
#include "TestConvexHull.h"
#include "TestIntegerRanges.h"
#include "TestOverlaps.h"
#include "TestPlaneBoundedVolume.h"
#include "TestPredicates.h"
#include "TestQueries2D.h"
#include "TestQueryFpContraction.h"
#include "TestRayCasts.h"
#include "TestSatManifold.h"
#include "TestShapeFpContraction.h"
#include "TestShapeSerialization.h"
#include "TestShapes2D.h"
#include "TestShapes3D.h"
#include "TestSweeps.h"
#include "TestTimeOfImpact.h"
#include "TestTriangulation.h"
#include "TestVolumeShapes.h"
#include "TestTriangleMesh.h"
#include "TestMeshBvh.h"
#include "TestDynamicAabbTree.h"
#include "TestRayCaster.h"
#include "TestAudioOcclusion.h"
#include "TestImageSources.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    try
    {
        GUCEF::CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( logFilename, "gucefGEOMETRY_TestApp_Log.txt" );
        GUCEF::CORE::CFileAccess logFileAccess( logFilename, "w" );
        GUCEF::CORE::CStdLogger logger( logFileAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        GUCEF::CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        GUCEF_TESTFW_INIT( "gucefGEOMETRY_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( GUCEF::TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        GUCEF::CORE::CString xmlPath = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( xmlPath, "gucefGEOMETRY_TestApp_Results.xml" );

        PerformGeometryModuleTests();
        PerformShapes3DTests();
        PerformShapes2DTests();
        PerformIntegerRangesTests();
        PerformExactKindShapesTests();
        PerformShapeSerializationTests();
        PerformShapeFpContractionTests();
        PerformRayCastsTests();
        PerformOverlapsTests();
        PerformClosestPointsTests();
        PerformVolumeShapesTests();
        PerformFrustumTests();
        PerformPlaneBoundedVolumeTests();
        PerformContainmentTests();
        PerformQueries2DTests();
        PerformQueryFpContractionTests();
        PerformPredicatesTests();
        PerformGjkTests();
        PerformEpaTests();
        PerformSatManifoldTests();
        PerformSweepsTests();
        PerformTimeOfImpactTests();
        PerformFittingTests();
        PerformGeoSpaceTests();
        PerformGeoSpaceConversionsTests();
        PerformGeoObjectsTests();
        PerformGeoPlaneTests();
        PerformGeoPolygonQueriesTests();
        PerformGeoSnapshotsTests();
        PerformGridTraversalTests();
        PerformSpatialStructuresTests();
        PerformSimplificationTests();
        PerformPointInPolygonTests();
        PerformKdTreeTests();
        PerformHeightFieldTests();
        PerformGeometryBatchKernelsTests();
        PerformConvexHullTests();
        PerformTriangulationTests();
        PerformConvexDecompositionTests();
        PerformDelaunayTests();
        PerformPolygonBooleansTests();
        PerformPolygonOffsetTests();
        PerformTriangleMeshTests();
        PerformMeshBvhTests();
        PerformDynamicAabbTreeTests();
        PerformRayCasterTests();
        PerformAudioOcclusionTests();
        PerformImageSourcesTests();

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
