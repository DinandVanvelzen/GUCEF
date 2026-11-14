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

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_H ? */
