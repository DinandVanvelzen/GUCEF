/*
 *  AssetWorkbenchLib: what a mesh holds: statistics and the geometry read back for overlays and the UV layout
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

#include <Ogre.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H
#include "gucefGUI_CPropertyRowDescriptor.h"
#define GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H
#endif /* GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H ? */

#include "AssetWorkbenchLib_CMeshInspector.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
Number( Float64 value, Int32 decimals )
{
    return GUI::CPropertyRowDescriptor::NumberToText( value, decimals );
}

CORE::CString
YesNo( bool value )
{
    return value ? "yes" : "no";
}

/** The floats of one element of every vertex, `components` of them (the rest of a wider element is dropped) */
bool
ReadFloatElement( const Ogre::VertexData& data, Ogre::VertexElementSemantic semantic, unsigned short index, UInt32 components, GUCEF::vector< Float32 >& out )
{
    const Ogre::VertexElement* element = data.vertexDeclaration->findElementBySemantic( semantic, index );
    if ( GUCEF_NULL == element )
        return false;
    const Ogre::VertexElementType type = element->getType();
    const UInt32 available = Ogre::VertexElement::getTypeCount( type );
    const bool isFloat = type == Ogre::VET_FLOAT1 || type == Ogre::VET_FLOAT2 || type == Ogre::VET_FLOAT3 || type == Ogre::VET_FLOAT4;
    if ( !isFloat || available < components )
        return false;

    Ogre::HardwareVertexBufferSharedPtr buffer = data.vertexBufferBinding->getBuffer( element->getSource() );
    const size_t vertexSize = buffer->getVertexSize();
    const size_t start = data.vertexStart;
    const size_t count = data.vertexCount;
    GUCEF::vector< UInt8 > raw( count * vertexSize );
    if ( raw.empty() )
        return false;
    buffer->readData( start * vertexSize, count * vertexSize, &raw[ 0 ] );

    out.resize( count * components );
    for ( size_t v=0; v<count; ++v )
    {
        const Float32* source = reinterpret_cast< const Float32* >( &raw[ v * vertexSize + element->getOffset() ] );
        for ( UInt32 c=0; c<components; ++c )
            out[ v * components + c ] = source[ c ];
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SMeshStatistics::SMeshStatistics( void )
    : subMeshCount( 0 )
    , vertexCount( 0 )
    , triangleCount( 0 )
    , boneCount( 0 )
    , animationCount( 0 )
    , lodLevelCount( 0 )
    , materialCount( 0 )
    , textureCount( 0 )
    , uvSetCount( 0 )
    , textureMemory( 0 )
    , hasNormals( false )
    , hasTangents( false )
    , hasVertexColours( false )
    , hasSkeleton( false )
    , boundingRadius( 0.0f )
    , materialNames()
    , textureNames()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<3; ++i )
    {
        boundsMin[ i ] = 0.0f;
        boundsMax[ i ] = 0.0f;
    }
}

/*-------------------------------------------------------------------------*/

SSubMeshGeometry::SSubMeshGeometry( void )
    : positions()
    , normals()
    , tangents()
    , texCoords()
    , indices()
    , vertexCount( 0 )
    , triangleCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Ogre::VertexData*
CMeshInspector::GetVertexData( const Ogre::MeshPtr& mesh, UInt32 subMeshIndex )
{GUCEF_TRACE;

    if ( !mesh || subMeshIndex >= mesh->getNumSubMeshes() )
        return GUCEF_NULL;
    Ogre::SubMesh* subMesh = mesh->getSubMesh( (unsigned short) subMeshIndex );
    return subMesh->useSharedVertices ? mesh->sharedVertexData : subMesh->vertexData;
}

/*-------------------------------------------------------------------------*/

UInt32
CMeshInspector::CountTriangles( Ogre::RenderOperation::OperationType operationType, UInt32 indexCount )
{GUCEF_TRACE;

    switch ( operationType )
    {
        case Ogre::RenderOperation::OT_TRIANGLE_LIST:   return indexCount / 3;
        case Ogre::RenderOperation::OT_TRIANGLE_STRIP:
        case Ogre::RenderOperation::OT_TRIANGLE_FAN:    return indexCount >= 3 ? indexCount - 2 : 0;
        default:                                        return 0;
    }
}

/*-------------------------------------------------------------------------*/

void
CMeshInspector::GetStatistics( const Ogre::MeshPtr& mesh, SMeshStatistics& statistics )
{GUCEF_TRACE;

    statistics = SMeshStatistics();
    if ( !mesh )
        return;

    statistics.subMeshCount = mesh->getNumSubMeshes();
    statistics.lodLevelCount = mesh->getNumLodLevels();
    statistics.hasNormals = statistics.subMeshCount > 0;
    statistics.hasTangents = statistics.subMeshCount > 0;
    GUCEF::set< CORE::CString > materials;
    bool sharedCounted = false;
    for ( UInt32 i=0; i<statistics.subMeshCount; ++i )
    {
        Ogre::SubMesh* subMesh = mesh->getSubMesh( (unsigned short) i );
        const Ogre::VertexData* vertices = GetVertexData( mesh, i );
        if ( GUCEF_NULL != vertices )
        {
            if ( !subMesh->useSharedVertices || !sharedCounted )
                statistics.vertexCount += (UInt32) vertices->vertexCount;
            sharedCounted = sharedCounted || subMesh->useSharedVertices;
            const Ogre::VertexDeclaration* declaration = vertices->vertexDeclaration;
            statistics.hasNormals = statistics.hasNormals && GUCEF_NULL != declaration->findElementBySemantic( Ogre::VES_NORMAL );
            statistics.hasTangents = statistics.hasTangents && GUCEF_NULL != declaration->findElementBySemantic( Ogre::VES_TANGENT );
            statistics.hasVertexColours = statistics.hasVertexColours || GUCEF_NULL != declaration->findElementBySemantic( Ogre::VES_DIFFUSE );
            UInt32 sets = 0;
            while ( GUCEF_NULL != declaration->findElementBySemantic( Ogre::VES_TEXTURE_COORDINATES, (unsigned short) sets ) )
                ++sets;
            if ( sets > statistics.uvSetCount )
                statistics.uvSetCount = sets;
        }
        else
        {
            statistics.hasNormals = false;
            statistics.hasTangents = false;
        }
        statistics.triangleCount += CountTriangles( subMesh->operationType, (UInt32) subMesh->indexData->indexCount );
        materials.insert( subMesh->getMaterialName() );
    }

    const Ogre::SkeletonPtr skeleton = mesh->getSkeleton();
    statistics.hasSkeleton = (bool) skeleton;
    if ( skeleton )
    {
        statistics.boneCount = skeleton->getNumBones();
        statistics.animationCount = skeleton->getNumAnimations();
    }

    // the bounds of the vertices when they can be read, else those of the mesh (with Ogre's padding)
    Ogre::AxisAlignedBox bounds;
    if ( !GetGeometryBounds( mesh, bounds ) )
        bounds = mesh->getBounds();
    if ( bounds.isFinite() )
    {
        statistics.boundsMin[ 0 ] = bounds.getMinimum().x;
        statistics.boundsMin[ 1 ] = bounds.getMinimum().y;
        statistics.boundsMin[ 2 ] = bounds.getMinimum().z;
        statistics.boundsMax[ 0 ] = bounds.getMaximum().x;
        statistics.boundsMax[ 1 ] = bounds.getMaximum().y;
        statistics.boundsMax[ 2 ] = bounds.getMaximum().z;
    }
    statistics.boundingRadius = mesh->getBoundingSphereRadius();

    // the materials and their textures: a texture counts once however many materials use it
    GUCEF::set< CORE::CString > textures;
    GUCEF::set< CORE::CString >::const_iterator m = materials.begin();
    while ( m != materials.end() )
    {
        statistics.materialNames.push_back( *m );
        try
        {
            Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName( (*m).STL_String(), mesh->getGroup() );
            if ( material )
            {
                for ( unsigned short t=0; t<material->getNumTechniques(); ++t )
                {
                    Ogre::Technique* technique = material->getTechnique( t );
                    for ( unsigned short p=0; p<technique->getNumPasses(); ++p )
                    {
                        Ogre::Pass* pass = technique->getPass( p );
                        for ( unsigned short u=0; u<pass->getNumTextureUnitStates(); ++u )
                        {
                            Ogre::TextureUnitState* unit = pass->getTextureUnitState( u );
                            for ( unsigned int f=0; f<unit->getNumFrames(); ++f )
                                if ( !unit->getFrameTextureName( f ).empty() )
                                    textures.insert( unit->getFrameTextureName( f ) );
                        }
                    }
                }
            }
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CMeshInspector: The material " << *m << " cannot be read: " << e.getFullDescription() );
        }
        ++m;
    }
    statistics.materialCount = (UInt32) statistics.materialNames.size();

    GUCEF::set< CORE::CString >::const_iterator t = textures.begin();
    while ( t != textures.end() )
    {
        statistics.textureNames.push_back( *t );
        try
        {
            Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName( (*t).STL_String(), mesh->getGroup() );
            if ( !texture )
                texture = Ogre::TextureManager::getSingleton().load( (*t).STL_String(), mesh->getGroup() );
            if ( texture )
                statistics.textureMemory += texture->getSize();
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CMeshInspector: The texture " << *t << " cannot be loaded: " << e.getDescription() );
        }
        ++t;
    }
    statistics.textureCount = (UInt32) statistics.textureNames.size();
}

/*-------------------------------------------------------------------------*/

bool
CMeshInspector::GetGeometryBounds( const Ogre::MeshPtr& mesh, Ogre::AxisAlignedBox& bounds, GUCEF::vector< Ogre::AxisAlignedBox >* partBounds, CORE::CString::StringVector* partNames )
{GUCEF_TRACE;

    bounds.setNull();
    // only from copies in memory, which a mesh has when it was loaded with shadow buffers (CModelView does). A buffer read back
    // from the GPU right after it was made can still be undefined (Vulkan), and the "shadow" of a Vulkan buffer can be its
    // staging buffer, so the buffers themselves do not tell
    if ( !mesh || !mesh->isVertexBufferShadowed() )
        return false;
    try
    {
        // sub meshes on the shared vertex data of the mesh read it once
        GUCEF::set< const Ogre::VertexData* > done;
        GUCEF::vector< Float32 > positions;
        // the name of every submesh, for the parts that are asked for
        GUCEF::vector< CORE::CString > subMeshNames( mesh->getNumSubMeshes() );
        if ( GUCEF_NULL != partBounds )
        {
            Ogre::Mesh::SubMeshNameMap::const_iterator n = mesh->getSubMeshNameMap().begin();
            while ( n != mesh->getSubMeshNameMap().end() )
            {
                if ( n->second < subMeshNames.size() )
                    subMeshNames[ n->second ] = CORE::CString( n->first );
                ++n;
            }
        }
        for ( UInt32 i=0; i<(UInt32) mesh->getNumSubMeshes(); ++i )
        {
            const Ogre::VertexData* vertices = GetVertexData( mesh, i );
            if ( GUCEF_NULL == vertices || 0 == vertices->vertexCount || !done.insert( vertices ).second )
                continue;
            positions.clear();
            if ( !ReadFloatElement( *vertices, Ogre::VES_POSITION, 0, 3, positions ) )
                continue;
            Ogre::Vector3 low( positions[ 0 ], positions[ 1 ], positions[ 2 ] );
            Ogre::Vector3 high = low;
            for ( size_t v=3; v+2<positions.size(); v+=3 )
            {
                const Ogre::Vector3 position( positions[ v ], positions[ v+1 ], positions[ v+2 ] );
                low.makeFloor( position );
                high.makeCeil( position );
            }
            bounds.merge( Ogre::AxisAlignedBox( low, high ) );
            if ( GUCEF_NULL != partBounds )
            {
                partBounds->push_back( Ogre::AxisAlignedBox( low, high ) );
                if ( GUCEF_NULL != partNames )
                    partNames->push_back( subMeshNames[ i ] );
            }
        }
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CMeshInspector: The vertex positions of " << mesh->getName() << " cannot be read: " << e.getDescription() );
        bounds.setNull();
    }
    return !bounds.isNull();
}

/*-------------------------------------------------------------------------*/

bool
CMeshInspector::ReadSubMesh( const Ogre::MeshPtr& mesh, UInt32 subMeshIndex, SSubMeshGeometry& geometry )
{GUCEF_TRACE;

    geometry = SSubMeshGeometry();
    if ( !mesh || subMeshIndex >= mesh->getNumSubMeshes() )
        return false;
    try
    {
        Ogre::SubMesh* subMesh = mesh->getSubMesh( (unsigned short) subMeshIndex );
        const Ogre::VertexData* vertices = GetVertexData( mesh, subMeshIndex );
        if ( GUCEF_NULL == vertices || !ReadFloatElement( *vertices, Ogre::VES_POSITION, 0, 3, geometry.positions ) )
            return false;
        geometry.vertexCount = (UInt32) vertices->vertexCount;
        ReadFloatElement( *vertices, Ogre::VES_NORMAL, 0, 3, geometry.normals );
        ReadFloatElement( *vertices, Ogre::VES_TANGENT, 0, 3, geometry.tangents );
        ReadFloatElement( *vertices, Ogre::VES_TEXTURE_COORDINATES, 0, 2, geometry.texCoords );

        // the indices, as a list of triangles
        const Ogre::IndexData* indexData = subMesh->indexData;
        UInt32 indexCount = (UInt32) indexData->indexCount;
        GUCEF::vector< UInt32 > raw;
        if ( indexCount > 0 && indexData->indexBuffer )
        {
            const bool is32 = Ogre::HardwareIndexBuffer::IT_32BIT == indexData->indexBuffer->getType();
            const size_t indexSize = is32 ? 4 : 2;
            GUCEF::vector< UInt8 > bytes( indexCount * indexSize );
            indexData->indexBuffer->readData( indexData->indexStart * indexSize, indexCount * indexSize, &bytes[ 0 ] );
            raw.resize( indexCount );
            for ( UInt32 i=0; i<indexCount; ++i )
                raw[ i ] = is32 ? reinterpret_cast< const UInt32* >( &bytes[ 0 ] )[ i ] : (UInt32) reinterpret_cast< const UInt16* >( &bytes[ 0 ] )[ i ];
        }
        else
        {
            indexCount = geometry.vertexCount;
            raw.resize( indexCount );
            for ( UInt32 i=0; i<indexCount; ++i )
                raw[ i ] = i;
        }

        switch ( subMesh->operationType )
        {
            case Ogre::RenderOperation::OT_TRIANGLE_LIST:
            {
                geometry.indices = raw;
                geometry.indices.resize( ( raw.size() / 3 ) * 3 );
                break;
            }
            case Ogre::RenderOperation::OT_TRIANGLE_STRIP:
            {
                for ( size_t i=2; i<raw.size(); ++i )
                {
                    // every second triangle of a strip has the opposite winding
                    geometry.indices.push_back( 0 == ( i & 1 ) ? raw[ i-2 ] : raw[ i-1 ] );
                    geometry.indices.push_back( 0 == ( i & 1 ) ? raw[ i-1 ] : raw[ i-2 ] );
                    geometry.indices.push_back( raw[ i ] );
                }
                break;
            }
            case Ogre::RenderOperation::OT_TRIANGLE_FAN:
            {
                for ( size_t i=2; i<raw.size(); ++i )
                {
                    geometry.indices.push_back( raw[ 0 ] );
                    geometry.indices.push_back( raw[ i-1 ] );
                    geometry.indices.push_back( raw[ i ] );
                }
                break;
            }
            default:
            {
                break;  // lines and points have no triangles
            }
        }
        geometry.triangleCount = (UInt32) ( geometry.indices.size() / 3 );
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CMeshInspector: The geometry of sub mesh " << subMeshIndex << " cannot be read back: " << e.getDescription() );
        geometry = SSubMeshGeometry();
        return false;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMeshInspector::FormatByteCount( UInt64 bytes )
{GUCEF_TRACE;

    if ( bytes < 1024 )
        return CORE::UInt64ToString( bytes ) + " B";
    if ( bytes < 1024 * 1024 )
        return Number( (Float64) bytes / 1024.0, 1 ) + " KB";
    if ( bytes < 1024ULL * 1024ULL * 1024ULL )
        return Number( (Float64) bytes / ( 1024.0 * 1024.0 ), 2 ) + " MB";
    return Number( (Float64) bytes / ( 1024.0 * 1024.0 * 1024.0 ), 2 ) + " GB";
}

/*-------------------------------------------------------------------------*/

void
CMeshInspector::WriteToDataNode( const SMeshStatistics& statistics, CORE::CDataNode& node )
{GUCEF_TRACE;

    CORE::CDataNode* counts = node.AddChild( "Statistics" );
    counts->SetAttribute( "Vertices", CORE::UInt32ToString( statistics.vertexCount ) );
    counts->SetAttribute( "Triangles", CORE::UInt32ToString( statistics.triangleCount ) );
    counts->SetAttribute( "Submeshes", CORE::UInt32ToString( statistics.subMeshCount ) );
    counts->SetAttribute( "Bones", CORE::UInt32ToString( statistics.boneCount ) );
    counts->SetAttribute( "Animations", CORE::UInt32ToString( statistics.animationCount ) );
    counts->SetAttribute( "LodLevels", CORE::UInt32ToString( statistics.lodLevelCount ) );
    counts->SetAttribute( "Materials", CORE::UInt32ToString( statistics.materialCount ) );
    counts->SetAttribute( "Textures", CORE::UInt32ToString( statistics.textureCount ) );
    counts->SetAttribute( "TextureMemory", FormatByteCount( statistics.textureMemory ) );

    CORE::CDataNode* features = node.AddChild( "Features" );
    features->SetAttribute( "Normals", YesNo( statistics.hasNormals ) );
    features->SetAttribute( "Tangents", YesNo( statistics.hasTangents ) );
    features->SetAttribute( "VertexColours", YesNo( statistics.hasVertexColours ) );
    features->SetAttribute( "Skeleton", YesNo( statistics.hasSkeleton ) );
    features->SetAttribute( "UvSets", CORE::UInt32ToString( statistics.uvSetCount ) );

    CORE::CDataNode* bounds = node.AddChild( "Bounds" );
    bounds->SetAttribute( "Minimum", Number( statistics.boundsMin[ 0 ], 3 ) + ", " + Number( statistics.boundsMin[ 1 ], 3 ) + ", " + Number( statistics.boundsMin[ 2 ], 3 ) );
    bounds->SetAttribute( "Maximum", Number( statistics.boundsMax[ 0 ], 3 ) + ", " + Number( statistics.boundsMax[ 1 ], 3 ) + ", " + Number( statistics.boundsMax[ 2 ], 3 ) );
    bounds->SetAttribute( "Size", Number( statistics.boundsMax[ 0 ] - statistics.boundsMin[ 0 ], 3 ) + " x " + Number( statistics.boundsMax[ 1 ] - statistics.boundsMin[ 1 ], 3 )
        + " x " + Number( statistics.boundsMax[ 2 ] - statistics.boundsMin[ 2 ], 3 ) );
    bounds->SetAttribute( "Radius", Number( statistics.boundingRadius, 3 ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
