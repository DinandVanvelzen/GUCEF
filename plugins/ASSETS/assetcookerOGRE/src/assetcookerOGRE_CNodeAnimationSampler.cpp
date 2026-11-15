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

#include <algorithm>
#include <math.h>

#include "assetcookerOGRE_CNodeAnimationSampler.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNodeAnimationSampler::CNodeAnimationSampler( const ASSETCONVERTER::CAssetScene& scene, const ASSETCONVERTER::CAssetAnimation& animation, UInt32 node )
    : m_rest()
{GUCEF_TRACE;

    m_channels[ 0 ] = m_channels[ 1 ] = m_channels[ 2 ] = GUCEF_NULL;
    if ( node < scene.nodes.size() )
        m_rest = CSceneMath::ToTrs( scene.nodes[ node ].transform );
    for ( size_t c=0; c<animation.channels.size(); ++c )
    {
        const ASSETCONVERTER::CAssetAnimationChannel& channel = animation.channels[ c ];
        if ( channel.targetNode != node || channel.times.empty() )
            continue;
        if ( channel.path <= ASSETCONVERTER::ANIMATIONPATH_SCALE && GUCEF_NULL == m_channels[ channel.path ] )
            m_channels[ channel.path ] = &channel;
    }
}

/*-------------------------------------------------------------------------*/

bool
CNodeAnimationSampler::IsAnimated( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_channels[ 0 ] || GUCEF_NULL != m_channels[ 1 ] || GUCEF_NULL != m_channels[ 2 ];
}

/*-------------------------------------------------------------------------*/

bool
CNodeAnimationSampler::HasNonLinearKeys( void ) const
{GUCEF_TRACE;

    for ( UInt32 p=0; p<3; ++p )
        if ( GUCEF_NULL != m_channels[ p ] && ASSETCONVERTER::ANIMATIONINTERPOLATION_LINEAR != m_channels[ p ]->interpolation && m_channels[ p ]->times.size() > 1 )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

const CSceneMath::STrs&
CNodeAnimationSampler::GetRest( void ) const
{GUCEF_TRACE;

    return m_rest;
}

/*-------------------------------------------------------------------------*/

void
CNodeAnimationSampler::GetKeyTimes( TFloatVector& times ) const
{GUCEF_TRACE;

    times.clear();
    for ( UInt32 p=0; p<3; ++p )
        if ( GUCEF_NULL != m_channels[ p ] )
            times.insert( times.end(), m_channels[ p ]->times.begin(), m_channels[ p ]->times.end() );
    std::sort( times.begin(), times.end() );
    times.erase( std::unique( times.begin(), times.end() ), times.end() );
}

/*-------------------------------------------------------------------------*/

void
CNodeAnimationSampler::ReadKey( const ASSETCONVERTER::CAssetAnimationChannel& channel, UInt32 key, UInt32 components, Float32* out )
{GUCEF_TRACE;

    const size_t keys = channel.times.size();
    const size_t stride = keys > 0 ? channel.values.size() / keys : 0;
    const size_t offset = ( ASSETCONVERTER::ANIMATIONINTERPOLATION_CUBICSPLINE == channel.interpolation && stride >= 3 * components ) ? components : 0;
    for ( UInt32 i=0; i<components; ++i )
    {
        const size_t index = (size_t) key * stride + offset + i;
        out[ i ] = index < channel.values.size() ? channel.values[ index ] : 0.0f;
    }
}

/*-------------------------------------------------------------------------*/

void
CNodeAnimationSampler::SamplePath( UInt32 path, Float32 time, Float32* out ) const
{GUCEF_TRACE;

    const ASSETCONVERTER::CAssetAnimationChannel& channel = *m_channels[ path ];
    const UInt32 components = ASSETCONVERTER::ANIMATIONPATH_ROTATION == path ? 4 : 3;
    const size_t keys = channel.times.size();

    // before the first and after the last key the value holds
    if ( keys == 1 || time <= channel.times[ 0 ] )
    {
        ReadKey( channel, 0, components, out );
        return;
    }
    if ( time >= channel.times[ keys-1 ] )
    {
        ReadKey( channel, (UInt32) keys-1, components, out );
        return;
    }
    size_t next = 1;
    while ( next < keys-1 && channel.times[ next ] < time )
        ++next;
    const size_t previous = next - 1;
    Float32 a[ 4 ];
    Float32 b[ 4 ];
    ReadKey( channel, (UInt32) previous, components, a );
    ReadKey( channel, (UInt32) next, components, b );
    if ( ASSETCONVERTER::ANIMATIONINTERPOLATION_STEP == channel.interpolation )
    {
        for ( UInt32 i=0; i<components; ++i )
            out[ i ] = time < channel.times[ next ] ? a[ i ] : b[ i ];
        return;
    }
    const Float32 span = channel.times[ next ] - channel.times[ previous ];
    const Float32 t = span > 0.0f ? ( time - channel.times[ previous ] ) / span : 0.0f;
    if ( 4 == components )
    {
        const Ogre::Quaternion qa( a[ 3 ], a[ 0 ], a[ 1 ], a[ 2 ] );
        const Ogre::Quaternion qb( b[ 3 ], b[ 0 ], b[ 1 ], b[ 2 ] );
        Ogre::Quaternion q = Ogre::Quaternion::Slerp( t, qa, qb, true );
        q.normalise();
        out[ 0 ] = (Float32) q.x; out[ 1 ] = (Float32) q.y; out[ 2 ] = (Float32) q.z; out[ 3 ] = (Float32) q.w;
        return;
    }
    for ( UInt32 i=0; i<components; ++i )
        out[ i ] = a[ i ] + ( b[ i ] - a[ i ] ) * t;
}

/*-------------------------------------------------------------------------*/

CSceneMath::STrs
CNodeAnimationSampler::Sample( Float32 time ) const
{GUCEF_TRACE;

    CSceneMath::STrs trs = m_rest;
    Float32 values[ 4 ];
    if ( GUCEF_NULL != m_channels[ ASSETCONVERTER::ANIMATIONPATH_TRANSLATION ] )
    {
        SamplePath( ASSETCONVERTER::ANIMATIONPATH_TRANSLATION, time, values );
        trs.position = Ogre::Vector3( values[ 0 ], values[ 1 ], values[ 2 ] );
    }
    if ( GUCEF_NULL != m_channels[ ASSETCONVERTER::ANIMATIONPATH_ROTATION ] )
    {
        SamplePath( ASSETCONVERTER::ANIMATIONPATH_ROTATION, time, values );
        trs.rotation = Ogre::Quaternion( values[ 3 ], values[ 0 ], values[ 1 ], values[ 2 ] );
        trs.rotation.normalise();
    }
    if ( GUCEF_NULL != m_channels[ ASSETCONVERTER::ANIMATIONPATH_SCALE ] )
    {
        SamplePath( ASSETCONVERTER::ANIMATIONPATH_SCALE, time, values );
        trs.scale = Ogre::Vector3( values[ 0 ], values[ 1 ], values[ 2 ] );
    }
    return trs;
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
