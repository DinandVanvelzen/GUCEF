/*
 *  assetpluginGLTF: builds the glTF text and binary buffer of a scene of the converter model
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFURI_H
#include "assetpluginGLTF_CGltfUri.h"
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFURI_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFURI_H ? */

#include "assetpluginGLTF_CGltfSceneWriter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GLTF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const char* const SourceName = "gltf";

static const char* const ExtLights = "KHR_lights_punctual";
static const char* const ExtTextureTransform = "KHR_texture_transform";
static const char* const ExtEmissiveStrength = "KHR_materials_emissive_strength";
static const char* const ExtUnlit = "KHR_materials_unlit";
static const char* const ExtLod = "MSFT_lod";
static const char* const ExtPhysicsShape = "OMI_physics_shape";
static const char* const ExtPhysicsBody = "OMI_physics_body";

bool
IsIdentity( const MATH::TFloat32Transform3D& transform )
{
    return 0.0f == transform.position.x && 0.0f == transform.position.y && 0.0f == transform.position.z &&
           0.0f == transform.rotation.x && 0.0f == transform.rotation.y && 0.0f == transform.rotation.z && 1.0f == transform.rotation.w &&
           1.0f == transform.scale.x && 1.0f == transform.scale.y && 1.0f == transform.scale.z;
}

UInt32
GetWrapNumber( IMAGE::TWrapMode mode )
{
    switch ( mode )
    {
        case IMAGE::WRAPMODE_CLAMP : return 33071;
        case IMAGE::WRAPMODE_MIRROR : return 33648;
        default : return 10497;
    }
}

bool
IsOne( const Float32* values, UInt32 count )
{
    for ( UInt32 i=0; i<count; ++i )
        if ( 1.0f != values[ i ] )
            return false;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGltfSceneWriter::CGltfSceneWriter( const ASSETCONVERTER::CAssetScene& scene  ,
                                    const CORE::CString& folder               ,
                                    const CORE::CString& binaryUri            ,
                                    ASSETCONVERTER::CConversionReport& report )
    : m_scene( scene )
    , m_folder( folder )
    , m_binaryUri( binaryUri )
    , m_report( report )
    , m_writer()
    , m_binary()
    , m_extensionsUsed()
    , m_imageUris()
    , m_lowerLodNodes()
    , m_lodGroupOfFirstLevel()
    , m_firstColliderNode()
    , m_colliders()
    , m_samplerWraps()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGltfSceneWriter::~CGltfSceneWriter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CGltfSceneWriter::GetVersion( void )
{GUCEF_TRACE;

    // 2: the masks of the vertices as the attribute _MASK_0
    // 3: the hidden face sets (extras of the scene "gucefHiddenFaceSets", of the primitives "gucefHiddenBy")
    return 3;
}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer&
CGltfSceneWriter::GetText( void ) const
{GUCEF_TRACE;

    return m_writer.GetText();
}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer&
CGltfSceneWriter::GetBinary( void ) const
{GUCEF_TRACE;

    return m_binary.GetBinary();
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CGltfSceneWriter::GetImageUris( void ) const
{GUCEF_TRACE;

    return m_imageUris;
}

/*-------------------------------------------------------------------------*/

bool
CGltfSceneWriter::IsLowerLodLevel( UInt32 node ) const
{GUCEF_TRACE;

    return m_lowerLodNodes.find( node ) != m_lowerLodNodes.end();
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneWriter::Prepare( void )
{GUCEF_TRACE;

    for ( size_t g=0; g<m_scene.lodGroups.size(); ++g )
    {
        const ASSETCONVERTER::CAssetLodGroup& group = m_scene.lodGroups[ g ];
        if ( group.levels.empty() || group.levels[ 0 ].node >= m_scene.nodes.size() )
            continue;
        m_lodGroupOfFirstLevel[ group.levels[ 0 ].node ] = (UInt32) g;
        for ( size_t l=1; l<group.levels.size(); ++l )
            if ( group.levels[ l ].node < m_scene.nodes.size() )
                m_lowerLodNodes.insert( group.levels[ l ].node );
    }

    UInt32 next = (UInt32) m_scene.nodes.size();
    for ( size_t n=0; n<m_scene.nodes.size(); ++n )
    {
        if ( m_scene.nodes[ n ].colliders.empty() )
            continue;
        m_firstColliderNode[ (UInt32) n ] = next;
        for ( size_t c=0; c<m_scene.nodes[ n ].colliders.size(); ++c )
        {
            SColliderEntry entry;
            entry.owner = (UInt32) n;
            entry.index = (UInt32) c;
            entry.node = next++;
            m_colliders.push_back( entry );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CGltfSceneWriter::Write( void )
{GUCEF_TRACE;

    if ( m_scene.coordinateSystem != MATH::CCoordinateSystem::GucefDefault() )
        m_report.AddWarning( SourceName, m_scene.name, "the scene is not in the coordinate system of glTF (" + m_scene.coordinateSystem.ToString() + "): it is written as it is" );

    Prepare();
    m_writer.BeginObject();
    WriteAsset();
    WriteScenes();
    WriteNodes();
    WriteMeshes();
    WriteSkins();
    WriteAnimations();
    WriteMaterials();
    WriteTextures();
    if ( m_binary.GetAccessorCount() > 0 )
    {
        m_binary.WriteTo( m_writer );
        m_writer.Key( "buffers" );
        m_writer.BeginArray();
        m_writer.BeginObject();
        m_writer.Member( "uri", m_binaryUri );
        m_writer.Member( "byteLength", m_binary.GetByteLength() );
        m_writer.EndObject();
        m_writer.EndArray();
    }
    WriteExtensions();
    m_writer.EndObject();
    m_writer.EndDocument();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneWriter::WriteAsset( void )
{GUCEF_TRACE;

    m_writer.Key( "asset" );
    m_writer.BeginObject();
    m_writer.Member( "version", "2.0" );
    m_writer.Member( "generator", "GUCEF asset converter, assetpluginGLTF " + CORE::UInt32ToString( GetVersion() ) );
    m_writer.EndObject();
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneWriter::WriteScenes( void )
{GUCEF_TRACE;

    TUInt32Vector roots;
    for ( size_t r=0; r<m_scene.rootNodes.size(); ++r )
        if ( m_scene.rootNodes[ r ] < m_scene.nodes.size() && !IsLowerLodLevel( m_scene.rootNodes[ r ] ) )
            roots.push_back( m_scene.rootNodes[ r ] );

    m_writer.Member( "scene", (Int32) 0 );
    m_writer.Key( "scenes" );
    m_writer.BeginArray();
    m_writer.BeginObject();
    if ( !m_scene.name.IsNULLOrEmpty() )
        m_writer.Member( "name", m_scene.name );
    if ( !roots.empty() )
        m_writer.MemberIntArray( "nodes", &roots[ 0 ], (UInt32) roots.size() );

    // the facts about the source that an importer keeps in the extras are not part of the scene
    CORE::CDataNode extras( m_scene.extras );
    extras.DelChild( "gltf" );
    extras.DelChild( "fbx" );
    // the outfits whose faces of a body the primitives name in "gucefHiddenBy"
    extras.DelChild( ASSETCONVERTER::CAssetHiddenFaceSet::SceneExtrasKey );
    if ( !m_scene.hiddenFaceSets.empty() )
        ASSETCONVERTER::CAssetHiddenFaceSet::SerializeList( m_scene.hiddenFaceSets, *extras.AddChild( ASSETCONVERTER::CAssetHiddenFaceSet::SceneExtrasKey, GUCEF_DATATYPE_ARRAY ) );
    if ( extras.GetAttCount() > 0 || extras.HasChildren() )
        m_writer.MemberNode( "extras", extras );
    m_writer.EndObject();
    m_writer.EndArray();
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneWriter::WriteNodeTransform( const ASSETCONVERTER::CAssetNode& node )
{GUCEF_TRACE;

    const MATH::TFloat32Transform3D& t = node.transform;
    if ( 0.0f != t.position.x || 0.0f != t.position.y || 0.0f != t.position.z )
    {
        const Float32 values[ 3 ] = { t.position.x, t.position.y, t.position.z };
        m_writer.MemberFloatArray( "translation", values, 3 );
    }
    if ( 0.0f != t.rotation.x || 0.0f != t.rotation.y || 0.0f != t.rotation.z || 1.0f != t.rotation.w )
    {
        const Float32 values[ 4 ] = { t.rotation.x, t.rotation.y, t.rotation.z, t.rotation.w };
        m_writer.MemberFloatArray( "rotation", values, 4 );
    }
    if ( 1.0f != t.scale.x || 1.0f != t.scale.y || 1.0f != t.scale.z )
    {
        const Float32 values[ 3 ] = { t.scale.x, t.scale.y, t.scale.z };
        m_writer.MemberFloatArray( "scale", values, 3 );
    }
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneWriter::WriteNodes( void )
{GUCEF_TRACE;

    if ( m_scene.nodes.empty() )
        return;

    m_writer.Key( "nodes" );
    m_writer.BeginArray();
    for ( size_t n=0; n<m_scene.nodes.size(); ++n )
    {
        const ASSETCONVERTER::CAssetNode& node = m_scene.nodes[ n ];
        m_writer.BeginObject();
        if ( !node.name.IsNULLOrEmpty() )
            m_writer.Member( "name", node.name );

        TUInt32Vector kids;
        for ( size_t c=0; c<node.children.size(); ++c )
            if ( node.children[ c ] < m_scene.nodes.size() && !IsLowerLodLevel( node.children[ c ] ) )
                kids.push_back( node.children[ c ] );
        GUCEF::map< UInt32, UInt32 >::const_iterator firstCollider = m_firstColliderNode.find( (UInt32) n );
        if ( firstCollider != m_firstColliderNode.end() )
            for ( size_t c=0; c<node.colliders.size(); ++c )
                kids.push_back( (*firstCollider).second + (UInt32) c );
        if ( !kids.empty() )
            m_writer.MemberIntArray( "children", &kids[ 0 ], (UInt32) kids.size() );

        const bool hasMesh = node.mesh >= 0 && (size_t) node.mesh < m_scene.meshes.size();
        if ( hasMesh )
        {
            m_writer.Member( "mesh", (Int32) node.mesh );
            if ( node.skin >= 0 && (size_t) node.skin < m_scene.skins.size() )
                m_writer.Member( "skin", (Int32) node.skin );
        }
        else
        if ( node.skin >= 0 )
        {
            m_report.AddWarning( SourceName, node.name, "the node has a skin but no mesh: the skin is not written for it" );
        }
        WriteNodeTransform( node );

        // extensions
        const bool hasLight = node.light >= 0 && (size_t) node.light < m_scene.lights.size();
        GUCEF::map< UInt32, UInt32 >::const_iterator lodGroup = m_lodGroupOfFirstLevel.find( (UInt32) n );
        const bool hasLod = lodGroup != m_lodGroupOfFirstLevel.end() && m_scene.lodGroups[ (*lodGroup).second ].levels.size() > 1;
        if ( hasLight || hasLod )
        {
            m_writer.Key( "extensions" );
            m_writer.BeginObject();
            if ( hasLight )
            {
                m_extensionsUsed.insert( ExtLights );
                m_writer.Key( ExtLights );
                m_writer.BeginObject();
                m_writer.Member( "light", (Int32) node.light );
                m_writer.EndObject();
            }
            if ( hasLod )
            {
                const ASSETCONVERTER::CAssetLodGroup& group = m_scene.lodGroups[ (*lodGroup).second ];
                TUInt32Vector ids;
                for ( size_t l=1; l<group.levels.size(); ++l )
                    ids.push_back( group.levels[ l ].node );
                m_extensionsUsed.insert( ExtLod );
                m_writer.Key( ExtLod );
                m_writer.BeginObject();
                m_writer.MemberIntArray( "ids", &ids[ 0 ], (UInt32) ids.size() );
                m_writer.EndObject();
            }
            m_writer.EndObject();
        }

        // extras: those of the node and what the glTF structure cannot hold
        CORE::CDataNode extras( node.extras );
        extras.SetName( "extras" );
        if ( !node.prefabInstance.IsNULLOrEmpty() )
            extras.SetAttribute( "prefabInstance", node.prefabInstance );
        if ( node.lodGroup >= 0 && (size_t) node.lodGroup < m_scene.lodGroups.size() && !m_scene.lodGroups[ node.lodGroup ].levels.empty() )
        {
            const ASSETCONVERTER::CAssetLodGroup& group = m_scene.lodGroups[ node.lodGroup ];
            CORE::CDataNode* holder = extras.AddChild( "gucefLodGroup", GUCEF_DATATYPE_OBJECT );
            holder->SetAttribute( "name", group.name );
            holder->SetAttribute( "first", (Int32) group.levels[ 0 ].node );
        }
        if ( hasLod )
        {
            const ASSETCONVERTER::CAssetLodGroup& group = m_scene.lodGroups[ (*lodGroup).second ];
            CORE::CDataNode::TVariantVector coverage;
            for ( size_t l=0; l<group.levels.size(); ++l )
                coverage.push_back( CORE::CVariant( group.levels[ l ].screenRelativeHeight ) );
            extras.SetValuesOfChildByName( "MSFT_screencoverage", coverage );
        }
        if ( IsLowerLodLevel( (UInt32) n ) )
            extras.SetAttribute( "gucefParent", (Int32) node.parent );
        // glTF has no particles: the system is in the "particles" list of the sidecar (design section 5.7.1)
        if ( node.particleSystem >= 0 )
            extras.SetAttribute( "gucefParticleSystem", node.particleSystem );
        if ( extras.GetAttCount() > 0 || extras.HasChildren() )
            m_writer.MemberNode( "extras", extras );
        m_writer.EndObject();
    }

    // the colliders, each a node of its own below its owner
    for ( size_t c=0; c<m_colliders.size(); ++c )
    {
        const ASSETCONVERTER::CAssetCollider& collider = m_scene.nodes[ m_colliders[ c ].owner ].colliders[ m_colliders[ c ].index ];
        ASSETCONVERTER::CAssetNode carrier;
        carrier.transform = collider.transform;
        m_writer.BeginObject();
        if ( !collider.name.IsNULLOrEmpty() )
            m_writer.Member( "name", collider.name );
        WriteNodeTransform( carrier );
        m_extensionsUsed.insert( ExtPhysicsBody );
        m_writer.Key( "extensions" );
        m_writer.BeginObject();
        m_writer.Key( ExtPhysicsBody );
        m_writer.BeginObject();
        m_writer.Key( collider.trigger ? "trigger" : "collider" );
        m_writer.BeginObject();
        m_writer.Member( "shape", (UInt32) c );
        m_writer.EndObject();
        m_writer.EndObject();
        m_writer.EndObject();
        m_writer.Key( "extras" );
        m_writer.BeginObject();
        m_writer.Member( "gucefCollider", true );
        m_writer.EndObject();
        m_writer.EndObject();
    }
    m_writer.EndArray();
}

/*-------------------------------------------------------------------------*/

bool
CGltfSceneWriter::WritePrimitive( const ASSETCONVERTER::CAssetSubMesh& subMesh, UInt32 morphTargetCount )
{GUCEF_TRACE;

    const UInt32 count = (UInt32) ( subMesh.positions.size() / 3 );
    if ( 0 == count || 0 != subMesh.positions.size() % 3 )
    {
        m_report.AddWarning( SourceName, m_scene.name, "a submesh without vertices (or with a broken position stream) is left out" );
        return false;
    }

    typedef CGltfBufferBuilder TBuilder;
    m_writer.BeginObject();
    m_writer.Key( "attributes" );
    m_writer.BeginObject();
    m_writer.Member( "POSITION", m_binary.AddFloats( &subMesh.positions[ 0 ], count, TBuilder::ELEMENT_VEC3, TBuilder::TARGET_ARRAY, true ) );
    if ( subMesh.normals.size() == (size_t) count * 3 )
        m_writer.Member( "NORMAL", m_binary.AddFloats( &subMesh.normals[ 0 ], count, TBuilder::ELEMENT_VEC3, TBuilder::TARGET_ARRAY, false ) );
    if ( subMesh.tangents.size() == (size_t) count * 4 )
        m_writer.Member( "TANGENT", m_binary.AddFloats( &subMesh.tangents[ 0 ], count, TBuilder::ELEMENT_VEC4, TBuilder::TARGET_ARRAY, false ) );
    for ( UInt32 uv=0; uv<ASSETCONVERTER::CAssetSubMesh::MaxUvSets; ++uv )
    {
        if ( subMesh.uvSets[ uv ].size() == (size_t) count * 2 )
            m_writer.Member( ( "TEXCOORD_" + CORE::UInt32ToString( uv ) ).C_String(), m_binary.AddFloats( &subMesh.uvSets[ uv ][ 0 ], count, TBuilder::ELEMENT_VEC2, TBuilder::TARGET_ARRAY, false ) );
        else
        if ( !subMesh.uvSets[ uv ].empty() )
            m_report.AddWarning( SourceName, m_scene.name, "UV set " + CORE::UInt32ToString( uv ) + " does not have two values per vertex and is left out" );
    }
    if ( subMesh.colours.size() == (size_t) count * 4 )
        m_writer.Member( "COLOR_0", m_binary.AddFloats( &subMesh.colours[ 0 ], count, TBuilder::ELEMENT_VEC4, TBuilder::TARGET_ARRAY, false ) );
    // masks are data of the materials, not colours: an attribute of the application (a name with an underscore), RGBA
    if ( subMesh.masks.size() == (size_t) count * 4 )
        m_writer.Member( "_MASK_0", m_binary.AddFloats( &subMesh.masks[ 0 ], count, TBuilder::ELEMENT_VEC4, TBuilder::TARGET_ARRAY, false ) );
    else
    if ( !subMesh.masks.empty() )
        m_report.AddWarning( SourceName, m_scene.name, "the masks of a submesh do not have four values per vertex and are left out" );

    const UInt32 influences = subMesh.influencesPerVertex;
    const bool skinned = influences > 0 && subMesh.joints.size() == (size_t) count * influences && subMesh.weights.size() == (size_t) count * influences;
    if ( skinned )
    {
        const UInt32 sets = ( influences + 3 ) / 4;
        for ( UInt32 s=0; s<sets; ++s )
        {
            GUCEF::vector< UInt32 > joints;
            GUCEF::vector< Float32 > weights;
            joints.resize( (size_t) count * 4, 0 );
            weights.resize( (size_t) count * 4, 0.0f );
            for ( UInt32 v=0; v<count; ++v )
            {
                for ( UInt32 j=0; j<4; ++j )
                {
                    const UInt32 slot = s * 4 + j;
                    if ( slot < influences )
                    {
                        joints[ (size_t) v * 4 + j ] = subMesh.joints[ (size_t) v * influences + slot ];
                        weights[ (size_t) v * 4 + j ] = subMesh.weights[ (size_t) v * influences + slot ];
                    }
                }
            }
            m_writer.Member( ( "JOINTS_" + CORE::UInt32ToString( s ) ).C_String(), m_binary.AddUInt16Vec4( &joints[ 0 ], count, TBuilder::TARGET_ARRAY ) );
            m_writer.Member( ( "WEIGHTS_" + CORE::UInt32ToString( s ) ).C_String(), m_binary.AddFloats( &weights[ 0 ], count, TBuilder::ELEMENT_VEC4, TBuilder::TARGET_ARRAY, false ) );
        }
    }
    else
    if ( influences > 0 )
    {
        m_report.AddWarning( SourceName, m_scene.name, "the joints and weights of a submesh do not match its vertices: they are left out" );
    }
    m_writer.EndObject();

    if ( !subMesh.indices.empty() )
    {
        bool inRange = 0 == subMesh.indices.size() % 3;
        for ( size_t i=0; i<subMesh.indices.size() && inRange; ++i )
            inRange = subMesh.indices[ i ] < count;
        if ( inRange )
            m_writer.Member( "indices", m_binary.AddIndices( &subMesh.indices[ 0 ], (UInt32) subMesh.indices.size() ) );
        else
            m_report.AddWarning( SourceName, m_scene.name, "the indices of a submesh are not triangles of its vertices: they are left out" );
    }
    if ( subMesh.materialIndex >= 0 && (size_t) subMesh.materialIndex < m_scene.materials.size() )
        m_writer.Member( "material", (Int32) subMesh.materialIndex );

    if ( morphTargetCount > 0 )
        WriteMorphTargets( subMesh, morphTargetCount, count );

    // the hidden face sets that hide the triangles of the primitive, by name (the sets are in the extras of the scene)
    CORE::CString::StringVector hiddenBy;
    for ( size_t h=0; h<m_scene.hiddenFaceSets.size(); ++h )
        if ( 0 != ( subMesh.hiddenBy & ASSETCONVERTER::CAssetHiddenFaceSet::BitOf( (UInt32) h ) ) )
            hiddenBy.push_back( m_scene.hiddenFaceSets[ h ].name );
    if ( skinned || !hiddenBy.empty() )
    {
        m_writer.Key( "extras" );
        m_writer.BeginObject();
        if ( skinned )
            m_writer.Member( "gucefInfluences", influences );
        if ( !hiddenBy.empty() )
        {
            m_writer.Key( ASSETCONVERTER::CAssetHiddenFaceSet::SubMeshExtrasKey.C_String() );
            m_writer.BeginArray();
            for ( size_t h=0; h<hiddenBy.size(); ++h )
                m_writer.String( hiddenBy[ h ] );
            m_writer.EndArray();
        }
        m_writer.EndObject();
    }
    m_writer.EndObject();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneWriter::WriteMorphTargets( const ASSETCONVERTER::CAssetSubMesh& subMesh, UInt32 morphTargetCount, UInt32 vertexCount )
{GUCEF_TRACE;

    typedef CGltfBufferBuilder TBuilder;
    m_writer.Key( "targets" );
    m_writer.BeginArray();
    for ( UInt32 t=0; t<morphTargetCount; ++t )
    {
        const ASSETCONVERTER::CAssetMorphTarget* target = t < subMesh.morphTargets.size() ? &subMesh.morphTargets[ t ] : GUCEF_NULL;
        const size_t expected = (size_t) vertexCount * 3;
        m_writer.BeginObject();
        if ( GUCEF_NULL != target && target->positionDeltas.size() == expected )
        {
            m_writer.Member( "POSITION", m_binary.AddFloats( &target->positionDeltas[ 0 ], vertexCount, TBuilder::ELEMENT_VEC3, TBuilder::TARGET_ARRAY, true ) );
        }
        else
        {
            // a target holds at least a position: one that does not move it
            GUCEF::vector< Float32 > zero;
            zero.resize( expected, 0.0f );
            m_writer.Member( "POSITION", m_binary.AddFloats( &zero[ 0 ], vertexCount, TBuilder::ELEMENT_VEC3, TBuilder::TARGET_ARRAY, true ) );
        }
        if ( GUCEF_NULL != target && target->normalDeltas.size() == expected )
            m_writer.Member( "NORMAL", m_binary.AddFloats( &target->normalDeltas[ 0 ], vertexCount, TBuilder::ELEMENT_VEC3, TBuilder::TARGET_ARRAY, false ) );
        if ( GUCEF_NULL != target && target->tangentDeltas.size() == expected )
            m_writer.Member( "TANGENT", m_binary.AddFloats( &target->tangentDeltas[ 0 ], vertexCount, TBuilder::ELEMENT_VEC3, TBuilder::TARGET_ARRAY, false ) );
        m_writer.EndObject();
    }
    m_writer.EndArray();
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneWriter::WriteMeshes( void )
{GUCEF_TRACE;

    if ( m_scene.meshes.empty() )
        return;

    m_writer.Key( "meshes" );
    m_writer.BeginArray();
    for ( size_t m=0; m<m_scene.meshes.size(); ++m )
    {
        const ASSETCONVERTER::CAssetMesh& mesh = m_scene.meshes[ m ];
        UInt32 targetCount = 0;
        const ASSETCONVERTER::CAssetSubMesh* withTargets = GUCEF_NULL;
        for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
        {
            if ( mesh.subMeshes[ s ].morphTargets.size() > targetCount )
                targetCount = (UInt32) mesh.subMeshes[ s ].morphTargets.size();
            if ( GUCEF_NULL == withTargets && !mesh.subMeshes[ s ].morphTargets.empty() )
                withTargets = &mesh.subMeshes[ s ];
        }

        m_writer.BeginObject();
        if ( !mesh.name.IsNULLOrEmpty() )
            m_writer.Member( "name", mesh.name );
        m_writer.Key( "primitives" );
        m_writer.BeginArray();
        for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
            WritePrimitive( mesh.subMeshes[ s ], targetCount );
        m_writer.EndArray();
        if ( targetCount > 0 )
        {
            GUCEF::vector< Float32 > weights;
            weights.resize( targetCount, 0.0f );
            bool named = false;
            for ( UInt32 t=0; t<targetCount; ++t )
            {
                if ( t < withTargets->morphTargets.size() )
                {
                    weights[ t ] = withTargets->morphTargets[ t ].defaultWeight;
                    named = named || !withTargets->morphTargets[ t ].name.IsNULLOrEmpty();
                }
            }
            m_writer.MemberFloatArray( "weights", &weights[ 0 ], targetCount );
            if ( named )
            {
                m_writer.Key( "extras" );
                m_writer.BeginObject();
                m_writer.Key( "targetNames" );
                m_writer.BeginArray();
                for ( UInt32 t=0; t<targetCount; ++t )
                    m_writer.String( t < withTargets->morphTargets.size() ? withTargets->morphTargets[ t ].name : CORE::CString::Empty );
                m_writer.EndArray();
                m_writer.EndObject();
            }
        }
        m_writer.EndObject();
    }
    m_writer.EndArray();
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneWriter::WriteSkins( void )
{GUCEF_TRACE;

    if ( m_scene.skins.empty() )
        return;

    m_writer.Key( "skins" );
    m_writer.BeginArray();
    for ( size_t s=0; s<m_scene.skins.size(); ++s )
    {
        const ASSETCONVERTER::CAssetSkin& skin = m_scene.skins[ s ];
        m_writer.BeginObject();
        if ( !skin.name.IsNULLOrEmpty() )
            m_writer.Member( "name", skin.name );
        if ( !skin.joints.empty() && skin.inverseBindMatrices.size() == skin.joints.size() )
        {
            GUCEF::vector< Float32 > matrices;
            matrices.reserve( skin.inverseBindMatrices.size() * 16 );
            for ( size_t m=0; m<skin.inverseBindMatrices.size(); ++m )
                matrices.insert( matrices.end(), skin.inverseBindMatrices[ m ].GetData(), skin.inverseBindMatrices[ m ].GetData() + 16 );
            m_writer.Member( "inverseBindMatrices", m_binary.AddFloats( &matrices[ 0 ], (UInt32) skin.inverseBindMatrices.size(), CGltfBufferBuilder::ELEMENT_MAT4, CGltfBufferBuilder::TARGET_NONE, false ) );
        }
        if ( skin.skeletonRoot >= 0 && (size_t) skin.skeletonRoot < m_scene.nodes.size() )
            m_writer.Member( "skeleton", (Int32) skin.skeletonRoot );
        if ( !skin.joints.empty() )
            m_writer.MemberIntArray( "joints", &skin.joints[ 0 ], (UInt32) skin.joints.size() );
        m_writer.EndObject();
    }
    m_writer.EndArray();
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneWriter::WriteAnimations( void )
{GUCEF_TRACE;

    if ( m_scene.animations.empty() )
        return;

    typedef CGltfBufferBuilder TBuilder;
    m_writer.Key( "animations" );
    m_writer.BeginArray();
    for ( size_t a=0; a<m_scene.animations.size(); ++a )
    {
        const ASSETCONVERTER::CAssetAnimation& animation = m_scene.animations[ a ];

        // the channels that can be written: keys of the right number, a node that is there
        GUCEF::vector< const ASSETCONVERTER::CAssetAnimationChannel* > channels;
        for ( size_t c=0; c<animation.channels.size(); ++c )
        {
            const ASSETCONVERTER::CAssetAnimationChannel& channel = animation.channels[ c ];
            const UInt32 keys = (UInt32) channel.times.size();
            const UInt32 multiplier = ASSETCONVERTER::ANIMATIONINTERPOLATION_CUBICSPLINE == channel.interpolation ? 3 : 1;
            bool valid = keys > 0 && channel.targetNode < m_scene.nodes.size() && !channel.values.empty();
            if ( valid )
            {
                if ( ASSETCONVERTER::ANIMATIONPATH_WEIGHTS == channel.path )
                    valid = 0 == channel.values.size() % ( (size_t) keys * multiplier );
                else
                    valid = channel.values.size() == (size_t) keys * multiplier * ( ASSETCONVERTER::ANIMATIONPATH_ROTATION == channel.path ? 4 : 3 );
            }
            if ( valid )
                channels.push_back( &channel );
            else
                m_report.AddWarning( SourceName, animation.name, "a channel of the animation does not match its keys and is left out" );
        }

        m_writer.BeginObject();
        if ( !animation.name.IsNULLOrEmpty() )
            m_writer.Member( "name", animation.name );
        m_writer.Key( "samplers" );
        m_writer.BeginArray();
        for ( size_t c=0; c<channels.size(); ++c )
        {
            const ASSETCONVERTER::CAssetAnimationChannel& channel = *channels[ c ];
            const UInt32 keys = (UInt32) channel.times.size();
            TBuilder::EElementType element = TBuilder::ELEMENT_VEC3;
            UInt32 elementCount = keys;
            if ( ASSETCONVERTER::ANIMATIONPATH_ROTATION == channel.path )
                element = TBuilder::ELEMENT_VEC4;
            if ( ASSETCONVERTER::ANIMATIONPATH_WEIGHTS == channel.path )
            {
                element = TBuilder::ELEMENT_SCALAR;
                elementCount = (UInt32) channel.values.size();
            }
            else
            if ( ASSETCONVERTER::ANIMATIONINTERPOLATION_CUBICSPLINE == channel.interpolation )
            {
                elementCount = keys * 3;
            }
            m_writer.BeginObject();
            m_writer.Member( "input", m_binary.AddFloats( &channel.times[ 0 ], keys, TBuilder::ELEMENT_SCALAR, TBuilder::TARGET_NONE, true ) );
            m_writer.Member( "interpolation", ASSETCONVERTER::ToString( channel.interpolation ) );
            m_writer.Member( "output", m_binary.AddFloats( &channel.values[ 0 ], elementCount, element, TBuilder::TARGET_NONE, false ) );
            m_writer.EndObject();
        }
        m_writer.EndArray();
        m_writer.Key( "channels" );
        m_writer.BeginArray();
        for ( size_t c=0; c<channels.size(); ++c )
        {
            m_writer.BeginObject();
            m_writer.Member( "sampler", (UInt32) c );
            m_writer.Key( "target" );
            m_writer.BeginObject();
            m_writer.Member( "node", channels[ c ]->targetNode );
            m_writer.Member( "path", ASSETCONVERTER::ToString( channels[ c ]->path ) );
            m_writer.EndObject();
            m_writer.EndObject();
        }
        m_writer.EndArray();
        m_writer.Key( "extras" );
        m_writer.BeginObject();
        m_writer.Member( "frameRate", animation.frameRate );
        m_writer.Member( "firstFrame", animation.firstFrame );
        m_writer.Member( "lastFrame", animation.lastFrame );
        m_writer.Member( "loop", animation.loop );
        m_writer.Member( "sampled", animation.sampled );
        if ( !animation.sourceTake.IsNULLOrEmpty() )
            m_writer.Member( "sourceTake", animation.sourceTake );
        m_writer.EndObject();
        m_writer.EndObject();
    }
    m_writer.EndArray();
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneWriter::WriteTextureInfo( const char* key, const ASSETCONVERTER::CAssetTextureReference& reference, bool isNormal, bool isOcclusion )
{GUCEF_TRACE;

    if ( !reference.IsUsed() || (size_t) reference.textureIndex >= m_scene.textures.size() )
        return;

    m_writer.Key( key );
    m_writer.BeginObject();
    m_writer.Member( "index", (Int32) reference.textureIndex );
    if ( 0 != reference.texCoord )
        m_writer.Member( "texCoord", reference.texCoord );
    if ( isNormal && 1.0f != reference.scale )
        m_writer.Member( "scale", reference.scale );
    if ( isOcclusion && 1.0f != reference.scale )
        m_writer.Member( "strength", reference.scale );
    if ( reference.hasTransform )
    {
        m_extensionsUsed.insert( ExtTextureTransform );
        m_writer.Key( "extensions" );
        m_writer.BeginObject();
        m_writer.Key( ExtTextureTransform );
        m_writer.BeginObject();
        m_writer.MemberFloatArray( "offset", reference.offset, 2 );
        m_writer.Member( "rotation", reference.rotation );
        m_writer.MemberFloatArray( "scale", reference.tiling, 2 );
        m_writer.EndObject();
        m_writer.EndObject();
    }
    m_writer.EndObject();
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneWriter::WriteMaterials( void )
{GUCEF_TRACE;

    if ( m_scene.materials.empty() )
        return;

    m_writer.Key( "materials" );
    m_writer.BeginArray();
    for ( size_t m=0; m<m_scene.materials.size(); ++m )
    {
        const ASSETCONVERTER::CAssetMaterial& material = m_scene.materials[ m ];
        m_writer.BeginObject();
        if ( !material.name.IsNULLOrEmpty() )
            m_writer.Member( "name", material.name );

        m_writer.Key( "pbrMetallicRoughness" );
        m_writer.BeginObject();
        if ( !IsOne( material.baseColorFactor, 4 ) )
            m_writer.MemberFloatArray( "baseColorFactor", material.baseColorFactor, 4 );
        WriteTextureInfo( "baseColorTexture", material.baseColorTexture, false, false );
        if ( 1.0f != material.metallicFactor )
            m_writer.Member( "metallicFactor", material.metallicFactor );
        if ( 1.0f != material.roughnessFactor )
            m_writer.Member( "roughnessFactor", material.roughnessFactor );
        WriteTextureInfo( "metallicRoughnessTexture", material.metallicRoughnessTexture, false, false );
        m_writer.EndObject();

        WriteTextureInfo( "normalTexture", material.normalTexture, true, false );
        WriteTextureInfo( "occlusionTexture", material.occlusionTexture, false, true );
        WriteTextureInfo( "emissiveTexture", material.emissiveTexture, false, false );
        if ( 0.0f != material.emissiveFactor[ 0 ] || 0.0f != material.emissiveFactor[ 1 ] || 0.0f != material.emissiveFactor[ 2 ] )
            m_writer.MemberFloatArray( "emissiveFactor", material.emissiveFactor, 3 );
        if ( ASSETCONVERTER::ALPHAMODE_OPAQUE != material.alphaMode )
            m_writer.Member( "alphaMode", ASSETCONVERTER::ToString( material.alphaMode ) );
        if ( ASSETCONVERTER::ALPHAMODE_MASK == material.alphaMode && 0.5f != material.alphaCutoff )
            m_writer.Member( "alphaCutoff", material.alphaCutoff );
        if ( material.doubleSided )
            m_writer.Member( "doubleSided", true );

        // the extensions: the ones the model has values for, and the rest as the source gave them
        GUCEF::vector< const CORE::CDataNode* > passThrough;
        CORE::CDataNode::const_iterator e = material.extensions.ConstBegin();
        while ( e != material.extensions.ConstEnd() )
        {
            const CORE::CString& name = (*e)->GetName();
            if ( !name.IsNULLOrEmpty() && name != ExtEmissiveStrength && name != ExtUnlit && name != ExtTextureTransform )
                passThrough.push_back( *e );
            ++e;
        }
        const bool strength = 1.0f != material.emissiveStrength;
        if ( strength || material.unlit || !passThrough.empty() )
        {
            m_writer.Key( "extensions" );
            m_writer.BeginObject();
            if ( strength )
            {
                m_extensionsUsed.insert( ExtEmissiveStrength );
                m_writer.Key( ExtEmissiveStrength );
                m_writer.BeginObject();
                m_writer.Member( "emissiveStrength", material.emissiveStrength );
                m_writer.EndObject();
            }
            if ( material.unlit )
            {
                m_extensionsUsed.insert( ExtUnlit );
                m_writer.Key( ExtUnlit );
                m_writer.BeginObject();
                m_writer.EndObject();
            }
            for ( size_t p=0; p<passThrough.size(); ++p )
            {
                m_extensionsUsed.insert( passThrough[ p ]->GetName() );
                m_writer.Key( passThrough[ p ]->GetName().C_String() );
                m_writer.Node( *passThrough[ p ] );
            }
            m_writer.EndObject();
        }
        m_writer.EndObject();
    }
    m_writer.EndArray();
}

/*-------------------------------------------------------------------------*/

UInt32
CGltfSceneWriter::GetSamplerIndex( const ASSETCONVERTER::CAssetTexture& texture )
{GUCEF_TRACE;

    const UInt32 key = GetWrapNumber( texture.wrapS ) * 65536 + GetWrapNumber( texture.wrapT ) % 65536;
    for ( size_t i=0; i<m_samplerWraps.size(); ++i )
        if ( m_samplerWraps[ i ] == key )
            return (UInt32) i;
    m_samplerWraps.push_back( key );
    return (UInt32) m_samplerWraps.size() - 1;
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneWriter::WriteTextures( void )
{GUCEF_TRACE;

    if ( m_scene.textures.empty() )
        return;

    GUCEF::vector< UInt32 > samplerOfTexture;
    for ( size_t t=0; t<m_scene.textures.size(); ++t )
        samplerOfTexture.push_back( GetSamplerIndex( m_scene.textures[ t ] ) );

    for ( size_t t=0; t<m_scene.textures.size(); ++t )
    {
        const ASSETCONVERTER::CAssetTexture& texture = m_scene.textures[ t ];
        CORE::CString uri;
        if ( !texture.bakedPath.IsNULLOrEmpty() )
        {
            uri = CGltfUri::MakeRelative( m_folder, texture.bakedPath );
        }
        else
        if ( !texture.sources.empty() )
        {
            uri = CGltfUri::MakeRelative( m_folder, texture.sources[ 0 ] );
            m_report.AddWarning( SourceName, texture.name, "the texture is not baked: its image names the source of the texture" );
        }
        else
        {
            m_report.AddWarning( SourceName, texture.name, "the texture has no source: its image has no file" );
        }
        m_imageUris.push_back( uri );
    }

    m_writer.Key( "textures" );
    m_writer.BeginArray();
    for ( size_t t=0; t<m_scene.textures.size(); ++t )
    {
        m_writer.BeginObject();
        if ( !m_scene.textures[ t ].name.IsNULLOrEmpty() )
            m_writer.Member( "name", m_scene.textures[ t ].name );
        m_writer.Member( "sampler", samplerOfTexture[ t ] );
        m_writer.Member( "source", (UInt32) t );
        m_writer.Key( "extras" );
        m_writer.BeginObject();
        m_writer.Member( "usage", ASSETCONVERTER::ToString( m_scene.textures[ t ].usage ) );
        m_writer.EndObject();
        m_writer.EndObject();
    }
    m_writer.EndArray();

    m_writer.Key( "images" );
    m_writer.BeginArray();
    for ( size_t t=0; t<m_scene.textures.size(); ++t )
    {
        m_writer.BeginObject();
        if ( !m_imageUris[ t ].IsNULLOrEmpty() )
            m_writer.Member( "uri", m_imageUris[ t ] );
        m_writer.EndObject();
    }
    m_writer.EndArray();

    m_writer.Key( "samplers" );
    m_writer.BeginArray();
    for ( size_t s=0; s<m_samplerWraps.size(); ++s )
    {
        m_writer.BeginObject();
        m_writer.Member( "wrapS", m_samplerWraps[ s ] / 65536 );
        m_writer.Member( "wrapT", m_samplerWraps[ s ] % 65536 );
        m_writer.EndObject();
    }
    m_writer.EndArray();
}

/*-------------------------------------------------------------------------*/

void
CGltfSceneWriter::WriteExtensions( void )
{GUCEF_TRACE;

    const bool hasLights = !m_scene.lights.empty();
    const bool hasShapes = !m_colliders.empty();
    if ( hasLights || hasShapes )
    {
        m_writer.Key( "extensions" );
        m_writer.BeginObject();
        if ( hasLights )
        {
            m_extensionsUsed.insert( ExtLights );
            m_writer.Key( ExtLights );
            m_writer.BeginObject();
            m_writer.Key( "lights" );
            m_writer.BeginArray();
            for ( size_t l=0; l<m_scene.lights.size(); ++l )
            {
                const ASSETCONVERTER::CAssetLight& light = m_scene.lights[ l ];
                m_writer.BeginObject();
                if ( !light.name.IsNULLOrEmpty() )
                    m_writer.Member( "name", light.name );
                m_writer.Member( "type", ASSETCONVERTER::ToString( light.type ) );
                m_writer.MemberFloatArray( "color", light.colour, 3 );
                m_writer.Member( "intensity", light.intensity );
                if ( light.range > 0.0f )
                    m_writer.Member( "range", light.range );
                if ( ASSETCONVERTER::LIGHTTYPE_SPOT == light.type )
                {
                    m_writer.Key( "spot" );
                    m_writer.BeginObject();
                    m_writer.Member( "innerConeAngle", light.innerConeAngle );
                    m_writer.Member( "outerConeAngle", light.outerConeAngle );
                    m_writer.EndObject();
                }
                m_writer.EndObject();
            }
            m_writer.EndArray();
            m_writer.EndObject();
        }
        if ( hasShapes )
        {
            m_extensionsUsed.insert( ExtPhysicsShape );
            m_writer.Key( ExtPhysicsShape );
            m_writer.BeginObject();
            m_writer.Key( "shapes" );
            m_writer.BeginArray();
            for ( size_t c=0; c<m_colliders.size(); ++c )
            {
                const ASSETCONVERTER::CAssetCollider& collider = m_scene.nodes[ m_colliders[ c ].owner ].colliders[ m_colliders[ c ].index ];
                ASSETCONVERTER::TColliderShape shape = collider.shape;
                if ( collider.IsMeshShape() && ( collider.meshIndex < 0 || (size_t) collider.meshIndex >= m_scene.meshes.size() ) )
                {
                    m_report.AddWarning( SourceName, collider.name, "the collider has no mesh: it is written as a box" );
                    shape = ASSETCONVERTER::COLLIDERSHAPE_BOX;
                }
                const CORE::CString typeName = ASSETCONVERTER::ToString( shape );
                m_writer.BeginObject();
                m_writer.Member( "type", typeName );
                m_writer.Key( typeName.C_String() );
                m_writer.BeginObject();
                switch ( shape )
                {
                    case ASSETCONVERTER::COLLIDERSHAPE_BOX :
                    {
                        const Float32 size[ 3 ] = { collider.halfExtents[ 0 ] * 2.0f, collider.halfExtents[ 1 ] * 2.0f, collider.halfExtents[ 2 ] * 2.0f };
                        m_writer.MemberFloatArray( "size", size, 3 );
                        break;
                    }
                    case ASSETCONVERTER::COLLIDERSHAPE_SPHERE :
                    {
                        m_writer.Member( "radius", collider.radius );
                        break;
                    }
                    case ASSETCONVERTER::COLLIDERSHAPE_CAPSULE :
                    case ASSETCONVERTER::COLLIDERSHAPE_CYLINDER :
                    {
                        m_writer.Member( "height", collider.height );
                        m_writer.Member( "radius", collider.radius );
                        break;
                    }
                    default :
                    {
                        m_writer.Member( "mesh", (Int32) collider.meshIndex );
                        break;
                    }
                }
                m_writer.EndObject();
                m_writer.EndObject();
            }
            m_writer.EndArray();
            m_writer.EndObject();
        }
        m_writer.EndObject();
    }

    if ( !m_extensionsUsed.empty() )
    {
        m_writer.Key( "extensionsUsed" );
        m_writer.BeginArray();
        CORE::CString::StringSet::const_iterator i = m_extensionsUsed.begin();
        while ( i != m_extensionsUsed.end() )
        {
            m_writer.String( *i );
            ++i;
        }
        m_writer.EndArray();
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GLTF */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
