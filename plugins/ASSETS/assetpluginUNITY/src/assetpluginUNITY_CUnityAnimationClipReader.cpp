/*
 *  assetpluginUNITY: the reader of Unity animation clips (.anim)
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

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#include "assetpluginUNITY_CUnityYamlReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H ? */

#include "assetpluginUNITY_CUnityAnimationClipReader.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline bool IsFiniteValue( Float32 value )
{
    return value == value && value < 1e30f && value > -1e30f;
}

/** Unity's Euler angles in degrees, applied Z, then X, then Y */
MATH::TFloat32Quaternion
EulerToQuaternion( Float32 x, Float32 y, Float32 z )
{
    const Float32 toHalfRadians = 0.5f * 0.0174532925f;
    const MATH::TFloat32Quaternion qx = MATH::TFloat32Quaternion::FromXYZW( sinf( x * toHalfRadians ), 0.0f, 0.0f, cosf( x * toHalfRadians ) );
    const MATH::TFloat32Quaternion qy = MATH::TFloat32Quaternion::FromXYZW( 0.0f, sinf( y * toHalfRadians ), 0.0f, cosf( y * toHalfRadians ) );
    const MATH::TFloat32Quaternion qz = MATH::TFloat32Quaternion::FromXYZW( 0.0f, 0.0f, sinf( z * toHalfRadians ), cosf( z * toHalfRadians ) );
    return ( qy * qx * qz ).Normalized();
}

/** A property of the Transform that float curves animate component by component */
struct SFloatGroup
{
    UInt32 node;
    CORE::CString property;     /**< "position", "rotation", "scale", "euler" */
    GUCEF::vector< Float32 > defaults;
    bool has[ 4 ];
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityAnimationClipReader::CUnityAnimationClipReader( CUnityProject& project )
    : m_project( project )
    , m_nodes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CUnityAnimationClipReader::ReadKeys( const CORE::CDataNode* curve, UInt32 components, TKeyVector& keys )
{GUCEF_TRACE;

    keys.clear();
    if ( GUCEF_NULL == curve )
        return;
    CUnityYamlReader::TNodeList items;
    CUnityYamlReader::GetItems( curve->FindChild( "m_Curve" ), items );
    static const char* const names = "xyzw";
    for ( size_t i=0; i<items.size(); ++i )
    {
        const CORE::CDataNode& item = *items[ i ];
        SKey key;
        key.time = CUnityYamlReader::GetFloat( item, "time", 0.0f );
        for ( UInt32 c=0; c<4; ++c )
            key.value[ c ] = key.inSlope[ c ] = key.outSlope[ c ] = 0.0f;
        if ( 1 == components )
        {
            key.value[ 0 ] = CUnityYamlReader::GetFloat( item, "value", 0.0f );
            key.inSlope[ 0 ] = CUnityYamlReader::GetFloat( item, "inSlope", 0.0f );
            key.outSlope[ 0 ] = CUnityYamlReader::GetFloat( item, "outSlope", 0.0f );
        }
        else
        {
            char component[ 5 ] = { 0, 0, 0, 0, 0 };
            for ( UInt32 c=0; c<components; ++c )
                component[ c ] = names[ c ];
            CUnityYamlReader::GetComponents( item, "value", component, key.value );
            CUnityYamlReader::GetComponents( item, "inSlope", component, key.inSlope );
            CUnityYamlReader::GetComponents( item, "outSlope", component, key.outSlope );
        }
        key.weighted = 0 != CUnityYamlReader::GetInt32( item, "weightedMode", 0 );
        keys.push_back( key );
    }
}

/*-------------------------------------------------------------------------*/

Float32
CUnityAnimationClipReader::Evaluate( const TKeyVector& keys, UInt32 component, Float32 time )
{GUCEF_TRACE;

    if ( keys.empty() )
        return 0.0f;
    if ( time <= keys[ 0 ].time )
        return keys[ 0 ].value[ component ];
    if ( time >= keys.back().time )
        return keys.back().value[ component ];
    size_t k = 0;
    while ( k + 1 < keys.size() && keys[ k + 1 ].time <= time )
        ++k;
    const SKey& a = keys[ k ];
    const SKey& b = keys[ k + 1 ];
    const Float32 span = b.time - a.time;
    if ( span <= 0.0f || !IsFiniteValue( a.outSlope[ component ] ) || !IsFiniteValue( b.inSlope[ component ] ) )
        return a.value[ component ];
    const Float32 s = ( time - a.time ) / span;
    const Float32 s2 = s * s;
    const Float32 s3 = s2 * s;
    return ( 2.0f * s3 - 3.0f * s2 + 1.0f ) * a.value[ component ] + ( s3 - 2.0f * s2 + s ) * span * a.outSlope[ component ] +
           ( -2.0f * s3 + 3.0f * s2 ) * b.value[ component ] + ( s3 - s2 ) * span * b.inSlope[ component ];
}

/*-------------------------------------------------------------------------*/

UInt32
CUnityAnimationClipReader::GetNode( UInt32 rootNode, const CORE::CString& path )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return rootNode;
    GUCEF::map< CORE::CString, UInt32 >::const_iterator known = m_nodes.find( path );
    if ( known != m_nodes.end() )
        return (*known).second;
    const Int32 slash = path.HasChar( '/', false );
    const UInt32 parent = slash > 0 ? GetNode( rootNode, path.SubstrFromRange( 0, (UInt32) slash ) ) : rootNode;
    const CORE::CString name = slash >= 0 ? path.SubstrFromRange( (UInt32) slash + 1, path.Length() ) : path;
    const UInt32 node = m_project.GetScene().AddNode( name, (Int32) parent );
    m_nodes[ path ] = node;
    return node;
}

/*-------------------------------------------------------------------------*/

void
CUnityAnimationClipReader::AddVectorChannel( const TKeyVector& keys, UInt32 node, ASSETCONVERTER::TAnimationPath path, UInt32 components, Float32 sampleRate,
                                             const CORE::CString& subject, ASSETCONVERTER::CAssetAnimation& animation )
{GUCEF_TRACE;

    if ( keys.empty() )
        return;
    bool weighted = false;
    bool allFinite = true;
    bool allInfinite = true;
    for ( size_t k=0; k<keys.size(); ++k )
    {
        weighted = weighted || keys[ k ].weighted;
        for ( UInt32 c=0; c<components; ++c )
        {
            const bool finite = IsFiniteValue( keys[ k ].inSlope[ c ] ) && IsFiniteValue( keys[ k ].outSlope[ c ] );
            allFinite = allFinite && finite;
            allInfinite = allInfinite && !IsFiniteValue( keys[ k ].outSlope[ c ] );
        }
    }

    ASSETCONVERTER::CAssetAnimationChannel channel;
    channel.targetNode = node;
    channel.path = path;
    if ( !weighted && ( allFinite || allInfinite ) )
    {
        // exactly: a cubic spline with Unity's slopes as tangents, or steps
        channel.interpolation = allFinite ? ASSETCONVERTER::ANIMATIONINTERPOLATION_CUBICSPLINE : ASSETCONVERTER::ANIMATIONINTERPOLATION_STEP;
        for ( size_t k=0; k<keys.size(); ++k )
        {
            if ( !channel.times.empty() && keys[ k ].time <= channel.times.back() )
                continue;
            channel.times.push_back( keys[ k ].time );
            if ( allFinite )
                for ( UInt32 c=0; c<components; ++c )
                    channel.values.push_back( keys[ k ].inSlope[ c ] );
            for ( UInt32 c=0; c<components; ++c )
                channel.values.push_back( keys[ k ].value[ c ] );
            if ( allFinite )
                for ( UInt32 c=0; c<components; ++c )
                    channel.values.push_back( keys[ k ].outSlope[ c ] );
        }
    }
    else
    {
        // sampled at the rate of the clip
        channel.interpolation = ASSETCONVERTER::ANIMATIONINTERPOLATION_LINEAR;
        const Float32 step = 1.0f / ( sampleRate > 0.0f ? sampleRate : 60.0f );
        const Float32 first = keys[ 0 ].time;
        const Float32 last = keys.back().time;
        for ( Float32 t = first; ; t += step )
        {
            const Float32 time = t > last ? last : t;
            channel.times.push_back( time );
            Float32 length = 0.0f;
            GUCEF::vector< Float32 > value( components, 0.0f );
            for ( UInt32 c=0; c<components; ++c )
            {
                value[ c ] = Evaluate( keys, c, time );
                length += value[ c ] * value[ c ];
            }
            if ( ASSETCONVERTER::ANIMATIONPATH_ROTATION == path && length > 0.0f )
                for ( UInt32 c=0; c<components; ++c )
                    value[ c ] /= sqrtf( length );
            channel.values.insert( channel.values.end(), value.begin(), value.end() );
            if ( time >= last )
                break;
        }
        animation.sampled = true;
        m_project.Approximate( subject, CORE::CString( "a " ) + ASSETCONVERTER::ToString( path ) + " curve with weighted or mixed tangents is sampled at the rate of the clip" );
    }
    animation.channels.push_back( channel );
}

/*-------------------------------------------------------------------------*/

bool
CUnityAnimationClipReader::ReadClip( const CORE::CDataNode& clipObject, const CORE::CString& subject, UInt32 rootNode, ASSETCONVERTER::CAssetAnimation& animation )
{GUCEF_TRACE;

    animation = ASSETCONVERTER::CAssetAnimation();
    animation.name = CUnityYamlReader::GetString( clipObject, "m_Name" );
    animation.sourceTake = animation.name;
    animation.sampled = false;
    const Float32 sampleRate = CUnityYamlReader::GetFloat( clipObject, "m_SampleRate", 60.0f );
    animation.frameRate = sampleRate;
    const CORE::CDataNode* settings = clipObject.FindChild( "m_AnimationClipSettings" );
    if ( GUCEF_NULL != settings )
    {
        animation.loop = CUnityYamlReader::GetBool( *settings, "m_LoopTime", false );
        animation.firstFrame = CUnityYamlReader::GetFloat( *settings, "m_StartTime", 0.0f ) * sampleRate;
        animation.lastFrame = CUnityYamlReader::GetFloat( *settings, "m_StopTime", 0.0f ) * sampleRate;
    }

    CUnityYamlReader::TNodeList compressed;
    CUnityYamlReader::GetItems( clipObject.FindChild( "m_CompressedRotationCurves" ), compressed );
    if ( !compressed.empty() )
        m_project.Skip( subject, CORE::UInt32ToString( (UInt32) compressed.size() ) + " compressed rotation curve(s) are not decoded" );

    // the vector curves
    struct SVectorList { const char* key; ASSETCONVERTER::TAnimationPath path; UInt32 components; };
    static const SVectorList lists[] = { { "m_PositionCurves", ASSETCONVERTER::ANIMATIONPATH_TRANSLATION, 3 },
                                         { "m_RotationCurves", ASSETCONVERTER::ANIMATIONPATH_ROTATION, 4 },
                                         { "m_ScaleCurves", ASSETCONVERTER::ANIMATIONPATH_SCALE, 3 } };
    for ( UInt32 l=0; l<3; ++l )
    {
        CUnityYamlReader::TNodeList curves;
        CUnityYamlReader::GetItems( clipObject.FindChild( lists[ l ].key ), curves );
        for ( size_t c=0; c<curves.size(); ++c )
        {
            TKeyVector keys;
            ReadKeys( curves[ c ]->FindChild( "curve" ), lists[ l ].components, keys );
            const UInt32 node = GetNode( rootNode, CUnityYamlReader::GetString( *curves[ c ], "path" ) );
            AddVectorChannel( keys, node, lists[ l ].path, lists[ l ].components, sampleRate, subject, animation );
        }
    }

    // Euler curves and the float curves of the Transform, combined per property and sampled
    typedef GUCEF::map< CORE::CString, SFloatGroup > TGroupMap;
    typedef GUCEF::map< CORE::CString, GUCEF::vector< TKeyVector > > TGroupKeyMap;
    TGroupMap groups;
    TGroupKeyMap groupKeys;
    CUnityYamlReader::TNodeList curves;
    CUnityYamlReader::GetItems( clipObject.FindChild( "m_EulerCurves" ), curves );
    for ( size_t c=0; c<curves.size(); ++c )
    {
        TKeyVector keys;
        ReadKeys( curves[ c ]->FindChild( "curve" ), 3, keys );
        const CORE::CString path = CUnityYamlReader::GetString( *curves[ c ], "path" );
        SFloatGroup& group = groups[ path + "|euler" ];
        group.node = GetNode( rootNode, path );
        group.property = "euler";
        group.defaults.assign( 3, 0.0f );
        GUCEF::vector< TKeyVector >& perComponent = groupKeys[ path + "|euler" ];
        perComponent.assign( 3, TKeyVector() );
        for ( UInt32 a=0; a<3; ++a )
        {
            group.has[ a ] = true;
            for ( size_t k=0; k<keys.size(); ++k )
            {
                SKey single = keys[ k ];
                single.value[ 0 ] = keys[ k ].value[ a ];
                single.inSlope[ 0 ] = keys[ k ].inSlope[ a ];
                single.outSlope[ 0 ] = keys[ k ].outSlope[ a ];
                perComponent[ a ].push_back( single );
            }
        }
        group.has[ 3 ] = false;
    }
    CUnityYamlReader::GetItems( clipObject.FindChild( "m_FloatCurves" ), curves );
    UInt32 humanoid = 0;
    CORE::CString::StringSet others;
    for ( size_t c=0; c<curves.size(); ++c )
    {
        const CORE::CDataNode& curve = *curves[ c ];
        const Int32 classId = CUnityYamlReader::GetInt32( curve, "classID", 0 );
        const CORE::CString attribute = CUnityYamlReader::GetString( curve, "attribute" );
        const CORE::CString path = CUnityYamlReader::GetString( curve, "path" );
        if ( 95 == classId )
        {
            ++humanoid;
            continue;
        }
        const Int32 dot = attribute.HasChar( '.', false );
        const CORE::CString property = dot > 0 ? attribute.SubstrFromRange( 0, (UInt32) dot ) : attribute;
        const CORE::CString component = dot > 0 ? attribute.SubstrFromRange( (UInt32) dot + 1, attribute.Length() ) : CORE::CString::Empty;
        CORE::CString kind;
        if ( "m_LocalPosition" == property ) kind = "position";
        else if ( "m_LocalScale" == property ) kind = "scale";
        else if ( "m_LocalRotation" == property ) kind = "rotation";
        else if ( "localEulerAnglesRaw" == property || "localEulerAngles" == property || "m_LocalEulerAngles" == property || "localEulerAnglesBaked" == property ) kind = "euler";
        Int32 index = -1;
        if ( "x" == component ) index = 0; else if ( "y" == component ) index = 1; else if ( "z" == component ) index = 2; else if ( "w" == component ) index = 3;
        if ( 4 != classId || kind.IsNULLOrEmpty() || index < 0 || ( 3 == index && "rotation" != kind ) )
        {
            others.insert( attribute + " (class " + CORE::Int32ToString( classId ) + ")" );
            continue;
        }
        const CORE::CString key = path + "|" + kind;
        SFloatGroup& group = groups[ key ];
        GUCEF::vector< TKeyVector >& perComponent = groupKeys[ key ];
        if ( perComponent.empty() )
        {
            group.node = GetNode( rootNode, path );
            group.property = kind;
            const UInt32 components = "rotation" == kind ? 4 : 3;
            group.defaults.assign( components, "scale" == kind ? 1.0f : 0.0f );
            if ( "rotation" == kind )
                group.defaults[ 3 ] = 1.0f;
            perComponent.assign( components, TKeyVector() );
            for ( UInt32 a=0; a<4; ++a )
                group.has[ a ] = false;
        }
        ReadKeys( curve.FindChild( "curve" ), 1, perComponent[ index ] );
        group.has[ index ] = true;
    }
    if ( humanoid > 0 )
        m_project.Skip( subject, CORE::UInt32ToString( humanoid ) + " humanoid muscle curve(s) are not convertible: they need retargeting" );
    if ( !others.empty() )
    {
        CORE::CString list;
        for ( CORE::CString::StringSet::const_iterator o = others.begin(); o != others.end(); ++o )
            list += ( list.IsNULLOrEmpty() ? "" : ", " ) + (*o);
        m_project.Skip( subject, "curves of properties other than the transform are not converted: " + list );
    }
    for ( TGroupMap::const_iterator g = groups.begin(); g != groups.end(); ++g )
    {
        const SFloatGroup& group = (*g).second;
        const GUCEF::vector< TKeyVector >& perComponent = groupKeys[ (*g).first ];
        Float32 first = 1e30f, last = -1e30f;
        for ( size_t a=0; a<perComponent.size(); ++a )
        {
            if ( perComponent[ a ].empty() )
                continue;
            if ( perComponent[ a ][ 0 ].time < first ) first = perComponent[ a ][ 0 ].time;
            if ( perComponent[ a ].back().time > last ) last = perComponent[ a ].back().time;
        }
        if ( first > last )
            continue;
        bool missing = false;
        for ( size_t a=0; a<perComponent.size(); ++a )
            missing = missing || !group.has[ a ];
        if ( missing )
            m_project.Approximate( subject, "the " + group.property + " of " + m_project.GetScene().nodes[ group.node ].name + " is animated in some components only: the others are taken as their defaults" );

        ASSETCONVERTER::CAssetAnimationChannel channel;
        channel.targetNode = group.node;
        channel.path = "position" == group.property ? ASSETCONVERTER::ANIMATIONPATH_TRANSLATION : ( "scale" == group.property ? ASSETCONVERTER::ANIMATIONPATH_SCALE : ASSETCONVERTER::ANIMATIONPATH_ROTATION );
        channel.interpolation = ASSETCONVERTER::ANIMATIONINTERPOLATION_LINEAR;
        const Float32 step = 1.0f / ( sampleRate > 0.0f ? sampleRate : 60.0f );
        for ( Float32 t = first; ; t += step )
        {
            const Float32 time = t > last ? last : t;
            GUCEF::vector< Float32 > value( perComponent.size(), 0.0f );
            for ( size_t a=0; a<perComponent.size(); ++a )
                value[ a ] = group.has[ a ] ? Evaluate( perComponent[ a ], 0, time ) : group.defaults[ a ];
            channel.times.push_back( time );
            if ( "euler" == group.property )
            {
                const MATH::TFloat32Quaternion q = EulerToQuaternion( value[ 0 ], value[ 1 ], value[ 2 ] );
                channel.values.push_back( q.x );
                channel.values.push_back( q.y );
                channel.values.push_back( q.z );
                channel.values.push_back( q.w );
            }
            else
            if ( "rotation" == group.property )
            {
                const MATH::TFloat32Quaternion q = MATH::TFloat32Quaternion::FromXYZW( value[ 0 ], value[ 1 ], value[ 2 ], value[ 3 ] ).Normalized();
                channel.values.push_back( q.x );
                channel.values.push_back( q.y );
                channel.values.push_back( q.z );
                channel.values.push_back( q.w );
            }
            else
            {
                channel.values.insert( channel.values.end(), value.begin(), value.end() );
            }
            if ( time >= last )
                break;
        }
        animation.channels.push_back( channel );
        animation.sampled = true;
        m_project.Approximate( subject, "the " + group.property + " curves of " + m_project.GetScene().nodes[ group.node ].name + " are sampled at the rate of the clip" );
    }

    if ( animation.lastFrame <= animation.firstFrame )
        animation.lastFrame = animation.firstFrame + animation.GetDuration() * sampleRate;
    if ( animation.channels.empty() )
    {
        m_project.Warn( subject, "the clip animates no transform" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityAnimationClipReader::ReadAsset( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    const CUnityYamlFile* file = m_project.GetYaml( vfsPath );
    if ( GUCEF_NULL == file )
        return false;
    const CUnityYamlDocument* document = file->FindFirstOfClass( 74 );
    if ( GUCEF_NULL == document )
    {
        m_project.Warn( m_project.Describe( vfsPath ), "the file holds no AnimationClip" );
        return false;
    }
    ASSETCONVERTER::CAssetScene& scene = m_project.GetScene();
    const UInt32 root = scene.AddNode( "Root", -1 );
    ASSETCONVERTER::CAssetAnimation animation;
    if ( !ReadClip( document->body, m_project.Describe( vfsPath ), root, animation ) )
        return false;
    if ( animation.name.IsNULLOrEmpty() )
        animation.name = ASSETCONVERTER::CVfsPaths::StemOf( vfsPath );
    scene.animations.push_back( animation );
    return true;
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
