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
 *  The exported instantiations of the random number, sampling, noise, octahedral and quantiser code for the predefined
 *  kinds (section 5.10 of the design), compiled here without floating-point contraction.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

#ifndef GUCEF_MATH_CTQUANTIZER_H
#include "gucefMATH_CTQuantizer.h"
#define GUCEF_MATH_CTQUANTIZER_H
#endif /* GUCEF_MATH_CTQUANTIZER_H ? */

#ifndef GUCEF_MATH_CTFRACTALNOISE_H
#include "gucefMATH_CTFractalNoise.h"
#define GUCEF_MATH_CTFRACTALNOISE_H
#endif /* GUCEF_MATH_CTFRACTALNOISE_H ? */

#ifndef GUCEF_MATH_CTDOMAINWARP_H
#include "gucefMATH_CTDomainWarp.h"
#define GUCEF_MATH_CTDOMAINWARP_H
#endif /* GUCEF_MATH_CTDOMAINWARP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      INSTANTIATIONS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_INSTANTIATE_SAMPLEWARPING( kind, vector2, vector3 ) GUCEF_MATH_SAMPLEWARPING_FUNCTIONS( GUCEF_MATH_INSTANTIATE_TEMPLATE, kind, vector2, vector3 )
GUCEF_MATH_SAMPLEWARPING_PREDEFINED( GUCEF_MATH_INSTANTIATE_SAMPLEWARPING )

#define GUCEF_MATH_INSTANTIATE_LOWDISCREPANCY( kind, vector3 ) GUCEF_MATH_LOWDISCREPANCY_FUNCTIONS( GUCEF_MATH_INSTANTIATE_TEMPLATE, kind, vector3 )
GUCEF_MATH_LOWDISCREPANCY_PREDEFINED( GUCEF_MATH_INSTANTIATE_LOWDISCREPANCY )

GUCEF_MATH_CTRANDOMSTREAM_PREDEFINED( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTALIASTABLE_PREDEFINED( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTQUANTIZER_PREDEFINED( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTFRACTALNOISE_PREDEFINED( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTDOMAINWARP_PREDEFINED( GUCEF_MATH_INSTANTIATE_TEMPLATE )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
