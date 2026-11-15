/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#include "OgreSkeletonManager.h"
#include "OgreSkeletonSerializer.h"
#include "OgreBone.h"
#include "OgreAnimation.h"
#include "OgreKeyFrame.h"

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "assetcookerOGRE_CBufferDataStream.h"
#include "assetcookerOGRE_CNodeAnimationSampler.h"
#include "assetcookerOGRE_COgreCookEnvironment.h"
#include "assetcookerOGRE_COutputNames.h"
#include "assetcookerOGRE_CSkeletonCooker.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "ogre";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSkeletonCooker::CSkeletonCooker( const ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report )
    : m_scene( scene )
    , m_report( report )
    , m_skeleton()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSkeletonCooker::~CSkeletonCooker()
{GUCEF_TRACE;

    Destroy();
}

/*-------------------------------------------------------------------------*/

const Ogre::SkeletonPtr&
CSkeletonCooker::GetSkeleton( void ) const
{GUCEF_TRACE;

    return m_skeleton;
}

/*-------------------------------------------------------------------------*/

void
CSkeletonCooker::Destroy( void )
{GUCEF_TRACE;

    if ( m_skeleton && GUCEF_NULL != Ogre::SkeletonManager::getSingletonPtr() )
        Ogre::SkeletonManager::getSingleton().remove( m_skeleton );
    m_skeleton.reset();
}

/*-------------------------------------------------------------------------*/

bool
CSkeletonCooker::Build( UInt32 skinIndex, const CORE::CString& resourceName )
{GUCEF_TRACE;

    Destroy();
    if ( skinIndex >= m_scene.skins.size() )
        return false;
    const ASSETCONVERTER::CAssetSkin& skin = m_scene.skins[ skinIndex ];
    const size_t jointCount = skin.joints.size();
    if ( 0 == jointCount || jointCount > 65535 )
    {
        m_report.AddError( SourceName, skin.name, "the skin has " + CORE::UInt32ToString( (UInt32) jointCount ) + " joints; an Ogre skeleton takes 1 to 65535" );
        return false;
    }

    // the joints by node
    GUCEF::map< UInt32, UInt32 > jointOfNode;
    for ( size_t j=0; j<jointCount; ++j )
        if ( skin.joints[ j ] < m_scene.nodes.size() && jointOfNode.find( skin.joints[ j ] ) == jointOfNode.end() )
            jointOfNode[ skin.joints[ j ] ] = (UInt32) j;

    // per joint: the parent joint, the rest transform from the frame of the parent joint (or the scene) to the frame of
    // the parent node, and the world bind transform
    GUCEF::vector< Int32 > parentJoint( jointCount, -1 );
    GUCEF::vector< Ogre::Affine3 > parentOffset( jointCount, Ogre::Affine3::IDENTITY );
    GUCEF::vector< Ogre::Affine3 > bindWorld( jointCount, Ogre::Affine3::IDENTITY );
    bool missingMatrices = false;
    for ( size_t j=0; j<jointCount; ++j )
    {
        const UInt32 node = skin.joints[ j ];
        if ( node >= m_scene.nodes.size() )
            continue;
        Ogre::Affine3 offset = Ogre::Affine3::IDENTITY;
        Int32 ancestor = m_scene.nodes[ node ].parent;
        UInt32 guard = 0;
        while ( ancestor >= 0 && (size_t) ancestor < m_scene.nodes.size() && guard++ < m_scene.nodes.size() )
        {
            GUCEF::map< UInt32, UInt32 >::const_iterator found = jointOfNode.find( (UInt32) ancestor );
            if ( found != jointOfNode.end() )
            {
                parentJoint[ j ] = (Int32) (*found).second;
                break;
            }
            offset = CSceneMath::ToAffine( m_scene.nodes[ ancestor ].transform ) * offset;
            ancestor = m_scene.nodes[ ancestor ].parent;
        }
        parentOffset[ j ] = offset;
        if ( j < skin.inverseBindMatrices.size() )
        {
            bindWorld[ j ] = CSceneMath::ToAffine( skin.inverseBindMatrices[ j ] ).inverse();
        }
        else
        {
            bindWorld[ j ] = CSceneMath::GetWorld( m_scene, (Int32) node );
            missingMatrices = true;
        }
    }
    if ( missingMatrices )
        m_report.AddApproximation( SourceName, skin.name, "the skin lacks inverse bind matrices: the rest pose of its joints is used as the binding pose" );

    try
    {
        m_skeleton = Ogre::SkeletonManager::getSingleton().create( resourceName.STL_String(), COgreCookEnvironment::ResourceGroupName.STL_String(), true );

        // the bones with their binding pose, relative to the parent bone
        CORE::CString::StringSet boneNames;
        GUCEF::vector< CSceneMath::STrs > bindLocal( jointCount );
        for ( size_t j=0; j<jointCount; ++j )
        {
            const UInt32 node = skin.joints[ j ];
            const CORE::CString nodeName = node < m_scene.nodes.size() ? m_scene.nodes[ node ].name : CORE::CString::Empty;
            const CORE::CString boneName = COutputNames::MakeUnique( COutputNames::Sanitize( nodeName, "bone" + CORE::UInt32ToString( (UInt32) j ) ), CORE::CString::Empty, boneNames );
            const Ogre::Affine3 local = parentJoint[ j ] >= 0 ? bindWorld[ parentJoint[ j ] ].inverse() * bindWorld[ j ] : bindWorld[ j ];
            bindLocal[ j ] = CSceneMath::Decompose( local );
            Ogre::Bone* bone = m_skeleton->createBone( boneName.STL_String(), (unsigned short) j );
            bone->setPosition( bindLocal[ j ].position );
            bone->setOrientation( bindLocal[ j ].rotation );
            bone->setScale( bindLocal[ j ].scale );
        }
        for ( size_t j=0; j<jointCount; ++j )
            if ( parentJoint[ j ] >= 0 )
                m_skeleton->getBone( (unsigned short) parentJoint[ j ] )->addChild( m_skeleton->getBone( (unsigned short) j ) );
        m_skeleton->setBindingPose();

        // the joints whose rest pose is not their binding pose
        GUCEF::vector< bool > restDiffers( jointCount, false );
        UInt32 restDiffersCount = 0;
        for ( size_t j=0; j<jointCount; ++j )
        {
            const UInt32 node = skin.joints[ j ];
            if ( node >= m_scene.nodes.size() )
                continue;
            const CSceneMath::STrs rest = CSceneMath::Decompose( parentOffset[ j ] * CSceneMath::ToAffine( m_scene.nodes[ node ].transform ) );
            restDiffers[ j ] = !CSceneMath::IsNear( rest, bindLocal[ j ] );
            if ( restDiffers[ j ] )
                ++restDiffersCount;
        }

        // the clips
        CORE::CString::StringSet animationNames;
        for ( size_t a=0; a<m_scene.animations.size(); ++a )
        {
            const ASSETCONVERTER::CAssetAnimation& clip = m_scene.animations[ a ];
            GUCEF::vector< CNodeAnimationSampler > samplers;
            bool animatesJoint = false;
            bool nonLinear = false;
            for ( size_t j=0; j<jointCount; ++j )
            {
                samplers.push_back( CNodeAnimationSampler( m_scene, clip, skin.joints[ j ] ) );
                animatesJoint = animatesJoint || samplers.back().IsAnimated();
                nonLinear = nonLinear || samplers.back().HasNonLinearKeys();
            }
            if ( !animatesJoint )
                continue;

            Float32 length = clip.GetDuration();
            if ( length < 0.0f )
                length = 0.0f;
            const CORE::CString name = COutputNames::MakeUnique( clip.name.IsNULLOrEmpty() ? "clip" + CORE::UInt32ToString( (UInt32) a ) : clip.name, CORE::CString::Empty, animationNames );
            Ogre::Animation* animation = m_skeleton->createAnimation( name.STL_String(), length );
            animation->setInterpolationMode( Ogre::Animation::IM_LINEAR );
            animation->setRotationInterpolationMode( Ogre::Animation::RIM_SPHERICAL );
            for ( size_t j=0; j<jointCount; ++j )
            {
                const bool animated = samplers[ j ].IsAnimated();
                if ( !animated && !restDiffers[ j ] )
                    continue;
                CNodeAnimationSampler::TFloatVector times;
                if ( animated )
                    samplers[ j ].GetKeyTimes( times );
                if ( times.empty() )
                    times.push_back( 0.0f );
                Ogre::Bone* bone = m_skeleton->getBone( (unsigned short) j );
                Ogre::NodeAnimationTrack* track = animation->createNodeTrack( (unsigned short) j, bone );
                for ( size_t k=0; k<times.size(); ++k )
                {
                    const Float32 time = times[ k ] < 0.0f ? 0.0f : ( times[ k ] > length ? length : times[ k ] );
                    const CSceneMath::STrs pose = CSceneMath::Decompose( parentOffset[ j ] * CSceneMath::ToAffine( samplers[ j ].Sample( times[ k ] ) ) );
                    Ogre::TransformKeyFrame* key = track->createNodeKeyFrame( time );
                    key->setTranslate( pose.position - bindLocal[ j ].position );
                    Ogre::Quaternion rotation = bindLocal[ j ].rotation.Inverse() * pose.rotation;
                    rotation.normalise();
                    key->setRotation( rotation );
                    const Ogre::Vector3& bindScale = bindLocal[ j ].scale;
                    key->setScale( Ogre::Vector3( 0 != bindScale.x ? pose.scale.x / bindScale.x : 1, 0 != bindScale.y ? pose.scale.y / bindScale.y : 1, 0 != bindScale.z ? pose.scale.z / bindScale.z : 1 ) );
                }
            }
            if ( nonLinear )
                m_report.AddApproximation( SourceName, clip.name, "step or cubic spline keys of joints are played back linearly between their values" );
        }
        if ( restDiffersCount > 0 )
            m_report.AddInfo( SourceName, skin.name, CORE::UInt32ToString( restDiffersCount ) + " joint(s) rest away from the binding pose: the clips hold them at rest" );
    }
    catch ( const Ogre::Exception& e )
    {
        m_report.AddError( SourceName, skin.name, "the skeleton cannot be built: " + CORE::CString( e.getDescription() ) );
        Destroy();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSkeletonCooker::Save( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    if ( !m_skeleton )
        return false;
    try
    {
        CBufferDataStream* buffer = OGRE_NEW CBufferDataStream();
        Ogre::DataStreamPtr stream( buffer );
        Ogre::SkeletonSerializer serializer;
        serializer.exportSkeleton( m_skeleton.get(), stream, Ogre::SKELETON_VERSION_LATEST, Ogre::Serializer::ENDIAN_LITTLE );
        if ( !buffer->StoreInVfs( vfsPath ) )
        {
            m_report.AddError( SourceName, vfsPath, "the skeleton cannot be written" );
            return false;
        }
    }
    catch ( const Ogre::Exception& e )
    {
        m_report.AddError( SourceName, vfsPath, "the skeleton cannot be serialised: " + CORE::CString( e.getDescription() ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
