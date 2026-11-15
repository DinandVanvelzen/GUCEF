/*
 *  assetpluginGODOT: the primitive meshes of Godot
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#include "assetpluginGODOT_CGodotVariant.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H ? */

#include "assetpluginGODOT_CGodotPrimitiveMeshes.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const Float64 Pi = 3.14159265358979323846;

namespace {

/** The vertices and triangles of a mesh in the making */
struct SBuilder
{
    ASSETCONVERTER::CAssetSubMesh& subMesh;

    explicit SBuilder( ASSETCONVERTER::CAssetSubMesh& target ) : subMesh( target ) {}

    UInt32 Add( Float32 x, Float32 y, Float32 z, Float32 nx, Float32 ny, Float32 nz, Float32 u, Float32 v )
    {
        subMesh.positions.push_back( x ); subMesh.positions.push_back( y ); subMesh.positions.push_back( z );
        subMesh.normals.push_back( nx ); subMesh.normals.push_back( ny ); subMesh.normals.push_back( nz );
        subMesh.uvSets[ 0 ].push_back( u ); subMesh.uvSets[ 0 ].push_back( v );
        return (UInt32) ( subMesh.positions.size() / 3 ) - 1;
    }

    void Triangle( UInt32 a, UInt32 b, UInt32 c )
    {
        subMesh.indices.push_back( a ); subMesh.indices.push_back( b ); subMesh.indices.push_back( c );
    }

    void Quad( UInt32 a, UInt32 b, UInt32 c, UInt32 d )
    {
        Triangle( a, b, c );
        Triangle( a, c, d );
    }

    /** Turns the triangles over when most of them face against the normals of their vertices */
    void FixWinding( void )
    {
        Int32 agree = 0;
        for ( size_t i=0; i+2<subMesh.indices.size(); i+=3 )
        {
            const Float32* a = &subMesh.positions[ subMesh.indices[ i ] * 3 ];
            const Float32* b = &subMesh.positions[ subMesh.indices[ i + 1 ] * 3 ];
            const Float32* c = &subMesh.positions[ subMesh.indices[ i + 2 ] * 3 ];
            const Float32 e1[ 3 ] = { b[ 0 ] - a[ 0 ], b[ 1 ] - a[ 1 ], b[ 2 ] - a[ 2 ] };
            const Float32 e2[ 3 ] = { c[ 0 ] - a[ 0 ], c[ 1 ] - a[ 1 ], c[ 2 ] - a[ 2 ] };
            const Float32 n[ 3 ] = { e1[ 1 ] * e2[ 2 ] - e1[ 2 ] * e2[ 1 ], e1[ 2 ] * e2[ 0 ] - e1[ 0 ] * e2[ 2 ], e1[ 0 ] * e2[ 1 ] - e1[ 1 ] * e2[ 0 ] };
            const Float32* vn = &subMesh.normals[ subMesh.indices[ i ] * 3 ];
            const Float32 dot = n[ 0 ] * vn[ 0 ] + n[ 1 ] * vn[ 1 ] + n[ 2 ] * vn[ 2 ];
            if ( dot > 1e-12f ) ++agree; else if ( dot < -1e-12f ) --agree;
        }
        if ( agree < 0 )
            subMesh.ReverseWinding();
    }
};

/** A point of the outline that is turned around the Y axis: the distance from the axis, the height, the direction of the normal and the V coordinate */
struct SOutlinePoint
{
    Float32 radius;
    Float32 y;
    Float32 normalRadius;
    Float32 normalY;
    Float32 v;
    bool breakAfter;
};

typedef GUCEF::vector< SOutlinePoint > TOutline;

void
AddOutlinePoint( TOutline& outline, Float32 radius, Float32 y, Float32 normalRadius, Float32 normalY, Float32 v, bool breakAfter )
{
    SOutlinePoint point;
    point.radius = radius; point.y = y; point.normalRadius = normalRadius; point.normalY = normalY; point.v = v; point.breakAfter = breakAfter;
    outline.push_back( point );
}

void
Revolve( const TOutline& outline, UInt32 segments, ASSETCONVERTER::CAssetSubMesh& subMesh )
{
    SBuilder builder( subMesh );
    if ( segments < 3 )
        segments = 3;
    GUCEF::vector< UInt32 > rowStart;
    for ( size_t p=0; p<outline.size(); ++p )
    {
        rowStart.push_back( (UInt32) ( subMesh.positions.size() / 3 ) );
        for ( UInt32 s=0; s<=segments; ++s )
        {
            const Float64 angle = 2.0 * Pi * s / segments;
            const Float32 c = (Float32) cos( angle );
            const Float32 sn = (Float32) sin( angle );
            const SOutlinePoint& point = outline[ p ];
            builder.Add( point.radius * c, point.y, point.radius * sn, point.normalRadius * c, point.normalY, point.normalRadius * sn, (Float32) s / segments, point.v );
        }
    }
    for ( size_t p=0; p+1<outline.size(); ++p )
    {
        if ( outline[ p ].breakAfter )
            continue;
        for ( UInt32 s=0; s<segments; ++s )
        {
            const UInt32 a = rowStart[ p ] + s;
            const UInt32 b = rowStart[ p + 1 ] + s;
            if ( outline[ p ].radius > 0.0f )
                builder.Triangle( a, b, a + 1 );
            if ( outline[ p + 1 ].radius > 0.0f )
                builder.Triangle( a + 1, b, b + 1 );
        }
    }
    builder.FixWinding();
}

Float32
Num( const CORE::CDataNode& section, const char* key, Float64 fallback )
{
    return (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, key ), fallback );
}

UInt32
Count( const CORE::CDataNode& section, const char* key, Int64 fallback, Int64 minimum )
{
    Int64 value = CGodotVariant::GetInteger( CGodotVariant::FindProperty( section, key ), fallback );
    if ( value < minimum )
        value = minimum;
    if ( value > 1024 )
        value = 1024;
    return (UInt32) value;
}

void
BuildBox( const CORE::CDataNode& section, ASSETCONVERTER::CAssetSubMesh& subMesh )
{
    Float32 size[ 3 ] = { 1.0f, 1.0f, 1.0f };
    CGodotVariant::GetVector3( CGodotVariant::FindProperty( section, "size" ), size );
    SBuilder builder( subMesh );
    // per face: the normal, then the two axes whose cross product is the normal
    static const Float32 faces[ 6 ][ 9 ] = {
        {  1, 0, 0,   0, 0,-1,   0, 1, 0 }, { -1, 0, 0,   0, 0, 1,   0, 1, 0 },
        {  0, 1, 0,   1, 0, 0,   0, 0,-1 }, {  0,-1, 0,   1, 0, 0,   0, 0, 1 },
        {  0, 0, 1,   1, 0, 0,   0, 1, 0 }, {  0, 0,-1,  -1, 0, 0,   0, 1, 0 } };
    for ( UInt32 f=0; f<6; ++f )
    {
        const Float32* n = &faces[ f ][ 0 ];
        const Float32* u = &faces[ f ][ 3 ];
        const Float32* v = &faces[ f ][ 6 ];
        UInt32 corner[ 4 ];
        static const Float32 signs[ 4 ][ 2 ] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
        for ( UInt32 c=0; c<4; ++c )
        {
            Float32 p[ 3 ];
            for ( UInt32 a=0; a<3; ++a )
                p[ a ] = 0.5f * size[ a ] * ( n[ a ] + signs[ c ][ 0 ] * u[ a ] + signs[ c ][ 1 ] * v[ a ] );
            corner[ c ] = builder.Add( p[ 0 ], p[ 1 ], p[ 2 ], n[ 0 ], n[ 1 ], n[ 2 ], signs[ c ][ 0 ] > 0 ? 1.0f : 0.0f, signs[ c ][ 1 ] > 0 ? 0.0f : 1.0f );
        }
        builder.Quad( corner[ 0 ], corner[ 1 ], corner[ 2 ], corner[ 3 ] );
    }
    builder.FixWinding();
}

void
BuildPlane( const CORE::CDataNode& section, bool isQuad, ASSETCONVERTER::CAssetSubMesh& subMesh )
{
    Float32 size[ 2 ] = { isQuad ? 1.0f : 2.0f, isQuad ? 1.0f : 2.0f };
    CGodotVariant::GetVector2( CGodotVariant::FindProperty( section, "size" ), size );
    Float32 centre[ 3 ] = { 0.0f, 0.0f, 0.0f };
    CGodotVariant::GetVector3( CGodotVariant::FindProperty( section, "center_offset" ), centre );
    const UInt32 widthSteps = isQuad ? 1 : Count( section, "subdivide_width", 0, 0 ) + 1;
    const UInt32 depthSteps = isQuad ? 1 : Count( section, "subdivide_depth", 0, 0 ) + 1;
    const Int32 orientation = (Int32) CGodotVariant::GetInteger( CGodotVariant::FindProperty( section, "orientation" ), isQuad ? 2 : 1 );

    // the normal and the two axes of the plane
    Float32 n[ 3 ] = { 0, 1, 0 };
    Float32 u[ 3 ] = { 1, 0, 0 };
    Float32 v[ 3 ] = { 0, 0, -1 };
    if ( 0 == orientation )
    {
        n[ 0 ] = 1; n[ 1 ] = 0; n[ 2 ] = 0;
        u[ 0 ] = 0; u[ 1 ] = 0; u[ 2 ] = -1;
        v[ 0 ] = 0; v[ 1 ] = 1; v[ 2 ] = 0;
    }
    else
    if ( 2 == orientation )
    {
        n[ 0 ] = 0; n[ 1 ] = 0; n[ 2 ] = 1;
        u[ 0 ] = 1; u[ 1 ] = 0; u[ 2 ] = 0;
        v[ 0 ] = 0; v[ 1 ] = 1; v[ 2 ] = 0;
    }
    SBuilder builder( subMesh );
    for ( UInt32 j=0; j<=depthSteps; ++j )
    {
        for ( UInt32 i=0; i<=widthSteps; ++i )
        {
            const Float32 s = (Float32) i / widthSteps;
            const Float32 t = (Float32) j / depthSteps;
            Float32 p[ 3 ];
            for ( UInt32 a=0; a<3; ++a )
                p[ a ] = centre[ a ] + size[ 0 ] * ( s - 0.5f ) * u[ a ] + size[ 1 ] * ( t - 0.5f ) * v[ a ];
            builder.Add( p[ 0 ], p[ 1 ], p[ 2 ], n[ 0 ], n[ 1 ], n[ 2 ], s, t );
        }
    }
    for ( UInt32 j=0; j<depthSteps; ++j )
        for ( UInt32 i=0; i<widthSteps; ++i )
        {
            const UInt32 a = j * ( widthSteps + 1 ) + i;
            builder.Quad( a, a + 1, a + widthSteps + 2, a + widthSteps + 1 );
        }
    builder.FixWinding();
}

void
BuildSphere( const CORE::CDataNode& section, ASSETCONVERTER::CAssetSubMesh& subMesh, bool& hemisphere )
{
    const Float32 radius = Num( section, "radius", 0.5 );
    const Float32 height = Num( section, "height", 1.0 );
    const UInt32 segments = Count( section, "radial_segments", 64, 3 );
    const UInt32 rings = Count( section, "rings", 32, 2 );
    hemisphere = CGodotVariant::GetBool( CGodotVariant::FindProperty( section, "is_hemisphere" ), false );
    const Float32 halfHeight = 0.5f * height;
    const UInt32 steps = hemisphere ? rings / 2 : rings;
    const Float64 span = hemisphere ? 0.5 * Pi : Pi;
    TOutline outline;
    for ( UInt32 i=0; i<=steps; ++i )
    {
        const Float64 angle = span * i / steps;
        const Float32 sn = (Float32) sin( angle );
        const Float32 cs = (Float32) cos( angle );
        const Float32 nr = halfHeight > 0.0f && radius > 0.0f ? sn / radius : sn;
        const Float32 ny = halfHeight > 0.0f ? cs / halfHeight : cs;
        const Float32 length = (Float32) sqrt( nr * nr + ny * ny );
        AddOutlinePoint( outline, radius * sn, halfHeight * cs, length > 0.0f ? nr / length : 0.0f, length > 0.0f ? ny / length : 1.0f, (Float32) i / steps, false );
    }
    Revolve( outline, segments, subMesh );
}

void
BuildCylinder( const CORE::CDataNode& section, ASSETCONVERTER::CAssetSubMesh& subMesh )
{
    const Float32 topRadius = Num( section, "top_radius", 0.5 );
    const Float32 bottomRadius = Num( section, "bottom_radius", 0.5 );
    const Float32 height = Num( section, "height", 2.0 );
    const UInt32 segments = Count( section, "radial_segments", 64, 3 );
    const UInt32 rings = Count( section, "rings", 4, 1 );
    const bool capTop = CGodotVariant::GetBool( CGodotVariant::FindProperty( section, "cap_top" ), true );
    const bool capBottom = CGodotVariant::GetBool( CGodotVariant::FindProperty( section, "cap_bottom" ), true );
    const Float32 half = 0.5f * height;

    // the side normal leans with the difference of the radii
    const Float32 lean = bottomRadius - topRadius;
    const Float32 length = (Float32) sqrt( height * height + lean * lean );
    const Float32 nr = length > 0.0f ? height / length : 1.0f;
    const Float32 ny = length > 0.0f ? lean / length : 0.0f;

    TOutline outline;
    if ( capTop )
    {
        AddOutlinePoint( outline, 0.0f, half, 0.0f, 1.0f, 0.0f, false );
        AddOutlinePoint( outline, topRadius, half, 0.0f, 1.0f, 1.0f, true );
    }
    for ( UInt32 i=0; i<=rings; ++i )
    {
        const Float32 t = (Float32) i / rings;
        AddOutlinePoint( outline, topRadius + ( bottomRadius - topRadius ) * t, half - height * t, nr, ny, t, false );
    }
    if ( capBottom )
    {
        outline.back().breakAfter = true;
        AddOutlinePoint( outline, bottomRadius, -half, 0.0f, -1.0f, 1.0f, false );
        AddOutlinePoint( outline, 0.0f, -half, 0.0f, -1.0f, 0.0f, false );
    }
    Revolve( outline, segments, subMesh );
}

void
BuildCapsule( const CORE::CDataNode& section, ASSETCONVERTER::CAssetSubMesh& subMesh )
{
    const Float32 radius = Num( section, "radius", 0.5 );
    Float32 height = Num( section, "height", 2.0 );
    if ( height < 2.0f * radius )
        height = 2.0f * radius;
    const UInt32 segments = Count( section, "radial_segments", 64, 3 );
    const UInt32 rings = Count( section, "rings", 8, 1 );
    const Float32 middle = 0.5f * ( height - 2.0f * radius );

    TOutline outline;
    for ( UInt32 i=0; i<=rings; ++i )
    {
        const Float64 angle = 0.5 * Pi * i / rings;
        AddOutlinePoint( outline, radius * (Float32) sin( angle ), middle + radius * (Float32) cos( angle ), (Float32) sin( angle ), (Float32) cos( angle ), 0.5f * (Float32) i / rings, false );
    }
    for ( UInt32 i=0; i<=rings; ++i )
    {
        const Float64 angle = 0.5 * Pi + 0.5 * Pi * i / rings;
        AddOutlinePoint( outline, radius * (Float32) sin( angle ), -middle + radius * (Float32) cos( angle ), (Float32) sin( angle ), (Float32) cos( angle ), 0.5f + 0.5f * (Float32) i / rings, false );
    }
    Revolve( outline, segments, subMesh );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CGodotPrimitiveMeshes::IsPrimitiveType( const CORE::CString& type )
{GUCEF_TRACE;

    return "BoxMesh" == type || "PlaneMesh" == type || "QuadMesh" == type || "SphereMesh" == type || "CylinderMesh" == type || "CapsuleMesh" == type;
}

/*-------------------------------------------------------------------------*/

bool
CGodotPrimitiveMeshes::Build( const CORE::CString& type, const CORE::CDataNode& section, ASSETCONVERTER::CAssetMesh& mesh )
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetSubMesh subMesh;
    subMesh.materialIndex = -1;
    bool hemisphere = false;
    if ( "BoxMesh" == type )
        BuildBox( section, subMesh );
    else
    if ( "PlaneMesh" == type )
        BuildPlane( section, false, subMesh );
    else
    if ( "QuadMesh" == type )
        BuildPlane( section, true, subMesh );
    else
    if ( "SphereMesh" == type )
        BuildSphere( section, subMesh, hemisphere );
    else
    if ( "CylinderMesh" == type )
        BuildCylinder( section, subMesh );
    else
    if ( "CapsuleMesh" == type )
        BuildCapsule( section, subMesh );
    else
        return false;
    mesh.subMeshes.push_back( subMesh );
    return true;
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
