/*
 *  assetpluginGODOT: the collision shapes of Godot
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

#ifndef GUCEF_GEOMETRY_CTQUICKHULL3D_H
#include "gucefGEOMETRY_CTQuickHull3D.h"
#define GUCEF_GEOMETRY_CTQUICKHULL3D_H
#endif /* GUCEF_GEOMETRY_CTQUICKHULL3D_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#include "assetpluginGODOT_CGodotVariant.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H ? */

#include "assetpluginGODOT_CGodotShapeReader.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static Float32
Absolute( Float32 value )
{
    return value < 0.0f ? -value : value;
}

/*-------------------------------------------------------------------------*/

/** A mesh of the points: their convex hull when it can be made, else the points as a list of triangles */
static Int32
AddHullMesh( ASSETCONVERTER::CAssetScene& scene, const CORE::CString& name, const GUCEF::vector< MATH::TFloat32Point3D >& points, bool& hullMade )
{
    ASSETCONVERTER::CAssetMesh mesh;
    mesh.name = name;
    ASSETCONVERTER::CAssetSubMesh subMesh;
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
        for ( size_t i=0; i+2<points.size(); i+=3 )
        {
            subMesh.indices.push_back( (UInt32) i );
            subMesh.indices.push_back( (UInt32) i + 1 );
            subMesh.indices.push_back( (UInt32) i + 2 );
        }
    }
    mesh.subMeshes.push_back( subMesh );
    scene.meshes.push_back( mesh );
    return (Int32) scene.meshes.size() - 1;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CGodotShapeReader::IsShapeType( const CORE::CString& type )
{GUCEF_TRACE;

    return "BoxShape3D" == type || "SphereShape3D" == type || "CapsuleShape3D" == type || "CylinderShape3D" == type ||
           "ConvexPolygonShape3D" == type || "ConcavePolygonShape3D" == type;
}

/*-------------------------------------------------------------------------*/

bool
CGodotShapeReader::Read( const CORE::CString& type, const CORE::CDataNode& section, const MATH::TFloat32Transform3D& relative, bool trigger, const CORE::CString& name, ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CAssetCollider& collider, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    collider = ASSETCONVERTER::CAssetCollider();
    collider.name = name;
    collider.trigger = trigger;
    collider.transform.position = relative.position;
    collider.transform.rotation = relative.rotation;
    collider.transform.scale = MATH::TFloat32Transform3D::TVector::One();
    const Float32 sx = Absolute( relative.scale.x );
    const Float32 sy = Absolute( relative.scale.y );
    const Float32 sz = Absolute( relative.scale.z );
    const bool uniform = Absolute( sx - sy ) < 1e-4f * ( sx + sy ) && Absolute( sy - sz ) < 1e-4f * ( sy + sz );

    if ( "BoxShape3D" == type )
    {
        Float32 size[ 3 ] = { 1.0f, 1.0f, 1.0f };
        CGodotVariant::GetVector3( CGodotVariant::FindProperty( section, "size" ), size );
        collider.shape = ASSETCONVERTER::COLLIDERSHAPE_BOX;
        collider.halfExtents[ 0 ] = 0.5f * size[ 0 ] * sx;
        collider.halfExtents[ 1 ] = 0.5f * size[ 1 ] * sy;
        collider.halfExtents[ 2 ] = 0.5f * size[ 2 ] * sz;
        return true;
    }
    if ( "SphereShape3D" == type )
    {
        const Float32 radius = (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, "radius" ), 0.5 );
        collider.shape = ASSETCONVERTER::COLLIDERSHAPE_SPHERE;
        collider.radius = radius * ( sx + sy + sz ) / 3.0f;
        if ( !uniform )
            report.AddApproximation( SourceName, name, "a sphere shape with a scale that is not uniform becomes a sphere with the mean scale" );
        return true;
    }
    if ( "CapsuleShape3D" == type || "CylinderShape3D" == type )
    {
        const Float32 radius = (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, "radius" ), 0.5 );
        const Float32 height = (Float32) CGodotVariant::GetNumber( CGodotVariant::FindProperty( section, "height" ), 2.0 );
        collider.shape = "CapsuleShape3D" == type ? ASSETCONVERTER::COLLIDERSHAPE_CAPSULE : ASSETCONVERTER::COLLIDERSHAPE_CYLINDER;
        collider.radius = radius * 0.5f * ( sx + sz );
        collider.height = height * sy;
        if ( ASSETCONVERTER::COLLIDERSHAPE_CAPSULE == collider.shape && collider.height < 2.0f * collider.radius )
            collider.height = 2.0f * collider.radius;
        if ( Absolute( sx - sz ) > 1e-4f * ( sx + sz ) )
            report.AddApproximation( SourceName, name, "a round shape with a different scale along X and Z becomes one with the mean of the two" );
        return true;
    }
    if ( "ConvexPolygonShape3D" == type || "ConcavePolygonShape3D" == type )
    {
        const bool convex = "ConvexPolygonShape3D" == type;
        CGodotVariant::TNumberVector numbers;
        if ( !CGodotVariant::GetNumbers( CGodotVariant::FindProperty( section, convex ? "points" : "data" ), numbers ) || numbers.size() < 9 )
        {
            report.AddSkipped( SourceName, name, "the shape has no points: no collider is made" );
            return false;
        }
        GUCEF::vector< MATH::TFloat32Point3D > points;
        for ( size_t i=0; i+2<numbers.size(); i+=3 )
            points.push_back( MATH::TFloat32Point3D( (Float32) numbers[ i ] * relative.scale.x, (Float32) numbers[ i + 1 ] * relative.scale.y, (Float32) numbers[ i + 2 ] * relative.scale.z ) );
        if ( convex )
        {
            bool hullMade = false;
            collider.shape = ASSETCONVERTER::COLLIDERSHAPE_CONVEX;
            collider.meshIndex = AddHullMesh( scene, name, points, hullMade );
            if ( !hullMade )
                report.AddApproximation( SourceName, name, "the points of the convex shape have no hull (they are flat or too few): they are kept as they are" );
        }
        else
        {
            // the triangles of Godot are wound clockwise
            ASSETCONVERTER::CAssetMesh mesh;
            mesh.name = name;
            ASSETCONVERTER::CAssetSubMesh subMesh;
            for ( size_t i=0; i<points.size(); ++i )
            {
                subMesh.positions.push_back( points[ i ].x );
                subMesh.positions.push_back( points[ i ].y );
                subMesh.positions.push_back( points[ i ].z );
            }
            for ( size_t i=0; i+2<points.size(); i+=3 )
            {
                subMesh.indices.push_back( (UInt32) i );
                subMesh.indices.push_back( (UInt32) i + 2 );
                subMesh.indices.push_back( (UInt32) i + 1 );
            }
            mesh.subMeshes.push_back( subMesh );
            scene.meshes.push_back( mesh );
            collider.shape = ASSETCONVERTER::COLLIDERSHAPE_TRIMESH;
            collider.meshIndex = (Int32) scene.meshes.size() - 1;
        }
        return true;
    }

    report.AddSkipped( SourceName, name, "the shape type " + type + " has no counterpart in OMI_physics_shape: no collider is made" );
    return false;
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
