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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#include "assetcookerOGRE_CNodeAnimationSampler.h"
#include "assetcookerOGRE_COutputNames.h"
#include "assetcookerOGRE_CDotSceneWriter.h"

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
static const Float32 UnlimitedRange = 100000.0f;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
Number( Ogre::Real value )
{
    return ASSETCONVERTER::CCanonicalJson::FormatFloat32( (Float32) value );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Attribute( const char* name, const CORE::CString& value )
{
    return CORE::CString( " " ) + name + "=\"" + CDotSceneWriter::Escape( value ) + "\"";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDotSceneWriter::CDotSceneWriter( const ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report )
    : m_scene( scene )
    , m_report( report )
    , m_entities()
    , m_particles()
    , m_excluded()
    , m_materialNames()
    , m_prefab( GUCEF_NULL )
    , m_text()
    , m_references()
    , m_names()
    , m_usedNames()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDotSceneWriter::SetEntity( UInt32 node, const CORE::CString& meshName, bool skinned )
{GUCEF_TRACE;

    SEntity entity;
    entity.mesh = meshName;
    entity.skinned = skinned;
    m_entities[ node ] = entity;
}

/*-------------------------------------------------------------------------*/

void
CDotSceneWriter::SetEntityProperty( UInt32 node, const CORE::CString& name, const CORE::CString& value )
{GUCEF_TRACE;

    TEntityMap::iterator entity = m_entities.find( node );
    if ( entity != m_entities.end() )
        (*entity).second.properties.push_back( "<property type=\"str\"" + Attribute( "name", name ) + Attribute( "data", value ) + " />" );
}

/*-------------------------------------------------------------------------*/

void CDotSceneWriter::Exclude( UInt32 node ) { m_excluded.insert( node ); }
void CDotSceneWriter::SetMaterialNames( const CORE::CString::StringVector& materialNames ) { m_materialNames = materialNames; }
const CORE::CString::StringVector& CDotSceneWriter::GetReferences( void ) const { return m_references; }
const CDotSceneWriter::TNameMap& CDotSceneWriter::GetNodeNames( void ) const { return m_names; }

/*-------------------------------------------------------------------------*/

CORE::CString
CDotSceneWriter::Escape( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString result;
    const char* c = text.C_String();
    for ( ; GUCEF_NULL != c && '\0' != *c; ++c )
    {
        switch ( *c )
        {
            case '&' : result += "&amp;"; break;
            case '<' : result += "&lt;"; break;
            case '>' : result += "&gt;"; break;
            case '"' : result += "&quot;"; break;
            case '\'' : result += "&apos;"; break;
            default :
                if ( (unsigned char) *c >= 32 )
                    result += *c;
                break;
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

void
CDotSceneWriter::Line( UInt32 depth, const CORE::CString& text )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<depth; ++i )
        m_text += "    ";
    m_text += text;
    m_text += "\n";
}

/*-------------------------------------------------------------------------*/

void
CDotSceneWriter::AddReference( const CORE::CString& meshName )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_references.size(); ++i )
        if ( m_references[ i ] == meshName )
            return;
    m_references.push_back( meshName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDotSceneWriter::GetName( UInt32 node )
{GUCEF_TRACE;

    TNameMap::const_iterator known = m_names.find( node );
    if ( known != m_names.end() )
        return (*known).second;
    const CORE::CString base = m_scene.nodes[ node ].name.IsNULLOrEmpty() ? "node" + CORE::UInt32ToString( node ) : m_scene.nodes[ node ].name;
    const CORE::CString name = COutputNames::MakeUnique( base, CORE::CString::Empty, m_usedNames );
    m_names[ node ] = name;
    return name;
}

/*-------------------------------------------------------------------------*/

void
CDotSceneWriter::WriteTransform( const CSceneMath::STrs& trs, UInt32 depth, bool always )
{GUCEF_TRACE;

    if ( always || trs.position != Ogre::Vector3::ZERO )
        Line( depth, "<position x=\"" + Number( trs.position.x ) + "\" y=\"" + Number( trs.position.y ) + "\" z=\"" + Number( trs.position.z ) + "\" />" );
    if ( always || trs.rotation != Ogre::Quaternion::IDENTITY )
        Line( depth, "<rotation qx=\"" + Number( trs.rotation.x ) + "\" qy=\"" + Number( trs.rotation.y ) + "\" qz=\"" + Number( trs.rotation.z ) + "\" qw=\"" + Number( trs.rotation.w ) + "\" />" );
    if ( always || trs.scale != Ogre::Vector3::UNIT_SCALE )
        Line( depth, "<scale x=\"" + Number( trs.scale.x ) + "\" y=\"" + Number( trs.scale.y ) + "\" z=\"" + Number( trs.scale.z ) + "\" />" );
}

/*-------------------------------------------------------------------------*/

void
CDotSceneWriter::WriteLight( UInt32 node, UInt32 depth )
{GUCEF_TRACE;

    const Int32 index = m_scene.nodes[ node ].light;
    if ( index < 0 || (size_t) index >= m_scene.lights.size() )
        return;
    const ASSETCONVERTER::CAssetLight& light = m_scene.lights[ index ];
    const char* type = "point";
    if ( ASSETCONVERTER::LIGHTTYPE_DIRECTIONAL == light.type )
        type = "directional";
    else
    if ( ASSETCONVERTER::LIGHTTYPE_SPOT == light.type )
        type = "spot";
    Line( depth, "<light" + Attribute( "name", GetName( node ) ) + Attribute( "type", type ) + Attribute( "powerScale", Number( light.intensity ) ) + ">" );
    const CORE::CString colour = "r=\"" + Number( light.colour[ 0 ] ) + "\" g=\"" + Number( light.colour[ 1 ] ) + "\" b=\"" + Number( light.colour[ 2 ] ) + "\"";
    Line( depth+1, "<colourDiffuse " + colour + " />" );
    Line( depth+1, "<colourSpecular " + colour + " />" );
    if ( ASSETCONVERTER::LIGHTTYPE_DIRECTIONAL != light.type )
    {
        const Float32 range = light.range > 0.0f ? light.range : UnlimitedRange;
        Line( depth+1, "<lightAttenuation range=\"" + Number( range ) + "\" constant=\"1\" linear=\"0\" quadratic=\"1\" />" );
    }
    if ( ASSETCONVERTER::LIGHTTYPE_SPOT == light.type )
        Line( depth+1, "<lightRange inner=\"" + Number( 2.0f * light.innerConeAngle ) + "\" outer=\"" + Number( 2.0f * light.outerConeAngle ) + "\" falloff=\"1\" />" );
    Line( depth, "</light>" );
}

/*-------------------------------------------------------------------------*/

void
CDotSceneWriter::SetParticleSystem( UInt32 node, const CORE::CString& templateName, Float32 prewarm, Float32 startDelay, bool playOnAwake )
{GUCEF_TRACE;

    SParticles particles;
    particles.templateName = templateName;
    particles.prewarm = prewarm;
    particles.startDelay = startDelay;
    particles.playOnAwake = playOnAwake;
    m_particles[ node ] = particles;
}

/*-------------------------------------------------------------------------*/

void
CDotSceneWriter::WriteParticleSystem( UInt32 node, UInt32 depth )
{GUCEF_TRACE;

    TParticleMap::const_iterator particles = m_particles.find( node );
    if ( particles == m_particles.end() )
        return;
    Line( depth, "<particleSystem" + Attribute( "name", GetName( node ) ) + Attribute( "template", (*particles).second.templateName ) + " />" );
}

/*-------------------------------------------------------------------------*/

void
CDotSceneWriter::WriteUserData( UInt32 node, UInt32 depth )
{GUCEF_TRACE;

    const ASSETCONVERTER::CAssetNode& source = m_scene.nodes[ node ];
    CORE::CString::StringVector lines;
    CORE::CString instance = source.prefabInstance;
    if ( instance.IsNULLOrEmpty() && GUCEF_NULL != m_prefab )
        for ( size_t i=0; i<m_prefab->instances.size() && instance.IsNULLOrEmpty(); ++i )
            if ( m_prefab->instances[ i ].node == node )
                instance = m_prefab->instances[ i ].sourcePrefab;
    if ( !instance.IsNULLOrEmpty() )
        lines.push_back( "<property type=\"str\" name=\"prefabInstance\"" + Attribute( "data", instance ) + " />" );
    TParticleMap::const_iterator particles = m_particles.find( node );
    if ( particles != m_particles.end() )
    {
        if ( (*particles).second.prewarm > 0.0f )
            lines.push_back( "<property type=\"float\" name=\"particlePrewarm\"" + Attribute( "data", Number( (*particles).second.prewarm ) ) + " />" );
        if ( (*particles).second.startDelay > 0.0f )
            lines.push_back( "<property type=\"float\" name=\"particleStartDelay\"" + Attribute( "data", Number( (*particles).second.startDelay ) ) + " />" );
        if ( !(*particles).second.playOnAwake )
            lines.push_back( "<property type=\"bool\" name=\"particlePlayOnAwake\" data=\"false\" />" );
    }
    CORE::CDataNode::TAttributeMap::const_iterator a = source.extras.AttributeBegin();
    while ( a != source.extras.AttributeEnd() )
    {
        const CORE::CVariant& value = (*a).second;
        const char* type = "str";
        if ( value.IsBoolean() )
            type = "bool";
        else
        if ( value.IsInteger() )
            type = "int";
        else
        if ( value.IsNumber() )
            type = "float";
        const CORE::CString data = value.IsBoolean() ? CORE::CString( value.AsBool() ? "true" : "false" ) : ( value.IsFloat() ? Number( value.AsFloat32() ) : value.AsString() );
        lines.push_back( "<property" + Attribute( "type", type ) + Attribute( "name", (*a).first ) + Attribute( "data", data ) + " />" );
        ++a;
    }
    if ( lines.empty() )
        return;
    Line( depth, "<userData>" );
    for ( size_t i=0; i<lines.size(); ++i )
        Line( depth+1, lines[ i ] );
    Line( depth, "</userData>" );
}

/*-------------------------------------------------------------------------*/

void
CDotSceneWriter::WriteAnimations( UInt32 node, UInt32 depth )
{GUCEF_TRACE;

    if ( m_scene.IsJoint( node ) )
        return;
    CORE::CString::StringVector parts;
    const CSceneMath::STrs rest = CSceneMath::ToTrs( m_scene.nodes[ node ].transform );
    bool nonLinear = false;
    for ( size_t c=0; c<m_scene.animations.size(); ++c )
    {
        const ASSETCONVERTER::CAssetAnimation& clip = m_scene.animations[ c ];
        CNodeAnimationSampler sampler( m_scene, clip, node );
        if ( !sampler.IsAnimated() )
            continue;
        nonLinear = nonLinear || sampler.HasNonLinearKeys();
        if ( parts.empty() )
            Line( depth, "<animations>" );
        const Float32 length = clip.GetDuration() > 0.0f ? clip.GetDuration() : 0.0f;
        const CORE::CString clipName = clip.name.IsNULLOrEmpty() ? "clip" + CORE::UInt32ToString( (UInt32) c ) : clip.name;
        parts.push_back( clipName );
        Line( depth+1, "<animation" + Attribute( "name", clipName + "/" + GetName( node ) ) + Attribute( "enable", "false" ) + Attribute( "loop", clip.loop ? "true" : "false" ) +
                       Attribute( "interpolationMode", "linear" ) + Attribute( "rotationInterpolationMode", "spherical" ) + Attribute( "length", Number( length ) ) + ">" );
        CNodeAnimationSampler::TFloatVector times;
        sampler.GetKeyTimes( times );
        for ( size_t k=0; k<times.size(); ++k )
        {
            const CSceneMath::STrs pose = sampler.Sample( times[ k ] );
            CSceneMath::STrs offset;
            offset.position = pose.position - rest.position;
            offset.rotation = rest.rotation.Inverse() * pose.rotation;
            offset.rotation.normalise();
            offset.scale = Ogre::Vector3( 0 != rest.scale.x ? pose.scale.x / rest.scale.x : 1, 0 != rest.scale.y ? pose.scale.y / rest.scale.y : 1, 0 != rest.scale.z ? pose.scale.z / rest.scale.z : 1 );
            Line( depth+2, "<keyframe time=\"" + Number( times[ k ] < 0.0f ? 0.0f : times[ k ] ) + "\">" );
            WriteTransform( offset, depth+3, true );
            Line( depth+2, "</keyframe>" );
        }
        Line( depth+1, "</animation>" );
    }
    if ( !parts.empty() )
        Line( depth, "</animations>" );
    if ( nonLinear )
        m_report.AddApproximation( SourceName, m_scene.nodes[ node ].name, "step or cubic spline keys of the node are played back linearly between their values" );
}

/*-------------------------------------------------------------------------*/

void
CDotSceneWriter::WriteNode( UInt32 node, UInt32 depth )
{GUCEF_TRACE;

    if ( node >= m_scene.nodes.size() || m_excluded.find( node ) != m_excluded.end() )
        return;
    const ASSETCONVERTER::CAssetNode& source = m_scene.nodes[ node ];
    const CORE::CString name = GetName( node );
    Line( depth, "<node" + Attribute( "name", name ) + ">" );
    WriteTransform( CSceneMath::ToTrs( source.transform ), depth+1, false );

    TEntityMap::const_iterator entity = m_entities.find( node );
    if ( entity != m_entities.end() && !(*entity).second.skinned )
    {
        // the material overrides of a prefab instance on this node
        CORE::CString::StringVector overrides;
        if ( GUCEF_NULL != m_prefab )
        {
            for ( size_t i=0; i<m_prefab->instances.size(); ++i )
            {
                if ( m_prefab->instances[ i ].node != node )
                    continue;
                CORE::CString::StringVector names;
                m_prefab->instances[ i ].overrides.GetValuesOfChildByName( "materials", names );
                for ( size_t n=0; n<names.size(); ++n )
                {
                    CORE::CString ogreName = names[ n ];
                    for ( size_t m=0; m<m_scene.materials.size() && m<m_materialNames.size(); ++m )
                        if ( m_scene.materials[ m ].name == names[ n ] )
                            ogreName = m_materialNames[ m ];
                    overrides.push_back( ogreName );
                }
            }
        }
        CORE::CString line = "<entity" + Attribute( "name", name ) + Attribute( "meshFile", (*entity).second.mesh );
        bool single = !overrides.empty();
        for ( size_t i=1; i<overrides.size() && single; ++i )
            single = overrides[ i ] == overrides[ 0 ];
        if ( single )
            line += Attribute( "material", overrides[ 0 ] );
        CORE::CString::StringVector properties( (*entity).second.properties );
        if ( !overrides.empty() && !single )
        {
            CORE::CString list;
            for ( size_t i=0; i<overrides.size(); ++i )
                list += ( 0 == i ? "" : ";" ) + overrides[ i ];
            properties.insert( properties.begin(), "<property type=\"str\" name=\"materials\"" + Attribute( "data", list ) + " />" );
        }
        if ( properties.empty() )
        {
            Line( depth+1, line + " />" );
        }
        else
        {
            Line( depth+1, line + ">" );
            Line( depth+2, "<userData>" );
            for ( size_t i=0; i<properties.size(); ++i )
                Line( depth+3, properties[ i ] );
            Line( depth+2, "</userData>" );
            Line( depth+1, "</entity>" );
        }
        AddReference( (*entity).second.mesh );
    }
    WriteLight( node, depth+1 );
    WriteParticleSystem( node, depth+1 );
    WriteUserData( node, depth+1 );
    WriteAnimations( node, depth+1 );
    for ( size_t c=0; c<source.children.size(); ++c )
        WriteNode( source.children[ c ], depth+1 );
    Line( depth, "</node>" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDotSceneWriter::Write( const TUInt32Vector& roots, const ASSETCONVERTER::CAssetPrefab* prefab, Int32 spaceRoot )
{GUCEF_TRACE;

    m_prefab = prefab;
    m_text.Clear();
    m_references.clear();
    m_names.clear();
    m_usedNames.clear();

    Line( 0, "<scene formatVersion=\"1.1\">" );
    Line( 1, "<nodes>" );
    for ( size_t r=0; r<roots.size(); ++r )
        WriteNode( roots[ r ], 2 );

    // skinned meshes are drawn in the space of the scene: their entities hang below a node that undoes the space of the root
    Ogre::Affine3 space = Ogre::Affine3::IDENTITY;
    if ( spaceRoot >= 0 && (size_t) spaceRoot < m_scene.nodes.size() )
        space = CSceneMath::GetWorld( m_scene, m_scene.nodes[ spaceRoot ].parent ).inverse();
    const CSceneMath::STrs spaceTrs = CSceneMath::Decompose( space );
    TEntityMap::const_iterator e = m_entities.begin();
    while ( e != m_entities.end() )
    {
        TNameMap::const_iterator written = m_names.find( (*e).first );
        if ( (*e).second.skinned && written != m_names.end() )
        {
            const CORE::CString holder = COutputNames::MakeUnique( (*written).second + ".skin", CORE::CString::Empty, m_usedNames );
            Line( 2, "<node" + Attribute( "name", holder ) + ">" );
            WriteTransform( spaceTrs, 3, false );
            const CORE::CString line = "<entity" + Attribute( "name", (*written).second ) + Attribute( "meshFile", (*e).second.mesh );
            if ( (*e).second.properties.empty() )
            {
                Line( 3, line + " />" );
            }
            else
            {
                Line( 3, line + ">" );
                Line( 4, "<userData>" );
                for ( size_t i=0; i<(*e).second.properties.size(); ++i )
                    Line( 5, (*e).second.properties[ i ] );
                Line( 4, "</userData>" );
                Line( 3, "</entity>" );
            }
            Line( 2, "</node>" );
            AddReference( (*e).second.mesh );
        }
        ++e;
    }
    Line( 1, "</nodes>" );
    Line( 0, "</scene>" );
    m_prefab = GUCEF_NULL;
    return m_text;
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
