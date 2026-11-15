/*
 *  AssetConverterLib: the built-in post-processor that reads the hints in node and animation names
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

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H
#include "gucefGEOMETRY_BoundingVolumeFitting.h"
#define GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H
#endif /* GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H ? */

#ifndef GUCEF_GEOMETRY_CTQUICKHULL3D_H
#include "gucefGEOMETRY_CTQuickHull3D.h"
#define GUCEF_GEOMETRY_CTQUICKHULL3D_H
#endif /* GUCEF_GEOMETRY_CTQUICKHULL3D_H ? */

#ifndef GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#include "gucefGEOMETRY_CTOrientedBox3D.h"
#define GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTORIENTEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTSPHERE_H
#include "gucefGEOMETRY_CTSphere.h"
#define GUCEF_GEOMETRY_CTSPHERE_H
#endif /* GUCEF_GEOMETRY_CTSPHERE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#include "AssetConverterLib_CNameHintsPostProcessor.h"

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

const CORE::CString CNameHintsPostProcessor::TypeName = "NameHints";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::vector< MATH::TFloat32Point3D > TPointVector;

bool
EndsWith( const CORE::CString& text, const CORE::CString& suffix, bool caseSensitive )
{
    if ( text.Length() < suffix.Length() )
        return false;
    const CORE::CString tail = text.SubstrFromRange( text.Length() - suffix.Length(), text.Length() );
    return caseSensitive ? tail == suffix : tail.Lowercase() == suffix.Lowercase();
}

CORE::CString
CutTail( const CORE::CString& text, UInt32 count )
{
    return text.SubstrFromRange( 0, text.Length() - count );
}

/** "name_12" gives "name"; a name without a numbered tail is returned as it is */
CORE::CString
StripNumberTail( const CORE::CString& name )
{
    const Int32 underscore = name.HasChar( '_', false );
    if ( underscore <= 0 || (UInt32) underscore + 1 >= name.Length() )
        return name;
    for ( UInt32 i=(UInt32) underscore+1; i<name.Length(); ++i )
        if ( name[ i ] < '0' || name[ i ] > '9' )
            return name;
    return name.SubstrFromRange( 0, (UInt32) underscore );
}

/** <name>_LOD<n>, letter case ignored: the name and n */
bool
ParseLodName( const CORE::CString& nodeName, CORE::CString& baseName, UInt32& level )
{
    // the last "_lod" of the name
    const CORE::CString lower = nodeName.Lowercase();
    Int32 at = -1;
    for ( UInt32 i=lower.Length() >= 4 ? lower.Length()-4 : 0; i>0 && at<0; --i )
        if ( '_' == lower[ i ] && 'l' == lower[ i+1 ] && 'o' == lower[ i+2 ] && 'd' == lower[ i+3 ] )
            at = (Int32) i;
    if ( at <= 0 )
        return false;
    const UInt32 digitsStart = (UInt32) at + 4;
    if ( digitsStart >= nodeName.Length() )
        return false;
    UInt32 value = 0;
    for ( UInt32 i=digitsStart; i<nodeName.Length(); ++i )
    {
        if ( nodeName[ i ] < '0' || nodeName[ i ] > '9' )
            return false;
        value = value * 10 + (UInt32) ( nodeName[ i ] - '0' );
    }
    baseName = nodeName.SubstrFromRange( 0, (UInt32) at );
    level = value;
    return true;
}

/** The vertices of the mesh of a node in the frame of another node */
void
GetPointsIn( const CAssetScene& scene, UInt32 node, UInt32 frameNode, TPointVector& points )
{
    points.clear();
    const Int32 meshIndex = scene.nodes[ node ].mesh;
    if ( meshIndex < 0 || (size_t) meshIndex >= scene.meshes.size() )
        return;
    const MATH::TFloat32Matrix4D toFrame = scene.GetWorldMatrix( frameNode ).Inverse() * scene.GetWorldMatrix( node );
    const CAssetMesh& mesh = scene.meshes[ meshIndex ];
    for ( size_t s=0; s<mesh.subMeshes.size(); ++s )
    {
        const CAssetSubMesh::TFloatVector& positions = mesh.subMeshes[ s ].positions;
        for ( size_t v=0; v+2<positions.size(); v+=3 )
            points.push_back( toFrame.TransformPoint( MATH::TFloat32Point3D( positions[ v ], positions[ v+1 ], positions[ v+2 ] ) ) );
    }
}

/** A mesh of the points: their convex hull when it can be made, else the points as they are with the triangles of the source */
Int32
AddConvexMesh( CAssetScene& scene, const CORE::CString& meshName, const TPointVector& points, const CAssetMesh& sourceMesh, bool& hullMade )
{
    CAssetMesh mesh;
    mesh.name = meshName;
    CAssetSubMesh subMesh;
    hullMade = false;
    GEOMETRY::CTQuickHull3D< Float32 > quickHull;
    if ( !points.empty() && GEOMETRY::QUICKHULL_OK == quickHull.Build( &points[ 0 ], (UInt32) points.size() ) && quickHull.GetHull().HasFaces() )
    {
        const GEOMETRY::CTConvexHull3D< Float32 >& hull = quickHull.GetHull();
        for ( UInt32 v=0; v<hull.GetVertexCount(); ++v )
        {
            subMesh.positions.push_back( hull.GetVertex( v ).x );
            subMesh.positions.push_back( hull.GetVertex( v ).y );
            subMesh.positions.push_back( hull.GetVertex( v ).z );
        }
        for ( UInt32 f=0; f<hull.GetFaceCount(); ++f )
        {
            const UInt32 corners = hull.GetFaceVertexCount( f );
            for ( UInt32 c=1; c+1<corners; ++c )
            {
                subMesh.indices.push_back( hull.GetFaceVertexIndex( f, 0 ) );
                subMesh.indices.push_back( hull.GetFaceVertexIndex( f, c ) );
                subMesh.indices.push_back( hull.GetFaceVertexIndex( f, c+1 ) );
            }
        }
        hullMade = true;
    }
    else
    {
        for ( size_t i=0; i<points.size(); ++i )
        {
            subMesh.positions.push_back( points[ i ].x );
            subMesh.positions.push_back( points[ i ].y );
            subMesh.positions.push_back( points[ i ].z );
        }
        UInt32 base = 0;
        for ( size_t s=0; s<sourceMesh.subMeshes.size(); ++s )
        {
            for ( size_t i=0; i<sourceMesh.subMeshes[ s ].indices.size(); ++i )
                subMesh.indices.push_back( base + sourceMesh.subMeshes[ s ].indices[ i ] );
            base += sourceMesh.subMeshes[ s ].GetVertexCount();
        }
    }
    mesh.subMeshes.push_back( subMesh );
    scene.meshes.push_back( mesh );
    return (Int32) scene.meshes.size() - 1;
}

/** A capsule along the longest axis of the principal-axes box of the points, holding every point */
bool
FitCapsule( const TPointVector& points, CAssetCollider& collider )
{
    GEOMETRY::CTOrientedBox3D< Float32 > box;
    if ( points.empty() || !GEOMETRY::FitOrientedBox( &points[ 0 ], (UInt32) points.size(), box ) )
        return false;
    MATH::TFloat32Vector3D axes[ 3 ];
    box.GetAxes( axes );
    UInt32 longest = 0;
    const Float32 extents[ 3 ] = { box.halfExtents.x, box.halfExtents.y, box.halfExtents.z };
    for ( UInt32 i=1; i<3; ++i )
        if ( extents[ i ] > extents[ longest ] )
            longest = i;
    const MATH::TFloat32Vector3D axis = axes[ longest ];
    const MATH::TFloat32Vector3D centre = box.centre.ToPositionVector();

    // the radius: the largest distance from the axis; the half length of the cylinder: what keeps every point in a cap
    Float32 radius = 0.0f;
    for ( size_t i=0; i<points.size(); ++i )
    {
        const MATH::TFloat32Vector3D d = points[ i ].ToPositionVector() - centre;
        const Float32 along = d.Dot( axis );
        const MATH::TFloat32Vector3D radial = d - axis * along;
        const Float32 distance = radial.GetLength();
        if ( distance > radius )
            radius = distance;
    }
    Float32 halfLength = 0.0f;
    for ( size_t i=0; i<points.size(); ++i )
    {
        const MATH::TFloat32Vector3D d = points[ i ].ToPositionVector() - centre;
        const Float32 along = d.Dot( axis );
        const MATH::TFloat32Vector3D radial = d - axis * along;
        const Float32 r2 = radius * radius - radial.Dot( radial );
        const Float32 needed = ( along < 0.0f ? -along : along ) - ( r2 > 0.0f ? (Float32) sqrt( r2 ) : 0.0f );
        if ( needed > halfLength )
            halfLength = needed;
    }

    // the capsule lies along its local Y: the rotation that takes Y to the axis
    MATH::TFloat32Quaternion rotation = MATH::TFloat32Quaternion::Identity();
    MATH::CTDirectionVector3D< Float32 > to;
    if ( MATH::CTDirectionVector3D< Float32 >::TryCreate( axis, to ) )
        rotation = MATH::TFloat32Quaternion::FromTo( MATH::CTDirectionVector3D< Float32 >::UnitY(), to );
    collider.shape = COLLIDERSHAPE_CAPSULE;
    collider.radius = radius;
    collider.height = 2.0f * ( halfLength + radius );
    collider.transform = MATH::TFloat32Transform3D( MATH::TFloat32Transform3D::TPositionVector( centre.x, centre.y, centre.z ), rotation.Normalized(), MATH::TFloat32Vector3D( 1.0f, 1.0f, 1.0f ) );
    return true;
}

/** Moves a node under another one, keeping where it is (the new parent must have the identity transform relative to the old) */
void
Reparent( CAssetScene& scene, UInt32 node, UInt32 newParent )
{
    const Int32 oldParent = scene.nodes[ node ].parent;
    CAssetScene::TUInt32Vector& siblings = oldParent >= 0 ? scene.nodes[ oldParent ].children : scene.rootNodes;
    siblings.erase( std::remove( siblings.begin(), siblings.end(), node ), siblings.end() );
    scene.nodes[ node ].parent = (Int32) newParent;
    scene.nodes[ newParent ].children.push_back( node );
}

struct SLodLevel
{
    UInt32 level;
    UInt32 node;
    bool operator<( const SLodLevel& other ) const { return level < other.level; }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNameHintsPostProcessor::CNameHintsPostProcessor( void )
    : CIAssetPostProcessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNameHintsPostProcessor::~CNameHintsPostProcessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CNameHintsPostProcessor::GetName( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CNameHintsPostProcessor::GetVersion( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CNameHintsPostProcessor::ResolveSet( const CORE::CString& set, const CAssetScene& scene )
{GUCEF_TRACE;

    const CORE::CString lower = set.Lowercase();
    if ( "auto" != lower && !lower.IsNULLOrEmpty() )
        return lower;
    const CORE::CString engine = scene.provenance.sourceEngine.Lowercase();
    const CORE::CString importer = scene.provenance.importerName.Lowercase();
    if ( "unreal" == engine || "unreal" == importer )
        return "unreal";
    if ( "godot" == engine || "godot" == importer )
        return "godot";
    if ( "unity" == engine || "unity" == importer )
        return "none";
    return "both";
}

/*-------------------------------------------------------------------------*/

bool
CNameHintsPostProcessor::Process( CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report )
{GUCEF_TRACE;

    CORE::CString set = settings.GetAttributeValueOrChildValueByName( "set" ).AsString();
    if ( set.IsNULLOrEmpty() )
        set = scene.settings.GetAttributeValueOrChildValueByName( "nameHints" ).AsString( "auto" );
    set = ResolveSet( set, scene );
    if ( "none" == set )
        return true;
    const bool unreal = "unreal" == set || "both" == set;
    const bool godot = "godot" == set || "both" == set;

    CAssetScene::TUInt32Set removed;
    CAssetScene::TUInt32Set colliderMeshes;

    // Godot: not imported
    if ( godot )
    {
        for ( size_t n=0; n<scene.nodes.size(); ++n )
        {
            if ( EndsWith( scene.nodes[ n ].name, "-noimp", false ) )
            {
                removed.insert( (UInt32) n );
                report.AddSkipped( TypeName, scene.nodes[ n ].name, "not imported (-noimp)" );
            }
        }
    }

    // Unreal: collision meshes
    if ( unreal )
    {
        static const char* const prefixes[] = { "UCX_", "UBX_", "USP_", "UCP_" };
        for ( size_t n=0; n<scene.nodes.size(); ++n )
        {
            const CORE::CString nodeName = scene.nodes[ n ].name;
            Int32 kind = -1;
            for ( Int32 p=0; p<4 && kind<0; ++p )
                if ( nodeName.Length() > 4 && nodeName.SubstrFromRange( 0, 4 ) == prefixes[ p ] )
                    kind = p;
            if ( kind < 0 || scene.nodes[ n ].mesh < 0 || removed.find( (UInt32) n ) != removed.end() )
                continue;
            const CORE::CString targetName = nodeName.CutChars( 4, true );
            Int32 target = scene.FindNode( targetName );
            if ( target < 0 || target == (Int32) n )
                target = scene.FindNode( StripNumberTail( targetName ) );
            if ( target < 0 || target == (Int32) n )
            {
                report.AddWarning( TypeName, nodeName, "there is no node named " + StripNumberTail( targetName ) + " for this collision mesh; it stays a visual mesh" );
                continue;
            }

            TPointVector points;
            GetPointsIn( scene, (UInt32) n, (UInt32) target, points );
            CAssetCollider collider;
            collider.name = nodeName;
            bool fitted = !points.empty();
            switch ( kind )
            {
                case 0:
                {
                    bool hullMade = false;
                    collider.shape = COLLIDERSHAPE_CONVEX;
                    collider.meshIndex = AddConvexMesh( scene, nodeName, points, scene.meshes[ scene.nodes[ n ].mesh ], hullMade );
                    if ( !hullMade )
                        report.AddApproximation( TypeName, nodeName, "the convex hull could not be made: the collision mesh is kept as it is" );
                    break;
                }
                case 1:
                {
                    GEOMETRY::CTOrientedBox3D< Float32 > box;
                    fitted = fitted && GEOMETRY::FitOrientedBox( &points[ 0 ], (UInt32) points.size(), box );
                    collider.shape = COLLIDERSHAPE_BOX;
                    collider.halfExtents[ 0 ] = box.halfExtents.x;
                    collider.halfExtents[ 1 ] = box.halfExtents.y;
                    collider.halfExtents[ 2 ] = box.halfExtents.z;
                    collider.transform = MATH::TFloat32Transform3D( box.centre.ToPositionVector(), box.rotation.Normalized(), MATH::TFloat32Vector3D( 1.0f, 1.0f, 1.0f ) );
                    break;
                }
                case 2:
                {
                    GEOMETRY::CTSphere< Float32 > sphere;
                    GUCEF::vector< MATH::TFloat32Point3D > scratch;
                    fitted = fitted && GEOMETRY::FitMinimumSphere( &points[ 0 ], (UInt32) points.size(), scratch, (UInt64) 1, sphere );
                    collider.shape = COLLIDERSHAPE_SPHERE;
                    collider.radius = sphere.radius;
                    collider.transform.position = sphere.centre.ToPositionVector();
                    break;
                }
                default:
                {
                    fitted = fitted && FitCapsule( points, collider );
                    break;
                }
            }
            if ( !fitted )
            {
                report.AddWarning( TypeName, nodeName, "no shape could be fitted to the vertices; the collision mesh is left out" );
            }
            else
            {
                scene.nodes[ target ].colliders.push_back( collider );
                report.AddInfo( TypeName, nodeName, ToString( collider.shape ) + " collider of " + scene.nodes[ target ].name );
            }
            colliderMeshes.insert( (UInt32) scene.nodes[ n ].mesh );
            removed.insert( (UInt32) n );
        }
    }

    // Godot: colliders from the visual mesh
    if ( godot )
    {
        static const char* const suffixes[] = { "-convcolonly", "-colonly", "-convcol", "-col" };
        for ( size_t n=0; n<scene.nodes.size(); ++n )
        {
            CAssetNode& node = scene.nodes[ n ];
            for ( UInt32 s=0; s<4; ++s )
            {
                const CORE::CString suffix = suffixes[ s ];
                if ( !EndsWith( node.name, suffix, false ) )
                    continue;
                node.name = CutTail( node.name, suffix.Length() );
                if ( node.mesh < 0 )
                {
                    report.AddWarning( TypeName, node.name + suffix, "the node has no mesh to make a collider of" );
                    break;
                }
                CAssetCollider collider;
                collider.name = node.name;
                collider.shape = ( 0 == s || 2 == s ) ? COLLIDERSHAPE_CONVEX : COLLIDERSHAPE_TRIMESH;
                collider.meshIndex = node.mesh;
                node.colliders.push_back( collider );
                if ( s < 2 )
                    node.mesh = -1;
                report.AddInfo( TypeName, node.name, ToString( collider.shape ) + " collider (" + suffix + ")" );
                break;
            }
        }
        for ( size_t a=0; a<scene.animations.size(); ++a )
        {
            CAssetAnimation& animation = scene.animations[ a ];
            if ( EndsWith( animation.name, "-loop", false ) )
            {
                animation.name = CutTail( animation.name, 5 );
                animation.loop = true;
            }
            else
            if ( EndsWith( animation.name, "-cycle", false ) )
            {
                animation.name = CutTail( animation.name, 6 );
                animation.loop = true;
            }
        }
    }

    // both: levels of detail, grouped by parent and name
    typedef GUCEF::map< CORE::CString, GUCEF::vector< SLodLevel > > TLodMap;
    TLodMap groups;
    for ( size_t n=0; n<scene.nodes.size(); ++n )
    {
        if ( removed.find( (UInt32) n ) != removed.end() || scene.IsLodLevel( (UInt32) n ) )
            continue;
        CORE::CString baseName;
        SLodLevel level;
        if ( !ParseLodName( scene.nodes[ n ].name, baseName, level.level ) )
            continue;
        level.node = (UInt32) n;
        groups[ CORE::Int32ToString( scene.nodes[ n ].parent ) + "/" + baseName ].push_back( level );
    }
    TLodMap::iterator g = groups.begin();
    while ( g != groups.end() )
    {
        GUCEF::vector< SLodLevel >& levels = (*g).second;
        std::sort( levels.begin(), levels.end() );
        const Int32 parent = scene.nodes[ levels[ 0 ].node ].parent;
        CORE::CString baseName;
        UInt32 ignored = 0;
        ParseLodName( scene.nodes[ levels[ 0 ].node ].name, baseName, ignored );

        // the holder: a sibling with the name, the parent when it holds only the levels, else a new node
        Int32 holder = -1;
        const CAssetScene::TUInt32Vector& siblings = parent >= 0 ? scene.nodes[ parent ].children : scene.rootNodes;
        for ( size_t i=0; i<siblings.size() && holder<0; ++i )
            if ( scene.nodes[ siblings[ i ] ].name == baseName && scene.nodes[ siblings[ i ] ].lodGroup < 0 )
                holder = (Int32) siblings[ i ];
        if ( holder < 0 && parent >= 0 && siblings.size() == levels.size() && scene.nodes[ parent ].lodGroup < 0 )
            holder = parent;
        if ( holder < 0 )
        {
            holder = (Int32) scene.AddNode( baseName, parent );
            for ( size_t i=0; i<levels.size(); ++i )
                Reparent( scene, levels[ i ].node, (UInt32) holder );
        }

        CAssetLodGroup group;
        group.name = baseName;
        for ( size_t i=0; i<levels.size(); ++i )
            group.AddLevel( levels[ i ].node, -1.0f );
        scene.lodGroups.push_back( group );
        scene.nodes[ holder ].lodGroup = (Int32) scene.lodGroups.size() - 1;
        report.AddInfo( TypeName, baseName, "LOD group of " + CORE::UInt32ToString( (UInt32) levels.size() ) + " level(s), held by " + scene.nodes[ holder ].name );
        ++g;
    }

    // the collision nodes and the meshes only they used go
    scene.RemoveNodes( removed );
    CAssetScene::TUInt32Set unusedMeshes;
    CAssetScene::TUInt32Set::const_iterator m = colliderMeshes.begin();
    while ( m != colliderMeshes.end() )
    {
        if ( !scene.IsMeshUsed( *m ) )
            unusedMeshes.insert( *m );
        ++m;
    }
    scene.RemoveMeshes( unusedMeshes );
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
