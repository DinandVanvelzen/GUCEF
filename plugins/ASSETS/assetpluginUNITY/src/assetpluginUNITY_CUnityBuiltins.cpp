/*
 *  assetpluginUNITY: the built-in resources of the Unity editor the importer knows
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

#include "assetpluginUNITY_CUnityBuiltins.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CUnityBuiltins::UrpLitShaderGuid = "933532a4fcc9baf4fa0491de14d08ed7";

const Int64 CUnityBuiltins::MaterialMainObjectId;
const Int64 CUnityBuiltins::TextureMainObjectId;
const Int64 CUnityBuiltins::ShaderMainObjectId;
const Int64 CUnityBuiltins::MonoScriptMainObjectId;
const Int64 CUnityBuiltins::PrefabMainObjectId;
const Int64 CUnityBuiltins::AvatarObjectId;
const Int64 CUnityBuiltins::DefaultMaterialId;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Adds a vertex: position, normal and UV (Unity's UV: v up; stored with v down as the model keeps them) */
UInt32
AddVertex( ASSETCONVERTER::CAssetSubMesh& subMesh, Float32 px, Float32 py, Float32 pz, Float32 nx, Float32 ny, Float32 nz, Float32 u, Float32 v )
{
    const UInt32 index = (UInt32) ( subMesh.positions.size() / 3 );
    subMesh.positions.push_back( px ); subMesh.positions.push_back( py ); subMesh.positions.push_back( pz );
    subMesh.normals.push_back( nx ); subMesh.normals.push_back( ny ); subMesh.normals.push_back( nz );
    subMesh.uvSets[ 0 ].push_back( u ); subMesh.uvSets[ 0 ].push_back( 1.0f - v );
    return index;
}

/** Adds the triangle so that its right-handed normal, (b - a) x (c - a), points along the normal of its vertices */
void
AddTriangle( ASSETCONVERTER::CAssetSubMesh& subMesh, UInt32 a, UInt32 b, UInt32 c )
{
    const Float32* p = &subMesh.positions[ 0 ];
    const Float32 e1[ 3 ] = { p[ b*3 ] - p[ a*3 ], p[ b*3+1 ] - p[ a*3+1 ], p[ b*3+2 ] - p[ a*3+2 ] };
    const Float32 e2[ 3 ] = { p[ c*3 ] - p[ a*3 ], p[ c*3+1 ] - p[ a*3+1 ], p[ c*3+2 ] - p[ a*3+2 ] };
    const Float32 n[ 3 ] = { e1[ 1 ] * e2[ 2 ] - e1[ 2 ] * e2[ 1 ], e1[ 2 ] * e2[ 0 ] - e1[ 0 ] * e2[ 2 ], e1[ 0 ] * e2[ 1 ] - e1[ 1 ] * e2[ 0 ] };
    const Float32* vn = &subMesh.normals[ a*3 ];
    subMesh.indices.push_back( a );
    if ( n[ 0 ] * vn[ 0 ] + n[ 1 ] * vn[ 1 ] + n[ 2 ] * vn[ 2 ] >= 0.0f )
    {
        subMesh.indices.push_back( b );
        subMesh.indices.push_back( c );
    }
    else
    {
        subMesh.indices.push_back( c );
        subMesh.indices.push_back( b );
    }
}

/** A quad of four vertices with one normal: corner 'origin', edges 'u' and 'v' */
void
AddQuad( ASSETCONVERTER::CAssetSubMesh& subMesh, const Float32 origin[ 3 ], const Float32 u[ 3 ], const Float32 v[ 3 ], const Float32 normal[ 3 ] )
{
    UInt32 corners[ 4 ];
    for ( UInt32 c=0; c<4; ++c )
    {
        const Float32 a = ( 1 == c || 2 == c ) ? 1.0f : 0.0f;
        const Float32 b = ( 2 == c || 3 == c ) ? 1.0f : 0.0f;
        corners[ c ] = AddVertex( subMesh, origin[ 0 ] + a * u[ 0 ] + b * v[ 0 ], origin[ 1 ] + a * u[ 1 ] + b * v[ 1 ], origin[ 2 ] + a * u[ 2 ] + b * v[ 2 ],
                                  normal[ 0 ], normal[ 1 ], normal[ 2 ], a, b );
    }
    AddTriangle( subMesh, corners[ 0 ], corners[ 1 ], corners[ 2 ] );
    AddTriangle( subMesh, corners[ 0 ], corners[ 2 ], corners[ 3 ] );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

Int64
CUnityBuiltins::GetShaderGraphMainObjectId( void )
{GUCEF_TRACE;

    return (Int64) -6465566751694194690LL;
}

/*-------------------------------------------------------------------------*/

bool
CUnityBuiltins::IsModelRootObjectId( Int64 fileId )
{GUCEF_TRACE;

    return (Int64) 919132149155446097LL == fileId || (Int64) -8679921383154817045LL == fileId || (Int64) -7511558181221131132LL == fileId;
}

/*-------------------------------------------------------------------------*/

const char*
CUnityBuiltins::GetShaderKindName( TShaderKind kind )
{GUCEF_TRACE;

    static const char* const names[] = { "standard", "standardSpecular", "legacyDiffuse", "urpLit", "hdrpLit", "shaderGraph", "custom",
                                         "builtinParticle", "builtinSprite", "builtinUnknown", "missing" };
    return kind < SHADERKIND_COUNT ? names[ kind ] : "";
}

/*-------------------------------------------------------------------------*/

CUnityBuiltins::TShaderKind
CUnityBuiltins::ClassifyBuiltinShader( Int64 fileId, CORE::CString& name )
{GUCEF_TRACE;

    switch ( fileId )
    {
        case 46: name = "Standard"; return SHADERKIND_STANDARD;
        case 45: name = "Standard (Specular setup)"; return SHADERKIND_STANDARDSPECULAR;
        case 7: name = "Legacy Shaders/Diffuse"; return SHADERKIND_LEGACYDIFFUSE;
        default: break;
    }
    if ( fileId >= 200 && fileId < 300 )
    {
        name = "built-in particle shader " + CORE::Int64ToString( fileId );
        return SHADERKIND_BUILTINPARTICLE;
    }
    if ( fileId >= 10700 && fileId < 10900 )
    {
        name = "built-in sprite, UI or sky shader " + CORE::Int64ToString( fileId );
        return SHADERKIND_BUILTINSPRITE;
    }
    name = "built-in shader " + CORE::Int64ToString( fileId );
    return SHADERKIND_BUILTINUNKNOWN;
}

/*-------------------------------------------------------------------------*/

bool
CUnityBuiltins::GetBuiltinMeshName( Int64 fileId, CORE::CString& name )
{GUCEF_TRACE;

    switch ( fileId )
    {
        case 10202: name = "Cube"; return true;
        case 10206: name = "Cylinder"; return true;
        case 10207: name = "Sphere"; return true;
        case 10208: name = "Capsule"; return true;
        case 10209: name = "Plane"; return true;
        case 10210: name = "Quad"; return true;
        default: return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CUnityBuiltins::MakeBuiltinMesh( Int64 fileId, ASSETCONVERTER::CAssetMesh& mesh )
{GUCEF_TRACE;

    mesh = ASSETCONVERTER::CAssetMesh();
    if ( !GetBuiltinMeshName( fileId, mesh.name ) )
        return false;
    ASSETCONVERTER::CAssetSubMesh subMesh;
    switch ( fileId )
    {
        case 10202:
        {
            // a cube of one unit around the origin, a face per side
            for ( UInt32 axis=0; axis<3; ++axis )
            {
                for ( Int32 side=-1; side<=1; side+=2 )
                {
                    Float32 normal[ 3 ] = { 0.0f, 0.0f, 0.0f };
                    normal[ axis ] = (Float32) side;
                    Float32 u[ 3 ] = { 0.0f, 0.0f, 0.0f };
                    Float32 v[ 3 ] = { 0.0f, 0.0f, 0.0f };
                    u[ ( axis + 1 ) % 3 ] = 1.0f;
                    v[ ( axis + 2 ) % 3 ] = 1.0f;
                    Float32 origin[ 3 ] = { -0.5f, -0.5f, -0.5f };
                    origin[ axis ] = 0.5f * (Float32) side;
                    AddQuad( subMesh, origin, u, v, normal );
                }
            }
            break;
        }
        case 10209:
        case 10210:
        {
            // the plane: ten by ten units on XZ facing up; the quad: one unit on XY facing -Z
            const bool plane = 10209 == fileId;
            const Float32 size = plane ? 10.0f : 1.0f;
            const Float32 origin[ 3 ] = { -0.5f * size, plane ? 0.0f : -0.5f * size, plane ? -0.5f * size : 0.0f };
            const Float32 u[ 3 ] = { size, 0.0f, 0.0f };
            const Float32 v[ 3 ] = { 0.0f, plane ? 0.0f : size, plane ? size : 0.0f };
            const Float32 normal[ 3 ] = { 0.0f, plane ? 1.0f : 0.0f, plane ? 0.0f : -1.0f };
            AddQuad( subMesh, origin, u, v, normal );
            break;
        }
        case 10207:
        {
            // a sphere of radius 0.5: 24 segments around, 16 rings
            const UInt32 segments = 24;
            const UInt32 rings = 16;
            const Float32 pi = 3.14159265358979f;
            for ( UInt32 r=0; r<=rings; ++r )
            {
                const Float32 theta = pi * (Float32) r / (Float32) rings;
                for ( UInt32 s=0; s<=segments; ++s )
                {
                    const Float32 phi = 2.0f * pi * (Float32) s / (Float32) segments;
                    const Float32 nx = sinf( theta ) * cosf( phi );
                    const Float32 ny = cosf( theta );
                    const Float32 nz = sinf( theta ) * sinf( phi );
                    AddVertex( subMesh, 0.5f * nx, 0.5f * ny, 0.5f * nz, nx, ny, nz, (Float32) s / (Float32) segments, 1.0f - (Float32) r / (Float32) rings );
                }
            }
            for ( UInt32 r=0; r<rings; ++r )
            {
                for ( UInt32 s=0; s<segments; ++s )
                {
                    const UInt32 a = r * ( segments + 1 ) + s;
                    const UInt32 b = a + segments + 1;
                    if ( r > 0 )
                        AddTriangle( subMesh, a, a + 1, b );
                    if ( r + 1 < rings )
                        AddTriangle( subMesh, a + 1, b + 1, b );
                }
            }
            break;
        }
        default:
            return false;
    }
    mesh.subMeshes.push_back( subMesh );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUnityBuiltins::MakeDefaultMaterial( ASSETCONVERTER::CAssetMaterial& material )
{GUCEF_TRACE;

    material = ASSETCONVERTER::CAssetMaterial();
    material.name = "Default-Material";
    for ( UInt32 c=0; c<4; ++c )
        material.baseColorFactor[ c ] = 1.0f;
    material.metallicFactor = 0.0f;
    material.roughnessFactor = 0.5f;
    material.source.shaderName = "Standard";
    material.source.properties.SetAttribute( "builtin", (Int64) DefaultMaterialId );
    material.source.properties.SetAttribute( "shaderKind", "builtinDefault" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
