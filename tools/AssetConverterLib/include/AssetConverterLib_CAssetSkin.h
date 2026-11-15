/*
 *  AssetConverterLib: a skin of the intermediate model: joints and inverse bind matrices
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

#ifndef ASSETCONVERTERLIB_CASSETSKIN_H
#define ASSETCONVERTERLIB_CASSETSKIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A skin: the joints (node indices), an inverse bind matrix per joint and the skeleton root (-1 for none) */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetSkin
{
    public:

    typedef GUCEF::vector< UInt32 > TUInt32Vector;
    typedef GUCEF::vector< MATH::TFloat32Matrix4D > TMatrixVector;

    CORE::CString name;
    TUInt32Vector joints;
    TMatrixVector inverseBindMatrices;
    Int32 skeletonRoot;

    CAssetSkin( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETSKIN_H ? */
