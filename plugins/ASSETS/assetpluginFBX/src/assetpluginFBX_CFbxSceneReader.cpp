/*
 *  assetpluginFBX: reads an FBX file into a scene of the converter model
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

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "assetpluginFBX_CFbxMaterialReader.h"
#include "assetpluginFBX_CFbxMeshReader.h"
#include "assetpluginFBX_CFbxAnimationReader.h"
#include "assetpluginFBX_CFbxSceneReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace FBX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const char* const SourceName = "fbx";

CORE::CString
ToString( const ufbx_string& text )
{
    return GUCEF_NULL != text.data && text.length > 0 ? CORE::CString( text.data, (UInt32) text.length ) : CORE::CString::Empty;
}

/** The axis the other way round: the signed axes come in pairs, positive first */
MATH::TSignedAxis
Opposite( ufbx_coordinate_axis axis )
{
    switch ( axis )
    {
        case UFBX_COORDINATE_AXIS_POSITIVE_X : return MATH::SIGNEDAXIS_NEGATIVE_X;
        case UFBX_COORDINATE_AXIS_NEGATIVE_X : return MATH::SIGNEDAXIS_POSITIVE_X;
        case UFBX_COORDINATE_AXIS_POSITIVE_Y : return MATH::SIGNEDAXIS_NEGATIVE_Y;
        case UFBX_COORDINATE_AXIS_NEGATIVE_Y : return MATH::SIGNEDAXIS_POSITIVE_Y;
        case UFBX_COORDINATE_AXIS_POSITIVE_Z : return MATH::SIGNEDAXIS_NEGATIVE_Z;
        default : return MATH::SIGNEDAXIS_POSITIVE_Z;
    }
}

MATH::TSignedAxis
Same( ufbx_coordinate_axis axis )
{
    switch ( axis )
    {
        case UFBX_COORDINATE_AXIS_POSITIVE_X : return MATH::SIGNEDAXIS_POSITIVE_X;
        case UFBX_COORDINATE_AXIS_NEGATIVE_X : return MATH::SIGNEDAXIS_NEGATIVE_X;
        case UFBX_COORDINATE_AXIS_POSITIVE_Y : return MATH::SIGNEDAXIS_POSITIVE_Y;
        case UFBX_COORDINATE_AXIS_NEGATIVE_Y : return MATH::SIGNEDAXIS_NEGATIVE_Y;
        case UFBX_COORDINATE_AXIS_POSITIVE_Z : return MATH::SIGNEDAXIS_POSITIVE_Z;
        default : return MATH::SIGNEDAXIS_NEGATIVE_Z;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFbxSceneReader::CFbxSceneReader( const CORE::CString& vfsPath                  ,
                                  ASSETCONVERTER::CAssetPackContext& context    ,
                                  ASSETCONVERTER::CAssetScene& scene            ,
                                  ASSETCONVERTER::CConversionReport& report     )
    : m_vfsPath( vfsPath )
    , m_context( context )
    , m_scene( scene )
    , m_report( report )
    , m_nodeMap()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFbxSceneReader::~CFbxSceneReader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CFbxSceneReader::Read( void )
{GUCEF_TRACE;

    CFbxDocument document;
    CORE::CString problem;
    if ( !document.Load( m_vfsPath, problem ) )
    {
        m_report.AddError( SourceName, m_vfsPath, problem );
        return false;
    }
    const ufbx_scene& fbx = *document.GetScene();

    ReadFacts( fbx );
    ReadCoordinateSystem( fbx );

    m_nodeMap.assign( fbx.nodes.count, -1 );
    for ( size_t c=0; c<fbx.root_node->children.count; ++c )
        ReadNode( fbx, *fbx.root_node->children.data[ c ], -1 );

    CFbxMaterialReader materials( fbx, m_vfsPath, m_context, m_scene, m_report );
    materials.ReadMaterials();
    ReadMeshes( fbx );
    ReadLights( fbx );
    ReadLodGroups( fbx );
    CFbxAnimationReader animations( fbx, m_nodeMap, m_scene, m_report );
    animations.ReadAnimations();

    if ( fbx.cameras.count > 0 )
        m_report.AddSkipped( SourceName, m_vfsPath, CORE::UInt32ToString( (UInt32) fbx.cameras.count ) + " camera(s) are not part of the model" );
    m_report.AddInfo( SourceName, m_vfsPath, "read " + CORE::UInt32ToString( (UInt32) m_scene.nodes.size() ) + " nodes, " + CORE::UInt32ToString( (UInt32) m_scene.meshes.size() ) + " meshes, " +
                      CORE::UInt32ToString( (UInt32) m_scene.materials.size() ) + " materials, " + CORE::UInt32ToString( (UInt32) m_scene.animations.size() ) + " animations" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFbxSceneReader::ReadFacts( const ufbx_scene& fbx )
{GUCEF_TRACE;

    if ( m_scene.provenance.sourceFormat.IsNULLOrEmpty() )
        m_scene.provenance.sourceFormat = "fbx";
    m_scene.provenance.sourceFormatVersion = CORE::UInt32ToString( fbx.metadata.version ) + ( fbx.metadata.ascii ? " ascii" : " binary" );
    m_scene.provenance.sourceEngine = ToString( fbx.metadata.creator );
    m_scene.normalMapConvention = ASSETCONVERTER::NORMALMAPCONVENTION_OPENGL;

    CORE::CDataNode* facts = m_scene.extras.AddChild( "fbx", GUCEF_DATATYPE_OBJECT );
    facts->SetAttribute( "version", fbx.metadata.version );
    facts->SetAttribute( "ascii", fbx.metadata.ascii );
    facts->SetAttribute( "creator", ToString( fbx.metadata.creator ) );
    facts->SetAttribute( "framesPerSecond", (Float32) fbx.settings.frames_per_second );
    facts->SetAttribute( "unitMeters", (Float32) fbx.settings.unit_meters );
    facts->SetAttribute( "originalUnitMeters", (Float32) fbx.settings.original_unit_meters );
    facts->SetAttribute( "animationStacks", (UInt32) fbx.anim_stacks.count );

    for ( size_t w=0; w<fbx.metadata.warnings.count && w<32; ++w )
        m_report.AddWarning( SourceName, m_vfsPath, "ufbx: " + ToString( fbx.metadata.warnings.data[ w ].description ) );
}

/*-------------------------------------------------------------------------*/

void
CFbxSceneReader::ReadCoordinateSystem( const ufbx_scene& fbx )
{GUCEF_TRACE;

    const ufbx_coordinate_axes& axes = fbx.settings.axes;
    Float64 unitsPerMetre = fbx.settings.unit_meters > 0.0 ? 1.0 / fbx.settings.unit_meters : 1.0;
    const Float64 rounded = floor( unitsPerMetre + 0.5 );
    if ( fabs( unitsPerMetre - rounded ) < 1e-6 * unitsPerMetre )
        unitsPerMetre = rounded;

    if ( UFBX_COORDINATE_AXIS_UNKNOWN == axes.right || UFBX_COORDINATE_AXIS_UNKNOWN == axes.up || UFBX_COORDINATE_AXIS_UNKNOWN == axes.front )
    {
        m_scene.coordinateSystem = MATH::CCoordinateSystem( MATH::SIGNEDAXIS_POSITIVE_X, MATH::SIGNEDAXIS_POSITIVE_Y, MATH::SIGNEDAXIS_NEGATIVE_Z, unitsPerMetre );
        m_report.AddWarning( SourceName, m_vfsPath, "the file does not state its axes: Y up, right handed is assumed" );
        return;
    }

    // 'front' of FBX is the opposite of forward
    const MATH::CCoordinateSystem system( Same( axes.right ), Same( axes.up ), Opposite( axes.front ), unitsPerMetre );
    if ( !system.IsValid() )
    {
        m_scene.coordinateSystem = MATH::CCoordinateSystem( MATH::SIGNEDAXIS_POSITIVE_X, MATH::SIGNEDAXIS_POSITIVE_Y, MATH::SIGNEDAXIS_NEGATIVE_Z, unitsPerMetre );
        m_report.AddWarning( SourceName, m_vfsPath, "the axes of the file are not valid: Y up, right handed is assumed" );
        return;
    }
    m_scene.coordinateSystem = system;
}

/*-------------------------------------------------------------------------*/

void
CFbxSceneReader::ReadNode( const ufbx_scene& fbx, const ufbx_node& node, Int32 parent )
{GUCEF_TRACE;

    const UInt32 index = m_scene.AddNode( ToString( node.name ), parent );
    m_nodeMap[ node.typed_id ] = (Int32) index;

    MATH::TFloat32Transform3D& transform = m_scene.nodes[ index ].transform;
    transform.position = MATH::TFloat32Transform3D::TPositionVector( (Float32) node.local_transform.translation.x, (Float32) node.local_transform.translation.y, (Float32) node.local_transform.translation.z );
    transform.rotation.x = (Float32) node.local_transform.rotation.x;
    transform.rotation.y = (Float32) node.local_transform.rotation.y;
    transform.rotation.z = (Float32) node.local_transform.rotation.z;
    transform.rotation.w = (Float32) node.local_transform.rotation.w;
    transform.scale.x = (Float32) node.local_transform.scale.x;
    transform.scale.y = (Float32) node.local_transform.scale.y;
    transform.scale.z = (Float32) node.local_transform.scale.z;

    for ( size_t c=0; c<node.children.count; ++c )
        ReadNode( fbx, *node.children.data[ c ], (Int32) index );
}

/*-------------------------------------------------------------------------*/

void
CFbxSceneReader::ReadMeshes( const ufbx_scene& fbx )
{GUCEF_TRACE;

    CFbxMeshReader meshes( fbx, m_nodeMap, m_scene, m_report );
    for ( size_t n=0; n<fbx.nodes.count; ++n )
    {
        const ufbx_node& node = *fbx.nodes.data[ n ];
        if ( GUCEF_NULL == node.mesh || m_nodeMap[ n ] < 0 )
            continue;
        Int32 mesh = -1;
        Int32 skin = -1;
        if ( meshes.ReadNodeMesh( node, mesh, skin ) )
        {
            m_scene.nodes[ m_nodeMap[ n ] ].mesh = mesh;
            m_scene.nodes[ m_nodeMap[ n ] ].skin = skin;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CFbxSceneReader::ReadLights( const ufbx_scene& fbx )
{GUCEF_TRACE;

    static const Float32 degreesToRadians = 0.0174532925f;
    const MATH::CTVector3D< Float32 > forward = m_scene.coordinateSystem.GetForward< Float32 >();
    for ( size_t n=0; n<fbx.nodes.count; ++n )
    {
        const ufbx_node& node = *fbx.nodes.data[ n ];
        if ( GUCEF_NULL == node.light || m_nodeMap[ n ] < 0 )
            continue;
        const ufbx_light& source = *node.light;
        if ( UFBX_LIGHT_AREA == source.type || UFBX_LIGHT_VOLUME == source.type )
        {
            m_report.AddSkipped( SourceName, ToString( node.name ), "area and volume lights are not part of the model" );
            continue;
        }

        ASSETCONVERTER::CAssetLight light;
        light.name = ToString( source.name );
        light.type = UFBX_LIGHT_DIRECTIONAL == source.type ? ASSETCONVERTER::LIGHTTYPE_DIRECTIONAL : ( UFBX_LIGHT_SPOT == source.type ? ASSETCONVERTER::LIGHTTYPE_SPOT : ASSETCONVERTER::LIGHTTYPE_POINT );
        light.colour[ 0 ] = (Float32) source.color.x;
        light.colour[ 1 ] = (Float32) source.color.y;
        light.colour[ 2 ] = (Float32) source.color.z;
        light.intensity = (Float32) source.intensity;
        light.innerConeAngle = (Float32) source.inner_angle * degreesToRadians;
        light.outerConeAngle = (Float32) source.outer_angle * degreesToRadians;
        m_scene.lights.push_back( light );
        const Int32 lightIndex = (Int32) m_scene.lights.size() - 1;

        // the light points along the forward axis of the scene; where the file aims it elsewhere a child node turns it
        Int32 holder = m_nodeMap[ n ];
        if ( UFBX_LIGHT_POINT != source.type )
        {
            const double length = sqrt( source.local_direction.x * source.local_direction.x + source.local_direction.y * source.local_direction.y + source.local_direction.z * source.local_direction.z );
            if ( length > 1e-9 )
            {
                const Float32 dx = (Float32) ( source.local_direction.x / length ), dy = (Float32) ( source.local_direction.y / length ), dz = (Float32) ( source.local_direction.z / length );
                const Float32 dot = forward.x * dx + forward.y * dy + forward.z * dz;
                if ( dot < 0.99999f )
                {
                    Float32 qx = forward.y * dz - forward.z * dy;
                    Float32 qy = forward.z * dx - forward.x * dz;
                    Float32 qz = forward.x * dy - forward.y * dx;
                    Float32 qw = 1.0f + dot;
                    if ( dot < -0.99999f )
                    {
                        // opposite directions: a half turn about any axis that is not along them
                        if ( fabs( forward.x ) < 0.9f ) { qx = 0.0f; qy = forward.z; qz = -forward.y; }
                        else { qx = -forward.z; qy = 0.0f; qz = forward.x; }
                        qw = 0.0f;
                    }
                    const Float32 norm = (Float32) sqrt( qx * qx + qy * qy + qz * qz + qw * qw );
                    const UInt32 child = m_scene.AddNode( ToString( node.name ) + "_light", holder );
                    m_scene.nodes[ child ].transform.rotation.x = qx / norm;
                    m_scene.nodes[ child ].transform.rotation.y = qy / norm;
                    m_scene.nodes[ child ].transform.rotation.z = qz / norm;
                    m_scene.nodes[ child ].transform.rotation.w = qw / norm;
                    holder = (Int32) child;
                }
            }
        }
        m_scene.nodes[ holder ].light = lightIndex;
    }
}

/*-------------------------------------------------------------------------*/

void
CFbxSceneReader::ReadLodGroups( const ufbx_scene& fbx )
{GUCEF_TRACE;

    for ( size_t n=0; n<fbx.nodes.count; ++n )
    {
        const ufbx_node& node = *fbx.nodes.data[ n ];
        if ( UFBX_ELEMENT_LOD_GROUP != node.attrib_type || m_nodeMap[ n ] < 0 || node.children.count < 2 )
            continue;
        ASSETCONVERTER::CAssetLodGroup group;
        group.name = ToString( node.name );
        for ( size_t c=0; c<node.children.count; ++c )
            if ( m_nodeMap[ node.children.data[ c ]->typed_id ] >= 0 )
                group.AddLevel( (UInt32) m_nodeMap[ node.children.data[ c ]->typed_id ], -1.0f );
        m_scene.lodGroups.push_back( group );
        m_scene.nodes[ m_nodeMap[ n ] ].lodGroup = (Int32) m_scene.lodGroups.size() - 1;
        m_report.AddApproximation( SourceName, group.name, "the LOD group of the file has distances, which are not converted to screen sizes: the levels are in the order of the children" );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FBX */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
