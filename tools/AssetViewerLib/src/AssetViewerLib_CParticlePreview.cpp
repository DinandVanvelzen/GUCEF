/*
 *  AssetViewerLib: plays the Ogre particle systems of what the viewer shows
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

#include <Ogre.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <OgreParticleSystemManager.h>
#include <OgreScriptCompiler.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "AssetViewerLib_CParticlePreview.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The longest step of a particle update */
const Float32 StepInSeconds = 1.0f / 30.0f;

/** The material scripts parsed so far, as "<group>|<file>" */
CORE::CString::StringSet&
GetParsedScripts( void )
{
    static CORE::CString::StringSet parsed;
    return parsed;
}

CORE::CString
FolderOf( const CORE::CString& fileName )
{
    const Int32 slash = fileName.HasChar( '/', false );
    return slash >= 0 ? fileName.SubstrFromRange( 0, (UInt32) slash ) : CORE::CString::Empty;
}

/** Parses a material script of a group once: Ogre refuses a material defined twice */
bool
ParseMaterialOnce( const CORE::CString& groupName, const CORE::CString& fileName )
{
    const CORE::CString key = groupName + "|" + fileName;
    if ( GetParsedScripts().find( key ) != GetParsedScripts().end() )
        return true;
    try
    {
        Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource( fileName.STL_String(), groupName.STL_String() );
        Ogre::MaterialManager::getSingleton().parseScript( stream, groupName.STL_String() );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CParticlePreview: The script " << fileName << " cannot be parsed: " << e.getDescription() );
        return false;
    }
    GetParsedScripts().insert( key );
    return true;
}

/** The names of the templates of a particle script: the words after "particle_system", without quotes */
void
GetTemplateNames( const CORE::CString& text, CORE::CString::StringVector& names )
{
    names.clear();
    const CORE::CString::StringVector lines = text.ParseElements( '\n', false );
    for ( size_t l=0; l<lines.size(); ++l )
    {
        const CORE::CString line = lines[ l ].Trim( true ).Trim( false );
        if ( !line.StartsWith( "particle_system " ) )
            continue;
        CORE::CString name = line.SubstrFromRange( 16, line.Length() ).Trim( true ).Trim( false );
        if ( name.Length() >= 2 && '"' == name[ 0 ] && '"' == name[ name.Length() - 1 ] )
            name = name.SubstrFromRange( 1, name.Length() - 1 );
        if ( !name.IsNULLOrEmpty() )
            names.push_back( name );
    }
}

/**
 *  Parses a particle script: templates are global in Ogre, so the templates of the same names that an earlier parse
 *  made (another mount of the same files, or the file before it was cooked again) are replaced
 */
bool
ParseParticleText( const CORE::CString& groupName, const CORE::CString& fileName, CORE::CString::StringVector& names )
{
    try
    {
        Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource( fileName.STL_String(), groupName.STL_String() );
        const Ogre::String text = stream->getAsString();
        GetTemplateNames( CORE::CString( text ), names );
        Ogre::ParticleSystemManager& particles = Ogre::ParticleSystemManager::getSingleton();
        for ( size_t n=0; n<names.size(); ++n )
            if ( GUCEF_NULL != particles.getTemplate( names[ n ].STL_String() ) )
                particles.removeTemplate( names[ n ].STL_String() );
        Ogre::DataStreamPtr copy( OGRE_NEW Ogre::MemoryDataStream( fileName.STL_String(), (void*) text.c_str(), text.size(), false, true ) );
        Ogre::ScriptCompilerManager::getSingleton().parseScript( copy, groupName.STL_String() );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CParticlePreview: The script " << fileName << " cannot be parsed: " << e.getDescription() );
        return false;
    }
    return true;
}

/** A name for a system the preview makes: unique in the scene managers */
Ogre::String
MakeSystemName( const CORE::CString& templateName )
{
    static UInt32 counter = 0;
    ++counter;
    return ( "Preview#" + CORE::UInt32ToString( counter ) + "#" + templateName ).STL_String();
}

Float32
GetPrewarm( Ogre::SceneNode* node )
{
    if ( GUCEF_NULL == node )
        return 0.0f;
    const Ogre::Any value = node->getUserObjectBindings().getUserAny( "particlePrewarm" );
    if ( !value.has_value() )
        return 0.0f;
    try
    {
        return (Float32) Ogre::any_cast< Ogre::Real >( value );
    }
    catch ( ... )
    {
        return 0.0f;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CParticlePreview::CParticlePreview( void )
    : m_systems()
    , m_ownedNodes()
    , m_ownedSceneManager( GUCEF_NULL )
    , m_time( 0.0f )
    , m_playing( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CParticlePreview::~CParticlePreview()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CParticlePreview::ParseScriptsNear( const CORE::CString& groupName, const CORE::CString& fileName )
{GUCEF_TRACE;

    if ( GUCEF_NULL == Ogre::ParticleSystemManager::getSingletonPtr() )
        return 0;
    Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
    const CORE::CString folder = FolderOf( fileName );
    const CORE::CString prefix = folder.IsNULLOrEmpty() ? CORE::CString::Empty : folder + "/";
    Ogre::StringVectorPtr particleScripts;
    Ogre::StringVectorPtr materialScripts;
    try
    {
        particleScripts = groups.findResourceNames( groupName.STL_String(), ( prefix + "*.particle" ).STL_String() );
        if ( !particleScripts || particleScripts->empty() )
            return 0;
        materialScripts = groups.findResourceNames( groupName.STL_String(), ( prefix + "*.material" ).STL_String() );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CParticlePreview: The scripts next to " << fileName << " cannot be listed: " << e.getDescription() );
        return 0;
    }

    // the materials first: the templates name them
    if ( materialScripts )
        for ( size_t i=0; i<materialScripts->size(); ++i )
            ParseMaterialOnce( groupName, (*materialScripts)[ i ] );
    UInt32 parsed = 0;
    CORE::CString::StringVector names;
    for ( size_t i=0; i<particleScripts->size(); ++i )
        if ( ParseParticleText( groupName, (*particleScripts)[ i ], names ) )
            ++parsed;
    return parsed;
}

/*-------------------------------------------------------------------------*/

bool
CParticlePreview::ParseParticleScript( const CORE::CString& groupName, const CORE::CString& fileName, CORE::CString::StringVector& templates )
{GUCEF_TRACE;

    templates.clear();
    if ( GUCEF_NULL == Ogre::ParticleSystemManager::getSingletonPtr() )
        return false;

    // the materials next to the script first; this script is parsed with the others there (or alone)
    if ( 0 == ParseScriptsNear( groupName, fileName ) )
    {
        CORE::CString::StringVector ignored;
        if ( !ParseParticleText( groupName, fileName, ignored ) )
            return false;
    }
    CORE::CString text;
    try
    {
        Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource( fileName.STL_String(), groupName.STL_String() );
        text = stream->getAsString();
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CParticlePreview: The script " << fileName << " cannot be read: " << e.getDescription() );
        return false;
    }
    CORE::CString::StringVector names;
    GetTemplateNames( text, names );
    for ( size_t n=0; n<names.size(); ++n )
        if ( GUCEF_NULL != Ogre::ParticleSystemManager::getSingleton().getTemplate( names[ n ].STL_String() ) )
            templates.push_back( names[ n ] );
    return !templates.empty();
}

/*-------------------------------------------------------------------------*/

void
CParticlePreview::CollectBelow( Ogre::SceneNode* node )
{GUCEF_TRACE;

    for ( size_t i=0; i<node->numAttachedObjects(); ++i )
    {
        Ogre::MovableObject* object = node->getAttachedObject( i );
        if ( "ParticleSystem" != object->getMovableType() )
            continue;
        SSystem entry;
        entry.system = static_cast< Ogre::ParticleSystem* >( object );
        entry.prewarm = GetPrewarm( node );
        entry.owned = false;
        // the time line drives the system, not Ogre's frame time
        entry.system->setSpeedFactor( 0.0f );
        m_systems.push_back( entry );
    }
    Ogre::Node::ChildNodeMap::const_iterator c = node->getChildren().begin();
    while ( c != node->getChildren().end() )
    {
        CollectBelow( static_cast< Ogre::SceneNode* >( *c ) );
        ++c;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CParticlePreview::Collect( Ogre::SceneNode* root )
{GUCEF_TRACE;

    Clear();
    if ( GUCEF_NULL == root )
        return 0;
    CollectBelow( root );
    if ( !m_systems.empty() )
        Restart();
    return (UInt32) m_systems.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CParticlePreview::CreateFromTemplates( Ogre::SceneManager* sceneManager, Ogre::SceneNode* parent, const CORE::CString::StringVector& templates )
{GUCEF_TRACE;

    Clear();
    if ( GUCEF_NULL == sceneManager || GUCEF_NULL == parent )
        return 0;
    m_ownedSceneManager = sceneManager;
    const Float32 spacing = 2.0f;
    const Float32 start = -0.5f * spacing * (Float32) ( templates.size() > 0 ? templates.size() - 1 : 0 );
    for ( size_t t=0; t<templates.size(); ++t )
    {
        try
        {
            Ogre::ParticleSystem* system = sceneManager->createParticleSystem( MakeSystemName( templates[ t ] ), templates[ t ].STL_String() );
            Ogre::SceneNode* node = parent->createChildSceneNode( Ogre::Vector3( start + spacing * (Float32) t, 0.0f, 0.0f ) );
            node->attachObject( system );
            system->setSpeedFactor( 0.0f );
            SSystem entry;
            entry.system = system;
            entry.prewarm = 0.0f;
            entry.owned = true;
            m_systems.push_back( entry );
            m_ownedNodes.push_back( node );
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CParticlePreview: The template " << templates[ t ] << " cannot be made: " << e.getDescription() );
        }
    }
    if ( !m_systems.empty() )
        Restart();
    return (UInt32) m_systems.size();
}

/*-------------------------------------------------------------------------*/

void
CParticlePreview::Clear( void )
{GUCEF_TRACE;

    for ( size_t s=0; s<m_systems.size(); ++s )
    {
        if ( !m_systems[ s ].owned || GUCEF_NULL == m_ownedSceneManager )
            continue;
        Ogre::ParticleSystem* system = m_systems[ s ].system;
        if ( GUCEF_NULL != system->getParentSceneNode() )
            system->getParentSceneNode()->detachObject( system );
        m_ownedSceneManager->destroyParticleSystem( system );
    }
    for ( size_t n=0; n<m_ownedNodes.size(); ++n )
        if ( GUCEF_NULL != m_ownedSceneManager )
            m_ownedSceneManager->destroySceneNode( m_ownedNodes[ n ] );
    m_systems.clear();
    m_ownedNodes.clear();
    m_ownedSceneManager = GUCEF_NULL;
    m_time = 0.0f;
}

/*-------------------------------------------------------------------------*/

UInt32
CParticlePreview::GetSystemCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_systems.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CParticlePreview::GetParticleCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t s=0; s<m_systems.size(); ++s )
        count += (UInt32) m_systems[ s ].system->getNumParticles();
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CParticlePreview::GetLengthInMs( void ) const
{GUCEF_TRACE;

    Float32 longest = 1.0f;
    for ( size_t s=0; s<m_systems.size(); ++s )
    {
        Ogre::ParticleSystem* system = m_systems[ s ].system;
        for ( unsigned short e=0; e<system->getNumEmitters(); ++e )
        {
            const Ogre::ParticleEmitter* emitter = system->getEmitter( e );
            Float32 length = (Float32) emitter->getMaxTimeToLive();
            if ( emitter->getMaxDuration() > 0.0f )
                length += (Float32) emitter->getMaxDuration();
            if ( emitter->getMaxRepeatDelay() > 0.0f )
                length += (Float32) emitter->getMaxRepeatDelay();
            longest = length > longest ? length : longest;
        }
    }
    longest = longest > 60.0f ? 60.0f : longest;
    return (UInt32) ( longest * 1000.0f + 0.5f );
}

/*-------------------------------------------------------------------------*/

void
CParticlePreview::SetPlaying( bool playing )
{GUCEF_TRACE;

    m_playing = playing;
}

/*-------------------------------------------------------------------------*/

bool
CParticlePreview::IsPlaying( void ) const
{GUCEF_TRACE;

    return m_playing;
}

/*-------------------------------------------------------------------------*/

void
CParticlePreview::Advance( Float32 seconds )
{GUCEF_TRACE;

    if ( seconds <= 0.0f || m_systems.empty() )
        return;
    Float32 remaining = seconds;
    while ( remaining > 0.0f )
    {
        const Float32 step = remaining > StepInSeconds ? StepInSeconds : remaining;
        for ( size_t s=0; s<m_systems.size(); ++s )
        {
            Ogre::ParticleSystem* system = m_systems[ s ].system;
            if ( GUCEF_NULL == system->getParentSceneNode() )
                continue;
            system->setSpeedFactor( 1.0f );
            system->_update( step );
            system->setSpeedFactor( 0.0f );
        }
        remaining -= step;
    }
    m_time += seconds;
}

/*-------------------------------------------------------------------------*/

void
CParticlePreview::Restart( void )
{GUCEF_TRACE;

    for ( size_t s=0; s<m_systems.size(); ++s )
    {
        Ogre::ParticleSystem* system = m_systems[ s ].system;
        system->clear();
        for ( unsigned short e=0; e<system->getNumEmitters(); ++e )
            system->getEmitter( e )->setEnabled( true );
    }
    m_time = 0.0f;
    for ( size_t s=0; s<m_systems.size(); ++s )
    {
        if ( m_systems[ s ].prewarm <= 0.0f )
            continue;
        Ogre::ParticleSystem* system = m_systems[ s ].system;
        Float32 remaining = m_systems[ s ].prewarm;
        while ( remaining > 0.0f && GUCEF_NULL != system->getParentSceneNode() )
        {
            const Float32 step = remaining > StepInSeconds ? StepInSeconds : remaining;
            system->setSpeedFactor( 1.0f );
            system->_update( step );
            system->setSpeedFactor( 0.0f );
            remaining -= step;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CParticlePreview::SetTime( Float32 seconds )
{GUCEF_TRACE;

    Restart();
    Advance( seconds );
}

/*-------------------------------------------------------------------------*/

Float32
CParticlePreview::GetTime( void ) const
{GUCEF_TRACE;

    return m_time;
}

/*-------------------------------------------------------------------------*/

bool
CParticlePreview::GetBoundingSphere( COrbitController::SVector3& center, Float32& radius ) const
{GUCEF_TRACE;

    Ogre::AxisAlignedBox box;
    for ( size_t s=0; s<m_systems.size(); ++s )
    {
        const Ogre::AxisAlignedBox& systemBox = m_systems[ s ].system->getWorldBoundingBox( true );
        if ( systemBox.isFinite() && !systemBox.isNull() )
            box.merge( systemBox );
    }
    if ( !box.isFinite() || box.isNull() )
        return false;
    const Ogre::Vector3 middle = box.getCenter();
    center = COrbitController::SVector3( middle.x, middle.y, middle.z );
    radius = 0.5f * box.getSize().length();
    if ( radius < 1e-3f )
        radius = 1.0f;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CParticlePreview::WriteInspectorData( CORE::CDataNode& root ) const
{GUCEF_TRACE;

    if ( m_systems.empty() )
        return;
    CORE::CDataNode* particles = root.AddChild( "Particles" );
    particles->SetAttribute( "Systems", CORE::UInt32ToString( (UInt32) m_systems.size() ) );
    particles->SetAttribute( "Particles", CORE::UInt32ToString( GetParticleCount() ) );
    particles->SetAttribute( "Time", CORE::ToString( m_time ) + " s" );
    particles->SetAttribute( "Playing", m_playing ? "true" : "false" );
    for ( size_t s=0; s<m_systems.size(); ++s )
    {
        const Ogre::ParticleSystem* system = m_systems[ s ].system;
        CORE::CDataNode* entry = particles->AddChild( "System" + CORE::UInt32ToString( (UInt32) s ) );
        entry->SetAttribute( "Name", CORE::CString( system->getName() ) );
        entry->SetAttribute( "Script", CORE::CString( system->getOrigin() ) );
        entry->SetAttribute( "Particles", CORE::UInt32ToString( (UInt32) system->getNumParticles() ) + " of " + CORE::UInt32ToString( (UInt32) system->getParticleQuota() ) );
        entry->SetAttribute( "Emitters", CORE::UInt32ToString( (UInt32) system->getNumEmitters() ) );
        entry->SetAttribute( "Affectors", CORE::UInt32ToString( (UInt32) system->getNumAffectors() ) );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
