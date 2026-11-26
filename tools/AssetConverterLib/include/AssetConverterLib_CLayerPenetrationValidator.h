/*
 *  AssetConverterLib: the built-in validator of the layers of a character: body surface through the garments
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

#ifndef ASSETCONVERTERLIB_CLAYERPENETRATIONVALIDATOR_H
#define ASSETCONVERTERLIB_CLAYERPENETRATIONVALIDATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETVALIDATOR_H
#include "AssetConverterLib_CIAssetValidator.h"
#define ASSETCONVERTERLIB_CIASSETVALIDATOR_H
#endif /* ASSETCONVERTERLIB_CIASSETVALIDATOR_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

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

class CAssetScene;

/**
 *  "LayerPenetration": where a body shows through the garments that are worn over it, in the rest pose. The parts come from the
 *  extras of the nodes: the attribute "gucefCharacterPart" is "body" or "garment" (an importer that assembles a character sets
 *  them); the first level of detail of each part is measured. For every vertex of the body, the line along its normal is
 *  intersected with the triangles of all garments: a garment surface in front of the vertex (up to "frontReach", default 0.10 m)
 *  covers it; a vertex without one, but with a garment surface behind it (up to "backReach", default 0.03 m), lies outside the
 *  garment: the body pokes through there, by the distance to that surface. Pokes of less than "minimumDepth" (default 0.0002 m)
 *  are not counted. Units are those of the scene (metres after the normalisation).
 *
 *  The visible body is measured: submeshes of the body that an active hidden face set hides (CAssetSubMesh::hiddenBy, the
 *  outfit is worn) are not drawn and are left out. Their vertices are counted apart.
 *
 *  The result: an info line with the counts and the five joints with the most pokes (the strongest influence of the vertex), and
 *  a result of the configured severity (default warning) when more than "maxVertices" (default 0) vertices poke through.
 *  Scenes without parts are not checked.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CLayerPenetrationValidator : public CIAssetValidator
{
    public:

    static const CORE::CString TypeName;        /**< "LayerPenetration" */
    static const CORE::CString PartAttribute;   /**< "gucefCharacterPart" */
    static const CORE::CString BodyPart;        /**< "body" */
    static const CORE::CString GarmentPart;     /**< "garment" */

    /** The measurement, for the report and for tests */
    struct ASSETCONVERTERLIB_PUBLIC_CPP SResult
    {
        UInt32 bodyVertices;            /**< vertices of the first level of the body */
        UInt32 coveredVertices;         /**< with a garment surface in front */
        UInt32 pokingVertices;          /**< outside every garment surface that lies behind them */
        UInt32 hiddenBodyVertices;      /**< vertices of submeshes of the body that an active hidden face set hides: not measured */
        Float32 deepestPoke;            /**< the largest distance of a poking vertex to the surface behind it */
        GUCEF::map< CORE::CString, UInt32 > pokesPerJoint;   /**< the strongest joint of each poking vertex, by node name */

        SResult( void );
    };

    CLayerPenetrationValidator( void );

    virtual ~CLayerPenetrationValidator() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Validate( const CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;

    /** The measurement itself; false when the scene has no body or no garment */
    static bool Measure( const CAssetScene& scene, Float32 frontReach, Float32 backReach, Float32 minimumDepth, SResult& result );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CLAYERPENETRATIONVALIDATOR_H ? */
