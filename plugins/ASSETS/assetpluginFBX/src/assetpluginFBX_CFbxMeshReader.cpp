/*
 *  assetpluginFBX: the meshes, skins and blend shapes of an FBX scene as meshes, skins and morph targets of the converter model
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
#include <algorithm>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "assetpluginFBX_CFbxMeshReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace FBX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const char* const SourceName = "fbx";

CORE::CString
ToString( const ufbx_string& text )
{
    return GUCEF_NULL != text.data && text.length > 0 ? CORE::CString( text.data, (UInt32) text.length ) : CORE::CString::Empty;
}

UInt32 AttributeIndex( const ufbx_vertex_vec3& attribute, UInt32 corner ) { return attribute.exists ? attribute.indices.data[ corner ] : 0; }
UInt32 AttributeIndex( const ufbx_vertex_vec2& attribute, UInt32 corner ) { return attribute.exists ? attribute.indices.data[ corner ] : 0; }
UInt32 AttributeIndex( const ufbx_vertex_vec4& attribute, UInt32 corner ) { return attribute.exists ? attribute.indices.data[ corner ] : 0; }

/** An influence of a vertex: the joint (an index in the joints of the skin) and its weight */
struct SInfluence
{
    UInt32 joint;
    Float32 weight;
};

bool
InfluenceOrder( const SInfluence& a, const SInfluence& b )
{
    if ( a.weight != b.weight )
        return a.weight > b.weight;
    return a.joint < b.joint;
}

MATH::TFloat32Matrix4D
ToMatrix( const ufbx_matrix& m )
{
    const Float32 values[ 16 ] =
    {
        (Float32) m.m00, (Float32) m.m10, (Float32) m.m20, 0.0f,
        (Float32) m.m01, (Float32) m.m11, (Float32) m.m21, 0.0f,
        (Float32) m.m02, (Float32) m.m12, (Float32) m.m22, 0.0f,
        (Float32) m.m03, (Float32) m.m13, (Float32) m.m23, 1.0f
    };
    return MATH::TFloat32Matrix4D::FromColumnMajor( values );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CFbxMeshReader::SCorner::operator<( const SCorner& other ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<8; ++i )
        if ( index[ i ] != other.index[ i ] )
            return index[ i ] < other.index[ i ];
    return false;
}

/*-------------------------------------------------------------------------*/

CFbxMeshReader::CFbxMeshReader( const ufbx_scene& fbx                       ,
                                const TIndexVector& nodeMap                 ,
                                ASSETCONVERTER::CAssetScene& scene          ,
                                ASSETCONVERTER::CConversionReport& report   )
    : m_fbx( fbx )
    , m_nodeMap( nodeMap )
    , m_scene( scene )
    , m_report( report )
    , m_meshes()
    , m_skins()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFbxMeshReader::~CFbxMeshReader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CFbxMeshReader::GetBlendChannels( const ufbx_mesh& mesh, GUCEF::vector< const ufbx_blend_channel* >& channels )
{GUCEF_TRACE;

    channels.clear();
    for ( size_t d=0; d<mesh.blend_deformers.count; ++d )
        for ( size_t c=0; c<mesh.blend_deformers.data[ d ]->channels.count; ++c )
            if ( mesh.blend_deformers.data[ d ]->channels.data[ c ]->keyframes.count > 0 )
                channels.push_back( mesh.blend_deformers.data[ d ]->channels.data[ c ] );
}

/*-------------------------------------------------------------------------*/

Int32
CFbxMeshReader::ReadSkin( const ufbx_skin_deformer& deformer, GUCEF::vector< Int32 >& jointOfCluster )
{GUCEF_TRACE;

    jointOfCluster.assign( deformer.clusters.count, -1 );
    ASSETCONVERTER::CAssetSkin skin;
    skin.name = ToString( deformer.name );
    for ( size_t c=0; c<deformer.clusters.count; ++c )
    {
        const ufbx_skin_cluster& cluster = *deformer.clusters.data[ c ];
        if ( GUCEF_NULL == cluster.bone_node || m_nodeMap[ cluster.bone_node->typed_id ] < 0 )
            continue;
        jointOfCluster[ c ] = (Int32) skin.joints.size();
        skin.joints.push_back( (UInt32) m_nodeMap[ cluster.bone_node->typed_id ] );
        skin.inverseBindMatrices.push_back( ToMatrix( cluster.geometry_to_bone ) );
    }
    if ( skin.joints.empty() )
        return -1;

    TSkinMap::const_iterator known = m_skins.find( deformer.typed_id );
    if ( known != m_skins.end() )
        return (*known).second;

    // the root of the skeleton: the one joint whose parent is not a joint
    GUCEF::set< UInt32 > joints( skin.joints.begin(), skin.joints.end() );
    Int32 root = -1;
    UInt32 roots = 0;
    for ( size_t j=0; j<skin.joints.size(); ++j )
    {
        const Int32 parent = m_scene.nodes[ skin.joints[ j ] ].parent;
        if ( parent < 0 || joints.find( (UInt32) parent ) == joints.end() )
        {
            root = (Int32) skin.joints[ j ];
            ++roots;
        }
    }
    skin.skeletonRoot = 1 == roots ? root : -1;
    m_scene.skins.push_back( skin );
    m_skins[ deformer.typed_id ] = (Int32) m_scene.skins.size() - 1;
    return (Int32) m_scene.skins.size() - 1;
}

/*-------------------------------------------------------------------------*/

bool
CFbxMeshReader::BuildSubMesh( const ufbx_mesh& mesh, const SPart& part, const ufbx_skin_deformer* deformer, const GUCEF::vector< Int32 >& jointOfCluster,
                              const TDenseVector& positionDeltas, const TDenseVector& normalDeltas, const GUCEF::vector< const ufbx_blend_channel* >& channels,
                              Int32 materialIndex, ASSETCONVERTER::CAssetSubMesh& subMesh )
{GUCEF_TRACE;

    const bool hasNormals = mesh.vertex_normal.exists;
    const ufbx_uv_set* tangentSet = mesh.uv_sets.count > 0 && mesh.uv_sets.data[ 0 ].vertex_tangent.exists && mesh.uv_sets.data[ 0 ].vertex_bitangent.exists && hasNormals ? &mesh.uv_sets.data[ 0 ] : GUCEF_NULL;
    const UInt32 uvSets = (UInt32) std::min< size_t >( mesh.uv_sets.count, ASSETCONVERTER::CAssetSubMesh::MaxUvSets );
    const ufbx_color_set* colourSet = mesh.color_sets.count > 0 && mesh.color_sets.data[ 0 ].vertex_color.exists ? &mesh.color_sets.data[ 0 ] : GUCEF_NULL;

    GUCEF::map< SCorner, UInt32 > unique;
    GUCEF::vector< UInt32 > sourceVertex;
    GUCEF::vector< UInt32 > triangle;
    triangle.resize( std::max< size_t >( mesh.max_face_triangles, 1 ) * 3 );

    for ( size_t f=0; f<part.faces.size(); ++f )
    {
        const ufbx_face face = mesh.faces.data[ part.faces[ f ] ];
        const UInt32 triangles = ufbx_triangulate_face( &triangle[ 0 ], triangle.size(), &mesh, face );
        for ( UInt32 t=0; t<triangles * 3; ++t )
        {
            const UInt32 corner = triangle[ t ];
            SCorner key;
            key.index[ 0 ] = mesh.vertex_indices.data[ corner ];
            key.index[ 1 ] = AttributeIndex( mesh.vertex_normal, corner );
            key.index[ 2 ] = GUCEF_NULL != tangentSet ? AttributeIndex( tangentSet->vertex_tangent, corner ) : 0;
            for ( UInt32 uv=0; uv<ASSETCONVERTER::CAssetSubMesh::MaxUvSets; ++uv )
                key.index[ 3 + uv ] = uv < uvSets ? AttributeIndex( mesh.uv_sets.data[ uv ].vertex_uv, corner ) : 0;
            // the colour index shares the last slot with nothing else: 3 + 4 = 7
            key.index[ 7 ] = GUCEF_NULL != colourSet ? AttributeIndex( colourSet->vertex_color, corner ) : 0;

            GUCEF::map< SCorner, UInt32 >::const_iterator found = unique.find( key );
            if ( found != unique.end() )
            {
                subMesh.indices.push_back( (*found).second );
                continue;
            }

            const UInt32 vertex = (UInt32) sourceVertex.size();
            unique[ key ] = vertex;
            sourceVertex.push_back( key.index[ 0 ] );
            subMesh.indices.push_back( vertex );

            const ufbx_vec3 position = mesh.vertices.data[ key.index[ 0 ] ];
            subMesh.positions.push_back( (Float32) position.x );
            subMesh.positions.push_back( (Float32) position.y );
            subMesh.positions.push_back( (Float32) position.z );
            ufbx_vec3 normal = { { { 0.0, 1.0, 0.0 } } };
            if ( hasNormals )
            {
                normal = mesh.vertex_normal.values.data[ key.index[ 1 ] ];
                subMesh.normals.push_back( (Float32) normal.x );
                subMesh.normals.push_back( (Float32) normal.y );
                subMesh.normals.push_back( (Float32) normal.z );
            }
            if ( GUCEF_NULL != tangentSet )
            {
                const ufbx_vec3 tangent = tangentSet->vertex_tangent.values.data[ key.index[ 2 ] ];
                const ufbx_vec3 bitangent = tangentSet->vertex_bitangent.values.data[ AttributeIndex( tangentSet->vertex_bitangent, corner ) ];
                const double cx = normal.y * tangent.z - normal.z * tangent.y;
                const double cy = normal.z * tangent.x - normal.x * tangent.z;
                const double cz = normal.x * tangent.y - normal.y * tangent.x;
                subMesh.tangents.push_back( (Float32) tangent.x );
                subMesh.tangents.push_back( (Float32) tangent.y );
                subMesh.tangents.push_back( (Float32) tangent.z );
                subMesh.tangents.push_back( cx * bitangent.x + cy * bitangent.y + cz * bitangent.z < 0.0 ? -1.0f : 1.0f );
            }
            for ( UInt32 uv=0; uv<uvSets; ++uv )
            {
                if ( !mesh.uv_sets.data[ uv ].vertex_uv.exists )
                    continue;
                const ufbx_vec2 value = mesh.uv_sets.data[ uv ].vertex_uv.values.data[ key.index[ 3 + uv ] ];
                subMesh.uvSets[ uv ].push_back( (Float32) value.x );
                subMesh.uvSets[ uv ].push_back( 1.0f - (Float32) value.y );
            }
            if ( GUCEF_NULL != colourSet )
            {
                const ufbx_vec4 colour = colourSet->vertex_color.values.data[ key.index[ 7 ] ];
                subMesh.colours.push_back( (Float32) colour.x );
                subMesh.colours.push_back( (Float32) colour.y );
                subMesh.colours.push_back( (Float32) colour.z );
                subMesh.colours.push_back( (Float32) colour.w );
            }
        }
    }
    if ( sourceVertex.empty() )
        return false;
    subMesh.materialIndex = materialIndex;

    // the influences: all of them, strongest first, normalised
    if ( GUCEF_NULL != deformer )
    {
        GUCEF::vector< GUCEF::vector< SInfluence > > influences;
        influences.resize( sourceVertex.size() );
        UInt32 most = 0;
        bool unbound = false;
        for ( size_t v=0; v<sourceVertex.size(); ++v )
        {
            if ( sourceVertex[ v ] < deformer->vertices.count )
            {
                const ufbx_skin_vertex& skinVertex = deformer->vertices.data[ sourceVertex[ v ] ];
                for ( UInt32 w=0; w<skinVertex.num_weights; ++w )
                {
                    const ufbx_skin_weight& weight = deformer->weights.data[ skinVertex.weight_begin + w ];
                    const Int32 joint = weight.cluster_index < jointOfCluster.size() ? jointOfCluster[ weight.cluster_index ] : -1;
                    if ( joint >= 0 && weight.weight > 0.0 )
                    {
                        SInfluence influence;
                        influence.joint = (UInt32) joint;
                        influence.weight = (Float32) weight.weight;
                        influences[ v ].push_back( influence );
                    }
                }
            }
            std::sort( influences[ v ].begin(), influences[ v ].end(), InfluenceOrder );
            if ( influences[ v ].empty() )
                unbound = true;
            if ( influences[ v ].size() > most )
                most = (UInt32) influences[ v ].size();
        }
        if ( most > 0 )
        {
            if ( unbound )
                m_report.AddApproximation( SourceName, ToString( mesh.name ), "vertices without a skin weight are bound to the first joint" );
            subMesh.influencesPerVertex = most;
            subMesh.joints.assign( sourceVertex.size() * most, 0 );
            subMesh.weights.assign( sourceVertex.size() * most, 0.0f );
            for ( size_t v=0; v<sourceVertex.size(); ++v )
            {
                Float32 sum = 0.0f;
                for ( size_t i=0; i<influences[ v ].size(); ++i )
                    sum += influences[ v ][ i ].weight;
                if ( influences[ v ].empty() )
                {
                    subMesh.weights[ v * most ] = 1.0f;
                    continue;
                }
                for ( size_t i=0; i<influences[ v ].size(); ++i )
                {
                    subMesh.joints[ v * most + i ] = influences[ v ][ i ].joint;
                    subMesh.weights[ v * most + i ] = influences[ v ][ i ].weight / sum;
                }
            }
        }
    }

    // the morph targets
    for ( size_t c=0; c<channels.size(); ++c )
    {
        ASSETCONVERTER::CAssetMorphTarget target;
        target.name = ToString( channels[ c ]->name );
        target.defaultWeight = (Float32) channels[ c ]->weight;
        target.positionDeltas.resize( sourceVertex.size() * 3, 0.0f );
        for ( size_t v=0; v<sourceVertex.size(); ++v )
            for ( UInt32 a=0; a<3; ++a )
                target.positionDeltas[ v * 3 + a ] = positionDeltas[ c ][ (size_t) sourceVertex[ v ] * 3 + a ];
        if ( !normalDeltas[ c ].empty() )
        {
            target.normalDeltas.resize( sourceVertex.size() * 3, 0.0f );
            for ( size_t v=0; v<sourceVertex.size(); ++v )
                for ( UInt32 a=0; a<3; ++a )
                    target.normalDeltas[ v * 3 + a ] = normalDeltas[ c ][ (size_t) sourceVertex[ v ] * 3 + a ];
        }
        subMesh.morphTargets.push_back( target );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFbxMeshReader::ReadNodeMesh( const ufbx_node& node, Int32& meshIndex, Int32& skinIndex )
{GUCEF_TRACE;

    meshIndex = -1;
    skinIndex = -1;
    const ufbx_mesh* mesh = node.mesh;
    if ( GUCEF_NULL == mesh )
        return false;
    const CORE::CString subject = ToString( mesh->name ).IsNULLOrEmpty() ? ToString( node.name ) : ToString( mesh->name );

    // the parts of the mesh with the same material
    GUCEF::vector< SPart > parts;
    if ( mesh->material_parts.count > 0 )
    {
        for ( size_t p=0; p<mesh->material_parts.count; ++p )
        {
            const ufbx_mesh_part& source = mesh->material_parts.data[ p ];
            if ( 0 == source.num_faces )
                continue;
            SPart part;
            part.slot = source.index;
            part.faces.assign( source.face_indices.data, source.face_indices.data + source.face_indices.count );
            parts.push_back( part );
        }
    }
    else
    if ( mesh->num_faces > 0 )
    {
        SPart part;
        part.slot = 0;
        part.faces.resize( mesh->num_faces );
        for ( size_t f=0; f<mesh->num_faces; ++f )
            part.faces[ f ] = (UInt32) f;
        parts.push_back( part );
    }

    // the materials of the parts: those of the node, else those of the mesh
    GUCEF::vector< Int32 > partMaterials;
    CORE::CString key = CORE::UInt32ToString( mesh->typed_id );
    for ( size_t p=0; p<parts.size(); ++p )
    {
        const ufbx_material* material = GUCEF_NULL;
        if ( parts[ p ].slot < node.materials.count )
            material = node.materials.data[ parts[ p ].slot ];
        else
        if ( parts[ p ].slot < mesh->materials.count )
            material = mesh->materials.data[ parts[ p ].slot ];
        const Int32 index = GUCEF_NULL != material && material->typed_id < m_scene.materials.size() ? (Int32) material->typed_id : -1;
        partMaterials.push_back( index );
        key += "|" + CORE::Int32ToString( index );
    }

    const ufbx_skin_deformer* deformer = mesh->skin_deformers.count > 0 ? mesh->skin_deformers.data[ 0 ] : GUCEF_NULL;
    if ( mesh->skin_deformers.count > 1 )
        m_report.AddApproximation( SourceName, subject, "the mesh has several skin deformers: the first is used" );
    GUCEF::vector< Int32 > jointOfCluster;
    if ( GUCEF_NULL != deformer )
        skinIndex = ReadSkin( *deformer, jointOfCluster );
    if ( skinIndex < 0 )
        deformer = GUCEF_NULL;

    TMeshMap::const_iterator known = m_meshes.find( key );
    if ( known != m_meshes.end() )
    {
        meshIndex = (*known).second;
        return true;
    }

    // the blend shapes, as the offsets of every vertex
    GUCEF::vector< const ufbx_blend_channel* > channels;
    GetBlendChannels( *mesh, channels );
    TDenseVector positionDeltas;
    TDenseVector normalDeltas;
    positionDeltas.resize( channels.size() );
    normalDeltas.resize( channels.size() );
    for ( size_t c=0; c<channels.size(); ++c )
    {
        const ufbx_blend_channel& channel = *channels[ c ];
        if ( channel.keyframes.count > 1 )
            m_report.AddApproximation( SourceName, subject, "blend channel " + ToString( channel.name ) + " has in-between shapes: the shape at the full weight is used" );
        const ufbx_blend_shape* shape = channel.keyframes.data[ channel.keyframes.count - 1 ].shape;
        positionDeltas[ c ].assign( mesh->num_vertices * 3, 0.0f );
        if ( shape->normal_offsets.count == shape->num_offsets && shape->num_offsets > 0 )
            normalDeltas[ c ].assign( mesh->num_vertices * 3, 0.0f );
        for ( size_t o=0; o<shape->num_offsets; ++o )
        {
            const size_t vertex = shape->offset_vertices.data[ o ];
            if ( vertex >= mesh->num_vertices )
                continue;
            positionDeltas[ c ][ vertex * 3 ] = (Float32) shape->position_offsets.data[ o ].x;
            positionDeltas[ c ][ vertex * 3 + 1 ] = (Float32) shape->position_offsets.data[ o ].y;
            positionDeltas[ c ][ vertex * 3 + 2 ] = (Float32) shape->position_offsets.data[ o ].z;
            if ( !normalDeltas[ c ].empty() )
            {
                normalDeltas[ c ][ vertex * 3 ] = (Float32) shape->normal_offsets.data[ o ].x;
                normalDeltas[ c ][ vertex * 3 + 1 ] = (Float32) shape->normal_offsets.data[ o ].y;
                normalDeltas[ c ][ vertex * 3 + 2 ] = (Float32) shape->normal_offsets.data[ o ].z;
            }
        }
    }

    ASSETCONVERTER::CAssetMesh result;
    result.name = subject;
    for ( size_t p=0; p<parts.size(); ++p )
    {
        ASSETCONVERTER::CAssetSubMesh subMesh;
        if ( BuildSubMesh( *mesh, parts[ p ], deformer, jointOfCluster, positionDeltas, normalDeltas, channels, partMaterials[ p ], subMesh ) )
            result.subMeshes.push_back( subMesh );
    }
    if ( result.subMeshes.empty() )
    {
        m_report.AddSkipped( SourceName, subject, "the mesh has no triangles" );
        skinIndex = -1;
        return false;
    }
    m_scene.meshes.push_back( result );
    meshIndex = (Int32) m_scene.meshes.size() - 1;
    m_meshes[ key ] = meshIndex;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FBX */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
