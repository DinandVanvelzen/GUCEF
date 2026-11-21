/*
 *  assetpluginUNREAL: The presentation hints of a character: the framing of a 3D portrait, the lights, the full body view
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPRESENTATIONHINTS_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPRESENTATIONHINTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H
#include "assetpluginUNREAL_CUnrealCharacterAssembler.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H
#include "assetpluginUNREAL_CUnrealMetaHumanCharacter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a viewer needs to open a character on its portrait. The numbers are in the convention of the intermediate (metres, +Y
 *  up, right-handed, the same as glTF), whatever the source units were, so that they stay valid after the normalisation, which
 *  does not touch extras. The hints are a tree for the key "gucefPresentation" of the extras of the scene; the schema is
 *  documented in the design of the Unreal reader (section "U8h1 as built").
 *
 *  The head: the joint "head" of the skeleton gives the position; the head bounds are those of the vertices of the body whose
 *  strongest influence is that joint or a joint below it. Without the joint or without such vertices the top 15 percent of the
 *  bounds along up stand for the head, and the hints say so ("estimated").
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealPresentationHints
{
    public:

    static const char* const ExtrasKey;         /**< "gucefPresentation" */
    static const char* const CharacterKey;      /**< "gucefCharacter": the settings and textures of the character record */

    static const Float32 PortraitFovDegrees;
    static const Float32 PortraitYawDegrees;
    static const Float32 PortraitPitchDegrees;

    struct SVector
    {
        Float32 v[ 3 ];

        SVector( void );
        SVector( Float32 x, Float32 y, Float32 z );
    };

    /** The framing of one view: look at 'target' from 'position' */
    struct SView
    {
        SVector target;
        SVector position;
        Float32 distance;
        Float32 fovDegrees;
        Float32 yawDegrees;                     /**< rotation about up from the facing direction, toward the side vector */
        Float32 pitchDegrees;                   /**< positive: the camera looks down */
        Float32 frameHeight;                    /**< the height of the picture at the target, metres */
    };

    struct SLight
    {
        Float32 yawDegrees;
        Float32 pitchDegrees;
        Float32 intensity;
        SVector towardLight;                    /**< unit vector from the character to the light */
    };

    struct SHints
    {
        SVector up;
        SVector facing;
        SVector side;                           /**< cross( up, facing ) */
        SVector boundsMin;
        SVector boundsMax;
        SVector headMin;
        SVector headMax;
        SVector headJoint;
        bool hasHeadJoint;
        bool headEstimated;
        SView portrait;
        SView fullBody;
        SVector turntableCentre;
        Float32 turntableRadius;
        SLight key;
        SLight fill;
        SLight rim;

        SHints( void );
    };

    /**
     *  The framing from the bounds of the character, the bounds of its head and the direction it faces; pure geometry, no scene.
     *  @param lightRotation the light rotation of the character asset in degrees; the key light yaw is its negative (the engine turns the other way), relative to the facing direction; the key stands at 45 when the asset states none
     */
    static void Compute( const SVector& boundsMin, const SVector& boundsMax, const SVector& headMin, const SVector& headMax, const SVector& facing,
                         bool hasLightRotation, Float32 lightRotation, SHints& hints );

    /**
     *  The hints of an assembled character of a scene that is still in the coordinate system of its source (the importer states
     *  it): bounds and head from the skeleton and the vertices of the first LOD of the body and the garments, converted to
     *  the intermediate convention.
     *  @param character the properties record, NULL when the package has none (then no light rotation, camera frame or environment is known)
     */
    static bool Build( const ASSETCONVERTER::CAssetScene& scene                          ,
                       const CUnrealCharacterAssembler::SResult& assembly                ,
                       const CUnrealMetaHumanCharacter* character                        ,
                       const CORE::CString& characterName                                ,
                       ASSETCONVERTER::CConversionReport& report                         ,
                       SHints& hints                                                     );

    /** The tree for the extras of the scene */
    static void ToDataNode( const SHints& hints, const CUnrealMetaHumanCharacter* character, const CORE::CString& characterName, CORE::CDataNode& node );

    private:

    CUnrealPresentationHints( void );    /**< only static members */
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPRESENTATIONHINTS_H ? */
