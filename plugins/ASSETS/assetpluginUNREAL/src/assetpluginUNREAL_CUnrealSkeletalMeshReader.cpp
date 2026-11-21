/*
 *  assetpluginUNREAL: Reads the native data of a skeletal mesh export: material slots, reference skeleton and editor LOD models
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#include "assetpluginUNREAL_UnrealVersions.h"
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#include "assetpluginUNREAL_CUnrealPropertyReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#include "assetpluginUNREAL_CUnrealObjectProperties.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H
#include "assetpluginUNREAL_CUnrealStripDataFlags.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHREADER_H
#include "assetpluginUNREAL_CUnrealSkeletalMeshReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETALMESHREADER_H ? */

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

/** Limits against damaged files: nothing in the corpus comes near them */
static const Int32 MaxLods = 32;
static const Int32 MaxSections = 4096;
static const Int32 MaxVerticesPerSection = 8 * 1024 * 1024;

/*-------------------------------------------------------------------------*/

CUnrealSkeletalMeshReader::CUnrealSkeletalMeshReader( void )
    : m_package( GUCEF_NULL )
    , m_bestPosition( 0 )
    , m_bestLine( 0 )
    , m_error()
    , m_notes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUnrealSkeletalMeshReader::GetError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CUnrealSkeletalMeshReader::GetNotes( void ) const
{GUCEF_TRACE;

    return m_notes;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealSkeletalMeshReader::Fail( const CUnrealArchiveReader& reader, UInt32 line )
{GUCEF_TRACE;

    // remembers the attempt that came furthest: it is the layout that is closest to the data
    if ( reader.GetPosition() >= m_bestPosition )
    {
        m_bestPosition = reader.GetPosition();
        m_bestLine = line;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CUnrealSkeletalMeshReader::ReadProperties( const CORE::CDataNode& root, CUnrealSkeletalMesh& mesh )
{GUCEF_TRACE;

    const CUnrealObjectProperties properties( root );
    const CORE::CDataNode* skeleton = properties.Find( "Skeleton" );
    if ( GUCEF_NULL != skeleton )
        mesh.skeletonAsset = CUnrealPackageIndex( skeleton->GetValue().AsInt32() );
    mesh.morphTargetCount = properties.GetCount( "MorphTargets" );
    mesh.socketCount = properties.GetCount( "Sockets" );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealSkeletalMeshReader::Read( CUnrealPackage& package, UInt32 exportIndex, CUnrealSkeletalMesh& mesh )
{GUCEF_TRACE;

    mesh.Clear();
    m_error = CORE::CString();
    m_notes.clear();
    m_package = &package;
    m_bestPosition = 0;
    m_bestLine = 0;
    if ( !package.IsExportRangeValid( exportIndex ) )
    {
        m_error = "The export " + CORE::UInt32ToString( exportIndex ) + " is not valid";
        return false;
    }
    mesh.name = package.GetExports()[ exportIndex ].objectName;

    CUnrealArchiveReader reader;
    if ( !package.OpenExportRange( exportIndex, reader ) )
    {
        m_error = "The data of the export " + mesh.name + " cannot be opened";
        return false;
    }

    // the tagged properties first
    CORE::CDataNode root( "root" );
    CUnrealPropertyReader properties( package );
    if ( !properties.ReadExport( reader, root, "SkeletalMesh" ) )
    {
        m_error = "The properties of " + mesh.name + " cannot be read: " + reader.GetError();
        return false;
    }
    ReadProperties( root, mesh );
    // the LOD infos: LODInfo, or SourceModels in releases that moved them; the screen size is a per platform value and the material map
    // redirects the material of the sections of a LOD
    GUCEF::vector< Float32 > screenSizes;
    GUCEF::vector< GUCEF::vector< Int32 > > materialMaps;
    {
        const CUnrealObjectProperties properties( root );
        // releases that have both keep the LOD infos in LODInfo and the import settings in SourceModels
        const CORE::CString infoName = properties.GetCount( "LODInfo" ) > 0 ? CORE::CString( "LODInfo" ) : CORE::CString( "SourceModels" );
        const UInt32 lodInfos = properties.GetCount( infoName );
        for ( UInt32 i=0; i<lodInfos && i<(UInt32) MaxLods; ++i )
        {
            const CORE::CString prefix = infoName + "/" + CORE::UInt32ToString( i );
            screenSizes.push_back( (Float32) properties.GetComponent( prefix + "/ScreenSize", "default", 0.0 ) );
            GUCEF::vector< Int32 > map;
            const UInt32 mapCount = properties.GetCount( prefix + "/LODMaterialMap" );
            for ( UInt32 m=0; m<mapCount; ++m )
                map.push_back( properties.GetInt32( prefix + "/LODMaterialMap/" + CORE::UInt32ToString( m ), 0 ) );
            materialMaps.push_back( map );
        }
    }

    // the native data: the guid flag of the object, then the strip flags
    if ( reader.ReadBool32() )
    {
        CUnrealGuid guid;
        reader.ReadGuid( guid );
    }
    CUnrealStripDataFlags stripFlags;
    if ( !stripFlags.Read( reader ) )
    {
        m_error = "The native data of " + mesh.name + " is truncated: " + reader.GetError();
        return false;
    }
    if ( stripFlags.IsEditorDataStripped() )
    {
        m_error = "The skeletal mesh " + mesh.name + " has no editor LOD models: its editor data is stripped (cooked content is not supported)";
        return false;
    }

    // the engine releases differ in a few fields: the combinations are tried, a wrong one fails on the structure
    const UInt64 nativeStart = reader.GetPosition();
    for ( UInt32 combination=0; combination<24; ++combination )
    {
        SVariant variant;
        variant.materialSlotWords = 7 - combination % 3;
        const UInt32 flags = combination / 3;
        variant.sectionRayTracingFlag = 0 == ( flags & 1 );
        variant.weights16 = 0 == ( flags & 2 );
        variant.modelStripFlags = 0 == ( flags & 4 );
        reader.ClearError();
        if ( !reader.Seek( nativeStart ) )
            continue;
        mesh.materialSlots.clear();
        mesh.skeleton.Clear();
        mesh.lods.clear();
        if ( !ReadHeader( package, reader, variant, mesh ) )
            continue;
        if ( !ReadLods( reader, variant, mesh ) )
            continue;
        UInt32 clamped = 0;
        for ( size_t l=0; l<mesh.lods.size(); ++l )
        {
            mesh.lods[ l ].screenSize = l < screenSizes.size() ? screenSizes[ l ] : 0.0f;
            for ( size_t s=0; s<mesh.lods[ l ].sections.size(); ++s )
            {
                CUnrealSkeletalMesh::SSection& section = mesh.lods[ l ].sections[ s ];
                if ( l < materialMaps.size() && section.materialIndex < materialMaps[ l ].size() && materialMaps[ l ][ section.materialIndex ] >= 0 )
                    section.materialIndex = (UInt32) materialMaps[ l ][ section.materialIndex ];
                // a section can name a slot the mesh does not have: the last slot is used, as the engine does
                if ( section.materialIndex >= mesh.materialSlots.size() && !mesh.materialSlots.empty() )
                {
                    section.materialIndex = (UInt32) mesh.materialSlots.size() - 1;
                    ++clamped;
                }
            }
        }
        if ( clamped > 0 )
            m_notes.push_back( CORE::UInt32ToString( clamped ) + " sections name a material slot that does not exist: the last slot is used" );
        m_notes.push_back( "layout: " + CORE::UInt32ToString( variant.materialSlotWords ) + " words of texture streaming data per material slot, section ray tracing flag " + ( variant.sectionRayTracingFlag ? "yes" : "no" ) +
                           ", " + ( variant.weights16 ? "16" : "8" ) + " bit weights, model strip flags " + ( variant.modelStripFlags ? "yes" : "no" ) );
        if ( mesh.morphTargetCount > 0 )
            m_notes.push_back( "the mesh declares " + CORE::UInt32ToString( mesh.morphTargetCount ) + " morph targets: not read" );
        return true;
    }
    mesh.materialSlots.clear();
    mesh.skeleton.Clear();
    mesh.lods.clear();
    m_error = "The native data of " + mesh.name + " does not match any known layout of the editor LOD models (the closest layout failed at line " + CORE::UInt32ToString( m_bestLine ) + " after " + CORE::UInt64ToString( m_bestPosition ) + " bytes)";
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealSkeletalMeshReader::ReadHeader( CUnrealPackage& package, CUnrealArchiveReader& reader, const SVariant& variant, CUnrealSkeletalMesh& mesh )
{GUCEF_TRACE;

    const bool doubles = package.GetVersions().IsAtLeast( UE5_LARGE_WORLD_COORDINATES );
    // the imported bounds: origin, box extent and sphere radius
    for ( UInt32 i=0; i<3; ++i )
        mesh.boundsOrigin[ i ] = doubles ? reader.ReadFloat64() : (Float64) reader.ReadFloat32();
    for ( UInt32 i=0; i<3; ++i )
        mesh.boundsExtent[ i ] = doubles ? reader.ReadFloat64() : (Float64) reader.ReadFloat32();
    mesh.boundsRadius = doubles ? reader.ReadFloat64() : (Float64) reader.ReadFloat32();

    const Int32 slotCount = reader.ReadInt32();
    if ( reader.HasError() || slotCount < 0 || slotCount > 4096 || !reader.CheckCount( (UInt64) slotCount, 16, "material slots" ) )
        return Fail( reader, __LINE__ );
    mesh.materialSlots.resize( (size_t) slotCount );
    for ( Int32 i=0; i<slotCount; ++i )
    {
        CUnrealSkeletalMesh::SMaterialSlot& slot = mesh.materialSlots[ (size_t) i ];
        slot.material = CUnrealPackageIndex( reader.ReadInt32() );
        if ( !package.GetNames().ReadName( reader, slot.slotName ) )
            return Fail( reader, __LINE__ );
        if ( reader.ReadBool32() && !package.GetNames().ReadName( reader, slot.importedSlotName ) )
            return Fail( reader, __LINE__ );
        // the texture streaming data of the slot (initialised flag, density override, four UV densities): its size differs
        // between the engine releases, the densities are not needed
        for ( UInt32 d=0; d<4; ++d )
            slot.uvDensities[ d ] = 0.0f;
        for ( UInt32 d=0; d<variant.materialSlotWords; ++d )
            reader.ReadInt32();
        if ( reader.HasError() )
            return Fail( reader, __LINE__ );
    }
    return mesh.skeleton.Read( reader, package.GetNames(), package.GetVersions(), 0 == ( package.GetSummary().packageFlags & 0x80000000 ) );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealSkeletalMeshReader::ReadLods( CUnrealArchiveReader& reader, const SVariant& variant, CUnrealSkeletalMesh& mesh )
{GUCEF_TRACE;

    if ( variant.modelStripFlags )
    {
        CUnrealStripDataFlags modelFlags;
        if ( !modelFlags.Read( reader ) || modelFlags.GetGlobalFlags() > 3 )
            return Fail( reader, __LINE__ );
    }
    const Int32 lodCount = reader.ReadInt32();
    if ( reader.HasError() || lodCount < 1 || lodCount > MaxLods || !reader.CheckCount( (UInt64) lodCount, 32, "LOD models" ) )
        return Fail( reader, __LINE__ );
    mesh.lods.resize( (size_t) lodCount );
    for ( Int32 l=0; l<lodCount; ++l )
    {
        mesh.lods[ (size_t) l ].screenSize = 0.0f;
        if ( !ReadLod( reader, variant, mesh.lods[ (size_t) l ] ) )
            return false;
        // the material indices of the sections are checked against the slots when the LOD infos are applied
        if ( mesh.materialSlots.empty() )
            return Fail( reader, __LINE__ );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealSkeletalMeshReader::ReadLod( CUnrealArchiveReader& reader, const SVariant& variant, CUnrealSkeletalMesh::SLod& lod )
{GUCEF_TRACE;

    CUnrealStripDataFlags lodFlags;
    if ( !lodFlags.Read( reader ) || lodFlags.GetGlobalFlags() > 3 )
        return Fail( reader, __LINE__ );

    const Int32 sectionCount = reader.ReadInt32();
    if ( reader.HasError() || sectionCount < 0 || sectionCount > MaxSections || !reader.CheckCount( (UInt64) sectionCount, 40, "LOD sections" ) )
        return Fail( reader, __LINE__ );
    lod.sections.resize( (size_t) sectionCount );
    for ( Int32 s=0; s<sectionCount; ++s )
        if ( !ReadSection( reader, variant, lod.sections[ (size_t) s ] ) )
            return false;

    // the user data of the sections in the source model: not needed, its size follows from the versions
    const Int32 userDataCount = reader.ReadInt32();
    if ( reader.HasError() || userDataCount < 0 || userDataCount > MaxSections )
        return Fail( reader, __LINE__ );
    const UInt64 userDataBytes = 2 + 4 + 1 + 4 + ( variant.sectionRayTracingFlag ? 4 : 0 ) + 4 + 4 + 2 + 20;
    if ( !reader.CheckCount( (UInt64) userDataCount, 4 + userDataBytes, "section user data" ) || !reader.Skip( (UInt64) userDataCount * ( 4 + userDataBytes ) ) )
        return Fail( reader, __LINE__ );

    const Int32 indexCount = reader.ReadInt32();
    if ( reader.HasError() || indexCount < 0 || 0 != indexCount % 3 || !reader.CheckCount( (UInt64) indexCount, 4, "LOD indices" ) )
        return Fail( reader, __LINE__ );
    lod.indices.resize( (size_t) indexCount );
    for ( Int32 i=0; i<indexCount; ++i )
        lod.indices[ (size_t) i ] = reader.ReadUInt32();

    const Int32 activeCount = reader.ReadInt32();
    if ( reader.HasError() || activeCount < 0 || !reader.CheckCount( (UInt64) activeCount, 2, "active bones" ) )
        return Fail( reader, __LINE__ );
    lod.activeBones.resize( (size_t) activeCount );
    for ( Int32 i=0; i<activeCount; ++i )
        lod.activeBones[ (size_t) i ] = reader.ReadUInt16();

    // imported mesh infos: not supported, the layout of their entries is not verified
    const Int32 importedInfoCount = reader.ReadInt32();
    if ( reader.HasError() || 0 != importedInfoCount )
        return Fail( reader, __LINE__ );

    reader.ReadInt32();     // size
    reader.ReadInt32();     // vertex count
    const Int32 requiredCount = reader.ReadInt32();
    if ( reader.HasError() || requiredCount < 0 || !reader.CheckCount( (UInt64) requiredCount, 2, "required bones" ) )
        return Fail( reader, __LINE__ );
    lod.requiredBones.resize( (size_t) requiredCount );
    for ( Int32 i=0; i<requiredCount; ++i )
        lod.requiredBones[ (size_t) i ] = reader.ReadUInt16();

    // raw point indices, the id of the raw mesh bulk data and its two flags, the map of vertices to imported vertices
    const Int32 rawPointCount = reader.ReadInt32();
    if ( reader.HasError() || rawPointCount < 0 || !reader.CheckCount( (UInt64) rawPointCount, 4, "raw point indices" ) || !reader.Skip( (UInt64) rawPointCount * 4 ) )
        return Fail( reader, __LINE__ );
    CORE::CString bulkDataId;
    if ( !reader.ReadString( bulkDataId ) )
        return Fail( reader, __LINE__ );
    reader.ReadBool32();
    reader.ReadBool32();
    const Int32 importMapCount = reader.ReadInt32();
    if ( reader.HasError() || importMapCount < 0 || !reader.CheckCount( (UInt64) importMapCount, 4, "imported vertex map" ) || !reader.Skip( (UInt64) importMapCount * 4 ) )
        return Fail( reader, __LINE__ );
    reader.ReadInt32();     // the highest imported vertex
    const Int32 uvSets = reader.ReadInt32();
    if ( reader.HasError() || uvSets < 1 || uvSets > (Int32) CUnrealSkeletalMesh::MaxUvSets )
        return Fail( reader, __LINE__ );
    lod.uvSetCount = (UInt32) uvSets;

    // skin weight profiles: not supported
    const Int32 profiles = reader.ReadInt32();
    if ( reader.HasError() || 0 != profiles )
        return Fail( reader, __LINE__ );

    // the triangles of the sections lie in the indices, their vertices in the sections
    for ( size_t s=0; s<lod.sections.size(); ++s )
    {
        const CUnrealSkeletalMesh::SSection& section = lod.sections[ s ];
        if ( (UInt64) section.baseIndex + (UInt64) section.triangleCount * 3 > lod.indices.size() )
            return Fail( reader, __LINE__ );
        for ( UInt32 i=0; i<section.triangleCount * 3; ++i )
        {
            const UInt32 index = lod.indices[ section.baseIndex + i ];
            if ( index < section.baseVertexIndex || index - section.baseVertexIndex >= section.vertices.size() )
                return Fail( reader, __LINE__ );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealSkeletalMeshReader::ReadSection( CUnrealArchiveReader& reader, const SVariant& variant, CUnrealSkeletalMesh::SSection& section )
{GUCEF_TRACE;

    CUnrealStripDataFlags sectionFlags;
    if ( !sectionFlags.Read( reader ) || sectionFlags.GetGlobalFlags() > 3 )
        return Fail( reader, __LINE__ );
    section.materialIndex = reader.ReadUInt16();
    section.baseIndex = reader.ReadUInt32();
    section.triangleCount = reader.ReadUInt32();
    reader.ReadBool32();    // recompute tangents
    reader.ReadUInt8();     // the vertex colour channel that masks tangent recomputation
    section.castShadow = reader.ReadBool32();
    if ( variant.sectionRayTracingFlag )
        reader.ReadBool32();
    section.baseVertexIndex = reader.ReadUInt32();

    // the vertices: position, tangent x and y, tangent z with the sign, four UV sets, colour, twelve bones and weights
    const Int32 vertexCount = reader.ReadInt32();
    const UInt64 vertexBytes = 12 + 12 + 12 + 16 + 32 + 4 + 24 + ( variant.weights16 ? 24 : 12 );
    if ( reader.HasError() || vertexCount < 0 || vertexCount > MaxVerticesPerSection || !reader.CheckCount( (UInt64) vertexCount, vertexBytes, "section vertices" ) )
        return Fail( reader, __LINE__ );
    section.vertices.resize( (size_t) vertexCount );
    for ( Int32 v=0; v<vertexCount; ++v )
    {
        CUnrealSkeletalMesh::SVertex& vertex = section.vertices[ (size_t) v ];
        for ( UInt32 c=0; c<3; ++c ) vertex.position[ c ] = reader.ReadFloat32();
        for ( UInt32 c=0; c<3; ++c ) vertex.tangentX[ c ] = reader.ReadFloat32();
        for ( UInt32 c=0; c<3; ++c ) vertex.tangentY[ c ] = reader.ReadFloat32();
        for ( UInt32 c=0; c<4; ++c ) vertex.tangentZ[ c ] = reader.ReadFloat32();
        for ( UInt32 u=0; u<CUnrealSkeletalMesh::MaxUvSets; ++u )
        {
            vertex.uv[ u ][ 0 ] = reader.ReadFloat32();
            vertex.uv[ u ][ 1 ] = reader.ReadFloat32();
        }
        // the colour is stored as B, G, R, A
        vertex.colour[ 2 ] = reader.ReadUInt8();
        vertex.colour[ 1 ] = reader.ReadUInt8();
        vertex.colour[ 0 ] = reader.ReadUInt8();
        vertex.colour[ 3 ] = reader.ReadUInt8();
        for ( UInt32 b=0; b<CUnrealSkeletalMesh::MaxInfluences; ++b ) vertex.bones[ b ] = reader.ReadUInt16();
        for ( UInt32 b=0; b<CUnrealSkeletalMesh::MaxInfluences; ++b )
            vertex.weights[ b ] = variant.weights16 ? reader.ReadUInt16() : (UInt16) ( reader.ReadUInt8() * 257 );
    }
    if ( reader.HasError() )
        return Fail( reader, __LINE__ );

    reader.ReadBool32();    // 16 bit bone indices
    const Int32 boneMapCount = reader.ReadInt32();
    if ( reader.HasError() || boneMapCount < 0 || boneMapCount > 65536 || !reader.CheckCount( (UInt64) boneMapCount, 2, "section bone map" ) )
        return Fail( reader, __LINE__ );
    section.boneMap.resize( (size_t) boneMapCount );
    for ( Int32 b=0; b<boneMapCount; ++b )
        section.boneMap[ (size_t) b ] = reader.ReadUInt16();
    const Int32 numVertices = reader.ReadInt32();
    const Int32 maxInfluences = reader.ReadInt32();
    if ( reader.HasError() || numVertices != vertexCount || maxInfluences < 1 || maxInfluences > (Int32) CUnrealSkeletalMesh::MaxInfluences )
        return Fail( reader, __LINE__ );
    section.maxInfluences = (UInt32) maxInfluences;

    // cloth mapping data per LOD: only empty lists are supported
    const Int32 clothLods = reader.ReadInt32();
    if ( reader.HasError() || clothLods < 0 || clothLods > MaxLods )
        return Fail( reader, __LINE__ );
    for ( Int32 c=0; c<clothLods; ++c )
        if ( 0 != reader.ReadInt32() || reader.HasError() )
            return Fail( reader, __LINE__ );
    reader.ReadInt16();     // the cloth asset of the section
    CUnrealGuid clothGuid;
    reader.ReadGuid( clothGuid );
    reader.ReadInt32();     // its LOD
    const Int32 overlapping = reader.ReadInt32();
    if ( reader.HasError() || overlapping < 0 || !reader.CheckCount( (UInt64) overlapping, 8, "overlapping vertices" ) )
        return Fail( reader, __LINE__ );
    for ( Int32 o=0; o<overlapping; ++o )
    {
        reader.ReadInt32();
        const Int32 listCount = reader.ReadInt32();
        if ( reader.HasError() || listCount < 0 || !reader.CheckCount( (UInt64) listCount, 4, "overlapping vertex list" ) || !reader.Skip( (UInt64) listCount * 4 ) )
            return Fail( reader, __LINE__ );
    }
    section.disabled = reader.ReadBool32();
    reader.ReadInt32();     // generate up to LOD
    reader.ReadInt32();     // original data section
    reader.ReadInt32();     // chunked parent section
    return !reader.HasError();
}

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
