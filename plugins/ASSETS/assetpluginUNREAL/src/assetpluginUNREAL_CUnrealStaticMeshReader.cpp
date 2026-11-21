/*
 *  assetpluginUNREAL: The StaticMesh export: material slots, bounds and the mesh descriptions of its source models
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

#include <string.h>
#include <math.h>

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#include "assetpluginUNREAL_CUnrealPropertyReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#include "assetpluginUNREAL_CUnrealObjectProperties.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H
#include "assetpluginUNREAL_CUnrealEditorBulkData.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H
#include "assetpluginUNREAL_CUnrealCompressedBuffer.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#include "assetpluginUNREAL_CUnrealLegacyBulkData.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYPAYLOADREADER_H
#include "assetpluginUNREAL_CUnrealLegacyPayloadReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYPAYLOADREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYPAYLOADREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H
#include "assetpluginUNREAL_CUnrealStripDataFlags.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H
#include "assetpluginUNREAL_CUnrealCustomVersionIds.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H
#include "assetpluginUNREAL_CUnrealStaticMeshReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTATICMESHREADER_H ? */

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

CUnrealStaticMeshReader::CUnrealStaticMeshReader( void )
    : m_error()
    , m_providerMissing( false )
    , m_withoutGeometry( false )
    , m_cancelled( false )
    , m_toolCode( 0 )
    , m_notes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString& CUnrealStaticMeshReader::GetError( void ) const { return m_error; }
bool CUnrealStaticMeshReader::IsProviderMissing( void ) const { return m_providerMissing; }
bool CUnrealStaticMeshReader::IsWithoutGeometry( void ) const { return m_withoutGeometry; }
bool CUnrealStaticMeshReader::IsCancelled( void ) const { return m_cancelled; }
Int32 CUnrealStaticMeshReader::GetToolCode( void ) const { return m_toolCode; }
const CORE::CString::StringVector& CUnrealStaticMeshReader::GetNotes( void ) const { return m_notes; }

/*-------------------------------------------------------------------------*/

bool
CUnrealStaticMeshReader::Fail( const CORE::CString& reason )
{GUCEF_TRACE;

    m_error = reason;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealStaticMeshReader::BuildLod( const CUnrealMeshDescription& description, const CUnrealSkeletalMesh::TMaterialSlotVector& slots, CUnrealStaticMesh::SLod& lod, CORE::CString& error )
{GUCEF_TRACE;

    lod = CUnrealStaticMesh::SLod();
    const CUnrealMeshDescription::SElement* vertices = description.FindElement( "Vertices" );
    const CUnrealMeshDescription::SElement* instances = description.FindElement( "VertexInstances" );
    const CUnrealMeshDescription::SElement* triangles = description.FindElement( "Triangles" );
    const CUnrealMeshDescription::SElement* groups = description.FindElement( "PolygonGroups" );
    if ( GUCEF_NULL == vertices || GUCEF_NULL == instances || GUCEF_NULL == triangles || vertices->channels.empty() || instances->channels.empty() || triangles->channels.empty() )
    {
        error = "The mesh description lacks the elements Vertices, VertexInstances or Triangles";
        return false;
    }
    const CUnrealMeshDescription::SChannel& vertexChannel = vertices->channels[ 0 ];
    const CUnrealMeshDescription::SChannel& instanceChannel = instances->channels[ 0 ];
    const CUnrealMeshDescription::SChannel& triangleChannel = triangles->channels[ 0 ];
    const CUnrealMeshDescription::SAttribute* position = vertexChannel.Find( "Position" );
    const CUnrealMeshDescription::SAttribute* instanceVertex = instanceChannel.Find( "VertexIndex" );
    const CUnrealMeshDescription::SAttribute* uv = instanceChannel.Find( "TextureCoordinate" );
    const CUnrealMeshDescription::SAttribute* normal = instanceChannel.Find( "Normal" );
    const CUnrealMeshDescription::SAttribute* tangent = instanceChannel.Find( "Tangent" );
    const CUnrealMeshDescription::SAttribute* sign = instanceChannel.Find( "BinormalSign" );
    const CUnrealMeshDescription::SAttribute* colour = instanceChannel.Find( "Color" );
    const CUnrealMeshDescription::SAttribute* corners = triangleChannel.Find( "VertexInstanceIndex" );
    const CUnrealMeshDescription::SAttribute* group = triangleChannel.Find( "PolygonGroupIndex" );
    if ( GUCEF_NULL == position || GUCEF_NULL == instanceVertex || GUCEF_NULL == corners || 3 != corners->valuesPerId || 1 != position->valuesPerId )
    {
        error = CORE::CString( "The mesh description lacks the attributes Position, VertexIndex or VertexInstanceIndex (" ) + ( GUCEF_NULL == position ? "no Position " : "" ) + ( GUCEF_NULL == instanceVertex ? "no VertexIndex " : "" ) + ( GUCEF_NULL == corners ? "no VertexInstanceIndex " : ( 3 != corners->valuesPerId ? "corners per triangle " + CORE::UInt32ToString( corners->valuesPerId ) : CORE::CString() ) ) + ")";
        return false;
    }

    // the names of the polygon groups tell the material slot of each
    GUCEF::vector< CORE::CString > groupNames;
    if ( GUCEF_NULL != groups && !groups->channels.empty() )
    {
        const CUnrealMeshDescription::SAttribute* names = groups->channels[ 0 ].Find( "ImportedMaterialSlotName" );
        if ( GUCEF_NULL != names && !names->names.empty() )
            groupNames = names->names[ 0 ];
    }

    UInt32 uvSets = GUCEF_NULL != uv ? (UInt32) uv->blocks.size() : 0;
    lod.sourceUvSetCount = uvSets;
    if ( uvSets > 4 )
        uvSets = 4;
    lod.uvSetCount = uvSets;

    // one section per polygon group; every vertex instance used by a section is a vertex of it
    typedef GUCEF::map< UInt32, UInt32 > TSectionMap;
    TSectionMap sectionOfGroup;
    GUCEF::vector< GUCEF::vector< Int32 > > remap;
    UInt32 agree = 0;
    for ( UInt32 t=0; t<triangleChannel.slotCount; ++t )
    {
        if ( !triangleChannel.IsAllocated( t ) )
            continue;
        const UInt32 groupId = GUCEF_NULL != group ? (UInt32) group->GetInt32( 0, t, 0 ) : 0;
        TSectionMap::iterator found = sectionOfGroup.find( groupId );
        if ( found == sectionOfGroup.end() )
        {
            CUnrealStaticMesh::SSection section;
            section.groupName = groupId < groupNames.size() ? groupNames[ groupId ] : CORE::CString::Empty;
            section.slot = -1;
            for ( size_t s=0; s<slots.size(); ++s )
                if ( !section.groupName.IsNULLOrEmpty() && ( slots[ s ].importedSlotName == section.groupName || slots[ s ].slotName == section.groupName ) )
                {
                    section.slot = (Int32) s;
                    break;
                }
            if ( section.slot < 0 && groupId < slots.size() )
                section.slot = (Int32) groupId;
            lod.sections.push_back( section );
            remap.push_back( GUCEF::vector< Int32 >( instanceChannel.slotCount, -1 ) );
            found = sectionOfGroup.insert( TSectionMap::value_type( groupId, (UInt32) lod.sections.size() - 1 ) ).first;
        }
        CUnrealStaticMesh::SSection& section = lod.sections[ (*found).second ];
        GUCEF::vector< Int32 >& map = remap[ (*found).second ];
        Float32 corner[ 3 ][ 3 ];
        Float32 normalSum[ 3 ] = { 0, 0, 0 };
        for ( UInt32 c=0; c<3; ++c )
        {
            const Int32 instanceId = corners->GetInt32( 0, t, c );
            if ( instanceId < 0 || (UInt32) instanceId >= instanceChannel.slotCount || !instanceChannel.IsAllocated( (UInt32) instanceId ) )
            {
                error = "A triangle refers to a vertex instance that does not exist";
                return false;
            }
            Int32 local = map[ (size_t) instanceId ];
            if ( local < 0 )
            {
                const Int32 vertexId = instanceVertex->GetInt32( 0, (UInt32) instanceId, 0 );
                if ( vertexId < 0 || (UInt32) vertexId >= vertexChannel.slotCount || !vertexChannel.IsAllocated( (UInt32) vertexId ) )
                {
                    error = "A vertex instance refers to a vertex that does not exist";
                    return false;
                }
                local = (Int32) ( section.positions.size() / 3 );
                map[ (size_t) instanceId ] = local;
                for ( UInt32 k=0; k<3; ++k )
                    section.positions.push_back( position->GetFloat( 0, (UInt32) vertexId, 0, k ) );
                for ( UInt32 k=0; k<3; ++k )
                    section.normals.push_back( GUCEF_NULL != normal ? normal->GetFloat( 0, (UInt32) instanceId, 0, k ) : ( 2 == k ? 1.0f : 0.0f ) );
                for ( UInt32 k=0; k<3; ++k )
                    section.tangents.push_back( GUCEF_NULL != tangent ? tangent->GetFloat( 0, (UInt32) instanceId, 0, k ) : ( 0 == k ? 1.0f : 0.0f ) );
                section.tangents.push_back( GUCEF_NULL != sign && sign->GetFloat( 0, (UInt32) instanceId, 0, 0 ) < 0.0f ? -1.0f : 1.0f );
                for ( UInt32 u=0; u<uvSets; ++u )
                {
                    section.uvSets[ u ].push_back( uv->GetFloat( u, (UInt32) instanceId, 0, 0 ) );
                    section.uvSets[ u ].push_back( uv->GetFloat( u, (UInt32) instanceId, 0, 1 ) );
                }
                if ( GUCEF_NULL != colour )
                    for ( UInt32 k=0; k<4; ++k )
                        section.colours.push_back( colour->GetFloat( 0, (UInt32) instanceId, 0, k ) );
            }
            section.indices.push_back( (UInt32) local );
            for ( UInt32 k=0; k<3; ++k )
            {
                corner[ c ][ k ] = section.positions[ (size_t) local * 3 + k ];
                normalSum[ k ] += section.normals[ (size_t) local * 3 + k ];
            }
        }
        const Float32 e1[ 3 ] = { corner[ 1 ][ 0 ] - corner[ 0 ][ 0 ], corner[ 1 ][ 1 ] - corner[ 0 ][ 1 ], corner[ 1 ][ 2 ] - corner[ 0 ][ 2 ] };
        const Float32 e2[ 3 ] = { corner[ 2 ][ 0 ] - corner[ 0 ][ 0 ], corner[ 2 ][ 1 ] - corner[ 0 ][ 1 ], corner[ 2 ][ 2 ] - corner[ 0 ][ 2 ] };
        const Float32 geometric[ 3 ] = { e1[ 1 ] * e2[ 2 ] - e1[ 2 ] * e2[ 1 ], e1[ 2 ] * e2[ 0 ] - e1[ 0 ] * e2[ 2 ], e1[ 0 ] * e2[ 1 ] - e1[ 1 ] * e2[ 0 ] };
        if ( geometric[ 0 ] * normalSum[ 0 ] + geometric[ 1 ] * normalSum[ 1 ] + geometric[ 2 ] * normalSum[ 2 ] > 0.0f )
            ++agree;
        ++lod.triangleCount;
    }
    // sections that carry colours: all or none (a vertex of a section without the attribute cannot happen, the attribute is per mesh)
    for ( size_t s=0; s<lod.sections.size(); ++s )
    {
        bool white = true;
        for ( size_t k=0; k<lod.sections[ s ].colours.size() && white; ++k )
            white = lod.sections[ s ].colours[ k ] >= 0.999f;
        if ( white )
            lod.sections[ s ].colours.clear();
        lod.vertexCount += (UInt32) ( lod.sections[ s ].positions.size() / 3 );
    }
    lod.windingAgreement = lod.triangleCount > 0 ? (Float32) agree / (Float32) lod.triangleCount : 0.0f;
    if ( 0 == lod.triangleCount )
    {
        error = "The mesh description has no triangles";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealStaticMeshReader::Read( CUnrealPackage& package, UInt32 exportIndex, CIUnrealOodleProvider* provider, const ASSETCONVERTER::CConversionReport* cancellation, CUnrealStaticMesh& out )
{GUCEF_TRACE;

    out = CUnrealStaticMesh();
    m_error.Clear();
    m_notes.clear();
    m_providerMissing = false;
    m_withoutGeometry = false;
    m_cancelled = false;
    m_toolCode = 0;
    if ( exportIndex >= package.GetExports().size() )
        return Fail( "The export does not exist" );
    out.name = package.GetExports()[ exportIndex ].objectName;

    CUnrealArchiveReader reader;
    if ( !package.OpenExportRange( exportIndex, reader ) )
        return Fail( "The data of the static mesh export cannot be opened" );
    CORE::CDataNode root( "root" );
    CUnrealPropertyReader properties( package );
    if ( !properties.ReadExport( reader, root, "StaticMesh" ) )
        return Fail( "The properties of the static mesh cannot be read: " + reader.GetError() );
    const CUnrealObjectProperties values( root );

    const UInt32 slotCount = values.GetCount( "StaticMaterials" );
    for ( UInt32 s=0; s<slotCount && s<4096; ++s )
    {
        const CORE::CString base = "StaticMaterials/" + CORE::UInt32ToString( s ) + "/";
        CUnrealSkeletalMesh::SMaterialSlot slot;
        slot.material = CUnrealPackageIndex( values.GetInt32( base + "MaterialInterface", 0 ) );
        slot.slotName = values.GetString( base + "MaterialSlotName" );
        slot.importedSlotName = values.GetString( base + "ImportedMaterialSlotName" );
        for ( UInt32 d=0; d<4; ++d )
            slot.uvDensities[ d ] = (Float32) values.GetDouble( base + "UVChannelData/LocalUVDensities", 0.0 );
        out.materialSlots.push_back( slot );
    }
    static const char* const components[ 3 ] = { "x", "y", "z" };
    for ( UInt32 c=0; c<3; ++c )
    {
        out.boundsOrigin[ c ] = values.GetComponent( "ExtendedBounds/Origin", components[ c ], 0.0 );
        out.boundsExtent[ c ] = values.GetComponent( "ExtendedBounds/BoxExtent", components[ c ], 0.0 );
    }

    const UInt32 modelCount = values.GetCount( "SourceModels" );
    if ( 0 == modelCount )
        return Fail( "The static mesh has no source models: it holds no editor data (a cooked package) or is of the UE 4 raw mesh form, which this reader does not read" );
    if ( !package.GetVersions().IsAtLeast( UE5_INITIAL_VERSION ) )
    {
        // UE 4: the mesh descriptions are legacy bulk data in the native data of the mesh itself
        if ( !ReadUe4SourceModels( package, reader, out.materialSlots, modelCount, out ) )
            return false;
        for ( UInt32 m=0; m<(UInt32) out.lods.size(); ++m )
            out.lods[ m ].screenSize = (Float32) values.GetDouble( "SourceModels/" + CORE::UInt32ToString( m ) + "/ScreenSize/default", 0.0 );
        return true;
    }
    for ( UInt32 m=0; m<modelCount; ++m )
    {
        const CORE::CString base = "SourceModels/" + CORE::UInt32ToString( m ) + "/";
        const CUnrealPackageIndex reference( values.GetInt32( base + "StaticMeshDescriptionBulkData", 0 ) );
        if ( !reference.IsExport() || reference.ToExportIndex() >= package.GetExports().size() )
        {
            m_notes.push_back( "Source model " + CORE::UInt32ToString( m ) + " has no mesh description in this package: left out" );
            continue;
        }
        // the object holds an editor bulk data record behind its (empty) properties
        const UInt32 holder = reference.ToExportIndex();
        CUnrealArchiveReader holderReader;
        CORE::CDataNode holderRoot( "root" );
        CUnrealPropertyReader holderProperties( package );
        if ( !package.OpenExportRange( holder, holderReader ) || !holderProperties.ReadExport( holderReader, holderRoot, package.GetClassName( package.GetExports()[ holder ] ) ) )
            return Fail( "The mesh description object of source model " + CORE::UInt32ToString( m ) + " cannot be read" );
        if ( holderReader.ReadBool32() )
        {
            CUnrealGuid guid;
            holderReader.ReadGuid( guid );
        }
        CUnrealEditorBulkData bulk;
        if ( !bulk.Read( holderReader, package.GetVersions() ) )
            return Fail( "The bulk data record of source model " + CORE::UInt32ToString( m ) + " cannot be read: " + holderReader.GetError() );
        CUnrealPayloadReference payload;
        bulk.ToReference( package.HasTrailer() ? &package.GetTrailer() : GUCEF_NULL, payload );
        if ( CUnrealPayloadReference::KIND_NONE == payload.kind || CUnrealPayloadReference::KIND_EMPTY == payload.kind )
        {
            m_notes.push_back( "Source model " + CORE::UInt32ToString( m ) + " holds no data (a level the engine makes itself): left out" );
            continue;
        }
        CUnrealCompressedBuffer buffer( package, provider );
        buffer.SetCancellationSource( cancellation );
        CORE::CDynamicBuffer data;
        if ( !buffer.Open( payload ) || !buffer.ReadAll( data ) )
        {
            m_providerMissing = buffer.IsProviderMissing();
            m_cancelled = buffer.IsCancelled();
            m_toolCode = buffer.GetToolCode();
            return Fail( buffer.GetError() );
        }
        CUnrealMeshDescription description;
        if ( !description.Read( static_cast< const UInt8* >( data.GetConstBufferPtr() ), data.GetDataSize() ) )
            return Fail( "The mesh description of source model " + CORE::UInt32ToString( m ) + " cannot be read: " + description.GetError() );
        CUnrealStaticMesh::SLod lod;
        CORE::CString error;
        if ( !BuildLod( description, out.materialSlots, lod, error ) )
            return Fail( "Source model " + CORE::UInt32ToString( m ) + ": " + error );
        lod.screenSize = (Float32) values.GetDouble( base + "ScreenSize/default", 0.0 );
        if ( lod.sourceUvSetCount > lod.uvSetCount )
            m_notes.push_back( "Source model " + CORE::UInt32ToString( m ) + " has " + CORE::UInt32ToString( lod.sourceUvSetCount ) + " UV sets: the first four are read" );
        out.lods.push_back( lod );
    }
    if ( out.lods.empty() )
    {
        m_withoutGeometry = true;
        return Fail( "The static mesh has no geometry data in the package: none of its source models holds a mesh description (stripped or empty)" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealStaticMeshReader::ReadUe4SourceModels( CUnrealPackage& package, CUnrealArchiveReader& reader, const CUnrealSkeletalMesh::TMaterialSlotVector& slots, UInt32 modelCount, CUnrealStaticMesh& out )
{GUCEF_TRACE;

    const CUnrealVersionContext& versions = package.GetVersions();
    const Int32 editorVersion = versions.GetCustomVersion( CUnrealCustomVersionIds::EditorObjectVersion() );
    if ( !versions.IsAtLeast( UE4_ADDED_PACKAGE_OWNER ) || editorVersion < 38 )
        return Fail( "The source models of this UE 4 static mesh (UE4 file version " + CORE::Int32ToString( versions.GetUe4Version() ) + ", editor object version " + CORE::Int32ToString( editorVersion ) +
                     ") are in the raw mesh form of the engines before 4.25, which this reader does not read" );

    // the native data after the properties: the object guid, the strip flags, the cooked flag, the body setup and the navigation
    // collision, two values of the editor data, the lighting guid, the sockets, then per source model a flag and its mesh description
    if ( reader.ReadBool32() )
    {
        CUnrealGuid guid;
        reader.ReadGuid( guid );
    }
    CUnrealStripDataFlags strip;
    if ( !strip.Read( reader ) )
        return Fail( "The strip flags of the static mesh cannot be read: " + reader.GetError() );
    if ( strip.IsEditorDataStripped() )
    {
        m_withoutGeometry = true;
        return Fail( "The static mesh has no geometry data in the package: its editor data is stripped" );
    }
    const UInt32 cooked = reader.ReadUInt32();
    reader.ReadInt32();     // the body setup
    reader.ReadInt32();     // the navigation collision
    reader.Skip( 8 );       // two values of the editor data that the mesh does not need
    CUnrealGuid lightingGuid;
    reader.ReadGuid( lightingGuid );
    const Int32 socketCount = reader.ReadInt32();
    if ( reader.HasError() || cooked > 1 || socketCount < 0 || socketCount > 65536 )
        return Fail( "The native data of the static mesh does not match the layout of UE 4.25 to 4.27 (cooked flag " + CORE::UInt32ToString( cooked ) + ", " + CORE::Int32ToString( socketCount ) + " sockets)" );
    if ( 0 != cooked )
        return Fail( "The static mesh is cooked: its source models are not in the package" );
    reader.Skip( (UInt64) socketCount * 4 );

    for ( UInt32 m=0; m<modelCount; ++m )
    {
        const UInt32 valid = reader.ReadUInt32();
        if ( reader.HasError() || valid > 1 )
            return Fail( "The native data of the static mesh does not match the layout of UE 4.25 to 4.27 at source model " + CORE::UInt32ToString( m ) );
        if ( 0 == valid )
        {
            m_notes.push_back( "Source model " + CORE::UInt32ToString( m ) + " holds no mesh description (a level the engine makes itself): left out" );
            continue;
        }
        CUnrealLegacyBulkData bulk;
        if ( !bulk.Read( reader, versions ) )
            return Fail( "The bulk data header of source model " + CORE::UInt32ToString( m ) + " cannot be read: " + reader.GetError() );
        const UInt64 headerEnd = reader.GetPosition();
        CUnrealPayloadReference payload;
        bulk.ToReference( package.GetSummary().bulkDataStartOffset, headerEnd, package.GetSource().GetSize(), payload );
        if ( 0 != ( bulk.flags & CUnrealLegacyBulkData::FLAG_FORCE_INLINE_PAYLOAD ) && bulk.sizeOnDisk > 0 && !reader.Skip( (UInt64) bulk.sizeOnDisk ) )
            return Fail( "The inline mesh description of source model " + CORE::UInt32ToString( m ) + " lies outside the static mesh" );
        CUnrealGuid guid;
        reader.ReadGuid( guid );
        const UInt32 guidIsHash = reader.ReadUInt32();
        if ( reader.HasError() || guidIsHash > 1 )
            return Fail( "The native data of the static mesh does not match the layout of UE 4.25 to 4.27 behind the bulk data of source model " + CORE::UInt32ToString( m ) );
        if ( CUnrealPayloadReference::KIND_EMPTY == payload.kind )
        {
            m_notes.push_back( "Source model " + CORE::UInt32ToString( m ) + " holds no data: left out" );
            continue;
        }
        CORE::CDynamicBuffer data;
        bool codecMissing = false;
        CORE::CString error;
        if ( !CUnrealLegacyPayloadReader::Read( package, payload, data, codecMissing, error ) )
        {
            m_providerMissing = codecMissing;
            return Fail( "The mesh description of source model " + CORE::UInt32ToString( m ) + " cannot be read: " + error );
        }
        CUnrealMeshDescription description;
        if ( !description.ReadUe4Layout( static_cast< const UInt8* >( data.GetConstBufferPtr() ), data.GetDataSize() ) )
            return Fail( "The mesh description of source model " + CORE::UInt32ToString( m ) + " cannot be read: " + description.GetError() );
        CUnrealStaticMesh::SLod lod;
        if ( !BuildLod( description, slots, lod, error ) )
            return Fail( "Source model " + CORE::UInt32ToString( m ) + ": " + error );
        if ( lod.sourceUvSetCount > lod.uvSetCount )
            m_notes.push_back( "Source model " + CORE::UInt32ToString( m ) + " has " + CORE::UInt32ToString( lod.sourceUvSetCount ) + " UV sets: the first four are read" );
        out.lods.push_back( lod );
    }
    if ( out.lods.empty() )
    {
        m_withoutGeometry = true;
        return Fail( "The static mesh has no geometry data in the package: none of its source models holds a mesh description (stripped or empty)" );
    }
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
