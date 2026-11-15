/*
 *  assetpluginFBX: the animation stacks of an FBX scene as clips of the converter model
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

#include "assetpluginFBX_CFbxMeshReader.h"
#include "assetpluginFBX_CFbxAnimationReader.h"

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

/** Keys at strictly increasing times, none before zero */
bool
AddTime( GUCEF::vector< Float32 >& times, double time )
{
    const Float32 value = time < 0.0 ? 0.0f : (Float32) time;
    if ( !times.empty() && value <= times.back() )
        return false;
    times.push_back( value );
    return true;
}

bool
Differs( const ufbx_vec3& a, const ufbx_vec3& b )
{
    return fabs( a.x - b.x ) > 1e-6 * ( 1.0 + fabs( b.x ) ) || fabs( a.y - b.y ) > 1e-6 * ( 1.0 + fabs( b.y ) ) || fabs( a.z - b.z ) > 1e-6 * ( 1.0 + fabs( b.z ) );
}

bool
Differs( const ufbx_quat& a, const ufbx_quat& b )
{
    // q and -q are the same rotation
    const double dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return fabs( dot ) < 1.0 - 1e-9;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFbxAnimationReader::CFbxAnimationReader( const ufbx_scene& fbx                     ,
                                          const TIndexVector& nodeMap               ,
                                          ASSETCONVERTER::CAssetScene& scene        ,
                                          ASSETCONVERTER::CConversionReport& report )
    : m_fbx( fbx )
    , m_nodeMap( nodeMap )
    , m_scene( scene )
    , m_report( report )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFbxAnimationReader::~CFbxAnimationReader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CFbxAnimationReader::ReadAnimations( void )
{GUCEF_TRACE;

    for ( size_t s=0; s<m_fbx.anim_stacks.count; ++s )
        ReadStack( *m_fbx.anim_stacks.data[ s ] );
}

/*-------------------------------------------------------------------------*/

void
CFbxAnimationReader::ReadStack( const ufbx_anim_stack& stack )
{GUCEF_TRACE;

    ufbx_bake_opts options;
    memset( &options, 0, sizeof( options ) );
    options.trim_start_time = true;
    options.key_reduction_enabled = true;
    options.key_reduction_rotation = true;

    ufbx_error error;
    memset( &error, 0, sizeof( error ) );
    ufbx_baked_anim* baked = ufbx_bake_anim( &m_fbx, stack.anim, &options, &error );
    if ( GUCEF_NULL == baked )
    {
        m_report.AddWarning( SourceName, ToString( stack.name ), "the animation take cannot be baked: " + CORE::CString( error.description.data, (UInt32) error.description.length ) );
        return;
    }

    ASSETCONVERTER::CAssetAnimation animation;
    animation.name = ToString( stack.name );
    animation.sourceTake = animation.name;
    animation.sampled = true;
    animation.loop = false;
    animation.frameRate = m_fbx.settings.frames_per_second > 0.0 ? (Float32) m_fbx.settings.frames_per_second : 30.0f;
    animation.firstFrame = (Float32) ( stack.time_begin * animation.frameRate );
    animation.lastFrame = (Float32) ( stack.time_end * animation.frameRate );

    for ( size_t b=0; b<baked->nodes.count; ++b )
    {
        const ufbx_baked_node& bakedNode = baked->nodes.data[ b ];
        if ( bakedNode.typed_id >= m_nodeMap.size() || m_nodeMap[ bakedNode.typed_id ] < 0 )
            continue;
        const ufbx_node& node = *m_fbx.nodes.data[ bakedNode.typed_id ];
        const UInt32 target = (UInt32) m_nodeMap[ bakedNode.typed_id ];

        if ( bakedNode.translation_keys.count > 0 && ( !bakedNode.constant_translation || Differs( bakedNode.translation_keys.data[ 0 ].value, node.local_transform.translation ) ) )
        {
            ASSETCONVERTER::CAssetAnimationChannel channel;
            channel.targetNode = target;
            channel.path = ASSETCONVERTER::ANIMATIONPATH_TRANSLATION;
            for ( size_t k=0; k<bakedNode.translation_keys.count; ++k )
            {
                const ufbx_baked_vec3& key = bakedNode.translation_keys.data[ k ];
                if ( !AddTime( channel.times, key.time ) )
                    continue;
                channel.values.push_back( (Float32) key.value.x );
                channel.values.push_back( (Float32) key.value.y );
                channel.values.push_back( (Float32) key.value.z );
            }
            animation.channels.push_back( channel );
        }
        if ( bakedNode.rotation_keys.count > 0 && ( !bakedNode.constant_rotation || Differs( bakedNode.rotation_keys.data[ 0 ].value, node.local_transform.rotation ) ) )
        {
            ASSETCONVERTER::CAssetAnimationChannel channel;
            channel.targetNode = target;
            channel.path = ASSETCONVERTER::ANIMATIONPATH_ROTATION;
            for ( size_t k=0; k<bakedNode.rotation_keys.count; ++k )
            {
                const ufbx_baked_quat& key = bakedNode.rotation_keys.data[ k ];
                if ( !AddTime( channel.times, key.time ) )
                    continue;
                channel.values.push_back( (Float32) key.value.x );
                channel.values.push_back( (Float32) key.value.y );
                channel.values.push_back( (Float32) key.value.z );
                channel.values.push_back( (Float32) key.value.w );
            }
            animation.channels.push_back( channel );
        }
        if ( bakedNode.scale_keys.count > 0 && ( !bakedNode.constant_scale || Differs( bakedNode.scale_keys.data[ 0 ].value, node.local_transform.scale ) ) )
        {
            ASSETCONVERTER::CAssetAnimationChannel channel;
            channel.targetNode = target;
            channel.path = ASSETCONVERTER::ANIMATIONPATH_SCALE;
            for ( size_t k=0; k<bakedNode.scale_keys.count; ++k )
            {
                const ufbx_baked_vec3& key = bakedNode.scale_keys.data[ k ];
                if ( !AddTime( channel.times, key.time ) )
                    continue;
                channel.values.push_back( (Float32) key.value.x );
                channel.values.push_back( (Float32) key.value.y );
                channel.values.push_back( (Float32) key.value.z );
            }
            animation.channels.push_back( channel );
        }
    }
    ReadWeights( stack, *baked, animation );

    if ( animation.channels.empty() )
        m_report.AddInfo( SourceName, animation.name, "the take animates nothing: no clip" );
    else
        m_scene.animations.push_back( animation );
    ufbx_free_baked_anim( baked );
}

/*-------------------------------------------------------------------------*/

void
CFbxAnimationReader::ReadWeights( const ufbx_anim_stack& stack, const ufbx_baked_anim& baked, ASSETCONVERTER::CAssetAnimation& animation )
{GUCEF_TRACE;

    for ( size_t n=0; n<m_fbx.nodes.count; ++n )
    {
        const ufbx_node& node = *m_fbx.nodes.data[ n ];
        if ( GUCEF_NULL == node.mesh || m_nodeMap[ n ] < 0 )
            continue;
        const Int32 meshIndex = m_scene.nodes[ m_nodeMap[ n ] ].mesh;
        if ( meshIndex < 0 )
            continue;
        GUCEF::vector< const ufbx_blend_channel* > channels;
        CFbxMeshReader::GetBlendChannels( *node.mesh, channels );
        if ( channels.empty() || m_scene.meshes[ meshIndex ].subMeshes.empty() || m_scene.meshes[ meshIndex ].subMeshes[ 0 ].morphTargets.size() != channels.size() )
            continue;

        // the times of the keys of the animated channels
        GUCEF::set< double > moments;
        for ( size_t e=0; e<baked.elements.count; ++e )
        {
            for ( size_t c=0; c<channels.size(); ++c )
            {
                if ( baked.elements.data[ e ].element_id != channels[ c ]->element_id )
                    continue;
                for ( size_t p=0; p<baked.elements.data[ e ].props.count; ++p )
                {
                    const ufbx_baked_prop& prop = baked.elements.data[ e ].props.data[ p ];
                    if ( prop.constant_value )
                        continue;
                    for ( size_t k=0; k<prop.keys.count; ++k )
                        moments.insert( prop.keys.data[ k ].time );
                }
            }
        }
        if ( moments.empty() )
            continue;

        ASSETCONVERTER::CAssetAnimationChannel channel;
        channel.targetNode = (UInt32) m_nodeMap[ n ];
        channel.path = ASSETCONVERTER::ANIMATIONPATH_WEIGHTS;
        GUCEF::set< double >::const_iterator t = moments.begin();
        while ( t != moments.end() )
        {
            if ( AddTime( channel.times, *t ) )
                for ( size_t c=0; c<channels.size(); ++c )
                    channel.values.push_back( (Float32) ufbx_evaluate_blend_weight( stack.anim, channels[ c ], *t + stack.time_begin ) );
            ++t;
        }
        animation.channels.push_back( channel );
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
