/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#ifndef GUCEF_ASSETCOOKER_OGRE_CLODCOOKER_H
#define GUCEF_ASSETCOOKER_OGRE_CLODCOOKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreMesh.h"

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_ASSETCOOKER_OGRE_MACROS_H
#include "assetcookerOGRE_macros.h"
#define GUCEF_ASSETCOOKER_OGRE_MACROS_H
#endif /* GUCEF_ASSETCOOKER_OGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The levels of detail of the cooked meshes, with Ogre's screen ratio pixel count strategy
 *  ("screen_ratio_pixel_count"): the value of a level is the part of the viewport that the bounding sphere of the
 *  object covers when it switches to that level.
 *
 *  A screen relative height h (the height of the object relative to the height of the screen, as LOD groups and the
 *  screen coverage of MSFT_lod give it, taken as the diameter of the bounding sphere) becomes the value pi / 4 * h^2,
 *  which is the covered part of a square viewport; on a viewport of aspect a Ogre's value is that divided by a, so
 *  wider screens switch somewhat earlier. The LOD bias b of the cook profile scales h by 2^-b: a positive bias keeps
 *  the detail longer.
 *
 *  - Source LODs (policy "source"): the meshes of the other levels of a LOD group are cooked as meshes of their own and
 *    become manual levels.
 *  - Generated LODs (policy "generate"): MeshLodGenerator reduces the mesh by the reduction table of the policy, the
 *    "lod" rules of the manifest:
 *        "lod": { "policy": "generate", "method": "proportional",
 *                 "levels": [ { "screenRelativeHeight": 0.25, "reduction": 0.5 }, { "screenRelativeHeight": 0.1, "reduction": 0.8 } ] }
 *    "method" is "proportional" (the part of the vertices removed, 0 to 1; the default), "constant" (the number of
 *    vertices removed) or "collapseCost" (reduce until the cost exceeds the value). Without "levels" the table above
 *    is used. Meshes that are a level of a LOD group of the source keep their source levels.
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP CLodCooker
{
    public:

    struct SLevel
    {
        Float32 screenRelativeHeight;
        CORE::CString manualMeshName;   /**< manual levels: the resource name of the mesh of the level */
        Float32 reduction;              /**< generated levels: the value of the reduction method */

        SLevel( void );
    };
    typedef GUCEF::vector< SLevel > TLevelVector;

    /** The value of the screen ratio pixel count strategy for a screen relative height */
    static Float32 ToStrategyValue( Float32 screenRelativeHeight, Float32 lodBias );

    /** Gives the mesh the manual levels (after level 0, from near to far); the number of levels made */
    static UInt32 ApplyManualLevels( Ogre::MeshPtr& mesh, const TLevelVector& levels, Float32 lodBias, ASSETCONVERTER::CConversionReport& report );

    /** Generates the levels of the reduction table; the number of levels made */
    static UInt32 GenerateLevels( Ogre::MeshPtr& mesh, const CORE::CDataNode& lodSettings, Float32 lodBias, ASSETCONVERTER::CConversionReport& report );

    /** The reduction table of the "lod" rules, or the default table */
    static void ReadReductionTable( const CORE::CDataNode& lodSettings, TLevelVector& levels, CORE::CString& method );

    private:

    CLodCooker( void );
    ~CLodCooker();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETCOOKER_OGRE_CLODCOOKER_H ? */
