/*
 *  AssetConverterLib: the built-in post-processor that makes tangents for normal mapped submeshes
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
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#include "AssetConverterLib_CGenerateTangentsPostProcessor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CGenerateTangentsPostProcessor::TypeName = "GenerateTangents";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
Normalize3( Float64* v )
{
    const Float64 length = sqrt( v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ] + v[ 2 ] * v[ 2 ] );
    if ( length > 1e-12 )
    {
        v[ 0 ] /= length;
        v[ 1 ] /= length;
        v[ 2 ] /= length;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGenerateTangentsPostProcessor::CGenerateTangentsPostProcessor( void )
    : CIAssetPostProcessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGenerateTangentsPostProcessor::~CGenerateTangentsPostProcessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CGenerateTangentsPostProcessor::GetName( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CGenerateTangentsPostProcessor::GetVersion( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CGenerateTangentsPostProcessor::GenerateTangents( CAssetSubMesh& subMesh, UInt32 uvSet )
{GUCEF_TRACE;

    const UInt32 vertexCount = subMesh.GetVertexCount();
    if ( 0 == vertexCount || !subMesh.HasNormals() || uvSet >= CAssetSubMesh::MaxUvSets || subMesh.uvSets[ uvSet ].size() != (size_t) vertexCount * 2 )
        return false;

    const CAssetSubMesh::TFloatVector& p = subMesh.positions;
    const CAssetSubMesh::TFloatVector& uv = subMesh.uvSets[ uvSet ];
    GUCEF::vector< Float64 > uDirections( (size_t) vertexCount * 3, 0.0 );
    GUCEF::vector< Float64 > vDirections( (size_t) vertexCount * 3, 0.0 );
    for ( size_t t=0; t+2<subMesh.indices.size(); t+=3 )
    {
        const UInt32 i0 = subMesh.indices[ t ], i1 = subMesh.indices[ t+1 ], i2 = subMesh.indices[ t+2 ];
        if ( i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount )
            continue;
        const Float64 e1[ 3 ] = { p[ i1*3 ] - p[ i0*3 ], p[ i1*3+1 ] - p[ i0*3+1 ], p[ i1*3+2 ] - p[ i0*3+2 ] };
        const Float64 e2[ 3 ] = { p[ i2*3 ] - p[ i0*3 ], p[ i2*3+1 ] - p[ i0*3+1 ], p[ i2*3+2 ] - p[ i0*3+2 ] };
        const Float64 du1 = uv[ i1*2 ] - uv[ i0*2 ], dv1 = uv[ i1*2+1 ] - uv[ i0*2+1 ];
        const Float64 du2 = uv[ i2*2 ] - uv[ i0*2 ], dv2 = uv[ i2*2+1 ] - uv[ i0*2+1 ];
        const Float64 r = du1 * dv2 - du2 * dv1;
        if ( r > -1e-20 && r < 1e-20 )
            continue;
        const Float64 f = 1.0 / r;
        for ( UInt32 c=0; c<3; ++c )
        {
            const Float64 u = ( e1[ c ] * dv2 - e2[ c ] * dv1 ) * f;
            const Float64 v = ( e2[ c ] * du1 - e1[ c ] * du2 ) * f;
            uDirections[ i0*3+c ] += u; uDirections[ i1*3+c ] += u; uDirections[ i2*3+c ] += u;
            vDirections[ i0*3+c ] += v; vDirections[ i1*3+c ] += v; vDirections[ i2*3+c ] += v;
        }
    }

    subMesh.tangents.assign( (size_t) vertexCount * 4, 0.0f );
    for ( UInt32 i=0; i<vertexCount; ++i )
    {
        Float64 n[ 3 ] = { subMesh.normals[ i*3 ], subMesh.normals[ i*3+1 ], subMesh.normals[ i*3+2 ] };
        Normalize3( n );
        const Float64* u = &uDirections[ i*3 ];
        const Float64 along = n[ 0 ] * u[ 0 ] + n[ 1 ] * u[ 1 ] + n[ 2 ] * u[ 2 ];
        Float64 t[ 3 ] = { u[ 0 ] - n[ 0 ] * along, u[ 1 ] - n[ 1 ] * along, u[ 2 ] - n[ 2 ] * along };
        if ( t[ 0 ] * t[ 0 ] + t[ 1 ] * t[ 1 ] + t[ 2 ] * t[ 2 ] < 1e-20 )
        {
            // no direction from the UVs: any direction perpendicular to the normal
            const Float64 a[ 3 ] = { ( n[ 0 ] > 0.9 || n[ 0 ] < -0.9 ) ? 0.0 : 1.0, ( n[ 0 ] > 0.9 || n[ 0 ] < -0.9 ) ? 1.0 : 0.0, 0.0 };
            const Float64 d = a[ 0 ] * n[ 0 ] + a[ 1 ] * n[ 1 ];
            t[ 0 ] = a[ 0 ] - n[ 0 ] * d; t[ 1 ] = a[ 1 ] - n[ 1 ] * d; t[ 2 ] = -n[ 2 ] * d;
        }
        Normalize3( t );
        const Float64 b[ 3 ] = { n[ 1 ] * t[ 2 ] - n[ 2 ] * t[ 1 ], n[ 2 ] * t[ 0 ] - n[ 0 ] * t[ 2 ], n[ 0 ] * t[ 1 ] - n[ 1 ] * t[ 0 ] };
        const Float64* v = &vDirections[ i*3 ];
        const Float64 handedness = ( b[ 0 ] * v[ 0 ] + b[ 1 ] * v[ 1 ] + b[ 2 ] * v[ 2 ] ) < 0.0 ? -1.0 : 1.0;
        subMesh.tangents[ i*4 ] = (Float32) t[ 0 ];
        subMesh.tangents[ i*4+1 ] = (Float32) t[ 1 ];
        subMesh.tangents[ i*4+2 ] = (Float32) t[ 2 ];
        subMesh.tangents[ i*4+3 ] = (Float32) handedness;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGenerateTangentsPostProcessor::Process( CAssetScene& scene, const CORE::CDataNode& /* settings */, CConversionReport& report )
{GUCEF_TRACE;

    UInt32 generated = 0;
    for ( size_t m=0; m<scene.meshes.size(); ++m )
    {
        for ( size_t s=0; s<scene.meshes[ m ].subMeshes.size(); ++s )
        {
            CAssetSubMesh& subMesh = scene.meshes[ m ].subMeshes[ s ];
            if ( subMesh.HasTangents() || subMesh.materialIndex < 0 || (size_t) subMesh.materialIndex >= scene.materials.size() )
                continue;
            const CAssetTextureReference& normalMap = scene.materials[ subMesh.materialIndex ].normalTexture;
            if ( !normalMap.IsUsed() )
                continue;
            if ( GenerateTangents( subMesh, normalMap.texCoord ) )
                ++generated;
            else
                report.AddWarning( TypeName, scene.meshes[ m ].name + "/" + CORE::UInt32ToString( (UInt32) s ), "has a normal map but no normals or no UV set " + CORE::UInt32ToString( normalMap.texCoord ) + ": no tangents" );
        }
    }
    if ( generated > 0 )
        report.AddApproximation( TypeName, "tangents", "the tangents of " + CORE::UInt32ToString( generated ) + " submesh(es) are generated per triangle from the UVs, not with MikkTSpace" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
