/*
 *  AssetConverterLib: brings an imported asset into the conventions of the intermediate
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

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_TRANSFORMS_H
#include "gucefMATH_transforms.h"
#define GUCEF_MATH_TRANSFORMS_H
#endif /* GUCEF_MATH_TRANSFORMS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#include "AssetConverterLib_CAssetNormaliser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The signed permutation of a conversion with the unit scale: v' = scale * P v */
struct SConversion
{
    Float32 p[ 9 ];         /**< column-major */
    Float32 scale;
    Float32 determinant;    /**< +1, or -1 for a mirror */

    void Apply( Float32* v, Float32 factor ) const
    {
        const Float32 x = v[ 0 ], y = v[ 1 ], z = v[ 2 ];
        for ( UInt32 r=0; r<3; ++r )
            v[ r ] = factor * ( p[ r ] * x + p[ 3 + r ] * y + p[ 6 + r ] * z );
    }

    void ApplyAbs( Float32* v ) const
    {
        const Float32 x = v[ 0 ], y = v[ 1 ], z = v[ 2 ];
        for ( UInt32 r=0; r<3; ++r )
            v[ r ] = ( p[ r ] != 0.0f ? x : 0.0f ) + ( p[ 3 + r ] != 0.0f ? y : 0.0f ) + ( p[ 6 + r ] != 0.0f ? z : 0.0f );
    }

    /** A rotation q = (v, w) of the source as rotation of the target: P R P^T is (det(P) P v, w); linear, so also for spline tangents */
    void ApplyRotation( Float32* q ) const
    {
        Apply( q, determinant );
    }

    void ApplyStream( GUCEF::vector< Float32 >& values, UInt32 stride, Float32 factor ) const
    {
        for ( size_t i=0; i+2<values.size(); i+=stride )
            Apply( &values[ i ], factor );
    }
};

SConversion
MakeConversion( const MATH::CCoordinateSystem& source, const MATH::CCoordinateSystem& target )
{
    SConversion conversion;
    Int32 permutation[ 9 ];
    source.GetConversionPermutation( target, permutation );
    for ( UInt32 i=0; i<9; ++i )
        conversion.p[ i ] = (Float32) permutation[ i ];
    conversion.scale = (Float32) source.GetConversionScale( target );
    conversion.determinant = source.ConversionFlipsWinding( target ) ? -1.0f : 1.0f;
    return conversion;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAssetNormaliser::ConvertCoordinateSystem( CAssetScene& scene, const MATH::CCoordinateSystem& source, const MATH::CCoordinateSystem& target )
{GUCEF_TRACE;

    const SConversion conversion = MakeConversion( source, target );
    const bool mirrored = conversion.determinant < 0.0f;

    // the hierarchy: every local transform re-expressed, which keeps the hierarchy consistent
    for ( size_t n=0; n<scene.nodes.size(); ++n )
    {
        CAssetNode& node = scene.nodes[ n ];
        node.transform = source.ConvertTransform( target, node.transform );

        // colliders: the frame of a primitive moves along, its shape scales; a mirror is taken by the symmetry of the
        // shape (a flip of its X axis), so its frame stays a rotation
        for ( size_t c=0; c<node.colliders.size(); ++c )
        {
            CAssetCollider& collider = node.colliders[ c ];
            if ( collider.IsMeshShape() )
            {
                collider.transform = source.ConvertTransform( target, collider.transform );
                continue;
            }
            Float32 position[ 3 ] = { collider.transform.position.x, collider.transform.position.y, collider.transform.position.z };
            conversion.Apply( position, conversion.scale );
            collider.transform.position = MATH::TFloat32Transform3D::TPositionVector( position[ 0 ], position[ 1 ], position[ 2 ] );
            const MATH::TFloat32Matrix3D p = MATH::TFloat32Matrix3D::FromColumnMajor( conversion.p );
            const Float32 flipValues[ 9 ] = { mirrored ? -1.0f : 1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f };
            const MATH::TFloat32Matrix3D flip = MATH::TFloat32Matrix3D::FromColumnMajor( flipValues );
            const MATH::TFloat32Matrix3D rotation = p * collider.transform.rotation.ToRotationMatrix3D() * flip;
            collider.transform.rotation = rotation.ToQuaternion().Normalized();
            for ( UInt32 i=0; i<3; ++i )
                collider.halfExtents[ i ] *= conversion.scale;
            collider.radius *= conversion.scale;
            collider.height *= conversion.scale;
        }
    }

    // the geometry
    for ( size_t m=0; m<scene.meshes.size(); ++m )
    {
        for ( size_t s=0; s<scene.meshes[ m ].subMeshes.size(); ++s )
        {
            CAssetSubMesh& subMesh = scene.meshes[ m ].subMeshes[ s ];
            conversion.ApplyStream( subMesh.positions, 3, conversion.scale );
            conversion.ApplyStream( subMesh.normals, 3, 1.0f );
            for ( size_t t=0; t+3<subMesh.tangents.size(); t+=4 )
            {
                conversion.Apply( &subMesh.tangents[ t ], 1.0f );
                subMesh.tangents[ t+3 ] *= conversion.determinant;
            }
            for ( size_t t=0; t<subMesh.morphTargets.size(); ++t )
            {
                conversion.ApplyStream( subMesh.morphTargets[ t ].positionDeltas, 3, conversion.scale );
                conversion.ApplyStream( subMesh.morphTargets[ t ].normalDeltas, 3, 1.0f );
                conversion.ApplyStream( subMesh.morphTargets[ t ].tangentDeltas, 3, 1.0f );
            }
            if ( mirrored )
                subMesh.ReverseWinding();
        }
    }

    // the skins: B' = C B C^-1 for the bind matrices, so also for their inverses
    const MATH::TFloat32Matrix4D toTarget = source.GetConversionMatrix4To< Float32 >( target );
    const MATH::TFloat32Matrix4D toSource = target.GetConversionMatrix4To< Float32 >( source );
    for ( size_t s=0; s<scene.skins.size(); ++s )
        for ( size_t j=0; j<scene.skins[ s ].inverseBindMatrices.size(); ++j )
            scene.skins[ s ].inverseBindMatrices[ j ] = toTarget * scene.skins[ s ].inverseBindMatrices[ j ] * toSource;

    // the animations: the keys as the node transforms (also the tangents of cubic splines, the conversion is linear)
    for ( size_t a=0; a<scene.animations.size(); ++a )
    {
        for ( size_t c=0; c<scene.animations[ a ].channels.size(); ++c )
        {
            CAssetAnimationChannel& channel = scene.animations[ a ].channels[ c ];
            switch ( channel.path )
            {
                case ANIMATIONPATH_TRANSLATION:
                    conversion.ApplyStream( channel.values, 3, conversion.scale );
                    break;
                case ANIMATIONPATH_ROTATION:
                    for ( size_t k=0; k+3<channel.values.size(); k+=4 )
                        conversion.ApplyRotation( &channel.values[ k ] );
                    break;
                case ANIMATIONPATH_SCALE:
                    for ( size_t k=0; k+2<channel.values.size(); k+=3 )
                        conversion.ApplyAbs( &channel.values[ k ] );
                    break;
                default:
                    break;
            }
        }
    }

    // ranges of lights are distances
    for ( size_t l=0; l<scene.lights.size(); ++l )
        scene.lights[ l ].range *= conversion.scale;

    // particle systems: their vectors, shape and distances (design section 5.7.2)
    for ( size_t p=0; p<scene.particleSystems.size(); ++p )
        scene.particleSystems[ p ].ConvertSpace( conversion.p, conversion.scale, conversion.determinant );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetNormaliser::FlipNormalMapGreen( CAssetScene& scene )
{GUCEF_TRACE;

    GUCEF::set< Int32 > normalMaps;
    for ( size_t m=0; m<scene.materials.size(); ++m )
    {
        const Int32 texture = scene.materials[ m ].normalTexture.textureIndex;
        if ( texture >= 0 && (size_t) texture < scene.textures.size() )
            normalMaps.insert( texture );
    }
    GUCEF::set< Int32 >::const_iterator i = normalMaps.begin();
    while ( i != normalMaps.end() )
    {
        scene.textures[ *i ].AddGreenFlip();
        ++i;
    }
    return (UInt32) normalMaps.size();
}

/*-------------------------------------------------------------------------*/

bool
CAssetNormaliser::Normalise( CAssetScene& scene, CConversionReport& report )
{GUCEF_TRACE;

    static const CORE::CString source = "Normalisation";
    MATH::CCoordinateSystem system = scene.coordinateSystem;

    const CORE::CString systemText = scene.settings.GetAttributeValueOrChildValueByName( "coordinateSystem" ).AsString();
    if ( !systemText.IsNULLOrEmpty() )
    {
        MATH::CCoordinateSystem overridden;
        if ( overridden.FromString( systemText ) )
            system = overridden;
        else
            report.AddWarning( source, "coordinateSystem", "the setting \"" + systemText + "\" is not a coordinate system such as (+X, +Y, -Z, 1); the one of the importer is used" );
    }
    const Float64 unitsPerMetre = scene.settings.GetAttributeValueOrChildValueByName( "unitsPerMetre" ).AsFloat64( 0.0 );
    if ( unitsPerMetre > 0.0 )
        system = MATH::CCoordinateSystem( system.GetRightAxis(), system.GetUpAxis(), system.GetForwardAxis(), unitsPerMetre );
    if ( !system.IsValid() )
    {
        report.AddError( source, "coordinateSystem", "the coordinate system of the source is not valid: " + system.ToString() );
        return false;
    }

    const MATH::CCoordinateSystem target = MATH::CCoordinateSystem::GucefDefault();
    const bool mirrored = system.ConversionFlipsWinding( target );
    scene.normalisation.SetAttribute( "from", system.ToString() );
    scene.normalisation.SetAttribute( "to", target.ToString() );
    scene.normalisation.SetAttribute( "scale", CORE::CVariant( (Float64) system.GetConversionScale( target ) ) );
    scene.normalisation.SetAttribute( "mirrored", mirrored );
    if ( system != target )
    {
        ConvertCoordinateSystem( scene, system, target );
        report.AddInfo( source, "coordinateSystem", "converted from " + system.ToString() + " to " + target.ToString() + ( mirrored ? ", mirrored: winding reversed" : "" ) );
    }
    scene.coordinateSystem = target;

    UInt32 flipped = 0;
    if ( NORMALMAPCONVENTION_DIRECTX == scene.normalMapConvention )
    {
        flipped = FlipNormalMapGreen( scene );
        scene.normalMapConvention = NORMALMAPCONVENTION_OPENGL;
        if ( flipped > 0 )
            report.AddInfo( source, "normal maps", "the green channel of " + CORE::UInt32ToString( flipped ) + " DirectX normal map(s) is flipped" );
    }
    scene.normalisation.SetAttribute( "normalMapsFlipped", flipped );
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
