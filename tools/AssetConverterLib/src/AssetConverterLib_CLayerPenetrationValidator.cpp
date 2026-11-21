/*
 *  AssetConverterLib: the built-in validator of the layers of a character: body surface through the garments
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
#include <stdio.h>

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

#include "AssetConverterLib_CLayerPenetrationValidator.h"

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

const CORE::CString CLayerPenetrationValidator::TypeName = "LayerPenetration";
const CORE::CString CLayerPenetrationValidator::PartAttribute = "gucefCharacterPart";
const CORE::CString CLayerPenetrationValidator::BodyPart = "body";
const CORE::CString CLayerPenetrationValidator::GarmentPart = "garment";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A vertex in the rest pose: its place, its normal and its strongest joint (-1 when it has none) */
struct SRestVertex
{
    Float32 p[ 3 ];
    Float32 n[ 3 ];
    Int32 joint;
};
typedef GUCEF::vector< SRestVertex > TRestVertexVector;
typedef GUCEF::vector< UInt32 > TUInt32Vector;

/** The node of the first level of a part: the node itself when it shows a mesh, the first level of its LOD group, or the first child with a mesh */
Int32
FirstLevelNode( const CAssetScene& scene, UInt32 node )
{
    const CAssetNode& part = scene.nodes[ node ];
    if ( part.mesh >= 0 )
        return (Int32) node;
    if ( part.lodGroup >= 0 && (size_t) part.lodGroup < scene.lodGroups.size() && !scene.lodGroups[ part.lodGroup ].levels.empty() )
        return (Int32) scene.lodGroups[ part.lodGroup ].levels[ 0 ].node;
    for ( size_t c=0; c<part.children.size(); ++c )
        if ( part.children[ c ] < scene.nodes.size() && scene.nodes[ part.children[ c ] ].mesh >= 0 )
            return (Int32) part.children[ c ];
    return -1;
}

/** The vertices of the mesh of a node in the rest pose of its skin (or in the place of the node without a skin), with the triangles */
void
RestPose( const CAssetScene& scene, UInt32 node, UInt32 hiddenMask, TRestVertexVector& vertices, TUInt32Vector& triangles, UInt32& hiddenVertices )
{
    const CAssetNode& owner = scene.nodes[ node ];
    if ( owner.mesh < 0 || (size_t) owner.mesh >= scene.meshes.size() )
        return;
    const CAssetMesh& mesh = scene.meshes[ owner.mesh ];
    const bool skinned = owner.skin >= 0 && (size_t) owner.skin < scene.skins.size();
    GUCEF::vector< MATH::TFloat32Matrix4D > jointMatrices;
    if ( skinned )
    {
        const CAssetSkin& skin = scene.skins[ owner.skin ];
        for ( size_t j=0; j<skin.joints.size(); ++j )
            jointMatrices.push_back( j < skin.inverseBindMatrices.size() ? scene.GetWorldMatrix( skin.joints[ j ] ) * skin.inverseBindMatrices[ j ] : scene.GetWorldMatrix( skin.joints[ j ] ) );
    }
    const MATH::TFloat32Matrix4D placement = scene.GetWorldMatrix( node );
    for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
    {
        const CAssetSubMesh& subMesh = mesh.subMeshes[ s ];
        if ( 0 != ( subMesh.hiddenBy & hiddenMask ) )
        {
            // not drawn while the outfit that hides it is worn
            hiddenVertices += subMesh.GetVertexCount();
            continue;
        }
        const UInt32 first = (UInt32) vertices.size();
        const UInt32 count = subMesh.GetVertexCount();
        const UInt32 influences = subMesh.influencesPerVertex;
        const bool useSkin = skinned && subMesh.IsSkinned() && subMesh.joints.size() == (size_t) count * influences && subMesh.weights.size() == (size_t) count * influences;
        for ( UInt32 v=0; v<count; ++v )
        {
            Float32 m[ 3 ][ 4 ] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
            SRestVertex rest;
            rest.joint = -1;
            Float32 strongest = 0.0f;
            Float32 total = 0.0f;
            if ( useSkin )
            {
                for ( UInt32 k=0; k<influences; ++k )
                {
                    const Float32 w = subMesh.weights[ (size_t) v * influences + k ];
                    const UInt32 joint = subMesh.joints[ (size_t) v * influences + k ];
                    if ( w <= 0.0f || joint >= jointMatrices.size() )
                        continue;
                    for ( UInt32 r=0; r<3; ++r )
                        for ( UInt32 c=0; c<4; ++c )
                            m[ r ][ c ] += w * jointMatrices[ joint ]( r, c );
                    total += w;
                    if ( w > strongest )
                    {
                        strongest = w;
                        rest.joint = (Int32) scene.skins[ owner.skin ].joints[ joint ];
                    }
                }
            }
            if ( total <= 0.0f )
            {
                for ( UInt32 r=0; r<3; ++r )
                    for ( UInt32 c=0; c<4; ++c )
                        m[ r ][ c ] = placement( r, c );
                total = 1.0f;
            }
            const Float32* p = &subMesh.positions[ (size_t) v * 3 ];
            for ( UInt32 r=0; r<3; ++r )
                rest.p[ r ] = ( m[ r ][ 0 ] * p[ 0 ] + m[ r ][ 1 ] * p[ 1 ] + m[ r ][ 2 ] * p[ 2 ] + m[ r ][ 3 ] ) / total;
            rest.n[ 0 ] = rest.n[ 1 ] = rest.n[ 2 ] = 0.0f;
            if ( subMesh.HasNormals() )
            {
                const Float32* n = &subMesh.normals[ (size_t) v * 3 ];
                for ( UInt32 r=0; r<3; ++r )
                    rest.n[ r ] = m[ r ][ 0 ] * n[ 0 ] + m[ r ][ 1 ] * n[ 1 ] + m[ r ][ 2 ] * n[ 2 ];
                const Float32 length = sqrtf( rest.n[ 0 ] * rest.n[ 0 ] + rest.n[ 1 ] * rest.n[ 1 ] + rest.n[ 2 ] * rest.n[ 2 ] );
                for ( UInt32 r=0; r<3 && length > 0.0f; ++r )
                    rest.n[ r ] /= length;
            }
            vertices.push_back( rest );
        }
        for ( size_t i=0; i+2<subMesh.indices.size(); i+=3 )
        {
            if ( subMesh.indices[ i ] >= count || subMesh.indices[ i+1 ] >= count || subMesh.indices[ i+2 ] >= count )
                continue;
            triangles.push_back( first + subMesh.indices[ i ] );
            triangles.push_back( first + subMesh.indices[ i+1 ] );
            triangles.push_back( first + subMesh.indices[ i+2 ] );
        }
    }
}

/** A uniform grid over the triangles of the garments: the cells each triangle's bounds touch */
class CTriangleGrid
{
    public:

    CTriangleGrid( const TRestVertexVector& vertices, const TUInt32Vector& triangles, Float32 cellSize )
        : m_vertices( vertices )
        , m_triangles( triangles )
        , m_cellSize( cellSize )
        , m_cells()
    {
        for ( size_t t=0; t+2<triangles.size(); t+=3 )
        {
            Int32 low[ 3 ], high[ 3 ];
            for ( UInt32 c=0; c<3; ++c )
            {
                Float32 a = vertices[ triangles[ t ] ].p[ c ], b = a;
                for ( UInt32 k=1; k<3; ++k )
                {
                    const Float32 value = vertices[ triangles[ t + k ] ].p[ c ];
                    a = value < a ? value : a;
                    b = value > b ? value : b;
                }
                low[ c ] = Cell( a );
                high[ c ] = Cell( b );
            }
            for ( Int32 x=low[ 0 ]; x<=high[ 0 ]; ++x )
                for ( Int32 y=low[ 1 ]; y<=high[ 1 ]; ++y )
                    for ( Int32 z=low[ 2 ]; z<=high[ 2 ]; ++z )
                        m_cells[ Key( x, y, z ) ].push_back( (UInt32) ( t / 3 ) );
        }
    }

    /** The distances along the line through 'origin' with 'direction' (unit) to the triangles it crosses within [ from, to ] */
    void Intersect( const Float32* origin, const Float32* direction, Float32 from, Float32 to, GUCEF::vector< Float32 >& distances ) const
    {
        distances.clear();
        GUCEF::set< UInt32 > candidates;
        const Float32 step = m_cellSize * 0.5f;
        for ( Float32 d=from; d<=to + step * 0.5f; d+=step )
        {
            const Int32 x = Cell( origin[ 0 ] + direction[ 0 ] * d ), y = Cell( origin[ 1 ] + direction[ 1 ] * d ), z = Cell( origin[ 2 ] + direction[ 2 ] * d );
            for ( Int32 dx=-1; dx<=1; ++dx )
                for ( Int32 dy=-1; dy<=1; ++dy )
                    for ( Int32 dz=-1; dz<=1; ++dz )
                    {
                        TCellMap::const_iterator found = m_cells.find( Key( x + dx, y + dy, z + dz ) );
                        if ( found != m_cells.end() )
                            candidates.insert( (*found).second.begin(), (*found).second.end() );
                    }
        }
        GUCEF::set< UInt32 >::const_iterator i = candidates.begin();
        while ( i != candidates.end() )
        {
            Float32 distance = 0.0f;
            if ( Hit( origin, direction, *i, distance ) && distance >= from && distance <= to )
                distances.push_back( distance );
            ++i;
        }
    }

    private:

    typedef GUCEF::map< UInt64, TUInt32Vector > TCellMap;

    Int32 Cell( Float32 value ) const
    {
        return (Int32) floor( value / m_cellSize );
    }

    static UInt64 Key( Int32 x, Int32 y, Int32 z )
    {
        return ( (UInt64) (UInt32) ( x + 1048576 ) & 0x1FFFFF ) | ( ( (UInt64) (UInt32) ( y + 1048576 ) & 0x1FFFFF ) << 21 ) | ( ( (UInt64) (UInt32) ( z + 1048576 ) & 0x1FFFFF ) << 42 );
    }

    /** Moller-Trumbore, both sides of the triangle */
    bool Hit( const Float32* o, const Float32* d, UInt32 triangle, Float32& distance ) const
    {
        const Float32* a = m_vertices[ m_triangles[ (size_t) triangle * 3 ] ].p;
        const Float32* b = m_vertices[ m_triangles[ (size_t) triangle * 3 + 1 ] ].p;
        const Float32* c = m_vertices[ m_triangles[ (size_t) triangle * 3 + 2 ] ].p;
        const Float32 e1[ 3 ] = { b[ 0 ] - a[ 0 ], b[ 1 ] - a[ 1 ], b[ 2 ] - a[ 2 ] };
        const Float32 e2[ 3 ] = { c[ 0 ] - a[ 0 ], c[ 1 ] - a[ 1 ], c[ 2 ] - a[ 2 ] };
        const Float32 h[ 3 ] = { d[ 1 ] * e2[ 2 ] - d[ 2 ] * e2[ 1 ], d[ 2 ] * e2[ 0 ] - d[ 0 ] * e2[ 2 ], d[ 0 ] * e2[ 1 ] - d[ 1 ] * e2[ 0 ] };
        const Float32 det = e1[ 0 ] * h[ 0 ] + e1[ 1 ] * h[ 1 ] + e1[ 2 ] * h[ 2 ];
        if ( fabs( det ) < 1e-12f )
            return false;
        const Float32 f = 1.0f / det;
        const Float32 s[ 3 ] = { o[ 0 ] - a[ 0 ], o[ 1 ] - a[ 1 ], o[ 2 ] - a[ 2 ] };
        const Float32 u = f * ( s[ 0 ] * h[ 0 ] + s[ 1 ] * h[ 1 ] + s[ 2 ] * h[ 2 ] );
        if ( u < -1e-6f || u > 1.0f + 1e-6f )
            return false;
        const Float32 q[ 3 ] = { s[ 1 ] * e1[ 2 ] - s[ 2 ] * e1[ 1 ], s[ 2 ] * e1[ 0 ] - s[ 0 ] * e1[ 2 ], s[ 0 ] * e1[ 1 ] - s[ 1 ] * e1[ 0 ] };
        const Float32 v = f * ( d[ 0 ] * q[ 0 ] + d[ 1 ] * q[ 1 ] + d[ 2 ] * q[ 2 ] );
        if ( v < -1e-6f || u + v > 1.0f + 1e-6f )
            return false;
        distance = f * ( e2[ 0 ] * q[ 0 ] + e2[ 1 ] * q[ 1 ] + e2[ 2 ] * q[ 2 ] );
        return true;
    }

    const TRestVertexVector& m_vertices;
    const TUInt32Vector& m_triangles;
    Float32 m_cellSize;
    TCellMap m_cells;
};

} /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLayerPenetrationValidator::SResult::SResult( void )
    : bodyVertices( 0 )
    , coveredVertices( 0 )
    , pokingVertices( 0 )
    , hiddenBodyVertices( 0 )
    , deepestPoke( 0.0f )
    , pokesPerJoint()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLayerPenetrationValidator::CLayerPenetrationValidator( void )
    : CIAssetValidator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLayerPenetrationValidator::~CLayerPenetrationValidator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CLayerPenetrationValidator::GetName( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CLayerPenetrationValidator::GetVersion( void ) const
{GUCEF_TRACE;

    // 2: the body faces that an active hidden face set hides are not measured
    return 2;
}

/*-------------------------------------------------------------------------*/

bool
CLayerPenetrationValidator::Measure( const CAssetScene& scene, Float32 frontReach, Float32 backReach, Float32 minimumDepth, SResult& result )
{GUCEF_TRACE;

    result = SResult();
    TRestVertexVector body;
    TUInt32Vector bodyTriangles;
    TRestVertexVector garments;
    TUInt32Vector garmentTriangles;
    for ( size_t n=0; n<scene.nodes.size(); ++n )
    {
        const CORE::CString part = scene.nodes[ n ].extras.GetAttributeValueOrChildValueByName( PartAttribute ).AsString();
        if ( part.IsNULLOrEmpty() )
            continue;
        const Int32 levelNode = FirstLevelNode( scene, (UInt32) n );
        if ( levelNode < 0 )
            continue;
        UInt32 unused = 0;
        if ( BodyPart == part )
            RestPose( scene, (UInt32) levelNode, scene.GetActiveHiddenFaceMask(), body, bodyTriangles, result.hiddenBodyVertices );
        else if ( GarmentPart == part )
            RestPose( scene, (UInt32) levelNode, 0, garments, garmentTriangles, unused );
    }
    if ( body.empty() || garmentTriangles.empty() )
        return false;

    // cells of a third of the back reach: a line visits a few dozen cells
    const Float32 cellSize = backReach > 0.0f ? backReach / 3.0f : 0.01f;
    const CTriangleGrid grid( garments, garmentTriangles, cellSize );
    GUCEF::vector< Float32 > distances;
    result.bodyVertices = (UInt32) body.size();
    for ( size_t v=0; v<body.size(); ++v )
    {
        const SRestVertex& vertex = body[ v ];
        if ( 0.0f == vertex.n[ 0 ] && 0.0f == vertex.n[ 1 ] && 0.0f == vertex.n[ 2 ] )
            continue;
        grid.Intersect( vertex.p, vertex.n, -backReach, frontReach, distances );
        bool covered = false;
        Float32 nearestBehind = -1.0f;
        for ( size_t d=0; d<distances.size(); ++d )
        {
            if ( distances[ d ] >= 0.0f )
                covered = true;
            else if ( nearestBehind < 0.0f || -distances[ d ] < nearestBehind )
                nearestBehind = -distances[ d ];
        }
        if ( covered )
        {
            ++result.coveredVertices;
            continue;
        }
        if ( nearestBehind < minimumDepth )
            continue;
        ++result.pokingVertices;
        if ( nearestBehind > result.deepestPoke )
            result.deepestPoke = nearestBehind;
        const CORE::CString joint = vertex.joint >= 0 && (size_t) vertex.joint < scene.nodes.size() ? scene.nodes[ vertex.joint ].name : CORE::CString( "none" );
        ++result.pokesPerJoint[ joint ];
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLayerPenetrationValidator::Validate( const CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report )
{GUCEF_TRACE;

    const TReportItemKind severity = GetSeverity( settings, REPORTITEM_WARNING );
    const Float32 frontReach = settings.GetAttributeValueOrChildValueByName( "frontReach" ).AsFloat32( 0.10f );
    const Float32 backReach = settings.GetAttributeValueOrChildValueByName( "backReach" ).AsFloat32( 0.03f );
    const Float32 minimumDepth = settings.GetAttributeValueOrChildValueByName( "minimumDepth" ).AsFloat32( 0.0002f );
    const UInt32 maxVertices = settings.GetAttributeValueOrChildValueByName( "maxVertices" ).AsUInt32( 0 );
    SResult result;
    if ( !Measure( scene, frontReach, backReach, minimumDepth, result ) )
        return true;

    // the joints with the most pokes, most first
    CORE::CString joints;
    GUCEF::map< CORE::CString, UInt32 > left( result.pokesPerJoint );
    for ( UInt32 i=0; i<5 && !left.empty(); ++i )
    {
        GUCEF::map< CORE::CString, UInt32 >::iterator best = left.begin();
        GUCEF::map< CORE::CString, UInt32 >::iterator j = left.begin();
        while ( j != left.end() )
        {
            if ( (*j).second > (*best).second )
                best = j;
            ++j;
        }
        joints += ( 0 == i ? "" : ", " ) + (*best).first + " " + CORE::UInt32ToString( (*best).second );
        left.erase( best );
    }
    char depth[ 32 ];
    sprintf( depth, "%.4f", result.deepestPoke );
    const CORE::CString subject = scene.name;
    const CORE::CString hiddenText = result.hiddenBodyVertices > 0 ? CORE::UInt32ToString( result.hiddenBodyVertices ) + " vertices of the body are hidden by the active hidden face sets (" +
        CAssetHiddenFaceSet::NamesOf( scene.hiddenFaceSets, scene.GetActiveHiddenFaceMask(), ", " ) + ") and not measured; of the visible body " : CORE::CString();
    report.AddInfo( TypeName, subject, hiddenText + CORE::UInt32ToString( result.coveredVertices ) + " of " + CORE::UInt32ToString( result.bodyVertices ) + " vertices of the body lie under a garment; " +
                    CORE::UInt32ToString( result.pokingVertices ) + " lie outside a garment surface behind them (the body shows through), by up to " + depth +
                    ( result.pokingVertices > 0 ? CORE::CString( "; most at the joints " ) + joints : CORE::CString() ) );
    if ( result.pokingVertices > maxVertices )
        report.AddItem( severity, TypeName, subject, CORE::UInt32ToString( result.pokingVertices ) + " vertices of the body show through the garments in the rest pose (by up to " + depth + ")" );
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
