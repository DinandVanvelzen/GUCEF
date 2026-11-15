/*
 *  AssetConverterLib: brings an imported asset into the conventions of the intermediate
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

#ifndef ASSETCONVERTERLIB_CASSETNORMALISER_H
#define ASSETCONVERTERLIB_CASSETNORMALISER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CCOORDINATESYSTEM_H
#include "gucefMATH_CCoordinateSystem.h"
#define GUCEF_MATH_CCOORDINATESYSTEM_H
#endif /* GUCEF_MATH_CCOORDINATESYSTEM_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */


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

class CConversionReport;

/*-------------------------------------------------------------------------*/

/**
 *  The normalisation: brings an asset from the conventions of its source into those of the intermediate, which are
 *  those of glTF: metres, +Y up, right-handed, -Z forward (MATH::CCoordinateSystem::GucefDefault()), normal maps with
 *  green up. Metal-roughness in G and B and occlusion in R are the mapping of the importers (texture operations).
 *
 *  - The coordinate system of the source is the one the importer states (CAssetScene::coordinateSystem), unless the
 *    effective settings override it: "coordinateSystem" as text "(+X, +Z, +Y, 100)" (right, up, forward, units per
 *    metre) or only "unitsPerMetre".
 *  - The conversion is exact apart from the unit scale (a signed permutation): node transforms and animation keys
 *    are re-expressed (P R P^T), vertex positions, normals, tangents and morph deltas converted, inverse bind matrices
 *    conjugated, colliders moved with their shapes scaled. When the conversion mirrors, triangle winding is reversed
 *    and the handedness of the tangents flips.
 *  - DirectX normal maps get a green flip at the end of their operation chain.
 *
 *  What it did is recorded in CAssetScene::normalisation, for the sidecar, and as an info item of the report.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetNormaliser
{
    public:

    /** False when the coordinate system of the source (or its override) is not valid */
    static bool Normalise( CAssetScene& scene, CConversionReport& report );

    /** Converts the geometry, hierarchy, skins, animations and colliders between the systems */
    static void ConvertCoordinateSystem( CAssetScene& scene, const MATH::CCoordinateSystem& source, const MATH::CCoordinateSystem& target );

    /** Adds a green flip to every texture that a material uses as normal map; the number of textures */
    static UInt32 FlipNormalMapGreen( CAssetScene& scene );

    private:

    CAssetNormaliser( void );
    ~CAssetNormaliser();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETNORMALISER_H ? */
