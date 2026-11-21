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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>
#include <math.h>

#include "OgreMeshManager.h"
#include "OgreMaterialManager.h"
#include "OgreMeshSerializer.h"
#include "OgreSubMesh.h"
#include "OgreHardwareBufferManager.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreKeyFrame.h"
#include "OgrePose.h"

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "assetcookerOGRE_CBoneSubsetSplitter.h"
#include "assetcookerOGRE_CBufferDataStream.h"
#include "assetcookerOGRE_CNodeAnimationSampler.h"
#include "assetcookerOGRE_COgreCookEnvironment.h"
#include "assetcookerOGRE_COutputNames.h"
#include "assetcookerOGRE_CMeshCooker.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "ogre";
static const UInt32 MaxWeights = 4;
static const UInt32 MaxBonesPerSubMesh = 256;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SInfluence
{
    UInt32 joint;
    Float32 weight;
};

bool
IsStronger( const SInfluence& a, const SInfluence& b )
{
    if ( a.weight != b.weight )
        return a.weight > b.weight;
    return a.joint < b.joint;
}

/** Whether two submeshes have the same streams, so they can be one */
bool
HaveSameStreams( const ASSETCONVERTER::CAssetSubMesh& a, const ASSETCONVERTER::CAssetSubMesh& b )
{
    if ( 0 == a.GetVertexCount() || 0 == b.GetVertexCount() )
        return false;
    if ( a.HasNormals() != b.HasNormals() || a.HasTangents() != b.HasTangents() || a.GetUvSetCount() != b.GetUvSetCount() ||
         a.colours.empty() != b.colours.empty() || a.masks.empty() != b.masks.empty() || a.IsSkinned() != b.IsSkinned() || a.morphTargets.size() != b.morphTargets.size() )
        return false;
    if ( !a.colours.empty() && a.colours.size() / a.GetVertexCount() != b.colours.size() / b.GetVertexCount() )
        return false;
    if ( a.IsSkinned() && a.influencesPerVertex != b.influencesPerVertex )
        return false;
    for ( size_t t=0; t<a.morphTargets.size(); ++t )
        if ( a.morphTargets[ t ].name != b.morphTargets[ t ].name || a.morphTargets[ t ].normalDeltas.empty() != b.morphTargets[ t ].normalDeltas.empty() )
            return false;
    return true;
}

/** A part of one as percent with one decimal: 0.15 is "15.0" */
CORE::CString
ToPercent( Float64 part )
{
    const UInt32 tenths = (UInt32) ( 1000.0 * part + 0.5 );
    return CORE::UInt32ToString( tenths / 10 ) + "." + CORE::UInt32ToString( tenths % 10 );
}

template< typename T >
void
AppendAll( GUCEF::vector< T >& to, const GUCEF::vector< T >& from )
{
    to.insert( to.end(), from.begin(), from.end() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMeshCooker::SInfluenceReduction::SInfluenceReduction( void )
    : reducedVertices( 0 )
    , mostInfluences( 0 )
    , lostWeightSum( 0.0 )
    , lostWeightMax( 0.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMeshCooker::ReduceInfluences( ASSETCONVERTER::CAssetSubMesh& subMesh, SInfluenceReduction& reduction )
{GUCEF_TRACE;

    const UInt32 n = subMesh.influencesPerVertex;
    const UInt32 vertexCount = subMesh.GetVertexCount();
    if ( n <= MaxWeights || !subMesh.IsSkinned() )
        return;

    ASSETCONVERTER::CAssetSubMesh::TUInt32Vector joints( (size_t) vertexCount * MaxWeights, 0 );
    ASSETCONVERTER::CAssetSubMesh::TFloatVector weights( (size_t) vertexCount * MaxWeights, 0.0f );
    GUCEF::vector< SInfluence > influences;
    for ( UInt32 v=0; v<vertexCount; ++v )
    {
        influences.clear();
        Float32 all = 0.0f;
        for ( UInt32 i=0; i<n; ++i )
        {
            const size_t at = (size_t) v * n + i;
            if ( at >= subMesh.weights.size() || at >= subMesh.joints.size() || subMesh.weights[ at ] <= 0.0f )
                continue;
            SInfluence influence;
            influence.joint = subMesh.joints[ at ];
            influence.weight = subMesh.weights[ at ];
            influences.push_back( influence );
            all += influence.weight;
        }
        std::sort( influences.begin(), influences.end(), IsStronger );
        if ( influences.size() > MaxWeights )
        {
            ++reduction.reducedVertices;
            if ( influences.size() > reduction.mostInfluences )
                reduction.mostInfluences = (UInt32) influences.size();
            influences.resize( MaxWeights );
        }
        Float32 kept = 0.0f;
        for ( size_t i=0; i<influences.size(); ++i )
            kept += influences[ i ].weight;
        if ( all > 0.0f && kept < all )
        {
            const Float32 lost = ( all - kept ) / all;
            reduction.lostWeightSum += lost;
            if ( lost > reduction.lostWeightMax )
                reduction.lostWeightMax = lost;
        }
        for ( size_t i=0; i<influences.size(); ++i )
        {
            joints[ (size_t) v * MaxWeights + i ] = influences[ i ].joint;
            weights[ (size_t) v * MaxWeights + i ] = kept > 0.0f ? influences[ i ].weight / kept : 0.0f;
        }
    }
    subMesh.joints.swap( joints );
    subMesh.weights.swap( weights );
    subMesh.influencesPerVertex = MaxWeights;
}

/*-------------------------------------------------------------------------*/

CMeshCooker::CMeshCooker( const ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report )
    : m_scene( scene )
    , m_report( report )
    , m_mesh()
    , m_madeMaterials()
    , m_name()
    , m_reduction()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMeshCooker::~CMeshCooker()
{GUCEF_TRACE;

    Destroy();
}

/*-------------------------------------------------------------------------*/

Ogre::MeshPtr&
CMeshCooker::GetMesh( void )
{GUCEF_TRACE;

    return m_mesh;
}

/*-------------------------------------------------------------------------*/

void
CMeshCooker::Destroy( void )
{GUCEF_TRACE;

    if ( m_mesh && GUCEF_NULL != Ogre::MeshManager::getSingletonPtr() )
        Ogre::MeshManager::getSingleton().remove( m_mesh );
    m_mesh.reset();
    if ( GUCEF_NULL != Ogre::MaterialManager::getSingletonPtr() )
        for ( size_t i=0; i<m_madeMaterials.size(); ++i )
            Ogre::MaterialManager::getSingleton().remove( m_madeMaterials[ i ] );
    m_madeMaterials.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMeshCooker::GetHiddenFaceMaterialName( const CORE::CString& baseMaterialName, const ASSETCONVERTER::CAssetScene& scene, UInt32 hiddenBy )
{GUCEF_TRACE;

    return baseMaterialName + "/hiddenBy=" + COutputNames::Sanitize( ASSETCONVERTER::CAssetHiddenFaceSet::NamesOf( scene.hiddenFaceSets, hiddenBy, "+" ), "set" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMeshCooker::GetHiddenFaceSubMeshName( const ASSETCONVERTER::CAssetScene& scene, UInt32 hiddenBy, UInt32 index )
{GUCEF_TRACE;

    return "hiddenBy=" + COutputNames::Sanitize( ASSETCONVERTER::CAssetHiddenFaceSet::NamesOf( scene.hiddenFaceSets, hiddenBy, "+" ), "set" ) + "#" + CORE::UInt32ToString( index );
}

/*-------------------------------------------------------------------------*/

UInt32
CMeshCooker::MergeSubMeshesByMaterial( const ASSETCONVERTER::CAssetMesh& mesh, ASSETCONVERTER::CAssetMesh& merged )
{GUCEF_TRACE;

    merged = mesh;
    merged.subMeshes.clear();
    UInt32 mergedAway = 0;
    for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
    {
        const ASSETCONVERTER::CAssetSubMesh& source = mesh.subMeshes[ s ];
        ASSETCONVERTER::CAssetSubMesh* target = GUCEF_NULL;
        for ( size_t m=0; m<merged.subMeshes.size() && GUCEF_NULL == target; ++m )
            if ( merged.subMeshes[ m ].materialIndex == source.materialIndex && merged.subMeshes[ m ].hiddenBy == source.hiddenBy && HaveSameStreams( merged.subMeshes[ m ], source ) )
                target = &merged.subMeshes[ m ];
        if ( GUCEF_NULL == target )
        {
            merged.subMeshes.push_back( source );
            continue;
        }

        const UInt32 offset = target->GetVertexCount();
        AppendAll( target->positions, source.positions );
        AppendAll( target->normals, source.normals );
        AppendAll( target->tangents, source.tangents );
        for ( UInt32 u=0; u<ASSETCONVERTER::CAssetSubMesh::MaxUvSets; ++u )
            AppendAll( target->uvSets[ u ], source.uvSets[ u ] );
        AppendAll( target->colours, source.colours );
        AppendAll( target->masks, source.masks );
        AppendAll( target->joints, source.joints );
        AppendAll( target->weights, source.weights );
        for ( size_t i=0; i<source.indices.size(); ++i )
            target->indices.push_back( source.indices[ i ] + offset );
        for ( size_t t=0; t<source.morphTargets.size(); ++t )
        {
            AppendAll( target->morphTargets[ t ].positionDeltas, source.morphTargets[ t ].positionDeltas );
            AppendAll( target->morphTargets[ t ].normalDeltas, source.morphTargets[ t ].normalDeltas );
            AppendAll( target->morphTargets[ t ].tangentDeltas, source.morphTargets[ t ].tangentDeltas );
        }
        ++mergedAway;
    }
    return mergedAway;
}

/*-------------------------------------------------------------------------*/

bool
CMeshCooker::GenerateTangents( ASSETCONVERTER::CAssetSubMesh& subMesh, UInt32 uvSet )
{GUCEF_TRACE;

    const UInt32 vertexCount = subMesh.GetVertexCount();
    if ( !subMesh.HasNormals() || uvSet >= ASSETCONVERTER::CAssetSubMesh::MaxUvSets || subMesh.uvSets[ uvSet ].size() < (size_t) vertexCount * 2 )
        return false;
    const ASSETCONVERTER::CAssetSubMesh::TFloatVector& p = subMesh.positions;
    const ASSETCONVERTER::CAssetSubMesh::TFloatVector& uv = subMesh.uvSets[ uvSet ];
    GUCEF::vector< Ogre::Vector3 > tangents( vertexCount, Ogre::Vector3::ZERO );
    GUCEF::vector< Ogre::Vector3 > bitangents( vertexCount, Ogre::Vector3::ZERO );
    for ( size_t i=0; i+2<subMesh.indices.size(); i+=3 )
    {
        const UInt32 a = subMesh.indices[ i ], b = subMesh.indices[ i+1 ], c = subMesh.indices[ i+2 ];
        if ( a >= vertexCount || b >= vertexCount || c >= vertexCount )
            continue;
        const Ogre::Vector3 pa( p[ a*3 ], p[ a*3+1 ], p[ a*3+2 ] );
        const Ogre::Vector3 e1 = Ogre::Vector3( p[ b*3 ], p[ b*3+1 ], p[ b*3+2 ] ) - pa;
        const Ogre::Vector3 e2 = Ogre::Vector3( p[ c*3 ], p[ c*3+1 ], p[ c*3+2 ] ) - pa;
        const Ogre::Real du1 = uv[ b*2 ] - uv[ a*2 ], dv1 = uv[ b*2+1 ] - uv[ a*2+1 ];
        const Ogre::Real du2 = uv[ c*2 ] - uv[ a*2 ], dv2 = uv[ c*2+1 ] - uv[ a*2+1 ];
        const Ogre::Real r = du1 * dv2 - du2 * dv1;
        if ( fabs( r ) < 1e-12 )
            continue;
        const Ogre::Vector3 t = ( e1 * dv2 - e2 * dv1 ) / r;
        const Ogre::Vector3 bt = ( e2 * du1 - e1 * du2 ) / r;
        tangents[ a ] += t; tangents[ b ] += t; tangents[ c ] += t;
        bitangents[ a ] += bt; bitangents[ b ] += bt; bitangents[ c ] += bt;
    }
    subMesh.tangents.clear();
    subMesh.tangents.reserve( (size_t) vertexCount * 4 );
    for ( UInt32 v=0; v<vertexCount; ++v )
    {
        const Ogre::Vector3 n( subMesh.normals[ v*3 ], subMesh.normals[ v*3+1 ], subMesh.normals[ v*3+2 ] );
        Ogre::Vector3 t = tangents[ v ] - n * n.dotProduct( tangents[ v ] );
        if ( t.squaredLength() < 1e-12 )
            t = n.perpendicular();
        t.normalise();
        const Ogre::Real w = n.crossProduct( t ).dotProduct( bitangents[ v ] ) < 0 ? -1.0f : 1.0f;
        subMesh.tangents.push_back( (Float32) t.x );
        subMesh.tangents.push_back( (Float32) t.y );
        subMesh.tangents.push_back( (Float32) t.z );
        subMesh.tangents.push_back( (Float32) w );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMeshCooker::UseMaterial( Ogre::SubMesh* subMesh, const CORE::CString& materialName )
{GUCEF_TRACE;

    Ogre::MaterialManager& materials = Ogre::MaterialManager::getSingleton();
    const Ogre::String name = materialName.STL_String();
    Ogre::MaterialPtr material = materials.getByName( name, COgreCookEnvironment::ResourceGroupName.STL_String() );
    if ( !material )
    {
        material = materials.create( name, COgreCookEnvironment::ResourceGroupName.STL_String() );
        m_madeMaterials.push_back( material );
    }
    subMesh->setMaterial( material );
}

/*-------------------------------------------------------------------------*/

bool
CMeshCooker::BuildSubMesh( const ASSETCONVERTER::CAssetSubMesh& sourceSubMesh, UInt32 index, const CORE::CString& materialName, bool skinned, Ogre::AxisAlignedBox& bounds )
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetSubMesh source( sourceSubMesh );
    const UInt32 vertexCount = source.GetVertexCount();
    if ( 0 == vertexCount || source.indices.empty() || 0 != source.indices.size() % 3 )
    {
        m_report.AddSkipped( SourceName, m_name, "submesh " + CORE::UInt32ToString( index ) + " has no triangles" );
        return false;
    }
    for ( size_t i=0; i<source.indices.size(); ++i )
    {
        if ( source.indices[ i ] >= vertexCount )
        {
            m_report.AddError( SourceName, m_name, "submesh " + CORE::UInt32ToString( index ) + " has an index beyond its vertices" );
            return false;
        }
    }

    // tangents where a normal map needs them
    if ( source.materialIndex >= 0 && (size_t) source.materialIndex < m_scene.materials.size() && !source.HasTangents() )
    {
        const ASSETCONVERTER::CAssetMaterial& material = m_scene.materials[ source.materialIndex ];
        if ( material.normalTexture.IsUsed() && !material.unlit )
        {
            if ( GenerateTangents( source, material.normalTexture.texCoord ) )
                m_report.AddApproximation( SourceName, m_name, "tangents of submesh " + CORE::UInt32ToString( index ) + " generated per triangle for the normal map (not MikkTSpace)" );
            else
                m_report.AddWarning( SourceName, m_name, "submesh " + CORE::UInt32ToString( index ) + " has a normal map but no normals or UVs to make tangents from" );
        }
    }

    Ogre::SubMesh* subMesh = m_mesh->createSubMesh();
    subMesh->useSharedVertices = false;
    subMesh->operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    UseMaterial( subMesh, materialName );

    // the vertex streams, interleaved in one buffer, then laid out as Ogre wants them for the animation of the mesh
    Ogre::VertexData* data = OGRE_NEW Ogre::VertexData();
    subMesh->vertexData = data;
    data->vertexCount = vertexCount;
    Ogre::VertexDeclaration* declaration = data->vertexDeclaration;
    size_t offset = 0;
    declaration->addElement( 0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION );
    offset += Ogre::VertexElement::getTypeSize( Ogre::VET_FLOAT3 );
    const bool normals = source.HasNormals();
    if ( normals )
    {
        declaration->addElement( 0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL );
        offset += Ogre::VertexElement::getTypeSize( Ogre::VET_FLOAT3 );
    }
    const bool tangents = source.tangents.size() >= (size_t) vertexCount * 4;
    if ( tangents )
    {
        declaration->addElement( 0, offset, Ogre::VET_FLOAT4, Ogre::VES_TANGENT );
        offset += Ogre::VertexElement::getTypeSize( Ogre::VET_FLOAT4 );
    }
    const UInt32 colourComponents = source.colours.empty() ? 0 : (UInt32) ( source.colours.size() / vertexCount );
    const bool colours = colourComponents >= 3;
    if ( colours )
    {
        declaration->addElement( 0, offset, Ogre::VET_UBYTE4_NORM, Ogre::VES_DIFFUSE );
        offset += Ogre::VertexElement::getTypeSize( Ogre::VET_UBYTE4_NORM );
    }
    // the masks of the vertices (data of the materials of the source, not colours) as the second colour of Ogre: nothing reads it
    // unless a pass asks for it, so the masks do not tint the model
    const bool masks = source.masks.size() == (size_t) vertexCount * 4;
    if ( masks )
    {
        declaration->addElement( 0, offset, Ogre::VET_UBYTE4_NORM, Ogre::VES_SPECULAR );
        offset += Ogre::VertexElement::getTypeSize( Ogre::VET_UBYTE4_NORM );
    }
    const UInt32 uvSets = source.GetUvSetCount();
    for ( UInt32 u=0; u<uvSets; ++u )
    {
        declaration->addElement( 0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, (unsigned short) u );
        offset += Ogre::VertexElement::getTypeSize( Ogre::VET_FLOAT2 );
    }
    const size_t vertexSize = offset;

    GUCEF::vector< UInt8 > bytes( vertexSize * vertexCount, 0 );
    for ( UInt32 v=0; v<vertexCount; ++v )
    {
        UInt8* vertex = &bytes[ (size_t) v * vertexSize ];
        Float32* f = reinterpret_cast< Float32* >( vertex );
        size_t at = 0;
        f[ at++ ] = source.positions[ v*3 ]; f[ at++ ] = source.positions[ v*3+1 ]; f[ at++ ] = source.positions[ v*3+2 ];
        bounds.merge( Ogre::Vector3( source.positions[ v*3 ], source.positions[ v*3+1 ], source.positions[ v*3+2 ] ) );
        if ( normals )
        {
            f[ at++ ] = source.normals[ v*3 ]; f[ at++ ] = source.normals[ v*3+1 ]; f[ at++ ] = source.normals[ v*3+2 ];
        }
        if ( tangents )
        {
            for ( UInt32 c=0; c<4; ++c )
                f[ at++ ] = source.tangents[ v*4+c ];
        }
        if ( colours )
        {
            UInt8* colour = reinterpret_cast< UInt8* >( f + at );
            for ( UInt32 c=0; c<4; ++c )
            {
                Float32 value = c < colourComponents ? source.colours[ (size_t) v * colourComponents + c ] : 1.0f;
                value = value < 0.0f ? 0.0f : ( value > 1.0f ? 1.0f : value );
                colour[ c ] = (UInt8) ( value * 255.0f + 0.5f );
            }
            ++at;
        }
        if ( masks )
        {
            UInt8* mask = reinterpret_cast< UInt8* >( f + at );
            for ( UInt32 c=0; c<4; ++c )
            {
                Float32 value = source.masks[ (size_t) v * 4 + c ];
                value = value < 0.0f ? 0.0f : ( value > 1.0f ? 1.0f : value );
                mask[ c ] = (UInt8) ( value * 255.0f + 0.5f );
            }
            ++at;
        }
        for ( UInt32 u=0; u<uvSets; ++u )
        {
            f[ at++ ] = source.uvSets[ u ][ v*2 ];
            f[ at++ ] = source.uvSets[ u ][ v*2+1 ];
        }
    }
    Ogre::HardwareBufferManager& buffers = Ogre::HardwareBufferManager::getSingleton();
    Ogre::HardwareVertexBufferSharedPtr vertexBuffer = buffers.createVertexBuffer( vertexSize, vertexCount, Ogre::HBU_GPU_ONLY, true );
    vertexBuffer->writeData( 0, bytes.size(), &bytes[ 0 ], true );
    data->vertexBufferBinding->setBinding( 0, vertexBuffer );

    bool hasPoseNormals = false;
    for ( size_t t=0; t<source.morphTargets.size(); ++t )
        hasPoseNormals = hasPoseNormals || source.morphTargets[ t ].normalDeltas.size() >= (size_t) vertexCount * 3;
    Ogre::VertexDeclaration* organised = declaration->getAutoOrganisedDeclaration( skinned, !source.morphTargets.empty(), hasPoseNormals && normals );
    data->reorganiseBuffers( organised );

    // the indices
    const bool wide = vertexCount > 65535;
    Ogre::IndexData* indexData = subMesh->indexData;
    indexData->indexStart = 0;
    indexData->indexCount = source.indices.size();
    indexData->indexBuffer = buffers.createIndexBuffer( wide ? Ogre::HardwareIndexBuffer::IT_32BIT : Ogre::HardwareIndexBuffer::IT_16BIT, source.indices.size(), Ogre::HBU_GPU_ONLY, true );
    if ( wide )
    {
        indexData->indexBuffer->writeData( 0, source.indices.size() * sizeof( UInt32 ), &source.indices[ 0 ], true );
    }
    else
    {
        GUCEF::vector< UInt16 > narrow( source.indices.begin(), source.indices.end() );
        indexData->indexBuffer->writeData( 0, narrow.size() * sizeof( UInt16 ), &narrow[ 0 ], true );
    }

    // the weights: the strongest four, renormalised
    if ( skinned && source.IsSkinned() && source.influencesPerVertex > 0 )
    {
        const UInt32 n = source.influencesPerVertex;
        GUCEF::vector< SInfluence > influences;
        for ( UInt32 v=0; v<vertexCount; ++v )
        {
            influences.clear();
            for ( UInt32 i=0; i<n; ++i )
            {
                const size_t at = (size_t) v * n + i;
                if ( at >= source.weights.size() || at >= source.joints.size() || source.weights[ at ] <= 0.0f )
                    continue;
                SInfluence influence;
                influence.joint = source.joints[ at ];
                influence.weight = source.weights[ at ];
                influences.push_back( influence );
            }
            std::sort( influences.begin(), influences.end(), IsStronger );
            if ( influences.size() > MaxWeights )
                influences.resize( MaxWeights );    // the submesh is reduced before it gets here, this keeps the limit whatever comes in
            Float32 total = 0.0f;
            for ( size_t i=0; i<influences.size(); ++i )
                total += influences[ i ].weight;
            for ( size_t i=0; i<influences.size(); ++i )
            {
                Ogre::VertexBoneAssignment assignment;
                assignment.vertexIndex = v;
                assignment.boneIndex = (unsigned short) influences[ i ].joint;
                assignment.weight = total > 0.0f ? influences[ i ].weight / total : 0.0f;
                subMesh->addBoneAssignment( assignment );
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMeshCooker::BuildPoses( const ASSETCONVERTER::CAssetMesh& mesh, const TUInt32Vector& animatedNodes )
{GUCEF_TRACE;

    // a pose per target and submesh; the index of each in the mesh
    GUCEF::vector< GUCEF::vector< Int32 > > poseIndex( mesh.subMeshes.size() );
    UInt32 targetCount = 0;
    bool defaultWeights = false;
    Ogre::AxisAlignedBox bounds = m_mesh->getBounds();
    for ( size_t s=0; s<mesh.subMeshes.size() && s<m_mesh->getNumSubMeshes(); ++s )
    {
        const ASSETCONVERTER::CAssetSubMesh& subMesh = mesh.subMeshes[ s ];
        const UInt32 vertexCount = subMesh.GetVertexCount();
        if ( subMesh.morphTargets.size() > targetCount )
            targetCount = (UInt32) subMesh.morphTargets.size();
        for ( size_t t=0; t<subMesh.morphTargets.size(); ++t )
        {
            const ASSETCONVERTER::CAssetMorphTarget& target = subMesh.morphTargets[ t ];
            defaultWeights = defaultWeights || 0.0f != target.defaultWeight;
            const CORE::CString name = target.name.IsNULLOrEmpty() ? "target" + CORE::UInt32ToString( (UInt32) t ) : target.name;
            Ogre::Pose* pose = m_mesh->createPose( (unsigned short) ( s + 1 ), name.STL_String() );
            poseIndex[ s ].push_back( (Int32) m_mesh->getPoseCount() - 1 );
            const bool withNormals = target.normalDeltas.size() >= (size_t) vertexCount * 3 && subMesh.HasNormals();
            for ( UInt32 v=0; v<vertexCount; ++v )
            {
                Ogre::Vector3f offset( 0, 0, 0 );
                if ( target.positionDeltas.size() >= (size_t) ( v + 1 ) * 3 )
                    offset = Ogre::Vector3f( target.positionDeltas[ v*3 ], target.positionDeltas[ v*3+1 ], target.positionDeltas[ v*3+2 ] );
                if ( withNormals )
                {
                    const Ogre::Vector3f normal( target.normalDeltas[ v*3 ], target.normalDeltas[ v*3+1 ], target.normalDeltas[ v*3+2 ] );
                    pose->addVertex( v, offset, normal );
                }
                else
                if ( offset != Ogre::Vector3f( 0, 0, 0 ) )
                {
                    pose->addVertex( v, offset );
                }
                if ( offset != Ogre::Vector3f( 0, 0, 0 ) )
                    bounds.merge( Ogre::Vector3( subMesh.positions[ v*3 ] + offset.x, subMesh.positions[ v*3+1 ] + offset.y, subMesh.positions[ v*3+2 ] + offset.z ) );
            }
        }
    }
    if ( 0 == targetCount )
        return;
    m_mesh->_setBounds( bounds, false );
    m_mesh->_setBoundingSphereRadius( std::max( bounds.getMaximum().length(), bounds.getMinimum().length() ) );
    if ( defaultWeights )
        m_report.AddApproximation( SourceName, m_name, "morph targets with a default weight other than 0: Ogre poses have no base weight, the mesh rests at weight 0" );

    // the clips that animate the weights of a node of the mesh
    CORE::CString::StringSet names;
    for ( size_t a=0; a<m_scene.animations.size(); ++a )
    {
        const ASSETCONVERTER::CAssetAnimation& clip = m_scene.animations[ a ];
        const ASSETCONVERTER::CAssetAnimationChannel* weights = GUCEF_NULL;
        for ( size_t c=0; c<clip.channels.size() && GUCEF_NULL == weights; ++c )
        {
            if ( ASSETCONVERTER::ANIMATIONPATH_WEIGHTS != clip.channels[ c ].path || clip.channels[ c ].times.empty() )
                continue;
            if ( std::find( animatedNodes.begin(), animatedNodes.end(), clip.channels[ c ].targetNode ) != animatedNodes.end() )
                weights = &clip.channels[ c ];
        }
        if ( GUCEF_NULL == weights )
            continue;

        Float32 length = clip.GetDuration();
        if ( length < 0.0f )
            length = 0.0f;
        const CORE::CString name = COutputNames::MakeUnique( clip.name.IsNULLOrEmpty() ? "clip" + CORE::UInt32ToString( (UInt32) a ) : clip.name, CORE::CString::Empty, names );
        Ogre::Animation* animation = m_mesh->createAnimation( name.STL_String(), length );
        animation->setInterpolationMode( Ogre::Animation::IM_LINEAR );
        GUCEF::vector< Float32 > values( targetCount, 0.0f );
        for ( size_t s=0; s<poseIndex.size(); ++s )
        {
            if ( poseIndex[ s ].empty() )
                continue;
            Ogre::VertexAnimationTrack* track = animation->createVertexTrack( (unsigned short) ( s + 1 ), Ogre::VAT_POSE );
            for ( size_t k=0; k<weights->times.size(); ++k )
            {
                CNodeAnimationSampler::ReadKey( *weights, (UInt32) k, targetCount, &values[ 0 ] );
                const Float32 time = weights->times[ k ] < 0.0f ? 0.0f : ( weights->times[ k ] > length ? length : weights->times[ k ] );
                Ogre::VertexPoseKeyFrame* key = track->createVertexPoseKeyFrame( time );
                for ( size_t t=0; t<poseIndex[ s ].size(); ++t )
                    key->addPoseReference( (unsigned short) poseIndex[ s ][ t ], values[ t ] );
            }
        }
        if ( ASSETCONVERTER::ANIMATIONINTERPOLATION_LINEAR != weights->interpolation && weights->times.size() > 1 )
            m_report.AddApproximation( SourceName, clip.name, "step or cubic spline keys of the morph weights are played back linearly between their values" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMeshCooker::Build( UInt32 meshIndex                                ,
                    const CORE::CString& resourceName               ,
                    const CORE::CString::StringVector& materialNames,
                    const CORE::CString& defaultMaterialName        ,
                    Int32 skinIndex                                 ,
                    const Ogre::SkeletonPtr& skeleton               ,
                    const TUInt32Vector& animatedNodes              )
{GUCEF_TRACE;

    Destroy();
    m_reduction = SInfluenceReduction();
    if ( meshIndex >= m_scene.meshes.size() )
        return false;
    m_name = m_scene.meshes[ meshIndex ].name;

    ASSETCONVERTER::CAssetMesh mesh;
    const UInt32 mergedAway = MergeSubMeshesByMaterial( m_scene.meshes[ meshIndex ], mesh );
    if ( mergedAway > 0 )
        m_report.AddInfo( SourceName, m_name, CORE::UInt32ToString( mergedAway ) + " submesh(es) merged into others of the same material" );

    const bool skinned = skinIndex >= 0 && (bool) skeleton;
    if ( skinned )
    {
        // the influences are reduced to what Ogre keeps first: the bones that count are those that remain. Then a submesh that
        // uses more bones than its blend indices can address is split by bone subsets
        for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
            ReduceInfluences( mesh.subMeshes[ s ], m_reduction );
        ASSETCONVERTER::CAssetMesh split;
        UInt32 extraSubMeshes = 0;
        const UInt32 splitCount = CBoneSubsetSplitter::Split( mesh, MaxBonesPerSubMesh, split, extraSubMeshes );
        if ( splitCount > 0 )
        {
            mesh = split;
            m_report.AddInfo( SourceName, m_name, CORE::UInt32ToString( splitCount ) + " submesh(es) use more than " + CORE::UInt32ToString( MaxBonesPerSubMesh ) +
                              " bones: split by bone subsets into " + CORE::UInt32ToString( extraSubMeshes + splitCount ) + " submeshes (the skeleton has " +
                              CORE::UInt32ToString( skeleton->getNumBones() ) + " bones)" );
        }
    }
    try
    {
        m_mesh = Ogre::MeshManager::getSingleton().createManual( resourceName.STL_String(), COgreCookEnvironment::ResourceGroupName.STL_String() );
        Ogre::AxisAlignedBox bounds;
        ASSETCONVERTER::CAssetMesh built;
        built.name = mesh.name;
        for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
        {
            const Int32 material = mesh.subMeshes[ s ].materialIndex;
            const UInt32 hiddenBy = mesh.subMeshes[ s ].hiddenBy;
            CORE::CString materialName = material >= 0 && (size_t) material < materialNames.size() ? materialNames[ material ] : defaultMaterialName;
            // the triangles that hidden face sets hide: a material variant and a submesh name that say so (the mesh keeps them)
            if ( 0 != hiddenBy )
                materialName = GetHiddenFaceMaterialName( materialName, m_scene, hiddenBy );
            if ( BuildSubMesh( mesh.subMeshes[ s ], (UInt32) s, materialName, skinned, bounds ) )
            {
                built.subMeshes.push_back( mesh.subMeshes[ s ] );
                if ( 0 != hiddenBy )
                    m_mesh->nameSubMesh( GetHiddenFaceSubMeshName( m_scene, hiddenBy, (UInt32) ( m_mesh->getNumSubMeshes() - 1 ) ).STL_String(), (unsigned short) ( m_mesh->getNumSubMeshes() - 1 ) );
            }
        }
        if ( 0 == m_mesh->getNumSubMeshes() )
        {
            m_report.AddError( SourceName, m_name, "the mesh has no submesh with triangles" );
            Destroy();
            return false;
        }
        if ( bounds.isNull() )
            bounds.setExtents( Ogre::Vector3::ZERO, Ogre::Vector3::ZERO );
        m_mesh->_setBounds( bounds, false );
        m_mesh->_setBoundingSphereRadius( std::max( bounds.getMaximum().length(), bounds.getMinimum().length() ) );

        if ( skinned )
            m_mesh->_notifySkeleton( skeleton );
        BuildPoses( built, animatedNodes );
    }
    catch ( const Ogre::Exception& e )
    {
        m_report.AddError( SourceName, m_name, "the mesh cannot be built: " + CORE::CString( e.getDescription() ) );
        Destroy();
        return false;
    }

    if ( m_reduction.reducedVertices > 0 )
        m_report.AddApproximation( SourceName, m_name, CORE::UInt32ToString( m_reduction.reducedVertices ) + " vertices had up to " + CORE::UInt32ToString( m_reduction.mostInfluences ) +
                                   " influences; reduced to the strongest 4 and renormalised for hardware skinning (the dropped influences had " +
                                   ToPercent( m_reduction.lostWeightSum / (Float64) m_reduction.reducedVertices ) + " percent of the weight of such a vertex on average, " +
                                   ToPercent( m_reduction.lostWeightMax ) + " percent at most)" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMeshCooker::Save( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    if ( !m_mesh )
        return false;
    try
    {
        CBufferDataStream* buffer = OGRE_NEW CBufferDataStream();
        Ogre::DataStreamPtr stream( buffer );
        Ogre::MeshSerializer serializer;
        serializer.exportMesh( m_mesh.get(), stream, Ogre::MESH_VERSION_LATEST, Ogre::Serializer::ENDIAN_LITTLE );
        if ( !buffer->StoreInVfs( vfsPath ) )
        {
            m_report.AddError( SourceName, vfsPath, "the mesh cannot be written" );
            return false;
        }
    }
    catch ( const Ogre::Exception& e )
    {
        m_report.AddError( SourceName, vfsPath, "the mesh cannot be serialised: " + CORE::CString( e.getDescription() ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
