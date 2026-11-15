/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

/*
 *      This is the main include header for the gucefMATH library.
 *      It is part of a framework called GUCEF.
 */

#ifndef GUCEF_MATH_H
#define GUCEF_MATH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library build configuration
 */
#ifndef GUCEF_MATH_CONFIG_H
#include "gucefMATH_config.h"
#define GUCEF_MATH_CONFIG_H
#endif /* GUCEF_MATH_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

/*
 *      Simple types
 */
#ifndef GUCEF_MATH_ETYPES_H
#include "gucefMATH_ETypes.h"
#define GUCEF_MATH_ETYPES_H
#endif /* GUCEF_MATH_ETYPES_H ? */

#ifdef __cplusplus

#ifndef GUCEF_MATH_CMODULE_H
#include "gucefMATH_CModule.h"
#define GUCEF_MATH_CMODULE_H
#endif /* GUCEF_MATH_CMODULE_H ? */

/*
 *      Numbers: status, numeric kinds, fixed point, constants, angles and scalar functions
 */
#ifndef GUCEF_MATH_MATHSTATUS_H
#include "gucefMATH_MathStatus.h"
#define GUCEF_MATH_MATHSTATUS_H
#endif /* GUCEF_MATH_MATHSTATUS_H ? */

#ifndef GUCEF_MATH_CTNUMERICCONVERTER_H
#include "gucefMATH_CTNumericConverter.h"
#define GUCEF_MATH_CTNUMERICCONVERTER_H
#endif /* GUCEF_MATH_CTNUMERICCONVERTER_H ? */

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

#ifndef GUCEF_MATH_CTMATHCONSTANTS_H
#include "gucefMATH_CTMathConstants.h"
#define GUCEF_MATH_CTMATHCONSTANTS_H
#endif /* GUCEF_MATH_CTMATHCONSTANTS_H ? */

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

#ifndef GUCEF_MATH_CTDEGREE_H
#include "gucefMATH_CTDegree.h"
#define GUCEF_MATH_CTDEGREE_H
#endif /* GUCEF_MATH_CTDEGREE_H ? */

#ifndef GUCEF_MATH_ANGLEFUNCTIONS_H
#include "gucefMATH_AngleFunctions.h"
#define GUCEF_MATH_ANGLEFUNCTIONS_H
#endif /* GUCEF_MATH_ANGLEFUNCTIONS_H ? */

#ifndef GUCEF_MATH_SCALARFUNCTIONS_H
#include "gucefMATH_ScalarFunctions.h"
#define GUCEF_MATH_SCALARFUNCTIONS_H
#endif /* GUCEF_MATH_SCALARFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTCOMPENSATEDSUM_H
#include "gucefMATH_CTCompensatedSum.h"
#define GUCEF_MATH_CTCOMPENSATEDSUM_H
#endif /* GUCEF_MATH_CTCOMPENSATEDSUM_H ? */

#ifndef GUCEF_MATH_FLOATDECOMPOSITION_H
#include "gucefMATH_FloatDecomposition.h"
#define GUCEF_MATH_FLOATDECOMPOSITION_H
#endif /* GUCEF_MATH_FLOATDECOMPOSITION_H ? */

#ifndef GUCEF_MATH_CFLOAT16_H
#include "gucefMATH_CFloat16.h"
#define GUCEF_MATH_CFLOAT16_H
#endif /* GUCEF_MATH_CFLOAT16_H ? */

#ifndef GUCEF_MATH_DECIMALCONVERSIONS_H
#include "gucefMATH_DecimalConversions.h"
#define GUCEF_MATH_DECIMALCONVERSIONS_H
#endif /* GUCEF_MATH_DECIMALCONVERSIONS_H ? */


/*
 *      Vectors, points, directions and CORE structure interoperability
 */
#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_CORESTRUCTINTEROP_H
#include "gucefMATH_CoreStructInterop.h"
#define GUCEF_MATH_CORESTRUCTINTEROP_H
#endif /* GUCEF_MATH_CORESTRUCTINTEROP_H ? */


/*
 *      Easing, random generators, bit functions and packing
 */
#ifndef GUCEF_MATH_EASINGFUNCTIONS_H
#include "gucefMATH_EasingFunctions.h"
#define GUCEF_MATH_EASINGFUNCTIONS_H
#endif /* GUCEF_MATH_EASINGFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CPCG32_H
#include "gucefMATH_CPcg32.h"
#define GUCEF_MATH_CPCG32_H
#endif /* GUCEF_MATH_CPCG32_H ? */

#ifndef GUCEF_MATH_CXOSHIRO256STARSTAR_H
#include "gucefMATH_CXoshiro256StarStar.h"
#define GUCEF_MATH_CXOSHIRO256STARSTAR_H
#endif /* GUCEF_MATH_CXOSHIRO256STARSTAR_H ? */

#ifndef GUCEF_MATH_CSPLITMIX64_H
#include "gucefMATH_CSplitMix64.h"
#define GUCEF_MATH_CSPLITMIX64_H
#endif /* GUCEF_MATH_CSPLITMIX64_H ? */

#ifndef GUCEF_MATH_BITFUNCTIONS_H
#include "gucefMATH_BitFunctions.h"
#define GUCEF_MATH_BITFUNCTIONS_H
#endif /* GUCEF_MATH_BITFUNCTIONS_H ? */

#ifndef GUCEF_MATH_PACKINGFUNCTIONS_H
#include "gucefMATH_PackingFunctions.h"
#define GUCEF_MATH_PACKINGFUNCTIONS_H
#endif /* GUCEF_MATH_PACKINGFUNCTIONS_H ? */


/*
 *      Spatial audio functions, attenuation and directivity
 */
#ifndef GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#include "gucefMATH_SpatialAudioFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTPIECEWISELINEARCURVE_H
#include "gucefMATH_CTPiecewiseLinearCurve.h"
#define GUCEF_MATH_CTPIECEWISELINEARCURVE_H
#endif /* GUCEF_MATH_CTPIECEWISELINEARCURVE_H ? */

#ifndef GUCEF_MATH_CTDISTANCEATTENUATION_H
#include "gucefMATH_CTDistanceAttenuation.h"
#define GUCEF_MATH_CTDISTANCEATTENUATION_H
#endif /* GUCEF_MATH_CTDISTANCEATTENUATION_H ? */

#ifndef GUCEF_MATH_CTDIRECTIVITYPATTERN_H
#include "gucefMATH_CTDirectivityPattern.h"
#define GUCEF_MATH_CTDIRECTIVITYPATTERN_H
#endif /* GUCEF_MATH_CTDIRECTIVITYPATTERN_H ? */


/*
 *      Statistics
 */
#ifndef GUCEF_MATH_CTRUNNINGSTATISTICS_H
#include "gucefMATH_CTRunningStatistics.h"
#define GUCEF_MATH_CTRUNNINGSTATISTICS_H
#endif /* GUCEF_MATH_CTRUNNINGSTATISTICS_H ? */

#ifndef GUCEF_MATH_STATISTICSFUNCTIONS_H
#include "gucefMATH_StatisticsFunctions.h"
#define GUCEF_MATH_STATISTICSFUNCTIONS_H
#endif /* GUCEF_MATH_STATISTICSFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CDECIMALSTATISTICS_H
#include "gucefMATH_CDecimalStatistics.h"
#define GUCEF_MATH_CDECIMALSTATISTICS_H
#endif /* GUCEF_MATH_CDECIMALSTATISTICS_H ? */


/*
 *      Complex numbers, polynomial roots, FFT and colour
 */
#ifndef GUCEF_MATH_CTCOMPLEX_H
#include "gucefMATH_CTComplex.h"
#define GUCEF_MATH_CTCOMPLEX_H
#endif /* GUCEF_MATH_CTCOMPLEX_H ? */

#ifndef GUCEF_MATH_CTPOLYNOMIALSOLVER_H
#include "gucefMATH_CTPolynomialSolver.h"
#define GUCEF_MATH_CTPOLYNOMIALSOLVER_H
#endif /* GUCEF_MATH_CTPOLYNOMIALSOLVER_H ? */

#ifndef GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H
#include "gucefMATH_DeterministicTrigonometry.h"
#define GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H
#endif /* GUCEF_MATH_DETERMINISTICTRIGONOMETRY_H ? */

#ifndef GUCEF_MATH_CREALFFT_H
#include "gucefMATH_CRealFft.h"
#define GUCEF_MATH_CREALFFT_H
#endif /* GUCEF_MATH_CREALFFT_H ? */

#ifndef GUCEF_MATH_CCOMPLEXFFT_H
#include "gucefMATH_CComplexFft.h"
#define GUCEF_MATH_CCOMPLEXFFT_H
#endif /* GUCEF_MATH_CCOMPLEXFFT_H ? */

#ifndef GUCEF_MATH_CTLINEARCOLOUR_H
#include "gucefMATH_CTLinearColour.h"
#define GUCEF_MATH_CTLINEARCOLOUR_H
#endif /* GUCEF_MATH_CTLINEARCOLOUR_H ? */

#ifndef GUCEF_MATH_COLOURFUNCTIONS_H
#include "gucefMATH_ColourFunctions.h"
#define GUCEF_MATH_COLOURFUNCTIONS_H
#endif /* GUCEF_MATH_COLOURFUNCTIONS_H ? */

/*
 *      Matrices, quaternions, Euler angles, transforms, projections and coordinate systems
 */
#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_TRANSFORMS_H
#include "gucefMATH_transforms.h"
#define GUCEF_MATH_TRANSFORMS_H
#endif /* GUCEF_MATH_TRANSFORMS_H ? */

#ifndef GUCEF_MATH_CQUATERNIONPACKING_H
#include "gucefMATH_CQuaternionPacking.h"
#define GUCEF_MATH_CQUATERNIONPACKING_H
#endif /* GUCEF_MATH_CQUATERNIONPACKING_H ? */

#ifndef GUCEF_MATH_SPROJECTIONCONVENTION_H
#include "gucefMATH_SProjectionConvention.h"
#define GUCEF_MATH_SPROJECTIONCONVENTION_H
#endif /* GUCEF_MATH_SPROJECTIONCONVENTION_H ? */

#ifndef GUCEF_MATH_CCOORDINATESYSTEM_H
#include "gucefMATH_CCoordinateSystem.h"
#define GUCEF_MATH_CCOORDINATESYSTEM_H
#endif /* GUCEF_MATH_CCOORDINATESYSTEM_H ? */


/*
 *      Smoothing, curves, paths, integrators and ballistics
 */
#ifndef GUCEF_MATH_SMOOTHINGFUNCTIONS_H
#include "gucefMATH_SmoothingFunctions.h"
#define GUCEF_MATH_SMOOTHINGFUNCTIONS_H
#endif /* GUCEF_MATH_SMOOTHINGFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTBEZIERCURVE2_H
#include "gucefMATH_CTBezierCurve2.h"
#define GUCEF_MATH_CTBEZIERCURVE2_H
#endif /* GUCEF_MATH_CTBEZIERCURVE2_H ? */

#ifndef GUCEF_MATH_CTBEZIERCURVE3_H
#include "gucefMATH_CTBezierCurve3.h"
#define GUCEF_MATH_CTBEZIERCURVE3_H
#endif /* GUCEF_MATH_CTBEZIERCURVE3_H ? */

#ifndef GUCEF_MATH_CTHERMITESPLINE_H
#include "gucefMATH_CTHermiteSpline.h"
#define GUCEF_MATH_CTHERMITESPLINE_H
#endif /* GUCEF_MATH_CTHERMITESPLINE_H ? */

#ifndef GUCEF_MATH_CTCATMULLROMSPLINE_H
#include "gucefMATH_CTCatmullRomSpline.h"
#define GUCEF_MATH_CTCATMULLROMSPLINE_H
#endif /* GUCEF_MATH_CTCATMULLROMSPLINE_H ? */

#ifndef GUCEF_MATH_CTBSPLINE_H
#include "gucefMATH_CTBSpline.h"
#define GUCEF_MATH_CTBSPLINE_H
#endif /* GUCEF_MATH_CTBSPLINE_H ? */

#ifndef GUCEF_MATH_CTARCLENGTHTABLE_H
#include "gucefMATH_CTArcLengthTable.h"
#define GUCEF_MATH_CTARCLENGTHTABLE_H
#endif /* GUCEF_MATH_CTARCLENGTHTABLE_H ? */

#ifndef GUCEF_MATH_CTCURVE_H
#include "gucefMATH_CTCurve.h"
#define GUCEF_MATH_CTCURVE_H
#endif /* GUCEF_MATH_CTCURVE_H ? */

#ifndef GUCEF_MATH_CTPATH_H
#include "gucefMATH_CTPath.h"
#define GUCEF_MATH_CTPATH_H
#endif /* GUCEF_MATH_CTPATH_H ? */

#ifndef GUCEF_MATH_CURVEQUERYFUNCTIONS_H
#include "gucefMATH_CurveQueryFunctions.h"
#define GUCEF_MATH_CURVEQUERYFUNCTIONS_H
#endif /* GUCEF_MATH_CURVEQUERYFUNCTIONS_H ? */

#ifndef GUCEF_MATH_INTEGRATORFUNCTIONS_H
#include "gucefMATH_IntegratorFunctions.h"
#define GUCEF_MATH_INTEGRATORFUNCTIONS_H
#endif /* GUCEF_MATH_INTEGRATORFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTRUNGEKUTTA4_H
#include "gucefMATH_CTRungeKutta4.h"
#define GUCEF_MATH_CTRUNGEKUTTA4_H
#endif /* GUCEF_MATH_CTRUNGEKUTTA4_H ? */

#ifndef GUCEF_MATH_CTBALLISTICS_H
#include "gucefMATH_CTBallistics.h"
#define GUCEF_MATH_CTBALLISTICS_H
#endif /* GUCEF_MATH_CTBALLISTICS_H ? */

/*
 *      Random distributions, sampling, noise, octahedral encoding and quantisation
 */
#ifndef GUCEF_MATH_SAMPLEWARPING_H
#include "gucefMATH_SampleWarping.h"
#define GUCEF_MATH_SAMPLEWARPING_H
#endif /* GUCEF_MATH_SAMPLEWARPING_H ? */

#ifndef GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#include "gucefMATH_LowDiscrepancySequences.h"
#define GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H
#endif /* GUCEF_MATH_LOWDISCREPANCYSEQUENCES_H ? */

#ifndef GUCEF_MATH_CTRANDOMSTREAM_H
#include "gucefMATH_CTRandomStream.h"
#define GUCEF_MATH_CTRANDOMSTREAM_H
#endif /* GUCEF_MATH_CTRANDOMSTREAM_H ? */

#ifndef GUCEF_MATH_CTALIASTABLE_H
#include "gucefMATH_CTAliasTable.h"
#define GUCEF_MATH_CTALIASTABLE_H
#endif /* GUCEF_MATH_CTALIASTABLE_H ? */

#ifndef GUCEF_MATH_CPERLINNOISE_H
#include "gucefMATH_CPerlinNoise.h"
#define GUCEF_MATH_CPERLINNOISE_H
#endif /* GUCEF_MATH_CPERLINNOISE_H ? */

#ifndef GUCEF_MATH_COPENSIMPLEX2NOISE_H
#include "gucefMATH_COpenSimplex2Noise.h"
#define GUCEF_MATH_COPENSIMPLEX2NOISE_H
#endif /* GUCEF_MATH_COPENSIMPLEX2NOISE_H ? */

#ifndef GUCEF_MATH_CVALUENOISE_H
#include "gucefMATH_CValueNoise.h"
#define GUCEF_MATH_CVALUENOISE_H
#endif /* GUCEF_MATH_CVALUENOISE_H ? */

#ifndef GUCEF_MATH_CWORLEYNOISE_H
#include "gucefMATH_CWorleyNoise.h"
#define GUCEF_MATH_CWORLEYNOISE_H
#endif /* GUCEF_MATH_CWORLEYNOISE_H ? */

#ifndef GUCEF_MATH_CTFRACTALNOISE_H
#include "gucefMATH_CTFractalNoise.h"
#define GUCEF_MATH_CTFRACTALNOISE_H
#endif /* GUCEF_MATH_CTFRACTALNOISE_H ? */

#ifndef GUCEF_MATH_CTDOMAINWARP_H
#include "gucefMATH_CTDomainWarp.h"
#define GUCEF_MATH_CTDOMAINWARP_H
#endif /* GUCEF_MATH_CTDOMAINWARP_H ? */

#ifndef GUCEF_MATH_COCTAHEDRALENCODING_H
#include "gucefMATH_COctahedralEncoding.h"
#define GUCEF_MATH_COCTAHEDRALENCODING_H
#endif /* GUCEF_MATH_COCTAHEDRALENCODING_H ? */

#ifndef GUCEF_MATH_CTQUANTIZER_H
#include "gucefMATH_CTQuantizer.h"
#define GUCEF_MATH_CTQUANTIZER_H
#endif /* GUCEF_MATH_CTQUANTIZER_H ? */

/*
 *      General and dynamic matrices, decompositions, least squares, spherical harmonics and ambisonics
 */
#ifndef GUCEF_MATH_CTMATRIX_H
#include "gucefMATH_CTMatrix.h"
#define GUCEF_MATH_CTMATRIX_H
#endif /* GUCEF_MATH_CTMATRIX_H ? */

#ifndef GUCEF_MATH_CTDYNAMICVECTOR_H
#include "gucefMATH_CTDynamicVector.h"
#define GUCEF_MATH_CTDYNAMICVECTOR_H
#endif /* GUCEF_MATH_CTDYNAMICVECTOR_H ? */

#ifndef GUCEF_MATH_CTDYNAMICMATRIX_H
#include "gucefMATH_CTDynamicMatrix.h"
#define GUCEF_MATH_CTDYNAMICMATRIX_H
#endif /* GUCEF_MATH_CTDYNAMICMATRIX_H ? */

#ifndef GUCEF_MATH_CTLUDECOMPOSITION_H
#include "gucefMATH_CTLuDecomposition.h"
#define GUCEF_MATH_CTLUDECOMPOSITION_H
#endif /* GUCEF_MATH_CTLUDECOMPOSITION_H ? */

#ifndef GUCEF_MATH_CTQRDECOMPOSITION_H
#include "gucefMATH_CTQrDecomposition.h"
#define GUCEF_MATH_CTQRDECOMPOSITION_H
#endif /* GUCEF_MATH_CTQRDECOMPOSITION_H ? */

#ifndef GUCEF_MATH_CTCHOLESKYDECOMPOSITION_H
#include "gucefMATH_CTCholeskyDecomposition.h"
#define GUCEF_MATH_CTCHOLESKYDECOMPOSITION_H
#endif /* GUCEF_MATH_CTCHOLESKYDECOMPOSITION_H ? */

#ifndef GUCEF_MATH_CTSYMMETRICEIGEN3_H
#include "gucefMATH_CTSymmetricEigen3.h"
#define GUCEF_MATH_CTSYMMETRICEIGEN3_H
#endif /* GUCEF_MATH_CTSYMMETRICEIGEN3_H ? */

#ifndef GUCEF_MATH_CTSVD3_H
#include "gucefMATH_CTSvd3.h"
#define GUCEF_MATH_CTSVD3_H
#endif /* GUCEF_MATH_CTSVD3_H ? */

#ifndef GUCEF_MATH_CTLEASTSQUARES_H
#include "gucefMATH_CTLeastSquares.h"
#define GUCEF_MATH_CTLEASTSQUARES_H
#endif /* GUCEF_MATH_CTLEASTSQUARES_H ? */

#ifndef GUCEF_MATH_CTSPHERICALHARMONICS_H
#include "gucefMATH_CTSphericalHarmonics.h"
#define GUCEF_MATH_CTSPHERICALHARMONICS_H
#endif /* GUCEF_MATH_CTSPHERICALHARMONICS_H ? */

#ifndef GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H
#include "gucefMATH_CTSphericalHarmonicsRotation.h"
#define GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H
#endif /* GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H ? */

#ifndef GUCEF_MATH_CAMBISONICFORMAT_H
#include "gucefMATH_CAmbisonicFormat.h"
#define GUCEF_MATH_CAMBISONICFORMAT_H
#endif /* GUCEF_MATH_CAMBISONICFORMAT_H ? */

/*
 *      Projection helpers, polar and spherical coordinates, frames, VBAP and vector Doppler
 */
#ifndef GUCEF_MATH_CTVIEWPORT_H
#include "gucefMATH_CTViewport.h"
#define GUCEF_MATH_CTVIEWPORT_H
#endif /* GUCEF_MATH_CTVIEWPORT_H ? */

#ifndef GUCEF_MATH_PROJECTIONFUNCTIONS_H
#include "gucefMATH_ProjectionFunctions.h"
#define GUCEF_MATH_PROJECTIONFUNCTIONS_H
#endif /* GUCEF_MATH_PROJECTIONFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTPOLARCOORDINATES_H
#include "gucefMATH_CTPolarCoordinates.h"
#define GUCEF_MATH_CTPOLARCOORDINATES_H
#endif /* GUCEF_MATH_CTPOLARCOORDINATES_H ? */

#ifndef GUCEF_MATH_CTSPHERICALCOORDINATES_H
#include "gucefMATH_CTSphericalCoordinates.h"
#define GUCEF_MATH_CTSPHERICALCOORDINATES_H
#endif /* GUCEF_MATH_CTSPHERICALCOORDINATES_H ? */

#ifndef GUCEF_MATH_CTORTHONORMALFRAME3D_H
#include "gucefMATH_CTOrthonormalFrame3D.h"
#define GUCEF_MATH_CTORTHONORMALFRAME3D_H
#endif /* GUCEF_MATH_CTORTHONORMALFRAME3D_H ? */

#ifndef GUCEF_MATH_CTVBAP2D_H
#include "gucefMATH_CTVbap2D.h"
#define GUCEF_MATH_CTVBAP2D_H
#endif /* GUCEF_MATH_CTVBAP2D_H ? */

#ifndef GUCEF_MATH_CTVBAP3D_H
#include "gucefMATH_CTVbap3D.h"
#define GUCEF_MATH_CTVBAP3D_H
#endif /* GUCEF_MATH_CTVBAP3D_H ? */

#ifndef GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H
#include "gucefMATH_SpatialAudioVectorFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H ? */

/*
 *      SIMD configuration, batch kernel dispatch and batch functions
 */
#ifndef GUCEF_MATH_SIMD_CONFIG_H
#include "gucefMATH_simd_config.h"
#define GUCEF_MATH_SIMD_CONFIG_H
#endif /* GUCEF_MATH_SIMD_CONFIG_H ? */

#ifndef GUCEF_MATH_MATHBATCHTYPES_H
#include "gucefMATH_MathBatchTypes.h"
#define GUCEF_MATH_MATHBATCHTYPES_H
#endif /* GUCEF_MATH_MATHBATCHTYPES_H ? */

#ifndef GUCEF_MATH_CMATHBATCHDISPATCHER_H
#include "gucefMATH_CMathBatchDispatcher.h"
#define GUCEF_MATH_CMATHBATCHDISPATCHER_H
#endif /* GUCEF_MATH_CMATHBATCHDISPATCHER_H ? */

#ifndef GUCEF_MATH_BATCHFUNCTIONS_H
#include "gucefMATH_BatchFunctions.h"
#define GUCEF_MATH_BATCHFUNCTIONS_H
#endif /* GUCEF_MATH_BATCHFUNCTIONS_H ? */






#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_H ? */
