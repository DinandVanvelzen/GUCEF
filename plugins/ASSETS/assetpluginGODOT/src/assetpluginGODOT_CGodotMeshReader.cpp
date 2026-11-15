/*
 *  assetpluginGODOT: the meshes of Godot resources
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
#include <string.h>

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#include "assetpluginGODOT_CGodotVariant.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H ? */

#include "assetpluginGODOT_CGodotMeshReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "godot";

static const Int64 FormatNormal = 2;
static const Int64 FormatTangent = 4;
static const Int64 FormatColour = 8;
static const Int64 FormatUv = 16;
static const Int64 FormatUv2 = 32;
static const Int64 FormatCustom0 = 64;
static const Int64 FormatBones = 1024;
static const Int64 FormatIndex = 4096;
static const Int64 Flag2dVertices = 33554432;
static const Int64 Flag8BoneWeights = 134217728;
static const Int64 FlagCompress = 536870912;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static Float32
ReadFloat( const UInt8* data )
{
    Float32 value;
    memcpy( &value, data, sizeof( value ) );
    return value;
}

/*-------------------------------------------------------------------------*/

static UInt32
ReadU16( const UInt8* data )
{
    return (UInt32) data[ 0 ] | ( (UInt32) data[ 1 ] << 8 );
}

/*-------------------------------------------------------------------------*/

static UInt32
CustomSize( UInt32 format )
{
    static const UInt32 sizes[ 8 ] = { 4, 4, 4, 8, 4, 8, 12, 16 };
    return sizes[ format & 7 ];
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CGodotMeshReader::DecodeOctahedron( Float32 x, Float32 y, Float32 out[ 3 ] )
{GUCEF_TRACE;

    const Float32 fx = x * 2.0f - 1.0f;
    const Float32 fy = y * 2.0f - 1.0f;
    Float32 nx = fx;
    Float32 ny = fy;
    const Float32 nz = 1.0f - (Float32) fabs( fx ) - (Float32) fabs( fy );
    Float32 t = -nz;
    t = t < 0.0f ? 0.0f : ( t > 1.0f ? 1.0f : t );
    nx += nx >= 0.0f ? -t : t;
    ny += ny >= 0.0f ? -t : t;
    const Float32 length = (Float32) sqrt( nx * nx + ny * ny + nz * nz );
    if ( length <= 0.0f )
    {
        out[ 0 ] = 0.0f; out[ 1 ] = 0.0f; out[ 2 ] = 1.0f;
        return;
    }
    out[ 0 ] = nx / length;
    out[ 1 ] = ny / length;
    out[ 2 ] = nz / length;
}

/*-------------------------------------------------------------------------*/

void
CGodotMeshReader::DecodeTangent( Float32 x, Float32 y, Float32 out[ 3 ], Float32& sign )
{GUCEF_TRACE;

    const Float32 stretched = y * 2.0f - 1.0f;
    sign = stretched >= 0.0f ? 1.0f : -1.0f;
    DecodeOctahedron( x, (Float32) fabs( stretched ), out );
}

/*-------------------------------------------------------------------------*/

static bool
ReadSurface( const CORE::CDataNode& surface, ASSETCONVERTER::CAssetSubMesh& subMesh, ASSETCONVERTER::CConversionReport& report, const CORE::CString& subject, UInt32 surfaceIndex )
{GUCEF_TRACE;

    const CORE::CString where = subject + " surface " + CORE::UInt32ToString( surfaceIndex );
    const Int64 format = CGodotVariant::GetInteger( CGodotVariant::FindMember( &surface, "format" ), 0 );
    const Int64 primitive = CGodotVariant::GetInteger( CGodotVariant::FindMember( &surface, "primitive" ), 3 );
    const UInt32 vertexCount = (UInt32) CGodotVariant::GetInteger( CGodotVariant::FindMember( &surface, "vertex_count" ), 0 );
    const UInt32 indexCount = (UInt32) CGodotVariant::GetInteger( CGodotVariant::FindMember( &surface, "index_count" ), 0 );

    if ( 3 != primitive && 4 != primitive )
    {
        report.AddSkipped( SourceName, where, "the surface is made of points or lines: it is not carried" );
        return false;
    }
    if ( 0 == vertexCount )
    {
        report.AddSkipped( SourceName, where, "the surface has no vertices" );
        return false;
    }
    if ( 0 != ( format & FlagCompress ) )
    {
        report.AddSkipped( SourceName, where, "the surface uses compressed attributes, which this importer does not read: save the mesh without compression" );
        return false;
    }

    CGodotVariant::TByteVector vertexData;
    CGodotVariant::TByteVector attributeData;
    CGodotVariant::TByteVector indexData;
    CGodotVariant::GetBytes( CGodotVariant::FindMember( &surface, "vertex_data" ), vertexData );
    CGodotVariant::GetBytes( CGodotVariant::FindMember( &surface, "attribute_data" ), attributeData );
    CGodotVariant::GetBytes( CGodotVariant::FindMember( &surface, "index_data" ), indexData );

    // the layout the format flags give
    const UInt32 positionStride = 0 != ( format & Flag2dVertices ) ? 8 : 12;
    const bool hasNormal = 0 != ( format & FormatNormal );
    const bool hasTangent = 0 != ( format & FormatTangent );
    const UInt32 normalStride = ( hasNormal ? 4 : 0 ) + ( hasTangent ? 4 : 0 );
    if ( vertexData.size() != (size_t) vertexCount * ( positionStride + normalStride ) )
    {
        report.AddSkipped( SourceName, where, "the vertex data does not have the size its format gives: the surface is not carried" );
        return false;
    }

    const bool hasColour = 0 != ( format & FormatColour );
    const bool hasUv = 0 != ( format & FormatUv );
    const bool hasUv2 = 0 != ( format & FormatUv2 );
    UInt32 attributeStride = ( hasColour ? 4 : 0 ) + ( hasUv ? 8 : 0 ) + ( hasUv2 ? 8 : 0 );
    for ( UInt32 c=0; c<4; ++c )
        if ( 0 != ( format & ( FormatCustom0 << c ) ) )
            attributeStride += CustomSize( (UInt32) ( ( format >> ( 13 + 3 * c ) ) & 7 ) );
    if ( attributeStride > 0 && attributeData.size() != (size_t) vertexCount * attributeStride )
    {
        report.AddSkipped( SourceName, where, "the attribute data does not have the size its format gives: the surface is not carried" );
        return false;
    }
    if ( 0 != ( format & FormatBones ) )
        report.AddInfo( SourceName, where, "the bone indices and weights of the surface are not carried: a skeleton is not part of a text scene" );

    // the vertices
    const UInt8* vertices = vertexData.empty() ? GUCEF_NULL : &vertexData[ 0 ];
    const UInt8* normals = vertices + (size_t) vertexCount * positionStride;
    subMesh.positions.resize( (size_t) vertexCount * 3 );
    if ( hasNormal )
        subMesh.normals.resize( (size_t) vertexCount * 3 );
    if ( hasTangent )
        subMesh.tangents.resize( (size_t) vertexCount * 4 );
    for ( UInt32 v=0; v<vertexCount; ++v )
    {
        const UInt8* position = vertices + (size_t) v * positionStride;
        subMesh.positions[ v * 3 ] = ReadFloat( position );
        subMesh.positions[ v * 3 + 1 ] = ReadFloat( position + 4 );
        subMesh.positions[ v * 3 + 2 ] = positionStride >= 12 ? ReadFloat( position + 8 ) : 0.0f;
        const UInt8* packed = normals + (size_t) v * normalStride;
        if ( hasNormal )
        {
            Float32 n[ 3 ];
            CGodotMeshReader::DecodeOctahedron( ReadU16( packed ) / 65535.0f, ReadU16( packed + 2 ) / 65535.0f, n );
            subMesh.normals[ v * 3 ] = n[ 0 ];
            subMesh.normals[ v * 3 + 1 ] = n[ 1 ];
            subMesh.normals[ v * 3 + 2 ] = n[ 2 ];
            packed += 4;
        }
        if ( hasTangent )
        {
            Float32 t[ 3 ];
            Float32 sign = 1.0f;
            CGodotMeshReader::DecodeTangent( ReadU16( packed ) / 65535.0f, ReadU16( packed + 2 ) / 65535.0f, t, sign );
            subMesh.tangents[ v * 4 ] = t[ 0 ];
            subMesh.tangents[ v * 4 + 1 ] = t[ 1 ];
            subMesh.tangents[ v * 4 + 2 ] = t[ 2 ];
            subMesh.tangents[ v * 4 + 3 ] = sign;
        }
    }

    // the attributes
    if ( attributeStride > 0 )
    {
        if ( hasColour )
            subMesh.colours.resize( (size_t) vertexCount * 4 );
        if ( hasUv )
            subMesh.uvSets[ 0 ].resize( (size_t) vertexCount * 2 );
        if ( hasUv2 )
            subMesh.uvSets[ 1 ].resize( (size_t) vertexCount * 2 );
        for ( UInt32 v=0; v<vertexCount; ++v )
        {
            const UInt8* attribute = &attributeData[ (size_t) v * attributeStride ];
            if ( hasColour )
            {
                for ( UInt32 c=0; c<4; ++c )
                    subMesh.colours[ v * 4 + c ] = attribute[ c ] / 255.0f;
                attribute += 4;
            }
            if ( hasUv )
            {
                subMesh.uvSets[ 0 ][ v * 2 ] = ReadFloat( attribute );
                subMesh.uvSets[ 0 ][ v * 2 + 1 ] = ReadFloat( attribute + 4 );
                attribute += 8;
            }
            if ( hasUv2 )
            {
                subMesh.uvSets[ 1 ][ v * 2 ] = ReadFloat( attribute );
                subMesh.uvSets[ 1 ][ v * 2 + 1 ] = ReadFloat( attribute + 4 );
            }
        }
    }

    // the indices
    if ( 0 != ( format & FormatIndex ) && indexCount > 0 )
    {
        const UInt32 indexSize = vertexCount <= 65535 ? 2 : 4;
        if ( indexData.size() != (size_t) indexCount * indexSize )
        {
            report.AddSkipped( SourceName, where, "the index data does not have the size its format gives: the surface is not carried" );
            return false;
        }
        subMesh.indices.resize( indexCount );
        for ( UInt32 i=0; i<indexCount; ++i )
        {
            const UInt8* index = &indexData[ (size_t) i * indexSize ];
            subMesh.indices[ i ] = 2 == indexSize ? ReadU16( index ) : ( ReadU16( index ) | ( ReadU16( index + 2 ) << 16 ) );
            if ( subMesh.indices[ i ] >= vertexCount )
            {
                report.AddSkipped( SourceName, where, "an index points past the vertices: the surface is not carried" );
                subMesh.indices.clear();
                return false;
            }
        }
    }
    else
    {
        subMesh.indices.resize( vertexCount );
        for ( UInt32 i=0; i<vertexCount; ++i )
            subMesh.indices[ i ] = i;
    }

    // a strip becomes a list of triangles; a list must be whole triangles
    if ( 4 == primitive )
    {
        ASSETCONVERTER::CAssetSubMesh::TUInt32Vector list;
        for ( size_t i=2; i<subMesh.indices.size(); ++i )
        {
            const UInt32 a = subMesh.indices[ i - 2 ];
            const UInt32 b = subMesh.indices[ i - 1 ];
            const UInt32 c = subMesh.indices[ i ];
            if ( a == b || b == c || a == c )
                continue;
            list.push_back( 0 == ( i & 1 ) ? a : b );
            list.push_back( 0 == ( i & 1 ) ? b : a );
            list.push_back( c );
        }
        subMesh.indices = list;
    }
    else
    if ( 0 != subMesh.indices.size() % 3 )
        subMesh.indices.resize( subMesh.indices.size() - subMesh.indices.size() % 3 );

    // clockwise front faces become counter clockwise ones
    subMesh.ReverseWinding();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotMeshReader::ReadSurfaces( const CORE::CDataNode& surfaces, ASSETCONVERTER::CAssetMesh& mesh, TNodeVector& surfaceMaterials, ASSETCONVERTER::CConversionReport& report, const CORE::CString& subject )
{GUCEF_TRACE;

    bool any = false;
    bool lodsReported = false;
    UInt32 surfaceIndex = 0;
    for ( const CORE::CDataNode* surface = surfaces.GetFirstChild(); GUCEF_NULL != surface; surface = surface->GetNext(), ++surfaceIndex )
    {
        if ( !CGodotVariant::IsType( surface, "Dictionary" ) )
            continue;
        any = true;
        ASSETCONVERTER::CAssetSubMesh subMesh;
        subMesh.materialIndex = -1;
        if ( !ReadSurface( *surface, subMesh, report, subject, surfaceIndex ) )
            subMesh = ASSETCONVERTER::CAssetSubMesh();
        mesh.subMeshes.push_back( subMesh );
        surfaceMaterials.push_back( CGodotVariant::FindMember( surface, "material" ) );

        const CORE::CDataNode* lods = CGodotVariant::FindMember( surface, "lods" );
        if ( !lodsReported && GUCEF_NULL != lods && lods->HasChildren() )
        {
            lodsReported = true;
            report.AddInfo( SourceName, subject, "the levels of detail generated for the mesh are not carried: the LOD policy of the pack decides" );
        }
        if ( GUCEF_NULL != CGodotVariant::FindMember( surface, "blend_shape_data" ) || GUCEF_NULL != CGodotVariant::FindMember( surface, "blend_shapes" ) )
            report.AddSkipped( SourceName, subject, "the blend shapes of the mesh are not carried" );
    }
    return any;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
