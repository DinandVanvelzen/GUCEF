/*
 *  assetpluginUNREAL: Assembles the body and the garments of a character into one skeleton and several skinned meshes
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

#include <math.h>
#include <stdio.h>

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H
#ifndef ASSETCONVERTERLIB_CLAYERPENETRATIONVALIDATOR_H
#include "AssetConverterLib_CLayerPenetrationValidator.h"
#define ASSETCONVERTERLIB_CLAYERPENETRATIONVALIDATOR_H
#endif /* ASSETCONVERTERLIB_CLAYERPENETRATIONVALIDATOR_H ? */

#include "assetpluginUNREAL_CUnrealCharacterAssembler.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCHARACTERASSEMBLER_H ? */

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "unreal";
static const UInt32 MaxMismatchTexts = 8;

const Float64 CUnrealCharacterAssembler::BindTranslationTolerance = 1e-3;
const Float64 CUnrealCharacterAssembler::BindRotationTolerance = 1e-6;
const Float64 CUnrealCharacterAssembler::BindScaleTolerance = 1e-4;

/*-------------------------------------------------------------------------*/

CUnrealCharacterAssembler::SPart::SPart( void )
    : mesh( GUCEF_NULL )
    , slotMaterials( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealCharacterAssembler::SPart::SPart( const CUnrealSkeletalMesh* meshOfPart, const GUCEF::vector< Int32 >* materialsOfPart )
    : mesh( meshOfPart )
    , slotMaterials( materialsOfPart )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealCharacterAssembler::SResult::SResult( void )
    : rootNode( 0 )
    , skinIndex( 0 )
    , jointNodes()
    , bodyHolderNode( 0 )
    , bodyMeshFirst( 0 )
    , garmentHolderNodes()
    , garmentMeshFirst()
    , extraJoints( 0 )
    , mismatches( 0 )
    , mismatchTexts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

/** Whether the bind pose of two bones is the same within the tolerances */
static bool
IsSameBindPose( const CUnrealReferenceSkeleton::SBone& a, const CUnrealReferenceSkeleton::SBone& b )
{GUCEF_TRACE;

    for ( UInt32 c=0; c<3; ++c )
        if ( fabs( a.translation[ c ] - b.translation[ c ] ) > CUnrealCharacterAssembler::BindTranslationTolerance || fabs( a.scale[ c ] - b.scale[ c ] ) > CUnrealCharacterAssembler::BindScaleTolerance )
            return false;
    // a quaternion and its negation are the same rotation
    Float64 dot = 0.0;
    for ( UInt32 c=0; c<4; ++c )
        dot += a.rotation[ c ] * b.rotation[ c ];
    return 1.0 - fabs( dot ) <= CUnrealCharacterAssembler::BindRotationTolerance;
}

/*-------------------------------------------------------------------------*/

/** How far two bind poses are apart: the largest translation difference in centimetres and the rotation difference in degrees */
static void
MeasureBindDifference( const CUnrealReferenceSkeleton::SBone& a, const CUnrealReferenceSkeleton::SBone& b, Float64& translation, Float64& degrees )
{GUCEF_TRACE;

    translation = 0.0;
    for ( UInt32 c=0; c<3; ++c )
        if ( fabs( a.translation[ c ] - b.translation[ c ] ) > translation )
            translation = fabs( a.translation[ c ] - b.translation[ c ] );
    Float64 dot = 0.0;
    for ( UInt32 c=0; c<4; ++c )
        dot += a.rotation[ c ] * b.rotation[ c ];
    dot = fabs( dot ) > 1.0 ? 1.0 : fabs( dot );
    degrees = 2.0 * acos( dot ) * 180.0 / 3.14159265358979;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ParentNameOf( const CUnrealReferenceSkeleton& skeleton, const CUnrealReferenceSkeleton::SBone& bone )
{GUCEF_TRACE;

    return bone.parent < 0 || (size_t) bone.parent >= skeleton.GetBones().size() ? CORE::CString::Empty : skeleton.GetBones()[ (size_t) bone.parent ].name;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealCharacterAssembler::CompareSkeletons( const CUnrealReferenceSkeleton& shared, const CUnrealReferenceSkeleton& other, UInt32& missing, CORE::CString::StringVector& texts )
{GUCEF_TRACE;

    missing = 0;
    UInt32 differing = 0;
    const CUnrealReferenceSkeleton::TBoneVector& bones = other.GetBones();
    for ( size_t b=0; b<bones.size(); ++b )
    {
        const Int32 index = shared.FindBone( bones[ b ].name );
        if ( index < 0 )
        {
            ++missing;
            continue;
        }
        const CUnrealReferenceSkeleton::SBone& sharedBone = shared.GetBones()[ (size_t) index ];
        const CORE::CString parentInShared = ParentNameOf( shared, sharedBone );
        const CORE::CString parentInOther = ParentNameOf( other, bones[ b ] );
        if ( parentInShared != parentInOther )
        {
            ++differing;
            if ( texts.size() < MaxMismatchTexts )
                texts.push_back( "bone " + bones[ b ].name + ": the parent is " + ( parentInOther.IsNULLOrEmpty() ? CORE::CString( "none" ) : parentInOther ) + " in the garment and " + ( parentInShared.IsNULLOrEmpty() ? CORE::CString( "none" ) : parentInShared ) + " in the body" );
        }
        else if ( !IsSameBindPose( sharedBone, bones[ b ] ) )
        {
            ++differing;
            if ( texts.size() < MaxMismatchTexts )
            {
                Float64 translation = 0.0;
                Float64 degrees = 0.0;
                MeasureBindDifference( sharedBone, bones[ b ], translation, degrees );
                char numbers[ 96 ];
                sprintf( numbers, " (translation up to %.3f cm, rotation %.3f degrees)", translation, degrees );
                texts.push_back( "bone " + bones[ b ].name + ": the bind pose of the garment differs from that of the body" + numbers );
            }
        }
    }
    return differing;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealCharacterAssembler::IsCompatible( const CUnrealReferenceSkeleton& shared, const CUnrealReferenceSkeleton& other )
{GUCEF_TRACE;

    const CUnrealReferenceSkeleton::TBoneVector& bones = other.GetBones();
    if ( bones.empty() || shared.GetBones().empty() || bones[ 0 ].name != shared.GetBones()[ 0 ].name )
        return false;
    size_t found = 0;
    for ( size_t b=0; b<bones.size(); ++b )
        if ( shared.FindBone( bones[ b ].name ) >= 0 )
            ++found;
    return found * 2 >= bones.size();
}

/*-------------------------------------------------------------------------*/

/** The bind pose of every bone of a skeleton in the space of the mesh: the local bind poses along the parents */
static void
GetBindWorlds( const CUnrealReferenceSkeleton& skeleton, GUCEF::vector< MATH::TFloat32Matrix4D >& worlds )
{GUCEF_TRACE;

    const CUnrealReferenceSkeleton::TBoneVector& bones = skeleton.GetBones();
    worlds.assign( bones.size(), MATH::TFloat32Matrix4D::Identity() );
    for ( size_t b=0; b<bones.size(); ++b )
    {
        ASSETCONVERTER::CAssetNode node;
        CUnrealSkeletalMeshConverter::SetBindPose( node, bones[ b ] );
        const MATH::TFloat32Matrix4D local = node.transform.ToMatrix();
        worlds[ b ] = bones[ b ].parent >= 0 && (size_t) bones[ b ].parent < b ? worlds[ (size_t) bones[ b ].parent ] * local : local;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealCharacterAssembler::MoveIntoSharedBindPose( const CUnrealReferenceSkeleton& garmentSkeleton   ,
                                                   const GUCEF::vector< UInt32 >& jointMap          ,
                                                   const SResult& result                            ,
                                                   const ASSETCONVERTER::CAssetScene& scene         ,
                                                   ASSETCONVERTER::CAssetSubMesh& subMesh           ,
                                                   Float32& largestMove                             )
{GUCEF_TRACE;

    // per joint of the skin: from the bind pose of the garment into the shared bind pose, SharedWorld * GarmentWorld^-1
    GUCEF::vector< MATH::TFloat32Matrix4D > garmentWorlds;
    GetBindWorlds( garmentSkeleton, garmentWorlds );
    GUCEF::vector< MATH::TFloat32Matrix4D > rebind( result.jointNodes.size(), MATH::TFloat32Matrix4D::Identity() );
    for ( size_t b=0; b<jointMap.size() && b<garmentWorlds.size(); ++b )
        if ( jointMap[ b ] < rebind.size() )
            rebind[ jointMap[ b ] ] = scene.GetWorldMatrix( result.jointNodes[ jointMap[ b ] ] ) * garmentWorlds[ b ].InverseAffine();

    const UInt32 vertexCount = subMesh.GetVertexCount();
    const UInt32 influences = subMesh.influencesPerVertex;
    if ( 0 == influences || subMesh.joints.size() != (size_t) vertexCount * influences || subMesh.weights.size() != (size_t) vertexCount * influences )
        return 0;
    const bool normals = subMesh.HasNormals();
    const bool tangents = subMesh.HasTangents();
    UInt32 moved = 0;
    for ( UInt32 v=0; v<vertexCount; ++v )
    {
        // the blend of the matrices with the weights of the vertex, as linear blend skinning does it
        Float32 m[ 3 ][ 4 ] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
        Float32 total = 0.0f;
        for ( UInt32 k=0; k<influences; ++k )
        {
            const Float32 w = subMesh.weights[ (size_t) v * influences + k ];
            const UInt32 joint = subMesh.joints[ (size_t) v * influences + k ];
            if ( w <= 0.0f || joint >= rebind.size() )
                continue;
            for ( UInt32 r=0; r<3; ++r )
                for ( UInt32 c=0; c<4; ++c )
                    m[ r ][ c ] += w * rebind[ joint ]( r, c );
            total += w;
        }
        if ( total <= 0.0f )
            continue;
        Float32* p = &subMesh.positions[ (size_t) v * 3 ];
        Float32 out[ 3 ];
        for ( UInt32 r=0; r<3; ++r )
            out[ r ] = ( m[ r ][ 0 ] * p[ 0 ] + m[ r ][ 1 ] * p[ 1 ] + m[ r ][ 2 ] * p[ 2 ] + m[ r ][ 3 ] ) / total;
        const Float32 dx = out[ 0 ] - p[ 0 ], dy = out[ 1 ] - p[ 1 ], dz = out[ 2 ] - p[ 2 ];
        const Float32 distance = sqrtf( dx * dx + dy * dy + dz * dz );
        if ( distance > largestMove )
            largestMove = distance;
        if ( distance > 0.0f )
            ++moved;
        p[ 0 ] = out[ 0 ]; p[ 1 ] = out[ 1 ]; p[ 2 ] = out[ 2 ];
        // directions turn with the linear part, then get their length back
        for ( UInt32 d=0; d<2; ++d )
        {
            if ( ( 0 == d && !normals ) || ( 1 == d && !tangents ) )
                continue;
            Float32* n = 0 == d ? &subMesh.normals[ (size_t) v * 3 ] : &subMesh.tangents[ (size_t) v * 4 ];
            Float32 turned[ 3 ];
            for ( UInt32 r=0; r<3; ++r )
                turned[ r ] = m[ r ][ 0 ] * n[ 0 ] + m[ r ][ 1 ] * n[ 1 ] + m[ r ][ 2 ] * n[ 2 ];
            const Float32 length = sqrtf( turned[ 0 ] * turned[ 0 ] + turned[ 1 ] * turned[ 1 ] + turned[ 2 ] * turned[ 2 ] );
            if ( length > 0.0f )
                for ( UInt32 r=0; r<3; ++r )
                    n[ r ] = turned[ r ] / length;
        }
    }
    return moved;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealCharacterAssembler::Assemble( const CORE::CString& characterName       ,
                                     const SPart& body                         ,
                                     const TPartVector& garments               ,
                                     ASSETCONVERTER::CAssetScene& scene        ,
                                     ASSETCONVERTER::CConversionReport& report ,
                                     SResult& result                           )
{GUCEF_TRACE;

    result = SResult();
    if ( GUCEF_NULL == body.mesh || GUCEF_NULL == body.slotMaterials || body.mesh->lods.empty() || body.mesh->skeleton.GetBones().empty() )
    {
        report.AddError( SourceName, characterName, "The body of the character has no LOD models or no bones" );
        return false;
    }

    result.rootNode = scene.AddNode( characterName );
    UInt32 skinIndex = 0;
    if ( !CUnrealSkeletalMeshConverter::AddSkeleton( body.mesh->skeleton, characterName, (Int32) result.rootNode, scene, skinIndex, result.jointNodes ) )
        return false;
    result.skinIndex = skinIndex;

    // joint of the skin by bone name
    GUCEF::map< CORE::CString, UInt32 > jointByName;
    const CUnrealReferenceSkeleton::TBoneVector& bodyBones = body.mesh->skeleton.GetBones();
    for ( size_t b=0; b<bodyBones.size(); ++b )
        jointByName[ bodyBones[ b ].name ] = (UInt32) b;

    result.bodyHolderNode = scene.AddNode( body.mesh->name, (Int32) result.rootNode );
    scene.nodes[ result.bodyHolderNode ].extras.SetAttribute( ASSETCONVERTER::CLayerPenetrationValidator::PartAttribute, ASSETCONVERTER::CLayerPenetrationValidator::BodyPart );
    result.bodyMeshFirst = (UInt32) scene.meshes.size();
    CUnrealSkeletalMeshConverter::AddMeshNodes( *body.mesh, *body.slotMaterials, GUCEF_NULL, (Int32) skinIndex, result.bodyHolderNode, scene, report );

    for ( size_t g=0; g<garments.size(); ++g )
    {
        const CUnrealSkeletalMesh& garment = *garments[ g ].mesh;
        const CUnrealReferenceSkeleton::TBoneVector& bones = garment.skeleton.GetBones();
        if ( garment.lods.empty() || bones.empty() || GUCEF_NULL == garments[ g ].slotMaterials )
        {
            report.AddWarning( SourceName, garment.name, "The garment has no LOD models or no bones: it is left out of the character" );
            result.garmentHolderNodes.push_back( 0 );
            result.garmentMeshFirst.push_back( 0 );
            continue;
        }

        // what does not match the body
        UInt32 missing = 0;
        CORE::CString::StringVector texts;
        const UInt32 differing = CompareSkeletons( body.mesh->skeleton, garment.skeleton, missing, texts );
        result.mismatches += differing;
        for ( size_t t=0; t<texts.size(); ++t )
        {
            if ( result.mismatchTexts.size() < MaxMismatchTexts )
                result.mismatchTexts.push_back( garment.name + ": " + texts[ t ] );
            report.AddWarning( SourceName, garment.name, texts[ t ] );
        }
        if ( differing > texts.size() )
            report.AddWarning( SourceName, garment.name, CORE::UInt32ToString( differing - (UInt32) texts.size() ) + " more bones differ from the body" );

        // the joint of every bone: the body's by name, an extra joint for a bone the body does not have
        GUCEF::vector< UInt32 > jointMap( bones.size(), 0 );
        UInt32 added = 0;
        for ( size_t b=0; b<bones.size(); ++b )
        {
            GUCEF::map< CORE::CString, UInt32 >::const_iterator found = jointByName.find( bones[ b ].name );
            if ( found != jointByName.end() )
            {
                jointMap[ b ] = (*found).second;
                continue;
            }
            const UInt32 parentJoint = bones[ b ].parent >= 0 && (size_t) bones[ b ].parent < b ? jointMap[ (size_t) bones[ b ].parent ] : 0;
            const UInt32 node = scene.AddNode( bones[ b ].name, (Int32) result.jointNodes[ parentJoint ] );
            CUnrealSkeletalMeshConverter::SetBindPose( scene.nodes[ node ], bones[ b ] );
            ASSETCONVERTER::CAssetSkin& skin = scene.skins[ skinIndex ];
            skin.joints.push_back( node );
            skin.inverseBindMatrices.push_back( scene.GetWorldMatrix( node ).InverseAffine() );
            result.jointNodes.push_back( node );
            jointMap[ b ] = (UInt32) result.jointNodes.size() - 1;
            jointByName[ bones[ b ].name ] = jointMap[ b ];
            ++added;
        }
        result.extraJoints += added;

        const UInt32 holder = scene.AddNode( garment.name, (Int32) result.rootNode );
        scene.nodes[ holder ].extras.SetAttribute( ASSETCONVERTER::CLayerPenetrationValidator::PartAttribute, ASSETCONVERTER::CLayerPenetrationValidator::GarmentPart );
        result.garmentHolderNodes.push_back( holder );
        result.garmentMeshFirst.push_back( (UInt32) scene.meshes.size() );
        CUnrealSkeletalMeshConverter::AddMeshNodes( garment, *garments[ g ].slotMaterials, &jointMap, (Int32) skinIndex, holder, scene, report );

        // a garment whose bind pose differs from the body follows the pose of the body with its own reference pose in the engine:
        // at the reference pose of the body its vertices move by the difference. That is baked here, so the one skin of the
        // character holds every vertex in its shared bind pose
        if ( differing > 0 )
        {
            UInt32 moved = 0;
            Float32 largestMove = 0.0f;
            for ( size_t m=result.garmentMeshFirst.back(); m<scene.meshes.size(); ++m )
                for ( size_t s=0; s<scene.meshes[ m ].subMeshes.size(); ++s )
                    moved += MoveIntoSharedBindPose( garment.skeleton, jointMap, result, scene, scene.meshes[ m ].subMeshes[ s ], largestMove );
            if ( moved > 0 )
            {
                char numbers[ 64 ];
                sprintf( numbers, "%.4f", largestMove );
                report.AddInfo( SourceName, garment.name, CORE::UInt32ToString( moved ) + " vertices moved from the bind pose of the garment into that of the body (up to " + numbers + " cm), as the engine places a garment that follows the pose of the body" );
            }
        }
        report.AddInfo( SourceName, garment.name, "Garment skinned onto the skeleton of " + body.mesh->name + " by joint name: " + CORE::UInt64ToString( bones.size() - missing ) + " of " + CORE::UInt64ToString( bones.size() ) + " bones matched, " +
            CORE::UInt32ToString( added ) + " extra joints, " + CORE::UInt32ToString( differing ) + " bones with another parent or bind pose" );
    }

    report.AddInfo( SourceName, characterName, "Character assembled from " + body.mesh->name + " and " + CORE::UInt64ToString( garments.size() ) + " garments on one skeleton of " + CORE::UInt64ToString( result.jointNodes.size() ) + " joints" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
