/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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

#include <string.h>

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

#include "TestMathModule.h"
#include "TestBitFunctions.h"
#include "TestColour.h"
#include "TestComplex.h"
#include "TestEasing.h"
#include "TestDeterministicTrigonometry.h"
#include "TestFft.h"
#include "TestPolynomialSolver.h"
#include "TestRandomGenerators.h"
#include "TestScalarPacking.h"
#include "TestSpatialAudioFunctions.h"
#include "TestStatistics.h"
#include "TestAngles.h"
#include "TestDirections.h"
#include "TestFixedPoint.h"
#include "TestFixedPointFunctions.h"
#include "TestFloat16.h"
#include "TestFpContraction.h"
#include "TestNumericConversions.h"
#include "TestNumericTraits.h"
#include "TestPoints.h"
#include "TestScalarFunctions.h"
#include "TestScalarFpContraction.h"
#include "TestVectorConversions.h"
#include "TestVectorSerialization.h"
#include "TestVectors.h"
#include "TestMatrices.h"
#include "TestQuaternion.h"
#include "TestEulerAngles.h"
#include "TestTransforms.h"
#include "TestQuaternionPacking.h"
#include "TestProjections.h"
#include "TestCoordinateSystems.h"
#include "TestRotationSerialization.h"
#include "TestMatrixFpContraction.h"
#include "TestSmoothing.h"
#include "TestCurves.h"
#include "TestCurve1D.h"
#include "TestPath.h"
#include "TestIntegrators.h"
#include "TestBallistics.h"
#include "TestCurveFpContraction.h"
#include "TestRandomStream.h"
#include "TestSampling.h"
#include "TestNoise.h"
#include "TestOctahedral.h"
#include "TestQuantizer.h"
#include "TestRandomFpContraction.h"
#include "TestAmbisonics.h"
#include "TestDecompositions.h"
#include "TestGeneralMatrices.h"
#include "TestGeneralMatrixFpContraction.h"
#include "TestLeastSquares.h"
#include "TestSphericalHarmonics.h"
#include "TestProjectionHelpers.h"
#include "TestSpherical.h"
#include "TestFrames.h"
#include "TestVbap.h"
#include "TestSpatialFpContraction.h"
#include "TestSimdEquivalence.h"
#include "TestBatchDispatcher.h"
#include "TestBatchKernels.h"
#include "TestMathRealtimeSafety.h"

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
        // started by a test as the process whose first batch call 16 threads make together, see TestBatchDispatcher.h
        if ( argc > 1 && 0 == strcmp( argv[ 1 ], GUCEF_MATH_TESTAPP_CONCURRENT_FIRST_CALL_ARG ) )
            return RunConcurrentFirstBatchCall();

        GUCEF::CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( logFilename, "gucefMATH_TestApp_Log.txt" );
        GUCEF::CORE::CFileAccess logFileAccess( logFilename, "w" );
        GUCEF::CORE::CStdLogger logger( logFileAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        GUCEF::CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        GUCEF_TESTFW_INIT( "gucefMATH_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( GUCEF::TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        GUCEF::CORE::CString xmlPath = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( xmlPath, "gucefMATH_TestApp_Results.xml" );

        PerformMathModuleTests();
        PerformBitFunctionsTests();
        PerformEasingTests();
        PerformDeterministicTrigonometryTests();
        PerformFftTests();
        PerformPolynomialSolverTests();
        PerformColourTests();
        PerformComplexTests();
        PerformRandomGeneratorsTests();
        PerformScalarPackingTests();
        PerformSpatialAudioFunctionsTests();
        PerformStatisticsTests();
        PerformNumericTraitsTests();
        PerformFixedPointTests();
        PerformFixedPointFunctionsTests();
        PerformAnglesTests();
        PerformScalarFunctionsTests();
        PerformScalarFpContractionTests();
        PerformFloat16Tests();
        PerformNumericConversionsTests();
        PerformVectorsTests();
        PerformPointsTests();
        PerformDirectionsTests();
        PerformVectorConversionsTests();
        PerformVectorSerializationTests();
        PerformFpContractionTests();
        PerformMatricesTests();
        PerformQuaternionTests();
        PerformEulerAnglesTests();
        PerformTransformsTests();
        PerformQuaternionPackingTests();
        PerformProjectionsTests();
        PerformCoordinateSystemsTests();
        PerformRotationSerializationTests();
        PerformMatrixFpContractionTests();
        PerformSmoothingTests();
        PerformCurvesTests();
        PerformCurve1DTests();
        PerformPathTests();
        PerformIntegratorsTests();
        PerformBallisticsTests();
        PerformCurveFpContractionTests();
        PerformRandomStreamTests();
        PerformSamplingTests();
        PerformNoiseTests();
        PerformOctahedralTests();
        PerformQuantizerTests();
        PerformRandomFpContractionTests();
        PerformAmbisonicsTests();
        PerformDecompositionsTests();
        PerformGeneralMatricesTests();
        PerformGeneralMatrixFpContractionTests();
        PerformLeastSquaresTests();
        PerformSphericalHarmonicsTests();
        PerformProjectionHelpersTests();
        PerformSphericalTests();
        PerformFramesTests();
        PerformVbapTests();
        PerformSpatialFpContractionTests();
        PerformSimdEquivalenceTests();
        PerformBatchDispatcherTests();
        PerformBatchKernelsTests();
        PerformMathRealtimeSafetyTests();

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
